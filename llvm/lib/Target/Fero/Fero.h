//=== Fero.h - Top-level interface for Fero representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Fero backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_Fero_H
#define LLVM_LIB_TARGET_Fero_Fero_H

#include "llvm/PassRegistry.h"
#include "MCTargetDesc/FeroMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class FeroTargetMachine;
  class FunctionPass;

  // Declare functions to create passes here!
  FunctionPass *createFeroMICombinePass();
  void initializeFeroMICombinePass(PassRegistry &);
  FunctionPass *createFeroPseudoExpansionPass();
  void initializeFeroPseudoExpansionPass(PassRegistry &);

} // end namespace llvm;

#endif // end LLVM_LIB_TARGET_Fero_Fero_H
