//===--- Fero.cpp - Implement Fero target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements FeroTargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Fero.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

const char *const FeroTargetInfo::GCCRegNames[] = {
  // Integer registers
  "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
};

const TargetInfo::GCCRegAlias GCCRegAliases[] = {
  {{}, "r0"}, {{}, "r1"},   {{}, "r2"},    {{}, "r3"},
  {{}, "r4"},   {{}, "r5"},   {{}, "r6"},    {{}, "r7"},
  {{}, "r8"},   {{}, "r9"},   {{}, "r10"},   {{}, "r11"},
  {{}, "r12"},  {{}, "r13"},  {{}, "r14"},   {{}, "r15"}};

ArrayRef<const char *> FeroTargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> FeroTargetInfo::getGCCRegAliases() const {
  return llvm::makeArrayRef(GCCRegAliases);
}

void FeroTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  // Define the __Fero__ macro when building for this target
  Builder.defineMacro("__Fero__");
}
