#include "lexer.h"
#include <string>


namespace charinfo
{
	LLVM_READNONE inline bool isWhitespace(char c)
	{
		return c == ' ' || c == '\t' || c == '\f' ||
			   c == '\v' || c == '\r' || c == '\n';
	}

	LLVM_READNONE inline bool isDigit(char c)
	{
		return c >= '0' && c <= '9';
	}

	LLVM_READNONE inline bool isLetter(char c)
	{
		return (c >= 'a' && c <= 'z') ||
			   (c >= 'A' && c <= 'Z');
	}

	LLVM_READNONE inline bool isOperator(char c)
	{
		return c == '+' || c == '-' || c == '*' ||
			   c == '/' || c == '^' || c == '=' ||
			   c == '%' || c == '<' || c == '>' ||
			   c == '!';
	}

	LLVM_READNONE inline bool isSpecialCharacter(char c)
	{
		return c == ';' || c == ',' || c == '(' ||
			   c == ')' || c == '{' || c == '}' ||  c == ',';
	}
}

void Lexer::next(Token &token)
{
	// Skips whitespace like " "
	while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
	{
		++BufferPtr;
	}

	// since end of context is 0 -> !0 = true -> end of context
	if (!*BufferPtr)
	{
		token.Kind = Token::eof;
		return;
	}

	// looking for keywords or identifiers like "int", a123 , ...
	if (charinfo::isLetter(*BufferPtr))
	{
		const char *end = BufferPtr + 1;

		// until reaches the end of lexeme
		// example: ".int " -> "i.nt " -> "in.t " -> "int. "
		while (charinfo::isLetter(*end) || charinfo::isDigit(*end))
		{
			++end;
		}

		llvm::StringRef Context(BufferPtr, end - BufferPtr); // start of lexeme, length of lexeme
		Token::TokenKind kind;
		if (Context == "int")
		{
			kind = Token::KW_int;
		}
		else if (Context == "if")
		{
			kind = Token::KW_if;
		}
		else if (Context == "bool")
		{
			kind = Token::KW_bool;
		}
		else if (Context == "else")
		{
			kind = Token::KW_else;
		}
		else if (Context == "while")
		{
			kind = Token::KW_while;
		}
		else if (Context == "for")
		{
			kind = Token::KW_for;
		}
		else if (Context == "and")
		{
			kind = Token::KW_and;
		}
		else if (Context == "or")
		{
			kind = Token::KW_or;
		}
		else if (Context == "true")
		{
			kind = Token::KW_true;
		}
		else if (Context == "false")
		{
			kind = Token::KW_false;
		}
		else if (Context == "print")
		{
			kind = Token::KW_print;
		}
		else
		{
			kind = Token::identifier;
		}

		formToken(token, end, kind);
		return;
	}

	else if (charinfo::isDigit(*BufferPtr))
	{

		const char *end = BufferPtr + 1;

		while (charinfo::isDigit(*end))
			++end;

		formToken(token, end, Token::number);
		return;
	}
	else if (charinfo::isSpecialCharacter(*BufferPtr))
	{
		switch (*BufferPtr)
		{
		case ';':
			formToken(token, BufferPtr + 1, Token::semi_colon);
			break;
		case ',':
			formToken(token, BufferPtr + 1, Token::comma);
			break;
		case '(':
			formToken(token, BufferPtr + 1, Token::l_paren);
			break;
		case ')':
			formToken(token, BufferPtr + 1, Token::r_paren);
			break;
		case '{':
			formToken(token, BufferPtr + 1, Token::l_brace);
			break;
		case '}':
			formToken(token, BufferPtr + 1, Token::r_brace);
			break;
		}
	}
	else
	{

		if (*BufferPtr == '=' && *(BufferPtr + 1) == '=')
		{ // ==
			formToken(token, BufferPtr + 2, Token::equal_equal);
		}
		else if (*BufferPtr == '+' && *(BufferPtr + 1) == '=')
		{ // +=
			formToken(token, BufferPtr + 2, Token::plus_equal);
		}
		else if (*BufferPtr == '-' && *(BufferPtr + 1) == '=')
		{ // -=
			formToken(token, BufferPtr + 2, Token::minus_equal);
		}

		else if (*BufferPtr == '*' && *(BufferPtr + 1) == '=')
		{ // *=
			formToken(token, BufferPtr + 2, Token::star_equal);
		}

		else if (*BufferPtr == '/' && *(BufferPtr + 1) == '=')
		{ // /=
			formToken(token, BufferPtr + 2, Token::slash_equal);
		}

		else if (*BufferPtr == '%' && *(BufferPtr + 1) == '=')
		{ // %=
			formToken(token, BufferPtr + 2, Token::mod_equal);
		}

		else if (*BufferPtr == '!' && *(BufferPtr + 1) == '=')
		{ // !=
			formToken(token, BufferPtr + 2, Token::not_equal);
		}

		else if (*BufferPtr == '<' && *(BufferPtr + 1) == '=')
		{ // <=
			formToken(token, BufferPtr + 2, Token::less_equal);
		}

		else if (*BufferPtr == '>' && *(BufferPtr + 1) == '=')
		{
			formToken(token, BufferPtr + 2, Token::greater_equal); // >=
		}
		else if (*BufferPtr == '/' && *(BufferPtr + 1) == '*')
		{
			formToken(token, BufferPtr + 2, Token::comment); // comment
		}
		else if (*BufferPtr == '*' && *(BufferPtr + 1) == '/')
		{
			formToken(token, BufferPtr + 2, Token::uncomment); // uncomment
		}
		else if (*BufferPtr == '+' && *(BufferPtr + 1) == '+')
		{
			formToken(token, BufferPtr + 2, Token::plus_plus); // ++
		}
		else if (*BufferPtr == '-' && *(BufferPtr + 1) == '-')
		{
			formToken(token, BufferPtr + 2, Token::minus_minus); // --
		}
		else if (*BufferPtr == '=')
		{
			formToken(token, BufferPtr + 1, Token::equal); // =
		}
		else if (*BufferPtr == '+')
		{
			formToken(token, BufferPtr + 1, Token::plus); // +
		}
		else if (*BufferPtr == '-')
		{
			formToken(token, BufferPtr + 1, Token::minus); // -
		}
		else if (*BufferPtr == '*')
		{
			formToken(token, BufferPtr + 1, Token::star); // *
		}
		else if (*BufferPtr == '/')
		{
			formToken(token, BufferPtr + 1, Token::slash); // /
		}
		else if (*BufferPtr == '^')
		{
			formToken(token, BufferPtr + 1, Token::power); // ^
		}
		else if (*BufferPtr == '>')
		{
			formToken(token, BufferPtr + 1, Token::greater); // >
		}
		else if (*BufferPtr == '<')
		{
			formToken(token, BufferPtr + 1, Token::less); // <
		}
		else if (*BufferPtr == '!')
		{
			formToken(token, BufferPtr + 1, Token::not ); // !
		}
		else if (*BufferPtr == '%')
		{
			formToken(token, BufferPtr + 1, Token::mod); // %
		}
		else
		{
			formToken(token, BufferPtr + 1, Token::unknown);
		}
	}

}

void Lexer::formToken(Token &Tok, const char *TokEnd, Token::TokenKind Kind)
{
	Tok.Kind = Kind;
	Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
	BufferPtr = TokEnd;
}