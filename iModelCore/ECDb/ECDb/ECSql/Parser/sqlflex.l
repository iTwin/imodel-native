
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

#include "InternalNode.h"

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

static sal_Int32	gatherString(sal_Char delim, sal_Int32 nTyp, bool checkForArrayIndex = false);
static sal_Int32	gatherName(const sal_Char*);
static sal_Int32	gatherNamePre(const sal_Char* );
static sal_Int32    parseString();
// has to be set before the parser starts
OSQLScanner* xxx_pGLOBAL_SQLSCAN = NULL;

#define SQL_NEW_NODE(text, token)	\
		SQLyylval.pParseNode = new OSQLInternalNode(text, token);	    

#define SQL_NEW_KEYWORD(token)						\
		SQLyylval.pParseNode = new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, (token));		return token;

#define SQL_NEW_INTNUM 		SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_INTNUM); return SQL_TOKEN_INTNUM;
#define SQL_NEW_APPROXNUM 	SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_APPROXNUM); return SQL_TOKEN_APPROXNUM;
#define SQL_NEW_DATE 		SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_ACCESS_DATE); return SQL_TOKEN_ACCESS_DATE;

#define YY_INPUT(buf,result,max_size)               \
{                                                   \
    int c = xxx_pGLOBAL_SQLSCAN->SQLyygetc();       \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);\
}

#define YY_FATAL_ERROR(msg)							\
{													\
	xxx_pGLOBAL_SQLSCAN->SQLyyerror(msg);			\
}													
	
//
//=============================================================================

%}

%s SQL
%s PREDICATE_ENG
%s PREDICATE_GER
%s DATE
%s STRING

%option noyywrap
%option never-interactive
%%

AFTER               {SQL_NEW_KEYWORD(SQL_TOKEN_AFTER);  }
ALL                 {SQL_NEW_KEYWORD(SQL_TOKEN_ALL);  }
ALTER               {SQL_NEW_KEYWORD(SQL_TOKEN_ALTER);  }
AND                 {SQL_NEW_KEYWORD(SQL_TOKEN_AND);  }
ANY                 {SQL_NEW_KEYWORD(SQL_TOKEN_ANY);  }
ARRAY_AGG           {SQL_NEW_KEYWORD(SQL_TOKEN_ARRAY_AGG);  }
AS                  {SQL_NEW_KEYWORD(SQL_TOKEN_AS);  }
ASC                 {SQL_NEW_KEYWORD(SQL_TOKEN_ASC);  }
AT                  {SQL_NEW_KEYWORD(SQL_TOKEN_AT);  }
ATOMIC              {SQL_NEW_KEYWORD(SQL_TOKEN_ATOMIC);  }
AUTHORIZATION       {SQL_NEW_KEYWORD(SQL_TOKEN_AUTHORIZATION);  }
AVG                 {SQL_NEW_KEYWORD(SQL_TOKEN_AVG);  }

BEFORE              {SQL_NEW_KEYWORD(SQL_TOKEN_BEFORE);  }
BEGIN               {SQL_NEW_KEYWORD(SQL_TOKEN_BEGIN);  }
BETWEEN             {SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN);  }

BOOLEAN             {SQL_NEW_KEYWORD(SQL_TOKEN_BOOLEAN);  }
BINARY              {SQL_NEW_KEYWORD(SQL_TOKEN_BINARY);  }
BIT                 {SQL_NEW_KEYWORD(SQL_TOKEN_BIT);  }
BIT_LENGTH          {SQL_NEW_KEYWORD(SQL_TOKEN_BIT_LENGTH);  }
BLOB                {SQL_NEW_KEYWORD(SQL_TOKEN_BLOB);  }
BY                  {SQL_NEW_KEYWORD(SQL_TOKEN_BY);  }

