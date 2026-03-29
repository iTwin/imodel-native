/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlLexer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// ---------------------------------------------------------------------------
// Keyword table (must stay sorted by string for binary search)
// ---------------------------------------------------------------------------
namespace {

struct KwEntry { Utf8CP name; ECSqlTokenType type; };

// Case-insensitive compare used by std::lower_bound
struct KwCmp { bool operator()(KwEntry const& a, KwEntry const& b) const { return BeStringUtilities::StricmpAscii(a.name, b.name) < 0; } };

static const KwEntry s_keywords[] = {
    { "ALL",               ECSqlTokenType::KW_ALL },
    { "AND",               ECSqlTokenType::KW_AND },
    { "ANY",               ECSqlTokenType::KW_ANY },
    { "AS",                ECSqlTokenType::KW_AS },
    { "ASC",               ECSqlTokenType::KW_ASC },
    { "AVG",               ECSqlTokenType::KW_AVG },
    { "BACKWARD",          ECSqlTokenType::KW_BACKWARD },
    { "BETWEEN",           ECSqlTokenType::KW_BETWEEN },
    { "BINARY",            ECSqlTokenType::KW_BINARY },
    { "BLOB",              ECSqlTokenType::KW_BLOB },
    { "BOOLEAN",           ECSqlTokenType::KW_BOOLEAN },
    { "BY",                ECSqlTokenType::KW_BY },
    { "CASE",              ECSqlTokenType::KW_CASE },
    { "CAST",              ECSqlTokenType::KW_CAST },
    { "COLLATE",           ECSqlTokenType::KW_COLLATE },
    { "COUNT",             ECSqlTokenType::KW_COUNT },
    { "CROSS",             ECSqlTokenType::KW_CROSS },
    { "CUME_DIST",         ECSqlTokenType::KW_CUME_DIST },
    { "CURRENT",           ECSqlTokenType::KW_CURRENT },
    { "CURRENT_DATE",      ECSqlTokenType::KW_CURRENT_DATE },
    { "CURRENT_TIME",      ECSqlTokenType::KW_CURRENT_TIME },
    { "CURRENT_TIMESTAMP", ECSqlTokenType::KW_CURRENT_TIMESTAMP },
    { "DATE",              ECSqlTokenType::KW_DATE },
    { "DELETE",            ECSqlTokenType::KW_DELETE },
    { "DENSE_RANK",        ECSqlTokenType::KW_DENSE_RANK },
    { "DESC",              ECSqlTokenType::KW_DESC },
    { "DISTINCT",          ECSqlTokenType::KW_DISTINCT },
    { "DOUBLE",            ECSqlTokenType::KW_DOUBLE },
    { "ECSQLOPTIONS",      ECSqlTokenType::KW_ECSQLOPTIONS },
    { "ELSE",              ECSqlTokenType::KW_ELSE },
    { "END",               ECSqlTokenType::KW_END },
    { "ESCAPE",            ECSqlTokenType::KW_ESCAPE },
    { "EVERY",             ECSqlTokenType::KW_EVERY },
    { "EXCEPT",            ECSqlTokenType::KW_EXCEPT },
    { "EXCLUDE",           ECSqlTokenType::KW_EXCLUDE },
    { "EXISTS",            ECSqlTokenType::KW_EXISTS },
    { "FALSE",             ECSqlTokenType::KW_FALSE },
    { "FILTER",            ECSqlTokenType::KW_FILTER },
    { "FIRST",             ECSqlTokenType::KW_FIRST },
    { "FIRST_VALUE",       ECSqlTokenType::KW_FIRST_VALUE },
    { "FLOAT",             ECSqlTokenType::KW_FLOAT },
    { "FOLLOWING",         ECSqlTokenType::KW_FOLLOWING },
    { "FOR",               ECSqlTokenType::KW_FOR },
    { "FORWARD",           ECSqlTokenType::KW_FORWARD },
    { "FROM",              ECSqlTokenType::KW_FROM },
    { "FULL",              ECSqlTokenType::KW_FULL },
    { "GROUP",             ECSqlTokenType::KW_GROUP },
    { "GROUP_CONCAT",      ECSqlTokenType::KW_GROUP_CONCAT },
    { "GROUPS",            ECSqlTokenType::KW_GROUPS },
    { "HAVING",            ECSqlTokenType::KW_HAVING },
    { "IIF",               ECSqlTokenType::KW_IIF },
    { "IN",                ECSqlTokenType::KW_IN },
    { "INNER",             ECSqlTokenType::KW_INNER },
    { "INSERT",            ECSqlTokenType::KW_INSERT },
    { "INT",               ECSqlTokenType::KW_INT },
    { "INT64",             ECSqlTokenType::KW_INT64 },
    { "INTEGER",           ECSqlTokenType::KW_INT },
    { "INTERSECT",         ECSqlTokenType::KW_INTERSECT },
    { "INTO",              ECSqlTokenType::KW_INTO },
    { "IS",                ECSqlTokenType::KW_IS },
    { "JOIN",              ECSqlTokenType::KW_JOIN },
    { "LAG",               ECSqlTokenType::KW_LAG },
    { "LAST",              ECSqlTokenType::KW_LAST },
    { "LAST_VALUE",        ECSqlTokenType::KW_LAST_VALUE },
    { "LEAD",              ECSqlTokenType::KW_LEAD },
    { "LEFT",              ECSqlTokenType::KW_LEFT },
    { "LIKE",              ECSqlTokenType::KW_LIKE },
    { "LIMIT",             ECSqlTokenType::KW_LIMIT },
    { "LONG",              ECSqlTokenType::KW_LONG },
    { "MATCH",             ECSqlTokenType::KW_MATCH },
    { "MAX",               ECSqlTokenType::KW_MAX },
    { "MIN",               ECSqlTokenType::KW_MIN },
    { "NATURAL",           ECSqlTokenType::KW_NATURAL },
    { "NAVIGATION_VALUE",  ECSqlTokenType::KW_NAVIGATION_VALUE },
    { "NO",                ECSqlTokenType::KW_NO },
    { "NOCASE",            ECSqlTokenType::KW_NOCASE },
    { "NOT",               ECSqlTokenType::KW_NOT },
    { "NTH_VALUE",         ECSqlTokenType::KW_NTH_VALUE },
    { "NTILE",             ECSqlTokenType::KW_NTILE },
    { "NULL",              ECSqlTokenType::KW_NULL },
    { "NULLS",             ECSqlTokenType::KW_NULLS },
    { "OFFSET",            ECSqlTokenType::KW_OFFSET },
    { "ON",                ECSqlTokenType::KW_ON },
    { "ONLY",              ECSqlTokenType::KW_ONLY },
    { "OPTIONS",           ECSqlTokenType::KW_ECSQLOPTIONS },  // alias
    { "OR",                ECSqlTokenType::KW_OR },
    { "ORDER",             ECSqlTokenType::KW_ORDER },
    { "OTHERS",            ECSqlTokenType::KW_OTHERS },
    { "OUTER",             ECSqlTokenType::KW_OUTER },
    { "OVER",              ECSqlTokenType::KW_OVER },
    { "PARTITION",         ECSqlTokenType::KW_PARTITION },
    { "PERCENT_RANK",      ECSqlTokenType::KW_PERCENT_RANK },
    { "PRAGMA",            ECSqlTokenType::KW_PRAGMA },
    { "PRECEDING",         ECSqlTokenType::KW_PRECEDING },
    { "RANGE",             ECSqlTokenType::KW_RANGE },
    { "RANK",              ECSqlTokenType::KW_RANK },
    { "REAL",              ECSqlTokenType::KW_REAL },
    { "RECURSIVE",         ECSqlTokenType::KW_RECURSIVE },
    { "RIGHT",             ECSqlTokenType::KW_RIGHT },
    { "ROW",               ECSqlTokenType::KW_ROW },
    { "ROWS",              ECSqlTokenType::KW_ROWS },
    { "ROW_NUMBER",        ECSqlTokenType::KW_ROW_NUMBER },
    { "RTRIM",             ECSqlTokenType::KW_RTRIM },
    { "SELECT",            ECSqlTokenType::KW_SELECT },
    { "SET",               ECSqlTokenType::KW_SET },
    { "SOME",              ECSqlTokenType::KW_SOME },
    { "STRING",            ECSqlTokenType::KW_STRING_KW },
    { "SUM",               ECSqlTokenType::KW_SUM },
    { "THEN",              ECSqlTokenType::KW_THEN },
    { "TIES",              ECSqlTokenType::KW_TIES },
    { "TIME",              ECSqlTokenType::KW_TIME },
    { "TIMESTAMP",         ECSqlTokenType::KW_TIMESTAMP },
    { "TOTAL",             ECSqlTokenType::KW_TOTAL },
    { "TRUE",              ECSqlTokenType::KW_TRUE },
    { "UNBOUNDED",         ECSqlTokenType::KW_UNBOUNDED },
    { "UNION",             ECSqlTokenType::KW_UNION },
    { "UNIQUE",            ECSqlTokenType::KW_UNIQUE },
    { "UNKNOWN",           ECSqlTokenType::KW_UNKNOWN },
    { "UPDATE",            ECSqlTokenType::KW_UPDATE },
    { "USING",             ECSqlTokenType::KW_USING },
    { "VALUE",             ECSqlTokenType::KW_VALUE },
    { "VALUES",            ECSqlTokenType::KW_VALUES },
    { "VARCHAR",           ECSqlTokenType::KW_VARCHAR },
    { "WHEN",              ECSqlTokenType::KW_WHEN },
    { "WHERE",             ECSqlTokenType::KW_WHERE },
    { "WINDOW",            ECSqlTokenType::KW_WINDOW },
    { "WITH",              ECSqlTokenType::KW_WITH },
    };

static const size_t s_numKeywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

} // anonymous namespace

