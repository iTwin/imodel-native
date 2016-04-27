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
#ifndef YY_SQLYY_D_DEV_BSW_DGNDB_0601DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_D_DEV_BSW_DGNDB_0601DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
     SQL_TOKEN_NULLIF = 404,
     SQL_TOKEN_COALESCE = 405,
     SQL_TOKEN_WHEN = 406,
     SQL_TOKEN_ELSE = 407,
     SQL_TOKEN_BEFORE = 408,
     SQL_TOKEN_AFTER = 409,
     SQL_TOKEN_INSTEAD = 410,
     SQL_TOKEN_EACH = 411,
     SQL_TOKEN_REFERENCING = 412,
     SQL_TOKEN_BEGIN = 413,
     SQL_TOKEN_ATOMIC = 414,
     SQL_TOKEN_TRIGGER = 415,
     SQL_TOKEN_ROW = 416,
     SQL_TOKEN_STATEMENT = 417,
     SQL_TOKEN_NEW = 418,
     SQL_TOKEN_OLD = 419,
     SQL_TOKEN_VALUE = 420,
     SQL_TOKEN_CURRENT_CATALOG = 421,
     SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP = 422,
     SQL_TOKEN_CURRENT_PATH = 423,
     SQL_TOKEN_CURRENT_ROLE = 424,
     SQL_TOKEN_CURRENT_SCHEMA = 425,
     SQL_TOKEN_VARCHAR = 426,
     SQL_TOKEN_VARBINARY = 427,
     SQL_TOKEN_VARYING = 428,
     SQL_TOKEN_OBJECT = 429,
     SQL_TOKEN_NCLOB = 430,
     SQL_TOKEN_NATIONAL = 431,
     SQL_TOKEN_LARGE = 432,
     SQL_TOKEN_CLOB = 433,
     SQL_TOKEN_BLOB = 434,
     SQL_TOKEN_BIGI = 435,
     SQL_TOKEN_INTERVAL = 436,
     SQL_TOKEN_OVER = 437,
     SQL_TOKEN_ROW_NUMBER = 438,
     SQL_TOKEN_NTILE = 439,
     SQL_TOKEN_LEAD = 440,
     SQL_TOKEN_LAG = 441,
     SQL_TOKEN_RESPECT = 442,
     SQL_TOKEN_IGNORE = 443,
     SQL_TOKEN_NULLS = 444,
     SQL_TOKEN_FIRST_VALUE = 445,
     SQL_TOKEN_LAST_VALUE = 446,
     SQL_TOKEN_NTH_VALUE = 447,
     SQL_TOKEN_FIRST = 448,
     SQL_TOKEN_LAST = 449,
     SQL_TOKEN_EXCLUDE = 450,
     SQL_TOKEN_OTHERS = 451,
     SQL_TOKEN_TIES = 452,
     SQL_TOKEN_FOLLOWING = 453,
     SQL_TOKEN_UNBOUNDED = 454,
     SQL_TOKEN_PRECEDING = 455,
     SQL_TOKEN_RANGE = 456,
     SQL_TOKEN_ROWS = 457,
     SQL_TOKEN_PARTITION = 458,
     SQL_TOKEN_WINDOW = 459,
     SQL_TOKEN_NO = 460,
     SQL_TOKEN_GETECCLASSID = 461,
     SQL_TOKEN_LIMIT = 462,
     SQL_TOKEN_OFFSET = 463,
     SQL_TOKEN_NEXT = 464,
     SQL_TOKEN_ONLY = 465,
     SQL_TOKEN_MATCH = 466,
     SQL_TOKEN_ECSQLOPTIONS = 467,
     SQL_TOKEN_BINARY = 468,
     SQL_TOKEN_BOOLEAN = 469,
     SQL_TOKEN_DOUBLE = 470,
     SQL_TOKEN_INTEGER = 471,
     SQL_TOKEN_INT = 472,
     SQL_TOKEN_LONG = 473,
     SQL_TOKEN_INT64 = 474,
     SQL_TOKEN_STRING = 475,
     SQL_TOKEN_DATE = 476,
     SQL_TOKEN_TIMESTAMP = 477,
     SQL_TOKEN_DATETIME = 478,
     SQL_TOKEN_OR = 479,
     SQL_TOKEN_AND = 480,
     SQL_EQUAL = 481,
     SQL_GREAT = 482,
     SQL_LESS = 483,
     SQL_NOTEQUAL = 484,
     SQL_GREATEQ = 485,
     SQL_LESSEQ = 486,
     SQL_CONCAT = 487,
     SQL_TOKEN_INVALIDSYMBOL = 488
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

