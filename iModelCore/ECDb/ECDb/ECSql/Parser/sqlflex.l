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
#define YY_EXIT 1				// YY_FATAL will not halt the application

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

static sal_Int32	gatherString(yyscan_t yyscanner, sal_Char delim, sal_Int32 nTyp, bool checkForArrayIndex = false);
static sal_Int32	gatherName(yyscan_t yyscanner, const sal_Char*);
static sal_Int32	gatherNamePre(yyscan_t yyscanner, const sal_Char* );
static sal_Int32    parseString(yyscan_t yyscanner);

#define SQL_NEW_NODE(text, token) \
		SQLyyget_lval(yyscanner)->pParseNode=yyget_extra(yyscanner)->NewNode(text, token);	    

#define SQL_NEW_KEYWORD(token) \
		SQLyyget_lval(yyscanner)->pParseNode=yyget_extra(yyscanner)->NewNode(aEmptyString, SQL_NODE_KEYWORD, (token)); return token;

#define SQL_NEW_INTNUM 		SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_INTNUM); return SQL_TOKEN_INTNUM;
#define SQL_NEW_APPROXNUM 	SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_APPROXNUM); return SQL_TOKEN_APPROXNUM;
#define SQL_NEW_DATE 		SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_ACCESS_DATE); return SQL_TOKEN_ACCESS_DATE;
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

ALL                 {SQL_NEW_KEYWORD(SQL_TOKEN_ALL);  }
AND                 {SQL_NEW_KEYWORD(SQL_TOKEN_AND);  }
ANY                 {SQL_NEW_KEYWORD(SQL_TOKEN_ANY);  }
AS                  {SQL_NEW_KEYWORD(SQL_TOKEN_AS);  }
ASC                 {SQL_NEW_KEYWORD(SQL_TOKEN_ASC);  }
AVG                 {SQL_NEW_KEYWORD(SQL_TOKEN_AVG);  }

BETWEEN             {SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN);  }

BOOLEAN             {SQL_NEW_KEYWORD(SQL_TOKEN_BOOLEAN);  }
BINARY              {SQL_NEW_KEYWORD(SQL_TOKEN_BINARY);  }
BLOB                {SQL_NEW_KEYWORD(SQL_TOKEN_BLOB);  }
BY                  {SQL_NEW_KEYWORD(SQL_TOKEN_BY);  }

CASE                {SQL_NEW_KEYWORD(SQL_TOKEN_CASE);  }
CAST                {SQL_NEW_KEYWORD(SQL_TOKEN_CAST);  }
COMMIT              {SQL_NEW_KEYWORD(SQL_TOKEN_COMMIT);  }
COUNT               {SQL_NEW_KEYWORD(SQL_TOKEN_COUNT);  }
CROSS               {SQL_NEW_KEYWORD(SQL_TOKEN_CROSS);  }
CURRENT_DATE        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE);  }
CURRENT_TIME        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIME);  }
CURRENT_TIMESTAMP   {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP);  }

DATE                {SQL_NEW_KEYWORD(SQL_TOKEN_DATE);  }
DEFAULT             {SQL_NEW_KEYWORD(SQL_TOKEN_DEFAULT);  }
DELETE              {SQL_NEW_KEYWORD(SQL_TOKEN_DELETE);  }
DESC                {SQL_NEW_KEYWORD(SQL_TOKEN_DESC);  }
DISTINCT            {SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT);  }
DOUBLE              {SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE);  }

ECSQLOPTIONS        {SQL_NEW_KEYWORD(SQL_TOKEN_ECSQLOPTIONS);  }
ELSE                {SQL_NEW_KEYWORD(SQL_TOKEN_ELSE);  }
END                 {SQL_NEW_KEYWORD(SQL_TOKEN_END);  }
EVERY               {SQL_NEW_KEYWORD(SQL_TOKEN_EVERY);  }
ESCAPE              {SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE);  }
EXCEPT              {SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT);  }
EXISTS              {SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS);  }

