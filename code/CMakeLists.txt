add_executable (compiler
  main.cpp
  code_generator.cpp
  lexer.cpp
  parser.cpp
  semantic.cpp
  error.cpp
  )
target_link_libraries(compiler PRIVATE ${llvm_libs})
