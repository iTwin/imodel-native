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
  YYSYMBOL_SQL_BITWISE_NOT = 86,           /* SQL_BITWISE_NOT  */
  YYSYMBOL_SQL_TOKEN_CURRENT_DATE = 87,    /* SQL_TOKEN_CURRENT_DATE  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIME = 88,    /* SQL_TOKEN_CURRENT_TIME  */
  YYSYMBOL_SQL_TOKEN_CURRENT_TIMESTAMP = 89, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_EVERY = 90,           /* SQL_TOKEN_EVERY  */
  YYSYMBOL_SQL_TOKEN_CASE = 91,            /* SQL_TOKEN_CASE  */
  YYSYMBOL_SQL_TOKEN_THEN = 92,            /* SQL_TOKEN_THEN  */
  YYSYMBOL_SQL_TOKEN_END = 93,             /* SQL_TOKEN_END  */
  YYSYMBOL_SQL_TOKEN_WHEN = 94,            /* SQL_TOKEN_WHEN  */
  YYSYMBOL_SQL_TOKEN_ELSE = 95,            /* SQL_TOKEN_ELSE  */
  YYSYMBOL_SQL_TOKEN_LIMIT = 96,           /* SQL_TOKEN_LIMIT  */
  YYSYMBOL_SQL_TOKEN_OFFSET = 97,          /* SQL_TOKEN_OFFSET  */
  YYSYMBOL_SQL_TOKEN_ONLY = 98,            /* SQL_TOKEN_ONLY  */
  YYSYMBOL_SQL_TOKEN_PRAGMA = 99,          /* SQL_TOKEN_PRAGMA  */
  YYSYMBOL_SQL_TOKEN_FOR = 100,            /* SQL_TOKEN_FOR  */
  YYSYMBOL_SQL_TOKEN_MATCH = 101,          /* SQL_TOKEN_MATCH  */
  YYSYMBOL_SQL_TOKEN_ECSQLOPTIONS = 102,   /* SQL_TOKEN_ECSQLOPTIONS  */
  YYSYMBOL_SQL_TOKEN_INTEGER = 103,        /* SQL_TOKEN_INTEGER  */
  YYSYMBOL_SQL_TOKEN_INT = 104,            /* SQL_TOKEN_INT  */
  YYSYMBOL_SQL_TOKEN_INT64 = 105,          /* SQL_TOKEN_INT64  */
  YYSYMBOL_SQL_TOKEN_LONG = 106,           /* SQL_TOKEN_LONG  */
  YYSYMBOL_SQL_TOKEN_BOOLEAN = 107,        /* SQL_TOKEN_BOOLEAN  */
  YYSYMBOL_SQL_TOKEN_DOUBLE = 108,         /* SQL_TOKEN_DOUBLE  */
  YYSYMBOL_SQL_TOKEN_REAL = 109,           /* SQL_TOKEN_REAL  */
  YYSYMBOL_SQL_TOKEN_FLOAT = 110,          /* SQL_TOKEN_FLOAT  */
  YYSYMBOL_SQL_TOKEN_STRING = 111,         /* SQL_TOKEN_STRING  */
  YYSYMBOL_SQL_TOKEN_VARCHAR = 112,        /* SQL_TOKEN_VARCHAR  */
  YYSYMBOL_SQL_TOKEN_BINARY = 113,         /* SQL_TOKEN_BINARY  */
  YYSYMBOL_SQL_TOKEN_BLOB = 114,           /* SQL_TOKEN_BLOB  */
  YYSYMBOL_SQL_TOKEN_DATE = 115,           /* SQL_TOKEN_DATE  */
  YYSYMBOL_SQL_TOKEN_TIME = 116,           /* SQL_TOKEN_TIME  */
  YYSYMBOL_SQL_TOKEN_TIMESTAMP = 117,      /* SQL_TOKEN_TIMESTAMP  */
  YYSYMBOL_SQL_TOKEN_OR = 118,             /* SQL_TOKEN_OR  */
  YYSYMBOL_SQL_TOKEN_AND = 119,            /* SQL_TOKEN_AND  */
  YYSYMBOL_SQL_BITWISE_OR = 120,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 121,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 122,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 123,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 124,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 125,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 126,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 127,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 128,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 129,                /* SQL_EQUAL  */
  YYSYMBOL_130_ = 130,                     /* '+'  */
  YYSYMBOL_131_ = 131,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 132,               /* SQL_CONCAT  */
  YYSYMBOL_133_ = 133,                     /* '*'  */
  YYSYMBOL_134_ = 134,                     /* '/'  */
  YYSYMBOL_135_ = 135,                     /* '%'  */
  YYSYMBOL_136_ = 136,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 137,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 138,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 139,     /* sql_single_statement  */
  YYSYMBOL_sql = 140,                      /* sql  */
  YYSYMBOL_pragma = 141,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 142,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 143,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 144,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 145,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 146,             /* pragma_value  */
  YYSYMBOL_pragma_path = 147,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 148,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 149,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 150,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 151,           /* cte_block_list  */
  YYSYMBOL_cte = 152,                      /* cte  */
  YYSYMBOL_column_commalist = 153,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 154,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 155,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 156, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 157,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 158,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 159,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 160,             /* opt_asc_desc  */
  YYSYMBOL_sql_not = 161,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 162,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 163,         /* select_statement  */
  YYSYMBOL_union_op = 164,                 /* union_op  */
  YYSYMBOL_commit_statement = 165,         /* commit_statement  */
  YYSYMBOL_delete_statement_searched = 166, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 167,         /* insert_statement  */
  YYSYMBOL_values_or_query_spec = 168,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 169, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 170,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 171, /* row_value_constructor_elem  */
  YYSYMBOL_rollback_statement = 172,       /* rollback_statement  */
  YYSYMBOL_select_statement_into = 173,    /* select_statement_into  */
  YYSYMBOL_opt_all_distinct = 174,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 175,     /* assignment_commalist  */
  YYSYMBOL_assignment = 176,               /* assignment  */
  YYSYMBOL_update_source = 177,            /* update_source  */
  YYSYMBOL_update_statement_searched = 178, /* update_statement_searched  */
  YYSYMBOL_target_commalist = 179,         /* target_commalist  */
  YYSYMBOL_target = 180,                   /* target  */
  YYSYMBOL_opt_where_clause = 181,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 182,  /* single_select_statement  */
  YYSYMBOL_selection = 183,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 184,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 185,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 186,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 187,                /* table_exp  */
  YYSYMBOL_from_clause = 188,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 189,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 190,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 191, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 192, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 193,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 194, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 195,                /* table_ref  */
  YYSYMBOL_where_clause = 196,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 197,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 198,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 199,              /* truth_value  */
  YYSYMBOL_boolean_primary = 200,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 201,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 202,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 203,             /* boolean_term  */
  YYSYMBOL_search_condition = 204,         /* search_condition  */
  YYSYMBOL_type_predicate = 205,           /* type_predicate  */
  YYSYMBOL_type_list = 206,                /* type_list  */
  YYSYMBOL_type_list_item = 207,           /* type_list_item  */
  YYSYMBOL_predicate = 208,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 209, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 210,     /* comparison_predicate  */
  YYSYMBOL_comparison = 211,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 212, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 213,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 214, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 215, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 216,           /* like_predicate  */
  YYSYMBOL_opt_escape = 217,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 218,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 219,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 220,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 221,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 222,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 223, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 224,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 225,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 226, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 227,             /* any_all_some  */
  YYSYMBOL_existence_test = 228,           /* existence_test  */
  YYSYMBOL_unique_test = 229,              /* unique_test  */
  YYSYMBOL_subquery = 230,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 231,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 232,           /* select_sublist  */
  YYSYMBOL_parameter_ref = 233,            /* parameter_ref  */
  YYSYMBOL_literal = 234,                  /* literal  */
  YYSYMBOL_as_clause = 235,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 236,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 237,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 238,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 239,                 /* fct_spec  */
  YYSYMBOL_function_name = 240,            /* function_name  */
  YYSYMBOL_general_set_fct = 241,          /* general_set_fct  */
  YYSYMBOL_set_fct_type = 242,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 243,          /* outer_join_type  */
  YYSYMBOL_join_condition = 244,           /* join_condition  */
  YYSYMBOL_join_spec = 245,                /* join_spec  */
  YYSYMBOL_join_type = 246,                /* join_type  */
  YYSYMBOL_cross_union = 247,              /* cross_union  */
  YYSYMBOL_qualified_join = 248,           /* qualified_join  */
  YYSYMBOL_ecrelationship_join = 249,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 250, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 251,             /* joined_table  */
  YYSYMBOL_named_columns_join = 252,       /* named_columns_join  */
  YYSYMBOL_all = 253,                      /* all  */
  YYSYMBOL_scalar_subquery = 254,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 255,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 256, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 257,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 258,        /* cast_target_array  */
  YYSYMBOL_cast_target = 259,              /* cast_target  */
  YYSYMBOL_cast_spec = 260,                /* cast_spec  */
  YYSYMBOL_value_exp_primary = 261,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 262,              /* num_primary  */
  YYSYMBOL_factor = 263,                   /* factor  */
  YYSYMBOL_term = 264,                     /* term  */
  YYSYMBOL_term_add_sub = 265,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 266,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 267,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 268,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 269,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 270,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 271,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 272,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 273,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 274,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 275,                /* value_exp  */
  YYSYMBOL_string_value_exp = 276,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 277,           /* char_value_exp  */
  YYSYMBOL_concatenation = 278,            /* concatenation  */
  YYSYMBOL_char_primary = 279,             /* char_primary  */
  YYSYMBOL_char_factor = 280,              /* char_factor  */
  YYSYMBOL_derived_column = 281,           /* derived_column  */
  YYSYMBOL_table_node = 282,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 283, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 284,     /* qualified_class_name  */
  YYSYMBOL_class_name = 285,               /* class_name  */
  YYSYMBOL_table_node_with_opt_member_func_call = 286, /* table_node_with_opt_member_func_call  */
  YYSYMBOL_table_node_path = 287,          /* table_node_path  */
  YYSYMBOL_table_node_path_entry = 288,    /* table_node_path_entry  */
  YYSYMBOL_opt_member_function_args = 289, /* opt_member_function_args  */
  YYSYMBOL_opt_column_array_idx = 290,     /* opt_column_array_idx  */
  YYSYMBOL_property_path = 291,            /* property_path  */
  YYSYMBOL_property_path_entry = 292,      /* property_path_entry  */
  YYSYMBOL_column_ref = 293,               /* column_ref  */
  YYSYMBOL_column = 294,                   /* column  */
  YYSYMBOL_case_expression = 295,          /* case_expression  */
  YYSYMBOL_case_specification = 296,       /* case_specification  */
  YYSYMBOL_simple_case = 297,              /* simple_case  */
  YYSYMBOL_searched_case = 298,            /* searched_case  */
  YYSYMBOL_simple_when_clause_list = 299,  /* simple_when_clause_list  */
  YYSYMBOL_simple_when_clause = 300,       /* simple_when_clause  */
  YYSYMBOL_when_operand_list = 301,        /* when_operand_list  */
  YYSYMBOL_when_operand = 302,             /* when_operand  */
  YYSYMBOL_searched_when_clause_list = 303, /* searched_when_clause_list  */
  YYSYMBOL_searched_when_clause = 304,     /* searched_when_clause  */
  YYSYMBOL_else_clause = 305,              /* else_clause  */
  YYSYMBOL_result = 306,                   /* result  */
  YYSYMBOL_result_expression = 307,        /* result_expression  */
  YYSYMBOL_case_operand = 308,             /* case_operand  */
  YYSYMBOL_parameter = 309,                /* parameter  */
  YYSYMBOL_range_variable = 310,           /* range_variable  */
  YYSYMBOL_opt_ecsqloptions_clause = 311,  /* opt_ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_clause = 312,      /* ecsqloptions_clause  */
  YYSYMBOL_ecsqloptions_list = 313,        /* ecsqloptions_list  */
  YYSYMBOL_ecsqloption = 314,              /* ecsqloption  */
  YYSYMBOL_ecsqloptionvalue = 315          /* ecsqloptionvalue  */
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
#define YYLAST   1791

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  138
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  178
/* YYNRULES -- Number of rules.  */
#define YYNRULES  387
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  589

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   370


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
       2,     2,     2,     2,     2,     2,     2,   135,     2,     2,
       3,     4,   133,   130,     5,   131,    13,   134,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   136,     2,     8,     2,     2,     2,     2,     2,     2,
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
     121,   122,   123,   124,   125,   126,   127,   128,   129,   132,
     137
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   225,   225,   226,   227,   228,   232,   238,   249,   252,
     260,   263,   264,   268,   276,   284,   285,   286,   287,   288,
     289,   290,   294,   299,   304,   316,   319,   323,   328,   336,
     350,   356,   364,   376,   381,   389,   394,   404,   405,   413,
     414,   425,   426,   436,   441,   449,   457,   466,   467,   468,
     472,   473,   479,   480,   481,   482,   483,   484,   485,   486,
     490,   495,   506,   507,   508,   511,   518,   530,   539,   549,
     554,   562,   565,   570,   579,   590,   591,   592,   596,   599,
     605,   612,   615,   627,   630,   636,   640,   641,   649,   657,
     661,   666,   669,   670,   673,   674,   682,   691,   692,   706,
     716,   719,   725,   726,   730,   733,   743,   744,   751,   752,
     757,   765,   766,   773,   781,   789,   797,   801,   810,   811,
     821,   822,   830,   831,   832,   833,   834,   837,   838,   839,
     840,   841,   842,   843,   853,   854,   864,   865,   873,   874,
     883,   884,   894,   903,   908,   916,   925,   926,   927,   928,
     929,   930,   931,   932,   933,   939,   946,   953,   978,   979,
     980,   981,   982,   983,   984,   992,  1002,  1010,  1020,  1030,
    1036,  1042,  1064,  1089,  1090,  1097,  1106,  1112,  1128,  1132,
    1140,  1149,  1155,  1171,  1180,  1189,  1198,  1208,  1209,  1210,
    1214,  1222,  1228,  1239,  1244,  1251,  1255,  1259,  1260,  1261,
    1262,  1263,  1265,  1277,  1289,  1301,  1317,  1318,  1324,  1328,
    1329,  1332,  1333,  1334,  1335,  1338,  1350,  1351,  1352,  1359,
    1368,  1382,  1387,  1396,  1404,  1416,  1417,  1418,  1419,  1420,
    1421,  1422,  1426,  1431,  1436,  1443,  1451,  1452,  1455,  1456,
    1461,  1462,  1470,  1482,  1492,  1501,  1506,  1520,  1521,  1522,
    1525,  1526,  1529,  1541,  1542,  1546,  1549,  1553,  1554,  1555,
    1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,
    1566,  1567,  1568,  1572,  1577,  1587,  1596,  1597,  1601,  1613,
    1614,  1615,  1616,  1617,  1618,  1625,  1629,  1633,  1634,  1640,
    1646,  1655,  1656,  1663,  1670,  1680,  1681,  1688,  1698,  1699,
    1706,  1713,  1720,  1730,  1737,  1742,  1747,  1752,  1758,  1764,
    1773,  1780,  1788,  1796,  1799,  1805,  1809,  1814,  1822,  1823,
    1824,  1828,  1832,  1833,  1836,  1843,  1853,  1856,  1860,  1870,
    1875,  1882,  1891,  1899,  1908,  1916,  1924,  1929,  1934,  1942,
    1951,  1952,  1962,  1963,  1971,  1976,  1994,  2000,  2008,  2019,
    2023,  2027,  2028,  2032,  2043,  2053,  2058,  2065,  2075,  2078,
    2083,  2084,  2085,  2086,  2087,  2088,  2091,  2096,  2103,  2113,
    2114,  2122,  2126,  2130,  2134,  2140,  2148,  2151,  2161,  2162,
    2166,  2175,  2180,  2188,  2194,  2204,  2205,  2206
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
  "SQL_BITWISE_NOT", "SQL_TOKEN_CURRENT_DATE", "SQL_TOKEN_CURRENT_TIME",
  "SQL_TOKEN_CURRENT_TIMESTAMP", "SQL_TOKEN_EVERY", "SQL_TOKEN_CASE",
  "SQL_TOKEN_THEN", "SQL_TOKEN_END", "SQL_TOKEN_WHEN", "SQL_TOKEN_ELSE",
  "SQL_TOKEN_LIMIT", "SQL_TOKEN_OFFSET", "SQL_TOKEN_ONLY",
  "SQL_TOKEN_PRAGMA", "SQL_TOKEN_FOR", "SQL_TOKEN_MATCH",
  "SQL_TOKEN_ECSQLOPTIONS", "SQL_TOKEN_INTEGER", "SQL_TOKEN_INT",
  "SQL_TOKEN_INT64", "SQL_TOKEN_LONG", "SQL_TOKEN_BOOLEAN",
  "SQL_TOKEN_DOUBLE", "SQL_TOKEN_REAL", "SQL_TOKEN_FLOAT",
  "SQL_TOKEN_STRING", "SQL_TOKEN_VARCHAR", "SQL_TOKEN_BINARY",
  "SQL_TOKEN_BLOB", "SQL_TOKEN_DATE", "SQL_TOKEN_TIME",
  "SQL_TOKEN_TIMESTAMP", "SQL_TOKEN_OR", "SQL_TOKEN_AND", "SQL_BITWISE_OR",
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
  "cast_target_array", "cast_target", "cast_spec", "value_exp_primary",
  "num_primary", "factor", "term", "term_add_sub", "num_value_exp",
  "datetime_primary", "datetime_value_fct", "datetime_factor",
  "datetime_term", "datetime_value_exp", "value_exp_commalist",
  "function_arg", "function_args_commalist", "value_exp",
  "string_value_exp", "char_value_exp", "concatenation", "char_primary",
  "char_factor", "derived_column", "table_node",
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

