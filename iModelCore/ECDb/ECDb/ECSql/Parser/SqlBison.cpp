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
  YYSYMBOL_SQL_TOKEN_GROUP_CONCAT = 152,   /* SQL_TOKEN_GROUP_CONCAT  */
  YYSYMBOL_SQL_TOKEN_OR = 153,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 154,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_ARROW = 155,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 156,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 157,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 158,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 159,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 160,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 161,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 162,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 163,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 164,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 165,                /* SQL_EQUAL  */
  YYSYMBOL_166_ = 166,                     /* '+'  */
  YYSYMBOL_167_ = 167,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 168,               /* SQL_CONCAT  */
  YYSYMBOL_169_ = 169,                     /* '*'  */
  YYSYMBOL_170_ = 170,                     /* '/'  */
  YYSYMBOL_171_ = 171,                     /* '%'  */
  YYSYMBOL_172_ = 172,                     /* '='  */
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
  YYSYMBOL_delete_statement_searched = 203, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 204,         /* insert_statement  */
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
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 225, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 226,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 227, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 228,                /* table_ref  */
  YYSYMBOL_where_clause = 229,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 230,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 231,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 232,              /* truth_value  */
  YYSYMBOL_boolean_primary = 233,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 234,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 235,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 236,             /* boolean_term  */
  YYSYMBOL_search_condition = 237,         /* search_condition  */
  YYSYMBOL_type_predicate = 238,           /* type_predicate  */
  YYSYMBOL_type_list = 239,                /* type_list  */
  YYSYMBOL_type_list_item = 240,           /* type_list_item  */
  YYSYMBOL_predicate = 241,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 242, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 243,     /* comparison_predicate  */
  YYSYMBOL_comparison = 244,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 245, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 246,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 247, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 248, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 249,           /* like_predicate  */
  YYSYMBOL_opt_escape = 250,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 251,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 252,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 253,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 254,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 255,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 256, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 257,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 258,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 259, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 260,             /* any_all_some  */
  YYSYMBOL_existence_test = 261,           /* existence_test  */
  YYSYMBOL_unique_test = 262,              /* unique_test  */
  YYSYMBOL_subquery = 263,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 264,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 265,           /* select_sublist  */
  YYSYMBOL_literal = 266,                  /* literal  */
  YYSYMBOL_as_clause = 267,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 268,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 269,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 270,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 271,                 /* fct_spec  */
  YYSYMBOL_function_name = 272,            /* function_name  */
  YYSYMBOL_general_set_fct = 273,          /* general_set_fct  */
  YYSYMBOL_opt_function_arg = 274,         /* opt_function_arg  */
  YYSYMBOL_set_fct_type = 275,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 276,          /* outer_join_type  */
  YYSYMBOL_join_condition = 277,           /* join_condition  */
  YYSYMBOL_join_spec = 278,                /* join_spec  */
  YYSYMBOL_join_type = 279,                /* join_type  */
  YYSYMBOL_cross_union = 280,              /* cross_union  */
  YYSYMBOL_qualified_join = 281,           /* qualified_join  */
  YYSYMBOL_window_function = 282,          /* window_function  */
  YYSYMBOL_window_function_type = 283,     /* window_function_type  */
  YYSYMBOL_ntile_function = 284,           /* ntile_function  */
  YYSYMBOL_opt_lead_or_lag_function = 285, /* opt_lead_or_lag_function  */
  YYSYMBOL_lead_or_lag_function = 286,     /* lead_or_lag_function  */
  YYSYMBOL_lead_or_lag = 287,              /* lead_or_lag  */
  YYSYMBOL_lead_or_lag_extent = 288,       /* lead_or_lag_extent  */
  YYSYMBOL_first_or_last_value_function = 289, /* first_or_last_value_function  */
  YYSYMBOL_first_or_last_value = 290,      /* first_or_last_value  */
  YYSYMBOL_nth_value_function = 291,       /* nth_value_function  */
  YYSYMBOL_opt_filter_clause = 292,        /* opt_filter_clause  */
  YYSYMBOL_window_name = 293,              /* window_name  */
  YYSYMBOL_window_name_or_specification = 294, /* window_name_or_specification  */
  YYSYMBOL_in_line_window_specification = 295, /* in_line_window_specification  */
  YYSYMBOL_opt_window_clause = 296,        /* opt_window_clause  */
  YYSYMBOL_window_definition_list = 297,   /* window_definition_list  */
  YYSYMBOL_window_definition = 298,        /* window_definition  */
  YYSYMBOL_new_window_name = 299,          /* new_window_name  */
  YYSYMBOL_window_specification = 300,     /* window_specification  */
  YYSYMBOL_opt_existing_window_name = 301, /* opt_existing_window_name  */
  YYSYMBOL_existing_window_name = 302,     /* existing_window_name  */
  YYSYMBOL_opt_window_partition_clause = 303, /* opt_window_partition_clause  */
  YYSYMBOL_opt_window_frame_clause = 304,  /* opt_window_frame_clause  */
  YYSYMBOL_window_partition_column_reference_list = 305, /* window_partition_column_reference_list  */
  YYSYMBOL_window_partition_column_reference = 306, /* window_partition_column_reference  */
  YYSYMBOL_opt_window_frame_exclusion = 307, /* opt_window_frame_exclusion  */
  YYSYMBOL_window_frame_units = 308,       /* window_frame_units  */
  YYSYMBOL_window_frame_extent = 309,      /* window_frame_extent  */
  YYSYMBOL_window_frame_start = 310,       /* window_frame_start  */
  YYSYMBOL_window_frame_preceding = 311,   /* window_frame_preceding  */
  YYSYMBOL_window_frame_between = 312,     /* window_frame_between  */
  YYSYMBOL_window_frame_bound = 313,       /* window_frame_bound  */
  YYSYMBOL_window_frame_bound_1 = 314,     /* window_frame_bound_1  */
  YYSYMBOL_window_frame_bound_2 = 315,     /* window_frame_bound_2  */
  YYSYMBOL_window_frame_following = 316,   /* window_frame_following  */
  YYSYMBOL_rank_function_type = 317,       /* rank_function_type  */
  YYSYMBOL_opt_collate_clause = 318,       /* opt_collate_clause  */
  YYSYMBOL_collating_function = 319,       /* collating_function  */
  YYSYMBOL_ecrelationship_join = 320,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 321, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 322,             /* joined_table  */
  YYSYMBOL_named_columns_join = 323,       /* named_columns_join  */
  YYSYMBOL_all = 324,                      /* all  */
  YYSYMBOL_scalar_subquery = 325,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 326,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 327, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 328,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 329,        /* cast_target_array  */
  YYSYMBOL_cast_target = 330,              /* cast_target  */
  YYSYMBOL_cast_spec = 331,                /* cast_spec  */
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
#define YYLAST   3091

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  174
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  215
/* YYNRULES -- Number of rules.  */
#define YYNRULES  467
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  736

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
       2,     2,     2,     2,     2,     2,     2,   171,     2,     2,
       3,     4,   169,   166,     5,   167,    13,   170,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   172,     2,     8,     2,     2,     2,     2,     2,     2,
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
     161,   162,   163,   164,   165,   168,   173
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   240,   240,   241,   242,   243,   247,   253,   265,   268,
     276,   279,   280,   284,   292,   300,   301,   302,   303,   304,
     305,   306,   310,   315,   320,   332,   335,   339,   344,   352,
     366,   372,   380,   392,   397,   405,   410,   420,   421,   429,
     430,   441,   442,   452,   457,   465,   474,   485,   486,   487,
     491,   492,   501,   501,   505,   506,   512,   513,   514,   515,
     516,   520,   525,   536,   537,   538,   543,   555,   564,   574,
     579,   587,   590,   594,   595,   596,   600,   603,   609,   616,
     619,   631,   632,   640,   648,   652,   657,   660,   661,   664,
     665,   673,   682,   683,   698,   708,   711,   717,   718,   722,
     725,   735,   736,   743,   744,   749,   757,   758,   765,   773,
     781,   789,   793,   802,   803,   813,   814,   822,   823,   824,
     825,   826,   829,   830,   831,   832,   833,   834,   835,   845,
     846,   856,   857,   865,   866,   875,   876,   886,   895,   900,
     908,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     931,   938,   945,   970,   971,   972,   973,   974,   975,   976,
     984,   994,  1002,  1012,  1022,  1028,  1034,  1056,  1081,  1082,
    1089,  1098,  1104,  1120,  1124,  1132,  1141,  1147,  1163,  1172,
    1181,  1190,  1200,  1201,  1202,  1206,  1214,  1220,  1231,  1236,
    1243,  1247,  1248,  1249,  1250,  1251,  1253,  1265,  1277,  1289,
    1305,  1306,  1312,  1316,  1317,  1320,  1321,  1322,  1323,  1326,
    1338,  1339,  1340,  1347,  1356,  1370,  1375,  1391,  1407,  1416,
    1424,  1433,  1442,  1453,  1471,  1472,  1481,  1482,  1483,  1484,
    1485,  1486,  1490,  1495,  1500,  1507,  1515,  1516,  1519,  1520,
    1525,  1526,  1534,  1546,  1556,  1565,  1569,  1580,  1587,  1594,
    1595,  1596,  1597,  1598,  1602,  1613,  1614,  1620,  1631,  1643,
    1644,  1648,  1653,  1664,  1665,  1669,  1683,  1684,  1694,  1698,
    1699,  1703,  1708,  1709,  1718,  1721,  1727,  1737,  1741,  1759,
    1760,  1764,  1769,  1770,  1781,  1782,  1792,  1795,  1801,  1811,
    1812,  1819,  1825,  1831,  1842,  1843,  1844,  1848,  1849,  1853,
    1859,  1866,  1875,  1884,  1895,  1896,  1902,  1906,  1907,  1916,
    1917,  1926,  1935,  1936,  1937,  1938,  1943,  1944,  1953,  1954,
    1955,  1959,  1973,  1974,  1975,  1978,  1979,  1982,  1994,  1995,
    1999,  2002,  2006,  2007,  2008,  2009,  2010,  2011,  2012,  2013,
    2014,  2015,  2016,  2017,  2018,  2019,  2020,  2021,  2025,  2030,
    2040,  2049,  2050,  2054,  2067,  2068,  2076,  2077,  2078,  2079,
    2080,  2081,  2082,  2089,  2093,  2097,  2098,  2104,  2110,  2119,
    2120,  2127,  2134,  2144,  2145,  2152,  2162,  2163,  2170,  2177,
    2184,  2194,  2201,  2206,  2211,  2216,  2222,  2228,  2237,  2244,
    2252,  2260,  2263,  2269,  2273,  2278,  2286,  2287,  2288,  2292,
    2296,  2297,  2300,  2307,  2317,  2320,  2324,  2334,  2339,  2346,
    2355,  2363,  2372,  2380,  2389,  2397,  2402,  2407,  2415,  2424,
    2425,  2435,  2436,  2444,  2449,  2467,  2473,  2478,  2486,  2498,
    2502,  2506,  2507,  2511,  2522,  2532,  2537,  2544,  2554,  2557,
    2562,  2563,  2564,  2565,  2566,  2567,  2570,  2575,  2582,  2592,
    2593,  2601,  2605,  2609,  2613,  2619,  2627,  2630,  2640,  2641,
    2645,  2654,  2659,  2667,  2673,  2683,  2684,  2685
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
  "general_set_fct", "opt_function_arg", "set_fct_type", "outer_join_type",
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

