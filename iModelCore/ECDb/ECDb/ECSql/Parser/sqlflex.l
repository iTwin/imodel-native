%top{
//------------------------------------------------------------
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
//------------------------------------------------------------

#pragma warning(disable:4603)
#include "ECDbPch.h"
#define YY_NO_UNISTD_H
#if defined __GNUC__
 //#pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
//#pragma warning(push, 1)
#pragma warning(disable:4505)
#pragma warning(disable:4244)

/**/
#ifdef yywrap
#undef  yywrap
#define yywrap() 1
#endif
/**/
#endif

}


%{
#define YY_EXIT 1                // YY_FATAL will not halt the application

#ifndef _CSTDARG_
#include <cstdarg>      // std::va_list
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _CONNECTIVITY_SQLYACC_HXX
#define _CONNECTIVITY_SQLYACC_HXX

#ifndef SQLYYDEBUG
#define SQLYYDEBUG 1
#endif

#include "SqlBison.h"
#endif
#include "SqlScan.h"
#include "SqlParse.h"


using namespace connectivity;

//=============================================================================
//
// Erzeugung der Blaetter fuer die Token
// Blaetter werden generell vom Lexer erzeugt

static Utf8String aEmptyString;

static sal_Int32    gatherString(yyscan_t yyscanner, sal_Char delim, sal_Int32 nTyp);
static sal_Int32    gatherName(yyscan_t yyscanner, const sal_Char*);
static sal_Int32    gatherNamePre(yyscan_t yyscanner, const sal_Char* );
static sal_Int32    parseString(yyscan_t yyscanner);

#define SQL_NEW_NODE(text, token) \
        SQLyyget_lval(yyscanner)->pParseNode=yyget_extra(yyscanner)->NewNode(text, token);

#define SQL_NEW_KEYWORD(token) \
        SQLyyget_lval(yyscanner)->pParseNode=yyget_extra(yyscanner)->NewNode(aEmptyString, SQL_NODE_KEYWORD, (token)); return token;

#define SQL_NEW_INTNUM         SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_INTNUM); return SQL_TOKEN_INTNUM;
#define SQL_NEW_APPROXNUM      SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_APPROXNUM); return SQL_TOKEN_APPROXNUM;
#define SQL_NEW_DATE           SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_ACCESS_DATE); return SQL_TOKEN_ACCESS_DATE;
#define YY_EXTRA_TYPE OSQLScanner*
#define YY_INPUT(buf,result,max_size) \
{ \
    int c = yyget_extra(yyscanner)->SQLyygetc(); \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
}
#define YY_FATAL_ERROR(msg) \
{ \
    yyget_extra(yyscanner)->SQLyyerror(msg); \
}

//
//=============================================================================

%}

%s SQL
%s PREDICATE_ENG
%s PREDICATE_GER
%s DATE
%s STRING
%option bison-bridge
%option reentrant
%option noyywrap
%option never-interactive
%%

[Aa][Ll][Ll]        {SQL_NEW_KEYWORD(SQL_TOKEN_ALL);  }
[Aa][Nn][Dd]        {SQL_NEW_KEYWORD(SQL_TOKEN_AND);  }
[Aa][Nn][Yy]        {SQL_NEW_KEYWORD(SQL_TOKEN_ANY);  }
[Aa][Ss]            {SQL_NEW_KEYWORD(SQL_TOKEN_AS);  }
[Aa][Ss][Cc]        {SQL_NEW_KEYWORD(SQL_TOKEN_ASC);  }
[Aa][Vv][Gg]        {SQL_NEW_KEYWORD(SQL_TOKEN_AVG);  }

[Bb][Aa][Cc][Kk][Ww][Aa][Rr][Dd] {SQL_NEW_KEYWORD(SQL_TOKEN_BACKWARD);  }
[Bb][Ee][Tt][Ww][Ee][Ee][Nn] {SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN);  }
[Bb][Ii][Nn][Aa][Rr][Yy] {SQL_NEW_KEYWORD(SQL_TOKEN_BINARY);  }
[Bb][Ll][Oo][Bb]    {SQL_NEW_KEYWORD(SQL_TOKEN_BLOB);  }
[Bb][Oo][Oo][Ll][Ee][Aa][Nn] {SQL_NEW_KEYWORD(SQL_TOKEN_BOOLEAN);  }
[Bb][Yy]            {SQL_NEW_KEYWORD(SQL_TOKEN_BY);  }