#define YYPACT_NINF (-471)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-366)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     423,    51,  -471,    85,    24,  -471,    49,    19,    84,   142,
     174,   188,   224,  -471,  -471,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,  -471,    97,  -471,   185,    19,   228,  -471,  -471,
    1369,   232,    25,    32,   236,   210,  -471,  -471,  -471,  -471,
    1445,    35,  -471,  -471,  -471,  -471,  -471,  -471,   241,   272,
    -471,    92,   336,   124,   279,  -471,  -471,  1118,   263,  -471,
    -471,  -471,  -471,  -471,    73,  -471,  -471,   286,   289,  -471,
    -471,  -471,  -471,   291,  -471,  -471,  -471,  1596,  -471,  -471,
    -471,  -471,  1217,  -471,  -471,  1445,  1445,  1445,  1596,  1596,
     274,   148,  -471,   302,  -471,    30,  -471,  -471,  -471,  -471,
     306,  -471,   317,  -471,  -471,  -471,  -471,  -471,   255,     9,
     205,  -471,  -471,  -471,  -471,  -471,    37,  -471,   192,  -471,
    -471,  -471,  -471,   316,  -471,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,  -471,    98,  -471,   236,   175,   329,   175,   202,
    -471,   283,  -471,  -471,  -471,  -471,    31,    27,   269,   288,
    -471,   179,  -471,  -471,   218,   180,   180,   253,  -471,  -471,
    -471,    98,   343,   185,    49,  -471,   697,   267,  -471,   368,
     373,    27,   322,   352,    26,  -471,  -471,  -471,  1445,    20,
     697,  -471,   697,  -471,   168,  -471,   307,   218,  -471,  -471,
    -471,  -471,  -471,    19,   222,  -471,   324,  1445,  -471,  -471,
    -471,  -471,  1042,    49,  1521,  1521,  1521,  1521,  1521,  1521,
    1521,  1521,  1521,  -471,   387,  1445,  -471,  -471,   308,    27,
     175,  -471,   389,   279,  1445,  -471,   396,  -471,   236,   236,
      19,   362,   422,    53,  -471,   319,  -471,    19,  -471,  1445,
    -471,  -471,  -471,  -471,  -471,  -471,  -471,   445,  -471,   427,
    -471,  -471,  -471,   262,  -471,  1369,   467,   582,   449,   431,
     449,  -471,  -471,  -471,  -471,  -471,  -471,   157,    62,   397,
    -471,  -471,   337,   340,  -471,  -471,  1445,  -471,  -471,  -471,
    -471,  -471,  -471,  -471,  -471,  -471,  -471,  -471,  -471,   311,
    1587,  1623,   254,  1641,   435,  -471,  -471,  -471,  -471,   211,
    -471,  -471,   296,  -471,  -471,  -471,  -471,   432,   218,   461,
    1445,    43,   -18,  1445,  -471,   374,   697,   371,  -471,   307,
     463,   590,    75,  -471,  -471,  -471,   417,  -471,  -471,  1445,
    -471,   299,   218,  -471,  -471,  1445,  -471,  -471,  -471,   255,
     255,     9,     9,     9,     9,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,   309,   469,  -471,  -471,   590,    19,    27,   267,
    1445,    44,  -471,  -471,  -471,   242,   443,   452,   430,    36,
      13,  -471,  -471,  -471,   416,  -471,   482,  1445,    50,  1293,
    -471,  -471,  -471,  -471,  -471,  -471,  -471,   431,   697,   697,
    -471,   364,   435,  -471,   368,  -471,    27,  1674,  -471,   491,
    1445,  1445,  -471,  -471,   158,    54,  -471,  1445,  -471,   494,
     496,   504,    55,  -471,   418,  -471,    19,   222,  -471,   475,
     460,   510,  -471,  1445,   513,  -471,   387,  -471,   590,  -471,
    -471,  -471,   218,   697,    72,  -471,  -471,  -471,   499,   500,
     515,  -471,  -471,  -471,  1118,  -471,  -471,   502,    -5,  1445,
     497,  -471,  -471,  -471,  -471,   449,   130,  -471,   337,   404,
    -471,  -471,   507,  -471,  -471,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,  -471,  -471,  -471,  -471,  -471,  -471,  -471,   501,
    -471,   525,  -471,   528,  -471,    65,   927,  1445,  -471,   590,
    -471,  1445,   697,   468,  -471,  -471,  -471,   369,  -471,   340,
     387,   332,  -471,  -471,    98,   380,   218,  1445,  -471,  -471,
     419,  -471,  -471,  -471,    12,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,  -471,    30,  -471,   512,  -471,  -471,  1445,   162,
    -471,  1445,  -471,  -471,  -471,  -471,  -471,   536,   340,   514,
     451,  -471,   387,   390,  -471,  -471,  -471,   541,  -471,  1445,
     420,  1445,  -471,   228,   400,  -471,  -471,   546,  1445,  -471,
     812,  1521,   267,  -471,  -471,  -471,  -471,   218,  -471,  -471,
    -471,    12,  -471,   554,  -471,   264,   138,   196,  -471,  -471,
     812,  -471,  -471,  -471,  -471,  1521,  -471,  -471,   205
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
      31,     0,    86,     0,    39,   330,   329,     0,     0,   375,
     201,   198,   199,   200,   342,   230,   225,     0,     0,   213,
     226,   227,   212,     0,   231,   228,   214,     0,   304,   306,
     305,   229,     0,   197,   326,     0,     0,     0,     0,     0,
     347,    97,   255,    91,   193,   210,   279,   209,   217,   280,
       0,   216,     0,   282,   285,   286,   287,   291,   295,   298,
     318,   310,   303,   311,   312,   320,   206,   319,   321,   323,
     327,   322,   195,   348,   344,   281,   283,   350,   351,   352,
     211,   110,   109,     0,   112,     0,   376,   340,   104,   335,
     336,     0,   234,   239,   232,   233,   238,     0,   240,     0,
     347,     0,    69,    71,    72,     0,     0,     8,    11,    12,
     254,     0,     0,     0,    75,    32,    50,   378,    87,     0,
       0,     0,     0,     0,     0,   374,   343,   346,     0,    75,
      50,   290,    50,   373,   369,   366,     0,     0,   307,   309,
     308,   289,   288,   106,     0,    88,    86,     0,   204,   205,
     203,   202,     0,    75,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   349,     0,     0,   328,   208,     0,     0,
     104,   103,     0,    39,     0,   339,     0,   114,     0,     0,
     106,     0,   342,    86,    78,     0,   241,   106,    68,     0,
      15,    16,    17,    20,    21,    19,    18,     0,    13,     0,
       7,    61,    28,     0,    30,     0,    50,    50,     0,    50,
       0,   162,   163,   159,   158,   161,   160,     0,    50,   134,
     136,   138,   140,   117,   127,   146,     0,   147,   171,   172,
     153,   177,   151,   182,   152,   148,   154,   149,   150,   128,
     129,   131,   132,   130,     0,    66,   379,   334,   333,   334,
     331,   332,     0,    36,    67,   192,   284,     0,   256,     0,
       0,     0,     0,     0,   367,     0,    50,   369,   355,     0,
      99,   100,    97,    83,    85,   196,   118,   194,   218,     0,
     316,     0,   372,   315,   371,     0,   292,   293,   294,   296,
     297,   301,   302,   299,   300,   207,   325,   324,   345,   113,
     377,   115,     0,    37,   338,   337,   242,   106,     0,   378,
       0,   238,    70,    14,    22,     9,     0,     0,    97,     0,
      72,    51,   137,   190,   164,   191,     0,     0,     0,     0,
     166,   169,   170,   176,   181,   184,   185,    50,    50,    50,
     157,   383,   380,   382,     0,    40,     0,     0,   223,     0,
       0,     0,   370,   354,     0,    71,   361,     0,   362,   171,
     177,   182,     0,   358,     0,   356,   106,     0,    74,     0,
     120,     0,   219,     0,     0,   341,     0,   105,   243,    79,
      82,    80,    81,    50,     0,   236,   244,   237,     0,     0,
       0,    27,   133,   175,     0,   180,   178,   173,   173,     0,
       0,   188,   187,   189,   156,     0,     0,   139,   141,     0,
     381,    35,   272,   263,   262,   264,   265,   259,   260,   266,
     261,   267,   271,   257,   258,   268,   269,   270,   273,   276,
     277,     0,   224,     0,   368,   157,    50,     0,   353,   101,
      84,     0,    50,    41,   220,   317,   222,     0,    34,   235,
       0,   247,    24,    23,     0,     0,   313,     0,   168,   167,
       0,   221,   186,   183,   106,   123,   125,   122,   124,   135,
     126,   386,   387,   385,   384,     0,   275,   278,     0,     0,
     360,     0,   364,   365,   363,   359,   357,   119,   121,     0,
      92,    38,     0,     0,   248,   249,   246,     0,   179,     0,
     174,     0,   107,     0,     0,   143,   274,     0,     0,   155,
      50,     0,   378,    93,    33,   252,    29,   314,   165,   145,
     142,   106,   215,    42,    43,    71,    47,    94,    98,   144,
      50,    48,    49,    46,    45,     0,    96,    44,    95
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -471,  -471,  -471,  -471,  -471,  -471,  -471,  -471,   407,  -471,
    -471,  -471,   401,  -471,  -471,    60,  -471,  -471,   342,  -471,
    -471,   -14,    -9,  -244,  -471,     2,  -471,  -471,  -471,  -471,
     398,  -471,   -39,   -78,  -471,  -471,   -74,  -471,   216,  -471,
    -471,  -471,   151,  -130,  -471,   320,  -471,  -471,  -471,   257,
    -471,  -471,   440,   357,  -471,  -447,   547,   -16,  -471,  -471,
    -471,   122,  -471,   350,   198,   200,  -154,  -471,  -471,    28,
    -367,  -471,  -471,  -254,   341,  -471,  -252,   345,  -471,   160,
    -247,  -471,  -471,  -243,  -471,  -471,  -471,  -471,  -471,  -471,
    -471,  -471,   -25,  -471,   413,  -471,   155,  -471,  -143,  -471,
    -471,  -148,  -471,  -471,  -471,  -471,  -471,  -471,   465,  -471,
    -471,  -471,  -471,  -471,  -471,  -471,  -135,  -471,  -471,  -471,
    -471,  -471,  -471,   240,     0,   131,   208,   152,  -470,  -471,
    -471,  -471,  -471,  -471,   128,  -264,   402,   -30,   -80,  -471,
    -471,  -471,   403,  -471,    74,  -471,   458,   470,  -124,  -471,
     189,  -471,  -471,  -471,   424,  -138,  -113,  -471,  -471,  -471,
    -471,  -471,   323,  -471,   165,   462,  -164,   321,  -284,  -471,
    -471,  -181,  -471,  -340,  -471,  -471,   252,  -471
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,   250,   157,   158,   159,   247,   365,
      25,   253,    50,    51,    13,   497,   302,   427,   172,   540,
     573,   574,   583,   267,    14,   173,    48,    16,    17,    18,
      19,   151,   268,   153,    20,    21,    30,   233,   234,   431,
      22,   322,   323,   167,    23,    91,   562,   586,   563,   195,
     196,   320,   226,   227,    32,    33,    34,    35,   168,   420,
     493,   519,   269,   270,   271,   272,   312,   520,   554,   555,
     274,   406,   275,   276,   408,   277,   278,   279,   280,   508,
     281,   282,   445,   283,   284,   385,   285,   286,   386,   455,
     287,   288,    92,    93,    94,   324,    95,   216,    96,    97,
      98,    99,   100,   101,   102,   148,   435,   436,   149,    36,
      37,    38,   546,    39,   437,   161,   103,   307,   478,   479,
     480,   481,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   505,   330,   331,   154,   117,   118,
     119,   120,   121,   122,    54,    55,    56,   301,   138,   139,
     140,   225,   177,   123,   124,   125,   498,   126,   127,   128,
     129,   317,   318,   412,   413,   184,   185,   315,   333,   334,
     186,   130,   223,   295,   296,   392,   393,   524
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     116,   152,    15,   217,   183,   188,   189,   190,   136,   235,
      52,   220,   273,   325,   379,   374,   381,   306,   290,   430,
     314,   383,  -108,   289,   378,   384,   311,   174,   293,   402,
     306,   291,   290,   303,   290,   133,  -108,   289,   155,   289,
     442,   507,   293,  -108,   293,   291,   399,   291,   400,    28,
     198,   232,   199,   165,   131,   187,   187,   187,   358,  -360,
     486,   213,   407,    29,   409,   421,   326,   553,   214,   410,
    -155,   424,   404,   411,   401,   500,  -221,   181,    28,    24,
     417,    27,   142,   141,   449,   371,   143,    40,   191,   192,
     255,   577,    29,   144,   145,   142,   137,   163,   176,   143,
     389,   345,   369,   359,   376,   310,   144,   145,   290,   290,
     146,   377,   433,   289,   289,   588,   483,   484,   293,   293,
     259,   291,   291,   132,   553,   193,   434,  -319,   329,   335,
     169,  -132,  -132,   514,   200,    26,   292,   170,   166,   207,
     208,   201,   552,   456,    45,   215,  -360,   487,   308,    31,
     292,   450,   292,   309,   389,   314,    46,  -155,   215,   495,
     150,   389,   134,   251,   156,   164,    41,   116,   290,   215,
     581,   164,   332,   289,    42,    47,     8,   321,   293,   515,
     582,   291,     8,   238,   239,   346,   261,   262,   263,   264,
     265,   266,   449,   576,   332,    43,   449,   516,   193,  -102,
     362,   240,   241,   536,   242,   194,   221,   517,   228,    49,
     518,   376,   376,   576,   356,   229,   376,   169,   377,   377,
     235,   361,   578,   558,   394,   116,   370,   292,    58,   243,
      59,    44,   531,   373,   532,   375,   325,   390,   405,   533,
     290,   290,   529,   534,   557,   289,   289,   244,   438,   141,
     293,   293,    53,   291,   291,   439,  -112,   245,   461,   -72,
     137,   142,   182,   313,   -47,   143,   366,   367,   -47,   -47,
     160,   -47,   144,   145,   -90,   162,   146,   -72,   -90,   499,
     332,   -90,   171,   332,   147,   290,   292,   175,   -72,   178,
     289,   246,   179,   585,   180,   293,   581,   448,   291,   332,
     395,   396,   512,   422,   423,   332,   582,   197,   -72,   202,
     501,   -47,   -72,   425,   423,   -72,   209,   210,   211,   212,
     203,   -90,   218,   -47,   -90,   209,   210,   211,   212,   219,
     432,   -90,   224,   -90,  -279,   336,   337,   338,   538,   236,
     454,   428,   -47,   230,   290,  -279,   -72,   187,   237,   289,
     215,   446,   -90,   249,   293,   -72,   305,   291,   292,   292,
     -47,   341,   342,   343,   344,  -279,   -47,   252,   485,   294,
     332,   332,  -279,   541,   542,   141,   544,   545,   -72,   -72,
     -72,   -72,   -72,   -72,   548,   549,   215,   142,   204,   205,
     206,   143,   297,   332,   565,   542,  -238,   299,   144,   145,
     489,   316,   146,   292,   570,   571,     8,   514,   530,   166,
     510,   213,  -279,   350,   506,   339,   340,   354,   355,    84,
     353,   166,   357,    60,    61,    62,    63,   550,   521,   564,
     513,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,   176,   360,   363,
       1,   364,   133,   515,   371,   387,   388,   332,   389,   391,
       2,   506,   292,   397,     3,   398,   313,   403,   416,   419,
     256,   516,   426,    58,   440,    59,   441,   187,   448,     4,
     193,   517,   575,   443,   518,   444,    60,    61,    62,    63,
     257,    64,   559,   459,     5,   482,     6,    65,   332,  -364,
      66,  -365,   575,    67,     7,    68,   547,     8,    83,  -363,
     491,   488,   568,   492,   494,   258,    69,   496,   504,   567,
     525,   511,     9,   502,   503,   259,   526,    65,   187,   527,
      66,    70,    71,   528,    72,    68,   556,   539,   551,    73,
     164,   549,    74,    75,    76,   566,   260,   561,   507,   560,
     572,     8,  -319,    77,    78,    79,    80,    81,    82,   580,
     543,    70,    71,   248,   254,   351,   587,   584,   490,    73,
     304,    83,    74,    75,   429,   368,   222,   349,    84,   418,
     135,   522,    85,    86,    87,   256,   457,    81,    58,   458,
      59,   261,   262,   263,   264,   265,   266,    88,    89,   579,
     150,    60,    61,    62,    63,   371,    64,   372,   509,   380,
     327,   231,    65,   382,   523,    66,   -51,   447,    67,   537,
      68,   347,  -286,  -286,  -286,  -286,   352,   569,   300,   141,
     258,    69,  -286,  -286,  -286,  -286,  -286,  -286,   414,   298,
     259,   142,   415,   348,   460,   143,    70,    71,   319,    72,
    -238,   535,   144,   145,    73,     0,   146,    74,    75,    76,
       0,   260,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,    85,    86,    87,
     256,     0,     0,    58,     0,    59,   261,   262,   263,   264,
     265,   266,    88,    89,     0,   150,    60,    61,    62,    63,
     257,    64,     0,     0,     0,     0,     0,    65,     0,     0,
      66,     0,     0,    67,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   258,    69,     0,     0,     0,
       0,     0,     0,     0,     0,   259,     0,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,   260,     0,     0,     0,
       0,     0,     0,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,    85,    86,    87,    57,     0,     0,    58,     0,
      59,   261,   262,   263,   264,   265,   266,    88,    89,     0,
     150,    60,    61,    62,    63,   371,    64,     0,     0,     0,
       0,     0,    65,     0,     0,    66,     0,     0,    67,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     258,    69,     0,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,     0,     0,     0,    70,    71,     0,    72,
       0,     0,     0,     0,    73,     0,     0,    74,    75,    76,
       0,   260,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,    85,    86,    87,
      57,     0,     0,    58,     0,    59,   261,   262,   263,   264,
     265,   266,    88,    89,     0,   150,    60,    61,    62,    63,
     371,    64,     0,     0,     0,     0,     0,    65,     0,     0,
      66,     0,     0,    67,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,   259,     0,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,     0,     0,     0,     0,
       0,     0,     0,    77,    78,    79,    80,    81,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,    85,    86,    87,    57,   328,     0,    58,     0,
      59,   261,   262,   263,   264,   265,   266,    88,    89,     0,
     150,    60,    61,    62,    63,     0,    64,     0,     0,     0,
       0,    28,    65,     0,     0,    66,     0,     0,    67,     0,
      68,     0,     0,     0,     0,    29,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
       0,     0,     0,     0,    73,     0,     0,    74,    75,    76,
       0,    57,     0,     0,    58,     0,    59,     0,    77,    78,
      79,    80,    81,    82,     0,     0,     0,    60,    61,    62,
      63,     0,    64,     0,     0,     0,    83,     0,    65,     0,
       0,    66,     0,    84,    67,     0,    68,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,    88,    89,     0,   150,     0,     0,     0,     0,
       0,     0,    70,    71,     0,    72,     0,     0,     0,     0,
      73,   164,     0,    74,    75,    76,     0,     0,     0,     0,
       0,     0,     8,     0,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,     0,    83,    58,     0,    59,     0,     0,     0,    84,
       0,     0,     0,    85,    86,    87,    60,    61,    62,    63,
       0,    64,     0,     0,     0,     0,     0,    65,    88,    89,
      66,   150,     0,    67,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,    57,     0,     0,    58,
       0,    59,     0,    77,    78,    79,    80,    81,    82,     0,
       0,   182,    60,    61,    62,    63,     0,    64,     0,     0,
       0,    83,   451,   452,     0,     0,    66,     0,    84,    67,
       0,    68,    85,    86,    87,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,    88,    89,     0,
     150,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,     0,     0,     0,     0,    73,     0,     0,   453,    75,
      76,     0,    57,     0,     0,    58,     0,    59,     0,    77,
      78,    79,    80,    81,    82,     0,     0,     0,    60,    61,
      62,    63,     0,    64,     0,     0,     0,    83,     0,    65,
       0,     0,    66,     0,    84,    67,     0,    68,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,    88,    89,     0,   150,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
       0,    73,     0,     0,    74,    75,    76,     0,    57,     0,
       0,    58,     0,    59,     0,    77,    78,    79,    80,    81,
      82,     0,     0,     0,    60,    61,    62,    63,     0,    64,
       0,     0,     0,    83,     0,    65,     0,     0,    66,     0,
      84,    67,     0,    68,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,    88,
      89,     0,    90,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      74,    75,    76,     0,    57,     0,     0,    58,     0,    59,
       0,    77,    78,    79,    80,    81,    82,     0,     0,     0,
      60,    61,    62,    63,     0,    64,     0,     0,     0,    83,
       0,    65,     0,     0,    66,     0,    84,    67,     0,    68,
      85,    86,    87,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,    88,    89,     0,   150,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,     0,
       0,     0,     0,    73,     0,     0,    74,    75,    76,    57,
       0,     0,    58,     0,    59,     0,     0,    77,     0,     0,
    -280,    81,    82,     0,     0,    60,    61,    62,    63,     0,
      64,  -280,     0,     0,     0,    83,    65,     0,     0,    66,
       0,     0,    67,     0,    68,     0,     0,     0,     0,     0,
       0,  -280,     0,     0,     0,    69,  -282,     0,  -280,     0,
       0,    88,    89,     0,   150,     0,     0,  -282,     0,     0,
      70,    71,     0,    72,  -281,     0,     0,     0,    73,     0,
       0,    74,    75,    76,     0,  -281,     0,  -282,     0,     0,
       0,     0,     0,     0,  -282,     0,    81,    82,  -280,     0,
       0,     0,     0,     0,     0,  -281,     0,     0,   462,     0,
      83,     0,  -281,     0,     0,     0,     0,  -280,  -280,  -280,
    -280,  -280,  -280,  -280,  -280,  -280,  -280,  -280,  -280,  -280,
    -280,  -280,  -280,     0,  -282,     0,     0,     0,     0,   150,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -281,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,     0,
       0,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477
};

