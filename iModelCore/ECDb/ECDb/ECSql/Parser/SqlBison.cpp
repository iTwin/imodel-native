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
     SQL_TOKEN_REVERSE = 301,
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
     SQL_TOKEN_USAGE = 365,
     SQL_TOKEN_USING = 366,
     SQL_TOKEN_VALUES = 367,
     SQL_TOKEN_VIEW = 368,
     SQL_TOKEN_WHERE = 369,
     SQL_TOKEN_WITH = 370,
     SQL_TOKEN_WORK = 371,
     SQL_TOKEN_BIT_LENGTH = 372,
     SQL_TOKEN_CHAR = 373,
     SQL_TOKEN_CHAR_LENGTH = 374,
     SQL_TOKEN_POSITION = 375,
     SQL_TOKEN_SUBSTRING = 376,
     SQL_TOKEN_SQL_TOKEN_INTNUM = 377,
     SQL_TOKEN_CURRENT_DATE = 378,
     SQL_TOKEN_CURRENT_TIMESTAMP = 379,
     SQL_TOKEN_CURDATE = 380,
     SQL_TOKEN_NOW = 381,
     SQL_TOKEN_EXTRACT = 382,
     SQL_TOKEN_DAYNAME = 383,
     SQL_TOKEN_DAYOFMONTH = 384,
     SQL_TOKEN_DAYOFWEEK = 385,
     SQL_TOKEN_DAYOFYEAR = 386,
     SQL_TOKEN_HOUR = 387,
     SQL_TOKEN_MINUTE = 388,
     SQL_TOKEN_MONTH = 389,
     SQL_TOKEN_MONTHNAME = 390,
     SQL_TOKEN_QUARTER = 391,
     SQL_TOKEN_DATEDIFF = 392,
     SQL_TOKEN_SECOND = 393,
     SQL_TOKEN_TIMESTAMPADD = 394,
     SQL_TOKEN_TIMESTAMPDIFF = 395,
     SQL_TOKEN_TIMEVALUE = 396,
     SQL_TOKEN_WEEK = 397,
     SQL_TOKEN_YEAR = 398,
     SQL_TOKEN_EVERY = 399,
     SQL_TOKEN_WITHIN = 400,
     SQL_TOKEN_ARRAY_AGG = 401,
     SQL_TOKEN_CASE = 402,
     SQL_TOKEN_THEN = 403,
     SQL_TOKEN_END = 404,
     SQL_TOKEN_NULLIF = 405,
     SQL_TOKEN_COALESCE = 406,
     SQL_TOKEN_WHEN = 407,
     SQL_TOKEN_ELSE = 408,
     SQL_TOKEN_BEFORE = 409,
     SQL_TOKEN_AFTER = 410,
     SQL_TOKEN_INSTEAD = 411,
     SQL_TOKEN_EACH = 412,
     SQL_TOKEN_REFERENCING = 413,
     SQL_TOKEN_BEGIN = 414,
     SQL_TOKEN_ATOMIC = 415,
     SQL_TOKEN_TRIGGER = 416,
     SQL_TOKEN_ROW = 417,
     SQL_TOKEN_STATEMENT = 418,
     SQL_TOKEN_NEW = 419,
     SQL_TOKEN_OLD = 420,
     SQL_TOKEN_VALUE = 421,
     SQL_TOKEN_CURRENT_CATALOG = 422,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 423,
     SQL_TOKEN_CURRENT_PATH = 424,
     SQL_TOKEN_CURRENT_ROLE = 425,
     SQL_TOKEN_CURRENT_SCHEMA = 426,
     SQL_TOKEN_VARCHAR = 427,
     SQL_TOKEN_VARBINARY = 428,
     SQL_TOKEN_VARYING = 429,
     SQL_TOKEN_OBJECT = 430,
     SQL_TOKEN_NCLOB = 431,
     SQL_TOKEN_NATIONAL = 432,
     SQL_TOKEN_LARGE = 433,
     SQL_TOKEN_CLOB = 434,
     SQL_TOKEN_BLOB = 435,
     SQL_TOKEN_BIGI = 436,
     SQL_TOKEN_INTERVAL = 437,
     SQL_TOKEN_OVER = 438,
     SQL_TOKEN_ROW_NUMBER = 439,
     SQL_TOKEN_NTILE = 440,
     SQL_TOKEN_LEAD = 441,
     SQL_TOKEN_LAG = 442,
     SQL_TOKEN_RESPECT = 443,
     SQL_TOKEN_IGNORE = 444,
     SQL_TOKEN_NULLS = 445,
     SQL_TOKEN_FIRST_VALUE = 446,
     SQL_TOKEN_LAST_VALUE = 447,
     SQL_TOKEN_NTH_VALUE = 448,
     SQL_TOKEN_FIRST = 449,
     SQL_TOKEN_LAST = 450,
     SQL_TOKEN_EXCLUDE = 451,
     SQL_TOKEN_OTHERS = 452,
     SQL_TOKEN_TIES = 453,
     SQL_TOKEN_FOLLOWING = 454,
     SQL_TOKEN_UNBOUNDED = 455,
     SQL_TOKEN_PRECEDING = 456,
     SQL_TOKEN_RANGE = 457,
     SQL_TOKEN_ROWS = 458,
     SQL_TOKEN_PARTITION = 459,
     SQL_TOKEN_WINDOW = 460,
     SQL_TOKEN_NO = 461,
     SQL_TOKEN_GETECCLASSID = 462,
     SQL_TOKEN_LIMIT = 463,
     SQL_TOKEN_OFFSET = 464,
     SQL_TOKEN_NEXT = 465,
     SQL_TOKEN_ONLY = 466,
     SQL_TOKEN_BINARY = 467,
     SQL_TOKEN_BOOLEAN = 468,
     SQL_TOKEN_DOUBLE = 469,
     SQL_TOKEN_INTEGER = 470,
     SQL_TOKEN_INT = 471,
     SQL_TOKEN_INT32 = 472,
     SQL_TOKEN_LONG = 473,
     SQL_TOKEN_INT64 = 474,
     SQL_TOKEN_STRING = 475,
     SQL_TOKEN_DATE = 476,
     SQL_TOKEN_TIMESTAMP = 477,
     SQL_TOKEN_DATETIME = 478,
     SQL_TOKEN_POINT2D = 479,
     SQL_TOKEN_POINT3D = 480,
     SQL_TOKEN_OR = 481,
     SQL_TOKEN_AND = 482,
     SQL_EQUAL = 483,
     SQL_GREAT = 484,
     SQL_LESS = 485,
     SQL_NOTEQUAL = 486,
     SQL_GREATEQ = 487,
     SQL_LESSEQ = 488,
     SQL_CONCAT = 489,
     SQL_TOKEN_INVALIDSYMBOL = 490
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
#define YYFINAL  251
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4974

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  257
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  303
/* YYNRULES -- Number of rules.  */
#define YYNRULES  628
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1027

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   490

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     4,   253,   250,     5,   251,    13,   254,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   255,     2,     8,     2,     2,     2,     2,     2,     2,
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
     241,   242,   243,   244,   245,   246,   247,   248,   249,   252,
     256
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
    1267,  1268,  1270,  1272,  1274,  1276,  1277,  1280,  1288,  1293,
    1295,  1297,  1302,  1309,  1316,  1323,  1326,  1328,  1330,  1332,
    1336,  1340,  1344,  1346,  1347,  1349,  1351,  1355,  1358,  1360,
    1362,  1364,  1365,  1369,  1370,  1372,  1376,  1379,  1381,  1383,
    1385,  1387,  1389,  1392,  1395,  1399,  1403,  1406,  1408,  1409,
    1411,  1415,  1416,  1418,  1422,  1425,  1426,  1428,  1430,  1432,
    1434,  1436,  1441,  1446,  1449,  1453,  1457,  1460,  1465,  1470,
    1474,  1476,  1482,  1487,  1490,  1493,  1497,  1500,  1502,  1507,
    1510,  1512,  1514,  1515,  1519,  1525,  1527,  1529,  1531,  1533,
    1535,  1537,  1539,  1541,  1543,  1545,  1548,  1551,  1553,  1555,
    1557,  1562,  1567,  1572,  1574,  1576,  1582,  1587,  1589,  1592,
    1597,  1599,  1603,  1605,  1607,  1609,  1611,  1613,  1615,  1617,
    1620,  1625,  1626,  1629,  1631,  1633,  1635,  1637,  1640,  1642,
    1643,  1646,  1648,  1652,  1662,  1663,  1666,  1668,  1670,  1673,
    1675,  1677,  1680,  1681,  1684,  1686,  1690,  1691,  1695,  1697,
    1699,  1700,  1703,  1705,  1711,  1713,  1717,  1719,  1721,  1724,
    1729,  1734,  1739,  1744,  1746,  1748,  1750,  1752,  1754
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     258,     0,    -1,   259,    -1,   259,     7,    -1,   281,    -1,
     260,    -1,   261,    -1,   272,    -1,   539,    -1,    47,   117,
     489,     3,   262,     4,    -1,   263,    -1,   262,     5,   263,
      -1,   264,    -1,   269,    -1,   520,   497,   265,    -1,    -1,
     265,   268,    -1,   456,    -1,   122,    -1,   103,    84,    -1,
      23,    94,    -1,   267,    -1,    56,   358,    -1,    56,    94,
      -1,    56,   266,    -1,    41,    -1,    41,     3,   333,     4,
      -1,   107,   489,    -1,   107,   489,     3,   270,     4,    -1,
     267,     3,   270,     4,    -1,    70,    84,     3,   270,     4,
     107,   489,    -1,    70,    84,     3,   270,     4,   107,   489,
       3,   270,     4,    -1,    41,     3,   333,     4,    -1,   270,
       5,   520,    -1,   520,    -1,   271,     5,   496,    -1,   496,
      -1,    47,   129,   489,   275,    31,   302,   273,    -1,    -1,
     131,    41,    99,    -1,    -1,     3,   270,     4,    -1,    -1,
       3,   271,     4,    -1,    -1,   100,    38,   277,    -1,   278,
      -1,   277,     5,   278,    -1,   334,   279,    -1,   290,   279,
      -1,    -1,    32,    -1,    58,    -1,    -1,    23,    -1,   283,
      -1,   284,    -1,   285,    -1,   286,    -1,   291,    -1,   292,
      -1,   297,    -1,   282,    -1,   302,    -1,   282,   121,   442,
     302,    -1,    43,   132,    -1,    57,    72,   323,   300,    -1,
      67,   536,    80,   298,    -1,    79,    80,   489,   275,   287,
      -1,   128,     3,   288,     4,    -1,   289,    -1,   288,     5,
     289,    -1,   290,    -1,   468,    -1,   108,   132,    -1,   110,
     293,   303,    80,   298,   316,    -1,    -1,    27,    -1,    59,
      -1,   295,    -1,   294,     5,   295,    -1,   496,   244,   296,
      -1,   468,    -1,    56,    -1,   124,   323,   111,   294,   300,
      -1,   299,    -1,   298,     5,   299,    -1,   357,    -1,    -1,
     324,    -1,   439,    -1,   110,   293,   303,   316,    -1,   253,
      -1,   355,    -1,    -1,   305,    -1,   225,   311,   308,    -1,
      -1,   312,    -1,   210,    -1,   226,    -1,   178,    -1,   219,
      -1,    -1,   310,    -1,    67,   307,   306,   308,   227,    -1,
     358,    -1,   358,    -1,    -1,   315,    -1,    -1,   225,   454,
      -1,   224,   454,   314,    -1,   317,   300,   325,   326,   399,
     276,   313,   304,   309,    -1,    72,   318,    -1,   323,    -1,
     318,     5,   323,    -1,    -1,    31,    -1,    -1,   178,    -1,
      -1,   319,    24,   274,    -1,    -1,   227,    -1,   322,   489,
     321,    -1,   322,   354,   538,   275,    -1,   435,    -1,   130,
     333,    -1,    -1,    74,    38,   271,    -1,    -1,    75,   333,
      -1,   120,    -1,    66,    -1,   123,    -1,    94,    -1,   334,
      -1,     3,   333,     4,    -1,   290,    -1,     3,   333,     4,
      -1,   328,    -1,   328,    81,   280,   327,    -1,   330,    -1,
      23,   330,    -1,   331,    -1,   332,   243,   331,    -1,   332,
      -1,   333,   242,   332,    -1,   336,    -1,   339,    -1,   350,
      -1,   352,    -1,   353,    -1,   345,    -1,   348,    -1,   342,
      -1,   337,   289,    -1,   289,   337,   289,    -1,   337,   289,
      -1,   246,    -1,   247,    -1,   244,    -1,   245,    -1,   249,
      -1,   248,    -1,    81,   280,    -1,   280,    36,   289,   243,
     289,    -1,   289,   338,    -1,   280,    85,   469,   343,    -1,
     280,    85,   449,   343,    -1,   289,   340,    -1,   289,   341,
      -1,   340,    -1,   341,    -1,    -1,    63,   469,    -1,    81,
     280,    94,    -1,   289,   344,    -1,   344,    -1,   354,    -1,
       3,   465,     4,    -1,   280,    76,   346,    -1,   289,   347,
      -1,   347,    -1,   337,   351,   354,    -1,   289,   349,    -1,
      30,    -1,    27,    -1,   113,    -1,    65,   354,    -1,   122,
     354,    -1,     3,   443,     4,    -1,   356,    -1,   355,     5,
     356,    -1,   488,    -1,   537,    -1,   232,    -1,    20,    -1,
      21,    -1,    22,    -1,    19,    -1,   358,   236,    -1,   358,
     232,    -1,   358,    20,    -1,   358,    22,    -1,    -1,    31,
     520,    -1,   520,    -1,   136,     3,   468,    76,   468,     4,
      -1,   136,     3,   465,     4,    -1,   360,    -1,   368,    -1,
     365,    -1,   135,     3,   468,     4,    -1,   138,     3,   468,
       4,    -1,    96,     3,   468,     4,    -1,   133,     3,   468,
       4,    -1,   362,    -1,   363,    -1,   364,    -1,   460,    -1,
     154,    -1,   366,    -1,   468,    -1,   143,     3,   367,    72,
     468,     4,    -1,   370,    -1,   358,    -1,   537,    -1,    94,
      -1,    66,    -1,   120,    -1,   182,    -1,   183,    -1,   184,
      -1,   185,    -1,   186,    -1,   187,    -1,   425,    -1,   485,
      -1,   374,     3,     4,    -1,   372,     3,     4,    -1,   374,
       3,   467,     4,    -1,   373,     3,   467,     4,    -1,   375,
      -1,   158,    -1,    24,    -1,   141,    -1,   142,    -1,   377,
     199,   397,    -1,   200,     3,     4,    -1,   425,    -1,   378,
      -1,   384,    -1,   390,    -1,   393,    -1,   201,     3,   381,
       4,    -1,   537,    -1,   358,    -1,   380,    -1,   379,    -1,
      -1,     5,   387,    -1,     5,   387,     5,   388,    -1,    -1,
     389,    -1,   385,     3,   386,   382,     4,   383,    -1,   202,
      -1,   203,    -1,   468,    -1,    21,    -1,   468,    -1,   204,
     206,    -1,   205,   206,    -1,   391,     3,   468,     4,   383,
      -1,   207,    -1,   208,    -1,    -1,   395,    -1,   209,     3,
     468,     5,   394,     4,   392,   383,    -1,   380,    -1,   379,
      -1,    72,   210,    -1,    72,   211,    -1,    24,    -1,   396,
      -1,   398,    -1,   404,    -1,    -1,   400,    -1,   221,   401,
      -1,   401,     5,   402,    -1,   402,    -1,   403,    31,   404,
      -1,   396,    -1,     3,   408,     4,    -1,    -1,   409,    -1,
      -1,   410,    -1,    -1,   414,    -1,   405,   406,   276,   407,
      -1,   396,    -1,   220,    38,   411,    -1,   411,     5,   412,
      -1,   412,    -1,   496,   499,    -1,    -1,   424,    -1,   415,
     416,   413,    -1,   219,    -1,   218,    -1,   417,    -1,   419,
      -1,   216,   217,    -1,   418,    -1,    49,   178,    -1,   369,
     217,    -1,    36,   420,   243,   421,    -1,   422,    -1,   422,
      -1,   417,    -1,   216,   215,    -1,   423,    -1,   369,   215,
      -1,   212,    49,   178,    -1,   212,    74,    -1,   212,   214,
      -1,   212,   222,   213,    -1,   426,     3,   293,   466,     4,
      -1,    46,     3,   253,     4,    -1,    46,     3,   293,   466,
       4,    -1,    35,    -1,    90,    -1,    91,    -1,   116,    -1,
     160,    -1,    30,    -1,   113,    -1,    87,    -1,    88,    -1,
      73,    -1,    98,   333,    -1,   428,    -1,   436,    -1,    -1,
      78,    -1,   427,    -1,   427,   101,    -1,   323,    48,    83,
     323,    -1,   323,    92,   430,    83,   323,    -1,   323,   430,
      83,   323,   429,    -1,   431,    -1,   323,   430,    83,   323,
     127,   489,   434,    -1,    -1,    61,    -1,    62,    -1,   433,
      -1,   432,    -1,   127,     3,   270,     4,    -1,   302,    -1,
     287,    -1,   437,    -1,     3,   441,     4,    -1,   438,    -1,
     301,    82,   442,   440,    -1,   438,    -1,   439,    -1,   443,
     121,   442,   301,    -1,   443,    64,   442,   301,    -1,    -1,
      27,    -1,   441,    -1,   354,    -1,   468,    -1,   447,    -1,
     228,    -1,   229,    -1,   230,    -1,   231,    -1,   232,    -1,
     233,    -1,   234,    -1,   235,    -1,   236,    -1,   239,    -1,
     237,    -1,   238,    -1,   240,    -1,   241,    -1,    39,     3,
     445,    31,   446,     4,    -1,   369,    -1,   371,    -1,   450,
      -1,   496,    -1,   444,    -1,   521,    -1,   376,    -1,     3,
     468,     4,    -1,   448,    -1,   223,     3,     4,    -1,   494,
      13,   223,     3,     4,    -1,   449,    -1,   361,    -1,   451,
      -1,   251,   451,    -1,   250,   451,    -1,   452,    -1,   453,
     253,   452,    -1,   453,   254,   452,    -1,   453,    -1,   454,
     250,   453,    -1,   454,   251,   453,    -1,   456,    -1,   139,
      -1,   140,    -1,   237,   469,    -1,   238,   469,    -1,   455,
      -1,   457,    -1,   458,    -1,   159,    -1,   150,    -1,    52,
      -1,   148,    -1,   149,    -1,   460,   502,    -1,   460,    -1,
     154,   502,    -1,   460,   502,    -1,   154,   514,    -1,   461,
     118,   462,    -1,   463,    -1,   468,    -1,   465,     5,   468,
      -1,   465,     7,   468,    -1,   533,    -1,   466,    -1,   467,
       5,   466,    -1,   467,     7,   466,    -1,   454,    -1,   469,
      -1,   459,    -1,   470,    -1,   474,    -1,   471,    -1,   470,
     250,   474,    -1,   468,   252,   468,    -1,   236,    -1,   475,
      -1,    42,   489,    -1,   472,    -1,   472,   473,    -1,   481,
      -1,   476,    -1,   477,    -1,   137,     3,   478,    72,   469,
     482,     4,    -1,   479,    -1,   480,    -1,    -1,   483,    -1,
     485,    -1,   486,    -1,   487,    -1,    -1,    69,   468,    -1,
     137,     3,   468,    72,   468,   482,     4,    -1,   137,     3,
     465,     4,    -1,   125,    -1,    89,    -1,   484,     3,   468,
       4,    -1,    45,     3,   469,   127,   489,     4,    -1,    45,
       3,   445,     5,   446,     4,    -1,   119,     3,   469,   127,
     489,     4,    -1,   468,   359,    -1,   492,    -1,   491,    -1,
     490,    -1,    24,    13,   491,    -1,    24,     6,   491,    -1,
      24,    13,   492,    -1,    24,    -1,    -1,    25,    -1,   495,
      -1,   494,    13,   495,    -1,    24,   493,    -1,   253,    -1,
     494,    -1,   500,    -1,    -1,    40,   111,    24,    -1,    -1,
     473,    -1,   501,   498,   499,    -1,   509,   499,    -1,   511,
      -1,   513,    -1,   517,    -1,   518,    -1,   519,    -1,    40,
     502,    -1,   134,   502,    -1,    40,   190,   503,    -1,   134,
     190,   503,    -1,   188,   503,    -1,   508,    -1,    -1,   503,
      -1,     3,    21,     4,    -1,    -1,   505,    -1,     3,   506,
       4,    -1,    21,   507,    -1,    -1,    14,    -1,    15,    -1,
      16,    -1,    17,    -1,    18,    -1,    40,   194,   191,   504,
      -1,   134,   194,   191,   504,    -1,   195,   504,    -1,   193,
      40,   502,    -1,   193,   134,   502,    -1,    93,   502,    -1,
     193,    40,   190,   503,    -1,   193,   134,   190,   503,    -1,
      93,   190,   503,    -1,   510,    -1,   193,    40,   194,   191,
     504,    -1,    93,   194,   191,   504,    -1,   192,   504,    -1,
     228,   502,    -1,   228,   190,   503,    -1,   189,   503,    -1,
     512,    -1,   228,   194,   191,   504,    -1,   196,   504,    -1,
     515,    -1,   516,    -1,    -1,     3,    21,     4,    -1,     3,
      21,     5,    21,     4,    -1,   231,    -1,   232,    -1,   234,
      -1,   233,    -1,   235,    -1,    68,    -1,   106,    -1,   230,
      -1,   229,    -1,   237,    -1,   238,   502,    -1,   198,   464,
      -1,    24,    -1,   522,    -1,   523,    -1,   166,     3,   465,
       4,    -1,   167,     3,   468,     4,    -1,   167,     3,   465,
       4,    -1,   524,    -1,   525,    -1,   163,   535,   526,   532,
     165,    -1,   163,   530,   532,   165,    -1,   527,    -1,   530,
     527,    -1,   168,   528,   164,   533,    -1,   529,    -1,   528,
       5,   529,    -1,   290,    -1,   335,    -1,   338,    -1,   347,
      -1,   340,    -1,   344,    -1,   531,    -1,   530,   531,    -1,
     168,   333,   164,   533,    -1,    -1,   169,   533,    -1,   534,
      -1,   468,    -1,   290,    -1,    24,    -1,     6,    24,    -1,
       8,    -1,    -1,   319,    24,    -1,   333,    -1,     3,   259,
       4,    -1,    47,   177,   559,   541,   542,    98,   492,   540,
     545,    -1,    -1,   174,   552,    -1,   170,    -1,   171,    -1,
     172,    97,    -1,    79,    -1,    57,    -1,   124,   543,    -1,
      -1,    97,   544,    -1,   270,    -1,   546,   548,   549,    -1,
      -1,    69,   173,   547,    -1,   178,    -1,   179,    -1,    -1,
     168,   329,    -1,   551,    -1,   175,   176,   550,     7,   165,
      -1,   551,    -1,   550,     7,   551,    -1,   259,    -1,   553,
      -1,   552,   553,    -1,   181,   320,   319,   557,    -1,   180,
     320,   319,   558,    -1,   181,   117,   319,   554,    -1,   180,
     117,   319,   555,    -1,   556,    -1,   556,    -1,    24,    -1,
      24,    -1,    24,    -1,    24,    -1
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
    3043,  3056,  3057,  3058,  3063,  3070,  3071,  3079,  3090,  3100,
    3101,  3104,  3114,  3124,  3136,  3149,  3158,  3163,  3168,  3175,
    3182,  3191,  3201,  3209,  3210,  3218,  3223,  3241,  3247,  3255,
    3325,  3328,  3329,  3338,  3339,  3342,  3349,  3355,  3356,  3357,
    3358,  3359,  3362,  3368,  3374,  3381,  3388,  3394,  3397,  3398,
    3401,  3410,  3411,  3414,  3424,  3432,  3433,  3438,  3443,  3448,
    3453,  3460,  3468,  3476,  3484,  3491,  3498,  3504,  3512,  3520,
    3527,  3530,  3539,  3547,  3555,  3561,  3568,  3574,  3577,  3585,
    3593,  3594,  3597,  3598,  3605,  3639,  3640,  3641,  3642,  3643,
    3660,  3661,  3662,  3673,  3676,  3685,  3714,  3725,  3747,  3748,
    3751,  3759,  3767,  3777,  3778,  3781,  3792,  3802,  3807,  3814,
    3824,  3827,  3832,  3833,  3834,  3835,  3836,  3837,  3840,  3845,
    3852,  3862,  3863,  3871,  3875,  3878,  3881,  3894,  3900,  3924,
    3927,  3937,  3951,  3954,  3969,  3972,  3980,  3981,  3982,  3990,
    3991,  3992,  4000,  4003,  4011,  4014,  4023,  4026,  4035,  4036,
    4039,  4042,  4050,  4051,  4062,  4067,  4074,  4078,  4083,  4091,
    4099,  4107,  4115,  4125,  4128,  4131,  4134,  4137,  4140
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
  "SQL_TOKEN_DROP", "SQL_TOKEN_FORWARD", "SQL_TOKEN_REVERSE",
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
  "SQL_TOKEN_UPPER", "SQL_TOKEN_USAGE", "SQL_TOKEN_USING",
  "SQL_TOKEN_VALUES", "SQL_TOKEN_VIEW", "SQL_TOKEN_WHERE",
  "SQL_TOKEN_WITH", "SQL_TOKEN_WORK", "SQL_TOKEN_BIT_LENGTH",
  "SQL_TOKEN_CHAR", "SQL_TOKEN_CHAR_LENGTH", "SQL_TOKEN_POSITION",
  "SQL_TOKEN_SUBSTRING", "SQL_TOKEN_SQL_TOKEN_INTNUM",
  "SQL_TOKEN_CURRENT_DATE", "SQL_TOKEN_CURRENT_TIMESTAMP",
  "SQL_TOKEN_CURDATE", "SQL_TOKEN_NOW", "SQL_TOKEN_EXTRACT",
  "SQL_TOKEN_DAYNAME", "SQL_TOKEN_DAYOFMONTH", "SQL_TOKEN_DAYOFWEEK",
  "SQL_TOKEN_DAYOFYEAR", "SQL_TOKEN_HOUR", "SQL_TOKEN_MINUTE",
  "SQL_TOKEN_MONTH", "SQL_TOKEN_MONTHNAME", "SQL_TOKEN_QUARTER",
  "SQL_TOKEN_DATEDIFF", "SQL_TOKEN_SECOND", "SQL_TOKEN_TIMESTAMPADD",
  "SQL_TOKEN_TIMESTAMPDIFF", "SQL_TOKEN_TIMEVALUE", "SQL_TOKEN_WEEK",
  "SQL_TOKEN_YEAR", "SQL_TOKEN_EVERY", "SQL_TOKEN_WITHIN",
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
  "upper_lower", "fold", "form_conversion", "char_translation",
  "derived_column", "table_node", "catalog_name", "schema_name",
  "table_name", "opt_column_array_idx", "property_path",
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
      43,    45,   489,    42,    47,    61,   490
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
     281,   281,   281,   282,   282,   283,   284,   285,   286,   287,
     288,   288,   289,   290,   291,   292,   293,   293,   293,   294,
     294,   295,   296,   296,   297,   298,   298,   299,   300,   300,
     301,   302,   303,   303,   304,   304,   305,   306,   306,   307,
     307,   308,   308,   309,   309,   310,   311,   312,   313,   313,
     314,   314,   315,   316,   317,   318,   318,   319,   319,   320,
     320,   321,   321,   322,   322,   323,   323,   323,   324,   325,
     325,   326,   326,   327,   327,   327,   327,   328,   328,   328,
     329,   330,   330,   331,   331,   332,   332,   333,   333,   334,
     334,   334,   334,   334,   334,   334,   334,   335,   336,   336,
     337,   337,   337,   337,   337,   337,   337,   338,   339,   340,
     341,   342,   342,   342,   342,   343,   343,   344,   345,   345,
     346,   346,   347,   348,   348,   349,   350,   351,   351,   351,
     352,   353,   354,   355,   355,   356,   357,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   359,   359,   359,   360,
     360,   361,   361,   361,   362,   362,   363,   364,   365,   365,
     365,   366,   366,   367,   367,   368,   369,   369,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   371,   371,
     371,   371,   371,   371,   372,   373,   374,   375,   375,   376,
     377,   377,   377,   377,   377,   377,   378,   379,   380,   381,
     381,   382,   382,   382,   383,   383,   384,   385,   385,   386,
     387,   388,   389,   389,   390,   391,   391,   392,   392,   393,
     394,   394,   395,   395,   396,   397,   397,   398,   399,   399,
     400,   401,   401,   402,   403,   404,   405,   405,   406,   406,
     407,   407,   408,   409,   410,   411,   411,   412,   413,   413,
     414,   415,   415,   416,   416,   417,   417,   417,   418,   419,
     420,   421,   422,   422,   422,   423,   424,   424,   424,   424,
     425,   425,   425,   426,   426,   426,   426,   426,   426,   426,
     427,   427,   427,   428,   429,   429,   430,   430,   430,   430,
     431,   432,   432,   432,   433,   434,   434,   434,   435,   435,
     436,   437,   437,   438,   438,   439,   439,   440,   441,   441,
     441,   442,   442,   443,   444,   445,   446,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   447,   447,   447,   447,
     447,   448,   449,   449,   449,   449,   449,   449,   449,   449,
     449,   450,   450,   451,   451,   452,   452,   452,   453,   453,
     453,   454,   454,   454,   455,   456,   456,   456,   456,   457,
     458,   459,   460,   460,   460,   460,   460,   461,   462,   462,
     463,   463,   464,   464,   465,   465,   465,   466,   467,   467,
     467,   468,   468,   468,   469,   470,   470,   471,   471,   472,
     472,   473,   474,   474,   475,   475,   476,   477,   478,   479,
     480,   481,   481,   481,   481,   482,   482,   483,   483,   484,
     484,   485,   486,   486,   487,   488,   489,   489,   489,   490,
     490,   491,   492,   493,   493,   494,   494,   495,   495,   496,
     497,   498,   498,   499,   499,   500,   500,   500,   500,   500,
     500,   500,   501,   501,   501,   501,   501,   501,   502,   502,
     503,   504,   504,   505,   506,   507,   507,   507,   507,   507,
     507,   508,   508,   508,   509,   509,   509,   509,   509,   509,
     509,   510,   510,   510,   511,   511,   511,   511,   512,   512,
     513,   513,   514,   514,   514,   515,   515,   515,   515,   515,
     516,   516,   516,   517,   518,   518,   519,   520,   521,   521,
     522,   522,   522,   523,   523,   524,   525,   526,   526,   527,
     528,   528,   529,   529,   529,   529,   529,   529,   530,   530,
     531,   532,   532,   533,   534,   535,   536,   537,   537,   538,
     538,   259,   259,   539,   540,   540,   541,   541,   541,   542,
     542,   542,   543,   543,   544,   545,   546,   546,   547,   547,
     548,   548,   549,   549,   550,   550,   551,   552,   552,   553,
     553,   553,   553,   554,   555,   556,   557,   558,   559
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
       0,     1,     1,     1,     1,     0,     2,     7,     4,     1,
       1,     4,     6,     6,     6,     2,     1,     1,     1,     3,
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
      53,    53,     0,   588,   201,   198,   199,   200,    53,   483,
     338,   333,     0,     0,     0,     0,     0,     0,     0,   230,
       0,     0,    53,   470,   334,   335,   229,     0,     0,    76,
     339,   336,     0,   231,     0,   123,   469,     0,     0,     0,
       0,     0,   415,   416,   247,   248,     0,   245,   337,     0,
       0,     0,   232,   233,   234,   235,   236,   237,     0,     0,
     267,   268,   275,   276,     0,     0,   197,   449,     0,     0,
     162,   163,   160,   161,   165,   164,     0,     0,   488,     0,
       2,     5,     6,     7,     0,     4,    62,    55,    56,    57,
      58,    53,   139,    59,    60,    61,    63,   141,   143,   145,
     147,   591,   137,   149,     0,   150,   173,   174,   156,   179,
     154,   184,   155,   151,   152,   153,   374,   227,   211,   404,
     218,   219,   220,   213,   212,   392,   226,   393,     0,     0,
       0,   244,   398,     0,   252,   253,     0,   254,     0,   255,
     238,     0,   396,   400,   403,   394,   405,   408,   411,   441,
     419,   414,   420,   421,   443,    73,   442,   444,   446,   452,
     445,   450,   455,   456,   454,   461,     0,   239,   463,   464,
     489,   485,   395,   397,   558,   559,   563,   564,   228,     8,
      53,     0,     0,   362,     0,    63,     0,   363,   365,   368,
     373,     0,    73,   587,    53,    54,   144,   484,   487,     0,
      65,     0,    76,     0,     0,     0,   123,     0,   190,   586,
       0,     0,   166,     0,    74,    77,    78,     0,     0,   191,
     124,     0,   346,   353,   359,   358,   127,     0,     0,     0,
     460,     0,     0,     0,    53,   585,   581,   578,     0,     0,
       0,     0,     0,     0,     0,     0,   417,   418,   407,   239,
     406,     1,     3,     0,     0,   371,     0,     0,   168,   171,
     172,   178,   183,   186,    53,    53,    53,   159,    72,   204,
     205,   203,   202,     0,     0,     0,     0,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,   453,     0,     0,
     373,     0,   592,   371,   138,   192,   371,   371,   399,    53,
      76,   361,     0,     0,   375,     0,   442,     0,     0,   482,
       0,   478,   477,   476,    42,   628,     0,    88,     0,     0,
      42,   177,     0,   488,     0,    93,   193,   206,   195,   442,
     589,   121,     0,   342,   347,   340,   341,   346,     0,   348,
       0,     0,     0,     0,   434,     0,   434,     0,   458,   459,
       0,   424,   425,   426,   423,   222,   422,   223,     0,   221,
     224,     0,     0,     0,     0,   579,     0,    53,   581,   567,
       0,     0,   434,     0,   434,   250,   258,   260,   259,     0,
     257,     0,   401,     0,   182,   180,   175,   175,   372,     0,
       0,   188,   187,   189,   158,     0,     0,   146,   148,   241,
     438,     0,   584,   437,   583,   240,     0,   296,   284,   285,
     249,   286,   287,   261,   269,     0,     0,   409,   410,   412,
     413,   448,   447,   462,   451,     0,   483,     0,   486,   364,
       0,    70,     0,     0,     0,     0,     0,     0,     0,   331,
       0,     0,     0,     0,     0,     0,   596,   597,     0,     0,
      53,    66,    89,     0,    67,    85,    87,   196,     0,   216,
     123,     0,    91,    88,     0,   557,     0,   475,   208,     0,
     118,     0,    42,     0,   125,   123,     0,    88,    79,   489,
       0,   349,   123,   217,   214,   210,     0,     0,     0,   468,
       0,     0,   215,     0,     0,   582,   566,    72,   573,     0,
     574,   173,   179,   184,     0,   570,     0,   568,   560,   562,
     561,   256,     0,     0,     0,   170,   169,    64,     0,   185,
     134,   136,   133,   135,   142,   243,     0,     0,   242,   303,
     298,     0,   297,     0,     0,   264,     0,   471,     0,    69,
       0,   367,   366,   370,    90,   369,     0,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   387,   388,   386,   389,
     390,     0,   376,     0,     0,   332,     0,   480,   482,   479,
     481,     0,     0,     0,    18,     0,    10,    12,     0,    13,
       0,     0,    36,     0,   598,   600,   599,   602,     0,   128,
       0,    68,   114,   115,     0,   129,   194,   207,     0,   590,
     126,    40,   350,   123,     0,    84,     0,     0,   346,   435,
     436,     0,   465,   442,     0,   580,   159,    53,     0,   565,
     281,   280,     0,   181,   176,     0,   439,   440,     0,    44,
     299,   295,   270,   262,   264,     0,     0,   274,   265,   330,
     402,    71,   391,   473,   472,     0,    53,     0,    19,     9,
       0,     0,   508,   550,   508,   551,   508,     0,     0,   511,
       0,   511,   511,     0,   508,   553,   552,   545,   546,   548,
     547,   549,   554,   508,    15,   490,   491,   507,   493,   530,
     497,   537,   498,   540,   541,   499,   500,   501,    43,     0,
      38,     0,   601,     0,    86,   123,    75,     0,   131,   474,
       0,   122,   351,    80,    83,    81,    82,    53,     0,   344,
     352,   345,   209,     0,     0,     0,   225,     0,   572,     0,
     576,   577,   575,   571,   569,   277,   167,     0,     0,   300,
       0,   266,   272,   273,   482,     0,     0,    11,     0,    34,
       0,     0,     0,   502,   509,     0,     0,   526,     0,     0,
     503,   506,   536,     0,   533,   512,   508,   508,   523,   539,
     542,   508,     0,   433,   556,     0,     0,   534,   555,    14,
       0,   493,   494,   496,    35,     0,    37,   604,   603,   594,
     116,     0,    53,   288,     0,   343,     0,   355,   466,   467,
     457,     0,   157,     0,   264,   278,   304,   306,   493,    53,
     312,   311,   302,   301,     0,   263,   271,    32,     0,    29,
       0,     0,   504,   511,   529,   511,   505,   511,   515,     0,
       0,     0,   524,     0,   525,     0,   431,   430,     0,   535,
     511,     0,    25,     0,     0,    21,    16,     0,   495,     0,
       0,   606,   130,   132,     0,    44,   289,    41,     0,   356,
     357,   354,   282,   283,   279,     0,   307,    45,    46,    50,
      50,     0,     0,     0,     0,   308,   313,   316,   314,     0,
      33,   510,   521,   532,   522,   516,   517,   518,   519,   520,
     514,   513,   527,   511,   528,     0,   508,   428,   432,   538,
      20,    53,    23,    24,    22,    17,    27,   492,    39,   119,
     119,   595,   617,     0,   593,   610,   294,   290,   292,     0,
     108,   360,   305,    53,    51,    52,    49,    48,     0,     0,
     322,     0,   320,   324,   317,   315,   318,     0,   310,   309,
       0,   531,   543,     0,   429,     0,     0,   117,   120,   117,
     117,   117,   618,     0,     0,    53,     0,     0,     0,    94,
     109,    47,   323,   325,     0,     0,   327,   328,     0,    30,
       0,    26,     0,     0,     0,     0,     0,   608,   609,   607,
      53,   611,     0,   616,   605,   612,   291,   293,   110,     0,
     103,    95,   319,   321,   326,   329,     0,   544,    28,   625,
     622,   624,   627,   620,   621,   623,   626,   619,     0,    53,
       0,   112,     0,   106,     0,   113,   104,     0,   140,     0,
     614,   111,   101,   102,    96,    99,   100,    97,    31,    53,
       0,    98,   107,   613,   615,     0,   105
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    79,   973,    81,    82,   575,   576,   577,   769,   893,
     578,   836,   579,   738,   581,    83,   776,   701,   445,   729,
     857,   858,   916,    84,    85,    86,    87,    88,    89,    90,
     183,   430,    91,    92,    93,    94,   217,   477,   478,   705,
      95,   454,   455,   451,   184,   301,   324,   980,   981,  1020,
    1017,  1014,  1005,  1006,  1002,  1021,   949,  1001,   950,   462,
     463,   592,   471,   939,   474,   221,   222,   452,   698,   783,
     524,    97,   971,    98,    99,   100,   101,   102,   498,   103,
     104,   500,   105,   106,   107,   108,   515,   109,   110,   384,
     111,   112,   263,   113,   395,   114,   115,   116,   325,   326,
     456,   117,   467,   118,   119,   120,   121,   122,   123,   357,
     358,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   377,   378,   379,   534,   637,   135,   136,   413,
     633,   805,   638,   137,   138,   794,   139,   622,   795,   906,
     410,   411,   845,   846,   907,   908,   909,   412,   530,   629,
     802,   531,   532,   630,   796,   797,   928,   803,   804,   865,
     920,   867,   868,   921,   982,   922,   923,   929,   140,   141,
     339,   709,   710,   340,   223,   224,   225,   851,   226,   711,
     187,   188,   189,   542,   190,   389,   191,   142,   303,   561,
     562,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   359,   762,   888,   763,   764,   343,   400,
     401,   155,   156,   157,   158,   159,   772,   160,   161,   162,
     163,   347,   348,   349,   164,   714,   165,   166,   167,   168,
     169,   328,   310,   311,   312,   313,   198,   170,   171,   172,
     674,   771,   773,   675,   676,   743,   744,   754,   755,   819,
     880,   677,   678,   679,   680,   681,   682,   826,   683,   684,
     685,   686,   687,   739,   173,   174,   175,   176,   177,   368,
     369,   504,   505,   236,   237,   366,   403,   404,   238,   210,
     178,   472,   179,   841,   449,   588,   692,   778,   904,   905,
     969,   945,   974,  1009,   975,   901,   902,   994,   990,   991,
     997,   993,   316
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -848
static const yytype_int16 yypact[] =
{
    1542,   795,    57,  -848,  -848,  -848,  -848,  -848,  2040,   147,
    -848,  -848,    88,   -26,   114,   149,   248,   272,   230,  -848,
     290,   251,   334,  -848,  -848,  -848,  -848,   366,   235,    76,
    -848,  -848,   370,  -848,   230,   191,  -848,   421,   426,   431,
     438,   460,  -848,  -848,  -848,  -848,   462,  -848,  -848,  3454,
     465,   475,  -848,  -848,  -848,  -848,  -848,  -848,   484,   510,
    -848,  -848,  -848,  -848,   528,   536,  -848,  -848,  4303,  4303,
    -848,  -848,  -848,  -848,  -848,  -848,  4721,  4721,  -848,   452,
     450,  -848,  -848,  -848,   299,  -848,   368,  -848,  -848,  -848,
    -848,   148,   107,  -848,  -848,  -848,  -848,   413,  -848,  -848,
     275,   301,  -848,  -848,  4303,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,    92,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   551,   560,
     563,  -848,  -848,   380,  -848,  -848,   579,  -848,   583,  -848,
     390,   595,  -848,  -848,  -848,  -848,  -848,  -848,   -87,   -23,
    -848,  -848,  -848,  -848,  -848,   350,  -848,   372,  -848,   574,
    -848,  -848,  -848,  -848,  -848,  -848,   633,   599,  -848,  -848,
     635,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
     795,   656,   665,  -848,   604,    23,    39,  -848,  -848,   611,
    -848,   192,    22,  -848,  1791,  -848,  -848,  -848,  -848,  4303,
    -848,  4303,    59,   673,   673,   683,   191,    63,  -848,  -848,
     629,   673,   626,  4303,  -848,  -848,  -848,  4512,  4303,  -848,
    -848,   152,   410,  -848,  -848,  -848,  -848,  4303,  4303,  4303,
    4303,  4303,  3036,  3245,  2289,  -848,   291,  -848,   555,  4303,
    4303,   723,    89,  4303,   725,   350,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,   730,  4303,   710,   118,  3676,  -848,  -848,
    -848,  -848,  -848,  -848,   334,  2289,  2289,  -848,  -848,  -848,
    -848,  -848,  -848,   736,  4303,  3885,   242,  4303,  4303,    76,
     471,   471,   471,   471,  4303,     7,   673,  -848,  4303,    18,
     737,  4303,  -848,   710,  -848,  -848,   710,   710,  -848,  1791,
      76,  -848,    39,   711,   350,   739,   618,   742,  4303,   157,
     744,  -848,  -848,  -848,   745,  -848,   491,   557,    63,   259,
     745,  -848,    26,   318,   327,   747,  -848,    16,  -848,   622,
     402,   402,   667,  -848,  -848,  -848,  -848,   388,    54,   652,
     671,    28,    31,   435,   -20,   515,   -27,   699,  -848,  -848,
      32,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   700,  -848,
     350,  3245,    22,   -65,  4303,  -848,   608,  2289,   605,  -848,
     555,   552,   350,   565,    33,  -848,    92,  -848,  -848,   771,
    -848,    52,  -848,  3245,  -848,  -848,   161,   -12,  -848,   666,
    4303,  -848,  -848,  -848,  -848,   230,   315,  -848,   275,  -848,
    -848,   596,   350,  -848,  -848,  -848,   616,   753,  -848,  -848,
    -848,  -848,  -848,   773,   350,    37,  4303,  -848,  -848,   -87,
     -87,  -848,  -848,  -848,  -848,    42,   754,   777,  -848,  -848,
     468,  -848,    63,    63,    63,  4512,   529,   529,   673,  -848,
     780,   761,   762,   347,    54,   756,  -848,  -848,   692,    81,
    2289,  -848,  -848,    78,   785,  -848,  -848,  -848,   663,  -848,
     191,   259,  -848,   669,  4303,  -848,   768,  -848,  -848,   673,
    -848,   772,   745,   776,  -848,   191,   712,    67,  -848,   784,
     558,  -848,   191,  -848,  -848,  -848,  4303,  4303,  4303,  -848,
    4303,  4303,  -848,  4303,  4303,  -848,  -848,    20,  -848,  4303,
    -848,   799,   800,   801,    72,  -848,   643,  -848,  -848,  -848,
    -848,  -848,    89,   620,  4303,  -848,  -848,  -848,   564,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  4303,  4303,  -848,  -848,
     589,   806,  -848,   790,   809,   300,   816,  -848,   817,  -848,
    4303,  -848,  -848,   604,  -848,   604,   750,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,   819,  -848,   820,   822,  -848,   814,  -848,   814,  -848,
    -848,   825,   748,   749,  -848,   495,  -848,  -848,   826,  -848,
    3164,   541,  -848,   666,  -848,  -848,  -848,   734,   738,   301,
     259,  -848,   830,   507,    74,   763,  -848,  -848,   835,  -848,
    -848,   840,   507,   191,    54,  -848,    54,  4094,   612,   350,
     350,    49,   -38,    80,    50,  -848,    95,  2787,  4303,  -848,
    -848,  -848,   846,  -848,   601,  4303,  -848,  -848,   813,   755,
    -848,  -848,  -848,   849,   300,   650,   651,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,   834,  2289,   856,  -848,  -848,
     347,   768,   128,  -848,   136,  -848,   142,   860,   860,   861,
      27,   861,   861,   273,   155,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,   860,  -848,  -848,   827,  -848,   574,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,    54,
     735,   768,  -848,   834,  -848,   191,  -848,   832,   793,  -848,
     768,  -848,   507,  -848,  -848,  -848,   350,  2289,   356,  -848,
    -848,  -848,  -848,  4303,   867,   868,  -848,   343,  -848,  4303,
    -848,  -848,  -848,  -848,  -848,   805,  -848,    54,   841,   374,
    4303,  -848,  -848,  -848,  -848,    40,   768,  -848,   606,  -848,
     844,   860,   682,  -848,  -848,   860,   689,  -848,   860,   690,
    -848,  -848,  -848,   862,  -848,  -848,   176,    47,  -848,  -848,
     879,   860,   769,  -848,  -848,   860,   697,  -848,  -848,   364,
     779,   574,  -848,  -848,  -848,   852,  -848,   890,  -848,   722,
     507,    54,  2289,   676,   628,   301,   768,   590,   350,  -848,
    -848,  4303,  -848,   454,   300,  -848,   893,  -848,   574,  2538,
    -848,  -848,  -848,  -848,   662,  -848,   350,  -848,   672,  -848,
     768,   895,  -848,   861,  -848,   861,  -848,   861,   535,   896,
     860,   713,  -848,   860,  -848,   880,  -848,   788,   295,  -848,
     861,   808,   904,   556,   673,  -848,  -848,   885,  -848,   811,
     486,   843,   908,   301,   753,   755,  -848,  -848,   684,  -848,
    -848,  -848,  -848,  -848,  -848,    54,  -848,   911,  -848,    93,
     124,   342,   740,   704,   705,   714,  -848,  -848,  -848,   818,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,   861,  -848,   687,   860,  -848,  -848,  -848,
    -848,  2289,  -848,  -848,    92,  -848,   921,  -848,  -848,   -41,
     -37,   486,  -848,   766,  -848,   774,  -848,   924,  -848,   909,
     717,  -848,  -848,  2538,  -848,  -848,  -848,  -848,   231,   267,
    -848,   703,  -848,  -848,  -848,  -848,  -848,   113,  -848,  -848,
     673,  -848,  -848,   922,  -848,    64,   768,   913,  -848,   913,
     913,   913,  -848,   523,   944,  1044,   753,   945,   471,   724,
    -848,  -848,  -848,  -848,   342,   778,  -848,  -848,   741,   947,
     948,  -848,   701,   927,   933,   927,   935,  -848,  -848,  -848,
    2289,  -848,   787,  -848,  -848,  -848,  -848,  -848,   151,   101,
     897,  -848,  -848,  -848,  -848,  -848,   768,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,    69,  1542,
     471,  -848,   -54,    92,  -122,  -848,  -848,   708,  -848,   953,
    -848,   -23,  -848,  -848,  -848,  -848,  -848,   101,  -848,  1293,
     -54,  -848,    92,  -848,  -848,   743,  -848
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -848,  -848,    62,  -848,  -848,  -848,   316,  -848,  -848,  -848,
     196,  -848,  -848,  -667,   186,  -848,  -848,  -848,  -256,   123,
    -848,    56,   112,    -9,  -848,  -848,  -848,  -848,  -848,  -848,
     516,  -848,   -93,   -44,  -848,  -848,  -132,  -848,   369,  -848,
    -848,   514,   386,  -103,   281,     0,   548,  -848,  -848,  -848,
    -848,   -36,  -848,  -848,  -848,  -848,  -848,  -848,  -848,   391,
    -848,  -848,  -322,    86,  -848,  -848,  -172,  -848,  -848,  -848,
    -848,  -848,  -848,   979,   726,   727,     1,  -738,  -848,  -848,
     -81,   898,  -848,   -77,   899,  -848,   607,   -76,  -848,  -848,
     -75,  -848,  -848,  -848,  -848,  -848,  -848,     5,  -848,   524,
    -848,  -236,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -733,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,   480,   488,  -848,  -848,  -575,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -163,
    -848,  -848,  -848,  -848,  -848,    55,  -848,    58,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,   153,  -848,  -848,  -848,  -848,
     202,  -848,  -848,  -848,  -848,    53,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,   674,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,   577,   283,  -848,   -88,   237,   694,  -848,   812,   573,
    -848,  -848,   760,  -848,   642,   445,   449,  -847,  -848,   182,
    -848,  -848,  -848,  -605,  -848,  -848,  -848,  -848,  -165,  -210,
     746,    21,   -61,  -848,  -848,  -848,   857,   732,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,   407,  -848,  -848,   -73,  -848,
    -848,  -848,  -192,  -848,   294,  -414,  -848,  -317,  -272,  -311,
    -848,  -848,  -664,  -848,  -848,  -571,  -177,  -613,  -848,  -848,
    -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -307,  -848,  -848,  -848,  -848,  -848,  -848,
     653,  -848,   405,   786,  -181,   657,  -346,  -848,  -848,  -848,
    -204,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,  -848,
    -848,  -848,  -848,  -848,  -616,  -848,   125,  -848,  -848,    65,
    -848,  -848,  -848
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -578
static const yytype_int16 yytable[] =
{
      96,   185,   186,   249,   249,   235,   376,   246,   247,   473,
     257,   267,   314,   212,   259,   261,   262,   428,   495,   320,
     468,   479,   192,   208,   777,  -572,   298,   480,   570,   331,
     459,   713,   483,   784,   317,   484,   492,   510,   380,   219,
     465,   535,   426,   294,   807,   490,   537,   466,   758,   759,
     740,   514,    14,   712,   716,   365,   488,   512,   761,   731,
     268,   860,    80,   182,   458,   345,   318,   756,   961,   808,
     308,   864,   604,  1008,   371,   373,   937,   617,   426,   590,
     940,   193,   256,   747,  -465,   750,   215,  -361,  1015,   245,
     245,   199,   290,   767,   424,     2,    23,     3,   440,   494,
    -157,   978,   768,   215,  1016,  -361,   200,   838,     4,     5,
       6,     7,   269,   409,   270,   457,   -72,   201,   216,   848,
       4,     5,     6,     7,  1012,   914,    32,   479,   919,   -72,
     -72,   740,    36,   582,   856,   216,   580,   938,   585,   740,
     306,   938,   296,   -72,    40,   740,   460,   416,   615,   713,
    -246,   915,   202,  1011,   390,   207,   914,   329,   740,   597,
     586,   757,   955,   441,   394,  1013,   280,   281,   435,   -72,
     442,   195,   197,   300,   -72,   860,   309,   266,   -72,   740,
     185,   186,   915,   -72,  -139,   822,   824,   956,   -72,   365,
     827,   181,   -72,   387,   253,   302,   295,   450,   431,   297,
     872,   192,   873,   254,   874,   587,   536,   249,   249,   249,
     249,   290,   423,   268,   284,   192,   600,   889,   513,   854,
     304,   919,   304,   887,   514,   284,   330,   282,   283,    22,
     290,   570,   284,   207,   322,   363,   618,   823,   327,   245,
    -442,   427,   182,    67,   529,   407,   564,   268,   341,   342,
     344,   346,   350,   360,   362,   396,   296,   457,   385,  -157,
     372,   374,  -139,  -139,   381,     2,   408,     3,   284,   962,
     931,    78,   724,   290,   284,   245,   376,   598,   284,   779,
     284,   266,   266,   284,   284,   284,   499,   479,   593,   284,
     501,   502,   503,   480,   284,   402,   402,   518,   414,   415,
     302,   284,   284,   602,   284,   421,   266,    78,   380,   425,
     608,   266,   307,   297,   209,   934,   626,   627,   741,  1007,
     192,    66,   742,   497,   271,   351,   745,   957,   272,   402,
     746,   211,   748,    66,   428,   958,   749,   -72,   -72,   -72,
     -72,   -72,   -72,   580,   206,   765,   268,   351,     2,   766,
       3,   -72,   -72,   -72,   -72,   -72,   -72,   195,   256,   786,
     595,     4,     5,     6,     7,   203,   820,   214,   363,   213,
     821,   465,   479,   218,   605,   253,  1000,   204,   774,   390,
     309,   520,   362,  1010,   254,   402,   457,   831,   571,   517,
     -92,   862,    70,    71,    72,    73,    74,    75,   -92,   460,
     519,   282,   283,  1024,   372,   832,   616,   461,    19,   521,
     479,  -403,  -403,  -403,  -403,  -403,   798,   572,   220,   253,
     833,   352,   353,   354,   227,   205,  -117,   760,   791,   228,
     613,   702,   356,   470,   229,   522,    26,   402,   523,   485,
     486,   230,   487,   352,   353,   354,   952,   641,   925,   886,
     573,   589,   251,   624,   356,   268,   327,   252,   332,   234,
     364,   333,    33,   231,   479,   232,   334,   573,   239,   574,
     582,   834,   539,   540,   233,   335,   336,     2,   240,     3,
     751,   752,   953,   333,   926,   327,   574,   241,   334,   255,
       4,     5,     6,     7,   264,     9,   268,   335,   336,   649,
     650,    10,   337,   870,   635,   636,    11,   609,   610,   611,
      12,   612,   245,   242,   614,   402,   787,    15,   265,   489,
     486,   338,   487,   780,    52,    53,    54,    55,    56,    57,
     432,   243,   726,   433,   434,   245,   719,    19,   479,   244,
     720,   721,   722,   266,   798,   688,   689,   402,   402,   875,
     876,   877,   878,   879,   273,   332,   508,   486,   918,   487,
      23,    24,    25,   274,   812,    26,   275,    27,   814,   509,
     486,   816,   487,   718,    66,     4,     5,     6,     7,   276,
     333,   268,   277,   690,    30,   334,   278,    31,   829,  -251,
    -346,    33,   800,   801,   335,   336,    36,   894,   279,   337,
     525,   526,   284,   527,    37,   332,    38,    39,   717,    41,
     809,   810,    44,    45,    46,   963,   286,   964,   965,   966,
     528,   526,   285,   527,   623,   486,   792,   487,   706,    47,
     333,    48,   847,   810,    49,   334,   288,    50,    51,   402,
    -346,  -462,   896,   882,   335,   336,   884,   735,   289,   337,
     892,   849,   850,    52,    53,    54,    55,    56,    57,   291,
     332,   446,   447,   448,   852,   853,   899,   900,     2,   292,
       3,    58,    59,    60,    61,   268,   869,   810,    62,    63,
      64,     4,     5,     6,     7,   333,   293,   450,   911,   810,
     334,   932,   933,   -90,    65,    42,    43,   309,   861,   335,
     336,   967,   968,    66,   337,   988,   810,   315,   785,   319,
     707,   862,  1018,   810,   543,   545,   544,   544,   248,   250,
     321,    76,    77,   367,    78,   417,   418,   375,    19,   382,
     387,   419,   420,   383,   788,   567,   569,   388,   959,   708,
     399,   429,   436,  1003,   437,   438,   439,   443,   444,   469,
     475,   806,   464,   481,   482,   859,    26,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   491,   493,   496,   364,   511,   300,   408,   533,   197,
     538,  1022,    33,   843,   565,   566,   568,   583,    66,   584,
     590,   181,   465,    68,    69,   603,   599,   606,   180,   450,
     601,     2,   607,     3,  -576,  -577,  -575,   625,   619,   628,
     631,   632,   245,   634,     4,     5,     6,     7,     8,     9,
     639,   640,   460,   642,   643,    10,   644,   645,   646,   651,
      11,   691,   647,   648,    12,   695,   693,   697,    13,   699,
      14,    15,    16,   700,    52,    53,    54,    55,    56,    57,
     725,   727,    17,  -442,   730,   728,   732,   733,   734,   736,
      18,    19,    20,   740,   753,   811,   775,   770,   782,   859,
     781,   789,   790,   813,    21,   249,    22,   793,   863,   799,
     815,   817,   825,   818,    23,    24,    25,   828,   830,    26,
     837,    27,   935,   839,    66,   810,   840,   844,   855,   871,
     881,   885,   890,    28,   883,    29,  -427,   891,    30,   897,
     898,    31,   903,   689,    32,    33,   913,    34,   924,    35,
      36,   925,   926,   181,   936,   930,   927,   249,    37,   946,
      38,    39,    40,    41,    42,    43,    44,    45,    46,   943,
     947,   948,   944,   960,   470,    96,   954,   970,   407,   979,
     986,   989,   987,    47,   985,    48,   984,   992,    49,   996,
    1019,    50,    51,   999,  1004,   835,   737,   842,   910,   951,
    1026,   998,   917,   703,   591,   594,   694,    52,    53,    54,
      55,    56,    57,   546,  1025,   696,   941,   196,   596,   258,
     260,   397,   620,   398,   516,    58,    59,    60,    61,    96,
     621,   976,    62,    63,    64,   977,   866,   983,   912,   541,
     563,   476,   453,   305,   386,   895,   287,   422,    65,    96,
     715,   406,   723,   507,   370,   506,   942,    66,     0,     0,
     995,    67,    68,    69,     0,     0,     0,     0,     0,    70,
      71,    72,    73,    74,    75,    76,    77,     1,    78,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,    12,     0,     0,     0,    13,     0,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,     0,    29,     0,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,    34,     0,    35,    36,
       0,     0,     0,     0,     0,     0,     0,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    47,     0,    48,     0,     0,    49,     0,     0,
      50,    51,     0,     0,     0,     0,     0,     0,     0,   972,
       0,     0,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,    61,     0,     0,
       0,    62,    63,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
      67,    68,    69,     0,     0,     0,     0,     0,    70,    71,
      72,    73,    74,    75,    76,    77,     1,    78,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,     0,     0,     0,    13,     0,    14,    15,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,     0,     0,     0,     0,     0,     0,     0,    18,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,     0,    29,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,    34,     0,    35,    36,     0,
       0,     0,     0,     0,     0,     0,    37,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,    48,     0,     0,    49,     0,  1023,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,    61,     0,     0,     0,
      62,    63,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,     0,     0,     0,
       0,     0,     0,     0,     0,    66,     0,     0,     0,    67,
      68,    69,     0,     0,     0,     0,     0,    70,    71,    72,
      73,    74,    75,    76,    77,     1,    78,     0,     2,     0,
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
       0,    32,    33,     0,    34,     0,    35,    36,     0,     0,
       0,     0,     0,     0,     0,    37,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      47,     0,    48,     0,     0,    49,     0,     0,    50,    51,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,    61,     0,     0,     0,    62,
      63,    64,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,     0,     0,     0,     0,
       0,     0,     0,     0,    66,     0,     0,     0,    67,    68,
      69,     0,     0,     0,     0,     0,    70,    71,    72,    73,
      74,    75,    76,    77,   299,    78,     0,     2,     0,     3,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   300,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,    34,     0,     0,    36,     0,     0,   181,
       0,     0,     0,     0,    37,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    47,
       0,    48,     0,     0,    49,     0,     0,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,    61,     0,     0,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,     0,     0,     0,
       0,     0,     0,    66,     0,     0,     0,    67,    68,    69,
       0,     0,     0,     0,     0,    70,    71,    72,    73,    74,
      75,    76,    77,   194,    78,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,   195,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,    11,   -54,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,    31,     0,     0,    32,
      33,     0,    34,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    37,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    47,     0,
      48,     0,     0,    49,     0,     0,    50,    51,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,    55,    56,    57,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,    61,     0,     0,     0,    62,    63,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,     0,     0,     0,     0,
       0,     0,    66,     0,     0,     0,    67,    68,    69,     0,
       0,     0,     0,     0,    70,    71,    72,    73,    74,    75,
      76,    77,   194,    78,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,    11,     0,     0,     0,    12,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,    26,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    30,     0,     0,    31,     0,     0,    32,    33,
       0,    34,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,    37,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,     0,    48,
       0,     0,    49,     0,     0,    50,    51,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,    61,     0,     0,     0,    62,    63,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,     0,     0,     0,     0,     0,     0,     0,
       0,    66,     0,     0,     0,    67,    68,    69,     0,     0,
       0,     0,     0,    70,    71,    72,    73,    74,    75,    76,
      77,   233,    78,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,   195,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,     0,     0,     0,    12,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
       0,     0,    26,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    30,     0,     0,    31,     0,     0,    32,    33,     0,
      34,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,    37,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,    48,     0,
       0,    49,     0,     0,    50,    51,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,    61,     0,     0,     0,    62,    63,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,     0,     0,     0,     0,     0,     0,     0,     0,
      66,     0,     0,     0,    67,    68,    69,     0,     0,     0,
       0,     0,    70,    71,    72,    73,    74,    75,    76,    77,
     233,    78,     0,     2,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
     195,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,     0,
       0,    26,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,     0,     0,    32,    33,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
      37,     0,    38,    39,    40,    41,    42,    43,    44,    45,
      46,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,     0,    48,     0,     0,
      49,     0,     0,    50,    51,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
      61,     0,     0,     0,    62,    63,    64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,     0,     0,     0,     0,     0,     0,     0,     0,    66,
       0,     0,     0,    67,    68,    69,     0,     0,     0,     0,
       0,    70,    71,    72,    73,    74,    75,    76,    77,   233,
      78,     0,     2,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     0,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,    11,     0,     0,     0,    12,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,     0,   351,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    30,
       0,     0,    31,     0,     0,    32,    33,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,    37,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
       0,     0,     0,     0,   352,   353,   354,     0,     0,     0,
     355,     0,     0,     0,    47,   356,    48,     0,     0,    49,
       0,     0,    50,    51,   652,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
       0,     0,   653,     0,     0,     0,    58,    59,    60,    61,
       0,     0,     0,    62,    63,    64,     0,     0,   361,     0,
       0,     2,     0,     3,     0,     0,     0,   654,     0,    65,
       0,     0,     0,     0,     4,     5,     6,     7,    66,     9,
     655,     0,    67,    68,    69,    10,     0,     0,     0,     0,
      11,     0,     0,     0,    12,     0,    76,    77,     0,    78,
      14,    15,     0,     0,     0,     0,     0,     0,   656,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   657,   658,     0,   300,   659,   660,    30,   661,
     662,    31,   663,     0,    32,    33,     0,     0,     0,     0,
      36,     0,     0,   181,     0,     0,     0,     0,    37,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,     0,
       0,     0,   664,   665,   666,   667,   668,   669,   670,   671,
       0,   672,   673,    47,     0,    48,     0,     0,    49,     0,
       0,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,     0,
       0,     0,    62,    63,    64,     0,     0,   233,     0,     0,
       2,     0,     3,     0,     0,     0,     0,     0,    65,     0,
       0,     0,     0,     4,     5,     6,     7,    66,     9,     0,
       0,    67,    68,    69,    10,     0,     0,     0,     0,    11,
       0,     0,     0,    12,     0,    76,    77,     0,    78,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    47,     0,    48,     0,     0,    49,     0,     0,
      50,    51,   234,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,    61,     0,     0,
       0,    62,    63,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,     0,   233,
       0,     0,     2,     0,     3,     0,    66,     0,     0,     0,
      67,    68,    69,     0,     0,     4,     5,     6,     7,     0,
       9,     0,     0,   391,    76,    77,   392,    78,     0,     0,
       0,    11,     0,     0,     0,    12,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   393,
       0,     0,    31,     0,     0,    32,    33,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,    37,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    47,     0,    48,     0,     0,    49,
       0,     0,    50,    51,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,    61,
       0,     0,     0,    62,    63,    64,     0,     0,   233,   405,
       0,     2,     0,     3,     0,     0,     0,     0,     0,    65,
       0,     0,     0,     0,     4,     5,     6,     7,    66,     9,
       0,     0,    67,    68,    69,    10,     0,     0,     0,     0,
      11,     0,     0,     0,    12,     0,    76,    77,     0,    78,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    30,     0,
       0,    31,     0,     0,    32,    33,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,    37,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    47,     0,    48,     0,     0,    49,     0,
       0,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,     0,
       0,     0,    62,    63,    64,     0,     0,   233,     0,     0,
       2,     0,     3,     0,     0,     0,     0,     0,    65,     0,
       0,     0,     0,     4,     5,     6,     7,    66,     9,     0,
       0,    67,    68,    69,    10,     0,     0,     0,     0,    11,
       0,     0,     0,    12,     0,    76,    77,     0,    78,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     704,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    47,     0,    48,     0,     0,    49,     0,     0,
      50,    51,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,    61,     0,     0,
       0,    62,    63,    64,     0,     0,   233,     0,     0,     2,
       0,     3,     0,     0,     0,     0,     0,    65,     0,     0,
       0,     0,     4,     5,     6,     7,    66,     9,     0,     0,
      67,    68,    69,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,     0,    76,    77,     0,    78,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,    37,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,    48,     0,     0,    49,     0,     0,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,    61,     0,     0,     0,
      62,    63,    64,     0,     0,   233,     0,     0,     2,     0,
       3,     0,     0,     0,     0,     0,    65,     0,     0,     0,
       0,     4,     5,     6,     7,    66,     9,     0,     0,    67,
      68,    69,    10,     0,     0,     0,     0,    11,     0,     0,
       0,    12,     0,    76,    77,     0,    78,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    30,     0,     0,    31,     0,
       0,    32,    33,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,    37,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      47,     0,    48,     0,     0,    49,     0,     0,    50,    51,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,    61,     0,     0,     0,    62,
      63,    64,     0,     0,   233,     0,     0,     2,     0,     3,
       0,     0,     0,     0,     0,    65,     0,     0,     0,     0,
       4,     5,     6,     7,    66,     9,     0,     0,    67,    68,
      69,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,     0,    76,    77,     0,   323,     0,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    31,     0,     0,
       0,    33,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,    37,     0,    38,    39,     0,    41,
       0,     0,    44,    45,    46,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    47,
       0,    48,     0,     0,    49,     0,     0,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,    61,     0,     0,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,     0,     0,     0,
       0,     0,     0,    66,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-848)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     1,     1,    76,    77,    49,   242,    68,    69,   331,
      91,   104,   204,    22,    91,    91,    91,   289,   364,   211,
     327,   338,     1,    18,   691,     5,     4,   338,   442,   221,
       4,    69,     4,   700,   206,     4,     4,     4,   242,    34,
      24,     4,    24,     4,     4,    72,     4,    31,   661,   662,
       3,    63,    45,     4,     4,   236,    76,     5,   663,   634,
     104,   799,     0,     1,   320,   230,     3,    40,     4,   736,
     202,   804,     5,     4,   239,   240,   117,     5,    24,     5,
     117,    24,    91,   654,     4,   656,    27,    64,   210,    68,
      69,     3,   180,   664,   286,     6,    89,     8,   308,   164,
       5,   948,   673,    27,   226,    82,   132,   771,    19,    20,
      21,    22,    20,   276,    22,   319,    23,     3,    59,   786,
      19,    20,    21,    22,   178,    32,   119,   444,   861,    36,
      23,     3,   125,   444,   798,    59,   443,   178,    57,     3,
     201,   178,    64,    36,   137,     3,    72,   279,   494,    69,
       3,    58,     3,  1000,    36,     3,    32,   218,     3,   466,
      79,   134,    49,     6,   257,   219,   253,   254,   300,    76,
      13,    23,    25,   110,    81,   913,    24,   242,    85,     3,
     180,   180,    58,    76,   164,   756,   757,    74,    81,   370,
     761,   128,    85,   254,    76,   194,     4,   130,   291,   121,
     813,   180,   815,    85,   817,   124,   416,   280,   281,   282,
     283,   299,   285,   257,   252,   194,   472,   830,   383,   794,
     199,   954,   201,   828,    63,   252,   221,   250,   251,    81,
     318,   645,   252,     3,   213,   234,   164,   190,   217,   218,
     252,   223,   180,   236,   407,     3,   438,   291,   227,   228,
     229,   230,   231,   232,   233,   264,    64,   461,   253,   164,
     239,   240,   242,   243,   243,     6,    24,     8,   252,   936,
     883,   253,   618,   361,   252,   254,   512,   469,   252,   693,
     252,   242,   242,   252,   252,   252,   367,   604,   460,   252,
     367,   367,   367,   604,   252,   274,   275,   390,   277,   278,
     299,   252,   252,   475,   252,   284,   242,   253,   512,   288,
     482,   242,   253,   121,    24,   886,   526,   527,   190,   986,
     299,   232,   194,   367,   232,    52,   190,   214,   236,   308,
     194,    80,   190,   232,   606,   222,   194,   244,   245,   246,
     247,   248,   249,   650,    72,   190,   390,    52,     6,   194,
       8,   244,   245,   246,   247,   248,   249,    23,   367,     3,
     463,    19,    20,    21,    22,   117,   190,   132,   367,     3,
     194,    24,   689,     3,   477,    76,   225,   129,   689,    36,
      24,    66,   361,   999,    85,   364,   590,    23,    41,   389,
      72,    49,   244,   245,   246,   247,   248,   249,    80,    72,
     395,   250,   251,  1019,   383,    41,   499,    80,    66,    94,
     727,   250,   251,   252,   253,   254,   727,    70,   227,    76,
      56,   148,   149,   150,     3,   177,    24,   154,    85,     3,
     491,   603,   159,    31,     3,   120,    94,   416,   123,     4,
       5,     3,     7,   148,   149,   150,   215,   540,   217,   154,
     103,   450,     0,   514,   159,   499,   435,     7,    48,   168,
     169,    73,   120,     3,   781,     3,    78,   103,     3,   122,
     781,   107,     4,     5,     3,    87,    88,     6,     3,     8,
     657,   658,   215,    73,   217,   464,   122,     3,    78,   121,
      19,    20,    21,    22,    81,    24,   540,    87,    88,     4,
       5,    30,    92,   810,   204,   205,    35,   486,   487,   488,
      39,   490,   491,     3,   493,   494,   708,    46,   243,     4,
       5,   111,     7,   695,   182,   183,   184,   185,   186,   187,
     293,     3,   625,   296,   297,   514,   617,    66,   855,     3,
     617,   617,   617,   242,   855,     4,     5,   526,   527,    14,
      15,    16,    17,    18,     3,    48,     4,     5,   216,     7,
      89,    90,    91,     3,   741,    94,     3,    96,   745,     4,
       5,   748,     7,   617,   232,    19,    20,    21,    22,   199,
      73,   625,     3,   583,   113,    78,     3,   116,   765,   199,
      83,   120,   218,   219,    87,    88,   125,   833,     3,    92,
       4,     5,   252,     7,   133,    48,   135,   136,   617,   138,
       4,     5,   141,   142,   143,   937,    42,   939,   940,   941,
       4,     5,   250,     7,     4,     5,   719,     7,   607,   158,
      73,   160,     4,     5,   163,    78,     3,   166,   167,   618,
      83,    42,   834,   820,    87,    88,   823,   646,    13,    92,
      94,    61,    62,   182,   183,   184,   185,   186,   187,     3,
      48,   170,   171,   172,   210,   211,   180,   181,     6,     4,
       8,   200,   201,   202,   203,   719,     4,     5,   207,   208,
     209,    19,    20,    21,    22,    73,    82,   130,     4,     5,
      78,     4,     5,    82,   223,   139,   140,    24,    36,    87,
      88,   178,   179,   232,    92,     4,     5,    24,   707,    80,
      98,    49,     4,     5,   433,   434,   433,   434,    76,    77,
      94,   250,   251,   168,   253,   280,   281,     4,    66,     4,
     791,   282,   283,     3,   713,   441,   442,    27,   930,   127,
       4,     4,    31,   979,     5,   127,     4,     3,     3,   127,
      83,   730,     5,   101,    83,   799,    94,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,    72,    72,   165,   169,     4,   110,    24,     5,    25,
       3,  1017,   120,   782,     4,    24,    24,    31,   232,    97,
       5,   128,    24,   237,   238,    83,    24,    13,     3,   130,
      24,     6,   244,     8,     5,     5,     5,   243,   165,   220,
       4,    21,   791,     4,    19,    20,    21,    22,    23,    24,
       4,     4,    72,     4,     4,    30,     4,    13,     3,     3,
      35,    97,    84,    84,    39,     5,    98,    74,    43,     4,
      45,    46,    47,     3,   182,   183,   184,   185,   186,   187,
       4,    38,    57,   252,     5,   100,   206,   206,    24,     3,
      65,    66,    67,     3,     3,    21,   131,    40,    75,   913,
      38,     4,     4,   191,    79,   948,    81,    72,   216,    38,
     191,   191,     3,    21,    89,    90,    91,   118,   191,    94,
     111,    96,   891,    41,   232,     5,   174,   221,     5,     4,
       4,    21,    94,   108,   191,   110,   118,     3,   113,    24,
      99,   116,    69,     5,   119,   120,     5,   122,   178,   124,
     125,   217,   217,   128,     3,   107,   212,  1000,   133,     5,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   173,
      31,   224,   168,    21,    31,   945,   243,     3,     3,   225,
       3,    24,     4,   158,   213,   160,   178,    24,   163,    24,
       7,   166,   167,   176,    67,   769,   650,   781,   845,   913,
     227,   970,   860,   604,   458,   461,   590,   182,   183,   184,
     185,   186,   187,   435,  1020,   594,   900,     8,   464,    91,
      91,   265,   512,   266,   387,   200,   201,   202,   203,   999,
     512,   946,   207,   208,   209,   947,   804,   954,   855,   432,
     437,   337,   318,   201,   254,   833,   159,   285,   223,  1019,
     613,   275,   617,   370,   238,   368,   901,   232,    -1,    -1,
     965,   236,   237,   238,    -1,    -1,    -1,    -1,    -1,   244,
     245,   246,   247,   248,   249,   250,   251,     3,   253,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    39,    -1,    -1,    -1,    43,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,   110,    -1,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,   122,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,   160,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,   201,   202,   203,    -1,    -1,
      -1,   207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   232,    -1,    -1,    -1,
     236,   237,   238,    -1,    -1,    -1,    -1,    -1,   244,   245,
     246,   247,   248,   249,   250,   251,     3,   253,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,    -1,    -1,    -1,    43,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,   110,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,   160,    -1,    -1,   163,    -1,   165,   166,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,   201,   202,   203,    -1,    -1,    -1,
     207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   232,    -1,    -1,    -1,   236,
     237,   238,    -1,    -1,    -1,    -1,    -1,   244,   245,   246,
     247,   248,   249,   250,   251,     3,   253,    -1,     6,    -1,
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
      -1,   119,   120,    -1,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,   160,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,   201,   202,   203,    -1,    -1,    -1,   207,
     208,   209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   232,    -1,    -1,    -1,   236,   237,
     238,    -1,    -1,    -1,    -1,    -1,   244,   245,   246,   247,
     248,   249,   250,   251,     3,   253,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   110,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,   122,    -1,    -1,   125,    -1,    -1,   128,
      -1,    -1,    -1,    -1,   133,    -1,   135,   136,   137,   138,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,   160,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,   201,   202,   203,    -1,    -1,    -1,   207,   208,
     209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   232,    -1,    -1,    -1,   236,   237,   238,
      -1,    -1,    -1,    -1,    -1,   244,   245,   246,   247,   248,
     249,   250,   251,     3,   253,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    36,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,
     120,    -1,   122,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
     160,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,   201,   202,   203,    -1,    -1,    -1,   207,   208,   209,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   232,    -1,    -1,    -1,   236,   237,   238,    -1,
      -1,    -1,    -1,    -1,   244,   245,   246,   247,   248,   249,
     250,   251,     3,   253,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,   120,
      -1,   122,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,   160,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,
     201,   202,   203,    -1,    -1,    -1,   207,   208,   209,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   232,    -1,    -1,    -1,   236,   237,   238,    -1,    -1,
      -1,    -1,    -1,   244,   245,   246,   247,   248,   249,   250,
     251,     3,   253,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    39,    -1,    -1,
      -1,    -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      -1,    -1,    94,    -1,    96,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,   116,    -1,    -1,   119,   120,    -1,
     122,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,   160,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,   201,
     202,   203,    -1,    -1,    -1,   207,   208,   209,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     232,    -1,    -1,    -1,   236,   237,   238,    -1,    -1,    -1,
      -1,    -1,   244,   245,   246,   247,   248,   249,   250,   251,
       3,   253,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,
      -1,    94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,    -1,   135,   136,   137,   138,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,   160,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,   201,   202,
     203,    -1,    -1,    -1,   207,   208,   209,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     223,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   232,
      -1,    -1,    -1,   236,   237,   238,    -1,    -1,    -1,    -1,
      -1,   244,   245,   246,   247,   248,   249,   250,   251,     3,
     253,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,   116,    -1,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,    -1,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,   167,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,   200,   201,   202,   203,
      -1,    -1,    -1,   207,   208,   209,    -1,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    -1,    93,    -1,   223,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,   232,    24,
     106,    -1,   236,   237,   238,    30,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    39,    -1,   250,   251,    -1,   253,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,
      -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   188,   189,    -1,   110,   192,   193,   113,   195,
     196,   116,   198,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,    -1,    -1,   128,    -1,    -1,    -1,    -1,   133,    -1,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    -1,
      -1,    -1,   228,   229,   230,   231,   232,   233,   234,   235,
      -1,   237,   238,   158,    -1,   160,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   200,   201,   202,   203,    -1,
      -1,    -1,   207,   208,   209,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   223,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,   232,    24,    -1,
      -1,   236,   237,   238,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    39,    -1,   250,   251,    -1,   253,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,   160,    -1,    -1,   163,    -1,    -1,
     166,   167,   168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,   201,   202,   203,    -1,    -1,
      -1,   207,   208,   209,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,     3,
      -1,    -1,     6,    -1,     8,    -1,   232,    -1,    -1,    -1,
     236,   237,   238,    -1,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    27,   250,   251,    30,   253,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,   116,    -1,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,   160,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,   201,   202,   203,
      -1,    -1,    -1,   207,   208,   209,    -1,    -1,     3,     4,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   223,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,   232,    24,
      -1,    -1,   236,   237,   238,    30,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    39,    -1,   250,   251,    -1,   253,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,
      -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,   116,    -1,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,   160,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   200,   201,   202,   203,    -1,
      -1,    -1,   207,   208,   209,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   223,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,   232,    24,    -1,
      -1,   236,   237,   238,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    39,    -1,   250,   251,    -1,   253,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,   160,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,   201,   202,   203,    -1,    -1,
      -1,   207,   208,   209,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   232,    24,    -1,    -1,
     236,   237,   238,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,    -1,   250,   251,    -1,   253,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,   160,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,   201,   202,   203,    -1,    -1,    -1,
     207,   208,   209,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,   232,    24,    -1,    -1,   236,
     237,   238,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    39,    -1,   250,   251,    -1,   253,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,
      -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,   160,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,   201,   202,   203,    -1,    -1,    -1,   207,
     208,   209,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,   232,    24,    -1,    -1,   236,   237,
     238,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,    -1,   250,   251,    -1,   253,    -1,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    -1,   135,   136,    -1,   138,
      -1,    -1,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,   160,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,   201,   202,   203,    -1,    -1,    -1,   207,   208,
     209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   223,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   232,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   253
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    35,    39,    43,    45,    46,    47,    57,    65,    66,
      67,    79,    81,    89,    90,    91,    94,    96,   108,   110,
     113,   116,   119,   120,   122,   124,   125,   133,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   158,   160,   163,
     166,   167,   182,   183,   184,   185,   186,   187,   200,   201,
     202,   203,   207,   208,   209,   223,   232,   236,   237,   238,
     244,   245,   246,   247,   248,   249,   250,   251,   253,   258,
     259,   260,   261,   272,   280,   281,   282,   283,   284,   285,
     286,   289,   290,   291,   292,   297,   302,   328,   330,   331,
     332,   333,   334,   336,   337,   339,   340,   341,   342,   344,
     345,   347,   348,   350,   352,   353,   354,   358,   360,   361,
     362,   363,   364,   365,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   384,   385,   390,   391,   393,
     425,   426,   444,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   468,   469,   470,   471,   472,
     474,   475,   476,   477,   481,   483,   484,   485,   486,   487,
     494,   495,   496,   521,   522,   523,   524,   525,   537,   539,
       3,   128,   259,   287,   301,   302,   333,   437,   438,   439,
     441,   443,   468,    24,     3,    23,   330,    25,   493,     3,
     132,     3,     3,   117,   129,   177,    72,     3,   354,    24,
     536,    80,   280,     3,   132,    27,    59,   293,     3,   354,
     227,   322,   323,   431,   432,   433,   435,     3,     3,     3,
       3,     3,     3,     3,   168,   290,   530,   531,   535,     3,
       3,     3,     3,     3,     3,   468,   469,   469,   451,   485,
     451,     0,     7,    76,    85,   121,   280,   337,   338,   340,
     341,   344,   347,   349,    81,   243,   242,   289,   290,    20,
      22,   232,   236,     3,     3,     3,   199,     3,     3,     3,
     253,   254,   250,   251,   252,   250,    42,   473,     3,    13,
     441,     3,     4,    82,     4,     4,    64,   121,     4,     3,
     110,   302,   333,   445,   468,   445,   469,   253,   293,    24,
     489,   490,   491,   492,   489,    24,   559,   323,     3,    80,
     489,    94,   468,   253,   303,   355,   356,   468,   488,   469,
     354,   489,    48,    73,    78,    87,    88,    92,   111,   427,
     430,   468,   468,   465,   468,   465,   468,   478,   479,   480,
     468,    52,   148,   149,   150,   154,   159,   366,   367,   460,
     468,     3,   468,   333,   169,   531,   532,   168,   526,   527,
     530,   465,   468,   465,   468,     4,   358,   379,   380,   381,
     537,   468,     4,     3,   346,   354,   449,   469,    27,   442,
      36,    27,    30,   113,   289,   351,   280,   331,   332,     4,
     466,   467,   468,   533,   534,     4,   467,     3,    24,   396,
     397,   398,   404,   386,   468,   468,   293,   452,   452,   453,
     453,   468,   474,   485,   489,   468,    24,   223,   495,     4,
     288,   289,   442,   442,   442,   293,    31,     5,   127,     4,
     466,     6,    13,     3,     3,   275,   170,   171,   172,   541,
     130,   300,   324,   443,   298,   299,   357,   537,   275,     4,
      72,    80,   316,   317,     5,    24,    31,   359,   520,   127,
      31,   319,   538,   319,   321,    83,   430,   294,   295,   494,
     496,   101,    83,     4,     4,     4,     5,     7,    76,     4,
      72,    72,     4,    72,   164,   533,   165,   290,   335,   337,
     338,   340,   344,   347,   528,   529,   532,   527,     4,     4,
       4,     4,     5,   465,    63,   343,   343,   302,   289,   354,
      66,    94,   120,   123,   327,     4,     5,     7,     4,   396,
     405,   408,   409,     5,   382,     4,   466,     4,     3,     4,
       5,   438,   440,   301,   439,   301,   303,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   446,   447,   446,   489,     4,    24,   491,    24,   491,
     492,    41,    70,   103,   122,   262,   263,   264,   267,   269,
     520,   271,   496,    31,    97,    57,    79,   124,   542,   333,
       5,   287,   318,   323,   298,   300,   356,   520,   489,    24,
     275,    24,   323,    83,     5,   300,    13,   244,   323,   468,
     468,   468,   468,   469,   468,   533,   289,     5,   164,   165,
     379,   380,   394,     4,   469,   243,   466,   466,   220,   406,
     410,     4,    21,   387,     4,   204,   205,   383,   389,     4,
       4,   289,     4,     4,     4,    13,     3,    84,    84,     4,
       5,     3,    40,    68,    93,   106,   134,   188,   189,   192,
     193,   195,   196,   198,   228,   229,   230,   231,   232,   233,
     234,   235,   237,   238,   497,   500,   501,   508,   509,   510,
     511,   512,   513,   515,   516,   517,   518,   519,     4,     5,
     302,    97,   543,    98,   299,     5,   316,    74,   325,     4,
       3,   274,   323,   295,    56,   296,   468,    98,   127,   428,
     429,   436,     4,    69,   482,   482,     4,   280,   290,   337,
     340,   344,   347,   529,   533,     4,   289,    38,   100,   276,
       5,   383,   206,   206,    24,   333,     3,   263,   270,   520,
       3,   190,   194,   502,   503,   190,   194,   502,   190,   194,
     502,   503,   503,     3,   504,   505,    40,   134,   504,   504,
     154,   460,   461,   463,   464,   190,   194,   502,   502,   265,
      40,   498,   473,   499,   496,   131,   273,   270,   544,   492,
     323,    38,    75,   326,   270,   333,     3,   489,   468,     4,
       4,    85,   289,    72,   392,   395,   411,   412,   496,    38,
     218,   219,   407,   414,   415,   388,   468,     4,   270,     4,
       5,    21,   503,   191,   503,   191,   503,   191,    21,   506,
     190,   194,   502,   190,   502,     3,   514,   502,   118,   503,
     191,    23,    41,    56,   107,   267,   268,   111,   499,    41,
     174,   540,   271,   333,   221,   399,   400,     4,   270,    61,
      62,   434,   210,   211,   383,     5,   499,   277,   278,   290,
     334,    36,    49,   216,   369,   416,   417,   418,   419,     4,
     520,     4,   504,   504,   504,    14,    15,    16,    17,    18,
     507,     4,   503,   191,   503,    21,   154,   460,   462,   504,
      94,     3,    94,   266,   358,   456,   489,    24,    99,   180,
     181,   552,   553,    69,   545,   546,   396,   401,   402,   403,
     276,     4,   412,     5,    32,    58,   279,   279,   216,   369,
     417,   420,   422,   423,   178,   217,   217,   212,   413,   424,
     107,   504,     4,     5,   502,   333,     3,   117,   178,   320,
     117,   320,   553,   173,   168,   548,     5,    31,   224,   313,
     315,   278,   215,   215,   243,    49,    74,   214,   222,   489,
      21,     4,   270,   319,   319,   319,   319,   178,   179,   547,
       3,   329,   175,   259,   549,   551,   402,   404,   454,   225,
     304,   305,   421,   422,   178,   213,     3,     4,     4,    24,
     555,   556,    24,   558,   554,   556,    24,   557,   333,   176,
     225,   314,   311,   358,    67,   309,   310,   270,     4,   550,
     551,   454,   178,   219,   308,   210,   226,   307,     4,     7,
     306,   312,   358,   165,   551,   308,   227
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 466:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 467:

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

  case 468:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 472:

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

  case 473:

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

  case 474:

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

  case 475:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 479:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 480:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 481:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 482:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 483:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 484:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 485:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 486:

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

  case 487:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 488:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 489:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 491:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 492:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 493:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 495:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 496:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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

  case 508:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 510:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 511:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 513:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 514:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 515:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 516:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("K", SQL_NODE_PUNCTUATION));
        }
    break;

  case 517:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("M", SQL_NODE_PUNCTUATION));
        }
    break;

  case 518:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("G", SQL_NODE_PUNCTUATION));
        }
    break;

  case 519:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("T", SQL_NODE_PUNCTUATION));
        }
    break;

  case 520:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("P", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 523:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 526:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 529:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 531:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 542:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 543:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 544:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 555:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            /*$$->append($3);*/
        }
    break;

  case 556:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 560:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 561:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 562:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 565:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 566:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 567:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 568:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 569:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 570:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 571:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 578:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 579:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 580:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 581:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 582:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 586:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 587:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 588:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 589:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 590:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 591:

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

  case 593:

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

  case 594:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 595:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 598:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 601:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 602:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 603:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 605:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 606:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
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

  case 610:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 611:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 613:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(";", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 614:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 615:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 617:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 618:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 619:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 620:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 621:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 622:

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
