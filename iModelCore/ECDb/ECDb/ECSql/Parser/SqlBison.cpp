/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */


/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

#pragma warning(disable:4603)
#include "ECDbPch.h"
#include "SqlTypes.h"
#include <vector>
#include <string.h>

#ifndef _CONNECTIVITY_SQLNODE_HXX
#include "SqlNode.h"
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "SqlParse.h"
#endif
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include "InternalNode.h"
#endif
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#include "SqlScan.h"
#endif

#if defined __GNUC__
//    #pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4273 4701 4706)
#endif





/* Substitute the variable and function names.  */
#define yyparse         SQLyyparse
#define yylex           SQLyylex
#define yyerror         SQLyyerror
#define yylval          SQLyylval
#define yychar          SQLyychar
#define yydebug         SQLyydebug
#define yynerrs         SQLyynerrs

/* Copy the first part of user declarations.  */


static Utf8String aEmptyString;

static connectivity::OSQLInternalNode* newNode(const sal_Char* pNewValue,
                                 const connectivity::SQLNodeType eNodeType,
                                 const sal_uInt32 nNodeID = 0)
{
    return new connectivity::OSQLInternalNode(pNewValue, eNodeType, nNodeID);
}

static connectivity::OSQLInternalNode* newNode(const Utf8String& _NewValue,
                                const connectivity::SQLNodeType eNodeType,
                                const sal_uInt32 nNodeID = 0)
{
    return new connectivity::OSQLInternalNode(_NewValue, eNodeType, nNodeID);
}

// yyi ist die interne Nr. der Regel, die gerade reduziert wird.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE             newNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE         newNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   newNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])
#define SQL_NEW_DOTLISTRULE   newNode(aEmptyString, SQL_NODE_DOTLISTRULE, yyr1[yyn])


connectivity::OSQLParser* xxx_pGLOBAL_SQLPARSER;

#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE
#endif

#define SQLyyerror(s)                        \
{                                            \
    xxx_pGLOBAL_SQLPARSER->error(s);        \
}

using namespace connectivity;
#define SQLyylex xxx_pGLOBAL_SQLPARSER->SQLlex



# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "SqlBison.h".  */
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

