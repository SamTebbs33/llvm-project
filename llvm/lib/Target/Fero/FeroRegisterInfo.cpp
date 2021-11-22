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

  markSuperRegs(Reserved, Fero::R0); // zero
  markSuperRegs(Reserved, Fero::R2); // sp
  markSuperRegs(Reserved, Fero::R3); // gp
  markSuperRegs(Reserved, Fero::R4); // tp

  return Reserved;
}

void FeroRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  llvm_unreachable("Unsupported eliminateFrameIndex");
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

