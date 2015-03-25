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
#define YYFINAL  254
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5615

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  261
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  306
/* YYNRULES -- Number of rules.  */
#define YYNRULES  638
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1046

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   494

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     4,   257,   254,     5,   255,    13,   258,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   259,     2,     8,     2,     2,     2,     2,     2,     2,
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
     251,   252,   253,   256,   260
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
     671,   675,   679,   684,   689,   691,   693,   695,   697,   699,
     703,   707,   709,   711,   713,   715,   717,   722,   724,   726,
     728,   730,   731,   734,   739,   740,   742,   749,   751,   753,
     755,   757,   759,   762,   765,   771,   773,   775,   776,   778,
     787,   789,   791,   794,   797,   799,   801,   803,   805,   806,
     808,   811,   815,   817,   821,   823,   827,   828,   830,   831,
     833,   834,   836,   841,   843,   847,   851,   853,   856,   857,
     859,   863,   865,   867,   869,   871,   874,   876,   879,   882,
     887,   889,   891,   893,   896,   898,   901,   905,   908,   911,
     915,   921,   926,   932,   934,   936,   938,   940,   942,   944,
     946,   948,   950,   952,   955,   957,   959,   960,   962,   964,
     967,   972,   978,   984,   986,   994,   995,   997,   999,  1001,
    1003,  1008,  1010,  1012,  1014,  1018,  1020,  1025,  1027,  1029,
    1034,  1039,  1040,  1042,  1044,  1046,  1048,  1050,  1052,  1054,
    1056,  1058,  1060,  1062,  1064,  1066,  1068,  1070,  1072,  1074,
    1076,  1078,  1085,  1087,  1089,  1091,  1093,  1095,  1097,  1099,
    1103,  1105,  1109,  1115,  1117,  1119,  1121,  1124,  1127,  1129,
    1133,  1137,  1139,  1143,  1147,  1149,  1151,  1153,  1156,  1159,
    1161,  1163,  1165,  1167,  1169,  1171,  1173,  1175,  1178,  1180,
    1183,  1186,  1189,  1193,  1195,  1197,  1201,  1205,  1207,  1209,
    1213,  1217,  1219,  1221,  1223,  1225,  1227,  1229,  1233,  1237,
    1239,  1241,  1244,  1246,  1249,  1251,  1253,  1255,  1263,  1265,
    1267,  1268,  1270,  1272,  1274,  1276,  1278,  1279,  1282,  1290,
    1295,  1297,  1299,  1304,  1311,  1318,  1325,  1330,  1335,  1339,
    1343,  1346,  1348,  1350,  1352,  1354,  1357,  1359,  1361,  1363,
    1367,  1371,  1375,  1377,  1378,  1380,  1382,  1386,  1389,  1391,
    1393,  1395,  1396,  1400,  1401,  1403,  1407,  1410,  1412,  1414,
    1416,  1418,  1420,  1423,  1426,  1430,  1434,  1437,  1439,  1440,
    1442,  1446,  1447,  1449,  1453,  1456,  1457,  1459,  1461,  1463,
    1465,  1467,  1472,  1477,  1480,  1484,  1488,  1491,  1496,  1501,
    1505,  1507,  1513,  1518,  1521,  1524,  1528,  1531,  1533,  1538,
    1541,  1543,  1545,  1546,  1550,  1556,  1558,  1560,  1562,  1564,
    1566,  1568,  1570,  1572,  1574,  1576,  1579,  1582,  1584,  1586,
    1588,  1593,  1598,  1603,  1605,  1607,  1613,  1618,  1620,  1623,
    1628,  1630,  1634,  1636,  1638,  1640,  1642,  1644,  1646,  1648,
    1651,  1656,  1657,  1660,  1662,  1664,  1666,  1668,  1671,  1673,
    1674,  1677,  1679,  1683,  1693,  1694,  1697,  1699,  1701,  1704,
    1706,  1708,  1711,  1712,  1715,  1717,  1721,  1722,  1726,  1728,
    1730,  1731,  1734,  1736,  1742,  1744,  1748,  1750,  1752,  1755,
    1760,  1765,  1770,  1775,  1777,  1779,  1781,  1783,  1785
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     262,     0,    -1,   263,    -1,   263,     7,    -1,   285,    -1,
     264,    -1,   265,    -1,   276,    -1,   546,    -1,    48,   119,
     496,     3,   266,     4,    -1,   267,    -1,   266,     5,   267,
      -1,   268,    -1,   273,    -1,   527,   504,   269,    -1,    -1,
     269,   272,    -1,   460,    -1,   126,    -1,   105,    85,    -1,
      23,    96,    -1,   271,    -1,    57,   362,    -1,    57,    96,
      -1,    57,   270,    -1,    42,    -1,    42,     3,   337,     4,
      -1,   109,   496,    -1,   109,   496,     3,   274,     4,    -1,
     271,     3,   274,     4,    -1,    71,    85,     3,   274,     4,
     109,   496,    -1,    71,    85,     3,   274,     4,   109,   496,
       3,   274,     4,    -1,    42,     3,   337,     4,    -1,   274,
       5,   527,    -1,   527,    -1,   275,     5,   503,    -1,   503,
      -1,    48,   133,   496,   279,    31,   306,   277,    -1,    -1,
     135,    42,   101,    -1,    -1,     3,   274,     4,    -1,    -1,
       3,   275,     4,    -1,    -1,   102,    39,   281,    -1,   282,
      -1,   281,     5,   282,    -1,   338,   283,    -1,   294,   283,
      -1,    -1,    32,    -1,    59,    -1,    -1,    23,    -1,   287,
      -1,   288,    -1,   289,    -1,   290,    -1,   295,    -1,   296,
      -1,   301,    -1,   286,    -1,   306,    -1,   286,   125,   446,
     306,    -1,    44,   136,    -1,    58,    73,   327,   304,    -1,
      68,   543,    81,   302,    -1,    80,    81,   496,   279,   291,
      -1,   132,     3,   292,     4,    -1,   293,    -1,   292,     5,
     293,    -1,   294,    -1,   472,    -1,   110,   136,    -1,   112,
     297,   307,    81,   302,   320,    -1,    -1,    27,    -1,    60,
      -1,   299,    -1,   298,     5,   299,    -1,   503,   248,   300,
      -1,   472,    -1,    57,    -1,   128,   327,   113,   298,   304,
      -1,   303,    -1,   302,     5,   303,    -1,   361,    -1,    -1,
     328,    -1,   443,    -1,   112,   297,   307,   320,    -1,   257,
      -1,   359,    -1,    -1,   309,    -1,   229,   315,   312,    -1,
      -1,   316,    -1,   214,    -1,   230,    -1,   182,    -1,   223,
      -1,    -1,   314,    -1,    68,   311,   310,   312,   231,    -1,
     362,    -1,   362,    -1,    -1,   319,    -1,    -1,   229,   458,
      -1,   228,   458,   318,    -1,   321,   304,   329,   330,   403,
     280,   317,   308,   313,    -1,    73,   322,    -1,   327,    -1,
     322,     5,   327,    -1,    -1,    31,    -1,    -1,   182,    -1,
      -1,   323,    24,   278,    -1,    -1,   231,    -1,   326,   496,
     325,    -1,   326,   358,   545,   279,    -1,   439,    -1,   134,
     337,    -1,    -1,    75,    39,   275,    -1,    -1,    76,   337,
      -1,   124,    -1,    67,    -1,   127,    -1,    96,    -1,   338,
      -1,     3,   337,     4,    -1,   294,    -1,     3,   337,     4,
      -1,   332,    -1,   332,    82,   284,   331,    -1,   334,    -1,
      23,   334,    -1,   335,    -1,   336,   247,   335,    -1,   336,
      -1,   337,   246,   336,    -1,   340,    -1,   343,    -1,   354,
      -1,   356,    -1,   357,    -1,   349,    -1,   352,    -1,   346,
      -1,   341,   293,    -1,   293,   341,   293,    -1,   341,   293,
      -1,   250,    -1,   251,    -1,   248,    -1,   249,    -1,   253,
      -1,   252,    -1,    82,   284,    -1,   284,    36,   293,   247,
     293,    -1,   293,   342,    -1,   284,    87,   473,   347,    -1,
     284,    87,   453,   347,    -1,   293,   344,    -1,   293,   345,
      -1,   344,    -1,   345,    -1,    -1,    64,   473,    -1,    82,
     284,    96,    -1,   293,   348,    -1,   348,    -1,   358,    -1,
       3,   469,     4,    -1,   284,    77,   350,    -1,   293,   351,
      -1,   351,    -1,   341,   355,   358,    -1,   293,   353,    -1,
      30,    -1,    27,    -1,   115,    -1,    66,   358,    -1,   126,
     358,    -1,     3,   447,     4,    -1,   360,    -1,   359,     5,
     360,    -1,   495,    -1,   544,    -1,   236,    -1,    20,    -1,
      21,    -1,    22,    -1,    19,    -1,   362,   240,    -1,   362,
     236,    -1,   362,    20,    -1,   362,    22,    -1,    -1,    31,
     527,    -1,   527,    -1,   140,     3,   472,    77,   472,     4,
      -1,   140,     3,   469,     4,    -1,   364,    -1,   372,    -1,
     369,    -1,   139,     3,   472,     4,    -1,   142,     3,   472,
       4,    -1,    98,     3,   472,     4,    -1,   137,     3,   472,
       4,    -1,   366,    -1,   367,    -1,   368,    -1,   464,    -1,
     158,    -1,   370,    -1,   472,    -1,   147,     3,   371,    73,
     472,     4,    -1,   374,    -1,   362,    -1,   544,    -1,    96,
      -1,    67,    -1,   124,    -1,   186,    -1,   187,    -1,   188,
      -1,   189,    -1,   190,    -1,   191,    -1,   429,    -1,   489,
      -1,   378,     3,     4,    -1,   376,     3,     4,    -1,   378,
       3,   471,     4,    -1,   377,     3,   471,     4,    -1,   379,
      -1,   162,    -1,    24,    -1,   145,    -1,   146,    -1,   381,
     203,   401,    -1,   204,     3,     4,    -1,   429,    -1,   382,
      -1,   388,    -1,   394,    -1,   397,    -1,   205,     3,   385,
       4,    -1,   544,    -1,   362,    -1,   384,    -1,   383,    -1,
      -1,     5,   391,    -1,     5,   391,     5,   392,    -1,    -1,
     393,    -1,   389,     3,   390,   386,     4,   387,    -1,   206,
      -1,   207,    -1,   472,    -1,    21,    -1,   472,    -1,   208,
     210,    -1,   209,   210,    -1,   395,     3,   472,     4,   387,
      -1,   211,    -1,   212,    -1,    -1,   399,    -1,   213,     3,
     472,     5,   398,     4,   396,   387,    -1,   384,    -1,   383,
      -1,    73,   214,    -1,    73,   215,    -1,    24,    -1,   400,
      -1,   402,    -1,   408,    -1,    -1,   404,    -1,   225,   405,
      -1,   405,     5,   406,    -1,   406,    -1,   407,    31,   408,
      -1,   400,    -1,     3,   412,     4,    -1,    -1,   413,    -1,
      -1,   414,    -1,    -1,   418,    -1,   409,   410,   280,   411,
      -1,   400,    -1,   224,    39,   415,    -1,   415,     5,   416,
      -1,   416,    -1,   503,   506,    -1,    -1,   428,    -1,   419,
     420,   417,    -1,   223,    -1,   222,    -1,   421,    -1,   423,
      -1,   220,   221,    -1,   422,    -1,    50,   182,    -1,   373,
     221,    -1,    36,   424,   247,   425,    -1,   426,    -1,   426,
      -1,   421,    -1,   220,   219,    -1,   427,    -1,   373,   219,
      -1,   216,    50,   182,    -1,   216,    75,    -1,   216,   218,
      -1,   216,   226,   217,    -1,   430,     3,   297,   470,     4,
      -1,    47,     3,   257,     4,    -1,    47,     3,   297,   470,
       4,    -1,    35,    -1,    92,    -1,    93,    -1,   118,    -1,
     164,    -1,    30,    -1,   115,    -1,    89,    -1,    90,    -1,
      74,    -1,   100,   337,    -1,   432,    -1,   440,    -1,    -1,
      79,    -1,   431,    -1,   431,   103,    -1,   327,    49,    84,
     327,    -1,   327,    94,   434,    84,   327,    -1,   327,   434,
      84,   327,   433,    -1,   435,    -1,   327,   434,    84,   327,
     131,   496,   438,    -1,    -1,    62,    -1,    63,    -1,   437,
      -1,   436,    -1,   131,     3,   274,     4,    -1,   306,    -1,
     291,    -1,   441,    -1,     3,   445,     4,    -1,   442,    -1,
     305,    83,   446,   444,    -1,   442,    -1,   443,    -1,   447,
     125,   446,   305,    -1,   447,    65,   446,   305,    -1,    -1,
      27,    -1,   445,    -1,   358,    -1,   472,    -1,   451,    -1,
     232,    -1,   233,    -1,   234,    -1,   235,    -1,   236,    -1,
     237,    -1,   238,    -1,   239,    -1,   240,    -1,   243,    -1,
     241,    -1,   242,    -1,   244,    -1,   245,    -1,    40,     3,
     449,    31,   450,     4,    -1,   373,    -1,   375,    -1,   454,
      -1,   503,    -1,   448,    -1,   528,    -1,   380,    -1,     3,
     472,     4,    -1,   452,    -1,   227,     3,     4,    -1,   501,
      13,   227,     3,     4,    -1,   453,    -1,   365,    -1,   455,
      -1,   255,   455,    -1,   254,   455,    -1,   456,    -1,   457,
     257,   456,    -1,   457,   258,   456,    -1,   457,    -1,   458,
     254,   457,    -1,   458,   255,   457,    -1,   460,    -1,   143,
      -1,   144,    -1,   241,   473,    -1,   242,   473,    -1,   459,
      -1,   461,    -1,   462,    -1,   163,    -1,   154,    -1,    53,
      -1,   152,    -1,   153,    -1,   464,   509,    -1,   464,    -1,
     158,   509,    -1,   464,   509,    -1,   158,   521,    -1,   465,
     120,   466,    -1,   467,    -1,   472,    -1,   469,     5,   472,
      -1,   469,     7,   472,    -1,   540,    -1,   470,    -1,   471,
       5,   470,    -1,   471,     7,   470,    -1,   458,    -1,   473,
      -1,   463,    -1,   474,    -1,   478,    -1,   475,    -1,   474,
     254,   478,    -1,   472,   256,   472,    -1,   240,    -1,   479,
      -1,    43,   496,    -1,   476,    -1,   476,   477,    -1,   485,
      -1,   480,    -1,   481,    -1,   141,     3,   482,    73,   473,
     486,     4,    -1,   483,    -1,   484,    -1,    -1,   487,    -1,
     489,    -1,   490,    -1,   491,    -1,   492,    -1,    -1,    70,
     472,    -1,   141,     3,   472,    73,   472,   486,     4,    -1,
     141,     3,   469,     4,    -1,   129,    -1,    91,    -1,   488,
       3,   472,     4,    -1,    46,     3,   473,   131,   496,     4,
      -1,    46,     3,   449,     5,   450,     4,    -1,   122,     3,
     473,   131,   496,     4,    -1,   123,     3,   493,     4,    -1,
     494,   472,    73,   472,    -1,   494,    73,   472,    -1,   472,
      73,   472,    -1,    73,   472,    -1,   472,    -1,    38,    -1,
      86,    -1,   121,    -1,   472,   363,    -1,   499,    -1,   498,
      -1,   497,    -1,    24,    13,   498,    -1,    24,     6,   498,
      -1,    24,    13,   499,    -1,    24,    -1,    -1,    25,    -1,
     502,    -1,   501,    13,   502,    -1,    24,   500,    -1,   257,
      -1,   501,    -1,   507,    -1,    -1,    41,   113,    24,    -1,
      -1,   477,    -1,   508,   505,   506,    -1,   516,   506,    -1,
     518,    -1,   520,    -1,   524,    -1,   525,    -1,   526,    -1,
      41,   509,    -1,   138,   509,    -1,    41,   194,   510,    -1,
     138,   194,   510,    -1,   192,   510,    -1,   515,    -1,    -1,
     510,    -1,     3,    21,     4,    -1,    -1,   512,    -1,     3,
     513,     4,    -1,    21,   514,    -1,    -1,    14,    -1,    15,
      -1,    16,    -1,    17,    -1,    18,    -1,    41,   198,   195,
     511,    -1,   138,   198,   195,   511,    -1,   199,   511,    -1,
     197,    41,   509,    -1,   197,   138,   509,    -1,    95,   509,
      -1,   197,    41,   194,   510,    -1,   197,   138,   194,   510,
      -1,    95,   194,   510,    -1,   517,    -1,   197,    41,   198,
     195,   511,    -1,    95,   198,   195,   511,    -1,   196,   511,
      -1,   232,   509,    -1,   232,   194,   510,    -1,   193,   510,
      -1,   519,    -1,   232,   198,   195,   511,    -1,   200,   511,
      -1,   522,    -1,   523,    -1,    -1,     3,    21,     4,    -1,
       3,    21,     5,    21,     4,    -1,   235,    -1,   236,    -1,
     238,    -1,   237,    -1,   239,    -1,    69,    -1,   108,    -1,
     234,    -1,   233,    -1,   241,    -1,   242,   509,    -1,   202,
     468,    -1,    24,    -1,   529,    -1,   530,    -1,   170,     3,
     469,     4,    -1,   171,     3,   472,     4,    -1,   171,     3,
     469,     4,    -1,   531,    -1,   532,    -1,   167,   542,   533,
     539,   169,    -1,   167,   537,   539,   169,    -1,   534,    -1,
     537,   534,    -1,   172,   535,   168,   540,    -1,   536,    -1,
     535,     5,   536,    -1,   294,    -1,   339,    -1,   342,    -1,
     351,    -1,   344,    -1,   348,    -1,   538,    -1,   537,   538,
      -1,   172,   337,   168,   540,    -1,    -1,   173,   540,    -1,
     541,    -1,   472,    -1,   294,    -1,    24,    -1,     6,    24,
      -1,     8,    -1,    -1,   323,    24,    -1,   337,    -1,     3,
     263,     4,    -1,    48,   181,   566,   548,   549,   100,   499,
     547,   552,    -1,    -1,   178,   559,    -1,   174,    -1,   175,
      -1,   176,    99,    -1,    80,    -1,    58,    -1,   128,   550,
      -1,    -1,    99,   551,    -1,   274,    -1,   553,   555,   556,
      -1,    -1,    70,   177,   554,    -1,   182,    -1,   183,    -1,
      -1,   172,   333,    -1,   558,    -1,   179,   180,   557,     7,
     169,    -1,   558,    -1,   557,     7,   558,    -1,   263,    -1,
     560,    -1,   559,   560,    -1,   185,   324,   323,   564,    -1,
     184,   324,   323,   565,    -1,   185,   119,   323,   561,    -1,
     184,   119,   323,   562,    -1,   563,    -1,   563,    -1,    24,
      -1,    24,    -1,    24,    -1,    24,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   258,   258,   260,   268,   269,   331,   332,   333,   337,
     348,   351,   357,   358,   362,   371,   372,   378,   381,   382,
     390,   394,   395,   399,   403,   409,   410,   416,   420,   430,
     436,   445,   457,   466,   471,   479,   484,   492,   504,   505,
     515,   516,   524,   525,   563,   564,   574,   579,   593,   601,
     610,   611,   612,   628,   629,   635,   637,   638,   639,   640,
     641,   643,   644,   648,   649,   659,   678,   687,   696,   705,
     715,   720,   741,   752,   760,   771,   782,   783,   784,   803,
     806,   812,   819,   820,   823,   833,   836,   842,   846,   847,
     853,   861,   872,   877,   880,   881,   884,   893,   894,   897,
     898,   901,   902,   905,   906,   909,   920,   923,   927,   928,
     931,   932,   940,   949,   965,   975,   978,   984,   985,   988,
     989,   992,   995,  1004,  1005,  1009,  1016,  1024,  1047,  1056,
    1057,  1065,  1066,  1074,  1075,  1076,  1077,  1080,  1081,  1088,
    1114,  1123,  1124,  1134,  1135,  1143,  1144,  1153,  1154,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1173,  1180,  1187,
    1212,  1213,  1214,  1215,  1216,  1217,  1228,  1236,  1274,  1285,
    1295,  1305,  1311,  1317,  1339,  1364,  1365,  1381,  1390,  1396,
    1412,  1416,  1424,  1433,  1439,  1455,  1464,  1489,  1490,  1491,
    1495,  1503,  1509,  1520,  1525,  1541,  1546,  1578,  1579,  1580,
    1581,  1582,  1584,  1596,  1608,  1620,  1636,  1637,  1643,  1646,
    1656,  1666,  1667,  1668,  1671,  1679,  1690,  1700,  1710,  1715,
    1720,  1727,  1732,  1740,  1741,  1772,  1784,  1785,  1788,  1789,
    1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,  1800,  1801,
    1802,  1809,  1816,  1824,  1839,  1842,  1846,  1850,  1852,  1879,
    1888,  1895,  1896,  1897,  1898,  1899,  1902,  1912,  1915,  1918,
    1919,  1922,  1923,  1929,  1939,  1940,  1944,  1956,  1957,  1960,
    1963,  1966,  1969,  1970,  1973,  1984,  1985,  1988,  1989,  1992,
    2006,  2007,  2010,  2016,  2024,  2027,  2028,  2031,  2034,  2035,
    2038,  2046,  2049,  2054,  2063,  2066,  2075,  2076,  2079,  2080,
    2083,  2084,  2087,  2093,  2096,  2105,  2108,  2113,  2121,  2122,
    2125,  2134,  2135,  2138,  2139,  2142,  2148,  2149,  2157,  2165,
    2175,  2178,  2181,  2182,  2188,  2191,  2199,  2206,  2212,  2218,
    2238,  2247,  2255,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2337,  2342,  2347,  2354,  2362,  2363,  2366,  2367,  2372,  2373,
    2381,  2393,  2403,  2412,  2417,  2431,  2432,  2433,  2436,  2437,
    2440,  2450,  2451,  2455,  2456,  2465,  2466,  2476,  2479,  2480,
    2488,  2498,  2499,  2502,  2505,  2508,  2511,  2518,  2519,  2520,
    2521,  2522,  2523,  2524,  2525,  2526,  2527,  2528,  2529,  2530,
    2531,  2534,  2546,  2547,  2548,  2549,  2550,  2551,  2552,  2553,
    2560,  2566,  2574,  2586,  2587,  2590,  2591,  2597,  2606,  2607,
    2614,  2624,  2625,  2632,  2646,  2653,  2663,  2668,  2674,  2707,
    2720,  2747,  2808,  2809,  2810,  2811,  2812,  2815,  2823,  2824,
    2833,  2839,  2848,  2855,  2860,  2863,  2867,  2880,  2907,  2910,
    2914,  2927,  2928,  2929,  2938,  2946,  2947,  2950,  2957,  2967,
    2968,  2971,  2979,  2980,  2988,  2989,  2992,  2999,  3012,  3036,
    3043,  3056,  3057,  3058,  3063,  3068,  3075,  3076,  3084,  3095,
    3105,  3106,  3109,  3119,  3129,  3141,  3153,  3163,  3171,  3178,
    3185,  3191,  3195,  3196,  3197,  3201,  3210,  3215,  3220,  3227,
    3234,  3243,  3253,  3261,  3262,  3270,  3275,  3293,  3299,  3307,
    3377,  3380,  3381,  3390,  3391,  3394,  3401,  3407,  3408,  3409,
    3410,  3411,  3414,  3420,  3426,  3433,  3440,  3446,  3449,  3450,
    3453,  3462,  3463,  3466,  3476,  3484,  3485,  3490,  3495,  3500,
    3505,  3512,  3520,  3528,  3536,  3543,  3550,  3556,  3564,  3572,
    3579,  3582,  3591,  3599,  3607,  3613,  3620,  3626,  3629,  3637,
    3645,  3646,  3649,  3650,  3657,  3691,  3692,  3693,  3694,  3695,
    3712,  3713,  3714,  3725,  3728,  3737,  3766,  3777,  3799,  3800,
    3803,  3811,  3819,  3829,  3830,  3833,  3844,  3854,  3859,  3866,
    3876,  3879,  3884,  3885,  3886,  3887,  3888,  3889,  3892,  3897,
    3904,  3914,  3915,  3923,  3927,  3930,  3933,  3946,  3952,  3976,
    3979,  3989,  4003,  4006,  4021,  4024,  4032,  4033,  4034,  4042,
    4043,  4044,  4052,  4055,  4063,  4066,  4075,  4078,  4087,  4088,
    4091,  4094,  4102,  4103,  4114,  4119,  4126,  4130,  4135,  4143,
    4151,  4159,  4167,  4177,  4180,  4183,  4186,  4189,  4192
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
  "SQL_TOKEN_INNER", "SQL_TOKEN_INSERT", "SQL_TOKEN_INTO", "SQL_TOKEN_IS",
  "SQL_TOKEN_INTERSECT", "SQL_TOKEN_JOIN", "SQL_TOKEN_KEY",
  "SQL_TOKEN_LEADING", "SQL_TOKEN_LIKE", "SQL_TOKEN_LOCAL",
  "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT", "SQL_TOKEN_LOWER", "SQL_TOKEN_MAX",
  "SQL_TOKEN_MIN", "SQL_TOKEN_NATURAL", "SQL_TOKEN_NCHAR",
  "SQL_TOKEN_NULL", "SQL_TOKEN_NUMERIC", "SQL_TOKEN_OCTET_LENGTH",
  "SQL_TOKEN_OF", "SQL_TOKEN_ON", "SQL_TOKEN_OPTION", "SQL_TOKEN_ORDER",
  "SQL_TOKEN_OUTER", "SQL_TOKEN_PRECISION", "SQL_TOKEN_PRIMARY",
  "SQL_TOKEN_PROCEDURE", "SQL_TOKEN_PUBLIC", "SQL_TOKEN_REAL",
  "SQL_TOKEN_REFERENCES", "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_SCHEMA",
  "SQL_TOKEN_SELECT", "SQL_TOKEN_SET", "SQL_TOKEN_SMALLINT",
  "SQL_TOKEN_SOME", "SQL_TOKEN_SQLCODE", "SQL_TOKEN_SQLERROR",
  "SQL_TOKEN_SUM", "SQL_TOKEN_TABLE", "SQL_TOKEN_TO", "SQL_TOKEN_TRAILING",
  "SQL_TOKEN_TRANSLATE", "SQL_TOKEN_TRIM", "SQL_TOKEN_TRUE",
  "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN",
  "SQL_TOKEN_UPDATE", "SQL_TOKEN_UPPER", "SQL_TOKEN_USAGE",
  "SQL_TOKEN_USING", "SQL_TOKEN_VALUES", "SQL_TOKEN_VIEW",
  "SQL_TOKEN_WHERE", "SQL_TOKEN_WITH", "SQL_TOKEN_WORK",
  "SQL_TOKEN_BIT_LENGTH", "SQL_TOKEN_CHAR", "SQL_TOKEN_CHAR_LENGTH",
  "SQL_TOKEN_POSITION", "SQL_TOKEN_SUBSTRING",
  "SQL_TOKEN_SQL_TOKEN_INTNUM", "SQL_TOKEN_CURRENT_DATE",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_CURDATE", "SQL_TOKEN_NOW",
  "SQL_TOKEN_EXTRACT", "SQL_TOKEN_DAYNAME", "SQL_TOKEN_DAYOFMONTH",
  "SQL_TOKEN_DAYOFWEEK", "SQL_TOKEN_DAYOFYEAR", "SQL_TOKEN_HOUR",
  "SQL_TOKEN_MINUTE", "SQL_TOKEN_MONTH", "SQL_TOKEN_MONTHNAME",
  "SQL_TOKEN_QUARTER", "SQL_TOKEN_DATEDIFF", "SQL_TOKEN_SECOND",
  "SQL_TOKEN_TIMESTAMPADD", "SQL_TOKEN_TIMESTAMPDIFF",
  "SQL_TOKEN_TIMEVALUE", "SQL_TOKEN_WEEK", "SQL_TOKEN_YEAR",
  "SQL_TOKEN_EVERY", "SQL_TOKEN_WITHIN", "SQL_TOKEN_ARRAY_AGG",
  "SQL_TOKEN_CASE", "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_NULLIF",
  "SQL_TOKEN_COALESCE", "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE",
  "SQL_TOKEN_BEFORE", "SQL_TOKEN_AFTER", "SQL_TOKEN_INSTEAD",
  "SQL_TOKEN_EACH", "SQL_TOKEN_REFERENCING", "SQL_TOKEN_BEGIN",
  "SQL_TOKEN_ATOMIC", "SQL_TOKEN_TRIGGER", "SQL_TOKEN_ROW",
  "SQL_TOKEN_STATEMENT", "SQL_TOKEN_NEW", "SQL_TOKEN_OLD",
  "SQL_TOKEN_VALUE", "SQL_TOKEN_CURRENT_CATALOG",
  "SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP", "SQL_TOKEN_CURRENT_PATH",
  "SQL_TOKEN_CURRENT_ROLE", "SQL_TOKEN_CURRENT_SCHEMA",
  "SQL_TOKEN_VARCHAR", "SQL_TOKEN_VARBINARY", "SQL_TOKEN_VARYING",
  "SQL_TOKEN_OBJECT", "SQL_TOKEN_NCLOB", "SQL_TOKEN_NATIONAL",
  "SQL_TOKEN_LARGE", "SQL_TOKEN_CLOB", "SQL_TOKEN_BLOB", "SQL_TOKEN_BIGI",
  "SQL_TOKEN_INTERVAL", "SQL_TOKEN_OVER", "SQL_TOKEN_ROW_NUMBER",
  "SQL_TOKEN_NTILE", "SQL_TOKEN_LEAD", "SQL_TOKEN_LAG",
  "SQL_TOKEN_RESPECT", "SQL_TOKEN_IGNORE", "SQL_TOKEN_NULLS",
  "SQL_TOKEN_FIRST_VALUE", "SQL_TOKEN_LAST_VALUE", "SQL_TOKEN_NTH_VALUE",
  "SQL_TOKEN_FIRST", "SQL_TOKEN_LAST", "SQL_TOKEN_EXCLUDE",
  "SQL_TOKEN_OTHERS", "SQL_TOKEN_TIES", "SQL_TOKEN_FOLLOWING",
  "SQL_TOKEN_UNBOUNDED", "SQL_TOKEN_PRECEDING", "SQL_TOKEN_RANGE",
  "SQL_TOKEN_ROWS", "SQL_TOKEN_PARTITION", "SQL_TOKEN_WINDOW",
  "SQL_TOKEN_NO", "SQL_TOKEN_GETECCLASSID", "SQL_TOKEN_LIMIT",
  "SQL_TOKEN_OFFSET", "SQL_TOKEN_NEXT", "SQL_TOKEN_ONLY",
  "SQL_TOKEN_BINARY", "SQL_TOKEN_BOOLEAN", "SQL_TOKEN_DOUBLE",
  "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT", "SQL_TOKEN_INT32",
  "SQL_TOKEN_LONG", "SQL_TOKEN_INT64", "SQL_TOKEN_STRING",
  "SQL_TOKEN_DATE", "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_DATETIME",
  "SQL_TOKEN_POINT2D", "SQL_TOKEN_POINT3D", "SQL_TOKEN_OR",
  "SQL_TOKEN_AND", "SQL_EQUAL", "SQL_GREAT", "SQL_LESS", "SQL_NOTEQUAL",
  "SQL_GREATEQ", "SQL_LESSEQ", "'+'", "'-'", "SQL_CONCAT", "'*'", "'/'",
  "'='", "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql_single_statement",
  "sql", "schema_element", "base_table_def",
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
  "general_value_spec", "fct_spec", "function_name0", "function_name12",
  "function_name", "date_function_0Argument", "window_function",
  "window_function_type", "ntile_function",
  "dynamic_parameter_specification", "simple_value_specification",
  "number_of_tiles", "opt_lead_or_lag_function", "opt_null_treatment",
  "lead_or_lag_function", "lead_or_lag", "lead_or_lag_extent", "offset",
  "default_expression", "null_treatment", "first_or_last_value_function",
  "first_or_last_value", "opt_from_first_or_last", "nth_value_function",
  "nth_row", "from_first_or_last", "window_name",
  "window_name_or_specification", "in_line_window_specification",
  "opt_window_clause", "window_clause", "window_definition_list",
  "window_definition", "new_window_name", "window_specification",
  "opt_existing_window_name", "opt_window_partition_clause",
  "opt_window_frame_clause", "window_specification_details",
  "existing_window_name", "window_partition_clause",
  "window_partition_column_reference_list",
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
  "value_exp_commalist", "function_arg", "function_args_commalist",
  "value_exp", "string_value_exp", "char_value_exp", "concatenation",
  "char_primary", "collate_clause", "char_factor", "string_value_fct",
  "bit_value_fct", "bit_substring_fct", "bit_value_exp", "bit_factor",
  "bit_primary", "char_value_fct", "for_length", "char_substring_fct",
  "upper_lower", "fold", "form_conversion", "char_translation", "trim_fct",
  "trim_operands", "trim_spec", "derived_column", "table_node",
  "catalog_name", "schema_name", "table_name", "opt_column_array_idx",
  "property_path", "property_path_entry", "column_ref", "data_type",
  "opt_char_set_spec", "opt_collate_clause", "predefined_type",
  "character_string_type", "opt_paren_precision", "paren_char_length",
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
     489,   490,   491,   492,    43,    45,   493,    42,    47,    61,
     494
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   261,   262,   262,   263,   263,   264,   264,   264,   265,
     266,   266,   267,   267,   268,   269,   269,   270,   271,   271,
     272,   272,   272,   272,   272,   272,   272,   272,   272,   273,
     273,   273,   273,   274,   274,   275,   275,   276,   277,   277,
     278,   278,   279,   279,   280,   280,   281,   281,   282,   282,
     283,   283,   283,   284,   284,   285,   285,   285,   285,   285,
     285,   285,   285,   286,   286,   287,   288,   289,   290,   291,
     292,   292,   293,   294,   295,   296,   297,   297,   297,   298,
     298,   299,   300,   300,   301,   302,   302,   303,   304,   304,
     305,   306,   307,   307,   308,   308,   309,   310,   310,   311,
     311,   312,   312,   313,   313,   314,   315,   316,   317,   317,
     318,   318,   319,   320,   321,   322,   322,   323,   323,   324,
     324,   325,   325,   326,   326,   327,   327,   327,   328,   329,
     329,   330,   330,   331,   331,   331,   331,   332,   332,   332,
     333,   334,   334,   335,   335,   336,   336,   337,   337,   338,
     338,   338,   338,   338,   338,   338,   338,   339,   340,   340,
     341,   341,   341,   341,   341,   341,   341,   342,   343,   344,
     345,   346,   346,   346,   346,   347,   347,   348,   349,   349,
     350,   350,   351,   352,   352,   353,   354,   355,   355,   355,
     356,   357,   358,   359,   359,   360,   361,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   363,   363,   363,   364,
     364,   365,   365,   365,   366,   366,   367,   368,   369,   369,
     369,   370,   370,   371,   371,   372,   373,   373,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   375,   375,
     375,   375,   375,   375,   376,   377,   378,   379,   379,   380,
     381,   381,   381,   381,   381,   381,   382,   383,   384,   385,
     385,   386,   386,   386,   387,   387,   388,   389,   389,   390,
     391,   392,   393,   393,   394,   395,   395,   396,   396,   397,
     398,   398,   399,   399,   400,   401,   401,   402,   403,   403,
     404,   405,   405,   406,   407,   408,   409,   409,   410,   410,
     411,   411,   412,   413,   414,   415,   415,   416,   417,   417,
     418,   419,   419,   420,   420,   421,   421,   421,   422,   423,
     424,   425,   426,   426,   426,   427,   428,   428,   428,   428,
     429,   429,   429,   430,   430,   430,   430,   430,   430,   430,
     431,   431,   431,   432,   433,   433,   434,   434,   434,   434,
     435,   436,   436,   436,   437,   438,   438,   438,   439,   439,
     440,   441,   441,   442,   442,   443,   443,   444,   445,   445,
     445,   446,   446,   447,   448,   449,   450,   451,   451,   451,
     451,   451,   451,   451,   451,   451,   451,   451,   451,   451,
     451,   452,   453,   453,   453,   453,   453,   453,   453,   453,
     453,   454,   454,   455,   455,   456,   456,   456,   457,   457,
     457,   458,   458,   458,   459,   460,   460,   460,   460,   461,
     462,   463,   464,   464,   464,   464,   464,   465,   466,   466,
     467,   467,   468,   468,   469,   469,   469,   470,   471,   471,
     471,   472,   472,   472,   473,   474,   474,   475,   475,   476,
     476,   477,   478,   478,   479,   479,   480,   481,   482,   483,
     484,   485,   485,   485,   485,   485,   486,   486,   487,   487,
     488,   488,   489,   490,   490,   491,   492,   493,   493,   493,
     493,   493,   494,   494,   494,   495,   496,   496,   496,   497,
     497,   498,   499,   500,   500,   501,   501,   502,   502,   503,
     504,   505,   505,   506,   506,   507,   507,   507,   507,   507,
     507,   507,   508,   508,   508,   508,   508,   508,   509,   509,
     510,   511,   511,   512,   513,   514,   514,   514,   514,   514,
     514,   515,   515,   515,   516,   516,   516,   516,   516,   516,
     516,   517,   517,   517,   518,   518,   518,   518,   519,   519,
     520,   520,   521,   521,   521,   522,   522,   522,   522,   522,
     523,   523,   523,   524,   525,   525,   526,   527,   528,   528,
     529,   529,   529,   530,   530,   531,   532,   533,   533,   534,
     535,   535,   536,   536,   536,   536,   536,   536,   537,   537,
     538,   539,   539,   540,   541,   542,   543,   544,   544,   545,
     545,   263,   263,   546,   547,   547,   548,   548,   548,   549,
     549,   549,   550,   550,   551,   552,   553,   553,   554,   554,
     555,   555,   556,   556,   557,   557,   558,   559,   559,   560,
     560,   560,   560,   561,   562,   563,   564,   565,   566
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
       3,     3,     4,     4,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     0,     2,     4,     0,     1,     6,     1,     1,     1,
       1,     1,     2,     2,     5,     1,     1,     0,     1,     8,
       1,     1,     2,     2,     1,     1,     1,     1,     0,     1,
       2,     3,     1,     3,     1,     3,     0,     1,     0,     1,
       0,     1,     4,     1,     3,     3,     1,     2,     0,     1,
       3,     1,     1,     1,     1,     2,     1,     2,     2,     4,
       1,     1,     1,     2,     1,     2,     3,     2,     2,     3,
       5,     4,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     0,     1,     1,     2,
       4,     5,     5,     1,     7,     0,     1,     1,     1,     1,
       4,     1,     1,     1,     3,     1,     4,     1,     1,     4,
       4,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     6,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     5,     1,     1,     1,     2,     2,     1,     3,
       3,     1,     3,     3,     1,     1,     1,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     2,
       2,     2,     3,     1,     1,     3,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     3,     3,     1,
       1,     2,     1,     2,     1,     1,     1,     7,     1,     1,
       0,     1,     1,     1,     1,     1,     0,     2,     7,     4,
       1,     1,     4,     6,     6,     6,     4,     4,     3,     3,
       2,     1,     1,     1,     1,     2,     1,     1,     1,     3,
       3,     3,     1,     0,     1,     1,     3,     2,     1,     1,
       1,     0,     3,     0,     1,     3,     2,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     2,     1,     0,     1,
       3,     0,     1,     3,     2,     0,     1,     1,     1,     1,
       1,     4,     4,     2,     3,     3,     2,     4,     4,     3,
       1,     5,     4,     2,     2,     3,     2,     1,     4,     2,
       1,     1,     0,     3,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       4,     4,     4,     1,     1,     5,     4,     1,     2,     4,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       4,     0,     2,     1,     1,     1,     1,     2,     1,     0,
       2,     1,     3,     9,     0,     2,     1,     1,     2,     1,
       1,     2,     0,     2,     1,     3,     0,     3,     1,     1,
       0,     2,     1,     5,     1,     3,     1,     1,     2,     4,
       4,     4,     4,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      53,    53,     0,   598,   201,   198,   199,   200,    53,   493,
     338,   333,     0,     0,     0,     0,     0,     0,     0,   230,
       0,     0,    53,   471,   334,   335,   229,     0,     0,    76,
     339,   336,     0,     0,   231,     0,   123,   470,     0,     0,
       0,     0,     0,   415,   416,   247,   248,     0,   245,   337,
       0,     0,     0,   232,   233,   234,   235,   236,   237,     0,
       0,   267,   268,   275,   276,     0,     0,   197,   449,     0,
       0,   162,   163,   160,   161,   165,   164,     0,     0,   498,
       0,     2,     5,     6,     7,     0,     4,    62,    55,    56,
      57,    58,    53,   139,    59,    60,    61,    63,   141,   143,
     145,   147,   601,   137,   149,     0,   150,   173,   174,   156,
     179,   154,   184,   155,   151,   152,   153,   374,   227,   211,
     404,   218,   219,   220,   213,   212,   392,   226,   393,     0,
       0,     0,   244,   398,     0,   252,   253,     0,   254,     0,
     255,   238,     0,   396,   400,   403,   394,   405,   408,   411,
     441,   419,   414,   420,   421,   443,    73,   442,   444,   446,
     452,   445,   450,   455,   456,   454,   461,     0,   239,   463,
     464,   465,   499,   495,   395,   397,   568,   569,   573,   574,
     228,     8,    53,     0,     0,   362,     0,    63,     0,   363,
     365,   368,   373,     0,    73,   597,    53,    54,   144,   494,
     497,     0,    65,     0,    76,     0,     0,     0,   123,     0,
     190,   596,     0,     0,   166,     0,    74,    77,    78,     0,
       0,     0,   191,   124,     0,   346,   353,   359,   358,   127,
       0,     0,     0,   460,     0,     0,     0,    53,   595,   591,
     588,     0,     0,     0,     0,     0,     0,     0,     0,   417,
     418,   407,   239,   406,     1,     3,     0,     0,   371,     0,
       0,   168,   171,   172,   178,   183,   186,    53,    53,    53,
     159,    72,   204,   205,   203,   202,     0,     0,     0,     0,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
     453,     0,     0,   373,     0,   602,   371,   138,   192,   371,
     371,   399,    53,    76,   361,     0,     0,   375,     0,   442,
       0,     0,   492,     0,   488,   487,   486,    42,   638,     0,
      88,     0,     0,    42,   177,     0,   498,     0,    93,   193,
     206,   195,   442,   482,     0,   483,   484,   481,     0,     0,
     599,   121,     0,   342,   347,   340,   341,   346,     0,   348,
       0,     0,     0,     0,   434,     0,   434,     0,   458,   459,
       0,   424,   425,   426,   423,   222,   422,   223,     0,   221,
     224,     0,     0,     0,     0,   589,     0,    53,   591,   577,
       0,     0,   434,     0,   434,   250,   258,   260,   259,     0,
     257,     0,   401,     0,   182,   180,   175,   175,   372,     0,
       0,   188,   187,   189,   158,     0,     0,   146,   148,   241,
     438,     0,   594,   437,   593,   240,     0,   296,   284,   285,
     249,   286,   287,   261,   269,     0,     0,   409,   410,   412,
     413,   448,   447,   462,   451,     0,   493,     0,   496,   364,
       0,    70,     0,     0,     0,     0,     0,     0,     0,   331,
       0,     0,     0,     0,     0,     0,   606,   607,     0,     0,
      53,    66,    89,     0,    67,    85,    87,   196,     0,   216,
     123,     0,    91,    88,     0,   567,     0,   485,   208,     0,
     480,     0,   476,     0,     0,   118,     0,    42,     0,   125,
     123,     0,    88,    79,   499,     0,   349,   123,   217,   214,
     210,     0,     0,     0,   469,     0,     0,   215,     0,     0,
     592,   576,    72,   583,     0,   584,   173,   179,   184,     0,
     580,     0,   578,   570,   572,   571,   256,     0,     0,     0,
     170,   169,    64,     0,   185,   134,   136,   133,   135,   142,
     243,     0,     0,   242,   303,   298,     0,   297,     0,     0,
     264,     0,   472,     0,    69,     0,   367,   366,   370,    90,
     369,     0,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   387,   388,   386,   389,   390,     0,   376,     0,     0,
     332,     0,   490,   492,   489,   491,     0,     0,     0,    18,
       0,    10,    12,     0,    13,     0,     0,    36,     0,   608,
     610,   609,   612,     0,   128,     0,    68,   114,   115,     0,
     129,   194,   207,     0,   479,   478,     0,   600,   126,    40,
     350,   123,     0,    84,     0,     0,   346,   435,   436,     0,
     466,   442,     0,   590,   159,    53,     0,   575,   281,   280,
       0,   181,   176,     0,   439,   440,     0,    44,   299,   295,
     270,   262,   264,     0,     0,   274,   265,   330,   402,    71,
     391,   474,   473,     0,    53,     0,    19,     9,     0,     0,
     518,   560,   518,   561,   518,     0,     0,   521,     0,   521,
     521,     0,   518,   563,   562,   555,   556,   558,   557,   559,
     564,   518,    15,   500,   501,   517,   503,   540,   507,   547,
     508,   550,   551,   509,   510,   511,    43,     0,    38,     0,
     611,     0,    86,   123,    75,     0,   131,   475,   477,     0,
     122,   351,    80,    83,    81,    82,    53,     0,   344,   352,
     345,   209,     0,     0,     0,   225,     0,   582,     0,   586,
     587,   585,   581,   579,   277,   167,     0,     0,   300,     0,
     266,   272,   273,   492,     0,     0,    11,     0,    34,     0,
       0,     0,   512,   519,     0,     0,   536,     0,     0,   513,
     516,   546,     0,   543,   522,   518,   518,   533,   549,   552,
     518,     0,   433,   566,     0,     0,   544,   565,    14,     0,
     503,   504,   506,    35,     0,    37,   614,   613,   604,   116,
       0,    53,   288,     0,   343,     0,   355,   467,   468,   457,
       0,   157,     0,   264,   278,   304,   306,   503,    53,   312,
     311,   302,   301,     0,   263,   271,    32,     0,    29,     0,
       0,   514,   521,   539,   521,   515,   521,   525,     0,     0,
       0,   534,     0,   535,     0,   431,   430,     0,   545,   521,
       0,    25,     0,     0,    21,    16,     0,   505,     0,     0,
     616,   130,   132,     0,    44,   289,    41,     0,   356,   357,
     354,   282,   283,   279,     0,   307,    45,    46,    50,    50,
       0,     0,     0,     0,   308,   313,   316,   314,     0,    33,
     520,   531,   542,   532,   526,   527,   528,   529,   530,   524,
     523,   537,   521,   538,     0,   518,   428,   432,   548,    20,
      53,    23,    24,    22,    17,    27,   502,    39,   119,   119,
     605,   627,     0,   603,   620,   294,   290,   292,     0,   108,
     360,   305,    53,    51,    52,    49,    48,     0,     0,   322,
       0,   320,   324,   317,   315,   318,     0,   310,   309,     0,
     541,   553,     0,   429,     0,     0,   117,   120,   117,   117,
     117,   628,     0,     0,    53,     0,     0,     0,    94,   109,
      47,   323,   325,     0,     0,   327,   328,     0,    30,     0,
      26,     0,     0,     0,     0,     0,   618,   619,   617,    53,
     621,     0,   626,   615,   622,   291,   293,   110,     0,   103,
      95,   319,   321,   326,   329,     0,   554,    28,   635,   632,
     634,   637,   630,   631,   633,   636,   629,     0,    53,     0,
     112,     0,   106,     0,   113,   104,     0,   140,     0,   624,
     111,   101,   102,    96,    99,   100,    97,    31,    53,     0,
      98,   107,   623,   625,     0,   105
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    80,   992,    82,    83,   590,   591,   592,   788,   912,
     593,   855,   594,   757,   596,    84,   795,   720,   455,   748,
     876,   877,   935,    85,    86,    87,    88,    89,    90,    91,
     185,   440,    92,    93,    94,    95,   219,   492,   493,   724,
      96,   464,   465,   461,   186,   304,   327,   999,  1000,  1039,
    1036,  1033,  1024,  1025,  1021,  1040,   968,  1020,   969,   472,
     473,   607,   486,   958,   489,   224,   225,   462,   716,   802,
     539,    98,   990,    99,   100,   101,   102,   103,   513,   104,
     105,   515,   106,   107,   108,   109,   530,   110,   111,   394,
     112,   113,   266,   114,   405,   115,   116,   117,   328,   329,
     466,   118,   477,   119,   120,   121,   122,   123,   124,   367,
     368,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   387,   388,   389,   549,   655,   136,   137,   423,
     651,   824,   656,   138,   139,   813,   140,   640,   814,   925,
     420,   421,   864,   865,   926,   927,   928,   422,   545,   647,
     821,   546,   547,   648,   815,   816,   947,   822,   823,   884,
     939,   886,   887,   940,  1001,   941,   942,   948,   141,   142,
     349,   728,   729,   350,   226,   227,   228,   870,   229,   730,
     189,   190,   191,   557,   192,   399,   193,   143,   306,   576,
     577,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   369,   781,   907,   782,   783,   353,   410,
     411,   156,   157,   158,   159,   160,   791,   161,   162,   163,
     164,   357,   358,   359,   165,   733,   166,   167,   168,   169,
     170,   171,   338,   339,   331,   313,   314,   315,   316,   200,
     172,   173,   174,   692,   790,   792,   693,   694,   762,   763,
     773,   774,   838,   899,   695,   696,   697,   698,   699,   700,
     845,   701,   702,   703,   704,   705,   758,   175,   176,   177,
     178,   179,   378,   379,   519,   520,   239,   240,   376,   413,
     414,   241,   212,   180,   487,   181,   860,   459,   603,   710,
     797,   923,   924,   988,   964,   993,  1028,   994,   920,   921,
    1013,  1009,  1010,  1016,  1012,   319
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -870
static const yytype_int16 yypact[] =
{
    1596,   837,    96,  -870,  -870,  -870,  -870,  -870,  2102,    94,
    -870,  -870,   100,    12,   159,   178,   244,   119,   203,  -870,
     191,   140,   226,  -870,  -870,  -870,  -870,   271,   223,    75,
    -870,  -870,   412,   427,  -870,   203,   212,  -870,   443,   453,
     461,   466,   475,  -870,  -870,  -870,  -870,   498,  -870,  -870,
    3753,   525,   536,  -870,  -870,  -870,  -870,  -870,  -870,   556,
     565,  -870,  -870,  -870,  -870,   574,   581,  -870,  -870,  4831,
    4831,  -870,  -870,  -870,  -870,  -870,  -870,  5358,  5358,  -870,
     488,   524,  -870,  -870,  -870,   349,  -870,   469,  -870,  -870,
    -870,  -870,    77,    86,  -870,  -870,  -870,  -870,   520,  -870,
    -870,   339,   358,  -870,  -870,  4831,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,   118,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,   605,
     621,   623,  -870,  -870,   413,  -870,  -870,   638,  -870,   640,
    -870,   463,   661,  -870,  -870,  -870,  -870,  -870,  -870,   130,
     220,  -870,  -870,  -870,  -870,  -870,   426,  -870,   439,  -870,
     657,  -870,  -870,  -870,  -870,  -870,  -870,   701,   662,  -870,
    -870,  -870,   693,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,   837,   705,   706,  -870,   626,   182,    22,  -870,
    -870,   628,  -870,   310,    16,  -870,  1849,  -870,  -870,  -870,
    -870,  4831,  -870,  4831,    67,   688,   688,   689,   212,    81,
    -870,  -870,   633,   688,   619,  4831,  -870,  -870,  -870,  5044,
    4831,  3327,  -870,  -870,   115,   327,  -870,  -870,  -870,  -870,
    4831,  4831,  4831,  4831,  4831,  3114,  3540,  2355,  -870,   309,
    -870,   552,  4831,  4831,   738,   104,  4831,   739,   426,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,   741,  4831,   718,   107,
    3979,  -870,  -870,  -870,  -870,  -870,  -870,   226,  2355,  2355,
    -870,  -870,  -870,  -870,  -870,  -870,   742,  4831,  4192,   148,
    4831,  4831,    75,  5257,  5257,  5257,  5257,  4831,   316,   688,
    -870,  4831,    84,   743,  4831,  -870,   718,  -870,  -870,   718,
     718,  -870,  1849,    75,  -870,    22,   719,   426,   744,   620,
     748,  4831,   449,   751,  -870,  -870,  -870,   752,  -870,   -60,
     511,    81,   504,   752,  -870,    23,   351,   387,   755,  -870,
      27,  -870,   625,  -870,  4831,  -870,  -870,   -48,   753,  4405,
     441,   441,   677,  -870,  -870,  -870,  -870,   462,    13,   659,
     679,    26,    28,   314,   -21,   509,   -26,   691,  -870,  -870,
      32,  -870,  -870,  -870,  -870,  -870,  -870,  -870,   692,  -870,
     426,  3540,    16,  -100,  4831,  -870,   597,  2355,   594,  -870,
     552,   571,   426,   576,    37,  -870,   118,  -870,  -870,   764,
    -870,    19,  -870,  3540,  -870,  -870,   154,     5,  -870,   660,
    4831,  -870,  -870,  -870,  -870,   203,   381,  -870,   339,  -870,
    -870,   602,   426,  -870,  -870,  -870,   606,   745,  -870,  -870,
    -870,  -870,  -870,   766,   426,    38,  4831,  -870,  -870,   130,
     130,  -870,  -870,  -870,  -870,    39,   750,   770,  -870,  -870,
     516,  -870,    81,    81,    81,  5044,   496,   496,   688,  -870,
     772,   754,   756,    62,    13,   746,  -870,  -870,   680,    49,
    2355,  -870,  -870,    71,   776,  -870,  -870,  -870,   650,  -870,
     212,   504,  -870,   649,  4831,  -870,   760,  -870,  -870,   688,
     426,  4831,  -870,  4831,   -25,  -870,   761,   752,   762,  -870,
     212,   703,    68,  -870,   775,   541,  -870,   212,  -870,  -870,
    -870,  4831,  4831,  4831,  -870,  4831,  4831,  -870,  4831,  4831,
    -870,  -870,    69,  -870,  4831,  -870,   785,   786,   787,    55,
    -870,   624,  -870,  -870,  -870,  -870,  -870,   104,   610,  4831,
    -870,  -870,  -870,   547,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  4831,  4831,  -870,  -870,   573,   791,  -870,   777,   795,
     326,   796,  -870,   797,  -870,  4831,  -870,  -870,   626,  -870,
     626,   730,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,   800,  -870,   801,   802,
    -870,   794,  -870,   794,  -870,  -870,   805,   724,   725,  -870,
     562,  -870,  -870,   808,  -870,   484,   592,  -870,   660,  -870,
    -870,  -870,   713,   714,   358,   504,  -870,   810,   613,    74,
     747,  -870,  -870,   809,   426,   426,  4831,  -870,  -870,   813,
     613,   212,    13,  -870,    13,  4618,   544,   426,   426,    51,
     -24,    91,    53,  -870,    60,  2861,  4831,  -870,  -870,  -870,
     814,  -870,   561,  4831,  -870,  -870,   780,   721,  -870,  -870,
    -870,   815,   326,   614,   615,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,   803,  2355,   818,  -870,  -870,    62,   760,
      50,  -870,   175,  -870,   176,   823,   823,   825,    48,   825,
     825,   300,   186,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,   823,  -870,  -870,   788,  -870,   657,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,    13,   695,   760,
    -870,   803,  -870,   212,  -870,   793,   757,  -870,   426,   760,
    -870,   613,  -870,  -870,  -870,   426,  2355,   361,  -870,  -870,
    -870,  -870,  4831,   830,   831,  -870,   284,  -870,  4831,  -870,
    -870,  -870,  -870,  -870,   763,  -870,    13,   798,   398,  4831,
    -870,  -870,  -870,  -870,    34,   760,  -870,   627,  -870,   817,
     823,   644,  -870,  -870,   823,   646,  -870,   823,   647,  -870,
    -870,  -870,   826,  -870,  -870,   188,    79,  -870,  -870,   841,
     823,   726,  -870,  -870,   823,   653,  -870,  -870,   371,   736,
     657,  -870,  -870,  -870,   811,  -870,   845,  -870,   673,   613,
      13,  2355,   629,   631,   358,   760,   486,   426,  -870,  -870,
    4831,  -870,   425,   326,  -870,   847,  -870,   657,  2608,  -870,
    -870,  -870,  -870,   383,  -870,   426,  -870,   643,  -870,   760,
     851,  -870,   825,  -870,   825,  -870,   825,   528,   858,   823,
     668,  -870,   823,  -870,   843,  -870,   749,   340,  -870,   825,
     769,   863,   346,   688,  -870,  -870,   844,  -870,   773,   474,
     806,   865,   358,   745,   721,  -870,  -870,   645,  -870,  -870,
    -870,  -870,  -870,  -870,    13,  -870,   866,  -870,    98,    73,
     465,   696,   652,   654,   663,  -870,  -870,  -870,   771,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,   825,  -870,   656,   823,  -870,  -870,  -870,  -870,
    2355,  -870,  -870,   118,  -870,   879,  -870,  -870,   -13,    -8,
     474,  -870,   709,  -870,   715,  -870,   884,  -870,   859,   664,
    -870,  -870,  2608,  -870,  -870,  -870,  -870,   298,   319,  -870,
     651,  -870,  -870,  -870,  -870,  -870,   126,  -870,  -870,   688,
    -870,  -870,   870,  -870,    40,   760,   868,  -870,   868,   868,
     868,  -870,   485,   891,  1090,   745,   893,  5257,   671,  -870,
    -870,  -870,  -870,   465,   720,  -870,  -870,   684,   894,   902,
    -870,   665,   883,   885,   883,   886,  -870,  -870,  -870,  2355,
    -870,   728,  -870,  -870,  -870,  -870,  -870,   168,    56,   846,
    -870,  -870,  -870,  -870,  -870,   760,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,    45,  1596,  5257,
    -870,   -69,   118,   -10,  -870,  -870,   667,  -870,   905,  -870,
     220,  -870,  -870,  -870,  -870,  -870,    56,  -870,  1343,   -69,
    -870,   118,  -870,  -870,   682,  -870
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -870,  -870,    63,  -870,  -870,  -870,   248,  -870,  -870,  -870,
     127,  -870,  -870,  -674,   120,  -870,  -870,  -870,  -253,    54,
    -870,   -11,    43,    -5,  -870,  -870,  -870,  -870,  -870,  -870,
     455,  -870,   -96,   -44,  -870,  -870,  -116,  -870,   302,  -870,
    -870,   454,   321,  -321,   230,     0,   482,  -870,  -870,  -870,
    -870,  -108,  -870,  -870,  -870,  -870,  -870,  -870,  -870,   323,
    -870,  -870,  -331,    17,  -870,  -870,  -180,  -870,  -870,  -870,
    -870,  -870,  -870,   926,   669,   670,     1,  -733,  -870,  -870,
     -81,   848,  -870,   -80,   849,  -870,   545,   -77,  -870,  -870,
     -71,  -870,  -870,  -870,  -870,  -870,  -870,    15,  -870,   464,
    -870,  -240,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -727,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,   416,   417,  -870,  -870,  -580,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -212,
    -870,  -870,  -870,  -870,  -870,   -19,  -870,   -18,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,    76,  -870,  -870,  -870,  -870,
     128,  -870,  -870,  -870,  -870,   -20,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,   607,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,   514,   235,  -870,  -102,   200,   636,  -870,   759,   521,
    -870,  -870,   710,  -870,   611,   407,   410,  -869,  -870,   106,
    -870,  -870,  -870,  -622,  -870,  -870,  -870,  -870,  -150,  -282,
     694,    21,   -62,  -870,  -870,  -870,   827,   683,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,   342,  -870,  -870,   -74,  -870,
    -870,  -870,  -870,  -870,  -870,  -190,  -870,   247,  -434,  -870,
    -317,  -279,  -309,  -870,  -870,  -689,  -870,  -870,  -620,  -333,
    -453,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -311,  -870,  -870,  -870,
    -870,  -870,  -870,   590,  -870,   350,   734,  -173,   608,  -360,
    -870,  -870,  -870,  -205,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -848,  -870,    72,
    -870,  -870,     4,  -870,  -870,  -870
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -588
static const yytype_int16 yytable[] =
{
      97,   187,   188,   252,   252,   386,   238,   249,   250,   270,
     488,   260,   262,   438,   510,   264,   317,   214,   585,   478,
     301,   265,   194,   323,   527,   481,   297,   469,   320,   450,
     498,   494,   499,   210,   341,   796,   507,   436,   826,   495,
     390,   525,   550,   552,   980,   803,   732,   505,   616,  1027,
     222,   475,   766,   759,   769,   731,   503,   735,   476,   780,
     635,   271,   786,    81,   184,  -157,   375,   419,   509,   529,
     468,   787,   750,   622,  -582,     4,     5,     6,     7,   605,
     293,   827,   759,   355,   321,   879,   475,   259,   311,   775,
     248,   248,   381,   383,   217,  -466,   883,  -246,   997,   434,
     197,   857,   217,   201,   586,   933,   956,   600,   436,   -72,
       2,   959,     3,  1031,   456,   457,   458,   467,   209,   199,
     195,   -72,   -72,     4,     5,     6,     7,   218,   875,   601,
     933,   867,   934,   587,   -72,   218,   299,   494,   272,   312,
     273,   309,   595,   400,   551,   597,   269,   470,   202,   633,
    1030,   417,   610,   938,  1032,   841,   843,   934,   332,    22,
     846,   732,   203,   -72,   404,   612,   426,   588,   -72,   957,
    1029,   623,   418,   -72,   957,   -72,   974,   602,   759,   759,
     -72,   204,   187,   188,   256,   -72,   776,   445,   589,   759,
    1043,   759,   208,   303,   257,   397,   300,   305,   441,   879,
     293,   975,   460,   194,  1034,   544,   209,   375,   287,   252,
     252,   252,   252,   183,   433,   211,   271,   194,   529,   293,
    1035,   213,   307,   636,   307,   906,   777,   778,  -157,   585,
     287,   287,   287,   873,   618,   287,   325,  -139,   373,   340,
     330,   248,   337,   528,   760,   184,   938,  -361,   761,   197,
     271,   351,   352,   354,   356,   360,   370,   372,   579,   644,
     645,  -442,   406,   382,   384,  -361,   467,   391,   269,   293,
      79,   395,   287,   842,   215,   287,   743,   798,   248,   287,
     269,   981,   287,   287,   287,   953,   269,   386,   287,   613,
     608,   269,    67,   287,   287,   287,   514,   516,   412,   412,
     517,   424,   425,   305,   533,   494,   518,   287,   431,   287,
     620,   437,   435,   495,   298,  -139,  -139,   626,   500,   501,
     400,   502,   390,   194,   310,    71,    72,    73,    74,    75,
      76,  1026,   412,   512,   -72,   -72,   -72,   -72,   -72,   -72,
      67,    79,   770,   771,   976,   438,   -72,   -72,   -72,   -72,
     -72,   -72,   977,   361,   274,   480,   271,   595,   275,   216,
     484,   256,    14,   205,   805,     4,     5,     6,     7,   764,
     767,   810,   259,   765,   768,   299,   342,   206,   373,   891,
     784,   892,   839,   893,   785,   312,   840,   283,   284,     2,
     494,     3,   372,   361,   850,   412,   908,  1019,   793,   532,
     467,   343,     4,     5,     6,     7,   344,    23,  -403,  -403,
    -403,  -403,  -403,   851,   382,   220,   345,   346,   634,   880,
     534,   347,   285,   286,   -92,   207,   256,   831,   852,   494,
     221,   833,   -92,   881,   835,   300,   257,   817,    32,    33,
     348,   721,   911,   223,   631,    37,   230,   412,   535,   950,
      19,   848,   362,   363,   364,   451,   231,    41,   779,   659,
     470,   604,   452,   366,   232,  -117,   330,   642,   471,   233,
     271,     2,   485,     3,   285,   286,   588,   536,   234,    26,
     853,   237,   374,   494,     4,     5,     6,     7,   254,    43,
      44,   597,   362,   363,   364,   330,   442,   589,   905,   443,
     444,   235,   614,   366,   615,   537,   901,    34,   538,   903,
       2,   271,     3,   504,   501,   881,   502,   971,   889,   944,
     554,   555,   627,   628,   629,   670,   630,   248,   242,   632,
     412,   255,    19,   799,   653,   654,   343,   806,   972,   243,
     945,   344,   894,   895,   896,   897,   898,   745,   868,   869,
     248,   345,   346,   671,   738,   739,    68,   494,   740,   244,
     342,    26,   412,   412,   741,   817,   667,   668,   245,    53,
      54,    55,    56,    57,    58,   523,   501,   246,   502,   672,
     524,   501,    67,   502,   247,   343,   268,    69,    70,    34,
     344,   737,   673,   342,   258,  -346,   706,   707,   708,   271,
     345,   346,   267,   882,   269,   347,   540,   541,   276,   542,
     543,   541,   913,   542,   641,   501,   279,   502,   343,    67,
     819,   820,   674,   344,   277,   982,   278,   983,   984,   985,
     736,   828,   829,   345,   346,   866,   829,   718,   347,   871,
     872,   280,   811,   281,   726,   460,   725,   888,   829,   930,
     829,    53,    54,    55,    56,    57,    58,   412,   918,   919,
     951,   952,   342,   915,   282,   754,  -251,   986,   987,  1007,
     829,  1037,   829,   558,   560,   727,   675,   676,   559,   559,
     677,   678,   287,   679,   680,   937,   681,   343,   251,   253,
     427,   428,   344,   288,   271,   429,   430,  -346,   582,   584,
     289,    67,   345,   346,   291,  -462,   292,   347,   294,   296,
     295,   -90,   312,   318,   322,   324,   682,   683,   684,   685,
     686,   687,   688,   689,   377,   690,   691,   804,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   385,   392,   393,   398,   409,   439,   397,   447,
     446,   448,   449,   807,   453,   454,   479,   482,  1022,   978,
     474,   490,   496,   497,   506,   508,   511,   374,   526,   418,
     825,   548,   303,   553,   878,   199,   580,   598,   581,   599,
     583,   605,   183,   460,   475,   617,   619,   621,   624,   625,
    -586,  -587,  -585,   637,   643,   649,  1041,   646,   650,   652,
     657,   658,   862,   470,   660,   661,   662,   663,   664,   665,
     666,   669,   709,   717,   711,   713,   719,  -442,   744,   746,
     749,   755,   715,   747,   751,   752,   759,   753,   772,   789,
     794,   248,   800,   801,   808,   809,   812,   818,   830,   832,
     182,   834,   836,     2,   844,     3,   847,   837,   849,   856,
     829,   859,   874,   858,   863,   890,     4,     5,     6,     7,
       8,     9,   900,   902,   904,   909,   910,    10,   916,  -427,
     707,   932,    11,   944,   917,   945,   922,    12,   943,   946,
     949,    13,   955,    14,    15,    16,   962,   963,   878,   965,
     966,   979,   967,   252,   989,    17,   417,  1005,   973,   485,
     998,  1004,  1003,    18,    19,    20,  1006,  1008,  1018,  1011,
    1015,   954,  1038,  1045,  1023,   854,   756,    21,   929,    22,
     861,   970,   936,   606,   722,   609,   712,   561,    23,    24,
      25,  1044,   714,    26,   198,    27,   960,   407,   611,   408,
     261,   263,   531,   638,   639,   252,   995,    28,   996,    29,
     931,   885,    30,  1002,   491,    31,   556,   463,   914,    32,
      33,    34,   308,    35,    97,    36,    37,   396,   578,   183,
     522,   432,   416,   734,    38,   380,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   742,   521,   290,  1014,     0,
    1017,     0,   961,     0,     0,     0,     0,     0,     0,    48,
       0,    49,     0,     0,    50,     0,     0,    51,    52,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,    53,    54,    55,    56,    57,    58,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    66,     0,     0,     0,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    68,    69,    70,
       0,     0,     0,     0,     0,    71,    72,    73,    74,    75,
      76,    77,    78,     1,    79,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,    11,     0,     0,     0,     0,
      12,     0,     0,     0,    13,     0,    14,    15,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       0,     0,     0,     0,     0,     0,    18,    19,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,    22,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      28,     0,    29,     0,     0,    30,     0,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,     0,    36,    37,
       0,     0,     0,     0,     0,     0,     0,    38,     0,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,    49,     0,     0,    50,     0,     0,
      51,    52,     0,     0,     0,     0,     0,     0,     0,   991,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,    62,     0,     0,
       0,    63,    64,    65,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    66,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,     0,
      68,    69,    70,     0,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,    77,    78,     1,    79,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,     0,    12,     0,     0,     0,    13,     0,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,    22,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,    29,     0,     0,    30,     0,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
       0,    36,    37,     0,     0,     0,     0,     0,     0,     0,
      38,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,    49,     0,     0,
      50,     0,  1042,    51,    52,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
      62,     0,     0,     0,    63,    64,    65,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    68,    69,    70,     0,     0,     0,     0,
       0,    71,    72,    73,    74,    75,    76,    77,    78,     1,
      79,     0,     2,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,    11,     0,     0,     0,     0,    12,     0,     0,     0,
      13,     0,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
       0,     0,    18,    19,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,    22,     0,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
       0,     0,    26,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    28,     0,    29,     0,
       0,    30,     0,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,     0,    36,    37,     0,     0,     0,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
      49,     0,     0,    50,     0,     0,    51,    52,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,    56,    57,    58,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,    62,     0,     0,     0,    63,    64,    65,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    66,     0,     0,     0,     0,     0,     0,
       0,     0,    67,     0,     0,     0,    68,    69,    70,     0,
       0,     0,     0,     0,    71,    72,    73,    74,    75,    76,
      77,    78,   302,    79,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   303,     0,     0,    30,     0,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,     0,     0,    37,     0,
       0,   183,     0,     0,     0,     0,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,    49,     0,     0,    50,     0,     0,    51,
      52,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    53,    54,    55,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,    62,     0,     0,     0,
      63,    64,    65,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
       0,     0,     0,     0,     0,    67,     0,     0,     0,    68,
      69,    70,     0,     0,     0,     0,     0,    71,    72,    73,
      74,    75,    76,    77,    78,   196,    79,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,   197,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,    11,   -54,     0,
       0,     0,    12,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,     0,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,    38,
       0,    39,    40,    41,    42,    43,    44,    45,    46,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,    49,     0,     0,    50,
       0,     0,    51,    52,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    53,    54,
      55,    56,    57,    58,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,    62,
       0,     0,     0,    63,    64,    65,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    68,    69,    70,     0,     0,     0,     0,     0,
      71,    72,    73,    74,    75,    76,    77,    78,   196,    79,
       0,     2,     0,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
      11,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,    49,
       0,     0,    50,     0,     0,    51,    52,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    53,    54,    55,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,     0,     0,     0,    63,    64,    65,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    66,     0,     0,     0,     0,     0,     0,     0,
       0,    67,     0,     0,     0,    68,    69,    70,     0,     0,
       0,     0,     0,    71,    72,    73,    74,    75,    76,    77,
      78,   236,    79,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,   197,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    68,    69,
      70,     0,     0,     0,     0,     0,    71,    72,    73,    74,
      75,    76,    77,    78,   236,    79,     0,     2,     0,     3,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,   197,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,     0,    32,    33,    34,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,    49,     0,     0,    50,     0,
       0,    51,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,    55,
      56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,    62,     0,
       0,     0,    63,    64,    65,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    66,     0,
       0,     0,     0,     0,     0,     0,     0,    67,     0,     0,
       0,    68,    69,    70,     0,     0,     0,     0,     0,    71,
      72,    73,    74,    75,    76,    77,    78,   236,    79,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     0,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,   361,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    30,
       0,     0,    31,     0,     0,     0,    32,    33,    34,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,     0,     0,     0,   362,   363,   364,     0,
       0,     0,   365,     0,     0,     0,    48,   366,    49,     0,
       0,    50,     0,     0,    51,    52,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      53,    54,    55,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,     0,     0,     0,    63,    64,    65,     0,     0,
     236,     0,     0,     2,     0,     3,     0,     0,     0,     0,
       0,    66,     0,     0,     0,     0,     4,     5,     6,     7,
      67,     9,     0,     0,    68,    69,    70,    10,     0,     0,
       0,     0,    11,     0,     0,   333,     0,    12,    77,    78,
       0,    79,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    30,     0,     0,    31,     0,     0,   336,    32,
      33,    34,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,    49,     0,     0,    50,     0,     0,    51,    52,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    53,    54,    55,    56,    57,    58,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,     0,     0,   371,     0,     0,     2,     0,     3,     0,
       0,     0,     0,     0,    66,     0,     0,     0,     0,     4,
       5,     6,     7,    67,     9,     0,     0,    68,    69,    70,
      10,     0,     0,     0,     0,    11,     0,     0,     0,     0,
      12,    77,    78,     0,    79,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   303,     0,     0,    30,     0,     0,    31,     0,
       0,     0,    32,    33,    34,     0,     0,     0,     0,    37,
       0,     0,   183,     0,     0,     0,     0,    38,     0,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,    49,     0,     0,    50,     0,     0,
      51,    52,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,    62,     0,     0,
       0,    63,    64,    65,     0,     0,   236,     0,     0,     2,
       0,     3,     0,     0,     0,     0,     0,    66,     0,     0,
       0,     0,     4,     5,     6,     7,    67,     9,     0,     0,
      68,    69,    70,    10,     0,     0,     0,     0,    11,     0,
       0,     0,     0,    12,    77,    78,     0,    79,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    30,     0,
       0,    31,     0,     0,     0,    32,    33,    34,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
      38,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,    49,     0,     0,
      50,     0,     0,    51,    52,   237,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
      62,     0,     0,     0,    63,    64,    65,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      66,     0,   236,     0,     0,     2,     0,     3,     0,    67,
       0,     0,     0,    68,    69,    70,     0,     0,     4,     5,
       6,     7,     0,     9,     0,     0,   401,    77,    78,   402,
      79,     0,     0,     0,    11,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   403,     0,     0,    31,     0,     0,
       0,    32,    33,    34,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,    49,     0,     0,    50,     0,     0,    51,
      52,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    53,    54,    55,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,    62,     0,     0,     0,
      63,    64,    65,     0,     0,   236,   415,     0,     2,     0,
       3,     0,     0,     0,     0,     0,    66,     0,     0,     0,
       0,     4,     5,     6,     7,    67,     9,     0,     0,    68,
      69,    70,    10,     0,     0,     0,     0,    11,     0,     0,
       0,     0,    12,    77,    78,     0,    79,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,     0,     0,
      31,     0,     0,     0,    32,    33,    34,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,    38,
       0,    39,    40,    41,    42,    43,    44,    45,    46,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,    49,     0,     0,    50,
       0,     0,    51,    52,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    53,    54,
      55,    56,    57,    58,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,    62,
       0,     0,     0,    63,    64,    65,     0,     0,   236,     0,
       0,     2,     0,     3,     0,     0,     0,     0,     0,    66,
       0,     0,     0,     0,     4,     5,     6,     7,    67,     9,
       0,     0,    68,    69,    70,    10,     0,     0,     0,     0,
      11,     0,     0,     0,     0,    12,    77,    78,     0,    79,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,   483,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,     0,    32,    33,    34,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,    49,
       0,     0,    50,     0,     0,    51,    52,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    53,    54,    55,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,     0,     0,     0,    63,    64,    65,     0,
       0,   236,     0,     0,     2,     0,     3,     0,     0,     0,
       0,     0,    66,     0,     0,     0,     0,     4,     5,     6,
       7,    67,     9,     0,     0,    68,    69,    70,    10,     0,
       0,     0,     0,    11,     0,     0,     0,     0,    12,    77,
      78,     0,    79,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   723,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,    31,     0,     0,     0,
      32,    33,    34,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,   236,     0,     0,     2,     0,     3,
       0,     0,     0,     0,     0,    66,     0,     0,     0,     0,
       4,     5,     6,     7,    67,     9,     0,     0,    68,    69,
      70,    10,     0,     0,     0,     0,    11,     0,     0,     0,
       0,    12,    77,    78,     0,    79,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,     0,    32,    33,    34,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,    49,     0,     0,    50,     0,
       0,    51,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,    55,
      56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,    62,     0,
       0,     0,    63,    64,    65,     0,     0,   236,     0,     0,
       2,     0,     3,     0,     0,     0,     0,     0,    66,     0,
       0,     0,     0,     4,     5,     6,     7,    67,     9,     0,
       0,    68,    69,    70,    10,     0,     0,     0,     0,    11,
       0,     0,     0,     0,    12,    77,    78,     0,    79,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    30,
       0,     0,    31,     0,     0,     0,    32,    33,    34,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,    49,     0,
       0,    50,     0,     0,    51,    52,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      53,    54,    55,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,     0,     0,     0,    63,    64,    65,     0,     0,
     236,     0,     0,     2,     0,     3,     0,     0,     0,     0,
       0,    66,     0,     0,     0,     0,     4,     5,     6,     7,
      67,     9,     0,     0,    68,    69,    70,    10,     0,     0,
       0,     0,    11,     0,     0,     0,     0,    12,    77,    78,
       0,   326,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,     0,    27,     0,     0,     0,     0,
       0,   236,     0,     0,     2,     0,     3,     0,     0,     0,
       0,     0,    30,     0,     0,    31,     0,     4,     5,     6,
       7,    34,     9,     0,     0,     0,    37,     0,    10,     0,
       0,     0,     0,    11,    38,     0,    39,    40,    12,    42,
       0,     0,    45,    46,    47,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,    49,     0,     0,    50,    19,     0,    51,    52,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    53,    54,    55,    56,    57,    58,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    30,     0,     0,    31,     0,     0,     0,
       0,     0,    34,     0,    66,     0,     0,    37,     0,     0,
       0,     0,     0,    67,     0,    38,     0,    39,    40,     0,
      42,     0,     0,    45,    46,    47,     0,     0,     0,     0,
       0,    77,    78,     0,    79,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     0,     0,    67,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    79
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-870)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     1,     1,    77,    78,   245,    50,    69,    70,   105,
     341,    92,    92,   292,   374,    92,   206,    22,   452,   330,
       4,    92,     1,   213,     5,    73,     4,     4,   208,   311,
       4,   348,     4,    18,   224,   709,     4,    24,     4,   348,
     245,     4,     4,     4,     4,   719,    70,    73,    73,     4,
      35,    24,   672,     3,   674,     4,    77,     4,    31,   681,
       5,   105,   682,     0,     1,     5,   239,   279,   168,    64,
     323,   691,   652,     5,     5,    19,    20,    21,    22,     5,
     182,   755,     3,   233,     3,   818,    24,    92,   204,    41,
      69,    70,   242,   243,    27,     4,   823,     3,   967,   289,
      23,   790,    27,     3,    42,    32,   119,    58,    24,    23,
       6,   119,     8,   182,   174,   175,   176,   322,     3,    25,
      24,    23,    36,    19,    20,    21,    22,    60,   817,    80,
      32,   805,    59,    71,    36,    60,    65,   454,    20,    24,
      22,   203,   453,    36,   426,   454,   246,    73,   136,   509,
    1019,     3,   473,   880,   223,   775,   776,    59,   220,    82,
     780,    70,     3,    77,   260,   476,   282,   105,    82,   182,
    1018,   492,    24,    87,   182,    77,    50,   128,     3,     3,
      82,     3,   182,   182,    77,    87,   138,   303,   126,     3,
    1038,     3,    73,   112,    87,   257,   125,   196,   294,   932,
     302,    75,   134,   182,   214,   417,     3,   380,   256,   283,
     284,   285,   286,   132,   288,    24,   260,   196,    64,   321,
     230,    81,   201,   168,   203,   847,   679,   680,   168,   663,
     256,   256,   256,   813,   487,   256,   215,   168,   237,   224,
     219,   220,   221,   393,   194,   182,   973,    65,   198,    23,
     294,   230,   231,   232,   233,   234,   235,   236,   448,   541,
     542,   256,   267,   242,   243,    83,   471,   246,   246,   371,
     257,   256,   256,   194,     3,   256,   636,   711,   257,   256,
     246,   955,   256,   256,   256,   905,   246,   527,   256,   479,
     470,   246,   236,   256,   256,   256,   377,   377,   277,   278,
     377,   280,   281,   302,   400,   622,   377,   256,   287,   256,
     490,   227,   291,   622,     4,   246,   247,   497,     4,     5,
      36,     7,   527,   302,   257,   248,   249,   250,   251,   252,
     253,  1005,   311,   377,   248,   249,   250,   251,   252,   253,
     236,   257,   675,   676,   218,   624,   248,   249,   250,   251,
     252,   253,   226,    53,   236,   334,   400,   668,   240,   136,
     339,    77,    46,   119,     3,    19,    20,    21,    22,   194,
     194,    87,   377,   198,   198,    65,    49,   133,   377,   832,
     194,   834,   194,   836,   198,    24,   198,   257,   258,     6,
     707,     8,   371,    53,    23,   374,   849,   229,   707,   399,
     605,    74,    19,    20,    21,    22,    79,    91,   254,   255,
     256,   257,   258,    42,   393,     3,    89,    90,   514,    36,
     405,    94,   254,   255,    73,   181,    77,   760,    57,   746,
       3,   764,    81,    50,   767,   125,    87,   746,   122,   123,
     113,   621,    96,   231,   506,   129,     3,   426,    67,   902,
      67,   784,   152,   153,   154,     6,     3,   141,   158,   555,
      73,   460,    13,   163,     3,    24,   445,   529,    81,     3,
     514,     6,    31,     8,   254,   255,   105,    96,     3,    96,
     109,   172,   173,   800,    19,    20,    21,    22,     0,   143,
     144,   800,   152,   153,   154,   474,   296,   126,   158,   299,
     300,     3,   481,   163,   483,   124,   839,   124,   127,   842,
       6,   555,     8,     4,     5,    50,     7,   219,   829,   221,
       4,     5,   501,   502,   503,    41,   505,   506,     3,   508,
     509,     7,    67,   713,   208,   209,    74,   727,   219,     3,
     221,    79,    14,    15,    16,    17,    18,   643,    62,    63,
     529,    89,    90,    69,   635,   635,   240,   874,   635,     3,
      49,    96,   541,   542,   635,   874,     4,     5,     3,   186,
     187,   188,   189,   190,   191,     4,     5,     3,     7,    95,
       4,     5,   236,     7,     3,    74,   247,   241,   242,   124,
      79,   635,   108,    49,   125,    84,     4,     5,   598,   643,
      89,    90,    82,   220,   246,    94,     4,     5,     3,     7,
       4,     5,   852,     7,     4,     5,   203,     7,    74,   236,
     222,   223,   138,    79,     3,   956,     3,   958,   959,   960,
     635,     4,     5,    89,    90,     4,     5,   616,    94,   214,
     215,     3,   738,     3,   100,   134,   625,     4,     5,     4,
       5,   186,   187,   188,   189,   190,   191,   636,   184,   185,
       4,     5,    49,   853,     3,   664,   203,   182,   183,     4,
       5,     4,     5,   443,   444,   131,   192,   193,   443,   444,
     196,   197,   256,   199,   200,   220,   202,    74,    77,    78,
     283,   284,    79,   254,   738,   285,   286,    84,   451,   452,
      43,   236,    89,    90,     3,    43,    13,    94,     3,    83,
       4,    83,    24,    24,    81,    96,   232,   233,   234,   235,
     236,   237,   238,   239,   172,   241,   242,   726,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,     4,     4,     3,    27,     4,     4,   810,     5,
      31,   131,     4,   732,     3,     3,   131,     4,   998,   949,
       5,    84,   103,    84,    73,    73,   169,   173,     4,    24,
     749,     5,   112,     3,   818,    25,     4,    31,    24,    99,
      24,     5,   132,   134,    24,    24,    24,    84,    13,   248,
       5,     5,     5,   169,   247,     4,  1036,   224,    21,     4,
       4,     4,   801,    73,     4,     4,     4,    13,     3,    85,
      85,     3,    99,     4,   100,     5,     3,   256,     4,    39,
       5,     3,    75,   102,   210,   210,     3,    24,     3,    41,
     135,   810,    39,    76,     4,     4,    73,    39,    21,   195,
       3,   195,   195,     6,     3,     8,   120,    21,   195,   113,
       5,   178,     5,    42,   225,     4,    19,    20,    21,    22,
      23,    24,     4,   195,    21,    96,     3,    30,    24,   120,
       5,     5,    35,   221,   101,   221,    70,    40,   182,   216,
     109,    44,     3,    46,    47,    48,   177,   172,   932,     5,
      31,    21,   228,   967,     3,    58,     3,     3,   247,    31,
     229,   217,   182,    66,    67,    68,     4,    24,   180,    24,
      24,   910,     7,   231,    68,   788,   668,    80,   864,    82,
     800,   932,   879,   468,   622,   471,   605,   445,    91,    92,
      93,  1039,   609,    96,     8,    98,   919,   268,   474,   269,
      92,    92,   397,   527,   527,  1019,   965,   110,   966,   112,
     874,   823,   115,   973,   347,   118,   442,   321,   852,   122,
     123,   124,   203,   126,   964,   128,   129,   257,   447,   132,
     380,   288,   278,   631,   137,   241,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   635,   378,   160,   984,    -1,
     989,    -1,   920,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,   164,    -1,    -1,   167,    -1,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,    -1,
      -1,    -1,    -1,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,
      -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,   212,
     213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   236,    -1,    -1,    -1,   240,   241,   242,
      -1,    -1,    -1,    -1,    -1,   248,   249,   250,   251,   252,
     253,   254,   255,     3,   257,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    -1,    -1,    96,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,    -1,   112,    -1,    -1,   115,    -1,    -1,   118,    -1,
      -1,    -1,   122,   123,   124,    -1,   126,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,    -1,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   204,   205,   206,   207,    -1,    -1,
      -1,   211,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   236,    -1,    -1,    -1,
     240,   241,   242,    -1,    -1,    -1,    -1,    -1,   248,   249,
     250,   251,   252,   253,   254,   255,     3,   257,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    -1,    -1,    96,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,    -1,   112,    -1,    -1,   115,    -1,
      -1,   118,    -1,    -1,    -1,   122,   123,   124,    -1,   126,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,
     167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
     187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,   205,   206,
     207,    -1,    -1,    -1,   211,   212,   213,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     227,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   236,
      -1,    -1,    -1,   240,   241,   242,    -1,    -1,    -1,    -1,
      -1,   248,   249,   250,   251,   252,   253,   254,   255,     3,
     257,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,
      -1,    -1,    96,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,   112,    -1,
      -1,   115,    -1,    -1,   118,    -1,    -1,    -1,   122,   123,
     124,    -1,   126,    -1,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,
     164,    -1,    -1,   167,    -1,    -1,   170,   171,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   186,   187,   188,   189,   190,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     204,   205,   206,   207,    -1,    -1,    -1,   211,   212,   213,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   236,    -1,    -1,    -1,   240,   241,   242,    -1,
      -1,    -1,    -1,    -1,   248,   249,   250,   251,   252,   253,
     254,   255,     3,   257,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   115,    -1,    -1,   118,    -1,    -1,
      -1,   122,   123,   124,    -1,   126,    -1,    -1,   129,    -1,
      -1,   132,    -1,    -1,    -1,    -1,   137,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,   170,
     171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   204,   205,   206,   207,    -1,    -1,    -1,
     211,   212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   236,    -1,    -1,    -1,   240,
     241,   242,    -1,    -1,    -1,    -1,    -1,   248,   249,   250,
     251,   252,   253,   254,   255,     3,   257,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    -1,    -1,    96,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
     118,    -1,    -1,    -1,   122,   123,   124,    -1,   126,    -1,
      -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,
      -1,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   204,   205,   206,   207,
      -1,    -1,    -1,   211,   212,   213,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   227,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   236,    -1,
      -1,    -1,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,
     248,   249,   250,   251,   252,   253,   254,   255,     3,   257,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
      -1,    96,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,   118,    -1,    -1,    -1,   122,   123,   124,
      -1,   126,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,
      -1,    -1,   167,    -1,    -1,   170,   171,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,
     205,   206,   207,    -1,    -1,    -1,   211,   212,   213,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   227,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   236,    -1,    -1,    -1,   240,   241,   242,    -1,    -1,
      -1,    -1,    -1,   248,   249,   250,   251,   252,   253,   254,
     255,     3,   257,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,   118,    -1,    -1,    -1,
     122,   123,   124,    -1,   126,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,   164,    -1,    -1,   167,    -1,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,
     212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   236,    -1,    -1,    -1,   240,   241,
     242,    -1,    -1,    -1,    -1,    -1,   248,   249,   250,   251,
     252,   253,   254,   255,     3,   257,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    -1,    -1,    96,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,   118,
      -1,    -1,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,
      -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   204,   205,   206,   207,    -1,
      -1,    -1,   211,   212,   213,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   227,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   236,    -1,    -1,
      -1,   240,   241,   242,    -1,    -1,    -1,    -1,    -1,   248,
     249,   250,   251,   252,   253,   254,   255,     3,   257,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,    -1,
      96,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,   118,    -1,    -1,    -1,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,    -1,   158,    -1,    -1,    -1,   162,   163,   164,    -1,
      -1,   167,    -1,    -1,   170,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,   205,
     206,   207,    -1,    -1,    -1,   211,   212,   213,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,   227,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     236,    24,    -1,    -1,   240,   241,   242,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    38,    -1,    40,   254,   255,
      -1,   257,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    91,    92,
      93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,   118,    -1,    -1,   121,   122,
     123,   124,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,   164,    -1,    -1,   167,    -1,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,   212,
     213,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,   236,    24,    -1,    -1,   240,   241,   242,
      30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      40,   254,   255,    -1,   257,    -1,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    -1,    -1,    96,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,   115,    -1,    -1,   118,    -1,
      -1,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,   129,
      -1,    -1,   132,    -1,    -1,    -1,    -1,   137,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,   147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   204,   205,   206,   207,    -1,    -1,
      -1,   211,   212,   213,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   236,    24,    -1,    -1,
     240,   241,   242,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    40,   254,   255,    -1,   257,    -1,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    -1,    -1,    96,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,   118,    -1,    -1,    -1,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,
     167,    -1,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
     187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,   205,   206,
     207,    -1,    -1,    -1,   211,   212,   213,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     227,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,   236,
      -1,    -1,    -1,   240,   241,   242,    -1,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    27,   254,   255,    30,
     257,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,   118,    -1,    -1,
      -1,   122,   123,   124,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,   164,    -1,    -1,   167,    -1,    -1,   170,
     171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   204,   205,   206,   207,    -1,    -1,    -1,
     211,   212,   213,    -1,    -1,     3,     4,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,   236,    24,    -1,    -1,   240,
     241,   242,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,   254,   255,    -1,   257,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    -1,    -1,    96,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
     118,    -1,    -1,    -1,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,   147,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,
      -1,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   204,   205,   206,   207,
      -1,    -1,    -1,   211,   212,   213,    -1,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   227,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,   236,    24,
      -1,    -1,   240,   241,   242,    30,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    40,   254,   255,    -1,   257,
      -1,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
      -1,    96,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,   118,    -1,    -1,    -1,   122,   123,   124,
      -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,
      -1,    -1,   167,    -1,    -1,   170,   171,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,
     205,   206,   207,    -1,    -1,    -1,   211,   212,   213,    -1,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,   227,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,   236,    24,    -1,    -1,   240,   241,   242,    30,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,   254,
     255,    -1,   257,    -1,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,   118,    -1,    -1,    -1,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,   164,    -1,    -1,   167,    -1,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,
     212,   213,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,   236,    24,    -1,    -1,   240,   241,
     242,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    40,   254,   255,    -1,   257,    -1,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    -1,    -1,    96,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,   118,
      -1,    -1,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,   147,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,    -1,
      -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   204,   205,   206,   207,    -1,
      -1,    -1,   211,   212,   213,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   227,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,   236,    24,    -1,
      -1,   240,   241,   242,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    40,   254,   255,    -1,   257,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,    -1,
      96,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,   118,    -1,    -1,    -1,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,    -1,   139,   140,   141,   142,   143,   144,   145,
     146,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,    -1,
      -1,   167,    -1,    -1,   170,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,   205,
     206,   207,    -1,    -1,    -1,   211,   212,   213,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,   227,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     236,    24,    -1,    -1,   240,   241,   242,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    40,   254,   255,
      -1,   257,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,    -1,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,   118,    -1,    19,    20,    21,
      22,   124,    24,    -1,    -1,    -1,   129,    -1,    30,    -1,
      -1,    -1,    -1,    35,   137,    -1,   139,   140,    40,   142,
      -1,    -1,   145,   146,   147,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,   164,    -1,    -1,   167,    67,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   186,   187,   188,   189,   190,   191,    91,
      92,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,
      -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,   212,
     213,    -1,    -1,   115,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,   124,    -1,   227,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,   236,    -1,   137,    -1,   139,   140,    -1,
     142,    -1,    -1,   145,   146,   147,    -1,    -1,    -1,    -1,
      -1,   254,   255,    -1,   257,    -1,    -1,    -1,    -1,    -1,
     162,    -1,   164,    -1,    -1,   167,    -1,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   204,   205,   206,   207,    -1,    -1,    -1,   211,
     212,   213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   227,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   236,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   257
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    35,    40,    44,    46,    47,    48,    58,    66,    67,
      68,    80,    82,    91,    92,    93,    96,    98,   110,   112,
     115,   118,   122,   123,   124,   126,   128,   129,   137,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   162,   164,
     167,   170,   171,   186,   187,   188,   189,   190,   191,   204,
     205,   206,   207,   211,   212,   213,   227,   236,   240,   241,
     242,   248,   249,   250,   251,   252,   253,   254,   255,   257,
     262,   263,   264,   265,   276,   284,   285,   286,   287,   288,
     289,   290,   293,   294,   295,   296,   301,   306,   332,   334,
     335,   336,   337,   338,   340,   341,   343,   344,   345,   346,
     348,   349,   351,   352,   354,   356,   357,   358,   362,   364,
     365,   366,   367,   368,   369,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   388,   389,   394,   395,
     397,   429,   430,   448,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   472,   473,   474,   475,
     476,   478,   479,   480,   481,   485,   487,   488,   489,   490,
     491,   492,   501,   502,   503,   528,   529,   530,   531,   532,
     544,   546,     3,   132,   263,   291,   305,   306,   337,   441,
     442,   443,   445,   447,   472,    24,     3,    23,   334,    25,
     500,     3,   136,     3,     3,   119,   133,   181,    73,     3,
     358,    24,   543,    81,   284,     3,   136,    27,    60,   297,
       3,     3,   358,   231,   326,   327,   435,   436,   437,   439,
       3,     3,     3,     3,     3,     3,     3,   172,   294,   537,
     538,   542,     3,     3,     3,     3,     3,     3,   472,   473,
     473,   455,   489,   455,     0,     7,    77,    87,   125,   284,
     341,   342,   344,   345,   348,   351,   353,    82,   247,   246,
     293,   294,    20,    22,   236,   240,     3,     3,     3,   203,
       3,     3,     3,   257,   258,   254,   255,   256,   254,    43,
     477,     3,    13,   445,     3,     4,    83,     4,     4,    65,
     125,     4,     3,   112,   306,   337,   449,   472,   449,   473,
     257,   297,    24,   496,   497,   498,   499,   496,    24,   566,
     327,     3,    81,   496,    96,   472,   257,   307,   359,   360,
     472,   495,   473,    38,    73,    86,   121,   472,   493,   494,
     358,   496,    49,    74,    79,    89,    90,    94,   113,   431,
     434,   472,   472,   469,   472,   469,   472,   482,   483,   484,
     472,    53,   152,   153,   154,   158,   163,   370,   371,   464,
     472,     3,   472,   337,   173,   538,   539,   172,   533,   534,
     537,   469,   472,   469,   472,     4,   362,   383,   384,   385,
     544,   472,     4,     3,   350,   358,   453,   473,    27,   446,
      36,    27,    30,   115,   293,   355,   284,   335,   336,     4,
     470,   471,   472,   540,   541,     4,   471,     3,    24,   400,
     401,   402,   408,   390,   472,   472,   297,   456,   456,   457,
     457,   472,   478,   489,   496,   472,    24,   227,   502,     4,
     292,   293,   446,   446,   446,   297,    31,     5,   131,     4,
     470,     6,    13,     3,     3,   279,   174,   175,   176,   548,
     134,   304,   328,   447,   302,   303,   361,   544,   279,     4,
      73,    81,   320,   321,     5,    24,    31,   363,   527,   131,
     472,    73,     4,    73,   472,    31,   323,   545,   323,   325,
      84,   434,   298,   299,   501,   503,   103,    84,     4,     4,
       4,     5,     7,    77,     4,    73,    73,     4,    73,   168,
     540,   169,   294,   339,   341,   342,   344,   348,   351,   535,
     536,   539,   534,     4,     4,     4,     4,     5,   469,    64,
     347,   347,   306,   293,   358,    67,    96,   124,   127,   331,
       4,     5,     7,     4,   400,   409,   412,   413,     5,   386,
       4,   470,     4,     3,     4,     5,   442,   444,   305,   443,
     305,   307,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   450,   451,   450,   496,
       4,    24,   498,    24,   498,   499,    42,    71,   105,   126,
     266,   267,   268,   271,   273,   527,   275,   503,    31,    99,
      58,    80,   128,   549,   337,     5,   291,   322,   327,   302,
     304,   360,   527,   496,   472,   472,    73,    24,   279,    24,
     327,    84,     5,   304,    13,   248,   327,   472,   472,   472,
     472,   473,   472,   540,   293,     5,   168,   169,   383,   384,
     398,     4,   473,   247,   470,   470,   224,   410,   414,     4,
      21,   391,     4,   208,   209,   387,   393,     4,     4,   293,
       4,     4,     4,    13,     3,    85,    85,     4,     5,     3,
      41,    69,    95,   108,   138,   192,   193,   196,   197,   199,
     200,   202,   232,   233,   234,   235,   236,   237,   238,   239,
     241,   242,   504,   507,   508,   515,   516,   517,   518,   519,
     520,   522,   523,   524,   525,   526,     4,     5,   306,    99,
     550,   100,   303,     5,   320,    75,   329,     4,   472,     3,
     278,   327,   299,    57,   300,   472,   100,   131,   432,   433,
     440,     4,    70,   486,   486,     4,   284,   294,   341,   344,
     348,   351,   536,   540,     4,   293,    39,   102,   280,     5,
     387,   210,   210,    24,   337,     3,   267,   274,   527,     3,
     194,   198,   509,   510,   194,   198,   509,   194,   198,   509,
     510,   510,     3,   511,   512,    41,   138,   511,   511,   158,
     464,   465,   467,   468,   194,   198,   509,   509,   269,    41,
     505,   477,   506,   503,   135,   277,   274,   551,   499,   327,
      39,    76,   330,   274,   337,     3,   496,   472,     4,     4,
      87,   293,    73,   396,   399,   415,   416,   503,    39,   222,
     223,   411,   418,   419,   392,   472,     4,   274,     4,     5,
      21,   510,   195,   510,   195,   510,   195,    21,   513,   194,
     198,   509,   194,   509,     3,   521,   509,   120,   510,   195,
      23,    42,    57,   109,   271,   272,   113,   506,    42,   178,
     547,   275,   337,   225,   403,   404,     4,   274,    62,    63,
     438,   214,   215,   387,     5,   506,   281,   282,   294,   338,
      36,    50,   220,   373,   420,   421,   422,   423,     4,   527,
       4,   511,   511,   511,    14,    15,    16,    17,    18,   514,
       4,   510,   195,   510,    21,   158,   464,   466,   511,    96,
       3,    96,   270,   362,   460,   496,    24,   101,   184,   185,
     559,   560,    70,   552,   553,   400,   405,   406,   407,   280,
       4,   416,     5,    32,    59,   283,   283,   220,   373,   421,
     424,   426,   427,   182,   221,   221,   216,   417,   428,   109,
     511,     4,     5,   509,   337,     3,   119,   182,   324,   119,
     324,   560,   177,   172,   555,     5,    31,   228,   317,   319,
     282,   219,   219,   247,    50,    75,   218,   226,   496,    21,
       4,   274,   323,   323,   323,   323,   182,   183,   554,     3,
     333,   179,   263,   556,   558,   406,   408,   458,   229,   308,
     309,   425,   426,   182,   217,     3,     4,     4,    24,   562,
     563,    24,   565,   561,   563,    24,   564,   337,   180,   229,
     318,   315,   362,    68,   313,   314,   274,     4,   557,   558,
     458,   182,   223,   312,   214,   230,   311,     4,     7,   310,
     316,   362,   169,   558,   312,   231
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

  case 249:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 250:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 256:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 261:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 262:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 263:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 264:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 266:

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

  case 274:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 277:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 279:

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

  case 282:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 283:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 288:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 290:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 291:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 292:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 293:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 295:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 296:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 298:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 300:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 304:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 305:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 306:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 307:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 308:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 310:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 315:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 317:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 318:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 319:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 323:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 325:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 326:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 327:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 328:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 329:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 330:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 331:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 332:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 340:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 341:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 342:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 343:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 346:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 347:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 349:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 350:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 351:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 352:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 354:

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

  case 355:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 360:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 364:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 366:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 369:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 370:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 371:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 391:

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

  case 399:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 401:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 402:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 406:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 409:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 410:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 412:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 413:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 414:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 415:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 416:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 417:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 418:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 419:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 420:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 421:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 427:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 429:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 430:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 431:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 432:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 434:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 435:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 436:

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

  case 438:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 439:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 440:

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

  case 447:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 448:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 451:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 453:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 456:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 457:

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

  case 458:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 459:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 460:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 463:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 464:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 465:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 466:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 467:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 468:

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

  case 469:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 472:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 473:

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

  case 474:

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

  case 475:

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

  case 476:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 477:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 478:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 479:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 480:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 485:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 486:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 487:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 488:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 489:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 490:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 491:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 492:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 493:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 494:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 495:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 496:

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

  case 497:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 498:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 499:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 501:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 502:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 503:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 505:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 506:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 512:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 513:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 514:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 515:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 516:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 518:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 520:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 521:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 523:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 524:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 525:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 526:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("K", SQL_NODE_PUNCTUATION));
        }
    break;

  case 527:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("M", SQL_NODE_PUNCTUATION));
        }
    break;

  case 528:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("G", SQL_NODE_PUNCTUATION));
        }
    break;

  case 529:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("T", SQL_NODE_PUNCTUATION));
        }
    break;

  case 530:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("P", SQL_NODE_PUNCTUATION));
        }
    break;

  case 531:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 532:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 533:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 534:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 535:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 536:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 537:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 538:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 539:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 542:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 543:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 544:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 545:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 546:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 548:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 549:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 552:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 553:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 554:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 565:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            /*$$->append($3);*/
        }
    break;

  case 566:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 570:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 571:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 572:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 575:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 576:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 577:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 578:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 579:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 580:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 581:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 588:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 589:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 590:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 591:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 592:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 596:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 597:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 598:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 599:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 600:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 601:

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

  case 603:

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

  case 604:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 605:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 608:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 611:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 612:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 613:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 615:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 616:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 617:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 620:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 621:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 623:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(";", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 624:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 625:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 627:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 628:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 629:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 630:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 631:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 632:

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
