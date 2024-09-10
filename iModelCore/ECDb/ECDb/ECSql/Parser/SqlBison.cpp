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
  YYSYMBOL_SQL_TOKEN_COUNT = 40,           /* SQL_TOKEN_COUNT  */
  YYSYMBOL_SQL_TOKEN_CROSS = 41,           /* SQL_TOKEN_CROSS  */
  YYSYMBOL_SQL_TOKEN_DELETE = 42,          /* SQL_TOKEN_DELETE  */
  YYSYMBOL_SQL_TOKEN_DESC = 43,            /* SQL_TOKEN_DESC  */
  YYSYMBOL_SQL_TOKEN_DISTINCT = 44,        /* SQL_TOKEN_DISTINCT  */
  YYSYMBOL_SQL_TOKEN_FORWARD = 45,         /* SQL_TOKEN_FORWARD  */
  YYSYMBOL_SQL_TOKEN_BACKWARD = 46,        /* SQL_TOKEN_BACKWARD  */
  YYSYMBOL_SQL_TOKEN_ESCAPE = 47,          /* SQL_TOKEN_ESCAPE  */
  YYSYMBOL_SQL_TOKEN_EXCEPT = 48,          /* SQL_TOKEN_EXCEPT  */
  YYSYMBOL_SQL_TOKEN_EXISTS = 49,          /* SQL_TOKEN_EXISTS  */
  YYSYMBOL_SQL_TOKEN_FALSE = 50,           /* SQL_TOKEN_FALSE  */
  YYSYMBOL_SQL_TOKEN_FROM = 51,            /* SQL_TOKEN_FROM  */
  YYSYMBOL_SQL_TOKEN_FULL = 52,            /* SQL_TOKEN_FULL  */
  YYSYMBOL_SQL_TOKEN_GROUP = 53,           /* SQL_TOKEN_GROUP  */
  YYSYMBOL_SQL_TOKEN_HAVING = 54,          /* SQL_TOKEN_HAVING  */
  YYSYMBOL_SQL_TOKEN_IN = 55,              /* SQL_TOKEN_IN  */
  YYSYMBOL_SQL_TOKEN_INNER = 56,           /* SQL_TOKEN_INNER  */
  YYSYMBOL_SQL_TOKEN_INSERT = 57,          /* SQL_TOKEN_INSERT  */
  YYSYMBOL_SQL_TOKEN_INTO = 58,            /* SQL_TOKEN_INTO  */
  YYSYMBOL_SQL_TOKEN_IS = 59,              /* SQL_TOKEN_IS  */
  YYSYMBOL_SQL_TOKEN_INTERSECT = 60,       /* SQL_TOKEN_INTERSECT  */
  YYSYMBOL_SQL_TOKEN_JOIN = 61,            /* SQL_TOKEN_JOIN  */
  YYSYMBOL_SQL_TOKEN_LIKE = 62,            /* SQL_TOKEN_LIKE  */
  YYSYMBOL_SQL_TOKEN_LEFT = 63,            /* SQL_TOKEN_LEFT  */
  YYSYMBOL_SQL_TOKEN_RIGHT = 64,           /* SQL_TOKEN_RIGHT  */
  YYSYMBOL_SQL_TOKEN_MAX = 65,             /* SQL_TOKEN_MAX  */
  YYSYMBOL_SQL_TOKEN_MIN = 66,             /* SQL_TOKEN_MIN  */
  YYSYMBOL_SQL_TOKEN_NATURAL = 67,         /* SQL_TOKEN_NATURAL  */
  YYSYMBOL_SQL_TOKEN_NULL = 68,            /* SQL_TOKEN_NULL  */
  YYSYMBOL_SQL_TOKEN_TOTAL = 69,           /* SQL_TOKEN_TOTAL  */
  YYSYMBOL_SQL_TOKEN_ON = 70,              /* SQL_TOKEN_ON  */
  YYSYMBOL_SQL_TOKEN_ORDER = 71,           /* SQL_TOKEN_ORDER  */
  YYSYMBOL_SQL_TOKEN_OUTER = 72,           /* SQL_TOKEN_OUTER  */
  YYSYMBOL_SQL_TOKEN_IIF = 73,             /* SQL_TOKEN_IIF  */
  YYSYMBOL_SQL_TOKEN_SELECT = 74,          /* SQL_TOKEN_SELECT  */
  YYSYMBOL_SQL_TOKEN_SET = 75,             /* SQL_TOKEN_SET  */
  YYSYMBOL_SQL_TOKEN_SOME = 76,            /* SQL_TOKEN_SOME  */
  YYSYMBOL_SQL_TOKEN_SUM = 77,             /* SQL_TOKEN_SUM  */
  YYSYMBOL_SQL_TOKEN_TRUE = 78,            /* SQL_TOKEN_TRUE  */
  YYSYMBOL_SQL_TOKEN_UNION = 79,           /* SQL_TOKEN_UNION  */
  YYSYMBOL_SQL_TOKEN_UNIQUE = 80,          /* SQL_TOKEN_UNIQUE  */
  YYSYMBOL_SQL_TOKEN_UNKNOWN = 81,         /* SQL_TOKEN_UNKNOWN  */
  YYSYMBOL_SQL_TOKEN_UPDATE = 82,          /* SQL_TOKEN_UPDATE  */
  YYSYMBOL_SQL_TOKEN_USING = 83,           /* SQL_TOKEN_USING  */
  YYSYMBOL_SQL_TOKEN_VALUE = 84,           /* SQL_TOKEN_VALUE  */
  YYSYMBOL_SQL_TOKEN_VALUES = 85,          /* SQL_TOKEN_VALUES  */
  YYSYMBOL_SQL_TOKEN_WHERE = 86,           /* SQL_TOKEN_WHERE  */
  YYSYMBOL_SQL_TOKEN_DOLLAR = 87,          /* SQL_TOKEN_DOLLAR  */
  YYSYMBOL_SQL_BITWISE_NOT = 88,           /* SQL_BITWISE_NOT  */
  YYSYMBOL_SQL_TOKEN_NAVIGATION_VALUE = 89, /* SQL_TOKEN_NAVIGATION_VALUE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_DATE = 90,    /* SQL_TOKEN_CURRENT_DATE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIME = 91,    /* SQL_TOKEN_CURRENT_TIME  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIMESTAMP = 92, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_EVERY = 93,           /* SQL_TOKEN_EVERY  */
  YYSYMBOL_SQL_TOKEN_CASE = 94,            /* SQL_TOKEN_CASE  */
  YYSYMBOL_SQL_TOKEN_THEN = 95,            /* SQL_TOKEN_THEN  */
  YYSYMBOL_SQL_TOKEN_END = 96,             /* SQL_TOKEN_END  */
  YYSYMBOL_SQL_TOKEN_WHEN = 97,            /* SQL_TOKEN_WHEN  */
  YYSYMBOL_SQL_TOKEN_ELSE = 98,            /* SQL_TOKEN_ELSE  */
  YYSYMBOL_SQL_TOKEN_LIMIT = 99,           /* SQL_TOKEN_LIMIT  */
  YYSYMBOL_SQL_TOKEN_OFFSET = 100,         /* SQL_TOKEN_OFFSET  */
  YYSYMBOL_SQL_TOKEN_ONLY = 101,           /* SQL_TOKEN_ONLY  */
  YYSYMBOL_SQL_TOKEN_PRAGMA = 102,         /* SQL_TOKEN_PRAGMA  */
  YYSYMBOL_SQL_TOKEN_FOR = 103,            /* SQL_TOKEN_FOR  */
  YYSYMBOL_SQL_TOKEN_MATCH = 104,          /* SQL_TOKEN_MATCH  */
  YYSYMBOL_SQL_TOKEN_ECSQLOPTIONS = 105,   /* SQL_TOKEN_ECSQLOPTIONS  */
  YYSYMBOL_SQL_TOKEN_INTEGER = 106,        /* SQL_TOKEN_INTEGER  */
  YYSYMBOL_SQL_TOKEN_INT = 107,            /* SQL_TOKEN_INT  */
  YYSYMBOL_SQL_TOKEN_INT64 = 108,          /* SQL_TOKEN_INT64  */
  YYSYMBOL_SQL_TOKEN_LONG = 109,           /* SQL_TOKEN_LONG  */
  YYSYMBOL_SQL_TOKEN_BOOLEAN = 110,        /* SQL_TOKEN_BOOLEAN  */
  YYSYMBOL_SQL_TOKEN_DOUBLE = 111,         /* SQL_TOKEN_DOUBLE  */
  YYSYMBOL_SQL_TOKEN_REAL = 112,           /* SQL_TOKEN_REAL  */
  YYSYMBOL_SQL_TOKEN_FLOAT = 113,          /* SQL_TOKEN_FLOAT  */
  YYSYMBOL_SQL_TOKEN_STRING = 114,         /* SQL_TOKEN_STRING  */
  YYSYMBOL_SQL_TOKEN_VARCHAR = 115,        /* SQL_TOKEN_VARCHAR  */
  YYSYMBOL_SQL_TOKEN_BINARY = 116,         /* SQL_TOKEN_BINARY  */
  YYSYMBOL_SQL_TOKEN_BLOB = 117,           /* SQL_TOKEN_BLOB  */
  YYSYMBOL_SQL_TOKEN_DATE = 118,           /* SQL_TOKEN_DATE  */
  YYSYMBOL_SQL_TOKEN_TIME = 119,           /* SQL_TOKEN_TIME  */
  YYSYMBOL_SQL_TOKEN_TIMESTAMP = 120,      /* SQL_TOKEN_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_OVER = 121,           /* SQL_TOKEN_OVER  */
  YYSYMBOL_SQL_TOKEN_ROW_NUMBER = 122,     /* SQL_TOKEN_ROW_NUMBER  */
  YYSYMBOL_SQL_TOKEN_NTILE = 123,          /* SQL_TOKEN_NTILE  */
  YYSYMBOL_SQL_TOKEN_LEAD = 124,           /* SQL_TOKEN_LEAD  */
  YYSYMBOL_SQL_TOKEN_LAG = 125,            /* SQL_TOKEN_LAG  */
  YYSYMBOL_SQL_TOKEN_FIRST_VALUE = 126,    /* SQL_TOKEN_FIRST_VALUE  */
  YYSYMBOL_SQL_TOKEN_LAST_VALUE = 127,     /* SQL_TOKEN_LAST_VALUE  */
  YYSYMBOL_SQL_TOKEN_NTH_VALUE = 128,      /* SQL_TOKEN_NTH_VALUE  */
  YYSYMBOL_SQL_TOKEN_EXCLUDE = 129,        /* SQL_TOKEN_EXCLUDE  */
  YYSYMBOL_SQL_TOKEN_OTHERS = 130,         /* SQL_TOKEN_OTHERS  */
  YYSYMBOL_SQL_TOKEN_TIES = 131,           /* SQL_TOKEN_TIES  */
  YYSYMBOL_SQL_TOKEN_FOLLOWING = 132,      /* SQL_TOKEN_FOLLOWING  */
  YYSYMBOL_SQL_TOKEN_UNBOUNDED = 133,      /* SQL_TOKEN_UNBOUNDED  */
  YYSYMBOL_SQL_TOKEN_PRECEDING = 134,      /* SQL_TOKEN_PRECEDING  */
  YYSYMBOL_SQL_TOKEN_RANGE = 135,          /* SQL_TOKEN_RANGE  */
  YYSYMBOL_SQL_TOKEN_ROWS = 136,           /* SQL_TOKEN_ROWS  */
  YYSYMBOL_SQL_TOKEN_PARTITION = 137,      /* SQL_TOKEN_PARTITION  */
  YYSYMBOL_SQL_TOKEN_WINDOW = 138,         /* SQL_TOKEN_WINDOW  */
  YYSYMBOL_SQL_TOKEN_NO = 139,             /* SQL_TOKEN_NO  */
  YYSYMBOL_SQL_TOKEN_CURRENT = 140,        /* SQL_TOKEN_CURRENT  */
  YYSYMBOL_SQL_TOKEN_ROW = 141,            /* SQL_TOKEN_ROW  */
  YYSYMBOL_SQL_TOKEN_RANK = 142,           /* SQL_TOKEN_RANK  */
  YYSYMBOL_SQL_TOKEN_DENSE_RANK = 143,     /* SQL_TOKEN_DENSE_RANK  */
  YYSYMBOL_SQL_TOKEN_PERCENT_RANK = 144,   /* SQL_TOKEN_PERCENT_RANK  */
  YYSYMBOL_SQL_TOKEN_CUME_DIST = 145,      /* SQL_TOKEN_CUME_DIST  */
  YYSYMBOL_SQL_TOKEN_COLLATE = 146,        /* SQL_TOKEN_COLLATE  */
  YYSYMBOL_SQL_TOKEN_NOCASE = 147,         /* SQL_TOKEN_NOCASE  */
  YYSYMBOL_SQL_TOKEN_RTRIM = 148,          /* SQL_TOKEN_RTRIM  */
  YYSYMBOL_SQL_TOKEN_FILTER = 149,         /* SQL_TOKEN_FILTER  */
  YYSYMBOL_SQL_TOKEN_GROUPS = 150,         /* SQL_TOKEN_GROUPS  */
  YYSYMBOL_SQL_TOKEN_GROUP_CONCAT = 151,   /* SQL_TOKEN_GROUP_CONCAT  */
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
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 172,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 173,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 174,     /* sql_single_statement  */
  YYSYMBOL_sql = 175,                      /* sql  */
  YYSYMBOL_pragma = 176,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 177,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 178,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 179,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 180,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 181,             /* pragma_value  */
  YYSYMBOL_pragma_path = 182,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 183,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 184,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 185,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 186,           /* cte_block_list  */
  YYSYMBOL_cte = 187,                      /* cte  */
  YYSYMBOL_column_commalist = 188,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 189,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 190,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 191, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 192,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 193,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 194,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 195,             /* opt_asc_desc  */
  YYSYMBOL_opt_null_order = 196,           /* opt_null_order  */
  YYSYMBOL_first_last_desc = 197,          /* first_last_desc  */
  YYSYMBOL_sql_not = 198,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 199,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 200,         /* select_statement  */
  YYSYMBOL_union_op = 201,                 /* union_op  */
  YYSYMBOL_delete_statement_searched = 202, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 203,         /* insert_statement  */
  YYSYMBOL_values_commalist = 204,         /* values_commalist  */
  YYSYMBOL_values_or_query_spec = 205,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 206, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 207,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 208, /* row_value_constructor_elem  */
  YYSYMBOL_opt_all_distinct = 209,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 210,     /* assignment_commalist  */
  YYSYMBOL_assignment = 211,               /* assignment  */
  YYSYMBOL_update_source = 212,            /* update_source  */
  YYSYMBOL_update_statement_searched = 213, /* update_statement_searched  */
  YYSYMBOL_opt_where_clause = 214,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 215,  /* single_select_statement  */
  YYSYMBOL_selection = 216,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 217,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 218,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 219,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 220,                /* table_exp  */
  YYSYMBOL_from_clause = 221,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 222,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 223,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 224, /* table_primary_as_range_column  */
  YYSYMBOL_opt_only_all = 225,             /* opt_only_all  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 226, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 227,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 228, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 229,                /* table_ref  */
  YYSYMBOL_where_clause = 230,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 231,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 232,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 233,              /* truth_value  */
  YYSYMBOL_boolean_primary = 234,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 235,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 236,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 237,             /* boolean_term  */
  YYSYMBOL_search_condition = 238,         /* search_condition  */
  YYSYMBOL_type_predicate = 239,           /* type_predicate  */
  YYSYMBOL_type_list = 240,                /* type_list  */
  YYSYMBOL_type_list_item = 241,           /* type_list_item  */
  YYSYMBOL_predicate = 242,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 243, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 244,     /* comparison_predicate  */
  YYSYMBOL_comparison = 245,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 246, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 247,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 248, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 249, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 250,           /* like_predicate  */
  YYSYMBOL_opt_escape = 251,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 252,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 253,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 254,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 255,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 256,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 257, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 258,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 259,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 260, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 261,             /* any_all_some  */
  YYSYMBOL_existence_test = 262,           /* existence_test  */
  YYSYMBOL_unique_test = 263,              /* unique_test  */
  YYSYMBOL_subquery = 264,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 265,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 266,           /* select_sublist  */
  YYSYMBOL_literal = 267,                  /* literal  */
  YYSYMBOL_as_clause = 268,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 269,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 270,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 271,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 272,                 /* fct_spec  */
  YYSYMBOL_function_name = 273,            /* function_name  */
  YYSYMBOL_value_creation_fct = 274,       /* value_creation_fct  */
  YYSYMBOL_aggregate_fct = 275,            /* aggregate_fct  */
  YYSYMBOL_opt_function_arg = 276,         /* opt_function_arg  */
  YYSYMBOL_set_fct_type = 277,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 278,          /* outer_join_type  */
  YYSYMBOL_join_condition = 279,           /* join_condition  */
  YYSYMBOL_join_spec = 280,                /* join_spec  */
  YYSYMBOL_join_type = 281,                /* join_type  */
  YYSYMBOL_cross_union = 282,              /* cross_union  */
  YYSYMBOL_qualified_join = 283,           /* qualified_join  */
  YYSYMBOL_window_function = 284,          /* window_function  */
  YYSYMBOL_window_function_type = 285,     /* window_function_type  */
  YYSYMBOL_ntile_function = 286,           /* ntile_function  */
  YYSYMBOL_opt_lead_or_lag_function = 287, /* opt_lead_or_lag_function  */
  YYSYMBOL_lead_or_lag_function = 288,     /* lead_or_lag_function  */
  YYSYMBOL_lead_or_lag = 289,              /* lead_or_lag  */
  YYSYMBOL_lead_or_lag_extent = 290,       /* lead_or_lag_extent  */
  YYSYMBOL_first_or_last_value_function = 291, /* first_or_last_value_function  */
  YYSYMBOL_first_or_last_value = 292,      /* first_or_last_value  */
  YYSYMBOL_nth_value_function = 293,       /* nth_value_function  */
  YYSYMBOL_opt_filter_clause = 294,        /* opt_filter_clause  */
  YYSYMBOL_window_name = 295,              /* window_name  */
  YYSYMBOL_window_name_or_specification = 296, /* window_name_or_specification  */
  YYSYMBOL_in_line_window_specification = 297, /* in_line_window_specification  */
  YYSYMBOL_opt_window_clause = 298,        /* opt_window_clause  */
  YYSYMBOL_window_definition_list = 299,   /* window_definition_list  */
  YYSYMBOL_window_definition = 300,        /* window_definition  */
  YYSYMBOL_new_window_name = 301,          /* new_window_name  */
  YYSYMBOL_window_specification = 302,     /* window_specification  */
  YYSYMBOL_opt_existing_window_name = 303, /* opt_existing_window_name  */
  YYSYMBOL_existing_window_name = 304,     /* existing_window_name  */
  YYSYMBOL_opt_window_partition_clause = 305, /* opt_window_partition_clause  */
  YYSYMBOL_opt_window_frame_clause = 306,  /* opt_window_frame_clause  */
  YYSYMBOL_window_partition_column_reference_list = 307, /* window_partition_column_reference_list  */
  YYSYMBOL_window_partition_column_reference = 308, /* window_partition_column_reference  */
  YYSYMBOL_opt_window_frame_exclusion = 309, /* opt_window_frame_exclusion  */
  YYSYMBOL_window_frame_units = 310,       /* window_frame_units  */
  YYSYMBOL_window_frame_extent = 311,      /* window_frame_extent  */
  YYSYMBOL_window_frame_start = 312,       /* window_frame_start  */
  YYSYMBOL_window_frame_preceding = 313,   /* window_frame_preceding  */
  YYSYMBOL_window_frame_between = 314,     /* window_frame_between  */
  YYSYMBOL_window_frame_bound = 315,       /* window_frame_bound  */
  YYSYMBOL_window_frame_bound_1 = 316,     /* window_frame_bound_1  */
  YYSYMBOL_window_frame_bound_2 = 317,     /* window_frame_bound_2  */
  YYSYMBOL_window_frame_following = 318,   /* window_frame_following  */
  YYSYMBOL_rank_function_type = 319,       /* rank_function_type  */
  YYSYMBOL_opt_collate_clause = 320,       /* opt_collate_clause  */
  YYSYMBOL_collating_function = 321,       /* collating_function  */
  YYSYMBOL_ecrelationship_join = 322,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 323, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 324,             /* joined_table  */
  YYSYMBOL_named_columns_join = 325,       /* named_columns_join  */
  YYSYMBOL_all = 326,                      /* all  */
  YYSYMBOL_scalar_subquery = 327,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 328,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 329, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 330,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 331,        /* cast_target_array  */
  YYSYMBOL_cast_target = 332,              /* cast_target  */
  YYSYMBOL_cast_spec = 333,                /* cast_spec  */
  YYSYMBOL_opt_optional_prop = 334,        /* opt_optional_prop  */
  YYSYMBOL_opt_extract_value = 335,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 336,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 337,              /* num_primary  */
  YYSYMBOL_factor = 338,                   /* factor  */
  YYSYMBOL_term = 339,                     /* term  */
  YYSYMBOL_term_add_sub = 340,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 341,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 342,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 343,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 344,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 345,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 346,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 347,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 348,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 349,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 350,                /* value_exp  */
  YYSYMBOL_string_value_exp = 351,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 352,           /* char_value_exp  */
  YYSYMBOL_concatenation = 353,            /* concatenation  */
  YYSYMBOL_char_primary = 354,             /* char_primary  */
  YYSYMBOL_char_factor = 355,              /* char_factor  */
  YYSYMBOL_derived_column = 356,           /* derived_column  */
  YYSYMBOL_table_node = 357,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 358, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 359,     /* qualified_class_name  */
  YYSYMBOL_class_name = 360,               /* class_name  */
  YYSYMBOL_table_node_ref = 361,           /* table_node_ref  */
  YYSYMBOL_table_node_with_opt_member_func_call = 362, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 363,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 364,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 365, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 366,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 367,            /* property_path  */
  YYSYMBOL_property_path_entry = 368,      /* property_path_entry  */
  YYSYMBOL_column_ref = 369,               /* column_ref  */
  YYSYMBOL_column = 370,                   /* column  */
  YYSYMBOL_case_expression = 371,          /* case_expression  */
  YYSYMBOL_case_specification = 372,       /* case_specification  */
  YYSYMBOL_simple_case = 373,              /* simple_case  */
  YYSYMBOL_searched_case = 374,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 375,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 376,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 377,        /* when_operand_list  */
  YYSYMBOL_when_operand = 378,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 379, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 380,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 381,              /* else_clause  */
  YYSYMBOL_result = 382,                   /* result  */
  YYSYMBOL_result_expression = 383,        /* result_expression  */
  YYSYMBOL_case_operand = 384,             /* case_operand  */
  YYSYMBOL_parameter = 385,                /* parameter  */
  YYSYMBOL_range_variable = 386,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 387,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 388,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 389,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 390,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 391          /* ecsqloptionvalue  */
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
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3147

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  173
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  219
/* YYNRULES -- Number of rules.  */
#define YYNRULES  480
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  765

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   405


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
     161,   162,   163,   164,   167,   172
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   241,   241,   242,   243,   244,   248,   254,   266,   269,
     277,   280,   281,   285,   293,   301,   302,   303,   304,   305,
     306,   307,   311,   316,   321,   333,   336,   340,   345,   353,
     365,   376,   382,   390,   402,   407,   415,   420,   430,   431,
     439,   440,   451,   452,   462,   467,   475,   484,   495,   496,
     497,   501,   502,   511,   511,   515,   516,   522,   523,   524,
     525,   526,   530,   535,   546,   547,   548,   553,   566,   573,
     584,   591,   601,   611,   616,   624,   627,   631,   632,   633,
     637,   640,   646,   653,   656,   669,   670,   678,   686,   690,
     695,   698,   699,   702,   703,   711,   720,   721,   736,   746,
     749,   755,   756,   760,   763,   773,   774,   779,   786,   787,
     794,   795,   800,   808,   809,   816,   824,   832,   840,   844,
     853,   854,   864,   865,   873,   874,   875,   876,   877,   880,
     881,   882,   883,   884,   885,   886,   896,   897,   907,   908,
     916,   917,   926,   927,   937,   946,   951,   959,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   982,   989,   996,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1035,  1045,  1053,
    1063,  1073,  1079,  1085,  1107,  1132,  1133,  1140,  1149,  1155,
    1171,  1175,  1183,  1192,  1198,  1214,  1223,  1232,  1241,  1251,
    1252,  1253,  1257,  1265,  1271,  1279,  1286,  1297,  1302,  1309,
    1313,  1314,  1315,  1316,  1317,  1319,  1331,  1343,  1355,  1371,
    1372,  1378,  1382,  1383,  1386,  1387,  1388,  1389,  1392,  1404,
    1405,  1406,  1407,  1414,  1422,  1434,  1435,  1439,  1453,  1469,
    1485,  1494,  1502,  1511,  1529,  1530,  1539,  1540,  1541,  1542,
    1543,  1544,  1548,  1553,  1558,  1565,  1573,  1574,  1577,  1578,
    1583,  1584,  1592,  1604,  1614,  1623,  1627,  1638,  1645,  1652,
    1653,  1654,  1655,  1656,  1660,  1671,  1672,  1678,  1689,  1701,
    1702,  1706,  1711,  1722,  1723,  1727,  1741,  1742,  1752,  1756,
    1757,  1761,  1766,  1767,  1776,  1779,  1785,  1795,  1799,  1817,
    1818,  1822,  1827,  1828,  1839,  1840,  1850,  1853,  1859,  1869,
    1870,  1877,  1883,  1889,  1900,  1901,  1902,  1906,  1907,  1911,
    1917,  1924,  1933,  1942,  1953,  1954,  1960,  1964,  1965,  1974,
    1975,  1984,  1993,  1994,  1995,  1996,  2001,  2002,  2011,  2012,
    2013,  2017,  2031,  2032,  2033,  2036,  2037,  2040,  2052,  2053,
    2057,  2060,  2064,  2065,  2066,  2067,  2068,  2069,  2070,  2071,
    2072,  2073,  2074,  2075,  2076,  2077,  2078,  2079,  2083,  2088,
    2098,  2107,  2108,  2112,  2124,  2125,  2132,  2133,  2141,  2142,
    2143,  2144,  2145,  2146,  2147,  2154,  2158,  2162,  2163,  2169,
    2175,  2184,  2185,  2192,  2199,  2209,  2210,  2217,  2227,  2228,
    2235,  2242,  2249,  2259,  2266,  2271,  2276,  2281,  2287,  2293,
    2302,  2309,  2317,  2325,  2328,  2334,  2338,  2343,  2351,  2352,
    2353,  2357,  2361,  2362,  2365,  2372,  2382,  2385,  2389,  2395,
    2405,  2410,  2417,  2426,  2434,  2443,  2451,  2460,  2468,  2473,
    2478,  2486,  2495,  2496,  2506,  2507,  2515,  2520,  2538,  2544,
    2549,  2557,  2569,  2573,  2577,  2578,  2582,  2593,  2603,  2608,
    2615,  2625,  2628,  2633,  2634,  2635,  2636,  2637,  2638,  2641,
    2646,  2653,  2663,  2664,  2672,  2676,  2680,  2684,  2690,  2698,
    2701,  2711,  2712,  2716,  2725,  2730,  2738,  2744,  2754,  2755,
    2756
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
  "SQL_TOKEN_FIRST", "SQL_TOKEN_LAST", "SQL_TOKEN_CAST", "SQL_TOKEN_COUNT",
  "SQL_TOKEN_CROSS", "SQL_TOKEN_DELETE", "SQL_TOKEN_DESC",
  "SQL_TOKEN_DISTINCT", "SQL_TOKEN_FORWARD", "SQL_TOKEN_BACKWARD",
  "SQL_TOKEN_ESCAPE", "SQL_TOKEN_EXCEPT", "SQL_TOKEN_EXISTS",
  "SQL_TOKEN_FALSE", "SQL_TOKEN_FROM", "SQL_TOKEN_FULL", "SQL_TOKEN_GROUP",
  "SQL_TOKEN_HAVING", "SQL_TOKEN_IN", "SQL_TOKEN_INNER",
  "SQL_TOKEN_INSERT", "SQL_TOKEN_INTO", "SQL_TOKEN_IS",
  "SQL_TOKEN_INTERSECT", "SQL_TOKEN_JOIN", "SQL_TOKEN_LIKE",
  "SQL_TOKEN_LEFT", "SQL_TOKEN_RIGHT", "SQL_TOKEN_MAX", "SQL_TOKEN_MIN",
  "SQL_TOKEN_NATURAL", "SQL_TOKEN_NULL", "SQL_TOKEN_TOTAL", "SQL_TOKEN_ON",
  "SQL_TOKEN_ORDER", "SQL_TOKEN_OUTER", "SQL_TOKEN_IIF",
  "SQL_TOKEN_SELECT", "SQL_TOKEN_SET", "SQL_TOKEN_SOME", "SQL_TOKEN_SUM",
  "SQL_TOKEN_TRUE", "SQL_TOKEN_UNION", "SQL_TOKEN_UNIQUE",
  "SQL_TOKEN_UNKNOWN", "SQL_TOKEN_UPDATE", "SQL_TOKEN_USING",
  "SQL_TOKEN_VALUE", "SQL_TOKEN_VALUES", "SQL_TOKEN_WHERE",
  "SQL_TOKEN_DOLLAR", "SQL_BITWISE_NOT", "SQL_TOKEN_NAVIGATION_VALUE",
  "SQL_TOKEN_CURRENT_DATE", "SQL_TOKEN_CURRENT_TIME",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_EVERY", "SQL_TOKEN_CASE",
  "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE",
  "SQL_TOKEN_LIMIT", "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY",
  "SQL_TOKEN_PRAGMA", "SQL_TOKEN_FOR", "SQL_TOKEN_MATCH",
  "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT",
  "SQL_TOKEN_INT64", "SQL_TOKEN_LONG", "SQL_TOKEN_BOOLEAN",
  "SQL_TOKEN_DOUBLE", "SQL_TOKEN_REAL", "SQL_TOKEN_FLOAT",
  "SQL_TOKEN_STRING", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_DATE", "SQL_TOKEN_TIME",
  "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_OVER", "SQL_TOKEN_ROW_NUMBER",
  "SQL_TOKEN_NTILE", "SQL_TOKEN_LEAD", "SQL_TOKEN_LAG",
  "SQL_TOKEN_FIRST_VALUE", "SQL_TOKEN_LAST_VALUE", "SQL_TOKEN_NTH_VALUE",
  "SQL_TOKEN_EXCLUDE", "SQL_TOKEN_OTHERS", "SQL_TOKEN_TIES",
  "SQL_TOKEN_FOLLOWING", "SQL_TOKEN_UNBOUNDED", "SQL_TOKEN_PRECEDING",
  "SQL_TOKEN_RANGE", "SQL_TOKEN_ROWS", "SQL_TOKEN_PARTITION",
  "SQL_TOKEN_WINDOW", "SQL_TOKEN_NO", "SQL_TOKEN_CURRENT", "SQL_TOKEN_ROW",
  "SQL_TOKEN_RANK", "SQL_TOKEN_DENSE_RANK", "SQL_TOKEN_PERCENT_RANK",
  "SQL_TOKEN_CUME_DIST", "SQL_TOKEN_COLLATE", "SQL_TOKEN_NOCASE",
  "SQL_TOKEN_RTRIM", "SQL_TOKEN_FILTER", "SQL_TOKEN_GROUPS",
  "SQL_TOKEN_GROUP_CONCAT", "SQL_TOKEN_OR", "SQL_TOKEN_AND", "SQL_ARROW",
  "SQL_BITWISE_OR", "SQL_BITWISE_AND", "SQL_BITWISE_SHIFT_LEFT",
  "SQL_BITWISE_SHIFT_RIGHT", "SQL_LESSEQ", "SQL_GREATEQ", "SQL_NOTEQUAL",
  "SQL_LESS", "SQL_GREAT", "SQL_EQUAL", "'+'", "'-'", "SQL_CONCAT", "'*'",
  "'/'", "'%'", "'='", "SQL_TOKEN_INVALIDSYMBOL", "$accept",
  "sql_single_statement", "sql", "pragma", "opt_pragma_for",
  "opt_pragma_set", "opt_pragma_set_val", "opt_pragma_func",
  "pragma_value", "pragma_path", "opt_cte_recursive", "cte_column_list",
  "cte_table_name", "cte_block_list", "cte", "column_commalist",
  "column_ref_commalist", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc",
  "opt_null_order", "first_last_desc", "sql_not", "manipulative_statement",
  "select_statement", "union_op", "delete_statement_searched",
  "insert_statement", "values_commalist", "values_or_query_spec",
  "row_value_constructor_commalist", "row_value_constructor",
  "row_value_constructor_elem", "opt_all_distinct", "assignment_commalist",
  "assignment", "update_source", "update_statement_searched",
  "opt_where_clause", "single_select_statement", "selection",
  "opt_limit_offset_clause", "opt_offset", "limit_offset_clause",
  "table_exp", "from_clause", "table_ref_commalist", "opt_as",
  "table_primary_as_range_column", "opt_only_all",
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
  "select_sublist", "literal", "as_clause", "unsigned_value_spec",
  "general_value_spec", "iif_spec", "fct_spec", "function_name",
  "value_creation_fct", "aggregate_fct", "opt_function_arg",
  "set_fct_type", "outer_join_type", "join_condition", "join_spec",
  "join_type", "cross_union", "qualified_join", "window_function",
  "window_function_type", "ntile_function", "opt_lead_or_lag_function",
  "lead_or_lag_function", "lead_or_lag", "lead_or_lag_extent",
  "first_or_last_value_function", "first_or_last_value",
  "nth_value_function", "opt_filter_clause", "window_name",
  "window_name_or_specification", "in_line_window_specification",
  "opt_window_clause", "window_definition_list", "window_definition",
  "new_window_name", "window_specification", "opt_existing_window_name",
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
  "cast_target", "cast_spec", "opt_optional_prop", "opt_extract_value",
  "value_exp_primary", "num_primary", "factor", "term", "term_add_sub",
  "num_value_exp", "datetime_primary", "datetime_value_fct",
  "datetime_factor", "datetime_term", "datetime_value_exp",
  "value_exp_commalist", "function_arg", "function_args_commalist",
  "value_exp", "string_value_exp", "char_value_exp", "concatenation",
  "char_primary", "char_factor", "derived_column", "table_node",
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

