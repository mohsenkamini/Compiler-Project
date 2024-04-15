add_executable (compiler
  main.cpp
  #CodeGen.cpp
  Lexer.cpp
  Parser.cpp
  #Sema.cpp
  Error.cpp
  )
target_link_libraries(compiler PRIVATE ${llvm_libs})
