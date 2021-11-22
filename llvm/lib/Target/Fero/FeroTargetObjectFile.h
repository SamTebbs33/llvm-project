//===-- FeroTargetObjectFile.h - Fero Object Info -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_FeroTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_Fero_FeroTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class FeroTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
};
} // end namespace llvm

#endif // end LLVM_LIB_TARGET_Fero_FeroTARGETOBJECTFILE_H
