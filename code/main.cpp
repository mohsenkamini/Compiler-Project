
#include "lexer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include "AST.h"
#include <string>
#include "code_generator.h"
#include "parser.h"
#include "semantic.h"

using namespace std;

// Input in llvm format
static llvm::cl::opt<std::string> Input(llvm::cl::Positional,
										llvm::cl::desc("<input expression>"),
										llvm::cl::init(""));

static llvm::cl::opt<std::string> FileName("f",
										   llvm::cl::desc("<Specify the file name>"),
										   llvm::cl::value_desc("filename"),
										   llvm::cl::init(""));

int main(int argc, const char **argv)
{
	// parse command line with builtin llvm function
	llvm::InitLLVM X(argc, argv);
	llvm::cl::ParseCommandLineOptions(argc, argv, "MAS-Lang Compiler\n");

	string contentString;
	llvm::StringRef contentRef;

	if (!FileName.empty()) // if filename is specified
	{
		std::string fileName = FileName;

		// Use llvm::MemoryBuffer::getFile with the fileName
		llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
			llvm::MemoryBuffer::getFile(fileName);

		if (auto error = fileOrErr.getError())
		{
			llvm::errs() << "Error opening file: " << error.message() << "\n";
			return 1;
		}
		// Use the file content from the MemoryBuffer
		contentString = (*fileOrErr)->getBuffer().str();
	}
	else // if input is given directly
	{
		contentString = Input;
	}

	contentRef = contentString;
	Token nextToken;
	Lexer lexer(contentRef);
	Parser Parser(lexer);
	std::unique_ptr<ProgramNode> TreePtr = Parser.parseProgram();
	ProgramNode *Tree = TreePtr.get();

	Semantic semantic;
	if (semantic.semantic(Tree))
	{
		llvm::errs() << "Semantic errors occurred...\n";
		return 1;
	}

	CodeGen CodeGenerator;
	bool optimize = true;
	int k = 2;
	CodeGenerator.compile(Tree, optimize, k);
	return 0;
}