[Cc][Aa][Ss][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_CASE);  }
[Cc][Aa][Ss][Tt]    {SQL_NEW_KEYWORD(SQL_TOKEN_CAST);  }
[Cc][Oo][Ll][Ll][Aa][Tt][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_COLLATE);  }
[Cc][Oo][Uu][Nn][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_COUNT);  }
[Cc][Rr][Oo][Ss][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_CROSS);  }
[Cc][Uu][Mm][Ee]_[Dd][Ii][Ss][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_CUME_DIST);  }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT);  }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Dd][Aa][Tt][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE);  }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Tt][Ii][Mm][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIME);  }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp] {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP);  }

[Dd][Aa][Tt][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_DATE);  }
[Dd][Ee][Ll][Ee][Tt][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_DELETE);  }
[Dd][Ee][Nn][Ss][Ee]_[Rr][Aa][Nn][Kk] {SQL_NEW_KEYWORD(SQL_TOKEN_DENSE_RANK);  }
[Dd][Ee][Ss][Cc]    {SQL_NEW_KEYWORD(SQL_TOKEN_DESC);  }
[Dd][Ii][Ss][Tt][Ii][Nn][Cc][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT);  }
[Dd][Oo][Uu][Bb][Ll][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE);  }

[Ee][Cc][Ss][Qq][Ll][Oo][Pp][Tt][Ii][Oo][Nn][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_ECSQLOPTIONS);  }
[Oo][Pp][Tt][Ii][Oo][Nn][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_ECSQLOPTIONS);  }
[Ee][Ll][Ss][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_ELSE);  }
[Ee][Nn][Dd]        {SQL_NEW_KEYWORD(SQL_TOKEN_END);  }
[Ee][Ss][Cc][Aa][Pp][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE);  }
[Ee][Vv][Ee][Rr][Yy] {SQL_NEW_KEYWORD(SQL_TOKEN_EVERY);  }
[Ee][Xx][Cc][Ee][Pp][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT);  }
[Ee][Xx][Cc][Ll][Uu][Dd][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_EXCLUDE); }
[Ee][Xx][Ii][Ss][Tt][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS);  }
[Ff][Aa][Ll][Ss][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_FALSE);  }
[Ff][Ll][Oo][Aa][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT);  }

[Ff][Ii][Rr][Ss][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST);  }
[Ll][Aa][Ss][Tt]    {SQL_NEW_KEYWORD(SQL_TOKEN_LAST);  }
[Nn][Uu][Ll][Ll][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_NULLS);  }

[Ff][Ii][Ll][Tt][Ee][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_FILTER);  }
[Ff][Ii][Rr][Ss][Tt]_[Vv][Aa][Ll][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST_VALUE);  }
[Ff][Oo][Ll][Ll][Oo][Ww][Ii][Nn][Gg] {SQL_NEW_KEYWORD(SQL_TOKEN_FOLLOWING);  }
[Ff][Oo][Rr]        {SQL_NEW_KEYWORD(SQL_TOKEN_FOR);  }
[Ff][Oo][Rr][Ww][Aa][Rr][Dd] {SQL_NEW_KEYWORD(SQL_TOKEN_FORWARD);  }
[Ff][Rr][Oo][Mm]    {SQL_NEW_KEYWORD(SQL_TOKEN_FROM);  }
[Ff][Uu][Ll][Ll]    {SQL_NEW_KEYWORD(SQL_TOKEN_FULL);  }

[Gg][Rr][Oo][Uu][Pp] {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP);  }
[Gg][Rr][Oo][Uu][Pp]_[Cc][Oo][Nn][Cc][Aa][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP_CONCAT);  }
[Gg][Rr][Oo][Uu][Pp][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_GROUPS);  }

[Hh][Aa][Vv][Ii][Nn][Gg] {SQL_NEW_KEYWORD(SQL_TOKEN_HAVING);  }

[Ii][Ii][Ff]        {SQL_NEW_KEYWORD(SQL_TOKEN_IIF);  }
[Ii][Nn]            {SQL_NEW_KEYWORD(SQL_TOKEN_IN);  }
[Ii][Nn][Nn][Ee][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_INNER);  }
[Ii][Nn][Ss][Ee][Rr][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_INSERT);  }
[Ii][Nn][Tt]([Ee][Gg][Ee][Rr])? {SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER);  }
[Ii][Nn][Tt]64      {SQL_NEW_KEYWORD(SQL_TOKEN_INT64);  }
[Ii][Nn][Tt][Ee][Rr][Ss][Ee][Cc][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT);  }
[Ii][Nn][Tt][Oo]    {SQL_NEW_KEYWORD(SQL_TOKEN_INTO);  }
[Ii][Ss]            {SQL_NEW_KEYWORD(SQL_TOKEN_IS);  }

