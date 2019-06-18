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
#ifndef YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_BITWISE_NOT = 322,
     SQL_TOKEN_CURRENT_DATE = 323,
     SQL_TOKEN_CURRENT_TIME = 324,
     SQL_TOKEN_CURRENT_TIMESTAMP = 325,
     SQL_TOKEN_EVERY = 326,
     SQL_TOKEN_CASE = 327,
     SQL_TOKEN_THEN = 328,
     SQL_TOKEN_END = 329,
     SQL_TOKEN_WHEN = 330,
     SQL_TOKEN_ELSE = 331,
     SQL_TOKEN_LIMIT = 332,
     SQL_TOKEN_OFFSET = 333,
     SQL_TOKEN_ONLY = 334,
     SQL_TOKEN_MATCH = 335,
     SQL_TOKEN_ECSQLOPTIONS = 336,
     SQL_TOKEN_INTEGER = 337,
     SQL_TOKEN_INT = 338,
     SQL_TOKEN_INT64 = 339,
     SQL_TOKEN_LONG = 340,
     SQL_TOKEN_BOOLEAN = 341,
     SQL_TOKEN_DOUBLE = 342,
     SQL_TOKEN_REAL = 343,
     SQL_TOKEN_FLOAT = 344,
     SQL_TOKEN_STRING = 345,
     SQL_TOKEN_VARCHAR = 346,
     SQL_TOKEN_BINARY = 347,
     SQL_TOKEN_BLOB = 348,
     SQL_TOKEN_DATE = 349,
     SQL_TOKEN_TIME = 350,
     SQL_TOKEN_TIMESTAMP = 351,
     SQL_TOKEN_OR = 352,
     SQL_TOKEN_AND = 353,
     SQL_BITWISE_OR = 354,
     SQL_BITWISE_AND = 355,
     SQL_BITWISE_SHIFT_RIGHT = 356,
     SQL_BITWISE_SHIFT_LEFT = 357,
     SQL_EQUAL = 358,
     SQL_GREAT = 359,
     SQL_LESS = 360,
     SQL_NOTEQUAL = 361,
     SQL_GREATEQ = 362,
     SQL_LESSEQ = 363,
     SQL_CONCAT = 364,
     SQL_TOKEN_INVALIDSYMBOL = 365
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

