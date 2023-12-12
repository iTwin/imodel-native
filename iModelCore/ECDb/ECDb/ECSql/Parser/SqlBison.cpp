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
  YYSYMBOL_SQL_TOKEN_CURRENT_DATE = 89,    /* SQL_TOKEN_CURRENT_DATE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIME = 90,    /* SQL_TOKEN_CURRENT_TIME  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIMESTAMP = 91, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_EVERY = 92,           /* SQL_TOKEN_EVERY  */
  YYSYMBOL_SQL_TOKEN_CASE = 93,            /* SQL_TOKEN_CASE  */
  YYSYMBOL_SQL_TOKEN_THEN = 94,            /* SQL_TOKEN_THEN  */
  YYSYMBOL_SQL_TOKEN_END = 95,             /* SQL_TOKEN_END  */
  YYSYMBOL_SQL_TOKEN_WHEN = 96,            /* SQL_TOKEN_WHEN  */
  YYSYMBOL_SQL_TOKEN_ELSE = 97,            /* SQL_TOKEN_ELSE  */
  YYSYMBOL_SQL_TOKEN_LIMIT = 98,           /* SQL_TOKEN_LIMIT  */
  YYSYMBOL_SQL_TOKEN_OFFSET = 99,          /* SQL_TOKEN_OFFSET  */
  YYSYMBOL_SQL_TOKEN_ONLY = 100,           /* SQL_TOKEN_ONLY  */
  YYSYMBOL_SQL_TOKEN_PRAGMA = 101,         /* SQL_TOKEN_PRAGMA  */
  YYSYMBOL_SQL_TOKEN_FOR = 102,            /* SQL_TOKEN_FOR  */
  YYSYMBOL_SQL_TOKEN_MATCH = 103,          /* SQL_TOKEN_MATCH  */
  YYSYMBOL_SQL_TOKEN_ECSQLOPTIONS = 104,   /* SQL_TOKEN_ECSQLOPTIONS  */
  YYSYMBOL_SQL_TOKEN_INTEGER = 105,        /* SQL_TOKEN_INTEGER  */
  YYSYMBOL_SQL_TOKEN_INT = 106,            /* SQL_TOKEN_INT  */
  YYSYMBOL_SQL_TOKEN_INT64 = 107,          /* SQL_TOKEN_INT64  */
  YYSYMBOL_SQL_TOKEN_LONG = 108,           /* SQL_TOKEN_LONG  */
  YYSYMBOL_SQL_TOKEN_BOOLEAN = 109,        /* SQL_TOKEN_BOOLEAN  */
  YYSYMBOL_SQL_TOKEN_DOUBLE = 110,         /* SQL_TOKEN_DOUBLE  */
  YYSYMBOL_SQL_TOKEN_REAL = 111,           /* SQL_TOKEN_REAL  */
  YYSYMBOL_SQL_TOKEN_FLOAT = 112,          /* SQL_TOKEN_FLOAT  */
  YYSYMBOL_SQL_TOKEN_STRING = 113,         /* SQL_TOKEN_STRING  */
  YYSYMBOL_SQL_TOKEN_VARCHAR = 114,        /* SQL_TOKEN_VARCHAR  */
  YYSYMBOL_SQL_TOKEN_BINARY = 115,         /* SQL_TOKEN_BINARY  */
  YYSYMBOL_SQL_TOKEN_BLOB = 116,           /* SQL_TOKEN_BLOB  */
  YYSYMBOL_SQL_TOKEN_DATE = 117,           /* SQL_TOKEN_DATE  */
  YYSYMBOL_SQL_TOKEN_TIME = 118,           /* SQL_TOKEN_TIME  */
  YYSYMBOL_SQL_TOKEN_TIMESTAMP = 119,      /* SQL_TOKEN_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_OVER = 120,           /* SQL_TOKEN_OVER  */
  YYSYMBOL_SQL_TOKEN_ROW_NUMBER = 121,     /* SQL_TOKEN_ROW_NUMBER  */
  YYSYMBOL_SQL_TOKEN_NTILE = 122,          /* SQL_TOKEN_NTILE  */
  YYSYMBOL_SQL_TOKEN_LEAD = 123,           /* SQL_TOKEN_LEAD  */
  YYSYMBOL_SQL_TOKEN_LAG = 124,            /* SQL_TOKEN_LAG  */
  YYSYMBOL_SQL_TOKEN_FIRST_VALUE = 125,    /* SQL_TOKEN_FIRST_VALUE  */
  YYSYMBOL_SQL_TOKEN_LAST_VALUE = 126,     /* SQL_TOKEN_LAST_VALUE  */
  YYSYMBOL_SQL_TOKEN_NTH_VALUE = 127,      /* SQL_TOKEN_NTH_VALUE  */
  YYSYMBOL_SQL_TOKEN_EXCLUDE = 128,        /* SQL_TOKEN_EXCLUDE  */
  YYSYMBOL_SQL_TOKEN_OTHERS = 129,         /* SQL_TOKEN_OTHERS  */
  YYSYMBOL_SQL_TOKEN_TIES = 130,           /* SQL_TOKEN_TIES  */
  YYSYMBOL_SQL_TOKEN_FOLLOWING = 131,      /* SQL_TOKEN_FOLLOWING  */
  YYSYMBOL_SQL_TOKEN_UNBOUNDED = 132,      /* SQL_TOKEN_UNBOUNDED  */
  YYSYMBOL_SQL_TOKEN_PRECEDING = 133,      /* SQL_TOKEN_PRECEDING  */
  YYSYMBOL_SQL_TOKEN_RANGE = 134,          /* SQL_TOKEN_RANGE  */
  YYSYMBOL_SQL_TOKEN_ROWS = 135,           /* SQL_TOKEN_ROWS  */
  YYSYMBOL_SQL_TOKEN_PARTITION = 136,      /* SQL_TOKEN_PARTITION  */
  YYSYMBOL_SQL_TOKEN_WINDOW = 137,         /* SQL_TOKEN_WINDOW  */
  YYSYMBOL_SQL_TOKEN_NO = 138,             /* SQL_TOKEN_NO  */
  YYSYMBOL_SQL_TOKEN_CURRENT = 139,        /* SQL_TOKEN_CURRENT  */
  YYSYMBOL_SQL_TOKEN_ROW = 140,            /* SQL_TOKEN_ROW  */
  YYSYMBOL_SQL_TOKEN_RANK = 141,           /* SQL_TOKEN_RANK  */
  YYSYMBOL_SQL_TOKEN_DENSE_RANK = 142,     /* SQL_TOKEN_DENSE_RANK  */
  YYSYMBOL_SQL_TOKEN_PERCENT_RANK = 143,   /* SQL_TOKEN_PERCENT_RANK  */
  YYSYMBOL_SQL_TOKEN_CUME_DIST = 144,      /* SQL_TOKEN_CUME_DIST  */
  YYSYMBOL_SQL_TOKEN_COLLATE = 145,        /* SQL_TOKEN_COLLATE  */
  YYSYMBOL_SQL_TOKEN_NOCASE = 146,         /* SQL_TOKEN_NOCASE  */
  YYSYMBOL_SQL_TOKEN_RTRIM = 147,          /* SQL_TOKEN_RTRIM  */
  YYSYMBOL_SQL_TOKEN_FILTER = 148,         /* SQL_TOKEN_FILTER  */
  YYSYMBOL_SQL_TOKEN_GROUPS = 149,         /* SQL_TOKEN_GROUPS  */
  YYSYMBOL_SQL_TOKEN_GROUP_CONCAT = 150,   /* SQL_TOKEN_GROUP_CONCAT  */
  YYSYMBOL_SQL_TOKEN_OR = 151,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 152,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_ARROW = 153,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 154,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 155,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 156,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 157,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 158,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 159,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 160,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 161,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 162,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 163,                /* SQL_EQUAL  */
  YYSYMBOL_164_ = 164,                     /* '+'  */
  YYSYMBOL_165_ = 165,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 166,               /* SQL_CONCAT  */
  YYSYMBOL_167_ = 167,                     /* '*'  */
  YYSYMBOL_168_ = 168,                     /* '/'  */
  YYSYMBOL_169_ = 169,                     /* '%'  */
  YYSYMBOL_170_ = 170,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 171,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 172,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 173,     /* sql_single_statement  */
  YYSYMBOL_sql = 174,                      /* sql  */
  YYSYMBOL_pragma = 175,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 176,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 177,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 178,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 179,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 180,             /* pragma_value  */
  YYSYMBOL_pragma_path = 181,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 182,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 183,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 184,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 185,           /* cte_block_list  */
  YYSYMBOL_cte = 186,                      /* cte  */
  YYSYMBOL_column_commalist = 187,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 188,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 189,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 190, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 191,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 192,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 193,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 194,             /* opt_asc_desc  */
  YYSYMBOL_opt_null_order = 195,           /* opt_null_order  */
  YYSYMBOL_first_last_desc = 196,          /* first_last_desc  */
  YYSYMBOL_sql_not = 197,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 198,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 199,         /* select_statement  */
  YYSYMBOL_union_op = 200,                 /* union_op  */
  YYSYMBOL_delete_statement_searched = 201, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 202,         /* insert_statement  */
  YYSYMBOL_values_or_query_spec = 203,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 204, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 205,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 206, /* row_value_constructor_elem  */
  YYSYMBOL_opt_all_distinct = 207,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 208,     /* assignment_commalist  */
  YYSYMBOL_assignment = 209,               /* assignment  */
  YYSYMBOL_update_source = 210,            /* update_source  */
  YYSYMBOL_update_statement_searched = 211, /* update_statement_searched  */
  YYSYMBOL_opt_where_clause = 212,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 213,  /* single_select_statement  */
  YYSYMBOL_selection = 214,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 215,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 216,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 217,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 218,                /* table_exp  */
  YYSYMBOL_from_clause = 219,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 220,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 221,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 222, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 223, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 224,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 225, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 226,                /* table_ref  */
  YYSYMBOL_where_clause = 227,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 228,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 229,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 230,              /* truth_value  */
  YYSYMBOL_boolean_primary = 231,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 232,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 233,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 234,             /* boolean_term  */
  YYSYMBOL_search_condition = 235,         /* search_condition  */
  YYSYMBOL_type_predicate = 236,           /* type_predicate  */
  YYSYMBOL_type_list = 237,                /* type_list  */
  YYSYMBOL_type_list_item = 238,           /* type_list_item  */
  YYSYMBOL_predicate = 239,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 240, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 241,     /* comparison_predicate  */
  YYSYMBOL_comparison = 242,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 243, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 244,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 245, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 246, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 247,           /* like_predicate  */
  YYSYMBOL_opt_escape = 248,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 249,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 250,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 251,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 252,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 253,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 254, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 255,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 256,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 257, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 258,             /* any_all_some  */
  YYSYMBOL_existence_test = 259,           /* existence_test  */
  YYSYMBOL_unique_test = 260,              /* unique_test  */
  YYSYMBOL_subquery = 261,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 262,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 263,           /* select_sublist  */
  YYSYMBOL_literal = 264,                  /* literal  */
  YYSYMBOL_as_clause = 265,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 266,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 267,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 268,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 269,                 /* fct_spec  */
  YYSYMBOL_function_name = 270,            /* function_name  */
  YYSYMBOL_aggregate_fct = 271,            /* aggregate_fct  */
  YYSYMBOL_opt_function_arg = 272,         /* opt_function_arg  */
  YYSYMBOL_set_fct_type = 273,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 274,          /* outer_join_type  */
  YYSYMBOL_join_condition = 275,           /* join_condition  */
  YYSYMBOL_join_spec = 276,                /* join_spec  */
  YYSYMBOL_join_type = 277,                /* join_type  */
  YYSYMBOL_cross_union = 278,              /* cross_union  */
  YYSYMBOL_qualified_join = 279,           /* qualified_join  */
  YYSYMBOL_window_function = 280,          /* window_function  */
  YYSYMBOL_window_function_type = 281,     /* window_function_type  */
  YYSYMBOL_ntile_function = 282,           /* ntile_function  */
  YYSYMBOL_opt_lead_or_lag_function = 283, /* opt_lead_or_lag_function  */
  YYSYMBOL_lead_or_lag_function = 284,     /* lead_or_lag_function  */
  YYSYMBOL_lead_or_lag = 285,              /* lead_or_lag  */
  YYSYMBOL_lead_or_lag_extent = 286,       /* lead_or_lag_extent  */
  YYSYMBOL_first_or_last_value_function = 287, /* first_or_last_value_function  */
  YYSYMBOL_first_or_last_value = 288,      /* first_or_last_value  */
  YYSYMBOL_nth_value_function = 289,       /* nth_value_function  */
  YYSYMBOL_opt_filter_clause = 290,        /* opt_filter_clause  */
  YYSYMBOL_window_name = 291,              /* window_name  */
  YYSYMBOL_window_name_or_specification = 292, /* window_name_or_specification  */
  YYSYMBOL_in_line_window_specification = 293, /* in_line_window_specification  */
  YYSYMBOL_opt_window_clause = 294,        /* opt_window_clause  */
  YYSYMBOL_window_definition_list = 295,   /* window_definition_list  */
  YYSYMBOL_window_definition = 296,        /* window_definition  */
  YYSYMBOL_new_window_name = 297,          /* new_window_name  */
  YYSYMBOL_window_specification = 298,     /* window_specification  */
  YYSYMBOL_opt_existing_window_name = 299, /* opt_existing_window_name  */
  YYSYMBOL_existing_window_name = 300,     /* existing_window_name  */
  YYSYMBOL_opt_window_partition_clause = 301, /* opt_window_partition_clause  */
  YYSYMBOL_opt_window_frame_clause = 302,  /* opt_window_frame_clause  */
  YYSYMBOL_window_partition_column_reference_list = 303, /* window_partition_column_reference_list  */
  YYSYMBOL_window_partition_column_reference = 304, /* window_partition_column_reference  */
  YYSYMBOL_opt_window_frame_exclusion = 305, /* opt_window_frame_exclusion  */
  YYSYMBOL_window_frame_units = 306,       /* window_frame_units  */
  YYSYMBOL_window_frame_extent = 307,      /* window_frame_extent  */
  YYSYMBOL_window_frame_start = 308,       /* window_frame_start  */
  YYSYMBOL_window_frame_preceding = 309,   /* window_frame_preceding  */
  YYSYMBOL_window_frame_between = 310,     /* window_frame_between  */
  YYSYMBOL_window_frame_bound = 311,       /* window_frame_bound  */
  YYSYMBOL_window_frame_bound_1 = 312,     /* window_frame_bound_1  */
  YYSYMBOL_window_frame_bound_2 = 313,     /* window_frame_bound_2  */
  YYSYMBOL_window_frame_following = 314,   /* window_frame_following  */
  YYSYMBOL_rank_function_type = 315,       /* rank_function_type  */
  YYSYMBOL_opt_collate_clause = 316,       /* opt_collate_clause  */
  YYSYMBOL_collating_function = 317,       /* collating_function  */
  YYSYMBOL_ecrelationship_join = 318,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 319, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 320,             /* joined_table  */
  YYSYMBOL_named_columns_join = 321,       /* named_columns_join  */
  YYSYMBOL_all = 322,                      /* all  */
  YYSYMBOL_scalar_subquery = 323,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 324,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 325, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 326,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 327,        /* cast_target_array  */
  YYSYMBOL_cast_target = 328,              /* cast_target  */
  YYSYMBOL_cast_spec = 329,                /* cast_spec  */
  YYSYMBOL_opt_optional_prop = 330,        /* opt_optional_prop  */
  YYSYMBOL_opt_extract_value = 331,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 332,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 333,              /* num_primary  */
  YYSYMBOL_factor = 334,                   /* factor  */
  YYSYMBOL_term = 335,                     /* term  */
  YYSYMBOL_term_add_sub = 336,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 337,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 338,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 339,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 340,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 341,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 342,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 343,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 344,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 345,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 346,                /* value_exp  */
  YYSYMBOL_string_value_exp = 347,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 348,           /* char_value_exp  */
  YYSYMBOL_concatenation = 349,            /* concatenation  */
  YYSYMBOL_char_primary = 350,             /* char_primary  */
  YYSYMBOL_char_factor = 351,              /* char_factor  */
  YYSYMBOL_derived_column = 352,           /* derived_column  */
  YYSYMBOL_table_node = 353,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 354, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 355,     /* qualified_class_name  */
  YYSYMBOL_class_name = 356,               /* class_name  */
  YYSYMBOL_table_node_ref = 357,           /* table_node_ref  */
  YYSYMBOL_table_node_with_opt_member_func_call = 358, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 359,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 360,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 361, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 362,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 363,            /* property_path  */
  YYSYMBOL_property_path_entry = 364,      /* property_path_entry  */
  YYSYMBOL_column_ref = 365,               /* column_ref  */
  YYSYMBOL_column = 366,                   /* column  */
  YYSYMBOL_case_expression = 367,          /* case_expression  */
  YYSYMBOL_case_specification = 368,       /* case_specification  */
  YYSYMBOL_simple_case = 369,              /* simple_case  */
  YYSYMBOL_searched_case = 370,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 371,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 372,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 373,        /* when_operand_list  */
  YYSYMBOL_when_operand = 374,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 375, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 376,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 377,              /* else_clause  */
  YYSYMBOL_result = 378,                   /* result  */
  YYSYMBOL_result_expression = 379,        /* result_expression  */
  YYSYMBOL_case_operand = 380,             /* case_operand  */
  YYSYMBOL_parameter = 381,                /* parameter  */
  YYSYMBOL_range_variable = 382,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 383,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 384,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 385,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 386,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 387          /* ecsqloptionvalue  */
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
#define YYFINAL  37
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3010

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  172
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  216
/* YYNRULES -- Number of rules.  */
#define YYNRULES  468
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  731

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   404


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
       2,     2,     2,     2,     2,     2,     2,   169,     2,     2,
       3,     4,   167,   164,     5,   165,    13,   168,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   170,     2,     8,     2,     2,     2,     2,     2,     2,
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
     161,   162,   163,   166,   171
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   238,   238,   239,   240,   241,   245,   251,   263,   266,
     274,   277,   278,   282,   290,   298,   299,   300,   301,   302,
     303,   304,   308,   313,   318,   330,   333,   337,   342,   350,
     364,   370,   378,   390,   395,   403,   408,   418,   419,   427,
     428,   439,   440,   450,   455,   463,   472,   483,   484,   485,
     489,   490,   499,   499,   503,   504,   510,   511,   512,   513,
     514,   518,   523,   534,   535,   536,   541,   553,   562,   572,
     577,   585,   588,   592,   593,   594,   598,   601,   607,   614,
     617,   629,   630,   638,   646,   650,   655,   658,   659,   662,
     663,   671,   680,   681,   696,   706,   709,   715,   716,   720,
     723,   733,   734,   741,   742,   747,   755,   756,   763,   771,
     779,   787,   791,   800,   801,   811,   812,   820,   821,   822,
     823,   824,   827,   828,   829,   830,   831,   832,   833,   843,
     844,   854,   855,   863,   864,   873,   874,   884,   893,   898,
     906,   915,   916,   917,   918,   919,   920,   921,   922,   923,
     929,   936,   943,   968,   969,   970,   971,   972,   973,   974,
     982,   992,  1000,  1010,  1020,  1026,  1032,  1054,  1079,  1080,
    1087,  1096,  1102,  1118,  1122,  1130,  1139,  1145,  1161,  1170,
    1179,  1188,  1198,  1199,  1200,  1204,  1212,  1218,  1229,  1234,
    1241,  1245,  1246,  1247,  1248,  1249,  1251,  1263,  1275,  1287,
    1303,  1304,  1310,  1314,  1315,  1318,  1319,  1320,  1321,  1324,
    1336,  1337,  1338,  1345,  1353,  1365,  1366,  1371,  1387,  1403,
    1412,  1420,  1429,  1447,  1448,  1457,  1458,  1459,  1460,  1461,
    1462,  1466,  1471,  1476,  1483,  1491,  1492,  1495,  1496,  1501,
    1502,  1510,  1522,  1532,  1541,  1545,  1556,  1563,  1570,  1571,
    1572,  1573,  1574,  1578,  1589,  1590,  1596,  1607,  1619,  1620,
    1624,  1629,  1640,  1641,  1645,  1659,  1660,  1670,  1674,  1675,
    1679,  1684,  1685,  1694,  1697,  1703,  1713,  1717,  1735,  1736,
    1740,  1745,  1746,  1757,  1758,  1768,  1771,  1777,  1787,  1788,
    1795,  1801,  1807,  1818,  1819,  1820,  1824,  1825,  1829,  1835,
    1842,  1851,  1860,  1871,  1872,  1878,  1882,  1883,  1892,  1893,
    1902,  1911,  1912,  1913,  1914,  1919,  1920,  1929,  1930,  1931,
    1935,  1949,  1950,  1951,  1954,  1955,  1958,  1970,  1971,  1975,
    1978,  1982,  1983,  1984,  1985,  1986,  1987,  1988,  1989,  1990,
    1991,  1992,  1993,  1994,  1995,  1996,  1997,  2001,  2006,  2016,
    2025,  2026,  2030,  2042,  2043,  2050,  2051,  2059,  2060,  2061,
    2062,  2063,  2064,  2065,  2072,  2076,  2080,  2081,  2087,  2093,
    2102,  2103,  2110,  2117,  2127,  2128,  2135,  2145,  2146,  2153,
    2160,  2167,  2177,  2184,  2189,  2194,  2199,  2205,  2211,  2220,
    2227,  2235,  2243,  2246,  2252,  2256,  2261,  2269,  2270,  2271,
    2275,  2279,  2280,  2283,  2290,  2300,  2303,  2307,  2317,  2322,
    2329,  2338,  2346,  2355,  2363,  2372,  2380,  2385,  2390,  2398,
    2407,  2408,  2418,  2419,  2427,  2432,  2450,  2456,  2461,  2469,
    2481,  2485,  2489,  2490,  2494,  2505,  2515,  2520,  2527,  2537,
    2540,  2545,  2546,  2547,  2548,  2549,  2550,  2553,  2558,  2565,
    2575,  2576,  2584,  2588,  2592,  2596,  2602,  2610,  2613,  2623,
    2624,  2628,  2637,  2642,  2650,  2656,  2666,  2667,  2668
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
  "SQL_TOKEN_FILTER", "SQL_TOKEN_GROUPS", "SQL_TOKEN_GROUP_CONCAT",
  "SQL_TOKEN_OR", "SQL_TOKEN_AND", "SQL_ARROW", "SQL_BITWISE_OR",
  "SQL_BITWISE_AND", "SQL_BITWISE_SHIFT_LEFT", "SQL_BITWISE_SHIFT_RIGHT",
  "SQL_LESSEQ", "SQL_GREATEQ", "SQL_NOTEQUAL", "SQL_LESS", "SQL_GREAT",
  "SQL_EQUAL", "'+'", "'-'", "SQL_CONCAT", "'*'", "'/'", "'%'", "'='",
  "SQL_TOKEN_INVALIDSYMBOL", "$accept", "sql_single_statement", "sql",
  "pragma", "opt_pragma_for", "opt_pragma_set", "opt_pragma_set_val",
  "opt_pragma_func", "pragma_value", "pragma_path", "opt_cte_recursive",
  "cte_column_list", "cte_table_name", "cte_block_list", "cte",
  "column_commalist", "column_ref_commalist", "opt_column_commalist",
  "opt_column_ref_commalist", "opt_order_by_clause",
  "ordering_spec_commalist", "ordering_spec", "opt_asc_desc",
  "opt_null_order", "first_last_desc", "sql_not", "manipulative_statement",
  "select_statement", "union_op", "delete_statement_searched",
  "insert_statement", "values_or_query_spec",
  "row_value_constructor_commalist", "row_value_constructor",
  "row_value_constructor_elem", "opt_all_distinct", "assignment_commalist",
  "assignment", "update_source", "update_statement_searched",
  "opt_where_clause", "single_select_statement", "selection",
  "opt_limit_offset_clause", "opt_offset", "limit_offset_clause",
  "table_exp", "from_clause", "table_ref_commalist", "opt_as",
  "table_primary_as_range_column", "opt_disqualify_polymorphic_constraint",
  "opt_only", "opt_disqualify_primary_join", "table_ref", "where_clause",
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
  "aggregate_fct", "opt_function_arg", "set_fct_type", "outer_join_type",
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