/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  313
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   6748

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  303
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  322
/* YYNRULES -- Number of rules.  */
#define YYNRULES  700
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1135

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   536

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     4,   299,   296,     5,   297,    13,   300,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   301,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,     2,    12,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   298,   302
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    16,    18,
      25,    27,    31,    33,    35,    39,    40,    43,    45,    47,
      50,    53,    55,    58,    61,    64,    66,    71,    74,    80,
      85,    93,   104,   109,   113,   115,   119,   121,   129,   130,
     134,   135,   139,   140,   144,   145,   149,   151,   155,   158,
     161,   162,   164,   166,   167,   169,   171,   173,   175,   177,
     179,   181,   183,   185,   187,   192,   195,   200,   205,   211,
     216,   218,   222,   224,   226,   229,   236,   237,   239,   241,
     243,   247,   251,   253,   255,   261,   263,   267,   269,   270,
     272,   274,   279,   281,   283,   284,   286,   290,   291,   293,
     295,   297,   299,   301,   302,   304,   310,   312,   314,   315,
     317,   318,   321,   325,   335,   338,   340,   344,   345,   347,
     348,   350,   351,   355,   356,   358,   362,   367,   369,   372,
     373,   377,   378,   381,   383,   385,   387,   389,   391,   395,
     397,   401,   403,   408,   410,   413,   415,   419,   421,   425,
     427,   429,   431,   433,   435,   437,   439,   441,   444,   448,
     451,   453,   455,   457,   459,   461,   463,   466,   472,   475,
     480,   485,   488,   491,   493,   495,   496,   499,   503,   506,
     508,   510,   514,   518,   521,   523,   527,   530,   532,   534,
     536,   539,   542,   546,   548,   552,   554,   556,   558,   560,
     562,   564,   566,   569,   572,   575,   578,   579,   582,   584,
     591,   596,   598,   600,   602,   607,   612,   617,   622,   624,
     626,   628,   630,   632,   634,   636,   643,   645,   647,   649,
     651,   653,   655,   657,   659,   661,   663,   665,   667,   669,
     671,   675,   679,   684,   689,   694,   699,   704,   709,   714,
     716,   718,   720,   722,   724,   726,   728,   730,   732,   734,
     736,   738,   740,   742,   744,   746,   748,   750,   752,   754,
     756,   758,   760,   762,   764,   766,   768,   770,   772,   774,
     776,   778,   780,   782,   784,   786,   788,   790,   792,   794,
     796,   798,   800,   802,   804,   806,   808,   810,   812,   814,
     816,   818,   820,   822,   824,   826,   830,   834,   836,   838,
     840,   842,   844,   849,   851,   853,   855,   857,   858,   861,
     866,   867,   869,   876,   878,   880,   882,   884,   886,   889,
     892,   898,   900,   902,   903,   905,   914,   916,   918,   921,
     924,   926,   928,   930,   932,   933,   935,   938,   942,   944,
     948,   950,   954,   955,   957,   958,   960,   961,   963,   968,
     970,   974,   978,   980,   983,   984,   986,   990,   992,   994,
     996,   998,  1001,  1003,  1006,  1009,  1014,  1016,  1018,  1020,
    1023,  1025,  1028,  1032,  1035,  1038,  1042,  1048,  1053,  1059,
    1061,  1063,  1065,  1067,  1069,  1071,  1073,  1075,  1077,  1079,
    1081,  1083,  1085,  1088,  1090,  1092,  1093,  1095,  1097,  1100,
    1105,  1111,  1117,  1119,  1127,  1128,  1130,  1132,  1134,  1136,
    1141,  1143,  1145,  1147,  1151,  1153,  1158,  1160,  1162,  1167,
    1172,  1173,  1175,  1177,  1179,  1181,  1183,  1185,  1187,  1189,
    1191,  1193,  1195,  1197,  1199,  1201,  1203,  1205,  1207,  1209,
    1211,  1218,  1220,  1222,  1224,  1226,  1228,  1230,  1232,  1236,
    1238,  1242,  1248,  1250,  1252,  1254,  1257,  1260,  1262,  1266,
    1270,  1272,  1276,  1280,  1282,  1284,  1286,  1289,  1292,  1294,
    1296,  1298,  1300,  1302,  1304,  1306,  1308,  1311,  1313,  1316,
    1319,  1322,  1326,  1328,  1332,  1338,  1346,  1348,  1352,  1356,
    1358,  1360,  1364,  1368,  1370,  1372,  1374,  1376,  1378,  1380,
    1384,  1388,  1390,  1392,  1395,  1397,  1400,  1402,  1404,  1406,
    1414,  1416,  1418,  1419,  1421,  1423,  1425,  1427,  1429,  1430,
    1433,  1441,  1446,  1448,  1450,  1455,  1462,  1469,  1476,  1481,
    1486,  1490,  1494,  1497,  1499,  1501,  1503,  1505,  1508,  1510,
    1512,  1514,  1518,  1522,  1526,  1528,  1529,  1531,  1533,  1537,
    1540,  1542,  1544,  1546,  1547,  1551,  1552,  1554,  1558,  1561,
    1563,  1565,  1567,  1569,  1571,  1574,  1577,  1581,  1585,  1588,
    1590,  1591,  1593,  1597,  1598,  1600,  1604,  1607,  1608,  1610,
    1612,  1614,  1616,  1618,  1623,  1628,  1631,  1635,  1639,  1642,
    1647,  1652,  1656,  1658,  1664,  1669,  1672,  1675,  1679,  1682,
    1684,  1689,  1692,  1694,  1696,  1697,  1701,  1707,  1709,  1711,
    1713,  1715,  1717,  1719,  1721,  1723,  1725,  1727,  1730,  1733,
    1735,  1737,  1739,  1744,  1749,  1754,  1756,  1758,  1764,  1769,
    1771,  1774,  1779,  1781,  1785,  1787,  1789,  1791,  1793,  1795,
    1797,  1799,  1802,  1807,  1808,  1811,  1813,  1815,  1817,  1819,
    1822,  1824,  1825,  1828,  1830,  1834,  1844,  1845,  1848,  1850,
    1852,  1855,  1857,  1859,  1862,  1863,  1866,  1868,  1872,  1873,
    1877,  1879,  1881,  1882,  1885,  1887,  1893,  1895,  1899,  1901,
    1903,  1906,  1911,  1916,  1921,  1926,  1928,  1930,  1932,  1934,
    1936
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     304,     0,    -1,   305,    -1,   305,     7,    -1,   327,    -1,
     306,    -1,   307,    -1,   318,    -1,   604,    -1,    48,   116,
     554,     3,   308,     4,    -1,   309,    -1,   308,     5,   309,
      -1,   310,    -1,   315,    -1,   585,   562,   311,    -1,    -1,
     311,   314,    -1,   515,    -1,   123,    -1,   102,    84,    -1,
      23,    93,    -1,   313,    -1,    57,   404,    -1,    57,    93,
      -1,    57,   312,    -1,    42,    -1,    42,     3,   379,     4,
      -1,   106,   554,    -1,   106,   554,     3,   316,     4,    -1,
     313,     3,   316,     4,    -1,    71,    84,     3,   316,     4,
     106,   554,    -1,    71,    84,     3,   316,     4,   106,   554,
       3,   316,     4,    -1,    42,     3,   379,     4,    -1,   316,
       5,   585,    -1,   585,    -1,   317,     5,   561,    -1,   561,
      -1,    48,   130,   554,   321,    31,   348,   319,    -1,    -1,
     132,    42,    98,    -1,    -1,     3,   316,     4,    -1,    -1,
       3,   317,     4,    -1,    -1,    99,    39,   323,    -1,   324,
      -1,   323,     5,   324,    -1,   380,   325,    -1,   336,   325,
      -1,    -1,    32,    -1,    59,    -1,    -1,    23,    -1,   329,
      -1,   330,    -1,   331,    -1,   332,    -1,   337,    -1,   338,
      -1,   343,    -1,   328,    -1,   348,    -1,   328,   122,   501,
     348,    -1,    44,   133,    -1,    58,    73,   369,   346,    -1,
      68,   601,    80,   344,    -1,   141,    80,   554,   321,   333,
      -1,   129,     3,   334,     4,    -1,   335,    -1,   334,     5,
     335,    -1,   336,    -1,   530,    -1,   107,   133,    -1,   109,
     339,   349,    80,   344,   362,    -1,    -1,    27,    -1,    60,
      -1,   341,    -1,   340,     5,   341,    -1,   561,   290,   342,
      -1,   530,    -1,    57,    -1,   125,   369,   110,   340,   346,
      -1,   345,    -1,   344,     5,   345,    -1,   403,    -1,    -1,
     370,    -1,   498,    -1,   109,   339,   349,   362,    -1,   299,
      -1,   401,    -1,    -1,   351,    -1,   271,   357,   354,    -1,
      -1,   358,    -1,   256,    -1,   272,    -1,   224,    -1,   265,
      -1,    -1,   356,    -1,    68,   353,   352,   354,   273,    -1,
     404,    -1,   404,    -1,    -1,   361,    -1,    -1,   271,   513,
      -1,   270,   513,   360,    -1,   363,   346,   371,   372,   458,
     322,   359,   350,   355,    -1,    73,   364,    -1,   369,    -1,
     364,     5,   369,    -1,    -1,    31,    -1,    -1,   224,    -1,
      -1,   365,    24,   320,    -1,    -1,   273,    -1,   368,   554,
     367,    -1,   368,   400,   603,   321,    -1,   494,    -1,   131,
     379,    -1,    -1,    75,    39,   317,    -1,    -1,    76,   379,
      -1,   121,    -1,    67,    -1,   124,    -1,    93,    -1,   380,
      -1,     3,   379,     4,    -1,   336,    -1,     3,   379,     4,
      -1,   374,    -1,   374,    81,   326,   373,    -1,   376,    -1,
      23,   376,    -1,   377,    -1,   378,   289,   377,    -1,   378,
      -1,   379,   288,   378,    -1,   382,    -1,   385,    -1,   396,
      -1,   398,    -1,   399,    -1,   391,    -1,   394,    -1,   388,
      -1,   383,   335,    -1,   335,   383,   335,    -1,   383,   335,
      -1,   292,    -1,   293,    -1,   290,    -1,   291,    -1,   295,
      -1,   294,    -1,    81,   326,    -1,   326,    36,   335,   289,
     335,    -1,   335,   384,    -1,   326,    86,   531,   389,    -1,
     326,    86,   508,   389,    -1,   335,   386,    -1,   335,   387,
      -1,   386,    -1,   387,    -1,    -1,    64,   531,    -1,    81,
     326,    93,    -1,   335,   390,    -1,   390,    -1,   400,    -1,
       3,   527,     4,    -1,   326,    77,   392,    -1,   335,   393,
      -1,   393,    -1,   383,   397,   400,    -1,   335,   395,    -1,
      30,    -1,    27,    -1,   112,    -1,    66,   400,    -1,   123,
     400,    -1,     3,   502,     4,    -1,   402,    -1,   401,     5,
     402,    -1,   553,    -1,   602,    -1,   278,    -1,    20,    -1,
      21,    -1,    22,    -1,    19,    -1,   404,   282,    -1,   404,
     278,    -1,   404,    20,    -1,   404,    22,    -1,    -1,    31,
     585,    -1,   585,    -1,   147,     3,   530,    77,   530,     4,
      -1,   147,     3,   527,     4,    -1,   406,    -1,   414,    -1,
     411,    -1,   137,     3,   530,     4,    -1,   138,     3,   530,
       4,    -1,    95,     3,   530,     4,    -1,   135,     3,   530,
       4,    -1,   408,    -1,   409,    -1,   410,    -1,   519,    -1,
     170,    -1,   412,    -1,   530,    -1,   159,     3,   413,    73,
     530,     4,    -1,   416,    -1,   404,    -1,   602,    -1,    93,
      -1,    67,    -1,   121,    -1,   228,    -1,   229,    -1,   230,
      -1,   231,    -1,   232,    -1,   233,    -1,   484,    -1,   547,
      -1,   424,     3,     4,    -1,   418,     3,     4,    -1,   419,
       3,   528,     4,    -1,   420,     3,   524,     4,    -1,   423,
       3,   525,     4,    -1,   428,     3,   526,     4,    -1,   424,
       3,   529,     4,    -1,   421,     3,   529,     4,    -1,   422,
       3,   529,     4,    -1,   430,    -1,   431,    -1,   425,    -1,
     432,    -1,   426,    -1,   433,    -1,   197,    -1,   174,    -1,
     187,    -1,   188,    -1,   144,    -1,   427,    -1,   429,    -1,
     434,    -1,    24,    -1,   143,    -1,   134,    -1,   146,    -1,
     151,    -1,   153,    -1,   148,    -1,   142,    -1,   150,    -1,
     149,    -1,   141,    -1,   136,    -1,   139,    -1,   140,    -1,
     145,    -1,   152,    -1,   157,    -1,   158,    -1,   192,    -1,
     176,    -1,   177,    -1,   178,    -1,   179,    -1,   181,    -1,
     182,    -1,   183,    -1,   184,    -1,   186,    -1,   198,    -1,
     199,    -1,   200,    -1,   201,    -1,   185,    -1,   190,    -1,
     189,    -1,   194,    -1,   196,    -1,   180,    -1,   191,    -1,
     193,    -1,   195,    -1,   202,    -1,   436,   245,   456,    -1,
     246,     3,     4,    -1,   484,    -1,   437,    -1,   443,    -1,
     449,    -1,   452,    -1,   247,     3,   440,     4,    -1,   602,
      -1,   404,    -1,   439,    -1,   438,    -1,    -1,     5,   446,
      -1,     5,   446,     5,   447,    -1,    -1,   448,    -1,   444,
       3,   445,   441,     4,   442,    -1,   248,    -1,   249,    -1,
     530,    -1,    21,    -1,   530,    -1,   250,   252,    -1,   251,
     252,    -1,   450,     3,   530,     4,   442,    -1,   253,    -1,
     254,    -1,    -1,   454,    -1,   255,     3,   530,     5,   453,
       4,   451,   442,    -1,   439,    -1,   438,    -1,    73,   256,
      -1,    73,   257,    -1,    24,    -1,   455,    -1,   457,    -1,
     463,    -1,    -1,   459,    -1,   267,   460,    -1,   460,     5,
     461,    -1,   461,    -1,   462,    31,   463,    -1,   455,    -1,
       3,   467,     4,    -1,    -1,   468,    -1,    -1,   469,    -1,
      -1,   473,    -1,   464,   465,   322,   466,    -1,   455,    -1,
     266,    39,   470,    -1,   470,     5,   471,    -1,   471,    -1,
     561,   564,    -1,    -1,   483,    -1,   474,   475,   472,    -1,
     265,    -1,   264,    -1,   476,    -1,   478,    -1,   262,   263,
      -1,   477,    -1,    50,   224,    -1,   415,   263,    -1,    36,
     479,   289,   480,    -1,   481,    -1,   481,    -1,   476,    -1,
     262,   261,    -1,   482,    -1,   415,   261,    -1,   258,    50,
     224,    -1,   258,    75,    -1,   258,   260,    -1,   258,   268,
     259,    -1,   485,     3,   339,   528,     4,    -1,    47,     3,
     299,     4,    -1,    47,     3,   339,   528,     4,    -1,    35,
      -1,    89,    -1,    90,    -1,   115,    -1,   203,    -1,    30,
      -1,   112,    -1,   206,    -1,   205,    -1,   204,    -1,   142,
      -1,   150,    -1,    74,    -1,    97,   379,    -1,   487,    -1,
     495,    -1,    -1,    79,    -1,   486,    -1,   486,   100,    -1,
     369,    49,    83,   369,    -1,   369,    91,   489,    83,   369,
      -1,   369,   489,    83,   369,   488,    -1,   490,    -1,   369,
     489,    83,   369,   128,   554,   493,    -1,    -1,    62,    -1,
      63,    -1,   492,    -1,   491,    -1,   128,     3,   316,     4,
      -1,   348,    -1,   333,    -1,   496,    -1,     3,   500,     4,
      -1,   497,    -1,   347,    82,   501,   499,    -1,   497,    -1,
     498,    -1,   502,   122,   501,   347,    -1,   502,    65,   501,
     347,    -1,    -1,    27,    -1,   500,    -1,   400,    -1,   530,
      -1,   506,    -1,   274,    -1,   275,    -1,   276,    -1,   277,
      -1,   278,    -1,   279,    -1,   280,    -1,   281,    -1,   282,
      -1,   285,    -1,   283,    -1,   284,    -1,   286,    -1,   287,
      -1,    40,     3,   504,    31,   505,     4,    -1,   415,    -1,
     417,    -1,   509,    -1,   561,    -1,   503,    -1,   586,    -1,
     435,    -1,     3,   530,     4,    -1,   507,    -1,   269,     3,
       4,    -1,   559,    13,   269,     3,     4,    -1,   508,    -1,
     407,    -1,   510,    -1,   297,   510,    -1,   296,   510,    -1,
     511,    -1,   512,   299,   511,    -1,   512,   300,   511,    -1,
     512,    -1,   513,   296,   512,    -1,   513,   297,   512,    -1,
     515,    -1,   155,    -1,   156,    -1,   283,   531,    -1,   284,
     531,    -1,   514,    -1,   516,    -1,   517,    -1,   175,    -1,
     166,    -1,    53,    -1,   164,    -1,   165,    -1,   519,   567,
      -1,   519,    -1,   170,   567,    -1,   519,   567,    -1,   170,
     579,    -1,   520,   117,   521,    -1,   522,    -1,   528,     5,
     528,    -1,   528,     5,   528,     5,   528,    -1,   528,     5,
     528,     5,   528,     5,   528,    -1,   530,    -1,   527,     5,
     530,    -1,   527,     7,   530,    -1,   598,    -1,   528,    -1,
     529,     5,   528,    -1,   529,     7,   528,    -1,   513,    -1,
     531,    -1,   518,    -1,   532,    -1,   536,    -1,   533,    -1,
     532,   296,   536,    -1,   530,   298,   530,    -1,   282,    -1,
     537,    -1,    43,   554,    -1,   534,    -1,   534,   535,    -1,
     543,    -1,   538,    -1,   539,    -1,   154,     3,   540,    73,
     531,   544,     4,    -1,   541,    -1,   542,    -1,    -1,   545,
      -1,   547,    -1,   548,    -1,   549,    -1,   550,    -1,    -1,
      70,   530,    -1,   154,     3,   530,    73,   530,   544,     4,
      -1,   154,     3,   527,     4,    -1,   126,    -1,    88,    -1,
     546,     3,   530,     4,    -1,    46,     3,   531,   128,   554,
       4,    -1,    46,     3,   504,     5,   505,     4,    -1,   119,
       3,   531,   128,   554,     4,    -1,   120,     3,   551,     4,
      -1,   552,   530,    73,   530,    -1,   552,    73,   530,    -1,
     530,    73,   530,    -1,    73,   530,    -1,   530,    -1,    38,
      -1,    85,    -1,   118,    -1,   530,   405,    -1,   557,    -1,
     556,    -1,   555,    -1,    24,    13,   556,    -1,    24,     6,
     556,    -1,    24,    13,   557,    -1,    24,    -1,    -1,    25,
      -1,   560,    -1,   559,    13,   560,    -1,    24,   558,    -1,
     299,    -1,   559,    -1,   565,    -1,    -1,    41,   110,    24,
      -1,    -1,   535,    -1,   566,   563,   564,    -1,   574,   564,
      -1,   576,    -1,   578,    -1,   582,    -1,   583,    -1,   584,
      -1,    41,   567,    -1,   136,   567,    -1,    41,   236,   568,
      -1,   136,   236,   568,    -1,   234,   568,    -1,   573,    -1,
      -1,   568,    -1,     3,    21,     4,    -1,    -1,   570,    -1,
       3,   571,     4,    -1,    21,   572,    -1,    -1,    14,    -1,
      15,    -1,    16,    -1,    17,    -1,    18,    -1,    41,   240,
     237,   569,    -1,   136,   240,   237,   569,    -1,   241,   569,
      -1,   239,    41,   567,    -1,   239,   136,   567,    -1,    92,
     567,    -1,   239,    41,   236,   568,    -1,   239,   136,   236,
     568,    -1,    92,   236,   568,    -1,   575,    -1,   239,    41,
     240,   237,   569,    -1,    92,   240,   237,   569,    -1,   238,
     569,    -1,   274,   567,    -1,   274,   236,   568,    -1,   235,
     568,    -1,   577,    -1,   274,   240,   237,   569,    -1,   242,
     569,    -1,   580,    -1,   581,    -1,    -1,     3,    21,     4,
      -1,     3,    21,     5,    21,     4,    -1,   277,    -1,   278,
      -1,   280,    -1,   279,    -1,   281,    -1,    69,    -1,   105,
      -1,   276,    -1,   275,    -1,   283,    -1,   284,   567,    -1,
     244,   523,    -1,    24,    -1,   587,    -1,   588,    -1,   212,
       3,   527,     4,    -1,   213,     3,   530,     4,    -1,   213,
       3,   527,     4,    -1,   589,    -1,   590,    -1,   209,   600,
     591,   597,   211,    -1,   209,   595,   597,   211,    -1,   592,
      -1,   595,   592,    -1,   214,   593,   210,   598,    -1,   594,
      -1,   593,     5,   594,    -1,   336,    -1,   381,    -1,   384,
      -1,   393,    -1,   386,    -1,   390,    -1,   596,    -1,   595,
     596,    -1,   214,   379,   210,   598,    -1,    -1,   215,   598,
      -1,   599,    -1,   530,    -1,   336,    -1,    24,    -1,     6,
      24,    -1,     8,    -1,    -1,   365,    24,    -1,   379,    -1,
       3,   305,     4,    -1,    48,   223,   624,   606,   607,    97,
     557,   605,   610,    -1,    -1,   220,   617,    -1,   216,    -1,
     217,    -1,   218,    96,    -1,   141,    -1,    58,    -1,   125,
     608,    -1,    -1,    96,   609,    -1,   316,    -1,   611,   613,
     614,    -1,    -1,    70,   219,   612,    -1,   224,    -1,   225,
      -1,    -1,   214,   375,    -1,   616,    -1,   221,   222,   615,
       7,   211,    -1,   616,    -1,   615,     7,   616,    -1,   305,
      -1,   618,    -1,   617,   618,    -1,   227,   366,   365,   622,
      -1,   226,   366,   365,   623,    -1,   227,   116,   365,   619,
      -1,   226,   116,   365,   620,    -1,   621,    -1,   621,    -1,
      24,    -1,    24,    -1,    24,    -1,    24,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   270,   270,   272,   280,   281,   343,   344,   345,   349,
     360,   363,   369,   370,   374,   383,   384,   390,   393,   394,
     402,   406,   407,   411,   415,   421,   422,   428,   432,   442,
     448,   457,   469,   478,   483,   491,   496,   504,   516,   517,
     527,   528,   536,   537,   575,   576,   586,   591,   605,   613,
     622,   623,   624,   640,   641,   647,   649,   650,   651,   652,
     653,   655,   656,   660,   661,   671,   690,   699,   708,   717,
     727,   732,   753,   764,   772,   783,   794,   795,   796,   815,
     818,   824,   831,   832,   835,   845,   848,   854,   858,   859,
     865,   873,   884,   889,   892,   893,   896,   905,   906,   909,
     910,   913,   914,   917,   918,   921,   932,   935,   939,   940,
     943,   944,   952,   961,   977,   987,   990,   996,   997,  1000,
    1001,  1004,  1007,  1016,  1017,  1021,  1028,  1036,  1059,  1068,
    1069,  1077,  1078,  1086,  1087,  1088,  1089,  1092,  1093,  1100,
    1126,  1135,  1136,  1146,  1147,  1155,  1156,  1165,  1166,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1185,  1192,  1199,
    1224,  1225,  1226,  1227,  1228,  1229,  1240,  1248,  1286,  1297,
    1307,  1317,  1323,  1329,  1351,  1376,  1377,  1393,  1402,  1408,
    1424,  1428,  1436,  1445,  1451,  1467,  1476,  1501,  1502,  1503,
    1507,  1515,  1521,  1532,  1537,  1553,  1558,  1590,  1591,  1592,
    1593,  1594,  1596,  1608,  1620,  1632,  1648,  1649,  1655,  1658,
    1668,  1678,  1679,  1680,  1683,  1691,  1702,  1712,  1722,  1727,
    1732,  1739,  1744,  1752,  1753,  1784,  1796,  1797,  1800,  1801,
    1802,  1803,  1804,  1805,  1806,  1807,  1808,  1809,  1812,  1813,
    1814,  1821,  1828,  1836,  1844,  1852,  1860,  1868,  1881,  1896,
    1897,  1900,  1902,  1905,  1906,  1909,  1910,  1911,  1912,  1915,
    1919,  1922,  1924,  1925,  1928,  1929,  1930,  1931,  1932,  1936,
    1937,  1938,  1941,  1944,  1948,  1949,  1950,  1951,  1952,  1955,
    1957,  1983,  1986,  1987,  1988,  1989,  1990,  1991,  1992,  1993,
    1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,  2002,  2003,
    2006,  2007,  2008,  2011,  2012,  2016,  2025,  2032,  2033,  2034,
    2035,  2036,  2039,  2049,  2052,  2055,  2056,  2059,  2060,  2066,
    2076,  2077,  2081,  2093,  2094,  2097,  2100,  2103,  2106,  2107,
    2110,  2121,  2122,  2125,  2126,  2129,  2143,  2144,  2147,  2153,
    2161,  2164,  2165,  2168,  2171,  2172,  2175,  2183,  2186,  2191,
    2200,  2203,  2212,  2213,  2216,  2217,  2220,  2221,  2224,  2230,
    2233,  2242,  2245,  2250,  2258,  2259,  2262,  2271,  2272,  2275,
    2276,  2279,  2285,  2286,  2294,  2302,  2312,  2315,  2318,  2319,
    2325,  2328,  2336,  2343,  2349,  2355,  2375,  2384,  2392,  2407,
    2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,  2477,
    2482,  2487,  2494,  2502,  2503,  2506,  2507,  2512,  2513,  2521,
    2533,  2543,  2552,  2557,  2571,  2572,  2573,  2576,  2577,  2580,
    2590,  2591,  2595,  2596,  2605,  2606,  2616,  2619,  2620,  2628,
    2638,  2639,  2642,  2645,  2648,  2651,  2658,  2659,  2660,  2661,
    2662,  2663,  2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,
    2674,  2686,  2687,  2688,  2689,  2690,  2691,  2692,  2693,  2700,
    2706,  2714,  2726,  2727,  2730,  2731,  2737,  2746,  2747,  2754,
    2764,  2765,  2772,  2786,  2793,  2803,  2808,  2814,  2847,  2860,
    2887,  2948,  2949,  2950,  2951,  2952,  2955,  2963,  2964,  2973,
    2979,  2988,  2995,  3000,  3006,  3015,  3025,  3028,  3032,  3045,
    3072,  3075,  3079,  3092,  3093,  3094,  3103,  3111,  3112,  3115,
    3122,  3132,  3133,  3136,  3144,  3145,  3153,  3154,  3157,  3164,
    3177,  3201,  3208,  3221,  3222,  3223,  3228,  3233,  3240,  3241,
    3249,  3260,  3270,  3271,  3274,  3284,  3294,  3306,  3318,  3328,
    3336,  3343,  3350,  3356,  3360,  3361,  3362,  3366,  3375,  3380,
    3385,  3392,  3399,  3408,  3418,  3426,  3427,  3435,  3440,  3458,
    3464,  3472,  3542,  3545,  3546,  3555,  3556,  3559,  3566,  3572,
    3573,  3574,  3575,  3576,  3579,  3585,  3591,  3598,  3605,  3611,
    3614,  3615,  3618,  3627,  3628,  3631,  3641,  3649,  3650,  3655,
    3660,  3665,  3670,  3677,  3685,  3693,  3701,  3708,  3715,  3721,
    3729,  3737,  3744,  3747,  3756,  3764,  3772,  3778,  3785,  3791,
    3794,  3802,  3810,  3811,  3814,  3815,  3822,  3856,  3857,  3858,
    3859,  3860,  3877,  3878,  3879,  3890,  3893,  3902,  3931,  3942,
    3964,  3965,  3968,  3976,  3984,  3994,  3995,  3998,  4009,  4019,
    4024,  4031,  4041,  4044,  4049,  4050,  4051,  4052,  4053,  4054,
    4057,  4062,  4069,  4079,  4080,  4088,  4092,  4095,  4098,  4111,
    4117,  4141,  4144,  4154,  4168,  4171,  4186,  4189,  4197,  4198,
    4199,  4207,  4208,  4209,  4217,  4220,  4228,  4231,  4240,  4243,
    4252,  4253,  4256,  4259,  4267,  4268,  4279,  4284,  4291,  4295,
    4300,  4308,  4316,  4324,  4332,  4342,  4345,  4348,  4351,  4354,
    4357
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "'('", "')'", "','", "':'", "';'", "'?'",
  "'['", "']'", "'{'", "'}'", "'.'", "'K'", "'M'", "'G'", "'T'", "'P'",
  "SQL_TOKEN_ACCESS_DATE", "SQL_TOKEN_REAL_NUM", "SQL_TOKEN_INTNUM",
  "SQL_TOKEN_APPROXNUM", "SQL_TOKEN_NOT", "SQL_TOKEN_NAME",
  "SQL_TOKEN_ARRAY_INDEX", "SQL_TOKEN_UMINUS", "SQL_TOKEN_ALL",
  "SQL_TOKEN_ALTER", "SQL_TOKEN_AMMSC", "SQL_TOKEN_ANY", "SQL_TOKEN_AS",
  "SQL_TOKEN_ASC", "SQL_TOKEN_AT", "SQL_TOKEN_AUTHORIZATION",
  "SQL_TOKEN_AVG", "SQL_TOKEN_BETWEEN", "SQL_TOKEN_BIT", "SQL_TOKEN_BOTH",
  "SQL_TOKEN_BY", "SQL_TOKEN_CAST", "SQL_TOKEN_CHARACTER",
  "SQL_TOKEN_CHECK", "SQL_TOKEN_COLLATE", "SQL_TOKEN_COMMIT",
  "SQL_TOKEN_CONTINUE", "SQL_TOKEN_CONVERT", "SQL_TOKEN_COUNT",
  "SQL_TOKEN_CREATE", "SQL_TOKEN_CROSS", "SQL_TOKEN_CURRENT",
  "SQL_TOKEN_CURSOR", "SQL_TOKEN_DATEVALUE", "SQL_TOKEN_DAY",
  "SQL_TOKEN_DEC", "SQL_TOKEN_DECIMAL", "SQL_TOKEN_DECLARE",
  "SQL_TOKEN_DEFAULT", "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC",
  "SQL_TOKEN_DISTINCT", "SQL_TOKEN_DROP", "SQL_TOKEN_FORWARD",
  "SQL_TOKEN_REVERSE", "SQL_TOKEN_ESCAPE", "SQL_TOKEN_EXCEPT",
  "SQL_TOKEN_EXISTS", "SQL_TOKEN_FALSE", "SQL_TOKEN_FETCH",
  "SQL_TOKEN_FLOAT", "SQL_TOKEN_FOR", "SQL_TOKEN_FOREIGN",
  "SQL_TOKEN_FOUND", "SQL_TOKEN_FROM", "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP",
  "SQL_TOKEN_HAVING", "SQL_TOKEN_IN", "SQL_TOKEN_INDICATOR",
  "SQL_TOKEN_INNER", "SQL_TOKEN_INTO", "SQL_TOKEN_IS",
  "SQL_TOKEN_INTERSECT", "SQL_TOKEN_JOIN", "SQL_TOKEN_KEY",
  "SQL_TOKEN_LEADING", "SQL_TOKEN_LIKE", "SQL_TOKEN_LOCAL",
  "SQL_TOKEN_LOWER", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN", "SQL_TOKEN_NATURAL",
  "SQL_TOKEN_NCHAR", "SQL_TOKEN_NULL", "SQL_TOKEN_NUMERIC",
  "SQL_TOKEN_OCTET_LENGTH", "SQL_TOKEN_OF", "SQL_TOKEN_ON",
  "SQL_TOKEN_OPTION", "SQL_TOKEN_ORDER", "SQL_TOKEN_OUTER",
  "SQL_TOKEN_PRECISION", "SQL_TOKEN_PRIMARY", "SQL_TOKEN_PROCEDURE",
  "SQL_TOKEN_PUBLIC", "SQL_TOKEN_REAL", "SQL_TOKEN_REFERENCES",
  "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_SCHEMA", "SQL_TOKEN_SELECT",
  "SQL_TOKEN_SET", "SQL_TOKEN_SMALLINT", "SQL_TOKEN_SOME",
  "SQL_TOKEN_SQLCODE", "SQL_TOKEN_SQLERROR", "SQL_TOKEN_SUM",
  "SQL_TOKEN_TABLE", "SQL_TOKEN_TO", "SQL_TOKEN_TRAILING",
  "SQL_TOKEN_TRANSLATE", "SQL_TOKEN_TRIM", "SQL_TOKEN_TRUE",
  "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN",
  "SQL_TOKEN_UPDATE", "SQL_TOKEN_UPPER", "SQL_TOKEN_USAGE",
  "SQL_TOKEN_USING", "SQL_TOKEN_VALUES", "SQL_TOKEN_VIEW",
  "SQL_TOKEN_WHERE", "SQL_TOKEN_WITH", "SQL_TOKEN_WORK", "SQL_TOKEN_ASCII",
  "SQL_TOKEN_BIT_LENGTH", "SQL_TOKEN_CHAR", "SQL_TOKEN_CHAR_LENGTH",
  "SQL_TOKEN_SQL_TOKEN_INTNUM", "SQL_TOKEN_CONCAT", "SQL_TOKEN_DIFFERENCE",
  "SQL_TOKEN_INSERT", "SQL_TOKEN_LEFT", "SQL_TOKEN_LENGTH",
  "SQL_TOKEN_LOCATE", "SQL_TOKEN_LOCATE_2", "SQL_TOKEN_LTRIM",
  "SQL_TOKEN_POSITION", "SQL_TOKEN_REPEAT", "SQL_TOKEN_REPLACE",
  "SQL_TOKEN_RIGHT", "SQL_TOKEN_RTRIM", "SQL_TOKEN_SOUNDEX",
  "SQL_TOKEN_SPACE", "SQL_TOKEN_SUBSTRING", "SQL_TOKEN_CURRENT_DATE",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_CURDATE", "SQL_TOKEN_NOW",
  "SQL_TOKEN_EXTRACT", "SQL_TOKEN_DAYNAME", "SQL_TOKEN_DAYOFMONTH",
  "SQL_TOKEN_DAYOFWEEK", "SQL_TOKEN_DAYOFYEAR", "SQL_TOKEN_HOUR",
  "SQL_TOKEN_MINUTE", "SQL_TOKEN_MONTH", "SQL_TOKEN_MONTHNAME",
  "SQL_TOKEN_QUARTER", "SQL_TOKEN_DATEDIFF", "SQL_TOKEN_SECOND",
  "SQL_TOKEN_TIMESTAMPADD", "SQL_TOKEN_TIMESTAMPDIFF",
  "SQL_TOKEN_TIMEVALUE", "SQL_TOKEN_WEEK", "SQL_TOKEN_YEAR",
  "SQL_TOKEN_ABS", "SQL_TOKEN_ACOS", "SQL_TOKEN_ASIN", "SQL_TOKEN_ATAN",
  "SQL_TOKEN_ATAN2", "SQL_TOKEN_CEILING", "SQL_TOKEN_COS", "SQL_TOKEN_COT",
  "SQL_TOKEN_DEGREES", "SQL_TOKEN_EXP", "SQL_TOKEN_FLOOR",
  "SQL_TOKEN_LOGF", "SQL_TOKEN_LOG", "SQL_TOKEN_LN", "SQL_TOKEN_LOG10",
  "SQL_TOKEN_MOD", "SQL_TOKEN_PI", "SQL_TOKEN_POWER", "SQL_TOKEN_RADIANS",
  "SQL_TOKEN_RAND", "SQL_TOKEN_ROUNDMAGIC", "SQL_TOKEN_ROUND",
  "SQL_TOKEN_SIGN", "SQL_TOKEN_SIN", "SQL_TOKEN_SQRT", "SQL_TOKEN_TAN",
  "SQL_TOKEN_TRUNCATE", "SQL_TOKEN_EVERY", "SQL_TOKEN_INTERSECTION",
  "SQL_TOKEN_FUSION", "SQL_TOKEN_COLLECT", "SQL_TOKEN_WITHIN",
  "SQL_TOKEN_ARRAY_AGG", "SQL_TOKEN_CASE", "SQL_TOKEN_THEN",
  "SQL_TOKEN_END", "SQL_TOKEN_NULLIF", "SQL_TOKEN_COALESCE",
  "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE", "SQL_TOKEN_BEFORE",
  "SQL_TOKEN_AFTER", "SQL_TOKEN_INSTEAD", "SQL_TOKEN_EACH",
  "SQL_TOKEN_REFERENCING", "SQL_TOKEN_BEGIN", "SQL_TOKEN_ATOMIC",
  "SQL_TOKEN_TRIGGER", "SQL_TOKEN_ROW", "SQL_TOKEN_STATEMENT",
  "SQL_TOKEN_NEW", "SQL_TOKEN_OLD", "SQL_TOKEN_VALUE",
  "SQL_TOKEN_CURRENT_CATALOG", "SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP",
  "SQL_TOKEN_CURRENT_PATH", "SQL_TOKEN_CURRENT_ROLE",
  "SQL_TOKEN_CURRENT_SCHEMA", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_VARBINARY",
  "SQL_TOKEN_VARYING", "SQL_TOKEN_OBJECT", "SQL_TOKEN_NCLOB",
  "SQL_TOKEN_NATIONAL", "SQL_TOKEN_LARGE", "SQL_TOKEN_CLOB",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_BIGI", "SQL_TOKEN_INTERVAL",
  "SQL_TOKEN_OVER", "SQL_TOKEN_ROW_NUMBER", "SQL_TOKEN_NTILE",
  "SQL_TOKEN_LEAD", "SQL_TOKEN_LAG", "SQL_TOKEN_RESPECT",
  "SQL_TOKEN_IGNORE", "SQL_TOKEN_NULLS", "SQL_TOKEN_FIRST_VALUE",
  "SQL_TOKEN_LAST_VALUE", "SQL_TOKEN_NTH_VALUE", "SQL_TOKEN_FIRST",
  "SQL_TOKEN_LAST", "SQL_TOKEN_EXCLUDE", "SQL_TOKEN_OTHERS",
  "SQL_TOKEN_TIES", "SQL_TOKEN_FOLLOWING", "SQL_TOKEN_UNBOUNDED",
  "SQL_TOKEN_PRECEDING", "SQL_TOKEN_RANGE", "SQL_TOKEN_ROWS",
  "SQL_TOKEN_PARTITION", "SQL_TOKEN_WINDOW", "SQL_TOKEN_NO",
  "SQL_TOKEN_GETECCLASSID", "SQL_TOKEN_LIMIT", "SQL_TOKEN_OFFSET",
  "SQL_TOKEN_NEXT", "SQL_TOKEN_ONLY", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BOOLEAN", "SQL_TOKEN_DOUBLE", "SQL_TOKEN_INTEGER",
  "SQL_TOKEN_INT", "SQL_TOKEN_INT32", "SQL_TOKEN_LONG", "SQL_TOKEN_INT64",
  "SQL_TOKEN_STRING", "SQL_TOKEN_DATE", "SQL_TOKEN_TIMESTAMP",
  "SQL_TOKEN_DATETIME", "SQL_TOKEN_POINT2D", "SQL_TOKEN_POINT3D",
  "SQL_TOKEN_OR", "SQL_TOKEN_AND", "SQL_EQUAL", "SQL_GREAT", "SQL_LESS",
  "SQL_NOTEQUAL", "SQL_GREATEQ", "SQL_LESSEQ", "'+'", "'-'", "SQL_CONCAT",
  "'*'", "'/'", "'='", "SQL_TOKEN_INVALIDSYMBOL", "$accept",
  "sql_single_statement", "sql", "schema_element", "base_table_def",
  "base_table_element_commalist", "base_table_element", "column_def",
  "column_def_opt_list", "nil_fkt", "unique_spec", "column_def_opt",
  "table_constraint_def", "column_commalist", "column_ref_commalist",
  "view_def", "opt_with_check_option", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc", "sql_not",
  "manipulative_statement", "union_statement", "commit_statement",
  "delete_statement_searched", "fetch_statement", "insert_statement",
  "values_or_query_spec", "table_value_const_list",
  "row_value_constructor", "row_value_constructor_elem",
  "rollback_statement", "select_statement_into", "opt_all_distinct",
  "assignment_commalist", "assignment", "update_source",
  "update_statement_searched", "target_commalist", "target",
  "opt_where_clause", "query_term", "select_statement", "selection",
  "opt_result_offset_clause", "result_offset_clause",
  "opt_fetch_first_row_count", "first_or_next", "row_or_rows",
  "opt_fetch_first_clause", "fetch_first_clause", "offset_row_count",
  "fetch_first_row_count", "opt_limit_offset_clause", "opt_offset",
  "limit_offset_clause", "table_exp", "from_clause", "table_ref_commalist",
  "opt_as", "opt_row", "table_primary_as_range_column", "opt_only",
  "table_ref", "where_clause", "opt_group_by_clause", "opt_having_clause",
  "truth_value", "boolean_primary",
  "parenthesized_boolean_value_expression", "boolean_test",
  "boolean_factor", "boolean_term", "search_condition", "predicate",
  "comparison_predicate_part_2", "comparison_predicate", "comparison",
  "between_predicate_part_2", "between_predicate",
  "character_like_predicate_part_2", "other_like_predicate_part_2",
  "like_predicate", "opt_escape", "null_predicate_part_2", "test_for_null",
  "in_predicate_value", "in_predicate_part_2", "in_predicate",
  "quantified_comparison_predicate_part_2", "all_or_any_predicate",
  "any_all_some", "existence_test", "unique_test", "subquery",
  "scalar_exp_commalist", "select_sublist", "parameter_ref", "literal",
  "as_clause", "position_exp", "num_value_fct", "char_length_exp",
  "octet_length_exp", "bit_length_exp", "length_exp", "datetime_field",
  "extract_field", "extract_exp", "unsigned_value_spec",
  "general_value_spec", "fct_spec", "function_name0", "function_name1",
  "function_name2", "function_name12", "function_name23", "function_name3",
  "function_name", "string_function_1Argument",
  "string_function_2Argument", "string_function_3Argument",
  "string_function_4Argument", "string_function",
  "date_function_0Argument", "numeric_function_0Argument",
  "numeric_function_1Argument", "numeric_function_2Argument",
  "numeric_function", "window_function", "window_function_type",
  "ntile_function", "dynamic_parameter_specification",
  "simple_value_specification", "number_of_tiles",
  "opt_lead_or_lag_function", "opt_null_treatment", "lead_or_lag_function",
  "lead_or_lag", "lead_or_lag_extent", "offset", "default_expression",
  "null_treatment", "first_or_last_value_function", "first_or_last_value",
  "opt_from_first_or_last", "nth_value_function", "nth_row",
  "from_first_or_last", "window_name", "window_name_or_specification",
  "in_line_window_specification", "opt_window_clause", "window_clause",
  "window_definition_list", "window_definition", "new_window_name",
  "window_specification", "opt_existing_window_name",
  "opt_window_partition_clause", "opt_window_frame_clause",
  "window_specification_details", "existing_window_name",
  "window_partition_clause", "window_partition_column_reference_list",
  "window_partition_column_reference", "opt_window_frame_exclusion",
  "window_frame_clause", "window_frame_units", "window_frame_extent",
  "window_frame_start", "window_frame_preceding", "window_frame_between",
  "window_frame_bound_1", "window_frame_bound_2", "window_frame_bound",
  "window_frame_following", "window_frame_exclusion", "general_set_fct",
  "set_fct_type", "outer_join_type", "join_condition", "join_spec",
  "join_type", "cross_union", "qualified_join", "relationship_join",
  "op_relationship_direction", "joined_table", "named_columns_join",
  "simple_table", "non_join_query_primary", "non_join_query_term",
  "query_primary", "non_join_query_exp", "all", "query_exp",
  "scalar_subquery", "cast_operand", "cast_target", "ec_data_type",
  "cast_spec", "value_exp_primary", "ecclassid_fct_spec", "num_primary",
  "factor", "term", "num_value_exp", "datetime_primary",
  "datetime_value_fct", "datetime_factor", "datetime_term",
  "datetime_value_exp", "non_second_datetime_field", "start_field",
  "end_field", "single_datetime_field", "interval_qualifier",
  "function_arg_commalist2", "function_arg_commalist3",
  "function_arg_commalist4", "value_exp_commalist", "function_arg",
  "function_args_commalist", "value_exp", "string_value_exp",
  "char_value_exp", "concatenation", "char_primary", "collate_clause",
  "char_factor", "string_value_fct", "bit_value_fct", "bit_substring_fct",
  "bit_value_exp", "bit_factor", "bit_primary", "char_value_fct",
  "for_length", "char_substring_fct", "upper_lower", "fold",
  "form_conversion", "char_translation", "trim_fct", "trim_operands",
  "trim_spec", "derived_column", "table_node", "catalog_name",
  "schema_name", "table_name", "opt_column_array_idx", "property_path",
  "property_path_entry", "column_ref", "data_type", "opt_char_set_spec",
  "opt_collate_clause", "predefined_type", "character_string_type",
  "opt_paren_precision", "paren_char_length",
  "opt_paren_char_large_length", "paren_character_large_object_length",
  "large_object_length", "opt_multiplier", "character_large_object_type",
  "national_character_string_type", "national_character_large_object_type",
  "binary_string_type", "binary_large_object_string_type", "numeric_type",
  "opt_paren_precision_scale", "exact_numeric_type",
  "approximate_numeric_type", "boolean_type", "datetime_type",
  "interval_type", "column", "case_expression", "case_abbreviation",
  "case_specification", "simple_case", "searched_case",
  "simple_when_clause_list", "simple_when_clause", "when_operand_list",
  "when_operand", "searched_when_clause_list", "searched_when_clause",
  "else_clause", "result", "result_expression", "case_operand", "cursor",
  "parameter", "range_variable", "trigger_definition", "op_referencing",
  "trigger_action_time", "trigger_event", "op_trigger_columnlist",
  "trigger_column_list", "triggered_action", "op_triggered_action_for",
  "trigger_for", "triggered_when_clause", "triggered_SQL_statement",
  "SQL_procedure_statement_list", "SQL_procedure_statement",
  "transition_table_or_variable_list", "transition_table_or_variable",
  "old_transition_table_name", "new_transition_table_name",
  "transition_table_name", "old_transition_variable_name",
  "new_transition_variable_name", "trigger_name", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,    40,    41,    44,    58,    59,    63,    91,
      93,   123,   125,    46,    75,    77,    71,    84,    80,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
     509,   510,   511,   512,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,    43,    45,   535,    42,
      47,    61,   536
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   303,   304,   304,   305,   305,   306,   306,   306,   307,
     308,   308,   309,   309,   310,   311,   311,   312,   313,   313,
     314,   314,   314,   314,   314,   314,   314,   314,   314,   315,
     315,   315,   315,   316,   316,   317,   317,   318,   319,   319,
     320,   320,   321,   321,   322,   322,   323,   323,   324,   324,
     325,   325,   325,   326,   326,   327,   327,   327,   327,   327,
     327,   327,   327,   328,   328,   329,   330,   331,   332,   333,
     334,   334,   335,   336,   337,   338,   339,   339,   339,   340,
     340,   341,   342,   342,   343,   344,   344,   345,   346,   346,
     347,   348,   349,   349,   350,   350,   351,   352,   352,   353,
     353,   354,   354,   355,   355,   356,   357,   358,   359,   359,
     360,   360,   361,   362,   363,   364,   364,   365,   365,   366,
     366,   367,   367,   368,   368,   369,   369,   369,   370,   371,
     371,   372,   372,   373,   373,   373,   373,   374,   374,   374,
     375,   376,   376,   377,   377,   378,   378,   379,   379,   380,
     380,   380,   380,   380,   380,   380,   380,   381,   382,   382,
     383,   383,   383,   383,   383,   383,   383,   384,   385,   386,
     387,   388,   388,   388,   388,   389,   389,   390,   391,   391,
     392,   392,   393,   394,   394,   395,   396,   397,   397,   397,
     398,   399,   400,   401,   401,   402,   403,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   405,   405,   405,   406,
     406,   407,   407,   407,   408,   408,   409,   410,   411,   411,
     411,   412,   412,   413,   413,   414,   415,   415,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   418,
     418,   419,   419,   420,   420,   421,   421,   421,   421,   422,
     423,   424,   424,   424,   425,   425,   425,   425,   425,   426,
     426,   426,   427,   428,   429,   429,   429,   429,   429,   430,
     430,   431,   432,   432,   432,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     433,   433,   433,   434,   434,   435,   436,   436,   436,   436,
     436,   436,   437,   438,   439,   440,   440,   441,   441,   441,
     442,   442,   443,   444,   444,   445,   446,   447,   448,   448,
     449,   450,   450,   451,   451,   452,   453,   453,   454,   454,
     455,   456,   456,   457,   458,   458,   459,   460,   460,   461,
     462,   463,   464,   464,   465,   465,   466,   466,   467,   468,
     469,   470,   470,   471,   472,   472,   473,   474,   474,   475,
     475,   476,   476,   476,   477,   478,   479,   480,   481,   481,
     481,   482,   483,   483,   483,   483,   484,   484,   484,   485,
     485,   485,   485,   485,   485,   485,   485,   485,   485,   486,
     486,   486,   487,   488,   488,   489,   489,   489,   489,   490,
     491,   491,   491,   492,   493,   493,   493,   494,   494,   495,
     496,   496,   497,   497,   498,   498,   499,   500,   500,   500,
     501,   501,   502,   503,   504,   505,   506,   506,   506,   506,
     506,   506,   506,   506,   506,   506,   506,   506,   506,   506,
     507,   508,   508,   508,   508,   508,   508,   508,   508,   508,
     509,   509,   510,   510,   511,   511,   511,   512,   512,   512,
     513,   513,   513,   514,   515,   515,   515,   515,   516,   517,
     518,   519,   519,   519,   519,   519,   520,   521,   521,   522,
     522,   523,   523,   524,   525,   526,   527,   527,   527,   528,
     529,   529,   529,   530,   530,   530,   531,   532,   532,   533,
     533,   534,   534,   535,   536,   536,   537,   537,   538,   539,
     540,   541,   542,   543,   543,   543,   543,   543,   544,   544,
     545,   545,   546,   546,   547,   548,   548,   549,   550,   551,
     551,   551,   551,   551,   552,   552,   552,   553,   554,   554,
     554,   555,   555,   556,   557,   558,   558,   559,   559,   560,
     560,   561,   562,   563,   563,   564,   564,   565,   565,   565,
     565,   565,   565,   565,   566,   566,   566,   566,   566,   566,
     567,   567,   568,   569,   569,   570,   571,   572,   572,   572,
     572,   572,   572,   573,   573,   573,   574,   574,   574,   574,
     574,   574,   574,   575,   575,   575,   576,   576,   576,   576,
     577,   577,   578,   578,   579,   579,   579,   580,   580,   580,
     580,   580,   581,   581,   581,   582,   583,   583,   584,   585,
     586,   586,   587,   587,   587,   588,   588,   589,   590,   591,
     591,   592,   593,   593,   594,   594,   594,   594,   594,   594,
     595,   595,   596,   597,   597,   598,   599,   600,   601,   602,
     602,   603,   603,   305,   305,   604,   605,   605,   606,   606,
     606,   607,   607,   607,   608,   608,   609,   610,   611,   611,
     612,   612,   613,   613,   614,   614,   615,   615,   616,   617,
     617,   618,   618,   618,   618,   619,   620,   621,   622,   623,
     624
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     6,
       1,     3,     1,     1,     3,     0,     2,     1,     1,     2,
       2,     1,     2,     2,     2,     1,     4,     2,     5,     4,
       7,    10,     4,     3,     1,     3,     1,     7,     0,     3,
       0,     3,     0,     3,     0,     3,     1,     3,     2,     2,
       0,     1,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     2,     4,     4,     5,     4,
       1,     3,     1,     1,     2,     6,     0,     1,     1,     1,
       3,     3,     1,     1,     5,     1,     3,     1,     0,     1,
       1,     4,     1,     1,     0,     1,     3,     0,     1,     1,
       1,     1,     1,     0,     1,     5,     1,     1,     0,     1,
       0,     2,     3,     9,     2,     1,     3,     0,     1,     0,
       1,     0,     3,     0,     1,     3,     4,     1,     2,     0,
       3,     0,     2,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     4,     1,     2,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     2,
       1,     1,     1,     1,     1,     1,     2,     5,     2,     4,
       4,     2,     2,     1,     1,     0,     2,     3,     2,     1,
       1,     3,     3,     2,     1,     3,     2,     1,     1,     1,
       2,     2,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     0,     2,     1,     6,
       4,     1,     1,     1,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     6,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     4,     4,     4,     4,     4,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     0,     2,     4,
       0,     1,     6,     1,     1,     1,     1,     1,     2,     2,
       5,     1,     1,     0,     1,     8,     1,     1,     2,     2,
       1,     1,     1,     1,     0,     1,     2,     3,     1,     3,
       1,     3,     0,     1,     0,     1,     0,     1,     4,     1,
       3,     3,     1,     2,     0,     1,     3,     1,     1,     1,
       1,     2,     1,     2,     2,     4,     1,     1,     1,     2,
       1,     2,     3,     2,     2,     3,     5,     4,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     0,     1,     1,     2,     4,
       5,     5,     1,     7,     0,     1,     1,     1,     1,     4,
       1,     1,     1,     3,     1,     4,     1,     1,     4,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       6,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     5,     1,     1,     1,     2,     2,     1,     3,     3,
       1,     3,     3,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     2,     2,
       2,     3,     1,     3,     5,     7,     1,     3,     3,     1,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     2,     1,     2,     1,     1,     1,     7,
       1,     1,     0,     1,     1,     1,     1,     1,     0,     2,
       7,     4,     1,     1,     4,     6,     6,     6,     4,     4,
       3,     3,     2,     1,     1,     1,     1,     2,     1,     1,
       1,     3,     3,     3,     1,     0,     1,     1,     3,     2,
       1,     1,     1,     0,     3,     0,     1,     3,     2,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     2,     1,
       0,     1,     3,     0,     1,     3,     2,     0,     1,     1,
       1,     1,     1,     4,     4,     2,     3,     3,     2,     4,
       4,     3,     1,     5,     4,     2,     2,     3,     2,     1,
       4,     2,     1,     1,     0,     3,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     1,
       1,     1,     4,     4,     4,     1,     1,     5,     4,     1,
       2,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     4,     0,     2,     1,     1,     1,     1,     2,
       1,     0,     2,     1,     3,     9,     0,     2,     1,     1,
       2,     1,     1,     2,     0,     2,     1,     3,     0,     3,
       1,     1,     0,     2,     1,     5,     1,     3,     1,     1,
       2,     4,     4,     4,     4,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      53,    53,     0,   660,   201,   198,   199,   200,    53,   555,
     394,   389,     0,     0,     0,     0,     0,     0,     0,   230,
       0,    53,   533,   390,   391,   229,     0,     0,    76,   395,
     392,     0,     0,   231,     0,   123,   532,   265,     0,   274,
       0,     0,   275,   276,   273,   270,   264,   259,   277,   266,
       0,   269,   272,   271,   267,   278,   268,     0,   474,   475,
     279,   280,     0,   256,   282,   283,   284,   285,   300,   286,
     287,   288,   289,   295,   290,   257,   258,   297,   296,   301,
     281,   302,   298,   303,   299,   255,   291,   292,   293,   294,
     304,   393,   398,   397,   396,     0,     0,     0,   232,   233,
     234,   235,   236,   237,     0,     0,   323,   324,   331,   332,
       0,     0,   197,   511,     0,     0,   162,   163,   160,   161,
     165,   164,     0,     0,   560,     0,     2,     5,     6,     7,
       0,     4,    62,    55,    56,    57,    58,    53,   139,    59,
      60,    61,    63,   141,   143,   145,   147,   663,   137,   149,
       0,   150,   173,   174,   156,   179,   154,   184,   155,   151,
     152,   153,   433,   227,   211,   463,   218,   219,   220,   213,
     212,   451,   226,   452,     0,     0,     0,     0,     0,     0,
       0,   251,   253,   260,     0,   261,   249,   250,   252,   254,
     262,   457,     0,   308,   309,     0,   310,     0,   311,   238,
       0,   455,   459,   462,   453,   464,   467,   470,   503,   478,
     473,   479,   480,   505,    73,   504,   506,   508,   514,   507,
     512,   517,   518,   516,   523,     0,   239,   525,   526,   527,
     561,   557,   454,   456,   630,   631,   635,   636,   228,     8,
      53,     0,     0,   421,     0,    63,     0,   422,   424,   427,
     432,     0,    73,   659,    53,    54,   273,   144,   556,   559,
       0,    65,     0,    76,     0,     0,     0,   123,     0,   190,
     658,     0,   166,     0,    74,    77,    78,     0,     0,     0,
     191,   124,     0,   405,   412,   418,   417,   127,     0,     0,
       0,     0,     0,   522,     0,     0,    53,   657,   653,   650,
       0,     0,     0,     0,     0,     0,     0,     0,   476,   477,
     466,   239,   465,     1,     3,     0,     0,   430,     0,     0,
     168,   171,   172,   178,   183,   186,    53,    53,    53,   159,
      72,   204,   205,   203,   202,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,     0,     0,     0,
       0,     0,     0,     0,   515,     0,     0,   432,     0,   664,
     430,   138,   192,   430,   430,   458,    53,    76,   420,     0,
       0,   434,     0,   504,     0,     0,   554,     0,   550,   549,
     548,    42,   700,     0,    88,     0,     0,   177,     0,   560,
       0,    93,   193,   206,   195,   504,   544,     0,   545,   546,
     543,     0,     0,   661,   121,     0,   401,   406,   405,     0,
     399,   400,   407,     0,     0,     0,     0,    42,     0,   496,
       0,   496,     0,   520,   521,   483,   484,   485,   482,   222,
     481,   223,     0,   221,   224,     0,     0,     0,     0,   651,
       0,    53,   653,   639,     0,     0,   496,     0,   496,   306,
     314,   316,   315,     0,   313,     0,   460,     0,   182,   180,
     175,   175,   431,     0,     0,   188,   187,   189,   158,     0,
       0,   146,   148,   241,     0,   656,   499,   655,     0,     0,
     500,     0,     0,     0,     0,   240,     0,     0,     0,   352,
     340,   341,   305,   342,   343,   317,   325,     0,     0,   468,
     469,   471,   472,   510,   509,   524,   513,     0,   555,     0,
     558,   423,     0,    70,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,     0,   668,   669,
       0,     0,    53,    66,    89,     0,    67,    85,    87,   196,
     216,   123,     0,    91,    88,     0,   629,     0,   547,   208,
       0,   542,     0,   538,     0,     0,   118,     0,    42,     0,
     125,   123,     0,    88,    79,   561,     0,   408,   123,   217,
     214,   215,     0,   210,     0,     0,     0,   531,     0,     0,
       0,     0,   654,   638,    72,   645,     0,   646,   173,   179,
     184,     0,   642,     0,   640,   632,   634,   633,   312,     0,
       0,     0,   170,   169,    64,     0,   185,   134,   136,   133,
     135,   142,   242,   243,     0,   247,     0,     0,   248,   244,
       0,   246,   245,     0,   359,   354,     0,   353,     0,     0,
     320,     0,   534,     0,    69,     0,   426,   425,   429,    90,
     428,     0,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   446,   447,   445,   448,   449,     0,   435,     0,     0,
     388,     0,   552,   554,   551,   553,     0,     0,     0,    18,
       0,    10,    12,     0,    13,     0,     0,    36,     0,   670,
     672,   674,   671,     0,   128,     0,   114,   115,     0,   129,
     194,   207,     0,   541,   540,     0,   662,   126,    40,   409,
     123,     0,    84,     0,     0,   405,    68,   497,   498,     0,
     528,   504,     0,   652,   159,    53,     0,   637,   337,   336,
       0,   181,   176,     0,   493,   501,   502,     0,     0,     0,
      44,   355,   351,   326,   318,   320,     0,     0,   330,   321,
     386,   461,    71,   450,   536,   535,     0,    53,     0,    19,
       9,     0,     0,   580,   622,   580,   623,   580,     0,     0,
     583,     0,   583,   583,     0,   580,   625,   624,   617,   618,
     620,   619,   621,   626,   580,    15,   562,   563,   579,   565,
     602,   569,   609,   570,   612,   613,   571,   572,   573,    43,
       0,    38,     0,   673,     0,    86,   123,    75,     0,   131,
     537,   539,     0,   122,   410,    80,    83,    81,    82,    53,
       0,   403,   411,   404,   209,     0,     0,     0,   225,     0,
     644,     0,   648,   649,   647,   643,   641,   333,   167,     0,
       0,     0,     0,   356,     0,   322,   328,   329,   554,     0,
       0,    11,     0,    34,     0,     0,     0,   574,   581,     0,
       0,   598,     0,     0,   575,   578,   608,     0,   605,   584,
     580,   580,   595,   611,   614,   580,     0,   492,   628,     0,
       0,   606,   627,    14,     0,   565,   566,   568,    35,     0,
      37,   676,   675,   666,   116,     0,    53,   344,     0,   402,
       0,   414,   529,   530,   519,     0,   157,     0,   320,   334,
     494,     0,   360,   362,   565,    53,   368,   367,   358,   357,
       0,   319,   327,    32,     0,    29,     0,     0,   576,   583,
     601,   583,   577,   583,   587,     0,     0,     0,   596,     0,
     597,     0,   490,   489,     0,   607,   583,     0,    25,     0,
       0,    21,    16,     0,   567,     0,     0,   678,   130,   132,
       0,    44,   345,    41,     0,   415,   416,   413,   338,   339,
     335,     0,     0,   363,    45,    46,    50,    50,     0,     0,
       0,     0,   364,   369,   372,   370,     0,    33,   582,   593,
     604,   594,   588,   589,   590,   591,   592,   586,   585,   599,
     583,   600,     0,   580,   487,   491,   610,    20,    53,    23,
      24,    22,    17,    27,   564,    39,   119,   119,   667,   689,
       0,   665,   682,   350,   346,   348,     0,   108,   419,   495,
     361,    53,    51,    52,    49,    48,     0,     0,   378,     0,
     376,   380,   373,   371,   374,     0,   366,   365,     0,   603,
     615,     0,   488,     0,     0,   117,   120,   117,   117,   117,
     690,     0,     0,    53,     0,     0,     0,    94,   109,    47,
     379,   381,     0,     0,   383,   384,     0,    30,     0,    26,
       0,     0,     0,     0,     0,   680,   681,   679,    53,   683,
       0,   688,   677,   684,   347,   349,   110,     0,   103,    95,
     375,   377,   382,   385,     0,   616,    28,   697,   694,   696,
     699,   692,   693,   695,   698,   691,     0,    53,     0,   112,
       0,   106,     0,   113,   104,     0,   140,     0,   686,   111,
     101,   102,    96,    99,   100,    97,    31,    53,     0,    98,
     107,   685,   687,     0,   105
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   125,  1081,   127,   128,   670,   671,   672,   873,  1000,
     673,   942,   674,   842,   676,   129,   880,   803,   527,   833,
     964,   965,  1024,   130,   131,   132,   133,   134,   135,   136,
     243,   512,   137,   138,   139,   140,   277,   563,   564,   807,
     141,   536,   537,   533,   244,   368,   390,  1088,  1089,  1128,
    1125,  1122,  1113,  1114,  1110,  1129,  1057,  1109,  1058,   543,
     544,   686,   557,  1047,   560,   282,   283,   534,   799,   887,
     611,   143,  1079,   144,   145,   146,   147,   148,   585,   149,
     150,   587,   151,   152,   153,   154,   602,   155,   156,   458,
     157,   158,   325,   159,   469,   160,   161,   162,   391,   392,
     538,   163,   548,   164,   165,   166,   167,   168,   169,   431,
     432,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   451,   452,   453,   629,   738,
     194,   195,   495,   734,   911,   739,   196,   197,   898,   198,
     720,   899,  1013,   492,   493,   951,   952,  1014,  1015,  1016,
     494,   625,   730,   908,   626,   627,   731,   902,   903,  1036,
     909,   910,   972,  1028,   974,   975,  1029,  1090,  1030,  1031,
    1037,   199,   200,   412,   811,   812,   413,   284,   285,   286,
     957,   287,   813,   247,   248,   249,   637,   250,   463,   251,
     201,   370,   656,   657,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   433,   866,   995,   867,
     868,   478,   483,   487,   418,   480,   481,   214,   215,   216,
     217,   218,   876,   219,   220,   221,   222,   422,   423,   424,
     223,   816,   224,   225,   226,   227,   228,   229,   401,   402,
     394,   377,   378,   379,   380,   259,   230,   231,   232,   775,
     875,   877,   776,   777,   847,   848,   858,   859,   925,   987,
     778,   779,   780,   781,   782,   783,   932,   784,   785,   786,
     787,   788,   843,   233,   234,   235,   236,   237,   442,   443,
     591,   592,   298,   299,   440,   476,   477,   300,   271,   238,
     558,   239,   947,   531,   683,   793,   882,  1011,  1012,  1077,
    1053,  1082,  1117,  1083,  1008,  1009,  1102,  1098,  1099,  1105,
    1101,   383
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -943
static const yytype_int16 yypact[] =
{
    1879,   994,    99,  -943,  -943,  -943,  -943,  -943,  2469,   112,
    -943,  -943,   184,    66,   223,   241,     2,   193,   246,  -943,
     250,   269,  -943,  -943,  -943,  -943,   331,   173,    89,  -943,
    -943,   360,   366,  -943,   246,   104,  -943,  -943,   386,  -943,
     398,   406,  -943,  -943,   333,  -943,  -943,  -943,  -943,  -943,
     447,  -943,  -943,  -943,  -943,  -943,  -943,   465,  -943,  -943,
    -943,  -943,   481,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  4414,   490,   497,  -943,  -943,
    -943,  -943,  -943,  -943,   501,   532,  -943,  -943,  -943,  -943,
     548,   584,  -943,  -943,  5702,  5702,  -943,  -943,  -943,  -943,
    -943,  -943,  6449,  6449,  -943,   541,   591,  -943,  -943,  -943,
     182,  -943,   482,  -943,  -943,  -943,  -943,   196,   124,  -943,
    -943,  -943,  -943,   525,  -943,  -943,   330,   343,  -943,  -943,
    5702,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,    97,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,   632,   644,   657,   674,   679,   684,
     689,  -943,  -943,  -943,   703,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,   376,  -943,  -943,   708,  -943,   709,  -943,   471,
     714,  -943,  -943,  -943,  -943,  -943,  -943,  -164,   200,  -943,
    -943,  -943,  -943,  -943,   420,  -943,   423,  -943,   677,  -943,
    -943,  -943,  -943,  -943,  -943,   718,   680,  -943,  -943,  -943,
     711,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
     994,   719,   721,  -943,   645,    57,    21,  -943,  -943,   646,
    -943,    78,    30,  -943,  2174,  -943,  -943,  -943,  -943,  -943,
    5702,  -943,  5702,     8,   705,   705,   706,   104,    63,  -943,
    -943,   652,   638,  5702,  -943,  -943,  -943,  5957,  5702,  3904,
    -943,  -943,   190,    85,  -943,  -943,  -943,  -943,  5702,  5702,
    5702,   705,  5702,  5702,  3649,  4159,  2764,  -943,   259,  -943,
     519,  5702,  5702,   730,    86,  5702,   731,   420,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,   733,  5702,   710,   105,  4682,
    -943,  -943,  -943,  -943,  -943,  -943,   269,  2764,  2764,  -943,
    -943,  -943,  -943,  -943,  -943,   735,  5702,  5702,  5702,  5702,
    5702,  4937,  5702,   199,  5702,  5702,    89,  6212,  6212,  6212,
    6212,  5702,   356,   705,  -943,  5702,    -4,   736,  5702,  -943,
     710,  -943,  -943,   710,   710,  -943,  2174,    89,  -943,    21,
     712,   420,   739,   613,   741,  5702,   380,   743,  -943,  -943,
    -943,   744,  -943,   308,   487,    63,   285,  -943,    37,   367,
     379,   746,  -943,    64,  -943,   621,  -943,  5702,  -943,  -943,
     -40,   762,  5192,   429,   429,   686,  -943,  -943,   486,    -5,
    -943,  -943,   667,   687,    39,    40,    42,   744,   474,   -22,
     502,   -17,   695,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,   698,  -943,   420,  4159,    30,  -123,  5702,  -943,
     561,  2764,   558,  -943,   519,   516,   420,   527,    43,  -943,
      97,  -943,  -943,   770,  -943,    24,  -943,  4159,  -943,  -943,
     132,   -37,  -943,   666,  5702,  -943,  -943,  -943,  -943,   246,
      60,  -943,   330,  -943,   772,   420,  -943,  -943,   773,   774,
    -943,   538,   598,   776,   777,  -943,   603,   779,   780,   757,
    -943,  -943,  -943,  -943,  -943,   781,   420,    45,  5702,  -943,
    -943,  -164,  -164,  -943,  -943,  -943,  -943,    46,   759,   784,
    -943,  -943,   542,  -943,    63,    63,    63,  5957,   478,   478,
     705,  -943,   786,   767,   768,   477,    -5,   763,  -943,  -943,
     697,    32,  2764,  -943,  -943,    56,   790,  -943,  -943,  -943,
    -943,   104,   285,  -943,   669,  5702,  -943,   775,  -943,  -943,
     705,   420,  5702,  -943,  5702,   -16,  -943,   778,   744,   782,
    -943,   104,   713,    72,  -943,   785,   507,  -943,   104,  -943,
    -943,  -943,   672,  -943,  5702,  5702,  5702,  -943,  5702,  5702,
    5702,  5702,  -943,  -943,    68,  -943,  5702,  -943,   798,   799,
     800,    59,  -943,   596,  -943,  -943,  -943,  -943,  -943,    86,
     618,  5702,  -943,  -943,  -943,   520,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  5702,  -943,  5702,  5702,  -943,  -943,
    5702,  -943,  -943,  5702,  -943,   545,   804,  -943,   791,   811,
     303,   812,  -943,   815,  -943,  5702,  -943,  -943,   645,  -943,
     645,   751,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,   822,  -943,   824,   825,
    -943,   817,  -943,   817,  -943,  -943,   828,   748,   749,  -943,
     553,  -943,  -943,   831,  -943,   579,   563,  -943,   666,  -943,
    -943,   740,  -943,   738,   343,   285,   832,   600,    75,   766,
    -943,  -943,   834,   420,   420,  5702,  -943,  -943,   839,   600,
     104,    -5,  -943,    -5,  5447,   560,  -943,   420,   420,    47,
     -25,    92,    55,  -943,    74,  3354,  5702,  -943,  -943,  -943,
     841,  -943,   549,  5702,  -943,  -943,  -943,   838,   843,   807,
     750,  -943,  -943,  -943,   845,   303,   599,   609,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,   840,  2764,   849,  -943,
    -943,   477,   775,    27,  -943,   142,  -943,   147,   862,   862,
     863,    44,   863,   863,   347,   148,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,   862,  -943,  -943,   826,  -943,   677,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
      -5,   737,   775,  -943,   840,  -943,   104,  -943,   829,   794,
    -943,   420,   775,  -943,   600,  -943,  -943,  -943,   420,  2764,
     208,  -943,  -943,  -943,  -943,  5702,   867,   868,  -943,   372,
    -943,  5702,  -943,  -943,  -943,  -943,  -943,   801,  -943,  5702,
    5702,    -5,   836,   312,  5702,  -943,  -943,  -943,  -943,    22,
     775,  -943,   577,  -943,   852,   862,   639,  -943,  -943,   862,
     640,  -943,   862,   641,  -943,  -943,  -943,   858,  -943,  -943,
     155,    50,  -943,  -943,   877,   862,   764,  -943,  -943,   862,
     647,  -943,  -943,   457,   783,   677,  -943,  -943,  -943,   844,
    -943,   878,  -943,   662,   600,    -5,  2764,   622,   592,   343,
     775,   580,   420,  -943,  -943,  5702,  -943,   388,   303,  -943,
    -943,   880,   883,  -943,   677,  3059,  -943,  -943,  -943,  -943,
     435,  -943,   420,  -943,   651,  -943,   775,   886,  -943,   863,
    -943,   863,  -943,   863,   557,   887,   862,   655,  -943,   862,
    -943,   873,  -943,   787,   374,  -943,   863,   802,   893,    90,
     705,  -943,  -943,   874,  -943,   803,   432,   827,   894,   343,
     757,   750,  -943,  -943,   665,  -943,  -943,  -943,  -943,  -943,
    -943,  5702,    -5,  -943,   895,  -943,   131,    81,   384,   678,
     643,   648,   649,  -943,  -943,  -943,   797,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
     863,  -943,   668,   862,  -943,  -943,  -943,  -943,  2764,  -943,
    -943,    97,  -943,   905,  -943,  -943,   -38,   -35,   432,  -943,
     690,  -943,   696,  -943,   907,  -943,   882,   650,  -943,  -943,
    -943,  3059,  -943,  -943,  -943,  -943,   176,   201,  -943,   625,
    -943,  -943,  -943,  -943,  -943,    11,  -943,  -943,   705,  -943,
    -943,   896,  -943,    48,   775,   884,  -943,   884,   884,   884,
    -943,   451,   913,  1289,   757,   915,  6212,   653,  -943,  -943,
    -943,  -943,   384,   699,  -943,  -943,   660,   918,   921,  -943,
     681,   898,   902,   898,   903,  -943,  -943,  -943,  2764,  -943,
     707,  -943,  -943,  -943,  -943,  -943,   139,    82,   860,  -943,
    -943,  -943,  -943,  -943,   775,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,    54,  1879,  6212,  -943,
     -49,    97,   -87,  -943,  -943,   685,  -943,   923,  -943,   200,
    -943,  -943,  -943,  -943,  -943,    82,  -943,  1584,   -49,  -943,
      97,  -943,  -943,   658,  -943
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -943,  -943,   130,  -943,  -943,  -943,   181,  -943,  -943,  -943,
      61,  -943,  -943,  -764,    51,  -943,  -943,  -943,  -345,   -18,
    -943,   -86,   -28,    -8,  -943,  -943,  -943,  -943,  -943,  -943,
     368,  -943,  -140,   -83,  -943,  -943,  -125,  -943,   240,  -943,
    -943,   400,   258,  -301,   178,     0,   427,  -943,  -943,  -943,
    -943,  -183,  -943,  -943,  -943,  -943,  -943,  -943,  -943,   260,
    -943,  -943,  -395,   -61,  -943,  -943,  -202,  -943,  -943,  -943,
    -943,  -943,  -943,   939,   623,   624,     1,  -806,  -943,  -943,
    -122,   814,  -943,  -120,   816,  -943,   488,  -116,  -943,  -943,
    -114,  -943,  -943,  -943,  -943,  -943,  -943,    14,  -943,   409,
    -943,  -298,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -840,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,   357,   358,  -943,  -943,  -664,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -283,  -943,  -943,  -943,  -943,  -943,   -99,  -943,
     -97,  -943,  -943,  -943,  -943,  -943,  -943,  -943,    -3,  -943,
    -943,  -943,  -943,    52,  -943,  -943,  -943,  -943,  -102,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,   555,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,   450,   180,  -943,  -178,   106,   581,
    -943,   715,   442,  -943,  -943,   654,  -943,   576,   353,   354,
    -942,  -943,    26,  -943,  -943,  -943,  -695,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -204,  -300,   177,    10,  -107,  -943,
    -943,  -943,   754,   615,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,   257,  -943,  -943,  -119,  -943,  -943,  -943,  -943,  -943,
    -943,  -260,  -943,   185,  -461,  -943,  -393,  -332,  -370,  -943,
    -943,  -784,  -943,  -943,  -613,  -675,  -525,  -943,  -943,  -943,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -379,  -943,  -943,  -943,  -943,  -943,  -943,   529,
    -943,   254,   675,  -224,   534,  -384,  -943,  -943,  -943,  -286,
    -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,  -943,
    -943,  -943,  -943,  -903,  -943,   -34,  -943,  -943,   -95,  -943,
    -943,  -943
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -650
static const yytype_int16 yytable[] =
{
     142,   245,   246,   311,   311,   381,   450,   308,   309,   559,
     329,   252,   297,   272,   549,   319,   565,   321,   454,   508,
     508,   323,   404,   324,   510,   361,   913,   601,   881,   599,
     844,   417,   269,   552,   365,   275,   474,   479,   888,   566,
     484,   540,   488,   569,   570,   815,   571,   597,   280,   630,
     632,   814,  1069,   844,   582,   576,   578,   695,  1116,   818,
     491,  1063,   357,   665,   715,   384,   385,   330,   276,   865,
     971,   835,   572,  -644,   439,   522,   914,   701,  1045,  -157,
     685,  1048,   362,   855,   856,   860,  1064,   581,   546,   420,
     680,   944,     2,   506,     3,   547,  -528,   445,   447,   967,
     539,     4,     5,     6,     7,     4,     5,     6,     7,     4,
       5,     6,     7,  1022,  1086,  -263,   275,   331,   264,   332,
     963,   363,  -420,   253,   307,   307,   954,   607,  1027,   318,
     126,   242,   265,   565,   405,   347,   348,   258,   375,  -420,
    1023,   464,   851,   363,   854,   844,   675,   -72,   541,   276,
     844,   844,   871,   608,   -72,   373,   677,   681,   844,   406,
     -72,   872,   815,  1022,   407,   328,  1119,   -72,   691,  1123,
     918,   395,   367,   682,   920,  1120,   408,   922,   364,   468,
     861,   609,   315,   999,   610,  1124,  1046,   260,   357,  1046,
    1023,   316,   241,   268,   935,   409,   601,   713,   631,   261,
     364,   -72,   489,   532,  1118,   -72,   624,   357,   -72,   461,
     -72,   890,   -72,   697,   376,   967,  1121,   -72,   513,   255,
     439,   498,  1027,   490,  1132,   266,   262,   410,   311,   311,
     311,   311,   376,   505,   960,   411,   330,   862,   863,   994,
     245,   246,   517,   689,   263,    58,    59,   928,   930,   268,
     252,   989,   933,   600,   991,   369,   539,   357,   351,   315,
     659,  -504,   702,   845,   252,   509,   267,   846,   316,   716,
     371,  1065,   371,   351,   270,   330,   351,    21,  -139,  1066,
    1070,   351,   351,   388,  -157,   665,   929,   393,   307,   400,
     692,     2,   255,     3,   124,   124,   403,   437,   414,   415,
     416,   450,   419,   421,   434,   436,   274,   374,   565,   328,
     328,   446,   448,   454,   724,   455,   725,   726,   470,   586,
     727,   588,   351,   728,   605,   589,   307,   590,   351,   459,
    1115,   566,   826,   883,   273,   351,   328,   351,   351,   687,
     351,   351,   328,   351,   351,   351,   475,   475,   475,   475,
     475,   475,   475,   351,   496,   497,  -139,  -139,   584,   699,
     112,   503,   351,   278,   112,   507,   705,   369,   112,   279,
     242,   510,   675,   114,   115,   333,   252,   281,   849,   334,
    1042,   330,   850,   852,   869,   475,   523,   853,   870,   288,
       2,   926,     3,   524,   979,   927,   980,   565,   981,   539,
     425,   289,    14,     4,     5,     6,     7,   551,   464,   290,
    1108,   996,   555,   291,   -72,   -72,   -72,   -72,   -72,   -72,
     878,   -72,   -72,   -72,   -72,   -72,   -72,   425,  -462,  -462,
    -462,  -462,  -462,   318,   969,   349,   350,  1060,   565,  1033,
     -92,     2,   437,     3,    22,   436,   714,   -92,   475,   315,
     292,    19,   541,  -117,     4,     5,     6,     7,   895,   542,
     556,   904,  1061,   604,  1034,  1039,   514,   446,   293,   515,
     516,   968,   711,   296,   438,    31,    32,    25,   573,   574,
     937,   575,    36,   606,   294,   969,   116,   117,   118,   119,
     120,   121,   565,   301,   722,   742,   349,   350,   804,   938,
     302,   546,    19,   330,   303,    33,   577,   574,   475,   575,
      57,   426,   427,   428,   939,   677,   482,   864,   486,   666,
     595,   574,   430,   575,   528,   529,   530,   393,    25,   900,
     901,   596,   574,   684,   575,   304,   405,   977,   426,   427,
     428,   313,   615,   616,   993,   617,   634,   635,   667,   430,
     891,   305,   330,   736,   737,   393,    33,   750,   751,   668,
     406,   406,   693,   940,   694,   407,   407,   789,   790,   565,
    -405,   982,   983,   984,   985,   986,   906,   907,   408,   668,
     669,   915,   916,   828,   707,   708,   709,   306,   710,   307,
     712,   475,   904,   821,   884,   822,   953,   916,   314,   823,
     669,   824,   618,   616,   317,   617,   326,   621,   616,   405,
     617,   307,    98,    99,   100,   101,   102,   103,   532,   327,
     753,   343,   721,   574,   475,   575,   475,   475,   410,   410,
     475,   328,   820,   475,   406,   335,   411,   411,   113,   407,
     330,  1001,   955,   956,   958,   959,  1026,   336,   754,   405,
    1071,   408,  1072,  1073,  1074,   976,   916,   809,  1006,  1007,
     337,  1019,   112,    98,    99,   100,   101,   102,   103,  1018,
     916,   755,  1040,  1041,   406,  1075,  1076,   338,   791,   407,
    1003,   896,   339,  -405,   756,  1096,   916,   340,   810,  1126,
     916,   408,   341,   638,   640,   639,   639,   970,   310,   312,
     499,   500,   410,   501,   502,   801,   342,   819,   662,   664,
     411,   344,   345,   112,   808,   757,  -307,   346,   351,   352,
     353,   355,   358,  -524,   356,   359,   475,   360,   -90,   376,
     382,   387,   386,   441,   449,   456,   457,   462,   330,   473,
     511,   520,   410,   518,   519,   521,   525,   526,   839,   550,
     411,   545,   642,   643,   644,   645,   646,   647,   648,   649,
     650,   651,   652,   653,   654,   655,   553,   567,   579,   561,
     568,   580,   583,   438,   598,   367,   612,   613,  1067,   614,
     619,   490,   620,   622,   258,   623,   628,   633,   461,  1111,
     660,   661,   663,   679,   678,   685,   700,   704,   703,   546,
     532,   241,   696,  -648,  -649,  -647,   698,   717,   732,   723,
     889,   729,   733,   758,   759,   735,   740,   760,   761,   741,
     762,   763,   966,   764,   541,   892,   743,  1130,   744,   745,
     746,   747,   748,   749,   752,   794,   792,   796,   800,   475,
     475,   798,   802,   829,   912,   827,   831,  -504,   830,   832,
     834,   836,   840,   765,   766,   767,   768,   769,   770,   771,
     772,   837,   773,   774,   838,   844,   857,   874,   885,   879,
     886,   893,   894,   917,   897,   905,   919,   921,   923,   924,
     931,   934,   946,   916,   936,   961,   945,   949,   962,   950,
     978,   988,   990,   943,   992,   997,   998,  1010,  1004,   790,
    1021,  1005,  1032,  1038,  -486,   307,  1033,  1035,  1044,  1051,
    1052,  1034,  1054,  1055,  1062,   556,  1078,  1068,   489,  1093,
    1056,  1094,  1097,  1092,  1087,  1095,  1100,  1104,  1112,  1107,
    1127,  1134,   841,  1017,   941,  1059,   948,   311,   966,  1025,
     706,   805,   688,   795,   641,  1133,  1049,   257,   797,   603,
     471,   320,   472,   322,   690,  1084,   718,   719,  1085,  1020,
    1091,   658,   973,   562,   636,  1002,   535,   504,   817,   825,
     460,   475,   354,   594,  1050,   444,   593,   372,  1103,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   311,
       0,     0,     0,     0,     0,     0,     0,   240,     0,  1043,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,    13,     0,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,   142,     0,     0,     0,     0,     0,     0,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,  1106,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    27,     0,    28,     0,     0,    29,   142,     0,    30,
       0,     0,     0,    31,    32,    33,     0,    34,     0,    35,
      36,     0,     0,   241,     0,     0,     0,   142,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,     1,   124,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,    13,     0,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,     0,    28,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,    34,     0,    35,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
    1080,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   112,     0,     0,
       0,   113,   114,   115,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,     1,   124,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,    13,     0,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    27,     0,    28,     0,     0,    29,     0,     0,    30,
       0,     0,     0,    31,    32,    33,     0,    34,     0,    35,
      36,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,  1131,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,     1,   124,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,    13,     0,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,     0,    28,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,    34,     0,    35,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   112,     0,     0,
       0,   113,   114,   115,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   366,   124,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   367,     0,     0,    29,     0,     0,    30,
       0,     0,     0,    31,    32,    33,     0,    34,     0,     0,
      36,     0,     0,   241,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,   256,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   254,   124,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,   255,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,   -54,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,    34,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
     256,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   112,     0,     0,
       0,   113,   114,   115,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   254,   124,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,    30,
       0,     0,     0,    31,    32,    33,     0,    34,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,   256,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   295,   124,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,   255,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,    34,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
     256,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   112,     0,     0,
       0,   113,   114,   115,     0,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   295,   124,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,   255,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,    30,
       0,     0,     0,    31,    32,    33,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,   256,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   295,   124,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     0,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,   425,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
     256,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,   426,   427,   428,     0,     0,     0,   429,
       0,     0,     0,    63,   430,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,   295,     0,     0,
       2,     0,     3,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     4,     5,     6,     7,   112,     9,     0,
       0,   113,   114,   115,    10,     0,     0,     0,     0,    11,
       0,     0,   396,     0,    12,   122,   123,     0,   124,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,   397,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   398,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,    30,
       0,     0,   399,    31,    32,    33,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,   256,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,   435,     0,     0,     2,     0,     3,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     4,     5,
       6,     7,   112,     9,     0,     0,   113,   114,   115,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
     122,   123,     0,   124,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   367,     0,
       0,    29,     0,     0,    30,     0,     0,     0,    31,    32,
      33,     0,     0,     0,     0,    36,     0,     0,   241,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
     256,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,   295,     0,     0,
       2,     0,     3,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     4,     5,     6,     7,   112,     9,     0,
       0,   113,   114,   115,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,   122,   123,     0,   124,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,    26,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,    30,
       0,     0,     0,    31,    32,    33,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,   256,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,    95,     0,     0,    96,    97,   296,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,     0,     0,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,     0,   295,     0,     0,     2,     0,
       3,     0,   112,     0,     0,     0,   113,   114,   115,     0,
       0,     4,     5,     6,     7,     0,     9,     0,     0,   465,
     122,   123,   466,   124,     0,     0,     0,    11,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   467,     0,     0,    30,     0,     0,
       0,    31,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,    43,   256,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,    95,     0,     0,    96,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,     0,     0,     0,   108,   109,   110,     0,     0,
     295,   485,     0,     2,     0,     3,     0,     0,     0,     0,
       0,   111,     0,     0,     0,     0,     4,     5,     6,     7,
     112,     9,     0,     0,   113,   114,   115,    10,     0,     0,
       0,     0,    11,     0,     0,     0,     0,    12,   122,   123,
       0,   124,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,    24,     0,     0,
      25,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
       0,     0,    30,     0,     0,     0,    31,    32,    33,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,    37,    38,    39,    40,    41,    42,    43,   256,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,     0,    95,     0,     0,    96,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,   105,   106,   107,     0,     0,     0,
     108,   109,   110,     0,     0,   295,     0,     0,     2,     0,
       3,     0,     0,     0,     0,     0,   111,     0,     0,     0,
       0,     4,     5,     6,     7,   112,     9,     0,     0,   113,
     114,   115,    10,     0,     0,     0,     0,    11,     0,     0,
       0,     0,    12,   122,   123,     0,   124,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,   554,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    29,     0,     0,    30,     0,     0,
       0,    31,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,    43,   256,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,    95,     0,     0,    96,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,     0,     0,     0,   108,   109,   110,     0,     0,
     295,     0,     0,     2,     0,     3,     0,     0,     0,     0,
       0,   111,     0,     0,     0,     0,     4,     5,     6,     7,
     112,     9,     0,     0,   113,   114,   115,    10,     0,     0,
       0,     0,    11,     0,     0,     0,     0,    12,   122,   123,
       0,   124,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   806,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,    24,     0,     0,
      25,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
       0,     0,    30,     0,     0,     0,    31,    32,    33,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,    37,    38,    39,    40,    41,    42,    43,   256,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,     0,    95,     0,     0,    96,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,   105,   106,   107,     0,     0,     0,
     108,   109,   110,     0,     0,   295,     0,     0,     2,     0,
       3,     0,     0,     0,     0,     0,   111,     0,     0,     0,
       0,     4,     5,     6,     7,   112,     9,     0,     0,   113,
     114,   115,    10,     0,     0,     0,     0,    11,     0,     0,
       0,     0,    12,   122,   123,     0,   124,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    29,     0,     0,    30,     0,     0,
       0,    31,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,    43,   256,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,    95,     0,     0,    96,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,     0,     0,     0,   108,   109,   110,     0,     0,
     295,     0,     0,     2,     0,     3,     0,     0,     0,     0,
       0,   111,     0,     0,     0,     0,     4,     5,     6,     7,
     112,     9,     0,     0,   113,   114,   115,    10,     0,     0,
       0,     0,    11,     0,     0,     0,     0,    12,   122,   123,
       0,   124,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,    24,     0,     0,
      25,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
       0,     0,    30,     0,     0,     0,    31,    32,    33,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,    37,    38,    39,    40,    41,    42,    43,   256,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,     0,    95,     0,     0,    96,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,   105,   106,   107,     0,     0,     0,
     108,   109,   110,     0,     0,   295,     0,     0,     2,     0,
       3,     0,     0,     0,     0,     0,   111,     0,     0,     0,
       0,     4,     5,     6,     7,   112,     9,     0,     0,   113,
     114,   115,    10,     0,     0,     0,     0,    11,     0,     0,
       0,     0,    12,   122,   123,     0,   389,     0,     0,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    29,     0,     0,    30,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,    43,   256,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     0,     0,     0,    60,
      61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,    95,     0,     0,    96,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   295,     0,     0,     2,     0,     3,   104,   105,
     106,   107,     0,     0,     0,   108,   109,   110,     4,     5,
       6,     7,     0,     9,     0,     0,     0,     0,     0,    10,
       0,   111,     0,     0,    11,     0,     0,     0,     0,    12,
     112,     0,     0,     0,     0,     0,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   122,   123,
       0,   124,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    29,     0,     0,    30,     0,     0,     0,     0,     0,
      33,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    37,    38,    39,    40,    41,    42,    43,
     256,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,     0,     0,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,    95,     0,
       0,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,     0,
       0,     0,   108,   109,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,     0,     0,   112,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   124
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-943)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     1,     1,   122,   123,   265,   304,   114,   115,   404,
     150,     1,    95,    21,   393,   137,   409,   137,   304,    24,
      24,   137,   282,   137,   356,     4,     4,    64,   792,     5,
       3,   291,    18,    73,     4,    27,   336,   337,   802,   409,
     340,     4,   342,     4,     4,    70,     4,     4,    34,     4,
       4,     4,     4,     3,   438,    77,    73,    73,     4,     4,
     343,    50,   240,   524,     5,   267,     3,   150,    60,   764,
     910,   735,   417,     5,   298,   375,   840,     5,   116,     5,
       5,   116,     4,   758,   759,    41,    75,   210,    24,   293,
      58,   875,     6,   353,     8,    31,     4,   301,   302,   905,
     386,    19,    20,    21,    22,    19,    20,    21,    22,    19,
      20,    21,    22,    32,  1056,     3,    27,    20,   116,    22,
     904,    65,    65,    24,   114,   115,   890,    67,   968,   137,
       0,     1,   130,   526,    49,   299,   300,    25,   263,    82,
      59,    36,   755,    65,   757,     3,   525,    23,    73,    60,
       3,     3,   765,    93,    23,   262,   526,   125,     3,    74,
      36,   774,    70,    32,    79,   288,  1108,    36,   547,   256,
     845,   278,   109,   141,   849,   224,    91,   852,   122,   319,
     136,   121,    77,    93,   124,   272,   224,     3,   366,   224,
      59,    86,   129,     3,   869,   110,    64,   581,   498,   133,
     122,    77,     3,   131,  1107,    81,   489,   385,    77,   316,
      86,     3,    81,   558,    24,  1021,   265,    86,   358,    23,
     444,   346,  1062,    24,  1127,   223,     3,   142,   347,   348,
     349,   350,    24,   352,   898,   150,   319,   762,   763,   934,
     240,   240,   367,   544,     3,   155,   156,   860,   861,     3,
     240,   926,   865,   457,   929,   254,   542,   435,   298,    77,
     520,   298,   563,   236,   254,   269,    73,   240,    86,   210,
     260,   260,   262,   298,    24,   358,   298,    81,   210,   268,
    1044,   298,   298,   273,   210,   746,   236,   277,   278,   279,
     550,     6,    23,     8,   299,   299,   282,   296,   288,   289,
     290,   599,   292,   293,   294,   295,   133,   299,   701,   288,
     288,   301,   302,   599,   614,   305,   616,   617,   326,   441,
     620,   441,   298,   623,   464,   441,   316,   441,   298,   315,
    1094,   701,   716,   794,     3,   298,   288,   298,   298,   541,
     298,   298,   288,   298,   298,   298,   336,   337,   338,   339,
     340,   341,   342,   298,   344,   345,   288,   289,   441,   561,
     278,   351,   298,     3,   278,   355,   568,   366,   278,     3,
     240,   703,   751,   283,   284,   278,   366,   273,   236,   282,
     993,   464,   240,   236,   236,   375,     6,   240,   240,     3,
       6,   236,     8,    13,   919,   240,   921,   790,   923,   685,
      53,     3,    46,    19,    20,    21,    22,   397,    36,     3,
     271,   936,   402,    80,   290,   291,   292,   293,   294,   295,
     790,   290,   291,   292,   293,   294,   295,    53,   296,   297,
     298,   299,   300,   441,    50,   296,   297,   261,   831,   263,
      73,     6,   441,     8,    88,   435,   586,    80,   438,    77,
       3,    67,    73,    24,    19,    20,    21,    22,    86,    80,
      31,   831,   261,   463,   263,   990,   360,   457,     3,   363,
     364,    36,   579,   214,   215,   119,   120,    93,     4,     5,
      23,     7,   126,   469,     3,    50,   290,   291,   292,   293,
     294,   295,   885,     3,   601,   635,   296,   297,   700,    42,
       3,    24,    67,   586,     3,   121,     4,     5,   498,     7,
     154,   164,   165,   166,    57,   885,   339,   170,   341,    42,
       4,     5,   175,     7,   216,   217,   218,   517,    93,   829,
     830,     4,     5,   532,     7,     3,    49,   916,   164,   165,
     166,     0,     4,     5,   170,     7,     4,     5,    71,   175,
     810,     3,   635,   250,   251,   545,   121,     4,     5,   102,
      74,    74,   552,   106,   554,    79,    79,     4,     5,   962,
      83,    14,    15,    16,    17,    18,   264,   265,    91,   102,
     123,     4,     5,   723,   574,   575,   576,     3,   578,   579,
     580,   581,   962,   715,   796,   715,     4,     5,     7,   715,
     123,   715,     4,     5,   122,     7,    81,     4,     5,    49,
       7,   601,   228,   229,   230,   231,   232,   233,   131,   289,
      41,   245,     4,     5,   614,     7,   616,   617,   142,   142,
     620,   288,   715,   623,    74,     3,   150,   150,   282,    79,
     723,   939,    62,    63,   256,   257,   262,     3,    69,    49,
    1045,    91,  1047,  1048,  1049,     4,     5,    97,   226,   227,
       3,   961,   278,   228,   229,   230,   231,   232,   233,     4,
       5,    92,     4,     5,    74,   224,   225,     3,   678,    79,
     940,   821,     3,    83,   105,     4,     5,     3,   128,     4,
       5,    91,     3,   515,   516,   515,   516,   262,   122,   123,
     347,   348,   142,   349,   350,   695,     3,   715,   523,   524,
     150,     3,     3,   278,   704,   136,   245,     3,   298,   296,
      43,     3,     3,    43,    13,     4,   716,    82,    82,    24,
      24,    93,    80,   214,     4,     4,     3,    27,   821,     4,
       4,   128,   142,    31,     5,     4,     3,     3,   747,   128,
     150,     5,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,     4,   100,    73,    83,
      83,    73,   211,   215,     4,   109,     4,     4,  1038,     5,
       4,    24,     5,     4,    25,     5,     5,     3,   895,  1087,
       4,    24,    24,    96,    31,     5,    83,   290,    13,    24,
     131,   129,    24,     5,     5,     5,    24,   211,     4,   289,
     809,   266,    21,   234,   235,     4,     4,   238,   239,     4,
     241,   242,   905,   244,    73,   815,     4,  1125,     4,     4,
      13,     3,    84,    84,     3,    97,    96,     5,     4,   829,
     830,    75,     3,     5,   834,     4,    39,   298,     5,    99,
       5,   252,     3,   274,   275,   276,   277,   278,   279,   280,
     281,   252,   283,   284,    24,     3,     3,    41,    39,   132,
      76,     4,     4,    21,    73,    39,   237,   237,   237,    21,
       3,   117,   220,     5,   237,     5,    42,   886,     5,   267,
       4,     4,   237,   110,    21,    93,     3,    70,    24,     5,
       5,    98,   224,   106,   117,   895,   263,   258,     3,   219,
     214,   263,     5,    31,   289,    31,     3,    21,     3,   259,
     270,     3,    24,   224,   271,     4,    24,    24,    68,   222,
       7,   273,   751,   951,   873,  1021,   885,  1056,  1021,   967,
     572,   701,   542,   685,   517,  1128,  1007,     8,   688,   461,
     327,   137,   328,   137,   545,  1054,   599,   599,  1055,   962,
    1062,   519,   910,   408,   514,   939,   385,   352,   711,   715,
     316,   961,   218,   444,  1008,   300,   442,   262,  1073,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,   998,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,  1053,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,  1078,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,   109,    -1,    -1,   112,  1107,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,   123,    -1,   125,
     126,    -1,    -1,   129,    -1,    -1,    -1,  1127,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    -1,    -1,    -1,   290,   291,   292,   293,   294,   295,
     296,   297,     3,   299,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,   109,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     221,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   278,    -1,    -1,
      -1,   282,   283,   284,    -1,    -1,    -1,    -1,    -1,   290,
     291,   292,   293,   294,   295,   296,   297,     3,   299,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,   109,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,   211,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    -1,    -1,    -1,   290,   291,   292,   293,   294,   295,
     296,   297,     3,   299,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,   109,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   278,    -1,    -1,
      -1,   282,   283,   284,    -1,    -1,    -1,    -1,    -1,   290,
     291,   292,   293,   294,   295,   296,   297,     3,   299,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,   123,    -1,    -1,
     126,    -1,    -1,   129,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    -1,    -1,    -1,   290,   291,   292,   293,   294,   295,
     296,   297,     3,   299,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,    -1,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   278,    -1,    -1,
      -1,   282,   283,   284,    -1,    -1,    -1,    -1,    -1,   290,
     291,   292,   293,   294,   295,   296,   297,     3,   299,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,   123,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    -1,    -1,    -1,   290,   291,   292,   293,   294,   295,
     296,   297,     3,   299,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,    -1,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   278,    -1,    -1,
      -1,   282,   283,   284,    -1,    -1,    -1,    -1,    -1,   290,
     291,   292,   293,   294,   295,   296,   297,     3,   299,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    -1,    -1,    -1,   290,   291,   292,   293,   294,   295,
     296,   297,     3,   299,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,    -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,   164,   165,   166,    -1,    -1,    -1,   170,
      -1,    -1,    -1,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,   278,    24,    -1,
      -1,   282,   283,   284,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    38,    -1,    40,   296,   297,    -1,   299,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,   278,    24,    -1,    -1,   282,   283,   284,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
     296,   297,    -1,   299,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,    -1,    -1,    -1,   126,    -1,    -1,   129,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,   278,    24,    -1,
      -1,   282,   283,   284,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,   296,   297,    -1,   299,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    -1,    -1,    93,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,    -1,    -1,   209,    -1,    -1,   212,   213,   214,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     246,   247,   248,   249,    -1,    -1,    -1,   253,   254,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,   278,    -1,    -1,    -1,   282,   283,   284,    -1,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    27,
     296,   297,    30,   299,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    -1,    -1,    93,    -1,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,   115,    -1,    -1,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,   209,    -1,    -1,   212,   213,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     228,   229,   230,   231,   232,   233,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   246,   247,
     248,   249,    -1,    -1,    -1,   253,   254,   255,    -1,    -1,
       3,     4,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,   269,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     278,    24,    -1,    -1,   282,   283,   284,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    40,   296,   297,
      -1,   299,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    -1,    -1,
      93,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
      -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,    -1,    -1,   209,    -1,    -1,   212,
     213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   228,   229,   230,   231,   232,
     233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   246,   247,   248,   249,    -1,    -1,    -1,
     253,   254,   255,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,   278,    24,    -1,    -1,   282,
     283,   284,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,   296,   297,    -1,   299,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    -1,    -1,    93,    -1,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,   115,    -1,    -1,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,   209,    -1,    -1,   212,   213,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     228,   229,   230,   231,   232,   233,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   246,   247,
     248,   249,    -1,    -1,    -1,   253,   254,   255,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,   269,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     278,    24,    -1,    -1,   282,   283,   284,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    40,   296,   297,
      -1,   299,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    -1,    -1,
      93,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
      -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,    -1,    -1,   209,    -1,    -1,   212,
     213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   228,   229,   230,   231,   232,
     233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   246,   247,   248,   249,    -1,    -1,    -1,
     253,   254,   255,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,   278,    24,    -1,    -1,   282,
     283,   284,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,   296,   297,    -1,   299,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    -1,    -1,    93,    -1,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,   115,    -1,    -1,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,   209,    -1,    -1,   212,   213,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     228,   229,   230,   231,   232,   233,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   246,   247,
     248,   249,    -1,    -1,    -1,   253,   254,   255,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,   269,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     278,    24,    -1,    -1,   282,   283,   284,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    40,   296,   297,
      -1,   299,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    -1,    -1,
      93,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
      -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,    -1,    -1,   209,    -1,    -1,   212,
     213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   228,   229,   230,   231,   232,
     233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   246,   247,   248,   249,    -1,    -1,    -1,
     253,   254,   255,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,   278,    24,    -1,    -1,   282,
     283,   284,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,   296,   297,    -1,   299,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    -1,    -1,    93,    -1,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,   115,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,    -1,    -1,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,    -1,
      -1,   209,    -1,    -1,   212,   213,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     228,   229,   230,   231,   232,   233,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,   246,   247,
     248,   249,    -1,    -1,    -1,   253,   254,   255,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,   269,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
     278,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   296,   297,
      -1,   299,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      -1,    -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,    -1,    -1,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,    -1,    -1,   209,    -1,
      -1,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   228,   229,   230,
     231,   232,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   246,   247,   248,   249,    -1,
      -1,    -1,   253,   254,   255,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   269,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   278,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   299
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    35,    40,    44,    46,    47,    48,    58,    66,    67,
      68,    81,    88,    89,    90,    93,    95,   107,   109,   112,
     115,   119,   120,   121,   123,   125,   126,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   174,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   209,   212,   213,   228,   229,
     230,   231,   232,   233,   246,   247,   248,   249,   253,   254,
     255,   269,   278,   282,   283,   284,   290,   291,   292,   293,
     294,   295,   296,   297,   299,   304,   305,   306,   307,   318,
     326,   327,   328,   329,   330,   331,   332,   335,   336,   337,
     338,   343,   348,   374,   376,   377,   378,   379,   380,   382,
     383,   385,   386,   387,   388,   390,   391,   393,   394,   396,
     398,   399,   400,   404,   406,   407,   408,   409,   410,   411,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   443,   444,   449,   450,   452,   484,
     485,   503,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   530,   531,   532,   533,   534,   536,
     537,   538,   539,   543,   545,   546,   547,   548,   549,   550,
     559,   560,   561,   586,   587,   588,   589,   590,   602,   604,
       3,   129,   305,   333,   347,   348,   379,   496,   497,   498,
     500,   502,   530,    24,     3,    23,   141,   376,    25,   558,
       3,   133,     3,     3,   116,   130,   223,    73,     3,   400,
      24,   601,   326,     3,   133,    27,    60,   339,     3,     3,
     400,   273,   368,   369,   490,   491,   492,   494,     3,     3,
       3,    80,     3,     3,     3,     3,   214,   336,   595,   596,
     600,     3,     3,     3,     3,     3,     3,   530,   531,   531,
     510,   547,   510,     0,     7,    77,    86,   122,   326,   383,
     384,   386,   387,   390,   393,   395,    81,   289,   288,   335,
     336,    20,    22,   278,   282,     3,     3,     3,     3,     3,
       3,     3,     3,   245,     3,     3,     3,   299,   300,   296,
     297,   298,   296,    43,   535,     3,    13,   500,     3,     4,
      82,     4,     4,    65,   122,     4,     3,   109,   348,   379,
     504,   530,   504,   531,   299,   339,    24,   554,   555,   556,
     557,   554,    24,   624,   369,     3,    80,    93,   530,   299,
     349,   401,   402,   530,   553,   531,    38,    73,    85,   118,
     530,   551,   552,   400,   554,    49,    74,    79,    91,   110,
     142,   150,   486,   489,   530,   530,   530,   554,   527,   530,
     527,   530,   540,   541,   542,    53,   164,   165,   166,   170,
     175,   412,   413,   519,   530,     3,   530,   379,   215,   596,
     597,   214,   591,   592,   595,   527,   530,   527,   530,     4,
     404,   438,   439,   440,   602,   530,     4,     3,   392,   400,
     508,   531,    27,   501,    36,    27,    30,   112,   335,   397,
     326,   377,   378,     4,   528,   530,   598,   599,   524,   528,
     528,   529,   529,   525,   528,     4,   529,   526,   528,     3,
      24,   455,   456,   457,   463,   445,   530,   530,   339,   511,
     511,   512,   512,   530,   536,   547,   554,   530,    24,   269,
     560,     4,   334,   335,   501,   501,   501,   339,    31,     5,
     128,     4,   528,     6,    13,     3,     3,   321,   216,   217,
     218,   606,   131,   346,   370,   502,   344,   345,   403,   602,
       4,    73,    80,   362,   363,     5,    24,    31,   405,   585,
     128,   530,    73,     4,    73,   530,    31,   365,   603,   365,
     367,    83,   489,   340,   341,   559,   561,   100,    83,     4,
       4,     4,   321,     4,     5,     7,    77,     4,    73,    73,
      73,   210,   598,   211,   336,   381,   383,   384,   386,   390,
     393,   593,   594,   597,   592,     4,     4,     4,     4,     5,
     527,    64,   389,   389,   348,   335,   400,    67,    93,   121,
     124,   373,     4,     4,     5,     4,     5,     7,     4,     4,
       5,     4,     4,     5,   455,   464,   467,   468,     5,   441,
       4,   528,     4,     3,     4,     5,   497,   499,   347,   498,
     347,   349,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   505,   506,   505,   554,
       4,    24,   556,    24,   556,   557,    42,    71,   102,   123,
     308,   309,   310,   313,   315,   585,   317,   561,    31,    96,
      58,   125,   141,   607,   379,     5,   364,   369,   344,   346,
     402,   585,   554,   530,   530,    73,    24,   321,    24,   369,
      83,     5,   346,    13,   290,   369,   333,   530,   530,   530,
     530,   531,   530,   598,   335,     5,   210,   211,   438,   439,
     453,     4,   531,   289,   528,   528,   528,   528,   528,   266,
     465,   469,     4,    21,   446,     4,   250,   251,   442,   448,
       4,     4,   335,     4,     4,     4,    13,     3,    84,    84,
       4,     5,     3,    41,    69,    92,   105,   136,   234,   235,
     238,   239,   241,   242,   244,   274,   275,   276,   277,   278,
     279,   280,   281,   283,   284,   562,   565,   566,   573,   574,
     575,   576,   577,   578,   580,   581,   582,   583,   584,     4,
       5,   348,    96,   608,    97,   345,     5,   362,    75,   371,
       4,   530,     3,   320,   369,   341,    57,   342,   530,    97,
     128,   487,   488,   495,     4,    70,   544,   544,     4,   326,
     336,   383,   386,   390,   393,   594,   598,     4,   335,     5,
       5,    39,    99,   322,     5,   442,   252,   252,    24,   379,
       3,   309,   316,   585,     3,   236,   240,   567,   568,   236,
     240,   567,   236,   240,   567,   568,   568,     3,   569,   570,
      41,   136,   569,   569,   170,   519,   520,   522,   523,   236,
     240,   567,   567,   311,    41,   563,   535,   564,   561,   132,
     319,   316,   609,   557,   369,    39,    76,   372,   316,   379,
       3,   554,   530,     4,     4,    86,   335,    73,   451,   454,
     528,   528,   470,   471,   561,    39,   264,   265,   466,   473,
     474,   447,   530,     4,   316,     4,     5,    21,   568,   237,
     568,   237,   568,   237,    21,   571,   236,   240,   567,   236,
     567,     3,   579,   567,   117,   568,   237,    23,    42,    57,
     106,   313,   314,   110,   564,    42,   220,   605,   317,   379,
     267,   458,   459,     4,   316,    62,    63,   493,   256,   257,
     442,     5,     5,   564,   323,   324,   336,   380,    36,    50,
     262,   415,   475,   476,   477,   478,     4,   585,     4,   569,
     569,   569,    14,    15,    16,    17,    18,   572,     4,   568,
     237,   568,    21,   170,   519,   521,   569,    93,     3,    93,
     312,   404,   515,   554,    24,    98,   226,   227,   617,   618,
      70,   610,   611,   455,   460,   461,   462,   322,     4,   528,
     471,     5,    32,    59,   325,   325,   262,   415,   476,   479,
     481,   482,   224,   263,   263,   258,   472,   483,   106,   569,
       4,     5,   567,   379,     3,   116,   224,   366,   116,   366,
     618,   219,   214,   613,     5,    31,   270,   359,   361,   324,
     261,   261,   289,    50,    75,   260,   268,   554,    21,     4,
     316,   365,   365,   365,   365,   224,   225,   612,     3,   375,
     221,   305,   614,   616,   461,   463,   513,   271,   350,   351,
     480,   481,   224,   259,     3,     4,     4,    24,   620,   621,
      24,   623,   619,   621,    24,   622,   379,   222,   271,   360,
     357,   404,    68,   355,   356,   316,     4,   615,   616,   513,
     224,   265,   354,   256,   272,   353,     4,     7,   352,   358,
     404,   211,   616,   354,   273
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

    { xxx_pGLOBAL_SQLPARSER->setParseTree( (yyvsp[(1) - (1)].pParseNode) ); }
    break;

  case 3:

    { xxx_pGLOBAL_SQLPARSER->setParseTree( (yyvsp[(1) - (2)].pParseNode) ); }
    break;

  case 5:

    {
                (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
       }
    break;

  case 9:

    {(yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 10:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 11:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 14:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            }
    break;

  case 15:

    {(yyval.pParseNode) = SQL_NEW_LISTRULE;}
    break;

  case 16:

    {(yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);}
    break;

  case 19:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 20:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 22:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 23:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 24:

    {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 26:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 27:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 28:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 29:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 30:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode));}
    break;

  case 31:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (10)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (10)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(6) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (10)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(9) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(10) - (10)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 32:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 33:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 34:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 35:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 36:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 37:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode));}
    break;

  case 38:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 39:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 40:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 41:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 42:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 43:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 44:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 45:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 46:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 47:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 48:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 49:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 50:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 53:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 64:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 65:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 66:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));}
    break;

  case 67:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));}
    break;

  case 68:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));}
    break;

  case 69:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 70:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 71:

    {    
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 74:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 75:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode)); }
    break;

  case 76:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 79:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 80:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 81:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 84:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));}
    break;

  case 85:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 86:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 88:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 90:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 91:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 92:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 94:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 96:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 97:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 103:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 105:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 108:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 110:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 111:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 112:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 113:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (9)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(9) - (9)].pParseNode));
        }
    break;

  case 114:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 115:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 116:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 117:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 119:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 121:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 122:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 123:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 125:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 126:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 128:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 129:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 130:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 131:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 132:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 138:

    { // boolean_primary: rule 2
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 139:

    {
            if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck())// boolean_primary: rule 3
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildComparsionRule((yyval.pParseNode),(yyvsp[(1) - (1)].pParseNode));
                if(nErg == 1)
                {
                    OSQLParseNode* pTemp = (yyval.pParseNode);
                    (yyval.pParseNode) = pTemp->removeAt((sal_uInt32)0);
                    delete pTemp;
                }
                else
                {
                    delete (yyval.pParseNode);
                    if(nErg)
                        YYERROR;
                    else
                        YYABORT;
                }
            }
            else
                YYERROR;
        }
    break;

  case 140:

    { // boolean_primary: rule 2
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 142:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 144:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 146:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 148:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 157:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 158:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 159:

    {
            if(xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // comparison_predicate: rule 2
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule((yyval.pParseNode),(yyvsp[(2) - (2)].pParseNode),(yyvsp[(1) - (2)].pParseNode));
                if(nErg == 1)
                {
                    OSQLParseNode* pTemp = (yyval.pParseNode);
                    (yyval.pParseNode) = pTemp->removeAt((sal_uInt32)0);
                    delete pTemp;
                }
                else
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
            }
            else
            {
                YYERROR;
            }
        }
    break;

  case 166:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 167:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // between_predicate: rule 2 
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                
                sal_Int16 nErg = xxx_pGLOBAL_SQLPARSER->buildPredicateRule((yyval.pParseNode),(yyvsp[(3) - (5)].pParseNode),(yyvsp[(2) - (5)].pParseNode),(yyvsp[(5) - (5)].pParseNode));
                if(nErg == 1)
                {
                    OSQLParseNode* pTemp = (yyval.pParseNode);
                    (yyval.pParseNode) = pTemp->removeAt((sal_uInt32)0);
                    OSQLParseNode* pColumnRef = (yyval.pParseNode)->removeAt((sal_uInt32)0);
                    (yyval.pParseNode)->insert(0,(yyvsp[(1) - (5)].pParseNode));
                    OSQLParseNode* pBetween_predicate = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::between_predicate));
                    pBetween_predicate->append(pColumnRef);
                    pBetween_predicate->append((yyval.pParseNode));
                    (yyval.pParseNode) = pBetween_predicate;
                    
                    delete pTemp;
                    delete (yyvsp[(4) - (5)].pParseNode);
                }
                else
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
            }
            else
            {
                (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
                (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
            }
        }
    break;

  case 168:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 169:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 170:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 171:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 172:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 173:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())  // like_predicate: rule 5
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule((yyvsp[(1) - (1)].pParseNode),p2nd,p3rd) )
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
                (yyvsp[(1) - (1)].pParseNode)->append(p3rd);
            }
            else
                YYERROR;
        }
    break;

  case 174:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // like_predicate: rule 6
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                if ( !xxx_pGLOBAL_SQLPARSER->buildLikeRule((yyvsp[(1) - (1)].pParseNode),p2nd,p3rd) )
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
                (yyvsp[(1) - (1)].pParseNode)->append(p3rd);
            }
            else
                YYERROR;
        }
    break;

  case 175:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 176:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 177:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 178:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 179:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())// test_for_null: rule 2
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 180:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 181:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 182:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 183:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 184:

    {
            if ( xxx_pGLOBAL_SQLPARSER->inPredicateCheck() )// in_predicate: rule 2
            {
                OSQLParseNode* pColumnRef = newNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(newNode(xxx_pGLOBAL_SQLPARSER->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 185:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 186:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 190:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 191:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 192:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 193:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 194:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 202:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                xxx_pGLOBAL_SQLPARSER->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 203:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                xxx_pGLOBAL_SQLPARSER->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 204:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                xxx_pGLOBAL_SQLPARSER->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 205:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                xxx_pGLOBAL_SQLPARSER->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 206:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 207:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 209:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 210:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 214:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 215:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 216:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 217:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 218:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 219:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 220:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 221:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 222:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 225:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 240:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 241:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 242:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 243:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 244:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 245:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 246:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 247:

    {
            if ( (yyvsp[(3) - (4)].pParseNode)->count() == 1 || (yyvsp[(3) - (4)].pParseNode)->count() == 2 )
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
                (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            }
            else
                YYERROR;
        }
    break;

  case 248:

    {
            if ( (yyvsp[(3) - (4)].pParseNode)->count() == 2 || (yyvsp[(3) - (4)].pParseNode)->count() == 3)
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
                (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            }
            else
                YYERROR;
        }
    break;

  case 305:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 306:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 312:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 317:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 318:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 319:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 320:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 322:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
    }
    break;

  case 330:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 333:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 335:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (8)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (8)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (8)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(7) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (8)].pParseNode));
    }
    break;

  case 338:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 339:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 344:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 346:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 347:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 348:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 349:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 351:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 352:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 354:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 356:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 360:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 361:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 362:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 363:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 364:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 366:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 371:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 373:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 374:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 375:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 379:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 381:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 382:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 383:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 384:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 385:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 386:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 387:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 388:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 399:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 400:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 401:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 402:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 405:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 406:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 408:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 409:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 410:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 411:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 413:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode));
        }
    break;

  case 414:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 419:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 423:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 425:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 428:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 429:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 430:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 450:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 458:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 460:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 461:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 465:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 466:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 468:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 469:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 471:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 472:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 473:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 474:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 475:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 476:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 477:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 478:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 479:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 480:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 486:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 488:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 489:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 490:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 491:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 493:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 494:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 495:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode));
        }
    break;

  case 496:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 497:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 498:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
                (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
            else
                YYERROR;
        }
    break;

  case 500:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 501:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 502:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
            {
                (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
                (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
            else
                YYERROR;
        }
    break;

  case 509:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 510:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 513:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 515:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 518:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 519:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 520:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 521:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 522:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 525:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 526:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 527:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 528:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 529:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 530:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 531:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 534:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 535:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 536:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 537:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 538:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 539:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 540:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 541:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 542:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 547:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 548:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 549:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 550:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 551:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 552:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 553:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 554:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 555:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 556:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 557:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 558:

    {
			auto last = (yyvsp[(1) - (3)].pParseNode)->getLast();
			if (last)
				{
				if (last->getFirst()->getNodeType() == SQL_NODE_PUNCTUATION) //'*'
					{
					SQLyyerror("'*' can only occur at the end of property path\n");
					YYERROR;
					}
				}

            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 559:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 560:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 561:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 563:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 564:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 565:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 567:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 568:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 574:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 575:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 576:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 577:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 578:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 580:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 582:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 583:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 585:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 586:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 587:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 588:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("K", SQL_NODE_PUNCTUATION));
        }
    break;

  case 589:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("M", SQL_NODE_PUNCTUATION));
        }
    break;

  case 590:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("G", SQL_NODE_PUNCTUATION));
        }
    break;

  case 591:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("T", SQL_NODE_PUNCTUATION));
        }
    break;

  case 592:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("P", SQL_NODE_PUNCTUATION));
        }
    break;

  case 593:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 594:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 595:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 596:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 597:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 598:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 599:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 600:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 601:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 603:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 604:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 605:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 606:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 607:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 608:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 610:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 611:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 614:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 615:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 616:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 627:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            /*$$->append($3);*/
        }
    break;

  case 628:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 632:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 633:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 634:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 637:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 638:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 639:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 640:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 641:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 642:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 643:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 650:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 651:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 652:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 653:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 654:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 658:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 659:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 660:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 661:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 662:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 663:

    {
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck()) // sql: rule 1
            {
                (yyval.pParseNode) = (yyvsp[(1) - (1)].pParseNode);
                if ( SQL_ISRULE((yyval.pParseNode),search_condition) )
                {
                    (yyval.pParseNode)->insert(0,newNode("(", SQL_NODE_PUNCTUATION));
                    (yyval.pParseNode)->append(newNode(")", SQL_NODE_PUNCTUATION));
                }
            }
            else
                YYERROR;
        }
    break;

  case 665:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(5) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(6) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(7) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(8) - (9)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(9) - (9)].pParseNode));
    }
    break;

  case 666:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 667:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 670:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 673:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 674:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 675:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 677:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 678:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 679:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 682:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 683:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 685:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(";", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 686:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 687:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 689:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 690:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 691:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 692:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 693:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 694:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}






