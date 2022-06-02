//===-- FeroInstrInfo.h - Fero Instruction Information ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Fero_FeroINSTRINFO_H
#define LLVM_LIB_TARGET_Fero_FeroINSTRINFO_H

#include "Fero.h"
#include "FeroRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "FeroGenInstrInfo.inc"

namespace llvm {

class FeroInstrInfo : public FeroGenInstrInfo {
public:
  explicit FeroInstrInfo(const FeroSubtarget &STI);
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   Register SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MI,
                                  Register DestReg, int FrameIndex,
                                  const TargetRegisterClass *RC,
                                  const TargetRegisterInfo *TRI) const override;
  /// Emit instructions to copy a pair of physical registers.
  ///
  /// This function should support copies within any legal register class as
  /// well as any cross-class copies created during instruction selection.
  ///
  /// The source and destination registers may overlap, which may require a
  /// careful implementation when multiple copy instructions are required for
  /// large registers. See for example the ARM target.
  void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, const DebugLoc &DL,
                           MCRegister DestReg, MCRegister SrcReg,
                           bool KillSrc) const override;

protected:
  const FeroSubtarget &Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_Fero_FeroINSTRINFO_H