[Jj][Oo][Ii][Nn]    {SQL_NEW_KEYWORD(SQL_TOKEN_JOIN);  }

[Ll][Aa][Gg]        {SQL_NEW_KEYWORD(SQL_TOKEN_LAG);  }
[Ll][Aa][Ss][Tt]_[Vv][Aa][Ll][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_LAST_VALUE); }
[Ll][Ee][Aa][Dd]    {SQL_NEW_KEYWORD(SQL_TOKEN_LEAD);  }
[Ll][Ee][Ff][Tt]    {SQL_NEW_KEYWORD(SQL_TOKEN_LEFT);  }
[Ll][Ii][Kk][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_LIKE);  }
[Ll][Ii][Mm][Ii][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_LIMIT);  }
[Ll][Oo][Nn][Gg]    {SQL_NEW_KEYWORD(SQL_TOKEN_LONG);  }

[Mm][Aa][Tt][Cc][Hh] {SQL_NEW_KEYWORD(SQL_TOKEN_MATCH);  }
[Mm][Aa][Xx]        {SQL_NEW_KEYWORD(SQL_TOKEN_MAX);  }
[Mm][Ii][Nn]        {SQL_NEW_KEYWORD(SQL_TOKEN_MIN);  }

[Nn][Aa][Tt][Uu][Rr][Aa][Ll] {SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL);  }
[Nn][Aa][Vv][Ii][Gg][Aa][Tt][Ii][Oo][Nn]_[Vv][Aa][Ll][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_NAVIGATION_VALUE);  }
[Nn][Oo]            {SQL_NEW_KEYWORD(SQL_TOKEN_NO);  }
[Nn][Oo][Cc][Aa][Ss][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_NOCASE);  }
[Nn][Oo][Tt]        {SQL_NEW_KEYWORD(SQL_TOKEN_NOT);  }
[Nn][Tt][Hh]_[Vv][Aa][Ll][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_NTH_VALUE); }
[Nn][Tt][Ii][Ll][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_NTILE);  }
[Nn][Uu][Ll][Ll]    {SQL_NEW_KEYWORD(SQL_TOKEN_NULL);  }

[Oo][Ff][Ff][Ss][Ee][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_OFFSET);  }
[Oo][Nn]            {SQL_NEW_KEYWORD(SQL_TOKEN_ON);  }
[Oo][Nn][Ll][Yy]    {SQL_NEW_KEYWORD(SQL_TOKEN_ONLY);  }
[Oo][Rr]            {SQL_NEW_KEYWORD(SQL_TOKEN_OR);  }
[Oo][Rr][Dd][Ee][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_ORDER);  }
[Oo][Tt][Hh][Ee][Rr][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_OTHERS);  }
[Oo][Uu][Tt][Ee][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_OUTER);  }
[Oo][Vv][Ee][Rr]    {SQL_NEW_KEYWORD(SQL_TOKEN_OVER);  }

[Pp][Aa][Rr][Tt][Ii][Tt][Ii][Oo][Nn] {SQL_NEW_KEYWORD(SQL_TOKEN_PARTITION);  }
[Pp][Ee][Rr][Cc][Ee][Nn][Tt]_[Rr][Aa][Nn][Kk] {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENT_RANK);  }
[Pp][Rr][Aa][Gg][Mm][Aa] {SQL_NEW_KEYWORD(SQL_TOKEN_PRAGMA);  }
[Pp][Rr][Ee][Cc][Ee][Dd][Ii][Nn][Gg] {SQL_NEW_KEYWORD(SQL_TOKEN_PRECEDING);  }