using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
//using namespace ::osl;
using namespace ::dbtools;
//    using namespace connectivity;

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
// string constants
struct _ConstAsciiString_
{
    sal_Int32 length;
    sal_Char  const* str;
    //operator Utf8String () const { return Utf8String (str); }
    operator const sal_Char * () const { return str; }
    operator Utf8String() const { return str; }
};

#define IMPLEMENT_CONSTASCII_STRING( name, string ) \
    _ConstAsciiString_ const name = { sizeof(string)-1, string }

IMPLEMENT_CONSTASCII_STRING(ERROR_STR_GENERAL, "Syntax error in SQL expression");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_VALUE_NO_LIKE, "The value #1 can not be used with LIKE.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_FIELD_NO_LIKE, "LIKE can not be used with this field.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COMPARE, "The entered criterion can not be compared with this field.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_DATE_COMPARE, "The field can not be compared with a date.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_REAL_COMPARE,    "The field can not be compared with a floating point number.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_INT_COMPARE,    "The field can not be compared with a number.");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE,    "The database does not contain a table named \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE_OR_QUERY,   "The database does contain neither a table nor a query named \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_COLUMN,    "The column \"#1\" is unknown in the table \"#2\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_TABLE_EXIST,    "The database already contains a table or view with name \"#\".");
IMPLEMENT_CONSTASCII_STRING(ERROR_STR_INVALID_QUERY_EXIST,    "The database already contains a query with name \"#\".");

