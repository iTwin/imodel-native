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

//#pragma warning(disable:4603) Give Error on iOS
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
#ifndef YY_SQLYY_D_DEV_DGNDB_BIM20DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_D_DEV_DGNDB_BIM20DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_BY = 277,
     SQL_TOKEN_CAST = 278,
     SQL_TOKEN_CHARACTER = 279,
     SQL_TOKEN_CHECK = 280,
     SQL_TOKEN_COLLATE = 281,
     SQL_TOKEN_COMMIT = 282,
     SQL_TOKEN_CONTINUE = 283,
     SQL_TOKEN_CONVERT = 284,
     SQL_TOKEN_COUNT = 285,
     SQL_TOKEN_CREATE = 286,
     SQL_TOKEN_CROSS = 287,
     SQL_TOKEN_CURRENT = 288,
     SQL_TOKEN_CURSOR = 289,
     SQL_TOKEN_DATEVALUE = 290,
     SQL_TOKEN_DAY = 291,
     SQL_TOKEN_DEC = 292,
     SQL_TOKEN_DECIMAL = 293,
     SQL_TOKEN_DECLARE = 294,
     SQL_TOKEN_DEFAULT = 295,
     SQL_TOKEN_DELETE = 296,
     SQL_TOKEN_DESC = 297,
     SQL_TOKEN_DISTINCT = 298,
     SQL_TOKEN_DROP = 299,
     SQL_TOKEN_FORWARD = 300,
     SQL_TOKEN_BACKWARD = 301,
     SQL_TOKEN_ESCAPE = 302,
     SQL_TOKEN_EXCEPT = 303,
     SQL_TOKEN_EXISTS = 304,
     SQL_TOKEN_FALSE = 305,
     SQL_TOKEN_FETCH = 306,
     SQL_TOKEN_FLOAT = 307,
     SQL_TOKEN_FOR = 308,
     SQL_TOKEN_FOREIGN = 309,
     SQL_TOKEN_FOUND = 310,
     SQL_TOKEN_FROM = 311,
     SQL_TOKEN_FULL = 312,
     SQL_TOKEN_GROUP = 313,
     SQL_TOKEN_HAVING = 314,
     SQL_TOKEN_IN = 315,
     SQL_TOKEN_INDICATOR = 316,
     SQL_TOKEN_INNER = 317,
     SQL_TOKEN_INSERT = 318,
     SQL_TOKEN_INTO = 319,
     SQL_TOKEN_IS = 320,
     SQL_TOKEN_INTERSECT = 321,
     SQL_TOKEN_JOIN = 322,
     SQL_TOKEN_KEY = 323,
     SQL_TOKEN_LIKE = 324,
     SQL_TOKEN_LOCAL = 325,
     SQL_TOKEN_LEFT = 326,
     SQL_TOKEN_RIGHT = 327,
     SQL_TOKEN_LOWER = 328,
     SQL_TOKEN_MAX = 329,
     SQL_TOKEN_MIN = 330,
     SQL_TOKEN_NATURAL = 331,
     SQL_TOKEN_NCHAR = 332,
     SQL_TOKEN_NULL = 333,
     SQL_TOKEN_NUMERIC = 334,
     SQL_TOKEN_OCTET_LENGTH = 335,
     SQL_TOKEN_OF = 336,
     SQL_TOKEN_ON = 337,
     SQL_TOKEN_OPTION = 338,
     SQL_TOKEN_ORDER = 339,
     SQL_TOKEN_OUTER = 340,
     SQL_TOKEN_PRECISION = 341,
     SQL_TOKEN_PRIMARY = 342,
     SQL_TOKEN_PROCEDURE = 343,
     SQL_TOKEN_PUBLIC = 344,
     SQL_TOKEN_REAL = 345,
     SQL_TOKEN_REFERENCES = 346,
     SQL_TOKEN_ROLLBACK = 347,
     SQL_TOKEN_SCHEMA = 348,
     SQL_TOKEN_SELECT = 349,
     SQL_TOKEN_SET = 350,
     SQL_TOKEN_SMALLINT = 351,
     SQL_TOKEN_SOME = 352,
     SQL_TOKEN_SQLCODE = 353,
     SQL_TOKEN_SQLERROR = 354,
     SQL_TOKEN_SUM = 355,
     SQL_TOKEN_TABLE = 356,
     SQL_TOKEN_TO = 357,
     SQL_TOKEN_TRANSLATE = 358,
     SQL_TOKEN_TRUE = 359,
     SQL_TOKEN_UNION = 360,
     SQL_TOKEN_UNIQUE = 361,
     SQL_TOKEN_UNKNOWN = 362,
     SQL_TOKEN_UPDATE = 363,
     SQL_TOKEN_UPPER = 364,
     SQL_TOKEN_USING = 365,
     SQL_TOKEN_VALUES = 366,
     SQL_TOKEN_VIEW = 367,
     SQL_TOKEN_WHERE = 368,
     SQL_TOKEN_WITH = 369,
     SQL_TOKEN_WORK = 370,
     SQL_TOKEN_BIT_LENGTH = 371,
     SQL_TOKEN_CHAR = 372,
     SQL_TOKEN_CHAR_LENGTH = 373,
     SQL_TOKEN_POSITION = 374,
     SQL_TOKEN_SUBSTRING = 375,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 376,
     SQL_TOKEN_CURRENT_DATE = 377,
     SQL_TOKEN_CURRENT_TIMESTAMP = 378,
     SQL_TOKEN_CURDATE = 379,
     SQL_TOKEN_NOW = 380,
     SQL_TOKEN_EXTRACT = 381,
     SQL_TOKEN_DAYNAME = 382,
     SQL_TOKEN_DAYOFMONTH = 383,
     SQL_TOKEN_DAYOFWEEK = 384,
     SQL_TOKEN_DAYOFYEAR = 385,
     SQL_TOKEN_HOUR = 386,
     SQL_TOKEN_MINUTE = 387,
     SQL_TOKEN_MONTH = 388,
     SQL_TOKEN_MONTHNAME = 389,
     SQL_TOKEN_QUARTER = 390,
     SQL_TOKEN_DATEDIFF = 391,
     SQL_TOKEN_SECOND = 392,
     SQL_TOKEN_TIMESTAMPADD = 393,
     SQL_TOKEN_TIMESTAMPDIFF = 394,
     SQL_TOKEN_TIMEVALUE = 395,
     SQL_TOKEN_WEEK = 396,
     SQL_TOKEN_YEAR = 397,
     SQL_TOKEN_EVERY = 398,
     SQL_TOKEN_WITHIN = 399,
     SQL_TOKEN_ARRAY_AGG = 400,
     SQL_TOKEN_CASE = 401,
     SQL_TOKEN_THEN = 402,
     SQL_TOKEN_END = 403,
     SQL_TOKEN_WHEN = 404,
     SQL_TOKEN_ELSE = 405,
     SQL_TOKEN_BEFORE = 406,
     SQL_TOKEN_AFTER = 407,
     SQL_TOKEN_INSTEAD = 408,
     SQL_TOKEN_EACH = 409,
     SQL_TOKEN_REFERENCING = 410,
     SQL_TOKEN_BEGIN = 411,
     SQL_TOKEN_ATOMIC = 412,
     SQL_TOKEN_TRIGGER = 413,
     SQL_TOKEN_ROW = 414,
     SQL_TOKEN_STATEMENT = 415,
     SQL_TOKEN_NEW = 416,
     SQL_TOKEN_OLD = 417,
     SQL_TOKEN_VALUE = 418,
     SQL_TOKEN_CURRENT_CATALOG = 419,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 420,
     SQL_TOKEN_CURRENT_PATH = 421,
     SQL_TOKEN_CURRENT_ROLE = 422,
     SQL_TOKEN_CURRENT_SCHEMA = 423,
     SQL_TOKEN_VARCHAR = 424,
     SQL_TOKEN_VARBINARY = 425,
     SQL_TOKEN_VARYING = 426,
     SQL_TOKEN_OBJECT = 427,
     SQL_TOKEN_NCLOB = 428,
     SQL_TOKEN_NATIONAL = 429,
     SQL_TOKEN_LARGE = 430,
     SQL_TOKEN_CLOB = 431,
     SQL_TOKEN_BLOB = 432,
     SQL_TOKEN_BIGI = 433,
     SQL_TOKEN_INTERVAL = 434,
     SQL_TOKEN_OVER = 435,
     SQL_TOKEN_ROW_NUMBER = 436,
     SQL_TOKEN_NTILE = 437,
     SQL_TOKEN_LEAD = 438,
     SQL_TOKEN_LAG = 439,
     SQL_TOKEN_RESPECT = 440,
     SQL_TOKEN_IGNORE = 441,
     SQL_TOKEN_NULLS = 442,
     SQL_TOKEN_FIRST_VALUE = 443,
     SQL_TOKEN_LAST_VALUE = 444,
     SQL_TOKEN_NTH_VALUE = 445,
     SQL_TOKEN_FIRST = 446,
     SQL_TOKEN_LAST = 447,
     SQL_TOKEN_EXCLUDE = 448,
     SQL_TOKEN_OTHERS = 449,
     SQL_TOKEN_TIES = 450,
     SQL_TOKEN_FOLLOWING = 451,
     SQL_TOKEN_UNBOUNDED = 452,
     SQL_TOKEN_PRECEDING = 453,
     SQL_TOKEN_RANGE = 454,
     SQL_TOKEN_ROWS = 455,
     SQL_TOKEN_PARTITION = 456,
     SQL_TOKEN_WINDOW = 457,
     SQL_TOKEN_NO = 458,
     SQL_TOKEN_GETECCLASSID = 459,
     SQL_TOKEN_LIMIT = 460,
     SQL_TOKEN_OFFSET = 461,
     SQL_TOKEN_NEXT = 462,
     SQL_TOKEN_ONLY = 463,
     SQL_TOKEN_MATCH = 464,
     SQL_TOKEN_ECSQLOPTIONS = 465,
     SQL_TOKEN_BINARY = 466,
     SQL_TOKEN_BOOLEAN = 467,
     SQL_TOKEN_DOUBLE = 468,
     SQL_TOKEN_INTEGER = 469,
     SQL_TOKEN_INT = 470,
     SQL_TOKEN_LONG = 471,
     SQL_TOKEN_INT64 = 472,
     SQL_TOKEN_STRING = 473,
     SQL_TOKEN_DATE = 474,
     SQL_TOKEN_TIMESTAMP = 475,
     SQL_TOKEN_DATETIME = 476,
     SQL_TOKEN_OR = 477,
     SQL_TOKEN_AND = 478,
     SQL_EQUAL = 479,
     SQL_GREAT = 480,
     SQL_LESS = 481,
     SQL_NOTEQUAL = 482,
     SQL_GREATEQ = 483,
     SQL_LESSEQ = 484,
     SQL_CONCAT = 485,
     SQL_TOKEN_INVALIDSYMBOL = 486
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

