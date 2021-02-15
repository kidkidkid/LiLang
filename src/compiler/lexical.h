#ifndef LILANG_LEXICAL
#define LILANG_LEXICAL

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
\n \\ \t
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
            kAssign,          // =
            kAddAssign,       // +=
            kSubAssign,       // -=
            kMulAssign,       // *=
            kDivAssign,       // /=
            kEqual,           // ==
            kNotEqual,        // !=
            kGreater,         // >
            kLess,            // <
            kNotGreater,      // <=
            kNotLess,         // >=
            kLogicAnd,        // &&
            kLogicOr,         // ||
            kLogicNot,        // !
            kIf,              // if
            kWhile,           // while
            kFor,             // for
            kLeftBracket,     // {
            kRightBracket,    // }
            kLeftParenthese,  // (
            kRightParenthese, // )
            kComma,           // ,
            kSemiColon        // ;
        };

        struct CodeToken
        {
            CodeType type;
            string_t token;
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
        };

        struct CodeError
        {
            string_t error_msg;
            int row_number;
            int column_number;

            typedef std::vector<CodeError> List;
        };

        class LexicalParser
        {
        public:
            LexicalParser();
            ~LexicalParser();
            CodeToken::List Parse(const string_t &, CodeError::List &);
        };
    } // namespace compiler
} // namespace lilang

#endif