IMPLEMENT_CONSTASCII_STRING(KEY_STR_LIKE, "LIKE");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NOT, "NOT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_NULL, "NULL");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_TRUE, "True");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_FALSE, "False");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_IS, "IS");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_BETWEEN, "BETWEEN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_OR, "OR");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AND, "AND");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_AVG, "AVG");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_COUNT, "COUNT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MAX, "MAX");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_MIN, "MIN");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_SUM, "SUM");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_EVERY, "EVERY");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_ANY, "ANY");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_SOME, "SOME");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_STDDEV_POP, "STDDEV_POP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_STDDEV_SAMP, "STDDEV_SAMP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_VAR_SAMP, "VAR_SAMP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_VAR_POP, "VAR_POP");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_COLLECT, "COLLECT");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_FUSION, "FUSION");
IMPLEMENT_CONSTASCII_STRING(KEY_STR_INTERSECTION, "INTERSECTION");

IMPLEMENT_CONSTASCII_STRING(FIELD_STR_NULLDATE, "NullDate");

IMPLEMENT_CONSTASCII_STRING(STR_SQL_TOKEN, "SQL_TOKEN_");

//==========================================================================
//= OParseContext
//==========================================================================
//-----------------------------------------------------------------------------
OParseContext::OParseContext()
{
}

