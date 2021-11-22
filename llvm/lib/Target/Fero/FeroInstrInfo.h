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

protected:
  const FeroSubtarget &Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_Fero_FeroINSTRINFO_H
