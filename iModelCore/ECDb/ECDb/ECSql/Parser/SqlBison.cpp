/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

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
#define yydebug         SQLyydebug
#define yynerrs         SQLyynerrs

/* First part of user prologue.  */

static Utf8String aEmptyString;

#define CREATE_NODE  context->GetScanner()->NewNode

// yyi is the internal number of the rule being reduced.
// Ueber die Mapping-Tabelle yyrmap wird daraus eine externe Regel-Nr.
#define SQL_NEW_RULE             context->GetScanner()->NewNode(aEmptyString, SQL_NODE_RULE, yyr1[yyn])
#define SQL_NEW_LISTRULE         context->GetScanner()->NewNode(aEmptyString, SQL_NODE_LISTRULE, yyr1[yyn])
#define SQL_NEW_COMMALISTRULE    context->GetScanner()->NewNode(aEmptyString, SQL_NODE_COMMALISTRULE, yyr1[yyn])
#define SQL_NEW_DOTLISTRULE      context->GetScanner()->NewNode(aEmptyString, SQL_NODE_DOTLISTRULE, yyr1[yyn])

#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE
#endif

#define SQLyyerror(context, s) \
    {                                 \
    context->error(s);                \
    }

using namespace connectivity;
#define SQLyylex context->SQLlex


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "SqlBison.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_3_ = 3,                         /* '('  */
  YYSYMBOL_4_ = 4,                         /* ')'  */
  YYSYMBOL_5_ = 5,                         /* ','  */
  YYSYMBOL_6_ = 6,                         /* ':'  */
  YYSYMBOL_7_ = 7,                         /* ';'  */
  YYSYMBOL_8_ = 8,                         /* '?'  */
  YYSYMBOL_9_ = 9,                         /* '['  */
  YYSYMBOL_10_ = 10,                       /* ']'  */
  YYSYMBOL_11_ = 11,                       /* '{'  */
  YYSYMBOL_12_ = 12,                       /* '}'  */
  YYSYMBOL_13_ = 13,                       /* '.'  */
  YYSYMBOL_14_K_ = 14,                     /* 'K'  */
  YYSYMBOL_15_M_ = 15,                     /* 'M'  */
  YYSYMBOL_16_G_ = 16,                     /* 'G'  */
  YYSYMBOL_17_T_ = 17,                     /* 'T'  */
  YYSYMBOL_18_P_ = 18,                     /* 'P'  */
  YYSYMBOL_SQL_TOKEN_ACCESS_DATE = 19,     /* SQL_TOKEN_ACCESS_DATE  */
  YYSYMBOL_SQL_TOKEN_REAL_NUM = 20,        /* SQL_TOKEN_REAL_NUM  */
  YYSYMBOL_SQL_TOKEN_INTNUM = 21,          /* SQL_TOKEN_INTNUM  */
  YYSYMBOL_SQL_TOKEN_APPROXNUM = 22,       /* SQL_TOKEN_APPROXNUM  */
  YYSYMBOL_SQL_TOKEN_NOT = 23,             /* SQL_TOKEN_NOT  */
  YYSYMBOL_SQL_TOKEN_NAME = 24,            /* SQL_TOKEN_NAME  */
  YYSYMBOL_SQL_TOKEN_ARRAY_INDEX = 25,     /* SQL_TOKEN_ARRAY_INDEX  */
  YYSYMBOL_SQL_TOKEN_UMINUS = 26,          /* SQL_TOKEN_UMINUS  */
  YYSYMBOL_SQL_TOKEN_WITH = 27,            /* SQL_TOKEN_WITH  */
  YYSYMBOL_SQL_TOKEN_RECURSIVE = 28,       /* SQL_TOKEN_RECURSIVE  */
  YYSYMBOL_SQL_TOKEN_ALL = 29,             /* SQL_TOKEN_ALL  */
  YYSYMBOL_SQL_TOKEN_ANY = 30,             /* SQL_TOKEN_ANY  */
  YYSYMBOL_SQL_TOKEN_AS = 31,              /* SQL_TOKEN_AS  */
  YYSYMBOL_SQL_TOKEN_ASC = 32,             /* SQL_TOKEN_ASC  */
  YYSYMBOL_SQL_TOKEN_AVG = 33,             /* SQL_TOKEN_AVG  */
  YYSYMBOL_SQL_TOKEN_BETWEEN = 34,         /* SQL_TOKEN_BETWEEN  */
  YYSYMBOL_SQL_TOKEN_BY = 35,              /* SQL_TOKEN_BY  */
  YYSYMBOL_SQL_TOKEN_NULLS = 36,           /* SQL_TOKEN_NULLS  */
  YYSYMBOL_SQL_TOKEN_FIRST = 37,           /* SQL_TOKEN_FIRST  */
  YYSYMBOL_SQL_TOKEN_LAST = 38,            /* SQL_TOKEN_LAST  */
  YYSYMBOL_SQL_TOKEN_CAST = 39,            /* SQL_TOKEN_CAST  */
  YYSYMBOL_SQL_TOKEN_COMMIT = 40,          /* SQL_TOKEN_COMMIT  */
  YYSYMBOL_SQL_TOKEN_COUNT = 41,           /* SQL_TOKEN_COUNT  */
  YYSYMBOL_SQL_TOKEN_CROSS = 42,           /* SQL_TOKEN_CROSS  */
  YYSYMBOL_SQL_TOKEN_DELETE = 43,          /* SQL_TOKEN_DELETE  */
  YYSYMBOL_SQL_TOKEN_DESC = 44,            /* SQL_TOKEN_DESC  */
  YYSYMBOL_SQL_TOKEN_DISTINCT = 45,        /* SQL_TOKEN_DISTINCT  */
  YYSYMBOL_SQL_TOKEN_FORWARD = 46,         /* SQL_TOKEN_FORWARD  */
  YYSYMBOL_SQL_TOKEN_BACKWARD = 47,        /* SQL_TOKEN_BACKWARD  */
  YYSYMBOL_SQL_TOKEN_ESCAPE = 48,          /* SQL_TOKEN_ESCAPE  */
  YYSYMBOL_SQL_TOKEN_EXCEPT = 49,          /* SQL_TOKEN_EXCEPT  */
  YYSYMBOL_SQL_TOKEN_EXISTS = 50,          /* SQL_TOKEN_EXISTS  */
  YYSYMBOL_SQL_TOKEN_FALSE = 51,           /* SQL_TOKEN_FALSE  */
  YYSYMBOL_SQL_TOKEN_FROM = 52,            /* SQL_TOKEN_FROM  */
  YYSYMBOL_SQL_TOKEN_FULL = 53,            /* SQL_TOKEN_FULL  */
  YYSYMBOL_SQL_TOKEN_GROUP = 54,           /* SQL_TOKEN_GROUP  */
  YYSYMBOL_SQL_TOKEN_HAVING = 55,          /* SQL_TOKEN_HAVING  */
  YYSYMBOL_SQL_TOKEN_IN = 56,              /* SQL_TOKEN_IN  */
  YYSYMBOL_SQL_TOKEN_INNER = 57,           /* SQL_TOKEN_INNER  */
  YYSYMBOL_SQL_TOKEN_INSERT = 58,          /* SQL_TOKEN_INSERT  */
  YYSYMBOL_SQL_TOKEN_INTO = 59,            /* SQL_TOKEN_INTO  */
  YYSYMBOL_SQL_TOKEN_IS = 60,              /* SQL_TOKEN_IS  */
  YYSYMBOL_SQL_TOKEN_INTERSECT = 61,       /* SQL_TOKEN_INTERSECT  */
  YYSYMBOL_SQL_TOKEN_JOIN = 62,            /* SQL_TOKEN_JOIN  */
  YYSYMBOL_SQL_TOKEN_LIKE = 63,            /* SQL_TOKEN_LIKE  */
  YYSYMBOL_SQL_TOKEN_LEFT = 64,            /* SQL_TOKEN_LEFT  */
  YYSYMBOL_SQL_TOKEN_RIGHT = 65,           /* SQL_TOKEN_RIGHT  */
  YYSYMBOL_SQL_TOKEN_MAX = 66,             /* SQL_TOKEN_MAX  */
  YYSYMBOL_SQL_TOKEN_MIN = 67,             /* SQL_TOKEN_MIN  */
  YYSYMBOL_SQL_TOKEN_NATURAL = 68,         /* SQL_TOKEN_NATURAL  */
  YYSYMBOL_SQL_TOKEN_NULL = 69,            /* SQL_TOKEN_NULL  */
  YYSYMBOL_SQL_TOKEN_TOTAL = 70,           /* SQL_TOKEN_TOTAL  */
  YYSYMBOL_SQL_TOKEN_ON = 71,              /* SQL_TOKEN_ON  */
  YYSYMBOL_SQL_TOKEN_ORDER = 72,           /* SQL_TOKEN_ORDER  */
  YYSYMBOL_SQL_TOKEN_OUTER = 73,           /* SQL_TOKEN_OUTER  */
  YYSYMBOL_SQL_TOKEN_ROLLBACK = 74,        /* SQL_TOKEN_ROLLBACK  */
  YYSYMBOL_SQL_TOKEN_IIF = 75,             /* SQL_TOKEN_IIF  */
  YYSYMBOL_SQL_TOKEN_SELECT = 76,          /* SQL_TOKEN_SELECT  */
  YYSYMBOL_SQL_TOKEN_SET = 77,             /* SQL_TOKEN_SET  */
  YYSYMBOL_SQL_TOKEN_SOME = 78,            /* SQL_TOKEN_SOME  */
  YYSYMBOL_SQL_TOKEN_SUM = 79,             /* SQL_TOKEN_SUM  */
  YYSYMBOL_SQL_TOKEN_TRUE = 80,            /* SQL_TOKEN_TRUE  */
  YYSYMBOL_SQL_TOKEN_UNION = 81,           /* SQL_TOKEN_UNION  */
  YYSYMBOL_SQL_TOKEN_UNIQUE = 82,          /* SQL_TOKEN_UNIQUE  */
  YYSYMBOL_SQL_TOKEN_UNKNOWN = 83,         /* SQL_TOKEN_UNKNOWN  */
  YYSYMBOL_SQL_TOKEN_UPDATE = 84,          /* SQL_TOKEN_UPDATE  */
  YYSYMBOL_SQL_TOKEN_USING = 85,           /* SQL_TOKEN_USING  */
  YYSYMBOL_SQL_TOKEN_VALUE = 86,           /* SQL_TOKEN_VALUE  */
  YYSYMBOL_SQL_TOKEN_VALUES = 87,          /* SQL_TOKEN_VALUES  */
  YYSYMBOL_SQL_TOKEN_WHERE = 88,           /* SQL_TOKEN_WHERE  */
  YYSYMBOL_SQL_TOKEN_DOLLAR = 89,          /* SQL_TOKEN_DOLLAR  */
  YYSYMBOL_SQL_BITWISE_NOT = 90,           /* SQL_BITWISE_NOT  */
  YYSYMBOL_SQL_TOKEN_CURRENT_DATE = 91,    /* SQL_TOKEN_CURRENT_DATE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIME = 92,    /* SQL_TOKEN_CURRENT_TIME  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIMESTAMP = 93, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_EVERY = 94,           /* SQL_TOKEN_EVERY  */
  YYSYMBOL_SQL_TOKEN_CASE = 95,            /* SQL_TOKEN_CASE  */
  YYSYMBOL_SQL_TOKEN_THEN = 96,            /* SQL_TOKEN_THEN  */
  YYSYMBOL_SQL_TOKEN_END = 97,             /* SQL_TOKEN_END  */
  YYSYMBOL_SQL_TOKEN_WHEN = 98,            /* SQL_TOKEN_WHEN  */
  YYSYMBOL_SQL_TOKEN_ELSE = 99,            /* SQL_TOKEN_ELSE  */
  YYSYMBOL_SQL_TOKEN_LIMIT = 100,          /* SQL_TOKEN_LIMIT  */
  YYSYMBOL_SQL_TOKEN_OFFSET = 101,         /* SQL_TOKEN_OFFSET  */
  YYSYMBOL_SQL_TOKEN_ONLY = 102,           /* SQL_TOKEN_ONLY  */
  YYSYMBOL_SQL_TOKEN_PRAGMA = 103,         /* SQL_TOKEN_PRAGMA  */
  YYSYMBOL_SQL_TOKEN_FOR = 104,            /* SQL_TOKEN_FOR  */
  YYSYMBOL_SQL_TOKEN_MATCH = 105,          /* SQL_TOKEN_MATCH  */
  YYSYMBOL_SQL_TOKEN_ECSQLOPTIONS = 106,   /* SQL_TOKEN_ECSQLOPTIONS  */
  YYSYMBOL_SQL_TOKEN_INTEGER = 107,        /* SQL_TOKEN_INTEGER  */
  YYSYMBOL_SQL_TOKEN_INT = 108,            /* SQL_TOKEN_INT  */
  YYSYMBOL_SQL_TOKEN_INT64 = 109,          /* SQL_TOKEN_INT64  */
  YYSYMBOL_SQL_TOKEN_LONG = 110,           /* SQL_TOKEN_LONG  */
  YYSYMBOL_SQL_TOKEN_BOOLEAN = 111,        /* SQL_TOKEN_BOOLEAN  */
  YYSYMBOL_SQL_TOKEN_DOUBLE = 112,         /* SQL_TOKEN_DOUBLE  */
  YYSYMBOL_SQL_TOKEN_REAL = 113,           /* SQL_TOKEN_REAL  */
  YYSYMBOL_SQL_TOKEN_FLOAT = 114,          /* SQL_TOKEN_FLOAT  */
  YYSYMBOL_SQL_TOKEN_STRING = 115,         /* SQL_TOKEN_STRING  */
  YYSYMBOL_SQL_TOKEN_VARCHAR = 116,        /* SQL_TOKEN_VARCHAR  */
  YYSYMBOL_SQL_TOKEN_BINARY = 117,         /* SQL_TOKEN_BINARY  */
  YYSYMBOL_SQL_TOKEN_BLOB = 118,           /* SQL_TOKEN_BLOB  */
  YYSYMBOL_SQL_TOKEN_DATE = 119,           /* SQL_TOKEN_DATE  */
  YYSYMBOL_SQL_TOKEN_TIME = 120,           /* SQL_TOKEN_TIME  */
  YYSYMBOL_SQL_TOKEN_TIMESTAMP = 121,      /* SQL_TOKEN_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_OVER = 122,           /* SQL_TOKEN_OVER  */
  YYSYMBOL_SQL_TOKEN_ROW_NUMBER = 123,     /* SQL_TOKEN_ROW_NUMBER  */
  YYSYMBOL_SQL_TOKEN_NTILE = 124,          /* SQL_TOKEN_NTILE  */
  YYSYMBOL_SQL_TOKEN_LEAD = 125,           /* SQL_TOKEN_LEAD  */
  YYSYMBOL_SQL_TOKEN_LAG = 126,            /* SQL_TOKEN_LAG  */
  YYSYMBOL_SQL_TOKEN_FIRST_VALUE = 127,    /* SQL_TOKEN_FIRST_VALUE  */
  YYSYMBOL_SQL_TOKEN_LAST_VALUE = 128,     /* SQL_TOKEN_LAST_VALUE  */
  YYSYMBOL_SQL_TOKEN_NTH_VALUE = 129,      /* SQL_TOKEN_NTH_VALUE  */
  YYSYMBOL_SQL_TOKEN_EXCLUDE = 130,        /* SQL_TOKEN_EXCLUDE  */
  YYSYMBOL_SQL_TOKEN_OTHERS = 131,         /* SQL_TOKEN_OTHERS  */
  YYSYMBOL_SQL_TOKEN_TIES = 132,           /* SQL_TOKEN_TIES  */
  YYSYMBOL_SQL_TOKEN_FOLLOWING = 133,      /* SQL_TOKEN_FOLLOWING  */
  YYSYMBOL_SQL_TOKEN_UNBOUNDED = 134,      /* SQL_TOKEN_UNBOUNDED  */
  YYSYMBOL_SQL_TOKEN_PRECEDING = 135,      /* SQL_TOKEN_PRECEDING  */
  YYSYMBOL_SQL_TOKEN_RANGE = 136,          /* SQL_TOKEN_RANGE  */
  YYSYMBOL_SQL_TOKEN_ROWS = 137,           /* SQL_TOKEN_ROWS  */
  YYSYMBOL_SQL_TOKEN_PARTITION = 138,      /* SQL_TOKEN_PARTITION  */
  YYSYMBOL_SQL_TOKEN_WINDOW = 139,         /* SQL_TOKEN_WINDOW  */
  YYSYMBOL_SQL_TOKEN_NO = 140,             /* SQL_TOKEN_NO  */
  YYSYMBOL_SQL_TOKEN_CURRENT = 141,        /* SQL_TOKEN_CURRENT  */
  YYSYMBOL_SQL_TOKEN_ROW = 142,            /* SQL_TOKEN_ROW  */
  YYSYMBOL_SQL_TOKEN_RANK = 143,           /* SQL_TOKEN_RANK  */
  YYSYMBOL_SQL_TOKEN_DENSE_RANK = 144,     /* SQL_TOKEN_DENSE_RANK  */
  YYSYMBOL_SQL_TOKEN_PERCENT_RANK = 145,   /* SQL_TOKEN_PERCENT_RANK  */
  YYSYMBOL_SQL_TOKEN_CUME_DIST = 146,      /* SQL_TOKEN_CUME_DIST  */
  YYSYMBOL_SQL_TOKEN_COLLATE = 147,        /* SQL_TOKEN_COLLATE  */
  YYSYMBOL_SQL_TOKEN_NOCASE = 148,         /* SQL_TOKEN_NOCASE  */
  YYSYMBOL_SQL_TOKEN_RTRIM = 149,          /* SQL_TOKEN_RTRIM  */
  YYSYMBOL_SQL_TOKEN_FILTER = 150,         /* SQL_TOKEN_FILTER  */
  YYSYMBOL_SQL_TOKEN_GROUPS = 151,         /* SQL_TOKEN_GROUPS  */
  YYSYMBOL_SQL_TOKEN_OR = 152,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 153,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_ARROW = 154,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 155,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 156,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 157,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 158,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 159,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 160,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 161,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 162,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 163,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 164,                /* SQL_EQUAL  */
  YYSYMBOL_165_ = 165,                     /* '+'  */
  YYSYMBOL_166_ = 166,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 167,               /* SQL_CONCAT  */
  YYSYMBOL_168_ = 168,                     /* '*'  */
  YYSYMBOL_169_ = 169,                     /* '/'  */
  YYSYMBOL_170_ = 170,                     /* '%'  */
  YYSYMBOL_171_ = 171,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_GROUP_CONCAT = 172,   /* SQL_TOKEN_GROUP_CONCAT  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 173,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 174,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 175,     /* sql_single_statement  */
  YYSYMBOL_sql = 176,                      /* sql  */
  YYSYMBOL_pragma = 177,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 178,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 179,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 180,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 181,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 182,             /* pragma_value  */
  YYSYMBOL_pragma_path = 183,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 184,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 185,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 186,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 187,           /* cte_block_list  */
  YYSYMBOL_cte = 188,                      /* cte  */
  YYSYMBOL_column_commalist = 189,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 190,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 191,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 192, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 193,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 194,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 195,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 196,             /* opt_asc_desc  */
  YYSYMBOL_opt_null_order = 197,           /* opt_null_order  */
  YYSYMBOL_first_last_desc = 198,          /* first_last_desc  */
  YYSYMBOL_sql_not = 199,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 200,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 201,         /* select_statement  */
  YYSYMBOL_union_op = 202,                 /* union_op  */
  YYSYMBOL_commit_statement = 203,         /* commit_statement  */
  YYSYMBOL_delete_statement_searched = 204, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 205,         /* insert_statement  */
  YYSYMBOL_values_or_query_spec = 206,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 207, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 208,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 209, /* row_value_constructor_elem  */
  YYSYMBOL_rollback_statement = 210,       /* rollback_statement  */
  YYSYMBOL_select_statement_into = 211,    /* select_statement_into  */
  YYSYMBOL_opt_all_distinct = 212,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 213,     /* assignment_commalist  */
  YYSYMBOL_assignment = 214,               /* assignment  */
  YYSYMBOL_update_source = 215,            /* update_source  */
  YYSYMBOL_update_statement_searched = 216, /* update_statement_searched  */
  YYSYMBOL_target_commalist = 217,         /* target_commalist  */
  YYSYMBOL_target = 218,                   /* target  */
  YYSYMBOL_opt_where_clause = 219,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 220,  /* single_select_statement  */
  YYSYMBOL_selection = 221,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 222,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 223,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 224,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 225,                /* table_exp  */
  YYSYMBOL_from_clause = 226,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 227,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 228,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 229, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 230, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 231,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 232, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 233,                /* table_ref  */
  YYSYMBOL_where_clause = 234,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 235,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 236,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 237,              /* truth_value  */
  YYSYMBOL_boolean_primary = 238,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 239,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 240,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 241,             /* boolean_term  */
  YYSYMBOL_search_condition = 242,         /* search_condition  */
  YYSYMBOL_type_predicate = 243,           /* type_predicate  */
  YYSYMBOL_type_list = 244,                /* type_list  */
  YYSYMBOL_type_list_item = 245,           /* type_list_item  */
  YYSYMBOL_predicate = 246,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 247, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 248,     /* comparison_predicate  */
  YYSYMBOL_comparison = 249,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 250, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 251,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 252, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 253, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 254,           /* like_predicate  */
  YYSYMBOL_opt_escape = 255,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 256,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 257,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 258,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 259,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 260,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 261, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 262,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 263,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 264, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 265,             /* any_all_some  */
  YYSYMBOL_existence_test = 266,           /* existence_test  */
  YYSYMBOL_unique_test = 267,              /* unique_test  */
  YYSYMBOL_subquery = 268,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 269,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 270,           /* select_sublist  */
  YYSYMBOL_parameter_ref = 271,            /* parameter_ref  */
  YYSYMBOL_literal = 272,                  /* literal  */
  YYSYMBOL_as_clause = 273,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 274,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 275,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 276,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 277,                 /* fct_spec  */
  YYSYMBOL_function_name = 278,            /* function_name  */
  YYSYMBOL_general_set_fct = 279,          /* general_set_fct  */
  YYSYMBOL_set_fct_type = 280,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 281,          /* outer_join_type  */
  YYSYMBOL_join_condition = 282,           /* join_condition  */
  YYSYMBOL_join_spec = 283,                /* join_spec  */
  YYSYMBOL_join_type = 284,                /* join_type  */
  YYSYMBOL_cross_union = 285,              /* cross_union  */
  YYSYMBOL_qualified_join = 286,           /* qualified_join  */
  YYSYMBOL_window_function = 287,          /* window_function  */
  YYSYMBOL_window_function_type = 288,     /* window_function_type  */
  YYSYMBOL_ntile_function = 289,           /* ntile_function  */
  YYSYMBOL_opt_lead_or_lag_function = 290, /* opt_lead_or_lag_function  */
  YYSYMBOL_lead_or_lag_function = 291,     /* lead_or_lag_function  */
  YYSYMBOL_lead_or_lag = 292,              /* lead_or_lag  */
  YYSYMBOL_lead_or_lag_extent = 293,       /* lead_or_lag_extent  */
  YYSYMBOL_first_or_last_value_function = 294, /* first_or_last_value_function  */
  YYSYMBOL_first_or_last_value = 295,      /* first_or_last_value  */
  YYSYMBOL_nth_value_function = 296,       /* nth_value_function  */
  YYSYMBOL_opt_filter_clause = 297,        /* opt_filter_clause  */
  YYSYMBOL_window_name = 298,              /* window_name  */
  YYSYMBOL_window_name_or_specification = 299, /* window_name_or_specification  */
  YYSYMBOL_in_line_window_specification = 300, /* in_line_window_specification  */
  YYSYMBOL_opt_window_clause = 301,        /* opt_window_clause  */
  YYSYMBOL_window_definition_list = 302,   /* window_definition_list  */
  YYSYMBOL_window_definition = 303,        /* window_definition  */
  YYSYMBOL_new_window_name = 304,          /* new_window_name  */
  YYSYMBOL_window_specification = 305,     /* window_specification  */
  YYSYMBOL_opt_existing_window_name = 306, /* opt_existing_window_name  */
  YYSYMBOL_existing_window_name = 307,     /* existing_window_name  */
  YYSYMBOL_opt_window_partition_clause = 308, /* opt_window_partition_clause  */
  YYSYMBOL_opt_window_frame_clause = 309,  /* opt_window_frame_clause  */
  YYSYMBOL_window_partition_column_reference_list = 310, /* window_partition_column_reference_list  */
  YYSYMBOL_window_partition_column_reference = 311, /* window_partition_column_reference  */
  YYSYMBOL_opt_window_frame_exclusion = 312, /* opt_window_frame_exclusion  */
  YYSYMBOL_window_frame_units = 313,       /* window_frame_units  */
  YYSYMBOL_window_frame_extent = 314,      /* window_frame_extent  */
  YYSYMBOL_window_frame_start = 315,       /* window_frame_start  */
  YYSYMBOL_window_frame_preceding = 316,   /* window_frame_preceding  */
  YYSYMBOL_window_frame_between = 317,     /* window_frame_between  */
  YYSYMBOL_window_frame_bound = 318,       /* window_frame_bound  */
  YYSYMBOL_window_frame_bound_1 = 319,     /* window_frame_bound_1  */
  YYSYMBOL_window_frame_bound_2 = 320,     /* window_frame_bound_2  */
  YYSYMBOL_window_frame_following = 321,   /* window_frame_following  */
  YYSYMBOL_rank_function_type = 322,       /* rank_function_type  */
  YYSYMBOL_opt_collate_clause = 323,       /* opt_collate_clause  */
  YYSYMBOL_collating_function = 324,       /* collating_function  */
  YYSYMBOL_ecrelationship_join = 325,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 326, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 327,             /* joined_table  */
  YYSYMBOL_named_columns_join = 328,       /* named_columns_join  */
  YYSYMBOL_all = 329,                      /* all  */
  YYSYMBOL_scalar_subquery = 330,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 331,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 332, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 333,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 334,        /* cast_target_array  */
  YYSYMBOL_cast_target = 335,              /* cast_target  */
  YYSYMBOL_cast_spec = 336,                /* cast_spec  */
  YYSYMBOL_opt_extract_value = 337,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 338,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 339,              /* num_primary  */
  YYSYMBOL_factor = 340,                   /* factor  */
  YYSYMBOL_term = 341,                     /* term  */
  YYSYMBOL_term_add_sub = 342,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 343,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 344,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 345,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 346,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 347,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 348,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 349,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 350,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 351,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 352,                /* value_exp  */
  YYSYMBOL_string_value_exp = 353,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 354,           /* char_value_exp  */
  YYSYMBOL_concatenation = 355,            /* concatenation  */
  YYSYMBOL_char_primary = 356,             /* char_primary  */
  YYSYMBOL_char_factor = 357,              /* char_factor  */
  YYSYMBOL_derived_column = 358,           /* derived_column  */
  YYSYMBOL_table_node = 359,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 360, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 361,     /* qualified_class_name  */
  YYSYMBOL_class_name = 362,               /* class_name  */
  YYSYMBOL_table_node_ref = 363,           /* table_node_ref  */
  YYSYMBOL_table_node_with_opt_member_func_call = 364, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 365,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 366,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 367, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 368,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 369,            /* property_path  */
  YYSYMBOL_property_path_entry = 370,      /* property_path_entry  */
  YYSYMBOL_column_ref = 371,               /* column_ref  */
  YYSYMBOL_column = 372,                   /* column  */
  YYSYMBOL_case_expression = 373,          /* case_expression  */
  YYSYMBOL_case_specification = 374,       /* case_specification  */
  YYSYMBOL_simple_case = 375,              /* simple_case  */
  YYSYMBOL_searched_case = 376,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 377,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 378,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 379,        /* when_operand_list  */
  YYSYMBOL_when_operand = 380,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 381, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 382,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 383,              /* else_clause  */
  YYSYMBOL_result = 384,                   /* result  */
  YYSYMBOL_result_expression = 385,        /* result_expression  */
  YYSYMBOL_case_operand = 386,             /* case_operand  */
  YYSYMBOL_parameter = 387,                /* parameter  */
  YYSYMBOL_range_variable = 388,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 389,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 390,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 391,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 392,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 393          /* ecsqloptionvalue  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3183

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  174
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  220
/* YYNRULES -- Number of rules.  */
#define YYNRULES  476
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  753

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   406


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   170,     2,     2,
       3,     4,   168,   165,     5,   166,    13,   169,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   171,     2,     8,     2,     2,     2,     2,     2,     2,
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
     161,   162,   163,   164,   167,   172,   173
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   244,   244,   245,   246,   247,   251,   257,   269,   272,
     280,   283,   284,   288,   296,   304,   305,   306,   307,   308,
     309,   310,   314,   319,   324,   336,   339,   343,   348,   356,
     370,   376,   384,   396,   401,   409,   414,   424,   425,   433,
     434,   445,   446,   456,   461,   469,   478,   489,   490,   491,
     495,   496,   505,   505,   509,   510,   516,   517,   518,   519,
     520,   521,   522,   523,   527,   532,   543,   544,   545,   548,
     555,   567,   576,   586,   591,   599,   602,   607,   616,   627,
     628,   629,   633,   636,   642,   649,   652,   664,   667,   673,
     677,   678,   686,   694,   698,   703,   706,   707,   710,   711,
     719,   728,   729,   744,   754,   757,   763,   764,   768,   771,
     781,   782,   789,   790,   795,   803,   804,   811,   819,   827,
     835,   839,   848,   849,   859,   860,   868,   869,   870,   871,
     872,   875,   876,   877,   878,   879,   880,   881,   891,   892,
     902,   903,   911,   912,   921,   922,   932,   941,   946,   954,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   977,
     984,   991,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1030,
    1040,  1048,  1058,  1068,  1074,  1080,  1102,  1127,  1128,  1135,
    1144,  1150,  1166,  1170,  1178,  1187,  1193,  1209,  1218,  1227,
    1236,  1246,  1247,  1248,  1252,  1260,  1266,  1277,  1282,  1289,
    1293,  1297,  1298,  1299,  1300,  1301,  1303,  1315,  1327,  1339,
    1355,  1356,  1362,  1366,  1367,  1370,  1371,  1372,  1373,  1376,
    1388,  1389,  1390,  1397,  1406,  1420,  1425,  1441,  1457,  1466,
    1474,  1483,  1492,  1502,  1511,  1525,  1526,  1527,  1528,  1529,
    1530,  1534,  1539,  1544,  1551,  1559,  1560,  1563,  1564,  1569,
    1570,  1578,  1590,  1600,  1609,  1613,  1624,  1631,  1638,  1639,
    1640,  1641,  1642,  1646,  1657,  1658,  1664,  1675,  1687,  1688,
    1692,  1697,  1708,  1709,  1713,  1727,  1728,  1738,  1742,  1743,
    1747,  1752,  1753,  1762,  1765,  1771,  1781,  1785,  1803,  1804,
    1808,  1813,  1814,  1825,  1826,  1836,  1839,  1845,  1855,  1856,
    1863,  1869,  1875,  1886,  1887,  1888,  1892,  1893,  1897,  1903,
    1910,  1919,  1928,  1939,  1940,  1946,  1950,  1951,  1960,  1961,
    1970,  1979,  1980,  1981,  1982,  1987,  1988,  1997,  1998,  1999,
    2003,  2017,  2018,  2019,  2022,  2023,  2026,  2038,  2039,  2043,
    2046,  2050,  2051,  2052,  2053,  2054,  2055,  2056,  2057,  2058,
    2059,  2060,  2061,  2062,  2063,  2064,  2065,  2069,  2074,  2084,
    2093,  2094,  2098,  2111,  2112,  2120,  2121,  2122,  2123,  2124,
    2125,  2126,  2133,  2137,  2141,  2142,  2148,  2154,  2163,  2164,
    2171,  2178,  2188,  2189,  2196,  2206,  2207,  2214,  2221,  2228,
    2238,  2245,  2250,  2255,  2260,  2266,  2272,  2281,  2288,  2296,
    2304,  2307,  2313,  2317,  2322,  2330,  2331,  2332,  2336,  2340,
    2341,  2344,  2351,  2361,  2364,  2368,  2378,  2383,  2390,  2399,
    2407,  2416,  2424,  2433,  2441,  2446,  2451,  2459,  2468,  2469,
    2479,  2480,  2488,  2493,  2511,  2517,  2522,  2530,  2542,  2546,
    2550,  2551,  2555,  2566,  2576,  2581,  2588,  2598,  2601,  2606,
    2607,  2608,  2609,  2610,  2611,  2614,  2619,  2626,  2636,  2637,
    2645,  2649,  2653,  2657,  2663,  2671,  2674,  2684,  2685,  2689,
    2698,  2703,  2711,  2717,  2727,  2728,  2729
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "'('", "')'", "','",
  "':'", "';'", "'?'", "'['", "']'", "'{'", "'}'", "'.'", "'K'", "'M'",
  "'G'", "'T'", "'P'", "SQL_TOKEN_ACCESS_DATE", "SQL_TOKEN_REAL_NUM",
  "SQL_TOKEN_INTNUM", "SQL_TOKEN_APPROXNUM", "SQL_TOKEN_NOT",
  "SQL_TOKEN_NAME", "SQL_TOKEN_ARRAY_INDEX", "SQL_TOKEN_UMINUS",
  "SQL_TOKEN_WITH", "SQL_TOKEN_RECURSIVE", "SQL_TOKEN_ALL",
  "SQL_TOKEN_ANY", "SQL_TOKEN_AS", "SQL_TOKEN_ASC", "SQL_TOKEN_AVG",
  "SQL_TOKEN_BETWEEN", "SQL_TOKEN_BY", "SQL_TOKEN_NULLS",
  "SQL_TOKEN_FIRST", "SQL_TOKEN_LAST", "SQL_TOKEN_CAST",
  "SQL_TOKEN_COMMIT", "SQL_TOKEN_COUNT", "SQL_TOKEN_CROSS",
  "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC", "SQL_TOKEN_DISTINCT",
  "SQL_TOKEN_FORWARD", "SQL_TOKEN_BACKWARD", "SQL_TOKEN_ESCAPE",
  "SQL_TOKEN_EXCEPT", "SQL_TOKEN_EXISTS", "SQL_TOKEN_FALSE",
  "SQL_TOKEN_FROM", "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP",
  "SQL_TOKEN_HAVING", "SQL_TOKEN_IN", "SQL_TOKEN_INNER",
  "SQL_TOKEN_INSERT", "SQL_TOKEN_INTO", "SQL_TOKEN_IS",
  "SQL_TOKEN_INTERSECT", "SQL_TOKEN_JOIN", "SQL_TOKEN_LIKE",
  "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN",
  "SQL_TOKEN_NATURAL", "SQL_TOKEN_NULL", "SQL_TOKEN_TOTAL", "SQL_TOKEN_ON",
  "SQL_TOKEN_ORDER", "SQL_TOKEN_OUTER", "SQL_TOKEN_ROLLBACK",
  "SQL_TOKEN_IIF", "SQL_TOKEN_SELECT", "SQL_TOKEN_SET", "SQL_TOKEN_SOME",
  "SQL_TOKEN_SUM", "SQL_TOKEN_TRUE", "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE",
  "SQL_TOKEN_UNKNOWN", "SQL_TOKEN_UPDATE", "SQL_TOKEN_USING",
  "SQL_TOKEN_VALUE", "SQL_TOKEN_VALUES", "SQL_TOKEN_WHERE",
  "SQL_TOKEN_DOLLAR", "SQL_BITWISE_NOT", "SQL_TOKEN_CURRENT_DATE",
  "SQL_TOKEN_CURRENT_TIME", "SQL_TOKEN_CURRENT_TIMESTAMP",
  "SQL_TOKEN_EVERY", "SQL_TOKEN_CASE", "SQL_TOKEN_THEN", "SQL_TOKEN_END",
  "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE", "SQL_TOKEN_LIMIT",
  "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY", "SQL_TOKEN_PRAGMA",
  "SQL_TOKEN_FOR", "SQL_TOKEN_MATCH", "SQL_TOKEN_ECSQLOPTIONS",
  "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT", "SQL_TOKEN_INT64",
  "SQL_TOKEN_LONG", "SQL_TOKEN_BOOLEAN", "SQL_TOKEN_DOUBLE",
  "SQL_TOKEN_REAL", "SQL_TOKEN_FLOAT", "SQL_TOKEN_STRING",
  "SQL_TOKEN_VARCHAR", "SQL_TOKEN_BINARY", "SQL_TOKEN_BLOB",
  "SQL_TOKEN_DATE", "SQL_TOKEN_TIME", "SQL_TOKEN_TIMESTAMP",
  "SQL_TOKEN_OVER", "SQL_TOKEN_ROW_NUMBER", "SQL_TOKEN_NTILE",
  "SQL_TOKEN_LEAD", "SQL_TOKEN_LAG", "SQL_TOKEN_FIRST_VALUE",
  "SQL_TOKEN_LAST_VALUE", "SQL_TOKEN_NTH_VALUE", "SQL_TOKEN_EXCLUDE",
  "SQL_TOKEN_OTHERS", "SQL_TOKEN_TIES", "SQL_TOKEN_FOLLOWING",
  "SQL_TOKEN_UNBOUNDED", "SQL_TOKEN_PRECEDING", "SQL_TOKEN_RANGE",
  "SQL_TOKEN_ROWS", "SQL_TOKEN_PARTITION", "SQL_TOKEN_WINDOW",
  "SQL_TOKEN_NO", "SQL_TOKEN_CURRENT", "SQL_TOKEN_ROW", "SQL_TOKEN_RANK",
  "SQL_TOKEN_DENSE_RANK", "SQL_TOKEN_PERCENT_RANK", "SQL_TOKEN_CUME_DIST",
  "SQL_TOKEN_COLLATE", "SQL_TOKEN_NOCASE", "SQL_TOKEN_RTRIM",
  "SQL_TOKEN_FILTER", "SQL_TOKEN_GROUPS", "SQL_TOKEN_OR", "SQL_TOKEN_AND",
  "SQL_ARROW", "SQL_BITWISE_OR", "SQL_BITWISE_AND",
  "SQL_BITWISE_SHIFT_LEFT", "SQL_BITWISE_SHIFT_RIGHT", "SQL_LESSEQ",
  "SQL_GREATEQ", "SQL_NOTEQUAL", "SQL_LESS", "SQL_GREAT", "SQL_EQUAL",
  "'+'", "'-'", "SQL_CONCAT", "'*'", "'/'", "'%'", "'='",
  "SQL_TOKEN_GROUP_CONCAT", "SQL_TOKEN_INVALIDSYMBOL", "$accept",
  "sql_single_statement", "sql", "pragma", "opt_pragma_for",
  "opt_pragma_set", "opt_pragma_set_val", "opt_pragma_func",
  "pragma_value", "pragma_path", "opt_cte_recursive", "cte_column_list",
  "cte_table_name", "cte_block_list", "cte", "column_commalist",
  "column_ref_commalist", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc",
  "opt_null_order", "first_last_desc", "sql_not", "manipulative_statement",
  "select_statement", "union_op", "commit_statement",
  "delete_statement_searched", "insert_statement", "values_or_query_spec",
  "row_value_constructor_commalist", "row_value_constructor",
  "row_value_constructor_elem", "rollback_statement",
  "select_statement_into", "opt_all_distinct", "assignment_commalist",
  "assignment", "update_source", "update_statement_searched",
  "target_commalist", "target", "opt_where_clause",
  "single_select_statement", "selection", "opt_limit_offset_clause",
  "opt_offset", "limit_offset_clause", "table_exp", "from_clause",
  "table_ref_commalist", "opt_as", "table_primary_as_range_column",
  "opt_disqualify_polymorphic_constraint", "opt_only",
  "opt_disqualify_primary_join", "table_ref", "where_clause",
  "opt_group_by_clause", "opt_having_clause", "truth_value",
  "boolean_primary", "boolean_test", "boolean_factor", "boolean_term",
  "search_condition", "type_predicate", "type_list", "type_list_item",
  "predicate", "comparison_predicate_part_2", "comparison_predicate",
  "comparison", "between_predicate_part_2", "between_predicate",
  "character_like_predicate_part_2", "other_like_predicate_part_2",
  "like_predicate", "opt_escape", "null_predicate_part_2", "test_for_null",
  "in_predicate_value", "in_predicate_part_2", "in_predicate",
  "quantified_comparison_predicate_part_2", "all_or_any_predicate",
  "rtreematch_predicate", "rtreematch_predicate_part_2", "any_all_some",
  "existence_test", "unique_test", "subquery", "scalar_exp_commalist",
  "select_sublist", "parameter_ref", "literal", "as_clause",
  "unsigned_value_spec", "general_value_spec", "iif_spec", "fct_spec",
  "function_name", "general_set_fct", "set_fct_type", "outer_join_type",
  "join_condition", "join_spec", "join_type", "cross_union",
  "qualified_join", "window_function", "window_function_type",
  "ntile_function", "opt_lead_or_lag_function", "lead_or_lag_function",
  "lead_or_lag", "lead_or_lag_extent", "first_or_last_value_function",
  "first_or_last_value", "nth_value_function", "opt_filter_clause",
  "window_name", "window_name_or_specification",
  "in_line_window_specification", "opt_window_clause",
  "window_definition_list", "window_definition", "new_window_name",
  "window_specification", "opt_existing_window_name",
  "existing_window_name", "opt_window_partition_clause",
  "opt_window_frame_clause", "window_partition_column_reference_list",
  "window_partition_column_reference", "opt_window_frame_exclusion",
  "window_frame_units", "window_frame_extent", "window_frame_start",
  "window_frame_preceding", "window_frame_between", "window_frame_bound",
  "window_frame_bound_1", "window_frame_bound_2", "window_frame_following",
  "rank_function_type", "opt_collate_clause", "collating_function",
  "ecrelationship_join", "op_relationship_direction", "joined_table",
  "named_columns_join", "all", "scalar_subquery", "cast_operand",
  "cast_target_primitive_type", "cast_target_scalar", "cast_target_array",
  "cast_target", "cast_spec", "opt_extract_value", "value_exp_primary",
  "num_primary", "factor", "term", "term_add_sub", "num_value_exp",
  "datetime_primary", "datetime_value_fct", "datetime_factor",
  "datetime_term", "datetime_value_exp", "value_exp_commalist",
  "function_arg", "function_args_commalist", "value_exp",
  "string_value_exp", "char_value_exp", "concatenation", "char_primary",
  "char_factor", "derived_column", "table_node",
  "tablespace_qualified_class_name", "qualified_class_name", "class_name",
  "table_node_ref", "table_node_with_opt_member_func_call",
  "table_node_path", "table_node_path_entry", "opt_member_function_args",
  "opt_column_array_idx", "property_path", "property_path_entry",
  "column_ref", "column", "case_expression", "case_specification",
  "simple_case", "searched_case", "simple_when_clause_list",
  "simple_when_clause", "when_operand_list", "when_operand",
  "searched_when_clause_list", "searched_when_clause", "else_clause",
  "result", "result_expression", "case_operand", "parameter",
  "range_variable", "opt_ecsqloptions_clause", "ecsqloptions_clause",
  "ecsqloptions_list", "ecsqloption", "ecsqloptionvalue", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-622)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-455)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     404,    62,  -622,    47,    57,  -622,    66,    26,   122,   125,
     164,   167,   180,  -622,  -622,  -622,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,   237,  -622,   134,    26,   185,  -622,  -622,
    2569,   196,    53,    28,   216,   531,  -622,  -622,  -622,  -622,
    2703,    36,  -622,  -622,  -622,  -622,  -622,  -622,   231,   259,
    -622,    67,   521,    99,   284,  -622,  -622,  1765,   279,  -622,
    -622,  -622,  -622,  -622,    78,  -622,  -622,   309,   314,  -622,
     322,   326,  -622,  -622,   328,  -622,  -622,  -622,  -622,  2971,
    -622,  -622,  -622,  -622,  2301,  -622,  -622,  2703,  2703,  2703,
     337,   341,  -622,  -622,  -622,  -622,   348,  -622,  -622,  -622,
    -622,   351,  2971,  2971,   289,   357,   152,  -622,   395,  -622,
      48,  -622,  -622,  -622,  -622,   408,    25,   417,  -622,   280,
    -622,  -622,   429,  -622,   431,  -622,   443,  -622,  -622,  -622,
    -622,  -622,   209,    64,   211,  -622,  -622,  -622,  -622,  -622,
      32,  -622,   272,  -622,  -622,  -622,  -622,    27,  -622,  -622,
    -622,  -622,  -622,  -622,  -622,  -622,  -622,    20,  -622,   216,
     195,   448,   195,   255,  -622,   413,  -622,  -622,  -622,  -622,
     282,    40,   393,   415,  -622,   316,  -622,  -622,   312,    70,
      70,   379,  -622,  -622,  -622,    20,   457,   134,    66,  -622,
     976,   380,  -622,   463,   469,    40,   407,   492,    19,  -622,
    -622,  -622,  2703,    44,    66,    66,   976,  -622,   976,  -622,
     235,  -622,   401,   312,  -622,  -622,  -622,   497,  2703,  2703,
      66,  -622,  -622,  1899,    26,   240,  -622,   420,  2703,  -622,
    -622,  -622,  -622,  1477,    66,   500,   397,  2703,  2703,   505,
    2837,  2837,  2837,  2837,  2837,  2837,  2837,  2837,  2837,  -622,
     490,  2703,  -622,  -622,   402,    40,    40,  -622,   195,  -622,
     496,   284,  2703,  -622,   498,  -622,   216,   216,    26,   459,
     499,    60,  -622,   359,  -622,    26,  -622,  2703,  -622,  -622,
    -622,  -622,  -622,  -622,  -622,   522,  -622,   501,   380,  -622,
    -622,   353,  -622,  2569,   642,   809,   524,   507,   524,  -622,
    -622,  -622,  -622,  -622,  -622,   243,    92,   471,  -622,  -622,
     383,   382,  -622,  -622,  2703,  -622,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,  -622,  -622,  -622,   387,   472,  2996,
      74,  3013,   513,  -622,  -622,  -622,  -622,   295,  -622,  -622,
     358,  -622,  -622,  -622,  -622,   527,   312,   534,  2703,  2703,
    2703,    30,   -51,  2703,  -622,   462,   976,   461,  -622,   401,
    -622,   557,   312,  -622,  -622,   559,  2703,  2703,   561,   562,
     403,    88,  -622,  -622,  -622,   514,  -622,  -622,  2703,  -622,
     367,  2703,   420,   168,   565,  -622,   567,  -622,  -622,  -622,
    -622,   209,   209,    64,    64,    64,    64,  -622,  -622,  -622,
    -622,   563,  -622,  -622,  -622,   370,   569,  -622,  -622,   403,
      26,    40,   380,  2703,   447,  -622,  -622,  -622,   324,  -622,
     544,   566,   540,    45,    16,  -622,  -622,  -622,   525,  -622,
     595,  2703,    68,  2435,  -622,  -622,  -622,  -622,  -622,  -622,
    -622,   507,   976,   976,  -622,   437,   513,  -622,   463,  -622,
      40,  3015,  -622,   598,   381,   392,  2703,  2703,  -622,  -622,
     169,    46,  -622,  2703,  -622,   599,   600,   602,    55,  -622,
     506,  -622,  -622,  2703,   422,   609,  2703,    26,   240,  -622,
     580,   564,   612,  -622,  2703,   613,   614,   596,  -622,  -622,
    -622,  -622,  -622,  2703,   618,  -622,  -622,   490,  -622,   403,
    -622,  -622,  -622,   312,   976,   198,  -622,  -622,  -622,   601,
     619,   621,  -622,  -622,  -622,  1765,  -622,  -622,   249,    -5,
    2703,   286,  -622,  -622,  -622,  -622,   524,   304,  -622,   383,
     433,  -622,  -622,   633,  -622,  -622,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,
     622,  -622,   645,  -622,  -622,  -622,   646,  -622,    69,  1310,
    2703,  -622,   648,  -622,  2703,  -622,   649,   403,  -622,  2703,
     976,   487,  -622,  -622,  -622,  -622,  -622,   516,  -622,   651,
    -622,   424,  -622,   382,   490,   335,   195,  -622,  -622,    20,
     434,   312,  2703,  -622,  -622,   504,  -622,  -622,  -622,  -622,
      37,  -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,    48,
    -622,   634,  -622,  -622,  2703,   248,  -622,  2703,  -622,  -622,
    -622,  -622,  -622,  -622,   655,  -622,   662,   382,   596,   588,
     635,   588,  2703,  -622,   490,   436,  -622,  -622,  -622,  -622,
     664,  -622,  2703,   510,  2703,  -622,   185,   444,  -622,  -622,
     667,  2703,  -622,  -622,  -622,   668,  -622,   643,   644,   578,
      40,   158,  -622,  -622,  -622,  -622,   312,  -622,  -622,  -622,
      37,  -622,   596,   677,  1143,  2837,   380,  -622,   679,  -622,
     535,  -622,  -622,  -622,   681,  1631,  -622,  -622,  -622,   682,
    -622,    54,   113,    87,  -622,    40,   110,  -622,  -622,  2033,
     551,   546,   560,  -622,  -622,   -26,  1143,  -622,  -622,   653,
     653,  2837,  -622,  -622,  -622,  -622,  -622,  -622,   556,   552,
    -622,  -622,   542,  -622,   -29,  -622,  -622,   151,  -622,  -622,
    -622,   426,  -622,  -622,   211,  -622,  -622,  2167,  -622,  -622,
    -622,  -622,   568,   554,  -622,  -622,  -622,   570,  -622,  -622,
    -622,  -622,  -622
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,    69,     0,     0,    77,    79,   110,     0,     0,
       0,     2,     4,    63,     6,    62,    56,    57,    58,    93,
      59,    60,    61,    64,    26,     0,   110,     0,    80,    81,
       0,   111,     0,   115,     0,   247,   254,   335,   334,   120,
       0,    10,     1,     3,     5,    68,    66,    67,   337,     0,
      31,     0,    90,     0,    39,   417,   416,     0,     0,   464,
     205,   202,   203,   204,   430,   238,   235,     0,     0,   217,
       0,     0,   216,   240,     0,   239,   236,   218,   436,     0,
     391,   393,   392,   237,     0,   201,   413,     0,     0,     0,
       0,     0,   268,   269,   272,   273,     0,   321,   322,   323,
     324,     0,     0,     0,   435,     0,   101,   339,    95,   197,
     214,   365,   213,   221,   366,     0,   220,     0,   370,   275,
     259,   260,     0,   261,     0,   262,     0,   368,   372,   373,
     374,   378,   382,   385,   405,   397,   390,   398,   399,   407,
     210,   406,   408,   410,   414,   409,   199,   363,   432,   367,
     369,   439,   440,   441,   215,   114,   113,     0,   116,     0,
     465,   428,   108,   423,   424,     0,   243,   248,   241,   242,
     247,     0,   249,     0,   435,     0,    73,    75,    76,     0,
       0,     8,    11,    12,   338,     0,     0,     0,    79,    32,
      54,   467,    91,     0,     0,     0,     0,     0,     0,   463,
     431,   434,     0,    79,    79,    79,    54,   377,    54,   462,
     458,   455,     0,     0,   394,   396,   395,     0,     0,     0,
      79,   376,   375,     0,   110,     0,    92,    90,     0,   208,
     209,   207,   206,     0,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   438,
       0,     0,   415,   212,     0,     0,     0,   437,   108,   107,
       0,    39,     0,   427,     0,   118,     0,     0,   110,     0,
     430,    90,    82,     0,   250,   110,    72,     0,    15,    16,
      17,    20,    21,    19,    18,     0,    13,     0,   467,    65,
      28,     0,    30,     0,    54,    54,     0,    54,     0,   166,
     167,   163,   162,   165,   164,     0,    54,   138,   140,   142,
     144,   121,   131,   150,     0,   151,   175,   176,   157,   181,
     155,   186,   156,   152,   158,   153,   154,   132,   133,   135,
     136,   134,     0,    70,   468,   421,   420,   421,   418,   419,
       0,    36,    71,   196,   371,     0,   340,     0,     0,     0,
       0,     0,     0,     0,   456,     0,    54,   458,   444,     0,
     257,     0,   461,   402,   460,     0,     0,     0,     0,   103,
     104,   101,    87,    89,   200,   122,   198,   222,     0,   403,
       0,     0,     0,     0,   264,   270,     0,   256,   379,   380,
     381,   383,   384,   388,   389,   386,   387,   211,   412,   411,
     433,   364,   117,   466,   119,     0,    37,   426,   425,   251,
     110,     0,   467,     0,   247,    74,    14,    22,     9,     7,
       0,     0,   101,     0,    76,    55,   141,   194,   168,   195,
       0,     0,     0,     0,   170,   173,   174,   180,   185,   188,
     189,    54,    54,    54,   161,   472,   469,   471,     0,    40,
       0,     0,   229,     0,     0,     0,     0,     0,   459,   443,
       0,    75,   450,     0,   451,   175,   181,   186,     0,   447,
       0,   445,   263,     0,     0,     0,     0,   110,     0,    78,
       0,   124,     0,   223,     0,     0,     0,   288,   277,   278,
     255,   279,   280,     0,     0,   271,   429,     0,   109,   252,
      83,    86,    84,    85,    54,     0,   245,   253,   246,     0,
       0,     0,    27,   137,   179,     0,   184,   182,   177,   177,
       0,     0,   192,   191,   193,   160,     0,     0,   143,   145,
       0,   470,    35,   356,   347,   346,   348,   349,   343,   344,
     350,   345,   351,   355,   341,   342,   352,   353,   354,   357,
     360,   361,     0,   230,   226,   227,     0,   457,   161,    54,
       0,   442,     0,   233,     0,   231,     0,   105,    88,     0,
      54,   281,   224,   404,   228,   276,   290,   291,   289,   265,
     267,     0,    34,   244,     0,   331,   108,    24,    23,     0,
       0,   400,     0,   172,   171,     0,   225,   190,   220,   187,
     110,   127,   129,   126,   128,   139,   130,   475,   476,   474,
     473,     0,   359,   362,     0,     0,   449,     0,   453,   454,
     452,   448,   446,   274,     0,   232,   123,   125,     0,    41,
       0,    41,     0,    38,     0,     0,   332,   333,   330,   422,
       0,   183,     0,   178,     0,   111,     0,     0,   147,   358,
       0,     0,   159,   234,   286,   282,   284,     0,     0,    96,
       0,   293,   266,    33,   336,    29,   401,   169,   149,   146,
     110,   219,     0,     0,    54,     0,   467,    97,   292,   296,
     325,   304,   303,   305,     0,     0,   148,   283,   285,    42,
      43,    47,    47,    98,   102,     0,     0,   297,   287,     0,
       0,     0,   298,   306,   307,     0,    54,    48,    49,    50,
      50,     0,   100,   295,   327,   328,   329,   326,     0,     0,
     313,   316,     0,   315,     0,   308,   310,     0,   294,   309,
      44,     0,    46,    45,    99,   317,   314,     0,   320,   311,
     300,   301,     0,     0,    52,    53,    51,     0,   318,   312,
     302,   299,   319
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,   517,  -622,
    -622,  -622,   511,  -622,  -622,   116,  -622,  -622,   440,    73,
    -622,    -1,    14,     0,  -622,  -280,  -622,     4,  -622,  -622,
    -622,  -622,   518,  -622,   -38,   -83,  -622,  -622,   377,  -622,
     302,  -622,  -622,  -622,   238,  -127,  -622,   430,  -622,  -622,
    -622,   344,  -622,  -622,   579,  -243,  -622,  -517,   686,   -18,
     343,  -622,  -622,   197,  -622,   435,   296,   283,  -168,  -622,
    -622,    58,  -621,  -622,  -622,  -287,   438,  -622,  -285,   439,
    -622,   222,  -281,  -622,  -622,  -269,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,   -21,  -622,   512,  -622,   212,  -622,
    -172,  -622,  -622,  -176,  -622,   225,  -622,  -622,  -622,  -622,
     573,  -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,  -341,  -622,  -622,  -622,  -622,    75,
    -622,    76,  -622,  -622,  -622,  -622,  -622,    56,  -622,  -622,
    -622,  -622,  -622,  -622,    11,  -622,  -622,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,  -622,  -622,  -128,  -622,  -622,  -622,
    -622,  -622,  -622,  -622,   321,   221,   182,   226,   143,  -591,
    -622,  -622,  -622,  -622,  -622,   184,  -171,  -126,   -30,   -82,
    -622,  -622,  -622,   502,  -622,   108,  -622,   581,   571,  -622,
    -146,  -622,   206,  -622,  -622,   503,   519,  -162,  -137,  -622,
    -622,  -622,  -622,  -622,   396,  -622,   199,   548,  -169,   416,
    -329,  -622,  -622,  -209,  -622,  -277,  -622,  -622,   330,  -622
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,   288,   181,   182,   183,   285,   418,
      25,   291,    50,    51,    13,   581,   340,   498,   196,   659,
     689,   690,   709,   732,   746,   305,    14,   197,    48,    16,
      17,    18,    19,   175,   306,   177,    20,    21,    30,   271,
     272,   502,    22,   371,   372,   191,    23,   106,   676,   712,
     677,   226,   227,   369,   264,   265,    32,    33,    34,    35,
     192,   481,   571,   605,   307,   308,   309,   310,   352,   606,
     647,   648,   312,   462,   313,   314,   464,   315,   316,   317,
     318,   593,   319,   320,   516,   321,   322,   439,   323,   324,
     440,   526,   325,   326,   107,   108,   109,   373,   110,   252,
     111,   112,   113,   114,   115,   116,   117,   172,   506,   507,
     173,    36,    37,   118,   119,   120,   494,   121,   122,   384,
     123,   124,   125,   236,   654,   490,   491,   629,   655,   656,
     657,   492,   577,   578,   631,   684,   678,   679,   728,   685,
     702,   703,   720,   704,   721,   722,   749,   723,   126,   697,
     717,    38,   638,    39,   508,   185,   127,   345,   549,   550,
     551,   552,   128,   257,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   590,   379,   380,   362,   141,
     142,   143,   144,   145,   146,    54,    55,    56,   339,   585,
     162,   163,   164,   263,   201,   147,   148,   149,   582,   150,
     151,   152,   153,   357,   358,   468,   469,   210,   211,   355,
     363,   364,   212,   154,   261,   333,   334,   446,   447,   610
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     140,   209,   176,   253,    15,   214,   215,   216,    52,   273,
     178,   419,   160,   258,   328,   402,   374,   428,   327,   433,
     344,   435,   311,   344,   458,   437,   432,   198,   331,  -112,
     328,   157,   328,   341,   327,   456,   327,   438,   351,   179,
     255,   354,   489,   592,   331,   457,   331,   361,   365,   513,
    -112,  -449,   368,   692,   178,   189,   249,   213,   213,   213,
     559,  -112,   329,   250,   270,   411,   385,   386,   229,   463,
     230,   465,   187,    28,  -159,   466,   460,   -75,   329,   -76,
     329,  -225,   155,   646,   693,   692,   707,   467,   -75,    29,
      24,   278,   279,   478,   280,    28,   188,   -76,   708,    26,
     375,   443,   520,   200,   738,   193,   739,     8,   -76,   729,
     -75,    29,   194,   397,   -75,   425,    27,   -75,   328,   328,
     734,   281,   327,   327,   430,    40,   423,   556,   557,    78,
     -76,   431,   331,   331,   -76,   501,   405,   -76,   251,   282,
     224,   251,  -449,   188,   412,   707,   576,  -258,   190,    41,
     283,   560,   297,   646,     8,   156,   231,   708,    49,   -75,
     330,   527,  -406,   232,    42,  -159,   329,   329,  -136,  -136,
     -76,   487,   346,   521,    43,  -258,   330,   453,   330,   -76,
     328,   256,   443,   251,   327,   284,   251,    44,   711,   289,
     354,    31,   488,   158,   331,   474,   475,   443,   140,   251,
     180,   584,   645,   520,   224,   740,   370,   482,   174,    53,
     485,   225,   347,   -75,   -75,   -75,   -75,   -75,   -75,  -106,
    -116,   398,   161,   454,   455,   430,   259,   714,   329,   243,
     244,   622,   431,   -76,   -76,   -76,   -76,   -76,   -76,   415,
     161,   251,   245,   246,   247,   248,    58,   178,    59,   273,
     409,   299,   300,   301,   302,   303,   304,   414,   715,   716,
     184,   266,   186,   140,   424,   330,   328,   328,   267,   374,
     327,   327,   617,   461,   618,   427,   444,   429,   619,   615,
     331,   331,   520,   741,   178,   650,    45,   195,   532,   -94,
     620,   742,   743,   -94,   681,   682,   -94,   592,    46,   430,
     207,   193,   562,   199,   430,   566,   431,   600,   448,   683,
     596,   651,   202,   573,   329,   329,    65,   203,    47,    66,
     276,   277,   579,   221,   222,   204,   330,    68,   328,   205,
     509,   206,   327,   208,   353,   166,   583,   510,   -94,   167,
     217,   -94,   331,   639,   218,   597,   168,   169,   -94,   519,
     -94,   219,    70,    71,   220,   601,    73,   420,   421,   586,
     223,    74,   449,   450,    75,    76,   245,   246,   247,   248,
     -94,   483,   484,   602,   496,   484,   329,   240,   241,   242,
      83,   636,   637,   503,   603,   554,   484,   604,   393,   394,
     395,   396,   499,   624,   328,   525,   555,   484,   327,   694,
     228,   213,   627,   178,  -373,  -373,  -373,  -373,   331,   517,
    -365,   233,   330,   330,  -373,  -373,  -373,  -373,  -373,  -373,
     234,  -365,   388,   389,   390,   558,   563,   564,   633,   634,
     235,     1,   237,   178,   238,   101,   600,   254,   641,   642,
     664,   634,   329,  -365,     2,   165,   239,     3,   669,   670,
    -365,   262,    60,    61,    62,    63,   166,   607,   105,   567,
     167,   662,     4,   744,   745,  -247,   274,   168,   169,   391,
     392,   170,   407,   408,   330,   268,   616,   275,     5,   251,
       6,   290,   595,   287,   601,   591,   332,   335,     7,   165,
     178,     8,  -365,   337,     8,  -366,   343,   663,   680,   356,
     166,   360,   602,   382,   167,   599,  -366,     9,   190,   387,
     643,   168,   169,   603,   249,   170,   604,    86,   504,   383,
     403,   410,   406,   413,   200,   417,   416,   157,  -366,   178,
     425,   441,   505,   680,   443,  -366,   442,   445,   452,   591,
     330,    85,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,   451,   459,
     353,   472,   213,   165,   473,   293,   476,   477,   480,   519,
     493,   495,   497,   165,   166,   511,   255,  -366,   167,   652,
     348,   349,   350,  -247,   166,   168,   169,   178,   167,   170,
     512,   691,   224,   640,   514,   168,   169,   366,   515,   170,
     367,   530,   553,   561,  -453,  -454,   667,  -452,   171,   190,
     378,   381,   666,   565,   178,   569,   572,   574,   575,   570,
     488,   213,   580,   691,   589,   587,   628,  -366,  -366,  -366,
    -366,  -366,  -366,  -366,  -366,  -366,  -366,  -366,  -366,  -366,
    -366,  -366,  -366,   588,   178,   294,   611,   612,    58,   613,
      59,   614,   623,   625,   630,   705,   632,   644,   649,   653,
     658,    60,    61,    62,    63,   295,    64,   642,   665,   724,
     660,   671,    65,   672,   673,    66,   178,  -406,   675,   674,
     487,    67,   696,    68,   695,   698,   725,   706,   726,   731,
     727,   735,   296,    69,   736,   737,   751,   286,   292,   750,
     635,   404,   297,   752,   661,   730,   710,   724,    70,    71,
     733,    72,    73,   500,   342,   479,   568,    74,   188,   159,
      75,    76,    77,   422,   298,   486,   529,   608,   686,     8,
     426,    78,    79,    80,    81,    82,    83,    84,   528,   260,
     376,   594,   609,   269,   434,   436,   598,   687,   748,   688,
      85,   713,   518,   626,   668,   471,   399,    86,   621,   401,
     359,    87,    88,    89,   336,    90,    91,    92,    93,    94,
      95,    96,     0,   470,   400,   338,   531,     0,     0,     0,
       0,     0,     0,     0,     0,    97,    98,    99,   100,     0,
       0,   101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   299,   300,   301,   302,   303,   304,   102,   103,     0,
     174,     0,   294,     0,   105,    58,     0,    59,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,    63,   425,    64,     0,     0,     0,     0,     0,    65,
       0,     0,    66,   -55,     0,     0,     0,     0,    67,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   296,
      69,     0,     0,     0,     0,     0,     0,     0,     0,   297,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
       0,     0,     0,     0,    74,     0,     0,    75,    76,    77,
       0,   298,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,    82,    83,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,    87,    88,
      89,     0,    90,    91,    92,    93,    94,    95,    96,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,     0,     0,   101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   299,   300,
     301,   302,   303,   304,   102,   103,     0,   174,     0,   294,
       0,   105,    58,     0,    59,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,    63,   295,
      64,     0,     0,     0,     0,     0,    65,     0,     0,    66,
       0,     0,     0,     0,     0,    67,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   296,    69,     0,     0,
       0,     0,     0,     0,     0,     0,   297,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,     0,     0,     0,
       0,    74,     0,     0,    75,    76,    77,     0,   298,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,    82,
      83,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,     0,     0,
       0,    86,     0,     0,     0,    87,    88,    89,     0,    90,
      91,    92,    93,    94,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,     0,     0,   101,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   299,   300,   301,   302,   303,
     304,   102,   103,     0,   174,     0,    57,     0,   105,    58,
       0,    59,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,    63,   425,    64,     0,     0,
       0,     0,     0,    65,     0,     0,    66,     0,     0,     0,
       0,     0,    67,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   296,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   297,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,     0,     0,     0,     0,    74,     0,
       0,    75,    76,    77,     0,   298,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    87,    88,    89,     0,    90,    91,    92,    93,
      94,    95,    96,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,    98,    99,   100,
       0,     0,   101,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   299,   300,   301,   302,   303,   304,   102,   103,
       0,   174,     0,    57,     0,   105,    58,     0,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,    63,   425,    64,     0,     0,     0,     0,     0,
      65,     0,     0,    66,     0,     0,     0,     0,     0,    67,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
     297,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,     0,     0,     0,     0,    74,     0,     0,    75,    76,
      77,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,     0,     0,     0,    86,     0,     0,     0,    87,
      88,    89,     0,    90,    91,    92,    93,    94,    95,    96,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,    98,    99,   100,     0,     0,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   299,
     300,   301,   302,   303,   304,   102,   103,     0,   174,     0,
      57,   377,   105,    58,     0,    59,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,    63,
       0,    64,     0,     0,     0,     0,    28,    65,     0,     0,
      66,     0,     0,     0,     0,     0,    67,     0,    68,     0,
       0,     0,    29,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,     0,     0,
       0,     0,    74,     0,     0,    75,    76,    77,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
      82,    83,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
       0,     0,    86,     0,     0,     0,    87,    88,    89,     0,
      90,    91,    92,    93,    94,    95,    96,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,    98,    99,   100,     0,     0,   101,     0,     0,     0,
       0,     0,     0,     0,    57,     0,     0,    58,     0,    59,
       0,     0,   102,   103,     0,   174,     0,     0,     0,   105,
      60,    61,    62,    63,     0,    64,     0,     0,     0,     0,
       0,    65,     0,     0,    66,   699,     0,     0,     0,     0,
      67,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,     0,     0,     0,     0,    74,     0,     0,    75,
      76,    77,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,    82,    83,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
      87,    88,    89,     0,    90,    91,    92,    93,    94,    95,
      96,     0,     0,     0,     0,   700,     0,     0,    57,     0,
       0,    58,   701,    59,    97,    98,    99,   100,     0,     0,
     101,     0,     0,     0,    60,    61,    62,    63,     0,    64,
       0,     0,     0,     0,     0,    65,   102,   103,    66,   174,
       0,     0,     0,   105,    67,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,     0,     0,     0,     0,
      74,   188,     0,    75,    76,    77,     0,     0,     0,     0,
       0,     0,     8,     0,    78,    79,    80,    81,    82,    83,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,     0,     0,
      86,     0,     0,     0,    87,    88,    89,     0,    90,    91,
      92,    93,    94,    95,    96,     0,     0,     0,     0,     0,
       0,     0,    57,     0,     0,    58,     0,    59,    97,    98,
      99,   100,     0,     0,   101,     0,     0,     0,    60,    61,
      62,    63,     0,    64,     0,     0,     0,     0,    28,    65,
     102,   103,    66,   174,     0,     0,     0,   105,    67,     0,
      68,     0,     0,     0,    29,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
       0,     0,     0,     0,    74,     0,     0,    75,    76,    77,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,    82,    83,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,    87,    88,
      89,     0,    90,    91,    92,    93,    94,    95,    96,     0,
       0,     0,     0,     0,     0,     0,    57,     0,     0,    58,
       0,    59,    97,    98,    99,   100,     0,     0,   101,     0,
       0,     0,    60,    61,    62,    63,     0,    64,     0,     0,
       0,     0,     0,    65,   102,   103,    66,   174,     0,     0,
       0,   105,    67,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,     0,     0,     0,     0,    74,     0,
       0,    75,    76,    77,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    87,    88,    89,     0,    90,    91,    92,    93,
      94,    95,    96,     0,     0,     0,     0,   718,     0,     0,
      57,     0,     0,    58,   719,    59,    97,    98,    99,   100,
       0,     0,   101,     0,     0,     0,    60,    61,    62,    63,
       0,    64,     0,     0,     0,     0,     0,    65,   102,   103,
      66,   174,     0,     0,     0,   105,    67,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,     0,     0,
       0,     0,    74,     0,     0,    75,    76,    77,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
      82,    83,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
       0,     0,    86,     0,     0,     0,    87,    88,    89,     0,
      90,    91,    92,    93,    94,    95,    96,     0,     0,     0,
       0,   747,     0,     0,    57,     0,     0,    58,   719,    59,
      97,    98,    99,   100,     0,     0,   101,     0,     0,     0,
      60,    61,    62,    63,     0,    64,     0,     0,     0,     0,
       0,    65,   102,   103,    66,   174,     0,     0,     0,   105,
      67,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,     0,     0,     0,     0,    74,     0,     0,    75,
      76,    77,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,    82,    83,    84,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
      87,    88,    89,     0,    90,    91,    92,    93,    94,    95,
      96,     0,     0,     0,     0,     0,     0,     0,    57,     0,
       0,    58,     0,    59,    97,    98,    99,   100,     0,     0,
     101,     0,     0,     0,    60,    61,    62,    63,     0,    64,
       0,     0,     0,     0,   522,   523,   102,   103,    66,   174,
       0,     0,     0,   105,    67,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,     0,     0,     0,     0,
      74,     0,     0,   524,    76,    77,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,    82,    83,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,     0,     0,     0,     0,     0,     0,
      86,     0,     0,     0,    87,    88,    89,     0,    90,    91,
      92,    93,    94,    95,    96,     0,     0,     0,     0,     0,
       0,     0,    57,     0,     0,    58,     0,    59,    97,    98,
      99,   100,     0,     0,   101,     0,     0,     0,    60,    61,
      62,    63,     0,    64,     0,     0,     0,     0,     0,    65,
     102,   103,    66,   174,     0,     0,     0,   105,    67,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
       0,     0,     0,     0,    74,     0,     0,    75,    76,    77,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,    82,    83,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,    87,    88,
      89,     0,    90,    91,    92,    93,    94,    95,    96,     0,
       0,     0,     0,     0,     0,     0,    57,     0,     0,    58,
       0,    59,    97,    98,    99,   100,     0,     0,   101,     0,
       0,     0,    60,    61,    62,    63,     0,    64,     0,     0,
       0,     0,     0,    65,   102,   103,    66,   104,     0,     0,
       0,   105,    67,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,     0,     0,     0,     0,    74,     0,
       0,    75,    76,    77,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    87,    88,    89,     0,    90,    91,    92,    93,
      94,    95,    96,     0,     0,     0,     0,     0,     0,     0,
      57,     0,     0,    58,     0,    59,    97,    98,    99,   100,
       0,     0,   101,     0,     0,     0,    60,    61,    62,    63,
       0,    64,     0,     0,     0,     0,     0,    65,   102,   103,
      66,   174,     0,     0,     0,   105,    67,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,     0,     0,
       0,     0,    74,     0,     0,    75,    76,    77,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,     0,     0,
       0,    83,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      90,    91,    92,    93,    94,    95,    96,     0,     0,     0,
       0,     0,     0,     0,    57,     0,     0,    58,     0,    59,
      97,    98,    99,   100,     0,     0,   101,     0,     0,     0,
      60,    61,    62,    63,     0,    64,     0,     0,     0,     0,
       0,    65,   102,   103,    66,   174,     0,     0,     0,   105,
      67,     0,    68,     0,     0,     0,     0,     0,     0,  -368,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
    -368,     0,     0,     0,     0,     0,  -367,    70,    71,   533,
      72,    73,     0,     0,     0,     0,    74,  -367,     0,    75,
      76,    77,  -368,     0,     0,     0,     0,     0,     0,  -368,
      78,     0,     0,     0,     0,    83,    84,     0,     0,  -367,
       0,     0,     0,     0,     0,     0,  -367,     0,     0,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    90,    91,    92,    93,    94,    95,
      96,  -368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    98,    99,   100,  -367,     0,
     101,     0,   534,   535,   536,   537,   538,   539,   540,   541,
     542,   543,   544,   545,   546,   547,   548,     0,     0,   174,
       0,     0,     0,   105,     0,     0,     0,     0,     0,     0,
       0,  -368,  -368,  -368,  -368,  -368,  -368,  -368,  -368,  -368,
    -368,  -368,  -368,  -368,  -368,  -368,  -368,     0,  -367,  -367,
    -367,  -367,  -367,  -367,  -367,  -367,  -367,  -367,  -367,  -367,
    -367,  -367,  -367,  -367
};

