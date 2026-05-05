/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Token types for the ECSQL hand-written lexer. Mirrors the SQL_TOKEN_* identifiers
//! from sqlflex.l so that ECSqlRDParser can use familiar names.
// @bsiclass
//+===============+===============+===============+===============+===============+======
enum class ECSqlTokenType : uint32_t
    {
    // Literal / identifier tokens
    Name        = 1,   //!< bare identifier (includes keyword-like names not matched first)
    IntNum,            //!< integer literal, e.g. 42 or 0xFF
    ApproxNum,         //!< floating-point literal, e.g. 3.14 or 1e5
    String,            //!< single-quoted string literal
    AccessDate,        //!< date literal
    Parameter,         //!< ? positional parameter
    NamedParam,        //!< :name named parameter  (text holds name without leading ':')
    Dollar,            //!< $  token  (used for instance access paths)

    // Punctuation / operator tokens
    Less,              //!< <
    Great,             //!< >
    Equal,             //!< =
    LessEq,            //!< <=
    GreatEq,           //!< >=
    NotEqual,          //!< <> or !=
    Concat,            //!< ||
    Arrow,             //!< ->
    BitwiseNot,        //!< ~
    BitwiseOr,         //!< |
    BitwiseAnd,        //!< &
    ShiftLeft,         //!< <<
    ShiftRight,        //!< >>

    // Single-character punctuation (stored as-is; token type == char value for these)
    Minus      = '-',
    Plus       = '+',
    Star       = '*',
    Slash      = '/',
    Percent    = '%',
    Colon      = ':',
    LParen     = '(',
    RParen     = ')',
    Comma      = ',',
    Dot        = '.',
    Semicolon  = ';',
    Question   = '?',
    LBrace     = '{',
    RBrace     = '}',
    LBracket   = '[',
    RBracket   = ']',

    // Keywords — must start above 256 to avoid collision with char-value tokens
    KW_ALL = 512,
    KW_AND,
    KW_ANY,
    KW_AS,
    KW_ASC,
    KW_AVG,
    KW_BACKWARD,
    KW_BETWEEN,
    KW_BINARY,
    KW_BLOB,
    KW_BOOLEAN,
    KW_BY,
    KW_CASE,
    KW_CAST,
    KW_COLLATE,
    KW_COUNT,
    KW_CROSS,
    KW_CUME_DIST,
    KW_CURRENT,
    KW_CURRENT_DATE,
    KW_CURRENT_TIME,
    KW_CURRENT_TIMESTAMP,
    KW_DATE,
    KW_DELETE,
    KW_DENSE_RANK,
    KW_DESC,
    KW_DISTINCT,
    KW_DOUBLE,
    KW_ECSQLOPTIONS, //!< ECSQLOPTIONS or OPTIONS
    KW_ELSE,
    KW_END,
    KW_ESCAPE,
    KW_EVERY,
    KW_EXCEPT,
    KW_EXCLUDE,
    KW_EXISTS,
    KW_FALSE,
    KW_FILTER,
    KW_FIRST,
    KW_FIRST_VALUE,
    KW_FLOAT,
    KW_FOLLOWING,
    KW_FOR,
    KW_FORWARD,
    KW_FROM,
    KW_FULL,
    KW_GROUP,
    KW_GROUP_CONCAT,
    KW_GROUPS,
    KW_HAVING,
    KW_IIF,
    KW_IN,
    KW_INNER,
    KW_INSERT,
    KW_INT,    //!< INT or INTEGER
    KW_INT64,
    KW_INTERSECT,
    KW_INTO,
    KW_IS,
    KW_JOIN,
    KW_LAG,
    KW_LAST,
    KW_LAST_VALUE,
    KW_LEAD,
    KW_LEFT,
    KW_LIKE,
    KW_LIMIT,
    KW_LONG,
    KW_MATCH,
    KW_MAX,
    KW_MIN,
    KW_NATURAL,
    KW_NAVIGATION_VALUE,
    KW_NO,
    KW_NOCASE,
    KW_NOT,
    KW_NTH_VALUE,
    KW_NTILE,
    KW_NULL,
    KW_NULLS,
    KW_OFFSET,
    KW_ON,
    KW_ONLY,
    KW_OPTIONS, //!< alias for KW_ECSQLOPTIONS
    KW_OR,
    KW_ORDER,
    KW_OTHERS,
    KW_OUTER,
    KW_OVER,
    KW_PARTITION,
    KW_PERCENT_RANK,
    KW_PRAGMA,
    KW_PRECEDING,
    KW_RANGE,
    KW_RANK,
    KW_REAL,
    KW_RECURSIVE,
    KW_RIGHT,
    KW_ROW,
    KW_ROWS,
    KW_ROW_NUMBER,
    KW_RTRIM,
    KW_SELECT,
    KW_SET,
    KW_SOME,
    KW_STRING_KW, //!< STRING keyword
    KW_SUM,
    KW_THEN,
    KW_TIES,
    KW_TIME,
    KW_TIMESTAMP,
    KW_TOTAL,
    KW_TRUE,
    KW_UNBOUNDED,
    KW_UNION,
    KW_UNIQUE,
    KW_UNKNOWN,
    KW_UPDATE,
    KW_USING,
    KW_VALUE,
    KW_VALUES,
    KW_VARCHAR,
    KW_WHEN,
    KW_WHERE,
    KW_WINDOW,
    KW_WITH,

    // Sentinel
    EndOfInput = 0,
    Invalid    = UINT32_MAX,
    };

