#include <iostream>
#include "lexical.h"

namespace lilang
{
    namespace compiler
    {
        CodeToken::List LexicalParser::ParseFile(const string_t &file_name, CodeError::List &err_list)
        {
            fstream_t f(file_name);
            stringstream_t ss;
            ss << f.rdbuf();
            return ParseString(ss.str(), err_list);
        }

        CodeToken::List LexicalParser::ParseString(const string_t &file, CodeError::List &err_list)
        {
            enum class ParseState
            {
                kStart,
                kInComment,
                kInHex,
                kInHexEnd,
                kIndecimal,
                kInOct,
                kInOctEnd,
                kInBinary,
                kInBinaryEnd,
                kInFloat,
                kInString,
                kInStringEscape,
                kInIdentifier
            };
            ParseState state = ParseState::kStart;
            const char *str = file.c_str();
            const char *row_begin = str;
            const char *token_begin = str;
            int row_number = 1;
            CodeToken cur_token;
            CodeToken::List tok_list;

            auto NextState = [&](ParseState s) {
                state = s;
            };

            auto AddToken = [&](int len, CodeType type) {
                cur_token.value = string_t(token_begin, len);
                cur_token.type = type;
                if (type == CodeType::kIdentifier)
                {
                    auto tok = cur_token.value;
                    if (tok == "if")
                    {
                        cur_token.type = CodeType::kIf;
                    }
                    else if (tok == "for")
                    {
                        cur_token.type = CodeType::kFor;
                    }
                    else if (tok == "while")
                    {
                        cur_token.type = CodeType::kWhile;
                    }
                    else if (tok == "else")
                    {
                        cur_token.type = CodeType::kElse;
                    }
                    else if (tok == "let")
                    {
                        cur_token.type = CodeType::kLet;
                    }
                    else if (tok == "fn")
                    {
                        cur_token.type = CodeType::kFn;
                    }
                    else if (tok == "return")
                    {
                        cur_token.type = CodeType::kReturn;
                    }
                }
                tok_list.push_back(cur_token);
                NextState(ParseState::kStart);
            };

            auto AddError = [&](string_t msg) {
                CodeError err;
                err.error_msg = msg;
                err.row_number = row_number;
                err.column_number = str - row_begin;
                err_list.push_back(err);
                NextState(ParseState::kStart);
            };
            while (*str)
            {
                // std::cout << *str << static_cast<int>(state) << std::endl;
                switch (state)
                {
                case ParseState::kStart:
                    token_begin = str;
                    cur_token.row_number = row_number;
                    cur_token.column_number = str - row_begin;
                    switch (*str)
                    {
                    case '/':
                        switch (*(str + 1))
                        {
                        case '/':

                            NextState(ParseState::kInComment);
                            str++;
                            break;
                        case '=':
                            AddToken(2, CodeType::kDivAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kDivide);
                        }
                        break;
                    case '+':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kAddAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kAdd);
                        }
                        break;
                    case '-':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kSubAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kSub);
                        }
                        break;
                    case '*':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kMulAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kMultiply);
                        }
                        break;
                    case '&':
                        switch (*(str + 1))
                        {
                        case '&':
                            AddToken(2, CodeType::kLogicAnd);
                            str++;
                            break;
                        case '=':
                            AddToken(2, CodeType::kBitsAndAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kBitsAnd);
                        }
                        break;
                    case '^':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kBitsXorAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kBitsXor);
                        }
                        break;
                    case '|':
                        switch (*(str + 1))
                        {
                        case '|':
                            AddToken(2, CodeType::kLogicOr);
                            str++;
                            break;
                        case '=':
                            AddToken(2, CodeType::kBitsOrAssign);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kBitsOr);
                        }
                        break;
                    case '>':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kNotLess);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kGreater);
                        }
                        break;
                    case '<':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kNotGreater);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kLess);
                        }
                        break;
                    case '!':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kNotEqual);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kLogicNot);
                        }
                        break;
                    case '=':
                        switch (*(str + 1))
                        {
                        case '=':
                            AddToken(2, CodeType::kEqual);
                            str++;
                            break;
                        default:
                            AddToken(1, CodeType::kAssign);
                        }
                        break;
                    case ':':
                        if (*(str + 1) == '=')
                        {
                            AddToken(2, CodeType::kShortAssign);
                            str++;
                        }
                        else
                        {
                            AddError("unexcepted colon");
                        }
                        break;
                    case '"':
                        NextState(ParseState::kInString);
                        token_begin++;
                        break;
                    case '{':
                        AddToken(1, CodeType::kLeftBrace);
                        break;
                    case '}':
                        AddToken(1, CodeType::kRightBrace);
                        break;
                    case '[':
                        AddToken(1, CodeType::kLeftBracket);
                        break;
                    case ']':
                        AddToken(1, CodeType::kRightBracket);
                        break;
                    case ',':
                        AddToken(1, CodeType::kComma);
                        break;
                    case ';':
                        AddToken(1, CodeType::kSemiColon);
                        break;
                    case '(':
                        AddToken(1, CodeType::kLeftParenthese);
                        break;
                    case ')':
                        AddToken(1, CodeType::kRightParenthese);
                        break;
                    case '0':
                        switch (*(str + 1))
                        {
                        case 'x':
                        case 'X':
                            NextState(ParseState::kInHex);
                            str++;
                            break;
                        case 'b':
                        case 'B':
                            NextState(ParseState::kInBinary);
                            str++;
                            break;
                        case 'o':
                        case 'O':
                            NextState(ParseState::kInOct);
                            str++;
                            break;
                        case '.':
                            NextState(ParseState::kInFloat);
                            str++;
                            break;
                        default:
                            NextState(ParseState::kInOctEnd);
                        }
                        break;
                    case '\n':
                    case '\t':
                    case '\r':
                    case ' ':
                        break;
                    default:
                        auto cur_char = *str;
                        if ((cur_char >= 'a' && cur_char <= 'z') || (cur_char >= 'A' && cur_char <= 'Z'))
                        {
                            NextState(ParseState::kInIdentifier);
                        }
                        else if (CodeToken::IsDecimalDigit(*str))
                        {
                            NextState(ParseState::kIndecimal);
                        }
                        else
                        {
                            AddError("unspported start character");
                        }
                    }
                    break;
                case ParseState::kInComment:
                    if (*str == '\n')
                    {
                        AddToken(str - token_begin, CodeType::kComment);
                        state = ParseState::kStart;
                    }
                    //continue
                    break;
                case ParseState::kInIdentifier:
                    if ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') ||
                        (*str >= '0' && *str <= '9') || *str == '_')
                    {
                        ;
                    }
                    else
                    {
                        AddToken(str - token_begin, CodeType::kIdentifier);
                        str--;
                    }
                    break;
                case ParseState::kInHex:
                    if (!CodeToken::IsHexDigit(*str))
                    {
                        AddError("invalid hex number");
                    }
                    else
                    {
                        NextState(ParseState::kInHexEnd);
                    }
                    break;
                case ParseState::kInHexEnd:
                    if (!CodeToken::IsHexDigit(*str))
                    {
                        AddToken(str - token_begin, CodeType::kNumber);
                        str--;
                    }
                    break;
                case ParseState::kInOct:
                    if (!CodeToken::IsOctalDigit(*str))
                    {
                        AddError("invalid octal number");
                    }
                    else
                    {
                        NextState(ParseState::kInOctEnd);
                    }
                    break;
                case ParseState::kInOctEnd:
                    if (!CodeToken::IsOctalDigit(*str))
                    {
                        AddToken(str - token_begin, CodeType::kNumber);
                        str--;
                    }
                    break;
                case ParseState::kInBinary:
                    if (!CodeToken::IsBinaryDigit(*str))
                    {
                        AddError("invalid binary number");
                    }
                    else
                    {
                        NextState(ParseState::kInBinaryEnd);
                    }
                    break;
                case ParseState::kInBinaryEnd:
                    if (!CodeToken::IsBinaryDigit(*str))
                    {
                        AddToken(str - token_begin, CodeType::kNumber);
                        str--; //not to read current character
                    }
                    break;
                case ParseState::kIndecimal:
                    if (*str == '.')
                    {
                        NextState(ParseState::kInFloat);
                    }
                    else if (*str < '0' || *str > '9')
                    {
                        AddToken(str - token_begin, CodeType::kNumber);
                        str--; //not to read current character
                    }
                    break;
                case ParseState::kInFloat:
                    if (*str < '0' || *str > '9')
                    {
                        AddToken(str - token_begin, CodeType::kNumber);
                        str--;
                    }
                    break;
                case ParseState::kInString:
                    switch (*str)
                    {
                    case '"':
                        AddToken(str - token_begin, CodeType::kStringLiteral);
                        break;
                    case '\\':
                        NextState(ParseState::kInStringEscape);
                        break;
                    case '\n':
                        AddError("string literal should not contain multi lines whthout escaping");
                    }
                    break;
                case ParseState::kInStringEscape:
                    switch (*str)
                    {
                    case 'n':
                    case 't':
                    case '\\':
                    case '"':
                    case '\n': //support multi lines
                        NextState(ParseState::kInString);
                        break;
                    default:
                        AddError("unspported escaping string literal");
                    }
                    break;
                }
                str++;
                if (*(str - 1) == '\n')
                {
                    row_number++;
                    row_begin = str;
                }
            }

            char before = *(str - 1);
            int len = str - token_begin;
            switch (state)
            {
            case ParseState::kInHexEnd:
            case ParseState::kInOctEnd:
            case ParseState::kInBinaryEnd:
            case ParseState::kIndecimal:
                AddToken(len, CodeType::kNumber);
                break;
            case ParseState::kInHex:
            case ParseState::kInBinary:
            case ParseState::kInOct:
                AddError("invalid number");
                break;
            case ParseState::kInString:
            case ParseState::kInStringEscape:
                AddError("string literal not end");
                break;
            case ParseState::kInIdentifier:
                AddToken(len, CodeType::kIdentifier);
                break;
            case ParseState::kInFloat:
                AddToken(len, CodeType::kFloat);
                break;
            case ParseState::kInComment:
                AddToken(len, CodeType::kComment);
                break;
            default:;
            }
            CodeToken end_token;
            end_token.type = CodeType::kEOF;
            end_token.row_number = row_number;
            end_token.column_number = str - row_begin;
            end_token.value = "$";
            tok_list.push_back(end_token);
            return tok_list;
        }

        string_t CodeToken::EscapeString(string_t str)
        {
            stringstream_t ss;
            for (char ch : str)
            {
                switch (ch)
                {
                case '\n':
                    ss << "\\\\n";
                    break;
                case '\\':
                    ss << "\\\\";
                    break;
                case '\t':
                    ss << "\\\t";
                    break;
                default:
                    ss << ch;
                    break;
                }
            }
            return ss.str();
        }

        string_t CodeToken::UnEscapeString(string_t str)
        {
            stringstream_t ss;
            bool escaping = false;
            for (char ch : str)
            {
                if (escaping)
                {
                    switch (ch)
                    {
                    case 'n':
                        ss << '\n';
                        break;
                    case 't':
                        ss << '\t';
                        break;
                    case '\\':
                        ss << '\\';
                        break;
                    case '"':
                        ss << '"';
                    default:
                        ss << ch;
                    }
                    escaping = false;
                }
                else
                {
                    if (ch == '\\')
                    {
                        escaping = true;
                    }
                    else
                    {
                        ss << ch;
                    }
                }
            }
            return ss.str();
        }

        bool CodeToken::IsBinaryDigit(char_t ch)
        {
            return ch == '0' || ch == '1';
        }

        bool CodeToken::IsHexDigit(char_t ch)
        {
            return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        }

        bool CodeToken::IsOctalDigit(char_t ch)
        {
            return ch >= '0' && ch <= '7';
        }

        bool CodeToken::IsDecimalDigit(char_t ch)
        {
            return ch >= '0' && ch <= '9';
        }

        string_t CodeToken::Type2Str(CodeType t)
        {
            switch (t)
            {
            case CodeType::kEOF:
                return "EOF";
            case CodeType::kIdentifier:
                return "IDENTIFIER";
            case CodeType::kComment:
                return "COMMENT";
            case CodeType::kNumber:
                return "NUMBER";
            case CodeType::kFloat:
                return "FLOAT";
            case CodeType::kStringLiteral:
                return "STRING";
            case CodeType::kAdd:
                return "ADD";
            case CodeType::kSub:
                return "SUB";
            case CodeType::kDivide:
                return "DIV";
            case CodeType::kMultiply:
                return "MUL";
            case CodeType::kBitsAnd:
                return "BITAND";
            case CodeType::kBitsOr:
                return "BITOR";
            case CodeType::kBitsXor:
                return "BITXOR";
            case CodeType::kMod:
                return "MOD";
            case CodeType::kAddAssign:
                return "ADDASSIGN";
            case CodeType::kSubAssign:
                return "SUBASSIGN";
            case CodeType::kDivAssign:
                return "DIVASSIGN";
            case CodeType::kMulAssign:
                return "MULASSIGN";
            case CodeType::kBitsAndAssign:
                return "BITSANDASSIGN";
            case CodeType::kBitsOrAssign:
                return "BITSORASSIGN";
            case CodeType::kBitsXorAssign:
                return "BITSXORASSIGN";
            case CodeType::kAssign:
                return "ASSIGN";
            case CodeType::kShortAssign:
                return "SHORTASSIGN";
            case CodeType::kEqual:
                return "EQ";
            case CodeType::kNotEqual:
                return "NOTEQ";
            case CodeType::kGreater:
                return "GRATER";
            case CodeType::kLess:
                return "LESS";
            case CodeType::kNotGreater:
                return "NOTGRATER";
            case CodeType::kNotLess:
                return "NOTLESS";
            case CodeType::kLogicAnd:
                return "AND";
            case CodeType::kLogicOr:
                return "OR";
            case CodeType::kLogicNot:
                return "NOT";
            case CodeType::kLeftBrace:
                return "LEFTBRACE";
            case CodeType::kRightBrace:
                return "RIGHTBRACE";
            case CodeType::kLeftBracket:
                return "LEFTBRACKET";
            case CodeType::kRightBracket:
                return "RIGHTBRCKET";
            case CodeType::kLeftParenthese:
                return "LEFTPAREN";
            case CodeType::kRightParenthese:
                return "RIGHTPAREN";
            case CodeType::kComma:
                return "COMMA";
            case CodeType::kSemiColon:
                return "SEMIECOLON";
            case CodeType::kIf:
                return "IF";
            case CodeType::kWhile:
                return "WHILE";
            case CodeType::kFor:
                return "FOR";
            case CodeType::kLet:
                return "LET";
            case CodeType::kFn:
                return "FN";
            case CodeType::kReturn:
                return "RETUEN";
            default:
                return "UNKNOWN";
            }
        }

        int CodeToken::Precedence(CodeType t)
        {
            switch (t)
            {
            case CodeType::kLogicOr:
                return 1;
            case CodeType::kLogicAnd:
                return 2;
            case CodeType::kEqual:
            case CodeType::kNotEqual:
            case CodeType::kLess:
            case CodeType::kGreater:
            case CodeType::kNotGreater:
            case CodeType::kNotLess:
                return 3;
            case CodeType::kAdd:
            case CodeType::kSub:
            case CodeType::kBitsOr:
            case CodeType::kBitsXor:
                return 4;
            case CodeType::kBitsAnd:
            case CodeType::kMultiply:
            case CodeType::kDivide:
            case CodeType::kMod:
                return 5;
            default:
                return 0;
            }
        }

    }
}
