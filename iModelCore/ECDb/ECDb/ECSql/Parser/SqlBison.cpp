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
  YYSYMBOL_values_commalist = 203,         /* values_commalist  */
  YYSYMBOL_values_or_query_spec = 204,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 205, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 206,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 207, /* row_value_constructor_elem  */
  YYSYMBOL_opt_all_distinct = 208,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 209,     /* assignment_commalist  */
  YYSYMBOL_assignment = 210,               /* assignment  */
  YYSYMBOL_update_source = 211,            /* update_source  */
  YYSYMBOL_update_statement_searched = 212, /* update_statement_searched  */
  YYSYMBOL_opt_where_clause = 213,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 214,  /* single_select_statement  */
  YYSYMBOL_selection = 215,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 216,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 217,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 218,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 219,                /* table_exp  */
  YYSYMBOL_from_clause = 220,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 221,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 222,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 223, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 224, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 225,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 226, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 227,                /* table_ref  */
  YYSYMBOL_where_clause = 228,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 229,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 230,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 231,              /* truth_value  */
  YYSYMBOL_boolean_primary = 232,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 233,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 234,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 235,             /* boolean_term  */
  YYSYMBOL_search_condition = 236,         /* search_condition  */
  YYSYMBOL_type_predicate = 237,           /* type_predicate  */
  YYSYMBOL_type_list = 238,                /* type_list  */
  YYSYMBOL_type_list_item = 239,           /* type_list_item  */
  YYSYMBOL_predicate = 240,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 241, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 242,     /* comparison_predicate  */
  YYSYMBOL_comparison = 243,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 244, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 245,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 246, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 247, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 248,           /* like_predicate  */
  YYSYMBOL_opt_escape = 249,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 250,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 251,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 252,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 253,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 254,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 255, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 256,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 257,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 258, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 259,             /* any_all_some  */
  YYSYMBOL_existence_test = 260,           /* existence_test  */
  YYSYMBOL_unique_test = 261,              /* unique_test  */
  YYSYMBOL_subquery = 262,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 263,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 264,           /* select_sublist  */
  YYSYMBOL_literal = 265,                  /* literal  */
  YYSYMBOL_as_clause = 266,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 267,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 268,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 269,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 270,                 /* fct_spec  */
  YYSYMBOL_function_name = 271,            /* function_name  */
  YYSYMBOL_aggregate_fct = 272,            /* aggregate_fct  */
  YYSYMBOL_opt_function_arg = 273,         /* opt_function_arg  */
  YYSYMBOL_set_fct_type = 274,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 275,          /* outer_join_type  */
  YYSYMBOL_join_condition = 276,           /* join_condition  */
  YYSYMBOL_join_spec = 277,                /* join_spec  */
  YYSYMBOL_join_type = 278,                /* join_type  */
  YYSYMBOL_cross_union = 279,              /* cross_union  */
  YYSYMBOL_qualified_join = 280,           /* qualified_join  */
  YYSYMBOL_window_function = 281,          /* window_function  */
  YYSYMBOL_window_function_type = 282,     /* window_function_type  */
  YYSYMBOL_ntile_function = 283,           /* ntile_function  */
  YYSYMBOL_opt_lead_or_lag_function = 284, /* opt_lead_or_lag_function  */
  YYSYMBOL_lead_or_lag_function = 285,     /* lead_or_lag_function  */
  YYSYMBOL_lead_or_lag = 286,              /* lead_or_lag  */
  YYSYMBOL_lead_or_lag_extent = 287,       /* lead_or_lag_extent  */
  YYSYMBOL_first_or_last_value_function = 288, /* first_or_last_value_function  */
  YYSYMBOL_first_or_last_value = 289,      /* first_or_last_value  */
  YYSYMBOL_nth_value_function = 290,       /* nth_value_function  */
  YYSYMBOL_opt_filter_clause = 291,        /* opt_filter_clause  */
  YYSYMBOL_window_name = 292,              /* window_name  */
  YYSYMBOL_window_name_or_specification = 293, /* window_name_or_specification  */
  YYSYMBOL_in_line_window_specification = 294, /* in_line_window_specification  */
  YYSYMBOL_opt_window_clause = 295,        /* opt_window_clause  */
  YYSYMBOL_window_definition_list = 296,   /* window_definition_list  */
  YYSYMBOL_window_definition = 297,        /* window_definition  */
  YYSYMBOL_new_window_name = 298,          /* new_window_name  */
  YYSYMBOL_window_specification = 299,     /* window_specification  */
  YYSYMBOL_opt_existing_window_name = 300, /* opt_existing_window_name  */
  YYSYMBOL_existing_window_name = 301,     /* existing_window_name  */
  YYSYMBOL_opt_window_partition_clause = 302, /* opt_window_partition_clause  */
  YYSYMBOL_opt_window_frame_clause = 303,  /* opt_window_frame_clause  */
  YYSYMBOL_window_partition_column_reference_list = 304, /* window_partition_column_reference_list  */
  YYSYMBOL_window_partition_column_reference = 305, /* window_partition_column_reference  */
  YYSYMBOL_opt_window_frame_exclusion = 306, /* opt_window_frame_exclusion  */
  YYSYMBOL_window_frame_units = 307,       /* window_frame_units  */
  YYSYMBOL_window_frame_extent = 308,      /* window_frame_extent  */
  YYSYMBOL_window_frame_start = 309,       /* window_frame_start  */
  YYSYMBOL_window_frame_preceding = 310,   /* window_frame_preceding  */
  YYSYMBOL_window_frame_between = 311,     /* window_frame_between  */
  YYSYMBOL_window_frame_bound = 312,       /* window_frame_bound  */
  YYSYMBOL_window_frame_bound_1 = 313,     /* window_frame_bound_1  */
  YYSYMBOL_window_frame_bound_2 = 314,     /* window_frame_bound_2  */
  YYSYMBOL_window_frame_following = 315,   /* window_frame_following  */
  YYSYMBOL_rank_function_type = 316,       /* rank_function_type  */
  YYSYMBOL_opt_collate_clause = 317,       /* opt_collate_clause  */
  YYSYMBOL_collating_function = 318,       /* collating_function  */
  YYSYMBOL_ecrelationship_join = 319,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 320, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 321,             /* joined_table  */
  YYSYMBOL_named_columns_join = 322,       /* named_columns_join  */
  YYSYMBOL_all = 323,                      /* all  */
  YYSYMBOL_scalar_subquery = 324,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 325,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 326, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 327,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 328,        /* cast_target_array  */
  YYSYMBOL_cast_target = 329,              /* cast_target  */
  YYSYMBOL_cast_spec = 330,                /* cast_spec  */
  YYSYMBOL_opt_optional_prop = 331,        /* opt_optional_prop  */
  YYSYMBOL_opt_extract_value = 332,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 333,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 334,              /* num_primary  */
  YYSYMBOL_factor = 335,                   /* factor  */
  YYSYMBOL_term = 336,                     /* term  */
  YYSYMBOL_term_add_sub = 337,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 338,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 339,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 340,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 341,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 342,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 343,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 344,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 345,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 346,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 347,                /* value_exp  */
  YYSYMBOL_string_value_exp = 348,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 349,           /* char_value_exp  */
  YYSYMBOL_concatenation = 350,            /* concatenation  */
  YYSYMBOL_char_primary = 351,             /* char_primary  */
  YYSYMBOL_char_factor = 352,              /* char_factor  */
  YYSYMBOL_derived_column = 353,           /* derived_column  */
  YYSYMBOL_table_node = 354,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 355, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 356,     /* qualified_class_name  */
  YYSYMBOL_class_name = 357,               /* class_name  */
  YYSYMBOL_table_node_ref = 358,           /* table_node_ref  */
  YYSYMBOL_table_node_with_opt_member_func_call = 359, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 360,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 361,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 362, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 363,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 364,            /* property_path  */
  YYSYMBOL_property_path_entry = 365,      /* property_path_entry  */
  YYSYMBOL_column_ref = 366,               /* column_ref  */
  YYSYMBOL_column = 367,                   /* column  */
  YYSYMBOL_case_expression = 368,          /* case_expression  */
  YYSYMBOL_case_specification = 369,       /* case_specification  */
  YYSYMBOL_simple_case = 370,              /* simple_case  */
  YYSYMBOL_searched_case = 371,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 372,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 373,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 374,        /* when_operand_list  */
  YYSYMBOL_when_operand = 375,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 376, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 377,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 378,              /* else_clause  */
  YYSYMBOL_result = 379,                   /* result  */
  YYSYMBOL_result_expression = 380,        /* result_expression  */
  YYSYMBOL_case_operand = 381,             /* case_operand  */
  YYSYMBOL_parameter = 382,                /* parameter  */
  YYSYMBOL_range_variable = 383,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 384,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 385,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 386,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 387,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 388          /* ecsqloptionvalue  */
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
#define YYLAST   3077

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  172
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  217
/* YYNRULES -- Number of rules.  */
#define YYNRULES  471
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  736

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
     514,   518,   523,   534,   535,   536,   541,   553,   563,   568,
     576,   586,   591,   599,   602,   606,   607,   608,   612,   615,
     621,   628,   631,   643,   644,   652,   660,   664,   669,   672,
     673,   676,   677,   685,   694,   695,   710,   720,   723,   729,
     730,   734,   737,   747,   748,   755,   756,   761,   769,   770,
     777,   785,   793,   801,   805,   814,   815,   825,   826,   834,
     835,   836,   837,   838,   841,   842,   843,   844,   845,   846,
     847,   857,   858,   868,   869,   877,   878,   887,   888,   898,
     907,   912,   920,   929,   930,   931,   932,   933,   934,   935,
     936,   937,   943,   950,   957,   982,   983,   984,   985,   986,
     987,   988,   996,  1006,  1014,  1024,  1034,  1040,  1046,  1068,
    1093,  1094,  1101,  1110,  1116,  1132,  1136,  1144,  1153,  1159,
    1175,  1184,  1193,  1202,  1212,  1213,  1214,  1218,  1226,  1232,
    1239,  1250,  1255,  1262,  1266,  1267,  1268,  1269,  1270,  1272,
    1284,  1296,  1308,  1324,  1325,  1331,  1335,  1336,  1339,  1340,
    1341,  1342,  1345,  1357,  1358,  1359,  1366,  1374,  1386,  1387,
    1392,  1408,  1424,  1433,  1441,  1450,  1468,  1469,  1478,  1479,
    1480,  1481,  1482,  1483,  1487,  1492,  1497,  1504,  1512,  1513,
    1516,  1517,  1522,  1523,  1531,  1543,  1553,  1562,  1566,  1577,
    1584,  1591,  1592,  1593,  1594,  1595,  1599,  1610,  1611,  1617,
    1628,  1640,  1641,  1645,  1650,  1661,  1662,  1666,  1680,  1681,
    1691,  1695,  1696,  1700,  1705,  1706,  1715,  1718,  1724,  1734,
    1738,  1756,  1757,  1761,  1766,  1767,  1778,  1779,  1789,  1792,
    1798,  1808,  1809,  1816,  1822,  1828,  1839,  1840,  1841,  1845,
    1846,  1850,  1856,  1863,  1872,  1881,  1892,  1893,  1899,  1903,
    1904,  1913,  1914,  1923,  1932,  1933,  1934,  1935,  1940,  1941,
    1950,  1951,  1952,  1956,  1970,  1971,  1972,  1975,  1976,  1979,
    1991,  1992,  1996,  1999,  2003,  2004,  2005,  2006,  2007,  2008,
    2009,  2010,  2011,  2012,  2013,  2014,  2015,  2016,  2017,  2018,
    2022,  2027,  2037,  2046,  2047,  2051,  2063,  2064,  2071,  2072,
    2080,  2081,  2082,  2083,  2084,  2085,  2086,  2093,  2097,  2101,
    2102,  2108,  2114,  2123,  2124,  2131,  2138,  2148,  2149,  2156,
    2166,  2167,  2174,  2181,  2188,  2198,  2205,  2210,  2215,  2220,
    2226,  2232,  2241,  2248,  2256,  2264,  2267,  2273,  2277,  2282,
    2290,  2291,  2292,  2296,  2300,  2301,  2304,  2311,  2321,  2324,
    2328,  2338,  2343,  2350,  2359,  2367,  2376,  2384,  2393,  2401,
    2406,  2411,  2419,  2428,  2429,  2439,  2440,  2448,  2453,  2471,
    2477,  2482,  2490,  2502,  2506,  2510,  2511,  2515,  2526,  2536,
    2541,  2548,  2558,  2561,  2566,  2567,  2568,  2569,  2570,  2571,
    2574,  2579,  2586,  2596,  2597,  2605,  2609,  2613,  2617,  2623,
    2631,  2634,  2644,  2645,  2649,  2658,  2663,  2671,  2677,  2687,
    2688,  2689
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
  "insert_statement", "values_commalist", "values_or_query_spec",
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

