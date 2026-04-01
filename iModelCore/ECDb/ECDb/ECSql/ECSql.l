/* =============================================================================
 * ECSql.l — Flex lexer for ECSQL
 *
 * Tokenises ECSQL source text into tokens consumed by the BISON grammar
 * defined in ECSql.y.  All SQL keywords are matched case-insensitively.
 *
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * =========================================================================== */

%{
#include <stdlib.h>
#include <string.h>
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_SQLITE_EC
#include "ECSql.tab.hpp"

/* Simple token — keywords, operators, punctuation (no semantic value). */
#define YY_TOKEN(tok) do { return (tok); } while (0)

/* String token — store a heap-allocated copy of yytext in yylval->strVal.
   Used for identifiers, literals, and named parameters. */
#define YY_STR_TOKEN(tok) do { yylval->strVal = strdup(yytext); return (tok); } while (0)

/* Quoted identifier token — strip the surrounding delimiter characters (e.g. [ ] or " ").
   Returns the inner content without the outer quotes, matching ECSqlLexer behaviour. */
#define YY_QUOTED_TOKEN(tok) do { \
    int inner = yyleng - 2; \
    if (inner < 0) inner = 0; \
    yylval->strVal = (char*)malloc((size_t)inner + 1); \
    if (yylval->strVal) { memcpy(yylval->strVal, yytext + 1, (size_t)inner); yylval->strVal[inner] = '\0'; } \
    return (tok); \
} while (0)

/* String literal token — strip outer single quotes and unescape '' -> '.
   Matches the behaviour of ECSqlRDParser::StripStringQuotes. */
#define YY_STRING_TOKEN(tok) do { \
    int inner = yyleng - 2; \
    if (inner < 0) inner = 0; \
    const char* src = yytext + 1; \
    char* out = (char*)malloc((size_t)inner + 1); \
    if (out) { \
        int j = 0; \
        for (int i = 0; i < inner; i++) { \
            if (src[i] == '\'' && i + 1 < inner && src[i+1] == '\'') { out[j++] = '\''; i++; } \
            else { out[j++] = src[i]; } \
        } \
        out[j] = '\0'; \
        yylval->strVal = out; \
    } \
    return (tok); \
} while (0)

/* Named parameter token (:name) — strip the leading ':' to match ECSqlLexer behaviour.
   The RD lexer returns the name WITHOUT the colon; parameter binders prepend ':' themselves. */
#define YY_NAMED_PARAM_TOKEN(tok) do { \
    yylval->strVal = (char*)malloc((size_t)yyleng); /* yyleng-1 chars + null */ \
    if (yylval->strVal) { memcpy(yylval->strVal, yytext + 1, (size_t)yyleng - 1); yylval->strVal[yyleng-1] = '\0'; } \
    return (tok); \
} while (0)
%}

%option noyywrap reentrant bison-bridge case-insensitive
%option nounput noinput

/* ─── Exclusive start conditions ─────────────────────────────────────────── */
%x BLOCK_COMMENT

/* ─── Named patterns ─────────────────────────────────────────────────────── */
DIGIT       [0-9]
HEX         [0-9a-fA-F]
ALPHA       [a-zA-Z_]
ALNUM       [a-zA-Z_0-9]
WS          [ \t\r\n]+

%%

 /* ═══════════════════════════ Whitespace & comments ═══════════════════════ */

