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
  YYSYMBOL_SQL_TOKEN_CAST = 36,            /* SQL_TOKEN_CAST  */
  YYSYMBOL_SQL_TOKEN_COMMIT = 37,          /* SQL_TOKEN_COMMIT  */
  YYSYMBOL_SQL_TOKEN_COUNT = 38,           /* SQL_TOKEN_COUNT  */
  YYSYMBOL_SQL_TOKEN_CROSS = 39,           /* SQL_TOKEN_CROSS  */
  YYSYMBOL_SQL_TOKEN_DEFAULT = 40,         /* SQL_TOKEN_DEFAULT  */
  YYSYMBOL_SQL_TOKEN_DELETE = 41,          /* SQL_TOKEN_DELETE  */
  YYSYMBOL_SQL_TOKEN_DESC = 42,            /* SQL_TOKEN_DESC  */
  YYSYMBOL_SQL_TOKEN_DISTINCT = 43,        /* SQL_TOKEN_DISTINCT  */
  YYSYMBOL_SQL_TOKEN_FORWARD = 44,         /* SQL_TOKEN_FORWARD  */
  YYSYMBOL_SQL_TOKEN_BACKWARD = 45,        /* SQL_TOKEN_BACKWARD  */
  YYSYMBOL_SQL_TOKEN_ESCAPE = 46,          /* SQL_TOKEN_ESCAPE  */
  YYSYMBOL_SQL_TOKEN_EXCEPT = 47,          /* SQL_TOKEN_EXCEPT  */
  YYSYMBOL_SQL_TOKEN_EXISTS = 48,          /* SQL_TOKEN_EXISTS  */
  YYSYMBOL_SQL_TOKEN_FALSE = 49,           /* SQL_TOKEN_FALSE  */
  YYSYMBOL_SQL_TOKEN_FROM = 50,            /* SQL_TOKEN_FROM  */
  YYSYMBOL_SQL_TOKEN_FULL = 51,            /* SQL_TOKEN_FULL  */
  YYSYMBOL_SQL_TOKEN_GROUP = 52,           /* SQL_TOKEN_GROUP  */
  YYSYMBOL_SQL_TOKEN_HAVING = 53,          /* SQL_TOKEN_HAVING  */
  YYSYMBOL_SQL_TOKEN_IN = 54,              /* SQL_TOKEN_IN  */
  YYSYMBOL_SQL_TOKEN_INNER = 55,           /* SQL_TOKEN_INNER  */
  YYSYMBOL_SQL_TOKEN_INSERT = 56,          /* SQL_TOKEN_INSERT  */
  YYSYMBOL_SQL_TOKEN_INTO = 57,            /* SQL_TOKEN_INTO  */
  YYSYMBOL_SQL_TOKEN_IS = 58,              /* SQL_TOKEN_IS  */
  YYSYMBOL_SQL_TOKEN_INTERSECT = 59,       /* SQL_TOKEN_INTERSECT  */
  YYSYMBOL_SQL_TOKEN_JOIN = 60,            /* SQL_TOKEN_JOIN  */
  YYSYMBOL_SQL_TOKEN_LIKE = 61,            /* SQL_TOKEN_LIKE  */
  YYSYMBOL_SQL_TOKEN_LEFT = 62,            /* SQL_TOKEN_LEFT  */
  YYSYMBOL_SQL_TOKEN_RIGHT = 63,           /* SQL_TOKEN_RIGHT  */
  YYSYMBOL_SQL_TOKEN_MAX = 64,             /* SQL_TOKEN_MAX  */
  YYSYMBOL_SQL_TOKEN_MIN = 65,             /* SQL_TOKEN_MIN  */
  YYSYMBOL_SQL_TOKEN_NATURAL = 66,         /* SQL_TOKEN_NATURAL  */
  YYSYMBOL_SQL_TOKEN_NULL = 67,            /* SQL_TOKEN_NULL  */
  YYSYMBOL_SQL_TOKEN_ON = 68,              /* SQL_TOKEN_ON  */
  YYSYMBOL_SQL_TOKEN_ORDER = 69,           /* SQL_TOKEN_ORDER  */
  YYSYMBOL_SQL_TOKEN_OUTER = 70,           /* SQL_TOKEN_OUTER  */
  YYSYMBOL_SQL_TOKEN_ROLLBACK = 71,        /* SQL_TOKEN_ROLLBACK  */
  YYSYMBOL_SQL_TOKEN_IIF = 72,             /* SQL_TOKEN_IIF  */
  YYSYMBOL_SQL_TOKEN_SELECT = 73,          /* SQL_TOKEN_SELECT  */
  YYSYMBOL_SQL_TOKEN_SET = 74,             /* SQL_TOKEN_SET  */
  YYSYMBOL_SQL_TOKEN_SOME = 75,            /* SQL_TOKEN_SOME  */
  YYSYMBOL_SQL_TOKEN_SUM = 76,             /* SQL_TOKEN_SUM  */
  YYSYMBOL_SQL_TOKEN_TRUE = 77,            /* SQL_TOKEN_TRUE  */
  YYSYMBOL_SQL_TOKEN_UNION = 78,           /* SQL_TOKEN_UNION  */
  YYSYMBOL_SQL_TOKEN_UNIQUE = 79,          /* SQL_TOKEN_UNIQUE  */
  YYSYMBOL_SQL_TOKEN_UNKNOWN = 80,         /* SQL_TOKEN_UNKNOWN  */
  YYSYMBOL_SQL_TOKEN_UPDATE = 81,          /* SQL_TOKEN_UPDATE  */
  YYSYMBOL_SQL_TOKEN_USING = 82,           /* SQL_TOKEN_USING  */
  YYSYMBOL_SQL_TOKEN_VALUE = 83,           /* SQL_TOKEN_VALUE  */
  YYSYMBOL_SQL_TOKEN_VALUES = 84,          /* SQL_TOKEN_VALUES  */
  YYSYMBOL_SQL_TOKEN_WHERE = 85,           /* SQL_TOKEN_WHERE  */
  YYSYMBOL_SQL_TOKEN_DOLLAR = 86,          /* SQL_TOKEN_DOLLAR  */
  YYSYMBOL_SQL_BITWISE_NOT = 87,           /* SQL_BITWISE_NOT  */
  YYSYMBOL_SQL_TOKEN_CURRENT_DATE = 88,    /* SQL_TOKEN_CURRENT_DATE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIME = 89,    /* SQL_TOKEN_CURRENT_TIME  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIMESTAMP = 90, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_EVERY = 91,           /* SQL_TOKEN_EVERY  */
  YYSYMBOL_SQL_TOKEN_CASE = 92,            /* SQL_TOKEN_CASE  */
  YYSYMBOL_SQL_TOKEN_THEN = 93,            /* SQL_TOKEN_THEN  */
  YYSYMBOL_SQL_TOKEN_END = 94,             /* SQL_TOKEN_END  */
  YYSYMBOL_SQL_TOKEN_WHEN = 95,            /* SQL_TOKEN_WHEN  */
  YYSYMBOL_SQL_TOKEN_ELSE = 96,            /* SQL_TOKEN_ELSE  */
  YYSYMBOL_SQL_TOKEN_LIMIT = 97,           /* SQL_TOKEN_LIMIT  */
  YYSYMBOL_SQL_TOKEN_OFFSET = 98,          /* SQL_TOKEN_OFFSET  */
  YYSYMBOL_SQL_TOKEN_ONLY = 99,            /* SQL_TOKEN_ONLY  */
  YYSYMBOL_SQL_TOKEN_PRAGMA = 100,         /* SQL_TOKEN_PRAGMA  */
  YYSYMBOL_SQL_TOKEN_FOR = 101,            /* SQL_TOKEN_FOR  */
  YYSYMBOL_SQL_TOKEN_MATCH = 102,          /* SQL_TOKEN_MATCH  */
  YYSYMBOL_SQL_TOKEN_ECSQLOPTIONS = 103,   /* SQL_TOKEN_ECSQLOPTIONS  */
  YYSYMBOL_SQL_TOKEN_INTEGER = 104,        /* SQL_TOKEN_INTEGER  */
  YYSYMBOL_SQL_TOKEN_INT = 105,            /* SQL_TOKEN_INT  */
  YYSYMBOL_SQL_TOKEN_INT64 = 106,          /* SQL_TOKEN_INT64  */
  YYSYMBOL_SQL_TOKEN_LONG = 107,           /* SQL_TOKEN_LONG  */
  YYSYMBOL_SQL_TOKEN_BOOLEAN = 108,        /* SQL_TOKEN_BOOLEAN  */
  YYSYMBOL_SQL_TOKEN_DOUBLE = 109,         /* SQL_TOKEN_DOUBLE  */
  YYSYMBOL_SQL_TOKEN_REAL = 110,           /* SQL_TOKEN_REAL  */
  YYSYMBOL_SQL_TOKEN_FLOAT = 111,          /* SQL_TOKEN_FLOAT  */
  YYSYMBOL_SQL_TOKEN_STRING = 112,         /* SQL_TOKEN_STRING  */
  YYSYMBOL_SQL_TOKEN_VARCHAR = 113,        /* SQL_TOKEN_VARCHAR  */
  YYSYMBOL_SQL_TOKEN_BINARY = 114,         /* SQL_TOKEN_BINARY  */
  YYSYMBOL_SQL_TOKEN_BLOB = 115,           /* SQL_TOKEN_BLOB  */
  YYSYMBOL_SQL_TOKEN_DATE = 116,           /* SQL_TOKEN_DATE  */
  YYSYMBOL_SQL_TOKEN_TIME = 117,           /* SQL_TOKEN_TIME  */
  YYSYMBOL_SQL_TOKEN_TIMESTAMP = 118,      /* SQL_TOKEN_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_OR = 119,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 120,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_ARROW = 121,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 122,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 123,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 124,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 125,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 126,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 127,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 128,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 129,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 130,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 131,                /* SQL_EQUAL  */
  YYSYMBOL_132_ = 132,                     /* '+'  */
  YYSYMBOL_133_ = 133,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 134,               /* SQL_CONCAT  */
  YYSYMBOL_135_ = 135,                     /* '*'  */
  YYSYMBOL_136_ = 136,                     /* '/'  */
  YYSYMBOL_137_ = 137,                     /* '%'  */
  YYSYMBOL_138_ = 138,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 139,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 140,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 141,     /* sql_single_statement  */
  YYSYMBOL_sql = 142,                      /* sql  */
  YYSYMBOL_pragma = 143,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 144,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 145,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 146,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 147,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 148,             /* pragma_value  */
  YYSYMBOL_pragma_path = 149,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 150,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 151,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 152,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 153,           /* cte_block_list  */
  YYSYMBOL_cte = 154,                      /* cte  */
  YYSYMBOL_column_commalist = 155,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 156,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 157,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 158, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 159,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 160,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 161,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 162,             /* opt_asc_desc  */
  YYSYMBOL_sql_not = 163,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 164,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 165,         /* select_statement  */
  YYSYMBOL_union_op = 166,                 /* union_op  */
  YYSYMBOL_commit_statement = 167,         /* commit_statement  */
  YYSYMBOL_delete_statement_searched = 168, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 169,         /* insert_statement  */
  YYSYMBOL_values_or_query_spec = 170,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 171, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 172,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 173, /* row_value_constructor_elem  */
  YYSYMBOL_rollback_statement = 174,       /* rollback_statement  */
  YYSYMBOL_select_statement_into = 175,    /* select_statement_into  */
  YYSYMBOL_opt_all_distinct = 176,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 177,     /* assignment_commalist  */
  YYSYMBOL_assignment = 178,               /* assignment  */
  YYSYMBOL_update_source = 179,            /* update_source  */
  YYSYMBOL_update_statement_searched = 180, /* update_statement_searched  */
  YYSYMBOL_target_commalist = 181,         /* target_commalist  */
  YYSYMBOL_target = 182,                   /* target  */
  YYSYMBOL_opt_where_clause = 183,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 184,  /* single_select_statement  */
  YYSYMBOL_selection = 185,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 186,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 187,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 188,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 189,                /* table_exp  */
  YYSYMBOL_from_clause = 190,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 191,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 192,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 193, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 194, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 195,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 196, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 197,                /* table_ref  */
  YYSYMBOL_where_clause = 198,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 199,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 200,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 201,              /* truth_value  */
  YYSYMBOL_boolean_primary = 202,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 203,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 204,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 205,             /* boolean_term  */
  YYSYMBOL_search_condition = 206,         /* search_condition  */
  YYSYMBOL_type_predicate = 207,           /* type_predicate  */
  YYSYMBOL_type_list = 208,                /* type_list  */
  YYSYMBOL_type_list_item = 209,           /* type_list_item  */
  YYSYMBOL_predicate = 210,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 211, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 212,     /* comparison_predicate  */
  YYSYMBOL_comparison = 213,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 214, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 215,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 216, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 217, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 218,           /* like_predicate  */
  YYSYMBOL_opt_escape = 219,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 220,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 221,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 222,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 223,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 224,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 225, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 226,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 227,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 228, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 229,             /* any_all_some  */
  YYSYMBOL_existence_test = 230,           /* existence_test  */
  YYSYMBOL_unique_test = 231,              /* unique_test  */
  YYSYMBOL_subquery = 232,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 233,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 234,           /* select_sublist  */
  YYSYMBOL_parameter_ref = 235,            /* parameter_ref  */
  YYSYMBOL_literal = 236,                  /* literal  */
  YYSYMBOL_as_clause = 237,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 238,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 239,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 240,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 241,                 /* fct_spec  */
  YYSYMBOL_function_name = 242,            /* function_name  */
  YYSYMBOL_general_set_fct = 243,          /* general_set_fct  */
  YYSYMBOL_set_fct_type = 244,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 245,          /* outer_join_type  */
  YYSYMBOL_join_condition = 246,           /* join_condition  */
  YYSYMBOL_join_spec = 247,                /* join_spec  */
  YYSYMBOL_join_type = 248,                /* join_type  */
  YYSYMBOL_cross_union = 249,              /* cross_union  */
  YYSYMBOL_qualified_join = 250,           /* qualified_join  */
  YYSYMBOL_ecrelationship_join = 251,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 252, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 253,             /* joined_table  */
  YYSYMBOL_named_columns_join = 254,       /* named_columns_join  */
  YYSYMBOL_all = 255,                      /* all  */
  YYSYMBOL_scalar_subquery = 256,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 257,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 258, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 259,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 260,        /* cast_target_array  */
  YYSYMBOL_cast_target = 261,              /* cast_target  */
  YYSYMBOL_cast_spec = 262,                /* cast_spec  */
  YYSYMBOL_opt_extract_value = 263,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 264,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 265,              /* num_primary  */
  YYSYMBOL_factor = 266,                   /* factor  */
  YYSYMBOL_term = 267,                     /* term  */
  YYSYMBOL_term_add_sub = 268,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 269,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 270,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 271,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 272,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 273,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 274,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 275,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 276,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 277,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 278,                /* value_exp  */
  YYSYMBOL_string_value_exp = 279,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 280,           /* char_value_exp  */
  YYSYMBOL_concatenation = 281,            /* concatenation  */
  YYSYMBOL_char_primary = 282,             /* char_primary  */
  YYSYMBOL_char_factor = 283,              /* char_factor  */
  YYSYMBOL_derived_column = 284,           /* derived_column  */
  YYSYMBOL_table_node = 285,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 286, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 287,     /* qualified_class_name  */
  YYSYMBOL_class_name = 288,               /* class_name  */
  YYSYMBOL_table_node_with_opt_member_func_call = 289, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 290,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 291,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 292, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 293,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 294,            /* property_path  */
  YYSYMBOL_property_path_entry = 295,      /* property_path_entry  */
  YYSYMBOL_column_ref = 296,               /* column_ref  */
  YYSYMBOL_column = 297,                   /* column  */
  YYSYMBOL_case_expression = 298,          /* case_expression  */
  YYSYMBOL_case_specification = 299,       /* case_specification  */
  YYSYMBOL_simple_case = 300,              /* simple_case  */
  YYSYMBOL_searched_case = 301,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 302,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 303,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 304,        /* when_operand_list  */
  YYSYMBOL_when_operand = 305,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 306, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 307,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 308,              /* else_clause  */
  YYSYMBOL_result = 309,                   /* result  */
  YYSYMBOL_result_expression = 310,        /* result_expression  */
  YYSYMBOL_case_operand = 311,             /* case_operand  */
  YYSYMBOL_parameter = 312,                /* parameter  */
  YYSYMBOL_range_variable = 313,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 314,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 315,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 316,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 317,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 318          /* ecsqloptionvalue  */
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
#define YYLAST   1895

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  140
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  179
/* YYNRULES -- Number of rules.  */
#define YYNRULES  390
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  601

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   372


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
       2,     2,     2,     2,     2,     2,     2,   137,     2,     2,
       3,     4,   135,   132,     5,   133,    13,   136,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   138,     2,     8,     2,     2,     2,     2,     2,     2,
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
     131,   134,   139
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   227,   227,   228,   229,   230,   234,   240,   251,   254,
     262,   265,   266,   270,   278,   286,   287,   288,   289,   290,
     291,   292,   296,   301,   306,   318,   321,   325,   330,   338,
     352,   358,   366,   378,   383,   391,   396,   406,   407,   415,
     416,   427,   428,   438,   443,   451,   459,   468,   469,   470,
     474,   475,   481,   482,   483,   484,   485,   486,   487,   488,
     492,   497,   508,   509,   510,   513,   520,   532,   541,   551,
     556,   564,   567,   572,   581,   592,   593,   594,   598,   601,
     607,   614,   617,   629,   632,   638,   642,   643,   651,   659,
     663,   668,   671,   672,   675,   676,   684,   693,   694,   708,
     718,   721,   727,   728,   732,   735,   745,   746,   753,   754,
     759,   767,   768,   775,   783,   791,   799,   803,   812,   813,
     823,   824,   832,   833,   834,   835,   836,   839,   840,   841,
     842,   843,   844,   845,   855,   856,   866,   867,   875,   876,
     885,   886,   896,   905,   910,   918,   927,   928,   929,   930,
     931,   932,   933,   934,   935,   941,   948,   955,   980,   981,
     982,   983,   984,   985,   986,   994,  1004,  1012,  1022,  1032,
    1038,  1044,  1066,  1091,  1092,  1099,  1108,  1114,  1130,  1134,
    1142,  1151,  1157,  1173,  1182,  1191,  1200,  1210,  1211,  1212,
    1216,  1224,  1230,  1241,  1246,  1253,  1257,  1261,  1262,  1263,
    1264,  1265,  1267,  1279,  1291,  1303,  1319,  1320,  1326,  1330,
    1331,  1334,  1335,  1336,  1337,  1340,  1352,  1353,  1354,  1361,
    1370,  1384,  1389,  1405,  1421,  1430,  1438,  1450,  1451,  1452,
    1453,  1454,  1458,  1463,  1468,  1475,  1483,  1484,  1487,  1488,
    1493,  1494,  1502,  1514,  1524,  1533,  1538,  1552,  1553,  1554,
    1557,  1558,  1561,  1573,  1574,  1578,  1581,  1585,  1586,  1587,
    1588,  1589,  1590,  1591,  1592,  1593,  1594,  1595,  1596,  1597,
    1598,  1599,  1600,  1604,  1609,  1619,  1628,  1629,  1633,  1646,
    1647,  1655,  1656,  1657,  1658,  1659,  1660,  1667,  1671,  1675,
    1676,  1682,  1688,  1697,  1698,  1705,  1712,  1722,  1723,  1730,
    1740,  1741,  1748,  1755,  1762,  1772,  1779,  1784,  1789,  1794,
    1800,  1806,  1815,  1822,  1830,  1838,  1841,  1847,  1851,  1856,
    1864,  1865,  1866,  1870,  1874,  1875,  1878,  1885,  1895,  1898,
    1902,  1912,  1917,  1924,  1933,  1941,  1950,  1958,  1966,  1971,
    1976,  1984,  1993,  1994,  2004,  2005,  2013,  2018,  2036,  2042,
    2047,  2055,  2067,  2071,  2075,  2076,  2080,  2091,  2101,  2106,
    2113,  2123,  2126,  2131,  2132,  2133,  2134,  2135,  2136,  2139,
    2144,  2151,  2161,  2162,  2170,  2174,  2178,  2182,  2188,  2196,
    2199,  2209,  2210,  2214,  2223,  2228,  2236,  2242,  2252,  2253,
    2254
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
  "cast_target_array", "cast_target", "cast_spec", "opt_extract_value",
  "value_exp_primary", "num_primary", "factor", "term", "term_add_sub",
  "num_value_exp", "datetime_primary", "datetime_value_fct",
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
  "ecsqloptions_list", "ecsqloption", "ecsqloptionvalue", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-492)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-369)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     385,    51,  -492,    63,    60,  -492,   169,    35,   175,   111,
     181,   168,   196,  -492,  -492,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,  -492,    33,  -492,   159,    35,   187,  -492,  -492,
    1408,   212,    16,    29,   250,    42,  -492,  -492,  -492,  -492,
    1485,    32,  -492,  -492,  -492,  -492,  -492,  -492,   258,   276,
    -492,    93,   345,   193,   296,  -492,  -492,  1177,   290,  -492,
    -492,  -492,  -492,  -492,    84,  -492,  -492,   315,   322,  -492,
     327,   334,  -492,   339,  -492,  -492,  -492,  -492,  1562,  -492,
    -492,  -492,  -492,  1254,  -492,  -492,  1485,  1485,  1485,  1562,
    1562,   132,   167,  -492,   342,  -492,    45,  -492,  -492,  -492,
    -492,   346,  -492,   351,  -492,  -492,  -492,  -492,  -492,   103,
      90,   199,  -492,  -492,  -492,  -492,  -492,    12,  -492,   228,
    -492,  -492,  -492,  -492,    47,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,  -492,  -492,    54,  -492,   250,   231,   361,   231,
     253,  -492,   317,  -492,  -492,  -492,  -492,   153,    13,   313,
     328,  -492,   277,  -492,  -492,   261,   109,   109,   298,  -492,
    -492,  -492,    54,   373,   159,   169,  -492,   749,   299,  -492,
     382,   394,    13,   343,   419,    18,  -492,  -492,  -492,  1485,
      19,   169,   169,   749,  -492,   749,  -492,   206,  -492,   337,
     261,  -492,  -492,  -492,  -492,  -492,    35,    82,  -492,   353,
    1485,  -492,  -492,  -492,  -492,  1100,   169,   395,   395,   395,
     395,   395,   395,   395,   395,   395,  -492,   412,  1485,  -492,
    -492,   330,    13,    13,  -492,   231,  -492,   421,   296,  1485,
    -492,   422,  -492,   250,   250,    35,   383,   423,    59,  -492,
     316,  -492,    35,  -492,  1485,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,   445,  -492,   426,  -492,  -492,  -492,   304,  -492,
    1408,   515,   632,   448,   429,   448,  -492,  -492,  -492,  -492,
    -492,  -492,   211,    66,   396,  -492,  -492,   333,   336,  -492,
    -492,  1485,  -492,  -492,  -492,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,  -492,  -492,  1596,  1612,  1727,  1582,  1743,   433,
    -492,  -492,  -492,  -492,   262,  -492,  -492,   307,  -492,  -492,
    -492,  -492,   430,   261,   459,  1485,  1485,  1485,    28,   -18,
    1485,  -492,   371,   749,   379,  -492,   337,   471,   439,    36,
    -492,  -492,  -492,   425,  -492,  -492,  1485,  -492,   323,   261,
    -492,  -492,  1485,  -492,  -492,  -492,   103,   103,    90,    90,
      90,    90,  -492,  -492,  -492,  -492,   451,  -492,  -492,  -492,
     335,   476,  -492,  -492,   439,    35,    13,   299,  1485,   504,
    -492,  -492,  -492,   263,   449,   460,   438,    21,    22,  -492,
    -492,  -492,   416,  -492,   489,  1485,    49,  1331,  -492,  -492,
    -492,  -492,  -492,  -492,  -492,   429,   749,   749,  -492,   362,
     433,  -492,   382,  -492,    13,  1777,  -492,   492,   341,   348,
    1485,  1485,  -492,  -492,   173,    50,  -492,  1485,  -492,   493,
     498,   501,    69,  -492,   403,  -492,    35,    82,  -492,   472,
     455,   505,  -492,  1485,   506,  -492,   412,  -492,   439,  -492,
    -492,  -492,   261,   749,    88,  -492,  -492,  -492,   487,   488,
     511,  -492,  -492,  -492,  1177,  -492,  -492,   257,    25,  1485,
    1730,  -492,  -492,  -492,  -492,   448,   249,  -492,   333,   264,
    -492,  -492,   503,  -492,  -492,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,  -492,  -492,  -492,  -492,  -492,  -492,  -492,   495,
    -492,   513,  -492,  -492,  -492,   517,  -492,    71,   983,  1485,
    -492,   439,  -492,  1485,   749,   456,  -492,  -492,  -492,   354,
    -492,   336,   412,   318,  -492,  -492,    54,   368,   261,  1485,
    -492,  -492,   406,  -492,  -492,  -492,    39,  -492,  -492,  -492,
    -492,  -492,  -492,  -492,  -492,    45,  -492,   508,  -492,  -492,
    1485,   195,  -492,  1485,  -492,  -492,  -492,  -492,  -492,   524,
     336,   509,   444,  -492,   412,   370,  -492,  -492,  -492,   542,
    -492,  1485,   413,  1485,  -492,   187,   381,  -492,  -492,   545,
    1485,  -492,   866,   395,   299,  -492,  -492,  -492,  -492,   261,
    -492,  -492,  -492,    39,  -492,   547,  -492,   273,   186,   172,
    -492,  -492,   866,  -492,  -492,  -492,  -492,   395,  -492,  -492,
     199
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    25,    65,     0,     0,    73,    75,   106,     0,     0,
       0,     2,     4,    59,     6,    58,    52,    53,    54,    89,
      55,    56,    57,    60,    26,     0,   106,     0,    76,    77,
       0,   107,     0,   111,     0,   238,   245,   251,   250,   116,
       0,    10,     1,     3,     5,    64,    62,    63,   253,     0,
      31,     0,    86,     0,    39,   332,   331,     0,     0,   378,
     201,   198,   199,   200,   344,   230,   227,     0,     0,   213,
       0,     0,   212,     0,   231,   228,   214,   350,     0,   306,
     308,   307,   229,     0,   197,   328,     0,     0,     0,     0,
       0,   349,    97,   255,    91,   193,   210,   281,   209,   217,
     282,     0,   216,     0,   284,   287,   288,   289,   293,   297,
     300,   320,   312,   305,   313,   314,   322,   206,   321,   323,
     325,   329,   324,   195,   279,   346,   283,   285,   353,   354,
     355,   211,   110,   109,     0,   112,     0,   379,   342,   104,
     337,   338,     0,   234,   239,   232,   233,   238,     0,   240,
       0,   349,     0,    69,    71,    72,     0,     0,     8,    11,
      12,   254,     0,     0,     0,    75,    32,    50,   381,    87,
       0,     0,     0,     0,     0,     0,   377,   345,   348,     0,
      75,    75,    75,    50,   292,    50,   376,   372,   369,     0,
       0,   309,   311,   310,   291,   290,   106,     0,    88,    86,
       0,   204,   205,   203,   202,     0,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   352,     0,     0,   330,
     208,     0,     0,     0,   351,   104,   103,     0,    39,     0,
     341,     0,   114,     0,     0,   106,     0,   344,    86,    78,
       0,   241,   106,    68,     0,    15,    16,    17,    20,    21,
      19,    18,     0,    13,     0,     7,    61,    28,     0,    30,
       0,    50,    50,     0,    50,     0,   162,   163,   159,   158,
     161,   160,     0,    50,   134,   136,   138,   140,   117,   127,
     146,     0,   147,   171,   172,   153,   177,   151,   182,   152,
     148,   154,   149,   150,   128,   129,   131,   132,   130,     0,
      66,   382,   336,   335,   336,   333,   334,     0,    36,    67,
     192,   286,     0,   256,     0,     0,     0,     0,     0,     0,
       0,   370,     0,    50,   372,   358,     0,    99,   100,    97,
      83,    85,   196,   118,   194,   218,     0,   318,     0,   375,
     317,   374,     0,   294,   295,   296,   298,   299,   303,   304,
     301,   302,   207,   327,   326,   347,   280,   113,   380,   115,
       0,    37,   340,   339,   242,   106,     0,   381,     0,   238,
      70,    14,    22,     9,     0,     0,    97,     0,    72,    51,
     137,   190,   164,   191,     0,     0,     0,     0,   166,   169,
     170,   176,   181,   184,   185,    50,    50,    50,   157,   386,
     383,   385,     0,    40,     0,     0,   225,     0,     0,     0,
       0,     0,   373,   357,     0,    71,   364,     0,   365,   171,
     177,   182,     0,   361,     0,   359,   106,     0,    74,     0,
     120,     0,   219,     0,     0,   343,     0,   105,   243,    79,
      82,    80,    81,    50,     0,   236,   244,   237,     0,     0,
       0,    27,   133,   175,     0,   180,   178,   173,   173,     0,
       0,   188,   187,   189,   156,     0,     0,   139,   141,     0,
     384,    35,   272,   263,   262,   264,   265,   259,   260,   266,
     261,   267,   271,   257,   258,   268,   269,   270,   273,   276,
     277,     0,   226,   222,   223,     0,   371,   157,    50,     0,
     356,   101,    84,     0,    50,    41,   220,   319,   224,     0,
      34,   235,     0,   247,    24,    23,     0,     0,   315,     0,
     168,   167,     0,   221,   186,   183,   106,   123,   125,   122,
     124,   135,   126,   389,   390,   388,   387,     0,   275,   278,
       0,     0,   363,     0,   367,   368,   366,   362,   360,   119,
     121,     0,    92,    38,     0,     0,   248,   249,   246,     0,
     179,     0,   174,     0,   107,     0,     0,   143,   274,     0,
       0,   155,    50,     0,   381,    93,    33,   252,    29,   316,
     165,   145,   142,   106,   215,    42,    43,    71,    47,    94,
      98,   144,    50,    48,    49,    46,    45,     0,    96,    44,
      95
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -492,  -492,  -492,  -492,  -492,  -492,  -492,  -492,   393,  -492,
    -492,  -492,   390,  -492,  -492,    44,  -492,  -492,   329,  -492,
    -492,   -34,   -27,  -250,  -492,     3,  -492,  -492,  -492,  -492,
     387,  -492,   -39,   -77,  -492,  -492,    20,  -492,   202,  -492,
    -492,  -492,   138,  -138,  -492,   309,  -492,  -492,  -492,   242,
    -492,  -492,   437,   350,  -492,  -449,   543,   -22,  -492,  -492,
    -492,   108,  -492,   319,   182,   188,  -154,  -492,  -492,     0,
    -478,  -492,  -492,  -257,   311,  -492,  -255,   320,  -492,   131,
    -254,  -492,  -492,  -253,  -492,  -492,  -492,  -492,  -492,  -492,
    -492,  -492,   -28,  -492,   397,  -492,   126,  -492,  -139,  -492,
    -492,  -143,  -492,  -492,  -492,  -492,  -492,  -492,   453,  -492,
    -492,  -492,  -492,  -492,  -492,  -492,  -136,  -492,  -492,  -492,
    -492,  -492,  -492,  -492,   213,     6,    83,   224,   121,  -491,
    -492,  -492,  -492,  -492,  -492,   105,  -264,  -144,   -30,   -79,
    -492,  -492,  -492,   375,  -492,    46,  -492,   441,   440,  -125,
    -492,   176,  -492,  -492,   386,   391,  -133,  -115,  -492,  -492,
    -492,  -492,  -492,   288,  -492,   117,   427,  -166,   293,  -290,
    -492,  -492,  -180,  -492,  -355,  -492,  -492,   218,  -492
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,   255,   158,   159,   160,   252,   373,
      25,   258,    50,    51,    13,   509,   307,   437,   173,   552,
     585,   586,   595,   272,    14,   174,    48,    16,    17,    18,
      19,   152,   273,   154,    20,    21,    30,   238,   239,   441,
      22,   329,   330,   168,    23,    92,   574,   598,   575,   198,
     199,   327,   231,   232,    32,    33,    34,    35,   169,   430,
     505,   531,   274,   275,   276,   277,   319,   532,   566,   567,
     279,   416,   280,   281,   418,   282,   283,   284,   285,   520,
     286,   287,   455,   288,   289,   393,   290,   291,   394,   465,
     292,   293,    93,    94,    95,   331,    96,   219,    97,    98,
      99,   100,   101,   102,   103,   149,   445,   446,   150,    36,
      37,    38,   558,    39,   447,   162,   104,   312,   488,   489,
     490,   491,   105,   224,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   517,   337,   338,   339,   118,
     119,   120,   121,   122,   123,    54,    55,    56,   306,   139,
     140,   141,   230,   178,   124,   125,   126,   510,   127,   128,
     129,   130,   324,   325,   422,   423,   187,   188,   322,   340,
     341,   189,   131,   228,   300,   301,   400,   401,   536
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     117,   153,   220,    15,    52,   137,   186,   191,   192,   193,
     155,   225,   440,   278,   382,   240,   387,   332,   389,   391,
     392,   321,   311,   386,   295,   452,   311,   175,   294,   318,
     412,   296,   134,   410,   298,   156,   216,   237,  -108,   308,
     295,   427,   295,   217,   294,   132,   294,   296,    28,   296,
     298,   407,   298,   155,   166,  -363,   190,   190,   190,  -108,
     222,   333,    29,  -108,   366,   201,   417,   202,   419,   420,
     421,   519,   431,   414,   498,   411,  -155,   565,   434,    24,
      45,   142,   589,   459,   184,   360,   196,  -221,    58,   379,
      59,   512,    46,   143,   588,   194,   195,   144,   164,    77,
     367,   397,   352,   384,   145,   146,   600,   377,   147,   177,
     385,    47,   138,    26,   588,   133,   148,    27,   295,   295,
     495,   496,   294,   294,   264,   296,   296,   165,   298,   298,
     245,   246,   -90,   247,   565,    41,   -90,   297,     8,   -90,
     397,  -132,  -132,  -363,   167,   466,   218,   397,   151,   313,
     203,   460,   218,   297,   314,   297,   218,   204,   248,  -321,
     321,   135,   499,   157,  -155,   256,   165,    31,   223,   507,
     117,   564,   408,   409,   328,    43,   249,     8,    40,   -90,
     295,    42,   -90,    49,   294,   260,   250,   296,   353,   -90,
     298,   -90,   266,   267,   268,   269,   270,   271,    28,   170,
     315,   316,   317,    44,   143,   370,   171,   459,   144,   548,
     -90,    53,    29,   364,   155,   145,   146,   196,   593,   590,
     369,   251,   210,   211,   197,   336,   342,   384,   594,   459,
     117,   378,   297,   240,   385,   381,  -112,   383,   207,   208,
     209,   543,   398,   544,   545,   546,   415,   332,   541,   384,
     569,   155,   526,   295,   295,  -102,   570,   294,   294,   233,
     296,   296,   226,   298,   298,   384,   234,   526,   170,   448,
     597,   471,   385,   -47,   138,   402,   449,   -47,   -47,   163,
     -47,   243,   244,    60,    61,    62,    63,   161,   533,   511,
     343,   344,   345,   297,   212,   213,   214,   215,   527,   172,
     295,   185,   320,   519,   294,   593,   458,   296,   374,   375,
     298,   403,   404,   527,   176,   594,   528,   524,   179,   513,
     -47,   212,   213,   214,   215,   180,   529,   432,   433,   530,
     181,   528,   -47,   348,   349,   350,   351,   182,   442,   435,
     433,   529,   183,   438,   530,   493,   433,   200,   464,   205,
     550,   -47,   494,   433,   206,   190,   456,   155,   553,   554,
     221,   295,   556,   557,   229,   294,   297,   297,   296,    84,
     -47,   298,   560,   561,   577,   554,   -47,   235,   497,  -288,
    -288,  -288,  -288,   241,   142,   582,   583,   155,   242,  -288,
    -288,  -288,  -288,  -288,  -288,   218,   143,   257,    57,   254,
     144,    58,   299,    59,   501,  -238,   302,   145,   146,   362,
     363,   147,     1,   297,    60,    61,    62,    63,   304,    64,
     522,   542,     2,   310,   518,    65,     3,     8,    66,   155,
     167,    67,   323,    68,   346,   347,   216,   525,   167,   576,
     562,     4,    85,   365,    69,   358,   361,   368,   177,   371,
     372,   134,   379,   396,   395,   397,     5,   399,     6,    70,
      71,   405,    72,   406,   222,   413,     7,    73,   155,     8,
      74,    75,    76,   518,   297,   320,   426,   429,   142,   436,
     450,    77,    78,   453,   451,     9,    82,    83,   196,   190,
     143,   458,   454,   469,   144,   587,   492,   500,  -367,  -238,
      84,   145,   146,  -368,   571,   147,  -366,   503,   504,   506,
     508,   514,   515,   155,   516,   587,   537,   539,   261,   559,
     538,    58,   540,    59,   580,   551,   563,    89,    90,   561,
     151,   579,   568,   155,    60,    61,    62,    63,   262,    64,
     190,   573,   155,   142,   572,    65,   578,  -321,    66,   584,
     253,    67,   592,    68,   259,   143,   555,   359,   599,   144,
     309,   596,   155,   263,    69,   502,   145,   146,   439,   376,
     147,   428,   443,   264,   227,   357,   136,   534,   467,    70,
      71,   380,    72,   591,   388,   468,   444,    73,   165,   521,
      74,    75,    76,   390,   265,   535,   354,   334,   457,     8,
     236,    77,    78,    79,    80,    81,    82,    83,   549,   356,
     303,   581,   305,   355,   425,   547,   326,   424,   470,     0,
      84,     0,     0,     0,     0,     0,     0,    85,     0,     0,
       0,    86,    87,    88,     0,   261,     0,     0,    58,     0,
      59,   266,   267,   268,   269,   270,   271,    89,    90,     0,
     151,    60,    61,    62,    63,   379,    64,     0,     0,     0,
       0,     0,    65,     0,     0,    66,   -51,     0,    67,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,    69,     0,     0,     0,     0,     0,     0,     0,     0,
     264,     0,     0,     0,     0,     0,    70,    71,     0,    72,
       0,     0,     0,     0,    73,     0,     0,    74,    75,    76,
       0,   265,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,    85,     0,     0,     0,    86,    87,
      88,     0,   261,     0,     0,    58,     0,    59,   266,   267,
     268,   269,   270,   271,    89,    90,     0,   151,    60,    61,
      62,    63,   262,    64,     0,     0,     0,     0,     0,    65,
       0,     0,    66,     0,     0,    67,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   263,    69,     0,
       0,     0,     0,     0,     0,     0,     0,   264,     0,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
       0,    73,     0,     0,    74,    75,    76,     0,   265,     0,
       0,     0,     0,     0,     0,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,    85,     0,     0,     0,    86,    87,    88,     0,    57,
       0,     0,    58,     0,    59,   266,   267,   268,   269,   270,
     271,    89,    90,     0,   151,    60,    61,    62,    63,   379,
      64,     0,     0,     0,     0,     0,    65,     0,     0,    66,
       0,     0,    67,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,    69,     0,     0,     0,     0,
       0,     0,     0,     0,   264,     0,     0,     0,     0,     0,
      70,    71,     0,    72,     0,     0,     0,     0,    73,     0,
       0,    74,    75,    76,     0,   265,     0,     0,     0,     0,
       0,     0,    77,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,    86,    87,    88,     0,    57,     0,     0,    58,
       0,    59,   266,   267,   268,   269,   270,   271,    89,    90,
       0,   151,    60,    61,    62,    63,   379,    64,     0,     0,
       0,     0,     0,    65,     0,     0,    66,     0,     0,    67,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,   264,     0,     0,     0,     0,     0,    70,    71,     0,
      72,     0,     0,     0,     0,    73,     0,     0,    74,    75,
      76,     0,     0,     0,     0,     0,     0,     0,     0,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,    85,     0,     0,     0,    86,
      87,    88,     0,    57,   335,     0,    58,     0,    59,   266,
     267,   268,   269,   270,   271,    89,    90,     0,   151,    60,
      61,    62,    63,     0,    64,     0,     0,     0,     0,    28,
      65,     0,     0,    66,     0,     0,    67,     0,    68,     0,
       0,     0,     0,    29,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,     0,     0,
       0,     0,    73,     0,     0,    74,    75,    76,     0,     0,
      57,     0,     0,    58,     0,    59,    77,    78,    79,    80,
      81,    82,    83,     0,     0,     0,    60,    61,    62,    63,
       0,    64,     0,     0,     0,    84,     0,    65,     0,     0,
      66,     0,    85,    67,     0,    68,    86,    87,    88,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,    89,    90,     0,   151,     0,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
     165,     0,    74,    75,    76,     0,     0,    57,     0,     0,
      58,     8,    59,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,    60,    61,    62,    63,     0,    64,     0,
       0,     0,    84,     0,    65,     0,     0,    66,     0,    85,
      67,     0,    68,    86,    87,    88,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,     0,    89,
      90,     0,   151,     0,     0,     0,     0,     0,    70,    71,
       0,    72,     0,     0,     0,     0,    73,     0,     0,    74,
      75,    76,     0,     0,    57,     0,     0,    58,     0,    59,
      77,    78,    79,    80,    81,    82,    83,     0,     0,   185,
      60,    61,    62,    63,     0,    64,     0,     0,     0,    84,
     461,   462,     0,     0,    66,     0,    85,    67,     0,    68,
      86,    87,    88,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,    89,    90,     0,   151,
       0,     0,     0,     0,     0,    70,    71,     0,    72,     0,
       0,     0,     0,    73,     0,     0,   463,    75,    76,     0,
       0,    57,     0,     0,    58,     0,    59,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    60,    61,    62,
      63,     0,    64,     0,     0,     0,    84,     0,    65,     0,
       0,    66,     0,    85,    67,     0,    68,    86,    87,    88,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,     0,    89,    90,     0,   151,     0,     0,     0,
       0,     0,    70,    71,     0,    72,     0,     0,     0,     0,
      73,     0,     0,    74,    75,    76,     0,     0,    57,     0,
       0,    58,     0,    59,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,    60,    61,    62,    63,     0,    64,
       0,     0,     0,    84,     0,    65,     0,     0,    66,     0,
      85,    67,     0,    68,    86,    87,    88,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
      89,    90,     0,    91,     0,     0,     0,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      74,    75,    76,     0,     0,    57,     0,     0,    58,     0,
      59,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,    60,    61,    62,    63,     0,    64,   -72,     0,     0,
      84,     0,    65,     0,     0,    66,     0,    85,    67,     0,
      68,    86,    87,    88,     0,   -72,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   -72,    89,    90,  -281,
     151,     0,     0,     0,     0,     0,    70,    71,     0,    72,
    -281,     0,     0,     0,    73,  -282,   -72,    74,    75,    76,
     -72,     0,     0,   -72,     0,     0,  -282,     0,    77,     0,
    -281,     0,     0,    82,    83,     0,     0,  -281,     0,     0,
       0,     0,     0,     0,     0,     0,  -282,    84,     0,     0,
       0,     0,     0,  -282,     0,   -72,     0,     0,     0,     0,
       0,     0,     0,     0,   -72,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   151,  -281,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   -72,   -72,
     -72,   -72,   -72,   -72,  -282,     0,   218,     0,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -284,     0,     0,     0,   523,     0,     0,     0,     0,     0,
      65,  -284,     0,    66,     0,     0,  -283,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,  -283,     0,     0,
       0,  -284,     0,     0,     0,     0,     0,     0,  -284,     0,
       0,     0,     0,     0,    70,    71,     0,  -283,     0,     0,
       0,   472,    73,     0,  -283,    74,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,     0,     0,     0,  -284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -283,     0,     0,     0,  -284,
    -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,
    -284,  -284,  -284,  -284,  -284,  -283,  -283,  -283,  -283,  -283,
    -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,
    -283,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487
};