FALSE               {SQL_NEW_KEYWORD(SQL_TOKEN_FALSE);  }
FLOAT               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT);  }
FORWARD             {SQL_NEW_KEYWORD(SQL_TOKEN_FORWARD);  }
BACKWARD            {SQL_NEW_KEYWORD(SQL_TOKEN_BACKWARD);  }

FROM                {SQL_NEW_KEYWORD(SQL_TOKEN_FROM);  }
FULL                {SQL_NEW_KEYWORD(SQL_TOKEN_FULL);  }

GROUP               {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP);  }

HAVING              {SQL_NEW_KEYWORD(SQL_TOKEN_HAVING);  }

IN                  {SQL_NEW_KEYWORD(SQL_TOKEN_IN);  }
INNER               {SQL_NEW_KEYWORD(SQL_TOKEN_INNER);  }
INSERT              {SQL_NEW_KEYWORD(SQL_TOKEN_INSERT);  }
INT(EGER)?          {SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER);  }
INT64               {SQL_NEW_KEYWORD(SQL_TOKEN_INT64);  }
INTERSECT           {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT);  }
INTO                {SQL_NEW_KEYWORD(SQL_TOKEN_INTO);  }
IS                  {SQL_NEW_KEYWORD(SQL_TOKEN_IS);  }

JOIN                {SQL_NEW_KEYWORD(SQL_TOKEN_JOIN);  }

LEFT                {SQL_NEW_KEYWORD(SQL_TOKEN_LEFT);  }
LIKE                {SQL_NEW_KEYWORD(SQL_TOKEN_LIKE);  }
LIMIT               {SQL_NEW_KEYWORD(SQL_TOKEN_LIMIT);  }
LONG                {SQL_NEW_KEYWORD(SQL_TOKEN_LONG);  }

MAX                 {SQL_NEW_KEYWORD(SQL_TOKEN_MAX);  }
MATCH               {SQL_NEW_KEYWORD(SQL_TOKEN_MATCH);  }
MIN                 {SQL_NEW_KEYWORD(SQL_TOKEN_MIN);  }

NATURAL             {SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL);  }
NOT                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOT);  }
NULL                {SQL_NEW_KEYWORD(SQL_TOKEN_NULL);  }

OFFSET              {SQL_NEW_KEYWORD(SQL_TOKEN_OFFSET);  }
ON                  {SQL_NEW_KEYWORD(SQL_TOKEN_ON);  }
ONLY                {SQL_NEW_KEYWORD(SQL_TOKEN_ONLY);  }
OR                  {SQL_NEW_KEYWORD(SQL_TOKEN_OR);  }
ORDER               {SQL_NEW_KEYWORD(SQL_TOKEN_ORDER);  }
OUTER               {SQL_NEW_KEYWORD(SQL_TOKEN_OUTER);  }

REAL                {SQL_NEW_KEYWORD(SQL_TOKEN_REAL);  }
ROLLBACK            {SQL_NEW_KEYWORD(SQL_TOKEN_ROLLBACK);  }
RIGHT               {SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT);  }

SELECT              {SQL_NEW_KEYWORD(SQL_TOKEN_SELECT);  }
SET                 {SQL_NEW_KEYWORD(SQL_TOKEN_SET);  }
SOME                {SQL_NEW_KEYWORD(SQL_TOKEN_SOME);  }
SUM                 {SQL_NEW_KEYWORD(SQL_TOKEN_SUM);  }
STRING              {SQL_NEW_KEYWORD(SQL_TOKEN_STRING);  }

THEN                {SQL_NEW_KEYWORD(SQL_TOKEN_THEN);  }
TIME                {SQL_NEW_KEYWORD(SQL_TOKEN_TIME);  }
TIMESTAMP           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP);  }
TRUE                {SQL_NEW_KEYWORD(SQL_TOKEN_TRUE);  }

UNION               {SQL_NEW_KEYWORD(SQL_TOKEN_UNION);  }
UNIQUE              {SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE);  }
UNKNOWN             {SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN);  }
UPDATE              {SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE);  }
USING               {SQL_NEW_KEYWORD(SQL_TOKEN_USING);  }