CASE                {SQL_NEW_KEYWORD(SQL_TOKEN_CASE);  }
CAST                {SQL_NEW_KEYWORD(SQL_TOKEN_CAST);  }
CHAR                {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR);  }
CHARACTER           {SQL_NEW_KEYWORD(SQL_TOKEN_CHARACTER);  }
CHAR(ACTER)?_LENGTH {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR_LENGTH);  }
CHECK               {SQL_NEW_KEYWORD(SQL_TOKEN_CHECK);  }
CLOB                {SQL_NEW_KEYWORD(SQL_TOKEN_CLOB);  }
COALESCE            {SQL_NEW_KEYWORD(SQL_TOKEN_COALESCE);  }
COLLATE             {SQL_NEW_KEYWORD(SQL_TOKEN_COLLATE);  }
COMMIT              {SQL_NEW_KEYWORD(SQL_TOKEN_COMMIT);  }
CONTINUE            {SQL_NEW_KEYWORD(SQL_TOKEN_CONTINUE);  }
CONVERT             {SQL_NEW_KEYWORD(SQL_TOKEN_CONVERT);  }
COUNT               {SQL_NEW_KEYWORD(SQL_TOKEN_COUNT);  }
CREATE              {SQL_NEW_KEYWORD(SQL_TOKEN_CREATE);  }
CROSS               {SQL_NEW_KEYWORD(SQL_TOKEN_CROSS);  }
CURRENT             {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT);  }
CURRENT_DATE        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE);  }
CURRENT_CATALOG                 {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_CATALOG);  }
CURRENT_DEFAULT_TRANSFORM_GROUP {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP);  }
CURRENT_PATH                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_PATH);  }
CURRENT_ROLE                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_ROLE);  }
CURRENT_SCHEMA                  {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_SCHEMA);  }
CURDATE             {SQL_NEW_KEYWORD(SQL_TOKEN_CURDATE);  }
CURRENT_TIMESTAMP   {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP);  }
CURSOR              {SQL_NEW_KEYWORD(SQL_TOKEN_CURSOR);  }
DATE                {SQL_NEW_KEYWORD(SQL_TOKEN_DATE);  }
DATEDIFF            {SQL_NEW_KEYWORD(SQL_TOKEN_DATEDIFF);  }
DATETIME            {SQL_NEW_KEYWORD(SQL_TOKEN_DATETIME);  }
DATEVALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_DATEVALUE);  }
DAY                 {SQL_NEW_KEYWORD(SQL_TOKEN_DAY);  }
DAYNAME             {SQL_NEW_KEYWORD(SQL_TOKEN_DAYNAME);  }
DAYOFMONTH          {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFMONTH);  }
DAYOFWEEK           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFWEEK);  }
DAYOFYEAR           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFYEAR);  }
DEC                 {SQL_NEW_KEYWORD(SQL_TOKEN_DEC);  }
DECIMAL             {SQL_NEW_KEYWORD(SQL_TOKEN_DECIMAL);  }
DECLARE             {SQL_NEW_KEYWORD(SQL_TOKEN_DECLARE);  }
DEFAULT             {SQL_NEW_KEYWORD(SQL_TOKEN_DEFAULT);  }
DELETE              {SQL_NEW_KEYWORD(SQL_TOKEN_DELETE);  }
DESC                {SQL_NEW_KEYWORD(SQL_TOKEN_DESC);  }
DISTINCT            {SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT);  }
DOUBLE              {SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE);  }
DROP                {SQL_NEW_KEYWORD(SQL_TOKEN_DROP);  }

EACH                {SQL_NEW_KEYWORD(SQL_TOKEN_EACH);  }
ECSQLOPTIONS        {SQL_NEW_KEYWORD(SQL_TOKEN_ECSQLOPTIONS);  }
ELSE                {SQL_NEW_KEYWORD(SQL_TOKEN_ELSE);  }
END                 {SQL_NEW_KEYWORD(SQL_TOKEN_END);  }
EVERY               {SQL_NEW_KEYWORD(SQL_TOKEN_EVERY);  }
ESCAPE              {SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE);  }
EXCEPT              {SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT);  }
EXCLUDE             {SQL_NEW_KEYWORD(SQL_TOKEN_EXCLUDE);  }
EXISTS              {SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS);  }
EXTRACT             {SQL_NEW_KEYWORD(SQL_TOKEN_EXTRACT);  }

