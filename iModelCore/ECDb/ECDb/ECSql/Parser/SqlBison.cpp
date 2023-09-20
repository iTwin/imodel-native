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
  YYSYMBOL_SQL_TOKEN_ON = 70,              /* SQL_TOKEN_ON  */
  YYSYMBOL_SQL_TOKEN_ORDER = 71,           /* SQL_TOKEN_ORDER  */
  YYSYMBOL_SQL_TOKEN_OUTER = 72,           /* SQL_TOKEN_OUTER  */
  YYSYMBOL_SQL_TOKEN_ROLLBACK = 73,        /* SQL_TOKEN_ROLLBACK  */
  YYSYMBOL_SQL_TOKEN_IIF = 74,             /* SQL_TOKEN_IIF  */
  YYSYMBOL_SQL_TOKEN_SELECT = 75,          /* SQL_TOKEN_SELECT  */
  YYSYMBOL_SQL_TOKEN_SET = 76,             /* SQL_TOKEN_SET  */
  YYSYMBOL_SQL_TOKEN_SOME = 77,            /* SQL_TOKEN_SOME  */
  YYSYMBOL_SQL_TOKEN_SUM = 78,             /* SQL_TOKEN_SUM  */
  YYSYMBOL_SQL_TOKEN_TRUE = 79,            /* SQL_TOKEN_TRUE  */
  YYSYMBOL_SQL_TOKEN_UNION = 80,           /* SQL_TOKEN_UNION  */
  YYSYMBOL_SQL_TOKEN_UNIQUE = 81,          /* SQL_TOKEN_UNIQUE  */
  YYSYMBOL_SQL_TOKEN_UNKNOWN = 82,         /* SQL_TOKEN_UNKNOWN  */
  YYSYMBOL_SQL_TOKEN_UPDATE = 83,          /* SQL_TOKEN_UPDATE  */
  YYSYMBOL_SQL_TOKEN_USING = 84,           /* SQL_TOKEN_USING  */
  YYSYMBOL_SQL_TOKEN_VALUE = 85,           /* SQL_TOKEN_VALUE  */
  YYSYMBOL_SQL_TOKEN_VALUES = 86,          /* SQL_TOKEN_VALUES  */
  YYSYMBOL_SQL_TOKEN_WHERE = 87,           /* SQL_TOKEN_WHERE  */
  YYSYMBOL_SQL_TOKEN_DOLLAR = 88,          /* SQL_TOKEN_DOLLAR  */
  YYSYMBOL_SQL_BITWISE_NOT = 89,           /* SQL_BITWISE_NOT  */
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
  YYSYMBOL_SQL_TOKEN_OR = 121,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 122,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_ARROW = 123,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 124,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 125,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 126,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 127,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 128,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 129,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 130,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 131,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 132,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 133,                /* SQL_EQUAL  */
  YYSYMBOL_134_ = 134,                     /* '+'  */
  YYSYMBOL_135_ = 135,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 136,               /* SQL_CONCAT  */
  YYSYMBOL_137_ = 137,                     /* '*'  */
  YYSYMBOL_138_ = 138,                     /* '/'  */
  YYSYMBOL_139_ = 139,                     /* '%'  */
  YYSYMBOL_140_ = 140,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 141,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 142,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 143,     /* sql_single_statement  */
  YYSYMBOL_sql = 144,                      /* sql  */
  YYSYMBOL_pragma = 145,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 146,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 147,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 148,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 149,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 150,             /* pragma_value  */
  YYSYMBOL_pragma_path = 151,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 152,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 153,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 154,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 155,           /* cte_block_list  */
  YYSYMBOL_cte = 156,                      /* cte  */
  YYSYMBOL_column_commalist = 157,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 158,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 159,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 160, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 161,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 162,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 163,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 164,             /* opt_asc_desc  */
  YYSYMBOL_opt_null_order = 165,           /* opt_null_order  */
  YYSYMBOL_first_last_desc = 166,          /* first_last_desc  */
  YYSYMBOL_sql_not = 167,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 168,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 169,         /* select_statement  */
  YYSYMBOL_union_op = 170,                 /* union_op  */
  YYSYMBOL_commit_statement = 171,         /* commit_statement  */
  YYSYMBOL_delete_statement_searched = 172, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 173,         /* insert_statement  */
  YYSYMBOL_values_commalist = 174,         /* values_commalist  */
  YYSYMBOL_values_or_query_spec = 175,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 176, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 177,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 178, /* row_value_constructor_elem  */
  YYSYMBOL_rollback_statement = 179,       /* rollback_statement  */
  YYSYMBOL_select_statement_into = 180,    /* select_statement_into  */
  YYSYMBOL_opt_all_distinct = 181,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 182,     /* assignment_commalist  */
  YYSYMBOL_assignment = 183,               /* assignment  */
  YYSYMBOL_update_source = 184,            /* update_source  */
  YYSYMBOL_update_statement_searched = 185, /* update_statement_searched  */
  YYSYMBOL_target_commalist = 186,         /* target_commalist  */
  YYSYMBOL_target = 187,                   /* target  */
  YYSYMBOL_opt_where_clause = 188,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 189,  /* single_select_statement  */
  YYSYMBOL_selection = 190,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 191,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 192,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 193,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 194,                /* table_exp  */
  YYSYMBOL_from_clause = 195,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 196,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 197,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 198, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 199, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 200,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 201, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 202,                /* table_ref  */
  YYSYMBOL_where_clause = 203,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 204,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 205,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 206,              /* truth_value  */
  YYSYMBOL_boolean_primary = 207,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 208,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 209,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 210,             /* boolean_term  */
  YYSYMBOL_search_condition = 211,         /* search_condition  */
  YYSYMBOL_type_predicate = 212,           /* type_predicate  */
  YYSYMBOL_type_list = 213,                /* type_list  */
  YYSYMBOL_type_list_item = 214,           /* type_list_item  */
  YYSYMBOL_predicate = 215,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 216, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 217,     /* comparison_predicate  */
  YYSYMBOL_comparison = 218,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 219, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 220,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 221, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 222, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 223,           /* like_predicate  */
  YYSYMBOL_opt_escape = 224,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 225,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 226,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 227,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 228,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 229,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 230, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 231,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 232,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 233, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 234,             /* any_all_some  */
  YYSYMBOL_existence_test = 235,           /* existence_test  */
  YYSYMBOL_unique_test = 236,              /* unique_test  */
  YYSYMBOL_subquery = 237,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 238,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 239,           /* select_sublist  */
  YYSYMBOL_parameter_ref = 240,            /* parameter_ref  */
  YYSYMBOL_literal = 241,                  /* literal  */
  YYSYMBOL_as_clause = 242,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 243,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 244,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 245,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 246,                 /* fct_spec  */
  YYSYMBOL_function_name = 247,            /* function_name  */
  YYSYMBOL_general_set_fct = 248,          /* general_set_fct  */
  YYSYMBOL_set_fct_type = 249,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 250,          /* outer_join_type  */
  YYSYMBOL_join_condition = 251,           /* join_condition  */
  YYSYMBOL_join_spec = 252,                /* join_spec  */
  YYSYMBOL_join_type = 253,                /* join_type  */
  YYSYMBOL_cross_union = 254,              /* cross_union  */
  YYSYMBOL_qualified_join = 255,           /* qualified_join  */
  YYSYMBOL_ecrelationship_join = 256,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 257, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 258,             /* joined_table  */
  YYSYMBOL_named_columns_join = 259,       /* named_columns_join  */
  YYSYMBOL_all = 260,                      /* all  */
  YYSYMBOL_scalar_subquery = 261,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 262,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 263, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 264,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 265,        /* cast_target_array  */
  YYSYMBOL_cast_target = 266,              /* cast_target  */
  YYSYMBOL_cast_spec = 267,                /* cast_spec  */
  YYSYMBOL_opt_optional_prop = 268,        /* opt_optional_prop  */
  YYSYMBOL_opt_extract_value = 269,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 270,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 271,              /* num_primary  */
  YYSYMBOL_factor = 272,                   /* factor  */
  YYSYMBOL_term = 273,                     /* term  */
  YYSYMBOL_term_add_sub = 274,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 275,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 276,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 277,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 278,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 279,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 280,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 281,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 282,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 283,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 284,                /* value_exp  */
  YYSYMBOL_string_value_exp = 285,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 286,           /* char_value_exp  */
  YYSYMBOL_concatenation = 287,            /* concatenation  */
  YYSYMBOL_char_primary = 288,             /* char_primary  */
  YYSYMBOL_char_factor = 289,              /* char_factor  */
  YYSYMBOL_derived_column = 290,           /* derived_column  */
  YYSYMBOL_table_node = 291,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 292, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 293,     /* qualified_class_name  */
  YYSYMBOL_class_name = 294,               /* class_name  */
  YYSYMBOL_table_node_ref = 295,           /* table_node_ref  */
  YYSYMBOL_table_node_with_opt_member_func_call = 296, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 297,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 298,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 299, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 300,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 301,            /* property_path  */
  YYSYMBOL_property_path_entry = 302,      /* property_path_entry  */
  YYSYMBOL_column_ref = 303,               /* column_ref  */
  YYSYMBOL_column = 304,                   /* column  */
  YYSYMBOL_case_expression = 305,          /* case_expression  */
  YYSYMBOL_case_specification = 306,       /* case_specification  */
  YYSYMBOL_simple_case = 307,              /* simple_case  */
  YYSYMBOL_searched_case = 308,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 309,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 310,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 311,        /* when_operand_list  */
  YYSYMBOL_when_operand = 312,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 313, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 314,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 315,              /* else_clause  */
  YYSYMBOL_result = 316,                   /* result  */
  YYSYMBOL_result_expression = 317,        /* result_expression  */
  YYSYMBOL_case_operand = 318,             /* case_operand  */
  YYSYMBOL_parameter = 319,                /* parameter  */
  YYSYMBOL_range_variable = 320,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 321,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 322,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 323,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 324,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 325          /* ecsqloptionvalue  */
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
#define YYLAST   1936

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  142
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  184
/* YYNRULES -- Number of rules.  */
#define YYNRULES  400
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  619

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   374


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
       2,     2,     2,     2,     2,     2,     2,   139,     2,     2,
       3,     4,   137,   134,     5,   135,    13,   138,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   140,     2,     8,     2,     2,     2,     2,     2,     2,
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
     131,   132,   133,   136,   141
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   229,   229,   230,   231,   232,   236,   242,   254,   257,
     265,   268,   269,   273,   281,   289,   290,   291,   292,   293,
     294,   295,   299,   304,   309,   321,   324,   328,   333,   341,
     355,   361,   369,   381,   386,   394,   399,   409,   410,   418,
     419,   430,   431,   441,   446,   454,   463,   474,   475,   476,
     480,   481,   490,   490,   494,   495,   501,   502,   503,   504,
     505,   506,   507,   508,   512,   517,   528,   529,   530,   533,
     540,   552,   562,   567,   575,   585,   590,   598,   601,   606,
     615,   626,   627,   628,   632,   635,   641,   648,   651,   663,
     666,   672,   676,   677,   685,   693,   694,   708,   713,   716,
     717,   720,   721,   729,   738,   739,   753,   763,   766,   772,
     773,   777,   780,   790,   791,   798,   799,   804,   812,   813,
     820,   828,   836,   844,   848,   857,   858,   868,   869,   877,
     878,   879,   880,   881,   884,   885,   886,   887,   888,   889,
     890,   900,   901,   911,   912,   920,   921,   930,   931,   941,
     950,   955,   963,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   986,   993,  1000,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1039,  1049,  1057,  1067,  1077,  1083,  1089,  1111,
    1136,  1137,  1144,  1153,  1159,  1175,  1179,  1187,  1196,  1202,
    1218,  1227,  1236,  1245,  1255,  1256,  1257,  1261,  1269,  1275,
    1286,  1291,  1298,  1302,  1306,  1307,  1308,  1309,  1310,  1312,
    1324,  1336,  1348,  1364,  1365,  1371,  1375,  1376,  1379,  1380,
    1381,  1382,  1385,  1397,  1398,  1399,  1406,  1415,  1429,  1434,
    1450,  1466,  1475,  1483,  1495,  1496,  1497,  1498,  1499,  1503,
    1508,  1513,  1520,  1528,  1529,  1532,  1533,  1538,  1539,  1547,
    1559,  1569,  1578,  1583,  1597,  1598,  1599,  1602,  1603,  1606,
    1618,  1619,  1623,  1626,  1630,  1631,  1632,  1633,  1634,  1635,
    1636,  1637,  1638,  1639,  1640,  1641,  1642,  1643,  1644,  1645,
    1649,  1654,  1664,  1673,  1674,  1678,  1690,  1691,  1698,  1699,
    1707,  1708,  1709,  1710,  1711,  1712,  1719,  1723,  1727,  1728,
    1734,  1740,  1749,  1750,  1757,  1764,  1774,  1775,  1782,  1792,
    1793,  1800,  1807,  1814,  1824,  1831,  1836,  1841,  1846,  1852,
    1858,  1867,  1874,  1882,  1890,  1893,  1899,  1903,  1908,  1916,
    1917,  1918,  1922,  1926,  1927,  1930,  1937,  1947,  1950,  1954,
    1964,  1969,  1976,  1985,  1993,  2002,  2010,  2019,  2027,  2032,
    2037,  2045,  2054,  2055,  2065,  2066,  2074,  2079,  2097,  2103,
    2108,  2116,  2128,  2132,  2136,  2137,  2141,  2152,  2162,  2167,
    2174,  2184,  2187,  2192,  2193,  2194,  2195,  2196,  2197,  2200,
    2205,  2212,  2222,  2223,  2231,  2235,  2239,  2243,  2249,  2257,
    2260,  2270,  2271,  2275,  2284,  2289,  2297,  2303,  2313,  2314,
    2315
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
  "SQL_TOKEN_NATURAL", "SQL_TOKEN_NULL", "SQL_TOKEN_ON", "SQL_TOKEN_ORDER",
  "SQL_TOKEN_OUTER", "SQL_TOKEN_ROLLBACK", "SQL_TOKEN_IIF",
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
  "select_statement", "union_op", "commit_statement",
  "delete_statement_searched", "insert_statement", "values_commalist",
  "values_or_query_spec", "row_value_constructor_commalist",
  "row_value_constructor", "row_value_constructor_elem",
  "rollback_statement", "select_statement_into", "opt_all_distinct",
  "assignment_commalist", "assignment", "update_source",
  "update_statement_searched", "target_commalist", "target",
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
  "select_sublist", "parameter_ref", "literal", "as_clause",
  "unsigned_value_spec", "general_value_spec", "iif_spec", "fct_spec",
  "function_name", "general_set_fct", "set_fct_type", "outer_join_type",
  "join_condition", "join_spec", "join_type", "cross_union",
  "qualified_join", "ecrelationship_join", "op_relationship_direction",
  "joined_table", "named_columns_join", "all", "scalar_subquery",
  "cast_operand", "cast_target_primitive_type", "cast_target_scalar",
  "cast_target_array", "cast_target", "cast_spec", "opt_optional_prop",
  "opt_extract_value", "value_exp_primary", "num_primary", "factor",
  "term", "term_add_sub", "num_value_exp", "datetime_primary",
  "datetime_value_fct", "datetime_factor", "datetime_term",
  "datetime_value_exp", "value_exp_commalist", "function_arg",
  "function_args_commalist", "value_exp", "string_value_exp",
  "char_value_exp", "concatenation", "char_primary", "char_factor",
  "derived_column", "table_node", "tablespace_qualified_class_name",
  "qualified_class_name", "class_name", "table_node_ref",
  "table_node_with_opt_member_func_call", "table_node_path",
  "table_node_path_entry", "opt_member_function_args",
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

