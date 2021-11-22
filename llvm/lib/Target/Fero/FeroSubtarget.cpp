//===-- FeroSubtarget.cpp - Fero Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Fero specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Fero.h"
#include "FeroSubtarget.h"
#include "FeroMachineFunction.h"
#include "FeroRegisterInfo.h"
#include "FeroTargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "fero-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "FeroGenSubtargetInfo.inc"

FeroSubtarget::FeroSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                               const TargetMachine &TM)
    : FeroGenSubtargetInfo(TT, CPU, CPU, FS),
      TSInfo(),
      InstrInfo(initializeSubtargetDependencies(TT, CPU, FS, TM)),
      FrameLowering(*this),
      TLInfo(TM, *this),
      RegInfo(*this) { }


FeroSubtarget &
FeroSubtarget::initializeSubtargetDependencies(const Triple &TT, StringRef CPU,
                                                StringRef FS,
                                                const TargetMachine &TM) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, CPUName, FS);

  return *this;
}