FALSE               {SQL_NEW_KEYWORD(SQL_TOKEN_FALSE);  }
FETCH               {SQL_NEW_KEYWORD(SQL_TOKEN_FETCH);  }
FIRST               {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST);  }
FIRST_VALUE         {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST_VALUE);  }
FLOAT               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT);  }
FOLLOWING           {SQL_NEW_KEYWORD(SQL_TOKEN_FOLLOWING);  }
FOR                 {SQL_NEW_KEYWORD(SQL_TOKEN_FOR);  }
FOREIGN             {SQL_NEW_KEYWORD(SQL_TOKEN_FOREIGN);  }
FORWARD             {SQL_NEW_KEYWORD(SQL_TOKEN_FORWARD);  }
REVERSE             {SQL_NEW_KEYWORD(SQL_TOKEN_REVERSE);  }

FOUND               {SQL_NEW_KEYWORD(SQL_TOKEN_FOUND);  }
FROM                {SQL_NEW_KEYWORD(SQL_TOKEN_FROM);  }
FULL                {SQL_NEW_KEYWORD(SQL_TOKEN_FULL);  }

GETECCLASSID        {SQL_NEW_KEYWORD(SQL_TOKEN_GETECCLASSID);  }
GROUP               {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP);  }

HAVING              {SQL_NEW_KEYWORD(SQL_TOKEN_HAVING);  }
HOUR                {SQL_NEW_KEYWORD(SQL_TOKEN_HOUR);  }

IGNORE              {SQL_NEW_KEYWORD(SQL_TOKEN_IGNORE);  }
IN                  {SQL_NEW_KEYWORD(SQL_TOKEN_IN);  }
INNER               {SQL_NEW_KEYWORD(SQL_TOKEN_INNER);  }
INSERT              {SQL_NEW_KEYWORD(SQL_TOKEN_INSERT);  }
INSTEAD             {SQL_NEW_KEYWORD(SQL_TOKEN_INSTEAD);  }
INT(EGER)?          {SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER);  }
INT32               {SQL_NEW_KEYWORD(SQL_TOKEN_INT32);  }
INT64               {SQL_NEW_KEYWORD(SQL_TOKEN_INT64);  }
INTERSECT           {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT);  }
INTERVAL            {SQL_NEW_KEYWORD(SQL_TOKEN_INTERVAL);  }
INTO                {SQL_NEW_KEYWORD(SQL_TOKEN_INTO);  }
IS                  {SQL_NEW_KEYWORD(SQL_TOKEN_IS);  }

JOIN                {SQL_NEW_KEYWORD(SQL_TOKEN_JOIN);  }

KEY                 {SQL_NEW_KEYWORD(SQL_TOKEN_KEY);  }

LAG                 {SQL_NEW_KEYWORD(SQL_TOKEN_LAG);  }
LARGE               {SQL_NEW_KEYWORD(SQL_TOKEN_LARGE);  }
LAST                {SQL_NEW_KEYWORD(SQL_TOKEN_LAST);  }
LAST_VALUE          {SQL_NEW_KEYWORD(SQL_TOKEN_LAST_VALUE);  }
LEAD                {SQL_NEW_KEYWORD(SQL_TOKEN_LEAD);  }
LEFT                {SQL_NEW_KEYWORD(SQL_TOKEN_LEFT);  }
LIKE                {SQL_NEW_KEYWORD(SQL_TOKEN_LIKE);  }
LIMIT               {SQL_NEW_KEYWORD(SQL_TOKEN_LIMIT);  }
LOCAL               {SQL_NEW_KEYWORD(SQL_TOKEN_LOCAL);  }
LONG                {SQL_NEW_KEYWORD(SQL_TOKEN_LONG);  }
LOWER               {SQL_NEW_KEYWORD(SQL_TOKEN_LOWER);  }

MAX                 {SQL_NEW_KEYWORD(SQL_TOKEN_MAX);  }
MATCH               {SQL_NEW_KEYWORD(SQL_TOKEN_MATCH);  }
MIN                 {SQL_NEW_KEYWORD(SQL_TOKEN_MIN);  }
MINUTE              {SQL_NEW_KEYWORD(SQL_TOKEN_MINUTE);  }
MONTH               {SQL_NEW_KEYWORD(SQL_TOKEN_MONTH);  }
MONTHNAME           {SQL_NEW_KEYWORD(SQL_TOKEN_MONTHNAME);  }

