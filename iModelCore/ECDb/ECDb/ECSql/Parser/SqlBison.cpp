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
     SQL_TOKEN_ANY = 267,
     SQL_TOKEN_AS = 268,
     SQL_TOKEN_ASC = 269,
     SQL_TOKEN_AVG = 270,
     SQL_TOKEN_BETWEEN = 271,
     SQL_TOKEN_BY = 272,
     SQL_TOKEN_CAST = 273,
     SQL_TOKEN_COMMIT = 274,
     SQL_TOKEN_COUNT = 275,
     SQL_TOKEN_CROSS = 276,
     SQL_TOKEN_DEFAULT = 277,
     SQL_TOKEN_DELETE = 278,
     SQL_TOKEN_DESC = 279,
     SQL_TOKEN_DISTINCT = 280,
     SQL_TOKEN_FORWARD = 281,
     SQL_TOKEN_BACKWARD = 282,
     SQL_TOKEN_ESCAPE = 283,
     SQL_TOKEN_EXCEPT = 284,
     SQL_TOKEN_EXISTS = 285,
     SQL_TOKEN_FALSE = 286,
     SQL_TOKEN_FROM = 287,
     SQL_TOKEN_FULL = 288,
     SQL_TOKEN_GROUP = 289,
     SQL_TOKEN_HAVING = 290,
     SQL_TOKEN_IN = 291,
     SQL_TOKEN_INNER = 292,
     SQL_TOKEN_INSERT = 293,
     SQL_TOKEN_INTO = 294,
     SQL_TOKEN_IS = 295,
     SQL_TOKEN_INTERSECT = 296,
     SQL_TOKEN_JOIN = 297,
     SQL_TOKEN_LIKE = 298,
     SQL_TOKEN_LEFT = 299,
     SQL_TOKEN_RIGHT = 300,
     SQL_TOKEN_MAX = 301,
     SQL_TOKEN_MIN = 302,
     SQL_TOKEN_NATURAL = 303,
     SQL_TOKEN_NULL = 304,
     SQL_TOKEN_ON = 305,
     SQL_TOKEN_ORDER = 306,
     SQL_TOKEN_OUTER = 307,
     SQL_TOKEN_ROLLBACK = 308,
     SQL_TOKEN_SELECT = 309,
     SQL_TOKEN_SET = 310,
     SQL_TOKEN_SOME = 311,
     SQL_TOKEN_SUM = 312,
     SQL_TOKEN_TRUE = 313,
     SQL_TOKEN_UNION = 314,
     SQL_TOKEN_UNIQUE = 315,
     SQL_TOKEN_UNKNOWN = 316,
     SQL_TOKEN_UPDATE = 317,
     SQL_TOKEN_USING = 318,
     SQL_TOKEN_VALUE = 319,
     SQL_TOKEN_VALUES = 320,
     SQL_TOKEN_WHERE = 321,
     SQL_TOKEN_CURRENT_DATE = 322,
     SQL_TOKEN_CURRENT_TIME = 323,
     SQL_TOKEN_CURRENT_TIMESTAMP = 324,
     SQL_TOKEN_EVERY = 325,
     SQL_TOKEN_CASE = 326,
     SQL_TOKEN_THEN = 327,
     SQL_TOKEN_END = 328,
     SQL_TOKEN_WHEN = 329,
     SQL_TOKEN_ELSE = 330,
     SQL_TOKEN_LIMIT = 331,
     SQL_TOKEN_OFFSET = 332,
     SQL_TOKEN_ONLY = 333,
     SQL_TOKEN_MATCH = 334,
     SQL_TOKEN_ECSQLOPTIONS = 335,
     SQL_TOKEN_INTEGER = 336,
     SQL_TOKEN_INT = 337,
     SQL_TOKEN_INT64 = 338,
     SQL_TOKEN_LONG = 339,
     SQL_TOKEN_BOOLEAN = 340,
     SQL_TOKEN_DOUBLE = 341,
     SQL_TOKEN_REAL = 342,
     SQL_TOKEN_FLOAT = 343,
     SQL_TOKEN_STRING = 344,
     SQL_TOKEN_VARCHAR = 345,
     SQL_TOKEN_BINARY = 346,
     SQL_TOKEN_BLOB = 347,
     SQL_TOKEN_DATE = 348,
     SQL_TOKEN_TIME = 349,
     SQL_TOKEN_TIMESTAMP = 350,
     SQL_TOKEN_OR = 351,
     SQL_TOKEN_AND = 352,
     SQL_EQUAL = 353,
     SQL_GREAT = 354,
     SQL_LESS = 355,
     SQL_NOTEQUAL = 356,
     SQL_GREATEQ = 357,
     SQL_LESSEQ = 358,
     SQL_CONCAT = 359,
     SQL_TOKEN_INVALIDSYMBOL = 360
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
#define YYFINAL  162
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1654

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  131
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  160
/* YYNRULES -- Number of rules.  */
#define YYNRULES  337
/* YYNRULES -- Number of states.  */
#define YYNSTATES  507

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   360

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   127,   115,     2,
       3,     4,   125,   122,     5,   123,    13,   126,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   129,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   114,    12,   128,     2,     2,     2,
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
     111,   112,   113,   116,   117,   118,   119,   120,   121,   124,
     130
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    14,    16,    20,    22,
      23,    27,    28,    32,    33,    37,    39,    43,    46,    49,
      50,    52,    54,    55,    57,    59,    61,    63,    65,    67,
      69,    71,    73,    78,    80,    82,    84,    86,    92,    98,
     103,   105,   109,   111,   113,   115,   122,   123,   125,   127,
     129,   133,   137,   139,   146,   148,   152,   154,   155,   157,
     162,   164,   166,   168,   169,   171,   172,   175,   179,   187,
     190,   192,   196,   197,   199,   200,   204,   205,   207,   211,
     216,   218,   221,   222,   226,   227,   230,   232,   234,   236,
     238,   240,   242,   246,   248,   250,   255,   257,   260,   262,
     266,   268,   272,   274,   276,   278,   280,   282,   284,   286,
     288,   290,   293,   297,   300,   302,   304,   306,   308,   310,
     312,   315,   321,   324,   329,   334,   337,   340,   342,   344,
     345,   348,   352,   355,   357,   359,   363,   367,   370,   372,
     376,   379,   382,   386,   388,   390,   392,   395,   398,   402,
     404,   408,   410,   412,   414,   416,   418,   420,   422,   425,
     428,   431,   434,   435,   438,   440,   442,   444,   446,   448,
     450,   452,   454,   458,   463,   469,   471,   477,   482,   488,
     490,   492,   494,   496,   498,   500,   502,   504,   506,   508,
     511,   513,   515,   516,   518,   520,   523,   528,   534,   540,
     542,   550,   551,   553,   555,   557,   559,   564,   565,   567,
     569,   571,   573,   575,   577,   579,   581,   583,   585,   587,
     589,   591,   593,   595,   597,   599,   601,   603,   605,   609,
     612,   614,   616,   623,   625,   627,   629,   631,   633,   637,
     639,   641,   643,   646,   649,   651,   655,   659,   663,   665,
     669,   673,   675,   677,   679,   681,   684,   687,   690,   692,
     694,   696,   698,   702,   704,   706,   710,   712,   714,   716,
     718,   720,   722,   726,   730,   732,   734,   737,   739,   741,
     745,   749,   753,   755,   757,   759,   763,   767,   770,   771,
     775,   776,   778,   780,   784,   787,   789,   791,   793,   795,
     797,   799,   805,   810,   812,   815,   820,   822,   826,   828,
     830,   832,   834,   836,   838,   840,   843,   848,   849,   852,
     854,   856,   858,   861,   863,   864,   867,   869,   873,   874,
     876,   879,   882,   884,   886,   890,   892,   894
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     132,     0,    -1,   133,    -1,   133,     7,    -1,   143,    -1,
     134,     5,   269,    -1,   269,    -1,   135,     5,   268,    -1,
     268,    -1,    -1,     3,   134,     4,    -1,    -1,     3,   135,
       4,    -1,    -1,    67,    33,   139,    -1,   140,    -1,   139,
       5,   140,    -1,   185,   141,    -1,   152,   141,    -1,    -1,
      30,    -1,    40,    -1,    -1,    23,    -1,   146,    -1,   147,
      -1,   148,    -1,   153,    -1,   154,    -1,   159,    -1,   144,
      -1,   163,    -1,   163,   145,   229,   144,    -1,    57,    -1,
      75,    -1,    45,    -1,    35,    -1,    39,    48,   174,   162,
     286,    -1,    54,    55,   257,   137,   149,    -1,    81,     3,
     150,     4,    -1,   151,    -1,   150,     5,   151,    -1,   152,
      -1,   250,    -1,    69,    -1,    70,   155,   164,    55,   160,
     168,    -1,    -1,    27,    -1,    41,    -1,   157,    -1,   156,
       5,   157,    -1,   268,   116,   158,    -1,   250,    -1,    78,
     174,    71,   156,   162,   286,    -1,   161,    -1,   160,     5,
     161,    -1,   210,    -1,    -1,   175,    -1,    70,   155,   164,
     168,    -1,   149,    -1,   125,    -1,   208,    -1,    -1,   167,
      -1,    -1,    93,   241,    -1,    92,   241,   166,    -1,   169,
     162,   176,   177,   138,   165,   286,    -1,    48,   170,    -1,
     174,    -1,   170,     5,   174,    -1,    -1,    29,    -1,    -1,
     171,    24,   136,    -1,    -1,    94,    -1,   173,   261,   172,
      -1,   173,   207,   285,   137,    -1,   227,    -1,    82,   184,
      -1,    -1,    50,    33,   247,    -1,    -1,    51,   184,    -1,
      74,    -1,    47,    -1,    77,    -1,    65,    -1,   185,    -1,
     180,    -1,     3,   184,     4,    -1,   250,    -1,   179,    -1,
     179,    56,   142,   178,    -1,   181,    -1,    23,   181,    -1,
     182,    -1,   183,   113,   182,    -1,   183,    -1,   184,   112,
     183,    -1,   187,    -1,   190,    -1,   201,    -1,   205,    -1,
     206,    -1,   196,    -1,   199,    -1,   193,    -1,   202,    -1,
     188,   151,    -1,   151,   188,   151,    -1,   188,   151,    -1,
     118,    -1,   119,    -1,   116,    -1,   117,    -1,   121,    -1,
     120,    -1,    56,   142,    -1,   142,    32,   151,   113,   151,
      -1,   151,   189,    -1,   142,    59,   251,   194,    -1,   142,
      59,   237,   194,    -1,   151,   191,    -1,   151,   192,    -1,
     191,    -1,   192,    -1,    -1,    44,   251,    -1,    56,   142,
      65,    -1,   151,   195,    -1,   195,    -1,   207,    -1,     3,
     247,     4,    -1,   142,    52,   197,    -1,   151,   198,    -1,
     198,    -1,   188,   204,   207,    -1,   151,   200,    -1,   151,
     203,    -1,   142,    95,   215,    -1,    28,    -1,    27,    -1,
      72,    -1,    46,   207,    -1,    76,   207,    -1,     3,   144,
       4,    -1,   209,    -1,   208,     5,   209,    -1,   256,    -1,
     284,    -1,    98,    -1,    20,    -1,    21,    -1,    22,    -1,
      19,    -1,   211,   105,    -1,   211,    98,    -1,   211,    20,
      -1,   211,    22,    -1,    -1,    29,   269,    -1,   269,    -1,
     214,    -1,   211,    -1,   284,    -1,    65,    -1,    47,    -1,
      74,    -1,   217,    -1,   216,     3,     4,    -1,   216,     3,
     249,     4,    -1,   216,     3,   155,   248,     4,    -1,    24,
      -1,   218,     3,   155,   248,     4,    -1,    36,     3,   125,
       4,    -1,    36,     3,   155,   248,     4,    -1,    31,    -1,
      62,    -1,    63,    -1,    73,    -1,    86,    -1,    28,    -1,
      72,    -1,    60,    -1,    61,    -1,    49,    -1,    66,   184,
      -1,   220,    -1,   228,    -1,    -1,    53,    -1,   219,    -1,
     219,    68,    -1,   174,    37,    58,   174,    -1,   174,    64,
     222,    58,   174,    -1,   174,   222,    58,   174,   221,    -1,
     223,    -1,   174,   222,    58,   174,    79,   261,   226,    -1,
      -1,    42,    -1,    43,    -1,   225,    -1,   224,    -1,    79,
       3,   134,     4,    -1,    -1,    27,    -1,   207,    -1,   250,
      -1,   107,    -1,   108,    -1,   101,    -1,   102,    -1,   104,
      -1,    98,    -1,    97,    -1,    99,    -1,   100,    -1,   103,
      -1,   105,    -1,   109,    -1,   110,    -1,   111,    -1,   106,
      -1,    24,    -1,   232,    -1,    24,    13,    24,    -1,   233,
      25,    -1,   233,    -1,   234,    -1,    34,     3,   231,    29,
     235,     4,    -1,   213,    -1,   215,    -1,   268,    -1,   230,
      -1,   270,    -1,     3,   250,     4,    -1,   236,    -1,   237,
      -1,   238,    -1,   123,   238,    -1,   122,   238,    -1,   239,
      -1,   240,   125,   239,    -1,   240,   126,   239,    -1,   240,
     127,   239,    -1,   240,    -1,   241,   122,   240,    -1,   241,
     123,   240,    -1,   243,    -1,    83,    -1,    85,    -1,    84,
      -1,   109,   251,    -1,   111,   251,    -1,   110,   251,    -1,
     242,    -1,   244,    -1,   245,    -1,   250,    -1,   247,     5,
     250,    -1,   281,    -1,   248,    -1,   249,     5,   248,    -1,
     241,    -1,   251,    -1,   246,    -1,   252,    -1,   255,    -1,
     253,    -1,   252,   122,   255,    -1,   250,   124,   250,    -1,
     105,    -1,   254,    -1,   250,   212,    -1,   259,    -1,   258,
      -1,    24,    13,   259,    -1,    24,    13,   260,    -1,    24,
       6,   260,    -1,    24,    -1,   262,    -1,   263,    -1,   262,
      13,   263,    -1,   262,     6,   263,    -1,    24,   264,    -1,
      -1,     3,   249,     4,    -1,    -1,    25,    -1,   267,    -1,
     266,    13,   267,    -1,    24,   265,    -1,   125,    -1,   266,
      -1,    24,    -1,   271,    -1,   272,    -1,   273,    -1,    87,
     283,   274,   280,    89,    -1,    87,   278,   280,    89,    -1,
     275,    -1,   278,   275,    -1,    90,   276,    88,   281,    -1,
     277,    -1,   276,     5,   277,    -1,   152,    -1,   186,    -1,
     189,    -1,   198,    -1,   191,    -1,   195,    -1,   279,    -1,
     278,   279,    -1,    90,   184,    88,   281,    -1,    -1,    91,
     281,    -1,   282,    -1,   250,    -1,   152,    -1,     6,    24,
      -1,     8,    -1,    -1,   171,    24,    -1,   184,    -1,     3,
     133,     4,    -1,    -1,   287,    -1,    96,   288,    -1,   288,
     289,    -1,   289,    -1,    24,    -1,    24,   116,   290,    -1,
     211,    -1,    24,    -1,   178,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   213,   213,   215,   220,   225,   230,   238,   243,   253,
     254,   262,   263,   274,   275,   285,   290,   298,   306,   315,
     316,   317,   321,   322,   328,   329,   330,   331,   332,   333,
     334,   338,   343,   354,   355,   356,   359,   366,   378,   387,
     397,   402,   410,   413,   418,   427,   438,   439,   440,   444,
     447,   453,   460,   463,   475,   478,   484,   488,   489,   497,
     505,   509,   514,   517,   518,   521,   522,   530,   539,   553,
     563,   566,   572,   573,   577,   580,   589,   590,   594,   601,
     609,   612,   621,   622,   632,   633,   641,   642,   643,   644,
     647,   648,   649,   664,   672,   673,   683,   684,   692,   693,
     702,   703,   713,   714,   715,   716,   717,   718,   719,   720,
     721,   727,   734,   741,   766,   767,   768,   769,   770,   771,
     772,   780,   790,   798,   808,   818,   824,   830,   852,   877,
     878,   885,   894,   900,   916,   920,   928,   937,   943,   959,
     968,   977,   986,   996,   997,   998,  1002,  1010,  1016,  1027,
    1032,  1039,  1043,  1047,  1048,  1049,  1050,  1051,  1053,  1065,
    1077,  1089,  1105,  1106,  1112,  1116,  1117,  1120,  1121,  1122,
    1123,  1126,  1127,  1134,  1143,  1157,  1162,  1171,  1179,  1191,
    1192,  1193,  1194,  1195,  1196,  1197,  1201,  1206,  1211,  1218,
    1226,  1227,  1230,  1231,  1236,  1237,  1245,  1257,  1267,  1276,
    1281,  1295,  1296,  1297,  1300,  1301,  1304,  1316,  1317,  1321,
    1324,  1328,  1329,  1330,  1331,  1332,  1333,  1334,  1335,  1336,
    1337,  1338,  1339,  1340,  1341,  1342,  1343,  1347,  1352,  1362,
    1371,  1372,  1376,  1388,  1389,  1390,  1391,  1392,  1393,  1400,
    1404,  1408,  1409,  1415,  1424,  1425,  1432,  1439,  1449,  1450,
    1457,  1466,  1473,  1478,  1483,  1488,  1494,  1500,  1509,  1516,
    1524,  1532,  1535,  1541,  1545,  1550,  1558,  1559,  1560,  1563,
    1567,  1568,  1571,  1578,  1588,  1591,  1595,  1605,  1610,  1617,
    1626,  1634,  1643,  1651,  1659,  1664,  1669,  1677,  1686,  1687,
    1697,  1698,  1706,  1711,  1729,  1735,  1743,  1754,  1758,  1762,
    1763,  1767,  1778,  1788,  1793,  1800,  1810,  1813,  1818,  1819,
    1820,  1821,  1822,  1823,  1826,  1831,  1838,  1848,  1849,  1857,
    1861,  1865,  1869,  1875,  1883,  1886,  1896,  1910,  1914,  1915,
    1919,  1928,  1933,  1941,  1947,  1957,  1958,  1959
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
  "SQL_TOKEN_ANY", "SQL_TOKEN_AS", "SQL_TOKEN_ASC", "SQL_TOKEN_AVG",
  "SQL_TOKEN_BETWEEN", "SQL_TOKEN_BY", "SQL_TOKEN_CAST",
  "SQL_TOKEN_COMMIT", "SQL_TOKEN_COUNT", "SQL_TOKEN_CROSS",
  "SQL_TOKEN_DEFAULT", "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC",
  "SQL_TOKEN_DISTINCT", "SQL_TOKEN_FORWARD", "SQL_TOKEN_BACKWARD",
  "SQL_TOKEN_ESCAPE", "SQL_TOKEN_EXCEPT", "SQL_TOKEN_EXISTS",
  "SQL_TOKEN_FALSE", "SQL_TOKEN_FROM", "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP",
  "SQL_TOKEN_HAVING", "SQL_TOKEN_IN", "SQL_TOKEN_INNER",
  "SQL_TOKEN_INSERT", "SQL_TOKEN_INTO", "SQL_TOKEN_IS",
  "SQL_TOKEN_INTERSECT", "SQL_TOKEN_JOIN", "SQL_TOKEN_LIKE",
  "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN",
  "SQL_TOKEN_NATURAL", "SQL_TOKEN_NULL", "SQL_TOKEN_ON", "SQL_TOKEN_ORDER",
  "SQL_TOKEN_OUTER", "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_SELECT",
  "SQL_TOKEN_SET", "SQL_TOKEN_SOME", "SQL_TOKEN_SUM", "SQL_TOKEN_TRUE",
  "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE", "SQL_TOKEN_UNKNOWN",
  "SQL_TOKEN_UPDATE", "SQL_TOKEN_USING", "SQL_TOKEN_VALUE",
  "SQL_TOKEN_VALUES", "SQL_TOKEN_WHERE", "SQL_TOKEN_CURRENT_DATE",
  "SQL_TOKEN_CURRENT_TIME", "SQL_TOKEN_CURRENT_TIMESTAMP",
  "SQL_TOKEN_EVERY", "SQL_TOKEN_CASE", "SQL_TOKEN_THEN", "SQL_TOKEN_END",
  "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE", "SQL_TOKEN_LIMIT",
  "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY", "SQL_TOKEN_MATCH",
  "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT",
  "SQL_TOKEN_INT64", "SQL_TOKEN_LONG", "SQL_TOKEN_BOOLEAN",
  "SQL_TOKEN_DOUBLE", "SQL_TOKEN_REAL", "SQL_TOKEN_FLOAT",
  "SQL_TOKEN_STRING", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_DATE", "SQL_TOKEN_TIME",
  "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_OR", "SQL_TOKEN_AND", "'|'", "'&'",
  "SQL_EQUAL", "SQL_GREAT", "SQL_LESS", "SQL_NOTEQUAL", "SQL_GREATEQ",
  "SQL_LESSEQ", "'+'", "'-'", "SQL_CONCAT", "'*'", "'/'", "'%'", "'~'",
  "'='", "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql_single_statement",
  "sql", "column_commalist", "column_ref_commalist",
  "opt_column_commalist", "opt_column_ref_commalist",
  "opt_order_by_clause", "ordering_spec_commalist", "ordering_spec",
  "opt_asc_desc", "sql_not", "manipulative_statement", "select_statement",
  "union_op", "commit_statement", "delete_statement_searched",
  "insert_statement", "values_or_query_spec",
  "row_value_constructor_commalist", "row_value_constructor",
  "row_value_constructor_elem", "rollback_statement",
  "select_statement_into", "opt_all_distinct", "assignment_commalist",
  "assignment", "update_source", "update_statement_searched",
  "target_commalist", "target", "opt_where_clause",
  "single_select_statement", "selection", "opt_limit_offset_clause",
  "opt_offset", "limit_offset_clause", "table_exp", "from_clause",
  "table_ref_commalist", "opt_as", "table_primary_as_range_column",
  "opt_only", "table_ref", "where_clause", "opt_group_by_clause",
  "opt_having_clause", "truth_value", "boolean_primary", "unary_predicate",
  "boolean_test", "boolean_factor", "boolean_term", "search_condition",
  "predicate", "comparison_predicate_part_2", "comparison_predicate",
  "comparison", "between_predicate_part_2", "between_predicate",
  "character_like_predicate_part_2", "other_like_predicate_part_2",
  "like_predicate", "opt_escape", "null_predicate_part_2", "test_for_null",
  "in_predicate_value", "in_predicate_part_2", "in_predicate",
  "quantified_comparison_predicate_part_2", "all_or_any_predicate",
  "rtreematch_predicate", "rtreematch_predicate_part_2", "any_all_some",
  "existence_test", "unique_test", "subquery", "scalar_exp_commalist",
  "select_sublist", "parameter_ref", "literal", "as_clause",
  "unsigned_value_spec", "general_value_spec", "fct_spec", "function_name",
  "general_set_fct", "set_fct_type", "outer_join_type", "join_condition",
  "join_spec", "join_type", "cross_union", "qualified_join",
  "ecrelationship_join", "op_relationship_direction", "joined_table",
  "named_columns_join", "all", "scalar_subquery", "cast_operand",
  "cast_target_primitive_type", "cast_target_scalar", "cast_target_array",
  "cast_target", "cast_spec", "value_exp_primary", "num_primary", "factor",
  "term", "num_value_exp", "datetime_primary", "datetime_value_fct",
  "datetime_factor", "datetime_term", "datetime_value_exp",
  "value_exp_commalist", "function_arg", "function_args_commalist",
  "value_exp", "string_value_exp", "char_value_exp", "concatenation",
  "char_primary", "char_factor", "derived_column", "table_node",
  "tablespace_qualified_class_name", "qualified_class_name", "class_name",
  "table_node_with_opt_member_func_call", "table_node_path",
  "table_node_path_entry", "opt_member_function_args",
  "opt_column_array_idx", "property_path", "property_path_entry",
  "column_ref", "column", "case_expression", "case_specification",
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
     349,   350,   351,   352,   124,    38,   353,   354,   355,   356,
     357,   358,    43,    45,   359,    42,    47,    37,   126,    61,
     360
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   131,   132,   132,   133,   134,   134,   135,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     141,   141,   142,   142,   143,   143,   143,   143,   143,   143,
     143,   144,   144,   145,   145,   145,   146,   147,   148,   149,
     150,   150,   151,   152,   153,   154,   155,   155,   155,   156,
     156,   157,   158,   159,   160,   160,   161,   162,   162,   163,
     163,   164,   164,   165,   165,   166,   166,   167,   168,   169,
     170,   170,   171,   171,   172,   172,   173,   173,   174,   174,
     174,   175,   176,   176,   177,   177,   178,   178,   178,   178,
     179,   179,   179,   180,   181,   181,   182,   182,   183,   183,
     184,   184,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   186,   187,   187,   188,   188,   188,   188,   188,   188,
     188,   189,   190,   191,   192,   193,   193,   193,   193,   194,
     194,   195,   196,   196,   197,   197,   198,   199,   199,   200,
     201,   202,   203,   204,   204,   204,   205,   206,   207,   208,
     208,   209,   210,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   212,   212,   212,   213,   213,   214,   214,   214,
     214,   215,   215,   215,   215,   216,   217,   217,   217,   218,
     218,   218,   218,   218,   218,   218,   219,   219,   219,   220,
     221,   221,   222,   222,   222,   222,   223,   224,   224,   224,
     225,   226,   226,   226,   227,   227,   228,   229,   229,   230,
     231,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   233,   233,   234,
     235,   235,   236,   237,   237,   237,   237,   237,   237,   237,
     238,   239,   239,   239,   240,   240,   240,   240,   241,   241,
     241,   242,   243,   243,   243,   243,   243,   243,   244,   245,
     246,   247,   247,   248,   249,   249,   250,   250,   250,   251,
     252,   252,   253,   253,   254,   255,   256,   257,   257,   258,
     259,   259,   260,   261,   262,   262,   262,   263,   264,   264,
     265,   265,   266,   266,   267,   267,   268,   269,   270,   271,
     271,   272,   273,   274,   274,   275,   276,   276,   277,   277,
     277,   277,   277,   277,   278,   278,   279,   280,   280,   281,
     282,   283,   284,   284,   285,   285,   133,   133,   286,   286,
     287,   288,   288,   289,   289,   290,   290,   290
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     3,     1,     3,     1,     0,
       3,     0,     3,     0,     3,     1,     3,     2,     2,     0,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     5,     5,     4,
       1,     3,     1,     1,     1,     6,     0,     1,     1,     1,
       3,     3,     1,     6,     1,     3,     1,     0,     1,     4,
       1,     1,     1,     0,     1,     0,     2,     3,     7,     2,
       1,     3,     0,     1,     0,     3,     0,     1,     3,     4,
       1,     2,     0,     3,     0,     2,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     4,     1,     2,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       2,     5,     2,     4,     4,     2,     2,     1,     1,     0,
       2,     3,     2,     1,     1,     3,     3,     2,     1,     3,
       2,     2,     3,     1,     1,     1,     2,     2,     3,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     4,     5,     1,     5,     4,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     0,     1,     1,     2,     4,     5,     5,     1,
       7,     0,     1,     1,     1,     1,     4,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     6,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     2,     2,     1,     3,     3,     3,     1,     3,
       3,     1,     1,     1,     1,     2,     2,     2,     1,     1,
       1,     1,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     3,     3,     1,     1,     2,     1,     1,     3,
       3,     3,     1,     1,     1,     3,     3,     2,     0,     3,
       0,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       1,     5,     4,     1,     2,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     4,     0,     2,     1,
       1,     1,     2,     1,     0,     2,     1,     3,     0,     1,
       2,     2,     1,     1,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      22,    22,     0,   323,   157,   154,   155,   156,    22,   290,
     184,   179,     0,    36,     0,     0,     0,   169,     0,    22,
     180,   181,   168,    44,    46,   185,   182,   170,     0,    76,
       0,   252,   254,   253,   183,     0,   153,   274,     0,     0,
       0,   116,   117,   114,   115,   119,   118,     0,     0,   295,
       0,     2,     0,     4,    30,    24,    25,    26,    60,    22,
      42,    27,    28,    29,    31,    94,    91,    96,    98,   100,
     326,    90,   102,     0,   103,   127,   128,   109,   133,   107,
     138,   108,   104,   110,   105,   106,   209,   166,   233,   165,
     234,     0,   171,     0,   236,   239,   240,   241,   244,   248,
     266,   258,   251,   259,   260,   268,    93,   267,   269,   271,
     275,   270,   296,   292,   235,   237,   298,   299,   300,   167,
       0,     0,     0,    43,   322,    22,    23,    97,   291,   294,
       0,    46,    76,     0,   146,     0,   120,    47,    48,     0,
     147,    77,     0,   192,   199,   205,   204,    80,     0,     0,
      22,   321,    43,   317,   314,     0,     0,   255,   257,   256,
     243,   242,     1,     3,     0,     0,     0,     0,   122,   125,
     126,   132,   137,   140,   141,    35,    33,    34,   207,    22,
      22,    22,   113,   160,   161,   159,   158,     0,    46,     0,
       0,     0,     0,     0,     0,     0,     0,   327,   148,    92,
     238,    46,     0,     0,     0,   210,     0,     0,    57,     0,
      11,   278,   277,   131,   295,     0,    62,   149,   162,   151,
     288,   324,    74,   283,   284,     0,   188,   193,   186,   187,
     192,     0,   194,     0,     0,    40,     0,     0,     0,   315,
       0,    22,   317,   303,     0,     0,   136,   134,   129,   129,
       0,     0,   144,   143,   145,   112,     0,   208,     0,     0,
      99,   101,   172,     0,   264,     0,   320,   263,   319,     0,
     245,   246,   247,   249,   250,   273,   272,   290,   293,     0,
       0,   177,     0,    22,   328,    58,     0,     0,     0,     0,
      76,     0,    59,    57,     0,   297,     0,   276,   164,     0,
     287,    73,     0,    11,     0,    78,     0,     0,    76,     0,
      57,    49,     0,   195,    76,    39,     0,     0,   318,   302,
       0,    42,   309,     0,   310,   127,   133,   138,     0,   306,
       0,   304,     0,   261,     0,   124,   123,     0,   175,   142,
     139,    32,    87,    89,    86,    88,    95,     0,   173,     0,
       0,     0,   226,   217,   216,   218,   219,   213,   214,   220,
     215,   221,   225,   211,   212,   222,   223,   224,   227,   230,
     231,     0,   178,    81,     0,    37,   329,   282,   281,   282,
     279,   280,     0,     8,    38,    69,    70,     0,    54,    56,
     152,    82,   150,   163,     0,   325,    79,     9,   286,   285,
     196,    76,     0,   328,     0,   192,    41,   316,   113,    22,
       0,   301,   135,     0,   130,     0,   174,   265,   176,     0,
     229,   232,   333,   330,   332,     0,    12,     0,    76,     0,
      45,     0,    84,   289,     0,    75,   197,    50,    53,    51,
      52,    22,     0,   190,   198,   191,     0,   308,     0,   312,
     313,   311,   307,   305,   262,   121,   228,     0,   331,     7,
      71,    55,     0,    22,    13,     0,     6,   189,     0,   201,
       0,   111,   336,   337,   335,   334,    83,    85,     0,    63,
      10,     0,     0,   202,   203,   200,    22,     0,   328,    64,
       5,   206,    14,    15,    42,    19,    65,    68,    22,    20,
      21,    18,    17,     0,    67,    16,    66
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    50,    51,   465,   382,   435,   289,   479,   492,   493,
     501,    52,    53,   202,   178,    55,    56,    57,    58,   234,
      59,    60,    61,    62,   139,   310,   311,   439,    63,   387,
     388,   284,    64,   215,   488,   504,   489,   292,   293,   385,
     302,   305,   142,   143,   285,   432,   464,   346,    65,    66,
      67,    68,    69,   237,    71,   322,    72,    73,   324,    74,
      75,    76,    77,   335,    78,    79,   246,    80,    81,   173,
      82,    83,   174,   256,    84,    85,    86,   216,   217,   389,
      87,   297,    88,    89,    90,    91,    92,    93,   232,   443,
     444,   233,   144,   145,   146,   485,   147,   445,   258,    94,
     204,   368,   369,   370,   371,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   332,   264,   265,   152,
     107,   108,   109,   110,   111,   219,   210,   211,   212,   381,
     222,   223,   224,   300,   129,   112,   113,   114,   466,   115,
     116,   117,   118,   242,   243,   328,   329,   153,   154,   240,
     267,   268,   155,   119,   303,   375,   376,   423,   424,   475
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -438
static const yytype_int16 yypact[] =
{
     425,   425,    63,  -438,  -438,  -438,  -438,  -438,   667,    39,
    -438,  -438,    56,  -438,    97,    94,   101,  -438,    89,   140,
    -438,  -438,  -438,  -438,    35,  -438,  -438,  -438,   101,    91,
     170,  -438,  -438,  -438,  -438,  1297,  -438,  -438,  1441,  1441,
    1441,  -438,  -438,  -438,  -438,  -438,  -438,   353,   353,  -438,
     179,   183,    -1,  -438,  -438,  -438,  -438,  -438,  -438,   130,
    -438,  -438,  -438,  -438,    22,   146,  -438,  -438,  -438,    98,
     110,  -438,  -438,  1441,  -438,  -438,  -438,  -438,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,  -438,    11,  -438,  -438,
    -438,   221,  -438,   239,  -438,  -438,  -438,  -438,  -438,    79,
      86,  -438,  -438,  -438,  -438,  -438,    96,  -438,   122,  -438,
    -438,  -438,   243,  -438,  -438,  -438,  -438,  -438,  -438,  -438,
     261,   274,    25,    21,  -438,   546,  -438,  -438,  -438,  -438,
    1441,    -5,    91,    81,  -438,   256,   203,  -438,  -438,  1513,
    -438,  -438,    50,   244,  -438,  -438,  -438,  -438,  1441,  1225,
     788,  -438,   158,   128,  -438,   194,   158,  -438,  -438,  -438,
    -438,  -438,  -438,  -438,   284,  1441,    43,  1369,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,  -438,  -438,   271,   140,
     788,   788,  -438,  -438,  -438,  -438,  -438,  1153,    35,    49,
      49,    49,    49,    49,  1441,   191,     6,  -438,  -438,  -438,
    -438,    35,   274,    25,   277,   158,   310,  1441,   523,    85,
     313,  -438,  -438,  -438,    55,   106,   320,  -438,    17,  -438,
     326,    84,    84,   151,  -438,   287,  -438,  -438,  -438,  -438,
     117,     6,   278,   289,   224,  -438,    19,   -51,  1441,  -438,
     253,   788,   257,  -438,   194,  1225,  -438,  -438,   196,   -18,
    1441,   264,  -438,  -438,  -438,  -438,   101,  -438,    81,   156,
    -438,    98,  -438,  1441,  -438,   230,   158,  -438,  -438,  1441,
    -438,  -438,  -438,    79,    79,  -438,  -438,   327,  -438,  1513,
    1543,  -438,   347,   788,   258,  -438,   331,   333,     6,   281,
      91,    48,  -438,   285,  1441,  -438,   344,  -438,  -438,  1441,
    -438,  -438,   345,   313,   346,  -438,   356,   356,    91,   308,
      42,  -438,   255,  -438,    91,  -438,  1441,  1441,  -438,  -438,
     108,    29,  -438,  1441,  -438,   377,   378,   385,    30,  -438,
     303,  -438,   248,   158,  1441,  -438,  -438,   280,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,  -438,   390,  -438,  1441,
     392,   349,   388,  -438,  -438,  -438,  -438,  -438,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,  -438,  -438,  -438,   373,
    -438,   401,  -438,   110,   383,  -438,  -438,  -438,  -438,   176,
    -438,  -438,   269,  -438,  -438,   406,   206,    40,  -438,  -438,
    -438,   362,  -438,  -438,   286,  -438,  -438,   411,  -438,  -438,
     206,    91,     6,   258,  1441,   342,  -438,  -438,    38,  1030,
    1441,  -438,  -438,  1441,   293,  1441,  -438,  -438,  -438,   395,
    -438,  -438,   304,   383,  -438,   331,  -438,     6,    91,    48,
    -438,   389,   372,  -438,   344,  -438,   206,  -438,  -438,  -438,
     158,   788,    57,  -438,  -438,  -438,   124,  -438,  1441,  -438,
    -438,  -438,  -438,  -438,   158,  -438,  -438,   311,  -438,  -438,
     206,  -438,  1441,   788,   357,   298,  -438,   110,   344,   267,
    1441,  -438,  -438,  -438,    11,  -438,   424,   110,   397,   340,
    -438,   344,   307,  -438,  -438,  -438,   909,    49,   258,  -438,
    -438,  -438,   429,  -438,   232,    59,   -30,  -438,   909,  -438,
    -438,  -438,  -438,    49,  -438,  -438,    86
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -438,  -438,   434,   -32,  -438,  -438,   135,  -438,  -438,   -56,
     -43,   -10,  -438,    18,  -438,  -438,  -438,  -438,   161,  -438,
     -62,   -31,  -438,  -438,    70,  -438,    52,  -438,  -438,  -438,
      28,  -245,  -438,   187,  -438,  -438,  -438,    71,  -438,  -438,
     246,  -438,  -438,  -115,  -438,  -438,  -438,    12,  -438,  -438,
     465,   294,   295,     2,  -404,  -438,  -438,   -49,   416,  -438,
     -46,   418,  -438,   231,   -45,  -438,  -438,   -44,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,     4,  -438,   188,  -438,
      26,  -438,  -438,  -438,   233,  -438,  -438,  -438,  -438,  -438,
    -438,   259,  -438,  -438,  -438,  -438,  -438,  -438,  -438,  -438,
    -438,  -438,  -438,  -438,  -438,  -438,   321,   291,    36,   148,
    -437,  -438,  -438,  -438,  -438,  -438,    23,  -191,   192,     0,
     -33,  -438,  -438,  -438,   297,  -438,  -438,  -438,   209,   207,
      58,  -438,    37,  -438,  -438,  -438,   306,  -204,  -206,  -438,
    -438,  -438,  -438,  -438,   260,  -438,   104,   350,  -129,   265,
    -210,  -438,  -438,  -270,  -438,  -359,  -438,  -438,    92,  -438
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -314
static const yytype_int16 yytable[] =
{
     106,   123,    70,   122,   151,   157,   158,   159,   106,   136,
     167,   182,   298,   169,   171,   172,   282,   208,    54,   121,
     134,   390,   137,   200,   239,   200,   334,   312,   318,   199,
     277,   183,   140,   184,  -308,   409,   138,   317,   156,   156,
     156,   295,  -175,  -111,   438,   429,   296,   402,   391,   166,
     496,   164,   149,   133,     2,     2,     3,     3,   165,   130,
     468,   181,   137,   503,   128,   403,   506,   175,     4,     5,
       6,     7,   347,     9,   220,   250,   138,    10,   350,   176,
      11,   220,   495,    12,   383,    14,   235,   124,   290,   499,
     393,   286,   192,   193,   495,   164,    17,   177,   287,   500,
     131,   -43,   165,   -61,   133,   255,  -267,   407,   -72,   185,
     -61,    20,    21,   301,    22,   239,   186,  -308,   410,   -43,
     206,    25,    26,    27,   283,   123,  -111,   203,   -43,   497,
     205,    49,   249,   -93,   -93,    34,    35,   181,   251,   218,
     250,   194,   132,   194,   135,   194,   221,    36,   -43,   236,
     106,   201,   -43,   126,   290,   -43,   250,   306,   417,   390,
     164,   291,    30,   126,   307,   156,   226,   165,   247,   259,
     227,    47,    48,   148,    49,   386,   164,   228,   229,   162,
     106,   106,   286,   470,   -43,   141,    19,   266,   337,   425,
     163,   -43,   323,   400,   275,   325,   326,   327,   312,   405,
     453,   207,   179,   342,   189,   190,   191,   266,   192,   193,
     321,   180,   -43,   -43,   -43,   -43,   -43,   -43,   150,   238,
     194,   343,   181,   459,   187,   270,   271,   272,   315,   316,
     344,   320,   -19,   345,   348,   349,   -19,   -19,   266,   -19,
     334,   106,   188,   225,   195,   333,    41,    42,    43,    44,
      45,    46,   412,   413,   406,   226,   196,   263,   269,   227,
     340,   408,   499,   266,  -192,   197,   228,   229,   213,   266,
     230,   279,   500,   426,   427,   490,   341,   -19,   198,   218,
     209,   225,   194,   106,   241,   373,   436,   245,   338,   -19,
     433,   349,    10,   226,   218,    11,    37,   227,   257,   266,
      14,   414,   480,   481,   228,   229,   280,   -19,   230,   483,
     484,   491,   481,   460,   281,   231,   288,   266,  -240,  -240,
    -240,  -240,  -240,  -240,   -19,   294,    20,    21,   -19,   299,
       4,     5,     6,     7,   156,   472,    25,    26,   160,   161,
     273,   274,   319,   398,   399,   308,   313,   314,   238,   266,
      34,   372,   128,   455,   374,   377,   149,   379,   342,     2,
     448,     3,    30,   449,   450,   451,   401,   283,   295,   395,
     397,   404,     4,     5,     6,     7,   343,     9,   447,   225,
     220,    10,  -312,  -313,    11,   344,   471,    12,   345,    14,
    -311,   226,   411,   415,   416,   227,   418,   290,   420,   446,
      17,   419,   228,   229,   440,   421,   230,   422,   441,    36,
     266,   428,   431,   454,   434,    20,    21,  -267,    22,   456,
     457,   442,   462,   463,   478,    25,    26,    27,     1,   413,
     486,     2,   487,     3,   498,   120,   482,   249,   396,    34,
      35,   106,   505,   467,     4,     5,     6,     7,     8,     9,
     384,    36,   502,    10,   437,   494,    11,   461,   430,    12,
      13,    14,   333,   106,    15,   477,   351,   494,   304,   473,
     156,    16,    17,   127,   260,   168,   261,   170,    49,    18,
     336,    19,   392,   474,   339,   476,   248,    20,    21,   309,
      22,   394,   276,   378,    23,    24,   380,    25,    26,    27,
     469,    28,   278,    29,   331,   244,    30,   330,    31,    32,
      33,    34,    35,   452,     0,   458,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    38,    39,    40,     0,     0,     0,
       0,    41,    42,    43,    44,    45,    46,    47,    48,   125,
      49,     0,     2,     0,     3,     0,     0,     0,     0,     0,
     225,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       9,     0,   226,     0,    10,     0,   227,    11,     0,     0,
      12,  -192,    14,   228,   229,     0,     0,   230,     0,     0,
       0,     0,    16,    17,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,   283,     0,     0,    20,    21,
       0,    22,     0,     0,     0,     0,   201,     0,    25,    26,
      27,     0,    28,     0,     0,     0,     0,    30,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,    38,    39,    40,     0,     0,
       0,     0,    41,    42,    43,    44,    45,    46,    47,    48,
     125,    49,     0,     2,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
     126,     9,     0,     0,     0,    10,     0,     0,    11,   -23,
       0,    12,     0,    14,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    16,    17,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,    20,
      21,     0,    22,     0,     0,     0,     0,     0,     0,    25,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    37,     0,     0,     0,    38,    39,    40,     0,
       0,     0,     0,    41,    42,    43,    44,    45,    46,    47,
      48,   125,    49,     0,     2,     0,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,     0,     0,     0,    10,     0,     0,    11,
       0,     0,    12,     0,    14,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    16,    17,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
      20,    21,     0,    22,     0,     0,     0,     0,     0,     0,
      25,    26,    27,     0,    28,     0,     0,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    38,    39,    40,
       0,     0,     0,     0,    41,    42,    43,    44,    45,    46,
      47,    48,   149,    49,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       6,     7,   126,     9,     0,     0,     0,    10,     0,     0,
      11,     0,     0,    12,     0,    14,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    16,    17,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,    20,    21,     0,    22,     0,     0,     0,     0,     0,
       0,    25,    26,    27,     0,    28,     0,     0,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,    37,     0,     0,     0,    38,    39,
      40,     0,     0,     0,     0,    41,    42,    43,    44,    45,
      46,    47,    48,   149,    49,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,   126,     9,     0,     0,     0,    10,     0,
       0,    11,     0,     0,    12,     0,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,    20,    21,     0,    22,     0,     0,     0,     0,
       0,     0,    25,    26,    27,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,    38,
      39,    40,     0,     0,     0,     0,    41,    42,    43,    44,
      45,    46,    47,    48,     0,    49,   149,   262,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
     137,    10,     0,     0,    11,     0,     0,    12,     0,    14,
       0,     0,     0,     0,   138,     0,     0,     0,     0,     0,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    20,    21,     0,    22,     0,
       0,     0,     0,     0,     0,    25,    26,    27,   149,     0,
       0,     2,     0,     3,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     4,     5,     6,     7,     0,     9,
       0,    36,     0,    10,     0,     0,    11,     0,    37,    12,
       0,    14,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    17,     0,     0,    47,    48,     0,    49,     0,
       0,     0,     0,     0,     0,     0,     0,    20,    21,     0,
      22,     0,     0,     0,     0,   201,     0,    25,    26,    27,
     149,     0,     0,     2,     0,     3,    30,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     4,     5,     6,     7,
       0,     9,     0,    36,     0,    10,     0,     0,    11,     0,
      37,    12,     0,    14,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,    47,    48,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,    20,
      21,     0,    22,     0,     0,     0,     0,     0,     0,    25,
      26,    27,   149,     0,     0,     2,     0,     3,     0,     0,
      31,    32,    33,    34,    35,     0,     0,   150,     4,     5,
       6,     7,     0,     9,     0,    36,   252,   253,     0,     0,
      11,     0,    37,    12,     0,    14,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    17,     0,     0,    47,
      48,     0,    49,     0,     0,     0,     0,     0,     0,     0,
       0,    20,    21,     0,    22,     0,     0,     0,     0,     0,
       0,   254,    26,    27,   149,     0,     0,     2,     0,     3,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       4,     5,     6,     7,     0,     9,     0,    36,     0,    10,
       0,     0,    11,     0,    37,    12,     0,    14,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       0,    47,    48,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,    20,    21,     0,    22,     0,     0,     0,
       0,     0,     0,    25,    26,    27,   149,     0,     0,     2,
       0,     3,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     4,     5,     6,     7,     0,     9,     0,    36,
       0,    10,     0,     0,    11,     0,    37,    12,     0,    14,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      17,     0,     0,    47,    48,     0,    49,   352,     0,     0,
       0,     0,     0,     0,     0,    20,    21,     0,    22,     0,
       0,     0,     0,     0,     0,    25,    26,    27,     0,     0,
       0,     0,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,    48,     0,   214,     0,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-438)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     1,     0,     1,    35,    38,    39,    40,     8,    19,
      59,    73,   218,    59,    59,    59,   207,   132,     0,     1,
      16,   291,    27,     4,   153,     4,    44,   231,   238,     4,
      24,    20,    28,    22,     5,     5,    41,    88,    38,    39,
      40,    24,     3,     5,   403,     5,    29,     5,   293,    59,
     487,    52,     3,     3,     6,     6,     8,     8,    59,     3,
       3,   112,    27,    93,    25,   310,   503,    45,    19,    20,
      21,    22,   263,    24,    24,    32,    41,    28,   269,    57,
      31,    24,   486,    34,   288,    36,   148,    24,    48,    30,
     296,     6,   122,   123,   498,    52,    47,    75,    13,    40,
       3,     5,    59,    48,     3,   167,   124,   317,    24,    98,
      55,    62,    63,    29,    65,   244,   105,    88,    88,    23,
     125,    72,    73,    74,    82,   125,    88,   125,    32,   488,
     130,   125,   165,   112,   113,    86,    87,   112,    95,   139,
      32,   124,    48,   124,    55,   124,   142,    98,    52,   149,
     150,    70,    56,    23,    48,    59,    32,     6,   349,   429,
      52,    55,    81,    23,    13,   165,    49,    59,   164,   179,
      53,   122,   123,     3,   125,   290,    52,    60,    61,     0,
     180,   181,     6,    59,    88,    94,    56,   187,   250,    13,
       7,    95,   241,   308,   194,   241,   241,   241,   402,   314,
     410,   131,    56,    47,   125,   126,   127,   207,   122,   123,
     241,   113,   116,   117,   118,   119,   120,   121,    90,    91,
     124,    65,   112,   427,     3,   189,   190,   191,     4,     5,
      74,   241,     0,    77,     4,     5,     4,     5,   238,     7,
      44,   241,     3,    37,   122,   245,   116,   117,   118,   119,
     120,   121,     4,     5,   316,    49,    13,   187,   188,    53,
     256,   323,    30,   263,    58,     4,    60,    61,    65,   269,
      64,   201,    40,     4,     5,   481,   258,    45,     4,   279,
      24,    37,   124,   283,    90,   283,   401,     3,    24,    57,
       4,     5,    28,    49,   294,    31,   105,    53,    27,   299,
      36,   334,     4,     5,    60,    61,    29,    75,    64,    42,
      43,     4,     5,   428,     4,    71,     3,   317,   122,   123,
     124,   125,   126,   127,    92,     5,    62,    63,    96,     3,
      19,    20,    21,    22,   334,    24,    72,    73,    47,    48,
     192,   193,    89,   306,   307,    58,    68,    58,    91,   349,
      86,     4,    25,   415,    96,    24,     3,    24,    47,     6,
     409,     8,    81,   409,   409,   409,    58,    82,    24,    24,
      24,   116,    19,    20,    21,    22,    65,    24,   409,    37,
      24,    28,     5,     5,    31,    74,   448,    34,    77,    36,
       5,    49,    89,   113,     4,    53,     4,    48,    25,   409,
      47,    13,    60,    61,   404,     4,    64,    24,    66,    98,
     410,     5,    50,   413,     3,    62,    63,   124,    65,    24,
     116,    79,    33,    51,    67,    72,    73,    74,     3,     5,
      33,     6,    92,     8,     5,     1,   468,   470,   303,    86,
      87,   441,   498,   441,    19,    20,    21,    22,    23,    24,
     289,    98,   495,    28,   402,   486,    31,   429,   387,    34,
      35,    36,   462,   463,    39,   463,   279,   498,   222,   457,
     470,    46,    47,     8,   180,    59,   181,    59,   125,    54,
     249,    56,   294,   457,   251,   462,   165,    62,    63,   230,
      65,   299,   195,   286,    69,    70,   287,    72,    73,    74,
     442,    76,   196,    78,   244,   155,    81,   242,    83,    84,
      85,    86,    87,   409,    -1,   423,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,     3,
     125,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    49,    -1,    28,    -1,    53,    31,    -1,    -1,
      34,    58,    36,    60,    61,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    82,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    -1,    70,    -1,    72,    73,
      74,    -1,    76,    -1,    -1,    -1,    -1,    81,    -1,    83,
      84,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
       3,   125,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,
      -1,    34,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      73,    74,    -1,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,     3,   125,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,
      -1,    -1,    34,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      72,    73,    74,    -1,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,     3,   125,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    28,    -1,    -1,
      31,    -1,    -1,    34,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    73,    74,    -1,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,     3,   125,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    28,    -1,
      -1,    31,    -1,    -1,    34,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,   125,     3,     4,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      27,    28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    98,    -1,    28,    -1,    -1,    31,    -1,   105,    34,
      -1,    36,   109,   110,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,   122,   123,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    70,    -1,    72,    73,    74,
       3,    -1,    -1,     6,    -1,     8,    81,    -1,    83,    84,
      85,    86,    87,    -1,    -1,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    98,    -1,    28,    -1,    -1,    31,    -1,
     105,    34,    -1,    36,   109,   110,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,   122,   123,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      73,    74,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      83,    84,    85,    86,    87,    -1,    -1,    90,    19,    20,
      21,    22,    -1,    24,    -1,    98,    27,    28,    -1,    -1,
      31,    -1,   105,    34,    -1,    36,   109,   110,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,   122,
     123,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    73,    74,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    83,    84,    85,    86,    87,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    98,    -1,    28,
      -1,    -1,    31,    -1,   105,    34,    -1,    36,   109,   110,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      -1,   122,   123,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    73,    74,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    98,
      -1,    28,    -1,    -1,    31,    -1,   105,    34,    -1,    36,
     109,   110,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,   122,   123,    -1,   125,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   122,   123,    -1,   125,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      28,    31,    34,    35,    36,    39,    46,    47,    54,    56,
      62,    63,    65,    69,    70,    72,    73,    74,    76,    78,
      81,    83,    84,    85,    86,    87,    98,   105,   109,   110,
     111,   116,   117,   118,   119,   120,   121,   122,   123,   125,
     132,   133,   142,   143,   144,   146,   147,   148,   149,   151,
     152,   153,   154,   159,   163,   179,   180,   181,   182,   183,
     184,   185,   187,   188,   190,   191,   192,   193,   195,   196,
     198,   199,   201,   202,   205,   206,   207,   211,   213,   214,
     215,   216,   217,   218,   230,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   250,   251,   252,   253,
     254,   255,   266,   267,   268,   270,   271,   272,   273,   284,
     133,   144,   184,   250,    24,     3,    23,   181,    25,   265,
       3,     3,    48,     3,   207,    55,   142,    27,    41,   155,
     207,    94,   173,   174,   223,   224,   225,   227,     3,     3,
      90,   152,   250,   278,   279,   283,   250,   251,   251,   251,
     238,   238,     0,     7,    52,    59,   142,   188,   189,   191,
     192,   195,   198,   200,   203,    45,    57,    75,   145,    56,
     113,   112,   151,    20,    22,    98,   105,     3,     3,   125,
     126,   127,   122,   123,   124,   122,    13,     4,     4,     4,
       4,    70,   144,   184,   231,   250,   125,   155,   174,    24,
     257,   258,   259,    65,   125,   164,   208,   209,   250,   256,
      24,   207,   261,   262,   263,    37,    49,    53,    60,    61,
      64,    71,   219,   222,   150,   151,   250,   184,    91,   279,
     280,    90,   274,   275,   278,     3,   197,   207,   237,   251,
      32,    95,    27,    28,    72,   151,   204,    27,   229,   142,
     182,   183,     4,   155,   248,   249,   250,   281,   282,   155,
     239,   239,   239,   240,   240,   250,   255,    24,   267,   155,
      29,     4,   248,    82,   162,   175,     6,    13,     3,   137,
      48,    55,   168,   169,     5,    24,    29,   212,   269,     3,
     264,    29,   171,   285,   171,   172,     6,    13,    58,   222,
     156,   157,   268,    68,    58,     4,     5,    88,   281,    89,
     142,   152,   186,   188,   189,   191,   195,   198,   276,   277,
     280,   275,   247,   250,    44,   194,   194,   151,    24,   215,
     207,   144,    47,    65,    74,    77,   178,   248,     4,     5,
     248,   164,    24,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   232,   233,
     234,   235,     4,   184,    96,   286,   287,    24,   260,    24,
     259,   260,   135,   268,   149,   170,   174,   160,   161,   210,
     284,   162,   209,   269,   249,    24,   137,    24,   263,   263,
     174,    58,     5,   162,   116,   174,   151,   281,   151,     5,
      88,    89,     4,     5,   251,   113,     4,   248,     4,    13,
      25,     4,    24,   288,   289,    13,     4,     5,     5,     5,
     168,    50,   176,     4,     3,   136,   174,   157,   286,   158,
     250,    66,    79,   220,   221,   228,   142,   152,   188,   191,
     195,   198,   277,   281,   250,   151,    24,   116,   289,   268,
     174,   161,    33,    51,   177,   134,   269,   184,     3,   261,
      59,   151,    24,   178,   211,   290,   247,   184,    67,   138,
       4,     5,   134,    42,    43,   226,    33,    92,   165,   167,
     269,     4,   139,   140,   152,   185,   241,   286,     5,    30,
      40,   141,   141,    93,   166,   140,   241
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
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

  case 53:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode));
            }
    break;

  case 54:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 55:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 57:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 59:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 61:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 63:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 65:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 66:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 67:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 68:

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

  case 69:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 70:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 71:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 72:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 74:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 75:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 76:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 78:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 79:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 81:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 82:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 83:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 84:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 85:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 92:

    { // boolean_primary: rule 2
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 93:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
    }
    break;

  case 95:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 97:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 99:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 101:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 111:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 112:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 113:

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

  case 120:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 121:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 122:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 125:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 126:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 127:

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

  case 128:

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

  case 129:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 130:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 131:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 132:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 133:

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

  case 134:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 135:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 136:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 137:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 138:

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

  case 139:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 142:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 146:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 147:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 148:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 149:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 150:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
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

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 163:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 172:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 176:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 177:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 178:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 186:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 187:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 188:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 189:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 192:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 193:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 195:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 196:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 197:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 198:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 200:

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

  case 201:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 206:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 207:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 229:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 232:

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

  case 238:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 242:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 243:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 245:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 246:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 247:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 249:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 250:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 251:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 252:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 253:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 254:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 255:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 256:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 257:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 258:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 259:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 260:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 261:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 262:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 264:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 265:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 272:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 273:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 276:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 277:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 278:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 279:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 280:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 281:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 282:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 283:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 284:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 285:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 286:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 287:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 288:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 289:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 290:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 291:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 292:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 293:

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

  case 294:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 295:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 296:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 301:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 302:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 303:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 304:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
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

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 307:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 314:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 315:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 316:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 317:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 318:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 322:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 323:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 324:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
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

  case 328:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 330:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 331:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 332:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 333:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 334:

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