[Rr][Aa][Nn][Gg][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_RANGE);  }
[Rr][Aa][Nn][Kk]    {SQL_NEW_KEYWORD(SQL_TOKEN_RANK);  }
[Rr][Ee][Aa][Ll]    {SQL_NEW_KEYWORD(SQL_TOKEN_REAL);  }
[Rr][Ee][Cc][Uu][Rr][Ss][Ii][Vv][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_RECURSIVE);  }
[Rr][Ii][Gg][Hh][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT);  }
[Rr][Oo][Ww]        {SQL_NEW_KEYWORD(SQL_TOKEN_ROW);  }
[Rr][Oo][Ww][Ss]    {SQL_NEW_KEYWORD(SQL_TOKEN_ROWS);  }
[Rr][Oo][Ww]_[Nn][Uu][Mm][Bb][Ee][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_ROW_NUMBER);  }
[Rr][Tt][Rr][Ii][Mm] {SQL_NEW_KEYWORD(SQL_TOKEN_RTRIM);  }

[Ss][Ee][Ll][Ee][Cc][Tt] {SQL_NEW_KEYWORD(SQL_TOKEN_SELECT);  }
[Ss][Ee][Tt]        {SQL_NEW_KEYWORD(SQL_TOKEN_SET);  }
[Ss][Oo][Mm][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_SOME);  }
[Ss][Tt][Rr][Ii][Nn][Gg] {SQL_NEW_KEYWORD(SQL_TOKEN_STRING);  }
[Ss][Uu][Mm]        {SQL_NEW_KEYWORD(SQL_TOKEN_SUM);  }

[Tt][Hh][Ee][Nn]    {SQL_NEW_KEYWORD(SQL_TOKEN_THEN);  }
[Tt][Ii][Ee][Ss]    {SQL_NEW_KEYWORD(SQL_TOKEN_TIES);  }
[Tt][Ii][Mm][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_TIME);  }
[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp] {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP);  }
[Tt][Oo][Tt][Aa][Ll] {SQL_NEW_KEYWORD(SQL_TOKEN_TOTAL);  }
[Tt][Rr][Uu][Ee]    {SQL_NEW_KEYWORD(SQL_TOKEN_TRUE);  }

[Uu][Nn][Bb][Oo][Uu][Nn][Dd][Ee][Dd] {SQL_NEW_KEYWORD(SQL_TOKEN_UNBOUNDED);  }
[Uu][Nn][Ii][Oo][Nn] {SQL_NEW_KEYWORD(SQL_TOKEN_UNION);  }
[Uu][Nn][Ii][Qq][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE);  }
[Uu][Nn][Kk][Nn][Oo][Ww][Nn] {SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN);  }
[Uu][Pp][Dd][Aa][Tt][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE);  }
[Uu][Ss][Ii][Nn][Gg] {SQL_NEW_KEYWORD(SQL_TOKEN_USING);  }

[Vv][Aa][Ll][Uu][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_VALUE);  }
[Vv][Aa][Ll][Uu][Ee][Ss] {SQL_NEW_KEYWORD(SQL_TOKEN_VALUES);  }
[Vv][Aa][Rr][Cc][Hh][Aa][Rr] {SQL_NEW_KEYWORD(SQL_TOKEN_VARCHAR);  }

[Ww][Hh][Ee][Nn]    {SQL_NEW_KEYWORD(SQL_TOKEN_WHEN);  }
[Ww][Hh][Ee][Rr][Ee] {SQL_NEW_KEYWORD(SQL_TOKEN_WHERE);  }
[Ww][Ii][Tt][Hh]    {SQL_NEW_KEYWORD(SQL_TOKEN_WITH);  }
[Ww][Ii][Nn][Dd][Oo][Ww] {SQL_NEW_KEYWORD(SQL_TOKEN_WINDOW);  }
"$"                 {SQL_NEW_KEYWORD(SQL_TOKEN_DOLLAR);}

"<"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_LESS);return SQL_LESS;}
">"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_GREAT);return SQL_GREAT;}
"="                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_EQUAL);return SQL_EQUAL;}
"<="                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_LESSEQ);return SQL_LESSEQ;}
">="                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_GREATEQ);return SQL_GREATEQ;}
"<>"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"!="                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"||"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_CONCAT);return SQL_CONCAT;}
"->"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_ARROW);return SQL_ARROW;}

"~"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_NOT);return SQL_BITWISE_NOT;}
"|"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_OR);return SQL_BITWISE_OR;}
"&"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_AND);return SQL_BITWISE_AND;}
"<<"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_SHIFT_LEFT);return SQL_BITWISE_SHIFT_LEFT;}
">>"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_SHIFT_RIGHT);return SQL_BITWISE_SHIFT_RIGHT;}

[-+*/%:(),.;?{}&|~] { return yytext[0]; }