NATIONAL            {SQL_NEW_KEYWORD(SQL_TOKEN_NATIONAL);  }
NATURAL             {SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL);  }
NCHAR               {SQL_NEW_KEYWORD(SQL_TOKEN_NCHAR);  }
NCLOB               {SQL_NEW_KEYWORD(SQL_TOKEN_NCLOB);  }
NEW                 {SQL_NEW_KEYWORD(SQL_TOKEN_NEW);  }
NEXT                {SQL_NEW_KEYWORD(SQL_TOKEN_NEXT);  }
NO                  {SQL_NEW_KEYWORD(SQL_TOKEN_NO);  }
NOT                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOT);  }
NOW                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOW);  }
NTH_VALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_NTH_VALUE);  }
NTILE               {SQL_NEW_KEYWORD(SQL_TOKEN_NTILE);  }
NULL                {SQL_NEW_KEYWORD(SQL_TOKEN_NULL);  }
NULLIF              {SQL_NEW_KEYWORD(SQL_TOKEN_NULLIF);  }
NULLS               {SQL_NEW_KEYWORD(SQL_TOKEN_NULLS);  }
NUMERIC             {SQL_NEW_KEYWORD(SQL_TOKEN_NUMERIC);  }

OBJECT              {SQL_NEW_KEYWORD(SQL_TOKEN_OBJECT);  }
OCTET_LENGTH        {SQL_NEW_KEYWORD(SQL_TOKEN_OCTET_LENGTH);  }
OF                  {SQL_NEW_KEYWORD(SQL_TOKEN_OF);  }
OFFSET              {SQL_NEW_KEYWORD(SQL_TOKEN_OFFSET);  }
OLD                 {SQL_NEW_KEYWORD(SQL_TOKEN_OLD);  }
ON                  {SQL_NEW_KEYWORD(SQL_TOKEN_ON);  }
ONLY                {SQL_NEW_KEYWORD(SQL_TOKEN_ONLY);  }
OPTION              {SQL_NEW_KEYWORD(SQL_TOKEN_OPTION);  }
OR                  {SQL_NEW_KEYWORD(SQL_TOKEN_OR);  }
ORDER               {SQL_NEW_KEYWORD(SQL_TOKEN_ORDER);  }
OTHERS              {SQL_NEW_KEYWORD(SQL_TOKEN_OTHERS);  }
OUTER               {SQL_NEW_KEYWORD(SQL_TOKEN_OUTER);  }
OVER                {SQL_NEW_KEYWORD(SQL_TOKEN_OVER);  }

PARTITION           {SQL_NEW_KEYWORD(SQL_TOKEN_PARTITION);  }
POINT2D             {SQL_NEW_KEYWORD(SQL_TOKEN_POINT2D);  }
POINT3D             {SQL_NEW_KEYWORD(SQL_TOKEN_POINT3D);  }
POSITION            {SQL_NEW_KEYWORD(SQL_TOKEN_POSITION);  }
PRECEDING           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECEDING);  }
PRECISION           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECISION);  }
PRIMARY             {SQL_NEW_KEYWORD(SQL_TOKEN_PRIMARY);  }
PROCEDURE           {SQL_NEW_KEYWORD(SQL_TOKEN_PROCEDURE);  }
PUBLIC              {SQL_NEW_KEYWORD(SQL_TOKEN_PUBLIC);  }

QUARTER             {SQL_NEW_KEYWORD(SQL_TOKEN_QUARTER);  }

RANGE               {SQL_NEW_KEYWORD(SQL_TOKEN_RANGE);  }
REAL                {SQL_NEW_KEYWORD(SQL_TOKEN_REAL);  }
REFERENCES          {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCES);  }
REFERENCING         {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCING);  }
RESPECT             {SQL_NEW_KEYWORD(SQL_TOKEN_RESPECT);  }
ROLLBACK            {SQL_NEW_KEYWORD(SQL_TOKEN_ROLLBACK);  }
ROW                 {SQL_NEW_KEYWORD(SQL_TOKEN_ROW);  }
ROWS                {SQL_NEW_KEYWORD(SQL_TOKEN_ROWS);  }
ROW_NUMBER          {SQL_NEW_KEYWORD(SQL_TOKEN_ROW_NUMBER);  }
RIGHT               {SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT);  }

