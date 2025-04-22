#include "lexer.h"
#include <cctype>

namespace charinfo {
    LLVM_READNONE inline bool isWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\f' || 
               c == '\v' || c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c) {
        return std::isdigit(c);
    }

    LLVM_READNONE inline bool isAlpha(char c) {
        return std::isalpha(c) || c == '_';
    }

    LLVM_READNONE inline bool isAlnum(char c) {
        return isAlpha(c) || isDigit(c);
    }
}

void Lexer::next(Token &token) {
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr)) ++BufferPtr;

    if (!*BufferPtr) {
        token.Kind = Token::eof;
        return;
    }

    if (*BufferPtr == '/' && *(BufferPtr + 1) == '*') {
        skipMultiLineComment();
        return next(token);
    }

    // identifiers
    if (charinfo::isAlpha(*BufferPtr)) {
        const char *end = BufferPtr + 1;
        while (charinfo::isAlnum(*end)) ++end;

        llvm::StringRef text(BufferPtr, end - BufferPtr);
        Token::TokenKind kind = Token::identifier;

        if (text == "int") kind = Token::KW_int;
        else if (text == "bool") kind = Token::KW_bool;
        else if (text == "float") kind = Token::KW_float;
        else if (text == "char") kind = Token::KW_char;
        else if (text == "string") kind = Token::KW_string;
        else if (text == "array") kind = Token::KW_array;
        else if (text == "if") kind = Token::KW_if;
        else if (text == "else") kind = Token::KW_else;
        else if (text == "while") kind = Token::KW_while;
        else if (text == "for") kind = Token::KW_for;
        else if (text == "and") kind = Token::KW_and;
        else if (text == "or") kind = Token::KW_or;
        else if (text == "true") kind = Token::KW_true;
        else if (text == "false") kind = Token::KW_false;
        else if (text == "print") kind = Token::KW_print;

        formToken(token, end, kind);
        return;
    }

    if (charinfo::isDigit(*BufferPtr) || (*BufferPtr == '.' && charinfo::isDigit(*(BufferPtr + 1))) {
        const char *end = BufferPtr;
        bool hasDot = false;
        
        while (charinfo::isDigit(*end) || (*end == '.' && !hasDot)) {
            if (*end == '.') hasDot = true;
            ++end;
        }

        formToken(token, end, hasDot ? Token::number : Token::number);
        return;
    }

    switch (*BufferPtr) {
        case ';': formToken(token, BufferPtr + 1, Token::semi_colon); break;
        case ',': formToken(token, BufferPtr + 1, Token::comma); break;
        case '(': formToken(token, BufferPtr + 1, Token::l_paren); break;
        case ')': formToken(token, BufferPtr + 1, Token::r_paren); break;
        case '{': formToken(token, BufferPtr + 1, Token::l_brace); break;
        case '}': formToken(token, BufferPtr + 1, Token::r_brace); break;
        
        case '+':
            if (*(BufferPtr + 1) == '+') formToken(token, BufferPtr + 2, Token::increment);
            else if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::plus_equal);
            else formToken(token, BufferPtr + 1, Token::plus);
            break;
            
        case '-':
            if (*(BufferPtr + 1) == '-') formToken(token, BufferPtr + 2, Token::decrement);
            else if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::minus_equal);
            else formToken(token, BufferPtr + 1, Token::minus);
            break;
            
        case '*':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::star_equal);
            else formToken(token, BufferPtr + 1, Token::star);
            break;
            
        case '/':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::slash_equal);
            else formToken(token, BufferPtr + 1, Token::slash);
            break;
            
        case '^': formToken(token, BufferPtr + 1, Token::power); break;
        case '%': 
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::mod_equal);
            else formToken(token, BufferPtr + 1, Token::mod);
            break;
            
        case '=':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::equal_equal);
            else formToken(token, BufferPtr + 1, Token::equal);
            break;
            
        case '!':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::not_equal);
            else formToken(token, BufferPtr + 1, Token::KW_not);
            break;
            
        case '<':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::less_equal);
            else formToken(token, BufferPtr + 1, Token::less);
            break;
            
        case '>':
            if (*(BufferPtr + 1) == '=') formToken(token, BufferPtr + 2, Token::greater_equal);
            else formToken(token, BufferPtr + 1, Token::greater);
            break;
            
        default:
            formToken(token, BufferPtr + 1, Token::unknown);
    }
}

void Lexer::skipMultiLineComment() {
    BufferPtr += 2;
    while (*BufferPtr && !(*BufferPtr == '*' && *(BufferPtr + 1) == '/')) {
        ++BufferPtr;
    }
    if (*BufferPtr) BufferPtr += 2;
}

void Lexer::formToken(Token &Tok, const char *TokEnd, Token::TokenKind Kind) {
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}
