//===-- FeroISelLowering.cpp - Fero DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Fero uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//
#include "FeroISelLowering.h"
#include "FeroSubtarget.h"
#include "FeroTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "fero-isellower"

#include "FeroGenCallingConv.inc"

FeroTargetLowering::FeroTargetLowering(const TargetMachine &TM,
                                         const FeroSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI)
{
  // Set up the register classes
  addRegisterClass(MVT::i16, &Fero::GPRRegClass);

  // Must, computeRegisterProperties - Once all of the register classes are
  // added, this allows us to compute derived properties we expose.
  computeRegisterProperties(Subtarget.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(Fero::R2);

  // Set scheduling preference. There are a few options:
  //    - None: No preference
  //    - Source: Follow source order
  //    - RegPressure: Scheduling for lowest register pressure
  //    - Hybrid: Scheduling for both latency and register pressure
  // Source (the option used by XCore) is no good when there are few registers
  // because the compiler will try to keep a lot more things into the register
  // which eventually results in a lot of stack spills for no good reason. So
  // use either RegPressure or Hybrid
  setSchedulingPreference(Sched::RegPressure);

  // Use i32 for setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  setOperationAction(ISD::BlockAddress,  MVT::i16, Custom);
  setOperationAction(ISD::ConstantPool,  MVT::i16, Custom);

  setOperationAction(ISD::BR_CC, MVT::i16, Expand);

  // Set minimum and preferred function alignment (log2)
  setMinFunctionAlignment(Align(1));
  setPrefFunctionAlignment(Align(1));

  // Set preferred loop alignment (log2)
  setPrefLoopAlignment(Align(1));
}

const char *FeroTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case FeroISD::Ret: return "FeroISD::Ret";
  case FeroISD::JmpAL: return "FeroISD::JmpAL";
  case FeroISD::JmpNorZ: return "FeroISD::JmpNorZ";
  case FeroISD::JmpC: return "FeroISD::JmpC";
  case FeroISD::JmpNotNNotZ: return "FeroISD::JmpNotNNotZ";
  case FeroISD::JmpNotN: return "FeroISD::JmpNotN";
  case FeroISD::JmpNotZ: return "FeroISD::JmpNotZ";
  case FeroISD::JmpNotC: return "FeroISD::JmpNotC";
  case FeroISD::JmpZ: return "FeroISD::JmpZ";
  case FeroISD::JmpN: return "FeroISD::JmpN";
  default:            return NULL;
  }
}

void FeroTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  default:
    llvm_unreachable("Don't know how to custom expand this!");
  }
}

//===----------------------------------------------------------------------===//
//@            Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

