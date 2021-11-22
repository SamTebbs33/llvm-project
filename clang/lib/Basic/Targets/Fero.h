//===--- Fero.h - Declare Fero target feature support ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares FeroTargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_Fero_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_Fero_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY FeroTargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];

public:
  FeroTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
    : TargetInfo(Triple) {
    // Description string has to be kept in sync with backend string at
    // llvm/lib/Target/Fero/FeroTargetMachine.cpp
    resetDataLayout("e"
                    // ELF name mangling
                    "-m:e"
                    // 16-bit pointers, 16-bit aligned
                    "-p:16:16"
                    // 16-bit integers, 16-bit aligned
                    "-i16:16"
                    // 16-bit native integer width i.e register are 16-bit
                    "-n16"
                    // 128-bit natural stack alignment
                    "-S128"
    );
    SuitableAlign = 128;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
    IntWidth = IntAlign = 16;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return false;
  }

  const char *getClobbers() const override {
    return "";
  }
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_Fero_H