#define YYPACT_NINF (-579)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-447)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     635,    92,    28,    33,    73,    41,   123,   127,   154,   169,
     182,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   159,  -579,
     179,    41,   203,  -579,  -579,  2368,   229,    44,    14,   243,
     260,  -579,  -579,  -579,  -579,  2517,    27,  -579,  -579,  -579,
    -579,  -579,  -579,   246,   268,  -579,    55,    29,   108,   325,
    -579,  -579,  1623,   312,  -579,  -579,  -579,  -579,  -579,    74,
    -579,  -579,   336,   373,  -579,   380,   383,  -579,  -579,   386,
    -579,  -579,  -579,  -579,  2815,  -579,  -579,  -579,  -579,  2070,
    -579,  -579,  2517,  2517,  2517,   388,   396,  -579,  -579,  -579,
    -579,   405,  -579,  -579,  -579,  -579,  -579,   418,  2815,  2815,
     251,   327,  -579,   424,  -579,    87,  -579,  -579,  -579,  -579,
     428,   -44,   433,  -579,   290,  -579,  -579,   440,  -579,   442,
    -579,   444,  -579,  -579,  -579,  -579,  -579,   125,     9,   261,
    -579,  -579,  -579,  -579,  -579,    31,  -579,   294,  -579,  -579,
    -579,  -579,    11,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,   143,  -579,   243,   215,   457,   215,   283,  -579,
     401,  -579,  -579,  -579,  -579,   135,    13,   399,   431,  -579,
     341,  -579,  -579,   302,   319,   319,   374,  -579,  -579,  -579,
     143,   469,   179,  -579,   878,   391,  -579,   472,   473,    13,
     413,   480,    17,  -579,  -579,  -579,  2517,    43,    73,    73,
     878,  -579,   878,  -579,   221,  -579,   419,   302,  -579,  -579,
    -579,   513,  2517,  2517,    73,  -579,  -579,    41,  -579,   432,
    2517,  -579,  -579,  -579,  -579,  1325,    73,   516,   402,  2517,
    2517,   517,  2666,  2666,  2666,  2666,  2666,  2666,  2666,  2666,
    2666,  -579,   501,  2517,  -579,  -579,   414,    13,    13,  -579,
     215,  -579,   502,   325,  2517,  -579,   504,  -579,   243,   243,
      41,   489,   526,    37,  -579,   392,  -579,    41,  -579,  2517,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,   548,  -579,   533,
     391,  -579,  -579,   370,  -579,   580,   729,   555,   536,   555,
    -579,  -579,  -579,  -579,  -579,  -579,   242,    63,   503,  -579,
    -579,   408,   410,  -579,  -579,  2517,  -579,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   197,   345,
     375,   147,  2841,   542,  -579,  -579,  -579,  -579,   313,  -579,
    -579,   376,  -579,  -579,  -579,  -579,   537,   302,   563,  2517,
    2517,  2517,    21,   -16,  2517,  -579,   474,   878,   476,  -579,
     419,  -579,   566,   302,  -579,  -579,   573,  2517,   574,   570,
     527,  -579,  -579,  2517,  -579,   389,  2517,   432,   134,   577,
    -579,   581,  -579,  -579,  -579,  -579,   125,   125,     9,     9,
       9,     9,  -579,  -579,  -579,  -579,   307,  -579,  -579,  -579,
     407,   584,  -579,  -579,   570,    41,    13,   391,  2517,   403,
    -579,  -579,  -579,   331,  -579,   558,   567,    18,    16,  -579,
    -579,  -579,   524,  -579,   590,  2517,    93,  2219,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,   536,   878,   878,  -579,   434,
     542,  -579,   472,  -579,    13,  2861,  -579,   591,   415,   423,
    2517,  2517,  -579,  -579,   180,    51,  -579,  2517,  -579,   589,
     602,   603,    62,  -579,   514,  -579,  -579,  2517,   607,    41,
     579,   561,   612,  -579,  2517,   613,   614,   597,  -579,  -579,
    -579,  -579,  -579,  2517,   619,  -579,  -579,  -579,  -579,   501,
    -579,   570,  -579,  -579,  -579,   302,   878,   213,  -579,  -579,
    -579,   600,   604,   629,  -579,  -579,  -579,  1623,  -579,  -579,
     285,    -4,  2517,    65,  -579,  -579,  -579,  -579,   555,   253,
    -579,   408,   762,  -579,  -579,   622,  -579,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,   611,  -579,   636,  -579,  -579,  -579,   637,  -579,
      66,  1176,  2517,  -579,   639,  2517,   640,   570,  2517,   878,
     510,  -579,  -579,  -579,  -579,  -579,   505,  -579,   645,  -579,
     430,  -579,   410,   501,   411,   215,  -579,  -579,   143,   460,
     302,  2517,  -579,  -579,   499,  -579,  -579,  -579,  -579,     1,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,    87,  -579,
     631,  -579,  -579,  2517,   223,  -579,  2517,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,   656,   410,   597,   592,   641,
     592,  2517,  -579,   501,   470,  -579,  -579,  -579,  -579,   660,
    -579,  2517,   500,  2517,  -579,   203,   475,  -579,  -579,   670,
    2517,  -579,  -579,   673,  -579,   644,   652,   593,    13,   149,
    -579,  -579,  -579,  -579,   302,  -579,  -579,  -579,     1,  -579,
     597,   676,  1027,  2666,   391,  -579,   683,  -579,   544,  -579,
    -579,  -579,   690,  1474,  -579,  -579,  -579,   691,  -579,   522,
     202,   249,  -579,    13,    86,  -579,  -579,  1772,   562,   560,
     582,  -579,  -579,   -58,  1027,  -579,  -579,   672,   672,  2666,
    -579,  -579,  -579,  -579,  -579,  -579,   578,   572,  -579,  -579,
     564,  -579,   -30,  -579,  -579,   110,  -579,  -579,  -579,   445,
    -579,  -579,   261,  -579,  -579,  1921,  -579,  -579,  -579,  -579,
     585,   575,  -579,  -579,  -579,   587,  -579,  -579,  -579,  -579,
    -579
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,     0,     0,    73,   101,     0,     0,     0,     2,
       4,    60,     6,    59,    56,    57,    84,    58,    61,    26,
       0,   101,     0,    74,    75,     0,   102,     0,   106,     0,
     237,   244,   325,   324,   111,     0,    10,     1,     3,     5,
      65,    63,    64,   327,     0,    31,     0,    81,     0,    39,
     409,   408,     0,     0,   456,   195,   192,   193,   194,   422,
     228,   225,     0,     0,   207,     0,     0,   206,   230,     0,
     229,   226,   208,   428,     0,   383,   385,   384,   227,     0,
     191,   405,     0,     0,     0,     0,     0,   258,   259,   262,
     263,     0,   311,   312,   313,   314,   216,     0,     0,     0,
     427,    92,   329,    86,   188,   204,   357,   203,   211,   358,
       0,   210,     0,   362,   265,   249,   250,     0,   251,     0,
     252,     0,   360,   364,   365,   366,   370,   374,   377,   397,
     389,   382,   390,   391,   399,   200,   398,   400,   402,   406,
     401,   190,   355,   424,   359,   361,   431,   432,   433,   205,
     105,   104,     0,   107,     0,   457,   420,    99,   415,   416,
       0,   233,   238,   231,   232,   237,     0,   239,     0,   427,
       0,    69,    71,    72,     0,     0,     8,    11,    12,   328,
       0,     0,     0,    32,    54,   459,    82,     0,     0,     0,
       0,     0,     0,   455,   423,   426,     0,    73,    73,    73,
      54,   369,    54,   454,   450,   447,     0,     0,   386,   388,
     387,     0,     0,     0,    73,   368,   367,   101,    83,    81,
       0,   198,   199,   197,   196,     0,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   430,     0,     0,   407,   202,     0,     0,     0,   429,
      99,    98,     0,    39,     0,   419,     0,   109,     0,     0,
     101,     0,   422,    81,    76,     0,   240,   101,    68,     0,
      15,    16,    17,    20,    21,    19,    18,     0,    13,     0,
     459,    62,    28,     0,    30,    54,    54,     0,    54,     0,
     157,   158,   154,   153,   156,   155,     0,    54,   129,   131,
     133,   135,   112,   122,   141,     0,   142,   166,   167,   148,
     172,   146,   177,   147,   143,   149,   144,   145,   123,   124,
     126,   127,   125,     0,    66,   460,   413,   412,   413,   410,
     411,     0,    36,    67,   187,   363,     0,   330,     0,     0,
       0,     0,     0,     0,     0,   448,     0,    54,   450,   436,
       0,   247,     0,   453,   394,   452,     0,     0,    94,    95,
     113,   189,   212,     0,   395,     0,     0,     0,     0,   254,
     260,     0,   246,   371,   372,   373,   375,   376,   380,   381,
     378,   379,   201,   404,   403,   425,   353,   108,   458,   110,
       0,    37,   418,   417,   241,   101,     0,   459,     0,   237,
      70,    14,    22,     9,     7,     0,     0,     0,    72,    55,
     132,   185,   159,   186,     0,     0,     0,     0,   161,   164,
     165,   171,   176,   179,   180,    54,    54,    54,   152,   464,
     461,   463,     0,    40,     0,     0,   220,     0,     0,     0,
       0,     0,   451,   435,     0,    71,   442,     0,   443,   166,
     172,   177,     0,   439,     0,   437,   253,     0,   223,   101,
       0,   115,     0,   213,     0,     0,     0,   278,   267,   268,
     245,   269,   270,     0,     0,   261,   354,   356,   421,     0,
     100,   242,    77,    80,    78,    79,    54,     0,   235,   243,
     236,     0,     0,     0,    27,   128,   170,     0,   175,   173,
     168,   168,     0,     0,   183,   182,   184,   151,     0,     0,
     134,   136,     0,   462,    35,   346,   337,   336,   338,   339,
     333,   334,   340,   335,   341,   345,   331,   332,   342,   343,
     344,   347,   350,   351,     0,   221,   217,   218,     0,   449,
     152,    54,     0,   434,     0,     0,     0,    96,     0,    54,
     271,   214,   396,   219,   266,   280,   281,   279,   255,   257,
       0,    34,   234,     0,   321,    99,    24,    23,     0,     0,
     392,     0,   163,   162,     0,   215,   181,   210,   178,   101,
     118,   120,   117,   119,   130,   121,   467,   468,   466,   465,
       0,   349,   352,     0,     0,   441,     0,   445,   446,   444,
     440,   438,   264,   224,   222,   114,   116,     0,    41,     0,
      41,     0,    38,     0,     0,   322,   323,   320,   414,     0,
     174,     0,   169,     0,   102,     0,     0,   138,   348,     0,
       0,   150,   276,   272,   274,     0,     0,    87,     0,   283,
     256,    33,   326,    29,   393,   160,   140,   137,   101,   209,
       0,     0,    54,     0,   459,    88,   282,   286,   315,   294,
     293,   295,     0,     0,   139,   273,   275,    42,    43,    47,
      47,    89,    93,     0,     0,   287,   277,     0,     0,     0,
     288,   296,   297,     0,    54,    48,    49,    50,    50,     0,
      91,   285,   317,   318,   319,   316,     0,     0,   303,   306,
       0,   305,     0,   298,   300,     0,   284,   299,    44,     0,
      46,    45,    90,   307,   304,     0,   310,   301,   290,   291,
       0,     0,    52,    53,    51,     0,   308,   302,   292,   289,
     309
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   538,  -579,
    -579,  -579,   543,  -579,  -579,   156,  -579,  -579,   478,   116,
    -579,    45,    58,    46,  -579,  -259,  -579,     4,  -579,  -579,
    -579,   556,  -579,   -33,   -78,   350,  -579,   337,  -579,  -579,
    -166,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   599,
    -236,  -579,  -495,   727,   -13,   390,  -579,  -579,   244,  -579,
     481,   332,   333,  -172,  -579,  -579,   113,  -578,  -579,  -579,
    -279,   467,  -579,  -278,   477,  -579,   265,  -265,  -579,  -579,
    -264,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   -19,
    -579,   550,   259,  -579,  -139,  -579,  -579,  -161,  -579,   269,
    -579,  -579,  -579,  -579,  -579,   608,  -579,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -328,
    -579,  -579,  -579,  -579,   126,  -579,   124,  -579,  -579,  -579,
    -579,  -579,   104,  -579,  -579,  -579,  -579,  -579,  -579,    70,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -136,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
     365,    20,   138,   252,   186,  -573,  -579,  -579,  -579,  -579,
    -579,   239,  -178,  -111,   -25,   -77,  -579,  -579,  -579,   545,
    -579,   164,  -579,   605,   609,  -579,  -138,  -579,   232,  -579,
    -579,   551,   553,  -153,  -132,  -579,  -579,  -579,  -579,  -579,
     451,  -579,   262,   586,  -158,   456,  -329,  -579,  -579,  -579,
    -579,  -269,  -579,  -579,   360,  -579
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     8,     9,    10,   280,   176,   177,   178,   277,   403,
      20,   283,    45,    46,    11,   560,   331,   480,   190,   637,
     667,   668,   687,   710,   724,   296,    12,   191,    43,    14,
      15,    16,   170,   297,   172,    25,   263,   264,   484,    17,
     185,    18,   101,   654,   690,   655,   218,   219,   358,   256,
     257,    27,    28,    29,    30,   186,   461,   550,   584,   298,
     299,   300,   301,   343,   585,   626,   627,   303,   446,   304,
     305,   448,   306,   307,   308,   309,   572,   310,   311,   498,
     312,   313,   423,   314,   315,   424,   508,   316,   317,   102,
     103,   104,   105,   244,   106,   107,   108,   109,   110,   111,
     546,   112,   167,   488,   489,   168,    31,    32,   113,   114,
     115,   474,   116,   117,   369,   118,   119,   120,   228,   632,
     470,   471,   608,   633,   634,   635,   472,   556,   557,   610,
     662,   656,   657,   706,   663,   680,   681,   698,   682,   699,
     700,   727,   701,   121,   675,   695,    33,   617,    34,   490,
     180,   122,   336,   531,   532,   533,   534,   123,   477,   249,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   569,   364,   365,   353,   136,   137,   138,   139,   140,
     141,    49,    50,    51,   330,   564,   157,   158,   159,   255,
     195,   142,   143,   144,   561,   145,   146,   147,   148,   348,
     349,   452,   453,   204,   205,   346,   354,   355,   206,   149,
     253,   324,   325,   430,   431,   589
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     135,   203,   171,   245,    13,   208,   209,   210,    47,   155,
     173,   404,   302,   265,   387,   442,   250,   152,   417,   419,
     335,   335,   495,   319,   247,  -103,   440,   192,   342,   412,
     174,   322,   421,   422,   352,   356,   332,   262,   416,   319,
     469,   319,   396,   571,  -103,   318,   345,   322,   320,   322,
     183,   370,   371,   360,   173,   241,  -441,   207,   207,   207,
     182,   318,   242,   318,   320,  -103,   320,   541,   447,   449,
     160,  -150,    23,   150,   670,   707,  -248,  -215,   441,    21,
     671,   161,   450,   451,   625,   162,   409,    24,   444,   575,
    -237,    22,   163,   164,   201,    60,   165,   397,    61,   194,
      73,   716,    23,   717,  -248,    63,   670,   221,   243,   222,
     382,   538,   539,   407,   187,   184,   712,    24,   215,   216,
      19,   188,   288,   184,   319,   319,    35,   502,   483,     4,
      65,    66,   322,   322,    68,   427,   243,   467,    69,   555,
       6,    70,    71,   390,   151,  -441,   318,   318,   414,   320,
     320,    36,   -72,   625,    37,   415,   542,    78,   468,   321,
    -150,   437,  -398,   718,   248,   624,   509,  -127,  -127,   427,
     -72,   337,   427,   235,   236,   321,    38,   321,   153,   458,
     169,   -72,   243,   243,   281,   462,   319,   161,   465,    39,
     175,   162,   345,   223,   322,   135,   503,   243,   163,   164,
     224,   692,   -72,    44,   359,    26,   -72,    40,   318,   -72,
     338,   320,    96,   601,   502,    97,   563,     4,   383,    41,
    -357,   290,   291,   292,   293,   294,   295,    48,     6,   438,
     439,  -357,   693,   694,   685,   414,   400,   156,    42,   -97,
     719,   -72,   415,   265,   173,   686,   251,   394,   720,   721,
     -72,   -85,  -357,  -107,   399,   -85,   579,   502,   -85,  -357,
     408,   321,   596,   597,   629,   319,   319,   156,   411,   445,
     413,   181,   428,   322,   322,   179,   598,   599,   414,   544,
     173,   514,   594,   659,   660,   630,   552,   318,   318,   258,
     320,   320,   232,   233,   234,   558,   259,   414,   661,   -85,
    -357,   160,   -85,   580,   415,   -72,   -72,   -72,   -72,   -72,
     -72,   -85,   161,   243,   562,   476,   162,   202,   344,   187,
     247,   581,   321,   163,   164,   319,   432,   165,   189,   618,
     -85,   582,   571,   322,   583,   166,   193,   491,   501,   196,
     270,   271,   576,   272,   492,   268,   269,   318,   689,   565,
     320,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,   603,  -358,   273,
     373,   374,   375,   485,   405,   406,   197,   606,   217,  -358,
     433,   434,   481,   198,   507,   672,   199,   274,   319,   200,
     207,   211,   173,   463,   464,   499,   322,   275,  -360,   212,
    -358,   321,   321,   237,   238,   239,   240,  -358,   213,  -360,
     318,   478,   464,   320,   540,   237,   238,   239,   240,   536,
     464,   214,   173,   378,   379,   380,   381,   537,   464,   220,
    -360,   225,   276,   640,   612,   613,   226,  -360,   227,  -365,
    -365,  -365,  -365,   229,   160,   230,   547,   231,  -358,  -365,
    -365,  -365,  -365,  -365,  -365,   161,   615,   616,   246,   162,
     254,   321,   260,   595,   620,   621,   163,   164,   243,   574,
     165,   266,   570,   486,   642,   613,   279,   173,  -360,   647,
     648,   641,   722,   723,   334,   658,   487,   376,   377,   578,
     392,   393,   267,   282,   622,   323,   326,   328,     6,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,   347,   173,   351,   184,   367,
     658,   372,   368,   570,   321,   241,   388,    81,   391,  -360,
    -360,  -360,  -360,  -360,  -360,  -360,  -360,  -360,  -360,  -360,
    -360,  -360,  -360,  -360,  -360,   -71,   207,   339,   340,   341,
     395,   194,   401,   501,   685,   398,   -71,   402,   152,   409,
     426,   427,   425,   631,   357,   686,   429,   436,   435,   443,
     456,   173,   619,   344,   669,   363,   366,   -71,   457,   459,
     460,   -71,   473,   285,   -71,   475,    53,   479,    54,   493,
     645,   494,   496,   497,  -445,   535,   644,   512,   173,    55,
      56,    57,    58,   286,    59,   207,   669,  -446,  -444,   543,
      60,   160,   545,    61,   548,   549,   551,   553,   554,    62,
      63,   468,   161,   559,   566,   -71,   162,   173,   567,   287,
      64,  -237,   568,   163,   164,   590,   591,   165,   683,   288,
     592,   609,   593,   602,   604,    65,    66,   607,    67,    68,
     611,   623,   702,    69,     4,   628,    70,    71,    72,   173,
     289,   621,     1,   636,   643,     6,  -398,    73,    74,    75,
      76,    77,    78,    79,   649,   651,   638,     2,   650,   467,
     -71,   -71,   -71,   -71,   -71,   -71,    80,   652,   673,   674,
     702,   653,     3,    81,   676,   703,   684,    82,    83,    84,
     704,    85,    86,    87,    88,    89,    90,    91,   709,     4,
     705,   713,   714,   278,   728,   729,   715,     5,   730,   614,
       6,    92,    93,    94,    95,   284,   639,    96,   688,   708,
      97,   389,   285,   482,   711,    53,     7,    54,   290,   291,
     292,   293,   294,   295,    98,    99,   333,   169,    55,    56,
      57,    58,   409,    59,   252,   154,   587,   466,   510,    60,
     511,   664,    61,   -55,   418,   579,   573,   410,    62,    63,
     361,   588,   577,   261,   420,   666,   665,   691,   287,    64,
     500,    55,    56,    57,    58,   726,   586,   605,   288,   646,
     513,   384,   350,   329,    65,    66,   327,    67,    68,   386,
     385,   455,    69,   600,   454,    70,    71,    72,     0,   289,
       0,     0,   580,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
     581,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     582,     0,    81,   583,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,   285,     0,     0,    53,     0,    54,   290,   291,   292,
     293,   294,   295,    98,    99,     0,   169,    55,    56,    57,
      58,   286,    59,     0,     0,     0,     0,     0,    60,     0,
       0,    61,     0,     0,     0,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,     0,   287,    64,     0,
       0,     0,     0,     0,     0,     0,     0,   288,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,     0,     0,
       0,    69,     0,     0,    70,    71,    72,     0,   289,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,    82,    83,    84,     0,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,    96,     0,     0,    97,     0,
      52,     0,     0,    53,     0,    54,   290,   291,   292,   293,
     294,   295,    98,    99,     0,   169,    55,    56,    57,    58,
     409,    59,     0,     0,     0,     0,     0,    60,     0,     0,
      61,     0,     0,     0,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,     0,   287,    64,     0,     0,
       0,     0,     0,     0,     0,     0,   288,     0,     0,     0,
       0,     0,    65,    66,     0,    67,    68,     0,     0,     0,
      69,     0,     0,    70,    71,    72,     0,   289,     0,     0,
       0,     0,     0,     0,    73,    74,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,    82,    83,    84,     0,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,    96,     0,     0,    97,     0,    52,
       0,     0,    53,     0,    54,   290,   291,   292,   293,   294,
     295,    98,    99,     0,   169,    55,    56,    57,    58,   409,
      59,     0,     0,     0,     0,     0,    60,     0,     0,    61,
       0,     0,     0,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,   288,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,     0,     0,    81,
       0,     0,     0,    82,    83,    84,     0,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,    96,     0,     0,    97,     0,    52,   362,
       0,    53,     0,    54,   290,   291,   292,   293,   294,   295,
      98,    99,     0,   169,    55,    56,    57,    58,     0,    59,
       0,     0,     0,     0,    23,    60,     0,     0,    61,     0,
       0,     0,     0,     0,    62,    63,     0,     0,     0,    24,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,    82,    83,    84,     0,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    96,     0,     0,    97,     0,    52,     0,     0,
      53,     0,    54,     0,     0,     0,     0,     0,     0,    98,
      99,     0,   169,    55,    56,    57,    58,     0,    59,     0,
       0,     0,     0,     0,    60,     0,     0,    61,   677,     0,
       0,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,   678,     0,     0,     0,
       0,     0,     0,   679,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,    52,     0,     0,    53,
       0,    54,     0,     0,     0,     0,     0,     0,    98,    99,
       0,   169,    55,    56,    57,    58,     0,    59,     0,     0,
       0,     0,     0,    60,     0,     0,    61,     0,     0,     0,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,     0,     0,     0,    69,     4,     0,    70,
      71,    72,     0,     0,     0,     0,     0,     0,     6,     0,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
      82,    83,    84,     0,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      96,     0,     0,    97,     0,    52,     0,     0,    53,     0,
      54,     0,     0,     0,     0,     0,     0,    98,    99,     0,
     169,    55,    56,    57,    58,     0,    59,     0,     0,     0,
       0,     0,    60,     0,     0,    61,     0,     0,     0,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,     0,     0,     0,    69,     0,     0,    70,    71,
      72,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,    82,
      83,    84,     0,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,   696,     0,     0,     0,     0,     0,
       0,   697,     0,    92,    93,    94,    95,     0,     0,    96,
       0,     0,    97,     0,    52,     0,     0,    53,     0,    54,
       0,     0,     0,     0,     0,     0,    98,    99,     0,   169,
      55,    56,    57,    58,     0,    59,     0,     0,     0,     0,
       0,    60,     0,     0,    61,     0,     0,     0,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,    66,     0,    67,
      68,     0,     0,     0,    69,     0,     0,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,    82,    83,
      84,     0,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,   725,     0,     0,     0,     0,     0,     0,
     697,     0,    92,    93,    94,    95,     0,     0,    96,     0,
       0,    97,     0,    52,     0,     0,    53,     0,    54,     0,
       0,     0,     0,     0,     0,    98,    99,     0,   169,    55,
      56,    57,    58,     0,    59,     0,     0,     0,     0,     0,
      60,     0,     0,    61,     0,     0,     0,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,     0,    67,    68,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,    73,    74,    75,
      76,    77,    78,    79,     0,     0,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,    82,    83,    84,
       0,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,     0,    52,     0,     0,    53,     0,    54,     0,     0,
       0,     0,     0,     0,    98,    99,     0,   169,    55,    56,
      57,    58,     0,    59,     0,     0,     0,     0,   504,   505,
       0,     0,    61,     0,     0,     0,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,    66,     0,    67,    68,     0,
       0,     0,    69,     0,     0,   506,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,    52,     0,     0,    53,     0,    54,     0,     0,     0,
       0,     0,     0,    98,    99,     0,   169,    55,    56,    57,
      58,     0,    59,     0,     0,     0,     0,     0,    60,     0,
       0,    61,     0,     0,     0,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,     0,     0,
       0,    69,     0,     0,    70,    71,    72,     0,     0,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,    82,    83,    84,     0,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,    96,     0,     0,    97,     0,
      52,     0,     0,    53,     0,    54,     0,     0,     0,     0,
       0,     0,    98,    99,     0,   100,    55,    56,    57,    58,
       0,    59,     0,     0,     0,     0,     0,    60,     0,     0,
      61,     0,     0,     0,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,    66,     0,    67,    68,     0,     0,     0,
      69,     0,     0,    70,    71,    72,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,    82,    83,    84,     0,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,    96,     0,     0,    97,     0,    52,
       0,     0,    53,     0,    54,     0,     0,     0,     0,     0,
       0,    98,    99,     0,   169,    55,    56,    57,    58,     0,
      59,     0,     0,     0,     0,     0,    60,     0,     0,    61,
       0,     0,     0,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,     0,     0,     0,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,    96,     0,     0,    97,     0,    52,     0,
       0,    53,     0,    54,     0,     0,     0,     0,     0,     0,
      98,    99,     0,   169,    55,    56,    57,    58,     0,    59,
       0,     0,     0,     0,     0,    60,     0,     0,    61,     0,
       0,     0,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,     0,  -359,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -359,     0,     0,     0,     0,
      65,    66,     0,    67,    68,   515,     0,     0,    69,     0,
       0,    70,    71,    72,     0,     0,  -359,     0,     0,     0,
       0,     0,    73,  -359,     0,     0,     0,    78,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,    87,    88,
      89,    90,    91,     0,  -359,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    96,     0,     0,    97,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     530,     0,   169,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,
    -359
};