VARCHAR             {SQL_NEW_KEYWORD(SQL_TOKEN_VARCHAR);  }
VALUE               {SQL_NEW_KEYWORD(SQL_TOKEN_VALUE);  }
VALUES              {SQL_NEW_KEYWORD(SQL_TOKEN_VALUES);  }

WHEN                {SQL_NEW_KEYWORD(SQL_TOKEN_WHEN);  }
WHERE               {SQL_NEW_KEYWORD(SQL_TOKEN_WHERE);  }


"<"                  { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_LESS);return SQL_LESS;}
">"                  { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_GREAT);return SQL_GREAT;}
"="                  { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_EQUAL);return SQL_EQUAL;}
"<="                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_LESSEQ);return SQL_LESSEQ;}
">="                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_GREATEQ);return SQL_GREATEQ;}
"<>"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"!="                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"||"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_CONCAT);return SQL_CONCAT;}

"~"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_NOT);return SQL_BITWISE_NOT;}
"|"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_OR);return SQL_BITWISE_OR;}
"&"                 { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_AND);return SQL_BITWISE_AND;}
"<<"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_SHIFT_LEFT);return SQL_BITWISE_SHIFT_LEFT;}
">>"                { SQL_NEW_NODE(Utf8String(yytext), SQL_NODE_BITWISE_SHIFT_RIGHT);return SQL_BITWISE_SHIFT_RIGHT;}

[-+*/%:(),.;?{}&|~]    { return yytext[0]; }


<SQL>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375_][A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375_0-9]*      {return gatherName(yyscanner, yytext);}

<SQL>([0-9]+)                   {SQL_NEW_INTNUM; }
<SQL>(0[Xx][0-9a-fA-F]+)          {SQL_NEW_INTNUM; }