{WS}                    { /* skip whitespace */ }
"--"[^\n]*              { /* skip line comment */ }
"/*"                    { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"    { BEGIN(INITIAL); }
<BLOCK_COMMENT>.|\n    { /* consume block-comment body */ }

 /* ═══════════════════════════ Keywords (case-insensitive) ════════════════ */

ALL                     { YY_TOKEN(KW_ALL); }
AND                     { YY_TOKEN(KW_AND); }
ANY                     { YY_TOKEN(KW_ANY); }
AS                      { YY_TOKEN(KW_AS); }
ASC                     { YY_TOKEN(KW_ASC); }
AVG                     { YY_TOKEN(KW_AVG); }
BACKWARD                { YY_TOKEN(KW_BACKWARD); }
BETWEEN                 { YY_TOKEN(KW_BETWEEN); }
BINARY                  { YY_TOKEN(KW_BINARY); }
BLOB                    { YY_TOKEN(KW_BLOB); }
BOOLEAN                 { YY_TOKEN(KW_BOOLEAN); }
BY                      { YY_TOKEN(KW_BY); }
CASE                    { YY_TOKEN(KW_CASE); }
CAST                    { YY_TOKEN(KW_CAST); }
COLLATE                 { YY_TOKEN(KW_COLLATE); }
COUNT                   { YY_TOKEN(KW_COUNT); }
CROSS                   { YY_TOKEN(KW_CROSS); }
CUME_DIST               { YY_TOKEN(KW_CUME_DIST); }
CURRENT                 { YY_TOKEN(KW_CURRENT); }
CURRENT_DATE            { YY_TOKEN(KW_CURRENT_DATE); }
CURRENT_TIME            { YY_TOKEN(KW_CURRENT_TIME); }
CURRENT_TIMESTAMP       { YY_TOKEN(KW_CURRENT_TIMESTAMP); }
DATE                    { YY_TOKEN(KW_DATE); }
DELETE                  { YY_TOKEN(KW_DELETE); }
DENSE_RANK              { YY_TOKEN(KW_DENSE_RANK); }
DESC                    { YY_TOKEN(KW_DESC); }
DISTINCT                { YY_TOKEN(KW_DISTINCT); }
DOUBLE                  { YY_TOKEN(KW_DOUBLE); }
ECSQLOPTIONS            { YY_TOKEN(KW_ECSQLOPTIONS); }
ELSE                    { YY_TOKEN(KW_ELSE); }
END                     { YY_TOKEN(KW_END); }
ESCAPE                  { YY_TOKEN(KW_ESCAPE); }
EVERY                   { YY_TOKEN(KW_EVERY); }
EXCEPT                  { YY_TOKEN(KW_EXCEPT); }
EXCLUDE                 { YY_TOKEN(KW_EXCLUDE); }
EXISTS                  { YY_TOKEN(KW_EXISTS); }
FALSE                   { YY_TOKEN(KW_FALSE); }
FILTER                  { YY_TOKEN(KW_FILTER); }
FIRST                   { YY_TOKEN(KW_FIRST); }
FIRST_VALUE             { YY_TOKEN(KW_FIRST_VALUE); }
FLOAT                   { YY_TOKEN(KW_FLOAT); }
FOLLOWING               { YY_TOKEN(KW_FOLLOWING); }
FOR                     { YY_TOKEN(KW_FOR); }
FORWARD                 { YY_TOKEN(KW_FORWARD); }
FROM                    { YY_TOKEN(KW_FROM); }
FULL                    { YY_TOKEN(KW_FULL); }
GROUP                   { YY_TOKEN(KW_GROUP); }
GROUP_CONCAT            { YY_TOKEN(KW_GROUP_CONCAT); }
GROUPS                  { YY_TOKEN(KW_GROUPS); }
HAVING                  { YY_TOKEN(KW_HAVING); }
IIF                     { YY_TOKEN(KW_IIF); }
IN                      { YY_TOKEN(KW_IN); }
INNER                   { YY_TOKEN(KW_INNER); }
INSERT                  { YY_TOKEN(KW_INSERT); }
INTEGER                 { YY_TOKEN(KW_INT); }
INT                     { YY_TOKEN(KW_INT); }
INT64                   { YY_TOKEN(KW_INT64); }
INTERSECT               { YY_TOKEN(KW_INTERSECT); }
INTO                    { YY_TOKEN(KW_INTO); }
IS                      { YY_TOKEN(KW_IS); }
JOIN                    { YY_TOKEN(KW_JOIN); }
LAG                     { YY_TOKEN(KW_LAG); }
LAST                    { YY_TOKEN(KW_LAST); }
LAST_VALUE              { YY_TOKEN(KW_LAST_VALUE); }
LEAD                    { YY_TOKEN(KW_LEAD); }
LEFT                    { YY_TOKEN(KW_LEFT); }
LIKE                    { YY_TOKEN(KW_LIKE); }
LIMIT                   { YY_TOKEN(KW_LIMIT); }
LONG                    { YY_TOKEN(KW_LONG); }
MATCH                   { YY_TOKEN(KW_MATCH); }
MAX                     { YY_TOKEN(KW_MAX); }
MIN                     { YY_TOKEN(KW_MIN); }
NATURAL                 { YY_TOKEN(KW_NATURAL); }
NAVIGATION_VALUE        { YY_TOKEN(KW_NAVIGATION_VALUE); }
NO                      { YY_TOKEN(KW_NO); }
NOCASE                  { YY_TOKEN(KW_NOCASE); }
NOT                     { YY_TOKEN(KW_NOT); }
NTH_VALUE               { YY_TOKEN(KW_NTH_VALUE); }
NTILE                   { YY_TOKEN(KW_NTILE); }
NULL                    { YY_TOKEN(KW_NULL); }
NULLS                   { YY_TOKEN(KW_NULLS); }
OFFSET                  { YY_TOKEN(KW_OFFSET); }
ON                      { YY_TOKEN(KW_ON); }
ONLY                    { YY_TOKEN(KW_ONLY); }
OPTIONS                 { YY_TOKEN(KW_OPTIONS); }
OR                      { YY_TOKEN(KW_OR); }
ORDER                   { YY_TOKEN(KW_ORDER); }
OTHERS                  { YY_TOKEN(KW_OTHERS); }
OUTER                   { YY_TOKEN(KW_OUTER); }
OVER                    { YY_TOKEN(KW_OVER); }
PARTITION               { YY_TOKEN(KW_PARTITION); }
PERCENT_RANK            { YY_TOKEN(KW_PERCENT_RANK); }
PRAGMA                  { YY_TOKEN(KW_PRAGMA); }
PRECEDING               { YY_TOKEN(KW_PRECEDING); }
RANGE                   { YY_TOKEN(KW_RANGE); }
RANK                    { YY_TOKEN(KW_RANK); }
REAL                    { YY_TOKEN(KW_REAL); }
RECURSIVE               { YY_TOKEN(KW_RECURSIVE); }
RIGHT                   { YY_TOKEN(KW_RIGHT); }
ROW                     { YY_TOKEN(KW_ROW); }
ROWS                    { YY_TOKEN(KW_ROWS); }
ROW_NUMBER              { YY_TOKEN(KW_ROW_NUMBER); }
RTRIM                   { YY_TOKEN(KW_RTRIM); }
SELECT                  { YY_TOKEN(KW_SELECT); }
SET                     { YY_TOKEN(KW_SET); }
SOME                    { YY_TOKEN(KW_SOME); }
STRING                  { YY_TOKEN(KW_STRING_KW); }
SUM                     { YY_TOKEN(KW_SUM); }
THEN                    { YY_TOKEN(KW_THEN); }
TIES                    { YY_TOKEN(KW_TIES); }
TIME                    { YY_TOKEN(KW_TIME); }
TIMESTAMP               { YY_TOKEN(KW_TIMESTAMP); }
TOTAL                   { YY_TOKEN(KW_TOTAL); }
TRUE                    { YY_TOKEN(KW_TRUE); }
UNBOUNDED               { YY_TOKEN(KW_UNBOUNDED); }
UNION                   { YY_TOKEN(KW_UNION); }
UNIQUE                  { YY_TOKEN(KW_UNIQUE); }
UNKNOWN                 { YY_TOKEN(KW_UNKNOWN); }
UPDATE                  { YY_TOKEN(KW_UPDATE); }
USING                   { YY_TOKEN(KW_USING); }
VALUE                   { YY_TOKEN(KW_VALUE); }
VALUES                  { YY_TOKEN(KW_VALUES); }
VARCHAR                 { YY_TOKEN(KW_VARCHAR); }
WHEN                    { YY_TOKEN(KW_WHEN); }
WHERE                   { YY_TOKEN(KW_WHERE); }
WINDOW                  { YY_TOKEN(KW_WINDOW); }
WITH                    { YY_TOKEN(KW_WITH); }

 /* ═══════════════════════════ Numeric literals ═══════════════════════════ */

0[xX]{HEX}+                                     { YY_STR_TOKEN(INTEGER_LITERAL); }
{DIGIT}+                                         { YY_STR_TOKEN(INTEGER_LITERAL); }

{DIGIT}+"."{DIGIT}*([eE][+-]?{DIGIT}+)?         { YY_STR_TOKEN(APPROX_LITERAL); }
{DIGIT}*"."{DIGIT}+([eE][+-]?{DIGIT}+)?         { YY_STR_TOKEN(APPROX_LITERAL); }
{DIGIT}+[eE][+-]?{DIGIT}+                       { YY_STR_TOKEN(APPROX_LITERAL); }

 /* ═══════════════════════════ String literal (single-quoted) ═════════════ */
 /* SQL-style: embedded quotes are doubled, e.g. 'it''s' */

'([^']|'')*'                                     { YY_STRING_TOKEN(STRING_LITERAL); }

 /* ═══════════════════════════ Quoted identifiers ═════════════════════════ */