// ---------------------------------------------------------------------------
// ECSqlToken helpers
// ---------------------------------------------------------------------------

bool ECSqlToken::TextEqualsI(Utf8CP s) const
    {
    if (!text || !s) return false;
    size_t slen = strlen(s);
    if (slen != len) return false;
    return BeStringUtilities::StricmpAscii(Utf8String(text, len).c_str(), s) == 0;
    }

// ---------------------------------------------------------------------------
// ECSqlLexer — construction and helpers
// ---------------------------------------------------------------------------

ECSqlLexer::ECSqlLexer(Utf8CP ecsql)
    : m_src(ecsql), m_cur(ecsql), m_line(1), m_col(1)
    {
    m_end = ecsql ? (ecsql + strlen(ecsql)) : ecsql;
    }

char ECSqlLexer::Peek(int ahead) const
    {
    Utf8CP p = m_cur + ahead;
    return (p < m_end) ? *p : '\0';
    }

char ECSqlLexer::Consume()
    {
    if (m_cur >= m_end)
        return '\0';
    char c = *m_cur++;
    if (c == '\n')
        {
        m_line++;
        m_col = 1;
        }
    else
        m_col++;
    return c;
    }

void ECSqlLexer::SkipWhitespaceAndComments()
    {
    for (;;)
        {
        // whitespace
        while (m_cur < m_end && (*m_cur == ' ' || *m_cur == '\t' || *m_cur == '\r' || *m_cur == '\n'))
            Consume();

        // line comment  -- ... \n
        if (m_cur + 1 < m_end && m_cur[0] == '-' && m_cur[1] == '-')
            {
            while (m_cur < m_end && *m_cur != '\n')
                Consume();
            continue;
            }
        break;
        }
    }

