//===-- FeroMCAsmInfo.h - Fero Asm Info ------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the FeroMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCASMINFO_H
#define LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class Triple;

class FeroMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit FeroMCAsmInfo(const Triple &TheTriple,
                          const MCTargetOptions &Options);
};

} // namespace llvm

#endif // end LLVM_LIB_TARGET_Fero_MCTARGETDESC_FeroMCASMINFO_H
