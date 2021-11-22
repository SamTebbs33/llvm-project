add_llvm_component_library(LLVMFeroInfo
  FeroTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Fero
)
