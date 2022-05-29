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
  // TODO Store using offset from frame index
  BuildMI(MBB, MI, DL, get(Fero::STR)).addReg(Fero::SP).addReg(SrcReg, getKillRegState(isKill));
}

void FeroInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                Register DestReg, int FrameIndex,
                                const TargetRegisterClass *RC,
                                const TargetRegisterInfo *TRI) const {
  assert(Fero::GPRRegClass.hasSubClassEq(RC) && "Register class not handled!");
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  // TODO Store using offset from frame index
  BuildMI(MBB, MI, MI->getDebugLoc(), get(Fero::MOV_indirect)).addReg(DestReg).addReg(Fero::SP);
}
