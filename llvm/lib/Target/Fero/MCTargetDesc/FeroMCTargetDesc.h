//===-- FeroMCTargetDesc.h - Fero Target Descriptions ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Fero specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCTARGETDESC_H
#define LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCTARGETDESC_H

// Defines symbolic names for Fero registers. This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "FeroGenRegisterInfo.inc"

// Defines symbolic names for the Fero instructions.
#define GET_INSTRINFO_ENUM
#include "FeroGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "FeroGenSubtargetInfo.inc"

#endif // end LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCTARGETDESC_H