static const yytype_int16 yycheck[] =
{
      30,    40,   117,     0,    26,    33,    83,    86,    87,    88,
      40,   136,   367,   167,   264,   148,   273,   197,   273,   273,
     273,   187,     4,   273,   167,     4,     4,    57,   167,   183,
     320,   167,     3,     5,   167,     3,    24,    24,     3,   172,
     183,     5,   185,    31,   183,    29,   185,   183,    29,   185,
     183,   315,   185,    83,    51,     5,    86,    87,    88,    24,
      13,   199,    43,    24,     5,    20,   323,    22,   323,   323,
     323,    46,   336,   323,     5,    93,     5,   526,   342,    28,
      47,    39,   573,    34,    78,   229,    50,     3,     6,    23,
       8,     3,    59,    51,   572,    89,    90,    55,     5,    86,
     238,   119,   217,    54,    62,    63,   597,   261,    66,    25,
      61,    78,    24,    50,   592,    99,    74,    57,   261,   262,
     410,   411,   261,   262,    58,   261,   262,    73,   261,   262,
      21,    22,     0,    24,   583,    24,     4,   167,    84,     7,
     119,   119,   120,    93,    85,   395,   134,   119,   135,   179,
     105,   102,   134,   183,   135,   185,   134,   112,    49,   134,
     326,   132,    93,   131,    93,   162,    73,   132,   121,   433,
     200,   132,   316,   317,   196,     7,    67,    84,     3,    47,
     323,     0,    50,    24,   323,   165,    77,   323,   218,    57,
     323,    59,   126,   127,   128,   129,   130,   131,    29,     6,
     180,   181,   182,     7,    51,   244,    13,    34,    55,   499,
      78,    24,    43,   235,   244,    62,    63,    50,    32,   574,
     242,   112,   132,   133,    57,   205,   206,    54,    42,    34,
     260,   261,   262,   366,    61,   263,    24,   265,   135,   136,
     137,   498,   281,   498,   498,   498,   323,   427,   498,    54,
     540,   281,     3,   396,   397,    24,    61,   396,   397,     6,
     396,   397,    31,   396,   397,    54,    13,     3,     6,     6,
      98,   404,    61,     0,    24,    13,    13,     4,     5,     3,
       7,     4,     5,    19,    20,    21,    22,    29,    24,   443,
     207,   208,   209,   323,   122,   123,   124,   125,    49,     3,
     443,    95,    96,    46,   443,    32,   385,   443,     4,     5,
     443,     4,     5,    49,    24,    42,    67,   460,     3,   444,
      47,   122,   123,   124,   125,     3,    77,     4,     5,    80,
       3,    67,    59,   212,   213,   214,   215,     3,   368,     4,
       5,    77,     3,   365,    80,     4,     5,     5,   387,     3,
     504,    78,     4,     5,     3,   385,   384,   387,     4,     5,
     132,   504,    44,    45,     3,   504,   396,   397,   504,   105,
      97,   504,     4,     5,     4,     5,   103,    60,   417,   122,
     123,   124,   125,    70,    39,     4,     5,   417,    60,   132,
     133,   134,   135,   136,   137,   134,    51,    24,     3,   101,
      55,     6,   103,     8,   426,    60,    24,    62,    63,   233,
     234,    66,    27,   443,    19,    20,    21,    22,    24,    24,
     459,   498,    37,     4,   454,    30,    41,    84,    33,   459,
      85,    36,    95,    38,   210,   211,    24,   465,    85,   554,
     519,    56,   112,    60,    49,    24,    24,   131,    25,     4,
      24,     3,    23,   120,    58,   119,    71,    24,    73,    64,
      65,    31,    67,     4,    13,    94,    81,    72,   498,    84,
      75,    76,    77,   503,   504,    96,     5,    52,    39,     3,
      31,    86,    87,    67,    24,   100,    91,    92,    50,   519,
      51,   570,     3,   131,    55,   572,     4,    94,     5,    60,
     105,    62,    63,     5,   543,    66,     5,    35,    53,     4,
       4,    24,    24,   543,     3,   592,    13,     4,     3,   516,
      25,     6,     5,     8,   563,    69,   120,   132,   133,     5,
     135,   561,    24,   563,    19,    20,    21,    22,    23,    24,
     570,    97,   572,    39,    35,    30,     4,   134,    33,     4,
     157,    36,     5,    38,   164,    51,   512,   228,   592,    55,
     173,   588,   592,    48,    49,   427,    62,    63,   366,   260,
      66,   329,    68,    58,   137,   225,    33,   469,   396,    64,
      65,   262,    67,   583,   273,   397,    82,    72,    73,   458,
      75,    76,    77,   273,    79,   469,   221,   200,   385,    84,
     147,    86,    87,    88,    89,    90,    91,    92,   503,   223,
     170,   565,   171,   222,   326,   498,   189,   324,   400,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    -1,     3,    -1,    -1,     6,    -1,
       8,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
     135,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,
     118,    -1,     3,    -1,    -1,     6,    -1,     8,   126,   127,
     128,   129,   130,   131,   132,   133,    -1,   135,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    -1,    -1,    36,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    75,    76,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,   116,   117,   118,    -1,     3,
      -1,    -1,     6,    -1,     8,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,   135,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      -1,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,   116,   117,   118,    -1,     3,    -1,    -1,     6,
      -1,     8,   126,   127,   128,   129,   130,   131,   132,   133,
      -1,   135,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    36,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    72,    -1,    -1,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,     3,     4,    -1,     6,    -1,     8,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,   135,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,
      30,    -1,    -1,    33,    -1,    -1,    36,    -1,    38,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
       3,    -1,    -1,     6,    -1,     8,    86,    87,    88,    89,
      90,    91,    92,    -1,    -1,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,   105,    -1,    30,    -1,    -1,
      33,    -1,   112,    36,    -1,    38,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,    -1,    -1,    -1,    -1,
      -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,
      73,    -1,    75,    76,    77,    -1,    -1,     3,    -1,    -1,
       6,    84,     8,    86,    87,    88,    89,    90,    91,    92,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,   105,    -1,    30,    -1,    -1,    33,    -1,   112,
      36,    -1,    38,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,   135,    -1,    -1,    -1,    -1,    -1,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    95,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,   105,
      29,    30,    -1,    -1,    33,    -1,   112,    36,    -1,    38,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
      -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,    -1,
      -1,     3,    -1,    -1,     6,    -1,     8,    86,    87,    88,
      89,    90,    91,    92,    -1,    -1,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,   105,    -1,    30,    -1,
      -1,    33,    -1,   112,    36,    -1,    38,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,   135,    -1,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    75,    76,    77,    -1,    -1,     3,    -1,
      -1,     6,    -1,     8,    86,    87,    88,    89,    90,    91,
      92,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,   105,    -1,    30,    -1,    -1,    33,    -1,
     112,    36,    -1,    38,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,    -1,    -1,    -1,    -1,    -1,    64,
      65,    -1,    67,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      75,    76,    77,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    86,    87,    88,    89,    90,    91,    92,    -1,    -1,
      -1,    19,    20,    21,    22,    -1,    24,     5,    -1,    -1,
     105,    -1,    30,    -1,    -1,    33,    -1,   112,    36,    -1,
      38,   116,   117,   118,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    34,   132,   133,    23,
     135,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,
      34,    -1,    -1,    -1,    72,    23,    54,    75,    76,    77,
      58,    -1,    -1,    61,    -1,    -1,    34,    -1,    86,    -1,
      54,    -1,    -1,    91,    92,    -1,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,   105,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,   102,    -1,   134,    -1,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      23,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,    34,    -1,    33,    -1,    -1,    23,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    -1,    -1,    -1,    64,    65,    -1,    54,    -1,    -1,
      -1,    24,    72,    -1,    61,    75,    76,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    37,    41,    56,    71,    73,    81,    84,   100,
     141,   142,   143,   154,   164,   165,   167,   168,   169,   170,
     174,   175,   180,   184,    28,   150,    50,    57,    29,    43,
     176,   132,   194,   195,   196,   197,   249,   250,   251,   253,
       3,    24,     0,     7,     7,    47,    59,    78,   166,    24,
     152,   153,   197,    24,   285,   286,   287,     3,     6,     8,
      19,    20,    21,    22,    24,    30,    33,    36,    38,    49,
      64,    65,    67,    72,    75,    76,    77,    86,    87,    88,
      89,    90,    91,    92,   105,   112,   116,   117,   118,   132,
     133,   135,   185,   232,   233,   234,   236,   238,   239,   240,
     241,   242,   243,   244,   256,   262,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   278,   279,   280,
     281,   282,   283,   284,   294,   295,   296,   298,   299,   300,
     301,   312,    29,    99,     3,   132,   196,   232,    24,   289,
     290,   291,    39,    51,    55,    62,    63,    66,    74,   245,
     248,   135,   171,   172,   173,   278,     3,   131,   145,   146,
     147,    29,   255,     3,     5,    73,   165,    85,   183,   198,
       6,    13,     3,   158,   165,   278,    24,    25,   293,     3,
       3,     3,     3,     3,   265,    95,   173,   306,   307,   311,
     278,   279,   279,   279,   265,   265,    50,    57,   189,   190,
       5,    20,    22,   105,   112,     3,     3,   135,   136,   137,
     132,   133,   122,   123,   124,   125,    24,    31,   134,   237,
     297,   132,    13,   121,   263,   289,    31,   192,   313,     3,
     292,   192,   193,     6,    13,    60,   248,    24,   177,   178,
     296,    70,    60,     4,     5,    21,    22,    24,    49,    67,
      77,   112,   148,   148,   101,   144,   165,    24,   151,   152,
     176,     3,    23,    48,    58,    79,   126,   127,   128,   129,
     130,   131,   163,   172,   202,   203,   204,   205,   206,   210,
     212,   213,   215,   216,   217,   218,   220,   221,   223,   224,
     226,   227,   230,   231,   238,   241,   256,   278,   296,   103,
     314,   315,    24,   288,    24,   287,   288,   156,   296,   170,
       4,     4,   257,   278,   135,   176,   176,   176,   206,   206,
      96,   307,   308,    95,   302,   303,   306,   191,   197,   181,
     182,   235,   312,   183,   234,     4,   176,   276,   277,   278,
     309,   310,   176,   266,   266,   266,   267,   267,   268,   268,
     268,   268,   297,   278,   283,   295,   294,   193,    24,   158,
     277,    24,   291,   291,   197,    60,     5,   183,   131,   197,
     172,     4,    24,   149,     4,     5,   185,   206,   278,    23,
     203,   232,   163,   232,    54,    61,   163,   213,   214,   216,
     217,   220,   223,   225,   228,    58,   120,   119,   172,    24,
     316,   317,    13,     4,     5,    31,     4,   276,   277,   277,
       5,    93,   309,    94,   163,   173,   211,   213,   214,   216,
     220,   223,   304,   305,   308,   303,     5,     5,   189,    52,
     199,   276,     4,     5,   276,     4,     3,   157,   197,   178,
     314,   179,   278,    68,    82,   246,   247,   254,     6,    13,
      31,    24,     4,    67,     3,   222,   232,   264,   279,    34,
     102,    29,    30,    75,   172,   229,   163,   204,   205,   131,
     317,   296,    24,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   258,   259,
     260,   261,     4,     4,     4,   309,   309,   172,     5,    93,
      94,   197,   182,    35,    53,   200,     4,   276,     4,   155,
     297,   206,     3,   289,    24,    24,     3,   275,   278,    46,
     219,   219,   172,    24,   241,   232,     3,    49,    67,    77,
      80,   201,   207,    24,   201,   236,   318,    13,    25,     4,
       5,   163,   173,   213,   216,   220,   223,   305,   309,   275,
     206,    69,   159,     4,     5,   155,    44,    45,   252,   165,
       4,     5,   279,   120,   132,   195,   208,   209,    24,   309,
      61,   172,    35,    97,   186,   188,   297,     4,     4,   278,
     172,   285,     4,     5,     4,   160,   161,   173,   210,   269,
     314,   209,     5,    32,    42,   162,   162,    98,   187,   161,
     269
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   140,   141,   141,   141,   141,   142,   143,   144,   144,
     145,   145,   145,   146,   147,   148,   148,   148,   148,   148,
     148,   148,   149,   149,   149,   150,   150,   151,   151,   152,
     153,   153,   154,   155,   155,   156,   156,   157,   157,   158,
     158,   159,   159,   160,   160,   161,   161,   162,   162,   162,
     163,   163,   164,   164,   164,   164,   164,   164,   164,   164,
     165,   165,   166,   166,   166,   167,   168,   169,   170,   171,
     171,   172,   173,   174,   175,   176,   176,   176,   177,   177,
     178,   179,   180,   181,   181,   182,   183,   183,   184,   184,
     185,   185,   186,   186,   187,   187,   188,   189,   189,   190,
     191,   191,   192,   192,   193,   193,   194,   194,   195,   195,
     195,   196,   196,   197,   197,   197,   197,   198,   199,   199,
     200,   200,   201,   201,   201,   201,   201,   202,   202,   202,
     202,   202,   202,   202,   203,   203,   204,   204,   205,   205,
     206,   206,   207,   208,   208,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   211,   212,   212,   213,   213,
     213,   213,   213,   213,   213,   214,   215,   216,   217,   218,
     218,   218,   218,   219,   219,   220,   221,   221,   222,   222,
     223,   224,   224,   225,   226,   227,   228,   229,   229,   229,
     230,   231,   232,   233,   233,   234,   235,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   237,   237,   237,   238,
     238,   239,   239,   239,   239,   240,   241,   241,   241,   241,
     241,   242,   243,   243,   243,   243,   243,   244,   244,   244,
     244,   244,   245,   245,   245,   246,   247,   247,   248,   248,
     248,   248,   249,   250,   250,   250,   251,   252,   252,   252,
     253,   253,   254,   255,   255,   256,   257,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   259,   259,   260,   261,   261,   262,   263,
     263,   264,   264,   264,   264,   264,   264,   264,   265,   266,
     266,   266,   266,   267,   267,   267,   267,   268,   268,   268,
     269,   269,   269,   269,   269,   270,   271,   271,   271,   271,
     271,   271,   272,   273,   274,   275,   275,   276,   277,   277,
     278,   278,   278,   279,   280,   280,   281,   281,   282,   283,
     284,   285,   285,   286,   287,   287,   288,   289,   290,   290,
     290,   291,   292,   292,   293,   293,   294,   294,   295,   295,
     295,   296,   297,   298,   299,   299,   300,   301,   302,   302,
     303,   304,   304,   305,   305,   305,   305,   305,   305,   306,
     306,   307,   308,   308,   309,   310,   311,   312,   312,   313,
     313,   314,   314,   315,   316,   316,   317,   317,   318,   318,
     318
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     4,     0,     2,
       0,     1,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     0,     1,     3,     1,     8,
       3,     1,     4,     3,     1,     3,     1,     0,     3,     0,
       3,     0,     3,     1,     3,     2,     2,     0,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     5,     5,     4,     1,
       3,     1,     1,     1,     6,     0,     1,     1,     1,     3,
       3,     1,     6,     1,     3,     1,     0,     1,     4,     1,
       1,     1,     0,     1,     0,     2,     3,     0,     7,     2,
       1,     3,     0,     1,     0,     3,     0,     1,     0,     2,
       2,     0,     1,     4,     3,     4,     1,     2,     0,     3,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     4,     1,     2,     1,     3,
       1,     3,     3,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     2,     1,     1,
       1,     1,     1,     1,     2,     5,     2,     4,     4,     2,
       2,     1,     1,     0,     2,     3,     2,     1,     1,     3,
       3,     2,     1,     3,     2,     2,     3,     1,     1,     1,
       2,     2,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     8,     1,     1,     3,     4,
       5,     1,     5,     5,     5,     4,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     0,     1,
       1,     2,     4,     5,     5,     1,     7,     0,     1,     1,
       1,     1,     4,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     6,     0,
       2,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       2,     2,     2,     1,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     1,     1,     1,     2,
       2,     2,     1,     1,     1,     1,     3,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       2,     1,     1,     3,     3,     3,     1,     1,     1,     3,
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

  case 7: /* pragma: SQL_TOKEN_PRAGMA SQL_TOKEN_NAME opt_pragma_set opt_pragma_for  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
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

  case 45: /* ordering_spec: predicate opt_asc_desc  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 46: /* ordering_spec: row_value_constructor_elem opt_asc_desc  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 47: /* opt_asc_desc: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 50: /* sql_not: %empty  */
    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 60: /* select_statement: single_select_statement  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 61: /* select_statement: single_select_statement union_op all select_statement  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 65: /* commit_statement: SQL_TOKEN_COMMIT  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
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

  case 73: /* rollback_statement: SQL_TOKEN_ROLLBACK  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 74: /* select_statement_into: SQL_TOKEN_SELECT opt_all_distinct selection SQL_TOKEN_INTO target_commalist table_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode)); }
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

  case 83: /* target_commalist: target  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 84: /* target_commalist: target_commalist ',' target  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 86: /* opt_where_clause: %empty  */
                                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 88: /* single_select_statement: SQL_TOKEN_SELECT opt_all_distinct selection table_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 90: /* selection: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 92: /* opt_limit_offset_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 94: /* opt_offset: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 95: /* opt_offset: SQL_TOKEN_OFFSET num_value_exp  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 96: /* limit_offset_clause: SQL_TOKEN_LIMIT num_value_exp opt_offset  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 97: /* table_exp: %empty  */
        { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 98: /* table_exp: from_clause opt_where_clause opt_group_by_clause opt_having_clause opt_order_by_clause opt_limit_offset_clause opt_ecsqloptions_clause  */
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

  case 99: /* from_clause: SQL_TOKEN_FROM table_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 100: /* table_ref_commalist: table_ref  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 101: /* table_ref_commalist: table_ref_commalist ',' table_ref  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 102: /* opt_as: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 104: /* table_primary_as_range_column: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 105: /* table_primary_as_range_column: opt_as SQL_TOKEN_NAME opt_column_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 106: /* opt_disqualify_polymorphic_constraint: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 107: /* opt_disqualify_polymorphic_constraint: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 108: /* opt_only: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 109: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ONLY  */
                                                             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ONLY", SQL_NODE_NAME));
        }
    break;

  case 110: /* opt_only: opt_disqualify_polymorphic_constraint SQL_TOKEN_ALL  */
                                                            {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("ALL", SQL_NODE_NAME));
        }
    break;

  case 111: /* opt_disqualify_primary_join: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 112: /* opt_disqualify_primary_join: '+'  */
             {
                    (yyval.pParseNode) = SQL_NEW_RULE;
                    (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
    }
    break;

  case 113: /* table_ref: opt_only opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 114: /* table_ref: opt_disqualify_primary_join table_node_with_opt_member_func_call table_primary_as_range_column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append(CREATE_NODE("", SQL_NODE_RULE, OSQLParser::RuleID(OSQLParseNode::opt_only)));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 115: /* table_ref: opt_only subquery range_variable opt_column_ref_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 117: /* where_clause: SQL_TOKEN_WHERE search_condition  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 118: /* opt_group_by_clause: %empty  */
                         {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 119: /* opt_group_by_clause: SQL_TOKEN_GROUP SQL_TOKEN_BY value_exp_commalist  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 120: /* opt_having_clause: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 121: /* opt_having_clause: SQL_TOKEN_HAVING search_condition  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 133: /* boolean_primary: '(' search_condition ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 135: /* boolean_test: boolean_primary SQL_TOKEN_IS sql_not truth_value  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 137: /* boolean_factor: SQL_TOKEN_NOT boolean_test  */
        { // boolean_factor: rule 1
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 139: /* boolean_term: boolean_term SQL_TOKEN_AND boolean_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // boolean_term: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 141: /* search_condition: search_condition SQL_TOKEN_OR boolean_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // search_condition
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 142: /* type_predicate: '(' type_list ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 143: /* type_list: type_list_item  */
        {
        (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 144: /* type_list: type_list ',' type_list_item  */
        {
        (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 145: /* type_list_item: opt_only table_node  */
    {
    (yyval.pParseNode) = SQL_NEW_RULE;
    (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
    (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 155: /* comparison_predicate_part_2: comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 156: /* comparison_predicate: row_value_constructor comparison row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // comparison_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 157: /* comparison_predicate: comparison row_value_constructor  */
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

  case 164: /* comparison: SQL_TOKEN_IS sql_not  */
        {
          (yyval.pParseNode) = SQL_NEW_RULE;
          (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
          (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 165: /* between_predicate_part_2: sql_not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 166: /* between_predicate: row_value_constructor between_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // between_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 167: /* character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 168: /* other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 169: /* like_predicate: row_value_constructor character_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // like_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 170: /* like_predicate: row_value_constructor other_like_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;  // like_predicate: rule 3
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 171: /* like_predicate: character_like_predicate_part_2  */
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

  case 172: /* like_predicate: other_like_predicate_part_2  */
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

  case 173: /* opt_escape: %empty  */
                                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 174: /* opt_escape: SQL_TOKEN_ESCAPE string_value_exp  */
            {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 175: /* null_predicate_part_2: SQL_TOKEN_IS sql_not SQL_TOKEN_NULL  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 176: /* test_for_null: row_value_constructor null_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test_for_null: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 177: /* test_for_null: null_predicate_part_2  */
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

  case 178: /* in_predicate_value: subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 179: /* in_predicate_value: '(' value_exp_commalist ')'  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 180: /* in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 181: /* in_predicate: row_value_constructor in_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;// in_predicate: rule 1
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 182: /* in_predicate: in_predicate_part_2  */
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

  case 183: /* quantified_comparison_predicate_part_2: comparison any_all_some subquery  */
    {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
    }
    break;

  case 184: /* all_or_any_predicate: row_value_constructor quantified_comparison_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 185: /* rtreematch_predicate: row_value_constructor rtreematch_predicate_part_2  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 186: /* rtreematch_predicate_part_2: sql_not SQL_TOKEN_MATCH fct_spec  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 190: /* existence_test: SQL_TOKEN_EXISTS subquery  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 191: /* unique_test: SQL_TOKEN_UNIQUE subquery  */
        {(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 192: /* subquery: '(' select_statement ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 193: /* scalar_exp_commalist: select_sublist  */
        {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 194: /* scalar_exp_commalist: scalar_exp_commalist ',' select_sublist  */
        {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
        }
    break;

  case 202: /* literal: literal SQL_TOKEN_STRING  */
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

  case 203: /* literal: literal SQL_TOKEN_INT  */
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

  case 204: /* literal: literal SQL_TOKEN_REAL_NUM  */
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

  case 205: /* literal: literal SQL_TOKEN_APPROXNUM  */
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

  case 206: /* as_clause: %empty  */
                    {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 207: /* as_clause: SQL_TOKEN_AS column  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 215: /* iif_spec: SQL_TOKEN_IIF '(' search_condition ',' result ',' result ')'  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-7].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-5].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));


        }
    break;

  case 218: /* fct_spec: function_name '(' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 219: /* fct_spec: function_name '(' function_args_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 220: /* fct_spec: function_name '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 222: /* general_set_fct: SQL_TOKEN_MAX '(' opt_all_distinct function_args_commalist ')'  */
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

  case 223: /* general_set_fct: SQL_TOKEN_MIN '(' opt_all_distinct function_args_commalist ')'  */
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

  case 224: /* general_set_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 225: /* general_set_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 226: /* general_set_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
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

  case 246: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_with_opt_member_func_call op_relationship_direction  */
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

  case 247: /* op_relationship_direction: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 252: /* named_columns_join: SQL_TOKEN_USING '(' column_commalist ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 253: /* all: %empty  */
               {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 273: /* cast_target_scalar: cast_target_primitive_type  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 274: /* cast_target_scalar: SQL_TOKEN_NAME '.' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 275: /* cast_target_array: cast_target_scalar SQL_TOKEN_ARRAY_INDEX  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 278: /* cast_spec: SQL_TOKEN_CAST '(' cast_operand SQL_TOKEN_AS cast_target ')'  */
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

  case 279: /* opt_extract_value: %empty  */
      { (yyval.pParseNode) = SQL_NEW_RULE; }
    break;

  case 280: /* opt_extract_value: SQL_ARROW property_path  */
        {
           (yyval.pParseNode) = SQL_NEW_RULE;
           (yyval.pParseNode)->append((yyvsp[0].pParseNode));

        }
    break;

  case 286: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 290: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 291: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 292: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 294: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 295: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 296: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 298: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 299: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 301: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 302: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 303: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 304: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 305: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 306: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 307: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 308: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 309: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 311: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 312: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 313: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 314: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 315: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 316: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 318: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 319: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 326: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 327: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 330: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 331: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 332: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 333: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 334: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 335: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 336: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 337: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 338: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 339: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 340: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 341: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 342: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 343: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 344: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 345: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 346: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 347: /* property_path: property_path '.' property_path_entry  */
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

  case 348: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 349: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 350: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 351: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 356: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 357: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 358: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 359: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 360: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 361: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 362: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 369: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 370: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 371: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 372: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 373: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 377: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 379: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 380: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 383: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 384: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 385: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 386: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 387: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