SCHEMA              {SQL_NEW_KEYWORD(SQL_TOKEN_SCHEMA);  }
SECOND              {SQL_NEW_KEYWORD(SQL_TOKEN_SECOND);  }
SELECT              {SQL_NEW_KEYWORD(SQL_TOKEN_SELECT);  }
SET                 {SQL_NEW_KEYWORD(SQL_TOKEN_SET);  }
SMALLINT            {SQL_NEW_KEYWORD(SQL_TOKEN_SMALLINT);  }
SOME                {SQL_NEW_KEYWORD(SQL_TOKEN_SOME);  }
STATEMENT           {SQL_NEW_KEYWORD(SQL_TOKEN_STATEMENT);  }
SUBSTRING           {SQL_NEW_KEYWORD(SQL_TOKEN_SUBSTRING);  }
SUM                 {SQL_NEW_KEYWORD(SQL_TOKEN_SUM);  }
STRING              {SQL_NEW_KEYWORD(SQL_TOKEN_STRING);  }

TABLE               {SQL_NEW_KEYWORD(SQL_TOKEN_TABLE);  }
THEN                {SQL_NEW_KEYWORD(SQL_TOKEN_THEN);  }
TIES                {SQL_NEW_KEYWORD(SQL_TOKEN_TIES);  }
TIMESTAMP           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP);  }
TIMESTAMPADD        {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPADD);  }
TIMESTAMPDIFF       {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPDIFF);  }
TIMEVALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEVALUE);  }
TO                  {SQL_NEW_KEYWORD(SQL_TOKEN_TO);  }
TRANSLATE           {SQL_NEW_KEYWORD(SQL_TOKEN_TRANSLATE);  }
TRIGGER             {SQL_NEW_KEYWORD(SQL_TOKEN_TRIGGER);  }
TRUE                {SQL_NEW_KEYWORD(SQL_TOKEN_TRUE);  }

UNBOUNDED           {SQL_NEW_KEYWORD(SQL_TOKEN_UNBOUNDED);  }
UNION               {SQL_NEW_KEYWORD(SQL_TOKEN_UNION);  }
UNIQUE              {SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE);  }
UNKNOWN             {SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN);  }
UPDATE              {SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE);  }
UPPER               {SQL_NEW_KEYWORD(SQL_TOKEN_UPPER);  }
USING               {SQL_NEW_KEYWORD(SQL_TOKEN_USING);  }

VARBINARY           {SQL_NEW_KEYWORD(SQL_TOKEN_VARBINARY);  }
VARCHAR             {SQL_NEW_KEYWORD(SQL_TOKEN_VARCHAR);  }
VARYING             {SQL_NEW_KEYWORD(SQL_TOKEN_VARYING);  }
VALUE               {SQL_NEW_KEYWORD(SQL_TOKEN_VALUE);  }
VALUES              {SQL_NEW_KEYWORD(SQL_TOKEN_VALUES);  }
VIEW                {SQL_NEW_KEYWORD(SQL_TOKEN_VIEW);  }

WEEK                {SQL_NEW_KEYWORD(SQL_TOKEN_WEEK);  }
WHEN                {SQL_NEW_KEYWORD(SQL_TOKEN_WHEN);  }
WHERE               {SQL_NEW_KEYWORD(SQL_TOKEN_WHERE);  }
WITH                {SQL_NEW_KEYWORD(SQL_TOKEN_WITH);  }
WITHIN              {SQL_NEW_KEYWORD(SQL_TOKEN_WITHIN);  }
WORK                {SQL_NEW_KEYWORD(SQL_TOKEN_WORK);  }

YEAR				{SQL_NEW_KEYWORD(SQL_TOKEN_YEAR);  }