//-----------------------------------------------------------------------------
OParseContext::~OParseContext()
{
}

//-----------------------------------------------------------------------------
Utf8String OParseContext::getErrorMessage(ErrorCode _eCode) const
{
    Utf8String aMsg;
    switch (_eCode)
    {
        case ERROR_GENERAL:                    aMsg = ERROR_STR_GENERAL; break;
        case ERROR_VALUE_NO_LIKE:            aMsg = ERROR_STR_VALUE_NO_LIKE; break;
        case ERROR_FIELD_NO_LIKE:            aMsg = ERROR_STR_FIELD_NO_LIKE; break;
        case ERROR_INVALID_COMPARE:            aMsg = ERROR_STR_INVALID_COMPARE; break;
        case ERROR_INVALID_INT_COMPARE:        aMsg = ERROR_STR_INVALID_INT_COMPARE; break;
        case ERROR_INVALID_DATE_COMPARE:    aMsg = ERROR_STR_INVALID_DATE_COMPARE; break;
        case ERROR_INVALID_REAL_COMPARE:    aMsg = ERROR_STR_INVALID_REAL_COMPARE; break;
        case ERROR_INVALID_TABLE:            aMsg = ERROR_STR_INVALID_TABLE; break;
        case ERROR_INVALID_TABLE_OR_QUERY:  aMsg = ERROR_STR_INVALID_TABLE_OR_QUERY; break;
        case ERROR_INVALID_COLUMN:            aMsg = ERROR_STR_INVALID_COLUMN; break;
        case ERROR_INVALID_TABLE_EXIST:        aMsg = ERROR_STR_INVALID_TABLE_EXIST; break;
        case ERROR_INVALID_QUERY_EXIST:        aMsg = ERROR_STR_INVALID_QUERY_EXIST; break;
        default:
            OSL_ENSURE( false, "OParseContext::getErrorMessage: unknown error code!" );
            break;
    }
    return aMsg;
}

