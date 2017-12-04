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
#define YYPURE 2

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

#define CREATE_NODE  context->GetScanner()->NewNode

// yyi ist die interne Nr. der Regel, die gerade reduziert wird.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE             context->GetScanner()->NewNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE         context->GetScanner()->NewNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE   context->GetScanner()->NewNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])
#define SQL_NEW_DOTLISTRULE   context->GetScanner()->NewNode(aEmptyString, SQL_NODE_DOTLISTRULE, yyr1[yyn])

#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE
#endif

#define SQLyyerror(context, s) \
    {                                 \
    context->error(s);                \
    }

using namespace connectivity;
#define SQLyylex context->SQLlex



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
#ifndef YY_SQLYY_D_DEV_DGNDB_BIM20CS_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_D_DEV_DGNDB_BIM20CS_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_OR = 414,
     SQL_TOKEN_AND = 415,
     SQL_EQUAL = 416,
     SQL_GREAT = 417,
     SQL_LESS = 418,
     SQL_NOTEQUAL = 419,
     SQL_GREATEQ = 420,
     SQL_LESSEQ = 421,
     SQL_CONCAT = 422,
     SQL_TOKEN_INVALIDSYMBOL = 423
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

#endif /* !YY_SQLYY_D_DEV_DGNDB_BIM20CS_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */

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
#define YYFINAL  226
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3277

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  194
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  236
/* YYNRULES -- Number of rules.  */
#define YYNRULES  478
/* YYNRULES -- Number of states.  */
#define YYNSTATES  753

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   423

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   190,   178,     2,
       3,     4,   188,   185,     5,   186,    13,   189,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   192,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   177,    12,   191,     2,     2,     2,
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
     171,   172,   173,   174,   175,   176,   179,   180,   181,   182,
     183,   184,   187,   193
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    11,    15,    17,    18,    22,
      23,    27,    28,    32,    34,    38,    41,    44,    45,    47,
      49,    50,    52,    54,    56,    58,    60,    62,    64,    66,
      68,    73,    75,    77,    79,    82,    88,    94,    99,   101,
     105,   107,   109,   112,   119,   120,   122,   124,   126,   130,
     134,   136,   138,   145,   147,   151,   153,   154,   156,   161,
     163,   165,   167,   168,   170,   171,   174,   178,   187,   190,
     192,   196,   197,   199,   200,   204,   205,   207,   211,   216,
     218,   221,   222,   226,   227,   230,   232,   234,   236,   238,
     240,   242,   246,   248,   250,   255,   257,   260,   262,   266,
     268,   272,   274,   276,   278,   280,   282,   284,   286,   288,
     290,   293,   297,   300,   302,   304,   306,   308,   310,   312,
     315,   321,   324,   329,   334,   337,   340,   342,   344,   345,
     348,   352,   355,   357,   359,   363,   367,   370,   372,   376,
     379,   382,   386,   388,   390,   392,   395,   398,   402,   404,
     408,   410,   412,   414,   416,   418,   420,   422,   425,   428,
     431,   434,   435,   438,   440,   447,   452,   454,   456,   458,
     463,   468,   473,   478,   480,   482,   484,   486,   488,   490,
     492,   499,   501,   503,   505,   507,   509,   511,   513,   515,
     517,   519,   521,   523,   525,   529,   533,   538,   543,   549,
     551,   553,   555,   557,   559,   563,   567,   569,   571,   573,
     575,   577,   582,   584,   586,   588,   590,   591,   594,   599,
     600,   602,   609,   611,   613,   615,   617,   619,   622,   625,
     631,   633,   635,   636,   638,   647,   649,   651,   654,   657,
     659,   661,   663,   665,   666,   668,   671,   675,   677,   681,
     683,   687,   688,   690,   691,   693,   694,   696,   701,   703,
     707,   711,   713,   716,   717,   719,   723,   725,   727,   729,
     731,   734,   736,   739,   742,   747,   749,   751,   753,   756,
     758,   761,   765,   768,   771,   775,   781,   786,   792,   794,
     796,   798,   800,   802,   804,   806,   808,   810,   812,   815,
     817,   819,   820,   822,   824,   827,   832,   838,   844,   846,
     854,   855,   857,   859,   861,   863,   868,   869,   871,   873,
     875,   877,   879,   881,   883,   885,   887,   889,   891,   893,
     895,   897,   899,   901,   903,   905,   907,   911,   914,   916,
     918,   925,   927,   929,   931,   933,   935,   937,   941,   943,
     945,   947,   949,   952,   955,   957,   961,   965,   969,   971,
     975,   979,   981,   983,   985,   988,   991,   993,   995,   997,
     999,  1001,  1003,  1005,  1007,  1009,  1013,  1015,  1017,  1021,
    1025,  1027,  1029,  1031,  1033,  1035,  1037,  1041,  1045,  1047,
    1049,  1052,  1054,  1057,  1059,  1061,  1063,  1071,  1073,  1075,
    1076,  1078,  1080,  1082,  1084,  1085,  1088,  1096,  1101,  1103,
    1105,  1110,  1117,  1124,  1131,  1134,  1136,  1138,  1142,  1146,
    1150,  1154,  1156,  1158,  1160,  1164,  1168,  1170,  1172,  1177,
    1178,  1180,  1182,  1186,  1189,  1191,  1193,  1194,  1196,  1198,
    1200,  1202,  1204,  1210,  1215,  1217,  1220,  1225,  1227,  1231,
    1233,  1235,  1237,  1239,  1241,  1243,  1245,  1248,  1253,  1254,
    1257,  1259,  1261,  1263,  1266,  1268,  1269,  1272,  1274,  1278,
    1279,  1281,  1284,  1287,  1289,  1291,  1295,  1297,  1299
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     195,     0,    -1,   205,    -1,   196,     5,   408,    -1,   408,
      -1,   197,     5,   406,    -1,   406,    -1,    -1,     3,   196,
       4,    -1,    -1,     3,   197,     4,    -1,    -1,    81,    37,
     201,    -1,   202,    -1,   201,     5,   202,    -1,   247,   203,
      -1,   214,   203,    -1,    -1,    32,    -1,    49,    -1,    -1,
      23,    -1,   208,    -1,   209,    -1,   210,    -1,   215,    -1,
     216,    -1,   221,    -1,   206,    -1,   225,    -1,   225,   207,
     352,   206,    -1,    69,    -1,    92,    -1,    54,    -1,    40,
     101,    -1,    48,    59,   236,   224,   425,    -1,    66,    67,
     395,   199,   211,    -1,    98,     3,   212,     4,    -1,   213,
      -1,   212,     5,   213,    -1,   214,    -1,   374,    -1,    83,
     101,    -1,    84,   217,   226,    67,   222,   230,    -1,    -1,
      27,    -1,    50,    -1,   219,    -1,   218,     5,   219,    -1,
     406,   179,   220,    -1,   374,    -1,    47,    -1,    95,   236,
      85,   218,   224,   425,    -1,   223,    -1,   222,     5,   223,
      -1,   272,    -1,    -1,   237,    -1,    84,   217,   226,   230,
      -1,   211,    -1,   188,    -1,   270,    -1,    -1,   229,    -1,
      -1,   157,   364,    -1,   156,   364,   228,    -1,   231,   224,
     238,   239,   314,   200,   227,   425,    -1,    59,   232,    -1,
     236,    -1,   232,     5,   236,    -1,    -1,    31,    -1,    -1,
     233,    24,   198,    -1,    -1,   158,    -1,   235,   399,   234,
      -1,   235,   269,   424,   199,    -1,   350,    -1,    99,   246,
      -1,    -1,    61,    37,   371,    -1,    -1,    62,   246,    -1,
      91,    -1,    56,    -1,    94,    -1,    78,    -1,   247,    -1,
     242,    -1,     3,   246,     4,    -1,   374,    -1,   241,    -1,
     241,    68,   204,   240,    -1,   243,    -1,    23,   243,    -1,
     244,    -1,   245,   176,   244,    -1,   245,    -1,   246,   175,
     245,    -1,   249,    -1,   252,    -1,   263,    -1,   267,    -1,
     268,    -1,   258,    -1,   261,    -1,   255,    -1,   264,    -1,
     250,   213,    -1,   213,   250,   213,    -1,   250,   213,    -1,
     181,    -1,   182,    -1,   179,    -1,   180,    -1,   184,    -1,
     183,    -1,    68,   204,    -1,   204,    35,   213,   176,   213,
      -1,   213,   251,    -1,   204,    71,   375,   256,    -1,   204,
      71,   360,   256,    -1,   213,   253,    -1,   213,   254,    -1,
     253,    -1,   254,    -1,    -1,    53,   375,    -1,    68,   204,
      78,    -1,   213,   257,    -1,   257,    -1,   269,    -1,     3,
     371,     4,    -1,   204,    63,   259,    -1,   213,   260,    -1,
     260,    -1,   250,   266,   269,    -1,   213,   262,    -1,   213,
     265,    -1,   204,   159,   286,    -1,    30,    -1,    27,    -1,
      86,    -1,    55,   269,    -1,    93,   269,    -1,     3,   206,
       4,    -1,   271,    -1,   270,     5,   271,    -1,   394,    -1,
     423,    -1,   162,    -1,    20,    -1,    21,    -1,    22,    -1,
      19,    -1,   273,   169,    -1,   273,   162,    -1,   273,    20,
      -1,   273,    22,    -1,    -1,    31,   408,    -1,   408,    -1,
     104,     3,   374,    63,   374,     4,    -1,   104,     3,   371,
       4,    -1,   275,    -1,   283,    -1,   280,    -1,   103,     3,
     374,     4,    -1,   106,     3,   374,     4,    -1,    79,     3,
     374,     4,    -1,   102,     3,   374,     4,    -1,   277,    -1,
     278,    -1,   279,    -1,   370,    -1,   115,    -1,   281,    -1,
     374,    -1,   111,     3,   282,    59,   374,     4,    -1,   285,
      -1,   273,    -1,   423,    -1,    78,    -1,    56,    -1,    91,
      -1,   126,    -1,   127,    -1,   128,    -1,   129,    -1,   130,
      -1,   131,    -1,   340,    -1,   289,     3,     4,    -1,   287,
       3,     4,    -1,   289,     3,   373,     4,    -1,   288,     3,
     373,     4,    -1,   289,     3,   217,   372,     4,    -1,   290,
      -1,   116,    -1,    24,    -1,   109,    -1,   110,    -1,   292,
     132,   312,    -1,   133,     3,     4,    -1,   340,    -1,   293,
      -1,   299,    -1,   305,    -1,   308,    -1,   134,     3,   296,
       4,    -1,   423,    -1,   273,    -1,   295,    -1,   294,    -1,
      -1,     5,   302,    -1,     5,   302,     5,   303,    -1,    -1,
     304,    -1,   300,     3,   301,   297,     4,   298,    -1,   135,
      -1,   136,    -1,   374,    -1,    21,    -1,   374,    -1,   137,
     139,    -1,   138,   139,    -1,   306,     3,   374,     4,   298,
      -1,   140,    -1,   141,    -1,    -1,   310,    -1,   142,     3,
     374,     5,   309,     4,   307,   298,    -1,   295,    -1,   294,
      -1,    59,   143,    -1,    59,   144,    -1,    24,    -1,   311,
      -1,   313,    -1,   319,    -1,    -1,   315,    -1,   154,   316,
      -1,   316,     5,   317,    -1,   317,    -1,   318,    31,   319,
      -1,   311,    -1,     3,   323,     4,    -1,    -1,   324,    -1,
      -1,   325,    -1,    -1,   329,    -1,   320,   321,   200,   322,
      -1,   311,    -1,   153,    37,   326,    -1,   326,     5,   327,
      -1,   327,    -1,   406,   407,    -1,    -1,   339,    -1,   330,
     331,   328,    -1,   152,    -1,   151,    -1,   332,    -1,   334,
      -1,   149,   150,    -1,   333,    -1,    44,   125,    -1,   284,
     150,    -1,    35,   335,   176,   336,    -1,   337,    -1,   337,
      -1,   332,    -1,   149,   148,    -1,   338,    -1,   284,   148,
      -1,   145,    44,   125,    -1,   145,    61,    -1,   145,   147,
      -1,   145,   155,   146,    -1,   341,     3,   217,   372,     4,
      -1,    42,     3,   188,     4,    -1,    42,     3,   217,   372,
       4,    -1,    34,    -1,    75,    -1,    76,    -1,    89,    -1,
     118,    -1,    30,    -1,    86,    -1,    72,    -1,    73,    -1,
      60,    -1,    80,   246,    -1,   343,    -1,   351,    -1,    -1,
      65,    -1,   342,    -1,   342,    82,    -1,   236,    43,    70,
     236,    -1,   236,    77,   345,    70,   236,    -1,   236,   345,
      70,   236,   344,    -1,   346,    -1,   236,   345,    70,   236,
      97,   399,   349,    -1,    -1,    51,    -1,    52,    -1,   348,
      -1,   347,    -1,    97,     3,   196,     4,    -1,    -1,    27,
      -1,   269,    -1,   374,    -1,   171,    -1,   172,    -1,   165,
      -1,   166,    -1,   168,    -1,   162,    -1,   161,    -1,   163,
      -1,   164,    -1,   167,    -1,   169,    -1,   173,    -1,   174,
      -1,   170,    -1,    24,    -1,   355,    -1,    24,    13,    24,
      -1,   356,    25,    -1,   356,    -1,   357,    -1,    38,     3,
     354,    31,   358,     4,    -1,   284,    -1,   286,    -1,   406,
      -1,   353,    -1,   409,    -1,   291,    -1,     3,   374,     4,
      -1,   359,    -1,   360,    -1,   276,    -1,   361,    -1,   186,
     361,    -1,   185,   361,    -1,   362,    -1,   363,   188,   362,
      -1,   363,   189,   362,    -1,   363,   190,   362,    -1,   363,
      -1,   364,   185,   363,    -1,   364,   186,   363,    -1,   366,
      -1,   107,    -1,   108,    -1,   173,   375,    -1,   174,   375,
      -1,   365,    -1,   367,    -1,   368,    -1,   117,    -1,   114,
      -1,    46,    -1,   112,    -1,   113,    -1,   374,    -1,   371,
       5,   374,    -1,   420,    -1,   372,    -1,   373,     5,   372,
      -1,   373,     7,   372,    -1,   364,    -1,   375,    -1,   369,
      -1,   376,    -1,   380,    -1,   377,    -1,   376,   185,   380,
      -1,   374,   187,   374,    -1,   169,    -1,   381,    -1,    39,
     395,    -1,   378,    -1,   378,   379,    -1,   387,    -1,   382,
      -1,   383,    -1,   105,     3,   384,    59,   375,   388,     4,
      -1,   385,    -1,   386,    -1,    -1,   389,    -1,   391,    -1,
     392,    -1,   393,    -1,    -1,    57,   374,    -1,   105,     3,
     374,    59,   374,   388,     4,    -1,   105,     3,   371,     4,
      -1,    96,    -1,    74,    -1,   390,     3,   374,     4,    -1,
      41,     3,   375,    97,   395,     4,    -1,    41,     3,   354,
       5,   358,     4,    -1,    90,     3,   375,    97,   395,     4,
      -1,   374,   274,    -1,   397,    -1,   396,    -1,    24,    13,
     397,    -1,    24,     6,   397,    -1,    24,    13,   398,    -1,
      24,     6,   398,    -1,    24,    -1,   400,    -1,   401,    -1,
     400,    13,   401,    -1,   400,     6,   401,    -1,    24,    -1,
     402,    -1,   289,     3,   373,     4,    -1,    -1,    25,    -1,
     405,    -1,   404,    13,   405,    -1,    24,   403,    -1,   188,
      -1,   404,    -1,    -1,   379,    -1,    24,    -1,   410,    -1,
     411,    -1,   412,    -1,   120,   422,   413,   419,   122,    -1,
     120,   417,   419,   122,    -1,   414,    -1,   417,   414,    -1,
     123,   415,   121,   420,    -1,   416,    -1,   415,     5,   416,
      -1,   214,    -1,   248,    -1,   251,    -1,   260,    -1,   253,
      -1,   257,    -1,   418,    -1,   417,   418,    -1,   123,   246,
     121,   420,    -1,    -1,   124,   420,    -1,   421,    -1,   374,
      -1,   214,    -1,     6,    24,    -1,     8,    -1,    -1,   233,
      24,    -1,   246,    -1,     3,   195,     4,    -1,    -1,   426,
      -1,   160,   427,    -1,   427,   428,    -1,   428,    -1,    24,
      -1,    24,   179,   429,    -1,   273,    -1,    24,    -1,   240,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   239,   239,   244,   249,   257,   262,   272,   273,   281,
     282,   293,   294,   304,   309,   317,   325,   334,   335,   336,
     340,   341,   347,   348,   349,   350,   351,   352,   353,   357,
     362,   373,   374,   375,   378,   385,   397,   406,   416,   421,
     429,   432,   437,   447,   458,   459,   460,   464,   467,   473,
     480,   481,   484,   496,   499,   505,   509,   510,   518,   526,
     530,   535,   538,   539,   542,   543,   551,   560,   575,   585,
     588,   594,   595,   599,   602,   611,   612,   616,   623,   631,
     634,   643,   644,   654,   655,   663,   664,   665,   666,   669,
     670,   671,   686,   694,   695,   705,   706,   714,   715,   724,
     725,   735,   736,   737,   738,   739,   740,   741,   742,   743,
     749,   756,   763,   788,   789,   790,   791,   792,   793,   794,
     802,   812,   820,   830,   840,   846,   852,   874,   899,   900,
     907,   916,   922,   938,   942,   950,   959,   965,   981,   990,
     999,  1008,  1018,  1019,  1020,  1024,  1032,  1038,  1049,  1054,
    1061,  1065,  1069,  1070,  1071,  1072,  1073,  1075,  1087,  1099,
    1111,  1127,  1128,  1134,  1137,  1147,  1157,  1158,  1159,  1162,
    1170,  1181,  1191,  1201,  1206,  1211,  1218,  1223,  1230,  1231,
    1235,  1247,  1248,  1251,  1252,  1253,  1254,  1255,  1256,  1257,
    1258,  1259,  1260,  1263,  1264,  1271,  1278,  1286,  1300,  1313,
    1316,  1320,  1324,  1325,  1330,  1339,  1346,  1347,  1348,  1349,
    1350,  1353,  1363,  1366,  1369,  1370,  1373,  1374,  1380,  1390,
    1391,  1395,  1407,  1408,  1411,  1414,  1417,  1420,  1421,  1424,
    1435,  1436,  1439,  1440,  1443,  1457,  1458,  1461,  1467,  1475,
    1478,  1479,  1482,  1485,  1486,  1489,  1497,  1500,  1505,  1514,
    1517,  1526,  1527,  1530,  1531,  1534,  1535,  1538,  1544,  1547,
    1556,  1559,  1564,  1572,  1573,  1576,  1585,  1586,  1589,  1590,
    1593,  1599,  1600,  1608,  1616,  1626,  1629,  1632,  1633,  1639,
    1642,  1650,  1657,  1663,  1669,  1679,  1688,  1696,  1708,  1709,
    1710,  1711,  1712,  1713,  1714,  1718,  1723,  1728,  1735,  1743,
    1744,  1747,  1748,  1753,  1754,  1762,  1774,  1784,  1793,  1798,
    1812,  1813,  1814,  1817,  1818,  1821,  1833,  1834,  1838,  1841,
    1845,  1846,  1847,  1848,  1849,  1850,  1851,  1852,  1853,  1854,
    1855,  1856,  1857,  1858,  1859,  1863,  1868,  1878,  1887,  1888,
    1892,  1904,  1905,  1906,  1907,  1908,  1909,  1910,  1917,  1921,
    1922,  1926,  1927,  1933,  1942,  1943,  1950,  1957,  1967,  1968,
    1975,  1989,  1996,  2002,  2007,  2013,  2023,  2030,  2038,  2046,
    2047,  2048,  2049,  2050,  2055,  2058,  2064,  2091,  2094,  2098,
    2111,  2112,  2113,  2116,  2124,  2125,  2128,  2135,  2145,  2146,
    2149,  2157,  2158,  2166,  2167,  2170,  2177,  2190,  2214,  2221,
    2224,  2225,  2226,  2231,  2238,  2239,  2247,  2258,  2268,  2269,
    2272,  2282,  2292,  2304,  2317,  2327,  2332,  2339,  2347,  2356,
    2364,  2373,  2381,  2389,  2394,  2399,  2407,  2412,  2419,  2429,
    2430,  2438,  2443,  2461,  2467,  2475,  2482,  2483,  2493,  2497,
    2501,  2502,  2506,  2517,  2527,  2532,  2539,  2549,  2552,  2557,
    2558,  2559,  2560,  2561,  2562,  2565,  2570,  2577,  2587,  2588,
    2596,  2600,  2603,  2607,  2613,  2621,  2624,  2634,  2648,  2652,
    2653,  2657,  2666,  2671,  2679,  2685,  2695,  2696,  2697
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
  "SQL_TOKEN_ASC", "SQL_TOKEN_AUTHORIZATION", "SQL_TOKEN_AVG",
  "SQL_TOKEN_BETWEEN", "SQL_TOKEN_BIT", "SQL_TOKEN_BY", "SQL_TOKEN_CAST",
  "SQL_TOKEN_COLLATE", "SQL_TOKEN_COMMIT", "SQL_TOKEN_CONVERT",
  "SQL_TOKEN_COUNT", "SQL_TOKEN_CROSS", "SQL_TOKEN_CURRENT",
  "SQL_TOKEN_CURSOR", "SQL_TOKEN_DAY", "SQL_TOKEN_DEFAULT",
  "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC", "SQL_TOKEN_DISTINCT",
  "SQL_TOKEN_FORWARD", "SQL_TOKEN_BACKWARD", "SQL_TOKEN_ESCAPE",
  "SQL_TOKEN_EXCEPT", "SQL_TOKEN_EXISTS", "SQL_TOKEN_FALSE",
  "SQL_TOKEN_FOR", "SQL_TOKEN_FOUND", "SQL_TOKEN_FROM", "SQL_TOKEN_FULL",
  "SQL_TOKEN_GROUP", "SQL_TOKEN_HAVING", "SQL_TOKEN_IN",
  "SQL_TOKEN_INDICATOR", "SQL_TOKEN_INNER", "SQL_TOKEN_INSERT",
  "SQL_TOKEN_INTO", "SQL_TOKEN_IS", "SQL_TOKEN_INTERSECT",
  "SQL_TOKEN_JOIN", "SQL_TOKEN_LIKE", "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT",
  "SQL_TOKEN_LOWER", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN", "SQL_TOKEN_NATURAL",
  "SQL_TOKEN_NULL", "SQL_TOKEN_OCTET_LENGTH", "SQL_TOKEN_ON",
  "SQL_TOKEN_ORDER", "SQL_TOKEN_OUTER", "SQL_TOKEN_ROLLBACK",
  "SQL_TOKEN_SELECT", "SQL_TOKEN_SET", "SQL_TOKEN_SOME",
  "SQL_TOKEN_SQLCODE", "SQL_TOKEN_SQLERROR", "SQL_TOKEN_SUM",
  "SQL_TOKEN_TRANSLATE", "SQL_TOKEN_TRUE", "SQL_TOKEN_UNION",
  "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN", "SQL_TOKEN_UPDATE",
  "SQL_TOKEN_UPPER", "SQL_TOKEN_USING", "SQL_TOKEN_VALUES",
  "SQL_TOKEN_WHERE", "SQL_TOKEN_WITH", "SQL_TOKEN_WORK",
  "SQL_TOKEN_BIT_LENGTH", "SQL_TOKEN_CHAR_LENGTH", "SQL_TOKEN_POSITION",
  "SQL_TOKEN_SUBSTRING", "SQL_TOKEN_SQL_TOKEN_INTNUM",
  "SQL_TOKEN_CURRENT_DATE", "SQL_TOKEN_CURRENT_TIMESTAMP",
  "SQL_TOKEN_CURDATE", "SQL_TOKEN_NOW", "SQL_TOKEN_EXTRACT",
  "SQL_TOKEN_HOUR", "SQL_TOKEN_MINUTE", "SQL_TOKEN_MONTH",
  "SQL_TOKEN_SECOND", "SQL_TOKEN_WEEK", "SQL_TOKEN_YEAR",
  "SQL_TOKEN_EVERY", "SQL_TOKEN_WITHIN", "SQL_TOKEN_CASE",
  "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE",
  "SQL_TOKEN_ROW", "SQL_TOKEN_VALUE", "SQL_TOKEN_CURRENT_CATALOG",
  "SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP", "SQL_TOKEN_CURRENT_PATH",
  "SQL_TOKEN_CURRENT_ROLE", "SQL_TOKEN_CURRENT_SCHEMA", "SQL_TOKEN_OVER",
  "SQL_TOKEN_ROW_NUMBER", "SQL_TOKEN_NTILE", "SQL_TOKEN_LEAD",
  "SQL_TOKEN_LAG", "SQL_TOKEN_RESPECT", "SQL_TOKEN_IGNORE",
  "SQL_TOKEN_NULLS", "SQL_TOKEN_FIRST_VALUE", "SQL_TOKEN_LAST_VALUE",
  "SQL_TOKEN_NTH_VALUE", "SQL_TOKEN_FIRST", "SQL_TOKEN_LAST",
  "SQL_TOKEN_EXCLUDE", "SQL_TOKEN_OTHERS", "SQL_TOKEN_TIES",
  "SQL_TOKEN_FOLLOWING", "SQL_TOKEN_UNBOUNDED", "SQL_TOKEN_PRECEDING",
  "SQL_TOKEN_RANGE", "SQL_TOKEN_ROWS", "SQL_TOKEN_PARTITION",
  "SQL_TOKEN_WINDOW", "SQL_TOKEN_NO", "SQL_TOKEN_LIMIT",
  "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY", "SQL_TOKEN_MATCH",
  "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT",
  "SQL_TOKEN_INT64", "SQL_TOKEN_LONG", "SQL_TOKEN_BOOLEAN",
  "SQL_TOKEN_DOUBLE", "SQL_TOKEN_REAL", "SQL_TOKEN_FLOAT",
  "SQL_TOKEN_STRING", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_DATE", "SQL_TOKEN_TIMESTAMP",
  "SQL_TOKEN_OR", "SQL_TOKEN_AND", "'|'", "'&'", "SQL_EQUAL", "SQL_GREAT",
  "SQL_LESS", "SQL_NOTEQUAL", "SQL_GREATEQ", "SQL_LESSEQ", "'+'", "'-'",
  "SQL_CONCAT", "'*'", "'/'", "'%'", "'~'", "'='",
  "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql", "column_commalist",
  "column_ref_commalist", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc", "sql_not",
  "manipulative_statement", "select_statement", "union_op",
  "commit_statement", "delete_statement_searched", "insert_statement",
  "values_or_query_spec", "row_value_constructor_commalist",
  "row_value_constructor", "row_value_constructor_elem",
  "rollback_statement", "select_statement_into", "opt_all_distinct",
  "assignment_commalist", "assignment", "update_source",
  "update_statement_searched", "target_commalist", "target",
  "opt_where_clause", "single_select_statement", "selection",
  "opt_limit_offset_clause", "opt_offset", "limit_offset_clause",
  "table_exp", "from_clause", "table_ref_commalist", "opt_as",
  "table_primary_as_range_column", "opt_only", "table_ref", "where_clause",
  "opt_group_by_clause", "opt_having_clause", "truth_value",
  "boolean_primary", "unary_predicate", "boolean_test", "boolean_factor",
  "boolean_term", "search_condition", "predicate",
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
  "value_exp_primary", "num_primary", "factor", "term", "num_value_exp",
  "datetime_primary", "datetime_value_fct", "datetime_factor",
  "datetime_term", "datetime_value_exp", "non_second_datetime_field",
  "value_exp_commalist", "function_arg", "function_args_commalist",
  "value_exp", "string_value_exp", "char_value_exp", "concatenation",
  "char_primary", "collate_clause", "char_factor", "string_value_fct",
  "bit_value_fct", "bit_substring_fct", "bit_value_exp", "bit_factor",
  "bit_primary", "char_value_fct", "for_length", "char_substring_fct",
  "upper_lower", "fold", "form_conversion", "char_translation",
  "derived_column", "table_node", "tablespace_qualified_class_name",
  "qualified_class_name", "class_name",
  "table_node_with_opt_member_func_call", "table_node_path",
  "table_node_path_entry", "member_function_call", "opt_column_array_idx",
  "property_path", "property_path_entry", "column_ref",
  "opt_collate_clause", "column", "case_expression", "case_specification",
  "simple_case", "searched_case", "simple_when_clause_list",
  "simple_when_clause", "when_operand_list", "when_operand",
  "searched_when_clause_list", "searched_when_clause", "else_clause",
  "result", "result_expression", "case_operand", "parameter",
  "range_variable", "opt_ecsqloptions_clause", "ecsqloptions_clause",
  "ecsqloptions_list", "ecsqloption", "ecsqloptionvalue", YY_NULL
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
     409,   410,   411,   412,   413,   414,   415,   124,    38,   416,
     417,   418,   419,   420,   421,    43,    45,   422,    42,    47,
      37,   126,    61,   423
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   194,   195,   196,   196,   197,   197,   198,   198,   199,
     199,   200,   200,   201,   201,   202,   202,   203,   203,   203,
     204,   204,   205,   205,   205,   205,   205,   205,   205,   206,
     206,   207,   207,   207,   208,   209,   210,   211,   212,   212,
     213,   214,   215,   216,   217,   217,   217,   218,   218,   219,
     220,   220,   221,   222,   222,   223,   224,   224,   225,   225,
     226,   226,   227,   227,   228,   228,   229,   230,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   236,
     237,   238,   238,   239,   239,   240,   240,   240,   240,   241,
     241,   241,   242,   243,   243,   244,   244,   245,   245,   246,
     246,   247,   247,   247,   247,   247,   247,   247,   247,   247,
     248,   249,   249,   250,   250,   250,   250,   250,   250,   250,
     251,   252,   253,   254,   255,   255,   255,   255,   256,   256,
     257,   258,   258,   259,   259,   260,   261,   261,   262,   263,
     264,   265,   266,   266,   266,   267,   268,   269,   270,   270,
     271,   272,   273,   273,   273,   273,   273,   273,   273,   273,
     273,   274,   274,   274,   275,   275,   276,   276,   276,   277,
     277,   278,   279,   280,   280,   280,   281,   281,   282,   282,
     283,   284,   284,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   286,   286,   286,   286,   286,   286,   287,
     288,   289,   290,   290,   291,   292,   292,   292,   292,   292,
     292,   293,   294,   295,   296,   296,   297,   297,   297,   298,
     298,   299,   300,   300,   301,   302,   303,   304,   304,   305,
     306,   306,   307,   307,   308,   309,   309,   310,   310,   311,
     312,   312,   313,   314,   314,   315,   316,   316,   317,   318,
     319,   320,   320,   321,   321,   322,   322,   323,   324,   325,
     326,   326,   327,   328,   328,   329,   330,   330,   331,   331,
     332,   332,   332,   333,   334,   335,   336,   337,   337,   337,
     338,   339,   339,   339,   339,   340,   340,   340,   341,   341,
     341,   341,   341,   341,   341,   342,   342,   342,   343,   344,
     344,   345,   345,   345,   345,   346,   347,   347,   347,   348,
     349,   349,   349,   350,   350,   351,   352,   352,   353,   354,
     355,   355,   355,   355,   355,   355,   355,   355,   355,   355,
     355,   355,   355,   355,   355,   356,   356,   357,   358,   358,
     359,   360,   360,   360,   360,   360,   360,   360,   360,   361,
     361,   362,   362,   362,   363,   363,   363,   363,   364,   364,
     364,   365,   366,   366,   366,   366,   367,   368,   369,   370,
     370,   370,   370,   370,   371,   371,   372,   373,   373,   373,
     374,   374,   374,   375,   376,   376,   377,   377,   378,   378,
     379,   380,   380,   381,   381,   382,   383,   384,   385,   386,
     387,   387,   387,   387,   388,   388,   389,   389,   390,   390,
     391,   392,   392,   393,   394,   395,   395,   396,   396,   397,
     397,   398,   399,   400,   400,   400,   401,   401,   402,   403,
     403,   404,   404,   405,   405,   406,   407,   407,   408,   409,
     410,   410,   411,   412,   413,   413,   414,   415,   415,   416,
     416,   416,   416,   416,   416,   417,   417,   418,   419,   419,
     420,   421,   422,   423,   423,   424,   424,   195,   195,   425,
     425,   426,   427,   427,   428,   428,   429,   429,   429
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     3,     1,     0,     3,     0,
       3,     0,     3,     1,     3,     2,     2,     0,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     1,     1,     2,     5,     5,     4,     1,     3,
       1,     1,     2,     6,     0,     1,     1,     1,     3,     3,
       1,     1,     6,     1,     3,     1,     0,     1,     4,     1,
       1,     1,     0,     1,     0,     2,     3,     8,     2,     1,
       3,     0,     1,     0,     3,     0,     1,     3,     4,     1,
       2,     0,     3,     0,     2,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     4,     1,     2,     1,     3,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     2,     1,     1,     1,     1,     1,     1,     2,
       5,     2,     4,     4,     2,     2,     1,     1,     0,     2,
       3,     2,     1,     1,     3,     3,     2,     1,     3,     2,
       2,     3,     1,     1,     1,     2,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     2,
       2,     0,     2,     1,     6,     4,     1,     1,     1,     4,
       4,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     5,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     0,     2,     4,     0,
       1,     6,     1,     1,     1,     1,     1,     2,     2,     5,
       1,     1,     0,     1,     8,     1,     1,     2,     2,     1,
       1,     1,     1,     0,     1,     2,     3,     1,     3,     1,
       3,     0,     1,     0,     1,     0,     1,     4,     1,     3,
       3,     1,     2,     0,     1,     3,     1,     1,     1,     1,
       2,     1,     2,     2,     4,     1,     1,     1,     2,     1,
       2,     3,     2,     2,     3,     5,     4,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     0,     1,     1,     2,     4,     5,     5,     1,     7,
       0,     1,     1,     1,     1,     4,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       6,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     2,     2,     1,     3,     3,     3,     1,     3,
       3,     1,     1,     1,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       2,     1,     2,     1,     1,     1,     7,     1,     1,     0,
       1,     1,     1,     1,     0,     2,     7,     4,     1,     1,
       4,     6,     6,     6,     2,     1,     1,     3,     3,     3,
       3,     1,     1,     1,     3,     3,     1,     1,     4,     0,
       1,     1,     3,     2,     1,     1,     0,     1,     1,     1,
       1,     1,     5,     4,     1,     2,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     4,     0,     2,
       1,     1,     1,     2,     1,     0,     2,     1,     3,     0,
       1,     2,     2,     1,     1,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      20,    20,     0,   464,   156,   153,   154,   155,    20,   429,
     293,   288,     0,     0,     0,     0,     0,     0,   185,     0,
      20,   409,   289,   290,   184,     0,     0,    44,   294,   291,
       0,   186,     0,    75,   408,     0,     0,     0,     0,     0,
       0,   362,   363,   202,   203,     0,   200,   292,     0,   187,
     188,   189,   190,   191,   192,     0,     0,   222,   223,   230,
     231,     0,   152,   388,     0,     0,   115,   116,   113,   114,
     118,   117,     0,     0,   434,     0,     0,     2,    28,    22,
      23,    24,    59,    20,    40,    25,    26,    27,    29,    93,
      90,    95,    97,    99,   467,    89,   101,     0,   102,   126,
     127,   108,   132,   106,   137,   107,   103,   109,   104,   105,
     318,   182,   166,   350,   173,   174,   175,   168,   167,   341,
     181,   342,     0,     0,     0,   199,   346,     0,   207,   208,
       0,   209,     0,   210,   193,     0,   344,   348,   349,   351,
     354,   358,   380,   366,   361,   367,   368,   382,    92,   381,
     383,   385,   391,   384,   389,   394,   395,   393,   400,     0,
     401,   402,   403,   435,   431,   343,   345,   439,   440,   441,
     183,     0,     0,     0,    41,   463,    20,    21,    96,   430,
     433,     0,    34,     0,    44,    75,     0,   145,     0,   119,
       0,    42,    45,    46,     0,     0,   146,    76,     0,   301,
     308,   314,   313,    79,     0,     0,     0,     0,   399,     0,
       0,     0,    20,   462,    41,   458,   455,     0,     0,     0,
       0,     0,   364,   365,   353,   352,     1,     0,     0,     0,
       0,   121,   124,   125,   131,   136,   139,   140,    33,    31,
      32,   316,    20,    20,    20,   112,   159,   160,   158,   157,
       0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
       0,     0,     0,     0,     0,   392,     0,     0,   468,   147,
      91,   347,    44,     0,     0,     0,   319,     0,   381,     0,
       0,    56,     0,     9,   416,   415,   130,     0,   434,     0,
      61,   148,   161,   150,   381,   426,   465,     0,    73,   422,
     423,   427,     0,   297,   302,   295,   296,   301,     0,   303,
       0,     0,    38,     0,     0,     0,   374,     0,   374,     0,
     397,   398,     0,   371,   372,   373,   370,   177,   369,   178,
       0,   176,   179,     0,     0,     0,   456,     0,    20,   458,
     444,     0,   205,   213,   215,   214,     0,   212,     0,     0,
     135,   133,   128,   128,     0,     0,   143,   142,   144,   111,
       0,   317,     0,     0,    98,   100,   195,   377,     0,   461,
     376,   460,   194,     0,     0,   251,   239,   240,   204,   241,
     242,   216,   224,     0,     0,   355,   356,   357,   359,   360,
     387,   386,   390,     0,   429,   432,     0,     0,     0,     0,
     286,     0,    20,   469,    57,     0,     0,     0,     0,   171,
      75,     0,    58,    56,     0,   438,     0,   414,   163,     0,
      72,     0,     9,     0,     0,    77,     0,     0,    75,     0,
      56,    47,     0,   304,    75,    37,     0,   172,   169,   165,
       0,     0,   407,     0,     0,   170,     0,     0,   459,   443,
       0,    40,   450,     0,   451,   126,   132,   137,     0,   447,
       0,   445,   211,     0,     0,   374,     0,   123,   122,     0,
     201,   141,   193,   138,    30,    86,    88,    85,    87,    94,
     197,     0,     0,     0,   196,   258,   253,     0,   252,     0,
       0,   219,     0,   410,     0,   334,   326,   325,   327,   328,
     322,   323,   329,   324,   330,   333,   320,   321,   331,   332,
     335,   338,   339,     0,     0,     0,   287,    80,     0,    35,
     470,   421,   418,   420,   417,   419,     0,     6,    36,    68,
      69,     0,    53,    55,   151,    81,   149,   162,     0,   466,
      78,     0,     7,   425,   424,   305,    75,     0,   469,     0,
     301,    39,   375,     0,   404,   381,     0,   457,   112,    20,
       0,   442,   236,   235,     0,   134,   129,     0,   378,   379,
     198,     0,    11,   254,   250,   225,   217,   219,     0,     0,
     229,   220,   285,     0,   337,   340,   412,   411,   474,   471,
     473,     0,     0,    10,     0,    75,     0,    43,     0,    83,
     413,   428,     0,    74,   306,    48,    52,    51,    49,    50,
      20,     0,   299,   307,   300,   164,     0,     0,     0,   180,
       0,   449,     0,   453,   454,   452,   448,   446,   232,   120,
       0,     0,   255,     0,   221,   227,   228,   336,     0,   472,
     421,     5,    70,    54,     0,    20,   243,     0,     4,   298,
       0,   310,   405,   406,   396,     0,   110,     0,   219,   233,
     259,   261,   436,    20,   267,   266,   257,   256,     0,   218,
     226,   477,   478,   476,   475,    82,    84,     0,    11,   244,
       8,     0,     0,   311,   312,   309,   237,   238,   234,     0,
     437,   262,    12,    13,    40,    17,     0,     0,     0,     0,
     263,   268,   271,   269,   249,   245,   247,     0,    62,     3,
     315,   260,    20,    18,    19,    16,    15,     0,     0,   277,
       0,   275,   279,   272,   270,   273,     0,   265,   264,     0,
       0,     0,   469,    63,    14,   278,   280,     0,     0,   282,
     283,     0,   246,   248,    64,    67,   274,   276,   281,   284,
       0,    66,    65
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    75,   647,   526,   603,   408,   632,   692,   693,   715,
      76,    77,   273,   241,    79,    80,    81,    82,   311,    83,
      84,    85,    86,   194,   430,   431,   608,    87,   531,   532,
     403,    88,   289,   732,   751,   733,   412,   413,   529,   421,
     425,   198,   199,   404,   599,   646,   479,    89,    90,    91,
      92,    93,   334,    95,   452,    96,    97,   454,    98,    99,
     100,   101,   467,   102,   103,   350,   104,   105,   236,   106,
     107,   237,   360,   108,   109,   110,   290,   291,   533,   111,
     417,   112,   113,   114,   115,   116,   117,   329,   330,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     344,   345,   346,   490,   580,   129,   130,   381,   576,   669,
     581,   131,   132,   658,   133,   564,   659,   704,   378,   379,
     678,   679,   705,   706,   707,   380,   486,   572,   666,   487,
     488,   573,   660,   661,   727,   667,   668,   700,   719,   702,
     703,   720,   746,   721,   722,   728,   134,   135,   309,   612,
     613,   310,   200,   201,   202,   685,   203,   614,   362,   136,
     275,   510,   511,   512,   513,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   331,   315,   367,   368,
     214,   149,   150,   151,   152,   265,   153,   154,   155,   156,
     319,   320,   321,   157,   617,   158,   159,   160,   161,   162,
     293,   283,   284,   285,   523,   298,   299,   300,   301,   180,
     163,   164,   165,   691,   648,   166,   167,   168,   169,   339,
     340,   458,   459,   215,   216,   337,   370,   371,   217,   170,
     422,   519,   520,   589,   590,   674
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -643
static const yytype_int16 yypact[] =
{
     584,   584,    74,  -643,  -643,  -643,  -643,  -643,   952,    81,
    -643,  -643,   110,    16,   125,   141,    95,   156,  -643,   132,
     250,  -643,  -643,  -643,  -643,   274,   203,    53,  -643,  -643,
     304,  -643,   156,   129,  -643,   356,   363,   370,   372,   380,
     384,  -643,  -643,  -643,  -643,   386,  -643,  -643,  2159,  -643,
    -643,  -643,  -643,  -643,  -643,   388,   393,  -643,  -643,  -643,
    -643,   399,  -643,  -643,  2630,  2630,  -643,  -643,  -643,  -643,
    -643,  -643,  3089,  3089,  -643,   293,    61,  -643,  -643,  -643,
    -643,  -643,  -643,    67,  -643,  -643,  -643,  -643,    27,   344,
    -643,  -643,  -643,   254,   234,  -643,  -643,  2630,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,   114,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,  -643,   413,   440,   453,  -643,  -643,   325,  -643,  -643,
     462,  -643,   467,  -643,   339,   470,  -643,  -643,  -643,  -643,
    -643,   143,   -92,  -643,  -643,  -643,  -643,  -643,   115,  -643,
     289,  -643,   436,  -643,  -643,  -643,  -643,  -643,  -643,   474,
    -643,  -643,  -643,   466,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,   476,   481,    22,    29,  -643,   768,  -643,  -643,  -643,
    -643,  2630,  -643,  2630,    14,   129,   159,  -643,   468,   408,
    2630,  -643,  -643,  -643,  2787,  2630,  -643,  -643,   106,   464,
    -643,  -643,  -643,  -643,  2630,  2630,  2630,  2630,  2630,  2630,
    1688,  2002,  1136,  -643,   301,   186,  -643,   374,   489,    38,
    2630,   301,  -643,  -643,  -643,  -643,  -643,   492,  2630,    28,
    2316,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,   469,   250,  1136,  1136,  -643,  -643,  -643,  -643,  -643,
     494,  2630,  1845,   128,  2630,  2630,    53,  2944,  2944,  2944,
    2944,  2944,  2630,    66,   468,  -643,  2630,     0,  -643,  -643,
    -643,  -643,    53,   481,    22,   475,   301,   500,   411,   505,
    2630,   243,    91,   507,  -643,  -643,  -643,    30,   233,   263,
     508,  -643,    47,  -643,   415,   511,   124,   512,   124,   269,
    -643,  -643,   446,  -643,  -643,  -643,  -643,   246,     0,   437,
     450,   353,  -643,    31,    35,   357,     6,   364,     2,   471,
    -643,  -643,    36,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
     473,  -643,   301,    39,   -42,  2630,  -643,   401,  1136,   397,
    -643,   374,  -643,   114,  -643,  -643,   521,  -643,    33,  2002,
    -643,  -643,   -21,   -11,  2630,   442,  -643,  -643,  -643,  -643,
     156,  -643,   159,   223,  -643,   254,  -643,  -643,   257,   301,
    -643,  -643,  -643,  2630,   284,   502,  -643,  -643,  -643,  -643,
    -643,   522,   301,    41,  2630,  -643,  -643,  -643,   143,   143,
    -643,  -643,  -643,    43,   509,  -643,  2787,   402,   402,   468,
    -643,   534,  1136,   379,  -643,   516,   516,     0,   444,  -643,
     129,   166,  -643,   447,  2630,  -643,   519,  -643,  -643,   468,
    -643,   523,   507,  2630,   524,  -643,   529,   529,   129,   484,
      62,  -643,   378,  -643,   129,  -643,  2630,  -643,  -643,  -643,
    2630,  2630,  -643,  2630,  2630,  -643,  2630,  2630,  -643,  -643,
      51,    69,  -643,  2630,  -643,   540,   550,   551,    71,  -643,
     455,  -643,  -643,    38,   366,   301,  2630,  -643,  -643,   383,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,  2630,  2630,   558,  -643,  -643,   425,   575,  -643,   559,
     577,   240,   578,  -643,   525,   570,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,   560,  -643,   582,   585,   587,  -643,   234,   564,  -643,
    -643,   315,  -643,  -643,  -643,  -643,   394,  -643,  -643,   588,
     341,    80,  -643,  -643,  -643,   533,  -643,  -643,   591,  -643,
    -643,   320,   593,  -643,  -643,   341,   129,     0,   379,  2473,
     283,  -643,   301,    44,    -2,    88,    46,  -643,    77,  1504,
    2630,  -643,  -643,  -643,   594,  -643,   412,  2630,  -643,  -643,
    -643,   563,   520,  -643,  -643,  -643,   597,   240,   477,   480,
    -643,  -643,  -643,   586,  -643,  -643,  -643,  -643,   430,   564,
    -643,   596,   596,  -643,     0,   129,   166,  -643,   590,   561,
    -643,  -643,   519,  -643,   341,  -643,  -643,  -643,  -643,   301,
    1136,   235,  -643,  -643,  -643,  -643,  2630,   608,   617,  -643,
      86,  -643,  2630,  -643,  -643,  -643,  -643,  -643,   569,  -643,
       0,   592,   268,  2630,  -643,  -643,  -643,  -643,    32,  -643,
    -643,  -643,   341,  -643,  2630,  1136,   479,   417,  -643,   234,
     519,   376,   301,  -643,  -643,  2630,  -643,   290,   240,  -643,
     625,  -643,   436,  1320,  -643,  -643,  -643,  -643,   332,  -643,
     301,  -643,  -643,   114,  -643,   626,   234,   502,   520,  -643,
    -643,   519,   433,  -643,  -643,  -643,  -643,  -643,  -643,     0,
    -643,  -643,   630,  -643,   280,    63,   373,   513,   486,   487,
     496,  -643,  -643,  -643,  -643,   637,  -643,   612,   488,  -643,
    -643,  -643,  1320,  -643,  -643,  -643,  -643,    89,   187,  -643,
     478,  -643,  -643,  -643,  -643,  -643,    26,  -643,  -643,   502,
     644,  2944,   379,  -643,  -643,  -643,  -643,   373,   526,  -643,
    -643,   503,  -643,  -643,   -84,  -643,  -643,  -643,  -643,  -643,
    2944,  -643,   -92
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -643,   652,     7,  -643,  -643,   239,   -23,  -643,   -47,   -29,
     -15,  -643,    20,  -643,  -643,  -643,  -643,   261,  -643,   -89,
     -48,  -643,  -643,  -109,  -643,   131,  -643,  -643,  -643,    75,
    -330,  -643,   276,  -643,  -643,  -643,   145,  -643,  -643,   385,
    -643,  -643,  -156,  -643,  -643,  -643,    58,  -643,  -643,   673,
     441,   454,     3,  -601,  -643,  -643,   -72,   602,  -643,   -70,
     614,  -643,   346,   -69,  -643,  -643,   -68,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,     5,  -643,   287,  -643,  -203,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -591,  -643,   348,  -643,  -643,  -186,  -643,  -643,  -643,  -643,
     242,   244,  -643,  -643,  -505,  -643,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -217,  -643,  -643,
    -643,  -643,  -643,   -20,  -643,   -24,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,    19,  -643,  -643,  -643,  -643,    48,  -643,
    -643,  -643,  -643,   -16,  -643,  -643,   367,  -643,  -643,  -643,
    -643,   416,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
     544,  -643,  -643,  -643,   330,  -643,   501,   377,    87,   192,
    -642,  -643,  -643,  -643,  -643,  -643,  -643,  -198,  -257,  -222,
       1,   -58,  -643,  -643,  -643,    68,   472,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,   176,  -643,  -643,  -643,  -643,  -643,
    -643,  -239,  -643,    49,  -378,   121,  -643,    42,  -643,  -643,
    -643,   482,  -289,  -643,  -274,  -643,  -643,  -643,  -643,  -643,
     392,  -643,   175,   527,  -166,   398,  -318,  -643,  -643,  -192,
    -643,  -517,  -643,  -643,   147,  -643
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -455
static const yytype_int16 yytable[] =
{
     213,   148,   174,    94,   173,   189,   222,   223,   245,   148,
     317,   230,   297,   232,   234,   235,   343,   448,   418,   432,
      78,   172,   187,   401,   394,   392,   270,   347,   525,   281,
     374,   606,   466,   271,   409,   437,   377,   196,   463,   438,
     445,   192,   466,   271,     2,   491,     3,   493,   615,   336,
     619,     4,     5,     6,     7,   616,   671,     4,     5,     6,
       7,   443,   695,   354,   193,   221,   221,   547,   229,   441,
     738,   415,   634,   750,  -449,   280,   559,   699,   416,   447,
     192,   238,  -110,   535,  -201,   596,   354,   739,   475,   744,
     177,   227,  -404,   260,   261,   713,   239,   405,   175,   228,
     548,   260,   261,   193,   406,   718,   179,    14,   752,   186,
     476,   695,   714,   181,   227,   312,   483,   182,   527,   240,
     -41,   354,   228,   477,   227,   278,   478,   492,   183,   557,
     295,   375,   228,   244,   246,    20,   247,   294,   -41,   410,
      21,   359,   537,   373,   184,   616,   718,   384,   -71,   227,
     -41,   464,   376,   688,   185,   420,    30,   655,   485,   186,
     515,   402,    34,   396,  -349,  -349,  -349,  -349,  -349,  -349,
     353,    39,     2,   740,     3,   336,  -381,   174,   -41,   274,
     538,   741,   276,   -41,   276,   262,   -41,   355,    74,   262,
    -449,   287,   560,   262,    62,   292,   221,   244,  -110,   188,
      62,   541,   279,   296,   -92,   -92,   313,   314,   316,   318,
     322,   332,   333,   148,   525,   745,   262,   262,   262,   534,
     262,   348,   262,   262,   568,   569,   262,   363,   262,   221,
     262,   262,   351,   262,   262,    63,   -41,   735,   650,   724,
     297,   297,   627,   272,   148,   148,    66,    67,    68,    69,
      70,    71,   369,   369,   530,   382,   383,    35,   432,   295,
     343,   480,   481,   390,   482,   469,   453,   393,   455,   456,
     457,   347,   545,   177,   -41,   426,   248,   190,   550,   475,
     -17,   369,   427,   249,   -17,   -17,   302,   197,   484,   481,
     451,   482,   -60,   226,   -41,   -41,   -41,   -41,   -41,   -41,
     -60,   476,   262,   303,   191,   641,   303,   195,   304,   212,
     335,   304,   713,  -301,   477,   305,   306,   478,   305,   306,
     307,   591,   410,   450,   601,   481,   302,   482,   592,   714,
     411,   257,   258,   259,   -17,   736,   369,   725,     2,   148,
       3,   662,   402,   303,   385,   386,   387,   551,   304,   -17,
     465,     4,     5,     6,     7,   305,   306,   435,   436,   204,
     307,   439,   440,   610,   558,   473,   205,   696,   442,   440,
     565,   440,   -17,   206,   369,   207,   697,   578,   579,     2,
     611,     3,   474,   208,   302,   369,   555,   209,    18,   210,
     604,   218,     4,     5,     6,     7,   219,   292,   593,   594,
     662,   303,   220,   148,   534,   517,   304,   709,   566,   244,
      24,  -301,   242,   305,   306,   292,   250,   697,   307,   664,
     665,   680,   681,    31,   369,   297,   495,   683,   684,    18,
     243,   -17,   -17,   686,   687,   673,   -17,   710,   681,   642,
     -17,   552,   553,   251,   554,   221,   675,   556,   369,   224,
     225,    24,   388,   389,   522,   524,   252,   253,    49,    50,
      51,    52,    53,    54,    31,   254,   470,   221,   543,   544,
     255,  -206,    10,   256,   263,   264,    11,   266,   629,   267,
     268,   698,   369,   369,    15,   269,   286,   622,   262,   623,
     624,   625,   282,   342,    62,   349,   361,   338,   366,    49,
      50,    51,    52,    53,    54,   398,   397,   302,   399,   400,
     407,   621,   419,   414,  -201,   423,   428,    22,    23,   433,
     434,   335,   717,   449,   303,   462,   376,   489,    28,   304,
     444,    29,   446,   656,   179,    62,   305,   306,   516,   518,
     521,   307,    35,   415,   620,  -453,   402,   539,   542,   308,
     609,    43,    44,   295,   546,  -454,  -452,   549,    46,   567,
      47,   369,   570,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   509,   561,   571,   574,
     575,   577,   582,   583,   410,   584,   585,     1,   588,   586,
       2,   587,     3,   595,   598,   600,   602,   353,   628,  -381,
     630,   631,   633,     4,     5,     6,     7,     8,     9,   638,
     637,   148,   653,   649,    10,   694,   635,   652,    11,   636,
     640,   654,    12,   645,    13,    14,    15,   644,   657,   663,
     689,   440,    16,   677,   670,   712,   724,   725,   723,    17,
      18,   726,   729,   730,   731,   465,   148,   375,   676,   749,
      19,   748,    20,   171,   737,   708,   221,   682,    21,    22,
      23,   540,    24,    25,   694,   734,   716,    26,    27,   528,
      28,   643,   494,    29,    30,    31,   597,    32,   605,    33,
      34,   178,    35,   424,   364,   231,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,   672,   233,   365,   468,
      46,   536,    47,   471,    48,   562,   743,   563,   711,   742,
      49,    50,    51,    52,    53,    54,   701,    55,    56,    57,
      58,   747,   472,   429,    59,    60,    61,   277,   514,   352,
     690,   618,   651,   461,   626,   391,   639,   460,     0,     0,
       0,     0,     0,     0,   341,     0,    62,     0,     0,   395,
       0,     0,     0,    63,     0,     0,     0,    64,    65,     0,
       0,     0,     0,    66,    67,    68,    69,    70,    71,    72,
      73,   176,    74,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,    21,    22,    23,     0,    24,    25,     0,     0,
       0,     0,   272,     0,    28,     0,     0,    29,    30,    31,
       0,    32,     0,     0,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       0,     0,     0,     0,    46,     0,    47,     0,    48,     0,
       0,     0,     0,     0,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,     0,     0,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,     0,     0,     0,    63,     0,     0,
       0,    64,    65,     0,     0,     0,     0,    66,    67,    68,
      69,    70,    71,    72,    73,   176,    74,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,   177,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    11,   -21,     0,     0,
      12,     0,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,    21,    22,    23,     0,
      24,    25,     0,     0,     0,     0,     0,     0,    28,     0,
       0,    29,    30,    31,     0,    32,     0,     0,    34,     0,
       0,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,     0,     0,     0,    46,     0,
      47,     0,    48,     0,     0,     0,     0,     0,    49,    50,
      51,    52,    53,    54,     0,    55,    56,    57,    58,     0,
       0,     0,    59,    60,    61,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    62,     0,     0,     0,     0,     0,
       0,    63,     0,     0,     0,    64,    65,     0,     0,     0,
       0,    66,    67,    68,    69,    70,    71,    72,    73,   176,
      74,     0,     2,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
      11,     0,     0,     0,    12,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    20,     0,     0,     0,     0,     0,
      21,    22,    23,     0,    24,    25,     0,     0,     0,     0,
       0,     0,    28,     0,     0,    29,    30,    31,     0,    32,
       0,     0,    34,     0,     0,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,     0,     0,
       0,     0,    46,     0,    47,     0,    48,     0,     0,     0,
       0,     0,    49,    50,    51,    52,    53,    54,     0,    55,
      56,    57,    58,     0,     0,     0,    59,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,     0,     0,     0,    63,     0,     0,     0,    64,
      65,     0,     0,     0,     0,    66,    67,    68,    69,    70,
      71,    72,    73,   211,    74,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,   177,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    11,     0,     0,     0,    12,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,     0,     0,     0,    21,    22,    23,     0,    24,    25,
       0,     0,     0,     0,     0,     0,    28,     0,     0,    29,
      30,    31,     0,    32,     0,     0,    34,     0,     0,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,     0,     0,    46,     0,    47,     0,
      48,     0,     0,     0,     0,     0,    49,    50,    51,    52,
      53,    54,     0,    55,    56,    57,    58,     0,     0,     0,
      59,    60,    61,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    62,     0,     0,     0,     0,     0,     0,    63,
       0,     0,     0,    64,    65,     0,     0,     0,     0,    66,
      67,    68,    69,    70,    71,    72,    73,   211,    74,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,   177,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,    11,     0,
       0,     0,    12,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
      28,     0,     0,    29,    30,    31,     0,     0,     0,     0,
      34,     0,     0,     0,     0,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     0,     0,     0,     0,
      46,     0,    47,     0,    48,     0,     0,     0,     0,     0,
      49,    50,    51,    52,    53,    54,     0,    55,    56,    57,
      58,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,     0,
       0,     0,     0,    63,     0,     0,     0,    64,    65,     0,
       0,     0,     0,    66,    67,    68,    69,    70,    71,    72,
      73,   211,    74,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     0,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,    14,
      15,     0,     0,     0,   323,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    28,     0,     0,    29,    30,    31,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
     324,   325,   326,   327,    46,   328,    47,     0,    48,     0,
       0,     0,     0,     0,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,     0,     0,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   211,   372,
      62,     2,     0,     3,     0,     0,     0,    63,     0,     0,
       0,    64,    65,     0,     4,     5,     6,     7,     0,     9,
       0,     0,   192,    72,    73,    10,    74,     0,     0,    11,
       0,     0,     0,    12,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
      22,    23,     0,    24,    25,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,    55,    56,
      57,    58,     0,     0,     0,    59,    60,    61,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   211,     0,    62,     2,     0,
       3,     0,     0,     0,    63,     0,     0,     0,    64,    65,
       0,     4,     5,     6,     7,     0,     9,     0,     0,     0,
      72,    73,    10,    74,     0,     0,    11,     0,     0,     0,
      12,     0,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,     0,
      24,    25,     0,     0,     0,     0,   272,     0,    28,     0,
       0,    29,    30,    31,     0,     0,     0,     0,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,     0,     0,     0,    46,     0,
      47,     0,    48,     0,     0,     0,     0,     0,    49,    50,
      51,    52,    53,    54,     0,    55,    56,    57,    58,     0,
       0,     0,    59,    60,    61,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,     0,    62,     2,     0,     3,     0,     0,
       0,    63,     0,     0,     0,    64,    65,     0,     4,     5,
       6,     7,     0,     9,     0,     0,     0,    72,    73,    10,
      74,     0,     0,    11,     0,     0,     0,    12,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,    22,    23,     0,    24,    25,     0,
       0,     0,     0,     0,     0,    28,     0,     0,    29,    30,
      31,     0,     0,     0,     0,    34,     0,     0,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,     0,     0,     0,    46,     0,    47,     0,    48,
       0,     0,   212,     0,     0,    49,    50,    51,    52,    53,
      54,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
       0,    62,     2,     0,     3,     0,     0,     0,    63,     0,
       0,     0,    64,    65,     0,     4,     5,     6,     7,     0,
       9,     0,     0,   356,    72,    73,   357,    74,     0,     0,
      11,     0,     0,     0,    12,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,    22,    23,     0,    24,    25,     0,     0,     0,     0,
       0,     0,   358,     0,     0,    29,    30,    31,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,     0,     0,
       0,     0,    46,     0,    47,     0,    48,     0,     0,     0,
       0,     0,    49,    50,    51,    52,    53,    54,     0,    55,
      56,    57,    58,     0,     0,     0,    59,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   211,     0,    62,     2,
       0,     3,     0,     0,     0,    63,     0,     0,     0,    64,
      65,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
     607,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    22,    23,
       0,    24,    25,     0,     0,     0,     0,     0,     0,    28,
       0,     0,    29,    30,    31,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,     0,     0,     0,    46,
       0,    47,     0,    48,     0,     0,     0,     0,     0,    49,
      50,    51,    52,    53,    54,     0,    55,    56,    57,    58,
       0,     0,     0,    59,    60,    61,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   211,     0,    62,     2,     0,     3,     0,
       0,     0,    63,     0,     0,     0,    64,    65,     0,     4,
       5,     6,     7,     0,     9,     0,     0,     0,    72,    73,
      10,    74,     0,     0,    11,     0,     0,     0,    12,     0,
       0,    14,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,    22,    23,     0,    24,    25,
       0,     0,     0,     0,     0,     0,    28,     0,     0,    29,
      30,    31,     0,     0,     0,     0,    34,     0,     0,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,     0,     0,    46,     0,    47,     0,
      48,     0,     0,     0,     0,     0,    49,    50,    51,    52,
      53,    54,     0,    55,    56,    57,    58,     0,     0,     0,
      59,    60,    61,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     211,     0,    62,     2,     0,     3,     0,     0,     0,    63,
       0,     0,     0,    64,    65,     0,     4,     5,     6,     7,
       0,     9,     0,     0,     0,    72,    73,    10,    74,     0,
       0,    11,     0,     0,     0,    12,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,    22,    23,     0,    24,    25,     0,     0,     0,
       0,     0,     0,    28,     0,     0,    29,    30,    31,     0,
       0,     0,     0,    34,     0,     0,     0,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,     0,     0,    46,     0,    47,     0,    48,     0,     0,
       0,     0,     0,    49,    50,    51,    52,    53,    54,     0,
      55,    56,    57,    58,     0,     0,     0,    59,    60,    61,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   211,     0,    62,
       2,     0,     3,     0,     0,     0,    63,     0,     0,     0,
      64,    65,     0,     4,     5,     6,     7,     0,     9,     0,
       0,     0,    72,    73,    10,   288,     0,     0,    11,     0,
       0,     0,    12,     0,     0,     0,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
      28,     0,     0,    29,     0,    31,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,    37,    38,     0,
      40,     0,     0,    43,    44,    45,     0,     0,     0,     0,
      46,     0,    47,     0,    48,     0,     0,     0,     0,     0,
      49,    50,    51,    52,    53,    54,     0,    55,    56,    57,
      58,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,   211,     0,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     4,     5,
       6,     7,     0,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    11,     0,     0,     0,    12,     0,    72,
      73,    15,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,    23,     0,    24,    25,     0,
       0,     0,     0,     0,     0,    28,     0,     0,    29,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,    37,    38,     0,    40,     0,     0,    43,    44,
      45,     0,     0,     0,     0,    46,     0,    47,     0,    48,
       0,     0,     0,     0,     0,    49,    50,    51,    52,    53,
      54,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-643)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      48,     0,     1,     0,     1,    20,    64,    65,    97,     8,
     208,    83,   198,    83,    83,    83,   219,   335,   292,   308,
       0,     1,    17,   280,    24,   264,     4,   219,   406,   185,
     252,   548,    53,     4,     4,     4,   253,    32,     5,     4,
       4,    27,    53,     4,     6,     4,     8,     4,     4,   215,
       4,    19,    20,    21,    22,    57,    24,    19,    20,    21,
      22,    59,   663,    35,    50,    64,    65,     5,    83,    63,
      44,    24,   577,   157,     5,   184,     5,   668,    31,   121,
      27,    54,     5,   413,     3,     5,    35,    61,    56,   731,
      23,    63,     4,   185,   186,    32,    69,     6,    24,    71,
     430,   185,   186,    50,    13,   696,    25,    41,   750,     3,
      78,   712,    49,     3,    63,   204,   373,   101,   407,    92,
       5,    35,    71,    91,    63,   183,    94,   384,     3,   447,
      24,     3,    71,   175,    20,    68,    22,   195,    23,    59,
      74,   230,   416,   252,     3,    57,   737,   256,    24,    63,
      35,   349,    24,   658,    59,    31,    90,    71,   375,     3,
     399,    99,    96,   272,   185,   186,   187,   188,   189,   190,
     228,   105,     6,   147,     8,   341,   187,   176,    63,   176,
     419,   155,   181,    68,   183,   187,    71,   159,   188,   187,
     121,   190,   121,   187,   162,   194,   195,   175,   121,    67,
     162,   423,   188,   198,   175,   176,   205,   206,   207,   208,
     209,   210,   211,   212,   592,   732,   187,   187,   187,   411,
     187,   220,   187,   187,   481,   482,   187,   242,   187,   228,
     187,   187,   227,   187,   187,   169,   121,   148,     3,   150,
     426,   427,   560,    84,   243,   244,   179,   180,   181,   182,
     183,   184,   251,   252,   410,   254,   255,    98,   547,    24,
     463,     4,     5,   262,     7,   354,   338,   266,   338,   338,
     338,   463,   428,    23,   159,     6,   162,     3,   434,    56,
       0,   280,    13,   169,     4,     5,    43,   158,     4,     5,
     338,     7,    59,     0,   179,   180,   181,   182,   183,   184,
      67,    78,   187,    60,   101,   594,    60,     3,    65,   123,
     124,    65,    32,    70,    91,    72,    73,    94,    72,    73,
      77,     6,    59,   338,     4,     5,    43,     7,    13,    49,
      67,   188,   189,   190,    54,   148,   335,   150,     6,   338,
       8,   630,    99,    60,   257,   258,   259,   436,    65,    69,
     349,    19,    20,    21,    22,    72,    73,     4,     5,     3,
      77,     4,     5,    80,   453,   360,     3,    35,     4,     5,
       4,     5,    92,     3,   373,     3,    44,   137,   138,     6,
      97,     8,   362,     3,    43,   384,   444,     3,    56,     3,
     546,     3,    19,    20,    21,    22,     3,   396,     4,     5,
     689,    60,     3,   402,   596,   402,    65,   681,   466,   175,
      78,    70,    68,    72,    73,   414,     3,    44,    77,   151,
     152,     4,     5,    91,   423,   611,    24,    51,    52,    56,
     176,   151,   152,   143,   144,   638,   156,     4,     5,   595,
     160,   440,   441,     3,   443,   444,   644,   446,   447,    72,
      73,    78,   260,   261,   405,   406,     3,   132,   126,   127,
     128,   129,   130,   131,    91,     3,    24,   466,   426,   427,
       3,   132,    30,     3,   185,    39,    34,     3,   567,    13,
       4,   149,   481,   482,    42,     4,    78,   559,   187,   559,
     559,   559,    24,     4,   162,     3,    27,   123,     4,   126,
     127,   128,   129,   130,   131,     5,    31,    43,    97,     4,
       3,   559,    97,     5,     3,     3,    70,    75,    76,    82,
      70,   124,   149,   122,    60,     4,    24,     5,    86,    65,
      59,    89,    59,   622,    25,   162,    72,    73,     4,   160,
      24,    77,    98,    24,   559,     5,    99,    24,    24,    85,
     549,   109,   110,    24,    70,     5,     5,   179,   116,   176,
     118,   560,     4,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   122,   153,     4,
      21,     4,     4,    13,    59,    25,     4,     3,    24,     4,
       6,     4,     8,     5,    61,     4,     3,   655,     4,   187,
      37,    81,     5,    19,    20,    21,    22,    23,    24,   179,
      24,   610,     4,   610,    30,   663,   139,   616,    34,   139,
      24,     4,    38,    62,    40,    41,    42,    37,    59,    37,
       5,     5,    48,   154,   633,     5,   150,   150,   125,    55,
      56,   145,     5,    31,   156,   644,   645,     3,   645,   146,
      66,   125,    68,     1,   176,   678,   655,   650,    74,    75,
      76,   422,    78,    79,   712,   712,   695,    83,    84,   408,
      86,   596,   396,    89,    90,    91,   531,    93,   547,    95,
      96,     8,    98,   298,   243,    83,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   638,    83,   244,   353,
     116,   414,   118,   355,   120,   463,   730,   463,   689,   729,
     126,   127,   128,   129,   130,   131,   668,   133,   134,   135,
     136,   737,   355,   307,   140,   141,   142,   183,   398,   228,
     662,   555,   611,   341,   559,   263,   589,   339,    -1,    -1,
      -1,    -1,    -1,    -1,   217,    -1,   162,    -1,    -1,   267,
      -1,    -1,    -1,   169,    -1,    -1,    -1,   173,   174,    -1,
      -1,    -1,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,     3,   188,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    -1,    78,    79,    -1,    -1,
      -1,    -1,    84,    -1,    86,    -1,    -1,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,    -1,    -1,    -1,   116,    -1,   118,    -1,   120,    -1,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
      -1,   133,   134,   135,   136,    -1,    -1,    -1,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,   173,   174,    -1,    -1,    -1,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,     3,   188,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    34,    35,    -1,    -1,
      38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    -1,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    89,    90,    91,    -1,    93,    -1,    -1,    96,    -1,
      -1,    -1,    -1,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,    -1,    -1,    -1,   116,    -1,
     118,    -1,   120,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,    -1,   133,   134,   135,   136,    -1,
      -1,    -1,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,     3,
     188,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    -1,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    89,    90,    91,    -1,    93,
      -1,    -1,    96,    -1,    -1,    -1,    -1,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,    -1,
      -1,    -1,   116,    -1,   118,    -1,   120,    -1,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,   130,   131,    -1,   133,
     134,   135,   136,    -1,    -1,    -1,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,     3,   188,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,
      -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    -1,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    89,
      90,    91,    -1,    93,    -1,    -1,    96,    -1,    -1,    -1,
      -1,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,    -1,    -1,    -1,   116,    -1,   118,    -1,
     120,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,    -1,   133,   134,   135,   136,    -1,    -1,    -1,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,     3,   188,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    89,    90,    91,    -1,    -1,    -1,    -1,
      96,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    -1,    -1,    -1,    -1,
     116,    -1,   118,    -1,   120,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,   130,   131,    -1,   133,   134,   135,
     136,    -1,    -1,    -1,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,   173,   174,    -1,
      -1,    -1,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,     3,   188,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    41,
      42,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    89,    90,    91,
      -1,    -1,    -1,    -1,    96,    -1,    -1,    -1,    -1,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,    -1,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
      -1,   133,   134,   135,   136,    -1,    -1,    -1,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
     162,     6,    -1,     8,    -1,    -1,    -1,   169,    -1,    -1,
      -1,   173,   174,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    27,   185,   186,    30,   188,    -1,    -1,    34,
      -1,    -1,    -1,    38,    -1,    -1,    41,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      75,    76,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    89,    90,    91,    -1,    -1,    -1,
      -1,    96,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,    -1,    -1,
      -1,   116,    -1,   118,    -1,   120,    -1,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,   130,   131,    -1,   133,   134,
     135,   136,    -1,    -1,    -1,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,    -1,   162,     6,    -1,
       8,    -1,    -1,    -1,   169,    -1,    -1,    -1,   173,   174,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
     185,   186,    30,   188,    -1,    -1,    34,    -1,    -1,    -1,
      38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    -1,
      78,    79,    -1,    -1,    -1,    -1,    84,    -1,    86,    -1,
      -1,    89,    90,    91,    -1,    -1,    -1,    -1,    96,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,    -1,    -1,    -1,   116,    -1,
     118,    -1,   120,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,    -1,   133,   134,   135,   136,    -1,
      -1,    -1,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,   162,     6,    -1,     8,    -1,    -1,
      -1,   169,    -1,    -1,    -1,   173,   174,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,   185,   186,    30,
     188,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,
      41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    -1,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    89,    90,
      91,    -1,    -1,    -1,    -1,    96,    -1,    -1,    -1,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,    -1,    -1,    -1,   116,    -1,   118,    -1,   120,
      -1,    -1,   123,    -1,    -1,   126,   127,   128,   129,   130,
     131,    -1,   133,   134,   135,   136,    -1,    -1,    -1,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
      -1,   162,     6,    -1,     8,    -1,    -1,    -1,   169,    -1,
      -1,    -1,   173,   174,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    27,   185,   186,    30,   188,    -1,    -1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    -1,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    89,    90,    91,    -1,    -1,
      -1,    -1,    96,    -1,    -1,    -1,    -1,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,    -1,
      -1,    -1,   116,    -1,   118,    -1,   120,    -1,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,   130,   131,    -1,   133,
     134,   135,   136,    -1,    -1,    -1,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,   162,     6,
      -1,     8,    -1,    -1,    -1,   169,    -1,    -1,    -1,   173,
     174,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   185,   186,    30,   188,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    89,    90,    91,    -1,    -1,    -1,    -1,    96,
      -1,    -1,    -1,    -1,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    -1,    -1,    -1,    -1,   116,
      -1,   118,    -1,   120,    -1,    -1,    -1,    -1,    -1,   126,
     127,   128,   129,   130,   131,    -1,   133,   134,   135,   136,
      -1,    -1,    -1,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,    -1,   162,     6,    -1,     8,    -1,
      -1,    -1,   169,    -1,    -1,    -1,   173,   174,    -1,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,   185,   186,
      30,   188,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,
      -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    -1,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    89,
      90,    91,    -1,    -1,    -1,    -1,    96,    -1,    -1,    -1,
      -1,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,    -1,    -1,    -1,   116,    -1,   118,    -1,
     120,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,    -1,   133,   134,   135,   136,    -1,    -1,    -1,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    -1,   162,     6,    -1,     8,    -1,    -1,    -1,   169,
      -1,    -1,    -1,   173,   174,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,   185,   186,    30,   188,    -1,
      -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    41,    42,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    -1,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    89,    90,    91,    -1,
      -1,    -1,    -1,    96,    -1,    -1,    -1,    -1,    -1,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    -1,
      -1,    -1,    -1,   116,    -1,   118,    -1,   120,    -1,    -1,
      -1,    -1,    -1,   126,   127,   128,   129,   130,   131,    -1,
     133,   134,   135,   136,    -1,    -1,    -1,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,   162,
       6,    -1,     8,    -1,    -1,    -1,   169,    -1,    -1,    -1,
     173,   174,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,   185,   186,    30,   188,    -1,    -1,    34,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    89,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,    -1,
     106,    -1,    -1,   109,   110,   111,    -1,    -1,    -1,    -1,
     116,    -1,   118,    -1,   120,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,   130,   131,    -1,   133,   134,   135,
     136,    -1,    -1,    -1,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,   185,
     186,    42,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    -1,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    89,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,   103,   104,    -1,   106,    -1,    -1,   109,   110,
     111,    -1,    -1,    -1,    -1,   116,    -1,   118,    -1,   120,
      -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,    -1,   133,   134,   135,   136,    -1,    -1,    -1,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    34,    38,    40,    41,    42,    48,    55,    56,    66,
      68,    74,    75,    76,    78,    79,    83,    84,    86,    89,
      90,    91,    93,    95,    96,    98,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   116,   118,   120,   126,
     127,   128,   129,   130,   131,   133,   134,   135,   136,   140,
     141,   142,   162,   169,   173,   174,   179,   180,   181,   182,
     183,   184,   185,   186,   188,   195,   204,   205,   206,   208,
     209,   210,   211,   213,   214,   215,   216,   221,   225,   241,
     242,   243,   244,   245,   246,   247,   249,   250,   252,   253,
     254,   255,   257,   258,   260,   261,   263,   264,   267,   268,
     269,   273,   275,   276,   277,   278,   279,   280,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   299,
     300,   305,   306,   308,   340,   341,   353,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   374,   375,
     376,   377,   378,   380,   381,   382,   383,   387,   389,   390,
     391,   392,   393,   404,   405,   406,   409,   410,   411,   412,
     423,   195,   206,   246,   374,    24,     3,    23,   243,    25,
     403,     3,   101,     3,     3,    59,     3,   269,    67,   204,
       3,   101,    27,    50,   217,     3,   269,   158,   235,   236,
     346,   347,   348,   350,     3,     3,     3,     3,     3,     3,
       3,     3,   123,   214,   374,   417,   418,   422,     3,     3,
       3,   374,   375,   375,   361,   361,     0,    63,    71,   204,
     250,   251,   253,   254,   257,   260,   262,   265,    54,    69,
      92,   207,    68,   176,   175,   213,    20,    22,   162,   169,
       3,     3,     3,   132,     3,     3,     3,   188,   189,   190,
     185,   186,   187,   185,    39,   379,     3,    13,     4,     4,
       4,     4,    84,   206,   246,   354,   374,   354,   375,   188,
     217,   236,    24,   395,   396,   397,    78,   374,   188,   226,
     270,   271,   374,   394,   375,    24,   269,   289,   399,   400,
     401,   402,    43,    60,    65,    72,    73,    77,    85,   342,
     345,   212,   213,   374,   374,   371,   374,   371,   374,   384,
     385,   386,   374,    46,   112,   113,   114,   115,   117,   281,
     282,   370,   374,   374,   246,   124,   418,   419,   123,   413,
     414,   417,     4,   273,   294,   295,   296,   423,   374,     3,
     259,   269,   360,   375,    35,   159,    27,    30,    86,   213,
     266,    27,   352,   204,   244,   245,     4,   372,   373,   374,
     420,   421,     4,   217,   373,     3,    24,   311,   312,   313,
     319,   301,   374,   374,   217,   362,   362,   362,   363,   363,
     374,   380,   395,   374,    24,   405,   217,    31,     5,    97,
       4,   372,    99,   224,   237,     6,    13,     3,   199,     4,
      59,    67,   230,   231,     5,    24,    31,   274,   408,    97,
      31,   233,   424,     3,   233,   234,     6,    13,    70,   345,
     218,   219,   406,    82,    70,     4,     5,     4,     4,     4,
       5,    63,     4,    59,    59,     4,    59,   121,   420,   122,
     204,   214,   248,   250,   251,   253,   257,   260,   415,   416,
     419,   414,     4,     5,   371,   374,    53,   256,   256,   213,
      24,   286,   340,   269,   206,    56,    78,    91,    94,   240,
       4,     5,     7,   372,     4,   311,   320,   323,   324,     5,
     297,     4,   372,     4,   226,    24,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     355,   356,   357,   358,   358,   395,     4,   246,   160,   425,
     426,    24,   397,   398,   397,   398,   197,   406,   211,   232,
     236,   222,   223,   272,   423,   224,   271,   408,   395,    24,
     199,   373,    24,   401,   401,   236,    70,     5,   224,   179,
     236,   213,   374,   374,   374,   375,   374,   420,   213,     5,
     121,   122,   294,   295,   309,     4,   375,   176,   372,   372,
       4,   153,   321,   325,     4,    21,   302,     4,   137,   138,
     298,   304,     4,    13,    25,     4,     4,     4,    24,   427,
     428,     6,    13,     4,     5,     5,     5,   230,    61,   238,
       4,     4,     3,   198,   236,   219,   425,    47,   220,   374,
      80,    97,   343,   344,   351,     4,    57,   388,   388,     4,
     204,   214,   250,   253,   257,   260,   416,   420,     4,   213,
      37,    81,   200,     5,   298,   139,   139,    24,   179,   428,
      24,   406,   236,   223,    37,    62,   239,   196,   408,   246,
       3,   399,   374,     4,     4,    71,   213,    59,   307,   310,
     326,   327,   406,    37,   151,   152,   322,   329,   330,   303,
     374,    24,   240,   273,   429,   371,   246,   154,   314,   315,
       4,     5,   196,    51,    52,   349,   143,   144,   298,     5,
     379,   407,   201,   202,   214,   247,    35,    44,   149,   284,
     331,   332,   333,   334,   311,   316,   317,   318,   200,   408,
       4,   327,     5,    32,    49,   203,   203,   149,   284,   332,
     335,   337,   338,   125,   150,   150,   145,   328,   339,     5,
      31,   156,   227,   229,   202,   148,   148,   176,    44,    61,
     147,   155,   317,   319,   364,   425,   336,   337,   125,   146,
     157,   228,   364
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
      yyerror (context, YY_("syntax error: cannot back up")); \
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
		  Type, Value, context); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, connectivity::OSQLParser* context)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, context)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    connectivity::OSQLParser* context;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (context);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, connectivity::OSQLParser* context)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, context)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    connectivity::OSQLParser* context;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, context);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, connectivity::OSQLParser* context)
