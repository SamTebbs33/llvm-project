//===-- FeroInstrInfo.cpp - Fero Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Fero implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "FeroInstrInfo.h"

#include "FeroTargetMachine.h"
#include "FeroMachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "fero-instrinfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "FeroGenInstrInfo.inc"

FeroInstrInfo::FeroInstrInfo(const FeroSubtarget &STI)
    : FeroGenInstrInfo(Fero::ADJCALLSTACKDOWN, Fero::ADJCALLSTACKUP),
      Subtarget(STI)
{
}

void FeroInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI,
                               Register SrcReg, bool isKill, int FrameIndex,
                               const TargetRegisterClass *RC,
                               const TargetRegisterInfo *TRI) const {
  assert(Fero::GPRRegClass.hasSubClassEq(RC) && "Register class not handled!");
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  if (!MBB.isLiveIn(Fero::R14)) MBB.addLiveIn(Fero::R14);
  BuildMI(MBB, MI, DL, get(Fero::STR)).addReg(Fero::R14).addImm(FrameIndex).addReg(SrcReg, getKillRegState(isKill));
}

void FeroInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                Register DestReg, int FrameIndex,
                                const TargetRegisterClass *RC,
                                const TargetRegisterInfo *TRI) const {
  assert(Fero::GPRRegClass.hasSubClassEq(RC) && "Register class not handled!");
  if (!MBB.isLiveIn(Fero::R14)) MBB.addLiveIn(Fero::R14);
  BuildMI(MBB, MI, MI->getDebugLoc(), get(Fero::MOV_indirect)).addDef(DestReg).addReg(Fero::R14).addImm(FrameIndex);
}

void FeroInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                       MachineBasicBlock::iterator MI, const DebugLoc &DL,
                       MCRegister DestReg, MCRegister SrcReg,
                       bool KillSrc) const {
  BuildMI(MBB, MI, MI->getDebugLoc(), get(Fero::MOV)).addDef(DestReg).addReg(SrcReg, getKillRegState(KillSrc));
}