//-----------------------------------------------------------------------------
Utf8String OParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
    Utf8String aKeyword;
    switch (_eKey)
    {
        case KEY_LIKE:        aKeyword = KEY_STR_LIKE; break;
        case KEY_NOT:        aKeyword = KEY_STR_NOT; break;
        case KEY_NULL:        aKeyword = KEY_STR_NULL; break;
        case KEY_TRUE:        aKeyword = KEY_STR_TRUE; break;
        case KEY_FALSE:        aKeyword = KEY_STR_FALSE; break;
        case KEY_IS:        aKeyword = KEY_STR_IS; break;
        case KEY_BETWEEN:    aKeyword = KEY_STR_BETWEEN; break;
        case KEY_OR:        aKeyword = KEY_STR_OR; break;
        case KEY_AND:        aKeyword = KEY_STR_AND; break;
        case KEY_AVG:        aKeyword = KEY_STR_AVG; break;
        case KEY_COUNT:        aKeyword = KEY_STR_COUNT; break;
        case KEY_MAX:        aKeyword = KEY_STR_MAX; break;
        case KEY_MIN:        aKeyword = KEY_STR_MIN; break;
        case KEY_SUM:        aKeyword = KEY_STR_SUM; break;
        case KEY_EVERY:     aKeyword = KEY_STR_EVERY; break;
        case KEY_ANY:       aKeyword = KEY_STR_ANY; break;
        case KEY_SOME:      aKeyword = KEY_STR_SOME; break;
        case KEY_COLLECT:   aKeyword = KEY_STR_COLLECT; break;
        case KEY_FUSION:    aKeyword = KEY_STR_FUSION; break;
        case KEY_INTERSECTION:aKeyword = KEY_STR_INTERSECTION; break;
        case KEY_NONE:      break;
        default:
            OSL_ENSURE( false, "OParseContext::getIntlKeywordAscii: unknown key!" );
            break;
    }
    return aKeyword;
}

