/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int SQLyydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SQL_TOKEN_ACCESS_DATE = 258,
     SQL_TOKEN_REAL_NUM = 259,
     SQL_TOKEN_INTNUM = 260,
     SQL_TOKEN_APPROXNUM = 261,
     SQL_TOKEN_NOT = 262,
     SQL_TOKEN_NAME = 263,
     SQL_TOKEN_ARRAY_INDEX = 264,
     SQL_TOKEN_UMINUS = 265,
     SQL_TOKEN_ALL = 266,
     SQL_TOKEN_ANY = 267,
     SQL_TOKEN_AS = 268,
     SQL_TOKEN_ASC = 269,
     SQL_TOKEN_AVG = 270,
     SQL_TOKEN_BETWEEN = 271,
     SQL_TOKEN_BY = 272,
     SQL_TOKEN_CAST = 273,
     SQL_TOKEN_COMMIT = 274,
     SQL_TOKEN_COUNT = 275,
     SQL_TOKEN_CROSS = 276,
     SQL_TOKEN_DEFAULT = 277,
     SQL_TOKEN_DELETE = 278,
     SQL_TOKEN_DESC = 279,
     SQL_TOKEN_DISTINCT = 280,
     SQL_TOKEN_FORWARD = 281,
     SQL_TOKEN_BACKWARD = 282,
     SQL_TOKEN_ESCAPE = 283,
     SQL_TOKEN_EXCEPT = 284,
     SQL_TOKEN_EXISTS = 285,
     SQL_TOKEN_FALSE = 286,
     SQL_TOKEN_FROM = 287,
     SQL_TOKEN_FULL = 288,
     SQL_TOKEN_GROUP = 289,
     SQL_TOKEN_HAVING = 290,
     SQL_TOKEN_IN = 291,
     SQL_TOKEN_INNER = 292,
     SQL_TOKEN_INSERT = 293,
     SQL_TOKEN_INTO = 294,
     SQL_TOKEN_IS = 295,
     SQL_TOKEN_INTERSECT = 296,
     SQL_TOKEN_JOIN = 297,
     SQL_TOKEN_LIKE = 298,
     SQL_TOKEN_LEFT = 299,
     SQL_TOKEN_RIGHT = 300,
     SQL_TOKEN_MAX = 301,
     SQL_TOKEN_MIN = 302,
     SQL_TOKEN_NATURAL = 303,
     SQL_TOKEN_NULL = 304,
     SQL_TOKEN_ON = 305,
     SQL_TOKEN_ORDER = 306,
     SQL_TOKEN_OUTER = 307,
     SQL_TOKEN_ROLLBACK = 308,
     SQL_TOKEN_SELECT = 309,
     SQL_TOKEN_SET = 310,
     SQL_TOKEN_SOME = 311,
     SQL_TOKEN_SUM = 312,
     SQL_TOKEN_TRUE = 313,
     SQL_TOKEN_UNION = 314,
     SQL_TOKEN_UNIQUE = 315,
     SQL_TOKEN_UNKNOWN = 316,
     SQL_TOKEN_UPDATE = 317,
     SQL_TOKEN_USING = 318,
     SQL_TOKEN_VALUE = 319,
     SQL_TOKEN_VALUES = 320,
     SQL_TOKEN_WHERE = 321,
     SQL_BITWISE_NOT = 322,
     SQL_TOKEN_CURRENT_DATE = 323,
     SQL_TOKEN_CURRENT_TIME = 324,
     SQL_TOKEN_CURRENT_TIMESTAMP = 325,
     SQL_TOKEN_EVERY = 326,
     SQL_TOKEN_CASE = 327,
     SQL_TOKEN_THEN = 328,
     SQL_TOKEN_END = 329,
     SQL_TOKEN_WHEN = 330,
     SQL_TOKEN_ELSE = 331,
     SQL_TOKEN_LIMIT = 332,
     SQL_TOKEN_OFFSET = 333,
     SQL_TOKEN_ONLY = 334,
     SQL_TOKEN_MATCH = 335,
     SQL_TOKEN_ECSQLOPTIONS = 336,
     SQL_TOKEN_INTEGER = 337,
     SQL_TOKEN_INT = 338,
     SQL_TOKEN_INT64 = 339,
     SQL_TOKEN_LONG = 340,
     SQL_TOKEN_BOOLEAN = 341,
     SQL_TOKEN_DOUBLE = 342,
     SQL_TOKEN_REAL = 343,
     SQL_TOKEN_FLOAT = 344,
     SQL_TOKEN_STRING = 345,
     SQL_TOKEN_VARCHAR = 346,
     SQL_TOKEN_BINARY = 347,
     SQL_TOKEN_BLOB = 348,
     SQL_TOKEN_DATE = 349,
     SQL_TOKEN_TIME = 350,
     SQL_TOKEN_TIMESTAMP = 351,
     SQL_TOKEN_OR = 352,
     SQL_TOKEN_AND = 353,
     SQL_BITWISE_OR = 354,
     SQL_BITWISE_AND = 355,
     SQL_BITWISE_SHIFT_RIGHT = 356,
     SQL_BITWISE_SHIFT_LEFT = 357,
     SQL_EQUAL = 358,
     SQL_GREAT = 359,
     SQL_LESS = 360,
     SQL_NOTEQUAL = 361,
     SQL_GREATEQ = 362,
     SQL_LESSEQ = 363,
     SQL_CONCAT = 364,
     SQL_TOKEN_INVALIDSYMBOL = 365
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


    connectivity::OSQLParseNode * pParseNode;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int SQLyyparse (void *YYPARSE_PARAM);
#else
int SQLyyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int SQLyyparse (connectivity::OSQLParser* context);
#else
int SQLyyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