#endif /* !YY_SQLYY_E_BSW_TEMP_IMODE02_SRC_IMODEL02_IMODELCORE_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */

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
#define YYFINAL  168
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1928

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  133
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  160
/* YYNRULES -- Number of rules.  */
#define YYNRULES  346
/* YYNRULES -- Number of states.  */
#define YYNSTATES  521

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   365

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   130,     2,     2,
       3,     4,   128,   125,     5,   126,    13,   129,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   131,     2,     8,     2,     2,     2,     2,     2,     2,
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
     121,   122,   123,   124,   127,   132
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
     238,   240,   242,   244,   246,   248,   250,   254,   256,   261,
     263,   266,   268,   272,   274,   278,   280,   282,   284,   286,
     288,   290,   292,   294,   296,   299,   303,   306,   308,   310,
     312,   314,   316,   318,   321,   327,   330,   335,   340,   343,
     346,   348,   350,   351,   354,   358,   361,   363,   365,   369,
     373,   376,   378,   382,   385,   388,   392,   394,   396,   398,
     401,   404,   408,   410,   414,   416,   418,   420,   422,   424,
     426,   428,   431,   434,   437,   440,   441,   444,   446,   448,
     450,   452,   454,   456,   458,   460,   464,   469,   475,   477,
     483,   488,   494,   496,   498,   500,   502,   504,   506,   508,
     510,   512,   514,   517,   519,   521,   522,   524,   526,   529,
     534,   540,   546,   548,   556,   557,   559,   561,   563,   565,
     570,   571,   573,   575,   577,   579,   581,   583,   585,   587,
     589,   591,   593,   595,   597,   599,   601,   603,   605,   607,
     609,   611,   615,   618,   620,   622,   629,   631,   633,   635,
     637,   639,   643,   645,   647,   649,   652,   655,   658,   660,
     664,   668,   672,   674,   678,   682,   684,   688,   692,   696,
     700,   702,   704,   706,   708,   711,   714,   717,   719,   721,
     723,   725,   729,   731,   733,   737,   739,   741,   743,   745,
     747,   749,   753,   757,   759,   761,   764,   766,   768,   772,
     776,   780,   782,   784,   786,   790,   794,   797,   798,   802,
     803,   805,   807,   811,   814,   816,   818,   820,   822,   824,
     826,   832,   837,   839,   842,   847,   849,   853,   855,   857,
     859,   861,   863,   865,   867,   870,   875,   876,   879,   881,
     883,   885,   888,   890,   891,   894,   896,   900,   901,   903,
     906,   909,   911,   913,   917,   919,   921
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     134,     0,    -1,   135,    -1,   135,     7,    -1,   145,    -1,
     136,     5,   271,    -1,   271,    -1,   137,     5,   270,    -1,
     270,    -1,    -1,     3,   136,     4,    -1,    -1,     3,   137,
       4,    -1,    -1,    67,    33,   141,    -1,   142,    -1,   141,
       5,   142,    -1,   186,   143,    -1,   154,   143,    -1,    -1,
      30,    -1,    40,    -1,    -1,    23,    -1,   148,    -1,   149,
      -1,   150,    -1,   155,    -1,   156,    -1,   161,    -1,   146,
      -1,   165,    -1,   165,   147,   230,   146,    -1,    57,    -1,
      75,    -1,    45,    -1,    35,    -1,    39,    48,   176,   164,
     288,    -1,    54,    55,   259,   139,   151,    -1,    81,     3,
     152,     4,    -1,   153,    -1,   152,     5,   153,    -1,   154,
      -1,   252,    -1,    69,    -1,    70,   157,   166,    55,   162,
     170,    -1,    -1,    27,    -1,    41,    -1,   159,    -1,   158,
       5,   159,    -1,   270,   119,   160,    -1,   252,    -1,    78,
     176,    71,   158,   164,   288,    -1,   163,    -1,   162,     5,
     163,    -1,   211,    -1,    -1,   177,    -1,    70,   157,   166,
     170,    -1,   151,    -1,   128,    -1,   209,    -1,    -1,   169,
      -1,    -1,    94,   243,    -1,    93,   243,   168,    -1,   171,
     164,   178,   179,   140,   167,   288,    -1,    48,   172,    -1,
     176,    -1,   172,     5,   176,    -1,    -1,    29,    -1,    -1,
     173,    24,   138,    -1,    -1,    95,    -1,   175,   263,   174,
      -1,   175,   208,   287,   139,    -1,   228,    -1,    82,   185,
      -1,    -1,    50,    33,   249,    -1,    -1,    51,   185,    -1,
      74,    -1,    47,    -1,    77,    -1,    65,    -1,   186,    -1,
     214,    -1,   216,    -1,   270,    -1,   231,    -1,   252,    -1,
       3,   185,     4,    -1,   181,    -1,   181,    56,   144,   180,
      -1,   182,    -1,    23,   182,    -1,   183,    -1,   184,   114,
     183,    -1,   184,    -1,   185,   113,   184,    -1,   188,    -1,
     191,    -1,   202,    -1,   206,    -1,   207,    -1,   197,    -1,
     200,    -1,   194,    -1,   203,    -1,   189,   153,    -1,   153,
     189,   153,    -1,   189,   153,    -1,   121,    -1,   122,    -1,
     119,    -1,   120,    -1,   124,    -1,   123,    -1,    56,   144,
      -1,   144,    32,   153,   114,   153,    -1,   153,   190,    -1,
     144,    59,   253,   195,    -1,   144,    59,   238,   195,    -1,
     153,   192,    -1,   153,   193,    -1,   192,    -1,   193,    -1,
      -1,    44,   253,    -1,    56,   144,    65,    -1,   153,   196,
      -1,   196,    -1,   208,    -1,     3,   249,     4,    -1,   144,
      52,   198,    -1,   153,   199,    -1,   199,    -1,   189,   205,
     208,    -1,   153,   201,    -1,   153,   204,    -1,   144,    96,
     216,    -1,    28,    -1,    27,    -1,    72,    -1,    46,   208,
      -1,    76,   208,    -1,     3,   146,     4,    -1,   210,    -1,
     209,     5,   210,    -1,   258,    -1,   286,    -1,    99,    -1,
      20,    -1,    21,    -1,    22,    -1,    19,    -1,   212,   106,
      -1,   212,    99,    -1,   212,    20,    -1,   212,    22,    -1,
      -1,    29,   271,    -1,   271,    -1,   215,    -1,   212,    -1,
     286,    -1,    65,    -1,    47,    -1,    74,    -1,   218,    -1,
     217,     3,     4,    -1,   217,     3,   251,     4,    -1,   217,
       3,   157,   250,     4,    -1,    24,    -1,   219,     3,   157,
     250,     4,    -1,    36,     3,   128,     4,    -1,    36,     3,
     157,   250,     4,    -1,    31,    -1,    62,    -1,    63,    -1,
      73,    -1,    87,    -1,    28,    -1,    72,    -1,    60,    -1,
      61,    -1,    49,    -1,    66,   185,    -1,   221,    -1,   229,
      -1,    -1,    53,    -1,   220,    -1,   220,    68,    -1,   176,
      37,    58,   176,    -1,   176,    64,   223,    58,   176,    -1,
     176,   223,    58,   176,   222,    -1,   224,    -1,   176,   223,
      58,   176,    79,   263,   227,    -1,    -1,    42,    -1,    43,
      -1,   226,    -1,   225,    -1,    79,     3,   136,     4,    -1,
      -1,    27,    -1,   208,    -1,   252,    -1,   108,    -1,   109,
      -1,   102,    -1,   103,    -1,   105,    -1,    99,    -1,    98,
      -1,   100,    -1,   101,    -1,   104,    -1,   106,    -1,   110,
      -1,   111,    -1,   112,    -1,   107,    -1,    24,    -1,   233,
      -1,    24,    13,    24,    -1,   234,    25,    -1,   234,    -1,
     235,    -1,    34,     3,   232,    29,   236,     4,    -1,   214,
      -1,   216,    -1,   270,    -1,   231,    -1,   272,    -1,     3,
     252,     4,    -1,   237,    -1,   238,    -1,   239,    -1,   126,
     239,    -1,   125,   239,    -1,    83,   239,    -1,   240,    -1,
     241,   128,   240,    -1,   241,   129,   240,    -1,   241,   130,
     240,    -1,   241,    -1,   242,   125,   241,    -1,   242,   126,
     241,    -1,   242,    -1,   243,   118,   242,    -1,   243,   117,
     242,    -1,   243,   115,   242,    -1,   243,   116,   242,    -1,
     245,    -1,    84,    -1,    86,    -1,    85,    -1,   110,   253,
      -1,   112,   253,    -1,   111,   253,    -1,   244,    -1,   246,
      -1,   247,    -1,   252,    -1,   249,     5,   252,    -1,   283,
      -1,   250,    -1,   251,     5,   250,    -1,   243,    -1,   253,
      -1,   248,    -1,   254,    -1,   257,    -1,   255,    -1,   254,
     125,   257,    -1,   252,   127,   252,    -1,   106,    -1,   256,
      -1,   252,   213,    -1,   261,    -1,   260,    -1,    24,    13,
     261,    -1,    24,    13,   262,    -1,    24,     6,   262,    -1,
      24,    -1,   264,    -1,   265,    -1,   264,    13,   265,    -1,
     264,     6,   265,    -1,    24,   266,    -1,    -1,     3,   251,
       4,    -1,    -1,    25,    -1,   269,    -1,   268,    13,   269,
      -1,    24,   267,    -1,   128,    -1,   268,    -1,    24,    -1,
     273,    -1,   274,    -1,   275,    -1,    88,   285,   276,   282,
      90,    -1,    88,   280,   282,    90,    -1,   277,    -1,   280,
     277,    -1,    91,   278,    89,   283,    -1,   279,    -1,   278,
       5,   279,    -1,   154,    -1,   187,    -1,   190,    -1,   199,
      -1,   192,    -1,   196,    -1,   281,    -1,   280,   281,    -1,
      91,   185,    89,   283,    -1,    -1,    92,   283,    -1,   284,
      -1,   252,    -1,   154,    -1,     6,    24,    -1,     8,    -1,
      -1,   173,    24,    -1,   185,    -1,     3,   135,     4,    -1,
      -1,   289,    -1,    97,   290,    -1,   290,   291,    -1,   291,
      -1,    24,    -1,    24,   119,   292,    -1,   212,    -1,    24,
      -1,   180,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   216,   216,   218,   223,   228,   233,   241,   246,   256,
     257,   265,   266,   277,   278,   288,   293,   301,   309,   318,
     319,   320,   324,   325,   331,   332,   333,   334,   335,   336,
     337,   341,   346,   357,   358,   359,   362,   369,   381,   390,
     400,   405,   413,   416,   421,   430,   441,   442,   443,   447,
     450,   456,   463,   466,   478,   481,   487,   491,   492,   500,
     508,   512,   517,   520,   521,   524,   525,   533,   542,   556,
     566,   569,   575,   576,   580,   583,   592,   593,   597,   604,
     612,   615,   624,   625,   635,   636,   644,   645,   646,   647,
     650,   651,   652,   653,   654,   655,   656,   666,   667,   677,
     678,   686,   687,   696,   697,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   721,   728,   735,   760,   761,   762,
     763,   764,   765,   766,   774,   784,   792,   802,   812,   818,
     824,   846,   871,   872,   879,   888,   894,   910,   914,   922,
     931,   937,   953,   962,   971,   980,   990,   991,   992,   996,
    1004,  1010,  1021,  1026,  1033,  1037,  1041,  1042,  1043,  1044,
    1045,  1047,  1059,  1071,  1083,  1099,  1100,  1106,  1110,  1111,
    1114,  1115,  1116,  1117,  1120,  1121,  1128,  1137,  1151,  1156,
    1165,  1173,  1185,  1186,  1187,  1188,  1189,  1190,  1191,  1195,
    1200,  1205,  1212,  1220,  1221,  1224,  1225,  1230,  1231,  1239,
    1251,  1261,  1270,  1275,  1289,  1290,  1291,  1294,  1295,  1298,
    1310,  1311,  1315,  1318,  1322,  1323,  1324,  1325,  1326,  1327,
    1328,  1329,  1330,  1331,  1332,  1333,  1334,  1335,  1336,  1337,
    1341,  1346,  1356,  1365,  1366,  1370,  1382,  1383,  1384,  1385,
    1386,  1387,  1394,  1398,  1402,  1403,  1409,  1415,  1424,  1425,
    1432,  1439,  1449,  1450,  1457,  1467,  1468,  1475,  1482,  1489,
    1499,  1506,  1511,  1516,  1521,  1527,  1533,  1542,  1549,  1557,
    1565,  1568,  1574,  1578,  1583,  1591,  1592,  1593,  1597,  1601,
    1602,  1605,  1612,  1622,  1625,  1629,  1639,  1644,  1651,  1660,
    1668,  1677,  1685,  1693,  1698,  1703,  1711,  1720,  1721,  1731,
    1732,  1740,  1745,  1763,  1769,  1777,  1788,  1792,  1796,  1797,
    1801,  1812,  1822,  1827,  1834,  1844,  1847,  1852,  1853,  1854,
    1855,  1856,  1857,  1860,  1865,  1872,  1882,  1883,  1891,  1895,
    1899,  1903,  1909,  1917,  1920,  1930,  1944,  1948,  1949,  1953,
    1962,  1967,  1975,  1981,  1991,  1992,  1993
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
  "SQL_TOKEN_VALUES", "SQL_TOKEN_WHERE", "SQL_BITWISE_NOT",
  "SQL_TOKEN_CURRENT_DATE", "SQL_TOKEN_CURRENT_TIME",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_EVERY", "SQL_TOKEN_CASE",
  "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE",
  "SQL_TOKEN_LIMIT", "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY",
  "SQL_TOKEN_MATCH", "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_INTEGER",
  "SQL_TOKEN_INT", "SQL_TOKEN_INT64", "SQL_TOKEN_LONG",
  "SQL_TOKEN_BOOLEAN", "SQL_TOKEN_DOUBLE", "SQL_TOKEN_REAL",
  "SQL_TOKEN_FLOAT", "SQL_TOKEN_STRING", "SQL_TOKEN_VARCHAR",
  "SQL_TOKEN_BINARY", "SQL_TOKEN_BLOB", "SQL_TOKEN_DATE", "SQL_TOKEN_TIME",
  "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_OR", "SQL_TOKEN_AND", "SQL_BITWISE_OR",
  "SQL_BITWISE_AND", "SQL_BITWISE_SHIFT_RIGHT", "SQL_BITWISE_SHIFT_LEFT",
  "SQL_EQUAL", "SQL_GREAT", "SQL_LESS", "SQL_NOTEQUAL", "SQL_GREATEQ",
  "SQL_LESSEQ", "'+'", "'-'", "SQL_CONCAT", "'*'", "'/'", "'%'", "'='",
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
  "boolean_primary", "boolean_test", "boolean_factor", "boolean_term",
  "search_condition", "predicate", "comparison_predicate_part_2",
  "comparison_predicate", "comparison", "between_predicate_part_2",
  "between_predicate", "character_like_predicate_part_2",
  "other_like_predicate_part_2", "like_predicate", "opt_escape",
  "null_predicate_part_2", "test_for_null", "in_predicate_value",
  "in_predicate_part_2", "in_predicate",
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
  "term", "term_add_sub", "num_value_exp", "datetime_primary",
  "datetime_value_fct", "datetime_factor", "datetime_term",
  "datetime_value_exp", "value_exp_commalist", "function_arg",
  "function_args_commalist", "value_exp", "string_value_exp",
  "char_value_exp", "concatenation", "char_primary", "char_factor",
  "derived_column", "table_node", "tablespace_qualified_class_name",
  "qualified_class_name", "class_name",
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
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,    43,    45,   364,    42,    47,
      37,    61,   365
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   133,   134,   134,   135,   136,   136,   137,   137,   138,
     138,   139,   139,   140,   140,   141,   141,   142,   142,   143,
     143,   143,   144,   144,   145,   145,   145,   145,   145,   145,
     145,   146,   146,   147,   147,   147,   148,   149,   150,   151,
     152,   152,   153,   154,   155,   156,   157,   157,   157,   158,
     158,   159,   160,   161,   162,   162,   163,   164,   164,   165,
     165,   166,   166,   167,   167,   168,   168,   169,   170,   171,
     172,   172,   173,   173,   174,   174,   175,   175,   176,   176,
     176,   177,   178,   178,   179,   179,   180,   180,   180,   180,
     181,   181,   181,   181,   181,   181,   181,   182,   182,   183,
     183,   184,   184,   185,   185,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   187,   188,   188,   189,   189,   189,
     189,   189,   189,   189,   190,   191,   192,   193,   194,   194,
     194,   194,   195,   195,   196,   197,   197,   198,   198,   199,
     200,   200,   201,   202,   203,   204,   205,   205,   205,   206,
     207,   208,   209,   209,   210,   211,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   213,   213,   213,   214,   214,
     215,   215,   215,   215,   216,   216,   216,   216,   217,   218,
     218,   218,   219,   219,   219,   219,   219,   219,   219,   220,
     220,   220,   221,   222,   222,   223,   223,   223,   223,   224,
     225,   225,   225,   226,   227,   227,   227,   228,   228,   229,
     230,   230,   231,   232,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     234,   234,   235,   236,   236,   237,   238,   238,   238,   238,
     238,   238,   238,   239,   240,   240,   240,   240,   241,   241,
     241,   241,   242,   242,   242,   243,   243,   243,   243,   243,
     244,   245,   245,   245,   245,   245,   245,   246,   247,   248,
     249,   249,   250,   251,   251,   252,   252,   252,   253,   254,
     254,   255,   255,   256,   257,   258,   259,   259,   260,   261,
     261,   262,   263,   264,   264,   264,   265,   266,   266,   267,
     267,   268,   268,   269,   269,   270,   271,   272,   273,   273,
     274,   275,   276,   276,   277,   278,   278,   279,   279,   279,
     279,   279,   279,   280,   280,   281,   282,   282,   283,   284,
     285,   286,   286,   287,   287,   135,   135,   288,   288,   289,
     290,   290,   291,   291,   292,   292,   292
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
       1,     1,     1,     1,     1,     1,     3,     1,     4,     1,
       2,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     2,     1,     1,     1,
       1,     1,     1,     2,     5,     2,     4,     4,     2,     2,
       1,     1,     0,     2,     3,     2,     1,     1,     3,     3,
       2,     1,     3,     2,     2,     3,     1,     1,     1,     2,
       2,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     0,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     4,     5,     1,     5,
       4,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     0,     1,     1,     2,     4,
       5,     5,     1,     7,     0,     1,     1,     1,     1,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     6,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     2,     2,     2,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     1,     1,     1,     2,     2,     2,     1,     1,     1,
       1,     3,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     2,     1,     1,     3,     3,
       3,     1,     1,     1,     3,     3,     2,     0,     3,     0,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     1,
       5,     4,     1,     2,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     4,     0,     2,     1,     1,
       1,     2,     1,     0,     2,     1,     3,     0,     1,     2,
       2,     1,     1,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      22,    22,     0,   332,   160,   157,   158,   159,    22,   299,
     187,   182,     0,    36,     0,     0,     0,   172,     0,    22,
     183,   184,   171,    44,    46,   188,   185,   173,     0,    76,
       0,     0,   261,   263,   262,   186,     0,   156,   283,     0,
       0,     0,   119,   120,   117,   118,   122,   121,     0,     0,
     304,     0,     2,     0,     4,    30,    24,    25,    26,    60,
      22,    42,    27,    28,    29,    31,    97,    99,   101,   103,
     335,    90,   105,     0,   106,   130,   131,   112,   136,   110,
     141,   111,   107,   113,   108,   109,   212,   169,    91,   168,
      92,     0,   174,     0,    94,   242,   243,   244,   248,   252,
     255,   275,   267,   260,   268,   269,   277,    95,   276,   278,
     280,   284,   279,   305,   301,    93,   240,   307,   308,   309,
     170,     0,     0,     0,    43,   331,    22,    23,   100,   300,
     303,     0,    46,    76,     0,   149,     0,   123,    47,    48,
       0,   150,    77,     0,   195,   202,   208,   207,    80,     0,
       0,   236,   237,   239,   247,   238,    22,   330,    43,   326,
     323,     0,     0,   264,   266,   265,   246,   245,     1,     3,
       0,     0,     0,     0,   125,   128,   129,   135,   140,   143,
     144,    35,    33,    34,   210,    22,    22,    22,   116,   163,
     164,   162,   161,     0,    46,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   336,   151,    96,
     241,    46,     0,     0,     0,   213,     0,     0,    57,     0,
      11,   287,   286,   134,   304,     0,    62,   152,   165,   154,
     297,   333,    74,   292,   293,     0,   191,   196,   189,   190,
     195,     0,   197,     0,     0,    40,     0,     0,     0,   324,
       0,    22,   326,   312,     0,     0,   139,   137,   132,   132,
       0,     0,   147,   146,   148,   115,     0,   211,     0,     0,
     102,   104,   175,     0,   273,     0,   329,   272,   328,     0,
     249,   250,   251,   253,   254,   258,   259,   257,   256,   282,
     281,   299,   302,     0,     0,   180,     0,    22,   337,    58,
       0,     0,     0,     0,    76,     0,    59,    57,     0,   306,
       0,   285,   167,     0,   296,    73,     0,    11,     0,    78,
       0,     0,    76,     0,    57,    49,     0,   198,    76,    39,
       0,     0,   327,   311,     0,    42,   318,     0,   319,   130,
     136,   141,     0,   315,     0,   313,     0,   270,     0,   127,
     126,     0,   178,   145,   142,    32,    87,    89,    86,    88,
      98,     0,   176,     0,     0,     0,   229,   220,   219,   221,
     222,   216,   217,   223,   218,   224,   228,   214,   215,   225,
     226,   227,   230,   233,   234,     0,   181,    81,     0,    37,
     338,   291,   290,   291,   288,   289,     0,     8,    38,    69,
      70,     0,    54,    56,   155,    82,   153,   166,     0,   334,
      79,     9,   295,   294,   199,    76,     0,   337,     0,   195,
      41,   325,   116,    22,     0,   310,   138,     0,   133,     0,
     177,   274,   179,     0,   232,   235,   342,   339,   341,     0,
      12,     0,    76,     0,    45,     0,    84,   298,     0,    75,
     200,    50,    53,    51,    52,    22,     0,   193,   201,   194,
       0,   317,     0,   321,   322,   320,   316,   314,   271,   124,
     231,     0,   340,     7,    71,    55,     0,    22,    13,     0,
       6,   192,     0,   204,     0,   114,   345,   346,   344,   343,
      83,    85,     0,    63,    10,     0,     0,   205,   206,   203,
      22,     0,   337,    64,     5,   209,    14,    15,    42,    19,
      65,    68,    22,    20,    21,    18,    17,     0,    67,    16,
      66
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    51,    52,   479,   396,   449,   303,   493,   506,   507,
     515,    53,    54,   212,   184,    56,    57,    58,    59,   244,
      60,    61,    62,    63,   140,   324,   325,   453,    64,   401,
     402,   298,    65,   225,   502,   518,   503,   306,   307,   399,
     316,   319,   143,   144,   299,   446,   478,   360,    66,    67,
      68,    69,   247,    71,   336,    72,    73,   338,    74,    75,
      76,    77,   349,    78,    79,   256,    80,    81,   179,    82,
      83,   180,   266,    84,    85,    86,   226,   227,   403,    87,
     311,   151,    89,   152,    91,    92,    93,   242,   457,   458,
     243,   145,   146,   147,   499,   148,   459,   268,   153,   214,
     382,   383,   384,   385,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   346,   274,   275,   158,
     108,   109,   110,   111,   112,   229,   220,   221,   222,   395,
     232,   233,   234,   314,   130,   113,   114,   155,   480,   116,
     117,   118,   119,   252,   253,   342,   343,   159,   160,   250,
     277,   278,   161,   120,   317,   389,   390,   437,   438,   489
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -441
static const yytype_int16 yypact[] =
{
     482,   482,    49,  -441,  -441,  -441,  -441,  -441,   702,    65,
    -441,  -441,    62,  -441,    78,     8,    81,  -441,    72,   141,
    -441,  -441,  -441,  -441,    76,  -441,  -441,  -441,    81,    97,
     211,  1664,  -441,  -441,  -441,  -441,  1314,  -441,  -441,  1460,
    1460,  1460,  -441,  -441,  -441,  -441,  -441,  -441,  1664,  1664,
    -441,   208,   221,   103,  -441,  -441,  -441,  -441,  -441,  -441,
     102,  -441,  -441,  -441,  -441,    22,   163,  -441,  -441,   118,
     127,  -441,  -441,  1460,  -441,  -441,  -441,  -441,  -441,  -441,
    -441,  -441,  -441,  -441,  -441,  -441,  -441,    24,   313,  -441,
    1650,   231,  -441,   240,  1689,  -441,  -441,  -441,  -441,   117,
      35,   217,  -441,  -441,  -441,  -441,  -441,   230,  -441,   134,
    -441,  -441,  -441,   239,  -441,  1730,  -441,  -441,  -441,  -441,
    -441,   257,   262,    29,    30,  -441,   592,  -441,  -441,  -441,
    -441,  1460,    11,    97,   -12,  -441,   245,   210,  -441,  -441,
    1533,  -441,  -441,    80,   178,  -441,  -441,  -441,  -441,  1460,
    1215,  -441,  -441,  -441,  -441,  -441,   812,  -441,   158,   166,
    -441,   204,   158,  -441,  -441,  -441,  -441,  -441,  -441,  -441,
     301,  1460,    19,  1387,  -441,  -441,  -441,  -441,  -441,  -441,
    -441,  -441,  -441,  -441,   280,   141,   812,   812,  -441,  -441,
    -441,  -441,  -441,  1142,    76,  1606,  1606,  1606,  1606,  1606,
    1606,  1606,  1606,  1606,  1460,   206,    13,  -441,  -441,  -441,
    -441,    76,   262,    29,   314,   158,   351,  1460,    38,   170,
     361,  -441,  -441,  -441,   130,   146,   364,  -441,    18,  -441,
     363,    92,    92,   194,  -441,   312,  -441,  -441,  -441,  -441,
     261,    13,   306,   318,   260,  -441,    26,   -19,  1460,  -441,
     288,   812,   291,  -441,   204,  1215,  -441,  -441,   212,    10,
    1460,   205,  -441,  -441,  -441,  -441,    81,  -441,   -12,    15,
    -441,   118,  -441,  1460,  -441,   267,   158,  -441,  -441,  1460,
    -441,  -441,  -441,   117,   117,    35,    35,    35,    35,  -441,
    -441,   362,  -441,  1533,  1809,  -441,   387,   812,   298,  -441,
     370,   372,    13,   316,    97,   197,  -441,   317,  1460,  -441,
     378,  -441,  -441,  1460,  -441,  -441,   379,   361,   381,  -441,
     382,   382,    97,   349,    69,  -441,   292,  -441,    97,  -441,
    1460,  1460,  -441,  -441,    34,    44,  -441,  1460,  -441,   405,
     407,   408,    45,  -441,   324,  -441,   276,   158,  1460,  -441,
    -441,   302,  -441,  -441,  -441,  -441,  -441,  -441,  -441,  -441,
    -441,   411,  -441,  1460,   413,   373,   410,  -441,  -441,  -441,
    -441,  -441,  -441,  -441,  -441,  -441,  -441,  -441,  -441,  -441,
    -441,  -441,  -441,   395,  -441,   421,  -441,   127,   402,  -441,
    -441,  -441,  -441,   224,  -441,  -441,   283,  -441,  -441,   440,
     340,    52,  -441,  -441,  -441,   396,  -441,  -441,   286,  -441,
    -441,   444,  -441,  -441,   340,    97,    13,   298,  1460,   307,
    -441,  -441,    58,  1032,  1460,  -441,  -441,  1460,   321,  1460,
    -441,  -441,  -441,   425,  -441,  -441,   331,   402,  -441,   370,
    -441,    13,    97,   197,  -441,   418,   401,  -441,   378,  -441,
     340,  -441,  -441,  -441,   158,   812,    98,  -441,  -441,  -441,
      53,  -441,  1460,  -441,  -441,  -441,  -441,  -441,   158,  -441,
    -441,    89,  -441,  -441,   340,  -441,  1460,   812,   386,   297,
    -441,   127,   378,   282,  1460,  -441,  -441,  -441,    24,  -441,
     449,   127,   423,   367,  -441,   378,   342,  -441,  -441,  -441,
     922,  1606,   298,  -441,  -441,  -441,   452,  -441,  1831,   137,
     200,  -441,   922,  -441,  -441,  -441,  -441,  1606,  -441,  -441,
     217
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -441,  -441,   461,   -17,  -441,  -441,   149,  -441,  -441,   -49,
     -41,    -1,  -441,    28,  -441,  -441,  -441,  -441,   167,  -441,
     -54,   -31,  -441,  -441,   -87,  -441,    55,  -441,  -441,  -441,
      31,  -252,  -441,   180,  -441,  -441,  -441,    71,  -441,  -441,
     243,  -441,  -441,  -131,  -441,  -441,  -441,    16,  -441,   471,
     296,   304,     6,  -436,  -441,  -441,   -39,   432,  -441,   -38,
     433,  -441,   235,   -35,  -441,  -441,   -33,  -441,  -441,  -441,
    -441,  -441,  -441,  -441,  -441,    32,  -441,   187,  -441,    25,
    -441,    12,  -441,     9,  -441,  -441,  -441,  -441,  -441,  -441,
     258,  -441,  -441,  -441,  -441,  -441,  -441,  -441,    23,  -441,
    -441,  -441,  -441,  -441,  -441,   326,   121,   -15,   160,   179,
    -440,  -441,  -441,  -441,  -441,  -441,    33,  -191,   186,     0,
     -25,  -441,  -441,  -441,   303,  -441,  -441,  -441,   213,   207,
      56,  -441,    41,  -441,  -441,  -441,   305,     3,  -196,  -441,
    -441,  -441,  -441,  -441,   265,  -441,    99,   354,  -106,   268,
    -213,  -441,  -441,  -269,  -441,  -374,  -441,  -441,    86,  -441
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -323
static const yytype_int16 yytable[] =
{
     107,   124,   218,   115,   115,   157,    70,   123,   107,    90,
      90,   115,    88,    88,   163,   164,   165,    90,   137,   188,
      88,   173,   175,    94,    94,   177,   296,   178,    55,   122,
     210,    94,   312,   209,   210,   332,   404,   291,   138,   162,
     162,   162,   309,   452,   189,   217,   190,   310,   135,  -317,
     423,   260,   139,   249,   348,   405,   133,   443,   211,   172,
     141,   510,   356,  -114,   509,   131,   260,   181,  -178,    30,
     331,   170,   417,   125,   416,   235,   509,   520,   171,   182,
     357,   132,   361,   134,   134,   260,   170,   236,   364,   358,
     129,   237,   359,   171,   187,   245,  -195,   183,   238,   239,
     304,   482,   240,   138,   230,   170,   273,   279,     4,     5,
       6,     7,   484,   486,   407,   261,   -72,   139,   421,   265,
     297,   315,   230,   191,   293,   127,   124,   136,   511,   115,
     192,   215,   213,  -317,   424,    90,   356,  -276,    88,   216,
     228,    50,   187,   -95,   -95,   204,   259,  -114,   249,    94,
     246,   297,   154,   204,   357,   170,   107,   204,    19,   115,
     198,   199,   171,   358,   127,    90,   359,   513,    88,   166,
     167,   162,   431,   400,   404,   231,   300,   514,   -61,    94,
     280,   281,   282,   301,   269,   -61,   107,   107,    37,   115,
     115,   414,   142,   276,   304,    90,    90,   419,    88,    88,
     320,   305,   257,     2,   289,     3,   351,   321,   168,    94,
      94,   467,   337,   339,   149,   235,   340,   276,   341,   185,
     335,    42,    43,    44,    45,    46,    47,   236,   169,   352,
     300,   237,   186,    10,   193,   -43,    11,   439,   238,   239,
     187,    14,   240,   194,   326,   195,   196,   197,   276,   241,
     334,   107,   206,   -43,   115,   347,   348,   156,   248,   205,
      90,   207,   -43,    88,   329,   330,   208,    20,    21,   219,
     353,   362,   363,   276,    94,   223,   420,    25,    26,   276,
     426,   427,   -43,   422,   450,   204,   -43,   440,   441,   -43,
     447,   363,    35,   228,   517,   251,   355,   107,   354,   504,
     115,   494,   495,   387,   255,   397,    90,   267,   228,    88,
     236,   474,    38,   276,   237,   200,   201,   202,   203,   -43,
      94,   238,   239,   428,   497,   498,   -43,  -243,  -243,  -243,
    -243,   276,   200,   201,   202,   203,  -236,  -243,  -243,  -243,
    -243,  -243,  -243,   294,   235,  -236,   505,   495,   162,   -43,
     -43,   -43,   -43,   -43,   -43,   295,   236,   204,   283,   284,
     237,   412,   413,   276,   302,  -236,   313,   238,   239,   308,
     322,   240,  -236,   455,   327,   469,   328,   235,   333,   285,
     286,   287,   288,   248,   462,   463,   456,   129,   464,   236,
     465,   386,   461,   237,   391,   388,   393,    30,  -195,   297,
     238,   239,   309,   409,   240,   411,   230,   415,   485,  -236,
    -321,   418,  -322,  -320,   425,   430,   429,   432,   454,   326,
     434,   304,   460,   433,   276,   435,   436,   468,  -236,  -236,
    -236,  -236,  -236,  -236,  -236,  -236,  -236,  -236,  -236,  -236,
    -236,  -236,  -236,  -236,   473,   442,   445,   448,  -276,   470,
     471,   476,   477,   492,   427,   107,   500,   512,   115,   259,
     501,   481,   121,   519,    90,   496,   410,    88,   516,   508,
     398,   451,   444,   365,   475,   318,   347,   107,    94,   128,
     115,   508,   270,   491,   162,     1,    90,   487,     2,    88,
       3,   271,   174,   176,   350,   406,   488,   258,   323,   408,
      94,     4,     5,     6,     7,     8,     9,   392,   290,   490,
      10,   292,   483,    11,   394,   254,    12,    13,    14,   345,
     344,    15,   466,   472,     0,     0,     0,     0,    16,    17,
       0,     0,     0,     0,     0,     0,    18,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,    23,    24,     0,    25,    26,    27,     0,    28,     0,
      29,     0,     0,    30,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   126,     0,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,     8,     9,     0,     0,     0,
      10,     0,     0,    11,     0,     0,    12,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,   211,     0,    25,    26,    27,     0,    28,     0,
       0,     0,     0,    30,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   126,     0,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,   127,     9,     0,     0,     0,
      10,     0,     0,    11,   -23,     0,    12,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,    28,     0,
       0,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   126,     0,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,     8,     9,     0,     0,     0,
      10,     0,     0,    11,     0,     0,    12,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,    28,     0,
       0,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   150,     0,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,   127,     9,     0,     0,     0,
      10,     0,     0,    11,     0,     0,    12,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,    28,     0,
       0,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   150,     0,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,   127,     9,     0,     0,     0,
      10,     0,     0,    11,     0,     0,    12,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,    38,     0,
       0,     0,    39,    40,    41,   150,   272,     0,     2,     0,
       3,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,     4,     5,     6,     7,     0,     9,     0,     0,   138,
      10,     0,     0,    11,     0,     0,    12,     0,    14,     0,
       0,     0,     0,   139,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    20,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,   150,     0,
       0,     2,     0,     3,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     4,     5,     6,     7,     0,     9,
       0,    37,     0,    10,     0,     0,    11,     0,    38,    12,
       0,    14,    39,    40,    41,     0,     0,     0,     0,     0,
       0,     0,    17,     0,     0,     0,     0,    48,    49,     0,
      50,     0,     0,     0,     0,     0,     0,    20,    21,     0,
      22,     0,     0,     0,     0,   211,     0,    25,    26,    27,
       0,     0,     0,     0,     0,     0,    30,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    37,     0,     0,   150,     0,     0,
       2,    38,     3,     0,     0,    39,    40,    41,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     0,     9,     0,
      48,    49,    10,    50,     0,    11,     0,     0,    12,     0,
      14,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    20,    21,     0,    22,
       0,     0,     0,     0,     0,     0,    25,    26,    27,     0,
     150,     0,     0,     2,     0,     3,     0,    31,    32,    33,
      34,    35,    36,     0,     0,   156,     4,     5,     6,     7,
       0,     9,     0,    37,   262,   263,     0,     0,    11,     0,
      38,    12,     0,    14,    39,    40,    41,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,    48,
      49,     0,    50,     0,     0,     0,     0,     0,     0,    20,
      21,     0,    22,     0,     0,     0,     0,     0,     0,   264,
      26,    27,     0,   150,     0,     0,     2,     0,     3,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     4,
       5,     6,     7,     0,     9,     0,    37,     0,    10,     0,
       0,    11,     0,    38,    12,     0,    14,    39,    40,    41,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,    48,    49,     0,    50,     0,     0,     0,     0,
       0,     0,    20,    21,     0,    22,     0,     0,     0,     0,
       0,     0,    25,    26,    27,     0,   150,     0,     0,     2,
       0,     3,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     4,     5,     6,     7,     0,     9,     0,    37,
       0,    10,     0,     0,    11,     0,    38,    12,     0,    14,
      39,    40,    41,     0,     0,     0,     0,     0,     0,     0,
      17,     0,     0,     0,     0,    48,    49,     0,    50,     0,
       0,     0,     0,     0,     0,    20,    21,     0,    22,     0,
       0,     0,     0,     0,     0,    25,    26,    27,     0,   150,
       0,     0,     2,     0,     3,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     4,     5,     6,     7,     0,
       9,     0,    37,     0,    10,     0,     0,    11,     0,    38,
      12,     0,    14,    39,    40,    41,     0,     0,     0,     0,
       0,     0,     0,    17,     0,     0,     0,     0,    48,    49,
       0,   224,     0,     0,     0,     0,     0,   150,    20,    21,
       2,    22,     3,  -237,     0,     0,     0,     0,    25,    26,
      27,     0,  -237,     4,     5,     6,     7,     0,     9,    31,
       0,     0,    10,    35,    36,    11,     0,     0,    12,     0,
      14,     0,  -237,     0,     0,    37,     0,     0,     0,  -237,
       0,    17,  -239,     0,     0,     0,     0,     0,     0,     0,
       0,  -239,     0,     0,     0,     0,    20,    21,     0,    22,
       0,    48,    49,     0,    50,     0,    25,    26,    27,     0,
       0,  -239,     0,     0,     0,     0,  -237,     0,  -239,     0,
       0,    35,    36,  -238,     0,     0,     0,     0,     0,     0,
       0,     0,  -238,    37,     0,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,     0,  -238,     0,     0,  -239,     0,     0,     0,  -238,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -239,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,
       0,     0,     0,     0,     0,     0,  -238,     0,     0,     0,
       0,   -19,     0,   366,     0,   -19,   -19,     0,   -19,     0,
       0,     0,     0,     0,     0,  -238,  -238,  -238,  -238,  -238,
    -238,  -238,  -238,  -238,  -238,  -238,  -238,  -238,  -238,  -238,
    -238,   513,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   514,     0,     0,     0,     0,   -19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   -19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   -19,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,     0,     0,   -19,     0,     0,     0,   -19
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-441)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     1,   133,     0,     1,    36,     0,     1,     8,     0,
       1,     8,     0,     1,    39,    40,    41,     8,    19,    73,
       8,    60,    60,     0,     1,    60,   217,    60,     0,     1,
       4,     8,   228,     4,     4,   248,   305,    24,    27,    39,
      40,    41,    24,   417,    20,   132,    22,    29,    16,     5,
       5,    32,    41,   159,    44,   307,    48,     5,    70,    60,
      28,   501,    47,     5,   500,     3,    32,    45,     3,    81,
      89,    52,   324,    24,     5,    37,   512,   517,    59,    57,
      65,     3,   273,     3,     3,    32,    52,    49,   279,    74,
      25,    53,    77,    59,   113,   149,    58,    75,    60,    61,
      48,     3,    64,    27,    24,    52,   193,   194,    19,    20,
      21,    22,    59,    24,   310,    96,    24,    41,   331,   173,
      82,    29,    24,    99,   211,    23,   126,    55,   502,   126,
     106,   131,   126,    89,    89,   126,    47,   127,   126,   128,
     140,   128,   113,   113,   114,   127,   171,    89,   254,   126,
     150,    82,    31,   127,    65,    52,   156,   127,    56,   156,
     125,   126,    59,    74,    23,   156,    77,    30,   156,    48,
      49,   171,   363,   304,   443,   143,     6,    40,    48,   156,
     195,   196,   197,    13,   185,    55,   186,   187,    99,   186,
     187,   322,    95,   193,    48,   186,   187,   328,   186,   187,
       6,    55,   170,     6,   204,     8,   260,    13,     0,   186,
     187,   424,   251,   251,     3,    37,   251,   217,   251,    56,
     251,   119,   120,   121,   122,   123,   124,    49,     7,    24,
       6,    53,   114,    28,     3,     5,    31,    13,    60,    61,
     113,    36,    64,     3,   241,   128,   129,   130,   248,    71,
     251,   251,    13,    23,   251,   255,    44,    91,    92,   125,
     251,     4,    32,   251,     4,     5,     4,    62,    63,    24,
     261,     4,     5,   273,   251,    65,   330,    72,    73,   279,
       4,     5,    52,   337,   415,   127,    56,     4,     5,    59,
       4,     5,    87,   293,    94,    91,   268,   297,   266,   495,
     297,     4,     5,   297,     3,   302,   297,    27,   308,   297,
      49,   442,   106,   313,    53,   115,   116,   117,   118,    89,
     297,    60,    61,   348,    42,    43,    96,   115,   116,   117,
     118,   331,   115,   116,   117,   118,    23,   125,   126,   127,
     128,   129,   130,    29,    37,    32,     4,     5,   348,   119,
     120,   121,   122,   123,   124,     4,    49,   127,   198,   199,
      53,   320,   321,   363,     3,    52,     3,    60,    61,     5,
      58,    64,    59,    66,    68,   429,    58,    37,    90,   200,
     201,   202,   203,    92,   423,   423,    79,    25,   423,    49,
     423,     4,   423,    53,    24,    97,    24,    81,    58,    82,
      60,    61,    24,    24,    64,    24,    24,    58,   462,    96,
       5,   119,     5,     5,    90,     4,   114,     4,   418,   416,
      25,    48,   423,    13,   424,     4,    24,   427,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   441,     5,    50,     3,   127,    24,
     119,    33,    51,    67,     5,   455,    33,     5,   455,   484,
      93,   455,     1,   512,   455,   482,   317,   455,   509,   500,
     303,   416,   401,   293,   443,   232,   476,   477,   455,     8,
     477,   512,   186,   477,   484,     3,   477,   471,     6,   477,
       8,   187,    60,    60,   259,   308,   471,   171,   240,   313,
     477,    19,    20,    21,    22,    23,    24,   300,   205,   476,
      28,   206,   456,    31,   301,   161,    34,    35,    36,   254,
     252,    39,   423,   437,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    -1,    72,    73,    74,    -1,    76,    -1,
      78,    -1,    -1,    81,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,    -1,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    70,    -1,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    -1,    81,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,    -1,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    -1,    -1,    31,    32,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,    -1,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,    -1,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,    -1,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,   110,   111,   112,     3,     4,    -1,     6,    -1,
       8,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    19,    20,    21,    22,    -1,    24,    -1,    -1,    27,
      28,    -1,    -1,    31,    -1,    -1,    34,    -1,    36,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    73,    74,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    83,    84,    85,    86,    87,
      88,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    99,    -1,    28,    -1,    -1,    31,    -1,   106,    34,
      -1,    36,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    70,    -1,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,    84,
      85,    86,    87,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,     3,    -1,    -1,
       6,   106,     8,    -1,    -1,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,
     125,   126,    28,   128,    -1,    31,    -1,    -1,    34,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    83,    84,    85,
      86,    87,    88,    -1,    -1,    91,    19,    20,    21,    22,
      -1,    24,    -1,    99,    27,    28,    -1,    -1,    31,    -1,
     106,    34,    -1,    36,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      73,    74,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,
      83,    84,    85,    86,    87,    88,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    24,    -1,    99,    -1,    28,    -1,
      -1,    31,    -1,   106,    34,    -1,    36,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    73,    74,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    83,    84,    85,    86,    87,    88,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    99,
      -1,    28,    -1,    -1,    31,    -1,   106,    34,    -1,    36,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    -1,     3,
      -1,    -1,     6,    -1,     8,    -1,    83,    84,    85,    86,
      87,    88,    -1,    -1,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    99,    -1,    28,    -1,    -1,    31,    -1,   106,
      34,    -1,    36,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,    -1,    -1,    -1,    -1,    -1,     3,    62,    63,
       6,    65,     8,    23,    -1,    -1,    -1,    -1,    72,    73,
      74,    -1,    32,    19,    20,    21,    22,    -1,    24,    83,
      -1,    -1,    28,    87,    88,    31,    -1,    -1,    34,    -1,
      36,    -1,    52,    -1,    -1,    99,    -1,    -1,    -1,    59,
      -1,    47,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,   125,   126,    -1,   128,    -1,    72,    73,    74,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    96,    -1,    59,    -1,
      -1,    87,    88,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    99,    -1,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,    -1,    52,    -1,    -1,    96,    -1,    -1,    -1,    59,
      -1,    -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    -1,
      -1,     0,    -1,    24,    -1,     4,     5,    -1,     7,    -1,
      -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    93,    -1,    -1,    -1,    97
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     6,     8,    19,    20,    21,    22,    23,    24,
      28,    31,    34,    35,    36,    39,    46,    47,    54,    56,
      62,    63,    65,    69,    70,    72,    73,    74,    76,    78,
      81,    83,    84,    85,    86,    87,    88,    99,   106,   110,
     111,   112,   119,   120,   121,   122,   123,   124,   125,   126,
     128,   134,   135,   144,   145,   146,   148,   149,   150,   151,
     153,   154,   155,   156,   161,   165,   181,   182,   183,   184,
     185,   186,   188,   189,   191,   192,   193,   194,   196,   197,
     199,   200,   202,   203,   206,   207,   208,   212,   214,   215,
     216,   217,   218,   219,   231,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   252,   253,   254,
     255,   256,   257,   268,   269,   270,   272,   273,   274,   275,
     286,   135,   146,   185,   252,    24,     3,    23,   182,    25,
     267,     3,     3,    48,     3,   208,    55,   144,    27,    41,
     157,   208,    95,   175,   176,   224,   225,   226,   228,     3,
       3,   214,   216,   231,   239,   270,    91,   154,   252,   280,
     281,   285,   252,   253,   253,   253,   239,   239,     0,     7,
      52,    59,   144,   189,   190,   192,   193,   196,   199,   201,
     204,    45,    57,    75,   147,    56,   114,   113,   153,    20,
      22,    99,   106,     3,     3,   128,   129,   130,   125,   126,
     115,   116,   117,   118,   127,   125,    13,     4,     4,     4,
       4,    70,   146,   185,   232,   252,   128,   157,   176,    24,
     259,   260,   261,    65,   128,   166,   209,   210,   252,   258,
      24,   208,   263,   264,   265,    37,    49,    53,    60,    61,
      64,    71,   220,   223,   152,   153,   252,   185,    92,   281,
     282,    91,   276,   277,   280,     3,   198,   208,   238,   253,
      32,    96,    27,    28,    72,   153,   205,    27,   230,   144,
     183,   184,     4,   157,   250,   251,   252,   283,   284,   157,
     240,   240,   240,   241,   241,   242,   242,   242,   242,   252,
     257,    24,   269,   157,    29,     4,   250,    82,   164,   177,
       6,    13,     3,   139,    48,    55,   170,   171,     5,    24,
      29,   213,   271,     3,   266,    29,   173,   287,   173,   174,
       6,    13,    58,   223,   158,   159,   270,    68,    58,     4,
       5,    89,   283,    90,   144,   154,   187,   189,   190,   192,
     196,   199,   278,   279,   282,   277,   249,   252,    44,   195,
     195,   153,    24,   216,   208,   146,    47,    65,    74,    77,
     180,   250,     4,     5,   250,   166,    24,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   233,   234,   235,   236,     4,   185,    97,   288,
     289,    24,   262,    24,   261,   262,   137,   270,   151,   172,
     176,   162,   163,   211,   286,   164,   210,   271,   251,    24,
     139,    24,   265,   265,   176,    58,     5,   164,   119,   176,
     153,   283,   153,     5,    89,    90,     4,     5,   253,   114,
       4,   250,     4,    13,    25,     4,    24,   290,   291,    13,
       4,     5,     5,     5,   170,    50,   178,     4,     3,   138,
     176,   159,   288,   160,   252,    66,    79,   221,   222,   229,
     144,   154,   189,   192,   196,   199,   279,   283,   252,   153,
      24,   119,   291,   270,   176,   163,    33,    51,   179,   136,
     271,   185,     3,   263,    59,   153,    24,   180,   212,   292,
     249,   185,    67,   140,     4,     5,   136,    42,    43,   227,
      33,    93,   167,   169,   271,     4,   141,   142,   154,   186,
     243,   288,     5,    30,    40,   143,   143,    94,   168,   142,
     243
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

  case 96:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 98:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 100:

    { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 102:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 104:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 114:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 115:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 116:

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

  case 123:

    {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
          (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 124:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 125:

    {    
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 126:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 127:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 128:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 129:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 130:

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

  case 131:

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

  case 132:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 133:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 134:

    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 135:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 136:

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

  case 137:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 138:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 139:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 140:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 141:

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

  case 142:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 143:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 144:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 145:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 149:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 150:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));}
    break;

  case 151:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 152:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 153:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
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

  case 164:

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

  case 165:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 166:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 175:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 176:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 177:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 179:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 180:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 181:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 189:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 190:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 191:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 192:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 195:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 196:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 198:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 199:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 200:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 201:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 203:

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

  case 204:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 209:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 210:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 230:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 231:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 232:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 235:

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

  case 241:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 245:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 246:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 247:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 249:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 250:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 251:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 253:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 254:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 256:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 257:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 258:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 259:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 260:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 261:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 262:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 263:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 264:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 265:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 266:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 267:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 268:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 269:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 270:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 271:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 273:

    {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 274:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 281:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 282:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 286:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 287:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 288:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 289:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 290:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 291:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 292:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 293:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 294:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 295:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
            }
    break;

  case 296:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;            
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 297:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 298:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 299:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 300:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 301:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 302:

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

  case 303:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 304:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 305:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 310:

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
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 312:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 313:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 314:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 315:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 316:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 323:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 324:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 325:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 326:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 327:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 331:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 332:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 333:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
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

  case 337:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 339:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 340:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 341:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
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
