add_llvm_component_group(Fero)

set(LLVM_TARGET_DEFINITIONS Fero.td)

tablegen(LLVM FeroGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM FeroGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM FeroGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM FeroGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM FeroGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM FeroGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM FeroGenCallingConv.inc -gen-callingconv)
tablegen(LLVM FeroGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM FeroGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM FeroGenAsmMatcher.inc -gen-asm-matcher)

# FeroCommonTableGen must be defined
add_public_tablegen_target(FeroCommonTableGen)

# FeroCodeGen should match with LLVMBuild.txt FeroCodeGen
add_llvm_target(FeroCodeGen
  FeroAsmPrinter.cpp
  FeroFrameLowering.cpp
  FeroInstrInfo.cpp
  FeroISelLowering.cpp
  FeroRegisterInfo.cpp
  FeroSubtarget.cpp
  FeroTargetObjectFile.cpp
  FeroISelDAGToDAG.cpp
  FeroTargetMachine.cpp
  FeroPseudoExpansion.cpp
  FeroMICombine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  FeroDesc
  FeroInfo
  SelectionDAG
  Support
  Target
  GlobalISel

  ADD_TO_COMPONENT
  Fero
)

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)