#define YYPACT_NINF (-581)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-446)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     631,    72,    33,    81,    65,    12,   152,   148,   176,   183,
     188,  -581,  -581,  -581,  -581,  -581,  -581,  -581,    40,  -581,
     184,    12,   197,  -581,  -581,  2442,   203,    47,    25,   215,
     602,  -581,  -581,  -581,  -581,  2593,    23,  -581,  -581,  -581,
    -581,  -581,  -581,   214,   222,  -581,    69,   373,   246,   244,
    -581,  -581,  1687,   232,  -581,  -581,  -581,  -581,  -581,    78,
    -581,  -581,   257,   274,  -581,   276,   284,  -581,  -581,   289,
    -581,  -581,  -581,  -581,  2895,  -581,  -581,  -581,  -581,  2140,
    -581,  -581,  2593,  2593,  2593,   292,   316,  -581,  -581,  -581,
    -581,   319,  -581,  -581,  -581,  -581,   321,   327,  2895,  2895,
     405,   286,  -581,   331,  -581,   121,  -581,  -581,  -581,  -581,
     344,   136,   346,  -581,   210,  -581,  -581,   348,  -581,   350,
    -581,   360,  -581,  -581,  -581,  -581,  -581,   201,   175,   263,
    -581,  -581,  -581,  -581,  -581,    44,  -581,   199,  -581,  -581,
    -581,  -581,    19,  -581,  -581,  -581,  -581,  -581,  -581,  -581,
    -581,  -581,   196,  -581,   215,   254,   364,   254,   285,  -581,
     323,  -581,  -581,  -581,  -581,   264,    29,   303,   328,  -581,
     383,  -581,  -581,   242,   313,   313,   279,  -581,  -581,  -581,
     196,   390,   184,  -581,   932,   312,  -581,   399,   408,    29,
     340,   425,    14,  -581,  -581,  -581,  2593,    37,    65,    65,
     932,  -581,   932,  -581,   297,  -581,   341,   242,  -581,  -581,
    -581,   443,  2593,  2593,    65,    65,  -581,  -581,    12,  -581,
     372,  2593,  -581,  -581,  -581,  -581,  1385,    65,   462,   347,
    2593,  2593,   459,  2744,  2744,  2744,  2744,  2744,  2744,  2744,
    2744,  2744,  -581,   453,  2593,  -581,  -581,   357,    29,    29,
    -581,   254,  -581,   457,   244,  2593,  -581,   469,  -581,   215,
     215,    12,   422,   471,    51,  -581,   333,  -581,    12,  -581,
    2593,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   511,  -581,
     496,   312,  -581,  -581,   397,  -581,   630,   781,   522,   509,
     522,  -581,  -581,  -581,  -581,  -581,  -581,   262,    70,   499,
    -581,  -581,   402,   407,  -581,  -581,  2593,  -581,  -581,  -581,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   343,
     377,   460,    39,  2920,   537,  -581,  -581,  -581,  -581,   314,
    -581,  -581,   403,  -581,  -581,  -581,  -581,   531,   242,   559,
    2593,  2593,  2593,    32,     0,  2593,  -581,   467,   932,   468,
    -581,   341,  -581,   564,   242,  -581,  -581,   565,  2593,  2593,
     566,   620,   518,  -581,  -581,  2593,  -581,   446,  2593,   372,
      84,   571,  -581,   573,  -581,  -581,  -581,  -581,   201,   201,
     175,   175,   175,   175,  -581,  -581,  -581,  -581,   569,  -581,
    -581,  -581,   448,   575,  -581,  -581,   620,    12,    29,   312,
    2593,   516,  -581,  -581,  -581,   326,  -581,   552,   561,    20,
      13,  -581,  -581,  -581,   519,  -581,   583,  2593,    57,  2291,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,   509,   932,   932,
    -581,   424,   537,  -581,   399,  -581,    29,  2942,  -581,   586,
     451,   454,  2593,  2593,  -581,  -581,   186,    38,  -581,  2593,
    -581,   587,   588,   590,    42,  -581,   494,  -581,  -581,  2593,
     470,   591,    12,   562,   543,   595,  -581,  2593,   599,   600,
     581,  -581,  -581,  -581,  -581,  -581,  2593,   603,  -581,  -581,
     453,  -581,   620,  -581,  -581,  -581,   242,   932,   113,  -581,
    -581,  -581,   582,   584,   607,  -581,  -581,  -581,  1687,  -581,
    -581,   144,   -10,  2593,   401,  -581,  -581,  -581,  -581,   522,
      77,  -581,   402,    85,  -581,  -581,   601,  -581,  -581,  -581,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,
    -581,  -581,  -581,   609,  -581,   608,  -581,  -581,  -581,   610,
    -581,    74,  1234,  2593,  -581,   633,  -581,  2593,  2593,   635,
     620,  2593,   932,   474,  -581,  -581,  -581,  -581,  -581,   497,
    -581,   636,  -581,   483,  -581,   407,   453,   445,   254,  -581,
    -581,   196,   514,   242,  2593,  -581,  -581,   486,  -581,  -581,
    -581,  -581,    31,  -581,  -581,  -581,  -581,  -581,  -581,  -581,
    -581,   121,  -581,   618,  -581,  -581,  2593,   190,  -581,  2593,
    -581,  -581,  -581,  -581,  -581,  -581,   641,  -581,  -581,   642,
     407,   581,   574,   613,   574,  2593,  -581,   453,   517,  -581,
    -581,  -581,  -581,   652,  -581,  2593,   500,  2593,  -581,   197,
     524,  -581,  -581,   657,  2593,  -581,  -581,  -581,   660,  -581,
     644,   637,   576,    29,   -39,  -581,  -581,  -581,  -581,   242,
    -581,  -581,  -581,    31,  -581,   581,   675,  1083,  2744,   312,
    -581,   678,  -581,   539,  -581,  -581,  -581,   683,  1536,  -581,
    -581,  -581,   686,  -581,   194,   178,   150,  -581,    29,    66,
    -581,  -581,  1838,   557,   551,   568,  -581,  -581,   -18,  1083,
    -581,  -581,   658,   658,  2744,  -581,  -581,  -581,  -581,  -581,
    -581,   567,   572,  -581,  -581,   547,  -581,   -49,  -581,  -581,
     164,  -581,  -581,  -581,   493,  -581,  -581,   263,  -581,  -581,
    1989,  -581,  -581,  -581,  -581,   580,   585,  -581,  -581,  -581,
     570,  -581,  -581,  -581,  -581,  -581
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,     0,     0,    73,   101,     0,     0,     0,     2,
       4,    60,     6,    59,    56,    57,    84,    58,    61,    26,
       0,   101,     0,    74,    75,     0,   102,     0,   106,     0,
     238,   245,   326,   325,   111,     0,    10,     1,     3,     5,
      65,    63,    64,   328,     0,    31,     0,    81,     0,    39,
     408,   407,     0,     0,   455,   195,   192,   193,   194,   421,
     229,   226,     0,     0,   207,     0,     0,   206,   231,     0,
     230,   227,   208,   427,     0,   382,   384,   383,   228,     0,
     191,   404,     0,     0,     0,     0,     0,   259,   260,   263,
     264,     0,   312,   313,   314,   315,     0,     0,     0,     0,
     426,    92,   330,    86,   188,   204,   356,   203,   211,   357,
       0,   210,     0,   361,   266,   250,   251,     0,   252,     0,
     253,     0,   359,   363,   364,   365,   369,   373,   376,   396,
     388,   381,   389,   390,   398,   200,   397,   399,   401,   405,
     400,   190,   354,   423,   358,   360,   430,   431,   432,   205,
     105,   104,     0,   107,     0,   456,   419,    99,   414,   415,
       0,   234,   239,   232,   233,   238,     0,   240,     0,   426,
       0,    69,    71,    72,     0,     0,     8,    11,    12,   329,
       0,     0,     0,    32,    54,   458,    82,     0,     0,     0,
       0,     0,     0,   454,   422,   425,     0,    73,    73,    73,
      54,   368,    54,   453,   449,   446,     0,     0,   385,   387,
     386,     0,     0,     0,    73,    73,   367,   366,   101,    83,
      81,     0,   198,   199,   197,   196,     0,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   429,     0,     0,   406,   202,     0,     0,     0,
     428,    99,    98,     0,    39,     0,   418,     0,   109,     0,
       0,   101,     0,   421,    81,    76,     0,   241,   101,    68,
       0,    15,    16,    17,    20,    21,    19,    18,     0,    13,
       0,   458,    62,    28,     0,    30,    54,    54,     0,    54,
       0,   157,   158,   154,   153,   156,   155,     0,    54,   129,
     131,   133,   135,   112,   122,   141,     0,   142,   166,   167,
     148,   172,   146,   177,   147,   143,   149,   144,   145,   123,
     124,   126,   127,   125,     0,    66,   459,   412,   411,   412,
     409,   410,     0,    36,    67,   187,   362,     0,   331,     0,
       0,     0,     0,     0,     0,     0,   447,     0,    54,   449,
     435,     0,   248,     0,   452,   393,   451,     0,     0,     0,
      94,    95,   113,   189,   212,     0,   394,     0,     0,     0,
       0,   255,   261,     0,   247,   370,   371,   372,   374,   375,
     379,   380,   377,   378,   201,   403,   402,   424,   355,   108,
     457,   110,     0,    37,   417,   416,   242,   101,     0,   458,
       0,   238,    70,    14,    22,     9,     7,     0,     0,     0,
      72,    55,   132,   185,   159,   186,     0,     0,     0,     0,
     161,   164,   165,   171,   176,   179,   180,    54,    54,    54,
     152,   463,   460,   462,     0,    40,     0,     0,   219,     0,
       0,     0,     0,     0,   450,   434,     0,    71,   441,     0,
     442,   166,   172,   177,     0,   438,     0,   436,   254,     0,
       0,   224,   101,     0,   115,     0,   213,     0,     0,     0,
     279,   268,   269,   246,   270,   271,     0,     0,   262,   420,
       0,   100,   243,    77,    80,    78,    79,    54,     0,   236,
     244,   237,     0,     0,     0,    27,   128,   170,     0,   175,
     173,   168,   168,     0,     0,   183,   182,   184,   151,     0,
       0,   134,   136,     0,   461,    35,   347,   338,   337,   339,
     340,   334,   335,   341,   336,   342,   346,   332,   333,   343,
     344,   345,   348,   351,   352,     0,   220,   216,   217,     0,
     448,   152,    54,     0,   433,     0,   221,     0,     0,     0,
      96,     0,    54,   272,   214,   395,   218,   267,   281,   282,
     280,   256,   258,     0,    34,   235,     0,   322,    99,    24,
      23,     0,     0,   391,     0,   163,   162,     0,   215,   181,
     210,   178,   101,   118,   120,   117,   119,   130,   121,   466,
     467,   465,   464,     0,   350,   353,     0,     0,   440,     0,
     444,   445,   443,   439,   437,   265,     0,   225,   223,   114,
     116,     0,    41,     0,    41,     0,    38,     0,     0,   323,
     324,   321,   413,     0,   174,     0,   169,     0,   102,     0,
       0,   138,   349,     0,     0,   150,   222,   277,   273,   275,
       0,     0,    87,     0,   284,   257,    33,   327,    29,   392,
     160,   140,   137,   101,   209,     0,     0,    54,     0,   458,
      88,   283,   287,   316,   295,   294,   296,     0,     0,   139,
     274,   276,    42,    43,    47,    47,    89,    93,     0,     0,
     288,   278,     0,     0,     0,   289,   297,   298,     0,    54,
      48,    49,    50,    50,     0,    91,   286,   318,   319,   320,
     317,     0,     0,   304,   307,     0,   306,     0,   299,   301,
       0,   285,   300,    44,     0,    46,    45,    90,   308,   305,
       0,   311,   302,   291,   292,     0,     0,    52,    53,    51,
       0,   309,   303,   293,   290,   310
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   529,  -581,
    -581,  -581,   534,  -581,  -581,   160,  -581,  -581,   475,    99,
    -581,    41,    53,    43,  -581,  -258,  -581,     4,  -581,  -581,
    -581,   541,  -581,   -33,   -78,   147,  -581,   334,  -581,  -581,
    -137,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   578,
    -237,  -581,  -522,   707,   -13,   368,  -581,  -581,   226,  -581,
     455,   318,   311,  -171,  -581,  -581,    88,  -565,  -581,  -581,
    -279,   449,  -581,  -278,   450,  -581,   241,  -277,  -581,  -581,
    -276,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   -19,
    -581,   523,   239,  -581,  -139,  -581,  -581,  -161,  -581,   256,
    -581,  -581,  -581,  -581,  -581,   596,  -581,  -581,  -581,  -581,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -328,
    -581,  -581,  -581,  -581,   107,  -581,   108,  -581,  -581,  -581,
    -581,  -581,    87,  -581,  -581,  -581,  -581,  -581,  -581,    46,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,
    -581,  -135,  -581,  -581,  -581,  -581,  -581,  -581,  -581,   351,
     142,   145,   315,   205,  -580,  -581,  -581,  -581,  -581,  -581,
     212,  -179,  -178,   -25,   -77,  -581,  -581,  -581,   520,  -581,
     140,  -581,   589,   593,  -581,  -138,  -581,   295,  -581,  -581,
     521,   530,  -154,  -132,  -581,  -581,  -581,  -581,  -581,   420,
    -581,   230,   577,  -140,   432,  -320,  -581,  -581,  -581,  -581,
    -270,  -581,  -581,   353,  -581
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     8,     9,    10,   281,   176,   177,   178,   278,   405,
      20,   284,    45,    46,    11,   563,   332,   481,   190,   642,
     672,   673,   692,   715,   729,   297,    12,   191,    43,    14,
      15,    16,   170,   298,   172,    25,   264,   265,   485,    17,
     185,    18,   101,   659,   695,   660,   219,   220,   360,   257,
     258,    27,    28,    29,    30,   186,   464,   553,   587,   299,
     300,   301,   302,   344,   588,   630,   631,   304,   448,   305,
     306,   450,   307,   308,   309,   310,   575,   311,   312,   499,
     313,   314,   425,   315,   316,   426,   509,   317,   318,   102,
     103,   104,   105,   245,   106,   107,   108,   109,   110,   111,
     549,   112,   167,   489,   490,   168,    31,    32,   113,   114,
     115,   477,   116,   117,   371,   118,   119,   120,   229,   637,
     473,   474,   612,   638,   639,   640,   475,   559,   560,   614,
     667,   661,   662,   711,   668,   685,   686,   703,   687,   704,
     705,   732,   706,   121,   680,   700,    33,   621,    34,   491,
     180,   122,   337,   532,   533,   534,   535,   123,   250,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     572,   366,   367,   354,   136,   137,   138,   139,   140,   141,
      49,    50,    51,   331,   567,   157,   158,   159,   256,   195,
     142,   143,   144,   564,   145,   146,   147,   148,   349,   350,
     454,   455,   204,   205,   347,   355,   356,   206,   149,   254,
     325,   326,   432,   433,   592
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     135,   203,   171,   246,    13,   208,   209,   210,    47,   155,
     173,   406,   266,   303,   389,  -103,   251,   336,   336,   419,
     421,   423,   424,   320,   496,   444,   174,   192,   152,   343,
     323,   414,   248,   353,   357,   333,  -103,   442,   574,   320,
     418,   320,   472,  -440,   -72,   319,   323,   542,   323,   321,
     183,   372,   373,   263,   173,  -103,   398,   207,   207,   207,
     629,   319,   -72,   319,   346,   321,    23,   321,   242,   449,
     451,   452,   453,   -72,   182,   243,   150,   392,   676,  -150,
     582,  -215,    24,   362,   721,    21,   722,   470,   582,    40,
     446,   503,   675,   411,    23,   -72,   443,   664,   665,   -72,
      19,    41,   -72,   194,    55,    56,    57,    58,   471,   589,
      24,   384,   666,   416,   717,   409,   566,   712,    73,   244,
     417,    42,   539,   540,   675,   320,   320,   399,   583,   484,
     289,   629,   323,   323,  -440,   -72,   583,   156,   543,   184,
      22,   222,   558,   223,   -72,     4,   584,   319,   319,   151,
     244,   321,   321,   429,   584,    35,     6,   585,  -397,   322,
     586,   439,   504,   440,   441,   585,  -127,  -127,   586,   510,
    -150,   338,    36,   429,   249,   322,    37,   322,    26,   460,
     461,   244,   244,   697,   282,   429,   465,   320,   175,   468,
      38,   153,   574,    80,   323,    39,   135,   628,   169,   -72,
     -72,   -72,   -72,   -72,   -72,   361,   339,   244,    44,   319,
     690,   346,   244,   321,   698,   699,   201,   -71,   723,   385,
     503,    48,   691,   604,   503,   181,   690,  -107,   -71,   224,
     291,   292,   293,   294,   295,   296,   225,   402,   691,   156,
     216,   217,   416,   179,   266,   173,   416,   189,   396,   417,
     -71,   694,   187,   634,   -71,   401,   193,   -71,  -249,   188,
     196,   410,   322,   599,   600,   601,   602,   320,   320,   413,
     447,   415,     4,   430,   323,   323,   633,   197,   -97,   198,
     545,   173,   515,     6,   597,   252,  -249,   199,   555,   319,
     319,   259,   200,   321,   321,   211,   724,   561,   260,   -71,
    -364,  -364,  -364,  -364,   725,   726,   238,   239,   240,   241,
    -364,  -364,  -364,  -364,  -364,  -364,   565,   161,   416,   212,
     187,   162,   213,   322,   214,   417,   320,   434,   163,   164,
     215,   622,   492,   323,   271,   272,   221,   273,   218,   493,
     502,   236,   237,   579,   340,   341,   342,   226,   319,   227,
     568,   230,   321,   231,   -71,   -71,   -71,   -71,   -71,   -71,
     228,   358,   359,   232,   274,   247,  -356,   255,   606,   607,
     233,   234,   235,   365,   368,   486,   267,  -356,   375,   376,
     377,   610,   275,   280,   482,   261,   508,   269,   270,   677,
     268,   320,   207,   276,   173,   202,   345,   500,   323,  -356,
    -357,   407,   408,   322,   322,   -85,  -356,   435,   436,   -85,
     244,  -357,   -85,   319,   283,   160,   541,   321,   324,   238,
     239,   240,   241,   327,   173,   578,   161,     6,   277,   335,
     162,    60,   329,  -357,    61,  -238,   645,   163,   164,   348,
    -357,   165,    63,   380,   381,   382,   383,   352,  -356,   550,
     466,   467,   479,   467,   -85,   537,   467,   -85,   538,   467,
     184,   184,   322,   374,   598,   369,   -85,    65,    66,   370,
     577,    68,    81,   573,   546,   547,    69,   242,   173,    70,
      71,   390,  -357,  -359,   397,   646,   -85,   616,   617,   663,
     581,   619,   620,   393,  -359,    78,   194,   626,   400,  -356,
    -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,
    -356,  -356,  -356,  -356,  -356,   403,  -359,   173,   624,   625,
     404,   647,   617,  -359,   663,   152,   573,   322,   652,   653,
     727,   728,   411,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,   207,
      96,   378,   379,    97,   394,   395,   428,   502,   160,   427,
     429,   431,   437,   438,   445,  -359,   635,   345,   458,   161,
     459,   462,   463,   162,   173,   623,   476,   478,   480,   674,
     163,   164,   248,   494,   165,   495,   498,   487,   497,   513,
     536,   544,  -444,  -445,   650,  -443,   548,   551,   552,   554,
     649,   488,   173,   556,   557,   471,   569,   562,   570,   207,
     571,   674,   595,   611,   593,   596,  -359,  -359,  -359,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,   173,   286,   594,   613,    53,   605,    54,   608,
     627,   615,   632,   688,   160,   636,   641,   625,   643,    55,
      56,    57,    58,   287,    59,   161,   648,   707,     1,   162,
      60,   654,   160,    61,   173,   655,   163,   164,  -397,    62,
     165,    63,   657,   161,     2,   656,   658,   162,   470,   166,
     288,    64,  -238,   678,   163,   164,   679,   681,   165,     3,
     289,   689,   708,   709,   714,   707,    65,    66,   710,    67,
      68,   720,   718,   735,   279,    69,     4,     4,    70,    71,
      72,   733,   290,   644,   719,     5,   285,     6,     6,    73,
      74,    75,    76,    77,    78,    79,   618,   734,   693,   391,
     713,   334,   483,   253,     7,   154,   716,   469,    80,   590,
     512,   669,   412,   576,   363,    81,   511,   420,   422,    82,
      83,    84,   591,    85,    86,    87,    88,    89,    90,    91,
     580,   262,   670,   609,   671,   696,   731,   386,   501,   651,
     388,   457,   603,    92,    93,    94,    95,   330,   387,    96,
     328,   456,    97,   351,   286,   514,     0,    53,     0,    54,
     291,   292,   293,   294,   295,   296,    98,    99,     0,   169,
      55,    56,    57,    58,   411,    59,     0,     0,     0,     0,
       0,    60,     0,     0,    61,   -55,     0,     0,     0,     0,
      62,     0,    63,     0,     0,     0,     0,     0,     0,     0,
       0,   288,    64,     0,     0,     0,     0,     0,     0,     0,
       0,   289,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,     0,   290,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
      82,    83,    84,     0,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      96,     0,     0,    97,     0,   286,     0,     0,    53,     0,
      54,   291,   292,   293,   294,   295,   296,    98,    99,     0,
     169,    55,    56,    57,    58,   287,    59,     0,     0,     0,
       0,     0,    60,     0,     0,    61,     0,     0,     0,     0,
       0,    62,     0,    63,     0,     0,     0,     0,     0,     0,
       0,     0,   288,    64,     0,     0,     0,     0,     0,     0,
       0,     0,   289,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,   290,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,    52,     0,     0,    53,
       0,    54,   291,   292,   293,   294,   295,   296,    98,    99,
       0,   169,    55,    56,    57,    58,   411,    59,     0,     0,
       0,     0,     0,    60,     0,     0,    61,     0,     0,     0,
       0,     0,    62,     0,    63,     0,     0,     0,     0,     0,
       0,     0,     0,   288,    64,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,   290,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,    82,    83,    84,     0,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    96,     0,     0,    97,     0,    52,     0,     0,
      53,     0,    54,   291,   292,   293,   294,   295,   296,    98,
      99,     0,   169,    55,    56,    57,    58,   411,    59,     0,
       0,     0,     0,     0,    60,     0,     0,    61,     0,     0,
       0,     0,     0,    62,     0,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,     0,     0,    81,
       0,     0,     0,    82,    83,    84,     0,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,    96,     0,     0,    97,     0,    52,   364,
       0,    53,     0,    54,   291,   292,   293,   294,   295,   296,
      98,    99,     0,   169,    55,    56,    57,    58,     0,    59,
       0,     0,     0,     0,    23,    60,     0,     0,    61,     0,
       0,     0,     0,     0,    62,     0,    63,     0,     0,     0,
      24,     0,     0,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,     0,     0,     0,     0,
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
     682,     0,     0,     0,     0,    62,     0,    63,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,    66,     0,    67,    68,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,     0,     0,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,    82,    83,    84,     0,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
     683,     0,     0,     0,     0,     0,     0,   684,     0,    92,
      93,    94,    95,     0,     0,    96,     0,     0,    97,     0,
      52,     0,     0,    53,     0,    54,     0,     0,     0,     0,
       0,     0,    98,    99,     0,   169,    55,    56,    57,    58,
       0,    59,     0,     0,     0,     0,     0,    60,     0,     0,
      61,     0,     0,     0,     0,     0,    62,     0,    63,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,     0,     0,
       0,     0,    69,     4,     0,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     6,     0,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,    96,     0,     0,    97,
       0,    52,     0,     0,    53,     0,    54,     0,     0,     0,
       0,     0,     0,    98,    99,     0,   169,    55,    56,    57,
      58,     0,    59,     0,     0,     0,     0,     0,    60,     0,
       0,    61,     0,     0,     0,     0,     0,    62,     0,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,    66,     0,    67,    68,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,    73,    74,    75,
      76,    77,    78,    79,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,    82,    83,    84,
       0,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,   701,     0,     0,     0,     0,     0,     0,   702,
       0,    92,    93,    94,    95,     0,     0,    96,     0,     0,
      97,     0,    52,     0,     0,    53,     0,    54,     0,     0,
       0,     0,     0,     0,    98,    99,     0,   169,    55,    56,
      57,    58,     0,    59,     0,     0,     0,     0,     0,    60,
       0,     0,    61,     0,     0,     0,     0,     0,    62,     0,
      63,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,     0,    67,    68,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,    82,    83,
      84,     0,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,   730,     0,     0,     0,     0,     0,     0,
     702,     0,    92,    93,    94,    95,     0,     0,    96,     0,
       0,    97,     0,    52,     0,     0,    53,     0,    54,     0,
       0,     0,     0,     0,     0,    98,    99,     0,   169,    55,
      56,    57,    58,     0,    59,     0,     0,     0,     0,     0,
      60,     0,     0,    61,     0,     0,     0,     0,     0,    62,
       0,    63,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,    66,     0,    67,
      68,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,     0,     0,   202,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,    82,
      83,    84,     0,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,    96,
       0,     0,    97,     0,    52,     0,     0,    53,     0,    54,
       0,     0,     0,     0,     0,     0,    98,    99,     0,   169,
      55,    56,    57,    58,     0,    59,     0,     0,     0,     0,
     505,   506,     0,     0,    61,     0,     0,     0,     0,     0,
      62,     0,    63,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,     0,     0,     0,     0,    69,     0,     0,   507,
      71,    72,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    62,     0,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    96,     0,     0,    97,     0,    52,     0,     0,    53,
       0,    54,     0,     0,     0,     0,     0,     0,    98,    99,
       0,   100,    55,    56,    57,    58,     0,    59,     0,     0,
       0,     0,     0,    60,     0,     0,    61,     0,     0,     0,
       0,     0,    62,     0,    63,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,     0,     0,     0,     0,    69,     0,
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
       0,     0,     0,     0,    60,     0,     0,    61,     0,     0,
       0,     0,     0,    62,     0,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,     0,     0,     0,     0,    69,
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
       0,     0,     0,     0,    62,     0,    63,     0,     0,     0,
       0,     0,     0,  -358,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,  -358,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,   516,     0,     0,     0,
      69,     0,     0,    70,    71,    72,  -358,     0,     0,     0,
       0,     0,     0,  -358,    73,     0,     0,     0,     0,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
      87,    88,    89,    90,    91,  -358,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,    96,     0,     0,    97,     0,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,   530,   531,   169,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358
};