<SQL>[A-Za-z\200-\375_][A-Za-z\200-\375_0-9]*      {return gatherName(yyscanner, yytext);}

<SQL>([0-9]+)                                {SQL_NEW_INTNUM; }
<SQL>(0[Xx][0-9a-fA-F]+)                     {SQL_NEW_INTNUM; }

<SQL>("."[0-9]*) |
<SQL>([0-9]+"."[0-9]*) |
<SQL>[0-9]+[eE][+-]?[0-9]+   |
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]+[eE][+-]?[0-9]+                {SQL_NEW_APPROXNUM;  }

<PREDICATE_GER,PREDICATE_ENG,DATE>[A-Za-z\200-\375][A-Za-z0-9_%.,*?\200-\375]* {return gatherNamePre(yyscanner,yytext);}

<PREDICATE_GER,PREDICATE_ENG>([0-9]+)        {SQL_NEW_INTNUM; }
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+)    {SQL_NEW_INTNUM; }
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+)    {SQL_NEW_INTNUM; }
<PREDICATE_ENG>([0-9]+"."[0-9]+) |
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+"."[0-9]+) |
<PREDICATE_ENG>("."[0-9]+)                   {SQL_NEW_APPROXNUM;  }
<PREDICATE_ENG>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_ENG>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_ENG>"."[0-9]*[eE][+-]?[0-9]+       {SQL_NEW_APPROXNUM;  }

<PREDICATE_GER>([0-9]+","[0-9]+) |
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+","[0-9]+) |
<PREDICATE_GER>(","[0-9]+)                    {SQL_NEW_APPROXNUM;  }
<PREDICATE_GER>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_GER>[0-9]+","[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_GER>","[0-9]*[eE][+-]?[0-9]+        {SQL_NEW_APPROXNUM;  }
<PREDICATE_GER,PREDICATE_ENG>[0-9.,][A-Za-z0-9_.,%]* {return gatherNamePre(yyscanner, yytext);}

<SQL>\" { return gatherString(yyscanner, '\"',0); }
<SQL>`  { return gatherString(yyscanner, '`' ,0); }
\'      { return gatherString(yyscanner, '\'',1); }

<PREDICATE_GER,PREDICATE_ENG,DATE,SQL>"[" { return gatherString(yyscanner, ']' ,0);}
<PREDICATE_GER,PREDICATE_ENG,DATE>#       { return gatherString(yyscanner, '#' ,2);}

