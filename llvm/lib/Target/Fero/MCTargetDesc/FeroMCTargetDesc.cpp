//===-- FeroMCTargetDesc.cpp - Fero Target Descriptions -----------------===//
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

#include "FeroMCTargetDesc.h"
#include "FeroInstPrinter.h"
#include "FeroMCAsmInfo.h"
#include "TargetInfo/FeroTargetInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "FeroGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "FeroGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "FeroGenRegisterInfo.inc"

static MCInstrInfo *createFeroMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitFeroMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createFeroMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *
createFeroMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";
  return createFeroMCSubtargetInfoImpl(TT, CPUName, CPUName, FS);
}

static MCInstPrinter *createFeroMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new FeroInstPrinter(MAI, MII, MRI);
}

static MCAsmInfo *createFeroMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new FeroMCAsmInfo(TT);

  unsigned WP = MRI.getDwarfRegNum(Fero::R2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, WP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

extern "C" void LLVMInitializeFeroTargetMC() {
  for (Target *T : {&getTheFeroTarget()}) {
    // Register the MC asm info.
    TargetRegistry::RegisterMCAsmInfo(*T, createFeroMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createFeroMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createFeroMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createFeroMCSubtargetInfo);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createFeroMCInstPrinter);
  }
}