#define YYTABLE_NINF (-450)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     282,    57,   106,    83,   262,    25,    94,   177,   217,   238,
     241,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   193,  -579,
     226,    25,   237,  -579,  -579,  2373,   263,    49,    17,   268,
     318,  -579,  -579,  -579,  -579,  2522,    31,  -579,  -579,  -579,
    -579,  -579,  -579,   279,   310,  -579,    90,   423,   290,   319,
    -579,  -579,  1628,   301,  -579,  -579,  -579,  -579,  -579,   233,
    -579,  -579,   328,   332,  -579,   334,   343,  -579,  -579,   346,
    -579,  -579,  -579,  -579,  2820,  -579,  -579,  -579,  -579,  2075,
    -579,  -579,  2522,  2522,  2522,   363,   368,  -579,  -579,  -579,
    -579,   370,  -579,  -579,  -579,  -579,  -579,   372,  2820,  2820,
     281,   327,  -579,   382,  -579,    42,  -579,  -579,  -579,  -579,
     389,   -39,   394,  -579,   252,  -579,  -579,   399,  -579,   401,
    -579,   408,  -579,  -579,  -579,  -579,  -579,   -24,   146,   315,
    -579,  -579,  -579,  -579,  -579,    36,  -579,   256,  -579,  -579,
    -579,  -579,    43,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,    18,  -579,   268,   283,   420,   283,   299,  -579,
     371,  -579,  -579,  -579,  -579,   374,    28,   357,   373,  -579,
     199,  -579,  -579,   278,    92,    92,   337,  -579,  -579,  -579,
      18,   425,   226,  -579,   883,   359,  -579,   437,   441,    28,
     388,   463,   438,   447,    21,  -579,  -579,  -579,  2522,    44,
     262,   262,   883,  -579,   883,  -579,   266,  -579,   380,   278,
    -579,  -579,  -579,   474,  2522,  2522,   262,  -579,  -579,    25,
    -579,   397,  2522,  -579,  -579,  -579,  -579,  1330,   262,   488,
     386,  2522,  2522,   506,  2671,  2671,  2671,  2671,  2671,  2671,
    2671,  2671,  2671,  -579,   496,  2522,  -579,  -579,   446,    28,
      28,  -579,   283,  -579,   536,   319,  2522,  -579,   537,  -579,
     268,   268,    25,   453,   525,    67,  -579,   400,  -579,    25,
    -579,  2522,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   558,
    -579,   540,   359,  -579,  -579,   484,  -579,   585,   734,   562,
     543,   562,  -579,  -579,  -579,  -579,  -579,  -579,   289,    70,
     510,  -579,  -579,   415,   419,  -579,  -579,  2522,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
     378,  2846,  2868,   157,  2893,   547,  -579,  -579,  -579,  -579,
     339,  -579,  -579,   490,  -579,  -579,  -579,  -579,   388,  -579,
     541,   278,   569,  2522,  2522,  2522,    39,   -12,  2522,  -579,
     480,   883,   477,  -579,   380,  -579,   574,   278,  -579,  -579,
     575,  2522,   576,   444,   529,  -579,  -579,  2522,  -579,   493,
    2522,   397,   210,   578,  -579,   580,  -579,  -579,  -579,  -579,
     -24,   -24,   146,   146,   146,   146,  -579,  -579,  -579,  -579,
     236,  -579,  -579,  -579,   497,   582,  -579,  -579,   444,    25,
      28,   359,  2522,   358,  -579,  -579,  -579,   341,  -579,   555,
     563,    41,    20,  -579,  -579,  -579,   521,  -579,   587,  2522,
      65,  2224,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   543,
     883,   883,  -579,   429,   547,  -579,   437,  -579,    28,  -579,
    2958,  -579,   590,   499,   508,  2522,  2522,  -579,  -579,    46,
      60,  -579,  2522,  -579,   591,   592,   593,    69,  -579,   504,
    -579,  -579,  2522,   595,    25,   567,   559,   608,  -579,  2522,
     610,   612,   596,  -579,  -579,  -579,  -579,  -579,  2522,   613,
    -579,  -579,  -579,  -579,   496,  -579,   444,  -579,  -579,  -579,
     278,   883,   273,  -579,  -579,  -579,   597,   598,   616,  -579,
    -579,  -579,  1628,  -579,  -579,   291,     8,  2522,   303,  -579,
    -579,  -579,  -579,   562,    88,  -579,   415,   280,  -579,  -579,
     614,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,   601,  -579,   619,
    -579,  -579,  -579,   623,  -579,    71,  1181,  2522,  -579,   625,
    2522,   626,   444,  2522,   883,   494,  -579,  -579,  -579,  -579,
    -579,   500,  -579,   628,  -579,   518,  -579,   419,   496,   479,
     283,  -579,  -579,    18,   522,   278,  2522,  -579,  -579,   485,
    -579,  -579,  -579,  -579,    14,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,  -579,    42,  -579,   615,  -579,  -579,  2522,   159,
    -579,  2522,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
     633,   419,   596,   570,   605,   570,  2522,  -579,   496,   526,
    -579,  -579,  -579,  -579,   638,  -579,  2522,   481,  2522,  -579,
     237,   544,  -579,  -579,   641,  2522,  -579,  -579,   643,  -579,
     618,   611,   554,    28,   -45,  -579,  -579,  -579,  -579,   278,
    -579,  -579,  -579,    14,  -579,   596,   652,  1032,  2671,   359,
    -579,   651,  -579,   523,  -579,  -579,  -579,   656,  1479,  -579,
    -579,  -579,   661,  -579,    64,    62,   100,  -579,    28,    93,
    -579,  -579,  1777,   534,   531,   551,  -579,  -579,   -56,  1032,
    -579,  -579,   644,   644,  2671,  -579,  -579,  -579,  -579,  -579,
    -579,   548,   542,  -579,  -579,   517,  -579,   -31,  -579,  -579,
      68,  -579,  -579,  -579,   515,  -579,  -579,   315,  -579,  -579,
    1926,  -579,  -579,  -579,  -579,   556,   546,  -579,  -579,  -579,
     552,  -579,  -579,  -579,  -579,  -579
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,     0,     0,    75,   103,     0,     0,     0,     2,
       4,    60,     6,    59,    56,    57,    86,    58,    61,    26,
       0,   103,     0,    76,    77,     0,   104,     0,   108,     0,
     240,   247,   328,   327,   113,     0,    10,     1,     3,     5,
      65,    63,    64,   330,     0,    31,     0,    83,     0,    39,
     412,   411,     0,     0,   459,   198,   195,   196,   197,   425,
     231,   228,     0,     0,   210,     0,     0,   209,   233,     0,
     232,   229,   211,   431,     0,   386,   388,   387,   230,     0,
     194,   408,     0,     0,     0,     0,     0,   261,   262,   265,
     266,     0,   314,   315,   316,   317,   219,     0,     0,     0,
     430,    94,   332,    88,   191,   207,   360,   206,   214,   361,
       0,   213,     0,   365,   268,   252,   253,     0,   254,     0,
     255,     0,   363,   367,   368,   369,   373,   377,   380,   400,
     392,   385,   393,   394,   402,   203,   401,   403,   405,   409,
     404,   193,   358,   427,   362,   364,   434,   435,   436,   208,
     107,   106,     0,   109,     0,   460,   423,   101,   418,   419,
       0,   236,   241,   234,   235,   240,     0,   242,     0,   430,
       0,    71,    73,    74,     0,     0,     8,    11,    12,   331,
       0,     0,     0,    32,    54,   462,    84,     0,     0,     0,
       0,     0,     0,    86,     0,   458,   426,   429,     0,    75,
      75,    75,    54,   372,    54,   457,   453,   450,     0,     0,
     389,   391,   390,     0,     0,     0,    75,   371,   370,   103,
      85,    83,     0,   201,   202,   200,   199,     0,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   433,     0,     0,   410,   205,     0,     0,
       0,   432,   101,   100,     0,    39,     0,   422,     0,   111,
       0,     0,   103,     0,   425,    83,    78,     0,   243,   103,
      70,     0,    15,    16,    17,    20,    21,    19,    18,     0,
      13,     0,   462,    62,    28,     0,    30,    54,    54,     0,
      54,     0,   159,   160,   156,   155,   158,   157,     0,    54,
     131,   133,   135,   137,   114,   124,   143,     0,   144,   168,
     169,   150,   174,   148,   179,   149,   145,   151,   146,   147,
     125,   126,   128,   129,   127,     0,    66,   463,   416,   415,
     416,   413,   414,     0,    36,    67,   190,   189,     0,   366,
       0,   333,     0,     0,     0,     0,     0,     0,     0,   451,
       0,    54,   453,   439,     0,   250,     0,   456,   397,   455,
       0,     0,    96,    97,   115,   192,   215,     0,   398,     0,
       0,     0,     0,   257,   263,     0,   249,   374,   375,   376,
     378,   379,   383,   384,   381,   382,   204,   407,   406,   428,
     356,   110,   461,   112,     0,    37,   421,   420,   244,   103,
       0,   462,     0,   240,    72,    14,    22,     9,     7,     0,
       0,     0,    74,    55,   134,   187,   161,   188,     0,     0,
       0,     0,   163,   166,   167,   173,   178,   181,   182,    54,
      54,    54,   154,   467,   464,   466,     0,    40,     0,    68,
       0,   223,     0,     0,     0,     0,     0,   454,   438,     0,
      73,   445,     0,   446,   168,   174,   179,     0,   442,     0,
     440,   256,     0,   226,   103,     0,   117,     0,   216,     0,
       0,     0,   281,   270,   271,   248,   272,   273,     0,     0,
     264,   357,   359,   424,     0,   102,   245,    79,    82,    80,
      81,    54,     0,   238,   246,   239,     0,     0,     0,    27,
     130,   172,     0,   177,   175,   170,   170,     0,     0,   185,
     184,   186,   153,     0,     0,   136,   138,     0,   465,    35,
     349,   340,   339,   341,   342,   336,   337,   343,   338,   344,
     348,   334,   335,   345,   346,   347,   350,   353,   354,     0,
     224,   220,   221,     0,   452,   154,    54,     0,   437,     0,
       0,     0,    98,     0,    54,   274,   217,   399,   222,   269,
     283,   284,   282,   258,   260,     0,    34,   237,     0,   324,
     101,    24,    23,     0,     0,   395,     0,   165,   164,     0,
     218,   183,   213,   180,   103,   120,   122,   119,   121,   132,
     123,   470,   471,   469,   468,     0,   352,   355,     0,     0,
     444,     0,   448,   449,   447,   443,   441,   267,   227,   225,
     116,   118,     0,    41,     0,    41,     0,    38,     0,     0,
     325,   326,   323,   417,     0,   176,     0,   171,     0,   104,
       0,     0,   140,   351,     0,     0,   152,   279,   275,   277,
       0,     0,    89,     0,   286,   259,    33,   329,    29,   396,
     162,   142,   139,   103,   212,     0,     0,    54,     0,   462,
      90,   285,   289,   318,   297,   296,   298,     0,     0,   141,
     276,   278,    42,    43,    47,    47,    91,    95,     0,     0,
     290,   280,     0,     0,     0,   291,   299,   300,     0,    54,
      48,    49,    50,    50,     0,    93,   288,   320,   321,   322,
     319,     0,     0,   306,   309,     0,   308,     0,   301,   303,
       0,   287,   302,    44,     0,    46,    45,    92,   310,   307,
       0,   313,   304,   293,   294,     0,     0,    52,    53,    51,
       0,   311,   305,   295,   292,   312
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,   509,  -579,
    -579,  -579,   505,  -579,  -579,   120,  -579,  -579,   434,    75,
    -579,     3,    19,     0,  -579,  -268,  -579,     4,  -579,  -579,
    -579,  -579,   -40,  -579,   -33,   -78,   208,  -579,   297,  -579,
    -579,  -137,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
     545,  -236,  -579,  -547,   668,   -10,   330,  -579,  -579,   182,
    -579,   417,   284,   285,  -170,  -579,  -579,    66,  -578,  -579,
    -579,  -282,   414,  -579,  -281,   416,  -579,   211,  -280,  -579,
    -579,  -276,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
     -20,  -579,   498,   201,  -579,  -141,  -579,  -579,  -151,  -579,
     213,  -579,  -579,  -579,  -579,  -579,   557,  -579,  -579,  -579,
    -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -332,  -579,  -579,  -579,  -579,    76,  -579,    74,  -579,  -579,
    -579,  -579,  -579,    45,  -579,  -579,  -579,  -579,  -579,  -579,
       5,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,  -579,  -136,  -579,  -579,  -579,  -579,  -579,  -579,  -579,
    -579,   305,   144,   181,   317,   277,  -572,  -579,  -579,  -579,
    -579,  -579,   180,  -185,  -214,   -25,   -77,  -579,  -579,  -579,
     486,  -579,   108,  -579,   553,   549,  -579,  -139,  -579,   296,
    -579,  -579,   489,   502,  -163,  -126,  -579,  -579,  -579,  -579,
    -579,   405,  -579,   214,   561,  -171,   409,  -312,  -579,  -579,
    -579,  -579,  -269,  -579,  -579,   329,  -579
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     8,     9,    10,   282,   176,   177,   178,   279,   407,
      20,   285,    45,    46,    11,   565,   333,   485,   190,   642,
     672,   673,   692,   715,   729,   298,    12,   191,    43,    14,
      15,   192,    16,   170,   299,   172,    25,   265,   266,   489,
      17,   185,    18,   101,   659,   695,   660,   220,   221,   362,
     258,   259,    27,    28,    29,    30,   186,   466,   555,   589,
     300,   301,   302,   303,   347,   590,   631,   632,   305,   451,
     306,   307,   453,   308,   309,   310,   311,   577,   312,   313,
     503,   314,   315,   427,   316,   317,   428,   513,   318,   319,
     102,   103,   104,   105,   246,   106,   107,   108,   109,   110,
     111,   551,   112,   167,   493,   494,   168,    31,    32,   113,
     114,   115,   479,   116,   117,   373,   118,   119,   120,   230,
     637,   475,   476,   613,   638,   639,   640,   477,   561,   562,
     615,   667,   661,   662,   711,   668,   685,   686,   703,   687,
     704,   705,   732,   706,   121,   680,   700,    33,   622,    34,
     495,   180,   122,   340,   536,   537,   538,   539,   123,   482,
     251,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   574,   368,   369,   357,   136,   137,   138,   139,
     140,   141,    49,    50,    51,   332,   569,   157,   158,   159,
     257,   197,   142,   143,   144,   566,   145,   146,   147,   148,
     352,   353,   457,   458,   206,   207,   350,   358,   359,   208,
     149,   255,   326,   327,   434,   435,   594
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     135,   205,   171,   267,    13,   210,   211,   212,   155,   247,
     173,    47,   193,   408,   304,   252,   391,   421,   423,   425,
     152,   324,   416,   426,   339,   339,   334,   194,  -105,   356,
     360,   420,   346,   321,   174,   349,   447,   630,  -105,   324,
     474,   324,   394,   320,   445,   500,   374,   375,   322,  -105,
     183,   321,   264,   321,   173,   576,   249,   209,   209,   209,
     243,   320,   223,   320,   224,  -444,   322,   244,   322,   452,
     454,   455,   400,    23,   546,   456,  -152,   712,   150,   675,
     507,  -251,   446,   449,   364,    19,   676,   -73,    24,   664,
     665,   584,     4,   413,   690,   182,   690,    35,   -73,   507,
     721,   418,   722,     6,   666,   691,   630,   691,   419,  -251,
     245,   675,   193,   272,   273,    73,   274,   411,   386,   -73,
     418,   723,   717,   -73,   324,   324,   -73,   419,   401,   290,
     443,   444,   488,   543,   544,   245,   321,   321,   585,   431,
     560,    22,   275,   234,   235,   236,   320,   320,   225,   151,
     335,   322,   322,   184,  -444,   226,   586,    21,   442,   323,
     276,   514,   -74,   547,     4,  -152,   587,   -73,   508,   588,
     277,  -129,  -129,   341,  -401,     6,   463,   323,   629,   323,
     -74,   153,   467,   349,   283,   470,   245,   245,   324,    26,
     431,   -74,   431,   507,   175,   169,   250,   135,   724,   694,
     321,    36,   245,   270,   271,   278,   725,   726,   697,   363,
     320,   342,   -74,   472,   418,   322,   -74,    37,   203,   -74,
     387,   635,   -73,   -73,   -73,   -73,   -73,   -73,   292,   293,
     294,   295,   296,   297,   473,   606,  -218,   267,   404,   698,
     699,    40,   217,   218,   481,    38,   173,   193,    39,   249,
      44,   -74,   398,    41,   239,   240,   241,   242,   196,   403,
     -74,    48,   412,   323,   601,   602,   603,   324,   324,   415,
     604,   417,    42,   450,   432,   519,   568,   549,   599,   321,
     321,   -87,   173,   584,   557,   -87,   634,  -109,   -87,   320,
     320,    23,   156,   563,   322,   322,   187,   156,   439,    55,
      56,    57,    58,   188,   591,   260,    24,   -99,   179,     1,
     237,   238,   261,   181,   253,   -74,   -74,   -74,   -74,   -74,
     -74,   567,   189,   245,     2,   195,   323,   580,   324,   -87,
     585,   198,   -87,    60,   623,   199,    61,   200,   576,     3,
     321,   -87,   506,    63,   418,   187,   201,   496,   586,   202,
     320,   419,   436,   570,   497,   322,     4,   581,   587,   160,
     -87,   588,   204,   348,     5,   608,   213,     6,    65,    66,
     161,   214,    68,   215,   162,   216,    69,   490,   219,    70,
      71,   163,   164,     7,   611,   165,    80,   222,   512,   486,
     677,   324,   227,   166,   209,    78,   173,   228,   504,   160,
     229,  -360,   231,   321,   232,   323,   323,   343,   344,   345,
     161,   233,  -360,   320,   162,   377,   378,   379,   322,   545,
     248,   163,   164,   256,   361,   165,   161,   173,   491,   268,
     162,   645,   262,  -360,   269,   367,   370,   163,   164,   281,
    -360,   492,   337,   338,   245,  -368,  -368,  -368,  -368,   284,
      96,   -69,   -69,    97,   552,  -368,  -368,  -368,  -368,  -368,
    -368,   328,   193,   325,   160,   330,   323,   336,   600,   239,
     240,   241,   242,     6,   579,   161,   351,   575,   355,   162,
     663,  -360,   173,   184,  -240,   160,   163,   164,   409,   410,
     165,   371,   646,   583,   437,   438,   161,   468,   469,   627,
     162,   483,   469,   541,   469,  -240,   372,   163,   164,   184,
     376,   165,   542,   469,   399,   663,   382,   383,   384,   385,
     243,   173,   617,   618,   620,   621,   625,   626,   575,   323,
     647,   618,  -360,  -360,  -360,  -360,  -360,  -360,  -360,  -360,
    -360,  -360,  -360,  -360,  -360,  -360,  -360,  -360,   652,   653,
     196,   209,   727,   728,   380,   381,   396,   397,   506,    81,
     392,   395,   405,   402,   406,   152,   413,   430,   636,   429,
     431,   433,   440,   441,   348,   448,   173,   624,   461,   674,
     462,   464,   465,   478,   480,   484,   498,   499,   287,   501,
     502,    53,   517,    54,   540,   650,  -448,  -449,  -447,   548,
     550,   649,   553,   173,    55,    56,    57,    58,   288,    59,
     209,   674,   556,   554,   558,    60,   559,   564,    61,   573,
     473,   571,   572,   597,    62,    63,   596,   595,   598,   607,
     609,   612,   173,   616,   289,    64,   614,   628,   626,   633,
     643,   641,   648,   688,   290,   654,   657,  -401,   655,   656,
      65,    66,   658,    67,    68,   472,   678,   707,    69,     4,
     681,    70,    71,    72,   173,   291,   689,   708,   679,   720,
       6,   709,    73,    74,    75,    76,    77,    78,    79,   710,
     714,   718,   719,   735,   280,   733,   734,   286,   619,   393,
     644,    80,   713,   716,   693,   707,   154,   487,    81,   592,
     254,   471,    82,    83,    84,   414,    85,    86,    87,    88,
      89,    90,    91,   422,   515,   424,   516,   578,   593,   669,
     365,   582,   263,   696,   505,   731,    92,    93,    94,    95,
     671,   670,    96,   610,   388,    97,   329,   287,   651,   390,
      53,   331,    54,   292,   293,   294,   295,   296,   297,    98,
      99,   389,   169,    55,    56,    57,    58,   413,    59,   460,
     605,   459,     0,   518,    60,     0,     0,    61,   -55,   354,
       0,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,     0,   289,    64,     0,     0,     0,     0,     0,
       0,     0,     0,   290,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,   291,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,   287,     0,     0,    53,
       0,    54,   292,   293,   294,   295,   296,   297,    98,    99,
       0,   169,    55,    56,    57,    58,   288,    59,     0,     0,
       0,     0,     0,    60,     0,     0,    61,     0,     0,     0,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,   289,    64,     0,     0,     0,     0,     0,     0,
       0,     0,   290,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,     0,     0,     0,    69,     0,     0,    70,
      71,    72,     0,   291,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
      82,    83,    84,     0,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      96,     0,     0,    97,     0,    52,     0,     0,    53,     0,
      54,   292,   293,   294,   295,   296,   297,    98,    99,     0,
     169,    55,    56,    57,    58,   413,    59,     0,     0,     0,
       0,     0,    60,     0,     0,    61,     0,     0,     0,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
       0,   289,    64,     0,     0,     0,     0,     0,     0,     0,
       0,   290,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,     0,     0,     0,    69,     0,     0,    70,    71,
      72,     0,   291,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,    82,
      83,    84,     0,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,    96,
       0,     0,    97,     0,    52,     0,     0,    53,     0,    54,
     292,   293,   294,   295,   296,   297,    98,    99,     0,   169,
      55,    56,    57,    58,   413,    59,     0,     0,     0,     0,
       0,    60,     0,     0,    61,     0,     0,     0,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     0,     0,     0,     0,
     290,     0,     0,     0,     0,     0,    65,    66,     0,    67,
      68,     0,     0,     0,    69,     0,     0,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,    82,    83,
      84,     0,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,    96,     0,
       0,    97,     0,    52,   366,     0,    53,     0,    54,   292,
     293,   294,   295,   296,   297,    98,    99,     0,   169,    55,
      56,    57,    58,     0,    59,     0,     0,     0,     0,    23,
      60,     0,     0,    61,     0,     0,     0,     0,     0,    62,
      63,     0,     0,     0,    24,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,     0,    67,    68,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,    73,    74,    75,
      76,    77,    78,    79,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,    82,    83,    84,
       0,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,     0,    52,     0,     0,    53,     0,    54,     0,     0,
       0,     0,     0,     0,    98,    99,     0,   169,    55,    56,
      57,    58,     0,    59,     0,     0,     0,     0,     0,    60,
       0,     0,    61,   682,     0,     0,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,    66,     0,    67,    68,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,   683,     0,     0,     0,     0,     0,     0,   684,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,    52,     0,     0,    53,     0,    54,     0,     0,     0,
       0,     0,     0,    98,    99,     0,   169,    55,    56,    57,
      58,     0,    59,     0,     0,     0,     0,     0,    60,     0,
       0,    61,     0,     0,     0,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,     0,     0,
       0,    69,     4,     0,    70,    71,    72,     0,     0,     0,
       0,     0,     0,     6,     0,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,    82,    83,    84,     0,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,    96,     0,     0,    97,     0,
      52,     0,     0,    53,     0,    54,     0,     0,     0,     0,
       0,     0,    98,    99,     0,   169,    55,    56,    57,    58,
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
      87,    88,    89,    90,    91,     0,     0,     0,     0,   701,
       0,     0,     0,     0,     0,     0,   702,     0,    92,    93,
      94,    95,     0,     0,    96,     0,     0,    97,     0,    52,
       0,     0,    53,     0,    54,     0,     0,     0,     0,     0,
       0,    98,    99,     0,   169,    55,    56,    57,    58,     0,
      59,     0,     0,     0,     0,     0,    60,     0,     0,    61,
       0,     0,     0,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,     0,     0,    81,
       0,     0,     0,    82,    83,    84,     0,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,   730,     0,
       0,     0,     0,     0,     0,   702,     0,    92,    93,    94,
      95,     0,     0,    96,     0,     0,    97,     0,    52,     0,
       0,    53,     0,    54,     0,     0,     0,     0,     0,     0,
      98,    99,     0,   169,    55,    56,    57,    58,     0,    59,
       0,     0,     0,     0,     0,    60,     0,     0,    61,     0,
       0,     0,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,     0,
       0,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,    82,    83,    84,     0,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    96,     0,     0,    97,     0,    52,     0,     0,
      53,     0,    54,     0,     0,     0,     0,     0,     0,    98,
      99,     0,   169,    55,    56,    57,    58,     0,    59,     0,
       0,     0,     0,   509,   510,     0,     0,    61,     0,     0,
       0,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,     0,     0,     0,    69,     0,     0,
     511,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,    52,     0,     0,    53,
       0,    54,     0,     0,     0,     0,     0,     0,    98,    99,
       0,   169,    55,    56,    57,    58,     0,    59,     0,     0,
       0,     0,     0,    60,     0,     0,    61,     0,     0,     0,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,     0,     0,     0,    69,     0,     0,    70,
      71,    72,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
      82,    83,    84,     0,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      96,     0,     0,    97,     0,    52,     0,     0,    53,     0,
      54,     0,     0,     0,     0,     0,     0,    98,    99,     0,
     100,    55,    56,    57,    58,     0,    59,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,    96,
       0,     0,    97,     0,    52,     0,     0,    53,     0,    54,
       0,     0,     0,     0,     0,     0,    98,    99,     0,   169,
      55,    56,    57,    58,     0,    59,     0,     0,     0,     0,
       0,    60,     0,     0,    61,     0,     0,     0,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,    66,     0,    67,
      68,     0,     0,     0,    69,     0,     0,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
       0,     0,     0,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,    96,     0,
       0,    97,     0,    52,     0,     0,    53,     0,    54,     0,
       0,     0,     0,     0,     0,    98,    99,     0,   169,    55,
      56,    57,    58,     0,    59,     0,     0,     0,     0,     0,
      60,     0,     0,    61,     0,     0,     0,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,     0,  -361,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -361,     0,     0,     0,     0,    65,    66,     0,    67,    68,
       0,  -363,     0,    69,     0,     0,    70,    71,    72,     0,
       0,  -361,  -363,     0,     0,     0,     0,    73,  -361,     0,
       0,     0,    78,    79,     0,     0,  -362,     0,     0,     0,
       0,     0,     0,  -363,     0,     0,    80,  -362,     0,     0,
    -363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,    87,    88,    89,    90,    91,  -362,  -361,
       0,     0,     0,     0,     0,  -362,     0,     0,     0,     0,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,  -363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   520,     0,     0,     0,     0,   169,     0,     0,
       0,     0,     0,     0,     0,     0,  -362,     0,     0,     0,
    -361,  -361,  -361,  -361,  -361,  -361,  -361,  -361,  -361,  -361,
    -361,  -361,  -361,  -361,  -361,  -361,     0,     0,     0,     0,
       0,     0,  -363,  -363,  -363,  -363,  -363,  -363,  -363,  -363,
    -363,  -363,  -363,  -363,  -363,  -363,  -363,  -363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -362,  -362,  -362,
    -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,
    -362,  -362,  -362,   521,   522,   523,   524,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   534,   535
};