static const yytype_int16 yycheck[] =
{
      30,    40,     0,   116,    82,    85,    86,    87,    33,   147,
      26,   135,   166,   194,   268,   259,   268,     4,   166,   359,
     184,   268,     3,   166,   268,   268,   180,    57,   166,   313,
       4,   166,   180,   171,   182,     3,    24,   180,     3,   182,
       4,    46,   180,    24,   182,   180,   310,   182,     5,    29,
      20,    24,    22,    51,    29,    85,    86,    87,     5,     5,
       5,    24,   316,    43,   316,   329,   196,   514,    31,   316,
       5,   335,   316,   316,    92,     3,     3,    77,    29,    28,
       5,    57,    51,    39,    34,    23,    55,     3,    88,    89,
     164,   561,    43,    62,    63,    51,    24,     5,    25,    55,
     118,   214,   256,   233,    54,   179,    62,    63,   256,   257,
      66,    61,    68,   256,   257,   585,   400,   401,   256,   257,
      58,   256,   257,    98,   571,    50,    82,   132,   202,   203,
       6,   118,   119,     3,   104,    50,   166,    13,    85,   130,
     131,   111,   130,   387,    47,   132,    92,    92,   178,   130,
     180,   101,   182,   133,   118,   319,    59,    92,   132,   423,
     133,   118,   130,   161,   129,    73,    24,   197,   316,   132,
      32,    73,   202,   316,     0,    78,    84,   193,   316,    49,
      42,   316,    84,     4,     5,   215,   124,   125,   126,   127,
     128,   129,    34,   560,   224,     7,    34,    67,    50,    24,
     239,    21,    22,   487,    24,    57,    31,    77,     6,    24,
      80,    54,    54,   580,   230,    13,    54,     6,    61,    61,
     358,   237,   562,    61,    13,   255,   256,   257,     6,    49,
       8,     7,   486,   258,   486,   260,   417,   276,   316,   486,
     388,   389,   486,   486,   528,   388,   389,    67,     6,    39,
     388,   389,    24,   388,   389,    13,    24,    77,   396,     5,
      24,    51,    94,    95,     0,    55,     4,     5,     4,     5,
      29,     7,    62,    63,     0,     3,    66,    23,     4,   433,
     310,     7,     3,   313,    74,   433,   316,    24,    34,     3,
     433,   111,     3,    97,     3,   433,    32,   377,   433,   329,
       4,     5,   450,     4,     5,   335,    42,     5,    54,     3,
     434,    47,    58,     4,     5,    61,   120,   121,   122,   123,
       3,    47,   130,    59,    50,   120,   121,   122,   123,    13,
     360,    57,     3,    59,    23,   204,   205,   206,   492,    70,
     379,   357,    78,    60,   492,    34,    92,   377,    60,   492,
     132,   376,    78,   100,   492,   101,     4,   492,   388,   389,
      96,   209,   210,   211,   212,    54,   102,    24,   407,   102,
     400,   401,    61,     4,     5,    39,    44,    45,   124,   125,
     126,   127,   128,   129,     4,     5,   132,    51,   133,   134,
     135,    55,    24,   423,     4,     5,    60,    24,    62,    63,
     416,    94,    66,   433,     4,     5,    84,     3,   486,    85,
     449,    24,   101,    24,   444,   207,   208,   228,   229,   111,
      24,    85,    60,    19,    20,    21,    22,   507,    24,   542,
     455,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,    25,   129,     4,
      27,    24,     3,    49,    23,    58,   119,   487,   118,    24,
      37,   491,   492,    31,    41,     4,    95,    93,     5,    52,
       3,    67,     3,     6,    31,     8,    24,   507,   558,    56,
      50,    77,   560,    67,    80,     3,    19,    20,    21,    22,
      23,    24,   531,   129,    71,     4,    73,    30,   528,     5,
      33,     5,   580,    36,    81,    38,   504,    84,   104,     5,
      35,    93,   551,    53,     4,    48,    49,     4,     3,   549,
      13,    24,    99,    24,    24,    58,    25,    30,   558,     4,
      33,    64,    65,     5,    67,    38,    24,    69,   119,    72,
      73,     5,    75,    76,    77,     4,    79,    96,    46,    35,
       4,    84,   132,    86,    87,    88,    89,    90,    91,     5,
     500,    64,    65,   156,   163,   223,   580,   576,   417,    72,
     172,   104,    75,    76,   358,   255,   136,   220,   111,   322,
      33,   459,   115,   116,   117,     3,   388,    90,     6,   389,
       8,   124,   125,   126,   127,   128,   129,   130,   131,   571,
     133,    19,    20,    21,    22,    23,    24,   257,   448,   268,
     197,   146,    30,   268,   459,    33,    34,   377,    36,   491,
      38,   218,   120,   121,   122,   123,   224,   553,   170,    39,
      48,    49,   130,   131,   132,   133,   134,   135,   317,   169,
      58,    51,   319,   219,   392,    55,    64,    65,   186,    67,
      60,   486,    62,    63,    72,    -1,    66,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,   115,   116,   117,
       3,    -1,    -1,     6,    -1,     8,   124,   125,   126,   127,
     128,   129,   130,   131,    -1,   133,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    -1,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,   115,   116,   117,     3,    -1,    -1,     6,    -1,
       8,   124,   125,   126,   127,   128,   129,   130,   131,    -1,
     133,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    -1,    -1,    36,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,   115,   116,   117,
       3,    -1,    -1,     6,    -1,     8,   124,   125,   126,   127,
     128,   129,   130,   131,    -1,   133,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    -1,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,   115,   116,   117,     3,     4,    -1,     6,    -1,
       8,   124,   125,   126,   127,   128,   129,   130,   131,    -1,
     133,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      -1,    29,    30,    -1,    -1,    33,    -1,    -1,    36,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,     3,    -1,    -1,     6,    -1,     8,    -1,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,   104,    -1,    30,    -1,
      -1,    33,    -1,   111,    36,    -1,    38,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      72,    73,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    -1,    86,    87,    88,    89,    90,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    -1,   104,     6,    -1,     8,    -1,    -1,    -1,   111,
      -1,    -1,    -1,   115,   116,   117,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    30,   130,   131,
      33,   133,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,     3,    -1,    -1,     6,
      -1,     8,    -1,    86,    87,    88,    89,    90,    91,    -1,
      -1,    94,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,   104,    29,    30,    -1,    -1,    33,    -1,   111,    36,
      -1,    38,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,   130,   131,    -1,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    72,    -1,    -1,    75,    76,
      77,    -1,     3,    -1,    -1,     6,    -1,     8,    -1,    86,
      87,    88,    89,    90,    91,    -1,    -1,    -1,    19,    20,
      21,    22,    -1,    24,    -1,    -1,    -1,   104,    -1,    30,
      -1,    -1,    33,    -1,   111,    36,    -1,    38,   115,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,   130,   131,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    75,    76,    77,    -1,     3,    -1,
      -1,     6,    -1,     8,    -1,    86,    87,    88,    89,    90,
      91,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,   104,    -1,    30,    -1,    -1,    33,    -1,
     111,    36,    -1,    38,   115,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,   130,
     131,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,    64,
      65,    -1,    67,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      75,    76,    77,    -1,     3,    -1,    -1,     6,    -1,     8,
      -1,    86,    87,    88,    89,    90,    91,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,   104,
      -1,    30,    -1,    -1,    33,    -1,   111,    36,    -1,    38,
     115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,     3,
      -1,    -1,     6,    -1,     8,    -1,    -1,    86,    -1,    -1,
      23,    90,    91,    -1,    -1,    19,    20,    21,    22,    -1,
      24,    34,    -1,    -1,    -1,   104,    30,    -1,    -1,    33,
      -1,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    49,    23,    -1,    61,    -1,
      -1,   130,   131,    -1,   133,    -1,    -1,    34,    -1,    -1,
      64,    65,    -1,    67,    23,    -1,    -1,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    34,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    -1,    90,    91,   101,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    24,    -1,
     104,    -1,    61,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,   101,    -1,    -1,    -1,    -1,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   101,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    37,    41,    56,    71,    73,    81,    84,    99,
     139,   140,   141,   152,   162,   163,   165,   166,   167,   168,
     172,   173,   178,   182,    28,   148,    50,    57,    29,    43,
     174,   130,   192,   193,   194,   195,   247,   248,   249,   251,
       3,    24,     0,     7,     7,    47,    59,    78,   164,    24,
     150,   151,   195,    24,   282,   283,   284,     3,     6,     8,
      19,    20,    21,    22,    24,    30,    33,    36,    38,    49,
      64,    65,    67,    72,    75,    76,    77,    86,    87,    88,
      89,    90,    91,   104,   111,   115,   116,   117,   130,   131,
     133,   183,   230,   231,   232,   234,   236,   237,   238,   239,
     240,   241,   242,   254,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   275,   276,   277,   278,
     279,   280,   281,   291,   292,   293,   295,   296,   297,   298,
     309,    29,    98,     3,   130,   194,   230,    24,   286,   287,
     288,    39,    51,    55,    62,    63,    66,    74,   243,   246,
     133,   169,   170,   171,   275,     3,   129,   143,   144,   145,
      29,   253,     3,     5,    73,   163,    85,   181,   196,     6,
      13,     3,   156,   163,   275,    24,    25,   290,     3,     3,
       3,   262,    94,   171,   303,   304,   308,   275,   276,   276,
     276,   262,   262,    50,    57,   187,   188,     5,    20,    22,
     104,   111,     3,     3,   133,   134,   135,   130,   131,   120,
     121,   122,   123,    24,    31,   132,   235,   294,   130,    13,
     286,    31,   190,   310,     3,   289,   190,   191,     6,    13,
      60,   246,    24,   175,   176,   293,    70,    60,     4,     5,
      21,    22,    24,    49,    67,    77,   111,   146,   146,   100,
     142,   163,    24,   149,   150,   174,     3,    23,    48,    58,
      79,   124,   125,   126,   127,   128,   129,   161,   170,   200,
     201,   202,   203,   204,   208,   210,   211,   213,   214,   215,
     216,   218,   219,   221,   222,   224,   225,   228,   229,   236,
     239,   254,   275,   293,   102,   311,   312,    24,   285,    24,
     284,   285,   154,   293,   168,     4,     4,   255,   275,   133,
     174,   204,   204,    95,   304,   305,    94,   299,   300,   303,
     189,   195,   179,   180,   233,   309,   181,   232,     4,   174,
     273,   274,   275,   306,   307,   174,   263,   263,   263,   264,
     264,   265,   265,   265,   265,   294,   275,   280,   292,   191,
      24,   156,   274,    24,   288,   288,   195,    60,     5,   181,
     129,   195,   170,     4,    24,   147,     4,     5,   183,   204,
     275,    23,   201,   230,   161,   230,    54,    61,   161,   211,
     212,   214,   215,   218,   221,   223,   226,    58,   119,   118,
     170,    24,   313,   314,    13,     4,     5,    31,     4,   273,
       5,    92,   306,    93,   161,   171,   209,   211,   212,   214,
     218,   221,   301,   302,   305,   300,     5,     5,   187,    52,
     197,   273,     4,     5,   273,     4,     3,   155,   195,   176,
     311,   177,   275,    68,    82,   244,   245,   252,     6,    13,
      31,    24,     4,    67,     3,   220,   230,   261,   276,    34,
     101,    29,    30,    75,   170,   227,   161,   202,   203,   129,
     314,   293,    24,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   256,   257,
     258,   259,     4,   306,   306,   170,     5,    92,    93,   195,
     180,    35,    53,   198,     4,   273,     4,   153,   294,   204,
       3,   286,    24,    24,     3,   272,   275,    46,   217,   217,
     170,    24,   239,   230,     3,    49,    67,    77,    80,   199,
     205,    24,   199,   234,   315,    13,    25,     4,     5,   161,
     171,   211,   214,   218,   221,   302,   306,   272,   204,    69,
     157,     4,     5,   153,    44,    45,   250,   163,     4,     5,
     276,   119,   130,   193,   206,   207,    24,   306,    61,   170,
      35,    96,   184,   186,   294,     4,     4,   275,   170,   282,
       4,     5,     4,   158,   159,   171,   208,   266,   311,   207,
       5,    32,    42,   160,   160,    97,   185,   159,   266
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   138,   139,   139,   139,   139,   140,   141,   142,   142,
     143,   143,   143,   144,   145,   146,   146,   146,   146,   146,
     146,   146,   147,   147,   147,   148,   148,   149,   149,   150,
     151,   151,   152,   153,   153,   154,   154,   155,   155,   156,
     156,   157,   157,   158,   158,   159,   159,   160,   160,   160,
     161,   161,   162,   162,   162,   162,   162,   162,   162,   162,
     163,   163,   164,   164,   164,   165,   166,   167,   168,   169,
     169,   170,   171,   172,   173,   174,   174,   174,   175,   175,
     176,   177,   178,   179,   179,   180,   181,   181,   182,   182,
     183,   183,   184,   184,   185,   185,   186,   187,   187,   188,
     189,   189,   190,   190,   191,   191,   192,   192,   193,   193,
     193,   194,   194,   195,   195,   195,   195,   196,   197,   197,
     198,   198,   199,   199,   199,   199,   199,   200,   200,   200,
     200,   200,   200,   200,   201,   201,   202,   202,   203,   203,
     204,   204,   205,   206,   206,   207,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   209,   210,   210,   211,   211,
     211,   211,   211,   211,   211,   212,   213,   214,   215,   216,
     216,   216,   216,   217,   217,   218,   219,   219,   220,   220,
     221,   222,   222,   223,   224,   225,   226,   227,   227,   227,
     228,   229,   230,   231,   231,   232,   233,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   235,   235,   235,   236,
     236,   237,   237,   237,   237,   238,   239,   239,   239,   239,
     239,   240,   241,   241,   241,   242,   242,   242,   242,   242,
     242,   242,   243,   243,   243,   244,   245,   245,   246,   246,
     246,   246,   247,   248,   248,   248,   249,   250,   250,   250,
     251,   251,   252,   253,   253,   254,   255,   256,   256,   256,
     256,   256,   256,   256,   256,   256,   256,   256,   256,   256,
     256,   256,   256,   257,   257,   258,   259,   259,   260,   261,
     261,   261,   261,   261,   261,   261,   262,   263,   263,   263,
     263,   264,   264,   264,   264,   265,   265,   265,   266,   266,
     266,   266,   266,   267,   268,   268,   268,   268,   268,   268,
     269,   270,   271,   272,   272,   273,   274,   274,   275,   275,
     275,   276,   277,   277,   278,   278,   279,   280,   281,   282,
     282,   283,   284,   284,   285,   286,   287,   287,   287,   288,
     289,   289,   290,   290,   291,   291,   292,   292,   293,   294,
     295,   296,   296,   297,   298,   299,   299,   300,   301,   301,
     302,   302,   302,   302,   302,   302,   303,   303,   304,   305,
     305,   306,   307,   308,   309,   309,   310,   310,   311,   311,
     312,   313,   313,   314,   314,   315,   315,   315
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
       5,     1,     5,     4,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     0,     1,
       1,     2,     4,     5,     5,     1,     7,     0,     1,     1,
       1,     1,     4,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     6,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     2,     2,
       2,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     1,     1,     1,     2,     2,     2,
       1,     1,     1,     1,     3,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     2,     1,
       1,     3,     3,     3,     1,     1,     1,     3,     3,     2,
       0,     3,     0,     1,     1,     3,     2,     1,     1,     1,
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

  case 222: /* general_set_fct: set_fct_type '(' opt_all_distinct function_arg ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 223: /* general_set_fct: SQL_TOKEN_COUNT '(' '*' ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 224: /* general_set_fct: SQL_TOKEN_COUNT '(' opt_all_distinct function_arg ')'  */
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

  case 284: /* value_exp_primary: '(' value_exp ')'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
        }
    break;

  case 288: /* factor: '-' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 289: /* factor: '+' num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 290: /* factor: SQL_BITWISE_NOT num_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("~", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 292: /* term: term '*' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 293: /* term: term '/' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("/", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 294: /* term: term '%' factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("%", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 296: /* term_add_sub: term_add_sub '+' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 297: /* term_add_sub: term_add_sub '-' term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("-", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 299: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_LEFT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("<<", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 300: /* num_value_exp: num_value_exp SQL_BITWISE_SHIFT_RIGHT term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(">>", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 301: /* num_value_exp: num_value_exp SQL_BITWISE_OR term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("|", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 302: /* num_value_exp: num_value_exp SQL_BITWISE_AND term_add_sub  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("&", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 303: /* datetime_primary: datetime_value_fct  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 304: /* datetime_value_fct: SQL_TOKEN_CURRENT_DATE  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 305: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIMESTAMP  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 306: /* datetime_value_fct: SQL_TOKEN_CURRENT_TIME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 307: /* datetime_value_fct: SQL_TOKEN_DATE string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 308: /* datetime_value_fct: SQL_TOKEN_TIMESTAMP string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 309: /* datetime_value_fct: SQL_TOKEN_TIME string_value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 310: /* datetime_factor: datetime_primary  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 311: /* datetime_term: datetime_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 312: /* datetime_value_exp: datetime_term  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 313: /* value_exp_commalist: value_exp  */
            {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 314: /* value_exp_commalist: value_exp_commalist ',' value_exp  */
            {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 316: /* function_args_commalist: function_arg  */
            {
            (yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 317: /* function_args_commalist: function_args_commalist ',' function_arg  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 324: /* concatenation: char_value_exp '+' char_factor  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE("+", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 325: /* concatenation: value_exp SQL_CONCAT value_exp  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 328: /* derived_column: value_exp as_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 329: /* table_node: qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 330: /* table_node: tablespace_qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 331: /* tablespace_qualified_class_name: SQL_TOKEN_NAME '.' qualified_class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 332: /* qualified_class_name: SQL_TOKEN_NAME '.' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(".", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 333: /* qualified_class_name: SQL_TOKEN_NAME ':' class_name  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 334: /* class_name: SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 335: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 336: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 337: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 338: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 339: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 340: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 341: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 342: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 343: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 344: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 345: /* property_path: property_path '.' property_path_entry  */
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

  case 346: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 347: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 348: /* column_ref: property_path  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 353: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 354: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 355: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 356: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 357: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 358: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 359: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 366: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 367: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 368: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 369: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 370: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 374: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 375: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 376: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 377: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 380: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 381: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 382: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 383: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 384: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