//-----------------------------------------------------------------------------
IParseContext::InternationalKeyCode OParseContext::getIntlKeyCode(const Utf8String& rToken) const
{
    static IParseContext::InternationalKeyCode Intl_TokenID[] =
    {
        KEY_LIKE, KEY_NOT, KEY_NULL, KEY_TRUE,
        KEY_FALSE, KEY_IS, KEY_BETWEEN, KEY_OR,
        KEY_AND, KEY_AVG, KEY_COUNT, KEY_MAX,
        KEY_MIN, KEY_SUM, KEY_EVERY,KEY_ANY,KEY_SOME,
        KEY_COLLECT,KEY_FUSION,KEY_INTERSECTION
    };

    sal_uInt32 nCount = sizeof Intl_TokenID / sizeof Intl_TokenID[0];
    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        Utf8String aKey = getIntlKeywordAscii(Intl_TokenID[i]);
        if (rToken.EqualsI(aKey))
            return Intl_TokenID[i];
    }

    return KEY_NONE;
}

//------------------------------------------------------------------------------
static Locale& impl_getLocaleInstance( )
{
    static Locale s_aLocale(
        Utf8String("en"),
        Utf8String("US"),
        Utf8String( )
    );
    return s_aLocale;
}

//------------------------------------------------------------------------------
void OParseContext::setDefaultLocale( const ::com::sun::star::lang::Locale& _rLocale )
{
    impl_getLocaleInstance() = _rLocale;
}