//=======================================================================================
//! A single token produced by ECSqlLexer. The text view points into the original
//! ECSQL source string (no heap allocation).
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlToken final
    {
    ECSqlTokenType type  = ECSqlTokenType::EndOfInput;
    Utf8CP         text  = nullptr; //!< start of token text in the source
    size_t         len   = 0;       //!< length of token text in bytes
    uint32_t       line  = 1;       //!< 1-based source line (for diagnostics)
    uint32_t       col   = 1;       //!< 1-based column

    bool IsKeyword()  const { return type >= ECSqlTokenType::KW_ALL && type <= ECSqlTokenType::KW_WITH; }
    bool IsEnd()      const { return type == ECSqlTokenType::EndOfInput; }
    bool IsInvalid()  const { return type == ECSqlTokenType::Invalid; }

    //! Return the token text as an Utf8String (heap-allocated copy).
    Utf8String GetText() const { return (text && len) ? Utf8String(text, len) : Utf8String(); }

    //! Return true when the token text (case-insensitive) equals the given literal.
    bool TextEqualsI(Utf8CP s) const;
    };

//=======================================================================================
//! Zero-allocation hand-written lexer for ECSQL.
//!
//! Usage:
//! @code
//!   ECSqlLexer lex(ecsqlSource);
//!   ECSqlToken tok = lex.Next();
//!   while (!tok.IsEnd()) { ... tok = lex.Next(); }
//! @endcode
//!
//! - All keywords are matched case-insensitively.
//! - Identifiers may be quoted with ", ` or []. The quotes are stripped from
//!   the returned token text, and the type is ECSqlTokenType::Name.
//! - Single-quoted strings return ECSqlTokenType::String; the enclosing quotes
//!   are included in the text so that callers can re-produce the literal.
//! - Comments (-- to end of line) are silently skipped.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlLexer final
    {
private:
    Utf8CP   m_src;      //!< start of source buffer (owned externally)
    Utf8CP   m_cur;      //!< current scan position
    Utf8CP   m_end;      //!< one-past-end of source
    uint32_t m_line;
    uint32_t m_col;

    // Advance helpers
    char Peek(int ahead = 0) const;
    char Consume();
    void SkipWhitespaceAndComments();

    // Building tokens
    ECSqlToken MakeToken(ECSqlTokenType t, Utf8CP start, size_t len, uint32_t line, uint32_t col) const;

    // Scanning sub-routines
    ECSqlToken ScanIdentifierOrKeyword();
    ECSqlToken ScanNumber();
    ECSqlToken ScanString(char delimiter);        //!< single-quoted string
    ECSqlToken ScanQuotedIdentifier(char closeDelim); //!< "", `` or []
    ECSqlToken ScanOperatorOrPunct();

    static ECSqlTokenType ClassifyKeyword(Utf8CP text, size_t len);

public:
    //! Construct a lexer over a null-terminated ECSQL string.
    explicit ECSqlLexer(Utf8CP ecsql);

    //! Return the next token. Repeated calls after EndOfInput return EndOfInput.
    ECSqlToken Next();

    //! Peek at the next token without consuming it.
    ECSqlToken Peek1();

    //! Return a snapshot of the current position so it can be restored with Restore().
    struct Snapshot { Utf8CP cur; uint32_t line; uint32_t col; };
    Snapshot SavePos() const;
    void RestorePos(Snapshot const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
