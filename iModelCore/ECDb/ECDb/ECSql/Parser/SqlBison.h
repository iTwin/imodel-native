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

#ifndef YY_SQLYY_D_DEV_DGNDB_BIM20PROPMAPREFACTOR_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_D_DEV_DGNDB_BIM20PROPMAPREFACTOR_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_FLOAT = 296,
     SQL_TOKEN_FOR = 297,
     SQL_TOKEN_FOUND = 298,
     SQL_TOKEN_FROM = 299,
     SQL_TOKEN_FULL = 300,
     SQL_TOKEN_GROUP = 301,
     SQL_TOKEN_HAVING = 302,
     SQL_TOKEN_IN = 303,
     SQL_TOKEN_INDICATOR = 304,
     SQL_TOKEN_INNER = 305,
     SQL_TOKEN_INSERT = 306,
     SQL_TOKEN_INTO = 307,
     SQL_TOKEN_IS = 308,
     SQL_TOKEN_INTERSECT = 309,
     SQL_TOKEN_JOIN = 310,
     SQL_TOKEN_LIKE = 311,
     SQL_TOKEN_LEFT = 312,
     SQL_TOKEN_RIGHT = 313,
     SQL_TOKEN_LOWER = 314,
     SQL_TOKEN_MAX = 315,
     SQL_TOKEN_MIN = 316,
     SQL_TOKEN_NATURAL = 317,
     SQL_TOKEN_NULL = 318,
     SQL_TOKEN_OCTET_LENGTH = 319,
     SQL_TOKEN_ON = 320,
     SQL_TOKEN_ORDER = 321,
     SQL_TOKEN_OUTER = 322,
     SQL_TOKEN_REAL = 323,
     SQL_TOKEN_ROLLBACK = 324,
     SQL_TOKEN_SELECT = 325,
     SQL_TOKEN_SET = 326,
     SQL_TOKEN_SOME = 327,
     SQL_TOKEN_SQLCODE = 328,
     SQL_TOKEN_SQLERROR = 329,
     SQL_TOKEN_SUM = 330,
     SQL_TOKEN_TRANSLATE = 331,
     SQL_TOKEN_TRUE = 332,
     SQL_TOKEN_UNION = 333,
     SQL_TOKEN_UNIQUE = 334,
     SQL_TOKEN_UNKNOWN = 335,
     SQL_TOKEN_UPDATE = 336,
     SQL_TOKEN_UPPER = 337,
     SQL_TOKEN_USING = 338,
     SQL_TOKEN_VALUES = 339,
     SQL_TOKEN_WHERE = 340,
     SQL_TOKEN_WITH = 341,
     SQL_TOKEN_WORK = 342,
     SQL_TOKEN_BIT_LENGTH = 343,
     SQL_TOKEN_CHAR_LENGTH = 344,
     SQL_TOKEN_POSITION = 345,
     SQL_TOKEN_SUBSTRING = 346,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 347,
     SQL_TOKEN_CURRENT_DATE = 348,
     SQL_TOKEN_CURRENT_TIMESTAMP = 349,
     SQL_TOKEN_CURDATE = 350,
     SQL_TOKEN_NOW = 351,
     SQL_TOKEN_EXTRACT = 352,
     SQL_TOKEN_HOUR = 353,
     SQL_TOKEN_MINUTE = 354,
     SQL_TOKEN_MONTH = 355,
     SQL_TOKEN_SECOND = 356,
     SQL_TOKEN_WEEK = 357,
     SQL_TOKEN_YEAR = 358,
     SQL_TOKEN_EVERY = 359,
     SQL_TOKEN_WITHIN = 360,
     SQL_TOKEN_CASE = 361,
     SQL_TOKEN_THEN = 362,
     SQL_TOKEN_END = 363,
     SQL_TOKEN_WHEN = 364,
     SQL_TOKEN_ELSE = 365,
     SQL_TOKEN_ROW = 366,
     SQL_TOKEN_VALUE = 367,
     SQL_TOKEN_CURRENT_CATALOG = 368,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 369,
     SQL_TOKEN_CURRENT_PATH = 370,
     SQL_TOKEN_CURRENT_ROLE = 371,
     SQL_TOKEN_CURRENT_SCHEMA = 372,
     SQL_TOKEN_VARCHAR = 373,
     SQL_TOKEN_VARBINARY = 374,
     SQL_TOKEN_BLOB = 375,
     SQL_TOKEN_BIGI = 376,
     SQL_TOKEN_OVER = 377,
     SQL_TOKEN_ROW_NUMBER = 378,
     SQL_TOKEN_NTILE = 379,
     SQL_TOKEN_LEAD = 380,
     SQL_TOKEN_LAG = 381,
     SQL_TOKEN_RESPECT = 382,
     SQL_TOKEN_IGNORE = 383,
     SQL_TOKEN_NULLS = 384,
     SQL_TOKEN_FIRST_VALUE = 385,
     SQL_TOKEN_LAST_VALUE = 386,
     SQL_TOKEN_NTH_VALUE = 387,
     SQL_TOKEN_FIRST = 388,
     SQL_TOKEN_LAST = 389,
     SQL_TOKEN_EXCLUDE = 390,
     SQL_TOKEN_OTHERS = 391,
     SQL_TOKEN_TIES = 392,
     SQL_TOKEN_FOLLOWING = 393,
     SQL_TOKEN_UNBOUNDED = 394,
     SQL_TOKEN_PRECEDING = 395,
     SQL_TOKEN_RANGE = 396,
     SQL_TOKEN_ROWS = 397,
     SQL_TOKEN_PARTITION = 398,
     SQL_TOKEN_WINDOW = 399,
     SQL_TOKEN_NO = 400,
     SQL_TOKEN_LIMIT = 401,
     SQL_TOKEN_OFFSET = 402,
     SQL_TOKEN_ONLY = 403,
     SQL_TOKEN_MATCH = 404,
     SQL_TOKEN_ECSQLOPTIONS = 405,
     SQL_TOKEN_BINARY = 406,
     SQL_TOKEN_BOOLEAN = 407,
     SQL_TOKEN_DOUBLE = 408,
     SQL_TOKEN_INTEGER = 409,
     SQL_TOKEN_INT = 410,
     SQL_TOKEN_LONG = 411,
     SQL_TOKEN_INT64 = 412,
     SQL_TOKEN_STRING = 413,
     SQL_TOKEN_DATE = 414,
     SQL_TOKEN_TIMESTAMP = 415,
     SQL_TOKEN_DATETIME = 416,
     SQL_TOKEN_OR = 417,
     SQL_TOKEN_AND = 418,
     SQL_EQUAL = 419,
     SQL_GREAT = 420,
     SQL_LESS = 421,
     SQL_NOTEQUAL = 422,
     SQL_GREATEQ = 423,
     SQL_LESSEQ = 424,
     SQL_CONCAT = 425,
     SQL_TOKEN_INVALIDSYMBOL = 426
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

extern YYSTYPE SQLyylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int SQLyyparse (void *YYPARSE_PARAM);
#else
int SQLyyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int SQLyyparse (void);
#else
int SQLyyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_SQLYY_D_DEV_DGNDB_BIM20PROPMAPREFACTOR_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
