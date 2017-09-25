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

#ifndef YY_SQLYY_E_BSW_BIM0200DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_E_BSW_BIM0200DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
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
     SQL_TOKEN_ALTER = 267,
     SQL_TOKEN_AMMSC = 268,
     SQL_TOKEN_ANY = 269,
     SQL_TOKEN_AS = 270,
     SQL_TOKEN_ASC = 271,
     SQL_TOKEN_AUTHORIZATION = 272,
     SQL_TOKEN_AVG = 273,
     SQL_TOKEN_BETWEEN = 274,
     SQL_TOKEN_BIT = 275,
     SQL_TOKEN_BY = 276,
     SQL_TOKEN_CAST = 277,
     SQL_TOKEN_COLLATE = 278,
     SQL_TOKEN_COMMIT = 279,
     SQL_TOKEN_CONVERT = 280,
     SQL_TOKEN_COUNT = 281,
     SQL_TOKEN_CROSS = 282,
     SQL_TOKEN_CURRENT = 283,
     SQL_TOKEN_CURSOR = 284,
     SQL_TOKEN_DAY = 285,
     SQL_TOKEN_DEFAULT = 286,
     SQL_TOKEN_DELETE = 287,
     SQL_TOKEN_DESC = 288,
     SQL_TOKEN_DISTINCT = 289,
     SQL_TOKEN_FORWARD = 290,
     SQL_TOKEN_BACKWARD = 291,
     SQL_TOKEN_ESCAPE = 292,
     SQL_TOKEN_EXCEPT = 293,
     SQL_TOKEN_EXISTS = 294,
     SQL_TOKEN_FALSE = 295,
     SQL_TOKEN_FOR = 296,
     SQL_TOKEN_FOUND = 297,
     SQL_TOKEN_FROM = 298,
     SQL_TOKEN_FULL = 299,
     SQL_TOKEN_GROUP = 300,
     SQL_TOKEN_HAVING = 301,
     SQL_TOKEN_IN = 302,
     SQL_TOKEN_INDICATOR = 303,
     SQL_TOKEN_INNER = 304,
     SQL_TOKEN_INSERT = 305,
     SQL_TOKEN_INTO = 306,
     SQL_TOKEN_IS = 307,
     SQL_TOKEN_INTERSECT = 308,
     SQL_TOKEN_JOIN = 309,
     SQL_TOKEN_LIKE = 310,
     SQL_TOKEN_LEFT = 311,
     SQL_TOKEN_RIGHT = 312,
     SQL_TOKEN_LOWER = 313,
     SQL_TOKEN_MAX = 314,
     SQL_TOKEN_MIN = 315,
     SQL_TOKEN_NATURAL = 316,
     SQL_TOKEN_NULL = 317,
     SQL_TOKEN_OCTET_LENGTH = 318,
     SQL_TOKEN_ON = 319,
     SQL_TOKEN_ORDER = 320,
     SQL_TOKEN_OUTER = 321,
     SQL_TOKEN_ROLLBACK = 322,
     SQL_TOKEN_SELECT = 323,
     SQL_TOKEN_SET = 324,
     SQL_TOKEN_SOME = 325,
     SQL_TOKEN_SQLCODE = 326,
     SQL_TOKEN_SQLERROR = 327,
     SQL_TOKEN_SUM = 328,
     SQL_TOKEN_TRANSLATE = 329,
     SQL_TOKEN_TRUE = 330,
     SQL_TOKEN_UNION = 331,
     SQL_TOKEN_UNIQUE = 332,
     SQL_TOKEN_UNKNOWN = 333,
     SQL_TOKEN_UPDATE = 334,
     SQL_TOKEN_UPPER = 335,
     SQL_TOKEN_USING = 336,
     SQL_TOKEN_VALUES = 337,
     SQL_TOKEN_WHERE = 338,
     SQL_TOKEN_WITH = 339,
     SQL_TOKEN_WORK = 340,
     SQL_TOKEN_BIT_LENGTH = 341,
     SQL_TOKEN_CHAR_LENGTH = 342,
     SQL_TOKEN_POSITION = 343,
     SQL_TOKEN_SUBSTRING = 344,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 345,
     SQL_TOKEN_CURRENT_DATE = 346,
     SQL_TOKEN_CURRENT_TIMESTAMP = 347,
     SQL_TOKEN_CURDATE = 348,
     SQL_TOKEN_NOW = 349,
     SQL_TOKEN_EXTRACT = 350,
     SQL_TOKEN_HOUR = 351,
     SQL_TOKEN_MINUTE = 352,
     SQL_TOKEN_MONTH = 353,
     SQL_TOKEN_SECOND = 354,
     SQL_TOKEN_WEEK = 355,
     SQL_TOKEN_YEAR = 356,
     SQL_TOKEN_EVERY = 357,
     SQL_TOKEN_WITHIN = 358,
     SQL_TOKEN_CASE = 359,
     SQL_TOKEN_THEN = 360,
     SQL_TOKEN_END = 361,
     SQL_TOKEN_WHEN = 362,
     SQL_TOKEN_ELSE = 363,
     SQL_TOKEN_ROW = 364,
     SQL_TOKEN_VALUE = 365,
     SQL_TOKEN_CURRENT_CATALOG = 366,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 367,
     SQL_TOKEN_CURRENT_PATH = 368,
     SQL_TOKEN_CURRENT_ROLE = 369,
     SQL_TOKEN_CURRENT_SCHEMA = 370,
     SQL_TOKEN_OVER = 371,
     SQL_TOKEN_ROW_NUMBER = 372,
     SQL_TOKEN_NTILE = 373,
     SQL_TOKEN_LEAD = 374,
     SQL_TOKEN_LAG = 375,
     SQL_TOKEN_RESPECT = 376,
     SQL_TOKEN_IGNORE = 377,
     SQL_TOKEN_NULLS = 378,
     SQL_TOKEN_FIRST_VALUE = 379,
     SQL_TOKEN_LAST_VALUE = 380,
     SQL_TOKEN_NTH_VALUE = 381,
     SQL_TOKEN_FIRST = 382,
     SQL_TOKEN_LAST = 383,
     SQL_TOKEN_EXCLUDE = 384,
     SQL_TOKEN_OTHERS = 385,
     SQL_TOKEN_TIES = 386,
     SQL_TOKEN_FOLLOWING = 387,
     SQL_TOKEN_UNBOUNDED = 388,
     SQL_TOKEN_PRECEDING = 389,
     SQL_TOKEN_RANGE = 390,
     SQL_TOKEN_ROWS = 391,
     SQL_TOKEN_PARTITION = 392,
     SQL_TOKEN_WINDOW = 393,
     SQL_TOKEN_NO = 394,
     SQL_TOKEN_LIMIT = 395,
     SQL_TOKEN_OFFSET = 396,
     SQL_TOKEN_ONLY = 397,
     SQL_TOKEN_MATCH = 398,
     SQL_TOKEN_ECSQLOPTIONS = 399,
     SQL_TOKEN_INTEGER = 400,
     SQL_TOKEN_INT = 401,
     SQL_TOKEN_INT64 = 402,
     SQL_TOKEN_LONG = 403,
     SQL_TOKEN_BOOLEAN = 404,
     SQL_TOKEN_DOUBLE = 405,
     SQL_TOKEN_REAL = 406,
     SQL_TOKEN_FLOAT = 407,
     SQL_TOKEN_STRING = 408,
     SQL_TOKEN_VARCHAR = 409,
     SQL_TOKEN_BINARY = 410,
     SQL_TOKEN_BLOB = 411,
     SQL_TOKEN_DATE = 412,
     SQL_TOKEN_TIMESTAMP = 413,
     SQL_TOKEN_DATETIME = 414,
     SQL_TOKEN_OR = 415,
     SQL_TOKEN_AND = 416,
     SQL_EQUAL = 417,
     SQL_GREAT = 418,
     SQL_LESS = 419,
     SQL_NOTEQUAL = 420,
     SQL_GREATEQ = 421,
     SQL_LESSEQ = 422,
     SQL_CONCAT = 423,
     SQL_TOKEN_INVALIDSYMBOL = 424
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

#endif /* !YY_SQLYY_E_BSW_BIM0200DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