"<"                  { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_LESS);return SQL_LESS;}
">"                  { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_GREAT);return SQL_GREAT;}
"="                  { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_EQUAL);return SQL_EQUAL;}
"<="                 { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_LESSEQ);return SQL_LESSEQ;}
">="                 { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_GREATEQ);return SQL_GREATEQ;}
"<>"                 { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"!="                 { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"||"                 { SQL_NEW_NODE(Utf8String(SQLyytext), SQL_NODE_CONCAT);return SQL_CONCAT;}


[-+*/%:(),.;?{}&|~]    { return SQLyytext[0]; }


<SQL>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375_][A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375_0-9]*      {return gatherName( SQLyytext);}

<SQL>([0-9]+)                   {SQL_NEW_INTNUM; }

<SQL>("."[0-9]*) |
<SQL>([0-9]+"."[0-9]*) |
<SQL>[0-9]+[eE][+-]?[0-9]+   |
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM;  }

<PREDICATE_GER,PREDICATE_ENG,DATE>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375][A-Za-z0-9_%.,*?\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375]* {return gatherNamePre(SQLyytext);}

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

<PREDICATE_GER,PREDICATE_ENG>[0-9.,][A-Za-z0-9_.,%]* {return gatherNamePre(SQLyytext);}

<SQL>\"	{ return gatherString('\"',0); }
<SQL>`	{ return gatherString('`' ,0); }											 
\'		{ return parseString(); }