// The BeyondRISC calling convention parameter registers.
static const MCPhysReg GPRArgRegs[] = {
  Fero::R0, Fero::R1, Fero::R2, Fero::R3
};

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue FeroTargetLowering::LowerFormalArguments(
                                    SDValue Chain,
                                    CallingConv::ID CallConv,
                                    bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const
{
  assert((CallingConv::C == CallConv || CallingConv::Fast == CallConv) &&
        "Unsupported CallingConv to FORMAL_ARGS");

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, Fero_CCallingConv);

  SmallVector<SDValue, 16> ArgValues;
  SDValue ArgValue;
  Function::const_arg_iterator CurOrigArg = MF.getFunction().arg_begin();
  unsigned CurArgIdx = 0;

  // Calculate the amount of stack space that we need to allocate to store
  // byval and variadic arguments that are passed in registers.
  // We need to know this before we allocate the first byval or variadic
  // argument, as they will be allocated a stack slot below the CFA (Canonical
  // Frame Address, the stack pointer at entry to the function).
  unsigned ArgRegBegin = Fero::R0;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    if (CCInfo.getInRegsParamsProcessed() >= CCInfo.getInRegsParamsCount())
      break;

    CCValAssign &VA = ArgLocs[i];
    unsigned Index = VA.getValNo();
    ISD::ArgFlagsTy Flags = Ins[Index].Flags;
    if (!Flags.isByVal())
      continue;

    assert(VA.isMemLoc() && "unexpected byval pointer in reg");
    unsigned RBegin, REnd;
    CCInfo.getInRegsParamInfo(CCInfo.getInRegsParamsProcessed(), RBegin, REnd);
    ArgRegBegin = std::min(ArgRegBegin, RBegin);

    CCInfo.nextInRegsParam();
  }
  CCInfo.rewindByValRegsInfo();

  int lastInsIndex = -1;
  if (isVarArg && MFI.hasVAStart()) {
    unsigned RegIdx = CCInfo.getFirstUnallocated(GPRArgRegs);
    if (RegIdx != array_lengthof(GPRArgRegs))
      ArgRegBegin = std::min(ArgRegBegin, (unsigned)GPRArgRegs[RegIdx]);
  }

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (Ins[VA.getValNo()].isOrigArg()) {
      std::advance(CurOrigArg,
                   Ins[VA.getValNo()].getOrigArgIndex() - CurArgIdx);
      CurArgIdx = Ins[VA.getValNo()].getOrigArgIndex();
    }
    // Arguments stored in registers.
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();

      if (VA.needsCustom()) {
        llvm_unreachable("Custom val assignment not supported by "
                         "FORMAL_ARGUMENTS Lowering");
      } else {
        const TargetRegisterClass *RC;

        if (RegVT == MVT::i16)
          RC = &Fero::GPRRegClass;
        else
          llvm_unreachable("RegVT not supported by FORMAL_ARGUMENTS Lowering");

        // Transform the arguments in physical registers into virtual ones.
        unsigned Reg = MF.addLiveIn(VA.getLocReg(), RC);
        ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);
      }

      // If this is an 8-bit value, it is really passed promoted
      // to 16 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::BCvt:
        ArgValue = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::SExt:
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::ZExt:
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      }

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()
      // sanity check
      assert(VA.isMemLoc());
      assert(VA.getValVT() != MVT::i32 && "i32 should already be lowered");

      int index = VA.getValNo();

      // Some Ins[] entries become multiple ArgLoc[] entries.
      // Process them only once.
      if (index != lastInsIndex)
      {
        llvm_unreachable("Cannot retrieve arguments from the stack");
      }
    }
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//@              Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool FeroTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                MachineFunction &MF, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                LLVMContext &Context) const
{
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, Fero_CRetConv);
}

/// LowerMemOpCallTo - Store the argument to the stack.
SDValue FeroTargetLowering::LowerMemOpCallTo(SDValue Chain,
                                              SDValue Arg, const SDLoc &dl,
                                              SelectionDAG &DAG,
                                              const CCValAssign &VA,
                                              ISD::ArgFlagsTy Flags) const {
  llvm_unreachable("Cannot store arguments to stack");
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
FeroTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     const SDLoc &dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals,
                                     bool isThisReturn, SDValue ThisVal) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallResult(Ins, Fero_CRetConv);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign VA = RVLocs[i];

    // Pass 'this' value directly from the argument to return value, to avoid
    // reg unit interference
    if (i == 0 && isThisReturn) {
      assert(!VA.needsCustom() && VA.getLocVT() == MVT::i16 &&
             "unexpected return calling convention register assignment");
      InVals.push_back(ThisVal);
      continue;
    }

    SDValue Val;
    if (VA.needsCustom()) {
        llvm_unreachable("Vector and floating point values not supported yet");
    } else {
      Val = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getLocVT(),
                               InFlag);
      Chain = Val.getValue(1);
      InFlag = Val.getValue(2);
    }

    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), Val);
      break;
    }

    InVals.push_back(Val);
  }

  return Chain;
}

SDValue FeroTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  llvm_unreachable("Cannot lower call");
}

