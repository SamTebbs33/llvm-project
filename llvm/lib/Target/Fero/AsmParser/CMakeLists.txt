add_llvm_component_library(LLVMFeroAsmParser
  FeroAsmParser.cpp

  LINK_COMPONENTS
  FeroDesc
  FeroInfo
  MC
  MCParser
  Support

  ADD_TO_COMPONENT
  Fero
  )