static const yytype_int16 yycheck[] =
{
      30,    84,    40,   140,     0,    87,    88,    89,    26,   171,
      40,   288,    33,   159,   190,   258,   225,   297,   190,   306,
       4,   306,   190,     4,   353,   306,   306,    57,   190,     3,
     206,     3,   208,   195,   206,     5,   208,   306,   206,     3,
      13,   210,   383,    48,   206,    96,   208,   218,   219,     4,
      24,     5,   223,   674,    84,    51,    24,    87,    88,    89,
       5,    24,   190,    31,    24,     5,   237,   238,    20,   356,
      22,   356,     5,    29,     5,   356,   356,    23,   206,     5,
     208,     3,    29,   600,   675,   706,    32,   356,    34,    45,
      28,    21,    22,     5,    24,    29,    76,    23,    44,    52,
     227,   152,    34,    25,   133,     6,   135,    87,    34,   135,
      56,    45,    13,   250,    60,    23,    59,    63,   294,   295,
     711,    51,   294,   295,    56,     3,   294,   456,   457,    89,
      56,    63,   294,   295,    60,   412,   262,    63,   167,    69,
      52,   167,    96,    76,   271,    32,   487,   122,    88,    24,
      80,    96,    60,   670,    87,   102,   108,    44,    24,   105,
     190,   441,   167,   115,     0,    96,   294,   295,   152,   153,
      96,     3,   202,   105,     7,   150,   206,   348,   208,   105,
     356,   154,   152,   167,   356,   115,   167,     7,   101,   185,
     359,   165,    24,   165,   356,   366,   367,   152,   228,   167,
     164,     3,   165,    34,    52,    54,   224,   378,   168,    24,
     381,    59,   168,   159,   160,   161,   162,   163,   164,    24,
      24,   251,    24,   349,   350,    56,    31,   117,   356,   165,
     166,   560,    63,   159,   160,   161,   162,   163,   164,   277,
      24,   167,   155,   156,   157,   158,     6,   277,     8,   411,
     268,   159,   160,   161,   162,   163,   164,   275,   148,   149,
      29,     6,     3,   293,   294,   295,   442,   443,    13,   478,
     442,   443,   559,   356,   559,   296,   314,   298,   559,   559,
     442,   443,    34,   132,   314,   614,    49,     3,   450,     0,
     559,   140,   141,     4,   136,   137,     7,    48,    61,    56,
      79,     6,   473,    24,    56,   476,    63,     3,    13,   151,
      24,    63,     3,   484,   442,   443,    30,     3,    81,    33,
       4,     5,   493,   102,   103,     3,   356,    41,   504,     3,
       6,     3,   504,    98,    99,    53,   504,    13,    49,    57,
       3,    52,   504,   586,     3,   521,    64,    65,    59,   431,
      61,     3,    66,    67,     3,    51,    70,     4,     5,   505,
       3,    75,     4,     5,    78,    79,   155,   156,   157,   158,
      81,     4,     5,    69,     4,     5,   504,   168,   169,   170,
      94,    46,    47,   413,    80,     4,     5,    83,   245,   246,
     247,   248,   410,   564,   570,   433,     4,     5,   570,   676,
       5,   431,   570,   433,   155,   156,   157,   158,   570,   430,
      23,     3,   442,   443,   165,   166,   167,   168,   169,   170,
       3,    34,   240,   241,   242,   463,     4,     5,     4,     5,
     150,    27,     3,   463,     3,   149,     3,   165,     4,     5,
       4,     5,   570,    56,    40,    42,     3,    43,     4,     5,
      63,     3,    19,    20,    21,    22,    53,    24,   172,   477,
      57,   632,    58,    37,    38,    62,    73,    64,    65,   243,
     244,    68,   266,   267,   504,    62,   559,    62,    74,   167,
      76,    24,   520,   104,    51,   515,   106,    24,    84,    42,
     520,    87,   105,    24,    87,    23,     4,   634,   660,    98,
      53,     4,    69,     3,    57,   526,    34,   103,    88,     4,
     592,    64,    65,    80,    24,    68,    83,   115,    71,   122,
      24,    62,    24,   164,    25,    24,     4,     3,    56,   559,
      23,    60,    85,   695,   152,    63,   153,    24,     4,   569,
     570,   108,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,    31,    97,
      99,     4,   592,    42,     5,   188,     5,     5,    54,   651,
       5,     4,     3,    42,    53,    31,    13,   105,    57,   617,
     203,   204,   205,    62,    53,    64,    65,   617,    57,    68,
      24,   674,    52,   589,    69,    64,    65,   220,     3,    68,
     223,   164,     4,    97,     5,     5,   644,     5,    77,    88,
     233,   234,   642,     4,   644,    35,     4,     4,     4,    55,
      24,   651,     4,   706,     3,    24,   139,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,    24,   674,     3,    13,    25,     6,     4,
       8,     5,     4,     4,   138,   685,     5,   153,    24,     4,
      72,    19,    20,    21,    22,    23,    24,     5,     4,   699,
      35,     4,    30,     5,    31,    33,   706,   167,   100,    35,
       3,    39,   147,    41,     5,     4,   135,     5,   142,    36,
     130,   135,    50,    51,   142,   153,   142,   180,   187,   131,
     584,   261,    60,   133,   631,   706,   692,   737,    66,    67,
     710,    69,    70,   411,   196,   371,   478,    75,    76,    33,
      78,    79,    80,   293,    82,   382,   443,   530,   670,    87,
     295,    89,    90,    91,    92,    93,    94,    95,   442,   160,
     228,   519,   530,   170,   306,   306,   521,   672,   737,   673,
     108,   695,   431,   569,   646,   359,   254,   115,   559,   256,
     212,   119,   120,   121,   193,   123,   124,   125,   126,   127,
     128,   129,    -1,   357,   255,   194,   446,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,
      -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,   160,   161,   162,   163,   164,   165,   166,    -1,
     168,    -1,     3,    -1,   172,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    39,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,    -1,    -1,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,   160,
     161,   162,   163,   164,   165,   166,    -1,   168,    -1,     3,
      -1,   172,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,    -1,
      -1,    75,    -1,    -1,    78,    79,    80,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,    -1,    -1,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,   160,   161,   162,   163,
     164,   165,   166,    -1,   168,    -1,     3,    -1,   172,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,    -1,   119,   120,   121,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
      -1,    -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,   160,   161,   162,   163,   164,   165,   166,
      -1,   168,    -1,     3,    -1,   172,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,
      70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,
     120,   121,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,    -1,    -1,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
     160,   161,   162,   163,   164,   165,   166,    -1,   168,    -1,
       3,     4,   172,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    29,    30,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,    -1,    -1,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,   165,   166,    -1,   168,    -1,    -1,    -1,   172,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,
      39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,   134,    -1,    -1,     3,    -1,
      -1,     6,   141,     8,   143,   144,   145,   146,    -1,    -1,
     149,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    30,   165,   166,    33,   168,
      -1,    -1,    -1,   172,    39,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    -1,    -1,    -1,    -1,
      75,    76,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,   120,   121,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,   143,   144,
     145,   146,    -1,    -1,   149,    -1,    -1,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,
     165,   166,    33,   168,    -1,    -1,    -1,   172,    39,    -1,
      41,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,   143,   144,   145,   146,    -1,    -1,   149,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,   165,   166,    33,   168,    -1,    -1,
      -1,   172,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,    -1,   119,   120,   121,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,   134,    -1,    -1,
       3,    -1,    -1,     6,   141,     8,   143,   144,   145,   146,
      -1,    -1,   149,    -1,    -1,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,   165,   166,
      33,   168,    -1,    -1,    -1,   172,    39,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,   134,    -1,    -1,     3,    -1,    -1,     6,   141,     8,
     143,   144,   145,   146,    -1,    -1,   149,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,   165,   166,    33,   168,    -1,    -1,    -1,   172,
      39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,
      -1,     6,    -1,     8,   143,   144,   145,   146,    -1,    -1,
     149,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    29,    30,   165,   166,    33,   168,
      -1,    -1,    -1,   172,    39,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,   120,   121,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,    -1,     6,    -1,     8,   143,   144,
     145,   146,    -1,    -1,   149,    -1,    -1,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
     165,   166,    33,   168,    -1,    -1,    -1,   172,    39,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,   143,   144,   145,   146,    -1,    -1,   149,    -1,
      -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,   165,   166,    33,   168,    -1,    -1,
      -1,   172,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,    -1,   119,   120,   121,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,   143,   144,   145,   146,
      -1,    -1,   149,    -1,    -1,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,   165,   166,
      33,   168,    -1,    -1,    -1,   172,    39,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
     143,   144,   145,   146,    -1,    -1,   149,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,   165,   166,    33,   168,    -1,    -1,    -1,   172,
      39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    -1,    -1,    23,    66,    67,    24,
      69,    70,    -1,    -1,    -1,    -1,    75,    34,    -1,    78,
      79,    80,    56,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      89,    -1,    -1,    -1,    -1,    94,    95,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,   105,    -1,
     149,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,   168,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,    -1,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    40,    43,    58,    74,    76,    84,    87,   103,
     175,   176,   177,   188,   200,   201,   203,   204,   205,   206,
     210,   211,   216,   220,    28,   184,    52,    59,    29,    45,
     212,   165,   230,   231,   232,   233,   285,   286,   325,   327,
       3,    24,     0,     7,     7,    49,    61,    81,   202,    24,
     186,   187,   233,    24,   359,   360,   361,     3,     6,     8,
      19,    20,    21,    22,    24,    30,    33,    39,    41,    51,
      66,    67,    69,    70,    75,    78,    79,    80,    89,    90,
      91,    92,    93,    94,    95,   108,   115,   119,   120,   121,
     123,   124,   125,   126,   127,   128,   129,   143,   144,   145,
     146,   149,   165,   166,   168,   172,   221,   268,   269,   270,
     272,   274,   275,   276,   277,   278,   279,   280,   287,   288,
     289,   291,   292,   294,   295,   296,   322,   330,   336,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     352,   353,   354,   355,   356,   357,   358,   369,   370,   371,
     373,   374,   375,   376,   387,    29,   102,     3,   165,   232,
     268,    24,   364,   365,   366,    42,    53,    57,    64,    65,
      68,    77,   281,   284,   168,   207,   208,   209,   352,     3,
     164,   179,   180,   181,    29,   329,     3,     5,    76,   201,
      88,   219,   234,     6,    13,     3,   192,   201,   352,    24,
      25,   368,     3,     3,     3,     3,     3,   339,    98,   209,
     381,   382,   386,   352,   353,   353,   353,     3,     3,     3,
       3,   339,   339,     3,    52,    59,   225,   226,     5,    20,
      22,   108,   115,     3,     3,   150,   297,     3,     3,     3,
     168,   169,   170,   165,   166,   155,   156,   157,   158,    24,
      31,   167,   273,   372,   165,    13,   154,   337,   364,    31,
     228,   388,     3,   367,   228,   229,     6,    13,    62,   284,
      24,   213,   214,   371,    73,    62,     4,     5,    21,    22,
      24,    51,    69,    80,   115,   182,   182,   104,   178,   201,
      24,   185,   186,   212,     3,    23,    50,    60,    82,   159,
     160,   161,   162,   163,   164,   199,   208,   238,   239,   240,
     241,   242,   246,   248,   249,   251,   252,   253,   254,   256,
     257,   259,   260,   262,   263,   266,   267,   274,   277,   330,
     352,   371,   106,   389,   390,    24,   362,    24,   361,   362,
     190,   371,   206,     4,     4,   331,   352,   168,   212,   212,
     212,   242,   242,    99,   382,   383,    98,   377,   378,   381,
       4,   350,   352,   384,   385,   350,   212,   212,   350,   227,
     233,   217,   218,   271,   387,   219,   270,     4,   212,   350,
     351,   212,     3,   122,   293,   350,   350,     4,   340,   340,
     340,   341,   341,   342,   342,   342,   342,   372,   352,   357,
     370,   369,   229,    24,   192,   351,    24,   366,   366,   233,
      62,     5,   219,   164,   233,   208,     4,    24,   183,   389,
       4,     5,   221,   242,   352,    23,   239,   268,   199,   268,
      56,    63,   199,   249,   250,   252,   253,   256,   259,   261,
     264,    60,   153,   152,   208,    24,   391,   392,    13,     4,
       5,    31,     4,   350,   351,   351,     5,    96,   384,    97,
     199,   209,   247,   249,   250,   252,   256,   259,   379,   380,
     383,   378,     4,     5,   350,   350,     5,     5,     5,   225,
      54,   235,   350,     4,     5,   350,   234,     3,    24,   298,
     299,   300,   305,     5,   290,     4,     4,     3,   191,   233,
     214,   389,   215,   352,    71,    85,   282,   283,   328,     6,
      13,    31,    24,     4,    69,     3,   258,   268,   338,   353,
      34,   105,    29,    30,    78,   208,   265,   199,   240,   241,
     164,   392,   371,    24,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   332,
     333,   334,   335,     4,     4,     4,   384,   384,   208,     5,
      96,    97,   350,     4,     5,     4,   350,   233,   218,    35,
      55,   236,     4,   350,     4,     4,   298,   306,   307,   350,
       4,   189,   372,   242,     3,   363,   364,    24,    24,     3,
     349,   352,    48,   255,   255,   208,    24,   277,   279,   268,
       3,    51,    69,    80,    83,   237,   243,    24,   237,   272,
     393,    13,    25,     4,     5,   199,   209,   249,   252,   256,
     259,   380,   384,     4,   350,     4,   349,   242,   139,   301,
     138,   308,     5,     4,     5,   189,    46,    47,   326,   229,
     201,     4,     5,   353,   153,   165,   231,   244,   245,    24,
     384,    63,   208,     4,   298,   302,   303,   304,    72,   193,
      35,   193,   350,   372,     4,     4,   352,   208,   359,     4,
       5,     4,     5,    31,    35,   100,   222,   224,   310,   311,
     371,   136,   137,   151,   309,   313,   245,   303,   305,   194,
     195,   209,   246,   343,   389,     5,   147,   323,     4,    34,
     134,   141,   314,   315,   317,   352,     5,    32,    44,   196,
     196,   101,   223,   311,   117,   148,   149,   324,   134,   141,
     316,   318,   319,   321,   352,   135,   142,   130,   312,   135,
     195,    36,   197,   197,   343,   135,   142,   153,   133,   135,
      54,   132,   140,   141,    37,    38,   198,   134,   318,   320,
     131,   142,   133
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   174,   175,   175,   175,   175,   176,   177,   178,   178,
     179,   179,   179,   180,   181,   182,   182,   182,   182,   182,
     182,   182,   183,   183,   183,   184,   184,   185,   185,   186,
     187,   187,   188,   189,   189,   190,   190,   191,   191,   192,
     192,   193,   193,   194,   194,   195,   195,   196,   196,   196,
     197,   197,   198,   198,   199,   199,   200,   200,   200,   200,
     200,   200,   200,   200,   201,   201,   202,   202,   202,   203,
     204,   205,   206,   207,   207,   208,   209,   210,   211,   212,
     212,   212,   213,   213,   214,   215,   216,   217,   217,   218,
     219,   219,   220,   220,   221,   221,   222,   222,   223,   223,
     224,   225,   225,   226,   227,   227,   228,   228,   229,   229,
     230,   230,   231,   231,   231,   232,   232,   233,   233,   233,
     233,   234,   235,   235,   236,   236,   237,   237,   237,   237,
     237,   238,   238,   238,   238,   238,   238,   238,   239,   239,
     240,   240,   241,   241,   242,   242,   243,   244,   244,   245,
     246,   246,   246,   246,   246,   246,   246,   246,   246,   247,
     248,   248,   249,   249,   249,   249,   249,   249,   249,   250,
     251,   252,   253,   254,   254,   254,   254,   255,   255,   256,
     257,   257,   258,   258,   259,   260,   260,   261,   262,   263,
     264,   265,   265,   265,   266,   267,   268,   269,   269,   270,
     271,   272,   272,   272,   272,   272,   272,   272,   272,   272,
     273,   273,   273,   274,   274,   275,   275,   275,   275,   276,
     277,   277,   277,   277,   277,   278,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   280,   280,   280,   280,   280,
     280,   281,   281,   281,   282,   283,   283,   284,   284,   284,
     284,   285,   286,   286,   286,   287,   288,   288,   288,   288,
     288,   288,   288,   289,   290,   290,   290,   291,   292,   292,
     293,   294,   295,   295,   296,   297,   297,   298,   299,   299,
     300,   301,   301,   302,   302,   303,   304,   305,   306,   306,
     307,   308,   308,   309,   309,   310,   310,   311,   312,   312,
     312,   312,   312,   313,   313,   313,   314,   314,   315,   315,
     315,   316,   317,   318,   318,   318,   319,   319,   320,   320,
     321,   322,   322,   322,   322,   323,   323,   324,   324,   324,
     325,   326,   326,   326,   327,   327,   328,   329,   329,   330,
     331,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   333,   333,   334,
     335,   335,   336,   337,   337,   338,   338,   338,   338,   338,
     338,   338,   338,   339,   340,   340,   340,   340,   341,   341,
     341,   341,   342,   342,   342,   343,   343,   343,   343,   343,
     344,   345,   345,   345,   345,   345,   345,   346,   347,   348,
     349,   349,   350,   351,   351,   352,   352,   352,   353,   354,
     354,   355,   355,   356,   357,   358,   359,   359,   360,   361,
     361,   362,   363,   364,   365,   365,   365,   366,   367,   367,
     368,   368,   369,   369,   370,   370,   370,   371,   372,   373,
     374,   374,   375,   376,   377,   377,   378,   379,   379,   380,
     380,   380,   380,   380,   380,   381,   381,   382,   383,   383,
     384,   385,   386,   387,   387,   388,   388,   389,   389,   390,
     391,   391,   392,   392,   393,   393,   393
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     5,     0,     2,
       0,     1,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     0,     1,     3,     1,     8,
       3,     1,     4,     3,     1,     3,     1,     0,     3,     0,
       3,     0,     3,     1,     3,     3,     3,     0,     1,     1,
       0,     2,     1,     1,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     1,     1,     1,
       5,     5,     4,     1,     3,     1,     1,     1,     6,     0,
       1,     1,     1,     3,     3,     1,     6,     1,     3,     1,
       0,     1,     4,     1,     1,     1,     0,     1,     0,     2,
       3,     0,     8,     2,     1,     3,     0,     1,     0,     3,
       0,     1,     0,     2,     2,     0,     1,     4,     3,     4,
       1,     2,     0,     3,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     4,
       1,     2,     1,     3,     1,     3,     3,     1,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     2,     1,     1,     1,     1,     1,     1,     2,     5,
       2,     4,     4,     2,     2,     1,     1,     0,     2,     3,
       2,     1,     1,     3,     3,     2,     1,     3,     2,     2,
       3,     1,     1,     1,     2,     2,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     8,
       1,     1,     3,     4,     5,     1,     5,     5,     5,     4,
       5,     5,     6,     5,     7,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     0,     1,     1,
       2,     4,     5,     5,     1,     4,     3,     3,     1,     1,
       1,     1,     1,     4,     0,     2,     4,     5,     1,     1,
       1,     4,     1,     1,     6,     0,     4,     1,     1,     1,
       1,     0,     2,     3,     1,     3,     1,     6,     0,     1,
       1,     0,     3,     0,     3,     3,     1,     2,     0,     3,
       2,     2,     3,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     4,     1,     2,     1,     1,     2,     1,     2,
       2,     1,     1,     1,     1,     0,     2,     1,     1,     1,
       7,     0,     1,     1,     1,     1,     4,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     6,     0,     2,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     2,     2,     2,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     1,     1,     1,     2,     2,     2,     1,     1,     1,
       1,     3,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     2,     1,     1,     3,     3,
       3,     1,     2,     1,     1,     3,     3,     2,     0,     3,
       0,     1,     1,     3,     2,     1,     1,     2,     1,     1,
       1,     1,     5,     4,     1,     2,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     4,     0,     2,
       1,     1,     1,     2,     1,     0,     2,     0,     1,     2,
       2,     1,     1,     3,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, context); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, connectivity::OSQLParser* context)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (context);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, connectivity::OSQLParser* context)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, context);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, connectivity::OSQLParser* context)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, context); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, connectivity::OSQLParser* context)
{
  YY_USE (yyvaluep);
  YY_USE (context);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (connectivity::OSQLParser* context)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* sql_single_statement: sql  */
                    { context->setParseTree( (yyvsp[0].pParseNode) ); }
    break;

  case 3: /* sql_single_statement: sql ';'  */
                    { context->setParseTree( (yyvsp[-1].pParseNode) ); }
    break;

  case 4: /* sql_single_statement: pragma  */
                    { context->setParseTree( (yyvsp[0].pParseNode) ); }
    break;

  case 5: /* sql_single_statement: pragma ';'  */
                    { context->setParseTree( (yyvsp[-1].pParseNode) ); }
    break;

  case 7: /* pragma: SQL_TOKEN_PRAGMA SQL_TOKEN_NAME opt_pragma_set opt_pragma_for opt_ecsqloptions_clause  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 8: /* opt_pragma_for: %empty  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 9: /* opt_pragma_for: SQL_TOKEN_FOR pragma_path  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 10: /* opt_pragma_set: %empty  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
    }
    break;

  case 13: /* opt_pragma_set_val: SQL_EQUAL pragma_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 14: /* opt_pragma_func: '(' pragma_value ')'  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    }
    break;

  case 22: /* pragma_path: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 23: /* pragma_path: pragma_path '.' SQL_TOKEN_NAME  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 24: /* pragma_path: pragma_path ':' SQL_TOKEN_NAME  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 25: /* opt_cte_recursive: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 27: /* cte_column_list: cte_column_list ',' SQL_TOKEN_NAME  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 28: /* cte_column_list: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 29: /* cte_table_name: SQL_TOKEN_NAME '(' cte_column_list ')' SQL_TOKEN_AS '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 30: /* cte_block_list: cte_block_list ',' cte_table_name  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 31: /* cte_block_list: cte_table_name  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 32: /* cte: SQL_TOKEN_WITH opt_cte_recursive cte_block_list select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 33: /* column_commalist: column_commalist ',' column  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 34: /* column_commalist: column  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 35: /* column_ref_commalist: column_ref_commalist ',' column_ref  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 36: /* column_ref_commalist: column_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 37: /* opt_column_commalist: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 38: /* opt_column_commalist: '(' column_commalist ')'  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 39: /* opt_column_ref_commalist: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 40: /* opt_column_ref_commalist: '(' column_ref_commalist ')'  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 41: /* opt_order_by_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 42: /* opt_order_by_clause: SQL_TOKEN_ORDER SQL_TOKEN_BY ordering_spec_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 43: /* ordering_spec_commalist: ordering_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 44: /* ordering_spec_commalist: ordering_spec_commalist ',' ordering_spec  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 45: /* ordering_spec: predicate opt_asc_desc opt_null_order  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 46: /* ordering_spec: row_value_constructor_elem opt_asc_desc opt_null_order  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));

        }
    break;

  case 47: /* opt_asc_desc: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 50: /* opt_null_order: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 51: /* opt_null_order: SQL_TOKEN_NULLS first_last_desc  */
         {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
         }
    break;

  case 54: /* sql_not: %empty  */
    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 64: /* select_statement: single_select_statement  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 65: /* select_statement: single_select_statement union_op all select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 69: /* commit_statement: SQL_TOKEN_COMMIT  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 70: /* delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM table_ref opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 71: /* insert_statement: SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_ref_commalist values_or_query_spec  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 72: /* values_or_query_spec: SQL_TOKEN_VALUES '(' row_value_constructor_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 73: /* row_value_constructor_commalist: row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 74: /* row_value_constructor_commalist: row_value_constructor_commalist ',' row_value_constructor  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 77: /* rollback_statement: SQL_TOKEN_ROLLBACK  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 78: /* select_statement_into: SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_INTO target_commalist table_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode)); }
    break;

  case 79: /* opt_all_distinct: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 82: /* assignment_commalist: assignment  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 83: /* assignment_commalist: assignment_commalist ',' assignment  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 84: /* assignment: column_ref SQL_EQUAL update_source  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 86: /* update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 87: /* target_commalist: target  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 88: /* target_commalist: target_commalist ',' target  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 90: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 92: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 94: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 96: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 98: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 99: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 100: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 101: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 102: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 103: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 104: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 105: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 106: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 108: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 109: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 110: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 111: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 112: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 113: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 114: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 115: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 116: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 117: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 118: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 119: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 121: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 122: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 123: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 124: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 125: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 137: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 139: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 141: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 143: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 145: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 146: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 147: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 148: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 149: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 159: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 160: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 161: /* comparison_predicate: comparison row_value_constructor  */
        {
            if(context->inPredicateCheck()) // comparison_predicate: rule 2
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                sal_Int16 nErg = context->buildPredicateRule((yyval.pParseNode),(yyvsp[0].pParseNode),(yyvsp[-1].pParseNode));
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

  case 168: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 169: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 170: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 171: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 172: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 173: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 174: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 175: /* like_predicate: character_like_predicate_part_2  */
        {
            if (context->inPredicateCheck())  // like_predicate: rule 5
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[0].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[0].pParseNode)->removeAt(2);
                if ( !context->buildLikeRule((yyvsp[0].pParseNode),p2nd,p3rd) )
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
                (yyvsp[0].pParseNode)->append(p3rd);
            }
            else
                YYERROR;
        }
    break;

  case 176: /* like_predicate: other_like_predicate_part_2  */
        {
            if (context->inPredicateCheck()) // like_predicate: rule 6
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                OSQLParseNode* p2nd = (yyvsp[0].pParseNode)->removeAt(2);
                OSQLParseNode* p3rd = (yyvsp[0].pParseNode)->removeAt(2);
                if ( !context->buildLikeRule((yyvsp[0].pParseNode),p2nd,p3rd) )
                {
                    delete (yyval.pParseNode);
                    YYABORT;
                }
                (yyvsp[0].pParseNode)->append(p3rd);
            }
            else
                YYERROR;
        }
    break;

  case 177: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 178: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 179: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 180: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 181: /* test_for_null: null_predicate_part_2  */
        {
            if (context->inPredicateCheck())// test_for_null: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 182: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 183: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 184: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 185: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 186: /* in_predicate: in_predicate_part_2  */
        {
            if ( context->inPredicateCheck() )// in_predicate: rule 2
            {
                OSQLParseNode* pColumnRef = CREATE_NODE(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
                pColumnRef->append(CREATE_NODE(context->getFieldName(),SQL_NODE_NAME));

                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append(pColumnRef);
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
            else
                YYERROR;
        }
    break;

  case 187: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 188: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 189: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 190: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 194: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 195: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 196: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 197: /* scalar_exp_commalist: select_sublist  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 198: /* scalar_exp_commalist: scalar_exp_commalist ',' select_sublist  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 206: /* literal: literal SQL_TOKEN_STRING  */
        {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 207: /* literal: literal SQL_TOKEN_INT  */
        {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 208: /* literal: literal SQL_TOKEN_REAL_NUM  */
        {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 209: /* literal: literal SQL_TOKEN_APPROXNUM  */
        {
            if (context->inPredicateCheck())
            {
                (yyval.pParseNode) = SQL_NEW_RULE;
                (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                (yyval.pParseNode)->append((yyvsp[0].pParseNode));
                context->reduceLiteral((yyval.pParseNode), sal_True);
            }
            else
                YYERROR;
        }
    break;

  case 210: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 211: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 219: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));


        }
    break;

  case 222: /* fct_spec: function_name '(' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 223: /* fct_spec: function_name '(' function_args_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 224: /* fct_spec: function_name '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 226: /* general_set_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
        {
            if((yyvsp[-1].pParseNode)->count() != 1)
                {
                SQLyyerror(context, "Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])");
                YYERROR;
                }

            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = (yyvsp[-1].pParseNode)->getChild(0));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));

        }
    break;

  case 227: /* general_set_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
        {
            if((yyvsp[-1].pParseNode)->count() != 1)
                {
                SQLyyerror(context, "Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])");
                YYERROR;
                }

            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = (yyvsp[-1].pParseNode)->getChild(0));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));

        }
    break;

  case 228: /* general_set_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 229: /* general_set_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 230: /* general_set_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 231: /* general_set_fct: SQL_TOKEN_GROUP_CONCAT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 232: /* general_set_fct: SQL_TOKEN_GROUP_CONCAT '(' function_arg ',' function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 233: /* general_set_fct: SQL_TOKEN_RTRIM '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 234: /* general_set_fct: SQL_TOKEN_RTRIM '(' opt_all_distinct function_arg ',' function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 241: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 242: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 243: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 244: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 247: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 248: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 250: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 251: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 252: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 253: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 255: /* window_function: window_function_type opt_filter_clause SQL_TOKEN_OVER window_name_or_specification  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 256: /* window_function_type: rank_function_type '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 257: /* window_function_type: SQL_TOKEN_ROW_NUMBER '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 263: /* ntile_function: SQL_TOKEN_NTILE '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 264: /* opt_lead_or_lag_function: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 265: /* opt_lead_or_lag_function: ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 266: /* opt_lead_or_lag_function: ',' function_arg ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 267: /* lead_or_lag_function: lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 271: /* first_or_last_value_function: first_or_last_value '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 274: /* nth_value_function: SQL_TOKEN_NTH_VALUE '(' function_arg ',' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-5].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 275: /* opt_filter_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 276: /* opt_filter_clause: SQL_TOKEN_FILTER '(' where_clause ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 281: /* opt_window_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 282: /* opt_window_clause: SQL_TOKEN_WINDOW window_definition_list  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 283: /* window_definition_list: window_definition_list ',' window_definition  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 284: /* window_definition_list: window_definition  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 285: /* window_definition: new_window_name SQL_TOKEN_AS window_specification  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 287: /* window_specification: '(' opt_existing_window_name opt_window_partition_clause opt_order_by_clause opt_window_frame_clause ')'  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-5].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
		(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 288: /* opt_existing_window_name: %empty  */
                                 {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 291: /* opt_window_partition_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 292: /* opt_window_partition_clause: SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 293: /* opt_window_frame_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 294: /* opt_window_frame_clause: window_frame_units window_frame_extent opt_window_frame_exclusion  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 295: /* window_partition_column_reference_list: window_partition_column_reference_list ',' window_partition_column_reference  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 296: /* window_partition_column_reference_list: window_partition_column_reference  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 297: /* window_partition_column_reference: column_ref opt_collate_clause  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 298: /* opt_window_frame_exclusion: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 299: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 300: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 301: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 302: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 308: /* window_frame_start: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 309: /* window_frame_start: value_exp SQL_TOKEN_PRECEDING  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 310: /* window_frame_start: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 311: /* window_frame_preceding: value_exp SQL_TOKEN_PRECEDING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 312: /* window_frame_between: SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 314: /* window_frame_bound: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 317: /* window_frame_bound_1: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 319: /* window_frame_bound_2: SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 320: /* window_frame_following: value_exp SQL_TOKEN_FOLLOWING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 325: /* opt_collate_clause: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 326: /* opt_collate_clause: SQL_TOKEN_COLLATE collating_function  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 330: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 331: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 336: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 337: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 357: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 358: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 359: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 362: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
      {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 363: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 364: /* opt_extract_value: SQL_ARROW property_path  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));

        }
    break;

  case 371: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 375: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 376: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 377: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 380: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 388: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 389: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 390: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 392: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 393: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 394: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 395: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 396: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 397: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 398: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 399: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 400: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 401: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 403: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 404: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 411: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 412: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 415: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 416: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 417: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 418: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 419: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 420: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 421: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 422: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 423: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 424: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 425: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 426: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 427: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 428: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 429: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 430: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 431: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 432: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 433: /* property_path: property_path '.' property_path_entry  */
        {
			auto last = (yyvsp[-2].pParseNode)->getLast();
			if (last)
				{
				if (last->getFirst()->getNodeType() == SQL_NODE_PUNCTUATION) //'*'
					{
					SQLyyerror(context, "'*' can only occur at the end of property path\n");
					YYERROR;
					}
				}

            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 434: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 435: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 436: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 437: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 442: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 443: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 444: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 445: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 446: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 447: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 448: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 455: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 456: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 457: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 458: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 459: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 463: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 464: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 465: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 466: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 467: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 469: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 470: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 471: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 472: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 473: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (context, YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, context);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (context, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, context);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, context);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
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
    m_pParseTree = pNewParseTree->detach();
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
OSQLParseNode* OSQLParser::parseTree (Utf8String& rErrorMessage, Utf8String const& rStatement, sal_Bool bInternational) {
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

    return m_pParseTree;
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
