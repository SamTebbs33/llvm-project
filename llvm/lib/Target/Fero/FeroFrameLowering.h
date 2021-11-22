//===-- FeroFrameLowering.h - Define frame lowering for Fero ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the FeroTargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_FeroFRAMELOWERING_H
#define LLVM_LIB_TARGET_Fero_FeroFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
  class FeroSubtarget;

class FeroFrameLowering : public TargetFrameLowering {
protected:
  const FeroSubtarget &STI;

public:
  explicit FeroFrameLowering(const FeroSubtarget &STI)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                          /*StackAlignment*/Align(4),
                          /*LocalAreaOffset*/0,
                          /*TransAl*/Align(4)),
      STI(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  bool hasFP(const MachineFunction &MF) const override;
};
} // end llvm namespace

#endif // end LLVM_LIB_TARGET_Fero_FeroFRAMELOWERING_H
