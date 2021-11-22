add_llvm_component_library(LLVMFeroDesc
  FeroMCAsmInfo.cpp
  FeroMCTargetDesc.cpp
  FeroInstPrinter.cpp

  LINK_COMPONENTS
  MC
  FeroInfo
  Support

  ADD_TO_COMPONENT
  Fero
)