#define YYPACT_NINF (-503)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-379)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     630,   116,  -503,   100,   140,  -503,    55,    39,   155,   197,
     229,   216,   228,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,    47,  -503,   213,    39,   217,  -503,  -503,
    1496,   219,    88,    44,   221,   653,  -503,  -503,  -503,  -503,
    1599,    35,  -503,  -503,  -503,  -503,  -503,  -503,   220,   261,
    -503,    76,   322,    85,   271,  -503,  -503,  1187,   242,  -503,
    -503,  -503,  -503,  -503,    84,  -503,  -503,   298,   312,  -503,
     315,   330,  -503,   337,  -503,  -503,  -503,  -503,  1753,  -503,
    -503,  -503,  -503,  1290,  -503,  -503,  1599,  1599,  1599,  1753,
    1753,   435,   163,  -503,   267,  -503,    73,  -503,  -503,  -503,
    -503,   346,  -503,   348,  -503,  -503,  -503,  -503,  -503,   147,
       0,   292,  -503,  -503,  -503,  -503,  -503,    38,  -503,   222,
    -503,  -503,  -503,  -503,    36,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,   141,  -503,   221,   193,   351,   193,
     263,  -503,   304,  -503,  -503,  -503,  -503,   218,    48,   353,
     368,  -503,   135,  -503,  -503,   224,   212,   212,   349,  -503,
    -503,  -503,   141,   367,   213,    55,  -503,   727,   327,  -503,
     425,   427,    48,   385,   468,    27,  -503,  -503,  -503,  1599,
      49,    55,    55,   727,  -503,   727,  -503,   223,  -503,   376,
     224,  -503,  -503,  -503,  -503,  -503,    42,   194,  -503,   387,
    1599,  -503,  -503,  -503,  -503,  1084,    55,  1702,  1702,  1702,
    1702,  1702,  1702,  1702,  1702,  1702,  -503,   452,  1599,  -503,
    -503,   363,    48,    48,  -503,   193,  -503,   454,   271,  1599,
    -503,   455,  -503,   221,   221,    39,   418,   456,    92,  -503,
     352,  -503,    39,  -503,  1599,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,   482,  -503,   464,   327,  -503,  -503,   178,  -503,
    1496,   489,   608,   486,   467,   486,  -503,  -503,  -503,  -503,
    -503,  -503,   162,    78,   431,  -503,  -503,   371,   379,  -503,
    -503,  1599,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,   269,   329,  1744,    65,  1782,   474,
    -503,  -503,  -503,  -503,   301,  -503,  -503,   319,  -503,  -503,
    -503,  -503,   471,   224,   499,  1599,  1599,  1599,    43,   -44,
    1599,  -503,   408,   727,   409,  -503,   376,   385,   501,   771,
      66,  -503,  -503,  -503,   462,  -503,  -503,  1599,  -503,   326,
     224,  -503,  -503,  1599,  -503,  -503,  -503,   147,   147,     0,
       0,     0,     0,  -503,  -503,  -503,  -503,    77,  -503,  -503,
    -503,   332,   514,  -503,  -503,   771,    39,    48,   327,  1599,
     716,  -503,  -503,  -503,   306,  -503,   487,   496,   469,    46,
      20,  -503,  -503,  -503,   457,  -503,   521,  1599,   231,  1393,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,   467,   727,   727,
    -503,   394,   474,  -503,   425,  -503,    48,  1816,  -503,   527,
     338,   342,  1599,  1599,  -503,  -503,   156,    50,  -503,  1599,
    -503,   529,   530,   532,    59,  -503,   437,  -503,   372,  -503,
      39,   194,    39,  -503,   497,   483,   537,  -503,  1599,   538,
    -503,  -503,  -503,   452,  -503,   771,  -503,  -503,  -503,   224,
     727,    79,  -503,  -503,  -503,   522,   523,   545,  -503,  -503,
    -503,  1187,  -503,  -503,   528,    12,  1599,   520,  -503,  -503,
    -503,  -503,   486,   259,  -503,   371,   276,  -503,  -503,   544,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,   526,  -503,   555,  -503,
    -503,  -503,   557,  -503,    62,   965,  1599,  -503,  -503,   385,
     771,  -503,  1599,   727,   494,  -503,  -503,  -503,   377,  -503,
     379,   452,   364,   193,  -503,  -503,   141,   424,   224,  1599,
    -503,  -503,   438,  -503,  -503,  -503,    37,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,    73,  -503,   547,  -503,  -503,
    1599,   157,  -503,  1599,  -503,  -503,  -503,  -503,  -503,  -503,
     564,   379,   549,   473,  -503,   452,   433,  -503,  -503,  -503,
    -503,   569,  -503,  1599,   449,  1599,  -503,   217,   436,  -503,
    -503,   584,  1599,  -503,   846,  1702,   327,  -503,  -503,  -503,
    -503,   224,  -503,  -503,  -503,    37,  -503,   585,  -503,   273,
      99,   288,  -503,  -503,   846,  -503,  -503,   553,   553,  1702,
    -503,  -503,   406,  -503,  -503,   292,  -503,  -503,  -503
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,    69,     0,     0,    79,    81,   113,     0,     0,
       0,     2,     4,    63,     6,    62,    56,    57,    58,    95,
      59,    60,    61,    64,    26,     0,   113,     0,    82,    83,
       0,   114,     0,   118,     0,   245,   252,   258,   257,   123,
       0,    10,     1,     3,     5,    68,    66,    67,   260,     0,
      31,     0,    92,     0,    39,   341,   340,     0,     0,   388,
     208,   205,   206,   207,   354,   237,   234,     0,     0,   220,
       0,     0,   219,     0,   238,   235,   221,   360,     0,   315,
     317,   316,   236,     0,   204,   337,     0,     0,     0,     0,
       0,   359,   104,   262,    98,   200,   217,   290,   216,   224,
     291,     0,   223,     0,   293,   296,   297,   298,   302,   306,
     309,   329,   321,   314,   322,   323,   331,   213,   330,   332,
     334,   338,   333,   202,   288,   356,   292,   294,   363,   364,
     365,   218,   117,   116,     0,   119,     0,   389,   352,   111,
     347,   348,     0,   241,   246,   239,   240,   245,     0,   247,
       0,   359,     0,    75,    77,    78,     0,     0,     8,    11,
      12,   261,     0,     0,     0,    81,    32,    54,   391,    93,
       0,     0,     0,     0,     0,     0,   387,   355,   358,     0,
      81,    81,    81,    54,   301,    54,   386,   382,   379,     0,
       0,   318,   320,   319,   300,   299,   113,     0,    94,    92,
       0,   211,   212,   210,   209,     0,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   362,     0,     0,   339,
     215,     0,     0,     0,   361,   111,   110,     0,    39,     0,
     351,     0,   121,     0,     0,   113,     0,   354,    92,    84,
       0,   248,   113,    74,     0,    15,    16,    17,    20,    21,
      19,    18,     0,    13,     0,   391,    65,    28,     0,    30,
       0,    54,    54,     0,    54,     0,   169,   170,   166,   165,
     168,   167,     0,    54,   141,   143,   145,   147,   124,   134,
     153,     0,   154,   178,   179,   160,   184,   158,   189,   159,
     155,   161,   156,   157,   135,   136,   138,   139,   137,     0,
      70,   392,   345,   344,   345,   342,   343,     0,    36,    71,
     199,   295,     0,   263,     0,     0,     0,     0,     0,     0,
       0,   380,     0,    54,   382,   368,     0,     0,   106,   107,
     104,    89,    91,   203,   125,   201,   225,     0,   327,     0,
     385,   326,   384,     0,   303,   304,   305,   307,   308,   312,
     313,   310,   311,   214,   336,   335,   357,   286,   120,   390,
     122,     0,    37,   350,   349,   249,   113,     0,   391,     0,
     245,    76,    14,    22,     9,     7,     0,     0,   104,     0,
      78,    55,   144,   197,   171,   198,     0,     0,     0,     0,
     173,   176,   177,   183,   188,   191,   192,    54,    54,    54,
     164,   396,   393,   395,     0,    40,     0,     0,   232,     0,
       0,     0,     0,     0,   383,   367,     0,    77,   374,     0,
     375,   178,   184,   189,     0,   371,     0,   369,     0,    73,
     113,     0,   113,    80,     0,   127,     0,   226,     0,     0,
     287,   289,   353,     0,   112,   250,    85,    88,    86,    87,
      54,     0,   243,   251,   244,     0,     0,     0,    27,   140,
     182,     0,   187,   185,   180,   180,     0,     0,   195,   194,
     196,   163,     0,     0,   146,   148,     0,   394,    35,   279,
     270,   269,   271,   272,   266,   267,   273,   268,   274,   278,
     264,   265,   275,   276,   277,   280,   283,   284,     0,   233,
     229,   230,     0,   381,   164,    54,     0,   366,    96,     0,
     108,    90,     0,    54,    41,   227,   328,   231,     0,    34,
     242,     0,   254,   111,    24,    23,     0,     0,   324,     0,
     175,   174,     0,   228,   193,   190,   113,   130,   132,   129,
     131,   142,   133,   399,   400,   398,   397,     0,   282,   285,
       0,     0,   373,     0,   377,   378,   376,   372,   370,    72,
     126,   128,     0,    99,    38,     0,     0,   255,   256,   253,
     346,     0,   186,     0,   181,     0,   114,     0,     0,   150,
     281,     0,     0,   162,    54,     0,   391,   100,    33,   259,
      29,   325,   172,   152,   149,   113,   222,    42,    43,    77,
      47,   101,   105,   151,    54,    48,    49,    50,    50,     0,
     103,    44,     0,    46,    45,   102,    52,    53,    51
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,   434,  -503,
    -503,  -503,   428,  -503,  -503,    72,  -503,  -503,   373,  -503,
    -503,    -9,    -1,    -8,  -503,  -247,  -503,     3,  -503,  -503,
    -503,  -503,  -503,  -161,  -503,   -39,   -79,  -503,  -503,   -76,
    -503,   237,  -503,  -503,  -503,   171,  -124,  -503,   345,  -503,
    -503,  -503,   280,  -503,  -503,   475,  -210,  -503,  -471,   573,
       4,  -503,  -503,  -503,   139,  -503,   374,   227,   234,  -148,
    -503,  -503,    40,  -492,  -503,  -503,  -255,   361,  -503,  -250,
     366,  -503,   172,  -244,  -503,  -503,  -237,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,   -25,  -503,   440,  -503,   167,
    -503,  -146,  -503,  -503,  -151,  -503,  -503,  -503,  -503,  -503,
    -503,   498,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -142,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,   257,   114,
      81,   235,   209,  -502,  -503,  -503,  -503,  -503,  -503,   134,
    -263,  -170,   -30,   -81,  -503,  -503,  -503,   429,  -503,    71,
    -503,   480,   490,  -503,  -122,  -503,   236,  -503,  -503,   446,
     439,  -139,  -115,  -503,  -503,  -503,  -503,  -503,   350,  -503,
     151,   491,  -165,   347,  -280,  -503,  -503,  -177,  -503,  -242,
    -503,  -503,   270,  -503
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,   255,   158,   159,   160,   252,   374,
      25,   258,    50,    51,    13,   518,   307,   444,   173,   563,
     597,   598,   607,   613,   618,   272,    14,   174,    48,    16,
      17,    18,   428,    19,   152,   273,   154,    20,    21,    30,
     238,   239,   448,    22,   330,   331,   168,    23,    92,   586,
     610,   587,   198,   199,   328,   231,   232,    32,    33,    34,
     329,   169,   435,   514,   541,   274,   275,   276,   277,   319,
     542,   578,   579,   279,   418,   280,   281,   420,   282,   283,
     284,   285,   530,   286,   287,   462,   288,   289,   395,   290,
     291,   396,   472,   292,   293,    93,    94,    95,   332,    96,
     219,    97,    98,    99,   100,   101,   102,   103,   149,   452,
     453,   150,    36,    37,    38,   569,    39,   454,   162,   104,
     312,   495,   496,   497,   498,   105,   441,   224,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   527,
     338,   339,   340,   118,   119,   120,   121,   122,   123,    54,
      55,    56,   306,   522,   139,   140,   141,   230,   178,   124,
     125,   126,   519,   127,   128,   129,   130,   324,   325,   424,
     425,   187,   188,   322,   341,   342,   189,   131,   228,   300,
     301,   402,   403,   546
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     117,   153,   220,    15,   186,   191,   192,   193,   137,   240,
     155,    35,   309,   375,   225,   358,   295,   384,   389,   278,
     333,   294,   321,   391,   311,   296,   388,   175,   298,   393,
      52,   311,   295,   308,   295,   318,   394,   294,   156,   294,
     414,   296,  -115,   296,   298,   327,   298,   134,   412,   222,
     459,   413,   409,   155,   166,  -373,   190,   190,   190,   361,
     529,  -115,   216,  -115,   505,   577,  -115,  -162,   419,   217,
     -78,   431,   237,   421,   436,   334,   416,   399,    28,   422,
     439,   164,   521,   601,    28,   440,   423,  -228,   -78,   260,
     222,   170,   600,   201,    29,   202,    45,   367,   171,   -78,
      29,   381,   353,   138,   315,   316,   317,   615,    46,   177,
     295,   295,   600,   379,   368,   294,   294,   132,   432,   296,
     296,   -78,   298,   298,   577,   -78,   447,    47,   -78,   337,
     343,   605,   502,   503,   210,   211,    77,   297,   264,   243,
     244,  -139,  -139,   606,    24,  -373,   410,   411,  -330,   313,
     473,   165,    26,   297,   506,   297,   218,  -162,    40,   223,
     -78,   321,     8,   218,   399,   256,   429,   399,   157,   -78,
     117,   576,   295,    31,   218,   516,    31,   294,   135,   167,
     203,   296,   376,   377,   298,   151,   314,   204,   354,   133,
     466,   466,   184,   -78,   -78,   -78,   -78,   -78,   -78,    27,
      58,   218,    59,   194,   195,   371,   266,   267,   268,   269,
     270,   271,   386,   386,   155,   196,   165,  -109,   386,   387,
     582,    41,   197,    43,   226,   387,   558,     8,   240,    42,
     117,   380,   297,   245,   246,    44,   247,    49,   383,   365,
     385,    53,   400,  -119,   417,   138,   370,   295,   295,   161,
     553,   155,   294,   294,   333,   554,   296,   296,   551,   298,
     298,   555,   536,   248,   163,   466,   176,   478,   556,   233,
     581,   143,   200,   -47,   172,   144,   234,   -47,   -47,   536,
     -47,   249,   145,   146,   207,   208,   209,   386,   344,   345,
     346,   250,  -290,   297,   387,    60,    61,    62,    63,   295,
     543,   179,   520,  -290,   294,   605,   465,   170,   296,   -47,
     537,   298,   455,   570,   404,   180,   534,   606,   181,   456,
     185,   320,   -47,   405,   406,  -290,   251,   537,   538,   523,
     437,   438,  -290,   182,   -47,   467,   442,   438,   539,   449,
     183,   540,   500,   438,   602,   538,   501,   438,   559,   205,
     471,   206,  -291,   -47,   229,   539,   221,   190,   540,   155,
     218,   463,   295,  -291,   142,   561,   235,   294,   297,   297,
     445,   296,   -47,  -290,   298,   143,   508,   509,   -47,   144,
     504,   564,   565,    84,  -245,  -291,   145,   146,   609,   155,
     147,   257,  -291,  -290,  -290,  -290,  -290,  -290,  -290,  -290,
    -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,   167,
     567,   568,   212,   213,   214,   215,   212,   213,   214,   215,
     297,   349,   350,   351,   352,   241,   552,   532,   572,   573,
     242,   528,   299,  -291,   510,   -97,   155,   589,   565,   -97,
     594,   595,   -97,   616,   617,   347,   348,   535,   574,   302,
     588,   304,   254,  -291,  -291,  -291,  -291,  -291,  -291,  -291,
    -291,  -291,  -291,  -291,  -291,  -291,  -291,  -291,  -291,   363,
     364,     8,   310,   323,   167,   155,   216,    85,   359,   362,
     366,   177,   528,   297,   -97,   369,   372,   -97,   373,   134,
     381,   397,   261,   398,   -97,    58,   -97,    59,   401,   190,
     399,   465,   407,   408,   415,   599,   430,   320,    60,    61,
      62,    63,   262,    64,   583,   -97,   434,   443,   457,    65,
     458,   196,    66,   155,   461,   599,   460,   476,    67,   571,
      68,   499,   512,   507,  -377,  -378,   592,  -376,   513,   263,
      69,   515,   517,   591,   533,   155,   524,   525,   526,   264,
      65,   548,   190,    66,   155,    70,    71,   547,    72,   549,
     575,    68,   550,    73,   165,   562,    74,    75,    76,   573,
     265,   580,   585,   590,   155,     8,   529,    77,    78,    79,
      80,    81,    82,    83,   584,  -330,    70,    71,   596,   612,
     604,   253,   259,   566,    73,   611,    84,    74,    75,   608,
     614,   360,   511,    85,   446,   378,   136,    86,    87,    88,
     433,   261,   227,    82,    58,   544,    59,   266,   267,   268,
     269,   270,   271,    89,    90,   474,   151,    60,    61,    62,
      63,   381,    64,   475,   390,   603,   382,   531,    65,   392,
     335,    66,   -55,   545,   464,   236,   560,    67,   593,    68,
     355,   305,  -297,  -297,  -297,  -297,   557,     1,   263,    69,
     303,   356,  -297,  -297,  -297,  -297,  -297,  -297,   264,   357,
       2,   426,   477,     3,    70,    71,   427,    72,     0,     0,
     326,     0,    73,     0,     0,    74,    75,    76,     4,   265,
       0,     0,     0,     0,     0,   142,    77,    78,    79,    80,
      81,    82,    83,     5,     0,     6,   143,     0,     0,     0,
     144,     0,     0,     7,     0,    84,     8,   145,   146,     0,
       0,   147,    85,     0,     0,     0,    86,    87,    88,   148,
     261,     0,     9,    58,     0,    59,   266,   267,   268,   269,
     270,   271,    89,    90,     0,   151,    60,    61,    62,    63,
     262,    64,     0,     0,     0,     0,     0,    65,   142,     0,
      66,     0,     0,     0,     0,     0,    67,     0,    68,   143,
       0,     0,     0,   144,     0,     0,     0,   263,    69,     0,
     145,   146,     0,     0,   147,     0,   450,   264,     0,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
     451,    73,     0,     0,    74,    75,    76,     0,   265,     0,
       0,     0,     0,   142,     0,    77,    78,    79,    80,    81,
      82,    83,     0,     0,   143,     0,     0,     0,   144,     0,
       0,     0,     0,  -245,    84,   145,   146,     0,     0,   147,
       0,    85,     0,     0,     0,    86,    87,    88,     0,    57,
       0,     0,    58,     0,    59,   266,   267,   268,   269,   270,
     271,    89,    90,     0,   151,    60,    61,    62,    63,   381,
      64,     0,     0,     0,     0,     0,    65,     0,     0,    66,
       0,     0,     0,     0,     0,    67,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   263,    69,     0,     0,
       0,     0,     0,     0,     0,     0,   264,     0,     0,     0,
       0,     0,    70,    71,     0,    72,     0,     0,     0,     0,
      73,     0,     0,    74,    75,    76,     0,   265,     0,     0,
       0,     0,     0,     0,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,    86,    87,    88,     0,    57,     0,
       0,    58,     0,    59,   266,   267,   268,   269,   270,   271,
      89,    90,     0,   151,    60,    61,    62,    63,   381,    64,
       0,     0,     0,     0,     0,    65,     0,     0,    66,     0,
       0,     0,     0,     0,    67,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,   264,     0,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,     0,     0,     0,     0,
       0,     0,     0,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,    86,    87,    88,     0,    57,   336,     0,
      58,     0,    59,   266,   267,   268,   269,   270,   271,    89,
      90,     0,   151,    60,    61,    62,    63,     0,    64,     0,
       0,     0,     0,    28,    65,     0,     0,    66,     0,     0,
       0,     0,     0,    67,     0,    68,     0,     0,     0,    29,
       0,     0,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,     0,     0,     0,     0,    73,     0,
       0,    74,    75,    76,     0,     0,     0,     0,     0,     0,
       0,     0,    77,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,    84,     0,    58,     0,    59,     0,     0,    85,     0,
       0,     0,    86,    87,    88,     0,    60,    61,    62,    63,
       0,    64,     0,     0,     0,     0,     0,    65,    89,    90,
      66,   151,     0,     0,     0,     0,    67,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
       0,    73,   165,     0,    74,    75,    76,     0,     0,     0,
       0,     0,     0,     8,     0,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    57,    84,     0,    58,     0,    59,     0,
       0,    85,     0,     0,     0,    86,    87,    88,     0,    60,
      61,    62,    63,     0,    64,     0,     0,     0,     0,     0,
      65,    89,    90,    66,   151,     0,     0,     0,     0,    67,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
       0,     0,     0,     0,    73,     0,     0,    74,    75,    76,
       0,     0,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,    83,     0,     0,   185,     0,     0,
       0,     0,     0,     0,     0,     0,    57,    84,     0,    58,
       0,    59,     0,     0,    85,     0,     0,     0,    86,    87,
      88,     0,    60,    61,    62,    63,     0,    64,     0,     0,
       0,     0,   468,   469,    89,    90,    66,   151,     0,     0,
       0,     0,    67,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
     470,    75,    76,     0,     0,     0,     0,     0,     0,     0,
       0,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    57,
      84,     0,    58,     0,    59,     0,     0,    85,     0,     0,
       0,    86,    87,    88,     0,    60,    61,    62,    63,     0,
      64,     0,     0,     0,     0,     0,    65,    89,    90,    66,
     151,     0,     0,     0,     0,    67,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    70,    71,     0,    72,     0,     0,     0,     0,
      73,     0,     0,    74,    75,    76,     0,     0,     0,     0,
       0,     0,     0,     0,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    57,    84,     0,    58,     0,    59,     0,     0,
      85,     0,     0,     0,    86,    87,    88,     0,    60,    61,
      62,    63,     0,    64,     0,     0,     0,     0,     0,    65,
      89,    90,    66,    91,     0,     0,     0,     0,    67,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,     0,
       0,     0,     0,    73,     0,     0,    74,    75,    76,     0,
       0,     0,     0,     0,     0,     0,     0,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    57,    84,     0,    58,     0,
      59,     0,     0,    85,     0,     0,     0,    86,    87,    88,
       0,    60,    61,    62,    63,     0,    64,     0,     0,     0,
       0,     0,    65,    89,    90,    66,   151,     0,     0,     0,
       0,    67,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    57,     0,     0,    58,
       0,    59,     0,     0,     0,     0,     0,  -293,    70,    71,
       0,    72,    60,    61,    62,    63,    73,    64,  -293,    74,
      75,    76,     0,    65,     0,     0,    66,     0,     0,     0,
      77,    78,    67,     0,    68,    82,    83,     0,     0,     0,
    -293,     0,     0,     0,    69,  -292,     0,  -293,     0,    84,
       0,     0,     0,     0,     0,     0,  -292,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      74,    75,    76,     0,     0,     0,    89,    90,  -292,   151,
     479,    77,     0,     0,     0,  -292,    82,    83,  -293,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,     0,     0,  -292,     0,     0,     0,
     151,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -292,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494
};