<DATE>[0-9]{1,4}[^ ]*[0-9] |
<DATE>[0-9]{1,4}[^ ]*[0-9][ ][0-9]{1,4}[^ ]*[0-9]        { SQL_NEW_DATE; }
<STRING>["-""+""*""/"":""("")"",""."";""?""{""}"]        { return yytext[0]; } /*  */
<STRING>"["                    { return gatherString(yyscanner, ']', 0); }
<STRING>[^ ':["?"]*            { return gatherNamePre(yyscanner, yytext); }
\n              {}
[ \t\r]+        {}
"--".*$         {}
.                {
                Utf8String tmp;
                tmp.Sprintf("Invalid symbol (char=%c, hex=%0x, dec=%" PRIu64 ")", yytext[0], yytext[0], (uint64_t) yytext[0]);
                YY_FATAL_ERROR(tmp.c_str());
                return SQL_TOKEN_INVALIDSYMBOL;
                }

%%

// Kludge around a bug (well, Posix incompatibility) in flex 2.5.x
// http://bugs.debian.org/cgi-bin/bugreport.cgi?archive=no&bug=189332
#if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5

  #ifndef YY_FLUSH_BUFFER
    #define YY_FLUSH_BUFFER SQLyy_flush_buffer(YY_CURRENT_BUFFER )
  #endif

  #ifndef yytext_ptr
    #define yytext_ptr yytext
  #endif

#endif

// Versions of flex apparently differ in whether input() resp. yyinput() returns
// zero or EOF upon end of file:
//------------------------------------------------------------------------------
inline bool checkeof(int c) { return c == 0 || c == EOF || c == 0xff; }
//------------------------------------------------------------------------------
sal_Int32 parseString (yyscan_t yyscanner)
    {
    sal_Char ch;
    sal_Char delim = '\''; //'' ''
    Utf8String sBuffer;
    sBuffer.reserve(256);
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    while (!checkeof (ch = yyinput (yyscanner)))
    {
    if (ch == delim)
        {
        ch = yyinput (yyscanner);
        if (checkeof (ch))
        {
        SQL_NEW_NODE (sBuffer, SQL_NODE_STRING);
        return SQL_TOKEN_STRING;
        }
        else if (ch == ' ')
        {
        ch = yyinput (yyscanner);
        if (ch == delim)
            {
            sBuffer.append (&delim, 1);
            }
        else
            {
            if (!checkeof (ch))
            unput (ch);

            SQL_NEW_NODE (sBuffer, SQL_NODE_STRING);
            return SQL_TOKEN_STRING;
            }
        }
        else if (ch == delim)
        {
        sBuffer.append (&delim, 1);
        continue;
        }
        else
        {
        if (!checkeof (ch))
            unput (ch);

        SQL_NEW_NODE (sBuffer, SQL_NODE_STRING);
        return SQL_TOKEN_STRING;
        }
        }
    else
        {
        sBuffer.append (&ch, 1);
        }
    }

    YY_FATAL_ERROR ("Unterminated name string");
    return SQL_TOKEN_INVALIDSYMBOL;
    }


/*
 * Read SQL string literal
 * Valid strings:
 *  ''  'a string'  'quote '' within string'
 *  ""  "a string"  "quote "" within string"
 * nTyp == 0 -> SQL_NODE_NAME
 * nTyp == 1 -> SQL_NODE_STRING
 * nTyp == 2 -> SQL_NODE_ACCESS_DATE
 */
sal_Int32 gatherString(yyscan_t yyscanner, sal_Char delim, sal_Int32 nTyp)
{
    sal_Char ch;
    Utf8String sBuffer;
    sBuffer.reserve(256);
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    assert(nTyp == 0 || nTyp == 1 || nTyp == 2);

    while (!checkeof(ch = yyinput(yyscanner)))
    {
        if (ch == delim)
        {
            if ((ch = yyinput(yyscanner)) != delim)
            {
                if (!checkeof(ch))
                    unput(ch);

                switch(nTyp)
                {
                case 0:
                    SQL_NEW_NODE(sBuffer, SQL_NODE_NAME);
                    return SQL_TOKEN_NAME;
                case 1:
                    SQL_NEW_NODE(sBuffer, SQL_NODE_STRING);
                    return SQL_TOKEN_STRING;
                case 2:
                    SQL_NEW_NODE(sBuffer, SQL_NODE_ACCESS_DATE);
                    return SQL_TOKEN_ACCESS_DATE;
                }
            }
            else
            {
                sBuffer.append(&ch, 1);
            }

        }
        else if (nTyp == 2 && (ch == '\r' || ch == '\n') )
            break;
        else
        {
            sBuffer.append(&ch, 1);
        }
    }
    YY_FATAL_ERROR("Unterminated name string");
    return SQL_TOKEN_INVALIDSYMBOL;
}

//------------------------------------------------------------------------------
sal_Int32 mapEnumToToken(IParseContext::InternationalKeyCode _eKeyCode )
    {
    sal_Int32 nTokenID = 0;
    switch( _eKeyCode )
    {
    case IParseContext::KEY_LIKE:       nTokenID = SQL_TOKEN_LIKE;      break;
    case IParseContext::KEY_NOT:        nTokenID = SQL_TOKEN_NOT;       break;
    case IParseContext::KEY_NULL:       nTokenID = SQL_TOKEN_NULL;      break;
    case IParseContext::KEY_TRUE:       nTokenID = SQL_TOKEN_TRUE;      break;
    case IParseContext::KEY_FALSE:      nTokenID = SQL_TOKEN_FALSE;     break;
    case IParseContext::KEY_IS:         nTokenID = SQL_TOKEN_IS;        break;
    case IParseContext::KEY_BETWEEN:    nTokenID = SQL_TOKEN_BETWEEN;   break;
    case IParseContext::KEY_OR:         nTokenID = SQL_TOKEN_OR;        break;
    case IParseContext::KEY_AND:        nTokenID = SQL_TOKEN_AND;       break;
    case IParseContext::KEY_AVG:        nTokenID = SQL_TOKEN_AVG;       break;
    case IParseContext::KEY_COUNT:      nTokenID = SQL_TOKEN_COUNT;     break;
    case IParseContext::KEY_MAX:        nTokenID = SQL_TOKEN_MAX;       break;
    case IParseContext::KEY_MIN:        nTokenID = SQL_TOKEN_MIN;       break;
    case IParseContext::KEY_SUM:        nTokenID = SQL_TOKEN_SUM;       break;
    case IParseContext::KEY_EVERY:      nTokenID = SQL_TOKEN_EVERY;     break;
    case IParseContext::KEY_ANY:        nTokenID = SQL_TOKEN_ANY;       break;
    case IParseContext::KEY_SOME:       nTokenID = SQL_TOKEN_SOME;      break;
    default:
        OSL_FAIL( "mapEnumToToken: unsupported key!" );
    }
    return nTokenID;
    }

/*
 * Read SQL Name literal
 * Valid Names or international keywords:
 *  As we have international keywords, we test first on them
 */
 //------------------------------------------------------------------------------
sal_Int32 gatherName(yyscan_t yyscanner, const sal_Char* text)
    {
    sal_Int32 nToken;
    OSL_ENSURE(yyget_extra(yyscanner),"You forgot to set the scanner!");
    IParseContext::InternationalKeyCode eKeyCode = yyget_extra(yyscanner)->getInternationalTokenID(text);
    switch (eKeyCode)
        {
        case IParseContext::KEY_LIKE:
        case IParseContext::KEY_NOT:
        case IParseContext::KEY_NULL:
        case IParseContext::KEY_TRUE:
        case IParseContext::KEY_FALSE:
        case IParseContext::KEY_IS:
        case IParseContext::KEY_BETWEEN:
        case IParseContext::KEY_OR:
        case IParseContext::KEY_AND:
        case IParseContext::KEY_COUNT:
        case IParseContext::KEY_AVG:
        case IParseContext::KEY_MAX:
        case IParseContext::KEY_MIN:
        case IParseContext::KEY_SUM:
        case IParseContext::KEY_EVERY:
        case IParseContext::KEY_ANY:
        case IParseContext::KEY_SOME:
        case IParseContext::KEY_COLLECT:
        case IParseContext::KEY_FUSION:
        case IParseContext::KEY_INTERSECTION:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
            SQL_NEW_NODE(Utf8String(text), SQL_NODE_NAME);
            return SQL_TOKEN_NAME;
        }
    }

/**
 Read SQL Name literal for predicate check
 Valid Names or internatioanl keywords:
 As we have international keywords, we test first on them
*/

//------------------------------------------------------------------------------
sal_Int32 gatherNamePre(yyscan_t yyscanner, const sal_Char* text)
{
    sal_Int32 nToken;
    OSL_ENSURE(yyget_extra(yyscanner),"You forgot to set the scanner!");
    IParseContext::InternationalKeyCode eKeyCode = yyget_extra(yyscanner)->getInternationalTokenID(text);
    switch (eKeyCode)
    {
        case IParseContext::KEY_LIKE:
        case IParseContext::KEY_NOT:
        case IParseContext::KEY_NULL:
        case IParseContext::KEY_TRUE:
        case IParseContext::KEY_FALSE:
        case IParseContext::KEY_IS:
        case IParseContext::KEY_BETWEEN:
        case IParseContext::KEY_OR:
        case IParseContext::KEY_AND:
        case IParseContext::KEY_COUNT:
        case IParseContext::KEY_AVG:
        case IParseContext::KEY_MAX:
        case IParseContext::KEY_MIN:
        case IParseContext::KEY_SUM:
        case IParseContext::KEY_EVERY:
        case IParseContext::KEY_ANY:
        case IParseContext::KEY_SOME:
        case IParseContext::KEY_COLLECT:
        case IParseContext::KEY_FUSION:
        case IParseContext::KEY_INTERSECTION:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
        // we need a special handling for parameter
            {
            Utf8String sStmt = yyget_extra(yyscanner)->getStatement();
            size_t nLength = strlen(text);
            size_t nPos = yyget_extra(yyscanner)->GetCurrentPos() - nLength - 2;
            if (sStmt[nPos] == ':')
            {
                SQL_NEW_NODE(Utf8String(text), SQL_NODE_NAME);
                nToken = SQL_TOKEN_NAME;
            }
            else
            {
                SQL_NEW_NODE(Utf8String(text), SQL_NODE_STRING);
                nToken = SQL_TOKEN_STRING;
            }
            }
    }
    return nToken;
}

using namespace connectivity;
static bool IN_SQLyyerror;

//------------------------------------------------------------------------------
OSQLScanner::OSQLScanner(Utf8CP rNewStatement, const IParseContext* pContext, sal_Bool bInternational)
    : m_nCurrentPos(0), m_bInternational(bInternational), m_pContext(pContext), yyscanner(nullptr), m_nRule(GetSQLRule()), m_sStatement(rNewStatement)
    {
    IN_SQLyyerror = false;
    yylex_init(&yyscanner);
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    YY_FLUSH_BUFFER;
    BEGIN(m_nRule);
    yyset_extra(this, yyscanner);
    }

//------------------------------------------------------------------------------
OSQLScanner::~OSQLScanner()
    {
    yylex_destroy(yyscanner);
    }

 //------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLyygetc(void)
    {
    sal_Int32 nPos = (static_cast<size_t>(m_nCurrentPos) >= m_sStatement.size()) ? -1 : m_sStatement[m_nCurrentPos];
    m_nCurrentPos++;
    return nPos;
    }

//------------------------------------------------------------------------------
void OSQLScanner::SQLyyerror(const char *fmt)
    {
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if(IN_SQLyyerror)
        return;

    IN_SQLyyerror = true;
    OSL_ENSURE(m_pContext, "OSQLScanner::SQLyyerror: No Context set");
    m_sErrorMessage = fmt;
    if (m_nCurrentPos < (sal_Int32)m_sStatement.size())
    {
        m_sErrorMessage.append(": ");
        Utf8String aError;
        static sal_Int32 BUFFERSIZE = 256;
        static sal_Char* Buffer = 0;
        if(!Buffer)
            Buffer = new sal_Char[BUFFERSIZE];

        sal_Char *s = Buffer;
        sal_Int32 nPos = 1;
        sal_Int32 ch = yytext ? (yytext[0] == 0 ? ' ' : yytext[0]): ' ';
        *s++ = ch;
        while (!checkeof(ch = yyinput(yyscanner)))
        {
            if (ch == ' ')
            {
                if ((ch = yyinput(yyscanner)) != ' ')
                {
                    if (!checkeof(ch))
                        unput(ch);
                }
                *s = '\0';
                aError.assign(Buffer);
                break;
            }
            else
            {
                *s++ = ch;
                if (++nPos == BUFFERSIZE)
                {
                    Utf8String aBuf(Buffer);
                    delete[] Buffer;
                    BUFFERSIZE *=2;
                    Buffer = new sal_Char[BUFFERSIZE];
            for(sal_Int32 i=0;i< (sal_Int32)aBuf.size();++i,++Buffer)
                        *Buffer = aBuf[i];
                    s = &Buffer[nPos];
                }
            }
        }

        m_sErrorMessage += aError;
        delete[] Buffer;
        Buffer = NULL;
    }
    IN_SQLyyerror = false;
    YY_FLUSH_BUFFER;
    }

//------------------------------------------------------------------------------
IParseContext::InternationalKeyCode OSQLScanner::getInternationalTokenID(const sal_Char* sToken) const
    {
    OSL_ENSURE(m_pContext, "OSQLScanner::getInternationalTokenID: No Context set");
    return (m_bInternational) ? m_pContext->getIntlKeyCode(Utf8String(sToken) ) : IParseContext::KEY_NONE;
    }

// -------------------------------------------------------------------------
sal_Int32    OSQLScanner::GetCurrentRule() const    { return m_nRule; }
sal_Int32    OSQLScanner::GetGERRule() const        { return PREDICATE_GER; }
sal_Int32    OSQLScanner::GetENGRule() const        { return PREDICATE_ENG; }
sal_Int32    OSQLScanner::GetSQLRule() const        { return SQL; }
sal_Int32    OSQLScanner::GetDATERule() const       { return DATE; }
sal_Int32    OSQLScanner::GetSTRINGRule() const     { return STRING; }

// -------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLlex(YYSTYPE* val)
    {
    return SQLyylex(val, yyscanner);
    }

// -------------------------------------------------------------------------
OSQLParseNode* OSQLScanner::NewNode(const sal_Char* pNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID)
    {
    return m_pGarbageCollector.NewNode(pNewValue, eNodeType, nNodeID);
    }

// -------------------------------------------------------------------------
OSQLParseNode* OSQLScanner::NewNode(Utf8String const& _rNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID)
    {
    return m_pGarbageCollector.NewNode(_rNewValue.c_str(), eNodeType, nNodeID);
    }

// -------------------------------------------------------------------------

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
//#pragma warning(pop)
#endif