/// HandleByVal - Every parameter *after* a byval parameter is passed
/// on the stack.  Remember the next parameter register to allocate,
/// and then confiscate the rest of the parameter registers to insure
/// this.
void FeroTargetLowering::HandleByVal(CCState *State, unsigned &Size,
                                      Align Alignment) const {
  // Byval (as with any stack) slots are always at least 4 byte aligned.
  Alignment = std::max(Alignment, Align(4));

  unsigned Reg = State->AllocateReg(GPRArgRegs);
  if (!Reg)
    return;

  unsigned AlignInRegs = Alignment.value() / 4;
  unsigned Waste = (Fero::R4 - Reg) % AlignInRegs;
  for (unsigned i = 0; i < Waste; ++i)
    Reg = State->AllocateReg(GPRArgRegs);

  if (!Reg)
    return;

  unsigned Excess = 4 * (Fero::R4 - Reg);

  // Special case when NSAA != SP and parameter size greater than size of
  // all remained GPR regs. In that case we can't split parameter, we must
  // send it to stack. We also must set NCRN to R4, so waste all
  // remained registers.
  const unsigned NSAAOffset = State->getNextStackOffset();
  if (NSAAOffset != 0 && Size > Excess) {
    while (State->AllocateReg(GPRArgRegs))
      ;
    return;
  }

  // First register for byval parameter is the first register that wasn't
  // allocated before this method call, so it would be "reg".
  // If parameter is small enough to be saved in range [reg, r4), then
  // the end (first after last) register would be reg + param-size-in-regs,
  // else parameter would be splitted between registers and stack,
  // end register would be r4 in this case.
  unsigned ByValRegBegin = Reg;
  unsigned ByValRegEnd = std::min<unsigned>(Reg + Size / 4, Fero::R4);
  State->addInRegsParamInfo(ByValRegBegin, ByValRegEnd);
  // Note, first register is allocated in the beginning of function already,
  // allocate remained amount of registers we need.
  for (unsigned i = Reg + 1; i != ByValRegEnd; ++i)
    State->AllocateReg(GPRArgRegs);
  // A byval parameter that is split between registers and memory needs its
  // size truncated here.
  // In the case where the entire structure fits in registers, we set the
  // size in memory to zero.
  Size = std::max<int>(Size - Excess, 0);
}

SDValue
FeroTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv, bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &dl, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of the return value to a location.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slots.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analyze outgoing return values.
  CCInfo.AnalyzeReturn(Outs, Fero_CRetConv);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain); // Operand #0 = Chain (updated below)

  // Copy the result values into the output registers.
  for (unsigned i = 0, realRVLocIdx = 0;
       i != RVLocs.size();
       ++i, ++realRVLocIdx) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Arg = OutVals[realRVLocIdx];
    bool ReturnF16 = false;

    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::BCvt:
      if (!ReturnF16)
        Arg = DAG.getNode(ISD::BITCAST, dl, VA.getLocVT(), Arg);
      break;
    }

    if (VA.needsCustom()) {
      llvm_unreachable("Custom val assignment not supported by "
                       "RETURN Lowering");
    } else {
      Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), Arg, Flag);
    }

    // Guarantee that all emitted copies are stuck together, avoiding something
    // bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(),
                                     ReturnF16 ? MVT::f16 : VA.getLocVT()));
  }

  // Update chain and glue.
  RetOps[0] = Chain;
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(FeroISD::Ret, dl, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

SDValue FeroTargetLowering::getGlobalAddressWrapper(SDValue GA,
                                                     const GlobalValue *GV,
                                                     SelectionDAG &DAG) const {
  llvm_unreachable("Unhandled global variable");
}

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

SDValue FeroTargetLowering::
LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("Unsupported global address");
}

SDValue FeroTargetLowering::
LowerConstantPool(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("Unsupported constant pool");
}

SDValue FeroTargetLowering::
LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("Unsupported block address");
}

SDValue
FeroTargetLowering::LowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const {
  return SDValue();
}

SDValue
FeroTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:        return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:         return LowerBlockAddress(Op, DAG);
  case ISD::ConstantPool:         return LowerConstantPool(Op, DAG);
  case ISD::RETURNADDR:           return LowerRETURNADDR(Op, DAG);
  default: llvm_unreachable("unimplemented operand");
  }
}

FeroISD::CondCode FeroISD::getCondCode(ISD::CondCode Code) {
  switch (Code) {
    case ISD::SETNE:
      return NotZ;
    case ISD::SETEQ:
      return Z;
    case ISD::SETGT:
      return NotNNotZ;
    case ISD::SETGE:
      return NotZ;
    case ISD::SETLT:
      return N;
    case ISD::SETLE:
      return NorZ;
    default:
      llvm_unreachable("unrecognised condition code");
  }
  return Al;
}
