//===-- FeroTargetMachine.cpp - Define TargetMachine for Fero -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Fero target spec.
//
//===----------------------------------------------------------------------===//

#include "FeroTargetMachine.h"
#include "FeroISelDAGToDAG.h"
#include "FeroSubtarget.h"
#include "FeroTargetObjectFile.h"
#include "TargetInfo/FeroTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeFeroTarget() {
  // Register the target.
  //- Little endian Target Machine
  RegisterTargetMachine<FeroTargetMachine> X(getTheFeroTarget());
}

static std::string computeDataLayout() {
  std::string Ret = "";

  // Little endian
  Ret += "e";

  // ELF name mangling
  Ret += "-m:e";

  // 16-bit pointers, 16-bit aligned
  Ret += "-p:16:16";

  // 16-bit integers, 16 bit aligned
  Ret += "-i16:16";

  // 16-bit native integer width i.e register are 16-bit
  Ret += "-n16";

  // 128-bit natural stack alignment
  Ret += "-S128";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(Optional<CodeModel::Model> CM,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

FeroTargetMachine::FeroTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL,
                                       bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(CM, RM),
                        getEffectiveCodeModel(CM, CodeModel::Medium), OL),
      TLOF(std::make_unique<FeroTargetObjectFile>()) {
  // initAsmInfo will display features by llc -march=fero on 3.7
  initAsmInfo();
}

const FeroSubtarget *
FeroTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<FeroSubtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

namespace {
class FeroPassConfig : public TargetPassConfig {
public:
  FeroPassConfig(FeroTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  FeroTargetMachine &getFeroTargetMachine() const {
    return getTM<FeroTargetMachine>();
  }

  bool addInstSelector() override;
  void addPreEmitPass() override;
};
}

TargetPassConfig *FeroTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new FeroPassConfig(*this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen Fero code.
bool FeroPassConfig::addInstSelector() {
  addPass(new FeroDAGToDAGISel(getFeroTargetMachine(), getOptLevel()));
  return false;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
void FeroPassConfig::addPreEmitPass() {
  addPass(createFeroPseudoExpansionPass());
  addPass(createFeroMICombinePass());
}