<SQL>("."[0-9]*) |
<SQL>([0-9]+"."[0-9]*) |
<SQL>[0-9]+[eE][+-]?[0-9]+   |
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM;  }
<PREDICATE_GER,PREDICATE_ENG,DATE>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375][A-Za-z0-9_%.,*?\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375]* {return gatherNamePre(yyscanner,yytext);}
<PREDICATE_GER,PREDICATE_ENG>([0-9]+)		{SQL_NEW_INTNUM; }
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+) 	{SQL_NEW_INTNUM; }
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+)	{SQL_NEW_INTNUM; }
<PREDICATE_ENG>([0-9]+"."[0-9]+) |
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+"."[0-9]+) | 
<PREDICATE_ENG>("."[0-9]+)					{SQL_NEW_APPROXNUM;  }
<PREDICATE_ENG>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_ENG>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_ENG>"."[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM;  }
<PREDICATE_GER>([0-9]+","[0-9]+) |
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+","[0-9]+) | 
<PREDICATE_GER>(","[0-9]+)					{SQL_NEW_APPROXNUM;  }
<PREDICATE_GER>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_GER>[0-9]+","[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_GER>","[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM;  }
<PREDICATE_GER,PREDICATE_ENG>[0-9.,][A-Za-z0-9_.,%]* {return gatherNamePre(yyscanner, yytext);}

<SQL>\"	{ return gatherString(yyscanner, '\"',0); }
<SQL>`	{ return gatherString(yyscanner, '`' ,0); }											 
\'		{ return parseString(yyscanner); }

<PREDICATE_GER,PREDICATE_ENG,DATE,SQL>"["		{ return gatherString(yyscanner,']' , 0, true);}
<PREDICATE_GER,PREDICATE_ENG,DATE>#	{ return gatherString(yyscanner,'#' ,2); }	
<DATE>[0-9]{1,4}[^ ]*[0-9] |
<DATE>[0-9]{1,4}[^ ]*[0-9][ ][0-9]{1,4}[^ ]*[0-9]		{ SQL_NEW_DATE; }
<STRING>["-""+""*""/"":""("")"",""."";""?""{""}"]		{ return yytext[0]; } /*  */
<STRING>"["					{ return gatherString(yyscanner,']' ,0, true); }
<STRING>[^ ':["?"]*			{ return gatherNamePre(yyscanner,yytext); }
\n              {}
[ \t\r]+        {}
"--".*$         {}
.				{
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
    int s = 1;
    while (!checkeof (ch = yyinput (yyscanner)))
        {
        if (ch == delim)
            {
            s++;
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
 *	''	'a string'	'quote '' within string'
 *	""	"a string"	"quote "" within string"
 * nTyp == 0 -> SQL_NODE_NAME | SQL_NODE_ARRAY_INDEX
 * nTyp == 1 -> SQL_NODE_STRING
 * nTyp == 2 -> SQL_NODE_ACCESS_DATE
 */
//------------------------------------------------------------------------------
sal_Int32 gatherString(yyscan_t yyscanner, sal_Char delim, sal_Int32 nTyp, bool checkForArrayIndex)
{
	sal_Char ch;
	Utf8String sBuffer;
    sBuffer.reserve(256);
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
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
						{
						bool isNumeric = checkForArrayIndex;
						if (checkForArrayIndex)
							{
							for (size_t i = 0; i < sBuffer.size(); i++)
								{
								if (sBuffer[i] == '-' || sBuffer[i] == '+')
									continue;
								isNumeric = (isNumeric & (isdigit (sBuffer[i]) != 0));
								}
							}

						bool checkForSign = true;
						if(isNumeric)
							{
							for (size_t i = 0; i < sBuffer.size(); i++)
								{
								if (sBuffer[i] == '-' || sBuffer[i] == '+')
									{
									if (checkForSign)
										{
										if (sBuffer[i] == '-')
											{
											YY_FATAL_ERROR("Invalid array index. Only positive integer is expected."); 
											return SQL_TOKEN_INVALIDSYMBOL;
											}
										checkForSign = false;
										}
									else
										{
										YY_FATAL_ERROR("Invalid array index. Only positive integer is expected."); 
										return SQL_TOKEN_INVALIDSYMBOL;
										}
									}
								}
							}

						SQL_NEW_NODE(sBuffer, isNumeric ? SQL_NODE_ARRAY_INDEX : SQL_NODE_NAME);
						return isNumeric ? SQL_TOKEN_ARRAY_INDEX : SQL_TOKEN_NAME;
						}
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
		else if (nTyp != 1 && (ch == '\r' || ch == '\n') )
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
sal_Int32	OSQLScanner::GetCurrentRule() const { return m_nRule; }
sal_Int32	OSQLScanner::GetGERRule() const		{ return PREDICATE_GER; }
sal_Int32	OSQLScanner::GetENGRule() const		{ return PREDICATE_ENG; }
sal_Int32	OSQLScanner::GetSQLRule() const		{ return SQL; }
sal_Int32	OSQLScanner::GetDATERule() const	{ return DATE; }
sal_Int32	OSQLScanner::GetSTRINGRule() const	{ return STRING; }

// -------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLlex(YYSTYPE* val)
    {
    return SQLyylex(val, yyscanner);
    }

// -------------------------------------------------------------------------
OSQLParseNode* OSQLScanner::NewNode(const sal_Char* pNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID)
    {
    auto newNode = new OSQLParseNode(pNewValue, eNodeType, nNodeID);
    m_pGarbageCollector.push_back(newNode);
    newNode->m_container = &m_pGarbageCollector;
    return newNode;
    }

// -------------------------------------------------------------------------
OSQLParseNode* OSQLScanner::NewNode(Utf8String const& _rNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID)
    {
    auto newNode = new OSQLParseNode(_rNewValue, eNodeType, nNodeID);
    m_pGarbageCollector.push_back(newNode);
    newNode->m_container = &m_pGarbageCollector;
    return newNode;
    }

// -------------------------------------------------------------------------

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
//#pragma warning(pop)
#endif