#endif /* !YY_SQLYY_D_DEV_BSW_DGNDB_0601DEV_SRC_ECDB_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */

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
#define YYFINAL  247
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4631

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  259
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  304
/* YYNRULES -- Number of rules.  */
#define YYNRULES  631
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1027

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   488

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   255,   243,     2,
       3,     4,   253,   250,     5,   251,    13,   254,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   257,     2,     8,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,    14,     2,    15,     2,     2,
      18,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   242,    12,   256,     2,     2,     2,
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
     241,   244,   245,   246,   247,   248,   249,   252,   258
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
    1055,  1057,  1059,  1061,  1063,  1070,  1072,  1074,  1076,  1078,
    1080,  1082,  1084,  1088,  1090,  1094,  1100,  1102,  1104,  1106,
    1109,  1112,  1114,  1118,  1122,  1126,  1128,  1132,  1136,  1138,
    1140,  1142,  1145,  1148,  1150,  1152,  1154,  1156,  1158,  1160,
    1162,  1164,  1167,  1169,  1172,  1175,  1178,  1182,  1184,  1186,
    1190,  1194,  1196,  1198,  1202,  1206,  1208,  1210,  1212,  1214,
    1216,  1218,  1222,  1226,  1228,  1230,  1233,  1235,  1238,  1240,
    1242,  1244,  1252,  1254,  1256,  1257,  1259,  1261,  1263,  1265,
    1266,  1269,  1277,  1282,  1284,  1286,  1291,  1298,  1305,  1312,
    1315,  1317,  1319,  1321,  1325,  1329,  1333,  1335,  1336,  1338,
    1340,  1344,  1347,  1349,  1351,  1353,  1354,  1358,  1359,  1361,
    1365,  1368,  1370,  1372,  1374,  1376,  1378,  1381,  1384,  1388,
    1392,  1395,  1397,  1398,  1400,  1404,  1405,  1407,  1411,  1414,
    1415,  1417,  1419,  1421,  1423,  1425,  1430,  1435,  1438,  1442,
    1446,  1449,  1454,  1459,  1463,  1465,  1471,  1476,  1479,  1482,
    1486,  1489,  1491,  1496,  1499,  1501,  1503,  1504,  1508,  1514,
    1516,  1518,  1520,  1522,  1524,  1526,  1528,  1530,  1532,  1535,
    1538,  1540,  1542,  1544,  1549,  1554,  1559,  1561,  1563,  1569,
    1574,  1576,  1579,  1584,  1586,  1590,  1592,  1594,  1596,  1598,
    1600,  1602,  1604,  1607,  1612,  1613,  1616,  1618,  1620,  1622,
    1624,  1627,  1629,  1630,  1633,  1635,  1639,  1649,  1650,  1653,
    1655,  1657,  1660,  1662,  1664,  1667,  1668,  1671,  1673,  1677,
    1678,  1682,  1684,  1686,  1687,  1690,  1692,  1698,  1700,  1704,
    1706,  1708,  1711,  1716,  1721,  1726,  1731,  1733,  1735,  1737,
    1739,  1741,  1743,  1744,  1746,  1749,  1752,  1754,  1756,  1760,
    1762,  1764
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     260,     0,    -1,   261,    -1,   261,     7,    -1,   283,    -1,
     262,    -1,   263,    -1,   274,    -1,   537,    -1,    47,   117,
     487,     3,   264,     4,    -1,   265,    -1,   264,     5,   265,
      -1,   266,    -1,   271,    -1,   518,   495,   267,    -1,    -1,
     267,   270,    -1,   454,    -1,   122,    -1,   103,    84,    -1,
      23,    94,    -1,   269,    -1,    56,   363,    -1,    56,    94,
      -1,    56,   268,    -1,    41,    -1,    41,     3,   336,     4,
      -1,   107,   487,    -1,   107,   487,     3,   272,     4,    -1,
     269,     3,   272,     4,    -1,    70,    84,     3,   272,     4,
     107,   487,    -1,    70,    84,     3,   272,     4,   107,   487,
       3,   272,     4,    -1,    41,     3,   336,     4,    -1,   272,
       5,   518,    -1,   518,    -1,   273,     5,   494,    -1,   494,
      -1,    47,   128,   487,   277,    31,   304,   275,    -1,    -1,
     130,    41,    99,    -1,    -1,     3,   272,     4,    -1,    -1,
       3,   273,     4,    -1,    -1,   100,    38,   279,    -1,   280,
      -1,   279,     5,   280,    -1,   337,   281,    -1,   293,   281,
      -1,    -1,    32,    -1,    58,    -1,    -1,    23,    -1,   286,
      -1,   287,    -1,   288,    -1,   289,    -1,   294,    -1,   295,
      -1,   300,    -1,   284,    -1,   304,    -1,   304,   285,   442,
     284,    -1,    82,    -1,   121,    -1,    64,    -1,    43,   131,
      -1,    57,    72,   325,   303,   558,    -1,    67,   534,    80,
     301,    -1,    79,    80,   487,   277,   290,    -1,   127,     3,
     291,     4,    -1,   292,    -1,   291,     5,   292,    -1,   293,
      -1,   466,    -1,   108,   131,    -1,   110,   296,   305,    80,
     301,   318,    -1,    -1,    27,    -1,    59,    -1,   298,    -1,
     297,     5,   298,    -1,   494,   244,   299,    -1,   466,    -1,
      56,    -1,   124,   325,   111,   297,   303,   558,    -1,   302,
      -1,   301,     5,   302,    -1,   362,    -1,    -1,   326,    -1,
     110,   296,   305,   318,    -1,   290,    -1,   253,    -1,   360,
      -1,    -1,   307,    -1,   224,   313,   310,    -1,    -1,   314,
      -1,   209,    -1,   225,    -1,   177,    -1,   218,    -1,    -1,
     312,    -1,    67,   309,   308,   310,   226,    -1,   363,    -1,
     363,    -1,    -1,   317,    -1,    -1,   224,   452,    -1,   223,
     452,   316,    -1,   319,   303,   327,   328,   404,   278,   315,
     306,   311,   558,    -1,    72,   320,    -1,   325,    -1,   320,
       5,   325,    -1,    -1,    31,    -1,    -1,   177,    -1,    -1,
     321,    24,   276,    -1,    -1,   226,    -1,   324,   487,   323,
      -1,   324,   359,   536,   277,    -1,   440,    -1,   129,   336,
      -1,    -1,    74,    38,   463,    -1,    -1,    75,   336,    -1,
     120,    -1,    66,    -1,   123,    -1,    94,    -1,   337,    -1,
     331,    -1,     3,   336,     4,    -1,   466,    -1,     3,   336,
       4,    -1,   330,    -1,   330,    81,   282,   329,    -1,   333,
      -1,    23,   333,    -1,   334,    -1,   335,   241,   334,    -1,
     335,    -1,   336,   240,   335,    -1,   339,    -1,   342,    -1,
     353,    -1,   357,    -1,   358,    -1,   348,    -1,   351,    -1,
     345,    -1,   354,    -1,   340,   292,    -1,   292,   340,   292,
      -1,   340,   292,    -1,   246,    -1,   247,    -1,   244,    -1,
     245,    -1,   249,    -1,   248,    -1,    81,   282,    -1,   282,
      36,   292,   241,   292,    -1,   292,   341,    -1,   282,    85,
     467,   346,    -1,   282,    85,   447,   346,    -1,   292,   343,
      -1,   292,   344,    -1,   343,    -1,   344,    -1,    -1,    63,
     467,    -1,    81,   282,    94,    -1,   292,   347,    -1,   347,
      -1,   359,    -1,     3,   463,     4,    -1,   282,    76,   349,
      -1,   292,   350,    -1,   350,    -1,   340,   356,   359,    -1,
     292,   352,    -1,   292,   355,    -1,   282,   227,   376,    -1,
      30,    -1,    27,    -1,   113,    -1,    65,   359,    -1,   122,
     359,    -1,     3,   284,     4,    -1,   361,    -1,   360,     5,
     361,    -1,   486,    -1,   535,    -1,   233,    -1,    20,    -1,
      21,    -1,    22,    -1,    19,    -1,   363,   236,    -1,   363,
     233,    -1,   363,    20,    -1,   363,    22,    -1,    -1,    31,
     518,    -1,   518,    -1,   135,     3,   466,    76,   466,     4,
      -1,   135,     3,   463,     4,    -1,   365,    -1,   373,    -1,
     370,    -1,   134,     3,   466,     4,    -1,   137,     3,   466,
       4,    -1,    96,     3,   466,     4,    -1,   132,     3,   466,
       4,    -1,   367,    -1,   368,    -1,   369,    -1,   458,    -1,
     153,    -1,   371,    -1,   466,    -1,   142,     3,   372,    72,
     466,     4,    -1,   375,    -1,   363,    -1,   535,    -1,    94,
      -1,    66,    -1,   120,    -1,   181,    -1,   182,    -1,   183,
      -1,   184,    -1,   185,    -1,   186,    -1,   430,    -1,   379,
       3,     4,    -1,   377,     3,     4,    -1,   379,     3,   465,
       4,    -1,   378,     3,   465,     4,    -1,   379,     3,   296,
     464,     4,    -1,   380,    -1,   157,    -1,    24,    -1,   140,
      -1,   141,    -1,   382,   198,   402,    -1,   199,     3,     4,
      -1,   430,    -1,   383,    -1,   389,    -1,   395,    -1,   398,
      -1,   200,     3,   386,     4,    -1,   535,    -1,   363,    -1,
     385,    -1,   384,    -1,    -1,     5,   392,    -1,     5,   392,
       5,   393,    -1,    -1,   394,    -1,   390,     3,   391,   387,
       4,   388,    -1,   201,    -1,   202,    -1,   466,    -1,    21,
      -1,   466,    -1,   203,   205,    -1,   204,   205,    -1,   396,
       3,   466,     4,   388,    -1,   206,    -1,   207,    -1,    -1,
     400,    -1,   208,     3,   466,     5,   399,     4,   397,   388,
      -1,   385,    -1,   384,    -1,    72,   209,    -1,    72,   210,
      -1,    24,    -1,   401,    -1,   403,    -1,   409,    -1,    -1,
     405,    -1,   220,   406,    -1,   406,     5,   407,    -1,   407,
      -1,   408,    31,   409,    -1,   401,    -1,     3,   413,     4,
      -1,    -1,   414,    -1,    -1,   415,    -1,    -1,   419,    -1,
     410,   411,   278,   412,    -1,   401,    -1,   219,    38,   416,
      -1,   416,     5,   417,    -1,   417,    -1,   494,   497,    -1,
      -1,   429,    -1,   420,   421,   418,    -1,   218,    -1,   217,
      -1,   422,    -1,   424,    -1,   215,   216,    -1,   423,    -1,
      49,   177,    -1,   374,   216,    -1,    36,   425,   241,   426,
      -1,   427,    -1,   427,    -1,   422,    -1,   215,   214,    -1,
     428,    -1,   374,   214,    -1,   211,    49,   177,    -1,   211,
      74,    -1,   211,   213,    -1,   211,   221,   212,    -1,   431,
       3,   296,   464,     4,    -1,    46,     3,   253,     4,    -1,
      46,     3,   296,   464,     4,    -1,    35,    -1,    90,    -1,
      91,    -1,   116,    -1,   159,    -1,    30,    -1,   113,    -1,
      87,    -1,    88,    -1,    73,    -1,    98,   336,    -1,   433,
      -1,   441,    -1,    -1,    78,    -1,   432,    -1,   432,   101,
      -1,   325,    48,    83,   325,    -1,   325,    92,   435,    83,
     325,    -1,   325,   435,    83,   325,   434,    -1,   436,    -1,
     325,   435,    83,   325,   126,   487,   439,    -1,    -1,    61,
      -1,    62,    -1,   438,    -1,   437,    -1,   126,     3,   272,
       4,    -1,    -1,    27,    -1,   359,    -1,   466,    -1,   229,
      -1,   230,    -1,   231,    -1,   232,    -1,   233,    -1,   234,
      -1,   235,    -1,   236,    -1,   239,    -1,   237,    -1,   238,
      -1,    24,    -1,    39,     3,   444,    31,   445,     4,    -1,
     374,    -1,   376,    -1,   448,    -1,   494,    -1,   443,    -1,
     519,    -1,   381,    -1,     3,   466,     4,    -1,   446,    -1,
     222,     3,     4,    -1,   492,    13,   222,     3,     4,    -1,
     447,    -1,   366,    -1,   449,    -1,   251,   449,    -1,   250,
     449,    -1,   450,    -1,   451,   253,   450,    -1,   451,   254,
     450,    -1,   451,   255,   450,    -1,   451,    -1,   452,   250,
     451,    -1,   452,   251,   451,    -1,   454,    -1,   138,    -1,
     139,    -1,   237,   467,    -1,   238,   467,    -1,   453,    -1,
     455,    -1,   456,    -1,   158,    -1,   149,    -1,    52,    -1,
     147,    -1,   148,    -1,   458,   500,    -1,   458,    -1,   153,
     500,    -1,   458,   500,    -1,   153,   512,    -1,   459,   118,
     460,    -1,   461,    -1,   466,    -1,   463,     5,   466,    -1,
     463,     7,   466,    -1,   531,    -1,   464,    -1,   465,     5,
     464,    -1,   465,     7,   464,    -1,   452,    -1,   467,    -1,
     457,    -1,   468,    -1,   472,    -1,   469,    -1,   468,   250,
     472,    -1,   466,   252,   466,    -1,   236,    -1,   473,    -1,
      42,   487,    -1,   470,    -1,   470,   471,    -1,   479,    -1,
     474,    -1,   475,    -1,   136,     3,   476,    72,   467,   480,
       4,    -1,   477,    -1,   478,    -1,    -1,   481,    -1,   483,
      -1,   484,    -1,   485,    -1,    -1,    69,   466,    -1,   136,
       3,   466,    72,   466,   480,     4,    -1,   136,     3,   463,
       4,    -1,   125,    -1,    89,    -1,   482,     3,   466,     4,
      -1,    45,     3,   467,   126,   487,     4,    -1,    45,     3,
     444,     5,   445,     4,    -1,   119,     3,   467,   126,   487,
       4,    -1,   466,   364,    -1,   490,    -1,   489,    -1,   488,
      -1,    24,    13,   489,    -1,    24,     6,   489,    -1,    24,
      13,   490,    -1,    24,    -1,    -1,    25,    -1,   493,    -1,
     492,    13,   493,    -1,    24,   491,    -1,   253,    -1,   492,
      -1,   498,    -1,    -1,    40,   111,    24,    -1,    -1,   471,
      -1,   499,   496,   497,    -1,   507,   497,    -1,   509,    -1,
     511,    -1,   515,    -1,   516,    -1,   517,    -1,    40,   500,
      -1,   133,   500,    -1,    40,   189,   501,    -1,   133,   189,
     501,    -1,   187,   501,    -1,   506,    -1,    -1,   501,    -1,
       3,    21,     4,    -1,    -1,   503,    -1,     3,   504,     4,
      -1,    21,   505,    -1,    -1,    14,    -1,    15,    -1,    16,
      -1,    17,    -1,    18,    -1,    40,   193,   190,   502,    -1,
     133,   193,   190,   502,    -1,   194,   502,    -1,   192,    40,
     500,    -1,   192,   133,   500,    -1,    93,   500,    -1,   192,
      40,   189,   501,    -1,   192,   133,   189,   501,    -1,    93,
     189,   501,    -1,   508,    -1,   192,    40,   193,   190,   502,
      -1,    93,   193,   190,   502,    -1,   191,   502,    -1,   229,
     500,    -1,   229,   189,   501,    -1,   188,   501,    -1,   510,
      -1,   229,   193,   190,   502,    -1,   195,   502,    -1,   513,
      -1,   514,    -1,    -1,     3,    21,     4,    -1,     3,    21,
       5,    21,     4,    -1,   232,    -1,   233,    -1,   234,    -1,
     235,    -1,    68,    -1,   106,    -1,   231,    -1,   230,    -1,
     237,    -1,   238,   500,    -1,   197,   462,    -1,    24,    -1,
     520,    -1,   521,    -1,   165,     3,   463,     4,    -1,   166,
       3,   466,     4,    -1,   166,     3,   463,     4,    -1,   522,
      -1,   523,    -1,   162,   533,   524,   530,   164,    -1,   162,
     528,   530,   164,    -1,   525,    -1,   528,   525,    -1,   167,
     526,   163,   531,    -1,   527,    -1,   526,     5,   527,    -1,
     293,    -1,   338,    -1,   341,    -1,   350,    -1,   343,    -1,
     347,    -1,   529,    -1,   528,   529,    -1,   167,   336,   163,
     531,    -1,    -1,   168,   531,    -1,   532,    -1,   466,    -1,
     293,    -1,    24,    -1,     6,    24,    -1,     8,    -1,    -1,
     321,    24,    -1,   336,    -1,     3,   261,     4,    -1,    47,
     176,   557,   539,   540,    98,   490,   538,   543,    -1,    -1,
     173,   550,    -1,   169,    -1,   170,    -1,   171,    97,    -1,
      79,    -1,    57,    -1,   124,   541,    -1,    -1,    97,   542,
      -1,   272,    -1,   544,   546,   547,    -1,    -1,    69,   172,
     545,    -1,   177,    -1,   178,    -1,    -1,   167,   332,    -1,
     549,    -1,   174,   175,   548,     7,   164,    -1,   549,    -1,
     548,     7,   549,    -1,   261,    -1,   551,    -1,   550,   551,
      -1,   180,   322,   321,   555,    -1,   179,   322,   321,   556,
      -1,   180,   117,   321,   552,    -1,   179,   117,   321,   553,
      -1,   554,    -1,   554,    -1,    24,    -1,    24,    -1,    24,
      -1,    24,    -1,    -1,   559,    -1,   228,   560,    -1,   560,
     561,    -1,   561,    -1,    24,    -1,    24,   244,   562,    -1,
     363,    -1,    24,    -1,   329,    -1
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
    1301,  1302,  1318,  1327,  1333,  1349,  1353,  1361,  1370,  1376,
    1392,  1401,  1426,  1435,  1445,  1446,  1447,  1451,  1459,  1465,
    1476,  1481,  1497,  1502,  1534,  1535,  1536,  1537,  1538,  1540,
    1552,  1564,  1576,  1592,  1593,  1599,  1602,  1612,  1622,  1623,
    1624,  1627,  1635,  1646,  1656,  1666,  1671,  1676,  1683,  1688,
    1696,  1697,  1728,  1740,  1741,  1744,  1745,  1746,  1747,  1748,
    1749,  1750,  1751,  1752,  1753,  1756,  1758,  1765,  1772,  1780,
    1794,  1806,  1809,  1813,  1817,  1819,  1846,  1855,  1862,  1863,
    1864,  1865,  1866,  1869,  1879,  1882,  1885,  1886,  1889,  1890,
    1896,  1906,  1907,  1911,  1923,  1924,  1927,  1930,  1933,  1936,
    1937,  1940,  1951,  1952,  1955,  1956,  1959,  1973,  1974,  1977,
    1983,  1991,  1994,  1995,  1998,  2001,  2002,  2005,  2013,  2016,
    2021,  2030,  2033,  2042,  2043,  2046,  2047,  2050,  2051,  2054,
    2060,  2063,  2072,  2075,  2080,  2088,  2089,  2092,  2101,  2102,
    2105,  2106,  2109,  2115,  2116,  2124,  2132,  2142,  2145,  2148,
    2149,  2155,  2158,  2166,  2173,  2179,  2185,  2205,  2214,  2222,
    2234,  2235,  2236,  2237,  2238,  2239,  2240,  2244,  2249,  2254,
    2261,  2269,  2270,  2273,  2274,  2279,  2280,  2288,  2300,  2310,
    2319,  2324,  2338,  2339,  2340,  2343,  2344,  2347,  2359,  2360,
    2364,  2367,  2371,  2372,  2373,  2374,  2375,  2376,  2377,  2378,
    2379,  2380,  2381,  2382,  2386,  2398,  2399,  2400,  2401,  2402,
    2403,  2404,  2405,  2412,  2418,  2426,  2438,  2439,  2443,  2444,
    2450,  2459,  2460,  2467,  2474,  2484,  2485,  2492,  2506,  2513,
    2519,  2524,  2530,  2563,  2576,  2603,  2664,  2665,  2666,  2667,
    2668,  2671,  2679,  2680,  2689,  2695,  2704,  2711,  2716,  2719,
    2723,  2736,  2763,  2766,  2770,  2783,  2784,  2785,  2788,  2796,
    2797,  2800,  2807,  2817,  2818,  2821,  2829,  2830,  2838,  2839,
    2842,  2849,  2862,  2886,  2893,  2906,  2907,  2908,  2913,  2920,
    2921,  2929,  2940,  2950,  2951,  2954,  2964,  2974,  2986,  2999,
    3008,  3013,  3018,  3025,  3032,  3041,  3051,  3059,  3060,  3068,
    3073,  3091,  3097,  3105,  3175,  3178,  3179,  3188,  3189,  3192,
    3199,  3205,  3206,  3207,  3208,  3209,  3212,  3218,  3224,  3231,
    3238,  3244,  3247,  3248,  3251,  3260,  3261,  3264,  3274,  3282,
    3283,  3288,  3293,  3298,  3303,  3310,  3318,  3326,  3334,  3341,
    3348,  3354,  3362,  3370,  3377,  3380,  3389,  3397,  3405,  3411,
    3418,  3424,  3427,  3435,  3443,  3444,  3447,  3448,  3455,  3489,
    3490,  3491,  3492,  3509,  3510,  3511,  3522,  3525,  3534,  3563,
    3574,  3596,  3597,  3600,  3608,  3616,  3626,  3627,  3630,  3641,
    3651,  3656,  3663,  3673,  3676,  3681,  3682,  3683,  3684,  3685,
    3686,  3689,  3694,  3701,  3711,  3712,  3720,  3724,  3727,  3730,
    3743,  3749,  3773,  3776,  3786,  3800,  3803,  3818,  3821,  3829,
    3830,  3831,  3839,  3840,  3841,  3849,  3852,  3860,  3863,  3872,
    3875,  3884,  3885,  3888,  3891,  3899,  3900,  3911,  3916,  3923,
    3927,  3932,  3940,  3948,  3956,  3964,  3974,  3977,  3980,  3983,
    3986,  3989,  3993,  3994,  3998,  4007,  4012,  4020,  4026,  4036,
    4037,  4038
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
  "scalar_subquery", "cast_operand", "cast_target", "cast_spec",
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
     479,   480,   124,    38,   481,   482,   483,   484,   485,   486,
      43,    45,   487,    42,    47,    37,   126,    61,   488
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   259,   260,   260,   261,   261,   262,   262,   262,   263,
     264,   264,   265,   265,   266,   267,   267,   268,   269,   269,
     270,   270,   270,   270,   270,   270,   270,   270,   270,   271,
     271,   271,   271,   272,   272,   273,   273,   274,   275,   275,
     276,   276,   277,   277,   278,   278,   279,   279,   280,   280,
     281,   281,   281,   282,   282,   283,   283,   283,   283,   283,
     283,   283,   283,   284,   284,   285,   285,   285,   286,   287,
     288,   289,   290,   291,   291,   292,   293,   294,   295,   296,
     296,   296,   297,   297,   298,   299,   299,   300,   301,   301,
     302,   303,   303,   304,   304,   305,   305,   306,   306,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   312,   313,
     314,   315,   315,   316,   316,   317,   318,   319,   320,   320,
     321,   321,   322,   322,   323,   323,   324,   324,   325,   325,
     325,   326,   327,   327,   328,   328,   329,   329,   329,   329,
     330,   330,   330,   331,   332,   333,   333,   334,   334,   335,
     335,   336,   336,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   338,   339,   339,   340,   340,   340,   340,   340,
     340,   340,   341,   342,   343,   344,   345,   345,   345,   345,
     346,   346,   347,   348,   348,   349,   349,   350,   351,   351,
     352,   353,   354,   355,   356,   356,   356,   357,   358,   359,
     360,   360,   361,   362,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   364,   364,   364,   365,   365,   366,   366,
     366,   367,   367,   368,   369,   370,   370,   370,   371,   371,
     372,   372,   373,   374,   374,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   376,   376,   376,   376,   376,
     376,   377,   378,   379,   380,   380,   381,   382,   382,   382,
     382,   382,   382,   383,   384,   385,   386,   386,   387,   387,
     387,   388,   388,   389,   390,   390,   391,   392,   393,   394,
     394,   395,   396,   396,   397,   397,   398,   399,   399,   400,
     400,   401,   402,   402,   403,   404,   404,   405,   406,   406,
     407,   408,   409,   410,   410,   411,   411,   412,   412,   413,
     414,   415,   416,   416,   417,   418,   418,   419,   420,   420,
     421,   421,   422,   422,   422,   423,   424,   425,   426,   427,
     427,   427,   428,   429,   429,   429,   429,   430,   430,   430,
     431,   431,   431,   431,   431,   431,   431,   432,   432,   432,
     433,   434,   434,   435,   435,   435,   435,   436,   437,   437,
     437,   438,   439,   439,   439,   440,   440,   441,   442,   442,
     443,   444,   445,   445,   445,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   446,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   448,   448,   449,   449,   450,   450,
     450,   451,   451,   451,   451,   452,   452,   452,   453,   454,
     454,   454,   454,   455,   456,   457,   458,   458,   458,   458,
     458,   459,   460,   460,   461,   461,   462,   462,   463,   463,
     463,   464,   465,   465,   465,   466,   466,   466,   467,   468,
     468,   469,   469,   470,   470,   471,   472,   472,   473,   473,
     474,   475,   476,   477,   478,   479,   479,   479,   479,   480,
     480,   481,   481,   482,   482,   483,   484,   484,   485,   486,
     487,   487,   487,   488,   488,   489,   490,   491,   491,   492,
     492,   493,   493,   494,   495,   496,   496,   497,   497,   498,
     498,   498,   498,   498,   498,   498,   499,   499,   499,   499,
     499,   499,   500,   500,   501,   502,   502,   503,   504,   505,
     505,   505,   505,   505,   505,   506,   506,   506,   507,   507,
     507,   507,   507,   507,   507,   508,   508,   508,   509,   509,
     509,   509,   510,   510,   511,   511,   512,   512,   512,   513,
     513,   513,   513,   514,   514,   514,   515,   516,   516,   517,
     518,   519,   519,   520,   520,   520,   521,   521,   522,   523,
     524,   524,   525,   526,   526,   527,   527,   527,   527,   527,
     527,   528,   528,   529,   530,   530,   531,   532,   533,   534,
     535,   535,   536,   536,   261,   261,   537,   538,   538,   539,
     539,   539,   540,   540,   540,   541,   541,   542,   543,   544,
     544,   545,   545,   546,   546,   547,   547,   548,   548,   549,
     550,   550,   551,   551,   551,   551,   552,   553,   554,   555,
     556,   557,   558,   558,   559,   560,   560,   561,   561,   562,
     562,   562
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
       1,     1,     1,     1,     6,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     5,     1,     1,     1,     2,
       2,     1,     3,     3,     3,     1,     3,     3,     1,     1,
       1,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     2,     2,     2,     3,     1,     1,     3,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     2,     1,     2,     1,     1,
       1,     7,     1,     1,     0,     1,     1,     1,     1,     0,
       2,     7,     4,     1,     1,     4,     6,     6,     6,     2,
       1,     1,     1,     3,     3,     3,     1,     0,     1,     1,
       3,     2,     1,     1,     1,     0,     3,     0,     1,     3,
       2,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       2,     1,     0,     1,     3,     0,     1,     3,     2,     0,
       1,     1,     1,     1,     1,     4,     4,     2,     3,     3,
       2,     4,     4,     3,     1,     5,     4,     2,     2,     3,
       2,     1,     4,     2,     1,     1,     0,     3,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       1,     1,     1,     4,     4,     4,     1,     1,     5,     4,
       1,     2,     4,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     4,     0,     2,     1,     1,     1,     1,
       2,     1,     0,     2,     1,     3,     9,     0,     2,     1,
       1,     2,     1,     1,     2,     0,     2,     1,     3,     0,
       3,     1,     1,     0,     2,     1,     5,     1,     3,     1,
       1,     2,     4,     4,     4,     4,     1,     1,     1,     1,
       1,     1,     0,     1,     2,     2,     1,     1,     3,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      53,    53,     0,   581,   208,   205,   206,   207,    53,   477,
     345,   340,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,    53,   464,   341,   342,   236,     0,     0,    79,
     346,   343,     0,   238,     0,   126,   463,     0,     0,     0,
       0,     0,     0,   409,   410,   254,   255,     0,   252,   344,
       0,     0,     0,   239,   240,   241,   242,   243,   244,     0,
       0,   274,   275,   282,   283,     0,     0,   204,   443,     0,
       0,   167,   168,   165,   166,   170,   169,     0,     0,   482,
       0,     2,     5,     6,     7,     0,     4,    62,    55,    56,
      57,    58,    94,    53,    75,    59,    60,    61,    63,   145,
     141,   147,   149,   151,   584,   140,   153,     0,   154,   178,
     179,   160,   184,   158,   189,   159,   155,   161,   156,   157,
     370,   234,   218,   397,   225,   226,   227,   220,   219,   385,
     233,   386,     0,     0,     0,   251,   391,     0,   259,   260,
       0,   261,     0,   262,   245,     0,   389,   393,   396,   387,
     398,   401,   405,   435,   413,   408,   414,   415,   437,   143,
     436,   438,   440,   446,   439,   444,   449,   450,   448,   455,
       0,   456,   457,   458,   483,   479,   388,   390,   551,   552,
     556,   557,   235,     8,     0,     0,     0,    76,   580,    53,
      54,   148,   478,   481,     0,    68,     0,    79,     0,     0,
       0,   126,     0,   197,   579,     0,     0,   171,     0,    77,
      80,    81,     0,     0,   198,   127,     0,   353,   360,   366,
     365,   130,     0,     0,     0,     0,   454,     0,     0,     0,
      53,   578,    76,   574,   571,     0,     0,     0,     0,     0,
       0,     0,     0,   411,   412,   400,   399,     1,     3,     0,
       0,     0,     0,   173,   176,   177,   183,   188,   191,   192,
      67,    65,    66,   368,    53,    53,    53,   164,   211,   212,
     210,   209,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,     0,     0,     0,     0,   447,     0,     0,
     585,   199,   142,   392,    79,     0,     0,     0,   371,     0,
     436,     0,     0,   476,     0,   472,   471,   470,    42,   621,
       0,    91,     0,    42,   182,     0,   482,     0,    96,   200,
     213,   202,   436,   582,   124,     0,   349,   354,   347,   348,
     353,     0,   355,     0,     0,    73,     0,     0,     0,   428,
       0,   428,     0,   452,   453,     0,   418,   419,   420,   417,
     229,   416,   230,     0,   228,   231,     0,     0,     0,   572,
       0,    53,   574,   560,     0,     0,   428,     0,   428,   257,
     265,   267,   266,     0,   264,     0,   394,     0,   187,   185,
     180,   180,     0,     0,   195,   194,   196,   163,     0,   369,
       0,     0,   150,   152,   247,   432,     0,   577,   431,   576,
     246,     0,     0,   303,   291,   292,   256,   293,   294,   268,
     276,     0,     0,   402,   403,   404,   406,   407,   442,   441,
     445,     0,   477,     0,   480,     0,     0,     0,     0,   338,
       0,     0,     0,     0,     0,     0,   589,   590,     0,     0,
      53,   622,    92,    70,    88,    90,   203,     0,   223,   126,
       0,    93,    91,     0,   550,     0,   469,   215,     0,   121,
       0,    42,     0,   128,   126,     0,    91,    82,   483,     0,
     356,   126,    72,     0,   224,   221,   217,     0,     0,     0,
     462,     0,     0,   222,     0,     0,   575,   559,     0,    75,
     566,     0,   567,   178,   184,   189,     0,   563,     0,   561,
     553,   555,   554,   263,     0,     0,     0,   175,   174,     0,
     253,   193,   245,   190,    64,   137,   139,   136,   138,   146,
     249,     0,     0,     0,   248,   310,   305,     0,   304,     0,
       0,   271,     0,   465,     0,     0,   383,   372,   373,   374,
     375,   376,   377,   378,   379,   381,   382,   380,     0,     0,
       0,   339,     0,   474,   476,   473,   475,     0,     0,     0,
      18,     0,    10,    12,     0,    13,     0,     0,    36,     0,
     591,   593,   592,   595,     0,   131,     0,    69,   623,     0,
      71,   117,   118,     0,   132,   201,   214,     0,   583,   129,
      40,   357,   126,     0,   622,     0,     0,   353,    74,   429,
     430,     0,   459,   436,     0,   573,   164,    53,     0,   558,
     288,   287,     0,   186,   181,     0,   433,   434,   250,     0,
      44,   306,   302,   277,   269,   271,     0,     0,   281,   272,
     337,   395,   384,   467,   466,     0,    53,     0,    19,     9,
       0,     0,   502,   543,   502,   544,   502,     0,     0,   505,
       0,   505,   505,     0,   502,   546,   545,   539,   540,   541,
     542,   547,   502,    15,   484,   485,   501,   487,   524,   491,
     531,   492,   534,   535,   493,   494,   495,    43,     0,    38,
       0,   594,     0,   627,   624,   626,    89,   126,    78,     0,
     134,   468,     0,   125,   358,    83,    87,    86,    84,    85,
      53,     0,   351,   359,   352,   216,     0,     0,     0,   232,
       0,   565,     0,   569,   570,   568,   564,   562,   284,   172,
       0,     0,   307,     0,   273,   279,   280,   476,     0,     0,
      11,     0,    34,     0,     0,     0,   496,   503,     0,     0,
     520,     0,     0,   497,   500,   530,     0,   527,   506,   502,
     502,   517,   533,   536,   502,     0,   427,   549,     0,     0,
     528,   548,    14,     0,   487,   488,   490,    35,     0,    37,
     597,   596,   587,     0,   625,   119,     0,    53,   295,     0,
     350,     0,   362,   460,   461,   451,     0,   162,     0,   271,
     285,   311,   313,   487,    53,   319,   318,   309,   308,     0,
     270,   278,    32,     0,    29,     0,     0,   498,   505,   523,
     505,   499,   505,   509,     0,     0,     0,   518,     0,   519,
       0,   425,   424,     0,   529,   505,     0,    25,     0,     0,
      21,    16,     0,   489,     0,     0,   599,   630,   631,   629,
     628,   133,   135,     0,    44,   296,    41,     0,   363,   364,
     361,   289,   290,   286,     0,   314,    45,    46,    50,    50,
       0,     0,     0,     0,   315,   320,   323,   321,     0,    33,
     504,   515,   526,   516,   510,   511,   512,   513,   514,   508,
     507,   521,   505,   522,     0,   502,   422,   426,   532,    20,
      53,    23,    24,    22,    17,    27,   486,    39,   122,   122,
     588,   610,     0,   586,   603,   301,   297,   299,     0,   111,
     367,   312,    53,    51,    52,    49,    48,     0,     0,   329,
       0,   327,   331,   324,   322,   325,     0,   317,   316,     0,
     525,   537,     0,   423,     0,     0,   120,   123,   120,   120,
     120,   611,     0,     0,    53,     0,     0,     0,    97,   112,
      47,   330,   332,     0,     0,   334,   335,     0,    30,     0,
      26,     0,     0,     0,     0,     0,   601,   602,   600,    53,
     604,     0,   609,   598,   605,   298,   300,   113,     0,   106,
      98,   326,   328,   333,   336,     0,   538,    28,   618,   615,
     617,   620,   613,   614,   616,   619,   612,     0,    53,     0,
     115,     0,   109,     0,   622,   107,     0,   144,     0,   607,
     114,   104,   105,    99,   102,   103,   100,   116,    31,    53,
       0,   101,   110,   606,   608,     0,   108
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    80,   972,    82,    83,   561,   562,   563,   762,   892,
     564,   831,   565,   731,   567,    84,   769,   693,   435,   722,
     856,   857,   915,    85,    86,    87,   263,    88,    89,    90,
      91,    92,   334,    93,    94,    95,    96,   212,   466,   467,
     698,    97,   443,   444,   441,    98,   317,   979,   980,  1020,
    1016,  1013,  1004,  1005,  1001,  1021,   948,  1000,   949,   451,
     452,   581,   460,   938,   463,   216,   217,   442,   690,   778,
     519,    99,   100,   970,   101,   102,   103,   104,   105,   490,
     106,   107,   492,   108,   109,   110,   111,   507,   112,   113,
     378,   114,   115,   258,   116,   117,   259,   388,   118,   119,
     120,   318,   319,   445,   121,   456,   122,   123,   124,   125,
     126,   127,   352,   353,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   371,   372,   373,   530,   628,
     139,   140,   409,   624,   800,   629,   141,   142,   789,   143,
     612,   790,   905,   406,   407,   844,   845,   906,   907,   908,
     408,   526,   620,   797,   527,   528,   621,   791,   792,   927,
     798,   799,   864,   919,   866,   867,   920,   981,   921,   922,
     928,   144,   145,   332,   702,   703,   333,   218,   219,   220,
     850,   221,   704,   390,   146,   297,   548,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   354,
     755,   887,   756,   757,   338,   395,   396,   159,   160,   161,
     162,   163,   765,   164,   165,   166,   167,   342,   343,   344,
     168,   707,   169,   170,   171,   172,   173,   321,   304,   305,
     306,   307,   193,   174,   175,   176,   663,   764,   766,   664,
     665,   736,   737,   747,   748,   814,   879,   666,   667,   668,
     669,   670,   671,   821,   672,   673,   674,   675,   676,   732,
     177,   178,   179,   180,   181,   362,   363,   496,   497,   233,
     234,   360,   398,   399,   235,   205,   182,   461,   183,   836,
     439,   574,   681,   771,   903,   904,   968,   944,   973,  1008,
     974,   900,   901,   993,   989,   990,   996,   992,   310,   577,
     578,   684,   685,   840
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -849
static const yytype_int16 yypact[] =
{
    1275,  1275,   146,  -849,  -849,  -849,  -849,  -849,  1773,   175,
    -849,  -849,   214,   110,   261,   267,   253,   234,   340,  -849,
     337,   304,   365,  -849,  -849,  -849,  -849,   397,   272,    93,
    -849,  -849,   409,  -849,   340,   228,  -849,   439,   449,   456,
     461,   467,   475,  -849,  -849,  -849,  -849,   479,  -849,  -849,
    3191,   481,   485,  -849,  -849,  -849,  -849,  -849,  -849,   488,
     495,  -849,  -849,  -849,  -849,   505,   507,  -849,  -849,  3833,
    3833,  -849,  -849,  -849,  -849,  -849,  -849,  4378,  4378,  -849,
     517,   508,  -849,  -849,  -849,   123,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,    75,  -849,  -849,  -849,  -849,    60,   442,
    -849,  -849,  -849,   287,   300,  -849,  -849,  3833,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,    92,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,   528,   552,   557,  -849,  -849,   392,  -849,  -849,
     562,  -849,   573,  -849,   399,   592,  -849,  -849,  -849,  -849,
    -849,  -849,   309,   184,  -849,  -849,  -849,  -849,  -849,    86,
    -849,   360,  -849,   570,  -849,  -849,  -849,  -849,  -849,  -849,
     611,  -849,  -849,  -849,   604,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,   630,   635,    26,    22,  -849,  1524,
    -849,  -849,  -849,  -849,  3833,  -849,  3833,    54,   624,   624,
     643,   228,    62,  -849,  -849,   574,   624,   571,  3833,  -849,
    -849,  -849,  4043,  3833,  -849,  -849,   142,   563,  -849,  -849,
    -849,  -849,  3833,  3833,  3833,  3833,  3833,  3833,   553,  2981,
    2022,  -849,   416,   258,  -849,   503,  3833,  3833,   667,    84,
    3833,   671,   416,  -849,  -849,  -849,  -849,  -849,  -849,   673,
    3833,    11,  3413,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,   650,   365,  2022,  2022,  -849,  -849,  -849,
    -849,  -849,   675,  3833,  2771,   343,  3833,  3833,    93,  4253,
    4253,  4253,  4253,  4253,  3833,   282,   624,  -849,  3833,    83,
    -849,  -849,  -849,  -849,    93,   635,    26,   651,   416,   676,
     558,   679,  3833,   126,   683,  -849,  -849,  -849,   693,  -849,
     226,   685,   385,   693,  -849,    24,    14,   131,   692,  -849,
      13,  -849,   572,   285,   285,   621,  -849,  -849,  -849,  -849,
     350,    55,   606,   625,   470,  -849,    27,    38,   202,   -37,
     453,   -27,   631,  -849,  -849,    45,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,   633,  -849,   416,    46,   -96,  3833,  -849,
     545,  2022,   548,  -849,   503,   458,   416,   502,    47,  -849,
      92,  -849,  -849,   709,  -849,    52,  -849,  2981,  -849,  -849,
     193,     0,  3833,   815,  -849,  -849,  -849,  -849,   340,  -849,
      62,   279,  -849,   287,  -849,  -849,   515,   416,  -849,  -849,
    -849,  3833,   520,   698,  -849,  -849,  -849,  -849,  -849,   712,
     416,    48,  3833,  -849,  -849,  -849,   309,   309,  -849,  -849,
    -849,    49,   699,   720,  -849,  4043,   313,   313,   624,  -849,
     721,   702,   703,    76,    55,   697,  -849,  -849,   632,    59,
    2022,   504,  -849,   725,  -849,  -849,  -849,   615,  -849,   228,
     385,  -849,   614,  3833,  -849,   722,  -849,  -849,   624,  -849,
     723,   693,   724,  -849,   228,   661,    57,  -849,   736,   506,
    -849,   228,  -849,  3833,  -849,  -849,  -849,  3833,  3833,  3833,
    -849,  3833,  3833,  -849,  3833,  3833,  -849,  -849,    74,    64,
    -849,  3833,  -849,   746,   751,   752,    80,  -849,   598,  -849,
    -849,  -849,  -849,  -849,    84,   530,  3833,  -849,  -849,   523,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  3833,  3833,   761,  -849,  -849,   547,   763,  -849,   748,
     766,   291,   767,  -849,   770,   706,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,   778,   783,
     784,  -849,   768,  -849,   768,  -849,  -849,   789,   711,   726,
    -849,   564,  -849,  -849,   790,  -849,  2648,   577,  -849,    62,
    -849,  -849,  -849,   708,   710,   300,   785,  -849,  -849,   385,
    -849,   806,   862,    69,   739,  -849,  -849,   811,  -849,  -849,
     814,   862,   228,    55,   504,    55,  3623,   782,  -849,   416,
     416,    50,   -10,    71,    51,  -849,    96,  2520,  3833,  -849,
    -849,  -849,   817,  -849,   566,  3833,  -849,  -849,  -849,   781,
     727,  -849,  -849,  -849,   820,   291,   623,   626,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,   802,  2022,   826,  -849,  -849,
      76,   722,    35,  -849,    58,  -849,    68,   829,   829,   830,
     121,   830,   830,    16,   155,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,   829,  -849,  -849,   795,  -849,   570,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,    55,   707,
     722,  -849,   802,   594,   785,  -849,  -849,   228,  -849,   798,
     765,  -849,   722,  -849,   862,  -849,  -849,  -849,  -849,   416,
    2022,   355,  -849,  -849,  -849,  -849,  3833,   837,   842,  -849,
     111,  -849,  3833,  -849,  -849,  -849,  -849,  -849,   775,  -849,
      55,   810,   367,  3833,  -849,  -849,  -849,  -849,    28,   722,
    -849,   582,  -849,   828,   829,   662,  -849,  -849,   829,   663,
    -849,   829,   664,  -849,  -849,  -849,   836,  -849,  -849,   179,
      30,  -849,  -849,   848,   829,   741,  -849,  -849,   829,   672,
    -849,  -849,    70,   753,   570,  -849,  -849,  -849,   824,  -849,
     867,  -849,   704,   356,  -849,   862,  3833,  2022,   655,   589,
     300,   722,   539,   416,  -849,  -849,  3833,  -849,   393,   291,
    -849,   871,  -849,   570,  2271,  -849,  -849,  -849,  -849,   447,
    -849,   416,  -849,   602,  -849,   722,   874,  -849,   830,  -849,
     830,  -849,   830,   486,   875,   829,   691,  -849,   829,  -849,
     861,  -849,   773,   266,  -849,   830,   792,   880,   333,   624,
    -849,  -849,   860,  -849,   793,   429,   819,  -849,  -849,    92,
    -849,   401,   300,   698,   727,  -849,  -849,   620,  -849,  -849,
    -849,  -849,  -849,  -849,    55,  -849,   889,  -849,  1101,   102,
    1032,   718,   682,   684,   696,  -849,  -849,  -849,   796,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,   830,  -849,   622,   829,  -849,  -849,  -849,  -849,
    2022,  -849,  -849,    92,  -849,   918,  -849,  -849,   -41,   -28,
     429,  -849,   750,  -849,   756,  -849,   919,  -849,   894,   714,
    -849,  -849,  2271,  -849,  -849,  -849,  -849,   216,   235,  -849,
     686,  -849,  -849,  -849,  -849,  -849,   144,  -849,  -849,   624,
    -849,  -849,   905,  -849,    31,   722,   898,  -849,   898,   898,
     898,  -849,   460,   927,   777,   698,   929,  4253,   717,  -849,
    -849,  -849,  -849,  1032,   769,  -849,  -849,   732,   930,   934,
    -849,   641,   923,   924,   923,   928,  -849,  -849,  -849,  2022,
    -849,   791,  -849,  -849,  -849,  -849,  -849,   132,   108,   886,
    -849,  -849,  -849,  -849,  -849,   722,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,    32,  1275,  4253,
    -849,   -93,    92,   117,   504,  -849,   648,  -849,   950,  -849,
     184,  -849,  -849,  -849,  -849,  -849,   108,  -849,  -849,  1026,
     -93,  -849,    92,  -849,  -849,   738,  -849
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -849,  -849,   656,  -849,  -849,  -849,   325,  -849,  -849,  -849,
     205,  -849,  -849,  -646,  -849,  -849,  -849,  -849,  -248,   125,
    -849,    61,   112,   -11,  -849,    21,  -849,  -849,  -849,  -849,
    -849,   533,  -849,   -99,   -49,  -849,  -849,   162,  -849,   377,
    -849,  -849,   525,   402,  -309,   413,   561,  -849,  -849,  -849,
    -849,   -33,  -849,  -849,  -849,  -849,  -849,  -849,  -849,   405,
    -849,  -849,  -318,    90,  -849,  -849,  -176,  -849,  -849,  -849,
     217,  -849,  -849,  -849,   983,   728,   729,    -1,  -728,  -849,
    -849,   -86,   899,  -849,   -81,   901,  -849,   616,   -75,  -849,
    -849,   -74,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
       6,  -849,   543,  -849,  -237,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -739,  -849,   617,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,   494,   497,  -849,  -849,  -577,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,  -202,  -849,  -849,  -849,  -849,  -849,    63,  -849,
      56,  -849,  -849,  -849,  -849,  -849,  -849,  -849,   149,  -849,
    -849,  -849,  -849,   206,  -849,  -849,  -849,  -849,    53,  -849,
    -849,   628,  -849,  -849,  -849,  -849,   674,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,   813,   580,  -849,   762,  -849,
     581,   299,   378,  -848,  -849,   188,  -849,  -849,  -849,  -597,
    -849,  -849,  -849,  -849,  -223,  -207,   743,     8,   -65,  -849,
    -849,  -849,   855,   734,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,   417,  -849,  -849,  -849,  -849,  -849,  -849,  -189,  -849,
     232,  -389,  -849,  -316,  -266,  -315,  -849,  -849,  -656,  -849,
    -849,  -574,  -261,  -461,  -849,  -849,  -849,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -300,
    -849,  -849,  -849,  -849,  -849,  -849,   669,  -849,   424,   800,
    -169,   677,  -317,  -849,  -849,  -849,  -210,  -849,  -849,  -849,
    -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,  -849,
    -609,  -849,   136,  -849,  -849,    73,  -849,  -849,  -849,  -573,
    -849,  -849,   357,  -849
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -571
static const yytype_int16 yytable[] =
{
     186,   231,   370,   340,   243,   244,   462,   252,   267,   187,
     308,   207,   254,   365,   367,   468,   469,   313,   256,   257,
     457,   696,   185,   424,   203,   311,   293,   324,   448,   374,
     292,   474,   802,   733,   770,   960,  1007,   454,   733,   479,
     214,   486,   475,   556,   455,   481,   779,   382,   724,   483,
     293,   502,   531,   533,   705,   709,   754,   504,   232,   706,
     863,   733,   593,   506,   359,   447,   859,   485,   346,  -565,
     740,   733,   743,   405,   579,  -459,   936,   242,   242,   422,
     760,   210,   251,   803,  1011,   607,   -95,   249,   761,   939,
       2,   -76,     3,   826,   -95,   430,   250,   420,   190,   977,
     454,  -162,   446,     4,     5,     6,     7,   422,   833,   -76,
     382,   827,   268,   211,   269,   232,   571,   557,   468,   568,
     210,   918,   -76,   335,   260,  1012,   828,     4,     5,     6,
       7,   300,   431,   566,   913,   847,   937,   855,   572,   432,
     706,   449,   261,   584,   266,   202,   558,   382,   322,   937,
     249,  1010,   211,   387,   505,   586,    22,   594,   733,   250,
     914,   749,   -76,   347,   348,   349,   303,   -76,   605,   753,
     188,   -76,   294,   559,   351,   817,   819,   829,  -253,   559,
     822,   262,   733,   573,   859,   381,   440,   249,   296,    37,
     751,   752,   560,   954,   523,   359,   786,   187,   560,   249,
     192,   525,   298,   449,   298,   532,   476,   477,   250,   478,
     295,   450,   853,   589,   918,   284,   315,   194,   955,   818,
     320,   242,   323,   295,   734,   284,   886,  -565,   735,   357,
     232,   336,   337,   339,   341,   345,   355,   356,   383,   550,
     446,   195,   284,   608,   366,   368,   556,   738,   375,   -76,
     295,   739,  -436,   391,   750,   379,   506,   741,   242,  -162,
     232,   742,  -143,  -143,   196,   284,   266,   370,   266,   587,
     197,   266,   266,   582,   284,   491,   284,   468,   469,   284,
     493,   397,   397,   509,   410,   411,   494,   495,   591,   961,
     284,   717,   418,   772,   374,   597,   421,   284,   284,   284,
     284,   284,   284,   284,   284,   423,   201,   301,    79,  -120,
     397,   933,   489,   -76,   616,   617,   459,    67,   346,    71,
      72,    73,    74,    75,    76,   270,  1014,    14,   271,   424,
     -76,   -76,   -76,   -76,   -76,   -76,    79,   536,   284,  1006,
     566,    67,  1015,   202,   758,   515,   403,   871,   759,   872,
     488,   873,     4,     5,     6,     7,   999,   956,   781,   302,
     357,   204,   468,   767,   888,   957,   397,   404,   815,   446,
     198,    23,   816,   516,   598,     4,     5,     6,     7,   303,
     837,   199,   282,   283,   206,   366,   744,   745,   190,  1009,
     232,     2,   606,     3,   513,   436,   437,   438,   295,   517,
     208,    32,   518,   209,   468,   793,   477,    36,   478,   397,
    1024,   514,   213,   347,   348,   349,   694,   603,    41,   885,
     397,   930,   515,   326,   351,   230,   358,   891,   327,   200,
     951,  1017,   924,   320,   282,   283,   401,   328,   329,   575,
     412,   614,   222,  -396,  -396,  -396,  -396,  -396,  -396,   952,
     516,   925,   223,     2,   215,     3,   425,   480,   477,   224,
     478,   320,   500,   477,   225,   478,     4,     5,     6,     7,
     226,    43,    44,   807,   472,   473,   517,   809,   227,   518,
     811,   232,   228,   860,   236,   599,   600,   601,   237,   602,
     242,   238,   604,   397,   626,   627,   861,   824,   239,   232,
     874,   875,   876,   877,   878,   869,   501,   477,   240,   478,
     241,   775,   782,    19,   242,   248,   719,   247,    68,   520,
     521,   712,   522,   264,   524,   521,   713,   522,   265,   397,
     397,   272,   714,   715,   613,   477,   839,   478,   468,   793,
     266,    26,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   841,   881,   273,   229,   883,   711,     2,
     274,     3,   279,   280,   281,   276,    67,    33,   639,   640,
      69,    70,     4,     5,     6,     7,   277,     9,   413,   414,
     415,   677,   678,    10,   795,   796,   804,   805,    11,    67,
     275,   893,    12,   846,   805,   278,   710,  -258,    14,    15,
     848,   849,   851,   852,   699,   346,   868,   805,   898,   899,
     285,   325,   286,   787,   288,   232,   397,   289,   962,    19,
     963,   964,   965,   232,   910,   805,   931,   932,    53,    54,
      55,    56,    57,    58,   290,   728,   326,   966,   967,   291,
     895,   327,    23,    24,    25,   987,   805,    26,   303,    27,
     328,   329,  1018,   805,   312,   330,    81,   184,   245,   246,
     416,   417,   862,   553,   555,   314,    30,   309,   284,    31,
     361,   369,    32,    33,   331,   376,   377,   389,    36,   394,
      67,   427,   426,   429,   428,    38,   433,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   434,   453,   458,   780,
     347,   348,   349,   482,   464,   484,   350,   470,   471,   487,
      48,   351,    49,   503,   783,    50,   358,   529,    51,    52,
     232,   381,   404,   534,   192,   551,   552,   554,   569,   570,
     579,   801,   576,   325,    53,    54,    55,    56,    57,    58,
     958,  1002,    37,   440,   592,   858,   454,   588,   590,   595,
     596,  -569,    59,    60,    61,    62,  -570,  -568,   326,    63,
      64,    65,   609,   327,   615,   618,   619,   622,  -353,   623,
     625,   630,   328,   329,   631,    66,   842,   330,   449,  1022,
       1,   635,   632,     2,   366,     3,    67,   633,   634,    68,
      69,    70,   636,   641,   242,   637,     4,     5,     6,     7,
       8,     9,   232,    77,    78,   680,    79,    10,   682,   683,
     638,   687,    11,   689,   440,   691,    12,   692,  -436,   720,
      13,   718,    14,    15,    16,   723,   727,   721,   725,   729,
     325,   726,   733,   746,    17,   763,   776,   768,   773,   510,
     777,   784,    18,    19,    20,    10,   785,   788,   794,   806,
      11,   820,   808,   810,   812,   326,    21,   813,    22,   823,
     327,    15,   825,   858,   832,   834,    23,    24,    25,   328,
     329,    26,   805,    27,   330,   843,   854,   835,   870,   880,
     700,   882,   884,   890,   896,    28,   889,    29,   902,   934,
      30,  -421,   897,    31,   912,   923,    32,    33,   924,    34,
     925,    35,    36,   929,    37,    24,    25,   926,   701,    38,
     325,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     232,   935,   942,   943,   945,   946,   959,   953,    30,   459,
     969,    31,   403,   985,    48,   326,    49,   947,   986,    50,
     327,   978,    51,    52,   984,  -353,   983,   988,   991,   328,
     329,   971,   995,  1003,   330,    45,    46,  1019,    53,    54,
      55,    56,    57,    58,  1026,   730,   998,   830,   997,   909,
     695,   916,    48,   950,    49,   583,    59,    60,    61,    62,
     580,   686,   679,    63,    64,    65,   535,  1025,   688,   940,
     838,   191,   253,   392,   255,   393,   585,   508,   610,    66,
     511,   611,   976,   911,   465,   865,   982,   549,   975,   299,
      67,   512,   380,    68,    69,    70,   894,   402,   287,   419,
     708,    71,    72,    73,    74,    75,    76,    77,    78,     1,
      79,   716,     2,   499,     3,   364,   941,   994,     2,   498,
       3,   774,     0,     0,     0,     4,     5,     6,     7,     8,
       9,     4,     5,     6,     7,     0,    10,     0,     0,     0,
       0,    11,     0,     0,     0,    12,     0,     0,     0,    13,
       0,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,   861,     0,    17,     0,     0,     0,     0,     0,     0,
       0,    18,    19,    20,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,     0,    21,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
      26,     0,    27,     0,   -75,     0,    26,     0,     0,     0,
       0,     0,     0,   913,    28,     0,    29,   -75,     0,    30,
       0,     0,    31,     0,     0,    32,    33,     0,    34,     0,
      35,    36,    33,    37,     0,     0,     0,     0,    38,   914,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
       0,     0,     0,     0,     0,     0,     0,   -75,     0,     0,
       0,     0,   -75,    48,     0,    49,   -75,     0,    50,     0,
    1023,    51,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,    55,
      56,    57,    58,    53,    54,    55,    56,    57,    58,     0,
       0,     0,     0,     0,     0,    59,    60,    61,    62,     0,
       0,     0,    63,    64,    65,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   917,    66,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    67,
       0,     0,    68,    69,    70,    67,     0,     0,     0,     0,
      71,    72,    73,    74,    75,    76,    77,    78,     1,    79,
       0,     2,     0,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
      11,     0,     0,     0,    12,     0,     0,     0,    13,     0,
      14,    15,    16,     0,     0,     0,     0,     0,   -75,     0,
       0,     0,    17,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    20,     0,     0,   -75,   -75,   -75,   -75,   -75,
     -75,     0,     0,     0,    21,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,    29,     0,     0,    30,     0,
       0,    31,     0,     0,    32,    33,     0,    34,     0,    35,
      36,     0,    37,     0,     0,     0,     0,    38,     0,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,    49,     0,     0,    50,     0,     0,
      51,    52,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,    62,     0,     0,
       0,    63,    64,    65,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    66,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    67,     0,
       0,    68,    69,    70,     0,     0,     0,     0,     0,    71,
      72,    73,    74,    75,    76,    77,    78,   189,    79,     0,
       2,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    11,
       0,     0,     0,    12,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   294,     0,     0,    30,     0,     0,
      31,     0,     0,    32,    33,     0,    34,     0,     0,    36,
       0,    37,     0,     0,     0,     0,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,    49,     0,     0,    50,     0,     0,    51,
      52,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    53,    54,    55,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,    62,     0,     0,     0,
      63,    64,    65,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    67,     0,     0,
      68,    69,    70,     0,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,    77,    78,   189,    79,     0,     2,
       0,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,   190,     9,     0,     0,
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
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,    68,
      69,    70,     0,     0,     0,     0,     0,    71,    72,    73,
      74,    75,    76,    77,    78,   189,    79,     0,     2,     0,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,    11,     0,     0,
       0,    12,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    30,     0,     0,    31,     0,
       0,    32,    33,     0,    34,     0,     0,    36,     0,     0,
       0,     0,     0,     0,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,    49,     0,     0,    50,     0,     0,    51,    52,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    53,    54,    55,    56,    57,    58,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    66,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,     0,     0,    68,    69,
      70,     0,     0,     0,     0,     0,    71,    72,    73,    74,
      75,    76,    77,    78,   229,    79,     0,     2,     0,     3,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,   190,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,    34,     0,     0,    36,     0,     0,     0,
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
       0,     0,     0,     0,    67,     0,     0,    68,    69,    70,
       0,     0,     0,     0,     0,    71,    72,    73,    74,    75,
      76,    77,    78,   229,    79,     0,     2,     0,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,   190,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,    11,     0,     0,     0,    12,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,     0,     0,    26,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,    31,     0,     0,    32,
      33,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,    49,
       0,     0,    50,     0,     0,    51,    52,     0,   642,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    53,    54,    55,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   643,     0,     0,    59,
      60,    61,    62,     0,     0,     0,    63,    64,    65,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   644,    66,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    67,   645,     0,    68,    69,    70,     0,
       0,     0,     0,     0,    71,    72,    73,    74,    75,    76,
      77,    78,     0,    79,   229,   400,     0,     2,     0,     3,
       0,   646,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     0,     9,     0,     0,   210,     0,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,   647,   648,    19,     0,   649,
     650,     0,   651,   652,     0,   653,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   654,   655,   656,
     657,   658,   659,   660,    30,   661,   662,    31,     0,     0,
      32,    33,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
      49,     0,     0,    50,     0,     0,    51,    52,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,    56,    57,    58,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,    62,     0,     0,     0,    63,    64,    65,
       0,     0,     0,     0,   229,     0,     0,     2,     0,     3,
       0,     0,     0,    66,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,    67,     9,     0,    68,    69,    70,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,    77,    78,     0,    79,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,     0,     0,     0,    36,     0,    37,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
      49,     0,     0,    50,     0,     0,    51,    52,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,    56,    57,    58,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,    62,     0,     0,     0,    63,    64,    65,
       0,     0,     0,     0,   229,     0,     0,     2,     0,     3,
       0,     0,     0,    66,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,    67,     9,     0,    68,    69,    70,
       0,    10,     0,     0,     0,     0,    11,     0,     0,     0,
      12,    77,    78,     0,    79,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    30,     0,     0,    31,     0,     0,
      32,    33,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
      49,     0,     0,    50,     0,     0,    51,    52,   230,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,    56,    57,    58,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,    62,     0,     0,     0,    63,    64,    65,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    66,     0,     0,   229,     0,     0,     2,
       0,     3,     0,     0,    67,     0,     0,    68,    69,    70,
       0,     0,     4,     5,     6,     7,     0,     9,     0,     0,
     384,    77,    78,   385,    79,     0,     0,     0,    11,     0,
       0,     0,    12,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   386,     0,     0,    31,
       0,     0,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,   229,     0,     0,     2,
       0,     3,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,    67,     9,     0,    68,
      69,    70,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,    77,    78,     0,    79,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   697,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,   229,     0,     0,     2,
       0,     3,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,    67,     9,     0,    68,
      69,    70,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,    77,    78,     0,    79,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,   229,     0,     0,     2,
       0,     3,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,    67,     9,     0,    68,
      69,    70,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,    77,    78,     0,    79,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,    32,    33,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    38,     0,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,    49,     0,     0,    50,     0,     0,    51,    52,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,   229,     0,     0,     2,
       0,     3,     0,     0,     0,    66,     0,     0,     0,     0,
       0,     0,     4,     5,     6,     7,    67,     9,     0,    68,
      69,    70,     0,    10,     0,     0,     0,     0,    11,     0,
       0,     0,    12,    77,    78,     0,   316,     0,     0,    15,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,     0,     0,    26,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,    31,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,     2,    38,     3,    39,    40,     0,
      42,     0,     0,    45,    46,    47,     0,     4,     5,     6,
       7,     0,     9,     0,     0,     0,     0,     0,    10,     0,
      48,     0,    49,    11,     0,    50,     0,    12,    51,    52,
       0,     0,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,    59,    60,    61,    62,     0,     0,     0,    63,
      64,    65,     0,     0,     0,     0,     0,     0,    24,    25,
       0,     0,    26,     0,    27,    66,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    67,     0,     0,     0,
       0,    30,     0,     0,    31,     0,     0,     0,    33,     0,
       0,     0,     0,    77,    78,     0,    79,     0,     0,     0,
      38,     0,    39,    40,     0,    42,     0,     0,    45,    46,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,    49,     0,     0,
      50,     0,     0,    51,    52,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
      62,     0,     0,     0,    63,    64,    65,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    67,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    79
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-849)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    50,   239,   226,    69,    70,   324,    93,   107,     1,
     199,    22,    93,   236,   237,   331,   331,   206,    93,    93,
     320,   594,     1,   289,    18,   201,     4,   216,     4,   239,
       4,     4,     4,     3,   680,     4,     4,    24,     3,    76,
      34,   358,     4,   432,    31,    72,   692,    36,   625,     4,
       4,     4,     4,     4,     4,     4,   653,     5,    50,    69,
     799,     3,     5,    63,   233,   313,   794,   163,    52,     5,
     644,     3,   646,   275,     5,     4,   117,    69,    70,    24,
     654,    27,    93,   729,   177,     5,    72,    76,   662,   117,
       6,     5,     8,    23,    80,   302,    85,   286,    23,   947,
      24,     5,   312,    19,    20,    21,    22,    24,   764,    23,
      36,    41,    20,    59,    22,   107,    57,    41,   434,   434,
      27,   860,    36,   222,    64,   218,    56,    19,    20,    21,
      22,   196,     6,   433,    32,   781,   177,   793,    79,    13,
      69,    72,    82,   452,   240,     3,    70,    36,   213,   177,
      76,   999,    59,   252,   377,   455,    81,   466,     3,    85,
      58,    40,    76,   147,   148,   149,    24,    81,   485,   153,
      24,    85,   110,   103,   158,   749,   750,   107,     3,   103,
     754,   121,     3,   124,   912,   250,   129,    76,   189,   127,
     651,   652,   122,    49,   401,   364,    85,   189,   122,    76,
      25,   403,   194,    72,   196,   412,     4,     5,    85,     7,
     189,    80,   789,   461,   953,   252,   208,     3,    74,   189,
     212,   213,   216,   202,   189,   252,   823,   163,   193,   230,
     222,   223,   224,   225,   226,   227,   228,   229,   227,   428,
     450,   131,   252,   163,   236,   237,   635,   189,   240,   163,
     229,   193,   252,   264,   133,   249,    63,   189,   250,   163,
     252,   193,   240,   241,     3,   252,   240,   504,   240,   458,
       3,   240,   240,   449,   252,   361,   252,   593,   593,   252,
     361,   273,   274,   382,   276,   277,   361,   361,   464,   935,
     252,   608,   284,   682,   504,   471,   288,   252,   252,   252,
     252,   252,   252,   252,   252,   222,    72,   253,   253,    24,
     302,   885,   361,   227,   521,   522,    31,   233,    52,   244,
     245,   246,   247,   248,   249,   233,   209,    45,   236,   595,
     244,   245,   246,   247,   248,   249,   253,    24,   252,   985,
     640,   233,   225,     3,   189,    66,     3,   808,   193,   810,
     361,   812,    19,    20,    21,    22,   224,   213,     3,   197,
     361,    24,   678,   678,   825,   221,   358,    24,   189,   579,
     117,    89,   193,    94,   473,    19,    20,    21,    22,    24,
      24,   128,   250,   251,    80,   377,   647,   648,    23,   998,
     382,     6,   491,     8,   388,   169,   170,   171,   377,   120,
       3,   119,   123,   131,   720,   720,     5,   125,     7,   401,
    1019,   390,     3,   147,   148,   149,   592,   482,   136,   153,
     412,   882,    66,    73,   158,   167,   168,    94,    78,   176,
     214,  1004,   216,   425,   250,   251,   274,    87,    88,   440,
     278,   506,     3,   250,   251,   252,   253,   254,   255,   214,
      94,   216,     3,     6,   226,     8,   294,     4,     5,     3,
       7,   453,     4,     5,     3,     7,    19,    20,    21,    22,
       3,   138,   139,   734,     4,     5,   120,   738,     3,   123,
     741,   473,     3,    36,     3,   477,   478,   479,     3,   481,
     482,     3,   484,   485,   203,   204,    49,   758,     3,   491,
      14,    15,    16,    17,    18,   805,     4,     5,     3,     7,
       3,   687,   701,    66,   506,     7,   615,     0,   236,     4,
       5,   607,     7,    81,     4,     5,   607,     7,   241,   521,
     522,     3,   607,   607,     4,     5,   773,     7,   854,   854,
     240,    94,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   776,   815,     3,     3,   818,   607,     6,
       3,     8,   253,   254,   255,     3,   233,   120,     4,     5,
     237,   238,    19,    20,    21,    22,     3,    24,   279,   280,
     281,     4,     5,    30,   217,   218,     4,     5,    35,   233,
     198,   828,    39,     4,     5,     3,   607,   198,    45,    46,
      61,    62,   209,   210,   596,    52,     4,     5,   179,   180,
     250,    48,    42,   712,     3,   607,   608,    13,   936,    66,
     938,   939,   940,   615,     4,     5,     4,     5,   181,   182,
     183,   184,   185,   186,     4,   636,    73,   177,   178,     4,
     829,    78,    89,    90,    91,     4,     5,    94,    24,    96,
      87,    88,     4,     5,    80,    92,     0,     1,    77,    78,
     282,   283,   215,   431,   432,    94,   113,    24,   252,   116,
     167,     4,   119,   120,   111,     4,     3,    27,   125,     4,
     233,     5,    31,     4,   126,   132,     3,   134,   135,   136,
     137,   138,   139,   140,   141,   142,     3,     5,   126,   700,
     147,   148,   149,    72,    83,    72,   153,   101,    83,   164,
     157,   158,   159,     4,   706,   162,   168,     5,   165,   166,
     712,   786,    24,     3,    25,     4,    24,    24,    31,    97,
       5,   723,   228,    48,   181,   182,   183,   184,   185,   186,
     929,   978,   127,   129,    83,   794,    24,    24,    24,    13,
     244,     5,   199,   200,   201,   202,     5,     5,    73,   206,
     207,   208,   164,    78,   241,     4,   219,     4,    83,    21,
       4,     4,    87,    88,     4,   222,   777,    92,    72,  1016,
       3,    13,     4,     6,   776,     8,   233,     4,     4,   236,
     237,   238,     3,     3,   786,    84,    19,    20,    21,    22,
      23,    24,   794,   250,   251,    97,   253,    30,    98,    24,
      84,     5,    35,    74,   129,     4,    39,     3,   252,    38,
      43,     4,    45,    46,    47,     5,    24,   100,   205,     3,
      48,   205,     3,     3,    57,    40,    38,   130,   244,    24,
      75,     4,    65,    66,    67,    30,     4,    72,    38,    21,
      35,     3,   190,   190,   190,    73,    79,    21,    81,   118,
      78,    46,   190,   912,   111,    41,    89,    90,    91,    87,
      88,    94,     5,    96,    92,   220,     5,   173,     4,     4,
      98,   190,    21,     3,    24,   108,    94,   110,    69,   890,
     113,   118,    99,   116,     5,   177,   119,   120,   216,   122,
     216,   124,   125,   107,   127,    90,    91,   211,   126,   132,
      48,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     912,     3,   172,   167,     5,    31,    21,   241,   113,    31,
       3,   116,     3,     3,   157,    73,   159,   223,     4,   162,
      78,   224,   165,   166,   212,    83,   177,    24,    24,    87,
      88,   174,    24,    67,    92,   140,   141,     7,   181,   182,
     183,   184,   185,   186,   226,   640,   175,   762,   969,   844,
     593,   859,   157,   912,   159,   450,   199,   200,   201,   202,
     447,   579,   569,   206,   207,   208,   425,  1020,   583,   899,
     773,     8,    93,   265,    93,   266,   453,   381,   504,   222,
     383,   504,   946,   854,   330,   799,   953,   427,   945,   196,
     233,   383,   250,   236,   237,   238,   828,   274,   163,   285,
     603,   244,   245,   246,   247,   248,   249,   250,   251,     3,
     253,   607,     6,   364,     8,   235,   900,   964,     6,   362,
       8,   684,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    19,    20,    21,    22,    -1,    30,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    43,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,
      94,    -1,    96,    -1,    23,    -1,    94,    -1,    -1,    -1,
      -1,    -1,    -1,    32,   108,    -1,   110,    36,    -1,   113,
      -1,    -1,   116,    -1,    -1,   119,   120,    -1,   122,    -1,
     124,   125,   120,   127,    -1,    -1,    -1,    -1,   132,    58,
     134,   135,   136,   137,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,    -1,    -1,
      -1,    -1,    81,   157,    -1,   159,    85,    -1,   162,    -1,
     164,   165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,
     184,   185,   186,   181,   182,   183,   184,   185,   186,    -1,
      -1,    -1,    -1,    -1,    -1,   199,   200,   201,   202,    -1,
      -1,    -1,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   215,   222,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,
      -1,    -1,   236,   237,   238,   233,    -1,    -1,    -1,    -1,
     244,   245,   246,   247,   248,   249,   250,   251,     3,   253,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    43,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,   227,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    -1,    -1,   244,   245,   246,   247,   248,
     249,    -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,
      -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,   110,    -1,    -1,   113,    -1,
      -1,   116,    -1,    -1,   119,   120,    -1,   122,    -1,   124,
     125,    -1,   127,    -1,    -1,    -1,    -1,   132,    -1,   134,
     135,   136,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,   159,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,
     185,   186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,   200,   201,   202,    -1,    -1,
      -1,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   222,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,
      -1,   236,   237,   238,    -1,    -1,    -1,    -1,    -1,   244,
     245,   246,   247,   248,   249,   250,   251,     3,   253,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,
      96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   110,    -1,    -1,   113,    -1,    -1,
     116,    -1,    -1,   119,   120,    -1,   122,    -1,    -1,   125,
      -1,   127,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,
     186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   222,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,    -1,
     236,   237,   238,    -1,    -1,    -1,    -1,    -1,   244,   245,
     246,   247,   248,   249,   250,   251,     3,   253,    -1,     6,
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
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,    -1,   236,
     237,   238,    -1,    -1,    -1,    -1,    -1,   244,   245,   246,
     247,   248,   249,   250,   251,     3,   253,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,
      -1,   119,   120,    -1,   122,    -1,    -1,   125,    -1,    -1,
      -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,   182,   183,   184,   185,   186,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   233,    -1,    -1,   236,   237,
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
      -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,   122,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     159,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,   184,   185,   186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,   200,   201,   202,    -1,    -1,    -1,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   233,    -1,    -1,   236,   237,   238,
      -1,    -1,    -1,    -1,    -1,   244,   245,   246,   247,   248,
     249,   250,   251,     3,   253,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,   119,
     120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,   159,
      -1,    -1,   162,    -1,    -1,   165,   166,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,   199,
     200,   201,   202,    -1,    -1,    -1,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,   222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   233,   106,    -1,   236,   237,   238,    -1,
      -1,    -1,    -1,    -1,   244,   245,   246,   247,   248,   249,
     250,   251,    -1,   253,     3,     4,    -1,     6,    -1,     8,
      -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    27,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,   187,   188,    66,    -1,   191,
     192,    -1,   194,   195,    -1,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   229,   230,   231,
     232,   233,   234,   235,   113,   237,   238,   116,    -1,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     159,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,   184,   185,   186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,   200,   201,   202,    -1,    -1,    -1,   206,   207,   208,
      -1,    -1,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,   233,    24,    -1,   236,   237,   238,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,   250,   251,    -1,   253,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   110,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,    -1,   127,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     159,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,   184,   185,   186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,   200,   201,   202,    -1,    -1,    -1,   206,   207,   208,
      -1,    -1,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    21,    22,   233,    24,    -1,   236,   237,   238,
      -1,    30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      39,   250,   251,    -1,   253,    -1,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    96,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,   116,    -1,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
     159,    -1,    -1,   162,    -1,    -1,   165,   166,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,   184,   185,   186,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,   200,   201,   202,    -1,    -1,    -1,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   222,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,   233,    -1,    -1,   236,   237,   238,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      27,   250,   251,    30,   253,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   233,    24,    -1,   236,
     237,   238,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,   250,   251,    -1,   253,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   233,    24,    -1,   236,
     237,   238,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,   250,   251,    -1,   253,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   233,    24,    -1,   236,
     237,   238,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,   250,   251,    -1,   253,    -1,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,   159,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,   233,    24,    -1,   236,
     237,   238,    -1,    30,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    39,   250,   251,    -1,   253,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    -1,    -1,    94,    -1,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,   116,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,    -1,    -1,     6,   132,     8,   134,   135,    -1,
     137,    -1,    -1,   140,   141,   142,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
     157,    -1,   159,    35,    -1,   162,    -1,    39,   165,   166,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,   186,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   199,   200,   201,   202,    -1,    -1,    -1,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      -1,    -1,    94,    -1,    96,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   233,    -1,    -1,    -1,
      -1,   113,    -1,    -1,   116,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,   250,   251,    -1,   253,    -1,    -1,    -1,
     132,    -1,   134,   135,    -1,   137,    -1,    -1,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,   159,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,
     182,   183,   184,   185,   186,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,   200,   201,
     202,    -1,    -1,    -1,   206,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   233,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   253
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
     162,   165,   166,   181,   182,   183,   184,   185,   186,   199,
     200,   201,   202,   206,   207,   208,   222,   233,   236,   237,
     238,   244,   245,   246,   247,   248,   249,   250,   251,   253,
     260,   261,   262,   263,   274,   282,   283,   284,   286,   287,
     288,   289,   290,   292,   293,   294,   295,   300,   304,   330,
     331,   333,   334,   335,   336,   337,   339,   340,   342,   343,
     344,   345,   347,   348,   350,   351,   353,   354,   357,   358,
     359,   363,   365,   366,   367,   368,   369,   370,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   389,
     390,   395,   396,   398,   430,   431,   443,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   466,
     467,   468,   469,   470,   472,   473,   474,   475,   479,   481,
     482,   483,   484,   485,   492,   493,   494,   519,   520,   521,
     522,   523,   535,   537,   261,   284,   336,   466,    24,     3,
      23,   333,    25,   491,     3,   131,     3,     3,   117,   128,
     176,    72,     3,   359,    24,   534,    80,   282,     3,   131,
      27,    59,   296,     3,   359,   226,   324,   325,   436,   437,
     438,   440,     3,     3,     3,     3,     3,     3,     3,     3,
     167,   293,   466,   528,   529,   533,     3,     3,     3,     3,
       3,     3,   466,   467,   467,   449,   449,     0,     7,    76,
      85,   282,   340,   341,   343,   344,   347,   350,   352,   355,
      64,    82,   121,   285,    81,   241,   240,   292,    20,    22,
     233,   236,     3,     3,     3,   198,     3,     3,     3,   253,
     254,   255,   250,   251,   252,   250,    42,   471,     3,    13,
       4,     4,     4,     4,   110,   284,   336,   444,   466,   444,
     467,   253,   296,    24,   487,   488,   489,   490,   487,    24,
     557,   325,    80,   487,    94,   466,   253,   305,   360,   361,
     466,   486,   467,   359,   487,    48,    73,    78,    87,    88,
      92,   111,   432,   435,   291,   292,   466,   466,   463,   466,
     463,   466,   476,   477,   478,   466,    52,   147,   148,   149,
     153,   158,   371,   372,   458,   466,   466,   336,   168,   529,
     530,   167,   524,   525,   528,   463,   466,   463,   466,     4,
     363,   384,   385,   386,   535,   466,     4,     3,   349,   359,
     447,   467,    36,   227,    27,    30,   113,   292,   356,    27,
     442,   282,   334,   335,     4,   464,   465,   466,   531,   532,
       4,   296,   465,     3,    24,   401,   402,   403,   409,   391,
     466,   466,   296,   450,   450,   450,   451,   451,   466,   472,
     487,   466,    24,   222,   493,   296,    31,     5,   126,     4,
     464,     6,    13,     3,     3,   277,   169,   170,   171,   539,
     129,   303,   326,   301,   302,   362,   535,   277,     4,    72,
      80,   318,   319,     5,    24,    31,   364,   518,   126,    31,
     321,   536,   321,   323,    83,   435,   297,   298,   492,   494,
     101,    83,     4,     5,     4,     4,     4,     5,     7,    76,
       4,    72,    72,     4,    72,   163,   531,   164,   282,   293,
     338,   340,   341,   343,   347,   350,   526,   527,   530,   525,
       4,     4,     4,     4,     5,   463,    63,   346,   346,   292,
      24,   376,   430,   359,   284,    66,    94,   120,   123,   329,
       4,     5,     7,   464,     4,   401,   410,   413,   414,     5,
     387,     4,   464,     4,     3,   305,    24,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   445,   445,
     487,     4,    24,   489,    24,   489,   490,    41,    70,   103,
     122,   264,   265,   266,   269,   271,   518,   273,   494,    31,
      97,    57,    79,   124,   540,   336,   228,   558,   559,     5,
     290,   320,   325,   301,   303,   361,   518,   487,    24,   277,
      24,   325,    83,     5,   303,    13,   244,   325,   292,   466,
     466,   466,   466,   467,   466,   531,   292,     5,   163,   164,
     384,   385,   399,     4,   467,   241,   464,   464,     4,   219,
     411,   415,     4,    21,   392,     4,   203,   204,   388,   394,
       4,     4,     4,     4,     4,    13,     3,    84,    84,     4,
       5,     3,    40,    68,    93,   106,   133,   187,   188,   191,
     192,   194,   195,   197,   229,   230,   231,   232,   233,   234,
     235,   237,   238,   495,   498,   499,   506,   507,   508,   509,
     510,   511,   513,   514,   515,   516,   517,     4,     5,   304,
      97,   541,    98,    24,   560,   561,   302,     5,   318,    74,
     327,     4,     3,   276,   325,   298,   558,    56,   299,   466,
      98,   126,   433,   434,   441,     4,    69,   480,   480,     4,
     282,   293,   340,   343,   347,   350,   527,   531,     4,   292,
      38,   100,   278,     5,   388,   205,   205,    24,   336,     3,
     265,   272,   518,     3,   189,   193,   500,   501,   189,   193,
     500,   189,   193,   500,   501,   501,     3,   502,   503,    40,
     133,   502,   502,   153,   458,   459,   461,   462,   189,   193,
     500,   500,   267,    40,   496,   471,   497,   494,   130,   275,
     272,   542,   490,   244,   561,   325,    38,    75,   328,   272,
     336,     3,   487,   466,     4,     4,    85,   292,    72,   397,
     400,   416,   417,   494,    38,   217,   218,   412,   419,   420,
     393,   466,     4,   272,     4,     5,    21,   501,   190,   501,
     190,   501,   190,    21,   504,   189,   193,   500,   189,   500,
       3,   512,   500,   118,   501,   190,    23,    41,    56,   107,
     269,   270,   111,   497,    41,   173,   538,    24,   329,   363,
     562,   463,   336,   220,   404,   405,     4,   272,    61,    62,
     439,   209,   210,   388,     5,   497,   279,   280,   293,   337,
      36,    49,   215,   374,   421,   422,   423,   424,     4,   518,
       4,   502,   502,   502,    14,    15,    16,    17,    18,   505,
       4,   501,   190,   501,    21,   153,   458,   460,   502,    94,
       3,    94,   268,   363,   454,   487,    24,    99,   179,   180,
     550,   551,    69,   543,   544,   401,   406,   407,   408,   278,
       4,   417,     5,    32,    58,   281,   281,   215,   374,   422,
     425,   427,   428,   177,   216,   216,   211,   418,   429,   107,
     502,     4,     5,   500,   336,     3,   117,   177,   322,   117,
     322,   551,   172,   167,   546,     5,    31,   223,   315,   317,
     280,   214,   214,   241,    49,    74,   213,   221,   487,    21,
       4,   272,   321,   321,   321,   321,   177,   178,   545,     3,
     332,   174,   261,   547,   549,   407,   409,   452,   224,   306,
     307,   426,   427,   177,   212,     3,     4,     4,    24,   553,
     554,    24,   556,   552,   554,    24,   555,   336,   175,   224,
     316,   313,   363,    67,   311,   312,   272,     4,   548,   549,
     452,   177,   218,   310,   209,   225,   309,   558,     4,     7,
     308,   314,   363,   164,   549,   310,   226
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
            (yyval.pParseNode)->append((yyvsp[(1) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (6)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (6)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(6) - (6)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 392:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 394:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 395:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 399:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 400:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 402:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 403:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 404:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 406:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 407:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 408:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 409:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 410:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 411:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 412:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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

  case 421:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 423:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 424:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 425:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 426:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 428:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 429:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 430:

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

  case 432:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 433:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 434:

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

  case 441:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 442:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 445:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 447:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 450:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 451:

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

  case 452:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 453:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 454:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
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

  case 460:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 461:

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

  case 462:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 465:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 466:

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

  case 467:

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

  case 468:

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

  case 469:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 472:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 473:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 474:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 475:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode) = newNode(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 476:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 477:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 478:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 479:

    {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 480:

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

  case 481:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
			(yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
		}
    break;

  case 482:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 483:

    {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
		}
    break;

  case 485:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 486:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 487:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 489:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 490:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 496:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 499:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 500:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 502:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 504:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 505:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 507:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
    }
    break;

  case 508:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 509:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 510:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("K", SQL_NODE_PUNCTUATION));
        }
    break;

  case 511:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("M", SQL_NODE_PUNCTUATION));
        }
    break;

  case 512:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("G", SQL_NODE_PUNCTUATION));
        }
    break;

  case 513:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("T", SQL_NODE_PUNCTUATION));
        }
    break;

  case 514:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("P", SQL_NODE_PUNCTUATION));
        }
    break;

  case 515:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 516:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 517:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 518:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 519:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 520:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        }
    break;

  case 525:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
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
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 528:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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

  case 530:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
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

  case 536:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 537:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 538:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode) = newNode(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 548:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            /*$$->append($3);*/
        }
    break;

  case 549:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 553:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 554:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 555:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode) = newNode("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode) = newNode(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 558:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
        }
    break;

  case 559:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 560:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 561:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 562:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 563:

    {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 564:

    {(yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
        (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);}
    break;

  case 571:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 572:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 573:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
        }
    break;

  case 574:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 575:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 579:

    {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));}
    break;

  case 580:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode) = newNode(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            }
    break;

  case 581:

    {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode) = newNode("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 582:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 583:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
            (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 584:

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

  case 586:

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

  case 587:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 588:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 591:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 594:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 595:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 596:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 598:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
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
        (yyval.pParseNode)->append((yyvsp[(1) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (3)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
    }
    break;

  case 603:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 604:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
    }
    break;

  case 606:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (5)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (5)].pParseNode) = newNode(";", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[(5) - (5)].pParseNode));
    }
    break;

  case 607:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 608:

    {
            (yyvsp[(1) - (3)].pParseNode)->append((yyvsp[(3) - (3)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (3)].pParseNode);
        }
    break;

  case 610:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 611:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 612:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(3) - (4)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(4) - (4)].pParseNode));
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

  case 622:

    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 624:

    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[(1) - (2)].pParseNode));
        (yyval.pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
        }
    break;

  case 625:

    {
            (yyvsp[(1) - (2)].pParseNode)->append((yyvsp[(2) - (2)].pParseNode));
            (yyval.pParseNode) = (yyvsp[(1) - (2)].pParseNode);
        }
    break;

  case 626:

    {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
        }
    break;

  case 627:

    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[(1) - (1)].pParseNode));
            }
    break;

  case 628:

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
    BeMutexHolder aGuard(getCriticalSection());
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
