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
#define YYFINAL  228
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3267

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  194
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  233
/* YYNRULES -- Number of rules.  */
#define YYNRULES  474
/* YYNRULES -- Number of states.  */
#define YYNSTATES  745

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
       0,     0,     3,     5,     8,    10,    14,    16,    20,    22,
      23,    27,    28,    32,    33,    37,    39,    43,    46,    49,
      50,    52,    54,    55,    57,    59,    61,    63,    65,    67,
      69,    71,    73,    78,    80,    82,    84,    87,    93,    99,
     104,   106,   110,   112,   114,   117,   124,   125,   127,   129,
     131,   135,   139,   141,   143,   150,   152,   156,   158,   159,
     161,   166,   168,   170,   172,   173,   175,   176,   179,   183,
     192,   195,   197,   201,   202,   204,   205,   209,   210,   212,
     216,   221,   223,   226,   227,   231,   232,   235,   237,   239,
     241,   243,   245,   247,   251,   253,   255,   260,   262,   265,
     267,   271,   273,   277,   279,   281,   283,   285,   287,   289,
     291,   293,   295,   298,   302,   305,   307,   309,   311,   313,
     315,   317,   320,   326,   329,   334,   339,   342,   345,   347,
     349,   350,   353,   357,   360,   362,   364,   368,   372,   375,
     377,   381,   384,   387,   391,   393,   395,   397,   400,   403,
     407,   409,   413,   415,   417,   419,   421,   423,   425,   427,
     430,   433,   436,   439,   440,   443,   445,   452,   457,   459,
     461,   463,   468,   473,   478,   483,   485,   487,   489,   491,
     493,   495,   497,   504,   506,   508,   510,   512,   514,   516,
     518,   520,   522,   524,   526,   528,   530,   534,   538,   543,
     548,   554,   556,   558,   560,   562,   564,   568,   572,   574,
     576,   578,   580,   582,   587,   589,   591,   593,   595,   596,
     599,   604,   605,   607,   614,   616,   618,   620,   622,   624,
     627,   630,   636,   638,   640,   641,   643,   652,   654,   656,
     659,   662,   664,   666,   668,   670,   671,   673,   676,   680,
     682,   686,   688,   692,   693,   695,   696,   698,   699,   701,
     706,   708,   712,   716,   718,   721,   722,   724,   728,   730,
     732,   734,   736,   739,   741,   744,   747,   752,   754,   756,
     758,   761,   763,   766,   770,   773,   776,   780,   786,   791,
     797,   799,   801,   803,   805,   807,   809,   811,   813,   815,
     817,   820,   822,   824,   825,   827,   829,   832,   837,   843,
     849,   851,   859,   860,   862,   864,   866,   868,   873,   874,
     876,   878,   880,   882,   884,   886,   888,   890,   892,   894,
     896,   898,   900,   902,   904,   906,   908,   910,   912,   916,
     919,   921,   923,   930,   932,   934,   936,   938,   940,   942,
     946,   948,   950,   952,   954,   957,   960,   962,   966,   970,
     974,   976,   980,   984,   986,   988,   990,   993,   996,   998,
    1000,  1002,  1004,  1006,  1008,  1010,  1012,  1014,  1018,  1020,
    1022,  1026,  1030,  1032,  1034,  1036,  1038,  1040,  1042,  1046,
    1050,  1052,  1054,  1057,  1059,  1062,  1064,  1066,  1068,  1076,
    1078,  1080,  1081,  1083,  1085,  1087,  1089,  1090,  1093,  1101,
    1106,  1108,  1110,  1115,  1122,  1129,  1136,  1139,  1141,  1143,
    1147,  1153,  1156,  1157,  1163,  1164,  1166,  1168,  1172,  1175,
    1177,  1179,  1180,  1182,  1184,  1186,  1188,  1190,  1196,  1201,
    1203,  1206,  1211,  1213,  1217,  1219,  1221,  1223,  1225,  1227,
    1229,  1231,  1234,  1239,  1240,  1243,  1245,  1247,  1249,  1252,
    1254,  1257,  1258,  1261,  1263,  1267,  1268,  1270,  1273,  1276,
    1278,  1280,  1284,  1286,  1288
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     195,     0,    -1,   196,    -1,   196,     7,    -1,   206,    -1,
     197,     5,   405,    -1,   405,    -1,   198,     5,   403,    -1,
     403,    -1,    -1,     3,   197,     4,    -1,    -1,     3,   198,
       4,    -1,    -1,    81,    37,   202,    -1,   203,    -1,   202,
       5,   203,    -1,   248,   204,    -1,   215,   204,    -1,    -1,
      32,    -1,    49,    -1,    -1,    23,    -1,   209,    -1,   210,
      -1,   211,    -1,   216,    -1,   217,    -1,   222,    -1,   207,
      -1,   226,    -1,   226,   208,   353,   207,    -1,    69,    -1,
      92,    -1,    54,    -1,    40,   101,    -1,    48,    59,   237,
     225,   422,    -1,    66,    67,   397,   200,   212,    -1,    98,
       3,   213,     4,    -1,   214,    -1,   213,     5,   214,    -1,
     215,    -1,   375,    -1,    83,   101,    -1,    84,   218,   227,
      67,   223,   231,    -1,    -1,    27,    -1,    50,    -1,   220,
      -1,   219,     5,   220,    -1,   403,   179,   221,    -1,   375,
      -1,    47,    -1,    95,   237,    85,   219,   225,   422,    -1,
     224,    -1,   223,     5,   224,    -1,   273,    -1,    -1,   238,
      -1,    84,   218,   227,   231,    -1,   212,    -1,   188,    -1,
     271,    -1,    -1,   230,    -1,    -1,   157,   365,    -1,   156,
     365,   229,    -1,   232,   225,   239,   240,   315,   201,   228,
     422,    -1,    59,   233,    -1,   237,    -1,   233,     5,   237,
      -1,    -1,    31,    -1,    -1,   234,    24,   199,    -1,    -1,
     158,    -1,   236,   398,   235,    -1,   236,   270,   421,   200,
      -1,   351,    -1,    99,   247,    -1,    -1,    61,    37,   372,
      -1,    -1,    62,   247,    -1,    91,    -1,    56,    -1,    94,
      -1,    78,    -1,   248,    -1,   243,    -1,     3,   247,     4,
      -1,   375,    -1,   242,    -1,   242,    68,   205,   241,    -1,
     244,    -1,    23,   244,    -1,   245,    -1,   246,   176,   245,
      -1,   246,    -1,   247,   175,   246,    -1,   250,    -1,   253,
      -1,   264,    -1,   268,    -1,   269,    -1,   259,    -1,   262,
      -1,   256,    -1,   265,    -1,   251,   214,    -1,   214,   251,
     214,    -1,   251,   214,    -1,   181,    -1,   182,    -1,   179,
      -1,   180,    -1,   184,    -1,   183,    -1,    68,   205,    -1,
     205,    35,   214,   176,   214,    -1,   214,   252,    -1,   205,
      71,   376,   257,    -1,   205,    71,   361,   257,    -1,   214,
     254,    -1,   214,   255,    -1,   254,    -1,   255,    -1,    -1,
      53,   376,    -1,    68,   205,    78,    -1,   214,   258,    -1,
     258,    -1,   270,    -1,     3,   372,     4,    -1,   205,    63,
     260,    -1,   214,   261,    -1,   261,    -1,   251,   267,   270,
      -1,   214,   263,    -1,   214,   266,    -1,   205,   159,   287,
      -1,    30,    -1,    27,    -1,    86,    -1,    55,   270,    -1,
      93,   270,    -1,     3,   207,     4,    -1,   272,    -1,   271,
       5,   272,    -1,   395,    -1,   420,    -1,   162,    -1,    20,
      -1,    21,    -1,    22,    -1,    19,    -1,   274,   169,    -1,
     274,   162,    -1,   274,    20,    -1,   274,    22,    -1,    -1,
      31,   405,    -1,   405,    -1,   104,     3,   375,    63,   375,
       4,    -1,   104,     3,   372,     4,    -1,   276,    -1,   284,
      -1,   281,    -1,   103,     3,   375,     4,    -1,   106,     3,
     375,     4,    -1,    79,     3,   375,     4,    -1,   102,     3,
     375,     4,    -1,   278,    -1,   279,    -1,   280,    -1,   371,
      -1,   115,    -1,   282,    -1,   375,    -1,   111,     3,   283,
      59,   375,     4,    -1,   286,    -1,   274,    -1,   420,    -1,
      78,    -1,    56,    -1,    91,    -1,   126,    -1,   127,    -1,
     128,    -1,   129,    -1,   130,    -1,   131,    -1,   341,    -1,
     290,     3,     4,    -1,   288,     3,     4,    -1,   290,     3,
     374,     4,    -1,   289,     3,   374,     4,    -1,   290,     3,
     218,   373,     4,    -1,   291,    -1,   116,    -1,    24,    -1,
     109,    -1,   110,    -1,   293,   132,   313,    -1,   133,     3,
       4,    -1,   341,    -1,   294,    -1,   300,    -1,   306,    -1,
     309,    -1,   134,     3,   297,     4,    -1,   420,    -1,   274,
      -1,   296,    -1,   295,    -1,    -1,     5,   303,    -1,     5,
     303,     5,   304,    -1,    -1,   305,    -1,   301,     3,   302,
     298,     4,   299,    -1,   135,    -1,   136,    -1,   375,    -1,
      21,    -1,   375,    -1,   137,   139,    -1,   138,   139,    -1,
     307,     3,   375,     4,   299,    -1,   140,    -1,   141,    -1,
      -1,   311,    -1,   142,     3,   375,     5,   310,     4,   308,
     299,    -1,   296,    -1,   295,    -1,    59,   143,    -1,    59,
     144,    -1,    24,    -1,   312,    -1,   314,    -1,   320,    -1,
      -1,   316,    -1,   154,   317,    -1,   317,     5,   318,    -1,
     318,    -1,   319,    31,   320,    -1,   312,    -1,     3,   324,
       4,    -1,    -1,   325,    -1,    -1,   326,    -1,    -1,   330,
      -1,   321,   322,   201,   323,    -1,   312,    -1,   153,    37,
     327,    -1,   327,     5,   328,    -1,   328,    -1,   403,   404,
      -1,    -1,   340,    -1,   331,   332,   329,    -1,   152,    -1,
     151,    -1,   333,    -1,   335,    -1,   149,   150,    -1,   334,
      -1,    44,   125,    -1,   285,   150,    -1,    35,   336,   176,
     337,    -1,   338,    -1,   338,    -1,   333,    -1,   149,   148,
      -1,   339,    -1,   285,   148,    -1,   145,    44,   125,    -1,
     145,    61,    -1,   145,   147,    -1,   145,   155,   146,    -1,
     342,     3,   218,   373,     4,    -1,    42,     3,   188,     4,
      -1,    42,     3,   218,   373,     4,    -1,    34,    -1,    75,
      -1,    76,    -1,    89,    -1,   118,    -1,    30,    -1,    86,
      -1,    72,    -1,    73,    -1,    60,    -1,    80,   247,    -1,
     344,    -1,   352,    -1,    -1,    65,    -1,   343,    -1,   343,
      82,    -1,   237,    43,    70,   237,    -1,   237,    77,   346,
      70,   237,    -1,   237,   346,    70,   237,   345,    -1,   347,
      -1,   237,   346,    70,   237,    97,   398,   350,    -1,    -1,
      51,    -1,    52,    -1,   349,    -1,   348,    -1,    97,     3,
     197,     4,    -1,    -1,    27,    -1,   270,    -1,   375,    -1,
     171,    -1,   172,    -1,   165,    -1,   166,    -1,   168,    -1,
     162,    -1,   161,    -1,   163,    -1,   164,    -1,   167,    -1,
     169,    -1,   173,    -1,   174,    -1,   170,    -1,    24,    -1,
     356,    -1,    24,    13,    24,    -1,   357,    25,    -1,   357,
      -1,   358,    -1,    38,     3,   355,    31,   359,     4,    -1,
     285,    -1,   287,    -1,   403,    -1,   354,    -1,   406,    -1,
     292,    -1,     3,   375,     4,    -1,   360,    -1,   361,    -1,
     277,    -1,   362,    -1,   186,   362,    -1,   185,   362,    -1,
     363,    -1,   364,   188,   363,    -1,   364,   189,   363,    -1,
     364,   190,   363,    -1,   364,    -1,   365,   185,   364,    -1,
     365,   186,   364,    -1,   367,    -1,   107,    -1,   108,    -1,
     173,   376,    -1,   174,   376,    -1,   366,    -1,   368,    -1,
     369,    -1,   117,    -1,   114,    -1,    46,    -1,   112,    -1,
     113,    -1,   375,    -1,   372,     5,   375,    -1,   417,    -1,
     373,    -1,   374,     5,   373,    -1,   374,     7,   373,    -1,
     365,    -1,   376,    -1,   370,    -1,   377,    -1,   381,    -1,
     378,    -1,   377,   185,   381,    -1,   375,   187,   375,    -1,
     169,    -1,   382,    -1,    39,   397,    -1,   379,    -1,   379,
     380,    -1,   388,    -1,   383,    -1,   384,    -1,   105,     3,
     385,    59,   376,   389,     4,    -1,   386,    -1,   387,    -1,
      -1,   390,    -1,   392,    -1,   393,    -1,   394,    -1,    -1,
      57,   375,    -1,   105,     3,   375,    59,   375,   389,     4,
      -1,   105,     3,   372,     4,    -1,    96,    -1,    74,    -1,
     391,     3,   375,     4,    -1,    41,     3,   376,    97,   397,
       4,    -1,    41,     3,   355,     5,   359,     4,    -1,    90,
       3,   376,    97,   397,     4,    -1,   375,   275,    -1,    13,
      -1,     6,    -1,    24,   396,    24,    -1,    24,   396,    24,
     396,    24,    -1,   397,   399,    -1,    -1,    13,   290,     3,
     374,     4,    -1,    -1,    25,    -1,   402,    -1,   401,    13,
     402,    -1,    24,   400,    -1,   188,    -1,   401,    -1,    -1,
     380,    -1,    24,    -1,   407,    -1,   408,    -1,   409,    -1,
     120,   419,   410,   416,   122,    -1,   120,   414,   416,   122,
      -1,   411,    -1,   414,   411,    -1,   123,   412,   121,   417,
      -1,   413,    -1,   412,     5,   413,    -1,   215,    -1,   249,
      -1,   252,    -1,   261,    -1,   254,    -1,   258,    -1,   415,
      -1,   414,   415,    -1,   123,   247,   121,   417,    -1,    -1,
     124,   417,    -1,   418,    -1,   375,    -1,   215,    -1,     6,
      24,    -1,     8,    -1,     8,    21,    -1,    -1,   234,    24,
      -1,   247,    -1,     3,   196,     4,    -1,    -1,   423,    -1,
     160,   424,    -1,   424,   425,    -1,   425,    -1,    24,    -1,
      24,   179,   426,    -1,   274,    -1,    24,    -1,   241,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   235,   235,   237,   245,   250,   255,   263,   268,   278,
     279,   287,   288,   299,   300,   310,   315,   323,   331,   340,
     341,   342,   346,   347,   353,   354,   355,   356,   357,   358,
     359,   363,   368,   379,   380,   381,   384,   391,   403,   412,
     422,   427,   435,   438,   443,   453,   464,   465,   466,   470,
     473,   479,   486,   487,   490,   502,   505,   511,   515,   516,
     524,   532,   536,   541,   544,   545,   548,   549,   557,   566,
     581,   591,   594,   600,   601,   605,   608,   617,   618,   622,
     629,   637,   640,   649,   650,   660,   661,   669,   670,   671,
     672,   675,   676,   677,   692,   700,   701,   711,   712,   720,
     721,   730,   731,   741,   742,   743,   744,   745,   746,   747,
     748,   749,   755,   762,   769,   794,   795,   796,   797,   798,
     799,   800,   808,   818,   826,   836,   846,   852,   858,   880,
     905,   906,   913,   922,   928,   944,   948,   956,   965,   971,
     987,   996,  1005,  1014,  1024,  1025,  1026,  1030,  1038,  1044,
    1055,  1060,  1067,  1071,  1075,  1076,  1077,  1078,  1079,  1081,
    1093,  1105,  1117,  1133,  1134,  1140,  1143,  1153,  1163,  1164,
    1165,  1168,  1176,  1187,  1197,  1207,  1212,  1217,  1224,  1229,
    1236,  1237,  1241,  1253,  1254,  1257,  1258,  1259,  1260,  1261,
    1262,  1263,  1264,  1265,  1266,  1269,  1270,  1277,  1284,  1292,
    1306,  1319,  1322,  1326,  1330,  1331,  1336,  1345,  1352,  1353,
    1354,  1355,  1356,  1359,  1369,  1372,  1375,  1376,  1379,  1380,
    1386,  1396,  1397,  1401,  1413,  1414,  1417,  1420,  1423,  1426,
    1427,  1430,  1441,  1442,  1445,  1446,  1449,  1463,  1464,  1467,
    1473,  1481,  1484,  1485,  1488,  1491,  1492,  1495,  1503,  1506,
    1511,  1520,  1523,  1532,  1533,  1536,  1537,  1540,  1541,  1544,
    1550,  1553,  1562,  1565,  1570,  1578,  1579,  1582,  1591,  1592,
    1595,  1596,  1599,  1605,  1606,  1614,  1622,  1632,  1635,  1638,
    1639,  1645,  1648,  1656,  1663,  1669,  1675,  1685,  1694,  1702,
    1714,  1715,  1716,  1717,  1718,  1719,  1720,  1724,  1729,  1734,
    1741,  1749,  1750,  1753,  1754,  1759,  1760,  1768,  1780,  1790,
    1799,  1804,  1818,  1819,  1820,  1823,  1824,  1827,  1839,  1840,
    1844,  1847,  1851,  1852,  1853,  1854,  1855,  1856,  1857,  1858,
    1859,  1860,  1861,  1862,  1863,  1864,  1865,  1869,  1874,  1884,
    1893,  1894,  1898,  1910,  1911,  1912,  1913,  1914,  1915,  1916,
    1923,  1927,  1928,  1932,  1933,  1939,  1948,  1949,  1956,  1963,
    1973,  1974,  1981,  1995,  2002,  2008,  2013,  2019,  2029,  2036,
    2044,  2052,  2053,  2054,  2055,  2056,  2061,  2064,  2070,  2097,
    2100,  2104,  2117,  2118,  2119,  2122,  2130,  2131,  2134,  2141,
    2151,  2152,  2155,  2163,  2164,  2172,  2173,  2176,  2183,  2196,
    2220,  2227,  2230,  2231,  2232,  2237,  2244,  2245,  2253,  2264,
    2274,  2275,  2278,  2288,  2298,  2310,  2323,  2332,  2333,  2337,
    2344,  2356,  2365,  2366,  2378,  2379,  2387,  2392,  2410,  2416,
    2424,  2431,  2432,  2442,  2446,  2450,  2451,  2455,  2466,  2476,
    2481,  2488,  2498,  2501,  2506,  2507,  2508,  2509,  2510,  2511,
    2514,  2519,  2526,  2536,  2537,  2545,  2549,  2552,  2556,  2562,
    2567,  2577,  2580,  2590,  2604,  2608,  2609,  2613,  2622,  2627,
    2635,  2641,  2651,  2652,  2653
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
  "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql_single_statement", "sql",
  "column_commalist", "column_ref_commalist", "opt_column_commalist",
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
  "derived_column", "table_node_separator", "table_node", "table_node_mf",
  "opt_member_func_call", "opt_column_array_idx", "property_path",
  "property_path_entry", "column_ref", "opt_collate_clause", "column",
  "case_expression", "case_specification", "simple_case", "searched_case",
  "simple_when_clause_list", "simple_when_clause", "when_operand_list",
  "when_operand", "searched_when_clause_list", "searched_when_clause",
  "else_clause", "result", "result_expression", "case_operand",
  "parameter", "range_variable", "opt_ecsqloptions_clause",
  "ecsqloptions_clause", "ecsqloptions_list", "ecsqloption",
  "ecsqloptionvalue", YY_NULL
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
       0,   194,   195,   195,   196,   197,   197,   198,   198,   199,
     199,   200,   200,   201,   201,   202,   202,   203,   203,   204,
     204,   204,   205,   205,   206,   206,   206,   206,   206,   206,
     206,   207,   207,   208,   208,   208,   209,   210,   211,   212,
     213,   213,   214,   215,   216,   217,   218,   218,   218,   219,
     219,   220,   221,   221,   222,   223,   223,   224,   225,   225,
     226,   226,   227,   227,   228,   228,   229,   229,   230,   231,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   237,   238,   239,   239,   240,   240,   241,   241,   241,
     241,   242,   242,   242,   243,   244,   244,   245,   245,   246,
     246,   247,   247,   248,   248,   248,   248,   248,   248,   248,
     248,   248,   249,   250,   250,   251,   251,   251,   251,   251,
     251,   251,   252,   253,   254,   255,   256,   256,   256,   256,
     257,   257,   258,   259,   259,   260,   260,   261,   262,   262,
     263,   264,   265,   266,   267,   267,   267,   268,   269,   270,
     271,   271,   272,   273,   274,   274,   274,   274,   274,   274,
     274,   274,   274,   275,   275,   275,   276,   276,   277,   277,
     277,   278,   278,   279,   280,   281,   281,   281,   282,   282,
     283,   283,   284,   285,   285,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   287,   287,   287,   287,   287,
     287,   288,   289,   290,   291,   291,   292,   293,   293,   293,
     293,   293,   293,   294,   295,   296,   297,   297,   298,   298,
     298,   299,   299,   300,   301,   301,   302,   303,   304,   305,
     305,   306,   307,   307,   308,   308,   309,   310,   310,   311,
     311,   312,   313,   313,   314,   315,   315,   316,   317,   317,
     318,   319,   320,   321,   321,   322,   322,   323,   323,   324,
     325,   326,   327,   327,   328,   329,   329,   330,   331,   331,
     332,   332,   333,   333,   333,   334,   335,   336,   337,   338,
     338,   338,   339,   340,   340,   340,   340,   341,   341,   341,
     342,   342,   342,   342,   342,   342,   342,   343,   343,   343,
     344,   345,   345,   346,   346,   346,   346,   347,   348,   348,
     348,   349,   350,   350,   350,   351,   351,   352,   353,   353,
     354,   355,   356,   356,   356,   356,   356,   356,   356,   356,
     356,   356,   356,   356,   356,   356,   356,   357,   357,   358,
     359,   359,   360,   361,   361,   361,   361,   361,   361,   361,
     361,   362,   362,   363,   363,   363,   364,   364,   364,   364,
     365,   365,   365,   366,   367,   367,   367,   367,   368,   369,
     370,   371,   371,   371,   371,   371,   372,   372,   373,   374,
     374,   374,   375,   375,   375,   376,   377,   377,   378,   378,
     379,   379,   380,   381,   381,   382,   382,   383,   384,   385,
     386,   387,   388,   388,   388,   388,   389,   389,   390,   390,
     391,   391,   392,   393,   393,   394,   395,   396,   396,   397,
     397,   398,   399,   399,   400,   400,   401,   401,   402,   402,
     403,   404,   404,   405,   406,   407,   407,   408,   409,   410,
     410,   411,   412,   412,   413,   413,   413,   413,   413,   413,
     414,   414,   415,   416,   416,   417,   418,   419,   420,   420,
     420,   421,   421,   196,   196,   422,   422,   423,   424,   424,
     425,   425,   426,   426,   426
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     3,     1,     3,     1,     0,
       3,     0,     3,     0,     3,     1,     3,     2,     2,     0,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     2,     5,     5,     4,
       1,     3,     1,     1,     2,     6,     0,     1,     1,     1,
       3,     3,     1,     1,     6,     1,     3,     1,     0,     1,
       4,     1,     1,     1,     0,     1,     0,     2,     3,     8,
       2,     1,     3,     0,     1,     0,     3,     0,     1,     3,
       4,     1,     2,     0,     3,     0,     2,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     4,     1,     2,     1,
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
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     6,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     2,     2,     1,     3,     3,     3,
       1,     3,     3,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     3,     3,
       1,     1,     2,     1,     2,     1,     1,     1,     7,     1,
       1,     0,     1,     1,     1,     1,     0,     2,     7,     4,
       1,     1,     4,     6,     6,     6,     2,     1,     1,     3,
       5,     2,     0,     5,     0,     1,     1,     3,     2,     1,
       1,     0,     1,     1,     1,     1,     1,     5,     4,     1,
       2,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     4,     0,     2,     1,     1,     1,     2,     1,
       2,     0,     2,     1,     3,     0,     1,     2,     2,     1,
       1,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      22,    22,     0,   459,   158,   155,   156,   157,    22,   424,
     295,   290,     0,     0,     0,     0,     0,     0,   187,     0,
      22,   411,   291,   292,   186,     0,     0,    46,   296,   293,
       0,   188,     0,    77,   410,     0,     0,     0,     0,     0,
       0,   364,   365,   204,   205,     0,   202,   294,     0,   189,
     190,   191,   192,   193,   194,     0,     0,   224,   225,   232,
     233,     0,   154,   390,     0,     0,   117,   118,   115,   116,
     120,   119,     0,     0,   429,     0,     2,     0,     4,    30,
      24,    25,    26,    61,    22,    42,    27,    28,    29,    31,
      95,    92,    97,    99,   101,   463,    91,   103,     0,   104,
     128,   129,   110,   134,   108,   139,   109,   105,   111,   106,
     107,   320,   184,   168,   352,   175,   176,   177,   170,   169,
     343,   183,   344,     0,     0,     0,   201,   348,     0,   209,
     210,     0,   211,     0,   212,   195,     0,   346,   350,   351,
     353,   356,   360,   382,   368,   363,   369,   370,   384,    94,
     383,   385,   387,   393,   386,   391,   396,   397,   395,   402,
       0,   403,   404,   405,   430,   426,   345,   347,   434,   435,
     436,   185,     0,     0,     0,    43,   458,   460,    22,    23,
      98,   425,   428,     0,    36,     0,    46,    77,     0,   147,
       0,   121,     0,    44,    47,    48,     0,     0,   148,    78,
       0,   303,   310,   316,   315,    81,     0,     0,     0,     0,
     401,     0,     0,     0,    22,   457,    43,   453,   450,     0,
       0,     0,     0,     0,   366,   367,   355,   354,     1,     3,
       0,     0,     0,     0,   123,   126,   127,   133,   138,   141,
     142,    35,    33,    34,   318,    22,    22,    22,   114,   161,
     162,   160,   159,     0,     0,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,   394,     0,
       0,   464,   149,    93,   349,    46,     0,     0,     0,   321,
       0,   383,     0,     0,    58,     0,    11,   132,     0,   429,
       0,    63,   150,   163,   152,   383,   461,   422,    75,     0,
     299,   304,   297,   298,   303,     0,   305,     0,     0,    40,
       0,     0,     0,   376,     0,   376,     0,   399,   400,     0,
     373,   374,   375,   372,   179,   371,   180,     0,   178,   181,
       0,     0,     0,   451,     0,    22,   453,   439,     0,   207,
     215,   217,   216,     0,   214,     0,     0,   137,   135,   130,
     130,     0,     0,   145,   144,   146,   113,     0,   319,     0,
       0,   100,   102,   197,   379,     0,   456,   378,   455,   196,
       0,     0,   253,   241,   242,   206,   243,   244,   218,   226,
       0,     0,   357,   358,   359,   361,   362,   389,   388,   392,
       0,   424,   427,     0,     0,     0,     0,   288,     0,    22,
     465,    59,   418,   417,     0,     0,     0,   173,    77,     0,
      60,    58,     0,   433,     0,   416,   165,     0,    74,     0,
      11,     0,   421,     0,    79,    77,     0,    58,    49,     0,
     306,    77,    39,     0,   174,   171,   167,     0,     0,   409,
       0,     0,   172,     0,     0,   454,   438,     0,    42,   445,
       0,   446,   128,   134,   139,     0,   442,     0,   440,   213,
       0,     0,   376,     0,   125,   124,     0,   203,   143,   195,
     140,    32,    88,    90,    87,    89,    96,   199,     0,     0,
       0,   198,   260,   255,     0,   254,     0,     0,   221,     0,
     412,     0,   336,   328,   327,   329,   330,   324,   325,   331,
     326,   332,   335,   322,   323,   333,   334,   337,   340,   341,
       0,     0,     0,   289,    82,     0,    37,   466,   419,     0,
       8,    38,    70,    71,     0,    55,    57,   153,    83,   151,
     164,     0,   462,    80,     0,     9,   307,    77,     0,   465,
       0,   303,    41,   377,     0,   406,   383,     0,   452,   114,
      22,     0,   437,   238,   237,     0,   136,   131,     0,   380,
     381,   200,     0,    13,   256,   252,   227,   219,   221,     0,
       0,   231,   222,   287,     0,   339,   342,   414,   413,   470,
     467,   469,     0,    12,     0,    77,     0,    45,     0,    85,
     415,     0,     0,    76,   308,    50,    54,    53,    51,    52,
      22,     0,   301,   309,   302,   166,     0,     0,     0,   182,
       0,   444,     0,   448,   449,   447,   443,   441,   234,   122,
       0,     0,   257,     0,   223,   229,   230,   338,     0,   468,
     420,     7,    72,    56,     0,    22,   245,     0,     0,     6,
     300,     0,   312,   407,   408,   398,     0,   112,     0,   221,
     235,   261,   263,   431,    22,   269,   268,   259,   258,     0,
     220,   228,   473,   474,   472,   471,    84,    86,     0,    13,
     246,   423,    10,     0,     0,   313,   314,   311,   239,   240,
     236,     0,   432,   264,    14,    15,    42,    19,     0,     0,
       0,     0,   265,   270,   273,   271,   251,   247,   249,     0,
      64,     5,   317,   262,    22,    20,    21,    18,    17,     0,
       0,   279,     0,   277,   281,   274,   272,   275,     0,   267,
     266,     0,     0,     0,   465,    65,    16,   280,   282,     0,
       0,   284,   285,     0,   248,   250,    66,    69,   276,   278,
     283,   286,     0,    68,    67
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    75,    76,   638,   519,   593,   406,   622,   684,   685,
     707,    77,    78,   276,   244,    80,    81,    82,    83,   308,
      84,    85,    86,    87,   196,   427,   428,   598,    88,   524,
     525,   400,    89,   290,   724,   743,   725,   410,   411,   522,
     419,   424,   200,   201,   401,   589,   636,   476,    90,    91,
      92,    93,    94,   331,    96,   449,    97,    98,   451,    99,
     100,   101,   102,   464,   103,   104,   347,   105,   106,   239,
     107,   108,   240,   357,   109,   110,   111,   291,   292,   526,
     112,   415,   113,   114,   115,   116,   117,   118,   326,   327,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   341,   342,   343,   487,   571,   130,   131,   378,   567,
     660,   572,   132,   133,   649,   134,   555,   650,   696,   375,
     376,   669,   670,   697,   698,   699,   377,   483,   563,   657,
     484,   485,   564,   651,   652,   719,   658,   659,   692,   711,
     694,   695,   712,   738,   713,   714,   720,   135,   136,   306,
     602,   603,   307,   202,   203,   204,   677,   205,   604,   359,
     137,   278,   507,   508,   509,   510,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   328,   312,   364,
     365,   216,   150,   151,   152,   153,   268,   154,   155,   156,
     157,   316,   317,   318,   158,   607,   159,   160,   161,   162,
     163,   294,   404,   297,   298,   422,   182,   164,   165,   166,
     683,   639,   167,   168,   169,   170,   336,   337,   455,   456,
     217,   218,   334,   367,   368,   219,   171,   420,   516,   517,
     580,   581,   665
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -602
static const yytype_int16 yypact[] =
{
     574,   574,    71,   111,  -602,  -602,  -602,  -602,   942,    75,
    -602,  -602,   151,    99,   242,   251,   198,   273,  -602,   211,
     258,  -602,  -602,  -602,  -602,   282,   199,    43,  -602,  -602,
     302,  -602,   273,   144,  -602,   313,   318,   356,   359,   362,
     367,  -602,  -602,  -602,  -602,   372,  -602,  -602,  2149,  -602,
    -602,  -602,  -602,  -602,  -602,   378,   382,  -602,  -602,  -602,
    -602,   385,  -602,  -602,  2620,  2620,  -602,  -602,  -602,  -602,
    -602,  -602,  3079,  3079,  -602,   323,   383,    94,  -602,  -602,
    -602,  -602,  -602,  -602,    57,  -602,  -602,  -602,  -602,    17,
     333,  -602,  -602,  -602,   228,   236,  -602,  -602,  2620,  -602,
    -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,  -602,     5,  -602,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,  -602,  -602,   409,   412,   414,  -602,  -602,   286,  -602,
    -602,   416,  -602,   418,  -602,   290,   421,  -602,  -602,  -602,
    -602,  -602,   137,   -26,  -602,  -602,  -602,  -602,  -602,   114,
    -602,   240,  -602,   389,  -602,  -602,  -602,  -602,  -602,  -602,
     426,  -602,  -602,  -602,   417,  -602,  -602,  -602,  -602,  -602,
    -602,  -602,   428,   429,    24,    20,  -602,  -602,   758,  -602,
    -602,  -602,  -602,  2620,  -602,  2620,     2,   144,    58,  -602,
     419,   369,  2620,  -602,  -602,  -602,  2777,  2620,  -602,  -602,
      78,   291,  -602,  -602,  -602,  -602,  2620,  2620,  2620,  2620,
    2620,  2620,  1678,  1992,  1126,  -602,   253,    68,  -602,   327,
     445,    41,  2620,   253,  -602,  -602,  -602,  -602,  -602,  -602,
     448,  2620,    21,  2306,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,  -602,  -602,  -602,   425,   258,  1126,  1126,  -602,  -602,
    -602,  -602,  -602,   450,  2620,  1835,    91,  2620,  2620,    43,
    2934,  2934,  2934,  2934,  2934,  2620,    33,   419,  -602,  2620,
       6,  -602,  -602,  -602,  -602,    43,   429,    24,   424,   253,
     451,   360,   463,  2620,   393,   237,   466,  -602,    30,    29,
      88,   467,  -602,    14,  -602,   376,   220,   458,   220,   404,
    -602,  -602,  -602,  -602,    25,     6,   394,   411,   201,  -602,
      31,    37,   260,    -4,   314,   -19,   427,  -602,  -602,    38,
    -602,  -602,  -602,  -602,  -602,  -602,  -602,   430,  -602,   253,
      39,   -54,  2620,  -602,   363,  1126,   364,  -602,   327,  -602,
       5,  -602,  -602,   483,  -602,    32,  1992,  -602,  -602,   123,
       0,  2620,   298,  -602,  -602,  -602,  -602,   273,  -602,    58,
      23,  -602,   228,  -602,  -602,   284,   253,  -602,  -602,  -602,
    2620,   299,   469,  -602,  -602,  -602,  -602,  -602,   485,   253,
      40,  2620,  -602,  -602,  -602,   137,   137,  -602,  -602,  -602,
      42,   470,  -602,  2777,   342,   342,   419,  -602,   487,  1126,
     334,  -602,  -602,  -602,   473,     6,   400,  -602,   144,    83,
    -602,   401,  2620,  -602,   475,  -602,  -602,   419,  -602,   477,
     466,   495,  -602,   496,  -602,   144,   453,    52,  -602,   345,
    -602,   144,  -602,  2620,  -602,  -602,  -602,  2620,  2620,  -602,
    2620,  2620,  -602,  2620,  2620,  -602,  -602,    47,    49,  -602,
    2620,  -602,   516,   520,   521,    50,  -602,   405,  -602,  -602,
      41,   337,   253,  2620,  -602,  -602,   352,  -602,  -602,  -602,
    -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  2620,  2620,
     525,  -602,  -602,   377,   528,  -602,   512,   532,   193,   533,
    -602,   479,   527,  -602,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,   517,  -602,
     539,   541,   544,  -602,   236,   526,  -602,  -602,   237,   341,
    -602,  -602,   548,   474,    63,  -602,  -602,  -602,   488,  -602,
    -602,   550,  -602,  -602,   552,   553,   474,   144,     6,   334,
    2463,   326,  -602,   253,    44,   -24,    73,    46,  -602,    67,
    1494,  2620,  -602,  -602,  -602,   554,  -602,   370,  2620,  -602,
    -602,  -602,   522,   480,  -602,  -602,  -602,   555,   193,   423,
     433,  -602,  -602,  -602,   540,  -602,  -602,  -602,  -602,   384,
     526,  -602,   549,  -602,     6,   144,    83,  -602,   537,   503,
    -602,  2620,   475,  -602,   474,  -602,  -602,  -602,  -602,   253,
    1126,   128,  -602,  -602,  -602,  -602,  2620,   571,   572,  -602,
      98,  -602,  2620,  -602,  -602,  -602,  -602,  -602,   519,  -602,
       6,   542,   197,  2620,  -602,  -602,  -602,  -602,    84,  -602,
    -602,  -602,   474,  -602,  2620,  1126,   431,   310,   348,  -602,
     236,   475,   303,   253,  -602,  -602,  2620,  -602,   214,   193,
    -602,   576,  -602,   389,  1310,  -602,  -602,  -602,  -602,   440,
    -602,   253,  -602,  -602,     5,  -602,   578,   236,   469,   480,
    -602,  -602,  -602,   475,   374,  -602,  -602,  -602,  -602,  -602,
    -602,     6,  -602,  -602,   579,  -602,   275,   109,  2085,   461,
     437,   441,   454,  -602,  -602,  -602,  -602,   585,  -602,   569,
     449,  -602,  -602,  -602,  1310,  -602,  -602,  -602,  -602,   124,
     142,  -602,   434,  -602,  -602,  -602,  -602,  -602,   225,  -602,
    -602,   469,   606,  2934,   334,  -602,  -602,  -602,  -602,  2085,
     486,  -602,  -602,   471,  -602,  -602,   -74,  -602,  -602,  -602,
    -602,  -602,  2934,  -602,   -26
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -602,  -602,   612,   -23,  -602,  -602,   200,   -50,  -602,   -83,
     -64,   -15,  -602,    18,  -602,  -602,  -602,  -602,   219,  -602,
     -90,   -48,  -602,  -602,  -111,  -602,    89,  -602,  -602,  -602,
      45,  -261,  -602,   233,  -602,  -602,  -602,   104,  -602,  -602,
     335,  -602,  -602,  -148,  -602,  -602,  -602,     9,  -602,  -602,
     624,   388,   392,     3,  -578,  -602,  -602,   -73,   557,  -602,
     -72,   559,  -602,   294,   -68,  -602,  -602,   -67,  -602,  -602,
    -602,  -602,  -602,  -602,  -602,  -602,     4,  -602,   234,  -602,
    -208,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,  -601,  -602,   293,  -602,  -602,   230,  -602,  -602,  -602,
    -602,   194,   195,  -602,  -602,  -504,  -602,  -602,  -602,  -602,
    -602,  -602,  -602,  -602,  -602,  -602,  -602,  -602,  -183,  -602,
    -602,  -602,  -602,  -602,   -62,  -602,   -61,  -602,  -602,  -602,
    -602,  -602,  -602,  -602,   -13,  -602,  -602,  -602,  -602,     7,
    -602,  -602,  -602,  -602,   -56,  -602,  -602,   319,  -602,  -602,
    -602,  -602,   358,  -602,  -602,  -602,  -602,  -602,  -602,  -602,
    -602,   489,  -602,  -602,  -602,   280,  -602,   455,   320,    77,
     146,  -589,  -602,  -602,  -602,  -602,  -602,  -602,  -200,  -257,
    -241,     1,   -58,  -602,  -602,  -602,    34,   422,  -602,  -602,
    -602,  -602,  -602,  -602,  -602,   143,  -602,  -602,  -602,  -602,
    -602,  -602,   173,  -168,    92,  -602,  -602,  -602,   436,  -285,
    -602,  -278,  -602,  -602,  -602,  -602,  -602,   357,  -602,   147,
     492,  -166,   381,  -309,  -602,  -602,  -189,  -602,  -508,  -602,
    -602,   116,  -602
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -450
static const yytype_int16 yytable[] =
{
     215,   149,   175,    95,   174,   191,   224,   225,   248,   149,
     314,   233,   235,   340,   371,   416,   237,   238,    79,   173,
     429,   189,   286,   445,   274,   249,   398,   250,   273,   194,
     391,   596,   344,   606,   407,   434,   198,   460,   413,   284,
     440,   435,   442,   274,   488,   414,   490,     2,   605,     3,
     609,   333,   195,   463,  -444,   550,   351,   538,   691,   438,
       4,     5,     6,     7,   624,   223,   223,   444,   586,   232,
     194,   241,  -112,   374,    14,   283,   687,  -406,  -203,   472,
     179,   188,   351,   742,   230,   300,   242,   710,   -62,     2,
     301,     3,   231,   195,   372,   176,   -62,   302,   303,   389,
     181,   473,   285,     4,     5,     6,     7,    21,   662,   243,
     230,   263,   264,   480,   474,   373,   309,   475,   231,   -43,
     520,   247,   408,    30,   489,    20,   687,   281,   710,    34,
     606,   641,   177,   351,   736,   548,   530,   -43,    39,   295,
     472,   705,   275,   356,   370,   680,   461,   408,   381,   -43,
     528,   399,   285,   744,   183,   409,    35,   230,   706,   263,
     264,   230,   473,   265,   393,   231,   539,   251,   265,   646,
    -444,   551,   333,   350,   252,   474,   463,   -43,   475,   175,
     352,   277,   -43,   265,   279,   -43,   279,  -383,  -112,   482,
     282,   214,   332,   288,    74,   -94,   -94,   293,   223,   247,
     184,   265,    63,    62,   296,   432,   433,   265,   310,   311,
     313,   315,   319,   329,   330,   149,   737,   265,   265,   265,
     527,   559,   560,   345,   265,   265,   265,   265,   512,   265,
     360,   265,   223,   265,   348,   -43,    66,    67,    68,    69,
      70,    71,   617,   402,   -73,   185,    62,   149,   149,   531,
     403,   418,   340,   429,   186,   366,   366,   187,   379,   380,
     523,   466,   450,   452,   436,   437,   387,   453,   454,   730,
     390,   344,   727,   -43,   716,   -19,   188,   536,   190,   -19,
     -19,   179,   -19,   541,   366,   192,   731,   448,   477,   478,
     728,   479,   717,   -43,   -43,   -43,   -43,   -43,   -43,   631,
     193,   265,   199,   481,   478,   197,   479,   705,  -351,  -351,
    -351,  -351,  -351,  -351,   671,   478,   206,   479,   439,   437,
     447,   207,   467,   228,   706,   260,   261,   262,    10,   -19,
     569,   570,    11,   366,   299,   653,   149,   382,   383,   384,
      15,   556,   437,   542,   -19,   583,   584,   462,   655,   656,
     637,   300,   672,   673,   675,   676,   301,   678,   679,   208,
     549,   470,   209,   302,   303,   210,   492,   -19,   304,   299,
     211,   366,   732,    22,    23,   212,   305,   471,   702,   673,
     733,   220,   366,   546,    28,   221,   300,    29,   222,   594,
     229,   301,   226,   227,   293,   701,   653,   527,   302,   303,
     149,   245,   514,   304,   246,   557,   600,    43,    44,   385,
     386,   247,   253,   293,    46,   254,    47,   255,   256,   257,
     664,   258,  -208,   601,   259,   266,   -19,   -19,   267,   269,
     270,   -19,   271,   272,   666,   -19,   299,   632,   543,   544,
     265,   545,   223,   285,   547,   366,     2,   287,     3,   339,
     335,   346,   358,   300,   363,   394,   395,   396,   301,     4,
       5,     6,     7,  -303,   223,   302,   303,   397,   619,   405,
     304,   421,   412,   417,   425,   688,   430,   612,   613,   366,
     366,   431,   614,   615,   689,   446,   441,   459,   332,   443,
     486,   513,   399,   373,   515,   181,    18,   518,    35,   413,
     399,   532,   611,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   299,    24,   467,
     535,  -448,   647,   537,   540,  -449,  -447,   552,   558,   561,
     562,    31,   565,   566,   300,   610,   568,   573,   408,   301,
     574,   599,   575,   576,  -303,   577,   302,   303,   578,   588,
     579,   304,   366,   585,   590,   591,   592,  -383,   618,   620,
     623,   621,   625,   628,   627,   635,    49,    50,    51,    52,
      53,    54,   626,   630,   634,   644,   645,     1,   648,   654,
       2,   681,     3,   437,   704,   668,   715,   716,   350,   690,
     721,   717,   366,     4,     5,     6,     7,     8,     9,   718,
     722,   149,    62,   640,    10,   723,   686,   643,    11,   372,
     729,   740,    12,   172,    13,    14,    15,   741,   674,   700,
     533,   726,    16,   708,   661,   521,   491,   595,   587,    17,
      18,   633,   180,   423,   361,   462,   149,   663,   667,   362,
      19,   234,    20,   236,   465,   468,   529,   223,    21,    22,
      23,   534,    24,    25,   553,   554,   686,    26,    27,   734,
      28,   735,   426,    29,    30,    31,   693,    32,   703,    33,
      34,   469,    35,   739,   280,   511,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,   349,   682,   388,   608,
      46,   582,    47,   642,    48,   458,   629,   616,     0,     0,
      49,    50,    51,    52,    53,    54,   392,    55,    56,    57,
      58,   338,     0,     0,    59,    60,    61,   457,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,     0,
       0,     0,     0,    63,     0,     0,     0,    64,    65,     0,
       0,     0,     0,    66,    67,    68,    69,    70,    71,    72,
      73,   178,    74,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,    21,    22,    23,     0,    24,    25,     0,     0,
       0,     0,   275,     0,    28,     0,     0,    29,    30,    31,
       0,    32,     0,     0,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       0,     0,     0,     0,    46,     0,    47,     0,    48,     0,
       0,     0,     0,     0,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,     0,     0,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,     0,     0,     0,    63,     0,     0,
       0,    64,    65,     0,     0,     0,     0,    66,    67,    68,
      69,    70,    71,    72,    73,   178,    74,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,   179,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    11,   -23,     0,     0,
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
       0,    66,    67,    68,    69,    70,    71,    72,    73,   178,
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
      71,    72,    73,   213,    74,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,   179,     9,     0,     0,     0,     0,     0,
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
      67,    68,    69,    70,    71,    72,    73,   213,    74,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,   179,     9,     0,
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
      73,   213,    74,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     0,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,    14,
      15,     0,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    22,    23,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    28,     0,     0,    29,    30,    31,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
     321,   322,   323,   324,    46,   325,    47,     0,    48,     0,
       0,     0,     0,     0,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,     0,     0,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   369,
      62,     2,     0,     3,     0,     0,     0,    63,     0,     0,
       0,    64,    65,     0,     4,     5,     6,     7,     0,     9,
       0,     0,   194,    72,    73,    10,    74,     0,     0,    11,
       0,     0,     0,    12,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,   195,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   213,     0,    62,     2,     0,
       3,     0,     0,     0,    63,     0,     0,     0,    64,    65,
       0,     4,     5,     6,     7,     0,     9,     0,     0,     0,
      72,    73,    10,    74,     0,     0,    11,     0,     0,     0,
      12,     0,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    22,    23,     0,
      24,    25,     0,     0,     0,     0,   275,     0,    28,     0,
       0,    29,    30,    31,     0,     0,     0,     0,    34,     0,
      35,     2,     0,     3,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     4,     5,     6,     7,    46,     0,
      47,     0,    48,     0,     0,     0,     0,     0,    49,    50,
      51,    52,    53,    54,     0,    55,    56,    57,    58,   689,
       0,     0,    59,    60,    61,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,    62,     2,     0,     3,     0,     0,
       0,    63,     0,    24,     0,    64,    65,     0,     4,     5,
       6,     7,     0,     9,     0,     0,    31,    72,    73,    10,
      74,     0,     0,    11,     0,     0,     0,    12,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,     0,    21,    22,    23,     0,    24,    25,     0,
       0,     0,     0,     0,   709,    28,     0,     0,    29,    30,
      31,     0,     0,     0,     0,    34,     0,    62,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,     0,     0,     0,    46,     0,    47,     0,    48,
       0,     0,   214,     0,     0,    49,    50,    51,    52,    53,
      54,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,    62,     2,     0,     3,     0,     0,     0,    63,     0,
       0,     0,    64,    65,     0,     4,     5,     6,     7,     0,
       9,     0,     0,   353,    72,    73,   354,    74,     0,     0,
      11,     0,     0,     0,    12,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,    22,    23,     0,    24,    25,     0,     0,     0,     0,
       0,     0,   355,     0,     0,    29,    30,    31,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,     0,     0,
       0,     0,    46,     0,    47,     0,    48,     0,     0,     0,
       0,     0,    49,    50,    51,    52,    53,    54,     0,    55,
      56,    57,    58,     0,     0,     0,    59,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,    62,     2,
       0,     3,     0,     0,     0,    63,     0,     0,     0,    64,
      65,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
     597,     0,     0,     0,     0,     0,     0,     0,     0,    18,
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
       0,     0,     0,   213,     0,    62,     2,     0,     3,     0,
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
     213,     0,    62,     2,     0,     3,     0,     0,     0,    63,
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
       0,     0,     0,     0,     0,     0,     0,   213,     0,    62,
       2,     0,     3,     0,     0,     0,    63,     0,     0,     0,
      64,    65,     0,     4,     5,     6,     7,     0,     9,     0,
       0,     0,    72,    73,    10,   289,     0,     0,    11,     0,
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
       0,     0,   213,     0,     0,     2,     0,     3,     0,     0,
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
  (!!((Yystate) == (-602)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      48,     0,     1,     0,     1,    20,    64,    65,    98,     8,
     210,    84,    84,   221,   255,   293,    84,    84,     0,     1,
     305,    17,   190,   332,     4,    20,   283,    22,     4,    27,
      24,   539,   221,    57,     4,     4,    32,     5,    24,   187,
      59,     4,     4,     4,     4,    31,     4,     6,     4,     8,
       4,   217,    50,    53,     5,     5,    35,     5,   659,    63,
      19,    20,    21,    22,   568,    64,    65,   121,     5,    84,
      27,    54,     5,   256,    41,   186,   654,     4,     3,    56,
      23,     3,    35,   157,    63,    60,    69,   688,    59,     6,
      65,     8,    71,    50,     3,    24,    67,    72,    73,   267,
      25,    78,    24,    19,    20,    21,    22,    74,    24,    92,
      63,   185,   186,   370,    91,    24,   206,    94,    71,     5,
     405,   175,    59,    90,   381,    68,   704,   185,   729,    96,
      57,     3,    21,    35,   723,   444,   414,    23,   105,   197,
      56,    32,    84,   233,   255,   649,   346,    59,   259,    35,
     411,    99,    24,   742,     3,    67,    98,    63,    49,   185,
     186,    63,    78,   187,   275,    71,   427,   162,   187,    71,
     121,   121,   338,   231,   169,    91,    53,    63,    94,   178,
     159,   178,    68,   187,   183,    71,   185,   187,   121,   372,
     188,   123,   124,   192,   188,   175,   176,   196,   197,   175,
     101,   187,   169,   162,   200,     4,     5,   187,   207,   208,
     209,   210,   211,   212,   213,   214,   724,   187,   187,   187,
     409,   478,   479,   222,   187,   187,   187,   187,   396,   187,
     245,   187,   231,   187,   230,   121,   179,   180,   181,   182,
     183,   184,   551,     6,    24,     3,   162,   246,   247,   417,
      13,    31,   460,   538,     3,   254,   255,    59,   257,   258,
     408,   351,   335,   335,     4,     5,   265,   335,   335,    44,
     269,   460,   148,   159,   150,     0,     3,   425,    67,     4,
       5,    23,     7,   431,   283,     3,    61,   335,     4,     5,
     148,     7,   150,   179,   180,   181,   182,   183,   184,   584,
     101,   187,   158,     4,     5,     3,     7,    32,   185,   186,
     187,   188,   189,   190,     4,     5,     3,     7,     4,     5,
     335,     3,    24,     0,    49,   188,   189,   190,    30,    54,
     137,   138,    34,   332,    43,   620,   335,   260,   261,   262,
      42,     4,     5,   433,    69,     4,     5,   346,   151,   152,
     591,    60,     4,     5,    51,    52,    65,   143,   144,     3,
     450,   357,     3,    72,    73,     3,    24,    92,    77,    43,
       3,   370,   147,    75,    76,     3,    85,   359,     4,     5,
     155,     3,   381,   441,    86,     3,    60,    89,     3,   537,
       7,    65,    72,    73,   393,   673,   681,   586,    72,    73,
     399,    68,   399,    77,   176,   463,    80,   109,   110,   263,
     264,   175,     3,   412,   116,     3,   118,     3,   132,     3,
     628,     3,   132,    97,     3,   185,   151,   152,    39,     3,
      13,   156,     4,     4,   634,   160,    43,   585,   437,   438,
     187,   440,   441,    24,   443,   444,     6,    78,     8,     4,
     123,     3,    27,    60,     4,    31,     5,    97,    65,    19,
      20,    21,    22,    70,   463,    72,    73,     4,   558,     3,
      77,    13,     5,    97,    70,    35,    82,   550,   550,   478,
     479,    70,   550,   550,    44,   122,    59,     4,   124,    59,
       5,     4,    99,    24,   160,    25,    56,    24,    98,    24,
      99,    24,   550,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,    43,    78,    24,
      24,     5,   612,    70,   179,     5,     5,   122,   176,     4,
     153,    91,     4,    21,    60,   550,     4,     4,    59,    65,
      13,   540,    25,     4,    70,     4,    72,    73,     4,    61,
      24,    77,   551,     5,     4,     3,     3,   187,     4,    37,
       5,    81,   139,   179,    24,    62,   126,   127,   128,   129,
     130,   131,   139,    24,    37,     4,     4,     3,    59,    37,
       6,     5,     8,     5,     5,   154,   125,   150,   646,   149,
       5,   150,   591,    19,    20,    21,    22,    23,    24,   145,
      31,   600,   162,   600,    30,   156,   654,   606,    34,     3,
     176,   125,    38,     1,    40,    41,    42,   146,   641,   669,
     420,   704,    48,   687,   623,   406,   393,   538,   524,    55,
      56,   586,     8,   298,   246,   634,   635,   628,   635,   247,
      66,    84,    68,    84,   350,   352,   412,   646,    74,    75,
      76,   421,    78,    79,   460,   460,   704,    83,    84,   721,
      86,   722,   304,    89,    90,    91,   659,    93,   681,    95,
      96,   352,    98,   729,   185,   395,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   231,   653,   266,   546,
     116,   518,   118,   601,   120,   338,   580,   550,    -1,    -1,
     126,   127,   128,   129,   130,   131,   270,   133,   134,   135,
     136,   219,    -1,    -1,   140,   141,   142,   336,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,
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
      98,     6,    -1,     8,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    19,    20,    21,    22,   116,    -1,
     118,    -1,   120,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,    -1,   133,   134,   135,   136,    44,
      -1,    -1,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,   162,     6,    -1,     8,    -1,    -1,
      -1,   169,    -1,    78,    -1,   173,   174,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    91,   185,   186,    30,
     188,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,
      41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    -1,    78,    79,    -1,
      -1,    -1,    -1,    -1,   149,    86,    -1,    -1,    89,    90,
      91,    -1,    -1,    -1,    -1,    96,    -1,   162,    -1,    -1,
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
     183,   184,   185,   186,   188,   195,   196,   205,   206,   207,
     209,   210,   211,   212,   214,   215,   216,   217,   222,   226,
     242,   243,   244,   245,   246,   247,   248,   250,   251,   253,
     254,   255,   256,   258,   259,   261,   262,   264,   265,   268,
     269,   270,   274,   276,   277,   278,   279,   280,   281,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     300,   301,   306,   307,   309,   341,   342,   354,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   375,
     376,   377,   378,   379,   381,   382,   383,   384,   388,   390,
     391,   392,   393,   394,   401,   402,   403,   406,   407,   408,
     409,   420,   196,   207,   247,   375,    24,    21,     3,    23,
     244,    25,   400,     3,   101,     3,     3,    59,     3,   270,
      67,   205,     3,   101,    27,    50,   218,     3,   270,   158,
     236,   237,   347,   348,   349,   351,     3,     3,     3,     3,
       3,     3,     3,     3,   123,   215,   375,   414,   415,   419,
       3,     3,     3,   375,   376,   376,   362,   362,     0,     7,
      63,    71,   205,   251,   252,   254,   255,   258,   261,   263,
     266,    54,    69,    92,   208,    68,   176,   175,   214,    20,
      22,   162,   169,     3,     3,     3,   132,     3,     3,     3,
     188,   189,   190,   185,   186,   187,   185,    39,   380,     3,
      13,     4,     4,     4,     4,    84,   207,   247,   355,   375,
     355,   376,   188,   218,   237,    24,   397,    78,   375,   188,
     227,   271,   272,   375,   395,   376,   270,   397,   398,    43,
      60,    65,    72,    73,    77,    85,   343,   346,   213,   214,
     375,   375,   372,   375,   372,   375,   385,   386,   387,   375,
      46,   112,   113,   114,   115,   117,   282,   283,   371,   375,
     375,   247,   124,   415,   416,   123,   410,   411,   414,     4,
     274,   295,   296,   297,   420,   375,     3,   260,   270,   361,
     376,    35,   159,    27,    30,    86,   214,   267,    27,   353,
     205,   245,   246,     4,   373,   374,   375,   417,   418,     4,
     218,   374,     3,    24,   312,   313,   314,   320,   302,   375,
     375,   218,   363,   363,   363,   364,   364,   375,   381,   397,
     375,    24,   402,   218,    31,     5,    97,     4,   373,    99,
     225,   238,     6,    13,   396,     3,   200,     4,    59,    67,
     231,   232,     5,    24,    31,   275,   405,    97,    31,   234,
     421,    13,   399,   234,   235,    70,   346,   219,   220,   403,
      82,    70,     4,     5,     4,     4,     4,     5,    63,     4,
      59,    59,     4,    59,   121,   417,   122,   205,   215,   249,
     251,   252,   254,   258,   261,   412,   413,   416,   411,     4,
       5,   372,   375,    53,   257,   257,   214,    24,   287,   341,
     270,   207,    56,    78,    91,    94,   241,     4,     5,     7,
     373,     4,   312,   321,   324,   325,     5,   298,     4,   373,
       4,   227,    24,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   356,   357,   358,
     359,   359,   397,     4,   247,   160,   422,   423,    24,   198,
     403,   212,   233,   237,   223,   224,   273,   420,   225,   272,
     405,   397,    24,   200,   290,    24,   237,    70,     5,   225,
     179,   237,   214,   375,   375,   375,   376,   375,   417,   214,
       5,   121,   122,   295,   296,   310,     4,   376,   176,   373,
     373,     4,   153,   322,   326,     4,    21,   303,     4,   137,
     138,   299,   305,     4,    13,    25,     4,     4,     4,    24,
     424,   425,   396,     4,     5,     5,     5,   231,    61,   239,
       4,     3,     3,   199,   237,   220,   422,    47,   221,   375,
      80,    97,   344,   345,   352,     4,    57,   389,   389,     4,
     205,   215,   251,   254,   258,   261,   413,   417,     4,   214,
      37,    81,   201,     5,   299,   139,   139,    24,   179,   425,
      24,   403,   237,   224,    37,    62,   240,   374,   197,   405,
     247,     3,   398,   375,     4,     4,    71,   214,    59,   308,
     311,   327,   328,   403,    37,   151,   152,   323,   330,   331,
     304,   375,    24,   241,   274,   426,   372,   247,   154,   315,
     316,     4,     4,     5,   197,    51,    52,   350,   143,   144,
     299,     5,   380,   404,   202,   203,   215,   248,    35,    44,
     149,   285,   332,   333,   334,   335,   312,   317,   318,   319,
     201,   405,     4,   328,     5,    32,    49,   204,   204,   149,
     285,   333,   336,   338,   339,   125,   150,   150,   145,   329,
     340,     5,    31,   156,   228,   230,   203,   148,   148,   176,
      44,    61,   147,   155,   318,   320,   365,   422,   337,   338,
     125,   146,   157,   229,   365
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
        case 2:

    { context->setParseTree( (yyvsp[(1) - (1)].pParseNode) ); }
    break;

  case 3:

    { context->setParseTree( (yyvsp[(1) - (2)].pParseNode) ); }
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

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 8:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
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

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 13:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 14:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 15:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 16:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 17:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 18:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 19:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 22:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 31:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 32:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 36:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 37:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
            }
    break;

  case 38:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));}
    break;

  case 39:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 40:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 41:

    {    
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 44:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 45:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode)); }
    break;

  case 46:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 49:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 50:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 51:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
    break;

  case 54:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
            }
    break;

  case 55:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 56:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 58:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 60:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 62:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 64:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 66:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 67:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 68:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 69:

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

  case 70:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 71:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 72:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 73:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 75:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 76:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 77:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 79:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 80:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 82:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 83:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 84:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 85:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 86:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 93:

    { // boolean_primary: rule 2
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 94:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
    }
    break;

  case 96:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 98:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 100:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 102:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 112:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 113:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 114:

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

  case 121:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 122:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 123:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 124:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 125:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 126:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 127:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 128:

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

  case 129:

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

  case 130:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 131:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 132:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 133:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 134:

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

  case 135:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 136:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 137:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 138:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 139:

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

  case 140:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 141:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 142:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 143:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 147:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 148:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 149:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 150:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 151:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
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

  case 162:

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

  case 163:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 164:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 166:

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

  case 167:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 174:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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

  case 178:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 179:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 182:

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

  case 196:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 197:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 198:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 199:

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

  case 200:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 206:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 207:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 213:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 218:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 219:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 220:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 221:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 223:

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

  case 231:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 234:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 236:

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

  case 239:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 240:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 245:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 247:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 248:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 249:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 250:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 252:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 253:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 255:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 257:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 261:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 262:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 263:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 264:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 265:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 267:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 272:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 274:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 275:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 276:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
    }
    break;

  case 280:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 284:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 285:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 286:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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

  case 288:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 289:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 299:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 300:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 303:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 304:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 306:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 307:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 308:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 311:

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

  case 312:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 317:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 318:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 337:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 338:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 339:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 342:

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

  case 349:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 354:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 355:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 357:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 358:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 359:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 361:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 362:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 365:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 366:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 367:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 368:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 369:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 370:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 376:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 377:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 379:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 380:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 381:

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

  case 388:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 389:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 392:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 394:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 404:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 405:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 406:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 408:

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

  case 409:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 412:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 415:

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

  case 416:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 419:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 420:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 421:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 422:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 423:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 424:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 425:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 426:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 427:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 430:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 431:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 437:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 438:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 439:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 440:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 441:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 442:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 443:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 450:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 451:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 452:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 453:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 454:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 458:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 459:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 460:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 461:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 462:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 463:

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

  case 465:

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
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 469:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 470:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 471:

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
