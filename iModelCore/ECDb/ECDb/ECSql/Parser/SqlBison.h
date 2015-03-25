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

#ifndef YY_SQLYY_C_DEV_BSW_GRAPHITE_06_WORK_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_C_DEV_BSW_GRAPHITE_06_WORK_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_AT = 272,
     SQL_TOKEN_AUTHORIZATION = 273,
     SQL_TOKEN_AVG = 274,
     SQL_TOKEN_BETWEEN = 275,
     SQL_TOKEN_BIT = 276,
     SQL_TOKEN_BOTH = 277,
     SQL_TOKEN_BY = 278,
     SQL_TOKEN_CAST = 279,
     SQL_TOKEN_CHARACTER = 280,
     SQL_TOKEN_CHECK = 281,
     SQL_TOKEN_COLLATE = 282,
     SQL_TOKEN_COMMIT = 283,
     SQL_TOKEN_CONTINUE = 284,
     SQL_TOKEN_CONVERT = 285,
     SQL_TOKEN_COUNT = 286,
     SQL_TOKEN_CREATE = 287,
     SQL_TOKEN_CROSS = 288,
     SQL_TOKEN_CURRENT = 289,
     SQL_TOKEN_CURSOR = 290,
     SQL_TOKEN_DATEVALUE = 291,
     SQL_TOKEN_DAY = 292,
     SQL_TOKEN_DEC = 293,
     SQL_TOKEN_DECIMAL = 294,
     SQL_TOKEN_DECLARE = 295,
     SQL_TOKEN_DEFAULT = 296,
     SQL_TOKEN_DELETE = 297,
     SQL_TOKEN_DESC = 298,
     SQL_TOKEN_DISTINCT = 299,
     SQL_TOKEN_DROP = 300,
     SQL_TOKEN_FORWARD = 301,
     SQL_TOKEN_REVERSE = 302,
     SQL_TOKEN_ESCAPE = 303,
     SQL_TOKEN_EXCEPT = 304,
     SQL_TOKEN_EXISTS = 305,
     SQL_TOKEN_FALSE = 306,
     SQL_TOKEN_FETCH = 307,
     SQL_TOKEN_FLOAT = 308,
     SQL_TOKEN_FOR = 309,
     SQL_TOKEN_FOREIGN = 310,
     SQL_TOKEN_FOUND = 311,
     SQL_TOKEN_FROM = 312,
     SQL_TOKEN_FULL = 313,
     SQL_TOKEN_GROUP = 314,
     SQL_TOKEN_HAVING = 315,
     SQL_TOKEN_IN = 316,
     SQL_TOKEN_INDICATOR = 317,
     SQL_TOKEN_INNER = 318,
     SQL_TOKEN_INSERT = 319,
     SQL_TOKEN_INTO = 320,
     SQL_TOKEN_IS = 321,
     SQL_TOKEN_INTERSECT = 322,
     SQL_TOKEN_JOIN = 323,
     SQL_TOKEN_KEY = 324,
     SQL_TOKEN_LEADING = 325,
     SQL_TOKEN_LIKE = 326,
     SQL_TOKEN_LOCAL = 327,
     SQL_TOKEN_LEFT = 328,
     SQL_TOKEN_RIGHT = 329,
     SQL_TOKEN_LOWER = 330,
     SQL_TOKEN_MAX = 331,
     SQL_TOKEN_MIN = 332,
     SQL_TOKEN_NATURAL = 333,
     SQL_TOKEN_NCHAR = 334,
     SQL_TOKEN_NULL = 335,
     SQL_TOKEN_NUMERIC = 336,
     SQL_TOKEN_OCTET_LENGTH = 337,
     SQL_TOKEN_OF = 338,
     SQL_TOKEN_ON = 339,
     SQL_TOKEN_OPTION = 340,
     SQL_TOKEN_ORDER = 341,
     SQL_TOKEN_OUTER = 342,
     SQL_TOKEN_PRECISION = 343,
     SQL_TOKEN_PRIMARY = 344,
     SQL_TOKEN_PROCEDURE = 345,
     SQL_TOKEN_PUBLIC = 346,
     SQL_TOKEN_REAL = 347,
     SQL_TOKEN_REFERENCES = 348,
     SQL_TOKEN_ROLLBACK = 349,
     SQL_TOKEN_SCHEMA = 350,
     SQL_TOKEN_SELECT = 351,
     SQL_TOKEN_SET = 352,
     SQL_TOKEN_SMALLINT = 353,
     SQL_TOKEN_SOME = 354,
     SQL_TOKEN_SQLCODE = 355,
     SQL_TOKEN_SQLERROR = 356,
     SQL_TOKEN_SUM = 357,
     SQL_TOKEN_TABLE = 358,
     SQL_TOKEN_TO = 359,
     SQL_TOKEN_TRAILING = 360,
     SQL_TOKEN_TRANSLATE = 361,
     SQL_TOKEN_TRIM = 362,
     SQL_TOKEN_TRUE = 363,
     SQL_TOKEN_UNION = 364,
     SQL_TOKEN_UNIQUE = 365,
     SQL_TOKEN_UNKNOWN = 366,
     SQL_TOKEN_UPDATE = 367,
     SQL_TOKEN_UPPER = 368,
     SQL_TOKEN_USAGE = 369,
     SQL_TOKEN_USING = 370,
     SQL_TOKEN_VALUES = 371,
     SQL_TOKEN_VIEW = 372,
     SQL_TOKEN_WHERE = 373,
     SQL_TOKEN_WITH = 374,
     SQL_TOKEN_WORK = 375,
     SQL_TOKEN_BIT_LENGTH = 376,
     SQL_TOKEN_CHAR = 377,
     SQL_TOKEN_CHAR_LENGTH = 378,
     SQL_TOKEN_POSITION = 379,
     SQL_TOKEN_SUBSTRING = 380,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 381,
     SQL_TOKEN_CURRENT_DATE = 382,
     SQL_TOKEN_CURRENT_TIMESTAMP = 383,
     SQL_TOKEN_CURDATE = 384,
     SQL_TOKEN_NOW = 385,
     SQL_TOKEN_EXTRACT = 386,
     SQL_TOKEN_DAYNAME = 387,
     SQL_TOKEN_DAYOFMONTH = 388,
     SQL_TOKEN_DAYOFWEEK = 389,
     SQL_TOKEN_DAYOFYEAR = 390,
     SQL_TOKEN_HOUR = 391,
     SQL_TOKEN_MINUTE = 392,
     SQL_TOKEN_MONTH = 393,
     SQL_TOKEN_MONTHNAME = 394,
     SQL_TOKEN_QUARTER = 395,
     SQL_TOKEN_DATEDIFF = 396,
     SQL_TOKEN_SECOND = 397,
     SQL_TOKEN_TIMESTAMPADD = 398,
     SQL_TOKEN_TIMESTAMPDIFF = 399,
     SQL_TOKEN_TIMEVALUE = 400,
     SQL_TOKEN_WEEK = 401,
     SQL_TOKEN_YEAR = 402,
     SQL_TOKEN_EVERY = 403,
     SQL_TOKEN_WITHIN = 404,
     SQL_TOKEN_ARRAY_AGG = 405,
     SQL_TOKEN_CASE = 406,
     SQL_TOKEN_THEN = 407,
     SQL_TOKEN_END = 408,
     SQL_TOKEN_NULLIF = 409,
     SQL_TOKEN_COALESCE = 410,
     SQL_TOKEN_WHEN = 411,
     SQL_TOKEN_ELSE = 412,
     SQL_TOKEN_BEFORE = 413,
     SQL_TOKEN_AFTER = 414,
     SQL_TOKEN_INSTEAD = 415,
     SQL_TOKEN_EACH = 416,
     SQL_TOKEN_REFERENCING = 417,
     SQL_TOKEN_BEGIN = 418,
     SQL_TOKEN_ATOMIC = 419,
     SQL_TOKEN_TRIGGER = 420,
     SQL_TOKEN_ROW = 421,
     SQL_TOKEN_STATEMENT = 422,
     SQL_TOKEN_NEW = 423,
     SQL_TOKEN_OLD = 424,
     SQL_TOKEN_VALUE = 425,
     SQL_TOKEN_CURRENT_CATALOG = 426,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 427,
     SQL_TOKEN_CURRENT_PATH = 428,
     SQL_TOKEN_CURRENT_ROLE = 429,
     SQL_TOKEN_CURRENT_SCHEMA = 430,
     SQL_TOKEN_VARCHAR = 431,
     SQL_TOKEN_VARBINARY = 432,
     SQL_TOKEN_VARYING = 433,
     SQL_TOKEN_OBJECT = 434,
     SQL_TOKEN_NCLOB = 435,
     SQL_TOKEN_NATIONAL = 436,
     SQL_TOKEN_LARGE = 437,
     SQL_TOKEN_CLOB = 438,
     SQL_TOKEN_BLOB = 439,
     SQL_TOKEN_BIGI = 440,
     SQL_TOKEN_INTERVAL = 441,
     SQL_TOKEN_OVER = 442,
     SQL_TOKEN_ROW_NUMBER = 443,
     SQL_TOKEN_NTILE = 444,
     SQL_TOKEN_LEAD = 445,
     SQL_TOKEN_LAG = 446,
     SQL_TOKEN_RESPECT = 447,
     SQL_TOKEN_IGNORE = 448,
     SQL_TOKEN_NULLS = 449,
     SQL_TOKEN_FIRST_VALUE = 450,
     SQL_TOKEN_LAST_VALUE = 451,
     SQL_TOKEN_NTH_VALUE = 452,
     SQL_TOKEN_FIRST = 453,
     SQL_TOKEN_LAST = 454,
     SQL_TOKEN_EXCLUDE = 455,
     SQL_TOKEN_OTHERS = 456,
     SQL_TOKEN_TIES = 457,
     SQL_TOKEN_FOLLOWING = 458,
     SQL_TOKEN_UNBOUNDED = 459,
     SQL_TOKEN_PRECEDING = 460,
     SQL_TOKEN_RANGE = 461,
     SQL_TOKEN_ROWS = 462,
     SQL_TOKEN_PARTITION = 463,
     SQL_TOKEN_WINDOW = 464,
     SQL_TOKEN_NO = 465,
     SQL_TOKEN_GETECCLASSID = 466,
     SQL_TOKEN_LIMIT = 467,
     SQL_TOKEN_OFFSET = 468,
     SQL_TOKEN_NEXT = 469,
     SQL_TOKEN_ONLY = 470,
     SQL_TOKEN_BINARY = 471,
     SQL_TOKEN_BOOLEAN = 472,
     SQL_TOKEN_DOUBLE = 473,
     SQL_TOKEN_INTEGER = 474,
     SQL_TOKEN_INT = 475,
     SQL_TOKEN_INT32 = 476,
     SQL_TOKEN_LONG = 477,
     SQL_TOKEN_INT64 = 478,
     SQL_TOKEN_STRING = 479,
     SQL_TOKEN_DATE = 480,
     SQL_TOKEN_TIMESTAMP = 481,
     SQL_TOKEN_DATETIME = 482,
     SQL_TOKEN_POINT2D = 483,
     SQL_TOKEN_POINT3D = 484,
     SQL_TOKEN_OR = 485,
     SQL_TOKEN_AND = 486,
     SQL_EQUAL = 487,
     SQL_GREAT = 488,
     SQL_LESS = 489,
     SQL_NOTEQUAL = 490,
     SQL_GREATEQ = 491,
     SQL_LESSEQ = 492,
     SQL_CONCAT = 493,
     SQL_TOKEN_INVALIDSYMBOL = 494
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

#endif /* !YY_SQLYY_C_DEV_BSW_GRAPHITE_06_WORK_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