ECSqlToken ECSqlLexer::MakeToken(ECSqlTokenType t, Utf8CP start, size_t len, uint32_t line, uint32_t col) const
    {
    ECSqlToken tok;
    tok.type = t;
    tok.text = start;
    tok.len  = len;
    tok.line = line;
    tok.col  = col;
    return tok;
    }

ECSqlLexer::Snapshot ECSqlLexer::SavePos() const
    {
    return Snapshot{m_cur, m_line, m_col};
    }

void ECSqlLexer::RestorePos(Snapshot const& s)
    {
    m_cur  = s.cur;
    m_line = s.line;
    m_col  = s.col;
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ClassifyKeyword
// ---------------------------------------------------------------------------

/*static*/ ECSqlTokenType ECSqlLexer::ClassifyKeyword(Utf8CP text, size_t len)
    {
    // Build a null-terminated uppercase copy for the binary search
    char upper[64];
    if (len == 0 || len >= sizeof(upper))
        return ECSqlTokenType::Name;

    for (size_t i = 0; i < len; i++)
        upper[i] = (char)toupper((unsigned char)text[i]);
    upper[len] = '\0';

    // Binary search in sorted keyword table
    size_t lo = 0, hi = s_numKeywords;
    while (lo < hi)
        {
        size_t mid = (lo + hi) / 2;
        int cmp = strcmp(upper, s_keywords[mid].name);
        if (cmp == 0)
            return s_keywords[mid].type;
        else if (cmp < 0)
            hi = mid;
        else
            lo = mid + 1;
        }
    return ECSqlTokenType::Name;
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ScanIdentifierOrKeyword
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::ScanIdentifierOrKeyword()
    {
    Utf8CP start = m_cur;
    uint32_t line = m_line, col = m_col;

    // An identifier starts with [A-Za-z_\x80-\xFF]
    while (m_cur < m_end)
        {
        unsigned char c = (unsigned char)*m_cur;
        if (c >= 'a' && c <= 'z') { Consume(); continue; }
        if (c >= 'A' && c <= 'Z') { Consume(); continue; }
        if (c == '_')              { Consume(); continue; }
        if (c >= '0' && c <= '9') { Consume(); continue; }
        if (c >= 0x80)             { Consume(); continue; }  // UTF-8 continuation
        break;
        }

    size_t len = (size_t)(m_cur - start);
    ECSqlTokenType type = ClassifyKeyword(start, len);
    return MakeToken(type, start, len, line, col);
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ScanNumber
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::ScanNumber()
    {
    Utf8CP start = m_cur;
    uint32_t line = m_line, col = m_col;
    bool isApprox = false;

    // Hex integer?
    if (Peek(0) == '0' && (Peek(1) == 'x' || Peek(1) == 'X'))
        {
        Consume(); Consume();
        while (m_cur < m_end && isxdigit((unsigned char)*m_cur))
            Consume();
        return MakeToken(ECSqlTokenType::IntNum, start, (size_t)(m_cur - start), line, col);
        }

    // Decimal digits before optional dot
    while (m_cur < m_end && isdigit((unsigned char)*m_cur))
        Consume();

    // Fractional part
    if (m_cur < m_end && *m_cur == '.' && isdigit((unsigned char)Peek(1)))
        {
        isApprox = true;
        Consume(); // '.'
        while (m_cur < m_end && isdigit((unsigned char)*m_cur))
            Consume();
        }
    else if (m_cur < m_end && *m_cur == '.')
        {
        // Could be  42.something_not_digit — leave dot alone
        }

    // Exponent
    if (m_cur < m_end && (*m_cur == 'e' || *m_cur == 'E'))
        {
        isApprox = true;
        Consume();
        if (m_cur < m_end && (*m_cur == '+' || *m_cur == '-'))
            Consume();
        while (m_cur < m_end && isdigit((unsigned char)*m_cur))
            Consume();
        }

    ECSqlTokenType t = isApprox ? ECSqlTokenType::ApproxNum : ECSqlTokenType::IntNum;
    return MakeToken(t, start, (size_t)(m_cur - start), line, col);
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ScanString  (single-quoted)
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::ScanString(char /*delimiter*/)
    {
    // We start *at* the opening quote.  The text of the token includes the
    // outer quotes so callers can distinguish '' escaping.
    Utf8CP start = m_cur;
    uint32_t line = m_line, col = m_col;

    Consume(); // opening '
    while (m_cur < m_end)
        {
        char c = Consume();
        if (c == '\'')
            {
            // doubled quote '' is an escape sequence → continue
            if (m_cur < m_end && *m_cur == '\'')
                Consume();
            else
                break; // closing quote
            }
        }
    return MakeToken(ECSqlTokenType::String, start, (size_t)(m_cur - start), line, col);
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ScanQuotedIdentifier  ("..." or `...` or [...])
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::ScanQuotedIdentifier(char closeDelim)
    {
    // We start *at* the opening delimiter.
    uint32_t line = m_line, col = m_col;

    Consume(); // consume the opening delimiter
    // The content of the token (returned as Name) excludes the delimiters.
    Utf8CP contentStart = m_cur;
    while (m_cur < m_end && *m_cur != closeDelim)
        Consume();
    size_t contentLen = (size_t)(m_cur - contentStart);
    if (m_cur < m_end)
        Consume(); // closing delimiter

    // Return as Name with the inner text (no surrounding quotes)
    return MakeToken(ECSqlTokenType::Name, contentStart, contentLen, line, col);
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::ScanOperatorOrPunct
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::ScanOperatorOrPunct()
    {
    Utf8CP start = m_cur;
    uint32_t line = m_line, col = m_col;
    char c = Consume();

    switch (c)
        {
        case '<':
            if (m_cur < m_end && *m_cur == '=')  { Consume(); return MakeToken(ECSqlTokenType::LessEq, start, 2, line, col); }
            if (m_cur < m_end && *m_cur == '>')  { Consume(); return MakeToken(ECSqlTokenType::NotEqual, start, 2, line, col); }
            if (m_cur < m_end && *m_cur == '<')  { Consume(); return MakeToken(ECSqlTokenType::ShiftLeft, start, 2, line, col); }
            return MakeToken(ECSqlTokenType::Less, start, 1, line, col);

        case '>':
            if (m_cur < m_end && *m_cur == '=')  { Consume(); return MakeToken(ECSqlTokenType::GreatEq, start, 2, line, col); }
            if (m_cur < m_end && *m_cur == '>')  { Consume(); return MakeToken(ECSqlTokenType::ShiftRight, start, 2, line, col); }
            return MakeToken(ECSqlTokenType::Great, start, 1, line, col);

        case '=':
            return MakeToken(ECSqlTokenType::Equal, start, 1, line, col);

        case '!':
            if (m_cur < m_end && *m_cur == '=')  { Consume(); return MakeToken(ECSqlTokenType::NotEqual, start, 2, line, col); }
            return MakeToken(ECSqlTokenType::Invalid, start, 1, line, col);

        case '|':
            if (m_cur < m_end && *m_cur == '|')  { Consume(); return MakeToken(ECSqlTokenType::Concat, start, 2, line, col); }
            return MakeToken(ECSqlTokenType::BitwiseOr, start, 1, line, col);

        case '-':
            if (m_cur < m_end && *m_cur == '>')  { Consume(); return MakeToken(ECSqlTokenType::Arrow, start, 2, line, col); }
            return MakeToken((ECSqlTokenType)'-', start, 1, line, col);

        case '~':  return MakeToken(ECSqlTokenType::BitwiseNot, start, 1, line, col);
        case '&':  return MakeToken(ECSqlTokenType::BitwiseAnd, start, 1, line, col);
        case '+':  return MakeToken((ECSqlTokenType)'+', start, 1, line, col);
        case '*':  return MakeToken((ECSqlTokenType)'*', start, 1, line, col);
        case '/':  return MakeToken((ECSqlTokenType)'/', start, 1, line, col);
        case '%':  return MakeToken((ECSqlTokenType)'%', start, 1, line, col);
        case ':':  return MakeToken((ECSqlTokenType)':', start, 1, line, col);
        case '(':  return MakeToken((ECSqlTokenType)'(', start, 1, line, col);
        case ')':  return MakeToken((ECSqlTokenType)')', start, 1, line, col);
        case ',':  return MakeToken((ECSqlTokenType)',', start, 1, line, col);
        case '.':
            // Could be .123 float literal
            if (m_cur < m_end && isdigit((unsigned char)*m_cur))
                {
                // back up and let ScanNumber handle it
                m_cur = start; m_line = line; m_col = col;
                return ScanNumber();
                }
            return MakeToken((ECSqlTokenType)'.', start, 1, line, col);
        case ';':  return MakeToken((ECSqlTokenType)';', start, 1, line, col);
        case '?':  return MakeToken(ECSqlTokenType::Parameter, start, 1, line, col);
        case '{':  return MakeToken((ECSqlTokenType)'{', start, 1, line, col);
        case '}':  return MakeToken((ECSqlTokenType)'}', start, 1, line, col);
        case '$':  return MakeToken(ECSqlTokenType::Dollar, start, 1, line, col);

        default:
            return MakeToken(ECSqlTokenType::Invalid, start, 1, line, col);
        }
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::Next
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::Next()
    {
    SkipWhitespaceAndComments();

    if (m_cur >= m_end)
        {
        ECSqlToken eof;
        eof.line = m_line; eof.col = m_col;
        return eof; // EndOfInput
        }

    char c = *m_cur;

    // Identifier or keyword
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (unsigned char)c >= 0x80)
        return ScanIdentifierOrKeyword();

    // Integer or float starting with digit
    if (c >= '0' && c <= '9')
        return ScanNumber();

    // Float starting with '.' (e.g. .5)
    if (c == '.' && m_cur + 1 < m_end && isdigit((unsigned char)m_cur[1]))
        return ScanNumber();

    // Single-quoted string
    if (c == '\'')
        return ScanString('\'');

    // Quoted identifiers
    if (c == '"')  return ScanQuotedIdentifier('"');
    if (c == '`')  return ScanQuotedIdentifier('`');
    if (c == '[')  return ScanQuotedIdentifier(']');

    // Named parameter :name
    if (c == ':' && m_cur + 1 < m_end)
        {
        char next = m_cur[1];
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') || next == '_')
            {
            uint32_t line = m_line, col = m_col;
            Consume(); // ':'
            Utf8CP nameStart = m_cur;
            while (m_cur < m_end)
                {
                unsigned char nc = (unsigned char)*m_cur;
                if ((nc >= 'a' && nc <= 'z') || (nc >= 'A' && nc <= 'Z') || nc == '_' || (nc >= '0' && nc <= '9'))
                    Consume();
                else
                    break;
                }
            // Return the name part only (without the ':')
            ECSqlToken tok;
            tok.type = ECSqlTokenType::NamedParam;
            tok.text = nameStart;
            tok.len  = (size_t)(m_cur - nameStart);
            tok.line = line;
            tok.col  = col;
            return tok;
            }
        }

    return ScanOperatorOrPunct();
    }

// ---------------------------------------------------------------------------
// ECSqlLexer::Peek1  (non-consuming lookahead)
// ---------------------------------------------------------------------------

ECSqlToken ECSqlLexer::Peek1()
    {
    auto snap = SavePos();
    ECSqlToken tok = Next();
    RestorePos(snap);
    return tok;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