//------------------------------------------------------------------------------
Locale OParseContext::getPreferredLocale( ) const
{
    return getDefaultLocale();
}

//------------------------------------------------------------------------------
const Locale& OParseContext::getDefaultLocale()
{
    return impl_getLocaleInstance();
}

//==========================================================================
//= misc
//==========================================================================
// Der (leider globale) yylval fuer die Uebergabe von
// Werten vom Scanner an den Parser. Die globale Variable
// wird nur kurzzeitig verwendet, der Parser liest die Variable
// sofort nach dem Scanner-Aufruf in eine gleichnamige eigene
// Member-Variable.

const double fMilliSecondsPerDay = 86400000.0;

//------------------------------------------------------------------------------


//------------------------------------------------------------------
Utf8String ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, sal_Bool bInternational)
{
    Utf8StringBuffer aMatchStr;
    if (pTokenNode->isToken())
    {
        sal_Char cEscape = 0;
        if (pEscapeNode->count())
            cEscape = Utf8StringHelper::toChar (pEscapeNode->getChild(1)->getTokenValue());

        // Platzhalter austauschen
        aMatchStr = pTokenNode->getTokenValue();
        const size_t nLen = aMatchStr.size();
        Utf8StringBuffer sSearch,sReplace;
        if ( bInternational )
        {
            sSearch.appendAscii("%_",2);
            sReplace.appendAscii("*?",2);
        }
        else
        {
            sSearch.appendAscii("*?",2);
            sReplace.appendAscii("%_",2);
        }
        
        bool wasEscape = false;
        for (size_t i = 0; i < nLen; i++)
        {
                const sal_Char c = aMatchStr.charAt(i);
                // SQL standard requires the escape to be followed
                // by a meta-character ('%', '_' or itself), else error
                // We are more lenient here and let it escape anything.
                // Especially since some databases (e.g. Microsoft SQL Server)
                // have more meta-characters than the standard, such as e.g. '[' and ']'
            if (wasEscape)
            {
                wasEscape=false;
                continue;
            }
            if (c == cEscape)
            {
                wasEscape=true;
                continue;
            }
            int match = -1;
            if (c == sSearch.charAt(0))
                match=0;
            else if (c == sSearch.charAt(1))
                match=1;

            if (match != -1)
            {
                aMatchStr.setCharAt(i, sReplace.charAt(match));
            }
        }
    }
    return aMatchStr.makeStringAndClear();
}


//==========================================================================
//= OSQLParser
//==========================================================================

sal_uInt32                OSQLParser::s_nRuleIDs[OSQLParseNode::rule_count + 1];
OSQLParser::RuleIDMap   OSQLParser::s_aReverseRuleIDLookup;
OParseContext            OSQLParser::s_aDefaultContext;

sal_Int32                  OSQLParser::s_nRefCount    = 0;
//BeCriticalSection        OSQLParser::s_aMutex;
OSQLScanner*            OSQLParser::s_pScanner = 0;
OSQLParseNodesGarbageCollector*        OSQLParser::s_pGarbageCollector = 0;
RefCountedPtr< ::com::sun::star::i18n::XLocaleData>        OSQLParser::s_xLocaleData = NULL;
//-----------------------------------------------------------------------------
void setParser(OSQLParser* _pParser)
{
    xxx_pGLOBAL_SQLPARSER = _pParser;
}
// -------------------------------------------------------------------------
void OSQLParser::setParseTree(OSQLParseNode * pNewParseTree)
{
    BeMutexHolder aGuard(getCriticalSection());
    m_pParseTree = pNewParseTree;
}
//-----------------------------------------------------------------------------

/** Delete all comments in a query.

    See also getComment()/concatComment() implementation for
    OQueryController::translateStatement().
 */
static Utf8String delComment( const Utf8String& rQuery )
{
    // First a quick search if there is any "--" or "//" or "/*", if not then the whole
    // copying loop is pointless.
      if (rQuery.find("--") == Utf8String::npos < 0 && rQuery.find( "//") == Utf8String::npos  &&
          rQuery.find( "/*") == Utf8String::npos)
        return rQuery;

    const sal_Char* pCopy = rQuery.c_str();
    size_t nQueryLen = rQuery.size();
    bool bIsText1  = false;     // "text"
    bool bIsText2  = false;     // 'text'
    bool bComment2 = false;     // /* comment */
    bool bComment  = false;     // -- or // comment
    Utf8StringBuffer aBuf(nQueryLen);
    for (sal_Int32 i=0; i < nQueryLen; ++i)
    {
        if (bComment2)
        {
            if ((i+1) < nQueryLen)
            {
                if (pCopy[i]=='*' && pCopy[i+1]=='/')
                {
                    bComment2 = false;
                    ++i;
                }
            }
            else
            {
                // comment can't close anymore, actually an error, but..
            }
            continue;
        }
        if (pCopy[i] == '\n')
            bComment = false;
        else if (!bComment)
        {
            if (pCopy[i] == '\"' && !bIsText2)
                bIsText1 = !bIsText1;
            else if (pCopy[i] == '\'' && !bIsText1)
                bIsText2 = !bIsText2;
            if (!bIsText1 && !bIsText2 && (i+1) < nQueryLen)
            {
                if ((pCopy[i]=='-' && pCopy[i+1]=='-') || (pCopy[i]=='/' && pCopy[i+1]=='/'))
                    bComment = true;
                else if ((pCopy[i]=='/' && pCopy[i+1]=='*'))
                    bComment2 = true;
            }
        }
        if (!bComment && !bComment2)
            aBuf.append( pCopy[i]);
    }
    return aBuf.makeStringAndClear();
}

OSQLParseNode* OSQLParser::parseTree(Utf8String& rErrorMessage,
                                     const Utf8String& rStatement,
                                     sal_Bool bInternational)
{


    // Guard the parsing
    BeMutexHolder aGuard(getCriticalSection());
    // must be reset
    setParser(this);

    // delete comments before parsing
    Utf8String sTemp = delComment(rStatement);
    // defines how to scan
    s_pScanner->SetRule(s_pScanner->GetSQLRule()); // initial
    s_pScanner->prepareScan(sTemp, m_pContext, bInternational);

    SQLyylval.pParseNode = NULL;
    //    SQLyypvt = NULL;
    m_pParseTree = NULL;
    m_sErrorMessage = Utf8String();

    // ... und den Parser anwerfen ...
    if (SQLyyparse() != 0)
    {
        // only set the error message, if it's not already set
        if (!m_sErrorMessage.size())
            m_sErrorMessage = s_pScanner->getErrorMessage();
        if (!m_sErrorMessage.size())
            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_GENERAL);

        rErrorMessage = m_sErrorMessage;

        // clear the garbage collector
        (*s_pGarbageCollector)->clearAndDelete();
        return NULL;
    }
    else
    {
        (*s_pGarbageCollector)->clear();

        // Das Ergebnis liefern (den Root Parse Node):

        //    OSL_ENSURE(Sdbyyval.pParseNode != NULL,"OSQLParser: Parser hat keinen ParseNode geliefert");
        //    return Sdbyyval.pParseNode;
        // geht nicht wegen Bug in MKS YACC-erzeugtem Code (es wird ein falscher ParseNode
        // geliefert).

        // Stattdessen setzt die Parse-Routine jetzt den Member pParseTree
        // - einfach diesen zurueckliefern:
        OSL_ENSURE(m_pParseTree != NULL,"OSQLParser: Parser hat keinen ParseTree geliefert");
        return m_pParseTree;
    }
}
//-----------------------------------------------------------------------------
Utf8String OSQLParser::TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext)
{
    Utf8String aStr;
    if (pContext)
    {
        IParseContext::InternationalKeyCode eKeyCode = IParseContext::KEY_NONE;
        switch( nTokenID )
        {
            case SQL_TOKEN_LIKE: eKeyCode = IParseContext::KEY_LIKE; break;
            case SQL_TOKEN_NOT: eKeyCode = IParseContext::KEY_NOT; break;
            case SQL_TOKEN_NULL: eKeyCode = IParseContext::KEY_NULL; break;
            case SQL_TOKEN_TRUE: eKeyCode = IParseContext::KEY_TRUE; break;
            case SQL_TOKEN_FALSE: eKeyCode = IParseContext::KEY_FALSE; break;
            case SQL_TOKEN_IS: eKeyCode = IParseContext::KEY_IS; break;
            case SQL_TOKEN_BETWEEN: eKeyCode = IParseContext::KEY_BETWEEN; break;
            case SQL_TOKEN_OR: eKeyCode = IParseContext::KEY_OR; break;
            case SQL_TOKEN_AND: eKeyCode = IParseContext::KEY_AND; break;
            case SQL_TOKEN_AVG: eKeyCode = IParseContext::KEY_AVG; break;
            case SQL_TOKEN_COUNT: eKeyCode = IParseContext::KEY_COUNT; break;
            case SQL_TOKEN_MAX: eKeyCode = IParseContext::KEY_MAX; break;
            case SQL_TOKEN_MIN: eKeyCode = IParseContext::KEY_MIN; break;
            case SQL_TOKEN_SUM: eKeyCode = IParseContext::KEY_SUM; break;
        }
        if ( eKeyCode != IParseContext::KEY_NONE )
            aStr = pContext->getIntlKeywordAscii(eKeyCode);
    }

    if (!aStr.size())
    {
        aStr = yytname[YYTRANSLATE(nTokenID)];
        if (aStr.substr(0, 10) == "SQL_TOKEN_")
        //if(!aStr.compareTo("SQL_TOKEN_",10))
            aStr = aStr.substr(10); //AK: Copy reset of the string into aStr
    }
    return aStr;
}


//-----------------------------------------------------------------------------
Utf8String OSQLParser::RuleIDToStr(sal_uInt32 nRuleID)
{
    OSL_ENSURE(nRuleID < (sizeof yytname/sizeof yytname[0]), "OSQLParser::RuleIDToStr: Invalid nRuleId!");
    return Utf8String(yytname[nRuleID]);
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::StrToRuleID(const Utf8String & rValue)
{
    // In yysvar nach dem angegebenen Namen suchen, den Index zurueckliefern
    // (oder 0, wenn nicht gefunden)
    static sal_uInt32 nLen = sizeof(yytname)/sizeof(yytname[0]);
    for (sal_uInt32 i = YYTRANSLATE(SQL_TOKEN_INVALIDSYMBOL); i < (nLen-1); i++)
    {
        if (yytname && rValue == yytname[i])
            return i;
    }

    // Nicht gefunden
    return 0;
}

//-----------------------------------------------------------------------------
OSQLParseNode::Rule OSQLParser::RuleIDToRule( sal_uInt32 _nRule )
{
    OSQLParser::RuleIDMap::const_iterator i (s_aReverseRuleIDLookup.find(_nRule));
    if (i == s_aReverseRuleIDLookup.end())
    {
    /*
        SAL_WARN("connectivity.parse",
         "connectivity::OSQLParser::RuleIDToRule cannot reverse-lookup rule. "
         "Reverse mapping incomplete? "
         "_nRule='" << _nRule << "' "
         "yytname[_nRule]='" << yytname[_nRule] << "'");
    */
        return OSQLParseNode::UNKNOWN_RULE;
    }
    else
        return i->second;
}

//-----------------------------------------------------------------------------
sal_uInt32 OSQLParser::RuleID(OSQLParseNode::Rule eRule)
{
    return s_nRuleIDs[(sal_uInt16)eRule];
}
// -------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pCompare,OSQLParseNode* pLiteral,OSQLParseNode* pLiteral2)
{
    OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
    pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
    OSQLParseNode* pComp = NULL;
    if ( SQL_ISTOKEN( pCompare, BETWEEN) && pLiteral2 )
        pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::between_predicate_part_2));
    else
        pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
    
    pComp->append(pColumnRef);
    pComp->append(pCompare);
    pComp->append(pLiteral);
    if ( pLiteral2 )
    {
        pComp->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD,SQL_TOKEN_AND));
        pComp->append(pLiteral2);        
    }
    pAppend->append(pComp);
    return 1;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildStringNodes(OSQLParseNode*& pLiteral)
{
    if(!pLiteral)
        return 1;

    if(SQL_ISRULE(pLiteral,fct_spec) || SQL_ISRULE(pLiteral,general_set_fct) || SQL_ISRULE(pLiteral,column_ref)
        || SQL_ISRULE(pLiteral,subquery))
        return 1; // here I have a function that I can't transform into a string

    if(pLiteral->getNodeType() == SQL_NODE_INTNUM || pLiteral->getNodeType() == SQL_NODE_APPROXNUM || pLiteral->getNodeType() == SQL_NODE_ACCESS_DATE)
    {
        OSQLParseNode* pParent = pLiteral->getParent();

        OSQLParseNode* pNewNode = new OSQLInternalNode(pLiteral->getTokenValue(), SQL_NODE_STRING);
        pParent->replace(pLiteral, pNewNode);
        delete pLiteral;
        pLiteral = NULL;
        return 1;
    }

    for(sal_uInt32 i=0;i<pLiteral->count();++i)
    {
        OSQLParseNode* pChild = pLiteral->getChild(i);
        buildStringNodes(pChild);
    }
    if(SQL_ISRULE(pLiteral,term) || SQL_ISRULE(pLiteral,value_exp_primary))
    {
        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
        return 0;
    }
    return 1;
}
//-----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral)
{
    OSQLParseNode* pComp = new OSQLInternalNode(Utf8String("="), SQL_NODE_EQUAL);
    return buildPredicateRule(pAppend,pLiteral,pComp);
}


//-----------------------------------------------------------------------------
void OSQLParser::reduceLiteral(OSQLParseNode*& pLiteral, sal_Bool bAppendBlank)
{
    OSL_ENSURE(pLiteral->isRule(), "This is no ::com::sun::star::chaos::Rule");
    OSL_ENSURE(pLiteral->count() == 2, "OSQLParser::ReduceLiteral() Invalid count");
    OSQLParseNode* pTemp = pLiteral;
    Utf8StringBuffer aValue(pLiteral->getChild(0)->getTokenValue());
    if (bAppendBlank)
    {
        aValue.appendAscii(" ");
    }
    
    aValue.append(pLiteral->getChild(1)->getTokenValue());

    pLiteral = new OSQLInternalNode(aValue.makeStringAndClear(),SQL_NODE_STRING);
    delete pTemp;
}

// -------------------------------------------------------------------------
void OSQLParser::error( const sal_Char* fmt)
{
    if(!m_sErrorMessage.size())
    {    
        Utf8String sStr(fmt);
        Utf8String sSQL_TOKEN("SQL_TOKEN_");

        size_t nPos1 = sStr.find(sSQL_TOKEN);
        if(nPos1 != Utf8String::npos)
        {
            Utf8String sFirst  = sStr.substr(0,nPos1);
            size_t nPos2 = sStr.find(sSQL_TOKEN,nPos1+1);
            if(nPos2 != Utf8String::npos)
            {
                Utf8String sSecond = sStr.substr(nPos1+sSQL_TOKEN.size(),nPos2-nPos1-sSQL_TOKEN.size());
                sFirst  += sSecond;
                sFirst  += sStr.substr(nPos2+sSQL_TOKEN.size());
            }
            else
                sFirst += sStr.substr(nPos1+sSQL_TOKEN.size());

            m_sErrorMessage = sFirst;
        }
        else
            m_sErrorMessage = sStr;

        Utf8String aError = s_pScanner->getErrorMessage();
        if(aError.size())
        {
            m_sErrorMessage += ", ";
            m_sErrorMessage += aError;
        }
    }
}
// -------------------------------------------------------------------------
int OSQLParser::SQLlex()
{
    return s_pScanner->SQLlex();
}

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif
