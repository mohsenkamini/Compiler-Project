add_executable (compiler
  main.cpp
  #CodeGen.cpp
  lexer.cpp
  parser.cpp
  #Sema.cpp
  )
target_link_libraries(compiler PRIVATE ${llvm_libs})