#define YYPACT_NINF (-604)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-459)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     495,    59,    75,    55,    78,    76,   101,   187,   165,   223,
     228,  -604,  -604,  -604,  -604,  -604,  -604,  -604,   169,  -604,
     216,    76,    68,  -604,  -604,   938,  -604,  -604,   226,  2588,
      22,  -604,  -604,  -604,  -604,  -604,  -604,   224,    98,  -604,
      73,   226,    84,   226,   256,  -604,  -604,   638,   254,  -604,
    -604,  -604,  -604,  -604,   788,    86,  -604,  -604,   291,   304,
     313,  -604,   317,   341,   347,  -604,  -604,   351,  -604,  -604,
    -604,   313,  -604,  2888,   358,  -604,  -604,  -604,  -604,  2288,
    -604,  -604,  2588,  2588,  2588,   366,   375,  -604,  -604,  -604,
    -604,   377,  -604,  -604,  -604,  -604,  -604,   381,  -604,  -604,
    -604,  -604,  -604,  -604,  2888,  2888,    91,    44,   238,  -604,
     337,   334,  -604,  -604,   266,    39,  -604,  -604,  2588,  -604,
    -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
    -604,  -604,   417,  -604,    46,   349,  -604,  -604,  2914,   428,
    -604,    -9,   432,  -604,   294,  -604,  -604,   445,  -604,   449,
    -604,   451,  2936,  -604,  -604,  -604,  -604,   197,   161,   252,
    -604,  -604,  -604,  -604,  -604,   128,  -604,   300,  -604,  -604,
    -604,  -604,    29,  -604,  2962,  -604,  -604,  -604,  -604,  -604,
     394,  1538,  -604,   386,  -604,  -604,  -604,  -604,   309,  -604,
      52,    52,   376,  -604,  -604,  -604,    63,   471,   499,   216,
    -604,   412,   505,   508,   256,     7,   438,   540,   542,   543,
      42,    15,  -604,  -604,  -604,   253,  -604,  -604,  2588,    56,
      53,  -604,   485,    78,    78,  1088,  -604,  -604,  1088,  1088,
    -604,   297,  -604,   457,   309,  -604,  -604,  -604,   555,  2588,
    2588,    78,  -604,  -604,   558,  2588,   251,  2438,  -604,  -604,
    -604,  -604,  -604,  -604,  -604,    37,  -604,   412,   317,  1088,
    -604,   538,  1088,  -604,  -604,  -604,  1088,  -604,  -604,  -604,
    -604,  1688,    78,   560,   443,  2588,  2588,   562,  2738,  2738,
    2738,  2738,  2738,  2738,  2738,  2738,  2738,  2588,  -604,   453,
       7,     7,  -604,     7,    20,  -604,  2588,  -604,  -604,  -604,
    -604,  -604,  -604,  -604,   564,  -604,   547,   467,  -604,  -604,
     420,    63,  -604,  1088,   467,  -604,  -604,  -604,   269,  -604,
    -604,   438,   549,   441,  -604,  -604,  2588,   446,  -604,  -604,
    -604,  -604,   544,   309,   574,  2588,  -604,  2588,  2588,    43,
     576,   -16,  2588,  -604,   483,  1088,   488,  -604,   457,  -604,
     584,   309,  -604,  -604,   585,  2588,  1538,  -604,  -604,   272,
       6,  2588,   518,  -604,  -604,  -604,  -604,   313,   565,   587,
     105,    27,   569,   557,  -604,  -604,  -604,  -604,   546,   260,
    -604,  -604,   266,  -604,  -604,  2588,  -604,   452,  2588,   412,
     262,   591,  -604,   598,  -604,  -604,  -604,  -604,   197,   197,
     161,   161,   161,   161,  -604,  -604,  -604,   141,    64,  -604,
     439,  -604,  -604,  -604,   296,   580,  -604,  -604,   575,   581,
     606,   460,  -604,   505,  -604,  -604,     7,   456,  -604,   611,
    3027,  -604,   612,   459,   463,  2588,  2588,  2588,  -604,  -604,
      54,    62,  -604,  2588,  -604,   610,   614,   618,    66,  -604,
     529,  -604,  -604,  2588,   621,   478,   309,  2588,  -604,  -604,
     464,  -604,  -604,  -604,  -604,    37,  -604,  -604,  -604,   569,
     318,   624,   318,   339,  -604,   568,  -604,  -604,  -604,  -604,
     307,   559,   571,   595,   582,    31,  -604,  -604,  -604,  -604,
    -604,  -604,   630,  -604,  2588,   631,   633,   615,  -604,  -604,
    -604,  -604,  -604,  2588,   643,  -604,  -604,  -604,     7,   467,
    2588,   625,   626,   484,   580,  -604,   639,  -604,  -604,  -604,
     482,  2588,   641,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
    -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,   627,
    -604,   651,  -604,  -604,  -604,   658,   621,  -604,    67,  1388,
    2588,  -604,   652,  2588,   663,  -604,  2588,   506,  2588,   557,
     318,  -604,   646,   256,  2588,  -604,   648,  -604,   569,   569,
      37,   613,  -604,    37,  2588,  1088,   537,  -604,   226,   496,
    -604,  -604,  -604,  -604,  -604,  -604,   539,  -604,   674,  -604,
    -604,  -604,  -604,   309,  -604,  -604,   355,  -604,    63,   516,
     656,  -604,  -604,  2588,   677,    69,  -604,  2588,  -604,  -604,
    -604,  -604,  -604,  -604,  -604,  -604,   309,  -604,  -604,  -604,
    -604,   521,   679,  -604,  -604,   557,    37,   414,   678,   460,
     615,   619,  -604,  -604,    31,   654,   619,  2588,  -604,  -604,
      46,  -604,   682,  -604,  -604,   687,  -604,  2588,  -604,  -604,
     538,  -604,   557,  1088,   281,  -604,  -604,  -604,  -604,   689,
    -604,   661,   660,   597,  -604,     7,   182,  -604,  -604,  -604,
     523,  -604,   460,   538,   489,   318,   615,   695,  1238,  2738,
     467,  -604,   694,  -604,   554,  -604,  -604,  -604,   697,  1838,
    -604,   538,   534,  -604,  -604,  -604,  -604,  -604,  -604,   700,
    -604,   330,   240,   166,  -604,     7,   122,  -604,  -604,  1988,
     579,   561,   588,  -604,  -604,   -50,  -604,  -604,  1238,  -604,
    -604,   672,   672,  2738,  -604,  -604,  -604,  -604,  -604,  -604,
     586,   578,  -604,  -604,   556,  -604,   -24,  -604,  -604,    24,
    -604,  -604,  -604,   503,  -604,  -604,   252,  -604,  -604,  2138,
    -604,  -604,  -604,  -604,   592,   593,  -604,  -604,  -604,   589,
    -604,  -604,  -604,  -604,  -604
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,     0,     0,    77,   105,     0,     0,     0,     2,
       4,    61,     6,    60,    57,    58,    88,    59,    62,    26,
       0,   105,     0,    78,    79,    55,   107,   106,     0,     0,
      10,     1,     3,     5,    66,    64,    65,   338,     0,    32,
       0,     0,     0,     0,    40,   421,   420,    55,     0,   468,
     204,   201,   202,   203,    55,   434,   239,   236,     0,     0,
       0,   216,    55,     0,     0,   215,   241,     0,   240,   237,
     217,     0,   440,     0,     0,   394,   396,   395,   238,     0,
     200,   416,     0,     0,     0,     0,     0,   269,   270,   273,
     274,     0,   322,   323,   324,   325,   226,     0,   164,   165,
     161,   160,   163,   162,     0,     0,   439,     0,    55,    75,
      96,   136,   138,   140,   142,   209,   129,   148,     0,   149,
     173,   174,   155,   179,   153,   184,   154,   150,   156,   151,
     152,   340,    90,   197,   213,   130,   212,   220,   131,     0,
     221,   219,     0,   373,   276,   260,   261,     0,   262,     0,
     263,     0,   133,   375,   376,   377,   381,   385,   388,   408,
     400,   393,   401,   402,   410,    76,   409,   411,   413,   417,
     412,   199,   366,   436,   132,   372,   443,   444,   445,   214,
       0,     0,   439,     0,    73,   368,   369,   371,    76,   370,
       0,     0,     8,    11,    12,   339,     0,     0,     0,     0,
      33,    85,     0,     0,    40,     0,     0,     0,     0,     0,
       0,    76,   467,    56,   139,   134,   435,   438,     0,    77,
       0,   192,   166,    77,    77,    55,   193,   380,    55,    55,
     466,   462,   459,     0,     0,   397,   399,   398,     0,     0,
       0,    77,   379,   378,     0,     0,     0,     0,   168,   171,
     172,   178,   183,   186,   187,   108,    87,    85,    55,    55,
     442,     0,    55,   419,   211,   159,    55,   207,   208,   206,
     205,     0,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   418,     0,
       0,     0,   441,     0,     0,    72,     0,    15,    16,    17,
      20,    21,    19,    18,     0,    13,     0,   471,    63,    28,
       0,     0,    31,    55,   471,    86,   425,   424,   425,   422,
     423,     0,   434,     0,    37,    68,     0,     0,   196,   195,
     135,   374,     0,   341,     0,     0,   177,     0,     0,     0,
       0,     0,     0,   460,     0,    55,   462,   448,     0,   258,
       0,   465,   405,   464,     0,     0,     0,   182,   180,   175,
     175,     0,     0,   190,   189,   191,   158,     0,   109,    98,
       0,   113,     0,    99,   255,   336,   335,   118,   120,     0,
     141,   210,   143,   198,   222,     0,   406,     0,     0,     0,
       0,   265,   271,     0,   257,   382,   383,   384,   386,   387,
     391,   392,   389,   390,   415,   414,   437,   364,    85,    80,
       0,    74,    14,    22,     9,     0,     7,   472,     0,     0,
       0,   119,    67,     0,    69,    41,     0,     0,   194,     0,
       0,   231,     0,     0,     0,     0,     0,     0,   463,   447,
       0,    75,   454,     0,   455,   173,   179,   184,     0,   451,
       0,   449,   264,     0,   234,     0,   403,     0,   170,   169,
       0,   225,   188,   219,   185,   108,   112,   111,   114,     0,
     469,   432,   103,   427,   428,     0,   244,   249,   242,   243,
     248,   250,     0,     0,   122,   108,   125,   127,   124,   126,
     137,   128,     0,   223,     0,     0,     0,   289,   278,   279,
     256,   280,   281,     0,     0,   272,   365,   367,     0,   471,
       0,     0,     0,   476,   473,   475,     0,    27,    30,    36,
      72,     0,   357,   348,   347,   349,   350,   344,   345,   351,
     346,   352,   356,   342,   343,   353,   354,   355,   358,   361,
     362,     0,   232,   228,   229,     0,   234,   461,   159,    55,
       0,   446,     0,     0,     0,   181,     0,   176,     0,   100,
     103,   102,     0,    40,     0,   431,     0,   116,     0,     0,
     108,     0,   251,   108,     0,    55,   282,   109,     0,     0,
     145,   224,   407,   230,   277,   291,   292,   290,   266,   268,
      81,    84,    82,    83,    24,    23,     0,   474,     0,     0,
       0,   360,   363,     0,     0,     0,   453,     0,   457,   458,
     456,   452,   450,   275,   235,   233,   404,   167,   115,   470,
     117,     0,    38,   430,   429,   252,   108,   248,   121,   123,
       0,    42,   147,   144,   108,     0,    42,     0,   479,   480,
     478,   477,     0,    70,   359,     0,   227,     0,   157,   433,
       0,   104,   253,    55,     0,   246,   254,   247,   287,   283,
     285,     0,     0,    91,   146,     0,   294,   267,    29,   218,
       0,    35,   245,     0,   332,   103,     0,     0,    55,     0,
     471,    92,   293,   297,   326,   305,   304,   306,     0,     0,
      39,     0,     0,   333,   334,   331,   426,   284,   286,    43,
      44,    48,    48,    93,    97,     0,     0,   298,   288,     0,
       0,     0,   299,   307,   308,     0,    34,   337,    55,    49,
      50,    51,    51,     0,    95,   296,   328,   329,   330,   327,
       0,     0,   314,   317,     0,   316,     0,   309,   311,     0,
     295,   310,    45,     0,    47,    46,    94,   318,   315,     0,
     321,   312,   301,   302,     0,     0,    53,    54,    52,     0,
     319,   313,   303,   300,   320
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,   519,  -604,
    -604,  -604,   536,  -604,   733,    65,  -604,  -604,  -195,   100,
    -604,    19,    38,    17,  -604,   -46,  -604,     3,  -604,  -604,
    -604,  -604,  -150,  -300,   -22,   -78,   110,  -604,   233,  -604,
    -604,  -219,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
     273,  -474,   721,  -604,  -444,   373,  -429,   357,  -604,  -604,
     151,  -604,   696,   490,   486,   -45,  -604,  -604,   117,  -603,
    -604,  -604,  -102,   645,  -604,  -100,   647,  -604,   399,   -91,
    -604,  -604,   -90,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
    -604,   -11,  -604,   501,   158,   603,   -10,  -604,  -604,   -15,
    -604,  -604,   407,   225,  -604,  -604,  -604,  -604,   290,  -604,
    -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
    -604,  -604,  -325,  -604,  -604,  -604,  -604,    96,  -604,    97,
    -604,  -604,  -604,  -604,  -604,    70,  -604,  -604,  -604,  -604,
    -604,  -604,    28,  -604,  -604,  -604,  -604,  -604,  -604,  -604,
    -604,  -604,  -604,  -604,    -2,  -604,  -604,  -604,  -604,  -604,
    -604,  -604,  -604,   528,    41,    12,   263,   189,  -598,  -604,
    -604,  -604,  -604,  -604,   202,  -157,  -287,   -25,   -71,  -604,
    -604,  -604,   498,   550,    -8,  -604,   590,   577,  -604,  -422,
    -604,   -12,  -604,  -604,   493,   500,   -20,  -247,  -604,  -604,
    -604,  -604,  -604,   437,  -604,   239,   572,  -167,   468,  -314,
    -604,  -604,  -604,  -604,  -293,  -604,  -604,   278,  -604
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     8,     9,    10,   307,   192,   193,   194,   304,   414,
      20,   310,    39,    40,   208,   670,   323,   651,   206,   663,
     699,   700,   721,   744,   758,   107,    12,   209,    37,    14,
      15,   327,    16,   183,   108,   109,    25,   408,   409,   592,
      17,   314,    18,   110,   680,   724,   681,   256,   257,   369,
     566,   567,    28,   370,   371,   372,   373,   315,   484,   576,
     490,   111,   112,   113,   114,   115,   491,   579,   580,   116,
     442,   117,   118,   444,   119,   120,   121,   122,   458,   123,
     124,   357,   125,   126,   253,   127,   128,   254,   367,   129,
     130,   131,   132,   133,   134,   263,   185,   136,   137,   186,
     139,   140,   141,   554,   142,   481,   655,   656,   482,   374,
     375,   143,   144,   145,   504,   146,   147,   391,   148,   149,
     150,   274,   658,   500,   501,   631,   659,   660,   661,   502,
     586,   587,   636,   688,   682,   683,   740,   689,   712,   713,
     732,   714,   733,   734,   761,   735,   151,   707,   729,   376,
     695,   377,   657,   196,   187,   332,   538,   539,   540,   541,
     153,   507,   292,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   455,   386,   387,   351,   166,   167,
     168,   169,   170,   171,    44,    45,    46,   320,   674,   472,
     473,   474,   565,   217,   172,   173,   189,   264,   175,   176,
     177,   178,   346,   347,   448,   449,   231,   232,   344,   352,
     353,   233,   179,   563,   416,   417,   514,   515,   641
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     165,   230,   210,    13,   188,   174,   247,   184,   249,   321,
     138,   235,   236,   237,   381,   135,   222,   251,   252,   331,
     180,   422,   211,   152,   331,   190,   427,   174,   438,   215,
     220,   322,   138,   201,   174,   204,   559,   135,   378,   138,
    -110,   578,   290,   200,   135,   152,   330,   560,   435,   221,
     433,   434,   152,   457,   188,  -110,   325,   234,   234,   234,
     226,  -110,   246,   260,   343,   499,   267,  -453,   268,   508,
     261,   549,  -157,   297,   298,   702,   299,   752,   199,   437,
       1,   703,   350,   354,   741,    23,   618,    19,   361,  -225,
     202,   -89,    42,   188,    72,   -89,   265,   203,   -89,   244,
      24,   197,   300,   361,    29,    26,   245,    23,   750,   244,
     751,   216,  -259,    22,   227,   702,   245,   287,   392,   393,
     301,   545,    24,   547,   244,   746,    21,     4,  -134,   198,
     302,   647,  -134,  -134,   466,  -134,   262,     4,   207,   -89,
    -259,   625,   -89,   287,   627,   242,   243,     4,     6,   506,
     313,   -89,   260,   269,   290,   753,   294,  -453,     6,   261,
     270,   550,  -157,   754,   755,    31,   303,  -134,  -134,    43,
     -89,   424,   585,  -409,   360,   182,  -134,    27,   432,  -134,
     339,   343,   287,   291,   341,   324,   191,   287,  -134,   509,
     578,   262,   468,   333,   262,   262,   577,   652,   454,   308,
     215,   696,   368,   165,   215,   174,   467,  -134,   174,   174,
     138,    30,   379,   138,   138,   135,   591,    34,   135,   135,
     234,   599,   188,   152,   334,   366,   152,   152,   492,    35,
      32,   495,   675,   358,   215,    33,   612,   215,   726,   174,
      38,   165,   174,   443,   138,   445,   174,   138,    36,   135,
      42,   138,   135,   195,   446,   447,   135,   152,   -76,   205,
     152,   213,   404,   485,   152,   497,   723,   441,   421,   727,
     728,   188,   719,   410,   411,   202,   -76,   621,   212,   546,
    -134,  -134,   423,   720,   673,   361,   498,   -76,   215,   645,
     395,   396,   397,   174,   218,   287,   552,    62,   138,   440,
     341,   188,   511,   135,   184,   471,   244,   219,   -76,   512,
     486,   152,   -76,   245,   420,   -76,   220,   685,   686,   457,
     215,   283,   284,   285,   286,   174,   281,   282,   487,   335,
     138,   456,   687,   337,   338,   135,   188,   582,   488,   460,
     213,   489,  -101,   152,   223,   568,   588,   462,   -76,   561,
     224,   355,   569,   -75,   225,   362,   464,   -76,   485,   476,
     470,   228,   719,   477,   -75,   278,   279,   280,   620,   238,
     478,   479,  -368,   720,    50,    51,    52,    53,   239,   638,
     240,   385,   388,  -368,   241,   -75,   557,   704,   255,   -75,
     295,   296,   -75,   258,   229,   342,   614,    98,    99,   100,
     101,   102,   103,   671,  -368,   486,   519,   283,   284,   285,
     286,  -368,   -76,   -76,   -76,   -76,   -76,   -76,   188,   259,
     287,   548,   266,   487,   418,   419,   671,  -376,  -376,  -376,
    -376,   271,   234,   488,   -75,   272,   489,  -376,  -376,  -376,
    -376,  -376,  -376,   273,   716,   425,   426,   607,   275,   608,
     428,   429,   276,  -368,   277,   475,   493,   494,   609,   610,
     520,   296,    80,   543,   494,   289,   476,   544,   494,   293,
     477,   606,   400,   401,   402,   403,   287,   478,   479,   306,
     667,   480,   555,   556,   653,   593,   -71,   -71,   410,   -75,
     -75,   -75,   -75,   -75,   -75,   309,   188,   654,   313,   184,
     633,   634,   311,   605,  -368,  -368,  -368,  -368,  -368,  -368,
    -368,  -368,  -368,  -368,  -368,  -368,  -368,  -368,  -368,  -368,
     643,   296,     1,     6,   188,   649,   494,   690,   691,   316,
     629,   616,   318,   188,   693,   694,   617,     2,   717,   691,
     756,   757,   461,   326,   398,   399,   328,   329,    56,   456,
     215,    57,     3,   336,   345,   174,   623,   624,    59,   349,
     138,   356,   260,   389,   390,   135,   394,    81,   412,     4,
     632,   413,   415,   152,   216,   430,   360,     5,   431,   439,
       6,   436,   188,    63,    64,   648,   342,    66,   452,  -114,
     453,    67,   465,   471,    68,    69,   503,     7,   475,   483,
     701,   642,   505,   510,   513,   517,   516,    74,   672,   476,
     518,    78,   262,   477,   521,  -457,   542,   558,  -248,  -458,
     478,   479,   234,  -456,   480,   551,   553,   564,   215,   570,
     574,   572,   573,   174,   581,   583,   575,   584,   138,   498,
     701,    47,   598,   135,    48,   684,    49,   589,   596,   594,
     595,   152,   601,   188,   600,   602,   613,    50,    51,    52,
      53,    54,    55,   603,   715,     1,    96,   615,    56,    97,
     619,    57,   622,  -409,   626,   630,   635,    58,    59,   637,
     644,   646,   650,   556,   736,   684,   668,    60,    61,   665,
     662,   669,   677,   188,   676,   678,   679,    62,   497,   705,
     706,   708,   738,    63,    64,   718,    65,    66,   743,   749,
     305,    67,     4,   737,    68,    69,    70,   739,    71,   748,
     747,   764,   762,   207,   736,    72,    73,    74,    75,    76,
      77,    78,    79,    11,   763,   312,   666,   742,   692,   745,
     722,   590,    41,   562,   469,    80,   496,   639,   382,   380,
     214,   664,    81,   248,   640,   250,    82,    83,    84,   459,
      85,    86,    87,    88,    89,    90,    91,   383,   288,   463,
     571,   604,   697,   359,   698,   725,   628,   760,   340,   317,
      92,    93,    94,    95,   407,   451,    96,   405,   611,    97,
     406,    47,   597,   319,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,   348,   182,    50,    51,    52,
      53,   213,    55,     0,   450,     0,     0,     0,    56,     0,
       0,    57,   -56,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,    71,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,    47,     0,     0,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   182,    50,    51,    52,
      53,    54,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,    71,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,    47,     0,     0,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   106,    50,    51,    52,
      53,    54,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,    71,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   182,    50,    51,    52,
      53,   213,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,    71,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   182,    50,    51,    52,
      53,   213,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     1,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     4,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,   207,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,   384,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,    23,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,    24,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,   709,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,   710,     0,     0,     0,     0,     0,     0,   711,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,   730,     0,     0,     0,     0,     0,     0,   731,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,   759,     0,     0,     0,     0,     0,     0,   731,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,   229,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,   363,   364,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,   365,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,     0,
       0,    67,     0,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,     0,     0,
       0,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   181,     0,     0,    48,     0,    49,     0,     0,     0,
       0,     0,     0,   104,   105,     0,   182,    50,    51,    52,
      53,     0,    55,     0,     0,     0,     0,     0,    56,     0,
       0,    57,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,     0,  -369,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -369,     0,
       0,     0,     0,    63,    64,     0,    65,    66,     0,  -371,
       0,    67,     0,     0,    68,    69,    70,     0,     0,  -369,
    -371,     0,     0,     0,     0,    72,  -369,    74,     0,     0,
       0,    78,    79,     0,     0,  -370,     0,     0,     0,     0,
       0,  -371,     0,     0,     0,    80,  -370,     0,  -371,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,    87,    88,    89,    90,    91,  -370,  -369,     0,
       0,     0,     0,     0,  -370,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
    -371,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   522,     0,     0,     0,     0,   182,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -370,     0,     0,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,     0,     0,     0,     0,     0,
       0,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -370,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,
    -370,  -370,  -370,   523,   524,   525,   526,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   537
};