#endif /* !YY_SQLYY_D_DEV_DGNDB_BIM20DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */

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
#define YYFINAL  242
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4398

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  257
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  306
/* YYNRULES -- Number of rules.  */
#define YYNRULES  632
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1022

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   486

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   253,   241,     2,
       3,     4,   251,   248,     5,   249,    13,   252,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   255,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   240,    12,   254,     2,     2,     2,
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
     231,   232,   233,   234,   235,   236,   237,   238,   239,   242,
     243,   244,   245,   246,   247,   250,   256
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
     179,   181,   183,   185,   187,   192,   194,   196,   198,   201,
     207,   212,   218,   223,   225,   229,   231,   233,   236,   243,
     244,   246,   248,   250,   254,   258,   260,   262,   269,   271,
     275,   277,   278,   280,   285,   287,   289,   291,   292,   294,
     298,   299,   301,   303,   305,   307,   309,   310,   312,   318,
     320,   322,   323,   325,   326,   329,   333,   344,   347,   349,
     353,   354,   356,   357,   359,   360,   364,   365,   367,   371,
     376,   378,   381,   382,   386,   387,   390,   392,   394,   396,
     398,   400,   402,   406,   408,   412,   414,   419,   421,   424,
     426,   430,   432,   436,   438,   440,   442,   444,   446,   448,
     450,   452,   454,   457,   461,   464,   466,   468,   470,   472,
     474,   476,   479,   485,   488,   493,   498,   501,   504,   506,
     508,   509,   512,   516,   519,   521,   523,   527,   531,   534,
     536,   540,   543,   546,   550,   552,   554,   556,   559,   562,
     566,   568,   572,   574,   576,   578,   580,   582,   584,   586,
     589,   592,   595,   598,   599,   602,   604,   611,   616,   618,
     620,   622,   627,   632,   637,   642,   644,   646,   648,   650,
     652,   654,   656,   663,   665,   667,   669,   671,   673,   675,
     677,   679,   681,   683,   685,   687,   689,   693,   697,   702,
     707,   713,   715,   717,   719,   721,   723,   727,   731,   733,
     735,   737,   739,   741,   746,   748,   750,   752,   754,   755,
     758,   763,   764,   766,   773,   775,   777,   779,   781,   783,
     786,   789,   795,   797,   799,   800,   802,   811,   813,   815,
     818,   821,   823,   825,   827,   829,   830,   832,   835,   839,
     841,   845,   847,   851,   852,   854,   855,   857,   858,   860,
     865,   867,   871,   875,   877,   880,   881,   883,   887,   889,
     891,   893,   895,   898,   900,   903,   906,   911,   913,   915,
     917,   920,   922,   925,   929,   932,   935,   939,   945,   950,
     956,   958,   960,   962,   964,   966,   968,   970,   972,   974,
     976,   979,   981,   983,   984,   986,   988,   991,   996,  1002,
    1008,  1010,  1018,  1019,  1021,  1023,  1025,  1027,  1032,  1033,
    1035,  1037,  1039,  1041,  1043,  1045,  1047,  1049,  1051,  1053,
    1055,  1057,  1059,  1061,  1063,  1065,  1069,  1072,  1074,  1076,
    1083,  1085,  1087,  1089,  1091,  1093,  1095,  1097,  1101,  1103,
    1107,  1113,  1115,  1117,  1119,  1122,  1125,  1127,  1131,  1135,
    1139,  1141,  1145,  1149,  1151,  1153,  1155,  1158,  1161,  1163,
    1165,  1167,  1169,  1171,  1173,  1175,  1177,  1180,  1182,  1185,
    1188,  1191,  1195,  1197,  1199,  1203,  1207,  1209,  1211,  1215,
    1219,  1221,  1223,  1225,  1227,  1229,  1231,  1235,  1239,  1241,
    1243,  1246,  1248,  1251,  1253,  1255,  1257,  1265,  1267,  1269,
    1270,  1272,  1274,  1276,  1278,  1279,  1282,  1290,  1295,  1297,
    1299,  1304,  1311,  1318,  1325,  1328,  1330,  1332,  1334,  1338,
    1342,  1346,  1348,  1349,  1351,  1353,  1357,  1360,  1362,  1364,
    1366,  1367,  1371,  1372,  1374,  1378,  1381,  1383,  1385,  1387,
    1389,  1391,  1394,  1397,  1401,  1405,  1408,  1410,  1411,  1413,
    1417,  1418,  1420,  1424,  1427,  1428,  1430,  1432,  1434,  1436,
    1438,  1443,  1448,  1451,  1455,  1459,  1462,  1467,  1472,  1476,
    1478,  1484,  1489,  1492,  1495,  1499,  1502,  1504,  1509,  1512,
    1514,  1516,  1517,  1521,  1527,  1529,  1531,  1533,  1535,  1537,
    1539,  1541,  1543,  1545,  1548,  1551,  1553,  1555,  1557,  1559,
    1565,  1570,  1572,  1575,  1580,  1582,  1586,  1588,  1590,  1592,
    1594,  1596,  1598,  1600,  1603,  1608,  1609,  1612,  1614,  1616,
    1618,  1620,  1623,  1625,  1626,  1629,  1631,  1635,  1645,  1646,
    1649,  1651,  1653,  1656,  1658,  1660,  1663,  1664,  1667,  1669,
    1673,  1674,  1678,  1680,  1682,  1683,  1686,  1688,  1694,  1696,
    1700,  1702,  1704,  1707,  1712,  1717,  1722,  1727,  1729,  1731,
    1733,  1735,  1737,  1739,  1740,  1742,  1745,  1748,  1750,  1752,
    1756,  1758,  1760
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     258,     0,    -1,   259,    -1,   259,     7,    -1,   281,    -1,
     260,    -1,   261,    -1,   272,    -1,   537,    -1,    47,   117,
     488,     3,   262,     4,    -1,   263,    -1,   262,     5,   263,
      -1,   264,    -1,   269,    -1,   519,   496,   265,    -1,    -1,
     265,   268,    -1,   455,    -1,   122,    -1,   103,    84,    -1,
      23,    94,    -1,   267,    -1,    56,   361,    -1,    56,    94,
      -1,    56,   266,    -1,    41,    -1,    41,     3,   334,     4,
      -1,   107,   488,    -1,   107,   488,     3,   270,     4,    -1,
     267,     3,   270,     4,    -1,    70,    84,     3,   270,     4,
     107,   488,    -1,    70,    84,     3,   270,     4,   107,   488,
       3,   270,     4,    -1,    41,     3,   334,     4,    -1,   270,
       5,   519,    -1,   519,    -1,   271,     5,   495,    -1,   495,
      -1,    47,   128,   488,   275,    31,   302,   273,    -1,    -1,
     130,    41,    99,    -1,    -1,     3,   270,     4,    -1,    -1,
       3,   271,     4,    -1,    -1,   100,    38,   277,    -1,   278,
      -1,   277,     5,   278,    -1,   335,   279,    -1,   291,   279,
      -1,    -1,    32,    -1,    58,    -1,    -1,    23,    -1,   284,
      -1,   285,    -1,   286,    -1,   287,    -1,   292,    -1,   293,
      -1,   298,    -1,   282,    -1,   302,    -1,   302,   283,   440,
     282,    -1,    82,    -1,   121,    -1,    64,    -1,    43,   131,
      -1,    57,    72,   323,   301,   558,    -1,    67,   534,    80,
     299,    -1,    79,    80,   488,   275,   288,    -1,   127,     3,
     289,     4,    -1,   290,    -1,   289,     5,   290,    -1,   291,
      -1,   467,    -1,   108,   131,    -1,   110,   294,   303,    80,
     299,   316,    -1,    -1,    27,    -1,    59,    -1,   296,    -1,
     295,     5,   296,    -1,   495,   242,   297,    -1,   467,    -1,
      56,    -1,   124,   323,   111,   295,   301,   558,    -1,   300,
      -1,   299,     5,   300,    -1,   360,    -1,    -1,   324,    -1,
     110,   294,   303,   316,    -1,   288,    -1,   251,    -1,   358,
      -1,    -1,   305,    -1,   222,   311,   308,    -1,    -1,   312,
      -1,   207,    -1,   223,    -1,   175,    -1,   216,    -1,    -1,
     310,    -1,    67,   307,   306,   308,   224,    -1,   361,    -1,
     361,    -1,    -1,   315,    -1,    -1,   222,   453,    -1,   221,
     453,   314,    -1,   317,   301,   325,   326,   402,   276,   313,
     304,   309,   558,    -1,    72,   318,    -1,   323,    -1,   318,
       5,   323,    -1,    -1,    31,    -1,    -1,   175,    -1,    -1,
     319,    24,   274,    -1,    -1,   224,    -1,   322,   488,   321,
      -1,   322,   357,   536,   275,    -1,   438,    -1,   129,   334,
      -1,    -1,    74,    38,   464,    -1,    -1,    75,   334,    -1,
     120,    -1,    66,    -1,   123,    -1,    94,    -1,   335,    -1,
     329,    -1,     3,   334,     4,    -1,   467,    -1,     3,   334,
       4,    -1,   328,    -1,   328,    81,   280,   327,    -1,   331,
      -1,    23,   331,    -1,   332,    -1,   333,   239,   332,    -1,
     333,    -1,   334,   238,   333,    -1,   337,    -1,   340,    -1,
     351,    -1,   355,    -1,   356,    -1,   346,    -1,   349,    -1,
     343,    -1,   352,    -1,   338,   290,    -1,   290,   338,   290,
      -1,   338,   290,    -1,   244,    -1,   245,    -1,   242,    -1,
     243,    -1,   247,    -1,   246,    -1,    81,   280,    -1,   280,
      36,   290,   239,   290,    -1,   290,   339,    -1,   280,    85,
     468,   344,    -1,   280,    85,   448,   344,    -1,   290,   341,
      -1,   290,   342,    -1,   341,    -1,   342,    -1,    -1,    63,
     468,    -1,    81,   280,    94,    -1,   290,   345,    -1,   345,
      -1,   357,    -1,     3,   464,     4,    -1,   280,    76,   347,
      -1,   290,   348,    -1,   348,    -1,   338,   354,   357,    -1,
     290,   350,    -1,   290,   353,    -1,   280,   225,   374,    -1,
      30,    -1,    27,    -1,   113,    -1,    65,   357,    -1,   122,
     357,    -1,     3,   282,     4,    -1,   359,    -1,   358,     5,
     359,    -1,   487,    -1,   535,    -1,   231,    -1,    20,    -1,
      21,    -1,    22,    -1,    19,    -1,   361,   234,    -1,   361,
     231,    -1,   361,    20,    -1,   361,    22,    -1,    -1,    31,
     519,    -1,   519,    -1,   135,     3,   467,    76,   467,     4,
      -1,   135,     3,   464,     4,    -1,   363,    -1,   371,    -1,
     368,    -1,   134,     3,   467,     4,    -1,   137,     3,   467,
       4,    -1,    96,     3,   467,     4,    -1,   132,     3,   467,
       4,    -1,   365,    -1,   366,    -1,   367,    -1,   459,    -1,
     153,    -1,   369,    -1,   467,    -1,   142,     3,   370,    72,
     467,     4,    -1,   373,    -1,   361,    -1,   535,    -1,    94,
      -1,    66,    -1,   120,    -1,   179,    -1,   180,    -1,   181,
      -1,   182,    -1,   183,    -1,   184,    -1,   428,    -1,   377,
       3,     4,    -1,   375,     3,     4,    -1,   377,     3,   466,
       4,    -1,   376,     3,   466,     4,    -1,   377,     3,   294,
     465,     4,    -1,   378,    -1,   157,    -1,    24,    -1,   140,
      -1,   141,    -1,   380,   196,   400,    -1,   197,     3,     4,
      -1,   428,    -1,   381,    -1,   387,    -1,   393,    -1,   396,
      -1,   198,     3,   384,     4,    -1,   535,    -1,   361,    -1,
     383,    -1,   382,    -1,    -1,     5,   390,    -1,     5,   390,
       5,   391,    -1,    -1,   392,    -1,   388,     3,   389,   385,
       4,   386,    -1,   199,    -1,   200,    -1,   467,    -1,    21,
      -1,   467,    -1,   201,   203,    -1,   202,   203,    -1,   394,
       3,   467,     4,   386,    -1,   204,    -1,   205,    -1,    -1,
     398,    -1,   206,     3,   467,     5,   397,     4,   395,   386,
      -1,   383,    -1,   382,    -1,    72,   207,    -1,    72,   208,
      -1,    24,    -1,   399,    -1,   401,    -1,   407,    -1,    -1,
     403,    -1,   218,   404,    -1,   404,     5,   405,    -1,   405,
      -1,   406,    31,   407,    -1,   399,    -1,     3,   411,     4,
      -1,    -1,   412,    -1,    -1,   413,    -1,    -1,   417,    -1,
     408,   409,   276,   410,    -1,   399,    -1,   217,    38,   414,
      -1,   414,     5,   415,    -1,   415,    -1,   495,   498,    -1,
      -1,   427,    -1,   418,   419,   416,    -1,   216,    -1,   215,
      -1,   420,    -1,   422,    -1,   213,   214,    -1,   421,    -1,
      49,   175,    -1,   372,   214,    -1,    36,   423,   239,   424,
      -1,   425,    -1,   425,    -1,   420,    -1,   213,   212,    -1,
     426,    -1,   372,   212,    -1,   209,    49,   175,    -1,   209,
      74,    -1,   209,   211,    -1,   209,   219,   210,    -1,   429,
       3,   294,   465,     4,    -1,    46,     3,   251,     4,    -1,
      46,     3,   294,   465,     4,    -1,    35,    -1,    90,    -1,
      91,    -1,   116,    -1,   159,    -1,    30,    -1,   113,    -1,
      87,    -1,    88,    -1,    73,    -1,    98,   334,    -1,   431,
      -1,   439,    -1,    -1,    78,    -1,   430,    -1,   430,   101,
      -1,   323,    48,    83,   323,    -1,   323,    92,   433,    83,
     323,    -1,   323,   433,    83,   323,   432,    -1,   434,    -1,
     323,   433,    83,   323,   126,   488,   437,    -1,    -1,    61,
      -1,    62,    -1,   436,    -1,   435,    -1,   126,     3,   270,
       4,    -1,    -1,    27,    -1,   357,    -1,   467,    -1,   227,
      -1,   228,    -1,   229,    -1,   230,    -1,   231,    -1,   232,
      -1,   233,    -1,   234,    -1,   237,    -1,   235,    -1,   236,
      -1,    24,    -1,   443,    -1,    24,    13,    24,    -1,   444,
      25,    -1,   444,    -1,   445,    -1,    39,     3,   442,    31,
     446,     4,    -1,   372,    -1,   374,    -1,   449,    -1,   495,
      -1,   441,    -1,   520,    -1,   379,    -1,     3,   467,     4,
      -1,   447,    -1,   220,     3,     4,    -1,   493,    13,   220,
       3,     4,    -1,   448,    -1,   364,    -1,   450,    -1,   249,
     450,    -1,   248,   450,    -1,   451,    -1,   452,   251,   451,
      -1,   452,   252,   451,    -1,   452,   253,   451,    -1,   452,
      -1,   453,   248,   452,    -1,   453,   249,   452,    -1,   455,
      -1,   138,    -1,   139,    -1,   235,   468,    -1,   236,   468,
      -1,   454,    -1,   456,    -1,   457,    -1,   158,    -1,   149,
      -1,    52,    -1,   147,    -1,   148,    -1,   459,   501,    -1,
     459,    -1,   153,   501,    -1,   459,   501,    -1,   153,   513,
      -1,   460,   118,   461,    -1,   462,    -1,   467,    -1,   464,
       5,   467,    -1,   464,     7,   467,    -1,   531,    -1,   465,
      -1,   466,     5,   465,    -1,   466,     7,   465,    -1,   453,
      -1,   468,    -1,   458,    -1,   469,    -1,   473,    -1,   470,
      -1,   469,   248,   473,    -1,   467,   250,   467,    -1,   234,
      -1,   474,    -1,    42,   488,    -1,   471,    -1,   471,   472,
      -1,   480,    -1,   475,    -1,   476,    -1,   136,     3,   477,
      72,   468,   481,     4,    -1,   478,    -1,   479,    -1,    -1,
     482,    -1,   484,    -1,   485,    -1,   486,    -1,    -1,    69,
     467,    -1,   136,     3,   467,    72,   467,   481,     4,    -1,
     136,     3,   464,     4,    -1,   125,    -1,    89,    -1,   483,
       3,   467,     4,    -1,    45,     3,   468,   126,   488,     4,
      -1,    45,     3,   442,     5,   446,     4,    -1,   119,     3,
     468,   126,   488,     4,    -1,   467,   362,    -1,   491,    -1,
     490,    -1,   489,    -1,    24,    13,   490,    -1,    24,     6,
     490,    -1,    24,    13,   491,    -1,    24,    -1,    -1,    25,
      -1,   494,    -1,   493,    13,   494,    -1,    24,   492,    -1,
     251,    -1,   493,    -1,   499,    -1,    -1,    40,   111,    24,
      -1,    -1,   472,    -1,   500,   497,   498,    -1,   508,   498,
      -1,   510,    -1,   512,    -1,   516,    -1,   517,    -1,   518,
      -1,    40,   501,    -1,   133,   501,    -1,    40,   187,   502,
      -1,   133,   187,   502,    -1,   185,   502,    -1,   507,    -1,
      -1,   502,    -1,     3,    21,     4,    -1,    -1,   504,    -1,
       3,   505,     4,    -1,    21,   506,    -1,    -1,    14,    -1,
      15,    -1,    16,    -1,    17,    -1,    18,    -1,    40,   191,
     188,   503,    -1,   133,   191,   188,   503,    -1,   192,   503,
      -1,   190,    40,   501,    -1,   190,   133,   501,    -1,    93,
     501,    -1,   190,    40,   187,   502,    -1,   190,   133,   187,
     502,    -1,    93,   187,   502,    -1,   509,    -1,   190,    40,
     191,   188,   503,    -1,    93,   191,   188,   503,    -1,   189,
     503,    -1,   227,   501,    -1,   227,   187,   502,    -1,   186,
     502,    -1,   511,    -1,   227,   191,   188,   503,    -1,   193,
     503,    -1,   514,    -1,   515,    -1,    -1,     3,    21,     4,
      -1,     3,    21,     5,    21,     4,    -1,   230,    -1,   231,
      -1,   232,    -1,   233,    -1,    68,    -1,   106,    -1,   229,
      -1,   228,    -1,   235,    -1,   236,   501,    -1,   195,   463,
      -1,    24,    -1,   521,    -1,   522,    -1,   523,    -1,   162,
     533,   524,   530,   164,    -1,   162,   528,   530,   164,    -1,
     525,    -1,   528,   525,    -1,   165,   526,   163,   531,    -1,
     527,    -1,   526,     5,   527,    -1,   291,    -1,   336,    -1,
     339,    -1,   348,    -1,   341,    -1,   345,    -1,   529,    -1,
     528,   529,    -1,   165,   334,   163,   531,    -1,    -1,   166,
     531,    -1,   532,    -1,   467,    -1,   291,    -1,    24,    -1,
       6,    24,    -1,     8,    -1,    -1,   319,    24,    -1,   334,
      -1,     3,   259,     4,    -1,    47,   174,   557,   539,   540,
      98,   491,   538,   543,    -1,    -1,   171,   550,    -1,   167,
      -1,   168,    -1,   169,    97,    -1,    79,    -1,    57,    -1,
     124,   541,    -1,    -1,    97,   542,    -1,   270,    -1,   544,
     546,   547,    -1,    -1,    69,   170,   545,    -1,   175,    -1,
     176,    -1,    -1,   165,   330,    -1,   549,    -1,   172,   173,
     548,     7,   164,    -1,   549,    -1,   548,     7,   549,    -1,
     259,    -1,   551,    -1,   550,   551,    -1,   178,   320,   319,
     555,    -1,   177,   320,   319,   556,    -1,   178,   117,   319,
     552,    -1,   177,   117,   319,   553,    -1,   554,    -1,   554,
      -1,    24,    -1,    24,    -1,    24,    -1,    24,    -1,    -1,
     559,    -1,   226,   560,    -1,   560,   561,    -1,   561,    -1,
      24,    -1,    24,   242,   562,    -1,   361,    -1,    24,    -1,
     327,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   266,   266,   268,   276,   277,   339,   340,   341,   345,
     356,   359,   365,   366,   370,   379,   380,   386,   389,   390,
     398,   402,   403,   407,   411,   417,   418,   424,   428,   438,
     444,   453,   465,   474,   479,   487,   492,   500,   512,   513,
     523,   524,   532,   533,   544,   545,   555,   560,   574,   582,
     591,   592,   593,   609,   610,   616,   617,   618,   619,   620,
     621,   622,   623,   627,   632,   643,   644,   645,   648,   655,
     666,   675,   684,   694,   699,   707,   710,   718,   729,   740,
     741,   742,   746,   749,   755,   762,   763,   766,   778,   781,
     787,   791,   792,   800,   808,   812,   817,   820,   821,   824,
     833,   834,   837,   838,   841,   842,   845,   846,   849,   860,
     863,   867,   868,   871,   872,   880,   889,   906,   916,   919,
     925,   926,   929,   930,   933,   936,   945,   946,   950,   957,
     965,   988,   997,   998,  1006,  1007,  1015,  1016,  1017,  1018,
    1021,  1022,  1023,  1038,  1046,  1055,  1056,  1066,  1067,  1075,
    1076,  1085,  1086,  1096,  1097,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1110,  1117,  1124,  1149,  1150,  1151,  1152,  1153,
    1154,  1165,  1173,  1211,  1222,  1232,  1242,  1248,  1254,  1276,
    1301,  1302,  1309,  1318,  1324,  1340,  1344,  1352,  1361,  1367,
    1383,  1392,  1417,  1426,  1436,  1437,  1438,  1442,  1450,  1456,
    1467,  1472,  1488,  1493,  1524,  1525,  1526,  1527,  1528,  1530,
    1542,  1554,  1566,  1582,  1583,  1589,  1592,  1602,  1612,  1613,
    1614,  1617,  1625,  1636,  1646,  1656,  1661,  1666,  1673,  1678,
    1686,  1687,  1718,  1730,  1731,  1734,  1735,  1736,  1737,  1738,
    1739,  1740,  1741,  1742,  1743,  1746,  1748,  1755,  1762,  1770,
    1784,  1797,  1800,  1804,  1808,  1810,  1837,  1846,  1853,  1854,
    1855,  1856,  1857,  1860,  1870,  1873,  1876,  1877,  1880,  1881,
    1887,  1897,  1898,  1902,  1914,  1915,  1918,  1921,  1924,  1927,
    1928,  1931,  1942,  1943,  1946,  1947,  1950,  1964,  1965,  1968,
    1974,  1982,  1985,  1986,  1989,  1992,  1993,  1996,  2004,  2007,
    2012,  2021,  2024,  2033,  2034,  2037,  2038,  2041,  2042,  2045,
    2051,  2054,  2063,  2066,  2071,  2079,  2080,  2083,  2092,  2093,
    2096,  2097,  2100,  2106,  2107,  2115,  2123,  2133,  2136,  2139,
    2140,  2146,  2149,  2157,  2164,  2170,  2176,  2196,  2205,  2213,
    2225,  2226,  2227,  2228,  2229,  2230,  2231,  2235,  2240,  2245,
    2252,  2260,  2261,  2264,  2265,  2270,  2271,  2279,  2291,  2301,
    2310,  2315,  2329,  2330,  2331,  2334,  2335,  2338,  2350,  2351,
    2355,  2358,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,
    2370,  2371,  2372,  2373,  2377,  2382,  2392,  2401,  2402,  2406,
    2418,  2419,  2420,  2421,  2422,  2423,  2424,  2425,  2432,  2438,
    2446,  2458,  2459,  2463,  2464,  2470,  2479,  2480,  2487,  2494,
    2504,  2505,  2512,  2526,  2533,  2539,  2544,  2550,  2583,  2596,
    2623,  2684,  2685,  2686,  2687,  2688,  2691,  2699,  2700,  2709,
    2715,  2724,  2731,  2736,  2739,  2743,  2756,  2783,  2786,  2790,
    2803,  2804,  2805,  2808,  2816,  2817,  2820,  2827,  2837,  2838,
    2841,  2849,  2850,  2858,  2859,  2862,  2869,  2882,  2906,  2913,
    2926,  2927,  2928,  2933,  2940,  2941,  2949,  2960,  2970,  2971,
    2974,  2984,  2994,  3006,  3019,  3028,  3033,  3038,  3045,  3052,
    3061,  3071,  3079,  3080,  3088,  3093,  3111,  3117,  3125,  3195,
    3198,  3199,  3208,  3209,  3212,  3219,  3225,  3226,  3227,  3228,
    3229,  3232,  3238,  3244,  3251,  3258,  3264,  3267,  3268,  3271,
    3280,  3281,  3284,  3294,  3302,  3303,  3308,  3313,  3318,  3323,
    3330,  3338,  3346,  3354,  3361,  3368,  3374,  3382,  3390,  3397,
    3400,  3409,  3417,  3425,  3431,  3438,  3444,  3447,  3455,  3463,
    3464,  3467,  3468,  3475,  3509,  3510,  3511,  3512,  3529,  3530,
    3531,  3542,  3545,  3554,  3583,  3594,  3616,  3620,  3621,  3625,
    3636,  3646,  3651,  3658,  3668,  3671,  3676,  3677,  3678,  3679,
    3680,  3681,  3684,  3689,  3696,  3706,  3707,  3715,  3719,  3722,
    3725,  3738,  3744,  3768,  3771,  3781,  3795,  3798,  3813,  3816,
    3824,  3825,  3826,  3834,  3835,  3836,  3844,  3847,  3855,  3858,
    3867,  3870,  3879,  3880,  3883,  3886,  3894,  3895,  3906,  3911,
    3918,  3922,  3927,  3935,  3943,  3951,  3959,  3969,  3972,  3975,
    3978,  3981,  3984,  3988,  3989,  3993,  4002,  4007,  4015,  4021,
    4031,  4032,  4033
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
  "SQL_TOKEN_AVG", "SQL_TOKEN_BETWEEN", "SQL_TOKEN_BIT", "SQL_TOKEN_BY",
  "SQL_TOKEN_CAST", "SQL_TOKEN_CHARACTER", "SQL_TOKEN_CHECK",
  "SQL_TOKEN_COLLATE", "SQL_TOKEN_COMMIT", "SQL_TOKEN_CONTINUE",
  "SQL_TOKEN_CONVERT", "SQL_TOKEN_COUNT", "SQL_TOKEN_CREATE",
  "SQL_TOKEN_CROSS", "SQL_TOKEN_CURRENT", "SQL_TOKEN_CURSOR",
  "SQL_TOKEN_DATEVALUE", "SQL_TOKEN_DAY", "SQL_TOKEN_DEC",
  "SQL_TOKEN_DECIMAL", "SQL_TOKEN_DECLARE", "SQL_TOKEN_DEFAULT",
  "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC", "SQL_TOKEN_DISTINCT",
  "SQL_TOKEN_DROP", "SQL_TOKEN_FORWARD", "SQL_TOKEN_BACKWARD",
  "SQL_TOKEN_ESCAPE", "SQL_TOKEN_EXCEPT", "SQL_TOKEN_EXISTS",
  "SQL_TOKEN_FALSE", "SQL_TOKEN_FETCH", "SQL_TOKEN_FLOAT", "SQL_TOKEN_FOR",
  "SQL_TOKEN_FOREIGN", "SQL_TOKEN_FOUND", "SQL_TOKEN_FROM",
  "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP", "SQL_TOKEN_HAVING", "SQL_TOKEN_IN",
  "SQL_TOKEN_INDICATOR", "SQL_TOKEN_INNER", "SQL_TOKEN_INSERT",
  "SQL_TOKEN_INTO", "SQL_TOKEN_IS", "SQL_TOKEN_INTERSECT",
  "SQL_TOKEN_JOIN", "SQL_TOKEN_KEY", "SQL_TOKEN_LIKE", "SQL_TOKEN_LOCAL",
  "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT", "SQL_TOKEN_LOWER", "SQL_TOKEN_MAX",
  "SQL_TOKEN_MIN", "SQL_TOKEN_NATURAL", "SQL_TOKEN_NCHAR",
  "SQL_TOKEN_NULL", "SQL_TOKEN_NUMERIC", "SQL_TOKEN_OCTET_LENGTH",
  "SQL_TOKEN_OF", "SQL_TOKEN_ON", "SQL_TOKEN_OPTION", "SQL_TOKEN_ORDER",
  "SQL_TOKEN_OUTER", "SQL_TOKEN_PRECISION", "SQL_TOKEN_PRIMARY",
  "SQL_TOKEN_PROCEDURE", "SQL_TOKEN_PUBLIC", "SQL_TOKEN_REAL",
  "SQL_TOKEN_REFERENCES", "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_SCHEMA",
  "SQL_TOKEN_SELECT", "SQL_TOKEN_SET", "SQL_TOKEN_SMALLINT",
  "SQL_TOKEN_SOME", "SQL_TOKEN_SQLCODE", "SQL_TOKEN_SQLERROR",
  "SQL_TOKEN_SUM", "SQL_TOKEN_TABLE", "SQL_TOKEN_TO",
  "SQL_TOKEN_TRANSLATE", "SQL_TOKEN_TRUE", "SQL_TOKEN_UNION",
  "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN", "SQL_TOKEN_UPDATE",
  "SQL_TOKEN_UPPER", "SQL_TOKEN_USING", "SQL_TOKEN_VALUES",
  "SQL_TOKEN_VIEW", "SQL_TOKEN_WHERE", "SQL_TOKEN_WITH", "SQL_TOKEN_WORK",
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
  "SQL_TOKEN_CASE", "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN",
  "SQL_TOKEN_ELSE", "SQL_TOKEN_BEFORE", "SQL_TOKEN_AFTER",
  "SQL_TOKEN_INSTEAD", "SQL_TOKEN_EACH", "SQL_TOKEN_REFERENCING",
  "SQL_TOKEN_BEGIN", "SQL_TOKEN_ATOMIC", "SQL_TOKEN_TRIGGER",
  "SQL_TOKEN_ROW", "SQL_TOKEN_STATEMENT", "SQL_TOKEN_NEW", "SQL_TOKEN_OLD",
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
  "SQL_TOKEN_MATCH", "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BOOLEAN", "SQL_TOKEN_DOUBLE", "SQL_TOKEN_INTEGER",
  "SQL_TOKEN_INT", "SQL_TOKEN_LONG", "SQL_TOKEN_INT64", "SQL_TOKEN_STRING",
  "SQL_TOKEN_DATE", "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_DATETIME",
  "SQL_TOKEN_OR", "SQL_TOKEN_AND", "'|'", "'&'", "SQL_EQUAL", "SQL_GREAT",
  "SQL_LESS", "SQL_NOTEQUAL", "SQL_GREATEQ", "SQL_LESSEQ", "'+'", "'-'",
  "SQL_CONCAT", "'*'", "'/'", "'%'", "'~'", "'='",
  "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql_single_statement", "sql",
  "schema_element", "base_table_def", "base_table_element_commalist",
  "base_table_element", "column_def", "column_def_opt_list", "nil_fkt",
  "unique_spec", "column_def_opt", "table_constraint_def",
  "column_commalist", "column_ref_commalist", "view_def",
  "opt_with_check_option", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc", "sql_not",
  "manipulative_statement", "select_statement", "union_op",
  "commit_statement", "delete_statement_searched", "fetch_statement",
  "insert_statement", "values_or_query_spec",
  "row_value_constructor_commalist", "row_value_constructor",
  "row_value_constructor_elem", "rollback_statement",
  "select_statement_into", "opt_all_distinct", "assignment_commalist",
  "assignment", "update_source", "update_statement_searched",
  "target_commalist", "target", "opt_where_clause",
  "single_select_statement", "selection", "opt_result_offset_clause",
  "result_offset_clause", "opt_fetch_first_row_count", "first_or_next",
  "row_or_rows", "opt_fetch_first_clause", "fetch_first_clause",
  "offset_row_count", "fetch_first_row_count", "opt_limit_offset_clause",
  "opt_offset", "limit_offset_clause", "table_exp", "from_clause",
  "table_ref_commalist", "opt_as", "opt_row",
  "table_primary_as_range_column", "opt_only", "table_ref", "where_clause",
  "opt_group_by_clause", "opt_having_clause", "truth_value",
  "boolean_primary", "unary_predicate",
  "parenthesized_boolean_value_expression", "boolean_test",
  "boolean_factor", "boolean_term", "search_condition", "predicate",
  "comparison_predicate_part_2", "comparison_predicate", "comparison",
  "between_predicate_part_2", "between_predicate",
  "character_like_predicate_part_2", "other_like_predicate_part_2",
  "like_predicate", "opt_escape", "null_predicate_part_2", "test_for_null",
  "in_predicate_value", "in_predicate_part_2", "in_predicate",
  "quantified_comparison_predicate_part_2", "all_or_any_predicate",
  "rtreematch_predicate", "rtreematch_predicate_part_2", "any_all_some",
  "existence_test", "unique_test", "subquery", "scalar_exp_commalist",
  "select_sublist", "parameter_ref", "literal", "as_clause",
  "position_exp", "num_value_fct", "char_length_exp", "octet_length_exp",
  "bit_length_exp", "length_exp", "datetime_field", "extract_field",
  "extract_exp", "unsigned_value_spec", "general_value_spec", "fct_spec",
  "function_name0", "function_name12", "function_name",
  "date_function_0Argument", "window_function", "window_function_type",
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
  "join_type", "cross_union", "qualified_join", "ecrelationship_join",
  "op_relationship_direction", "joined_table", "named_columns_join", "all",
  "scalar_subquery", "cast_operand", "cast_target_primitive_type",
  "cast_target_scalar", "cast_target_array", "cast_target", "cast_spec",
  "value_exp_primary", "ecclassid_fct_spec", "num_primary", "factor",
  "term", "num_value_exp", "datetime_primary", "datetime_value_fct",
  "datetime_factor", "datetime_term", "datetime_value_exp",
  "non_second_datetime_field", "start_field", "end_field",
  "single_datetime_field", "interval_qualifier", "value_exp_commalist",
  "function_arg", "function_args_commalist", "value_exp",
  "string_value_exp", "char_value_exp", "concatenation", "char_primary",
  "collate_clause", "char_factor", "string_value_fct", "bit_value_fct",
  "bit_substring_fct", "bit_value_exp", "bit_factor", "bit_primary",
  "char_value_fct", "for_length", "char_substring_fct", "upper_lower",
  "fold", "form_conversion", "char_translation", "derived_column",
  "table_node", "catalog_name", "schema_name", "table_name",
  "opt_column_array_idx", "property_path", "property_path_entry",
  "column_ref", "data_type", "opt_char_set_spec", "opt_collate_clause",
  "predefined_type", "character_string_type", "opt_paren_precision",
  "paren_char_length", "opt_paren_char_large_length",
  "paren_character_large_object_length", "large_object_length",
  "opt_multiplier", "character_large_object_type",
  "national_character_string_type", "national_character_large_object_type",
  "binary_string_type", "binary_large_object_string_type", "numeric_type",
  "opt_paren_precision_scale", "exact_numeric_type",
  "approximate_numeric_type", "boolean_type", "datetime_type",
  "interval_type", "column", "case_expression", "case_specification",
  "simple_case", "searched_case", "simple_when_clause_list",
  "simple_when_clause", "when_operand_list", "when_operand",
  "searched_when_clause_list", "searched_when_clause", "else_clause",
  "result", "result_expression", "case_operand", "cursor", "parameter",
  "range_variable", "trigger_definition", "op_referencing",
  "trigger_action_time", "trigger_event", "op_trigger_columnlist",
  "trigger_column_list", "triggered_action", "op_triggered_action_for",
  "trigger_for", "triggered_when_clause", "triggered_SQL_statement",
  "SQL_procedure_statement_list", "SQL_procedure_statement",
  "transition_table_or_variable_list", "transition_table_or_variable",
  "old_transition_table_name", "new_transition_table_name",
  "transition_table_name", "old_transition_variable_name",
  "new_transition_variable_name", "trigger_name",
  "opt_ecsqloptions_clause", "ecsqloptions_clause", "ecsqloptions_list",
  "ecsqloption", "ecsqloptionvalue", YY_NULL
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
     124,    38,   479,   480,   481,   482,   483,   484,    43,    45,
     485,    42,    47,    37,   126,    61,   486
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   257,   258,   258,   259,   259,   260,   260,   260,   261,
     262,   262,   263,   263,   264,   265,   265,   266,   267,   267,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   269,
     269,   269,   269,   270,   270,   271,   271,   272,   273,   273,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   279,   280,   280,   281,   281,   281,   281,   281,
     281,   281,   281,   282,   282,   283,   283,   283,   284,   285,
     286,   287,   288,   289,   289,   290,   291,   292,   293,   294,
     294,   294,   295,   295,   296,   297,   297,   298,   299,   299,
     300,   301,   301,   302,   302,   303,   303,   304,   304,   305,
     306,   306,   307,   307,   308,   308,   309,   309,   310,   311,
     312,   313,   313,   314,   314,   315,   316,   317,   318,   318,
     319,   319,   320,   320,   321,   321,   322,   322,   323,   323,
     323,   324,   325,   325,   326,   326,   327,   327,   327,   327,
     328,   328,   328,   329,   330,   331,   331,   332,   332,   333,
     333,   334,   334,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   336,   337,   337,   338,   338,   338,   338,   338,
     338,   338,   339,   340,   341,   342,   343,   343,   343,   343,
     344,   344,   345,   346,   346,   347,   347,   348,   349,   349,
     350,   351,   352,   353,   354,   354,   354,   355,   356,   357,
     358,   358,   359,   360,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   362,   362,   362,   363,   363,   364,   364,
     364,   365,   365,   366,   367,   368,   368,   368,   369,   369,
     370,   370,   371,   372,   372,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   374,   374,   374,   374,   374,
     374,   375,   376,   377,   378,   378,   379,   380,   380,   380,
     380,   380,   380,   381,   382,   383,   384,   384,   385,   385,
     385,   386,   386,   387,   388,   388,   389,   390,   391,   392,
     392,   393,   394,   394,   395,   395,   396,   397,   397,   398,
     398,   399,   400,   400,   401,   402,   402,   403,   404,   404,
     405,   406,   407,   408,   408,   409,   409,   410,   410,   411,
     412,   413,   414,   414,   415,   416,   416,   417,   418,   418,
     419,   419,   420,   420,   420,   421,   422,   423,   424,   425,
     425,   425,   426,   427,   427,   427,   427,   428,   428,   428,
     429,   429,   429,   429,   429,   429,   429,   430,   430,   430,
     431,   432,   432,   433,   433,   433,   433,   434,   435,   435,
     435,   436,   437,   437,   437,   438,   438,   439,   440,   440,
     441,   442,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   443,   443,   444,   444,   445,   446,   446,   447,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   449,
     449,   450,   450,   451,   451,   451,   452,   452,   452,   452,
     453,   453,   453,   454,   455,   455,   455,   455,   456,   457,
     458,   459,   459,   459,   459,   459,   460,   461,   461,   462,
     462,   463,   463,   464,   464,   464,   465,   466,   466,   466,
     467,   467,   467,   468,   469,   469,   470,   470,   471,   471,
     472,   473,   473,   474,   474,   475,   476,   477,   478,   479,
     480,   480,   480,   480,   481,   481,   482,   482,   483,   483,
     484,   485,   485,   486,   487,   488,   488,   488,   489,   489,
     490,   491,   492,   492,   493,   493,   494,   494,   495,   496,
     497,   497,   498,   498,   499,   499,   499,   499,   499,   499,
     499,   500,   500,   500,   500,   500,   500,   501,   501,   502,
     503,   503,   504,   505,   506,   506,   506,   506,   506,   506,
     507,   507,   507,   508,   508,   508,   508,   508,   508,   508,
     509,   509,   509,   510,   510,   510,   510,   511,   511,   512,
     512,   513,   513,   513,   514,   514,   514,   514,   515,   515,
     515,   516,   517,   517,   518,   519,   520,   521,   521,   522,
     523,   524,   524,   525,   526,   526,   527,   527,   527,   527,
     527,   527,   528,   528,   529,   530,   530,   531,   532,   533,
     534,   535,   535,   536,   536,   259,   259,   537,   538,   538,
     539,   539,   539,   540,   540,   540,   541,   541,   542,   543,
     544,   544,   545,   545,   546,   546,   547,   547,   548,   548,
     549,   550,   550,   551,   551,   551,   551,   552,   553,   554,
     555,   556,   557,   558,   558,   559,   560,   560,   561,   561,
     562,   562,   562
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
       1,     1,     1,     1,     4,     1,     1,     1,     2,     5,
       4,     5,     4,     1,     3,     1,     1,     2,     6,     0,
       1,     1,     1,     3,     3,     1,     1,     6,     1,     3,
       1,     0,     1,     4,     1,     1,     1,     0,     1,     3,
       0,     1,     1,     1,     1,     1,     0,     1,     5,     1,
       1,     0,     1,     0,     2,     3,    10,     2,     1,     3,
       0,     1,     0,     1,     0,     3,     0,     1,     3,     4,
       1,     2,     0,     3,     0,     2,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     4,     1,     2,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     1,     1,     1,     1,     1,
       1,     2,     5,     2,     4,     4,     2,     2,     1,     1,
       0,     2,     3,     2,     1,     1,     3,     3,     2,     1,
       3,     2,     2,     3,     1,     1,     1,     2,     2,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     0,     2,     1,     6,     4,     1,     1,
       1,     4,     4,     4,     4,     1,     1,     1,     1,     1,
       1,     1,     6,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       5,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     0,     2,
       4,     0,     1,     6,     1,     1,     1,     1,     1,     2,
       2,     5,     1,     1,     0,     1,     8,     1,     1,     2,
       2,     1,     1,     1,     1,     0,     1,     2,     3,     1,
       3,     1,     3,     0,     1,     0,     1,     0,     1,     4,
       1,     3,     3,     1,     2,     0,     1,     3,     1,     1,
       1,     1,     2,     1,     2,     2,     4,     1,     1,     1,
       2,     1,     2,     3,     2,     2,     3,     5,     4,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     0,     1,     1,     2,     4,     5,     5,
       1,     7,     0,     1,     1,     1,     1,     4,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     6,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       5,     1,     1,     1,     2,     2,     1,     3,     3,     3,
       1,     3,     3,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     2,     2,
       2,     3,     1,     1,     3,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       2,     1,     2,     1,     1,     1,     7,     1,     1,     0,
       1,     1,     1,     1,     0,     2,     7,     4,     1,     1,
       4,     6,     6,     6,     2,     1,     1,     1,     3,     3,
       3,     1,     0,     1,     1,     3,     2,     1,     1,     1,
       0,     3,     0,     1,     3,     2,     1,     1,     1,     1,
       1,     2,     2,     3,     3,     2,     1,     0,     1,     3,
       0,     1,     3,     2,     0,     1,     1,     1,     1,     1,
       4,     4,     2,     3,     3,     2,     4,     4,     3,     1,
       5,     4,     2,     2,     3,     2,     1,     4,     2,     1,
       1,     0,     3,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     1,     1,     1,     1,     5,
       4,     1,     2,     4,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     4,     0,     2,     1,     1,     1,
       1,     2,     1,     0,     2,     1,     3,     9,     0,     2,
       1,     1,     2,     1,     1,     2,     0,     2,     1,     3,
       0,     3,     1,     1,     0,     2,     1,     5,     1,     3,
       1,     1,     2,     4,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     0,     1,     2,     2,     1,     1,     3,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      53,    53,     0,   582,   208,   205,   206,   207,    53,   482,
     345,   340,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,    53,   469,   341,   342,   236,     0,     0,    79,
     346,   343,     0,   238,     0,   126,   468,     0,     0,     0,
       0,     0,     0,   414,   415,   254,   255,     0,   252,   344,
       0,   239,   240,   241,   242,   243,   244,     0,     0,   274,
     275,   282,   283,     0,     0,   204,   448,     0,     0,   167,
     168,   165,   166,   170,   169,     0,     0,   487,     0,     2,
       5,     6,     7,     0,     4,    62,    55,    56,    57,    58,
      94,    53,    75,    59,    60,    61,    63,   145,   141,   147,
     149,   151,   585,   140,   153,     0,   154,   178,   179,   160,
     184,   158,   189,   159,   155,   161,   156,   157,   370,   234,
     218,   402,   225,   226,   227,   220,   219,   390,   233,   391,
       0,     0,     0,   251,   396,     0,   259,   260,     0,   261,
       0,   262,   245,     0,   394,   398,   401,   392,   403,   406,
     410,   440,   418,   413,   419,   420,   442,   143,   441,   443,
     445,   451,   444,   449,   454,   455,   453,   460,     0,   461,
     462,   463,   488,   484,   393,   395,   556,   557,   558,   235,
       8,     0,     0,     0,    76,   581,    53,    54,   148,   483,
     486,     0,    68,     0,    79,     0,     0,     0,   126,     0,
     197,   580,     0,     0,   171,     0,    77,    80,    81,     0,
       0,   198,   127,     0,   353,   360,   366,   365,   130,     0,
       0,     0,     0,   459,     0,     0,     0,    53,   579,    76,
     575,   572,     0,     0,     0,     0,     0,     0,   416,   417,
     405,   404,     1,     3,     0,     0,     0,     0,   173,   176,
     177,   183,   188,   191,   192,    67,    65,    66,   368,    53,
      53,    53,   164,   211,   212,   210,   209,     0,     0,     0,
       0,     0,     0,    79,     0,     0,     0,     0,     0,     0,
       0,     0,   452,     0,     0,   586,   199,   142,   397,    79,
       0,     0,     0,   371,     0,   441,     0,     0,   481,     0,
     477,   476,   475,    42,   622,     0,    91,     0,    42,   182,
       0,   487,     0,    96,   200,   213,   202,   441,   583,   124,
       0,   349,   354,   347,   348,   353,     0,   355,     0,     0,
      73,     0,     0,     0,   433,     0,   433,     0,   457,   458,
       0,   423,   424,   425,   422,   229,   421,   230,     0,   228,
     231,     0,     0,     0,   573,     0,    53,   575,   561,     0,
     257,   265,   267,   266,     0,   264,     0,   399,     0,   187,
     185,   180,   180,     0,     0,   195,   194,   196,   163,     0,
     369,     0,     0,   150,   152,   247,   437,     0,   578,   436,
     577,   246,     0,     0,   303,   291,   292,   256,   293,   294,
     268,   276,     0,     0,   407,   408,   409,   411,   412,   447,
     446,   450,     0,   482,     0,   485,     0,     0,     0,     0,
     338,     0,     0,     0,     0,     0,     0,   590,   591,     0,
       0,    53,   623,    92,    70,    88,    90,   203,     0,   223,
     126,     0,    93,    91,     0,   555,     0,   474,   215,     0,
     121,     0,    42,     0,   128,   126,     0,    91,    82,   488,
       0,   356,   126,    72,     0,   224,   221,   217,     0,     0,
       0,   467,     0,     0,   222,     0,     0,   576,   560,     0,
      75,   567,     0,   568,   178,   184,   189,     0,   564,     0,
     562,   263,     0,     0,   433,     0,   175,   174,     0,   253,
     193,   245,   190,    64,   137,   139,   136,   138,   146,   249,
       0,     0,     0,   248,   310,   305,     0,   304,     0,     0,
     271,     0,   470,     0,     0,   383,   372,   373,   374,   375,
     376,   377,   378,   379,   381,   382,   380,   384,   387,   388,
       0,     0,     0,   339,     0,   479,   481,   478,   480,     0,
       0,     0,    18,     0,    10,    12,     0,    13,     0,     0,
      36,     0,   592,   594,   593,   596,     0,   131,     0,    69,
     624,     0,    71,   117,   118,     0,   132,   201,   214,     0,
     584,   129,    40,   357,   126,     0,   623,     0,     0,   353,
      74,   434,   435,     0,   464,   441,     0,   574,   164,    53,
       0,   559,   288,   287,     0,   186,   181,     0,   438,   439,
     250,     0,    44,   306,   302,   277,   269,   271,     0,     0,
     281,   272,   337,   400,     0,   386,   389,   472,   471,     0,
      53,     0,    19,     9,     0,     0,   507,   548,   507,   549,
     507,     0,     0,   510,     0,   510,   510,     0,   507,   551,
     550,   544,   545,   546,   547,   552,   507,    15,   489,   490,
     506,   492,   529,   496,   536,   497,   539,   540,   498,   499,
     500,    43,     0,    38,     0,   595,     0,   628,   625,   627,
      89,   126,    78,     0,   134,   473,     0,   125,   358,    83,
      87,    86,    84,    85,    53,     0,   351,   359,   352,   216,
       0,     0,     0,   232,     0,   566,     0,   570,   571,   569,
     565,   563,   284,   172,     0,     0,   307,     0,   273,   279,
     280,   385,   481,     0,     0,    11,     0,    34,     0,     0,
       0,   501,   508,     0,     0,   525,     0,     0,   502,   505,
     535,     0,   532,   511,   507,   507,   522,   538,   541,   507,
       0,   432,   554,     0,     0,   533,   553,    14,     0,   492,
     493,   495,    35,     0,    37,   598,   597,   588,     0,   626,
     119,     0,    53,   295,     0,   350,     0,   362,   465,   466,
     456,     0,   162,     0,   271,   285,   311,   313,   492,    53,
     319,   318,   309,   308,     0,   270,   278,    32,     0,    29,
       0,     0,   503,   510,   528,   510,   504,   510,   514,     0,
       0,     0,   523,     0,   524,     0,   430,   429,     0,   534,
     510,     0,    25,     0,     0,    21,    16,     0,   494,     0,
       0,   600,   631,   632,   630,   629,   133,   135,     0,    44,
     296,    41,     0,   363,   364,   361,   289,   290,   286,     0,
     314,    45,    46,    50,    50,     0,     0,     0,     0,   315,
     320,   323,   321,     0,    33,   509,   520,   531,   521,   515,
     516,   517,   518,   519,   513,   512,   526,   510,   527,     0,
     507,   427,   431,   537,    20,    53,    23,    24,    22,    17,
      27,   491,    39,   122,   122,   589,   611,     0,   587,   604,
     301,   297,   299,     0,   111,   367,   312,    53,    51,    52,
      49,    48,     0,     0,   329,     0,   327,   331,   324,   322,
     325,     0,   317,   316,     0,   530,   542,     0,   428,     0,
       0,   120,   123,   120,   120,   120,   612,     0,     0,    53,
       0,     0,     0,    97,   112,    47,   330,   332,     0,     0,
     334,   335,     0,    30,     0,    26,     0,     0,     0,     0,
       0,   602,   603,   601,    53,   605,     0,   610,   599,   606,
     298,   300,   113,     0,   106,    98,   326,   328,   333,   336,
       0,   543,    28,   619,   616,   618,   621,   614,   615,   617,
     620,   613,     0,    53,     0,   115,     0,   109,     0,   623,
     107,     0,   144,     0,   608,   114,   104,   105,    99,   102,
     103,   100,   116,    31,    53,     0,   101,   110,   607,   609,
       0,   108
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    78,   967,    80,    81,   553,   554,   555,   757,   887,
     556,   826,   557,   726,   559,    82,   764,   687,   426,   716,
     851,   852,   910,    83,    84,    85,   258,    86,    87,    88,
      89,    90,   329,    91,    92,    93,    94,   209,   457,   458,
     692,    95,   434,   435,   432,    96,   312,   974,   975,  1015,
    1011,  1008,   999,  1000,   996,  1016,   943,   995,   944,   442,
     443,   573,   451,   933,   454,   213,   214,   433,   684,   773,
     508,    97,    98,   965,    99,   100,   101,   102,   103,   481,
     104,   105,   483,   106,   107,   108,   109,   496,   110,   111,
     369,   112,   113,   253,   114,   115,   254,   379,   116,   117,
     118,   313,   314,   436,   119,   447,   120,   121,   122,   123,
     124,   125,   347,   348,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   362,   363,   364,   519,   620,
     137,   138,   400,   616,   795,   621,   139,   140,   784,   141,
     604,   785,   900,   397,   398,   839,   840,   901,   902,   903,
     399,   515,   612,   792,   516,   517,   613,   786,   787,   922,
     793,   794,   859,   914,   861,   862,   915,   976,   916,   917,
     923,   142,   143,   327,   696,   697,   328,   215,   216,   217,
     845,   218,   698,   381,   144,   292,   537,   538,   539,   540,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   349,   750,   882,   751,   752,   333,   386,   387,
     157,   158,   159,   160,   161,   760,   162,   163,   164,   165,
     337,   338,   339,   166,   701,   167,   168,   169,   170,   171,
     316,   299,   300,   301,   302,   190,   172,   173,   174,   657,
     759,   761,   658,   659,   731,   732,   742,   743,   809,   874,
     660,   661,   662,   663,   664,   665,   816,   666,   667,   668,
     669,   670,   727,   175,   176,   177,   178,   357,   358,   487,
     488,   230,   231,   355,   389,   390,   232,   202,   179,   452,
     180,   831,   430,   566,   675,   766,   898,   899,   963,   939,
     968,  1003,   969,   895,   896,   988,   984,   985,   991,   987,
     305,   569,   570,   678,   679,   835
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -915
static const yytype_int16 yypact[] =
{
    1279,  1279,    73,  -915,  -915,  -915,  -915,  -915,  1773,    91,
    -915,  -915,    70,   -16,   164,   208,   -39,   143,   246,  -915,
     284,   184,   321,  -915,  -915,  -915,  -915,   379,   262,    68,
    -915,  -915,   398,  -915,   246,   190,  -915,   444,   465,   471,
     495,   519,   529,  -915,  -915,  -915,  -915,   533,  -915,  -915,
    3314,  -915,  -915,  -915,  -915,  -915,  -915,   552,   564,  -915,
    -915,  -915,  -915,   583,   587,  -915,  -915,  3772,  3772,  -915,
    -915,  -915,  -915,  -915,  -915,  4147,  4147,  -915,   417,   424,
    -915,  -915,  -915,    89,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,   123,  -915,  -915,  -915,  -915,   333,   411,  -915,  -915,
    -915,   355,   359,  -915,  -915,  3772,  -915,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   102,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
     606,   623,   625,  -915,  -915,   440,  -915,  -915,   636,  -915,
     639,  -915,   453,   653,  -915,  -915,  -915,  -915,  -915,  -915,
     360,  -146,  -915,  -915,  -915,  -915,  -915,    78,  -915,   372,
    -915,   616,  -915,  -915,  -915,  -915,  -915,  -915,   660,  -915,
    -915,  -915,   673,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,   675,   685,    22,    56,  -915,  1526,  -915,  -915,  -915,
    -915,  3772,  -915,  3772,     7,   667,   667,   670,   190,    71,
    -915,  -915,   632,   667,   602,  3772,  -915,  -915,  -915,  3897,
    3772,  -915,  -915,   167,   617,  -915,  -915,  -915,  -915,  3772,
    3772,  3772,  3772,  3772,  3772,  2761,  3189,  2020,  -915,   467,
      31,  -915,   558,   720,    85,  3772,   722,   467,  -915,  -915,
    -915,  -915,  -915,  -915,   724,  3772,    90,  3439,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   702,   321,
    2020,  2020,  -915,  -915,  -915,  -915,  -915,   726,  3772,  2981,
     186,  3772,  3772,    68,  4022,  4022,  4022,  4022,  4022,  3772,
      67,   667,  -915,  3772,     1,  -915,  -915,  -915,  -915,    68,
     685,    22,   701,   467,   728,   605,   730,  3772,   158,   732,
    -915,  -915,  -915,   733,  -915,   456,   574,   365,   733,  -915,
      25,   128,   300,   734,  -915,    61,  -915,   618,   306,   306,
     662,  -915,  -915,  -915,  -915,   318,    17,   642,   663,   313,
    -915,    33,    42,   414,   -13,   466,   -15,   676,  -915,  -915,
      43,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   677,  -915,
     467,    46,  -102,  3772,  -915,   586,  2020,   581,  -915,   558,
    -915,   102,  -915,  -915,   747,  -915,    40,  -915,  3189,  -915,
    -915,   185,   -25,  3772,   597,  -915,  -915,  -915,  -915,   246,
    -915,    71,    82,  -915,   355,  -915,  -915,   489,   467,  -915,
    -915,  -915,  3772,   538,   729,  -915,  -915,  -915,  -915,  -915,
     750,   467,    47,  3772,  -915,  -915,  -915,   360,   360,  -915,
    -915,  -915,    48,   727,   754,  -915,  3897,   230,   230,   667,
    -915,   755,   736,   737,   476,    17,   731,  -915,  -915,   661,
      54,  2020,   537,  -915,   759,  -915,  -915,  -915,   638,  -915,
     190,   365,  -915,   637,  3772,  -915,   743,  -915,  -915,   667,
    -915,   744,   733,   745,  -915,   190,   687,    51,  -915,   760,
     530,  -915,   190,  -915,  3772,  -915,  -915,  -915,  3772,  3772,
    3772,  -915,  3772,  3772,  -915,  3772,  3772,  -915,  -915,    23,
      76,  -915,  3772,  -915,   769,   770,   771,    81,  -915,   613,
    -915,  -915,    85,   561,   467,  3772,  -915,  -915,   539,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    3772,  3772,   777,  -915,  -915,   565,   779,  -915,   763,   781,
     221,   782,  -915,   783,   718,   786,  -915,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   767,  -915,
     790,   791,   792,  -915,   787,  -915,   787,  -915,  -915,   795,
     717,   719,  -915,   336,  -915,  -915,   799,  -915,   445,   439,
    -915,    71,  -915,  -915,  -915,   713,   714,   359,   789,  -915,
    -915,   365,  -915,   806,   619,    69,   740,  -915,  -915,   812,
    -915,  -915,   814,   619,   190,    17,   537,    17,  3647,   443,
    -915,   467,   467,    49,   -34,    80,    50,  -915,   139,  2514,
    3772,  -915,  -915,  -915,   815,  -915,   568,  3772,  -915,  -915,
    -915,   784,   721,  -915,  -915,  -915,   818,   221,   622,   624,
    -915,  -915,  -915,  -915,   802,  -915,  -915,  -915,  -915,   805,
    2020,   830,  -915,  -915,   476,   743,   122,  -915,   152,  -915,
     187,   831,   831,   832,    28,   832,   832,   435,   192,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,   831,  -915,  -915,   796,
    -915,   616,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,    17,   707,   743,  -915,   805,   596,   789,  -915,
    -915,   190,  -915,   801,   765,  -915,   743,  -915,   619,  -915,
    -915,  -915,  -915,   467,  2020,   235,  -915,  -915,  -915,  -915,
    3772,   837,   839,  -915,   327,  -915,  3772,  -915,  -915,  -915,
    -915,  -915,   772,  -915,    17,   807,   240,  3772,  -915,  -915,
    -915,  -915,  -915,    24,   743,  -915,   484,  -915,   825,   831,
     659,  -915,  -915,   831,   665,  -915,   831,   668,  -915,  -915,
    -915,   827,  -915,  -915,   203,    36,  -915,  -915,   846,   831,
     739,  -915,  -915,   831,   671,  -915,  -915,   488,   749,   616,
    -915,  -915,  -915,   813,  -915,   850,  -915,   690,   429,  -915,
     619,  3772,  2020,   644,   497,   359,   743,   486,   467,  -915,
    -915,  3772,  -915,   369,   221,  -915,   858,  -915,   616,  2267,
    -915,  -915,  -915,  -915,   326,  -915,   467,  -915,   576,  -915,
     743,   861,  -915,   832,  -915,   832,  -915,   832,   546,   863,
     831,   681,  -915,   831,  -915,   847,  -915,   752,   506,  -915,
     832,   778,   868,   118,   667,  -915,  -915,   849,  -915,   788,
     428,   808,  -915,  -915,   102,  -915,   521,   359,   729,   721,
    -915,  -915,   664,  -915,  -915,  -915,  -915,  -915,  -915,    17,
    -915,   873,  -915,   328,   130,   420,   705,   669,   672,   679,
    -915,  -915,  -915,   775,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,   832,  -915,   666,
     831,  -915,  -915,  -915,  -915,  2020,  -915,  -915,   102,  -915,
     882,  -915,  -915,   -52,   -46,   428,  -915,   738,  -915,   725,
    -915,   884,  -915,   860,   678,  -915,  -915,  2267,  -915,  -915,
    -915,  -915,   375,   436,  -915,   655,  -915,  -915,  -915,  -915,
    -915,   197,  -915,  -915,   667,  -915,  -915,   871,  -915,    27,
     743,   865,  -915,   865,   865,   865,  -915,   507,   894,   785,
     729,   897,  4022,   684,  -915,  -915,  -915,  -915,   420,   741,
    -915,  -915,   692,   900,   907,  -915,   680,   889,   890,   889,
     904,  -915,  -915,  -915,  2020,  -915,   756,  -915,  -915,  -915,
    -915,  -915,   -65,    98,   851,  -915,  -915,  -915,  -915,  -915,
     743,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,    29,  1279,  4022,  -915,   -88,   102,   112,   537,
    -915,   694,  -915,   923,  -915,  -146,  -915,  -915,  -915,  -915,
    -915,    98,  -915,  -915,  1032,   -88,  -915,   102,  -915,  -915,
     708,  -915
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -915,  -915,   700,  -915,  -915,  -915,   297,  -915,  -915,  -915,
     176,  -915,  -915,  -642,  -915,  -915,  -915,  -915,  -259,    95,
    -915,    30,    84,   -14,  -915,    21,  -915,  -915,  -915,  -915,
    -915,   498,  -915,   -87,   -49,  -915,  -915,   156,  -915,   350,
    -915,  -915,   499,   368,  -285,   380,   527,  -915,  -915,  -915,
    -915,   -70,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   371,
    -915,  -915,  -317,    55,  -915,  -915,  -177,  -915,  -915,  -915,
     180,  -915,  -915,  -915,   942,   691,   693,    -1,  -720,  -915,
    -915,   -84,   862,  -915,   -75,   864,  -915,   580,   -74,  -915,
    -915,   -72,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
       6,  -915,   512,  -915,  -231,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -724,  -915,   584,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,   468,   469,  -915,  -915,  -575,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,  -215,  -915,  -915,  -915,  -915,  -915,    19,  -915,
      32,  -915,  -915,  -915,  -915,  -915,  -915,  -915,   113,  -915,
    -915,  -915,  -915,   177,  -915,  -915,  -915,  -915,    26,  -915,
    -915,   598,  -915,  -915,  -915,  -915,   645,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,   793,  -915,  -915,  -915,   557,
    -915,   735,  -915,   640,   370,   442,  -852,  -915,   153,  -915,
    -915,  -915,  -604,  -915,  -915,  -915,  -915,  -217,  -152,   709,
       8,   -63,  -915,  -915,  -915,   816,   699,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,   386,  -915,  -915,  -915,  -915,  -915,
    -915,  -183,  -915,   299,  -387,  -915,  -316,  -261,  -315,  -915,
    -915,  -692,  -915,  -915,  -576,  -254,  -493,  -915,  -915,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,  -303,  -915,  -915,  -915,  -915,  -915,   628,  -915,
     389,   761,  -182,   635,  -326,  -915,  -915,  -915,  -219,  -915,
    -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,  -915,
    -915,  -915,  -914,  -915,    99,  -915,  -915,    37,  -915,  -915,
    -915,  -572,  -915,  -915,   317,  -915
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -572
static const yytype_int16 yytable[] =
{
     183,   228,   453,   361,   238,   239,   335,   247,   204,   184,
     459,   460,   448,   303,   690,   365,   249,   251,   262,   252,
     308,   306,   182,   415,   200,   413,   287,   477,   797,   439,
     319,   955,   765,  1002,   207,   700,   548,   465,   495,   728,
     211,   413,   718,   749,   774,   492,   466,   474,   354,   438,
     288,   520,   522,   699,   703,   396,   585,   472,   229,   373,
     288,   476,   735,   470,   738,   931,   208,   828,   744,   854,
     858,   934,   755,   191,   571,   237,   237,   246,   195,  1004,
     756,  -566,   798,   -76,  -464,   445,   599,  1006,   437,   196,
     972,     2,   446,     3,  -253,   207,   850,   185,   411,   244,
    1019,   -76,   277,   278,     4,     5,     6,     7,   245,   459,
     560,   563,    14,   229,   -76,   192,   189,     4,     5,     6,
       7,   558,   263,   932,   264,   728,   373,   208,  1007,   932,
     295,   913,   330,   564,   842,   197,   261,     4,     5,     6,
       7,   440,  1005,   578,  -162,   421,   187,   317,   504,   700,
     597,   493,   746,   747,   -76,   728,    23,   994,   576,   -76,
     378,   745,   908,   -76,   422,   244,   244,   193,   812,   814,
     199,   423,   586,   817,   245,   245,   505,   354,   565,   514,
     431,   289,   372,   277,   278,   291,    32,   854,   909,   394,
     728,   298,    36,   581,   184,   728,   227,   353,    37,   293,
     -95,   293,   506,    41,    22,   507,   728,   290,   -95,   848,
     395,   194,   886,   310,   881,   198,   279,   315,   237,   318,
     290,   414,   437,   813,   913,  -441,   352,   229,   331,   332,
     334,   336,   340,   350,   351,   279,   542,   279,   776,  -566,
     512,   -76,   548,   366,   600,   382,   949,   290,   495,   199,
     370,   521,    77,   237,   525,   229,    43,    44,   296,   298,
     261,   361,   261,   574,   203,   261,   579,   261,    77,   459,
     460,   950,   482,   365,   711,   279,   388,   388,   583,   401,
     402,   484,   485,   279,   486,   589,   498,   409,   956,   767,
     279,   412,   279,   279,  -143,  -143,   279,   279,   279,   279,
     279,    66,  -162,   -76,   928,   388,   279,   480,   201,   729,
     866,   279,   867,   730,   868,   374,    65,   463,   464,  1009,
     -76,   -76,   -76,   -76,   -76,   -76,   415,   883,   279,    65,
    -120,   558,     2,   265,     3,  1010,   266,   450,  1001,   733,
     633,   634,   479,   734,   187,     4,     5,     6,     7,    65,
     297,   -75,   437,    67,    68,   352,   459,   762,   608,   609,
     908,   388,   855,   373,   -75,    69,    70,    71,    72,    73,
      74,     2,   440,     3,   736,   856,   494,   590,   737,   753,
     441,   229,   205,   754,   925,   502,   909,   739,   740,   290,
     810,   321,    19,   206,   811,   598,   322,   255,   459,   788,
     388,   210,   503,   244,   -75,   323,   324,   688,   951,   -75,
     595,   388,   781,   -75,   212,   256,   952,   242,   467,   468,
      26,   469,   618,   619,   315,   392,     2,  1012,     3,   403,
     567,   243,   606,  -401,  -401,  -401,  -401,  -401,  -401,     4,
       5,     6,     7,   671,   672,   416,    33,   219,     4,     5,
       6,     7,   315,   832,   257,   790,   791,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   220,   856,
     471,   468,   229,   469,   221,   802,   591,   592,   593,   804,
     594,   237,   806,   596,   388,   636,    19,   341,   799,   800,
     229,   320,   259,   509,   510,   504,   511,   864,   222,   819,
     445,   841,   800,   237,   770,    51,    52,    53,    54,    55,
      56,   821,   777,   637,    26,   706,   321,   549,   388,   388,
     713,   322,   223,   505,   707,   708,   468,   709,   469,   822,
     323,   324,   224,   459,   788,   325,   225,   834,   638,   857,
      33,   694,   513,   510,   823,   511,   550,   843,   844,   506,
     705,   639,   507,   -75,   836,   233,   876,    65,   341,   878,
     869,   870,   871,   872,   873,   605,   468,   234,   469,   695,
     -75,   -75,   -75,   -75,   -75,   -75,   846,   847,   640,   551,
     863,   800,   342,   343,   344,   704,   235,   946,   748,   919,
     236,   551,   888,   346,   260,   824,   693,   261,   552,    51,
      52,    53,    54,    55,    56,   893,   894,   229,   388,   267,
     552,   274,   275,   276,   957,   229,   958,   959,   960,   782,
     280,   499,   320,   427,   428,   429,   268,    10,   269,   723,
     641,   642,    11,   912,   643,   644,   270,   645,   646,   271,
     647,   890,   272,    15,   404,   405,   406,   321,   947,  -258,
     920,    65,   322,   342,   343,   344,   273,  -353,   281,   880,
      65,   323,   324,   283,   346,   320,   325,   320,   905,   800,
     926,   927,   648,   649,   650,   651,   652,   653,   654,   285,
     655,   656,   961,   962,   982,   800,   284,    24,    25,   286,
     321,   298,   321,   775,   304,   322,   309,   322,  1013,   800,
      79,   181,  -353,   431,   323,   324,   323,   324,   778,   325,
      30,   325,   307,    31,   229,   240,   241,   279,   372,   407,
     408,   545,   547,   356,   360,   796,   367,   368,   326,   380,
     385,   419,   417,   418,   420,   424,   425,    45,    46,   444,
     853,   953,   997,   461,   449,   455,   462,   353,   473,   475,
     478,   491,   189,   395,    48,   518,    49,   523,   562,   543,
     544,   546,   561,   568,   571,    37,   431,   445,   580,   582,
     584,   837,   588,   587,  -570,  -571,  -569,   601,   607,   494,
    1017,   610,   611,   614,   615,   617,   622,   623,     1,   237,
     440,     2,   625,     3,   626,   627,   628,   229,   630,   624,
     629,   631,   635,   632,     4,     5,     6,     7,     8,     9,
     674,   681,   676,   677,   683,    10,   685,   686,  -441,   712,
      11,   715,   714,   717,    12,   719,   721,   720,    13,   722,
      14,    15,    16,   724,   728,   741,   758,   763,   768,   771,
     772,   779,    17,   780,   783,   789,   801,   803,   808,   815,
      18,    19,    20,   805,   829,   800,   807,   818,   853,   820,
     827,   830,   838,   849,    21,   865,    22,   875,   879,   877,
    -426,   885,   884,   891,    23,    24,    25,   897,   907,    26,
     918,    27,   924,   919,   929,   930,   920,   892,   921,   940,
     938,   941,   954,    28,   948,    29,   450,   964,    30,   942,
     394,    31,   979,   980,    32,    33,   973,    34,   937,    35,
      36,   981,    37,   983,   986,   229,   978,    38,   998,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   990,   993,
    1014,   725,  1021,   825,   904,   689,   572,   945,   911,   680,
     575,   673,    48,   524,    49,  1020,   682,    50,   833,   935,
     188,   383,   497,   248,   384,   250,   577,   966,   500,   970,
     602,   603,   906,   992,    51,    52,    53,    54,    55,    56,
     456,   860,   501,   971,   977,   541,   889,   282,   393,   410,
     371,   702,    57,    58,    59,    60,   294,   490,   710,    61,
      62,    63,   489,   359,   936,   769,   989,     0,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,     0,     0,    66,
      67,    68,     0,     0,     0,     0,     0,    69,    70,    71,
      72,    73,    74,    75,    76,     1,    77,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,    11,     0,     0,
       0,    12,     0,     0,     0,    13,     0,    14,    15,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      28,     0,    29,     0,     0,    30,     0,     0,    31,     0,
       0,    32,    33,     0,    34,     0,    35,    36,     0,    37,
       0,     0,     0,     0,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,    49,     0,     0,    50,     0,  1018,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,    52,    53,    54,    55,    56,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    57,
      58,    59,    60,     0,     0,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,    66,    67,    68,     0,
       0,     0,     0,     0,    69,    70,    71,    72,    73,    74,
      75,    76,     1,    77,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,    12,     0,
       0,     0,    13,     0,    14,    15,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    28,     0,    29,
       0,     0,    30,     0,     0,    31,     0,     0,    32,    33,
       0,    34,     0,    35,    36,     0,    37,     0,     0,     0,
       0,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,    49,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    57,    58,    59,    60,
       0,     0,     0,    61,    62,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,     0,     0,    66,    67,    68,     0,     0,     0,     0,
       0,    69,    70,    71,    72,    73,    74,    75,    76,   186,
      77,     0,     2,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,    11,     0,     0,     0,    12,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,     0,     0,    30,
       0,     0,    31,     0,     0,    32,    33,     0,    34,     0,
       0,    36,     0,    37,     0,     0,     0,     0,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,    49,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,    52,    53,    54,    55,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    57,    58,    59,    60,     0,     0,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,     0,     0,
      66,    67,    68,     0,     0,     0,     0,     0,    69,    70,
      71,    72,    73,    74,    75,    76,   186,    77,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,   187,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,   -54,
       0,     0,    12,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,    34,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,    52,    53,    54,    55,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,    58,    59,    60,     0,     0,     0,    61,    62,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,    66,    67,    68,
       0,     0,     0,     0,     0,    69,    70,    71,    72,    73,
      74,    75,    76,   186,    77,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,    11,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,    31,     0,     0,    32,
      33,     0,    34,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,    49,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
      52,    53,    54,    55,    56,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    57,    58,    59,
      60,     0,     0,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,     0,     0,    66,    67,    68,     0,     0,     0,
       0,     0,    69,    70,    71,    72,    73,    74,    75,    76,
     226,    77,     0,     2,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
     187,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,    32,    33,     0,    34,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    38,
       0,    39,    40,    41,    42,    43,    44,    45,    46,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,    49,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    57,    58,    59,    60,     0,     0,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
       0,    66,    67,    68,     0,     0,     0,     0,     0,    69,
      70,    71,    72,    73,    74,    75,    76,   226,    77,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,   187,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,    12,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,    49,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,    52,    53,    54,    55,    56,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    57,    58,    59,    60,     0,     0,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,     0,     0,    66,    67,
      68,     0,     0,     0,     0,     0,    69,    70,    71,    72,
      73,    74,    75,    76,   226,    77,     0,     2,     0,     3,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     0,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,     0,   341,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,   342,   343,
     344,     0,     0,     0,   345,     0,     0,     0,    48,   346,
      49,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,    52,    53,    54,    55,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    58,
      59,    60,     0,     0,     0,    61,    62,    63,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,   226,   391,     0,     2,     0,     3,
       0,     0,    65,     0,     0,    66,    67,    68,     0,     0,
       4,     5,     6,     7,     0,     9,     0,     0,   207,    75,
      76,    10,    77,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     208,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
      49,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,    52,    53,    54,    55,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    58,
      59,    60,     0,     0,     0,    61,    62,    63,     0,     0,
       0,     0,   226,     0,     0,     2,     0,     3,     0,     0,
       0,    64,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,    65,     9,     0,    66,    67,    68,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,    12,    75,
      76,     0,    77,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
       0,     0,    30,     0,     0,    31,     0,     0,    32,    33,
       0,     0,     0,     0,    36,     0,    37,   226,     0,     0,
       2,    38,     3,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,     4,     5,     6,     7,     0,     9,     0,
       0,     0,     0,     0,    10,     0,    48,     0,    49,    11,
       0,    50,     0,    12,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,    57,    58,    59,    60,
       0,     0,     0,    61,    62,    63,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,    64,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,     0,     0,    66,    67,    68,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,     0,    75,    76,    36,
      77,     0,   226,     0,     0,     2,    38,     3,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,     4,     5,
       6,     7,     0,     9,     0,     0,   375,     0,     0,   376,
       0,    48,     0,    49,    11,     0,    50,     0,    12,   227,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,    51,    52,    53,    54,    55,    56,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,    57,    58,    59,    60,     0,     0,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    64,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,     0,     0,    66,    67,
      68,     0,   377,     0,     0,    31,     0,     0,    32,    33,
       0,     0,    75,    76,    36,    77,     0,     0,     0,     0,
       0,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,    49,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    57,    58,    59,    60,
       0,     0,     0,    61,    62,    63,     0,     0,     0,     0,
     226,     0,     0,     2,     0,     3,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
      65,     9,     0,    66,    67,    68,     0,    10,     0,     0,
       0,     0,    11,     0,     0,     0,    12,    75,    76,     0,
      77,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   691,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,    32,    33,     0,     0,
       0,     0,    36,     0,     0,   226,     0,     0,     2,    38,
       3,    39,    40,    41,    42,    43,    44,    45,    46,    47,
       0,     4,     5,     6,     7,     0,     9,     0,     0,     0,
       0,     0,    10,     0,    48,     0,    49,    11,     0,    50,
       0,    12,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    51,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    57,    58,    59,    60,     0,     0,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,    64,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
       0,    66,    67,    68,     0,    30,     0,     0,    31,     0,
       0,    32,    33,     0,     0,    75,    76,    36,    77,     0,
     226,     0,     0,     2,    38,     3,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,     4,     5,     6,     7,
       0,     9,     0,     0,     0,     0,     0,    10,     0,    48,
       0,    49,    11,     0,    50,     0,    12,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    51,    52,    53,    54,    55,    56,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,    57,
      58,    59,    60,     0,     0,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,    64,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,    66,    67,    68,     0,
      30,     0,     0,    31,     0,     0,    32,    33,     0,     0,
      75,    76,    36,    77,     0,   226,     0,     0,     2,    38,
       3,    39,    40,    41,    42,    43,    44,    45,    46,    47,
       0,     4,     5,     6,     7,     0,     9,     0,     0,     0,
       0,     0,    10,     0,    48,     0,    49,    11,     0,    50,
       0,    12,     0,     0,     0,     0,     0,     0,    15,     0,
       0,     0,     0,     0,     0,     0,    51,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    57,    58,    59,    60,     0,     0,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,     0,     0,    26,    64,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
       0,    66,    67,    68,     0,    30,     0,     0,    31,     0,
       0,     0,    33,     0,     0,    75,    76,     0,   311,     0,
     226,     0,     0,     2,    38,     3,    39,    40,     0,    42,
       0,     0,    45,    46,    47,     0,     4,     5,     6,     7,
       0,     9,     0,     0,     0,     0,     0,    10,     0,    48,
       0,    49,    11,     0,    50,     0,    12,     0,     0,     0,
       0,     0,     0,    15,     0,     0,     0,     0,     0,     0,
       0,    51,    52,    53,    54,    55,    56,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,    57,
      58,    59,    60,     0,     0,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,     0,    24,    25,     0,
       0,    26,    64,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,     0,    33,     0,     0,
      75,    76,     0,    77,     0,     0,     0,     0,     0,    38,
       0,    39,    40,     0,    42,     0,     0,    45,    46,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,    49,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    57,    58,    59,    60,     0,     0,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-915)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    50,   319,   234,    67,    68,   223,    91,    22,     1,
     326,   326,   315,   196,   586,   234,    91,    91,   105,    91,
     203,   198,     1,   284,    18,    24,     4,   353,     4,     4,
     213,     4,   674,     4,    27,    69,   423,     4,    63,     3,
      34,    24,   617,   647,   686,     5,     4,     4,   230,   308,
       4,     4,     4,     4,     4,   270,     5,    72,    50,    36,
       4,   163,   638,    76,   640,   117,    59,   759,    40,   789,
     794,   117,   648,     3,     5,    67,    68,    91,   117,   993,
     656,     5,   724,     5,     4,    24,     5,   175,   307,   128,
     942,     6,    31,     8,     3,    27,   788,    24,   281,    76,
    1014,    23,   248,   249,    19,    20,    21,    22,    85,   425,
     425,    57,    45,   105,    36,   131,    25,    19,    20,    21,
      22,   424,    20,   175,    22,     3,    36,    59,   216,   175,
     193,   855,   219,    79,   776,   174,   238,    19,    20,    21,
      22,    72,   994,   446,     5,   297,    23,   210,    66,    69,
     476,   368,   645,   646,    76,     3,    89,   222,   443,    81,
     247,   133,    32,    85,     6,    76,    76,     3,   744,   745,
       3,    13,   457,   749,    85,    85,    94,   359,   124,   394,
     129,   110,   245,   248,   249,   186,   119,   907,    58,     3,
       3,    24,   125,   452,   186,     3,   165,   166,   127,   191,
      72,   193,   120,   136,    81,   123,     3,   186,    80,   784,
      24,     3,    94,   205,   818,    72,   250,   209,   210,   213,
     199,   220,   441,   187,   948,   250,   227,   219,   220,   221,
     222,   223,   224,   225,   226,   250,   419,   250,     3,   163,
     392,   163,   629,   235,   163,   259,    49,   226,    63,     3,
     244,   403,   251,   245,    24,   247,   138,   139,   251,    24,
     238,   492,   238,   440,    80,   238,   449,   238,   251,   585,
     585,    74,   356,   492,   600,   250,   268,   269,   455,   271,
     272,   356,   356,   250,   356,   462,   373,   279,   930,   676,
     250,   283,   250,   250,   238,   239,   250,   250,   250,   250,
     250,   234,   163,   225,   880,   297,   250,   356,    24,   187,
     803,   250,   805,   191,   807,   225,   231,     4,     5,   207,
     242,   243,   244,   245,   246,   247,   587,   820,   250,   231,
      24,   634,     6,   231,     8,   223,   234,    31,   980,   187,
       4,     5,   356,   191,    23,    19,    20,    21,    22,   231,
     194,    23,   571,   235,   236,   356,   672,   672,   510,   511,
      32,   353,    36,    36,    36,   242,   243,   244,   245,   246,
     247,     6,    72,     8,   187,    49,   368,   464,   191,   187,
      80,   373,     3,   191,   877,   379,    58,   641,   642,   368,
     187,    73,    66,   131,   191,   482,    78,    64,   714,   714,
     392,     3,   381,    76,    76,    87,    88,   584,   211,    81,
     473,   403,    85,    85,   224,    82,   219,     0,     4,     5,
      94,     7,   201,   202,   416,   269,     6,   999,     8,   273,
     431,     7,   495,   248,   249,   250,   251,   252,   253,    19,
      20,    21,    22,     4,     5,   289,   120,     3,    19,    20,
      21,    22,   444,    24,   121,   215,   216,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,     3,    49,
       4,     5,   464,     7,     3,   729,   468,   469,   470,   733,
     472,   473,   736,   475,   476,    40,    66,    52,     4,     5,
     482,    48,    81,     4,     5,    66,     7,   800,     3,   753,
      24,     4,     5,   495,   681,   179,   180,   181,   182,   183,
     184,    23,   695,    68,    94,   599,    73,    41,   510,   511,
     607,    78,     3,    94,   599,   599,     5,   599,     7,    41,
      87,    88,     3,   849,   849,    92,     3,   768,    93,   213,
     120,    98,     4,     5,    56,     7,    70,    61,    62,   120,
     599,   106,   123,   225,   771,     3,   810,   231,    52,   813,
      14,    15,    16,    17,    18,     4,     5,     3,     7,   126,
     242,   243,   244,   245,   246,   247,   207,   208,   133,   103,
       4,     5,   147,   148,   149,   599,     3,   212,   153,   214,
       3,   103,   823,   158,   239,   107,   588,   238,   122,   179,
     180,   181,   182,   183,   184,   177,   178,   599,   600,     3,
     122,   251,   252,   253,   931,   607,   933,   934,   935,   706,
     248,    24,    48,   167,   168,   169,     3,    30,     3,   630,
     185,   186,    35,   213,   189,   190,   196,   192,   193,     3,
     195,   824,     3,    46,   274,   275,   276,    73,   212,   196,
     214,   231,    78,   147,   148,   149,     3,    83,    42,   153,
     231,    87,    88,     3,   158,    48,    92,    48,     4,     5,
       4,     5,   227,   228,   229,   230,   231,   232,   233,     4,
     235,   236,   175,   176,     4,     5,    13,    90,    91,     4,
      73,    24,    73,   694,    24,    78,    94,    78,     4,     5,
       0,     1,    83,   129,    87,    88,    87,    88,   700,    92,
     113,    92,    80,   116,   706,    75,    76,   250,   781,   277,
     278,   422,   423,   165,     4,   717,     4,     3,   111,    27,
       4,   126,    31,     5,     4,     3,     3,   140,   141,     5,
     789,   924,   973,   101,   126,    83,    83,   166,    72,    72,
     164,     4,    25,    24,   157,     5,   159,     3,    97,     4,
      24,    24,    31,   226,     5,   127,   129,    24,    24,    24,
      83,   772,   242,    13,     5,     5,     5,   164,   239,   771,
    1011,     4,   217,     4,    21,     4,     4,     4,     3,   781,
      72,     6,    25,     8,     4,     4,     4,   789,     3,    13,
      13,    84,     3,    84,    19,    20,    21,    22,    23,    24,
      97,     5,    98,    24,    74,    30,     4,     3,   250,     4,
      35,   100,    38,     5,    39,   203,    24,   203,    43,    24,
      45,    46,    47,     3,     3,     3,    40,   130,   242,    38,
      75,     4,    57,     4,    72,    38,    21,   188,    21,     3,
      65,    66,    67,   188,    41,     5,   188,   118,   907,   188,
     111,   171,   218,     5,    79,     4,    81,     4,    21,   188,
     118,     3,    94,    24,    89,    90,    91,    69,     5,    94,
     175,    96,   107,   214,   885,     3,   214,    99,   209,     5,
     165,    31,    21,   108,   239,   110,    31,     3,   113,   221,
       3,   116,   210,     3,   119,   120,   222,   122,   170,   124,
     125,     4,   127,    24,    24,   907,   175,   132,    67,   134,
     135,   136,   137,   138,   139,   140,   141,   142,    24,   173,
       7,   634,   224,   757,   839,   585,   438,   907,   854,   571,
     441,   561,   157,   416,   159,  1015,   575,   162,   768,   894,
       8,   260,   372,    91,   261,    91,   444,   172,   374,   940,
     492,   492,   849,   964,   179,   180,   181,   182,   183,   184,
     325,   794,   374,   941,   948,   418,   823,   161,   269,   280,
     245,   595,   197,   198,   199,   200,   193,   359,   599,   204,
     205,   206,   357,   232,   895,   678,   959,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,    -1,   234,
     235,   236,    -1,    -1,    -1,    -1,    -1,   242,   243,   244,
     245,   246,   247,   248,   249,     3,   251,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    39,    -1,    -1,    -1,    43,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,   110,    -1,    -1,   113,    -1,    -1,   116,    -1,
      -1,   119,   120,    -1,   122,    -1,   124,   125,    -1,   127,
      -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,   159,    -1,    -1,   162,    -1,   164,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
     198,   199,   200,    -1,    -1,    -1,   204,   205,   206,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   220,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   231,    -1,    -1,   234,   235,   236,    -1,
      -1,    -1,    -1,    -1,   242,   243,   244,   245,   246,   247,
     248,   249,     3,   251,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    43,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,   110,
      -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,   120,
      -1,   122,    -1,   124,   125,    -1,   127,    -1,    -1,    -1,
      -1,   132,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,   159,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,   180,
     181,   182,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,   198,   199,   200,
      -1,    -1,    -1,   204,   205,   206,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   220,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     231,    -1,    -1,   234,   235,   236,    -1,    -1,    -1,    -1,
      -1,   242,   243,   244,   245,   246,   247,   248,   249,     3,
     251,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,   113,
      -1,    -1,   116,    -1,    -1,   119,   120,    -1,   122,    -1,
      -1,   125,    -1,   127,    -1,    -1,    -1,    -1,   132,    -1,
     134,   135,   136,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,   159,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   179,   180,   181,   182,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,   198,   199,   200,    -1,    -1,    -1,
     204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,    -1,
     234,   235,   236,    -1,    -1,    -1,    -1,    -1,   242,   243,
     244,   245,   246,   247,   248,   249,     3,   251,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,   122,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   179,   180,   181,   182,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     197,   198,   199,   200,    -1,    -1,    -1,   204,   205,   206,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   220,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   231,    -1,    -1,   234,   235,   236,
      -1,    -1,    -1,    -1,    -1,   242,   243,   244,   245,   246,
     247,   248,   249,     3,   251,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,
     120,    -1,   122,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,   159,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,
     180,   181,   182,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,   198,   199,
     200,    -1,    -1,    -1,   204,   205,   206,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     220,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   231,    -1,    -1,   234,   235,   236,    -1,    -1,    -1,
      -1,    -1,   242,   243,   244,   245,   246,   247,   248,   249,
       3,   251,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,
      -1,    94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,   119,   120,    -1,   122,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,   132,
      -1,   134,   135,   136,   137,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,   159,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   197,   198,   199,   200,    -1,    -1,
      -1,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,
      -1,   234,   235,   236,    -1,    -1,    -1,    -1,    -1,   242,
     243,   244,   245,   246,   247,   248,   249,     3,   251,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,   159,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   179,   180,   181,   182,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   197,   198,   199,   200,    -1,    -1,    -1,   204,   205,
     206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   220,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   231,    -1,    -1,   234,   235,
     236,    -1,    -1,    -1,    -1,    -1,   242,   243,   244,   245,
     246,   247,   248,   249,     3,   251,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,    -1,   157,   158,
     159,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     179,   180,   181,   182,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,   198,
     199,   200,    -1,    -1,    -1,   204,   205,   206,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   220,    -1,    -1,     3,     4,    -1,     6,    -1,     8,
      -1,    -1,   231,    -1,    -1,   234,   235,   236,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    27,   248,
     249,    30,   251,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     159,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     179,   180,   181,   182,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,   198,
     199,   200,    -1,    -1,    -1,   204,   205,   206,    -1,    -1,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,   220,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,   231,    24,    -1,   234,   235,   236,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    39,   248,
     249,    -1,   251,    -1,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
      -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,    -1,   127,     3,    -1,    -1,
       6,   132,     8,   134,   135,   136,   137,   138,   139,   140,
     141,   142,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,   157,    -1,   159,    35,
      -1,   162,    -1,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,   180,
     181,   182,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,   197,   198,   199,   200,
      -1,    -1,    -1,   204,   205,   206,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,   220,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     231,    -1,    -1,   234,   235,   236,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,    -1,   248,   249,   125,
     251,    -1,     3,    -1,    -1,     6,   132,     8,   134,   135,
     136,   137,   138,   139,   140,   141,   142,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    27,    -1,    -1,    30,
      -1,   157,    -1,   159,    35,    -1,   162,    -1,    39,   165,
      -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   179,   180,   181,   182,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,   197,   198,   199,   200,    -1,    -1,    -1,   204,   205,
     206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    -1,    -1,    94,   220,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   231,    -1,    -1,   234,   235,
     236,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,   120,
      -1,    -1,   248,   249,   125,   251,    -1,    -1,    -1,    -1,
      -1,   132,    -1,   134,   135,   136,   137,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,   159,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   179,   180,
     181,   182,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,   198,   199,   200,
      -1,    -1,    -1,   204,   205,   206,    -1,    -1,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,   220,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
     231,    24,    -1,   234,   235,   236,    -1,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    39,   248,   249,    -1,
     251,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,
      -1,    94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,    -1,    -1,     3,    -1,    -1,     6,   132,
       8,   134,   135,   136,   137,   138,   139,   140,   141,   142,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,   157,    -1,   159,    35,    -1,   162,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,   197,   198,   199,   200,    -1,    -1,
      -1,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    -1,    -1,    94,   220,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,
      -1,   234,   235,   236,    -1,   113,    -1,    -1,   116,    -1,
      -1,   119,   120,    -1,    -1,   248,   249,   125,   251,    -1,
       3,    -1,    -1,     6,   132,     8,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,   157,
      -1,   159,    35,    -1,   162,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,   197,
     198,   199,   200,    -1,    -1,    -1,   204,   205,   206,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,
      -1,    94,   220,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   231,    -1,    -1,   234,   235,   236,    -1,
     113,    -1,    -1,   116,    -1,    -1,   119,   120,    -1,    -1,
     248,   249,   125,   251,    -1,     3,    -1,    -1,     6,   132,
       8,   134,   135,   136,   137,   138,   139,   140,   141,   142,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,   157,    -1,   159,    35,    -1,   162,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,   197,   198,   199,   200,    -1,    -1,
      -1,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    -1,    -1,    94,   220,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,
      -1,   234,   235,   236,    -1,   113,    -1,    -1,   116,    -1,
      -1,    -1,   120,    -1,    -1,   248,   249,    -1,   251,    -1,
       3,    -1,    -1,     6,   132,     8,   134,   135,    -1,   137,
      -1,    -1,   140,   141,   142,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,   157,
      -1,   159,    35,    -1,   162,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,   197,
     198,   199,   200,    -1,    -1,    -1,   204,   205,   206,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    94,   220,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   231,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,    -1,   120,    -1,    -1,
     248,   249,    -1,   251,    -1,    -1,    -1,    -1,    -1,   132,
      -1,   134,   135,    -1,   137,    -1,    -1,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,   159,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   197,   198,   199,   200,    -1,    -1,
      -1,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   231,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   251
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    35,    39,    43,    45,    46,    47,    57,    65,    66,
      67,    79,    81,    89,    90,    91,    94,    96,   108,   110,
     113,   116,   119,   120,   122,   124,   125,   127,   132,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   157,   159,
     162,   179,   180,   181,   182,   183,   184,   197,   198,   199,
     200,   204,   205,   206,   220,   231,   234,   235,   236,   242,
     243,   244,   245,   246,   247,   248,   249,   251,   258,   259,
     260,   261,   272,   280,   281,   282,   284,   285,   286,   287,
     288,   290,   291,   292,   293,   298,   302,   328,   329,   331,
     332,   333,   334,   335,   337,   338,   340,   341,   342,   343,
     345,   346,   348,   349,   351,   352,   355,   356,   357,   361,
     363,   364,   365,   366,   367,   368,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   387,   388,   393,
     394,   396,   428,   429,   441,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   467,   468,   469,
     470,   471,   473,   474,   475,   476,   480,   482,   483,   484,
     485,   486,   493,   494,   495,   520,   521,   522,   523,   535,
     537,   259,   282,   334,   467,    24,     3,    23,   331,    25,
     492,     3,   131,     3,     3,   117,   128,   174,    72,     3,
     357,    24,   534,    80,   280,     3,   131,    27,    59,   294,
       3,   357,   224,   322,   323,   434,   435,   436,   438,     3,
       3,     3,     3,     3,     3,     3,     3,   165,   291,   467,
     528,   529,   533,     3,     3,     3,     3,   467,   468,   468,
     450,   450,     0,     7,    76,    85,   280,   338,   339,   341,
     342,   345,   348,   350,   353,    64,    82,   121,   283,    81,
     239,   238,   290,    20,    22,   231,   234,     3,     3,     3,
     196,     3,     3,     3,   251,   252,   253,   248,   249,   250,
     248,    42,   472,     3,    13,     4,     4,     4,     4,   110,
     282,   334,   442,   467,   442,   468,   251,   294,    24,   488,
     489,   490,   491,   488,    24,   557,   323,    80,   488,    94,
     467,   251,   303,   358,   359,   467,   487,   468,   357,   488,
      48,    73,    78,    87,    88,    92,   111,   430,   433,   289,
     290,   467,   467,   464,   467,   464,   467,   477,   478,   479,
     467,    52,   147,   148,   149,   153,   158,   369,   370,   459,
     467,   467,   334,   166,   529,   530,   165,   524,   525,   528,
       4,   361,   382,   383,   384,   535,   467,     4,     3,   347,
     357,   448,   468,    36,   225,    27,    30,   113,   290,   354,
      27,   440,   280,   332,   333,     4,   465,   466,   467,   531,
     532,     4,   294,   466,     3,    24,   399,   400,   401,   407,
     389,   467,   467,   294,   451,   451,   451,   452,   452,   467,
     473,   488,   467,    24,   220,   494,   294,    31,     5,   126,
       4,   465,     6,    13,     3,     3,   275,   167,   168,   169,
     539,   129,   301,   324,   299,   300,   360,   535,   275,     4,
      72,    80,   316,   317,     5,    24,    31,   362,   519,   126,
      31,   319,   536,   319,   321,    83,   433,   295,   296,   493,
     495,   101,    83,     4,     5,     4,     4,     4,     5,     7,
      76,     4,    72,    72,     4,    72,   163,   531,   164,   280,
     291,   336,   338,   339,   341,   345,   348,   526,   527,   530,
     525,     4,     5,   464,   467,    63,   344,   344,   290,    24,
     374,   428,   357,   282,    66,    94,   120,   123,   327,     4,
       5,     7,   465,     4,   399,   408,   411,   412,     5,   385,
       4,   465,     4,     3,   303,    24,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   443,   444,   445,
     446,   446,   488,     4,    24,   490,    24,   490,   491,    41,
      70,   103,   122,   262,   263,   264,   267,   269,   519,   271,
     495,    31,    97,    57,    79,   124,   540,   334,   226,   558,
     559,     5,   288,   318,   323,   299,   301,   359,   519,   488,
      24,   275,    24,   323,    83,     5,   301,    13,   242,   323,
     290,   467,   467,   467,   467,   468,   467,   531,   290,     5,
     163,   164,   382,   383,   397,     4,   468,   239,   465,   465,
       4,   217,   409,   413,     4,    21,   390,     4,   201,   202,
     386,   392,     4,     4,    13,    25,     4,     4,     4,    13,
       3,    84,    84,     4,     5,     3,    40,    68,    93,   106,
     133,   185,   186,   189,   190,   192,   193,   195,   227,   228,
     229,   230,   231,   232,   233,   235,   236,   496,   499,   500,
     507,   508,   509,   510,   511,   512,   514,   515,   516,   517,
     518,     4,     5,   302,    97,   541,    98,    24,   560,   561,
     300,     5,   316,    74,   325,     4,     3,   274,   323,   296,
     558,    56,   297,   467,    98,   126,   431,   432,   439,     4,
      69,   481,   481,     4,   280,   291,   338,   341,   345,   348,
     527,   531,     4,   290,    38,   100,   276,     5,   386,   203,
     203,    24,    24,   334,     3,   263,   270,   519,     3,   187,
     191,   501,   502,   187,   191,   501,   187,   191,   501,   502,
     502,     3,   503,   504,    40,   133,   503,   503,   153,   459,
     460,   462,   463,   187,   191,   501,   501,   265,    40,   497,
     472,   498,   495,   130,   273,   270,   542,   491,   242,   561,
     323,    38,    75,   326,   270,   334,     3,   488,   467,     4,
       4,    85,   290,    72,   395,   398,   414,   415,   495,    38,
     215,   216,   410,   417,   418,   391,   467,     4,   270,     4,
       5,    21,   502,   188,   502,   188,   502,   188,    21,   505,
     187,   191,   501,   187,   501,     3,   513,   501,   118,   502,
     188,    23,    41,    56,   107,   267,   268,   111,   498,    41,
     171,   538,    24,   327,   361,   562,   464,   334,   218,   402,
     403,     4,   270,    61,    62,   437,   207,   208,   386,     5,
     498,   277,   278,   291,   335,    36,    49,   213,   372,   419,
     420,   421,   422,     4,   519,     4,   503,   503,   503,    14,
      15,    16,    17,    18,   506,     4,   502,   188,   502,    21,
     153,   459,   461,   503,    94,     3,    94,   266,   361,   455,
     488,    24,    99,   177,   178,   550,   551,    69,   543,   544,
     399,   404,   405,   406,   276,     4,   415,     5,    32,    58,
     279,   279,   213,   372,   420,   423,   425,   426,   175,   214,
     214,   209,   416,   427,   107,   503,     4,     5,   501,   334,
       3,   117,   175,   320,   117,   320,   551,   170,   165,   546,
       5,    31,   221,   313,   315,   278,   212,   212,   239,    49,
      74,   211,   219,   488,    21,     4,   270,   319,   319,   319,
     319,   175,   176,   545,     3,   330,   172,   259,   547,   549,
     405,   407,   453,   222,   304,   305,   424,   425,   175,   210,
       3,     4,     4,    24,   553,   554,    24,   556,   552,   554,
      24,   555,   334,   173,   222,   314,   311,   361,    67,   309,
     310,   270,     4,   548,   549,   453,   175,   216,   308,   207,
     223,   307,   558,     4,     7,   306,   312,   361,   164,   549,
     308,   224
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

  case 63:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
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

  case 68:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 69:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
            }
    break;

  case 70:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));}
    break;

  case 71:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));}
    break;

  case 72:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 73:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 74:

    {    
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 77:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 78:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode)); }
    break;

  case 79:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 82:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 83:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 84:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 87:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
            }
    break;

  case 88:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 89:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 91:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 93:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 95:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 97:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 99:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 100:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 106:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 108:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 111:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 113:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 114:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 115:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 116:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(9) - (10)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(10) - (10)].pParseNode));
        }
    break;

  case 117:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 118:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 119:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 120:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 122:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 124:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 128:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 129:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 131:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 132:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 133:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 134:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 135:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 142:

    { // boolean_primary: rule 2
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 143:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
    }
    break;

  case 144:

    { // boolean_primary: rule 2
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 146:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 148:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 150:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 152:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 162:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 163:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 164:

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

  case 171:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 172:

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

  case 173:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 174:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 175:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 176:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 177:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 178:

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

  case 179:

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

  case 180:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 181:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 182:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 183:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 184:

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

  case 185:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 186:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 187:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 188:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 189:

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

  case 190:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 191:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 192:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 193:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 197:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 198:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 199:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 200:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 201:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 209:

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

  case 210:

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

  case 211:

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

  case 212:

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

  case 213:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 214:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 216:

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

  case 217:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 221:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 222:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 223:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 224:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 225:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 226:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 227:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 228:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 229:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 232:

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

  case 246:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 247:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 248:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 249:

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

  case 250:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 256:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 257:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 263:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 268:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 269:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 270:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 271:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 273:

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

  case 281:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 284:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 286:

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

  case 289:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 290:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 295:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 297:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 298:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 299:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 300:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 302:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 303:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 305:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 307:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 311:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 312:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 313:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 314:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 315:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 317:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 322:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 324:

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
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 330:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 332:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 333:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 334:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 335:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 336:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 337:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 338:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 339:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 347:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 348:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 349:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 350:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 353:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 354:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 356:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 357:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 358:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 359:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 361:

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

  case 362:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 367:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 368:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 384:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 385:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 386:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 389:

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

  case 397:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 399:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 400:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 404:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 405:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 408:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 409:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 411:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 412:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 413:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
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

  case 426:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 428:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 433:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 434:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 435:

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

  case 437:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 438:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 439:

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

  case 446:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 447:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 450:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 452:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 455:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 456:

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

  case 457:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 458:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 459:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 462:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 463:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 464:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 465:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 466:

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

  case 467:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 470:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 471:

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

  case 472:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 477:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 478:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 479:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 480:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 481:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 482:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 483:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 484:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 485:

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

  case 486:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 487:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 488:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 490:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 491:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 492:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 494:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 495:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 501:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 502:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 503:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 504:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 505:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 507:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 509:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 510:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 512:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 515:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("K", SQL_NODE_PUNCTUATION));
        }
    break;

  case 516:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("M", SQL_NODE_PUNCTUATION));
        }
    break;

  case 517:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("G", SQL_NODE_PUNCTUATION));
        }
    break;

  case 518:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("T", SQL_NODE_PUNCTUATION));
        }
    break;

  case 519:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("P", SQL_NODE_PUNCTUATION));
        }
    break;

  case 520:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 521:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 522:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 523:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 524:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 525:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 526:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 527:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 528:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 530:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 541:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 542:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 543:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 553:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            /*$$->append($3);*/
        }
    break;

  case 554:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 559:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 560:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 561:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 562:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 563:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 564:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 565:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 572:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 573:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 574:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 575:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 576:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 580:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 581:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 582:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 583:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 584:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 585:

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

  case 587:

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

  case 588:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 589:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 592:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
    }
    break;

  case 597:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 599:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 600:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
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

  case 607:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(";", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 608:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 609:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 611:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 612:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 613:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 614:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 615:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 616:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 623:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 625:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 626:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
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
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 629:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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
using namespace ::dbtools;

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
    Utf8String aMatchStr;
    if (pTokenNode->isToken())
    {
        sal_Char cEscape = 0;
        if (pEscapeNode->count())
            cEscape = Utf8StringHelper::toChar (pEscapeNode->getChild(1)->getTokenValue());

        // Platzhalter austauschen
        aMatchStr = pTokenNode->getTokenValue();
        const size_t nLen = aMatchStr.size();
        Utf8String sSearch,sReplace;
        if ( bInternational )
        {
            sSearch.append("%_");
            sReplace.append("*?");
        }
        else
        {
            sSearch.append("*?");
            sReplace.append("%_");
        }
        
        bool wasEscape = false;
        for (size_t i = 0; i < nLen; i++)
        {
                const sal_Char c = aMatchStr[i];
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
            if (c == sSearch[0])
                match=0;
            else if (c == sSearch[1])
                match=1;

            if (match != -1)
            {
                aMatchStr[i] = sReplace[(size_t)match];
            }
        }
    }
    return aMatchStr;
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
    m_pParseTree = pNewParseTree;
}
//-----------------------------------------------------------------------------

/** Delete all comments in a query.

    See also getComment()/concatComment() implementation for
    OQueryController::translateStatement().
 */
static Utf8String delComment(Utf8String const& rQuery)
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
    Utf8String aBuf;
    aBuf.reserve(nQueryLen);
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
            aBuf.append(&pCopy[i], 1);
    }
    return aBuf;
}

OSQLParseNode* OSQLParser::parseTree(Utf8String& rErrorMessage,
                                     Utf8String const& rStatement,
                                     sal_Bool bInternational)
{


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
Utf8CP OSQLParser::RuleIDToStr(sal_uInt32 nRuleID)
{
    OSL_ENSURE(nRuleID < (sizeof yytname/sizeof yytname[0]), "OSQLParser::RuleIDToStr: Invalid nRuleId!");
    return yytname[nRuleID];
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
    Utf8String aValue(pLiteral->getChild(0)->getTokenValue());
    if (bAppendBlank)
    {
        aValue.append(" ");
    }
    
    aValue.append(pLiteral->getChild(1)->getTokenValue());

    pLiteral = new OSQLInternalNode(aValue,SQL_NODE_STRING);
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