static const yytype_int16 yycheck[] =
{
      25,    79,    35,   135,     0,    82,    83,    84,    21,    28,
      35,   280,   184,   166,   250,   344,   154,     3,   297,   297,
       4,     4,     4,   184,    13,    24,     5,    52,   200,   288,
       3,   184,   297,   297,   212,   213,   189,    24,   297,   200,
     368,   202,     5,    47,     3,   184,   204,   200,   184,   202,
      46,   229,   230,   219,    79,    24,     5,    82,    83,    84,
       5,   200,    31,   202,   200,    24,   202,     5,   347,   347,
      41,     5,    29,    29,   652,   133,   120,     3,    94,    51,
     653,    52,   347,   347,   579,    56,    23,    44,   347,    24,
      61,    58,    63,    64,    74,    30,    67,   263,    33,    25,
      87,   131,    29,   133,   148,    40,   684,    20,   166,    22,
     242,   440,   441,   285,     6,    86,   689,    44,    98,    99,
      28,    13,    59,    86,   285,   286,     3,    34,   397,    74,
      65,    66,   285,   286,    69,   151,   166,     3,    73,   467,
      85,    76,    77,   254,   100,    94,   285,   286,    55,   285,
     286,    24,     5,   648,     0,    62,    94,    92,    24,   184,
      94,   339,   166,    53,   153,   164,   425,   151,   152,   151,
      23,   196,   151,   164,   165,   200,     7,   202,   164,   357,
     167,    34,   166,   166,   180,   363,   347,    52,   366,     7,
     163,    56,   350,   106,   347,   220,   103,   166,    63,    64,
     113,   115,    55,    24,   217,   164,    59,    48,   347,    62,
     167,   347,   147,   542,    34,   150,     3,    74,   243,    60,
      23,   158,   159,   160,   161,   162,   163,    24,    85,   340,
     341,    34,   146,   147,    32,    55,   269,    24,    79,    24,
     130,    94,    62,   396,   269,    43,    31,   260,   138,   139,
     103,     0,    55,    24,   267,     4,     3,    34,     7,    62,
     285,   286,   541,   541,   593,   426,   427,    24,   287,   347,
     289,     3,   305,   426,   427,    29,   541,   541,    55,   457,
     305,   434,   541,   134,   135,    62,   464,   426,   427,     6,
     426,   427,   167,   168,   169,   473,    13,    55,   149,    48,
     103,    41,    51,    50,    62,   158,   159,   160,   161,   162,
     163,    60,    52,   166,   486,     8,    56,    96,    97,     6,
      13,    68,   347,    63,    64,   486,    13,    67,     3,   565,
      79,    78,    47,   486,    81,    75,    24,     6,   415,     3,
      21,    22,   503,    24,    13,     4,     5,   486,    99,   487,
     486,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   545,    23,    50,
     232,   233,   234,   398,     4,     5,     3,   549,    51,    34,
       4,     5,   395,     3,   417,   654,     3,    68,   549,     3,
     415,     3,   417,     4,     5,   414,   549,    78,    23,     3,
      55,   426,   427,   154,   155,   156,   157,    62,     3,    34,
     549,     4,     5,   549,   447,   154,   155,   156,   157,     4,
       5,     3,   447,   237,   238,   239,   240,     4,     5,     5,
      55,     3,   113,   611,     4,     5,     3,    62,   148,   154,
     155,   156,   157,     3,    41,     3,   459,     3,   103,   164,
     165,   166,   167,   168,   169,    52,    45,    46,   164,    56,
       3,   486,    61,   541,     4,     5,    63,    64,   166,   502,
      67,    72,   497,    70,     4,     5,   102,   502,   103,     4,
       5,   613,    37,    38,     4,   638,    83,   235,   236,   508,
     258,   259,    61,    24,   571,   104,    24,    24,    85,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,    96,   541,     4,    86,     3,
     673,     4,   120,   548,   549,    24,    24,   113,    24,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,    23,   571,   197,   198,   199,
      61,    25,     4,   630,    32,   163,    34,    24,     3,    23,
     152,   151,    59,   596,   214,    43,    24,     4,    31,    95,
       4,   596,   568,    97,   652,   225,   226,    55,     5,     5,
      53,    59,     5,     3,    62,     4,     6,     3,     8,    31,
     623,    24,    68,     3,     5,     4,   621,   163,   623,    19,
      20,    21,    22,    23,    24,   630,   684,     5,     5,    95,
      30,    41,     5,    33,    35,    54,     4,     4,     4,    39,
      40,    24,    52,     4,    24,   103,    56,   652,    24,    49,
      50,    61,     3,    63,    64,    13,    25,    67,   663,    59,
       4,   136,     5,     4,     4,    65,    66,   137,    68,    69,
       5,   152,   677,    73,    74,    24,    76,    77,    78,   684,
      80,     5,    27,    71,     4,    85,   166,    87,    88,    89,
      90,    91,    92,    93,     4,    31,    35,    42,     5,     3,
     158,   159,   160,   161,   162,   163,   106,    35,     5,   145,
     715,    98,    57,   113,     4,   133,     5,   117,   118,   119,
     140,   121,   122,   123,   124,   125,   126,   127,    36,    74,
     128,   133,   140,   175,   129,   140,   152,    82,   131,   563,
      85,   141,   142,   143,   144,   182,   610,   147,   670,   684,
     150,   253,     3,   396,   688,     6,   101,     8,   158,   159,
     160,   161,   162,   163,   164,   165,   190,   167,    19,    20,
      21,    22,    23,    24,   155,    28,   512,   367,   426,    30,
     427,   648,    33,    34,   297,     3,   501,   286,    39,    40,
     220,   512,   503,   165,   297,   651,   650,   673,    49,    50,
     415,    19,    20,    21,    22,   715,    24,   548,    59,   625,
     430,   246,   206,   188,    65,    66,   187,    68,    69,   248,
     247,   350,    73,   541,   348,    76,    77,    78,    -1,    80,
      -1,    -1,    50,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      78,    -1,   113,    81,    -1,    -1,   117,   118,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
     141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,   150,
      -1,     3,    -1,    -1,     6,    -1,     8,   158,   159,   160,
     161,   162,   163,   164,   165,    -1,   167,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,    -1,   147,    -1,    -1,   150,    -1,
       3,    -1,    -1,     6,    -1,     8,   158,   159,   160,   161,
     162,   163,   164,   165,    -1,   167,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,    -1,
      73,    -1,    -1,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,
      -1,    -1,     6,    -1,     8,   158,   159,   160,   161,   162,
     163,   164,   165,    -1,   167,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    68,    69,    -1,    -1,    -1,    73,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,     4,
      -1,     6,    -1,     8,   158,   159,   160,   161,   162,   163,
     164,   165,    -1,   167,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    29,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    -1,    68,    69,    -1,    -1,    -1,    73,    -1,
      -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,   117,   118,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,    -1,   147,    -1,    -1,   150,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   164,
     165,    -1,   167,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
      -1,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,   141,   142,   143,   144,    -1,
      -1,   147,    -1,    -1,   150,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   164,   165,
      -1,   167,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      -1,    68,    69,    -1,    -1,    -1,    73,    74,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      87,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,
     117,   118,   119,    -1,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,    -1,
     147,    -1,    -1,   150,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,   164,   165,    -1,
     167,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,
      68,    69,    -1,    -1,    -1,    73,    -1,    -1,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,   141,   142,   143,   144,    -1,    -1,   147,
      -1,    -1,   150,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,   164,   165,    -1,   167,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,
      69,    -1,    -1,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,   117,   118,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,
      -1,   150,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,   164,   165,    -1,   167,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,
      -1,    -1,    -1,    73,    -1,    -1,    76,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93,    -1,    -1,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,    -1,   117,   118,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,
     150,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   165,    -1,   167,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,
      -1,    -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,    -1,    -1,   117,   118,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,   150,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   164,   165,    -1,   167,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,    -1,   147,    -1,    -1,   150,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,   164,   165,    -1,   167,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,    -1,
      73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,
      -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,   164,   165,    -1,   167,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    68,    69,    -1,    -1,    -1,    73,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,    -1,   167,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      65,    66,    -1,    68,    69,    24,    -1,    -1,    73,    -1,
      -1,    76,    77,    78,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    87,    62,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,    -1,   147,    -1,    -1,   150,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    42,    57,    74,    82,    85,   101,   173,   174,
     175,   186,   198,   199,   201,   202,   203,   211,   213,    28,
     182,    51,    58,    29,    44,   207,   164,   223,   224,   225,
     226,   278,   279,   318,   320,     3,    24,     0,     7,     7,
      48,    60,    79,   200,    24,   184,   185,   226,    24,   353,
     354,   355,     3,     6,     8,    19,    20,    21,    22,    24,
      30,    33,    39,    40,    50,    65,    66,    68,    69,    73,
      76,    77,    78,    87,    88,    89,    90,    91,    92,    93,
     106,   113,   117,   118,   119,   121,   122,   123,   124,   125,
     126,   127,   141,   142,   143,   144,   147,   150,   164,   165,
     167,   214,   261,   262,   263,   264,   266,   267,   268,   269,
     270,   271,   273,   280,   281,   282,   284,   285,   287,   288,
     289,   315,   323,   329,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   346,   347,   348,   349,   350,
     351,   352,   363,   364,   365,   367,   368,   369,   370,   381,
      29,   100,     3,   164,   225,   261,    24,   358,   359,   360,
      41,    52,    56,    63,    64,    67,    75,   274,   277,   167,
     204,   205,   206,   346,     3,   163,   177,   178,   179,    29,
     322,     3,     5,   199,    86,   212,   227,     6,    13,     3,
     190,   199,   346,    24,    25,   362,     3,     3,     3,     3,
       3,   333,    96,   206,   375,   376,   380,   346,   347,   347,
     347,     3,     3,     3,     3,   333,   333,    51,   218,   219,
       5,    20,    22,   106,   113,     3,     3,   148,   290,     3,
       3,     3,   167,   168,   169,   164,   165,   154,   155,   156,
     157,    24,    31,   166,   265,   366,   164,    13,   153,   331,
     358,    31,   221,   382,     3,   361,   221,   222,     6,    13,
      61,   277,    24,   208,   209,   365,    72,    61,     4,     5,
      21,    22,    24,    50,    68,    78,   113,   180,   180,   102,
     176,   199,    24,   183,   184,     3,    23,    49,    59,    80,
     158,   159,   160,   161,   162,   163,   197,   205,   231,   232,
     233,   234,   235,   239,   241,   242,   244,   245,   246,   247,
     249,   250,   252,   253,   255,   256,   259,   260,   266,   269,
     323,   346,   365,   104,   383,   384,    24,   356,    24,   355,
     356,   188,   365,   203,     4,     4,   324,   346,   167,   207,
     207,   207,   235,   235,    97,   376,   377,    96,   371,   372,
     375,     4,   344,   346,   378,   379,   344,   207,   220,   226,
     212,   263,     4,   207,   344,   345,   207,     3,   120,   286,
     344,   344,     4,   334,   334,   334,   335,   335,   336,   336,
     336,   336,   366,   346,   351,   364,   363,   222,    24,   190,
     345,    24,   360,   360,   226,    61,     5,   212,   163,   226,
     205,     4,    24,   181,   383,     4,     5,   235,   346,    23,
     232,   261,   197,   261,    55,    62,   197,   242,   243,   245,
     246,   249,   252,   254,   257,    59,   152,   151,   205,    24,
     385,   386,    13,     4,     5,    31,     4,   344,   345,   345,
       5,    94,   378,    95,   197,   206,   240,   242,   243,   245,
     249,   252,   373,   374,   377,   372,     4,     5,   344,     5,
      53,   228,   344,     4,     5,   344,   227,     3,    24,   291,
     292,   293,   298,     5,   283,     4,     8,   330,     4,     3,
     189,   226,   209,   383,   210,   346,    70,    83,   275,   276,
     321,     6,    13,    31,    24,     4,    68,     3,   251,   261,
     332,   347,    34,   103,    29,    30,    76,   205,   258,   197,
     233,   234,   163,   386,   365,    24,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   325,   326,   327,   328,     4,     4,     4,   378,   378,
     205,     5,    94,    95,   344,     5,   272,   226,    35,    54,
     229,     4,   344,     4,     4,   291,   299,   300,   344,     4,
     187,   366,   235,     3,   357,   358,    24,    24,     3,   343,
     346,    47,   248,   248,   205,    24,   269,   271,   261,     3,
      50,    68,    78,    81,   230,   236,    24,   230,   264,   387,
      13,    25,     4,     5,   197,   206,   242,   245,   249,   252,
     374,   378,     4,   344,     4,   343,   235,   137,   294,   136,
     301,     5,     4,     5,   187,    45,    46,   319,   222,   199,
       4,     5,   347,   152,   164,   224,   237,   238,    24,   378,
      62,   205,   291,   295,   296,   297,    71,   191,    35,   191,
     344,   366,     4,     4,   346,   205,   353,     4,     5,     4,
       5,    31,    35,    98,   215,   217,   303,   304,   365,   134,
     135,   149,   302,   306,   238,   296,   298,   192,   193,   206,
     239,   337,   383,     5,   145,   316,     4,    34,   132,   139,
     307,   308,   310,   346,     5,    32,    43,   194,   194,    99,
     216,   304,   115,   146,   147,   317,   132,   139,   309,   311,
     312,   314,   346,   133,   140,   128,   305,   133,   193,    36,
     195,   195,   337,   133,   140,   152,   131,   133,    53,   130,
     138,   139,    37,    38,   196,   132,   311,   313,   129,   140,
     131
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   172,   173,   173,   173,   173,   174,   175,   176,   176,
     177,   177,   177,   178,   179,   180,   180,   180,   180,   180,
     180,   180,   181,   181,   181,   182,   182,   183,   183,   184,
     185,   185,   186,   187,   187,   188,   188,   189,   189,   190,
     190,   191,   191,   192,   192,   193,   193,   194,   194,   194,
     195,   195,   196,   196,   197,   197,   198,   198,   198,   198,
     198,   199,   199,   200,   200,   200,   201,   202,   203,   204,
     204,   205,   206,   207,   207,   207,   208,   208,   209,   210,
     211,   212,   212,   213,   213,   214,   214,   215,   215,   216,
     216,   217,   218,   218,   219,   220,   220,   221,   221,   222,
     222,   223,   223,   224,   224,   224,   225,   225,   226,   226,
     226,   226,   227,   228,   228,   229,   229,   230,   230,   230,
     230,   230,   231,   231,   231,   231,   231,   231,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   237,   237,
     238,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     240,   241,   241,   242,   242,   242,   242,   242,   242,   242,
     243,   244,   245,   246,   247,   247,   247,   247,   248,   248,
     249,   250,   250,   251,   251,   252,   253,   253,   254,   255,
     256,   257,   258,   258,   258,   259,   260,   261,   262,   262,
     263,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     265,   265,   265,   266,   266,   267,   267,   267,   267,   268,
     269,   269,   269,   269,   269,   270,   270,   271,   271,   271,
     271,   271,   271,   272,   272,   273,   273,   273,   273,   273,
     273,   274,   274,   274,   275,   276,   276,   277,   277,   277,
     277,   278,   279,   279,   279,   280,   281,   281,   281,   281,
     281,   281,   281,   282,   283,   283,   283,   284,   285,   285,
     286,   287,   288,   288,   289,   290,   290,   291,   292,   292,
     293,   294,   294,   295,   295,   296,   297,   298,   299,   299,
     300,   301,   301,   302,   302,   303,   303,   304,   305,   305,
     305,   305,   305,   306,   306,   306,   307,   307,   308,   308,
     308,   309,   310,   311,   311,   311,   312,   312,   313,   313,
     314,   315,   315,   315,   315,   316,   316,   317,   317,   317,
     318,   319,   319,   319,   320,   320,   321,   322,   322,   323,
     324,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   326,   326,   327,
     328,   328,   329,   330,   330,   331,   331,   332,   332,   332,
     332,   332,   332,   332,   332,   333,   334,   334,   334,   334,
     335,   335,   335,   335,   336,   336,   336,   337,   337,   337,
     337,   337,   338,   339,   339,   339,   339,   339,   339,   340,
     341,   342,   343,   343,   344,   345,   345,   346,   346,   346,
     347,   348,   348,   349,   349,   350,   351,   352,   353,   353,
     354,   355,   355,   356,   357,   358,   359,   359,   359,   360,
     361,   361,   362,   362,   363,   363,   364,   364,   364,   365,
     366,   367,   368,   368,   369,   370,   371,   371,   372,   373,
     373,   374,   374,   374,   374,   374,   374,   375,   375,   376,
     377,   377,   378,   379,   380,   381,   381,   382,   382,   383,
     383,   384,   385,   385,   386,   386,   387,   387,   387
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
       1,     1,     4,     1,     1,     1,     5,     5,     4,     1,
       3,     1,     1,     0,     1,     1,     1,     3,     3,     1,
       6,     0,     1,     4,     1,     1,     1,     0,     1,     0,
       2,     3,     0,     8,     2,     1,     3,     0,     1,     0,
       3,     0,     1,     0,     2,     2,     0,     1,     4,     3,
       4,     1,     2,     0,     3,     0,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       4,     1,     2,     1,     3,     1,     3,     3,     1,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     2,     1,     1,     1,     1,     1,     1,     2,
       5,     2,     4,     4,     2,     2,     1,     1,     0,     2,
       3,     2,     1,     1,     3,     3,     2,     1,     3,     2,
       2,     3,     1,     1,     1,     2,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     8,
       1,     1,     3,     4,     5,     1,     1,     5,     5,     5,
       4,     5,     6,     0,     2,     1,     1,     1,     1,     1,
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
       1,     1,     6,     0,     1,     0,     3,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     2,     2,     2,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     1,     1,     1,     2,     2,     2,     1,
       1,     1,     1,     3,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     1,     2,     1,     1,
       3,     3,     3,     1,     2,     1,     1,     3,     3,     2,
       0,     3,     0,     1,     1,     3,     2,     1,     1,     2,
       1,     1,     1,     1,     5,     4,     1,     2,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     4,
       0,     2,     1,     1,     1,     2,     1,     0,     2,     0,
       1,     2,     2,     1,     1,     3,     1,     1,     1
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

  case 61: /* select_statement: single_select_statement  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 62: /* select_statement: single_select_statement union_op all select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 66: /* delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM table_ref opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 67: /* insert_statement: SQL_TOKEN_INSERT SQL_TOKEN_INTO table_node opt_column_ref_commalist values_or_query_spec  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 68: /* values_or_query_spec: SQL_TOKEN_VALUES '(' row_value_constructor_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 69: /* row_value_constructor_commalist: row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 70: /* row_value_constructor_commalist: row_value_constructor_commalist ',' row_value_constructor  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 73: /* opt_all_distinct: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 76: /* assignment_commalist: assignment  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 77: /* assignment_commalist: assignment_commalist ',' assignment  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 78: /* assignment: column_ref SQL_EQUAL update_source  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 80: /* update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 81: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 83: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 85: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 87: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 89: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 90: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 91: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 92: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 93: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
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

  case 94: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 95: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 96: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 97: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 99: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 100: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 101: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 102: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 103: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 104: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 105: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 106: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 107: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 108: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 109: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 110: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 112: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 113: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 114: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 115: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 116: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 128: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 130: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 132: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 134: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 136: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 137: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 138: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 139: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 140: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 150: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 151: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 152: /* comparison_predicate: comparison row_value_constructor  */
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

  case 159: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 160: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 161: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 162: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 163: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 164: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 165: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 166: /* like_predicate: character_like_predicate_part_2  */
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

  case 167: /* like_predicate: other_like_predicate_part_2  */
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

  case 168: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 169: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 170: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 171: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 172: /* test_for_null: null_predicate_part_2  */
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

  case 173: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 174: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 175: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 176: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 177: /* in_predicate: in_predicate_part_2  */
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

  case 178: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 179: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 180: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 181: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 185: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 186: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 187: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 188: /* scalar_exp_commalist: select_sublist  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 189: /* scalar_exp_commalist: scalar_exp_commalist ',' select_sublist  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 196: /* literal: literal SQL_TOKEN_STRING  */
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

  case 197: /* literal: literal SQL_TOKEN_INT  */
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

  case 198: /* literal: literal SQL_TOKEN_REAL_NUM  */
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

  case 199: /* literal: literal SQL_TOKEN_APPROXNUM  */
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

  case 200: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 201: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 209: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));


        }
    break;

  case 212: /* fct_spec: function_name '(' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 213: /* fct_spec: function_name '(' function_args_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 214: /* fct_spec: function_name '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 217: /* aggregate_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 218: /* aggregate_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 219: /* aggregate_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 220: /* aggregate_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 221: /* aggregate_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 222: /* aggregate_fct: SQL_TOKEN_GROUP_CONCAT '(' opt_all_distinct function_arg opt_function_arg ')'  */
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

  case 223: /* opt_function_arg: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 224: /* opt_function_arg: ',' function_arg  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 231: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 232: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 233: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 234: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 237: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 238: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 240: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 241: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 242: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 243: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 245: /* window_function: window_function_type opt_filter_clause SQL_TOKEN_OVER window_name_or_specification  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 246: /* window_function_type: rank_function_type '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 247: /* window_function_type: SQL_TOKEN_ROW_NUMBER '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 253: /* ntile_function: SQL_TOKEN_NTILE '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 254: /* opt_lead_or_lag_function: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 255: /* opt_lead_or_lag_function: ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 256: /* opt_lead_or_lag_function: ',' function_arg ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 257: /* lead_or_lag_function: lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 261: /* first_or_last_value_function: first_or_last_value '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 264: /* nth_value_function: SQL_TOKEN_NTH_VALUE '(' function_arg ',' function_arg ')'  */
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

  case 265: /* opt_filter_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 266: /* opt_filter_clause: SQL_TOKEN_FILTER '(' where_clause ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 271: /* opt_window_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 272: /* opt_window_clause: SQL_TOKEN_WINDOW window_definition_list  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 273: /* window_definition_list: window_definition_list ',' window_definition  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 274: /* window_definition_list: window_definition  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 275: /* window_definition: new_window_name SQL_TOKEN_AS window_specification  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 277: /* window_specification: '(' opt_existing_window_name opt_window_partition_clause opt_order_by_clause opt_window_frame_clause ')'  */
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

  case 278: /* opt_existing_window_name: %empty  */
                                 {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 281: /* opt_window_partition_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 282: /* opt_window_partition_clause: SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 283: /* opt_window_frame_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 284: /* opt_window_frame_clause: window_frame_units window_frame_extent opt_window_frame_exclusion  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 285: /* window_partition_column_reference_list: window_partition_column_reference_list ',' window_partition_column_reference  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 286: /* window_partition_column_reference_list: window_partition_column_reference  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 287: /* window_partition_column_reference: column_ref opt_collate_clause  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 288: /* opt_window_frame_exclusion: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 289: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 290: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 291: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 292: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 298: /* window_frame_start: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 299: /* window_frame_start: value_exp SQL_TOKEN_PRECEDING  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 300: /* window_frame_start: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 301: /* window_frame_preceding: value_exp SQL_TOKEN_PRECEDING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 302: /* window_frame_between: SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 304: /* window_frame_bound: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 307: /* window_frame_bound_1: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 309: /* window_frame_bound_2: SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* window_frame_following: value_exp SQL_TOKEN_FOLLOWING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 315: /* opt_collate_clause: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 316: /* opt_collate_clause: SQL_TOKEN_COLLATE collating_function  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 320: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 321: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 326: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 327: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 347: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 348: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 349: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 352: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 353: /* opt_optional_prop: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 354: /* opt_optional_prop: '?'  */
             {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
    }
    break;

  case 355: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 356: /* opt_extract_value: SQL_ARROW property_path opt_optional_prop  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 363: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 367: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 368: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 369: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 371: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 372: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 373: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 375: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 376: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 380: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 385: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 388: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 389: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 390: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 392: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 393: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 395: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 396: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 403: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 404: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 407: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 408: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 409: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 410: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 411: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 412: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 413: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 414: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 415: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 416: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 417: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 418: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 419: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 420: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 421: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 422: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 423: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 424: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 425: /* property_path: property_path '.' property_path_entry  */
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

  case 426: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 427: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 428: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 429: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 434: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 435: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 436: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 437: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 438: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 439: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 440: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 447: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 448: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 449: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 450: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 451: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 455: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 456: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 457: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 458: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 459: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 461: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 462: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 463: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 464: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 465: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