<PREDICATE_GER,PREDICATE_ENG,DATE,SQL>"["		{ return gatherString(']' , 0, true);}
<PREDICATE_GER,PREDICATE_ENG,DATE>#	{ return gatherString('#' ,2); }	
<DATE>[0-9]{1,4}[^ ]*[0-9] |
<DATE>[0-9]{1,4}[^ ]*[0-9][ ][0-9]{1,4}[^ ]*[0-9]		{ SQL_NEW_DATE; }
<STRING>["-""+""*""/"":""("")"",""."";""?""{""}"]		{ return SQLyytext[0]; } /*  */
<STRING>"["					{ return gatherString(']' ,0, true); }
<STRING>[^ ':["?"]*			{ return gatherNamePre(SQLyytext); }

\n              {}

[ \t\r]+        {}

"--".*$         {}

.				{
				Utf8String tmp; 
				tmp.Sprintf("Invalid symbol (char=%c, hex=%0x, dec=%d)", SQLyytext[0], SQLyytext[0], SQLyytext[0]); 
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
    #define yytext_ptr SQLyytext 
  #endif 
 
#endif 

// Versions of flex apparently differ in whether input() resp. yyinput() returns
// zero or EOF upon end of file:
inline bool checkeof(int c) { return c == 0 || c == EOF || c == 0xff; }

sal_Int32 parseString ()
    {
    sal_Char ch;
    sal_Char delim = '\''; //'' ''
    Utf8StringBuffer sBuffer (256);

    int s = 1;
    while (!checkeof (ch = yyinput ()))
        {
        if (ch == delim)
            {
            s++;
            ch = yyinput ();
            if (checkeof (ch))
                {
                SQL_NEW_NODE (sBuffer.makeStringAndClear (), SQL_NODE_STRING);
                return SQL_TOKEN_STRING;
                }
            else if (ch == ' ')
                {
                ch = yyinput ();
                if (ch == delim)
                    {
                    sBuffer.append (delim);
                    }
                else
                    {
                    if (!checkeof (ch))
                        unput (ch);

                    SQL_NEW_NODE (sBuffer.makeStringAndClear (), SQL_NODE_STRING);
                    return SQL_TOKEN_STRING;
                    }
                }
            else if (ch == delim)
                {
                sBuffer.append (delim);
                continue;
                }
            else
                {
                if (!checkeof (ch))
                    unput (ch);

                SQL_NEW_NODE (sBuffer.makeStringAndClear (), SQL_NODE_STRING);
                return SQL_TOKEN_STRING;
                }
            }
        else
            {
            sBuffer.append (ch);
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
sal_Int32 gatherString( sal_Char delim, sal_Int32 nTyp, bool checkForArrayIndex)
{
	sal_Char ch;
	Utf8StringBuffer sBuffer(256);

	while (!checkeof(ch = yyinput())) 
	{
		if (ch == delim) 
		{
			if ((ch = yyinput()) != delim) 
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
								if (sBuffer.charAt(i) == '-' || sBuffer.charAt(i) == '+')
									continue;
								isNumeric = (isNumeric & (isdigit (sBuffer.charAt(i)) != 0));
								}
							}

						bool checkForSign = true;
						if(isNumeric)
							{
							for (size_t i = 0; i < sBuffer.size(); i++)
								{
								if (sBuffer.charAt(i) == '-' || sBuffer.charAt(i) == '+')
									{
									if (checkForSign)
										{
										if (sBuffer.charAt(i) == '-')
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

						SQL_NEW_NODE(sBuffer.makeStringAndClear(), isNumeric ? SQL_NODE_ARRAY_INDEX : SQL_NODE_NAME);
						return isNumeric ? SQL_TOKEN_ARRAY_INDEX : SQL_TOKEN_NAME;
						}
					case 1:
						SQL_NEW_NODE(sBuffer.makeStringAndClear(), SQL_NODE_STRING);
						return SQL_TOKEN_STRING;
					case 2:
						SQL_NEW_NODE(sBuffer.makeStringAndClear(), SQL_NODE_ACCESS_DATE);
						return SQL_TOKEN_ACCESS_DATE;
				}				
			} 
			else
			{
			    sBuffer.append(ch);
			}

		} 
		else if (nTyp != 1 && (ch == '\r' || ch == '\n') )
			break;					
		else
		{
		    sBuffer.append(ch);
		}
	}
	YY_FATAL_ERROR("Unterminated name string"); 
	return SQL_TOKEN_INVALIDSYMBOL;
}

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
sal_Int32 gatherName(const sal_Char* text)
{
	sal_Int32 nToken;
	OSL_ENSURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
	IParseContext::InternationalKeyCode eKeyCode = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
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
sal_Int32 gatherNamePre(const sal_Char* text)
{ 
	sal_Int32 nToken;
	OSL_ENSURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
	IParseContext::InternationalKeyCode eKeyCode = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
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
			Utf8String sStmt = xxx_pGLOBAL_SQLSCAN->getStatement();
			size_t nLength = strlen(text);
			size_t nPos = xxx_pGLOBAL_SQLSCAN->GetCurrentPos() - nLength - 2;
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
OSQLScanner::OSQLScanner()
			: m_nCurrentPos(0)
		    , m_bInternational(sal_False)
			, m_pContext(NULL)
			, m_nRule(0) // 0 is INITIAL
{
	IN_SQLyyerror = false;
}

//------------------------------------------------------------------------------
OSQLScanner::~OSQLScanner()
{
}
//------------------------------------------------------------------------------
void OSQLScanner::SQLyyerror(const char *fmt)
{

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
		sal_Int32 ch = SQLyytext ? (SQLyytext[0] == 0 ? ' ' : SQLyytext[0]): ' ';
		*s++ = ch;
		while (!checkeof(ch = yyinput())) 
		{
			if (ch == ' ') 
			{
				if ((ch = yyinput()) != ' ') 
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
void OSQLScanner::prepareScan(const Utf8String & rNewStatement, const IParseContext* pContext, sal_Bool bInternational)
{
	YY_FLUSH_BUFFER;
	BEGIN(m_nRule);

	m_sErrorMessage = Utf8String();	
	m_sStatement	= rNewStatement;
	m_nCurrentPos	= 0;
	m_bInternational = bInternational;
	m_pContext		= pContext;
}

//------------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLyygetc(void)
{
	sal_Int32 nPos = (static_cast<size_t>(m_nCurrentPos) >= m_sStatement.size()) ? -1 : m_sStatement[m_nCurrentPos];
    m_nCurrentPos++;
    return nPos;
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
void OSQLScanner::setScanner(sal_Bool _bNull)
{
	xxx_pGLOBAL_SQLSCAN = _bNull ? NULL : this;
}
// -------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLlex()
{
	return SQLyylex();
}

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
//#pragma warning(pop)
#endif