static const yytype_int16 yycheck[] =
{
      30,    40,   117,     0,    83,    86,    87,    88,    33,   148,
      40,     7,   173,   255,   136,   225,   167,   264,   273,   167,
     197,   167,   187,   273,     4,   167,   273,    57,   167,   273,
      26,     4,   183,   172,   185,   183,   273,   183,     3,   185,
     320,   183,     3,   185,   183,     3,   185,     3,     5,    13,
       4,    95,   315,    83,    51,     5,    86,    87,    88,   229,
      48,    24,    24,    24,     5,   536,    24,     5,   323,    31,
       5,     5,    24,   323,   337,   199,   323,   121,    29,   323,
     343,     5,     3,   585,    29,     8,   323,     3,    23,   165,
      13,     6,   584,    20,    45,    22,    49,     5,    13,    34,
      45,    23,   217,    24,   180,   181,   182,   609,    61,    25,
     261,   262,   604,   261,   238,   261,   262,    29,    52,   261,
     262,    56,   261,   262,   595,    60,   368,    80,    63,   205,
     206,    32,   412,   413,   134,   135,    88,   167,    60,     4,
       5,   121,   122,    44,    28,    95,   316,   317,   136,   179,
     397,    75,    52,   183,    95,   185,   136,    95,     3,   123,
      95,   326,    86,   136,   121,   162,   327,   121,   133,   104,
     200,   134,   323,   134,   136,   438,   134,   323,   134,    87,
     107,   323,     4,     5,   323,   137,   137,   114,   218,   101,
      34,    34,    78,   128,   129,   130,   131,   132,   133,    59,
       6,   136,     8,    89,    90,   244,   128,   129,   130,   131,
     132,   133,    56,    56,   244,    52,    75,    24,    56,    63,
      63,    24,    59,     7,    31,    63,   506,    86,   367,     0,
     260,   261,   262,    21,    22,     7,    24,    24,   263,   235,
     265,    24,   281,    24,   323,    24,   242,   398,   399,    29,
     505,   281,   398,   399,   431,   505,   398,   399,   505,   398,
     399,   505,     3,    51,     3,    34,    24,   406,   505,     6,
     550,    53,     5,     0,     3,    57,    13,     4,     5,     3,
       7,    69,    64,    65,   137,   138,   139,    56,   207,   208,
     209,    79,    23,   323,    63,    19,    20,    21,    22,   450,
      24,     3,   450,    34,   450,    32,   387,     6,   450,    36,
      51,   450,     6,   523,    13,     3,   467,    44,     3,    13,
      97,    98,    49,     4,     5,    56,   114,    51,    69,   451,
       4,     5,    63,     3,    61,   104,     4,     5,    79,   369,
       3,    82,     4,     5,   586,    69,     4,     5,   509,     3,
     389,     3,    23,    80,     3,    79,   134,   387,    82,   389,
     136,   386,   513,    34,    42,   513,    62,   513,   398,   399,
     366,   513,    99,   104,   513,    53,     4,     5,   105,    57,
     419,     4,     5,   107,    62,    56,    64,    65,   100,   419,
      68,    24,    63,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    87,
      46,    47,   124,   125,   126,   127,   124,   125,   126,   127,
     450,   212,   213,   214,   215,    72,   505,   466,     4,     5,
      62,   461,   105,   104,   430,     0,   466,     4,     5,     4,
       4,     5,     7,    37,    38,   210,   211,   472,   529,    24,
     565,    24,   103,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   233,
     234,    86,     4,    97,    87,   505,    24,   114,    24,    24,
      62,    25,   512,   513,    49,   133,     4,    52,    24,     3,
      23,    60,     3,   122,    59,     6,    61,     8,    24,   529,
     121,   582,    31,     4,    96,   584,     5,    98,    19,    20,
      21,    22,    23,    24,   553,    80,    54,     3,    31,    30,
      24,    52,    33,   553,     3,   604,    69,   133,    39,   526,
      41,     4,    35,    96,     5,     5,   575,     5,    55,    50,
      51,     4,     4,   573,    24,   575,    24,    24,     3,    60,
      30,    25,   582,    33,   584,    66,    67,    13,    69,     4,
     122,    41,     5,    74,    75,    71,    77,    78,    79,     5,
      81,    24,    99,     4,   604,    86,    48,    88,    89,    90,
      91,    92,    93,    94,    35,   136,    66,    67,     4,    36,
       5,   157,   164,   521,    74,   604,   107,    77,    78,   600,
     608,   228,   431,   114,   367,   260,    33,   118,   119,   120,
     330,     3,   137,    93,     6,   476,     8,   128,   129,   130,
     131,   132,   133,   134,   135,   398,   137,    19,    20,    21,
      22,    23,    24,   399,   273,   595,   262,   465,    30,   273,
     200,    33,    34,   476,   387,   147,   512,    39,   577,    41,
     221,   171,   124,   125,   126,   127,   505,    27,    50,    51,
     170,   222,   134,   135,   136,   137,   138,   139,    60,   223,
      40,   324,   402,    43,    66,    67,   326,    69,    -1,    -1,
     189,    -1,    74,    -1,    -1,    77,    78,    79,    58,    81,
      -1,    -1,    -1,    -1,    -1,    42,    88,    89,    90,    91,
      92,    93,    94,    73,    -1,    75,    53,    -1,    -1,    -1,
      57,    -1,    -1,    83,    -1,   107,    86,    64,    65,    -1,
      -1,    68,   114,    -1,    -1,    -1,   118,   119,   120,    76,
       3,    -1,   102,     6,    -1,     8,   128,   129,   130,   131,
     132,   133,   134,   135,    -1,   137,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    42,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    39,    -1,    41,    53,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    50,    51,    -1,
      64,    65,    -1,    -1,    68,    -1,    70,    60,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,
      84,    74,    -1,    -1,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    -1,    42,    -1,    88,    89,    90,    91,    92,
      93,    94,    -1,    -1,    53,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,   107,    64,    65,    -1,    -1,    68,
      -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,     3,
      -1,    -1,     6,    -1,     8,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,   137,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,    -1,   118,   119,   120,    -1,     3,    -1,
      -1,     6,    -1,     8,   128,   129,   130,   131,   132,   133,
     134,   135,    -1,   137,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    -1,     3,     4,    -1,
       6,    -1,     8,   128,   129,   130,   131,   132,   133,   134,
     135,    -1,   137,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,    -1,    29,    30,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,   107,    -1,     6,    -1,     8,    -1,    -1,   114,    -1,
      -1,    -1,   118,   119,   120,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,   134,   135,
      33,   137,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    88,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,   107,    -1,     6,    -1,     8,    -1,
      -1,   114,    -1,    -1,    -1,   118,   119,   120,    -1,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,   134,   135,    33,   137,    -1,    -1,    -1,    -1,    39,
      -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    -1,    -1,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,   107,    -1,     6,
      -1,     8,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
     120,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    -1,    29,    30,   134,   135,    33,   137,    -1,    -1,
      -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
     107,    -1,     6,    -1,     8,    -1,    -1,   114,    -1,    -1,
      -1,   118,   119,   120,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    30,   134,   135,    33,
     137,    -1,    -1,    -1,    -1,    39,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,   107,    -1,     6,    -1,     8,    -1,    -1,
     114,    -1,    -1,    -1,   118,   119,   120,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,    -1,    -1,    30,
     134,   135,    33,   137,    -1,    -1,    -1,    -1,    39,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,   107,    -1,     6,    -1,
       8,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,   134,   135,    33,   137,    -1,    -1,    -1,
      -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    23,    66,    67,
      -1,    69,    19,    20,    21,    22,    74,    24,    34,    77,
      78,    79,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      88,    89,    39,    -1,    41,    93,    94,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    51,    23,    -1,    63,    -1,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    66,
      67,    -1,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      77,    78,    79,    -1,    -1,    -1,   134,   135,    56,   137,
      24,    88,    -1,    -1,    -1,    63,    93,    94,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,    -1,   104,    -1,    -1,    -1,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    40,    43,    58,    73,    75,    83,    86,   102,
     143,   144,   145,   156,   168,   169,   171,   172,   173,   175,
     179,   180,   185,   189,    28,   152,    52,    59,    29,    45,
     181,   134,   199,   200,   201,   202,   254,   255,   256,   258,
       3,    24,     0,     7,     7,    49,    61,    80,   170,    24,
     154,   155,   202,    24,   291,   292,   293,     3,     6,     8,
      19,    20,    21,    22,    24,    30,    33,    39,    41,    51,
      66,    67,    69,    74,    77,    78,    79,    88,    89,    90,
      91,    92,    93,    94,   107,   114,   118,   119,   120,   134,
     135,   137,   190,   237,   238,   239,   241,   243,   244,   245,
     246,   247,   248,   249,   261,   267,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   284,   285,   286,
     287,   288,   289,   290,   301,   302,   303,   305,   306,   307,
     308,   319,    29,   101,     3,   134,   201,   237,    24,   296,
     297,   298,    42,    53,    57,    64,    65,    68,    76,   250,
     253,   137,   176,   177,   178,   284,     3,   133,   147,   148,
     149,    29,   260,     3,     5,    75,   169,    87,   188,   203,
       6,    13,     3,   160,   169,   284,    24,    25,   300,     3,
       3,     3,     3,     3,   271,    97,   178,   313,   314,   318,
     284,   285,   285,   285,   271,   271,    52,    59,   194,   195,
       5,    20,    22,   107,   114,     3,     3,   137,   138,   139,
     134,   135,   124,   125,   126,   127,    24,    31,   136,   242,
     304,   134,    13,   123,   269,   296,    31,   197,   320,     3,
     299,   197,   198,     6,    13,    62,   253,    24,   182,   183,
     303,    72,    62,     4,     5,    21,    22,    24,    51,    69,
      79,   114,   150,   150,   103,   146,   169,    24,   153,   154,
     181,     3,    23,    50,    60,    81,   128,   129,   130,   131,
     132,   133,   167,   177,   207,   208,   209,   210,   211,   215,
     217,   218,   220,   221,   222,   223,   225,   226,   228,   229,
     231,   232,   235,   236,   243,   246,   261,   284,   303,   105,
     321,   322,    24,   294,    24,   293,   294,   158,   303,   175,
       4,     4,   262,   284,   137,   181,   181,   181,   211,   211,
      98,   314,   315,    97,   309,   310,   313,     3,   196,   202,
     186,   187,   240,   319,   188,   239,     4,   181,   282,   283,
     284,   316,   317,   181,   272,   272,   272,   273,   273,   274,
     274,   274,   274,   304,   284,   289,   302,   301,   198,    24,
     160,   283,    24,   298,   298,   202,    62,     5,   188,   133,
     202,   177,     4,    24,   151,   321,     4,     5,   190,   211,
     284,    23,   208,   237,   167,   237,    56,    63,   167,   218,
     219,   221,   222,   225,   228,   230,   233,    60,   122,   121,
     177,    24,   323,   324,    13,     4,     5,    31,     4,   282,
     283,   283,     5,    95,   316,    96,   167,   178,   216,   218,
     219,   221,   225,   228,   311,   312,   315,   310,   174,   175,
       5,     5,    52,   194,    54,   204,   282,     4,     5,   282,
       8,   268,     4,     3,   159,   202,   183,   321,   184,   284,
      70,    84,   251,   252,   259,     6,    13,    31,    24,     4,
      69,     3,   227,   237,   270,   285,    34,   104,    29,    30,
      77,   177,   234,   167,   209,   210,   133,   324,   303,    24,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   263,   264,   265,   266,     4,
       4,     4,   316,   316,   177,     5,    95,    96,     4,     5,
     202,   187,    35,    55,   205,     4,   282,     4,   157,   304,
     211,     3,   295,   296,    24,    24,     3,   281,   284,    48,
     224,   224,   177,    24,   246,   237,     3,    51,    69,    79,
      82,   206,   212,    24,   206,   241,   325,    13,    25,     4,
       5,   167,   178,   218,   221,   225,   228,   312,   316,   175,
     281,   211,    71,   161,     4,     5,   157,    46,    47,   257,
     198,   169,     4,     5,   285,   122,   134,   200,   213,   214,
      24,   316,    63,   177,    35,    99,   191,   193,   304,     4,
       4,   284,   177,   291,     4,     5,     4,   162,   163,   178,
     215,   275,   321,   214,     5,    32,    44,   164,   164,   100,
     192,   163,    36,   165,   165,   275,    37,    38,   166
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   142,   143,   143,   143,   143,   144,   145,   146,   146,
     147,   147,   147,   148,   149,   150,   150,   150,   150,   150,
     150,   150,   151,   151,   151,   152,   152,   153,   153,   154,
     155,   155,   156,   157,   157,   158,   158,   159,   159,   160,
     160,   161,   161,   162,   162,   163,   163,   164,   164,   164,
     165,   165,   166,   166,   167,   167,   168,   168,   168,   168,
     168,   168,   168,   168,   169,   169,   170,   170,   170,   171,
     172,   173,   174,   174,   175,   176,   176,   177,   178,   179,
     180,   181,   181,   181,   182,   182,   183,   184,   185,   186,
     186,   187,   188,   188,   189,   189,   189,   190,   190,   191,
     191,   192,   192,   193,   194,   194,   195,   196,   196,   197,
     197,   198,   198,   199,   199,   200,   200,   200,   201,   201,
     202,   202,   202,   202,   203,   204,   204,   205,   205,   206,
     206,   206,   206,   206,   207,   207,   207,   207,   207,   207,
     207,   208,   208,   209,   209,   210,   210,   211,   211,   212,
     213,   213,   214,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   216,   217,   217,   218,   218,   218,   218,   218,
     218,   218,   219,   220,   221,   222,   223,   223,   223,   223,
     224,   224,   225,   226,   226,   227,   227,   228,   229,   229,
     230,   231,   232,   233,   234,   234,   234,   235,   236,   237,
     238,   238,   239,   240,   241,   241,   241,   241,   241,   241,
     241,   241,   241,   242,   242,   242,   243,   243,   244,   244,
     244,   244,   245,   246,   246,   246,   246,   246,   247,   248,
     248,   248,   248,   248,   249,   249,   249,   249,   249,   250,
     250,   250,   251,   252,   252,   253,   253,   253,   253,   254,
     255,   255,   255,   256,   257,   257,   257,   258,   258,   259,
     260,   260,   261,   262,   263,   263,   263,   263,   263,   263,
     263,   263,   263,   263,   263,   263,   263,   263,   263,   263,
     264,   264,   265,   266,   266,   267,   268,   268,   269,   269,
     270,   270,   270,   270,   270,   270,   270,   271,   272,   272,
     272,   272,   273,   273,   273,   273,   274,   274,   274,   275,
     275,   275,   275,   275,   276,   277,   277,   277,   277,   277,
     277,   278,   279,   280,   281,   281,   282,   283,   283,   284,
     284,   284,   285,   286,   286,   287,   287,   288,   289,   290,
     291,   291,   292,   293,   293,   294,   295,   296,   297,   297,
     297,   298,   299,   299,   300,   300,   301,   301,   302,   302,
     302,   303,   304,   305,   306,   306,   307,   308,   309,   309,
     310,   311,   311,   312,   312,   312,   312,   312,   312,   313,
     313,   314,   315,   315,   316,   317,   318,   319,   319,   320,
     320,   321,   321,   322,   323,   323,   324,   324,   325,   325,
     325
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
       5,     5,     3,     1,     4,     1,     3,     1,     1,     1,
       6,     0,     1,     1,     1,     3,     3,     1,     6,     1,
       3,     1,     0,     1,     4,     1,     7,     1,     1,     0,
       1,     0,     2,     3,     0,     7,     2,     1,     3,     0,
       1,     0,     3,     0,     1,     0,     2,     2,     0,     1,
       4,     3,     4,     1,     2,     0,     3,     0,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     4,     1,     2,     1,     3,     1,     3,     3,
       1,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     1,     1,     1,     1,     1,
       1,     2,     5,     2,     4,     4,     2,     2,     1,     1,
       0,     2,     3,     2,     1,     1,     3,     3,     2,     1,
       3,     2,     2,     3,     1,     1,     1,     2,     2,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     8,     1,     1,     3,     4,     5,     1,     5,
       5,     5,     4,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     0,     1,     1,     2,     4,
       5,     5,     1,     7,     0,     1,     1,     1,     1,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     6,     0,     1,     0,     3,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     2,
       2,     2,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     1,     1,     1,     2,     2,
       2,     1,     1,     1,     1,     3,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     2,
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

  case 72: /* values_commalist: values_commalist ',' values_or_query_spec  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 73: /* values_commalist: values_or_query_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 74: /* values_or_query_spec: SQL_TOKEN_VALUES '(' row_value_constructor_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 75: /* row_value_constructor_commalist: row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 76: /* row_value_constructor_commalist: row_value_constructor_commalist ',' row_value_constructor  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 79: /* rollback_statement: SQL_TOKEN_ROLLBACK  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 80: /* select_statement_into: SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_INTO target_commalist table_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode)); }
    break;

  case 81: /* opt_all_distinct: %empty  */
            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 84: /* assignment_commalist: assignment  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 85: /* assignment_commalist: assignment_commalist ',' assignment  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 86: /* assignment: column_ref SQL_EQUAL update_source  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 88: /* update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 89: /* target_commalist: target  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 90: /* target_commalist: target_commalist ',' target  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 92: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 94: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 96: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_FROM '(' values_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-6].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 97: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 99: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 101: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 102: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 103: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 104: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 105: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
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

  case 106: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 107: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 108: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 109: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 111: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 112: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 113: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 114: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 115: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 116: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 117: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 118: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 119: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 120: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 121: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 122: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 124: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 125: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 126: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 127: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 128: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 140: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 142: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 144: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 146: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 148: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 149: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 150: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 151: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 152: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 162: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 163: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 164: /* comparison_predicate: comparison row_value_constructor  */
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

  case 171: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 172: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 173: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 174: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 175: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 176: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 177: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 178: /* like_predicate: character_like_predicate_part_2  */
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

  case 179: /* like_predicate: other_like_predicate_part_2  */
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

  case 180: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 181: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 182: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 183: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 184: /* test_for_null: null_predicate_part_2  */
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

  case 185: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 186: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 187: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 188: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 189: /* in_predicate: in_predicate_part_2  */
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

  case 190: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 191: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 192: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 193: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 197: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 198: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 199: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 200: /* scalar_exp_commalist: select_sublist  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 201: /* scalar_exp_commalist: scalar_exp_commalist ',' select_sublist  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 209: /* literal: literal SQL_TOKEN_STRING  */
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

  case 210: /* literal: literal SQL_TOKEN_INT  */
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

  case 211: /* literal: literal SQL_TOKEN_REAL_NUM  */
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

  case 212: /* literal: literal SQL_TOKEN_APPROXNUM  */
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

  case 213: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 214: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 222: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));


        }
    break;

  case 225: /* fct_spec: function_name '(' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 226: /* fct_spec: function_name '(' function_args_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 227: /* fct_spec: function_name '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 229: /* general_set_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 230: /* general_set_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 231: /* general_set_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 232: /* general_set_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 233: /* general_set_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 239: /* outer_join_type: SQL_TOKEN_LEFT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 240: /* outer_join_type: SQL_TOKEN_RIGHT  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 241: /* outer_join_type: SQL_TOKEN_FULL  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 242: /* join_condition: SQL_TOKEN_ON search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 245: /* join_type: %empty  */
                        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 246: /* join_type: SQL_TOKEN_INNER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 248: /* join_type: outer_join_type SQL_TOKEN_OUTER  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 249: /* cross_union: table_ref SQL_TOKEN_CROSS SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 250: /* qualified_join: table_ref SQL_TOKEN_NATURAL join_type SQL_TOKEN_JOIN table_ref  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 251: /* qualified_join: table_ref join_type SQL_TOKEN_JOIN table_ref join_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 253: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 254: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 259: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 260: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 280: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 281: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 282: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 285: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 286: /* opt_optional_prop: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 287: /* opt_optional_prop: '?'  */
             {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
    }
    break;

  case 288: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 289: /* opt_extract_value: SQL_ARROW property_path opt_optional_prop  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 295: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 299: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 300: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 301: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 303: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 304: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 305: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 307: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 308: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 311: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 312: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 313: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 314: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 315: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 316: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 317: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 318: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 319: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 320: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 321: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 322: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 323: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 324: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 325: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 327: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 328: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 335: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 336: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 339: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 340: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 341: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 342: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 343: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 344: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 345: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 346: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 347: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 348: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 349: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 350: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 351: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 352: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 353: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 354: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 355: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 356: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 357: /* property_path: property_path '.' property_path_entry  */
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

  case 358: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 359: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 360: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 361: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 366: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 367: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 368: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 369: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 370: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 371: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 372: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 379: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 380: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 381: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 383: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 388: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 389: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 390: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 391: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 393: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 394: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 395: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 396: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 397: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
