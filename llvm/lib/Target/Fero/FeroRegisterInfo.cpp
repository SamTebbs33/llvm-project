//===-- FeroRegisterInfo.cpp - Fero Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Fero implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "FeroRegisterInfo.h"
#include "FeroSubtarget.h"
#include "llvm/Support/Debug.h"

#define GET_REGINFO_TARGET_DESC
#include "FeroGenRegisterInfo.inc"

#define DEBUG_TYPE "fero-reginfo"

using namespace llvm;

FeroRegisterInfo::FeroRegisterInfo(const FeroSubtarget &ST)
  : FeroGenRegisterInfo(Fero::R1, /*DwarfFlavour*/0, /*EHFlavor*/0,
                         /*PC*/0), Subtarget(ST) {}

const MCPhysReg *
FeroRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return Fero_CalleeSavedRegs_SaveList;
}

const TargetRegisterClass *FeroRegisterInfo::intRegClass(unsigned Size) const {
  return &Fero::GPRRegClass;
}

const uint32_t *
FeroRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID) const {
  return Fero_CalleeSavedRegs_RegMask;
}

BitVector FeroRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  markSuperRegs(Reserved, Fero::R13); // task id
  markSuperRegs(Reserved, Fero::R14); // sp
  markSuperRegs(Reserved, Fero::R15); // return address

  return Reserved;
}

void FeroRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  bool HasFP = TFI->hasFP(MF);
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex) +
               MI.getOperand(FIOperandNum + 1).getImm();

  // Addressable stack objects are addressed using neg. offsets from fp
  // or pos. offsets from sp/basepointer
  if (!HasFP || (hasStackRealignment(MF) && FrameIndex >= 0))
    Offset += MF.getFrameInfo().getStackSize();

  Register FrameReg = Fero::R14;
  Register Dest = MI.getOperand(0).getReg();

  if (Offset == 0) {
    BuildMI(*MI.getParent(), II, DL, TII->get(Fero::MOV), Dest).addReg(FrameReg);
  } else {
    BuildMI(*MI.getParent(), II, DL, TII->get(Fero::LD), Dest).addImm(Offset);
    BuildMI(*MI.getParent(), II, DL, TII->get(Fero::ADD), Dest)
      .addReg(Dest)
      .addReg(FrameReg);
  }
  MI.eraseFromParent();
}

bool
FeroRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

bool
FeroRegisterInfo::requiresFrameIndexScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
FeroRegisterInfo::requiresFrameIndexReplacementScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
FeroRegisterInfo::trackLivenessAfterRegAlloc(const MachineFunction &MF) const {
  return true;
}

Register FeroRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  llvm_unreachable("Unsupported getFrameRegister");
}