static const yytype_int16 yycheck[] =
{
      25,    79,    47,     0,    29,    25,   108,    29,   108,   204,
      25,    82,    83,    84,   261,    25,    62,   108,   108,     4,
      28,   314,    47,    25,     4,     3,   326,    47,   342,    54,
       3,    24,    47,    41,    54,    43,   465,    47,   257,    54,
       3,   485,    13,    40,    54,    47,     4,   469,     5,    60,
     337,   338,    54,    47,    79,    24,   206,    82,    83,    84,
      71,    24,   108,    24,   231,   390,    20,     5,    22,     5,
      31,     5,     5,    21,    22,   678,    24,    53,     5,    95,
      27,   679,   239,   240,   134,    29,   560,    28,    34,     3,
       6,     0,    24,   118,    87,     4,   118,    13,     7,    55,
      44,     3,    50,    34,     3,    29,    62,    29,   132,    55,
     134,    25,   121,    58,    73,   718,    62,   167,   275,   276,
      68,   435,    44,   437,    55,   723,    51,    74,     0,    31,
      78,    62,     4,     5,    29,     7,   152,    74,    85,    48,
     149,   570,    51,   167,   573,   104,   105,    74,    85,     8,
      86,    60,    24,   107,    13,   131,   181,    95,    85,    31,
     114,    95,    95,   139,   140,     0,   114,   152,   153,   101,
      79,   321,   497,   167,   245,   168,    48,   101,   335,    51,
     225,   348,   167,   154,   229,   205,   164,   167,    60,   408,
     634,   152,   165,   218,   152,   152,   165,   626,   355,   196,
     225,   675,   165,   228,   229,   225,   101,    79,   228,   229,
     225,    24,   258,   228,   229,   225,   509,    48,   228,   229,
     245,   521,   247,   225,   168,   247,   228,   229,   385,    60,
       7,   388,   654,   244,   259,     7,   550,   262,   116,   259,
      24,   266,   262,   345,   259,   345,   266,   262,    79,   259,
      24,   266,   262,    29,   345,   345,   266,   259,     5,     3,
     262,    23,   287,     3,   266,     3,   100,   345,   313,   147,
     148,   296,    32,   293,   296,     6,    23,   564,    24,   436,
     152,   153,    13,    43,     3,    34,    24,    34,   313,   603,
     278,   279,   280,   313,     3,   167,   453,    59,   313,   345,
     345,   326,     6,   313,   326,    24,    55,     3,    55,    13,
      50,   313,    59,    62,   311,    62,     3,   135,   136,    47,
     345,   155,   156,   157,   158,   345,   165,   166,    68,   219,
     345,   356,   150,   223,   224,   345,   361,   494,    78,   361,
      23,    81,    24,   345,     3,     6,   503,   362,    95,    31,
       3,   241,    13,    23,     3,   104,   367,   104,     3,    52,
     371,     3,    32,    56,    34,   168,   169,   170,   563,     3,
      63,    64,    23,    43,    19,    20,    21,    22,     3,    24,
       3,   271,   272,    34,     3,    55,   457,   680,    51,    59,
       4,     5,    62,    59,    97,    98,   553,   159,   160,   161,
     162,   163,   164,   650,    55,    50,   426,   155,   156,   157,
     158,    62,   159,   160,   161,   162,   163,   164,   443,   153,
     167,   443,     5,    68,     4,     5,   673,   155,   156,   157,
     158,     3,   457,    78,   104,     3,    81,   165,   166,   167,
     168,   169,   170,   149,   691,     4,     5,   549,     3,   549,
       4,     5,     3,   104,     3,    41,     4,     5,   549,   549,
       4,     5,   107,     4,     5,   165,    52,     4,     5,    75,
      56,   549,   283,   284,   285,   286,   167,    63,    64,   103,
     637,    67,     4,     5,    70,   510,     4,     5,   508,   159,
     160,   161,   162,   163,   164,    24,   521,    83,    86,   521,
       4,     5,     3,   549,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
       4,     5,    27,    85,   549,     4,     5,     4,     5,    24,
     575,   556,    24,   558,    45,    46,   558,    42,     4,     5,
      37,    38,    24,     3,   281,   282,     4,     4,    30,   574,
     575,    33,    57,    68,    97,   575,   568,   569,    40,     4,
     575,     3,    24,     3,   121,   575,     4,   114,     4,    74,
     578,    24,   105,   575,    25,    31,   647,    82,     4,    96,
      85,     5,   607,    65,    66,   607,    98,    69,     4,    24,
       5,    73,     5,    24,    76,    77,     5,   102,    41,    53,
     678,   598,     4,   164,    24,    24,    31,    89,   653,    52,
       4,    93,   152,    56,     3,     5,     4,   153,    61,     5,
      63,    64,   647,     5,    67,    96,     5,     3,   653,    61,
      35,    72,    61,   653,     4,     4,    54,     4,   653,    24,
     718,     3,     3,   653,     6,   665,     8,     4,   164,    24,
      24,   653,    25,   678,    13,     4,     4,    19,    20,    21,
      22,    23,    24,     5,   689,    27,   148,     4,    30,   151,
      24,    33,    24,   167,    61,   138,   137,    39,    40,     5,
      24,     4,     3,     5,   709,   705,     4,    49,    50,    35,
      71,     4,    31,   718,     5,    35,    99,    59,     3,     5,
     146,     4,   141,    65,    66,     5,    68,    69,    36,   153,
     191,    73,    74,   134,    76,    77,    78,   129,    80,   141,
     134,   132,   130,    85,   749,    87,    88,    89,    90,    91,
      92,    93,    94,     0,   141,   199,   636,   718,   673,   722,
     702,   508,    21,   470,   371,   107,   389,   596,   262,   259,
      54,   634,   114,   108,   596,   108,   118,   119,   120,   360,
     122,   123,   124,   125,   126,   127,   128,   266,   165,   362,
     480,   546,   676,   245,   677,   705,   574,   749,   228,   202,
     142,   143,   144,   145,   291,   348,   148,   289,   549,   151,
     290,     3,   514,   203,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,   233,   168,    19,    20,    21,
      22,    23,    24,    -1,   346,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,    -1,   168,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,    -1,   168,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,    -1,   168,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,    -1,   168,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,   159,   160,   161,
     162,   163,   164,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    27,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    74,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,     4,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    -1,    -1,
      -1,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   165,   166,    -1,   168,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    23,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    55,
      34,    -1,    -1,    -1,    -1,    87,    62,    89,    -1,    -1,
      -1,    93,    94,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,   107,    34,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,    55,   104,    -1,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,
     142,   143,   144,   145,    -1,    -1,   148,    -1,    -1,   151,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,    -1,    -1,    -1,    -1,    -1,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    42,    57,    74,    82,    85,   102,   174,   175,
     176,   187,   199,   200,   202,   203,   205,   213,   215,    28,
     183,    51,    58,    29,    44,   209,    29,   101,   225,     3,
      24,     0,     7,     7,    48,    60,    79,   201,    24,   185,
     186,   225,    24,   101,   357,   358,   359,     3,     6,     8,
      19,    20,    21,    22,    23,    24,    30,    33,    39,    40,
      49,    50,    59,    65,    66,    68,    69,    73,    76,    77,
      78,    80,    87,    88,    89,    90,    91,    92,    93,    94,
     107,   114,   118,   119,   120,   122,   123,   124,   125,   126,
     127,   128,   142,   143,   144,   145,   148,   151,   159,   160,
     161,   162,   163,   164,   165,   166,   168,   198,   207,   208,
     216,   234,   235,   236,   237,   238,   242,   244,   245,   247,
     248,   249,   250,   252,   253,   255,   256,   258,   259,   262,
     263,   264,   265,   266,   267,   269,   270,   271,   272,   273,
     274,   275,   277,   284,   285,   286,   288,   289,   291,   292,
     293,   319,   327,   333,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   350,   351,   352,   353,   354,
     355,   356,   367,   368,   369,   371,   372,   373,   374,   385,
     357,     3,   168,   206,   207,   269,   272,   327,   350,   369,
       3,   164,   178,   179,   180,    29,   326,     3,    31,     5,
     200,   357,     6,    13,   357,     3,   191,    85,   187,   200,
     238,   350,    24,    23,   235,   350,    25,   366,     3,     3,
       3,   264,   198,     3,     3,     3,   264,   337,     3,    97,
     208,   379,   380,   384,   350,   351,   351,   351,     3,     3,
       3,     3,   337,   337,    55,    62,   198,   245,   246,   248,
     249,   252,   255,   257,   260,    51,   220,   221,    59,   153,
      24,    31,   152,   268,   370,   207,     5,    20,    22,   107,
     114,     3,     3,   149,   294,     3,     3,     3,   168,   169,
     170,   165,   166,   155,   156,   157,   158,   167,   268,   165,
      13,   154,   335,    75,   350,     4,     5,    21,    22,    24,
      50,    68,    78,   114,   181,   181,   103,   177,   200,    24,
     184,     3,   185,    86,   214,   230,    24,   360,    24,   359,
     360,   191,    24,   189,   369,   205,     3,   204,     4,     4,
       4,     4,   328,   350,   168,   209,    68,   209,   209,   238,
     356,   238,    98,   380,   381,    97,   375,   376,   379,     4,
     348,   350,   382,   383,   348,   209,     3,   254,   264,   336,
     351,    34,   104,    29,    30,    76,   207,   261,   165,   222,
     226,   227,   228,   229,   282,   283,   322,   324,   214,   198,
     236,   370,   237,   266,     4,   209,   348,   349,   209,     3,
     121,   290,   348,   348,     4,   338,   338,   338,   339,   339,
     340,   340,   340,   340,   350,   355,   368,   367,   210,   211,
     369,   207,     4,    24,   182,   105,   387,   388,     4,     5,
     200,   238,   387,    13,   205,     4,     5,   206,     4,     5,
      31,     4,   348,   349,   349,     5,     5,    95,   382,    96,
     198,   208,   243,   245,   246,   248,   252,   255,   377,   378,
     381,   376,     4,     5,   348,   347,   350,    47,   251,   251,
     207,    24,   272,   275,   264,     5,    29,   101,   165,   228,
     264,    24,   362,   363,   364,    41,    52,    56,    63,    64,
      67,   278,   281,    53,   231,     3,    50,    68,    78,    81,
     233,   239,   348,     4,     5,   348,   230,     3,    24,   295,
     296,   297,   302,     5,   287,     4,     8,   334,     5,   214,
     164,     6,    13,    24,   389,   390,    31,    24,     4,   369,
       4,     3,    24,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   329,   330,
     331,   332,     4,     4,     4,   382,   348,   382,   207,     5,
      95,    96,   348,     5,   276,     4,     5,   351,   153,   229,
     362,    31,   223,   386,     3,   365,   223,   224,     6,    13,
      61,   281,    72,    61,    35,    54,   232,   165,   227,   240,
     241,     4,   348,     4,     4,   295,   303,   304,   348,     4,
     211,   387,   212,   350,    24,    24,   164,   390,     3,   206,
      13,    25,     4,     5,   276,   198,   208,   245,   248,   252,
     255,   378,   382,     4,   348,     4,   350,   207,   224,    24,
     191,   349,    24,   364,   364,   229,    61,   229,   347,   238,
     138,   298,   357,     4,     5,   137,   305,     5,    24,   233,
     267,   391,   200,     4,    24,   382,     4,    62,   207,     4,
       3,   190,   229,    70,    83,   279,   280,   325,   295,   299,
     300,   301,    71,   192,   241,    35,   192,   348,     4,     4,
     188,   370,   238,     3,   361,   362,     5,    31,    35,    99,
     217,   219,   307,   308,   369,   135,   136,   150,   306,   310,
       4,     5,   188,    45,    46,   323,   224,   300,   302,   193,
     194,   208,   242,   341,   387,     5,   146,   320,     4,    34,
     133,   140,   311,   312,   314,   350,   370,     4,     5,    32,
      43,   195,   195,   100,   218,   308,   116,   147,   148,   321,
     133,   140,   313,   315,   316,   318,   350,   134,   141,   129,
     309,   134,   194,    36,   196,   196,   341,   134,   141,   153,
     132,   134,    53,   131,   139,   140,    37,    38,   197,   133,
     315,   317,   130,   141,   132
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   173,   174,   174,   174,   174,   175,   176,   177,   177,
     178,   178,   178,   179,   180,   181,   181,   181,   181,   181,
     181,   181,   182,   182,   182,   183,   183,   184,   184,   185,
     185,   186,   186,   187,   188,   188,   189,   189,   190,   190,
     191,   191,   192,   192,   193,   193,   194,   194,   195,   195,
     195,   196,   196,   197,   197,   198,   198,   199,   199,   199,
     199,   199,   200,   200,   201,   201,   201,   202,   203,   203,
     204,   204,   205,   206,   206,   207,   208,   209,   209,   209,
     210,   210,   211,   212,   213,   214,   214,   215,   215,   216,
     216,   217,   217,   218,   218,   219,   220,   220,   221,   222,
     222,   223,   223,   224,   224,   225,   225,   225,   226,   226,
     227,   227,   227,   228,   228,   229,   229,   229,   229,   230,
     231,   231,   232,   232,   233,   233,   233,   233,   233,   234,
     234,   234,   234,   234,   234,   234,   235,   235,   236,   236,
     237,   237,   238,   238,   239,   240,   240,   241,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   243,   244,   244,
     245,   245,   245,   245,   245,   245,   245,   246,   247,   248,
     249,   250,   250,   250,   250,   251,   251,   252,   253,   253,
     254,   254,   255,   256,   256,   257,   258,   259,   260,   261,
     261,   261,   262,   263,   264,   264,   264,   265,   265,   266,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   268,
     268,   268,   269,   269,   270,   270,   270,   270,   271,   272,
     272,   272,   272,   272,   272,   273,   273,   274,   275,   275,
     275,   275,   275,   275,   276,   276,   277,   277,   277,   277,
     277,   277,   278,   278,   278,   279,   280,   280,   281,   281,
     281,   281,   282,   283,   283,   283,   284,   285,   285,   285,
     285,   285,   285,   285,   286,   287,   287,   287,   288,   289,
     289,   290,   291,   292,   292,   293,   294,   294,   295,   296,
     296,   297,   298,   298,   299,   299,   300,   301,   302,   303,
     303,   304,   305,   305,   306,   306,   307,   307,   308,   309,
     309,   309,   309,   309,   310,   310,   310,   311,   311,   312,
     312,   312,   313,   314,   315,   315,   315,   316,   316,   317,
     317,   318,   319,   319,   319,   319,   320,   320,   321,   321,
     321,   322,   323,   323,   323,   324,   324,   325,   326,   326,
     327,   328,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   330,   330,
     331,   332,   332,   333,   334,   334,   335,   335,   336,   336,
     336,   336,   336,   336,   336,   336,   337,   338,   338,   338,
     338,   339,   339,   339,   339,   340,   340,   340,   341,   341,
     341,   341,   341,   342,   343,   343,   343,   343,   343,   343,
     344,   345,   346,   347,   347,   348,   349,   349,   350,   350,
     350,   351,   352,   352,   353,   353,   354,   355,   356,   356,
     357,   357,   358,   359,   359,   360,   361,   362,   363,   363,
     363,   364,   365,   365,   366,   366,   367,   367,   368,   368,
     368,   369,   370,   371,   372,   372,   373,   374,   375,   375,
     376,   377,   377,   378,   378,   378,   378,   378,   378,   379,
     379,   380,   381,   381,   382,   383,   384,   385,   385,   386,
     386,   387,   387,   388,   389,   389,   390,   390,   391,   391,
     391
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     5,     0,     2,
       0,     1,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     0,     1,     3,     1,     8,
       5,     3,     1,     4,     3,     1,     3,     1,     0,     3,
       0,     3,     0,     3,     1,     3,     3,     3,     0,     1,
       1,     0,     2,     1,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     6,     5,     6,
       5,     3,     4,     1,     3,     1,     1,     0,     1,     1,
       1,     3,     3,     1,     7,     0,     1,     4,     1,     1,
       1,     0,     1,     0,     2,     3,     0,     8,     2,     1,
       3,     0,     1,     0,     3,     0,     1,     1,     0,     1,
       0,     2,     2,     0,     1,     4,     3,     4,     1,     2,
       0,     3,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     4,     1,     2,
       1,     3,     1,     3,     3,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     2,
       1,     1,     1,     1,     1,     1,     2,     5,     2,     4,
       4,     2,     2,     1,     1,     0,     2,     3,     2,     1,
       1,     3,     3,     2,     1,     3,     2,     2,     3,     1,
       1,     1,     2,     2,     4,     3,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     0,
       2,     1,     1,     1,     1,     1,     1,     1,     8,     1,
       1,     1,     3,     4,     5,     1,     1,     7,     5,     5,
       5,     4,     5,     6,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     0,     1,
       1,     2,     4,     5,     5,     1,     4,     3,     3,     1,
       1,     1,     1,     1,     4,     0,     2,     4,     5,     1,
       1,     1,     4,     1,     1,     6,     0,     4,     1,     1,
       1,     1,     0,     2,     3,     1,     3,     1,     6,     0,
       1,     1,     0,     3,     0,     3,     3,     1,     2,     0,
       3,     2,     2,     3,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     4,     1,     2,     1,     1,     2,     1,
       2,     2,     1,     1,     1,     1,     0,     2,     1,     1,
       1,     7,     0,     1,     1,     1,     1,     4,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     6,     0,     1,     0,     3,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     2,     2,
       2,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     1,     1,     1,     2,     2,     2,
       1,     1,     1,     1,     3,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     2,     2,
       1,     1,     3,     3,     3,     1,     2,     1,     1,     3,
       3,     2,     0,     3,     0,     1,     1,     3,     2,     1,
       1,     2,     1,     1,     1,     1,     5,     4,     1,     2,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       2,     4,     0,     2,     1,     1,     1,     2,     1,     0,
       2,     0,     1,     2,     2,     1,     1,     3,     1,     1,
       1
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

  case 30: /* cte_table_name: SQL_TOKEN_NAME SQL_TOKEN_AS '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 31: /* cte_block_list: cte_block_list ',' cte_table_name  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 32: /* cte_block_list: cte_table_name  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 33: /* cte: SQL_TOKEN_WITH opt_cte_recursive cte_block_list select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 34: /* column_commalist: column_commalist ',' column  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 35: /* column_commalist: column  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 36: /* column_ref_commalist: column_ref_commalist ',' column_ref  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 37: /* column_ref_commalist: column_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 38: /* opt_column_commalist: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 39: /* opt_column_commalist: '(' column_commalist ')'  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 40: /* opt_column_ref_commalist: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 41: /* opt_column_ref_commalist: '(' column_ref_commalist ')'  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));}
    break;

  case 42: /* opt_order_by_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 43: /* opt_order_by_clause: SQL_TOKEN_ORDER SQL_TOKEN_BY ordering_spec_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 44: /* ordering_spec_commalist: ordering_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 45: /* ordering_spec_commalist: ordering_spec_commalist ',' ordering_spec  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 46: /* ordering_spec: predicate opt_asc_desc opt_null_order  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 47: /* ordering_spec: row_value_constructor_elem opt_asc_desc opt_null_order  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));

        }
    break;

  case 48: /* opt_asc_desc: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 51: /* opt_null_order: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 52: /* opt_null_order: SQL_TOKEN_NULLS first_last_desc  */
         {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
         }
    break;

  case 55: /* sql_not: %empty  */
    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 62: /* select_statement: single_select_statement  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 63: /* select_statement: single_select_statement union_op all select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 67: /* delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM opt_only_all table_node opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 68: /* insert_statement: SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_ref_commalist values_or_query_spec  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 69: /* insert_statement: SQL_TOKEN_INSERT SQL_TOKEN_INTO SQL_TOKEN_ONLY table_node opt_column_ref_commalist values_or_query_spec  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 70: /* values_commalist: values_commalist ',' '(' row_value_constructor_commalist ')'  */
        {
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode) = (yyvsp[-4].pParseNode);
        }
    break;

  case 71: /* values_commalist: '(' row_value_constructor_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
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

  case 77: /* opt_all_distinct: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 80: /* assignment_commalist: assignment  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 81: /* assignment_commalist: assignment_commalist ',' assignment  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 82: /* assignment: column_ref SQL_EQUAL update_source  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 84: /* update_statement_searched: SQL_TOKEN_UPDATE opt_only_all table_node SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 85: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 87: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 89: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 91: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 93: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 94: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 95: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 96: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 97: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
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

  case 98: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 99: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 100: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 101: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 103: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 104: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 105: /* opt_only_all: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 106: /* opt_only_all: SQL_TOKEN_ONLY  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 107: /* opt_only_all: SQL_TOKEN_ALL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 108: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 109: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 110: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 111: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 112: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 113: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 114: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 115: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 116: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 117: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 119: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 120: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 121: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 122: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 123: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 135: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 137: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 139: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 141: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 143: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 144: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 145: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 146: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 147: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 157: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 158: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 159: /* comparison_predicate: comparison row_value_constructor  */
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

  case 166: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 167: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 168: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 169: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 170: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 171: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 172: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 173: /* like_predicate: character_like_predicate_part_2  */
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

  case 174: /* like_predicate: other_like_predicate_part_2  */
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

  case 175: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 176: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 177: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 178: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 179: /* test_for_null: null_predicate_part_2  */
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

  case 180: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 181: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 182: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 183: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 184: /* in_predicate: in_predicate_part_2  */
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

  case 185: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 186: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 187: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 188: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 192: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 193: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 194: /* subquery: '(' SQL_TOKEN_VALUES values_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 195: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 196: /* subquery: '(' cte ')'  */
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

  case 205: /* literal: literal SQL_TOKEN_STRING  */
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

  case 206: /* literal: literal SQL_TOKEN_INT  */
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

  case 207: /* literal: literal SQL_TOKEN_REAL_NUM  */
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

  case 208: /* literal: literal SQL_TOKEN_APPROXNUM  */
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

  case 209: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 210: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 218: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
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

  case 227: /* value_creation_fct: SQL_TOKEN_NAVIGATION_VALUE '(' derived_column ',' function_arg opt_function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 228: /* aggregate_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 229: /* aggregate_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 230: /* aggregate_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 231: /* aggregate_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 232: /* aggregate_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 233: /* aggregate_fct: SQL_TOKEN_GROUP_CONCAT '(' opt_all_distinct function_arg opt_function_arg ')'  */
        {
            if ((yyvsp[-3].pParseNode)->isToken() && (yyvsp[-1].pParseNode)->count()!=0)
                {
                SQLyyerror(context, "Aggregate function can use DISTINCT or ALL keywords only with one argument.");
                YYERROR;
                }
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 234: /* opt_function_arg: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 235: /* opt_function_arg: ',' function_arg  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 242: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 243: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 244: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 245: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 248: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 249: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 251: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 252: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 253: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 254: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 256: /* window_function: window_function_type opt_filter_clause SQL_TOKEN_OVER window_name_or_specification  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 257: /* window_function_type: rank_function_type '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 258: /* window_function_type: SQL_TOKEN_ROW_NUMBER '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 264: /* ntile_function: SQL_TOKEN_NTILE '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 265: /* opt_lead_or_lag_function: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 266: /* opt_lead_or_lag_function: ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 267: /* opt_lead_or_lag_function: ',' function_arg ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 268: /* lead_or_lag_function: lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 272: /* first_or_last_value_function: first_or_last_value '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 275: /* nth_value_function: SQL_TOKEN_NTH_VALUE '(' function_arg ',' function_arg ')'  */
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

  case 276: /* opt_filter_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 277: /* opt_filter_clause: SQL_TOKEN_FILTER '(' where_clause ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 282: /* opt_window_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 283: /* opt_window_clause: SQL_TOKEN_WINDOW window_definition_list  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 284: /* window_definition_list: window_definition_list ',' window_definition  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 285: /* window_definition_list: window_definition  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 286: /* window_definition: new_window_name SQL_TOKEN_AS window_specification  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 288: /* window_specification: '(' opt_existing_window_name opt_window_partition_clause opt_order_by_clause opt_window_frame_clause ')'  */
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

  case 289: /* opt_existing_window_name: %empty  */
                                 {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 292: /* opt_window_partition_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 293: /* opt_window_partition_clause: SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 294: /* opt_window_frame_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 295: /* opt_window_frame_clause: window_frame_units window_frame_extent opt_window_frame_exclusion  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 296: /* window_partition_column_reference_list: window_partition_column_reference_list ',' window_partition_column_reference  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 297: /* window_partition_column_reference_list: window_partition_column_reference  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 298: /* window_partition_column_reference: column_ref opt_collate_clause  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 299: /* opt_window_frame_exclusion: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 300: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 301: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 302: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 303: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 309: /* window_frame_start: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 310: /* window_frame_start: value_exp SQL_TOKEN_PRECEDING  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 311: /* window_frame_start: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 312: /* window_frame_preceding: value_exp SQL_TOKEN_PRECEDING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 313: /* window_frame_between: SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 315: /* window_frame_bound: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 318: /* window_frame_bound_1: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 320: /* window_frame_bound_2: SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 321: /* window_frame_following: value_exp SQL_TOKEN_FOLLOWING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 326: /* opt_collate_clause: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 327: /* opt_collate_clause: SQL_TOKEN_COLLATE collating_function  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 331: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 332: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 337: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 338: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 358: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 359: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 360: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 363: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 364: /* opt_optional_prop: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 365: /* opt_optional_prop: '?'  */
             {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
    }
    break;

  case 366: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 367: /* opt_extract_value: SQL_ARROW property_path opt_optional_prop  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 374: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 378: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 380: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 389: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 390: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 392: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 393: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 394: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 395: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 396: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 397: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 398: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 399: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 400: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 401: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 402: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 403: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 404: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 406: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 407: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 414: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 415: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 418: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 419: /* derived_column: search_condition as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 420: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 421: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 422: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 423: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 424: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 425: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 426: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 427: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 428: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 429: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 430: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 431: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 432: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 433: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 434: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 435: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 436: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 437: /* property_path: property_path '.' property_path_entry  */
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

  case 438: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 439: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 440: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 441: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 446: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 447: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 448: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 449: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 450: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 451: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 452: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 459: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 460: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 461: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 462: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 463: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 467: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 468: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 469: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 470: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 471: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 473: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 474: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 475: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 476: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 477: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