static const yytype_int16 yycheck[] =
{
      25,    79,    35,   166,     0,    82,    83,    84,    28,   135,
      35,    21,    52,   282,   184,   154,   252,   299,   299,   299,
       3,   184,   290,   299,     4,     4,   189,    52,     3,   214,
     215,   299,   202,   184,     3,   206,   348,   584,    24,   202,
     372,   204,   256,   184,     5,     4,   231,   232,   184,    24,
      46,   202,    24,   204,    79,    47,    13,    82,    83,    84,
      24,   202,    20,   204,    22,     5,   202,    31,   204,   351,
     351,   351,     5,    29,     5,   351,     5,   133,    29,   657,
      34,   120,    94,   351,   221,    28,   658,    23,    44,   134,
     135,     3,    74,    23,    32,     5,    32,     3,    34,    34,
     131,    55,   133,    85,   149,    43,   653,    43,    62,   148,
     166,   689,   152,    21,    22,    87,    24,   287,   244,    55,
      55,    53,   694,    59,   287,   288,    62,    62,   265,    59,
     344,   345,   401,   445,   446,   166,   287,   288,    50,   151,
     472,    58,    50,   167,   168,   169,   287,   288,   106,   100,
     190,   287,   288,    86,    94,   113,    68,    51,   343,   184,
      68,   429,     5,    94,    74,    94,    78,   103,   103,    81,
      78,   151,   152,   198,   166,    85,   361,   202,   164,   204,
      23,   164,   367,   354,   180,   370,   166,   166,   351,   164,
     151,    34,   151,    34,   163,   167,   153,   222,   130,    99,
     351,    24,   166,     4,     5,   113,   138,   139,   115,   219,
     351,   167,    55,     3,    55,   351,    59,     0,    74,    62,
     245,    62,   158,   159,   160,   161,   162,   163,   158,   159,
     160,   161,   162,   163,    24,   547,     3,   400,   271,   146,
     147,    48,    98,    99,     8,     7,   271,   287,     7,    13,
      24,    94,   262,    60,   154,   155,   156,   157,    25,   269,
     103,    24,   287,   288,   546,   546,   546,   430,   431,   289,
     546,   291,    79,   351,   307,   438,     3,   462,   546,   430,
     431,     0,   307,     3,   469,     4,   598,    24,     7,   430,
     431,    29,    24,   478,   430,   431,     6,    24,   338,    19,
      20,    21,    22,    13,    24,     6,    44,    24,    29,    27,
     164,   165,    13,     3,    31,   158,   159,   160,   161,   162,
     163,   491,     3,   166,    42,    24,   351,    24,   491,    48,
      50,     3,    51,    30,   570,     3,    33,     3,    47,    57,
     491,    60,   419,    40,    55,     6,     3,     6,    68,     3,
     491,    62,    13,   492,    13,   491,    74,   508,    78,    41,
      79,    81,    96,    97,    82,   550,     3,    85,    65,    66,
      52,     3,    69,     3,    56,     3,    73,   402,    51,    76,
      77,    63,    64,   101,   554,    67,   106,     5,   421,   399,
     659,   554,     3,    75,   419,    92,   421,     3,   418,    41,
     148,    23,     3,   554,     3,   430,   431,   199,   200,   201,
      52,     3,    34,   554,    56,   234,   235,   236,   554,   452,
     164,    63,    64,     3,   216,    67,    52,   452,    70,    72,
      56,   616,    61,    55,    61,   227,   228,    63,    64,   102,
      62,    83,     4,     5,   166,   154,   155,   156,   157,    24,
     147,     4,     5,   150,   464,   164,   165,   166,   167,   168,
     169,    24,   502,   104,    41,    24,   491,     4,   546,   154,
     155,   156,   157,    85,   507,    52,    96,   502,     4,    56,
     643,   103,   507,    86,    61,    41,    63,    64,     4,     5,
      67,     3,   618,   513,     4,     5,    52,     4,     5,   576,
      56,     4,     5,     4,     5,    61,   120,    63,    64,    86,
       4,    67,     4,     5,    61,   678,   239,   240,   241,   242,
      24,   546,     4,     5,    45,    46,     4,     5,   553,   554,
       4,     5,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,     4,     5,
      25,   576,    37,    38,   237,   238,   260,   261,   635,   113,
      24,    24,     4,   163,    24,     3,    23,   152,   601,    59,
     151,    24,    31,     4,    97,    95,   601,   573,     4,   657,
       5,     5,    53,     5,     4,     3,    31,    24,     3,    68,
       3,     6,   163,     8,     4,   628,     5,     5,     5,    95,
       5,   626,    35,   628,    19,    20,    21,    22,    23,    24,
     635,   689,     4,    54,     4,    30,     4,     4,    33,     3,
      24,    24,    24,     4,    39,    40,    25,    13,     5,     4,
       4,   137,   657,     5,    49,    50,   136,   152,     5,    24,
      35,    71,     4,   668,    59,     4,    35,   166,     5,    31,
      65,    66,    98,    68,    69,     3,     5,   682,    73,    74,
       4,    76,    77,    78,   689,    80,     5,   133,   145,   152,
      85,   140,    87,    88,    89,    90,    91,    92,    93,   128,
      36,   133,   140,   131,   175,   129,   140,   182,   568,   255,
     615,   106,   689,   693,   675,   720,    28,   400,   113,   517,
     155,   371,   117,   118,   119,   288,   121,   122,   123,   124,
     125,   126,   127,   299,   430,   299,   431,   506,   517,   653,
     222,   508,   165,   678,   419,   720,   141,   142,   143,   144,
     656,   655,   147,   553,   248,   150,   187,     3,   630,   250,
       6,   188,     8,   158,   159,   160,   161,   162,   163,   164,
     165,   249,   167,    19,    20,    21,    22,    23,    24,   354,
     546,   352,    -1,   434,    30,    -1,    -1,    33,    34,   208,
      -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,
      76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
      -1,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
      -1,   147,    -1,    -1,   150,    -1,     3,    -1,    -1,     6,
      -1,     8,   158,   159,   160,   161,   162,   163,   164,   165,
      -1,   167,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,    65,    66,
      -1,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,
     117,   118,   119,    -1,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,    -1,
     147,    -1,    -1,   150,    -1,     3,    -1,    -1,     6,    -1,
       8,   158,   159,   160,   161,   162,   163,   164,   165,    -1,
     167,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,
      68,    69,    -1,    -1,    -1,    73,    -1,    -1,    76,    77,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,    -1,   147,
      -1,    -1,   150,    -1,     3,    -1,    -1,     6,    -1,     8,
     158,   159,   160,   161,   162,   163,   164,   165,    -1,   167,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,
      69,    -1,    -1,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,   117,   118,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,
      -1,   150,    -1,     3,     4,    -1,     6,    -1,     8,   158,
     159,   160,   161,   162,   163,   164,   165,    -1,   167,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,
      30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,
      -1,    -1,    -1,    73,    -1,    -1,    76,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,    -1,   117,   118,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,
     150,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   165,    -1,   167,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,
      -1,    -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,    -1,    -1,   117,   118,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
     141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,   150,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   164,   165,    -1,   167,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    -1,    -1,
      -1,    73,    74,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    87,    88,    89,    90,    91,
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
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,
     143,   144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,
      -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,   164,   165,    -1,   167,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    68,    69,    -1,    -1,    -1,    73,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,   143,
     144,    -1,    -1,   147,    -1,    -1,   150,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,    -1,   167,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    -1,    68,    69,    -1,    -1,    -1,    73,    -1,
      -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    -1,
      -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,   117,   118,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,    -1,   147,    -1,    -1,   150,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   164,
     165,    -1,   167,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    29,    30,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
      -1,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
      -1,   147,    -1,    -1,   150,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   164,   165,
      -1,   167,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      -1,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,    -1,   147,
      -1,    -1,   150,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,   164,   165,    -1,   167,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,
      69,    -1,    -1,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,
      -1,   150,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,   164,   165,    -1,   167,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,
      -1,    23,    -1,    73,    -1,    -1,    76,    77,    78,    -1,
      -1,    55,    34,    -1,    -1,    -1,    -1,    87,    62,    -1,
      -1,    -1,    92,    93,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,   106,    34,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,   127,    55,   103,
      -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,    -1,   147,    -1,    -1,
     150,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    42,    57,    74,    82,    85,   101,   173,   174,
     175,   186,   198,   199,   201,   202,   204,   212,   214,    28,
     182,    51,    58,    29,    44,   208,   164,   224,   225,   226,
     227,   279,   280,   319,   321,     3,    24,     0,     7,     7,
      48,    60,    79,   200,    24,   184,   185,   227,    24,   354,
     355,   356,     3,     6,     8,    19,    20,    21,    22,    24,
      30,    33,    39,    40,    50,    65,    66,    68,    69,    73,
      76,    77,    78,    87,    88,    89,    90,    91,    92,    93,
     106,   113,   117,   118,   119,   121,   122,   123,   124,   125,
     126,   127,   141,   142,   143,   144,   147,   150,   164,   165,
     167,   215,   262,   263,   264,   265,   267,   268,   269,   270,
     271,   272,   274,   281,   282,   283,   285,   286,   288,   289,
     290,   316,   324,   330,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   347,   348,   349,   350,   351,
     352,   353,   364,   365,   366,   368,   369,   370,   371,   382,
      29,   100,     3,   164,   226,   262,    24,   359,   360,   361,
      41,    52,    56,    63,    64,    67,    75,   275,   278,   167,
     205,   206,   207,   347,     3,   163,   177,   178,   179,    29,
     323,     3,     5,   199,    86,   213,   228,     6,    13,     3,
     190,   199,   203,   204,   347,    24,    25,   363,     3,     3,
       3,     3,     3,   334,    96,   207,   376,   377,   381,   347,
     348,   348,   348,     3,     3,     3,     3,   334,   334,    51,
     219,   220,     5,    20,    22,   106,   113,     3,     3,   148,
     291,     3,     3,     3,   167,   168,   169,   164,   165,   154,
     155,   156,   157,    24,    31,   166,   266,   367,   164,    13,
     153,   332,   359,    31,   222,   383,     3,   362,   222,   223,
       6,    13,    61,   278,    24,   209,   210,   366,    72,    61,
       4,     5,    21,    22,    24,    50,    68,    78,   113,   180,
     180,   102,   176,   199,    24,   183,   184,     3,    23,    49,
      59,    80,   158,   159,   160,   161,   162,   163,   197,   206,
     232,   233,   234,   235,   236,   240,   242,   243,   245,   246,
     247,   248,   250,   251,   253,   254,   256,   257,   260,   261,
     267,   270,   324,   347,   366,   104,   384,   385,    24,   357,
      24,   356,   357,   188,   366,   204,     4,     4,     5,     4,
     325,   347,   167,   208,   208,   208,   236,   236,    97,   377,
     378,    96,   372,   373,   376,     4,   345,   347,   379,   380,
     345,   208,   221,   227,   213,   264,     4,   208,   345,   346,
     208,     3,   120,   287,   345,   345,     4,   335,   335,   335,
     336,   336,   337,   337,   337,   337,   367,   347,   352,   365,
     364,   223,    24,   190,   346,    24,   361,   361,   227,    61,
       5,   213,   163,   227,   206,     4,    24,   181,   384,     4,
       5,   236,   347,    23,   233,   262,   197,   262,    55,    62,
     197,   243,   244,   246,   247,   250,   253,   255,   258,    59,
     152,   151,   206,    24,   386,   387,    13,     4,     5,   204,
      31,     4,   345,   346,   346,     5,    94,   379,    95,   197,
     207,   241,   243,   244,   246,   250,   253,   374,   375,   378,
     373,     4,     5,   345,     5,    53,   229,   345,     4,     5,
     345,   228,     3,    24,   292,   293,   294,   299,     5,   284,
       4,     8,   331,     4,     3,   189,   227,   210,   384,   211,
     347,    70,    83,   276,   277,   322,     6,    13,    31,    24,
       4,    68,     3,   252,   262,   333,   348,    34,   103,    29,
      30,    76,   206,   259,   197,   234,   235,   163,   387,   366,
      24,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   326,   327,   328,   329,
       4,     4,     4,   379,   379,   206,     5,    94,    95,   345,
       5,   273,   227,    35,    54,   230,     4,   345,     4,     4,
     292,   300,   301,   345,     4,   187,   367,   236,     3,   358,
     359,    24,    24,     3,   344,   347,    47,   249,   249,   206,
      24,   270,   272,   262,     3,    50,    68,    78,    81,   231,
     237,    24,   231,   265,   388,    13,    25,     4,     5,   197,
     207,   243,   246,   250,   253,   375,   379,     4,   345,     4,
     344,   236,   137,   295,   136,   302,     5,     4,     5,   187,
      45,    46,   320,   223,   199,     4,     5,   348,   152,   164,
     225,   238,   239,    24,   379,    62,   206,   292,   296,   297,
     298,    71,   191,    35,   191,   345,   367,     4,     4,   347,
     206,   354,     4,     5,     4,     5,    31,    35,    98,   216,
     218,   304,   305,   366,   134,   135,   149,   303,   307,   239,
     297,   299,   192,   193,   207,   240,   338,   384,     5,   145,
     317,     4,    34,   132,   139,   308,   309,   311,   347,     5,
      32,    43,   194,   194,    99,   217,   305,   115,   146,   147,
     318,   132,   139,   310,   312,   313,   315,   347,   133,   140,
     128,   306,   133,   193,    36,   195,   195,   338,   133,   140,
     152,   131,   133,    53,   130,   138,   139,    37,    38,   196,
     132,   312,   314,   129,   140,   131
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
     198,   199,   199,   200,   200,   200,   201,   202,   203,   203,
     204,   205,   205,   206,   207,   208,   208,   208,   209,   209,
     210,   211,   212,   213,   213,   214,   214,   215,   215,   216,
     216,   217,   217,   218,   219,   219,   220,   221,   221,   222,
     222,   223,   223,   224,   224,   225,   225,   225,   226,   226,
     227,   227,   227,   227,   228,   229,   229,   230,   230,   231,
     231,   231,   231,   231,   232,   232,   232,   232,   232,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     238,   238,   239,   240,   240,   240,   240,   240,   240,   240,
     240,   240,   241,   242,   242,   243,   243,   243,   243,   243,
     243,   243,   244,   245,   246,   247,   248,   248,   248,   248,
     249,   249,   250,   251,   251,   252,   252,   253,   254,   254,
     255,   256,   257,   258,   259,   259,   259,   260,   261,   262,
     262,   263,   263,   264,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   266,   266,   266,   267,   267,   268,   268,
     268,   268,   269,   270,   270,   270,   270,   270,   271,   271,
     272,   272,   272,   272,   272,   272,   273,   273,   274,   274,
     274,   274,   274,   274,   275,   275,   275,   276,   277,   277,
     278,   278,   278,   278,   279,   280,   280,   280,   281,   282,
     282,   282,   282,   282,   282,   282,   283,   284,   284,   284,
     285,   286,   286,   287,   288,   289,   289,   290,   291,   291,
     292,   293,   293,   294,   295,   295,   296,   296,   297,   298,
     299,   300,   300,   301,   302,   302,   303,   303,   304,   304,
     305,   306,   306,   306,   306,   306,   307,   307,   307,   308,
     308,   309,   309,   309,   310,   311,   312,   312,   312,   313,
     313,   314,   314,   315,   316,   316,   316,   316,   317,   317,
     318,   318,   318,   319,   320,   320,   320,   321,   321,   322,
     323,   323,   324,   325,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     327,   327,   328,   329,   329,   330,   331,   331,   332,   332,
     333,   333,   333,   333,   333,   333,   333,   333,   334,   335,
     335,   335,   335,   336,   336,   336,   336,   337,   337,   337,
     338,   338,   338,   338,   338,   339,   340,   340,   340,   340,
     340,   340,   341,   342,   343,   344,   344,   345,   346,   346,
     347,   347,   347,   348,   349,   349,   350,   350,   351,   352,
     353,   354,   354,   355,   356,   356,   357,   358,   359,   360,
     360,   360,   361,   362,   362,   363,   363,   364,   364,   365,
     365,   365,   366,   367,   368,   369,   369,   370,   371,   372,
     372,   373,   374,   374,   375,   375,   375,   375,   375,   375,
     376,   376,   377,   378,   378,   379,   380,   381,   382,   382,
     383,   383,   384,   384,   385,   386,   386,   387,   387,   388,
     388,   388
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
       1,     1,     4,     1,     1,     1,     5,     5,     3,     1,
       4,     1,     3,     1,     1,     0,     1,     1,     1,     3,
       3,     1,     6,     0,     1,     4,     1,     1,     1,     0,
       1,     0,     2,     3,     0,     8,     2,     1,     3,     0,
       1,     0,     3,     0,     1,     0,     2,     2,     0,     1,
       4,     3,     4,     1,     2,     0,     3,     0,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     4,     1,     2,     1,     3,     1,     3,     3,
       1,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     1,     1,     1,     1,     1,
       1,     2,     5,     2,     4,     4,     2,     2,     1,     1,
       0,     2,     3,     2,     1,     1,     3,     3,     2,     1,
       3,     2,     2,     3,     1,     1,     1,     2,     2,     3,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     8,     1,     1,     3,     4,     5,     1,     1,
       5,     5,     5,     4,     5,     6,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       0,     1,     1,     2,     4,     5,     5,     1,     4,     3,
       3,     1,     1,     1,     1,     1,     4,     0,     2,     4,
       5,     1,     1,     1,     4,     1,     1,     6,     0,     4,
       1,     1,     1,     1,     0,     2,     3,     1,     3,     1,
       6,     0,     1,     1,     0,     3,     0,     3,     3,     1,
       2,     0,     3,     2,     2,     3,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     4,     1,     2,     1,     1,
       2,     1,     2,     2,     1,     1,     1,     1,     0,     2,
       1,     1,     1,     7,     0,     1,     1,     1,     1,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     6,     0,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       2,     2,     2,     1,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     1,     1,     1,     2,
       2,     2,     1,     1,     1,     1,     3,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       2,     1,     1,     3,     3,     3,     1,     2,     1,     1,
       3,     3,     2,     0,     3,     0,     1,     1,     3,     2,
       1,     1,     2,     1,     1,     1,     1,     5,     4,     1,
       2,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     4,     0,     2,     1,     1,     1,     2,     1,
       0,     2,     0,     1,     2,     2,     1,     1,     3,     1,
       1,     1
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

  case 68: /* values_commalist: values_commalist ',' values_or_query_spec  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 69: /* values_commalist: values_or_query_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 70: /* values_or_query_spec: SQL_TOKEN_VALUES '(' row_value_constructor_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 71: /* row_value_constructor_commalist: row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 72: /* row_value_constructor_commalist: row_value_constructor_commalist ',' row_value_constructor  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 75: /* opt_all_distinct: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 78: /* assignment_commalist: assignment  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 79: /* assignment_commalist: assignment_commalist ',' assignment  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 80: /* assignment: column_ref SQL_EQUAL update_source  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 82: /* update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 83: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 85: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 87: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 89: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 91: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 92: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 93: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 94: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 95: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_window_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
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

  case 96: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 97: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 98: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 99: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 101: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 102: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 103: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 104: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 105: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 106: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 107: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 108: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 109: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 110: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 111: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 112: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 114: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 115: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 116: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 117: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 118: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 130: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 132: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 134: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 136: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 138: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 139: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 140: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 141: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 142: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 152: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 153: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 154: /* comparison_predicate: comparison row_value_constructor  */
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

  case 161: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 162: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 163: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 164: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 165: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 166: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 167: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 168: /* like_predicate: character_like_predicate_part_2  */
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

  case 169: /* like_predicate: other_like_predicate_part_2  */
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

  case 170: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 171: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 172: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 173: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 174: /* test_for_null: null_predicate_part_2  */
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

  case 175: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 176: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 177: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 178: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 179: /* in_predicate: in_predicate_part_2  */
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

  case 180: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 181: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 182: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 183: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 187: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 188: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 189: /* subquery: '(' values_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 190: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 191: /* scalar_exp_commalist: select_sublist  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 192: /* scalar_exp_commalist: scalar_exp_commalist ',' select_sublist  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 199: /* literal: literal SQL_TOKEN_STRING  */
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

  case 200: /* literal: literal SQL_TOKEN_INT  */
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

  case 201: /* literal: literal SQL_TOKEN_REAL_NUM  */
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

  case 202: /* literal: literal SQL_TOKEN_APPROXNUM  */
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

  case 203: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 204: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 212: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));


        }
    break;

  case 215: /* fct_spec: function_name '(' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 216: /* fct_spec: function_name '(' function_args_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 217: /* fct_spec: function_name '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 220: /* aggregate_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 221: /* aggregate_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 222: /* aggregate_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 223: /* aggregate_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 224: /* aggregate_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 225: /* aggregate_fct: SQL_TOKEN_GROUP_CONCAT '(' opt_all_distinct function_arg opt_function_arg ')'  */
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

  case 226: /* opt_function_arg: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 227: /* opt_function_arg: ',' function_arg  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 234: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 235: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 236: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 237: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 240: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 241: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 243: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 244: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 245: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 246: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 248: /* window_function: window_function_type opt_filter_clause SQL_TOKEN_OVER window_name_or_specification  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 249: /* window_function_type: rank_function_type '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 250: /* window_function_type: SQL_TOKEN_ROW_NUMBER '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 256: /* ntile_function: SQL_TOKEN_NTILE '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 257: /* opt_lead_or_lag_function: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 258: /* opt_lead_or_lag_function: ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 259: /* opt_lead_or_lag_function: ',' function_arg ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 260: /* lead_or_lag_function: lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 264: /* first_or_last_value_function: first_or_last_value '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 267: /* nth_value_function: SQL_TOKEN_NTH_VALUE '(' function_arg ',' function_arg ')'  */
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

  case 268: /* opt_filter_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 269: /* opt_filter_clause: SQL_TOKEN_FILTER '(' where_clause ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 274: /* opt_window_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 275: /* opt_window_clause: SQL_TOKEN_WINDOW window_definition_list  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 276: /* window_definition_list: window_definition_list ',' window_definition  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 277: /* window_definition_list: window_definition  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 278: /* window_definition: new_window_name SQL_TOKEN_AS window_specification  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 280: /* window_specification: '(' opt_existing_window_name opt_window_partition_clause opt_order_by_clause opt_window_frame_clause ')'  */
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

  case 281: /* opt_existing_window_name: %empty  */
                                 {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 284: /* opt_window_partition_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 285: /* opt_window_partition_clause: SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 286: /* opt_window_frame_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 287: /* opt_window_frame_clause: window_frame_units window_frame_extent opt_window_frame_exclusion  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 288: /* window_partition_column_reference_list: window_partition_column_reference_list ',' window_partition_column_reference  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 289: /* window_partition_column_reference_list: window_partition_column_reference  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 290: /* window_partition_column_reference: column_ref opt_collate_clause  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 291: /* opt_window_frame_exclusion: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 292: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 293: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 294: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 295: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 301: /* window_frame_start: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 302: /* window_frame_start: value_exp SQL_TOKEN_PRECEDING  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 303: /* window_frame_start: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 304: /* window_frame_preceding: value_exp SQL_TOKEN_PRECEDING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 305: /* window_frame_between: SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 307: /* window_frame_bound: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* window_frame_bound_1: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 312: /* window_frame_bound_2: SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 313: /* window_frame_following: value_exp SQL_TOKEN_FOLLOWING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 318: /* opt_collate_clause: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 319: /* opt_collate_clause: SQL_TOKEN_COLLATE collating_function  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 323: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 324: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 329: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 330: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 350: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 351: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 352: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 355: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 356: /* opt_optional_prop: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 357: /* opt_optional_prop: '?'  */
             {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
    }
    break;

  case 358: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 359: /* opt_extract_value: SQL_ARROW property_path opt_optional_prop  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 366: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 370: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 371: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 372: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 374: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 375: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 376: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 385: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 388: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 389: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 390: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 392: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 393: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 394: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 395: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 396: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 398: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 399: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 406: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 407: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 410: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 411: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 412: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 413: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 414: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 415: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 416: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 417: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 418: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 419: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 420: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 421: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 422: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 423: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 424: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 425: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 426: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 427: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 428: /* property_path: property_path '.' property_path_entry  */
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

  case 429: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 430: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 431: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 432: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 437: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 438: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 439: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 440: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 441: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 442: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 443: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 450: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 451: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 452: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 453: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 454: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 458: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 459: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 460: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 461: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 462: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 464: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 465: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 466: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 467: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 468: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
