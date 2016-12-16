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
#define YYFINAL  227
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3428

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  233
/* YYNRULES -- Number of rules.  */
#define YYNRULES  471
/* YYNRULES -- Number of states.  */
#define YYNSTATES  742

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   426

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   193,   181,     2,
       3,     4,   191,   188,     5,   189,    13,   192,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   195,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   180,    12,   194,     2,     2,     2,
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
     171,   172,   173,   174,   175,   176,   177,   178,   179,   182,
     183,   184,   185,   186,   187,   190,   196
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
     896,   898,   900,   902,   904,   906,   910,   913,   915,   917,
     924,   926,   928,   930,   932,   934,   936,   940,   942,   944,
     946,   948,   951,   954,   956,   960,   964,   968,   970,   974,
     978,   980,   982,   984,   987,   990,   992,   994,   996,   998,
    1000,  1002,  1004,  1006,  1008,  1012,  1016,  1018,  1020,  1024,
    1028,  1030,  1032,  1034,  1036,  1038,  1040,  1044,  1048,  1050,
    1052,  1055,  1057,  1060,  1062,  1064,  1066,  1074,  1076,  1078,
    1079,  1081,  1083,  1085,  1087,  1088,  1091,  1099,  1104,  1106,
    1108,  1113,  1120,  1127,  1134,  1137,  1139,  1141,  1143,  1147,
    1151,  1155,  1157,  1158,  1160,  1162,  1166,  1169,  1171,  1173,
    1174,  1176,  1178,  1180,  1182,  1184,  1190,  1195,  1197,  1200,
    1205,  1207,  1211,  1213,  1215,  1217,  1219,  1221,  1223,  1225,
    1228,  1233,  1234,  1237,  1239,  1241,  1243,  1246,  1248,  1249,
    1252,  1254,  1258,  1259,  1261,  1264,  1267,  1269,  1271,  1275,
    1277,  1279
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,   199,    -1,   199,     7,    -1,   209,    -1,
     200,     5,   408,    -1,   408,    -1,   201,     5,   406,    -1,
     406,    -1,    -1,     3,   200,     4,    -1,    -1,     3,   201,
       4,    -1,    -1,    82,    37,   205,    -1,   206,    -1,   205,
       5,   206,    -1,   251,   207,    -1,   218,   207,    -1,    -1,
      32,    -1,    49,    -1,    -1,    23,    -1,   212,    -1,   213,
      -1,   214,    -1,   219,    -1,   220,    -1,   225,    -1,   210,
      -1,   229,    -1,   229,   211,   356,   210,    -1,    70,    -1,
      94,    -1,    54,    -1,    40,   103,    -1,    48,    60,   240,
     228,   425,    -1,    67,    68,   399,   203,   215,    -1,   100,
       3,   216,     4,    -1,   217,    -1,   216,     5,   217,    -1,
     218,    -1,   378,    -1,    85,   103,    -1,    86,   221,   230,
      68,   226,   234,    -1,    -1,    27,    -1,    50,    -1,   223,
      -1,   222,     5,   223,    -1,   406,   182,   224,    -1,   378,
      -1,    47,    -1,    97,   240,    87,   222,   228,   425,    -1,
     227,    -1,   226,     5,   227,    -1,   276,    -1,    -1,   241,
      -1,    86,   221,   230,   234,    -1,   215,    -1,   191,    -1,
     274,    -1,    -1,   233,    -1,    -1,   163,   368,    -1,   162,
     368,   232,    -1,   235,   228,   242,   243,   318,   204,   231,
     425,    -1,    60,   236,    -1,   240,    -1,   236,     5,   240,
      -1,    -1,    31,    -1,    -1,   237,    24,   202,    -1,    -1,
     164,    -1,   239,   399,   238,    -1,   239,   273,   424,   203,
      -1,   354,    -1,   101,   250,    -1,    -1,    62,    37,   375,
      -1,    -1,    63,   250,    -1,    93,    -1,    56,    -1,    96,
      -1,    79,    -1,   251,    -1,   246,    -1,     3,   250,     4,
      -1,   378,    -1,   245,    -1,   245,    69,   208,   244,    -1,
     247,    -1,    23,   247,    -1,   248,    -1,   249,   179,   248,
      -1,   249,    -1,   250,   178,   249,    -1,   253,    -1,   256,
      -1,   267,    -1,   271,    -1,   272,    -1,   262,    -1,   265,
      -1,   259,    -1,   268,    -1,   254,   217,    -1,   217,   254,
     217,    -1,   254,   217,    -1,   184,    -1,   185,    -1,   182,
      -1,   183,    -1,   187,    -1,   186,    -1,    69,   208,    -1,
     208,    35,   217,   179,   217,    -1,   217,   255,    -1,   208,
      72,   379,   260,    -1,   208,    72,   364,   260,    -1,   217,
     257,    -1,   217,   258,    -1,   257,    -1,   258,    -1,    -1,
      53,   379,    -1,    69,   208,    79,    -1,   217,   261,    -1,
     261,    -1,   273,    -1,     3,   375,     4,    -1,   208,    64,
     263,    -1,   217,   264,    -1,   264,    -1,   254,   270,   273,
      -1,   217,   266,    -1,   217,   269,    -1,   208,   165,   290,
      -1,    30,    -1,    27,    -1,    88,    -1,    55,   273,    -1,
      95,   273,    -1,     3,   210,     4,    -1,   275,    -1,   274,
       5,   275,    -1,   398,    -1,   423,    -1,   171,    -1,    20,
      -1,    21,    -1,    22,    -1,    19,    -1,   277,   174,    -1,
     277,   171,    -1,   277,    20,    -1,   277,    22,    -1,    -1,
      31,   408,    -1,   408,    -1,   106,     3,   378,    64,   378,
       4,    -1,   106,     3,   375,     4,    -1,   279,    -1,   287,
      -1,   284,    -1,   105,     3,   378,     4,    -1,   108,     3,
     378,     4,    -1,    80,     3,   378,     4,    -1,   104,     3,
     378,     4,    -1,   281,    -1,   282,    -1,   283,    -1,   374,
      -1,   117,    -1,   285,    -1,   378,    -1,   113,     3,   286,
      60,   378,     4,    -1,   289,    -1,   277,    -1,   423,    -1,
      79,    -1,    56,    -1,    93,    -1,   128,    -1,   129,    -1,
     130,    -1,   131,    -1,   132,    -1,   133,    -1,   344,    -1,
     293,     3,     4,    -1,   291,     3,     4,    -1,   293,     3,
     377,     4,    -1,   292,     3,   377,     4,    -1,   293,     3,
     221,   376,     4,    -1,   294,    -1,   118,    -1,    24,    -1,
     111,    -1,   112,    -1,   296,   138,   316,    -1,   139,     3,
       4,    -1,   344,    -1,   297,    -1,   303,    -1,   309,    -1,
     312,    -1,   140,     3,   300,     4,    -1,   423,    -1,   277,
      -1,   299,    -1,   298,    -1,    -1,     5,   306,    -1,     5,
     306,     5,   307,    -1,    -1,   308,    -1,   304,     3,   305,
     301,     4,   302,    -1,   141,    -1,   142,    -1,   378,    -1,
      21,    -1,   378,    -1,   143,   145,    -1,   144,   145,    -1,
     310,     3,   378,     4,   302,    -1,   146,    -1,   147,    -1,
      -1,   314,    -1,   148,     3,   378,     5,   313,     4,   311,
     302,    -1,   299,    -1,   298,    -1,    60,   149,    -1,    60,
     150,    -1,    24,    -1,   315,    -1,   317,    -1,   323,    -1,
      -1,   319,    -1,   160,   320,    -1,   320,     5,   321,    -1,
     321,    -1,   322,    31,   323,    -1,   315,    -1,     3,   327,
       4,    -1,    -1,   328,    -1,    -1,   329,    -1,    -1,   333,
      -1,   324,   325,   204,   326,    -1,   315,    -1,   159,    37,
     330,    -1,   330,     5,   331,    -1,   331,    -1,   406,   407,
      -1,    -1,   343,    -1,   334,   335,   332,    -1,   158,    -1,
     157,    -1,   336,    -1,   338,    -1,   155,   156,    -1,   337,
      -1,    44,   127,    -1,   288,   156,    -1,    35,   339,   179,
     340,    -1,   341,    -1,   341,    -1,   336,    -1,   155,   154,
      -1,   342,    -1,   288,   154,    -1,   151,    44,   127,    -1,
     151,    62,    -1,   151,   153,    -1,   151,   161,   152,    -1,
     345,     3,   221,   376,     4,    -1,    42,     3,   191,     4,
      -1,    42,     3,   221,   376,     4,    -1,    34,    -1,    76,
      -1,    77,    -1,    91,    -1,   120,    -1,    30,    -1,    88,
      -1,    73,    -1,    74,    -1,    61,    -1,    81,   250,    -1,
     347,    -1,   355,    -1,    -1,    66,    -1,   346,    -1,   346,
      83,    -1,   240,    43,    71,   240,    -1,   240,    78,   349,
      71,   240,    -1,   240,   349,    71,   240,   348,    -1,   350,
      -1,   240,   349,    71,   240,    99,   399,   353,    -1,    -1,
      51,    -1,    52,    -1,   352,    -1,   351,    -1,    99,     3,
     200,     4,    -1,    -1,    27,    -1,   273,    -1,   378,    -1,
     167,    -1,   168,    -1,   169,    -1,   170,    -1,   171,    -1,
     172,    -1,   173,    -1,   174,    -1,   177,    -1,   175,    -1,
     176,    -1,    24,    -1,   359,    -1,    24,    13,    24,    -1,
     360,    25,    -1,   360,    -1,   361,    -1,    38,     3,   358,
      31,   362,     4,    -1,   288,    -1,   290,    -1,   406,    -1,
     357,    -1,   409,    -1,   295,    -1,     3,   378,     4,    -1,
     363,    -1,   364,    -1,   280,    -1,   365,    -1,   189,   365,
      -1,   188,   365,    -1,   366,    -1,   367,   191,   366,    -1,
     367,   192,   366,    -1,   367,   193,   366,    -1,   367,    -1,
     368,   188,   367,    -1,   368,   189,   367,    -1,   370,    -1,
     109,    -1,   110,    -1,   175,   379,    -1,   176,   379,    -1,
     369,    -1,   371,    -1,   372,    -1,   119,    -1,   116,    -1,
      46,    -1,   114,    -1,   115,    -1,   378,    -1,   375,     5,
     378,    -1,   375,     7,   378,    -1,   420,    -1,   376,    -1,
     377,     5,   376,    -1,   377,     7,   376,    -1,   368,    -1,
     379,    -1,   373,    -1,   380,    -1,   384,    -1,   381,    -1,
     380,   188,   384,    -1,   378,   190,   378,    -1,   174,    -1,
     385,    -1,    39,   399,    -1,   382,    -1,   382,   383,    -1,
     391,    -1,   386,    -1,   387,    -1,   107,     3,   388,    60,
     379,   392,     4,    -1,   389,    -1,   390,    -1,    -1,   393,
      -1,   395,    -1,   396,    -1,   397,    -1,    -1,    58,   378,
      -1,   107,     3,   378,    60,   378,   392,     4,    -1,   107,
       3,   375,     4,    -1,    98,    -1,    75,    -1,   394,     3,
     378,     4,    -1,    41,     3,   379,    99,   399,     4,    -1,
      41,     3,   358,     5,   362,     4,    -1,    92,     3,   379,
      99,   399,     4,    -1,   378,   278,    -1,   402,    -1,   401,
      -1,   400,    -1,    24,    13,   401,    -1,    24,     6,   401,
      -1,    24,    13,   402,    -1,    24,    -1,    -1,    25,    -1,
     405,    -1,   404,    13,   405,    -1,    24,   403,    -1,   191,
      -1,   404,    -1,    -1,   383,    -1,    24,    -1,   410,    -1,
     411,    -1,   412,    -1,   122,   422,   413,   419,   124,    -1,
     122,   417,   419,   124,    -1,   414,    -1,   417,   414,    -1,
     125,   415,   123,   420,    -1,   416,    -1,   415,     5,   416,
      -1,   218,    -1,   252,    -1,   255,    -1,   264,    -1,   257,
      -1,   261,    -1,   418,    -1,   417,   418,    -1,   125,   250,
     123,   420,    -1,    -1,   126,   420,    -1,   421,    -1,   378,
      -1,   218,    -1,     6,    24,    -1,     8,    -1,    -1,   237,
      24,    -1,   250,    -1,     3,   199,     4,    -1,    -1,   426,
      -1,   166,   427,    -1,   427,   428,    -1,   428,    -1,    24,
      -1,    24,   182,   429,    -1,   277,    -1,    24,    -1,   244,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   252,   252,   254,   262,   267,   272,   280,   285,   295,
     296,   304,   305,   316,   317,   327,   332,   340,   348,   357,
     358,   359,   363,   364,   370,   371,   372,   373,   374,   375,
     376,   380,   385,   396,   397,   398,   401,   408,   420,   429,
     439,   444,   452,   455,   460,   470,   481,   482,   483,   487,
     490,   496,   503,   504,   507,   519,   522,   528,   532,   533,
     541,   549,   553,   558,   561,   562,   565,   566,   574,   583,
     598,   608,   611,   617,   618,   622,   625,   634,   635,   639,
     646,   654,   657,   666,   667,   675,   676,   684,   685,   686,
     687,   690,   691,   692,   707,   715,   716,   726,   727,   735,
     736,   745,   746,   756,   757,   758,   759,   760,   761,   762,
     763,   764,   770,   777,   784,   809,   810,   811,   812,   813,
     814,   815,   823,   861,   869,   879,   889,   895,   901,   923,
     948,   949,   956,   965,   971,   987,   991,   999,  1008,  1014,
    1030,  1039,  1048,  1057,  1067,  1068,  1069,  1073,  1081,  1087,
    1098,  1103,  1110,  1114,  1118,  1119,  1120,  1121,  1122,  1124,
    1136,  1148,  1160,  1176,  1177,  1183,  1186,  1196,  1206,  1207,
    1208,  1211,  1219,  1230,  1240,  1250,  1255,  1260,  1267,  1272,
    1279,  1280,  1284,  1296,  1297,  1300,  1301,  1302,  1303,  1304,
    1305,  1306,  1307,  1308,  1309,  1312,  1313,  1320,  1327,  1335,
    1349,  1362,  1365,  1369,  1373,  1374,  1379,  1388,  1395,  1396,
    1397,  1398,  1399,  1402,  1412,  1415,  1418,  1419,  1422,  1423,
    1429,  1439,  1440,  1444,  1456,  1457,  1460,  1463,  1466,  1469,
    1470,  1473,  1484,  1485,  1488,  1489,  1492,  1506,  1507,  1510,
    1516,  1524,  1527,  1528,  1531,  1534,  1535,  1538,  1546,  1549,
    1554,  1563,  1566,  1575,  1576,  1579,  1580,  1583,  1584,  1587,
    1593,  1596,  1605,  1608,  1613,  1621,  1622,  1625,  1634,  1635,
    1638,  1639,  1642,  1648,  1649,  1657,  1665,  1675,  1678,  1681,
    1682,  1688,  1691,  1699,  1706,  1712,  1718,  1728,  1737,  1745,
    1757,  1758,  1759,  1760,  1761,  1762,  1763,  1767,  1772,  1777,
    1784,  1792,  1793,  1796,  1797,  1802,  1803,  1811,  1823,  1833,
    1842,  1847,  1861,  1862,  1863,  1866,  1867,  1870,  1882,  1883,
    1887,  1890,  1894,  1895,  1896,  1897,  1898,  1899,  1900,  1901,
    1902,  1903,  1904,  1905,  1909,  1914,  1924,  1933,  1934,  1938,
    1950,  1951,  1952,  1953,  1954,  1955,  1956,  1963,  1967,  1968,
    1972,  1973,  1979,  1988,  1989,  1996,  2003,  2013,  2014,  2021,
    2035,  2042,  2048,  2053,  2059,  2069,  2076,  2084,  2092,  2093,
    2094,  2095,  2096,  2101,  2104,  2108,  2121,  2148,  2151,  2155,
    2168,  2169,  2170,  2173,  2181,  2182,  2185,  2192,  2202,  2203,
    2206,  2214,  2215,  2223,  2224,  2227,  2234,  2247,  2271,  2278,
    2281,  2282,  2283,  2288,  2295,  2296,  2304,  2315,  2325,  2326,
    2329,  2339,  2349,  2361,  2374,  2383,  2388,  2393,  2400,  2407,
    2416,  2426,  2434,  2435,  2443,  2448,  2466,  2472,  2480,  2487,
    2488,  2498,  2502,  2506,  2507,  2511,  2522,  2532,  2537,  2544,
    2554,  2557,  2562,  2563,  2564,  2565,  2566,  2567,  2570,  2575,
    2582,  2592,  2593,  2601,  2605,  2608,  2612,  2618,  2626,  2629,
    2639,  2653,  2657,  2658,  2662,  2671,  2676,  2684,  2690,  2700,
    2701,  2702
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
  "SQL_TOKEN_FLOAT", "SQL_TOKEN_FOR", "SQL_TOKEN_FOUND", "SQL_TOKEN_FROM",
  "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP", "SQL_TOKEN_HAVING", "SQL_TOKEN_IN",
  "SQL_TOKEN_INDICATOR", "SQL_TOKEN_INNER", "SQL_TOKEN_INSERT",
  "SQL_TOKEN_INTO", "SQL_TOKEN_IS", "SQL_TOKEN_INTERSECT",
  "SQL_TOKEN_JOIN", "SQL_TOKEN_LIKE", "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT",
  "SQL_TOKEN_LOWER", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN", "SQL_TOKEN_NATURAL",
  "SQL_TOKEN_NULL", "SQL_TOKEN_OCTET_LENGTH", "SQL_TOKEN_ON",
  "SQL_TOKEN_ORDER", "SQL_TOKEN_OUTER", "SQL_TOKEN_REAL",
  "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_SELECT", "SQL_TOKEN_SET",
  "SQL_TOKEN_SOME", "SQL_TOKEN_SQLCODE", "SQL_TOKEN_SQLERROR",
  "SQL_TOKEN_SUM", "SQL_TOKEN_TRANSLATE", "SQL_TOKEN_TRUE",
  "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN",
  "SQL_TOKEN_UPDATE", "SQL_TOKEN_UPPER", "SQL_TOKEN_USING",
  "SQL_TOKEN_VALUES", "SQL_TOKEN_WHERE", "SQL_TOKEN_WITH",
  "SQL_TOKEN_WORK", "SQL_TOKEN_BIT_LENGTH", "SQL_TOKEN_CHAR_LENGTH",
  "SQL_TOKEN_POSITION", "SQL_TOKEN_SUBSTRING",
  "SQL_TOKEN_SQL_TOKEN_INTNUM", "SQL_TOKEN_CURRENT_DATE",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_CURDATE", "SQL_TOKEN_NOW",
  "SQL_TOKEN_EXTRACT", "SQL_TOKEN_HOUR", "SQL_TOKEN_MINUTE",
  "SQL_TOKEN_MONTH", "SQL_TOKEN_SECOND", "SQL_TOKEN_WEEK",
  "SQL_TOKEN_YEAR", "SQL_TOKEN_EVERY", "SQL_TOKEN_WITHIN",
  "SQL_TOKEN_CASE", "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN",
  "SQL_TOKEN_ELSE", "SQL_TOKEN_ROW", "SQL_TOKEN_VALUE",
  "SQL_TOKEN_CURRENT_CATALOG", "SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP",
  "SQL_TOKEN_CURRENT_PATH", "SQL_TOKEN_CURRENT_ROLE",
  "SQL_TOKEN_CURRENT_SCHEMA", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_VARBINARY",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_BIGI", "SQL_TOKEN_OVER",
  "SQL_TOKEN_ROW_NUMBER", "SQL_TOKEN_NTILE", "SQL_TOKEN_LEAD",
  "SQL_TOKEN_LAG", "SQL_TOKEN_RESPECT", "SQL_TOKEN_IGNORE",
  "SQL_TOKEN_NULLS", "SQL_TOKEN_FIRST_VALUE", "SQL_TOKEN_LAST_VALUE",
  "SQL_TOKEN_NTH_VALUE", "SQL_TOKEN_FIRST", "SQL_TOKEN_LAST",
  "SQL_TOKEN_EXCLUDE", "SQL_TOKEN_OTHERS", "SQL_TOKEN_TIES",
  "SQL_TOKEN_FOLLOWING", "SQL_TOKEN_UNBOUNDED", "SQL_TOKEN_PRECEDING",
  "SQL_TOKEN_RANGE", "SQL_TOKEN_ROWS", "SQL_TOKEN_PARTITION",
  "SQL_TOKEN_WINDOW", "SQL_TOKEN_NO", "SQL_TOKEN_LIMIT",
  "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY", "SQL_TOKEN_MATCH",
  "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_BINARY", "SQL_TOKEN_BOOLEAN",
  "SQL_TOKEN_DOUBLE", "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT",
  "SQL_TOKEN_LONG", "SQL_TOKEN_INT64", "SQL_TOKEN_STRING",
  "SQL_TOKEN_DATE", "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_DATETIME",
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
  "derived_column", "table_node", "catalog_name", "schema_name",
  "table_name", "opt_column_array_idx", "property_path",
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
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     124,    38,   419,   420,   421,   422,   423,   424,    43,    45,
     425,    42,    47,    37,   126,    61,   426
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   197,   198,   198,   199,   200,   200,   201,   201,   202,
     202,   203,   203,   204,   204,   205,   205,   206,   206,   207,
     207,   207,   208,   208,   209,   209,   209,   209,   209,   209,
     209,   210,   210,   211,   211,   211,   212,   213,   214,   215,
     216,   216,   217,   218,   219,   220,   221,   221,   221,   222,
     222,   223,   224,   224,   225,   226,   226,   227,   228,   228,
     229,   229,   230,   230,   231,   231,   232,   232,   233,   234,
     235,   236,   236,   237,   237,   238,   238,   239,   239,   240,
     240,   240,   241,   242,   242,   243,   243,   244,   244,   244,
     244,   245,   245,   245,   246,   247,   247,   248,   248,   249,
     249,   250,   250,   251,   251,   251,   251,   251,   251,   251,
     251,   251,   252,   253,   253,   254,   254,   254,   254,   254,
     254,   254,   255,   256,   257,   258,   259,   259,   259,   259,
     260,   260,   261,   262,   262,   263,   263,   264,   265,   265,
     266,   267,   268,   269,   270,   270,   270,   271,   272,   273,
     274,   274,   275,   276,   277,   277,   277,   277,   277,   277,
     277,   277,   277,   278,   278,   278,   279,   279,   280,   280,
     280,   281,   281,   282,   283,   284,   284,   284,   285,   285,
     286,   286,   287,   288,   288,   289,   289,   289,   289,   289,
     289,   289,   289,   289,   289,   290,   290,   290,   290,   290,
     290,   291,   292,   293,   294,   294,   295,   296,   296,   296,
     296,   296,   296,   297,   298,   299,   300,   300,   301,   301,
     301,   302,   302,   303,   304,   304,   305,   306,   307,   308,
     308,   309,   310,   310,   311,   311,   312,   313,   313,   314,
     314,   315,   316,   316,   317,   318,   318,   319,   320,   320,
     321,   322,   323,   324,   324,   325,   325,   326,   326,   327,
     328,   329,   330,   330,   331,   332,   332,   333,   334,   334,
     335,   335,   336,   336,   336,   337,   338,   339,   340,   341,
     341,   341,   342,   343,   343,   343,   343,   344,   344,   344,
     345,   345,   345,   345,   345,   345,   345,   346,   346,   346,
     347,   348,   348,   349,   349,   349,   349,   350,   351,   351,
     351,   352,   353,   353,   353,   354,   354,   355,   356,   356,
     357,   358,   359,   359,   359,   359,   359,   359,   359,   359,
     359,   359,   359,   359,   360,   360,   361,   362,   362,   363,
     364,   364,   364,   364,   364,   364,   364,   364,   365,   365,
     366,   366,   366,   367,   367,   367,   367,   368,   368,   368,
     369,   370,   370,   370,   370,   371,   372,   373,   374,   374,
     374,   374,   374,   375,   375,   375,   376,   377,   377,   377,
     378,   378,   378,   379,   380,   380,   381,   381,   382,   382,
     383,   384,   384,   385,   385,   386,   387,   388,   389,   390,
     391,   391,   391,   391,   392,   392,   393,   393,   394,   394,
     395,   396,   396,   397,   398,   399,   399,   399,   400,   400,
     401,   402,   403,   403,   404,   404,   405,   405,   406,   407,
     407,   408,   409,   410,   410,   411,   412,   413,   413,   414,
     415,   415,   416,   416,   416,   416,   416,   416,   417,   417,
     418,   419,   419,   420,   421,   422,   423,   423,   424,   424,
     199,   199,   425,   425,   426,   427,   427,   428,   428,   429,
     429,   429
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
       1,     1,     1,     1,     1,     3,     2,     1,     1,     6,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     2,     2,     1,     3,     3,     3,     1,     3,     3,
       1,     1,     1,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       2,     1,     2,     1,     1,     1,     7,     1,     1,     0,
       1,     1,     1,     1,     0,     2,     7,     4,     1,     1,
       4,     6,     6,     6,     2,     1,     1,     1,     3,     3,
       3,     1,     0,     1,     1,     3,     2,     1,     1,     0,
       1,     1,     1,     1,     1,     5,     4,     1,     2,     4,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       4,     0,     2,     1,     1,     1,     2,     1,     0,     2,
       1,     3,     0,     1,     2,     2,     1,     1,     3,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      22,    22,     0,   457,   158,   155,   156,   157,    22,   422,
     295,   290,     0,     0,     0,     0,     0,     0,   187,     0,
      22,   409,   291,   292,   186,     0,     0,    46,   296,   293,
       0,   188,     0,    77,   408,     0,     0,     0,     0,     0,
       0,   361,   362,   204,   205,     0,   202,   294,     0,   189,
     190,   191,   192,   193,   194,     0,     0,   224,   225,   232,
     233,     0,   154,   388,     0,     0,   117,   118,   115,   116,
     120,   119,     0,     0,   427,     0,     2,     0,     4,    30,
      24,    25,    26,    61,    22,    42,    27,    28,    29,    31,
      95,    92,    97,    99,   101,   460,    91,   103,     0,   104,
     128,   129,   110,   134,   108,   139,   109,   105,   111,   106,
     107,   320,   184,   168,   349,   175,   176,   177,   170,   169,
     340,   183,   341,     0,     0,     0,   201,   345,     0,   209,
     210,     0,   211,     0,   212,   195,     0,   343,   347,   348,
     350,   353,   357,   380,   365,   360,   366,   367,   382,    94,
     381,   383,   385,   391,   384,   389,   394,   395,   393,   400,
       0,   401,   402,   403,   428,   424,   342,   344,   432,   433,
     434,   185,     0,     0,     0,    43,   456,    22,    23,    98,
     423,   426,     0,    36,     0,    46,    77,     0,   147,     0,
     121,     0,    44,    47,    48,     0,     0,   148,    78,     0,
     303,   310,   316,   315,    81,     0,     0,     0,     0,   399,
       0,     0,     0,    22,   455,    43,   451,   448,     0,     0,
       0,     0,     0,   363,   364,   352,   351,     1,     3,     0,
       0,     0,     0,   123,   126,   127,   133,   138,   141,   142,
      35,    33,    34,   318,    22,    22,    22,   114,   161,   162,
     160,   159,     0,     0,     0,     0,     0,     0,    46,     0,
       0,     0,     0,     0,     0,     0,     0,   392,     0,     0,
     461,   149,    93,   346,    46,     0,     0,     0,   321,     0,
     381,     0,     0,    58,   421,    11,   417,   416,   415,   132,
       0,   427,     0,    63,   150,   163,   152,   381,   458,    75,
       0,   299,   304,   297,   298,   303,     0,   305,     0,     0,
      40,     0,     0,     0,   373,     0,   373,     0,   397,   398,
       0,   370,   371,   372,   369,   179,   368,   180,     0,   178,
     181,     0,     0,     0,   449,     0,    22,   451,   437,     0,
     207,   215,   217,   216,     0,   214,     0,     0,   137,   135,
     130,   130,     0,     0,   145,   144,   146,   113,     0,   319,
       0,     0,   100,   102,   197,   377,     0,   454,   376,   453,
     196,     0,     0,   253,   241,   242,   206,   243,   244,   218,
     226,     0,     0,   354,   355,   356,   358,   359,   387,   386,
     390,     0,   422,   425,     0,     0,     0,     0,   288,     0,
      22,   462,    59,     0,     0,     0,     0,   173,    77,     0,
      60,    58,     0,   431,     0,   414,   165,     0,    74,     0,
      11,     0,    79,    77,     0,    58,    49,     0,   306,    77,
      39,     0,   174,   171,   167,     0,     0,     0,   407,     0,
       0,   172,     0,     0,   452,   436,     0,    42,   443,     0,
     444,   128,   134,   139,     0,   440,     0,   438,   213,     0,
       0,   373,     0,   125,   124,     0,   203,   143,   195,   140,
      32,    88,    90,    87,    89,    96,   199,     0,     0,     0,
     198,   260,   255,     0,   254,     0,     0,   221,     0,   410,
       0,   333,   322,   323,   324,   325,   326,   327,   328,   329,
     331,   332,   330,   334,   337,   338,     0,     0,     0,   289,
      82,     0,    37,   463,     0,   419,   421,   418,   420,     0,
       8,    38,    70,    71,     0,    55,    57,   153,    83,   151,
     164,     0,   459,    80,     9,   307,    77,     0,   462,     0,
     303,    41,   374,   375,     0,   404,   381,     0,   450,   114,
      22,     0,   435,   238,   237,     0,   136,   131,     0,   378,
     379,   200,     0,    13,   256,   252,   227,   219,   221,     0,
       0,   231,   222,   287,     0,   336,   339,   412,   411,   467,
     464,   466,     0,    12,     0,    77,     0,    45,     0,    85,
     413,     0,    76,   308,    50,    54,    53,    51,    52,    22,
       0,   301,   309,   302,   166,     0,     0,     0,   182,     0,
     442,     0,   446,   447,   445,   441,   439,   234,   122,     0,
       0,   257,     0,   223,   229,   230,   335,     0,   465,   421,
       7,    72,    56,     0,    22,   245,     0,     6,   300,     0,
     312,   405,   406,   396,     0,   112,     0,   221,   235,   261,
     263,   429,    22,   269,   268,   259,   258,     0,   220,   228,
     470,   471,   469,   468,    84,    86,     0,    13,   246,    10,
       0,     0,   313,   314,   311,   239,   240,   236,     0,   430,
     264,    14,    15,    42,    19,     0,     0,     0,     0,   265,
     270,   273,   271,   251,   247,   249,     0,    64,     5,   317,
     262,    22,    20,    21,    18,    17,     0,     0,   279,     0,
     277,   281,   274,   272,   275,     0,   267,   266,     0,     0,
       0,   462,    65,    16,   280,   282,     0,     0,   284,   285,
       0,   248,   250,    66,    69,   276,   278,   283,   286,     0,
      68,    67
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    75,    76,   636,   519,   592,   406,   621,   681,   682,
     704,    77,    78,   275,   243,    80,    81,    82,    83,   309,
      84,    85,    86,    87,   195,   425,   426,   597,    88,   524,
     525,   401,    89,   292,   721,   740,   722,   410,   411,   522,
     419,   422,   199,   200,   402,   589,   635,   475,    90,    91,
      92,    93,    94,   332,    96,   448,    97,    98,   450,    99,
     100,   101,   102,   463,   103,   104,   348,   105,   106,   238,
     107,   108,   239,   358,   109,   110,   111,   293,   294,   526,
     112,   415,   113,   114,   115,   116,   117,   118,   327,   328,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   342,   343,   344,   486,   571,   130,   131,   379,   567,
     658,   572,   132,   133,   647,   134,   555,   648,   693,   376,
     377,   667,   668,   694,   695,   696,   378,   482,   563,   655,
     483,   484,   564,   649,   650,   716,   656,   657,   689,   708,
     691,   692,   709,   735,   710,   711,   717,   135,   136,   307,
     601,   602,   308,   201,   202,   203,   674,   204,   603,   360,
     137,   277,   503,   504,   505,   506,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   329,   313,   365,
     366,   215,   150,   151,   152,   153,   267,   154,   155,   156,
     157,   317,   318,   319,   158,   606,   159,   160,   161,   162,
     163,   296,   285,   286,   287,   288,   181,   164,   165,   166,
     680,   637,   167,   168,   169,   170,   337,   338,   454,   455,
     216,   217,   335,   368,   369,   218,   171,   420,   512,   513,
     580,   581,   663
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -631
static const yytype_int16 yypact[] =
{
     571,   571,    70,  -631,  -631,  -631,  -631,  -631,   945,    76,
    -631,  -631,    59,    11,   122,   137,    94,   143,  -631,    89,
     167,  -631,  -631,  -631,  -631,   192,    95,    50,  -631,  -631,
     236,  -631,   143,    97,  -631,   256,   269,   286,   293,   297,
     314,  -631,  -631,  -631,  -631,   322,  -631,  -631,  2173,  -631,
    -631,  -631,  -631,  -631,  -631,   327,   330,  -631,  -631,  -631,
    -631,   363,  -631,  -631,  2653,  2653,  -631,  -631,  -631,  -631,
    -631,  -631,  3109,  3109,  -631,   368,   369,    80,  -631,  -631,
    -631,  -631,  -631,  -631,   108,  -631,  -631,  -631,  -631,    13,
     312,  -631,  -631,  -631,   208,   211,  -631,  -631,  2653,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,    82,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,   387,   391,   393,  -631,  -631,   259,  -631,
    -631,   396,  -631,   397,  -631,   267,   403,  -631,  -631,  -631,
    -631,  -631,   111,    93,  -631,  -631,  -631,  -631,  -631,    58,
    -631,   219,  -631,   370,  -631,  -631,  -631,  -631,  -631,  -631,
     407,  -631,  -631,  -631,   401,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,   413,   418,    21,    27,  -631,   758,  -631,  -631,
    -631,  -631,  2653,  -631,  2653,     9,    97,     6,  -631,   409,
     346,  2653,  -631,  -631,  -631,  2813,  2653,  -631,  -631,    61,
     384,  -631,  -631,  -631,  -631,  2653,  2653,  2653,  2653,  2653,
    2653,  1693,  2013,  1132,  -631,   252,   150,  -631,   321,   443,
      67,  2653,   252,  -631,  -631,  -631,  -631,  -631,  -631,   445,
    2653,     8,  2333,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,   422,   167,  1132,  1132,  -631,  -631,  -631,
    -631,  -631,   448,  2653,  1853,    71,  2653,  2653,    50,  2961,
    2961,  2961,  2961,  2961,  2653,   277,   409,  -631,  2653,     2,
    -631,  -631,  -631,  -631,    50,   418,    21,   425,   252,   454,
     354,   456,  2653,   440,    85,   458,  -631,  -631,  -631,  -631,
      26,    87,    91,   459,  -631,     4,  -631,   371,   160,   160,
     394,  -631,  -631,  -631,  -631,   213,     2,   389,   402,   280,
    -631,    30,    34,   294,   -19,   319,    -8,   415,  -631,  -631,
      35,  -631,  -631,  -631,  -631,  -631,  -631,  -631,   416,  -631,
     252,    36,   -67,  2653,  -631,   343,  1132,   358,  -631,   321,
    -631,    82,  -631,  -631,   481,  -631,    43,  2013,  -631,  -631,
     -24,   -20,  2653,   378,  -631,  -631,  -631,  -631,   143,  -631,
       6,    60,  -631,   208,  -631,  -631,   324,   252,  -631,  -631,
    -631,  2653,   331,   462,  -631,  -631,  -631,  -631,  -631,   482,
     252,    37,  2653,  -631,  -631,  -631,   111,   111,  -631,  -631,
    -631,    38,   463,  -631,  2813,   139,   139,   409,  -631,   487,
    1132,   328,  -631,   469,   471,     2,   399,  -631,    97,   129,
    -631,   414,  2653,  -631,   476,  -631,  -631,   409,  -631,   479,
     458,   484,  -631,    97,   439,    44,  -631,   334,  -631,    97,
    -631,  2653,  -631,  -631,  -631,  2653,  2653,  2653,  -631,  2653,
    2653,  -631,  2653,  2653,  -631,  -631,    41,    52,  -631,  2653,
    -631,   512,   514,   515,    53,  -631,   398,  -631,  -631,    67,
     335,   252,  2653,  -631,  -631,   344,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  2653,  2653,   520,
    -631,  -631,   367,   521,  -631,   506,   524,   176,   525,  -631,
     472,   518,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,   508,  -631,   530,   532,   533,  -631,
     211,   519,  -631,  -631,   526,  -631,   526,  -631,  -631,   340,
    -631,  -631,   537,   350,    48,  -631,  -631,  -631,   483,  -631,
    -631,   534,  -631,  -631,   541,   350,    97,     2,   328,  2493,
     431,  -631,   252,   252,    40,   -11,    78,    42,  -631,    63,
    1506,  2653,  -631,  -631,  -631,   542,  -631,   357,  2653,  -631,
    -631,  -631,   511,   467,  -631,  -631,  -631,   545,   176,   406,
     408,  -631,  -631,  -631,   531,  -631,  -631,  -631,  -631,   372,
     519,  -631,   535,  -631,     2,    97,   129,  -631,   523,   493,
    -631,   476,  -631,   350,  -631,  -631,  -631,  -631,   252,  1132,
      96,  -631,  -631,  -631,  -631,  2653,   553,   554,  -631,    86,
    -631,  2653,  -631,  -631,  -631,  -631,  -631,   501,  -631,     2,
     527,   189,  2653,  -631,  -631,  -631,  -631,   336,  -631,  -631,
    -631,   350,  -631,  2653,  1132,   405,   349,  -631,   211,   476,
     310,   252,  -631,  -631,  2653,  -631,   215,   176,  -631,   557,
    -631,   370,  1319,  -631,  -631,  -631,  -631,  3140,  -631,   252,
    -631,  -631,    82,  -631,   155,   211,   462,   467,  -631,  -631,
     476,   366,  -631,  -631,  -631,  -631,  -631,  -631,     2,  -631,
    -631,   558,  -631,   273,    92,  3257,   441,   410,   411,   419,
    -631,  -631,  -631,  -631,   564,  -631,   540,   420,  -631,  -631,
    -631,  1319,  -631,  -631,  -631,  -631,    33,    47,  -631,   404,
    -631,  -631,  -631,  -631,  -631,    99,  -631,  -631,   462,   569,
    2961,   328,  -631,  -631,  -631,  -631,  3257,   446,  -631,  -631,
     423,  -631,  -631,   -92,  -631,  -631,  -631,  -631,  -631,  2961,
    -631,    93
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -631,  -631,   575,   -61,  -631,  -631,   161,   -87,  -631,  -117,
     -99,   -15,  -631,    20,  -631,  -631,  -631,  -631,   181,  -631,
     -90,   -48,  -631,  -631,  -125,  -631,    51,  -631,  -631,  -631,
      10,  -308,  -631,   195,  -631,  -631,  -631,    73,  -631,  -631,
     299,  -631,  -631,  -159,  -631,  -631,  -631,   -28,  -631,  -631,
     595,   362,   364,     3,  -591,  -631,  -631,   -73,   536,  -631,
     -70,   538,  -631,   257,   -69,  -631,  -631,   -68,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,     5,  -631,   202,  -631,
    -208,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -607,  -631,   262,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,   157,   158,  -631,  -631,  -513,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -185,  -631,
    -631,  -631,  -631,  -631,  -100,  -631,   -98,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,   -54,  -631,  -631,  -631,  -631,   -32,
    -631,  -631,  -631,  -631,   -97,  -631,  -631,   275,  -631,  -631,
    -631,  -631,   325,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,   447,  -631,  -631,  -631,   237,  -631,   412,   301,    90,
     116,  -630,  -631,  -631,  -631,  -631,  -631,  -631,  -199,  -259,
     382,     1,   -58,  -631,  -631,  -631,   -12,   376,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,    98,  -631,  -631,  -631,  -631,
    -631,  -631,  -182,  -631,   -18,  -380,  -631,  -631,   374,  -287,
    -631,  -282,  -631,  -631,  -631,  -631,  -631,   313,  -631,   104,
     437,  -165,   295,  -315,  -631,  -631,  -188,  -631,  -484,  -631,
    -631,    69,  -631
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -448
static const yytype_int16 yytable[] =
{
     214,   149,   175,    95,   174,   190,   223,   224,   247,   149,
     315,   232,   341,   416,   234,   236,   237,   299,   444,   427,
      79,   173,   188,   399,   518,   272,   392,   283,   413,   462,
     407,   273,   345,   462,   432,   414,   193,   197,   433,   441,
     273,   487,   489,   352,   604,   437,   608,   605,   459,   537,
     688,   334,   439,   586,   595,   623,   443,  -442,   550,   194,
     282,   684,   182,   -43,   187,   222,   222,   240,  -112,   231,
     375,   739,   229,     2,   373,     3,   352,   193,   707,  -203,
     230,   -43,  -404,   241,   390,   284,     4,     5,     6,     7,
     733,   403,   274,   -43,   176,   374,   262,   263,   404,   639,
     194,   180,   248,   528,   249,   229,    35,   242,   408,   741,
     684,   246,   479,   230,   183,   310,   471,   538,   520,   707,
     284,   352,   -43,   488,   702,   184,   280,   -43,   548,   371,
     -43,   178,   530,   382,   677,     2,   605,     3,   297,   472,
     185,   703,   357,   727,   229,   400,   187,   -62,   460,   394,
     229,   408,   230,   473,   186,   -62,   474,   189,   644,   409,
     435,   728,   436,   491,  -348,  -348,  -348,  -348,  -348,  -348,
    -381,   264,   351,   353,   334,  -442,   551,    20,   175,   264,
     276,   -43,   264,   278,   -73,   278,  -112,   724,   481,   713,
     178,   418,   290,    74,   264,   191,   295,   222,   192,   246,
     281,   725,   518,   714,   298,   -94,   -94,   311,   312,   314,
     316,   320,   330,   331,   149,   508,   264,   264,   559,   560,
     264,   527,   346,   -43,   264,   264,   264,   264,   264,   361,
     264,   222,   264,   264,   349,   531,   616,   734,    62,   196,
     -43,   -43,   -43,   -43,   -43,   -43,   149,   149,   264,   523,
     427,   341,   729,   250,   367,   367,   251,   380,   381,   205,
     730,   198,   465,   449,   535,   388,   451,   452,   453,   391,
     540,   345,   206,   -19,   301,   213,   333,   -19,   -19,   302,
     -19,   262,   263,   367,   430,   431,   303,   304,   447,   207,
      66,    67,    68,    69,    70,    71,   208,   630,   434,   435,
     209,   436,   259,   260,   261,   702,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,   502,   210,    14,   569,
     570,   446,   703,   438,   435,   211,   436,   -19,   476,   477,
     219,   478,   651,   220,   367,   480,   477,   149,   478,   556,
     435,   541,   436,   -19,   583,   584,   653,   654,   461,   383,
     384,   385,    21,   669,   670,     4,     5,     6,     7,   549,
     660,   672,   673,   469,   675,   676,   221,   -19,   227,    30,
     699,   670,   367,   225,   226,    34,   228,   593,   386,   387,
     470,   244,   546,   367,    39,   515,   517,   245,   698,   246,
     252,   651,   471,   300,   253,   295,   254,   255,   527,   256,
     257,   149,   466,   510,   557,  -208,   258,   265,    10,   266,
     268,   301,    11,   295,   269,   472,   302,   270,   640,   662,
      15,  -303,   271,   303,   304,   289,   631,   300,   305,   473,
     -19,   -19,   474,   284,   664,   -19,   542,   543,   544,   -19,
     545,   222,   264,   547,   367,   301,   336,   340,   347,   359,
     302,    63,   364,   397,    22,    23,   395,   303,   304,   396,
     398,   405,   305,   222,   412,   423,    28,   445,   618,    29,
     417,   306,   428,   429,   300,   440,   442,   611,   367,   367,
     612,   613,   614,   300,   333,   458,   374,   485,   180,    43,
      44,   509,   301,   514,   511,   516,    46,   302,    47,    35,
     413,   301,   610,   532,   303,   304,   302,    62,   534,   305,
     536,  -303,   599,   303,   304,   400,   539,  -446,   305,  -447,
    -445,   645,   552,   558,   561,   565,   562,   566,   568,   573,
     600,   574,   408,   575,   576,   609,   577,   578,   590,   582,
     598,   400,   585,   579,   591,   588,   617,  -381,   619,   620,
     622,   624,   367,   625,   627,   626,   634,   642,   643,   629,
     633,   646,   678,   701,   652,   666,   713,   714,   712,   718,
     715,   719,   373,   737,     1,   738,   172,     2,   671,     3,
     697,   533,   720,   726,   723,   705,   351,   521,   594,   490,
       4,     5,     6,     7,     8,     9,   632,   587,   421,   661,
     149,    10,   638,   179,   683,    11,   641,   362,   464,    12,
     363,    13,    14,    15,   529,   467,   553,   554,   731,    16,
     233,   732,   235,   659,   700,   690,    17,    18,   468,   736,
     424,   279,   456,   507,   461,   149,   372,   665,    19,   679,
      20,   389,   350,   393,   607,   222,    21,    22,    23,   628,
      24,    25,   457,   683,   615,   339,    26,    27,     0,    28,
       0,     0,    29,    30,    31,     0,    32,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,     0,     0,     0,    46,
       0,    47,     0,    48,     0,     0,     0,     0,     0,    49,
      50,    51,    52,    53,    54,     0,     0,     0,     0,     0,
      55,    56,    57,    58,     0,     0,     0,    59,    60,    61,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    62,     0,     0,    63,    64,    65,     0,     0,
       0,     0,     0,    66,    67,    68,    69,    70,    71,    72,
      73,   177,    74,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,    21,    22,    23,     0,    24,    25,     0,
       0,     0,     0,     0,   274,     0,    28,     0,     0,    29,
      30,    31,     0,    32,     0,     0,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,     0,     0,    46,     0,    47,     0,
      48,     0,     0,     0,     0,     0,    49,    50,    51,    52,
      53,    54,     0,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    62,
       0,     0,    63,    64,    65,     0,     0,     0,     0,     0,
      66,    67,    68,    69,    70,    71,    72,    73,   177,    74,
       0,     2,     0,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     5,     6,     7,   178,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,    11,
     -23,     0,     0,    12,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    20,     0,     0,     0,     0,     0,
      21,    22,    23,     0,    24,    25,     0,     0,     0,     0,
       0,     0,     0,    28,     0,     0,    29,    30,    31,     0,
      32,     0,     0,    34,     0,     0,     0,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,     0,     0,    46,     0,    47,     0,    48,     0,     0,
       0,     0,     0,    49,    50,    51,    52,    53,    54,     0,
       0,     0,     0,     0,    55,    56,    57,    58,     0,     0,
       0,    59,    60,    61,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,    63,
      64,    65,     0,     0,     0,     0,     0,    66,    67,    68,
      69,    70,    71,    72,    73,   177,    74,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,    14,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,    21,    22,    23,
       0,    24,    25,     0,     0,     0,     0,     0,     0,     0,
      28,     0,     0,    29,    30,    31,     0,    32,     0,     0,
      34,     0,     0,     0,     0,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     0,     0,     0,     0,
      46,     0,    47,     0,    48,     0,     0,     0,     0,     0,
      49,    50,    51,    52,    53,    54,     0,     0,     0,     0,
       0,    55,    56,    57,    58,     0,     0,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,    63,    64,    65,     0,
       0,     0,     0,     0,    66,    67,    68,    69,    70,    71,
      72,    73,   212,    74,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,   178,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    11,     0,     0,     0,    12,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,     0,     0,     0,    21,    22,    23,     0,    24,    25,
       0,     0,     0,     0,     0,     0,     0,    28,     0,     0,
      29,    30,    31,     0,    32,     0,     0,    34,     0,     0,
       0,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,     0,     0,     0,     0,    46,     0,    47,
       0,    48,     0,     0,     0,     0,     0,    49,    50,    51,
      52,    53,    54,     0,     0,     0,     0,     0,    55,    56,
      57,    58,     0,     0,     0,    59,    60,    61,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,    63,    64,    65,     0,     0,     0,     0,
       0,    66,    67,    68,    69,    70,    71,    72,    73,   212,
      74,     0,     2,     0,     3,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,   178,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
      11,     0,     0,     0,    12,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,    21,    22,    23,     0,    24,    25,     0,     0,     0,
       0,     0,     0,     0,    28,     0,     0,    29,    30,    31,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       0,     0,     0,     0,    46,     0,    47,     0,    48,     0,
       0,     0,     0,     0,    49,    50,    51,    52,    53,    54,
       0,     0,     0,     0,     0,    55,    56,    57,    58,     0,
       0,     0,    59,    60,    61,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
      63,    64,    65,     0,     0,     0,     0,     0,    66,    67,
      68,    69,    70,    71,    72,    73,   212,    74,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,   322,   323,   324,
     325,    46,   326,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,   370,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
     193,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,   194,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,   274,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,   213,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
     354,    72,    73,   355,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,   356,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
     596,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     2,
       0,     3,     0,     0,    62,     0,     0,    63,    64,    65,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
       0,    72,    73,    10,    74,     0,     0,    11,     0,     0,
       0,    12,     0,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      23,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    29,    30,    31,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     0,
       0,    46,     0,    47,     0,    48,     0,     0,     0,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
      60,    61,     0,     0,   212,     0,     0,     2,     0,     3,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,    62,     9,     0,    63,    64,    65,
       0,    10,     0,     0,     0,    11,     0,     0,     0,    12,
       0,    72,    73,    15,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,     0,
      24,    25,     0,     0,     0,     0,     0,     0,     0,    28,
       0,     0,    29,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,    37,    38,     0,    40,
       0,     0,    43,    44,    45,     0,     0,     0,     0,    46,
       0,    47,     0,    48,     0,     0,     0,     0,     0,    49,
      50,    51,    52,    53,    54,     0,     0,     0,     0,     0,
      55,    56,    57,    58,     0,     0,     0,    59,    60,    61,
       0,     0,   212,     0,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,    62,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    11,     0,     0,     2,    12,     3,    72,
      73,    15,    74,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   685,     0,     0,     0,     0,
       0,     0,     0,     0,   686,    22,    23,     0,    24,    25,
       0,     0,     0,     0,     0,     0,    18,    28,     0,     0,
      29,     0,    31,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,    38,     0,    40,     0,    24,
      43,    44,    45,     0,     0,     0,     0,    46,     0,    47,
       0,    48,     0,    31,     0,     0,     0,    49,    50,    51,
      52,    53,    54,     0,     0,     0,     0,     0,    55,    56,
      57,    58,     0,     0,     0,    59,    60,    61,     0,     0,
       0,     0,     0,     2,     0,     3,     0,     0,    49,    50,
      51,    52,    53,    54,     0,     0,     4,     5,     6,     7,
      62,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   687,     0,     0,     0,     0,
      74,   686,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    62,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,    50,    51,    52,    53,
      54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   706,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    62
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-631)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      48,     0,     1,     0,     1,    20,    64,    65,    98,     8,
     209,    84,   220,   295,    84,    84,    84,   199,   333,   306,
       0,     1,    17,   282,   404,     4,    24,   186,    24,    53,
       4,     4,   220,    53,     4,    31,    27,    32,     4,     4,
       4,     4,     4,    35,     4,    64,     4,    58,     5,     5,
     657,   216,    60,     5,   538,   568,   123,     5,     5,    50,
     185,   652,     3,     5,     3,    64,    65,    54,     5,    84,
     255,   163,    64,     6,     3,     8,    35,    27,   685,     3,
      72,    23,     4,    70,   266,    24,    19,    20,    21,    22,
     720,     6,    86,    35,    24,    24,   188,   189,    13,     3,
      50,    25,    20,   411,    22,    64,   100,    94,    60,   739,
     701,   178,   371,    72,   103,   205,    56,   425,   405,   726,
      24,    35,    64,   382,    32,     3,   184,    69,   443,   254,
      72,    23,   414,   258,   647,     6,    58,     8,   196,    79,
       3,    49,   232,    44,    64,   101,     3,    60,   347,   274,
      64,    60,    72,    93,    60,    68,    96,    68,    72,    68,
       5,    62,     7,    24,   188,   189,   190,   191,   192,   193,
     190,   190,   230,   165,   339,   123,   123,    69,   177,   190,
     177,   123,   190,   182,    24,   184,   123,   154,   373,   156,
      23,    31,   191,   191,   190,     3,   195,   196,   103,   178,
     191,   154,   582,   156,   199,   178,   179,   206,   207,   208,
     209,   210,   211,   212,   213,   397,   190,   190,   477,   478,
     190,   409,   221,   165,   190,   190,   190,   190,   190,   244,
     190,   230,   190,   190,   229,   417,   551,   721,   171,     3,
     182,   183,   184,   185,   186,   187,   245,   246,   190,   408,
     537,   459,   153,   171,   253,   254,   174,   256,   257,     3,
     161,   164,   352,   336,   423,   264,   336,   336,   336,   268,
     429,   459,     3,     0,    61,   125,   126,     4,     5,    66,
       7,   188,   189,   282,     4,     5,    73,    74,   336,     3,
     182,   183,   184,   185,   186,   187,     3,   584,     4,     5,
       3,     7,   191,   192,   193,    32,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,     3,    41,   143,
     144,   336,    49,     4,     5,     3,     7,    54,     4,     5,
       3,     7,   619,     3,   333,     4,     5,   336,     7,     4,
       5,   431,     7,    70,     4,     5,   157,   158,   347,   259,
     260,   261,    75,     4,     5,    19,    20,    21,    22,   449,
      24,    51,    52,   358,   149,   150,     3,    94,     0,    92,
       4,     5,   371,    72,    73,    98,     7,   536,   262,   263,
     360,    69,   440,   382,   107,   403,   404,   179,   670,   178,
       3,   678,    56,    43,     3,   394,     3,   138,   586,     3,
       3,   400,    24,   400,   462,   138,     3,   188,    30,    39,
       3,    61,    34,   412,    13,    79,    66,     4,   600,   627,
      42,    71,     4,    73,    74,    79,   585,    43,    78,    93,
     157,   158,    96,    24,   633,   162,   435,   436,   437,   166,
     439,   440,   190,   442,   443,    61,   125,     4,     3,    27,
      66,   174,     4,    99,    76,    77,    31,    73,    74,     5,
       4,     3,    78,   462,     5,    71,    88,   124,   558,    91,
      99,    87,    83,    71,    43,    60,    60,   550,   477,   478,
     550,   550,   550,    43,   126,     4,    24,     5,    25,   111,
     112,     4,    61,    24,   166,    24,   118,    66,   120,   100,
      24,    61,   550,    24,    73,    74,    66,   171,    24,    78,
      71,    71,    81,    73,    74,   101,   182,     5,    78,     5,
       5,   611,   124,   179,     4,     4,   159,    21,     4,     4,
      99,    13,    60,    25,     4,   550,     4,     4,     4,    13,
     539,   101,     5,    24,     3,    62,     4,   190,    37,    82,
       5,   145,   551,   145,   182,    24,    63,     4,     4,    24,
      37,    60,     5,     5,    37,   160,   156,   156,   127,     5,
     151,    31,     3,   127,     3,   152,     1,     6,   639,     8,
     667,   420,   162,   179,   701,   684,   644,   406,   537,   394,
      19,    20,    21,    22,    23,    24,   586,   524,   299,   627,
     599,    30,   599,     8,   652,    34,   605,   245,   351,    38,
     246,    40,    41,    42,   412,   353,   459,   459,   718,    48,
      84,   719,    84,   622,   678,   657,    55,    56,   353,   726,
     305,   184,   337,   396,   633,   634,   254,   634,    67,   651,
      69,   265,   230,   269,   546,   644,    75,    76,    77,   580,
      79,    80,   339,   701,   550,   218,    85,    86,    -1,    88,
      -1,    -1,    91,    92,    93,    -1,    95,    -1,    97,    98,
      -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,    -1,   118,
      -1,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,    -1,    -1,    -1,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   171,    -1,    -1,   174,   175,   176,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,     3,   191,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    -1,    79,    80,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    88,    -1,    -1,    91,
      92,    93,    -1,    95,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,    -1,    -1,    -1,    -1,   118,    -1,   120,    -1,
     122,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,    -1,    -1,    -1,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,   174,   175,   176,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,     3,   191,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,
      35,    -1,    -1,    38,    -1,    -1,    41,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    -1,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    91,    92,    93,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    -1,   118,    -1,   120,    -1,   122,    -1,    -1,
      -1,    -1,    -1,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,    -1,    -1,
      -1,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,   174,
     175,   176,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,     3,   191,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,
      38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    91,    92,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    -1,    -1,    -1,    -1,
     118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   171,    -1,    -1,   174,   175,   176,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,     3,   191,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,
      41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    -1,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      91,    92,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    -1,    -1,    -1,    -1,   118,    -1,   120,
      -1,   122,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,    -1,    -1,    -1,   146,   147,   148,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,   174,   175,   176,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,     3,
     191,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    -1,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    91,    92,    93,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
      -1,    -1,    -1,    -1,   118,    -1,   120,    -1,   122,    -1,
      -1,    -1,    -1,    -1,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,    -1,
      -1,    -1,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
     174,   175,   176,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,     3,   191,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      27,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,   125,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      27,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   171,    -1,    -1,   174,   175,   176,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   188,   189,    30,   191,    -1,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,   118,    -1,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,    -1,    -1,    -1,   146,
     147,   148,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,   171,    24,    -1,   174,   175,   176,
      -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    38,
      -1,   188,   189,    42,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    -1,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    91,    -1,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,   106,    -1,   108,
      -1,    -1,   111,   112,   113,    -1,    -1,    -1,    -1,   118,
      -1,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,    -1,    -1,    -1,   146,   147,   148,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,   171,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,     6,    38,     8,   188,
     189,    42,   191,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    76,    77,    -1,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    88,    -1,    -1,
      91,    -1,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,   105,   106,    -1,   108,    -1,    79,
     111,   112,   113,    -1,    -1,    -1,    -1,   118,    -1,   120,
      -1,   122,    -1,    93,    -1,    -1,    -1,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,    -1,    -1,    -1,   146,   147,   148,    -1,    -1,
      -1,    -1,    -1,     6,    -1,     8,    -1,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,    19,    20,    21,    22,
     171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,    -1,    -1,
     191,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      30,    34,    38,    40,    41,    42,    48,    55,    56,    67,
      69,    75,    76,    77,    79,    80,    85,    86,    88,    91,
      92,    93,    95,    97,    98,   100,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   118,   120,   122,   128,
     129,   130,   131,   132,   133,   139,   140,   141,   142,   146,
     147,   148,   171,   174,   175,   176,   182,   183,   184,   185,
     186,   187,   188,   189,   191,   198,   199,   208,   209,   210,
     212,   213,   214,   215,   217,   218,   219,   220,   225,   229,
     245,   246,   247,   248,   249,   250,   251,   253,   254,   256,
     257,   258,   259,   261,   262,   264,   265,   267,   268,   271,
     272,   273,   277,   279,   280,   281,   282,   283,   284,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     303,   304,   309,   310,   312,   344,   345,   357,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   378,
     379,   380,   381,   382,   384,   385,   386,   387,   391,   393,
     394,   395,   396,   397,   404,   405,   406,   409,   410,   411,
     412,   423,   199,   210,   250,   378,    24,     3,    23,   247,
      25,   403,     3,   103,     3,     3,    60,     3,   273,    68,
     208,     3,   103,    27,    50,   221,     3,   273,   164,   239,
     240,   350,   351,   352,   354,     3,     3,     3,     3,     3,
       3,     3,     3,   125,   218,   378,   417,   418,   422,     3,
       3,     3,   378,   379,   379,   365,   365,     0,     7,    64,
      72,   208,   254,   255,   257,   258,   261,   264,   266,   269,
      54,    70,    94,   211,    69,   179,   178,   217,    20,    22,
     171,   174,     3,     3,     3,   138,     3,     3,     3,   191,
     192,   193,   188,   189,   190,   188,    39,   383,     3,    13,
       4,     4,     4,     4,    86,   210,   250,   358,   378,   358,
     379,   191,   221,   240,    24,   399,   400,   401,   402,    79,
     378,   191,   230,   274,   275,   378,   398,   379,   273,   399,
      43,    61,    66,    73,    74,    78,    87,   346,   349,   216,
     217,   378,   378,   375,   378,   375,   378,   388,   389,   390,
     378,    46,   114,   115,   116,   117,   119,   285,   286,   374,
     378,   378,   250,   126,   418,   419,   125,   413,   414,   417,
       4,   277,   298,   299,   300,   423,   378,     3,   263,   273,
     364,   379,    35,   165,    27,    30,    88,   217,   270,    27,
     356,   208,   248,   249,     4,   376,   377,   378,   420,   421,
       4,   221,   377,     3,    24,   315,   316,   317,   323,   305,
     378,   378,   221,   366,   366,   366,   367,   367,   378,   384,
     399,   378,    24,   405,   221,    31,     5,    99,     4,   376,
     101,   228,   241,     6,    13,     3,   203,     4,    60,    68,
     234,   235,     5,    24,    31,   278,   408,    99,    31,   237,
     424,   237,   238,    71,   349,   222,   223,   406,    83,    71,
       4,     5,     4,     4,     4,     5,     7,    64,     4,    60,
      60,     4,    60,   123,   420,   124,   208,   218,   252,   254,
     255,   257,   261,   264,   415,   416,   419,   414,     4,     5,
     375,   378,    53,   260,   260,   217,    24,   290,   344,   273,
     210,    56,    79,    93,    96,   244,     4,     5,     7,   376,
       4,   315,   324,   327,   328,     5,   301,     4,   376,     4,
     230,    24,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   359,   360,   361,   362,   362,   399,     4,
     250,   166,   425,   426,    24,   401,    24,   401,   402,   201,
     406,   215,   236,   240,   226,   227,   276,   423,   228,   275,
     408,   399,    24,   203,    24,   240,    71,     5,   228,   182,
     240,   217,   378,   378,   378,   378,   379,   378,   420,   217,
       5,   123,   124,   298,   299,   313,     4,   379,   179,   376,
     376,     4,   159,   325,   329,     4,    21,   306,     4,   143,
     144,   302,   308,     4,    13,    25,     4,     4,     4,    24,
     427,   428,    13,     4,     5,     5,     5,   234,    62,   242,
       4,     3,   202,   240,   223,   425,    47,   224,   378,    81,
      99,   347,   348,   355,     4,    58,   392,   392,     4,   208,
     218,   254,   257,   261,   264,   416,   420,     4,   217,    37,
      82,   204,     5,   302,   145,   145,    24,   182,   428,    24,
     406,   240,   227,    37,    63,   243,   200,   408,   250,     3,
     399,   378,     4,     4,    72,   217,    60,   311,   314,   330,
     331,   406,    37,   157,   158,   326,   333,   334,   307,   378,
      24,   244,   277,   429,   375,   250,   160,   318,   319,     4,
       5,   200,    51,    52,   353,   149,   150,   302,     5,   383,
     407,   205,   206,   218,   251,    35,    44,   155,   288,   335,
     336,   337,   338,   315,   320,   321,   322,   204,   408,     4,
     331,     5,    32,    49,   207,   207,   155,   288,   336,   339,
     341,   342,   127,   156,   156,   151,   332,   343,     5,    31,
     162,   231,   233,   206,   154,   154,   179,    44,    62,   153,
     161,   321,   323,   368,   425,   340,   341,   127,   152,   163,
     232,   368
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
    break;

  case 11:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 12:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));}
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
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
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

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));}
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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

  case 121:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 122:

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

  case 129:

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

  case 135:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 136:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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

  case 160:

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

  case 161:

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

  case 162:

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
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 167:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 171:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 172:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 173:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 174:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 196:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 197:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 198:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 199:

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

  case 200:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 213:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 218:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 219:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 220:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
    }
    break;

  case 231:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (8)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (8)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(5) - (8)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (8)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 288:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 289:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 318:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 334:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 335:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 336:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 339:

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

  case 346:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 351:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 352:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 354:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 355:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 356:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 358:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 359:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 360:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
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

  case 373:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 374:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 375:

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
            if (xxx_pGLOBAL_SQLPARSER->inPredicateCheck())
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
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(2) - (7)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (7)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(7) - (7)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 410:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 411:

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

  case 412:

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

  case 413:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 418:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 419:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 420:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 423:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 424:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 425:

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

  case 426:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 427:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 428:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 429:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 435:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 436:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 437:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 438:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 439:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 440:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 441:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 448:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 449:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 450:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 451:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 452:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 456:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 457:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 458:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 459:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 460:

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

  case 462:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 464:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 465:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 466:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 467:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 468:

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
