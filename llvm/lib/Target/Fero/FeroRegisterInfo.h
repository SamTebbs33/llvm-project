//===-- FeroRegisterInfo.h - Fero Register Information Impl -----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Fero_FeroREGISTERINFO_H
#define LLVM_LIB_TARGET_Fero_FeroREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "FeroGenRegisterInfo.inc"

namespace llvm {
class FeroSubtarget;

class FeroRegisterInfo : public FeroGenRegisterInfo {
protected:
  const FeroSubtarget &Subtarget;

public:
  FeroRegisterInfo(const FeroSubtarget &Subtarget);

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;
  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;
  bool requiresFrameIndexReplacementScavenging(
                                    const MachineFunction &MF) const override;

  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  const TargetRegisterClass *intRegClass(unsigned Size) const;
};

} // end namespace llvm

#endif // end LLVM_LIB_TARGET_Fero_FeroREGISTERINFO_H
