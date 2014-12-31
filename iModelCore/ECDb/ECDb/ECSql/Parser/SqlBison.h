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

#ifndef YY_SQLYY_E_DEV_WORKING_GRAPHITE0502_SRC_BESQLITE_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_E_DEV_WORKING_GRAPHITE0502_SRC_BESQLITE_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_INTO = 319,
     SQL_TOKEN_IS = 320,
     SQL_TOKEN_INTERSECT = 321,
     SQL_TOKEN_JOIN = 322,
     SQL_TOKEN_KEY = 323,
     SQL_TOKEN_LEADING = 324,
     SQL_TOKEN_LIKE = 325,
     SQL_TOKEN_LOCAL = 326,
     SQL_TOKEN_LOWER = 327,
     SQL_TOKEN_MAX = 328,
     SQL_TOKEN_MIN = 329,
     SQL_TOKEN_NATURAL = 330,
     SQL_TOKEN_NCHAR = 331,
     SQL_TOKEN_NULL = 332,
     SQL_TOKEN_NUMERIC = 333,
     SQL_TOKEN_OCTET_LENGTH = 334,
     SQL_TOKEN_OF = 335,
     SQL_TOKEN_ON = 336,
     SQL_TOKEN_OPTION = 337,
     SQL_TOKEN_ORDER = 338,
     SQL_TOKEN_OUTER = 339,
     SQL_TOKEN_PRECISION = 340,
     SQL_TOKEN_PRIMARY = 341,
     SQL_TOKEN_PROCEDURE = 342,
     SQL_TOKEN_PUBLIC = 343,
     SQL_TOKEN_REAL = 344,
     SQL_TOKEN_REFERENCES = 345,
     SQL_TOKEN_ROLLBACK = 346,
     SQL_TOKEN_SCHEMA = 347,
     SQL_TOKEN_SELECT = 348,
     SQL_TOKEN_SET = 349,
     SQL_TOKEN_SMALLINT = 350,
     SQL_TOKEN_SOME = 351,
     SQL_TOKEN_SQLCODE = 352,
     SQL_TOKEN_SQLERROR = 353,
     SQL_TOKEN_SUM = 354,
     SQL_TOKEN_TABLE = 355,
     SQL_TOKEN_TO = 356,
     SQL_TOKEN_TRAILING = 357,
     SQL_TOKEN_TRANSLATE = 358,
     SQL_TOKEN_TRIM = 359,
     SQL_TOKEN_TRUE = 360,
     SQL_TOKEN_UNION = 361,
     SQL_TOKEN_UNIQUE = 362,
     SQL_TOKEN_UNKNOWN = 363,
     SQL_TOKEN_UPDATE = 364,
     SQL_TOKEN_UPPER = 365,
     SQL_TOKEN_USAGE = 366,
     SQL_TOKEN_USING = 367,
     SQL_TOKEN_VALUES = 368,
     SQL_TOKEN_VIEW = 369,
     SQL_TOKEN_WHERE = 370,
     SQL_TOKEN_WITH = 371,
     SQL_TOKEN_WORK = 372,
     SQL_TOKEN_ASCII = 373,
     SQL_TOKEN_BIT_LENGTH = 374,
     SQL_TOKEN_CHAR = 375,
     SQL_TOKEN_CHAR_LENGTH = 376,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 377,
     SQL_TOKEN_CONCAT = 378,
     SQL_TOKEN_DIFFERENCE = 379,
     SQL_TOKEN_INSERT = 380,
     SQL_TOKEN_LEFT = 381,
     SQL_TOKEN_LENGTH = 382,
     SQL_TOKEN_LOCATE = 383,
     SQL_TOKEN_LOCATE_2 = 384,
     SQL_TOKEN_LTRIM = 385,
     SQL_TOKEN_POSITION = 386,
     SQL_TOKEN_REPEAT = 387,
     SQL_TOKEN_REPLACE = 388,
     SQL_TOKEN_RIGHT = 389,
     SQL_TOKEN_RTRIM = 390,
     SQL_TOKEN_SOUNDEX = 391,
     SQL_TOKEN_SPACE = 392,
     SQL_TOKEN_SUBSTRING = 393,
     SQL_TOKEN_CURRENT_DATE = 394,
     SQL_TOKEN_CURRENT_TIMESTAMP = 395,
     SQL_TOKEN_CURDATE = 396,
     SQL_TOKEN_NOW = 397,
     SQL_TOKEN_EXTRACT = 398,
     SQL_TOKEN_DAYNAME = 399,
     SQL_TOKEN_DAYOFMONTH = 400,
     SQL_TOKEN_DAYOFWEEK = 401,
     SQL_TOKEN_DAYOFYEAR = 402,
     SQL_TOKEN_HOUR = 403,
     SQL_TOKEN_MINUTE = 404,
     SQL_TOKEN_MONTH = 405,
     SQL_TOKEN_MONTHNAME = 406,
     SQL_TOKEN_QUARTER = 407,
     SQL_TOKEN_DATEDIFF = 408,
     SQL_TOKEN_SECOND = 409,
     SQL_TOKEN_TIMESTAMPADD = 410,
     SQL_TOKEN_TIMESTAMPDIFF = 411,
     SQL_TOKEN_TIMEVALUE = 412,
     SQL_TOKEN_WEEK = 413,
     SQL_TOKEN_YEAR = 414,
     SQL_TOKEN_ABS = 415,
     SQL_TOKEN_ACOS = 416,
     SQL_TOKEN_ASIN = 417,
     SQL_TOKEN_ATAN = 418,
     SQL_TOKEN_ATAN2 = 419,
     SQL_TOKEN_CEILING = 420,
     SQL_TOKEN_COS = 421,
     SQL_TOKEN_COT = 422,
     SQL_TOKEN_DEGREES = 423,
     SQL_TOKEN_EXP = 424,
     SQL_TOKEN_FLOOR = 425,
     SQL_TOKEN_LOGF = 426,
     SQL_TOKEN_LOG = 427,
     SQL_TOKEN_LN = 428,
     SQL_TOKEN_LOG10 = 429,
     SQL_TOKEN_MOD = 430,
     SQL_TOKEN_PI = 431,
     SQL_TOKEN_POWER = 432,
     SQL_TOKEN_RADIANS = 433,
     SQL_TOKEN_RAND = 434,
     SQL_TOKEN_ROUNDMAGIC = 435,
     SQL_TOKEN_ROUND = 436,
     SQL_TOKEN_SIGN = 437,
     SQL_TOKEN_SIN = 438,
     SQL_TOKEN_SQRT = 439,
     SQL_TOKEN_TAN = 440,
     SQL_TOKEN_TRUNCATE = 441,
     SQL_TOKEN_EVERY = 442,
     SQL_TOKEN_INTERSECTION = 443,
     SQL_TOKEN_FUSION = 444,
     SQL_TOKEN_COLLECT = 445,
     SQL_TOKEN_WITHIN = 446,
     SQL_TOKEN_ARRAY_AGG = 447,
     SQL_TOKEN_CASE = 448,
     SQL_TOKEN_THEN = 449,
     SQL_TOKEN_END = 450,
     SQL_TOKEN_NULLIF = 451,
     SQL_TOKEN_COALESCE = 452,
     SQL_TOKEN_WHEN = 453,
     SQL_TOKEN_ELSE = 454,
     SQL_TOKEN_BEFORE = 455,
     SQL_TOKEN_AFTER = 456,
     SQL_TOKEN_INSTEAD = 457,
     SQL_TOKEN_EACH = 458,
     SQL_TOKEN_REFERENCING = 459,
     SQL_TOKEN_BEGIN = 460,
     SQL_TOKEN_ATOMIC = 461,
     SQL_TOKEN_TRIGGER = 462,
     SQL_TOKEN_ROW = 463,
     SQL_TOKEN_STATEMENT = 464,
     SQL_TOKEN_NEW = 465,
     SQL_TOKEN_OLD = 466,
     SQL_TOKEN_VALUE = 467,
     SQL_TOKEN_CURRENT_CATALOG = 468,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 469,
     SQL_TOKEN_CURRENT_PATH = 470,
     SQL_TOKEN_CURRENT_ROLE = 471,
     SQL_TOKEN_CURRENT_SCHEMA = 472,
     SQL_TOKEN_VARCHAR = 473,
     SQL_TOKEN_VARBINARY = 474,
     SQL_TOKEN_VARYING = 475,
     SQL_TOKEN_OBJECT = 476,
     SQL_TOKEN_NCLOB = 477,
     SQL_TOKEN_NATIONAL = 478,
     SQL_TOKEN_LARGE = 479,
     SQL_TOKEN_CLOB = 480,
     SQL_TOKEN_BLOB = 481,
     SQL_TOKEN_BIGI = 482,
     SQL_TOKEN_INTERVAL = 483,
     SQL_TOKEN_OVER = 484,
     SQL_TOKEN_ROW_NUMBER = 485,
     SQL_TOKEN_NTILE = 486,
     SQL_TOKEN_LEAD = 487,
     SQL_TOKEN_LAG = 488,
     SQL_TOKEN_RESPECT = 489,
     SQL_TOKEN_IGNORE = 490,
     SQL_TOKEN_NULLS = 491,
     SQL_TOKEN_FIRST_VALUE = 492,
     SQL_TOKEN_LAST_VALUE = 493,
     SQL_TOKEN_NTH_VALUE = 494,
     SQL_TOKEN_FIRST = 495,
     SQL_TOKEN_LAST = 496,
     SQL_TOKEN_EXCLUDE = 497,
     SQL_TOKEN_OTHERS = 498,
     SQL_TOKEN_TIES = 499,
     SQL_TOKEN_FOLLOWING = 500,
     SQL_TOKEN_UNBOUNDED = 501,
     SQL_TOKEN_PRECEDING = 502,
     SQL_TOKEN_RANGE = 503,
     SQL_TOKEN_ROWS = 504,
     SQL_TOKEN_PARTITION = 505,
     SQL_TOKEN_WINDOW = 506,
     SQL_TOKEN_NO = 507,
     SQL_TOKEN_GETECCLASSID = 508,
     SQL_TOKEN_LIMIT = 509,
     SQL_TOKEN_OFFSET = 510,
     SQL_TOKEN_NEXT = 511,
     SQL_TOKEN_ONLY = 512,
     SQL_TOKEN_BINARY = 513,
     SQL_TOKEN_BOOLEAN = 514,
     SQL_TOKEN_DOUBLE = 515,
     SQL_TOKEN_INTEGER = 516,
     SQL_TOKEN_INT = 517,
     SQL_TOKEN_INT32 = 518,
     SQL_TOKEN_LONG = 519,
     SQL_TOKEN_INT64 = 520,
     SQL_TOKEN_STRING = 521,
     SQL_TOKEN_DATE = 522,
     SQL_TOKEN_TIMESTAMP = 523,
     SQL_TOKEN_DATETIME = 524,
     SQL_TOKEN_POINT2D = 525,
     SQL_TOKEN_POINT3D = 526,
     SQL_TOKEN_OR = 527,
     SQL_TOKEN_AND = 528,
     SQL_EQUAL = 529,
     SQL_GREAT = 530,
     SQL_LESS = 531,
     SQL_NOTEQUAL = 532,
     SQL_GREATEQ = 533,
     SQL_LESSEQ = 534,
     SQL_CONCAT = 535,
     SQL_TOKEN_INVALIDSYMBOL = 536
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

#endif /* !YY_SQLYY_E_DEV_WORKING_GRAPHITE0502_SRC_BESQLITE_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
