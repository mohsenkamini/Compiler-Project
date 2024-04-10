#ifndef LEXER_H
#define LEXER_H
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

class Lexer;

class Token
{
    friend class Lexer;

public:
    enum TokenKind : unsigned short
    {
        semi_colon,    // ;
        unknown,       // unknown token
        identifier,    // identifier like a, b, c, d, etc.
        number,        // number like 1, 2, 3, 4, etc.
        comma,         // ,
        plus,          // +
        minus,         // -
        star,          // *
        mod,           // %
        slash,         // /
        power,         // ^
        l_paren,       // (
        r_paren,       // )
        plus_equal,    // +=
        minus_equal,   // -=
        star_equal,    // *=
        mod_equal,     // %=
        slash_equal,   // /=
        equal,         // =
        equal_equal,   // ==
        not_equal,     // !=
        less,          // <
        less_equal,    // <=
        greater,       // >
        greater_equal, // >=
        comment,       // /*
        uncomment,     // */
        KW_int,        // int
        KW_if,         // if
        KW_else,       // else
        KW_while,      // while
        KW_for,        // for
        KW_and,        // and
        KW_or,         // or
        KW_true,       // true
        KW_false,      // false
        eof,           // end of file
    };

private:
    TokenKind Kind;       // <type of token>
    llvm::StringRef Text; // <token context>

public:
    TokenKind getKind() const { return Kind; }
    bool is(TokenKind K) const { return Kind == K; }
    llvm::StringRef getText() const { return Text; }

    // kind="+" isOneOf(plus, minus) -> true
    bool isOneOf(TokenKind K1, TokenKind K2) const
    {
        return is(K1) || is(K2);
    }

    template <typename... Ts> // variadic template can take several inputs
    bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks) const
    {
        return is(K1) || isOneOf(K2, Ks...);
    }
};

class Lexer
{
    const char *BufferStart;
    const char *BufferPtr;

public:
    Lexer(const llvm::StringRef &Buffer)
    {
        BufferStart = Buffer.begin();
        BufferPtr = BufferStart;
    }
    void next(Token &token);

private:
    void formToken(Token &Result, const char *TokEnd, Token::TokenKind Kind);
};

#endif