#else
static void
yy_reduce_print (yyvsp, yyrule, context)
    YYSTYPE *yyvsp;
    int yyrule;
    connectivity::OSQLParser* context;
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
		       		       , context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, context); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, connectivity::OSQLParser* context)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, context)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    connectivity::OSQLParser* context;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (context);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




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
yyparse (connectivity::OSQLParser* context)
#else
int
yyparse (context)
    connectivity::OSQLParser* context;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
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
        case 3:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 4:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 5:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 6:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 7:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 8:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 9:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 10:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 11:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 12:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 13:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 14:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 15:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 16:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 17:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 20:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 29:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 30:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 34:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 35:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
            }
    break;

  case 36:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));}
    break;

  case 37:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 38:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 39:

    {    
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 42:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 43:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode)); }
    break;

  case 44:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 47:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 48:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 49:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 52:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
            }
    break;

  case 53:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 54:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 56:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 58:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 60:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 62:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 64:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 65:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 66:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 67:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (8)].pParseNode));
        }
    break;

  case 68:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 69:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 70:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 71:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 73:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 74:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 75:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 77:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 78:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 80:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 81:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 82:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 83:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 84:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 91:

    { // boolean_primary: rule 2
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 92:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
    }
    break;

  case 94:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 96:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 98:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 100:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 110:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 111:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 112:

    {
            if(context->inPredicateCheck()) // comparison_predicate: rule 2
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                sal_Int16 nErg = context->buildPredicateRule((yyval.pParseNode),(yyvsp[(2) - (2)].pParseNode),(yyvsp[(1) - (2)].pParseNode));
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

  case 119:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 120:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 121:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 122:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 123:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 124:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 125:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 126:

    {
            if (context->inPredicateCheck())  // like_predicate: rule 5
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                if ( !context->buildLikeRule((yyvsp[(1) - (1)].pParseNode),p2nd,p3rd) )
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

  case 127:

    {
            if (context->inPredicateCheck()) // like_predicate: rule 6
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[(1) - (1)].pParseNode)->removeAt(2);
                if ( !context->buildLikeRule((yyvsp[(1) - (1)].pParseNode),p2nd,p3rd) )
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

  case 128:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 129:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 130:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 131:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 132:

    {
            if (context->inPredicateCheck())// test_for_null: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 133:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 134:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 135:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 136:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 137:

    {
            if ( context->inPredicateCheck() )// in_predicate: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 138:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 139:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 140:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 141:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 145:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 146:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 147:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 148:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 149:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 157:

    {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 158:

    {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 159:

    {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 160:

    {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 161:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 162:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 164:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 165:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 169:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 170:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 171:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 172:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 173:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 174:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 175:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 176:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 177:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 180:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 194:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 195:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 196:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 197:

    {
            if ( (yyvsp[(3) - (4)].pParseNode)->count() == 1 || (yyvsp[(3) - (4)].pParseNode)->count() == 2 )
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
                (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
                (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            }
            else
                YYERROR;
        }
    break;

  case 198:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 204:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 205:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 211:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 216:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 217:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 218:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 219:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 221:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
    }
    break;

  case 229:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 232:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 234:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (8)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (8)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (8)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(7) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(8) - (8)].pParseNode));
    }
    break;

  case 237:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 238:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 243:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 245:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 246:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 247:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 248:

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
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 251:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 253:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 255:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 259:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 260:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 261:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 262:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 263:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 265:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 270:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 272:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 273:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 274:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 278:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 280:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 281:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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

  case 284:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 285:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 286:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 287:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 295:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 296:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 297:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 298:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 301:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 302:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 304:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 305:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 306:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 307:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 309:

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

  case 310:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 315:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 316:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 335:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 336:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 337:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 340:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 347:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 352:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 353:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 355:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 356:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 357:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 359:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 360:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 361:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 362:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 363:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 364:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 365:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 366:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 367:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 368:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 374:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 375:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 377:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 378:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 379:

    {
            if (context->inPredicateCheck())
            {
                (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
                (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
            else
                YYERROR;
        }
    break;

  case 386:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 387:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 390:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 392:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 395:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 396:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 397:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 398:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 399:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 402:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 403:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 404:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 405:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 406:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 410:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 411:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 412:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 413:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 414:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 418:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 419:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 420:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 421:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 422:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 423:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 424:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 425:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 426:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 427:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 428:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
         }
    break;

  case 429:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 430:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 431:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 432:

    {
			auto last = (yyvsp[(1) - (3)].pParseNode)->getLast();
			if (last)
				{
				if (last->getFirst()->getNodeType() == SQL_NODE_PUNCTUATION) //'*'
					{
					SQLyyerror(context, "'*' can only occur at the end of property path\n");
					YYERROR;
					}
				}

            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 433:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 434:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 435:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 436:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 442:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 443:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 444:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 445:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 446:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 447:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 448:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 455:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 456:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 457:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 458:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 459:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 463:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 464:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 465:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
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
            if (context->inPredicateCheck()) // sql: rule 1
            {
                (yyval.pParseNode) = (yyvsp[(1) - (1)].pParseNode);
                if ( SQL_ISRULE((yyval.pParseNode),search_condition) )
                {
                    (yyval.pParseNode)->insert(0,CREATE_NODE("(", SQL_NODE_PUNCTUATION));
                    (yyval.pParseNode)->append(CREATE_NODE(")", SQL_NODE_PUNCTUATION));
                }
            }
            else
                YYERROR;
        }
    break;

  case 469:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 471:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 472:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 473:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
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
      yyerror (context, YY_("syntax error"));
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
        yyerror (context, yymsgp);
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
		      yytoken, &yylval, context);
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
		  yystos[yystate], yyvsp, context);
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
  yyerror (context, YY_("memory exhausted"));
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
                  yytoken, &yylval, context);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, context);
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

// -------------------------------------------------------------------------
void OSQLParser::setParseTree(OSQLParseNode * pNewParseTree)
    {
    m_pParseTree =std::unique_ptr<OSQLParseNode>(pNewParseTree->detach());
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
//-----------------------------------------------------------------------------
std::unique_ptr<OSQLParseNode> OSQLParser::parseTree (Utf8String& rErrorMessage,Utf8String const& rStatement, sal_Bool bInternational) {
    Utf8String sTemp = delComment(rStatement);
    m_scanner = std::unique_ptr<OSQLScanner>(new OSQLScanner(sTemp.c_str(), m_pContext, sal_True));
    m_pParseTree = nullptr;
    m_sErrorMessage.clear();
    if (SQLyyparse(this) != 0)
        {
        // only set the error message, if it's not already set
        if (!m_sErrorMessage.size())
            m_sErrorMessage = m_scanner->getErrorMessage();

        rErrorMessage = m_sErrorMessage;
        return nullptr;
        }

    return std::move(m_pParseTree);
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

    pLiteral = m_scanner->NewNode(aValue,SQL_NODE_STRING);
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

        Utf8String aError = m_scanner->getErrorMessage();
        if(aError.size())
        {
            m_sErrorMessage += ", ";
            m_sErrorMessage += aError;
        }
    }
}
// -------------------------------------------------------------------------
int OSQLParser::SQLlex(YYSTYPE* val)
{
    return m_scanner->SQLlex(val);
}

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif
