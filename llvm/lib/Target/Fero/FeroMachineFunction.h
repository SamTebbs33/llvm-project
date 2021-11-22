//=== FeroMachineFunctionInfo.h - Private data used for Fero ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Fero specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_FeroMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_Fero_FeroMACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// FeroFunctionInfo - This class is derived from MachineFunction private
/// Fero target-specific information for each MachineFunction.
class FeroFunctionInfo : public MachineFunctionInfo {
private:
  MachineFunction &MF;

public:
  FeroFunctionInfo(MachineFunction &MF) : MF(MF) {}
};

} // end of namespace llvm

#endif // end LLVM_LIB_TARGET_Fero_FeroMACHINEFUNCTION_H
