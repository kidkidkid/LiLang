#ifndef LILANG_COMPILER_LEXICAL
#define LILANG_COMPILER_LEXICAL

#include <vector>

#include "../listl.h"

/*
string literal:
x = "abcd\n" ===> a,b,c,d,\\,n    support
x = "abcd    ===> a,b,c,d,\n      not support
"
x = "abcd\   ===> a,b,c,d,\\,\n   support
"
string escaping:
\n \\ \t \"

float:
1.   support
.1   not support
*/
namespace lilang
{
    namespace compiler
    {
        enum class CodeType
        {
            kEOF,
            kIdentifier,
            kComment,         // //*
            kNumber,          // includes decimal, hexadecimal, octal, binary
            kFloat,           // {0-9}.{0-9}
            kStringLiteral,   // "*"
            kAdd,             // +
            kSub,             // -
            kDivide,          // /
            kMultiply,        // *
            kBitsAnd,         // &
            kBitsOr,          // |
            kBitsXor,         // ^
            kMod,             // %
            kAddAssign,       // +=
            kSubAssign,       // -=
            kDivAssign,       // /=
            kMulAssign,       // *=
            kBitsAndAssign,   // &=
            kBitsOrAssign,    // |=
            kBitsXorAssign,   // ^=
            kAssign,          // =
            kShortAssign,     // :=
            kEqual,           // ==
            kNotEqual,        // !=
            kGreater,         // >
            kLess,            // <
            kNotGreater,      // <=
            kNotLess,         // >=
            kLogicAnd,        // &&
            kLogicOr,         // ||
            kLogicNot,        // !
            kLeftBrace,       // {
            kRightBrace,      // }
            kLeftParenthese,  // (
            kRightParenthese, // )
            kLeftBracket,     // [
            kRightBracket,    // ]
            kComma,           // ,
            kSemiColon,       // ;
                              //keyword
            kIf,              // if
            kElse,            // else
            kWhile,           // while
            kFor,             // for
            kLet,             // let
            kFn,              // fn
            kReturn,          // return
        };

        struct CodeToken
        {
            CodeType type;
            string_t value;
            int row_number;
            int column_number;

            typedef std::vector<CodeToken> List;
            static string_t EscapeString(string_t);
            static string_t UnEscapeString(string_t);
            static bool IsHexDigit(char_t);
            static bool IsOctalDigit(char_t);
            static bool IsBinaryDigit(char_t);
            static bool IsDecimalDigit(char_t);
            static string_t Type2Str(CodeType);
            static int Precedence(CodeType);
        };

        struct CodeError
        {
            string_t error_msg;
            int row_number;
            int column_number;

            typedef std::vector<CodeError> List;
        };

        struct LexicalParser
        {
            int _;
            static CodeToken::List ParseString(const string_t &, CodeError::List &);
            static CodeToken::List ParseFile(const string_t &, CodeError::List &);
        };

    }
}

#endif