static const yytype_int16 yycheck[] =
{
      25,    79,    35,   135,     0,    82,    83,    84,    21,    28,
      35,   281,   166,   184,   251,     3,   154,     4,     4,   298,
     298,   298,   298,   184,     4,   345,     3,    52,     3,   200,
     184,   289,    13,   212,   213,   189,    24,     5,    48,   200,
     298,   202,   370,     5,     5,   184,   200,     5,   202,   184,
      46,   230,   231,    24,    79,    24,     5,    82,    83,    84,
     582,   200,    23,   202,   204,   200,    29,   202,    24,   348,
     348,   348,   348,    34,     5,    31,    29,   255,   658,     5,
       3,     3,    45,   220,   133,    52,   135,     3,     3,    49,
     348,    34,   657,    23,    29,    56,    96,   136,   137,    60,
      28,    61,    63,    25,    19,    20,    21,    22,    24,    24,
      45,   243,   151,    56,   694,   286,     3,   135,    89,   168,
      63,    81,   442,   443,   689,   286,   287,   264,    51,   399,
      60,   653,   286,   287,    96,    96,    51,    24,    96,    88,
      59,    20,   470,    22,   105,    76,    69,   286,   287,   102,
     168,   286,   287,   153,    69,     3,    87,    80,   168,   184,
      83,   340,   105,   341,   342,    80,   153,   154,    83,   427,
      96,   196,    24,   153,   155,   200,     0,   202,   166,   358,
     359,   168,   168,   117,   180,   153,   365,   348,   165,   368,
       7,   166,    48,   108,   348,     7,   221,   166,   169,   160,
     161,   162,   163,   164,   165,   218,   169,   168,    24,   348,
      32,   351,   168,   348,   148,   149,    74,    23,    54,   244,
      34,    24,    44,   543,    34,     3,    32,    24,    34,   108,
     160,   161,   162,   163,   164,   165,   115,   270,    44,    24,
      98,    99,    56,    29,   398,   270,    56,     3,   261,    63,
      56,   101,     6,    63,    60,   268,    24,    63,   122,    13,
       3,   286,   287,   542,   542,   542,   542,   428,   429,   288,
     348,   290,    76,   306,   428,   429,   596,     3,    24,     3,
     459,   306,   436,    87,   542,    31,   150,     3,   467,   428,
     429,     6,     3,   428,   429,     3,   132,   476,    13,   105,
     156,   157,   158,   159,   140,   141,   156,   157,   158,   159,
     166,   167,   168,   169,   170,   171,   487,    53,    56,     3,
       6,    57,     3,   348,     3,    63,   487,    13,    64,    65,
       3,   568,     6,   487,    21,    22,     5,    24,    52,    13,
     417,   166,   167,   504,   197,   198,   199,     3,   487,     3,
     488,     3,   487,     3,   160,   161,   162,   163,   164,   165,
     150,   214,   215,     3,    51,   166,    23,     3,   547,   548,
     169,   170,   171,   226,   227,   400,    73,    34,   233,   234,
     235,   552,    69,   104,   397,    62,   419,     4,     5,   659,
      62,   552,   417,    80,   419,    98,    99,   416,   552,    56,
      23,     4,     5,   428,   429,     0,    63,     4,     5,     4,
     168,    34,     7,   552,    24,    42,   449,   552,   106,   156,
     157,   158,   159,    24,   449,    24,    53,    87,   115,     4,
      57,    30,    24,    56,    33,    62,   615,    64,    65,    98,
      63,    68,    41,   238,   239,   240,   241,     4,   105,   462,
       4,     5,     4,     5,    49,     4,     5,    52,     4,     5,
      88,    88,   487,     4,   542,     3,    61,    66,    67,   122,
     503,    70,   115,   498,     4,     5,    75,    24,   503,    78,
      79,    24,   105,    23,    62,   617,    81,     4,     5,   643,
     509,    46,    47,    24,    34,    94,    25,   574,   165,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,     4,    56,   542,     4,     5,
      24,     4,     5,    63,   678,     3,   551,   552,     4,     5,
      37,    38,    23,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   574,
     149,   236,   237,   152,   259,   260,   154,   634,    42,    60,
     153,    24,    31,     4,    97,   105,   599,    99,     4,    53,
       5,     5,    54,    57,   599,   571,     5,     4,     3,   657,
      64,    65,    13,    31,    68,    24,     3,    71,    69,   165,
       4,    97,     5,     5,   627,     5,     5,    35,    55,     4,
     625,    85,   627,     4,     4,    24,    24,     4,    24,   634,
       3,   689,     4,   139,    13,     5,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   657,     3,    25,   138,     6,     4,     8,     4,
     154,     5,    24,   668,    42,     4,    72,     5,    35,    19,
      20,    21,    22,    23,    24,    53,     4,   682,    27,    57,
      30,     4,    42,    33,   689,     5,    64,    65,   168,    39,
      68,    41,    35,    53,    43,    31,   100,    57,     3,    77,
      50,    51,    62,     5,    64,    65,   147,     4,    68,    58,
      60,     5,   135,   142,    36,   720,    66,    67,   130,    69,
      70,   154,   135,   133,   175,    75,    76,    76,    78,    79,
      80,   131,    82,   614,   142,    84,   182,    87,    87,    89,
      90,    91,    92,    93,    94,    95,   566,   142,   675,   254,
     689,   190,   398,   155,   103,    28,   693,   369,   108,   513,
     429,   653,   287,   502,   221,   115,   428,   298,   298,   119,
     120,   121,   513,   123,   124,   125,   126,   127,   128,   129,
     504,   165,   655,   551,   656,   678,   720,   247,   417,   629,
     249,   351,   542,   143,   144,   145,   146,   188,   248,   149,
     187,   349,   152,   206,     3,   432,    -1,     6,    -1,     8,
     160,   161,   162,   163,   164,   165,   166,   167,    -1,   169,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,
      39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,    -1,
     149,    -1,    -1,   152,    -1,     3,    -1,    -1,     6,    -1,
       8,   160,   161,   162,   163,   164,   165,   166,   167,    -1,
     169,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    66,    67,
      -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      78,    79,    80,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,   120,   121,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,
      -1,   149,    -1,    -1,   152,    -1,     3,    -1,    -1,     6,
      -1,     8,   160,   161,   162,   163,   164,   165,   166,   167,
      -1,   169,    19,    20,    21,    22,    23,    24,    -1,    -1,
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
      -1,    -1,   149,    -1,    -1,   152,    -1,     3,    -1,    -1,
       6,    -1,     8,   160,   161,   162,   163,   164,   165,   166,
     167,    -1,   169,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,   121,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,    -1,    -1,   149,    -1,    -1,   152,    -1,     3,     4,
      -1,     6,    -1,     8,   160,   161,   162,   163,   164,   165,
     166,   167,    -1,   169,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    29,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,   120,   121,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,    -1,    -1,   149,    -1,    -1,   152,    -1,     3,
      -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,   166,   167,    -1,   169,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,    -1,
      -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,   143,
     144,   145,   146,    -1,    -1,   149,    -1,    -1,   152,    -1,
       3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,   166,   167,    -1,   169,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,    -1,    -1,   149,    -1,    -1,   152,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   169,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    -1,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,   121,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,   143,   144,   145,   146,    -1,    -1,   149,    -1,    -1,
     152,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,   143,   144,   145,   146,    -1,    -1,   149,    -1,
      -1,   152,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,
      70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,
     120,   121,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,    -1,    -1,   149,
      -1,    -1,   152,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      29,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,    -1,
     149,    -1,    -1,   152,    -1,     3,    -1,    -1,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,
      -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,   120,   121,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,
      -1,   149,    -1,    -1,   152,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,
      -1,   169,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,    -1,   119,   120,   121,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
      -1,    -1,   149,    -1,    -1,   152,    -1,     3,    -1,    -1,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,    -1,    -1,   149,    -1,    -1,   152,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
     166,   167,    -1,   169,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    24,    -1,    -1,    -1,
      75,    -1,    -1,    78,    79,    80,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    89,    -1,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,    -1,    -1,   149,    -1,    -1,   152,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    43,    58,    76,    84,    87,   103,   175,   176,
     177,   188,   200,   201,   203,   204,   205,   213,   215,    28,
     184,    52,    59,    29,    45,   209,   166,   225,   226,   227,
     228,   280,   281,   320,   322,     3,    24,     0,     7,     7,
      49,    61,    81,   202,    24,   186,   187,   228,    24,   354,
     355,   356,     3,     6,     8,    19,    20,    21,    22,    24,
      30,    33,    39,    41,    51,    66,    67,    69,    70,    75,
      78,    79,    80,    89,    90,    91,    92,    93,    94,    95,
     108,   115,   119,   120,   121,   123,   124,   125,   126,   127,
     128,   129,   143,   144,   145,   146,   149,   152,   166,   167,
     169,   216,   263,   264,   265,   266,   268,   269,   270,   271,
     272,   273,   275,   282,   283,   284,   286,   287,   289,   290,
     291,   317,   325,   331,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   347,   348,   349,   350,   351,
     352,   353,   364,   365,   366,   368,   369,   370,   371,   382,
      29,   102,     3,   166,   227,   263,    24,   359,   360,   361,
      42,    53,    57,    64,    65,    68,    77,   276,   279,   169,
     206,   207,   208,   347,     3,   165,   179,   180,   181,    29,
     324,     3,     5,   201,    88,   214,   229,     6,    13,     3,
     192,   201,   347,    24,    25,   363,     3,     3,     3,     3,
       3,   334,    98,   208,   376,   377,   381,   347,   348,   348,
     348,     3,     3,     3,     3,     3,   334,   334,    52,   220,
     221,     5,    20,    22,   108,   115,     3,     3,   150,   292,
       3,     3,     3,   169,   170,   171,   166,   167,   156,   157,
     158,   159,    24,    31,   168,   267,   367,   166,    13,   155,
     332,   359,    31,   223,   383,     3,   362,   223,   224,     6,
      13,    62,   279,    24,   210,   211,   366,    73,    62,     4,
       5,    21,    22,    24,    51,    69,    80,   115,   182,   182,
     104,   178,   201,    24,   185,   186,     3,    23,    50,    60,
      82,   160,   161,   162,   163,   164,   165,   199,   207,   233,
     234,   235,   236,   237,   241,   243,   244,   246,   247,   248,
     249,   251,   252,   254,   255,   257,   258,   261,   262,   268,
     271,   325,   347,   366,   106,   384,   385,    24,   357,    24,
     356,   357,   190,   366,   205,     4,     4,   326,   347,   169,
     209,   209,   209,   237,   237,    99,   377,   378,    98,   372,
     373,   376,     4,   345,   347,   379,   380,   345,   209,   209,
     222,   228,   214,   265,     4,   209,   345,   346,   209,     3,
     122,   288,   345,   345,     4,   335,   335,   335,   336,   336,
     337,   337,   337,   337,   367,   347,   352,   365,   364,   224,
      24,   192,   346,    24,   361,   361,   228,    62,     5,   214,
     165,   228,   207,     4,    24,   183,   384,     4,     5,   237,
     347,    23,   234,   263,   199,   263,    56,    63,   199,   244,
     245,   247,   248,   251,   254,   256,   259,    60,   154,   153,
     207,    24,   386,   387,    13,     4,     5,    31,     4,   345,
     346,   346,     5,    96,   379,    97,   199,   208,   242,   244,
     245,   247,   251,   254,   374,   375,   378,   373,     4,     5,
     345,   345,     5,    54,   230,   345,     4,     5,   345,   229,
       3,    24,   293,   294,   295,   300,     5,   285,     4,     4,
       3,   191,   228,   211,   384,   212,   347,    71,    85,   277,
     278,   323,     6,    13,    31,    24,     4,    69,     3,   253,
     263,   333,   348,    34,   105,    29,    30,    78,   207,   260,
     199,   235,   236,   165,   387,   366,    24,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   327,   328,   329,   330,     4,     4,     4,   379,
     379,   207,     5,    96,    97,   345,     4,     5,     5,   274,
     228,    35,    55,   231,     4,   345,     4,     4,   293,   301,
     302,   345,     4,   189,   367,   237,     3,   358,   359,    24,
      24,     3,   344,   347,    48,   250,   250,   207,    24,   271,
     273,   263,     3,    51,    69,    80,    83,   232,   238,    24,
     232,   266,   388,    13,    25,     4,     5,   199,   208,   244,
     247,   251,   254,   375,   379,     4,   345,   345,     4,   344,
     237,   139,   296,   138,   303,     5,     4,     5,   189,    46,
      47,   321,   224,   201,     4,     5,   348,   154,   166,   226,
     239,   240,    24,   379,    63,   207,     4,   293,   297,   298,
     299,    72,   193,    35,   193,   345,   367,     4,     4,   347,
     207,   354,     4,     5,     4,     5,    31,    35,   100,   217,
     219,   305,   306,   366,   136,   137,   151,   304,   308,   240,
     298,   300,   194,   195,   208,   241,   338,   384,     5,   147,
     318,     4,    34,   134,   141,   309,   310,   312,   347,     5,
      32,    44,   196,   196,   101,   218,   306,   117,   148,   149,
     319,   134,   141,   311,   313,   314,   316,   347,   135,   142,
     130,   307,   135,   195,    36,   197,   197,   338,   135,   142,
     154,   133,   135,    54,   132,   140,   141,    37,    38,   198,
     134,   313,   315,   131,   142,   133
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
     200,   201,   201,   202,   202,   202,   203,   204,   205,   206,
     206,   207,   208,   209,   209,   209,   210,   210,   211,   212,
     213,   214,   214,   215,   215,   216,   216,   217,   217,   218,
     218,   219,   220,   220,   221,   222,   222,   223,   223,   224,
     224,   225,   225,   226,   226,   226,   227,   227,   228,   228,
     228,   228,   229,   230,   230,   231,   231,   232,   232,   232,
     232,   232,   233,   233,   233,   233,   233,   233,   233,   234,
     234,   235,   235,   236,   236,   237,   237,   238,   239,   239,
     240,   241,   241,   241,   241,   241,   241,   241,   241,   241,
     242,   243,   243,   244,   244,   244,   244,   244,   244,   244,
     245,   246,   247,   248,   249,   249,   249,   249,   250,   250,
     251,   252,   252,   253,   253,   254,   255,   255,   256,   257,
     258,   259,   260,   260,   260,   261,   262,   263,   264,   264,
     265,   266,   266,   266,   266,   266,   266,   266,   266,   266,
     267,   267,   267,   268,   268,   269,   269,   269,   269,   270,
     271,   271,   271,   271,   271,   272,   273,   273,   273,   273,
     273,   273,   273,   273,   274,   274,   275,   275,   275,   275,
     275,   275,   276,   276,   276,   277,   278,   278,   279,   279,
     279,   279,   280,   281,   281,   281,   282,   283,   283,   283,
     283,   283,   283,   283,   284,   285,   285,   285,   286,   287,
     287,   288,   289,   290,   290,   291,   292,   292,   293,   294,
     294,   295,   296,   296,   297,   297,   298,   299,   300,   301,
     301,   302,   303,   303,   304,   304,   305,   305,   306,   307,
     307,   307,   307,   307,   308,   308,   308,   309,   309,   310,
     310,   310,   311,   312,   313,   313,   313,   314,   314,   315,
     315,   316,   317,   317,   317,   317,   318,   318,   319,   319,
     319,   320,   321,   321,   321,   322,   322,   323,   324,   324,
     325,   326,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   328,   328,
     329,   330,   330,   331,   332,   332,   333,   333,   333,   333,
     333,   333,   333,   333,   334,   335,   335,   335,   335,   336,
     336,   336,   336,   337,   337,   337,   338,   338,   338,   338,
     338,   339,   340,   340,   340,   340,   340,   340,   341,   342,
     343,   344,   344,   345,   346,   346,   347,   347,   347,   348,
     349,   349,   350,   350,   351,   352,   353,   354,   354,   355,
     356,   356,   357,   358,   359,   360,   360,   360,   361,   362,
     362,   363,   363,   364,   364,   365,   365,   365,   366,   367,
     368,   369,   369,   370,   371,   372,   372,   373,   374,   374,
     375,   375,   375,   375,   375,   375,   376,   376,   377,   378,
     378,   379,   380,   381,   382,   382,   383,   383,   384,   384,
     385,   386,   386,   387,   387,   388,   388,   388
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
       1,     1,     3,     4,     5,     1,     5,     5,     5,     4,
       5,     5,     7,     6,     0,     2,     1,     1,     1,     1,
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
       2,     1,     1,     6,     0,     2,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     2,     2,     2,     1,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     1,     1,     1,     2,     2,     2,     1,     1,
       1,     1,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     3,     3,     1,     1,     2,     1,     1,     3,
       3,     3,     1,     2,     1,     1,     3,     3,     2,     0,
       3,     0,     1,     1,     3,     2,     1,     1,     2,     1,
       1,     1,     1,     5,     4,     1,     2,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     4,     0,
       2,     1,     1,     1,     2,     1,     0,     2,     0,     1,
       2,     2,     1,     1,     3,     1,     1,     1
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

  case 216: /* general_set_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 217: /* general_set_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 218: /* general_set_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 219: /* general_set_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 220: /* general_set_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 221: /* general_set_fct: SQL_TOKEN_RTRIM '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 222: /* general_set_fct: SQL_TOKEN_RTRIM '(' opt_all_distinct function_arg ',' function_arg ')'  */
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

  case 223: /* general_set_fct: SQL_TOKEN_GROUP_CONCAT '(' opt_all_distinct function_arg opt_function_arg ')'  */
        {
            if ((yyvsp[-3].pParseNode)->isToken()!=0 && (yyvsp[-1].pParseNode)->count()!=0)
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

  case 224: /* opt_function_arg: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 225: /* opt_function_arg: ',' function_arg  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 232: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 233: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 234: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 235: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 238: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 239: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 241: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 242: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 243: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 244: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 246: /* window_function: window_function_type opt_filter_clause SQL_TOKEN_OVER window_name_or_specification  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 247: /* window_function_type: rank_function_type '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 248: /* window_function_type: SQL_TOKEN_ROW_NUMBER '(' ')'  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 254: /* ntile_function: SQL_TOKEN_NTILE '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 255: /* opt_lead_or_lag_function: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 256: /* opt_lead_or_lag_function: ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 257: /* opt_lead_or_lag_function: ',' function_arg ',' function_arg  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(",", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 258: /* lead_or_lag_function: lead_or_lag '(' lead_or_lag_extent opt_lead_or_lag_function ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-4].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 262: /* first_or_last_value_function: first_or_last_value '(' function_arg ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
	}
    break;

  case 265: /* nth_value_function: SQL_TOKEN_NTH_VALUE '(' function_arg ',' function_arg ')'  */
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

  case 266: /* opt_filter_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 267: /* opt_filter_clause: SQL_TOKEN_FILTER '(' where_clause ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 272: /* opt_window_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 273: /* opt_window_clause: SQL_TOKEN_WINDOW window_definition_list  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 274: /* window_definition_list: window_definition_list ',' window_definition  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 275: /* window_definition_list: window_definition  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 276: /* window_definition: new_window_name SQL_TOKEN_AS window_specification  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 278: /* window_specification: '(' opt_existing_window_name opt_window_partition_clause opt_order_by_clause opt_window_frame_clause ')'  */
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

  case 279: /* opt_existing_window_name: %empty  */
                                 {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 282: /* opt_window_partition_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 283: /* opt_window_partition_clause: SQL_TOKEN_PARTITION SQL_TOKEN_BY window_partition_column_reference_list  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 284: /* opt_window_frame_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 285: /* opt_window_frame_clause: window_frame_units window_frame_extent opt_window_frame_exclusion  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 286: /* window_partition_column_reference_list: window_partition_column_reference_list ',' window_partition_column_reference  */
                        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
			(yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 287: /* window_partition_column_reference_list: window_partition_column_reference  */
                        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 288: /* window_partition_column_reference: column_ref opt_collate_clause  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 289: /* opt_window_frame_exclusion: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 290: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 291: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_GROUP  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 292: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_TIES  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 293: /* opt_window_frame_exclusion: SQL_TOKEN_EXCLUDE SQL_TOKEN_NO SQL_TOKEN_OTHERS  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 299: /* window_frame_start: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 300: /* window_frame_start: value_exp SQL_TOKEN_PRECEDING  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
	    }
    break;

  case 301: /* window_frame_start: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 302: /* window_frame_preceding: value_exp SQL_TOKEN_PRECEDING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 303: /* window_frame_between: SQL_TOKEN_BETWEEN window_frame_bound_1 SQL_TOKEN_AND window_frame_bound_2  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-3].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-2].pParseNode));
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 305: /* window_frame_bound: SQL_TOKEN_CURRENT SQL_TOKEN_ROW  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 308: /* window_frame_bound_1: SQL_TOKEN_UNBOUNDED SQL_TOKEN_PRECEDING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* window_frame_bound_2: SQL_TOKEN_UNBOUNDED SQL_TOKEN_FOLLOWING  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 311: /* window_frame_following: value_exp SQL_TOKEN_FOLLOWING  */
        {
		(yyval.pParseNode) = SQL_NEW_RULE;
		(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
		(yyval.pParseNode)->append((yyvsp[0].pParseNode));
	}
    break;

  case 316: /* opt_collate_clause: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 317: /* opt_collate_clause: SQL_TOKEN_COLLATE collating_function  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 321: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 322: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 327: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 328: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 348: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 349: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 350: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 353: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 354: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 355: /* opt_extract_value: SQL_ARROW property_path  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));

        }
    break;

  case 362: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 366: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 367: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 368: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 370: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 371: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 372: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 374: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 375: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 377: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 380: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 385: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 388: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 389: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 390: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 392: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 394: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 395: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 402: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 403: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 406: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 407: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 408: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 409: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 410: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 411: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 412: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 413: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 414: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 415: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 416: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 417: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 418: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 419: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 420: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 421: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 422: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 423: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 424: /* property_path: property_path '.' property_path_entry  */
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

  case 425: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 426: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 427: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 428: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 433: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 434: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 435: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 436: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 437: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 438: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 439: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 446: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 447: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 448: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 449: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 450: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 454: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 455: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 456: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 457: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 458: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 460: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 461: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 462: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 463: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 464: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