\"([^"\\]|\\.)*\"                                { YY_QUOTED_TOKEN(NAME); }
`([^`\\]|\\.)*`                                  { YY_QUOTED_TOKEN(NAME); }
"["[^\]]*"]"                                     { YY_QUOTED_TOKEN(NAME); }

 /* ═══════════════════════════ Bare identifiers ═══════════════════════════ */

{ALPHA}{ALNUM}*                                  { YY_STR_TOKEN(NAME); }

 /* ═══════════════════════════ Parameters ═════════════════════════════════ */

"?"                     { YY_TOKEN(PARAMETER); }
":"{ALPHA}{ALNUM}*      { YY_NAMED_PARAM_TOKEN(NAMED_PARAM); }

 /* ═══════════════════════════ Multi-character operators ═══════════════════ */

"<="                    { YY_TOKEN(LESS_EQ); }
">="                    { YY_TOKEN(GREAT_EQ); }
"<>"                    { YY_TOKEN(NOT_EQUAL); }
"!="                    { YY_TOKEN(NOT_EQUAL); }
"||"                    { YY_TOKEN(CONCAT); }
"->"                    { YY_TOKEN(ARROW); }
"<<"                    { YY_TOKEN(SHIFT_LEFT); }
">>"                    { YY_TOKEN(SHIFT_RIGHT); }

 /* ═══════════════════════════ Single-character tokens ═════════════════════ */

"<"                     { YY_TOKEN(LESS); }
">"                     { YY_TOKEN(GREAT); }
"="                     { YY_TOKEN(EQUAL); }
"~"                     { YY_TOKEN(BITWISE_NOT); }
"|"                     { YY_TOKEN(BITWISE_OR); }
"&"                     { YY_TOKEN(BITWISE_AND); }

"-"                     { YY_TOKEN(MINUS); }
"+"                     { YY_TOKEN(PLUS); }
"*"                     { YY_TOKEN(STAR); }
"/"                     { YY_TOKEN(SLASH); }
"%"                     { YY_TOKEN(PERCENT); }
":"                     { YY_TOKEN(COLON); }
"("                     { YY_TOKEN(LPAREN); }
")"                     { YY_TOKEN(RPAREN); }
","                     { YY_TOKEN(COMMA); }
"."                     { YY_TOKEN(DOT); }
";"                     { YY_TOKEN(SEMICOLON); }
"{"                     { YY_TOKEN(LBRACE); }
"}"                     { YY_TOKEN(RBRACE); }
"["                     { YY_TOKEN(LBRACKET); }
"]"                     { YY_TOKEN(RBRACKET); }
"$"                     { YY_TOKEN(DOLLAR); }

 /* ═══════════════════════════ Catch-all (invalid character) ══════════════ */

.                       { return yytext[0]; }

%%
