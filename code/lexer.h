#ifndef LEXER_H
#define LEXER_H
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include <string>

class Lexer;

class Token {
    friend class Lexer;

public:
    enum TokenKind : unsigned short {
        // پایه
        semi_colon,      // ;
        unknown,         // ناشناخته
        identifier,      // شناسه
        number,          // عدد
        comma,           // ,
        l_paren,         // (
        r_paren,         // )
        l_brace,         // {
        r_brace,         // }
        eof,             // پایان فایل
        
        // عملگرها
        plus,            // +
        minus,           // -
        star,            // *
        mod,             // %
        slash,           // /
        power,           // ^
        plus_equal,      // +=
        minus_equal,     // -=
        star_equal,      // *=
        mod_equal,       // %=
        slash_equal,     // /=
        equal,           // =
        equal_equal,     // ==
        not_equal,       // !=
        less,            // <
        less_equal,      // <=
        greater,         // >
        greater_equal,   // >=
        
        // کلمات کلیدی
        KW_int,          // int
        KW_bool,         // bool
        KW_float,        // float
        KW_char,         // char
        KW_string,       // string
        KW_array,        // array
        KW_if,           // if
        KW_else,         // else
        KW_while,        // while
        KW_for,          // for
        KW_and,          // and
        KW_or,           // or
        KW_true,         // true
        KW_false,        // false
        KW_print,        // print
        KW_not,          // !
        
        // توکنهای خاص
        comment_start,   /* 
        comment_end,     */
        increment,       // ++
        decrement        // --
    };

private:
    TokenKind Kind;          // نوع توکن
    llvm::StringRef Text;    // متن توکن

public:
    TokenKind getKind() const { return Kind; }
    bool is(TokenKind K) const { return Kind == K; }
    llvm::StringRef getText() const { return Text; }

    template <typename... Ts>
    bool isOneOf(TokenKind K1, Ts... Ks) const {
        return is(K1) || isOneOf(Ks...);
    }

    bool isOneOf(TokenKind K1, TokenKind K2) const {
        return is(K1) || is(K2);
    }
};

class Lexer {
    const char *BufferStart;
    const char *BufferPtr;

public:
    explicit Lexer(const llvm::StringRef &Buffer) {
        BufferStart = Buffer.begin();
        BufferPtr = BufferStart;
    }
    
    void next(Token &token);

private:
    void formToken(Token &Result, const char *TokEnd, Token::TokenKind Kind);
    void skipMultiLineComment();
};

#endif
