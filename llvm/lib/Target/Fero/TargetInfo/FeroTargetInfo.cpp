//===-- FeroTargetInfo.cpp - Fero Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/FeroTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheFeroTarget() {
  static Target TheFeroTarget;
  return TheFeroTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeFeroTargetInfo() {
  RegisterTarget<Triple::fero> X(getTheFeroTarget(), "fero",
                                  "16-bit Fero", "Fero");
}
