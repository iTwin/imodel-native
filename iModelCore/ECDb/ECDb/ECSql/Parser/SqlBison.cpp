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
  YYSYMBOL_SQL_TOKEN_DELETE = 40,          /* SQL_TOKEN_DELETE  */
  YYSYMBOL_SQL_TOKEN_DESC = 41,            /* SQL_TOKEN_DESC  */
  YYSYMBOL_SQL_TOKEN_DISTINCT = 42,        /* SQL_TOKEN_DISTINCT  */
  YYSYMBOL_SQL_TOKEN_FORWARD = 43,         /* SQL_TOKEN_FORWARD  */
  YYSYMBOL_SQL_TOKEN_BACKWARD = 44,        /* SQL_TOKEN_BACKWARD  */
  YYSYMBOL_SQL_TOKEN_ESCAPE = 45,          /* SQL_TOKEN_ESCAPE  */
  YYSYMBOL_SQL_TOKEN_EXCEPT = 46,          /* SQL_TOKEN_EXCEPT  */
  YYSYMBOL_SQL_TOKEN_EXISTS = 47,          /* SQL_TOKEN_EXISTS  */
  YYSYMBOL_SQL_TOKEN_FALSE = 48,           /* SQL_TOKEN_FALSE  */
  YYSYMBOL_SQL_TOKEN_FROM = 49,            /* SQL_TOKEN_FROM  */
  YYSYMBOL_SQL_TOKEN_FULL = 50,            /* SQL_TOKEN_FULL  */
  YYSYMBOL_SQL_TOKEN_GROUP = 51,           /* SQL_TOKEN_GROUP  */
  YYSYMBOL_SQL_TOKEN_HAVING = 52,          /* SQL_TOKEN_HAVING  */
  YYSYMBOL_SQL_TOKEN_IN = 53,              /* SQL_TOKEN_IN  */
  YYSYMBOL_SQL_TOKEN_INNER = 54,           /* SQL_TOKEN_INNER  */
  YYSYMBOL_SQL_TOKEN_INSERT = 55,          /* SQL_TOKEN_INSERT  */
  YYSYMBOL_SQL_TOKEN_INTO = 56,            /* SQL_TOKEN_INTO  */
  YYSYMBOL_SQL_TOKEN_IS = 57,              /* SQL_TOKEN_IS  */
  YYSYMBOL_SQL_TOKEN_INTERSECT = 58,       /* SQL_TOKEN_INTERSECT  */
  YYSYMBOL_SQL_TOKEN_JOIN = 59,            /* SQL_TOKEN_JOIN  */
  YYSYMBOL_SQL_TOKEN_LIKE = 60,            /* SQL_TOKEN_LIKE  */
  YYSYMBOL_SQL_TOKEN_LEFT = 61,            /* SQL_TOKEN_LEFT  */
  YYSYMBOL_SQL_TOKEN_RIGHT = 62,           /* SQL_TOKEN_RIGHT  */
  YYSYMBOL_SQL_TOKEN_MAX = 63,             /* SQL_TOKEN_MAX  */
  YYSYMBOL_SQL_TOKEN_MIN = 64,             /* SQL_TOKEN_MIN  */
  YYSYMBOL_SQL_TOKEN_NATURAL = 65,         /* SQL_TOKEN_NATURAL  */
  YYSYMBOL_SQL_TOKEN_NULL = 66,            /* SQL_TOKEN_NULL  */
  YYSYMBOL_SQL_TOKEN_ON = 67,              /* SQL_TOKEN_ON  */
  YYSYMBOL_SQL_TOKEN_ORDER = 68,           /* SQL_TOKEN_ORDER  */
  YYSYMBOL_SQL_TOKEN_OUTER = 69,           /* SQL_TOKEN_OUTER  */
  YYSYMBOL_SQL_TOKEN_ROLLBACK = 70,        /* SQL_TOKEN_ROLLBACK  */
  YYSYMBOL_SQL_TOKEN_IIF = 71,             /* SQL_TOKEN_IIF  */
  YYSYMBOL_SQL_TOKEN_SELECT = 72,          /* SQL_TOKEN_SELECT  */
  YYSYMBOL_SQL_TOKEN_SET = 73,             /* SQL_TOKEN_SET  */
  YYSYMBOL_SQL_TOKEN_SOME = 74,            /* SQL_TOKEN_SOME  */
  YYSYMBOL_SQL_TOKEN_SUM = 75,             /* SQL_TOKEN_SUM  */
  YYSYMBOL_SQL_TOKEN_TRUE = 76,            /* SQL_TOKEN_TRUE  */
  YYSYMBOL_SQL_TOKEN_UNION = 77,           /* SQL_TOKEN_UNION  */
  YYSYMBOL_SQL_TOKEN_UNIQUE = 78,          /* SQL_TOKEN_UNIQUE  */
  YYSYMBOL_SQL_TOKEN_UNKNOWN = 79,         /* SQL_TOKEN_UNKNOWN  */
  YYSYMBOL_SQL_TOKEN_UPDATE = 80,          /* SQL_TOKEN_UPDATE  */
  YYSYMBOL_SQL_TOKEN_USING = 81,           /* SQL_TOKEN_USING  */
  YYSYMBOL_SQL_TOKEN_VALUE = 82,           /* SQL_TOKEN_VALUE  */
  YYSYMBOL_SQL_TOKEN_VALUES = 83,          /* SQL_TOKEN_VALUES  */
  YYSYMBOL_SQL_TOKEN_WHERE = 84,           /* SQL_TOKEN_WHERE  */
  YYSYMBOL_SQL_TOKEN_DOLLAR = 85,          /* SQL_TOKEN_DOLLAR  */
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
  YYSYMBOL_SQL_ARROW = 120,                /* SQL_ARROW  */
  YYSYMBOL_SQL_BITWISE_OR = 121,           /* SQL_BITWISE_OR  */
  YYSYMBOL_SQL_BITWISE_AND = 122,          /* SQL_BITWISE_AND  */
  YYSYMBOL_SQL_BITWISE_SHIFT_LEFT = 123,   /* SQL_BITWISE_SHIFT_LEFT  */
  YYSYMBOL_SQL_BITWISE_SHIFT_RIGHT = 124,  /* SQL_BITWISE_SHIFT_RIGHT  */
  YYSYMBOL_SQL_LESSEQ = 125,               /* SQL_LESSEQ  */
  YYSYMBOL_SQL_GREATEQ = 126,              /* SQL_GREATEQ  */
  YYSYMBOL_SQL_NOTEQUAL = 127,             /* SQL_NOTEQUAL  */
  YYSYMBOL_SQL_LESS = 128,                 /* SQL_LESS  */
  YYSYMBOL_SQL_GREAT = 129,                /* SQL_GREAT  */
  YYSYMBOL_SQL_EQUAL = 130,                /* SQL_EQUAL  */
  YYSYMBOL_131_ = 131,                     /* '+'  */
  YYSYMBOL_132_ = 132,                     /* '-'  */
  YYSYMBOL_SQL_CONCAT = 133,               /* SQL_CONCAT  */
  YYSYMBOL_134_ = 134,                     /* '*'  */
  YYSYMBOL_135_ = 135,                     /* '/'  */
  YYSYMBOL_136_ = 136,                     /* '%'  */
  YYSYMBOL_137_ = 137,                     /* '='  */
  YYSYMBOL_SQL_TOKEN_INVALIDSYMBOL = 138,  /* SQL_TOKEN_INVALIDSYMBOL  */
  YYSYMBOL_YYACCEPT = 139,                 /* $accept  */
  YYSYMBOL_sql_single_statement = 140,     /* sql_single_statement  */
  YYSYMBOL_sql = 141,                      /* sql  */
  YYSYMBOL_pragma = 142,                   /* pragma  */
  YYSYMBOL_opt_pragma_for = 143,           /* opt_pragma_for  */
  YYSYMBOL_opt_pragma_set = 144,           /* opt_pragma_set  */
  YYSYMBOL_opt_pragma_set_val = 145,       /* opt_pragma_set_val  */
  YYSYMBOL_opt_pragma_func = 146,          /* opt_pragma_func  */
  YYSYMBOL_pragma_value = 147,             /* pragma_value  */
  YYSYMBOL_pragma_path = 148,              /* pragma_path  */
  YYSYMBOL_opt_cte_recursive = 149,        /* opt_cte_recursive  */
  YYSYMBOL_cte_column_list = 150,          /* cte_column_list  */
  YYSYMBOL_cte_table_name = 151,           /* cte_table_name  */
  YYSYMBOL_cte_block_list = 152,           /* cte_block_list  */
  YYSYMBOL_cte = 153,                      /* cte  */
  YYSYMBOL_column_commalist = 154,         /* column_commalist  */
  YYSYMBOL_column_ref_commalist = 155,     /* column_ref_commalist  */
  YYSYMBOL_opt_column_commalist = 156,     /* opt_column_commalist  */
  YYSYMBOL_opt_column_ref_commalist = 157, /* opt_column_ref_commalist  */
  YYSYMBOL_opt_order_by_clause = 158,      /* opt_order_by_clause  */
  YYSYMBOL_ordering_spec_commalist = 159,  /* ordering_spec_commalist  */
  YYSYMBOL_ordering_spec = 160,            /* ordering_spec  */
  YYSYMBOL_opt_asc_desc = 161,             /* opt_asc_desc  */
  YYSYMBOL_sql_not = 162,                  /* sql_not  */
  YYSYMBOL_manipulative_statement = 163,   /* manipulative_statement  */
  YYSYMBOL_select_statement = 164,         /* select_statement  */
  YYSYMBOL_union_op = 165,                 /* union_op  */
  YYSYMBOL_commit_statement = 166,         /* commit_statement  */
  YYSYMBOL_delete_statement_searched = 167, /* delete_statement_searched  */
  YYSYMBOL_insert_statement = 168,         /* insert_statement  */
  YYSYMBOL_values_or_query_spec = 169,     /* values_or_query_spec  */
  YYSYMBOL_row_value_constructor_commalist = 170, /* row_value_constructor_commalist  */
  YYSYMBOL_row_value_constructor = 171,    /* row_value_constructor  */
  YYSYMBOL_row_value_constructor_elem = 172, /* row_value_constructor_elem  */
  YYSYMBOL_rollback_statement = 173,       /* rollback_statement  */
  YYSYMBOL_select_statement_into = 174,    /* select_statement_into  */
  YYSYMBOL_opt_all_distinct = 175,         /* opt_all_distinct  */
  YYSYMBOL_assignment_commalist = 176,     /* assignment_commalist  */
  YYSYMBOL_assignment = 177,               /* assignment  */
  YYSYMBOL_update_source = 178,            /* update_source  */
  YYSYMBOL_update_statement_searched = 179, /* update_statement_searched  */
  YYSYMBOL_target_commalist = 180,         /* target_commalist  */
  YYSYMBOL_target = 181,                   /* target  */
  YYSYMBOL_opt_where_clause = 182,         /* opt_where_clause  */
  YYSYMBOL_single_select_statement = 183,  /* single_select_statement  */
  YYSYMBOL_selection = 184,                /* selection  */
  YYSYMBOL_opt_limit_offset_clause = 185,  /* opt_limit_offset_clause  */
  YYSYMBOL_opt_offset = 186,               /* opt_offset  */
  YYSYMBOL_limit_offset_clause = 187,      /* limit_offset_clause  */
  YYSYMBOL_table_exp = 188,                /* table_exp  */
  YYSYMBOL_from_clause = 189,              /* from_clause  */
  YYSYMBOL_table_ref_commalist = 190,      /* table_ref_commalist  */
  YYSYMBOL_opt_as = 191,                   /* opt_as  */
  YYSYMBOL_table_primary_as_range_column = 192, /* table_primary_as_range_column  */
  YYSYMBOL_opt_disqualify_polymorphic_constraint = 193, /* opt_disqualify_polymorphic_constraint  */
  YYSYMBOL_opt_only = 194,                 /* opt_only  */
  YYSYMBOL_opt_disqualify_primary_join = 195, /* opt_disqualify_primary_join  */
  YYSYMBOL_table_ref = 196,                /* table_ref  */
  YYSYMBOL_where_clause = 197,             /* where_clause  */
  YYSYMBOL_opt_group_by_clause = 198,      /* opt_group_by_clause  */
  YYSYMBOL_opt_having_clause = 199,        /* opt_having_clause  */
  YYSYMBOL_truth_value = 200,              /* truth_value  */
  YYSYMBOL_boolean_primary = 201,          /* boolean_primary  */
  YYSYMBOL_boolean_test = 202,             /* boolean_test  */
  YYSYMBOL_boolean_factor = 203,           /* boolean_factor  */
  YYSYMBOL_boolean_term = 204,             /* boolean_term  */
  YYSYMBOL_search_condition = 205,         /* search_condition  */
  YYSYMBOL_type_predicate = 206,           /* type_predicate  */
  YYSYMBOL_type_list = 207,                /* type_list  */
  YYSYMBOL_type_list_item = 208,           /* type_list_item  */
  YYSYMBOL_predicate = 209,                /* predicate  */
  YYSYMBOL_comparison_predicate_part_2 = 210, /* comparison_predicate_part_2  */
  YYSYMBOL_comparison_predicate = 211,     /* comparison_predicate  */
  YYSYMBOL_comparison = 212,               /* comparison  */
  YYSYMBOL_between_predicate_part_2 = 213, /* between_predicate_part_2  */
  YYSYMBOL_between_predicate = 214,        /* between_predicate  */
  YYSYMBOL_character_like_predicate_part_2 = 215, /* character_like_predicate_part_2  */
  YYSYMBOL_other_like_predicate_part_2 = 216, /* other_like_predicate_part_2  */
  YYSYMBOL_like_predicate = 217,           /* like_predicate  */
  YYSYMBOL_opt_escape = 218,               /* opt_escape  */
  YYSYMBOL_null_predicate_part_2 = 219,    /* null_predicate_part_2  */
  YYSYMBOL_test_for_null = 220,            /* test_for_null  */
  YYSYMBOL_in_predicate_value = 221,       /* in_predicate_value  */
  YYSYMBOL_in_predicate_part_2 = 222,      /* in_predicate_part_2  */
  YYSYMBOL_in_predicate = 223,             /* in_predicate  */
  YYSYMBOL_quantified_comparison_predicate_part_2 = 224, /* quantified_comparison_predicate_part_2  */
  YYSYMBOL_all_or_any_predicate = 225,     /* all_or_any_predicate  */
  YYSYMBOL_rtreematch_predicate = 226,     /* rtreematch_predicate  */
  YYSYMBOL_rtreematch_predicate_part_2 = 227, /* rtreematch_predicate_part_2  */
  YYSYMBOL_any_all_some = 228,             /* any_all_some  */
  YYSYMBOL_existence_test = 229,           /* existence_test  */
  YYSYMBOL_unique_test = 230,              /* unique_test  */
  YYSYMBOL_subquery = 231,                 /* subquery  */
  YYSYMBOL_scalar_exp_commalist = 232,     /* scalar_exp_commalist  */
  YYSYMBOL_select_sublist = 233,           /* select_sublist  */
  YYSYMBOL_parameter_ref = 234,            /* parameter_ref  */
  YYSYMBOL_literal = 235,                  /* literal  */
  YYSYMBOL_as_clause = 236,                /* as_clause  */
  YYSYMBOL_unsigned_value_spec = 237,      /* unsigned_value_spec  */
  YYSYMBOL_general_value_spec = 238,       /* general_value_spec  */
  YYSYMBOL_iif_spec = 239,                 /* iif_spec  */
  YYSYMBOL_fct_spec = 240,                 /* fct_spec  */
  YYSYMBOL_function_name = 241,            /* function_name  */
  YYSYMBOL_general_set_fct = 242,          /* general_set_fct  */
  YYSYMBOL_set_fct_type = 243,             /* set_fct_type  */
  YYSYMBOL_outer_join_type = 244,          /* outer_join_type  */
  YYSYMBOL_join_condition = 245,           /* join_condition  */
  YYSYMBOL_join_spec = 246,                /* join_spec  */
  YYSYMBOL_join_type = 247,                /* join_type  */
  YYSYMBOL_cross_union = 248,              /* cross_union  */
  YYSYMBOL_qualified_join = 249,           /* qualified_join  */
  YYSYMBOL_ecrelationship_join = 250,      /* ecrelationship_join  */
  YYSYMBOL_op_relationship_direction = 251, /* op_relationship_direction  */
  YYSYMBOL_joined_table = 252,             /* joined_table  */
  YYSYMBOL_named_columns_join = 253,       /* named_columns_join  */
  YYSYMBOL_all = 254,                      /* all  */
  YYSYMBOL_scalar_subquery = 255,          /* scalar_subquery  */
  YYSYMBOL_cast_operand = 256,             /* cast_operand  */
  YYSYMBOL_cast_target_primitive_type = 257, /* cast_target_primitive_type  */
  YYSYMBOL_cast_target_scalar = 258,       /* cast_target_scalar  */
  YYSYMBOL_cast_target_array = 259,        /* cast_target_array  */
  YYSYMBOL_cast_target = 260,              /* cast_target  */
  YYSYMBOL_cast_spec = 261,                /* cast_spec  */
  YYSYMBOL_opt_extract_value = 262,        /* opt_extract_value  */
  YYSYMBOL_value_exp_primary = 263,        /* value_exp_primary  */
  YYSYMBOL_num_primary = 264,              /* num_primary  */
  YYSYMBOL_factor = 265,                   /* factor  */
  YYSYMBOL_term = 266,                     /* term  */
  YYSYMBOL_term_add_sub = 267,             /* term_add_sub  */
  YYSYMBOL_num_value_exp = 268,            /* num_value_exp  */
  YYSYMBOL_datetime_primary = 269,         /* datetime_primary  */
  YYSYMBOL_datetime_value_fct = 270,       /* datetime_value_fct  */
  YYSYMBOL_datetime_factor = 271,          /* datetime_factor  */
  YYSYMBOL_datetime_term = 272,            /* datetime_term  */
  YYSYMBOL_datetime_value_exp = 273,       /* datetime_value_exp  */
  YYSYMBOL_value_exp_commalist = 274,      /* value_exp_commalist  */
  YYSYMBOL_function_arg = 275,             /* function_arg  */
  YYSYMBOL_function_args_commalist = 276,  /* function_args_commalist  */
  YYSYMBOL_value_exp = 277,                /* value_exp  */
  YYSYMBOL_string_value_exp = 278,         /* string_value_exp  */
  YYSYMBOL_char_value_exp = 279,           /* char_value_exp  */
  YYSYMBOL_concatenation = 280,            /* concatenation  */
  YYSYMBOL_char_primary = 281,             /* char_primary  */
  YYSYMBOL_char_factor = 282,              /* char_factor  */
  YYSYMBOL_derived_column = 283,           /* derived_column  */
  YYSYMBOL_table_node = 284,               /* table_node  */
  YYSYMBOL_tablespace_qualified_class_name = 285, /* tablespace_qualified_class_name  */
  YYSYMBOL_qualified_class_name = 286,     /* qualified_class_name  */
  YYSYMBOL_class_name = 287,               /* class_name  */
  YYSYMBOL_table_node_ref = 288,           /* table_node_ref  */
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
#define YYLAST   1662

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  139
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  180
/* YYNRULES -- Number of rules.  */
#define YYNRULES  391
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  604

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   371


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
       2,     2,     2,     2,     2,     2,     2,   136,     2,     2,
       3,     4,   134,   131,     5,   132,    13,   135,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     7,
       2,   137,     2,     8,     2,     2,     2,     2,     2,     2,
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
     133,   138
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   227,   227,   228,   229,   230,   234,   240,   252,   255,
     263,   266,   267,   271,   279,   287,   288,   289,   290,   291,
     292,   293,   297,   302,   307,   319,   322,   326,   331,   339,
     353,   359,   367,   379,   384,   392,   397,   407,   408,   416,
     417,   428,   429,   439,   444,   452,   460,   469,   470,   471,
     475,   476,   482,   483,   484,   485,   486,   487,   488,   489,
     493,   498,   509,   510,   511,   514,   521,   533,   542,   552,
     557,   565,   568,   573,   582,   593,   594,   595,   599,   602,
     608,   615,   618,   630,   633,   639,   643,   644,   652,   660,
     664,   669,   672,   673,   676,   677,   685,   694,   695,   709,
     719,   722,   728,   729,   733,   736,   746,   747,   754,   755,
     760,   768,   769,   776,   784,   792,   800,   804,   813,   814,
     824,   825,   833,   834,   835,   836,   837,   840,   841,   842,
     843,   844,   845,   846,   856,   857,   867,   868,   876,   877,
     886,   887,   897,   906,   911,   919,   928,   929,   930,   931,
     932,   933,   934,   935,   936,   942,   949,   956,   981,   982,
     983,   984,   985,   986,   987,   995,  1005,  1013,  1023,  1033,
    1039,  1045,  1067,  1092,  1093,  1100,  1109,  1115,  1131,  1135,
    1143,  1152,  1158,  1174,  1183,  1192,  1201,  1211,  1212,  1213,
    1217,  1225,  1231,  1242,  1247,  1254,  1258,  1262,  1263,  1264,
    1265,  1266,  1268,  1280,  1292,  1304,  1320,  1321,  1327,  1331,
    1332,  1335,  1336,  1337,  1338,  1341,  1353,  1354,  1355,  1362,
    1371,  1385,  1390,  1406,  1422,  1431,  1439,  1451,  1452,  1453,
    1454,  1455,  1459,  1464,  1469,  1476,  1484,  1485,  1488,  1489,
    1494,  1495,  1503,  1515,  1525,  1534,  1539,  1553,  1554,  1555,
    1558,  1559,  1562,  1574,  1575,  1579,  1582,  1586,  1587,  1588,
    1589,  1590,  1591,  1592,  1593,  1594,  1595,  1596,  1597,  1598,
    1599,  1600,  1601,  1605,  1610,  1620,  1629,  1630,  1634,  1647,
    1648,  1656,  1657,  1658,  1659,  1660,  1661,  1668,  1672,  1676,
    1677,  1683,  1689,  1698,  1699,  1706,  1713,  1723,  1724,  1731,
    1741,  1742,  1749,  1756,  1763,  1773,  1780,  1785,  1790,  1795,
    1801,  1807,  1816,  1823,  1831,  1839,  1842,  1848,  1852,  1857,
    1865,  1866,  1867,  1871,  1875,  1876,  1879,  1886,  1896,  1899,
    1903,  1913,  1918,  1925,  1934,  1942,  1951,  1959,  1968,  1976,
    1981,  1986,  1994,  2003,  2004,  2014,  2015,  2023,  2028,  2046,
    2052,  2057,  2065,  2077,  2081,  2085,  2086,  2090,  2101,  2111,
    2116,  2123,  2133,  2136,  2141,  2142,  2143,  2144,  2145,  2146,
    2149,  2154,  2161,  2171,  2172,  2180,  2184,  2188,  2192,  2198,
    2206,  2209,  2219,  2220,  2224,  2233,  2238,  2246,  2252,  2262,
    2263,  2264
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

#define YYPACT_NINF (-514)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-370)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     304,    18,  -514,    16,    61,  -514,    50,    27,    85,    59,
     111,   216,   220,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,    43,  -514,   135,    27,   229,  -514,  -514,
    1218,   255,    79,    26,   258,   393,  -514,  -514,  -514,  -514,
    1294,    33,  -514,  -514,  -514,  -514,  -514,  -514,   263,   300,
    -514,    55,   256,   232,   327,  -514,  -514,   990,   296,  -514,
    -514,  -514,  -514,  -514,    70,  -514,  -514,   336,   340,  -514,
     345,   348,  -514,   368,  -514,  -514,  -514,  -514,  1370,  -514,
    -514,  -514,  -514,  1066,  -514,  -514,  1294,  1294,  1294,  1370,
    1370,   222,    37,  -514,   370,  -514,    58,  -514,  -514,  -514,
    -514,   380,  -514,   392,  -514,  -514,  -514,  -514,  -514,   264,
       4,   213,  -514,  -514,  -514,  -514,  -514,    35,  -514,   242,
    -514,  -514,  -514,  -514,    31,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,   127,  -514,   258,   246,   401,   246,
     285,  -514,   199,  -514,  -514,  -514,  -514,   154,    14,   342,
     354,  -514,   198,  -514,  -514,   282,   152,   152,   323,  -514,
    -514,  -514,   127,   407,   135,    50,  -514,   686,   331,  -514,
     415,   427,    14,   363,   444,    19,  -514,  -514,  -514,  1294,
      32,    50,    50,   686,  -514,   686,  -514,   112,  -514,   359,
     282,  -514,  -514,  -514,  -514,  -514,    27,   317,  -514,   372,
    1294,  -514,  -514,  -514,  -514,   914,    50,   386,   386,   386,
     386,   386,   386,   386,   386,   386,  -514,   435,  1294,  -514,
    -514,   362,    14,    14,  -514,   246,  -514,   441,   327,  1294,
    -514,   446,  -514,   258,   258,    27,   419,   454,    77,  -514,
     350,  -514,    27,  -514,  1294,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,   477,  -514,   459,   331,  -514,  -514,   269,  -514,
    1218,   534,   610,   481,   462,   481,  -514,  -514,  -514,  -514,
    -514,  -514,   247,    67,   429,  -514,  -514,   369,   371,  -514,
    -514,  1294,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  1404,  1442,  1488,  1390,  1526,   463,
    -514,  -514,  -514,  -514,   303,  -514,  -514,   349,  -514,  -514,
    -514,  -514,   464,   282,   490,  1294,  1294,  1294,    38,   -21,
    1294,  -514,   405,   686,   404,  -514,   359,   495,   512,    63,
    -514,  -514,  -514,   450,  -514,  -514,  1294,  -514,   373,   282,
    -514,  -514,  1294,  -514,  -514,  -514,   264,   264,     4,     4,
       4,     4,  -514,  -514,  -514,  -514,   489,  -514,  -514,  -514,
     397,   500,  -514,  -514,   512,    27,    14,   331,  1294,   443,
    -514,  -514,  -514,   306,  -514,   476,   485,   465,    29,    21,
    -514,  -514,  -514,   445,  -514,   509,  1294,   223,  1142,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,   462,   686,   686,  -514,
     383,   463,  -514,   415,  -514,    14,  1489,  -514,   515,   413,
     422,  1294,  1294,  -514,  -514,    47,    80,  -514,  1294,  -514,
     517,   518,   520,    93,  -514,   436,  -514,    27,   317,  -514,
     491,   478,   524,  -514,  1294,   527,  -514,   435,  -514,   512,
    -514,  -514,  -514,   282,   686,    81,  -514,  -514,  -514,   508,
     511,   530,  -514,  -514,  -514,   990,  -514,  -514,    10,    17,
    1294,   322,  -514,  -514,  -514,  -514,   481,   218,  -514,   369,
     266,  -514,  -514,   525,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
     514,  -514,   537,  -514,  -514,  -514,   539,  -514,    99,   838,
    1294,  -514,   512,  -514,  1294,   686,   479,  -514,  -514,  -514,
     424,  -514,   371,   435,   347,   246,  -514,  -514,   127,   431,
     282,  1294,  -514,  -514,   430,  -514,  -514,  -514,    40,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,    58,  -514,   522,
    -514,  -514,  1294,    62,  -514,  1294,  -514,  -514,  -514,  -514,
    -514,   543,   371,   526,   456,  -514,   435,   433,  -514,  -514,
    -514,  -514,   546,  -514,  1294,   426,  1294,  -514,   229,   440,
    -514,  -514,   556,  1294,  -514,   762,   386,   331,  -514,  -514,
    -514,  -514,   282,  -514,  -514,  -514,    40,  -514,   558,  -514,
     179,    89,   205,  -514,  -514,   762,  -514,  -514,  -514,  -514,
     386,  -514,  -514,   213
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
      31,     0,    86,     0,    39,   332,   331,     0,     0,   379,
     201,   198,   199,   200,   345,   230,   227,     0,     0,   213,
       0,     0,   212,     0,   231,   228,   214,   351,     0,   306,
     308,   307,   229,     0,   197,   328,     0,     0,     0,     0,
       0,   350,    97,   255,    91,   193,   210,   281,   209,   217,
     282,     0,   216,     0,   284,   287,   288,   289,   293,   297,
     300,   320,   312,   305,   313,   314,   322,   206,   321,   323,
     325,   329,   324,   195,   279,   347,   283,   285,   354,   355,
     356,   211,   110,   109,     0,   112,     0,   380,   343,   104,
     338,   339,     0,   234,   239,   232,   233,   238,     0,   240,
       0,   350,     0,    69,    71,    72,     0,     0,     8,    11,
      12,   254,     0,     0,     0,    75,    32,    50,   382,    87,
       0,     0,     0,     0,     0,     0,   378,   346,   349,     0,
      75,    75,    75,    50,   292,    50,   377,   373,   370,     0,
       0,   309,   311,   310,   291,   290,   106,     0,    88,    86,
       0,   204,   205,   203,   202,     0,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   353,     0,     0,   330,
     208,     0,     0,     0,   352,   104,   103,     0,    39,     0,
     342,     0,   114,     0,     0,   106,     0,   345,    86,    78,
       0,   241,   106,    68,     0,    15,    16,    17,    20,    21,
      19,    18,     0,    13,     0,   382,    61,    28,     0,    30,
       0,    50,    50,     0,    50,     0,   162,   163,   159,   158,
     161,   160,     0,    50,   134,   136,   138,   140,   117,   127,
     146,     0,   147,   171,   172,   153,   177,   151,   182,   152,
     148,   154,   149,   150,   128,   129,   131,   132,   130,     0,
      66,   383,   336,   335,   336,   333,   334,     0,    36,    67,
     192,   286,     0,   256,     0,     0,     0,     0,     0,     0,
       0,   371,     0,    50,   373,   359,     0,    99,   100,    97,
      83,    85,   196,   118,   194,   218,     0,   318,     0,   376,
     317,   375,     0,   294,   295,   296,   298,   299,   303,   304,
     301,   302,   207,   327,   326,   348,   280,   113,   381,   115,
       0,    37,   341,   340,   242,   106,     0,   382,     0,   238,
      70,    14,    22,     9,     7,     0,     0,    97,     0,    72,
      51,   137,   190,   164,   191,     0,     0,     0,     0,   166,
     169,   170,   176,   181,   184,   185,    50,    50,    50,   157,
     387,   384,   386,     0,    40,     0,     0,   225,     0,     0,
       0,     0,     0,   374,   358,     0,    71,   365,     0,   366,
     171,   177,   182,     0,   362,     0,   360,   106,     0,    74,
       0,   120,     0,   219,     0,     0,   344,     0,   105,   243,
      79,    82,    80,    81,    50,     0,   236,   244,   237,     0,
       0,     0,    27,   133,   175,     0,   180,   178,   173,   173,
       0,     0,   188,   187,   189,   156,     0,     0,   139,   141,
       0,   385,    35,   272,   263,   262,   264,   265,   259,   260,
     266,   261,   267,   271,   257,   258,   268,   269,   270,   273,
     276,   277,     0,   226,   222,   223,     0,   372,   157,    50,
       0,   357,   101,    84,     0,    50,    41,   220,   319,   224,
       0,    34,   235,     0,   247,   104,    24,    23,     0,     0,
     315,     0,   168,   167,     0,   221,   186,   183,   106,   123,
     125,   122,   124,   135,   126,   390,   391,   389,   388,     0,
     275,   278,     0,     0,   364,     0,   368,   369,   367,   363,
     361,   119,   121,     0,    92,    38,     0,     0,   248,   249,
     246,   337,     0,   179,     0,   174,     0,   107,     0,     0,
     143,   274,     0,     0,   155,    50,     0,   382,    93,    33,
     252,    29,   316,   165,   145,   142,   106,   215,    42,    43,
      71,    47,    94,    98,   144,    50,    48,    49,    46,    45,
       0,    96,    44,    95
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,   411,  -514,
    -514,  -514,   412,  -514,  -514,    56,  -514,  -514,   351,  -514,
    -514,   -20,   -13,  -232,  -514,     3,  -514,  -514,  -514,  -514,
     410,  -514,   -39,   -79,  -514,  -514,    54,  -514,   214,  -514,
    -514,  -514,   156,  -122,  -514,   325,  -514,  -514,  -514,   257,
    -514,  -514,   451,  -211,  -514,  -483,   554,   -18,  -514,  -514,
    -514,   119,  -514,   328,   195,   196,  -155,  -514,  -514,     7,
    -386,  -514,  -514,  -256,   326,  -514,  -253,   329,  -514,   136,
    -251,  -514,  -514,  -247,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,   -24,  -514,   396,  -514,   131,  -514,  -143,  -514,
    -514,  -148,  -514,  -514,  -514,  -514,  -514,  -514,   457,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -136,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,   217,   123,   173,   253,   151,  -513,
    -514,  -514,  -514,  -514,  -514,   103,  -267,  -135,   -30,   -81,
    -514,  -514,  -514,   390,  -514,    46,  -514,   455,   458,  -514,
    -123,  -514,   234,  -514,  -514,   414,   417,  -133,  -115,  -514,
    -514,  -514,  -514,  -514,   289,  -514,   128,   447,  -166,   311,
    -302,  -514,  -514,  -181,  -514,  -244,  -514,  -514,   240,  -514
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,   255,   158,   159,   160,   252,   373,
      25,   258,    50,    51,    13,   510,   307,   438,   173,   554,
     588,   589,   598,   272,    14,   174,    48,    16,    17,    18,
      19,   152,   273,   154,    20,    21,    30,   238,   239,   442,
      22,   329,   330,   168,    23,    92,   577,   601,   578,   198,
     199,   327,   231,   232,    32,    33,    34,    35,   169,   431,
     506,   533,   274,   275,   276,   277,   319,   534,   569,   570,
     279,   417,   280,   281,   419,   282,   283,   284,   285,   522,
     286,   287,   456,   288,   289,   394,   290,   291,   395,   466,
     292,   293,    93,    94,    95,   331,    96,   219,    97,    98,
      99,   100,   101,   102,   103,   149,   446,   447,   150,    36,
      37,    38,   560,    39,   448,   162,   104,   312,   489,   490,
     491,   492,   105,   224,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   519,   337,   338,   339,   118,
     119,   120,   121,   122,   123,    54,    55,    56,   306,   514,
     139,   140,   141,   230,   178,   124,   125,   126,   511,   127,
     128,   129,   130,   324,   325,   423,   424,   187,   188,   322,
     340,   341,   189,   131,   228,   300,   301,   401,   402,   538
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     117,   153,   220,    15,   186,   191,   192,   193,    52,   137,
     155,   374,   278,   225,   357,   240,   332,   388,   413,   295,
     390,   321,   392,   311,   294,   311,   393,   175,   318,   134,
    -108,   296,   383,   453,   298,   295,   156,   295,   237,   308,
     294,   387,   294,   411,   222,   568,    24,   296,   408,   296,
     298,  -108,   298,   155,   166,   521,   190,   190,   190,   216,
     164,    28,   521,   592,  -108,    26,   217,   418,   428,   432,
     420,   412,   421,  -221,    29,   435,   422,   333,   201,    28,
     202,   460,   366,    41,   513,  -364,   196,   603,    40,    45,
     380,   415,    29,   197,   360,   177,   460,   398,   499,    77,
     385,    46,   352,   568,  -155,   138,   378,   386,   132,   496,
     497,    42,   196,   295,   295,   385,   367,    27,   294,   294,
      47,   596,   573,   441,   264,   296,   296,   165,   298,   298,
     597,  -288,  -288,  -288,  -288,   210,   211,   297,     8,  -132,
    -132,  -288,  -288,  -288,  -288,  -288,  -288,   398,   151,   313,
    -321,   223,   218,   297,   218,   297,   398,   135,    31,    49,
     321,   167,   203,   157,   467,   256,   314,   508,   218,   204,
     117,   567,  -364,   245,   246,   295,   247,   133,   328,   -47,
     294,   409,   410,   -47,   -47,   500,   -47,   296,   353,   591,
     298,  -155,   266,   267,   268,   269,   270,   271,   550,   165,
     248,   184,   243,   244,   143,   370,   185,   320,   144,   591,
       8,   596,   194,   195,   155,   145,   146,   364,   249,   260,
     597,   528,   -90,    43,   369,   -47,   -90,    44,   250,   -90,
     117,   379,   297,   240,   315,   316,   317,   -47,   170,   382,
     572,   384,   399,   545,   416,   171,   546,   332,   547,   295,
     295,   155,   548,    53,   294,   294,   -47,   460,   235,   336,
     342,   296,   296,   251,   298,   298,   529,   543,   -90,   528,
    -102,   -90,   472,   375,   376,   -47,   385,   226,   -90,  -112,
     -90,   -47,   138,   386,   530,    60,    61,    62,    63,   512,
     535,   233,   161,   297,   531,   142,   295,   532,   234,   -90,
     385,   294,   600,   163,   561,   459,   143,   386,   296,   170,
     144,   298,   449,   526,   529,  -238,   403,   145,   146,   450,
     176,   147,   515,    58,   461,    59,   212,   213,   214,   215,
     172,     1,   530,   593,   212,   213,   214,   215,   443,   179,
     167,     2,   531,   180,     3,   532,   525,   439,   181,   465,
     552,   182,    65,   404,   405,    66,   190,   295,   155,     4,
      68,   457,   294,   348,   349,   350,   351,   297,   297,   296,
      84,   183,   298,   221,     5,   200,     6,   433,   434,   498,
     343,   344,   345,   205,     7,    70,    71,     8,   155,    57,
     558,   559,    58,    73,    59,   206,    74,    75,   207,   208,
     209,   436,   434,     9,   229,    60,    61,    62,    63,   502,
      64,   241,    82,   242,   297,   218,    65,   494,   434,    66,
     544,   524,    67,   254,    68,   520,   495,   434,   555,   556,
     155,   257,   142,   299,    69,   563,   564,   580,   556,   302,
     565,   579,   527,   143,   585,   586,     8,   144,   310,    70,
      71,   304,    72,   323,   145,   146,   167,    73,   147,   216,
      74,    75,    76,   346,   347,   358,   148,   362,   363,   155,
     361,    77,    78,    85,   520,   297,    82,    83,   365,   177,
     368,   371,   142,   372,   134,   380,   396,   400,   397,   398,
      84,   190,   459,   143,   407,   406,   590,   144,   414,   320,
     427,   430,   222,   437,   145,   146,   574,   451,   147,   452,
     444,   454,   455,   470,   196,   155,   590,    89,    90,   493,
     151,   562,  -368,  -369,   445,  -367,   504,   583,   507,   501,
     505,   509,   516,   518,   582,   517,   155,   261,   539,   540,
      58,   541,    59,   190,   542,   155,   571,   553,   564,   566,
     581,   142,   576,    60,    61,    62,    63,   262,    64,  -321,
     587,   575,   143,   595,    65,   155,   144,    66,   253,   557,
      67,  -238,    68,   145,   146,   602,   259,   147,   599,   359,
     440,   263,    69,   309,   503,   377,   429,   136,   227,   536,
     381,   264,   468,   594,   469,   523,   334,    70,    71,   389,
      72,   537,   391,   458,   236,    73,   165,   551,    74,    75,
      76,   354,   265,   261,   584,   426,    58,     8,    59,    77,
      78,    79,    80,    81,    82,    83,   305,   549,   303,    60,
      61,    62,    63,   380,    64,   425,   326,   356,    84,   355,
      65,   471,     0,    66,   -51,    85,    67,     0,    68,    86,
      87,    88,     0,     0,     0,     0,     0,   263,    69,   266,
     267,   268,   269,   270,   271,    89,    90,   264,   151,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
       0,    73,     0,     0,    74,    75,    76,     0,   265,   261,
       0,     0,    58,     0,    59,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,    60,    61,    62,    63,   262,
      64,     0,     0,     0,    84,     0,    65,     0,     0,    66,
       0,    85,    67,     0,    68,    86,    87,    88,     0,     0,
       0,     0,     0,   263,    69,   266,   267,   268,   269,   270,
     271,    89,    90,   264,   151,     0,     0,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      74,    75,    76,     0,   265,    57,     0,     0,    58,     0,
      59,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,    60,    61,    62,    63,   380,    64,     0,     0,     0,
      84,     0,    65,     0,     0,    66,     0,    85,    67,     0,
      68,    86,    87,    88,     0,     0,     0,     0,     0,   263,
      69,   266,   267,   268,   269,   270,   271,    89,    90,   264,
     151,     0,     0,     0,     0,    70,    71,     0,    72,     0,
       0,     0,     0,    73,     0,     0,    74,    75,    76,     0,
     265,    57,     0,     0,    58,     0,    59,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    60,    61,    62,
      63,   380,    64,     0,     0,     0,    84,     0,    65,     0,
       0,    66,     0,    85,    67,     0,    68,    86,    87,    88,
       0,     0,     0,     0,     0,     0,    69,   266,   267,   268,
     269,   270,   271,    89,    90,   264,   151,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,     0,    57,   335,     0,
      58,     0,    59,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,    60,    61,    62,    63,     0,    64,     0,
       0,     0,    84,    28,    65,     0,     0,    66,     0,    85,
      67,     0,    68,    86,    87,    88,    29,     0,     0,     0,
       0,     0,    69,   266,   267,   268,   269,   270,   271,    89,
      90,     0,   151,     0,     0,     0,     0,    70,    71,     0,
      72,     0,     0,     0,     0,    73,     0,     0,    74,    75,
      76,     0,     0,    57,     0,     0,    58,     0,    59,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,    60,
      61,    62,    63,     0,    64,     0,     0,     0,    84,     0,
      65,     0,     0,    66,     0,    85,    67,     0,    68,    86,
      87,    88,     0,     0,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,    89,    90,     0,   151,     0,
       0,     0,     0,    70,    71,     0,    72,     0,     0,     0,
       0,    73,   165,     0,    74,    75,    76,     0,     0,    57,
       0,     0,    58,     8,    59,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,    60,    61,    62,    63,     0,
      64,     0,     0,     0,    84,     0,    65,     0,     0,    66,
       0,    85,    67,     0,    68,    86,    87,    88,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,    89,    90,     0,   151,     0,     0,     0,     0,    70,
      71,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      74,    75,    76,     0,     0,    57,     0,     0,    58,     0,
      59,    77,    78,    79,    80,    81,    82,    83,     0,     0,
     185,    60,    61,    62,    63,     0,    64,     0,     0,     0,
      84,   462,   463,     0,     0,    66,     0,    85,    67,     0,
      68,    86,    87,    88,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,    89,    90,     0,
     151,     0,     0,     0,     0,    70,    71,     0,    72,     0,
       0,     0,     0,    73,     0,     0,   464,    75,    76,     0,
       0,    57,     0,     0,    58,     0,    59,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    60,    61,    62,
      63,     0,    64,     0,     0,     0,    84,     0,    65,     0,
       0,    66,     0,    85,    67,     0,    68,    86,    87,    88,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,    89,    90,     0,   151,     0,     0,     0,
       0,    70,    71,     0,    72,     0,     0,     0,     0,    73,
       0,     0,    74,    75,    76,     0,     0,    57,     0,     0,
      58,     0,    59,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,    60,    61,    62,    63,     0,    64,     0,
       0,     0,    84,     0,    65,     0,     0,    66,     0,    85,
      67,     0,    68,    86,    87,    88,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,    89,
      90,     0,    91,     0,     0,     0,     0,    70,    71,     0,
      72,     0,     0,     0,     0,    73,     0,     0,    74,    75,
      76,     0,     0,    57,     0,     0,    58,     0,    59,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,    60,
      61,    62,    63,     0,    64,   -72,     0,     0,    84,     0,
      65,     0,     0,    66,     0,    85,    67,     0,    68,    86,
      87,    88,     0,   -72,     0,     0,     0,     0,    69,     0,
       0,     0,     0,     0,   -72,    89,    90,  -281,   151,     0,
       0,     0,     0,    70,    71,     0,    72,     0,  -281,     0,
       0,    73,     0,   -72,    74,    75,    76,   -72,     0,     0,
     -72,     0,     0,     0,     0,    77,     0,  -281,     0,     0,
      82,    83,     0,     0,  -281,  -282,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,  -282,     0,     0,     0,
       0,     0,   -72,     0,     0,     0,     0,     0,     0,     0,
       0,   -72,     0,     0,     0,  -282,     0,     0,     0,     0,
       0,     0,  -282,     0,   151,  -281,     0,     0,     0,     0,
       0,  -284,     0,   473,     0,   -72,   -72,   -72,   -72,   -72,
     -72,     0,  -284,   218,     0,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -284,     0,  -282,     0,     0,     0,     0,  -284,  -283,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -283,     0,     0,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -283,
       0,     0,     0,     0,     0,     0,  -283,     0,     0,  -284,
       0,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,     0,     0,  -284,
    -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,  -284,
    -284,  -284,  -284,  -284,  -284,     0,     0,  -283,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -283,  -283,  -283,
    -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,
    -283,  -283,  -283
};

static const yytype_int16 yycheck[] =
{
      30,    40,   117,     0,    83,    86,    87,    88,    26,    33,
      40,   255,   167,   136,   225,   148,   197,   273,   320,   167,
     273,   187,   273,     4,   167,     4,   273,    57,   183,     3,
       3,   167,   264,     4,   167,   183,     3,   185,    24,   172,
     183,   273,   185,     5,    13,   528,    28,   183,   315,   185,
     183,    24,   185,    83,    51,    45,    86,    87,    88,    24,
       5,    29,    45,   576,    24,    49,    31,   323,     5,   336,
     323,    92,   323,     3,    42,   342,   323,   199,    20,    29,
      22,    34,     5,    24,     3,     5,    49,   600,     3,    46,
      23,   323,    42,    56,   229,    25,    34,   118,     5,    85,
      53,    58,   217,   586,     5,    24,   261,    60,    29,   411,
     412,     0,    49,   261,   262,    53,   238,    56,   261,   262,
      77,    32,    60,   367,    57,   261,   262,    72,   261,   262,
      41,   121,   122,   123,   124,   131,   132,   167,    83,   118,
     119,   131,   132,   133,   134,   135,   136,   118,   134,   179,
     133,   120,   133,   183,   133,   185,   118,   131,   131,    24,
     326,    84,   104,   130,   396,   162,   134,   434,   133,   111,
     200,   131,    92,    21,    22,   323,    24,    98,   196,     0,
     323,   316,   317,     4,     5,    92,     7,   323,   218,   575,
     323,    92,   125,   126,   127,   128,   129,   130,   500,    72,
      48,    78,     4,     5,    50,   244,    94,    95,    54,   595,
      83,    32,    89,    90,   244,    61,    62,   235,    66,   165,
      41,     3,     0,     7,   242,    46,     4,     7,    76,     7,
     260,   261,   262,   366,   180,   181,   182,    58,     6,   263,
     542,   265,   281,   499,   323,    13,   499,   428,   499,   397,
     398,   281,   499,    24,   397,   398,    77,    34,    59,   205,
     206,   397,   398,   111,   397,   398,    48,   499,    46,     3,
      24,    49,   405,     4,     5,    96,    53,    31,    56,    24,
      58,   102,    24,    60,    66,    19,    20,    21,    22,   444,
      24,     6,    29,   323,    76,    39,   444,    79,    13,    77,
      53,   444,    97,     3,   515,   386,    50,    60,   444,     6,
      54,   444,     6,   461,    48,    59,    13,    61,    62,    13,
      24,    65,   445,     6,   101,     8,   121,   122,   123,   124,
       3,    27,    66,   577,   121,   122,   123,   124,   368,     3,
      84,    37,    76,     3,    40,    79,    24,   365,     3,   388,
     505,     3,    30,     4,     5,    33,   386,   505,   388,    55,
      38,   385,   505,   212,   213,   214,   215,   397,   398,   505,
     104,     3,   505,   131,    70,     5,    72,     4,     5,   418,
     207,   208,   209,     3,    80,    63,    64,    83,   418,     3,
      43,    44,     6,    71,     8,     3,    74,    75,   134,   135,
     136,     4,     5,    99,     3,    19,    20,    21,    22,   427,
      24,    69,    90,    59,   444,   133,    30,     4,     5,    33,
     499,   460,    36,   100,    38,   455,     4,     5,     4,     5,
     460,    24,    39,   102,    48,     4,     5,     4,     5,    24,
     521,   556,   466,    50,     4,     5,    83,    54,     4,    63,
      64,    24,    66,    94,    61,    62,    84,    71,    65,    24,
      74,    75,    76,   210,   211,    24,    73,   233,   234,   499,
      24,    85,    86,   111,   504,   505,    90,    91,    59,    25,
     130,     4,    39,    24,     3,    23,    57,    24,   119,   118,
     104,   521,   573,    50,     4,    31,   575,    54,    93,    95,
       5,    51,    13,     3,    61,    62,   545,    31,    65,    24,
      67,    66,     3,   130,    49,   545,   595,   131,   132,     4,
     134,   518,     5,     5,    81,     5,    35,   566,     4,    93,
      52,     4,    24,     3,   564,    24,   566,     3,    13,    25,
       6,     4,     8,   573,     5,   575,    24,    68,     5,   119,
       4,    39,    96,    19,    20,    21,    22,    23,    24,   133,
       4,    35,    50,     5,    30,   595,    54,    33,   157,   513,
      36,    59,    38,    61,    62,   595,   164,    65,   591,   228,
     366,    47,    48,   173,   428,   260,   329,    33,   137,   470,
     262,    57,   397,   586,   398,   459,   200,    63,    64,   273,
      66,   470,   273,   386,   147,    71,    72,   504,    74,    75,
      76,   221,    78,     3,   568,   326,     6,    83,     8,    85,
      86,    87,    88,    89,    90,    91,   171,   499,   170,    19,
      20,    21,    22,    23,    24,   324,   189,   223,   104,   222,
      30,   401,    -1,    33,    34,   111,    36,    -1,    38,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    47,    48,   125,
     126,   127,   128,   129,   130,   131,   132,    57,   134,    -1,
      -1,    -1,    -1,    63,    64,    -1,    66,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    74,    75,    76,    -1,    78,     3,
      -1,    -1,     6,    -1,     8,    85,    86,    87,    88,    89,
      90,    91,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,   104,    -1,    30,    -1,    -1,    33,
      -1,   111,    36,    -1,    38,   115,   116,   117,    -1,    -1,
      -1,    -1,    -1,    47,    48,   125,   126,   127,   128,   129,
     130,   131,   132,    57,   134,    -1,    -1,    -1,    -1,    63,
      64,    -1,    66,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      74,    75,    76,    -1,    78,     3,    -1,    -1,     6,    -1,
       8,    85,    86,    87,    88,    89,    90,    91,    -1,    -1,
      -1,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
     104,    -1,    30,    -1,    -1,    33,    -1,   111,    36,    -1,
      38,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    47,
      48,   125,   126,   127,   128,   129,   130,   131,   132,    57,
     134,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    74,    75,    76,    -1,
      78,     3,    -1,    -1,     6,    -1,     8,    85,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,   104,    -1,    30,    -1,
      -1,    33,    -1,   111,    36,    -1,    38,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    48,   125,   126,   127,
     128,   129,   130,   131,   132,    57,   134,    -1,    -1,    -1,
      -1,    63,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    74,    75,    76,    -1,    -1,     3,     4,    -1,
       6,    -1,     8,    85,    86,    87,    88,    89,    90,    91,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,   104,    29,    30,    -1,    -1,    33,    -1,   111,
      36,    -1,    38,   115,   116,   117,    42,    -1,    -1,    -1,
      -1,    -1,    48,   125,   126,   127,   128,   129,   130,   131,
     132,    -1,   134,    -1,    -1,    -1,    -1,    63,    64,    -1,
      66,    -1,    -1,    -1,    -1,    71,    -1,    -1,    74,    75,
      76,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,    85,
      86,    87,    88,    89,    90,    91,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    24,    -1,    -1,    -1,   104,    -1,
      30,    -1,    -1,    33,    -1,   111,    36,    -1,    38,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,    -1,
      -1,    -1,    -1,    63,    64,    -1,    66,    -1,    -1,    -1,
      -1,    71,    72,    -1,    74,    75,    76,    -1,    -1,     3,
      -1,    -1,     6,    83,     8,    85,    86,    87,    88,    89,
      90,    91,    -1,    -1,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,   104,    -1,    30,    -1,    -1,    33,
      -1,   111,    36,    -1,    38,   115,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,    -1,    -1,    -1,    -1,    63,
      64,    -1,    66,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      74,    75,    76,    -1,    -1,     3,    -1,    -1,     6,    -1,
       8,    85,    86,    87,    88,    89,    90,    91,    -1,    -1,
      94,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
     104,    29,    30,    -1,    -1,    33,    -1,   111,    36,    -1,
      38,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    74,    75,    76,    -1,
      -1,     3,    -1,    -1,     6,    -1,     8,    85,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,   104,    -1,    30,    -1,
      -1,    33,    -1,   111,    36,    -1,    38,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,    -1,    -1,    -1,
      -1,    63,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    74,    75,    76,    -1,    -1,     3,    -1,    -1,
       6,    -1,     8,    85,    86,    87,    88,    89,    90,    91,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    24,    -1,
      -1,    -1,   104,    -1,    30,    -1,    -1,    33,    -1,   111,
      36,    -1,    38,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,    -1,    -1,    -1,    -1,    63,    64,    -1,
      66,    -1,    -1,    -1,    -1,    71,    -1,    -1,    74,    75,
      76,    -1,    -1,     3,    -1,    -1,     6,    -1,     8,    85,
      86,    87,    88,    89,    90,    91,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    24,     5,    -1,    -1,   104,    -1,
      30,    -1,    -1,    33,    -1,   111,    36,    -1,    38,   115,
     116,   117,    -1,    23,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    34,   131,   132,    23,   134,    -1,
      -1,    -1,    -1,    63,    64,    -1,    66,    -1,    34,    -1,
      -1,    71,    -1,    53,    74,    75,    76,    57,    -1,    -1,
      60,    -1,    -1,    -1,    -1,    85,    -1,    53,    -1,    -1,
      90,    91,    -1,    -1,    60,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    34,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,   134,   101,    -1,    -1,    -1,    -1,
      -1,    23,    -1,    24,    -1,   125,   126,   127,   128,   129,
     130,    -1,    34,   133,    -1,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,    53,    -1,   101,    -1,    -1,    -1,    -1,    60,    23,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    -1,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,   101,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    27,    37,    40,    55,    70,    72,    80,    83,    99,
     140,   141,   142,   153,   163,   164,   166,   167,   168,   169,
     173,   174,   179,   183,    28,   149,    49,    56,    29,    42,
     175,   131,   193,   194,   195,   196,   248,   249,   250,   252,
       3,    24,     0,     7,     7,    46,    58,    77,   165,    24,
     151,   152,   196,    24,   284,   285,   286,     3,     6,     8,
      19,    20,    21,    22,    24,    30,    33,    36,    38,    48,
      63,    64,    66,    71,    74,    75,    76,    85,    86,    87,
      88,    89,    90,    91,   104,   111,   115,   116,   117,   131,
     132,   134,   184,   231,   232,   233,   235,   237,   238,   239,
     240,   241,   242,   243,   255,   261,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   277,   278,   279,
     280,   281,   282,   283,   294,   295,   296,   298,   299,   300,
     301,   312,    29,    98,     3,   131,   195,   231,    24,   289,
     290,   291,    39,    50,    54,    61,    62,    65,    73,   244,
     247,   134,   170,   171,   172,   277,     3,   130,   144,   145,
     146,    29,   254,     3,     5,    72,   164,    84,   182,   197,
       6,    13,     3,   157,   164,   277,    24,    25,   293,     3,
       3,     3,     3,     3,   264,    94,   172,   306,   307,   311,
     277,   278,   278,   278,   264,   264,    49,    56,   188,   189,
       5,    20,    22,   104,   111,     3,     3,   134,   135,   136,
     131,   132,   121,   122,   123,   124,    24,    31,   133,   236,
     297,   131,    13,   120,   262,   289,    31,   191,   313,     3,
     292,   191,   192,     6,    13,    59,   247,    24,   176,   177,
     296,    69,    59,     4,     5,    21,    22,    24,    48,    66,
      76,   111,   147,   147,   100,   143,   164,    24,   150,   151,
     175,     3,    23,    47,    57,    78,   125,   126,   127,   128,
     129,   130,   162,   171,   201,   202,   203,   204,   205,   209,
     211,   212,   214,   215,   216,   217,   219,   220,   222,   223,
     225,   226,   229,   230,   237,   240,   255,   277,   296,   102,
     314,   315,    24,   287,    24,   286,   287,   155,   296,   169,
       4,     4,   256,   277,   134,   175,   175,   175,   205,   205,
      95,   307,   308,    94,   302,   303,   306,   190,   196,   180,
     181,   234,   312,   182,   233,     4,   175,   275,   276,   277,
     309,   310,   175,   265,   265,   265,   266,   266,   267,   267,
     267,   267,   297,   277,   282,   295,   294,   192,    24,   157,
     276,    24,   291,   291,   196,    59,     5,   182,   130,   196,
     171,     4,    24,   148,   314,     4,     5,   184,   205,   277,
      23,   202,   231,   162,   231,    53,    60,   162,   212,   213,
     215,   216,   219,   222,   224,   227,    57,   119,   118,   171,
      24,   316,   317,    13,     4,     5,    31,     4,   275,   276,
     276,     5,    92,   309,    93,   162,   172,   210,   212,   213,
     215,   219,   222,   304,   305,   308,   303,     5,     5,   188,
      51,   198,   275,     4,     5,   275,     4,     3,   156,   196,
     177,   314,   178,   277,    67,    81,   245,   246,   253,     6,
      13,    31,    24,     4,    66,     3,   221,   231,   263,   278,
      34,   101,    29,    30,    74,   171,   228,   162,   203,   204,
     130,   317,   296,    24,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   257,
     258,   259,   260,     4,     4,     4,   309,   309,   171,     5,
      92,    93,   196,   181,    35,    52,   199,     4,   275,     4,
     154,   297,   205,     3,   288,   289,    24,    24,     3,   274,
     277,    45,   218,   218,   171,    24,   240,   231,     3,    48,
      66,    76,    79,   200,   206,    24,   200,   235,   318,    13,
      25,     4,     5,   162,   172,   212,   215,   219,   222,   305,
     309,   274,   205,    68,   158,     4,     5,   154,    43,    44,
     251,   192,   164,     4,     5,   278,   119,   131,   194,   207,
     208,    24,   309,    60,   171,    35,    96,   185,   187,   297,
       4,     4,   277,   171,   284,     4,     5,     4,   159,   160,
     172,   209,   268,   314,   208,     5,    32,    41,   161,   161,
      97,   186,   160,   268
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   139,   140,   140,   140,   140,   141,   142,   143,   143,
     144,   144,   144,   145,   146,   147,   147,   147,   147,   147,
     147,   147,   148,   148,   148,   149,   149,   150,   150,   151,
     152,   152,   153,   154,   154,   155,   155,   156,   156,   157,
     157,   158,   158,   159,   159,   160,   160,   161,   161,   161,
     162,   162,   163,   163,   163,   163,   163,   163,   163,   163,
     164,   164,   165,   165,   165,   166,   167,   168,   169,   170,
     170,   171,   172,   173,   174,   175,   175,   175,   176,   176,
     177,   178,   179,   180,   180,   181,   182,   182,   183,   183,
     184,   184,   185,   185,   186,   186,   187,   188,   188,   189,
     190,   190,   191,   191,   192,   192,   193,   193,   194,   194,
     194,   195,   195,   196,   196,   196,   196,   197,   198,   198,
     199,   199,   200,   200,   200,   200,   200,   201,   201,   201,
     201,   201,   201,   201,   202,   202,   203,   203,   204,   204,
     205,   205,   206,   207,   207,   208,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   210,   211,   211,   212,   212,
     212,   212,   212,   212,   212,   213,   214,   215,   216,   217,
     217,   217,   217,   218,   218,   219,   220,   220,   221,   221,
     222,   223,   223,   224,   225,   226,   227,   228,   228,   228,
     229,   230,   231,   232,   232,   233,   234,   235,   235,   235,
     235,   235,   235,   235,   235,   235,   236,   236,   236,   237,
     237,   238,   238,   238,   238,   239,   240,   240,   240,   240,
     240,   241,   242,   242,   242,   242,   242,   243,   243,   243,
     243,   243,   244,   244,   244,   245,   246,   246,   247,   247,
     247,   247,   248,   249,   249,   249,   250,   251,   251,   251,
     252,   252,   253,   254,   254,   255,   256,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   258,   258,   259,   260,   260,   261,   262,
     262,   263,   263,   263,   263,   263,   263,   263,   264,   265,
     265,   265,   265,   266,   266,   266,   266,   267,   267,   267,
     268,   268,   268,   268,   268,   269,   270,   270,   270,   270,
     270,   270,   271,   272,   273,   274,   274,   275,   276,   276,
     277,   277,   277,   278,   279,   279,   280,   280,   281,   282,
     283,   284,   284,   285,   286,   286,   287,   288,   289,   290,
     290,   290,   291,   292,   292,   293,   293,   294,   294,   295,
     295,   295,   296,   297,   298,   299,   299,   300,   301,   302,
     302,   303,   304,   304,   305,   305,   305,   305,   305,   305,
     306,   306,   307,   308,   308,   309,   310,   311,   312,   312,
     313,   313,   314,   314,   315,   316,   316,   317,   317,   318,
     318,   318
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     5,     0,     2,
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

  case 246: /* ecrelationship_join: table_ref join_type SQL_TOKEN_JOIN table_ref SQL_TOKEN_USING table_node_ref op_relationship_direction  */
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

  case 337: /* table_node_ref: table_node_with_opt_member_func_call table_primary_as_range_column  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 338: /* table_node_with_opt_member_func_call: table_node_path  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 339: /* table_node_path: table_node_path_entry  */
            {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 340: /* table_node_path: table_node_path '.' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 341: /* table_node_path: table_node_path ':' table_node_path_entry  */
            {
            (yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-2].pParseNode);
            }
    break;

  case 342: /* table_node_path_entry: SQL_TOKEN_NAME opt_member_function_args  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 343: /* opt_member_function_args: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 344: /* opt_member_function_args: '(' function_args_commalist ')'  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode) = CREATE_NODE("(", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE(")", SQL_NODE_PUNCTUATION));
		}
    break;

  case 345: /* opt_column_array_idx: %empty  */
                {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 346: /* opt_column_array_idx: SQL_TOKEN_ARRAY_INDEX  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 347: /* property_path: property_path_entry  */
        {
            (yyval.pParseNode) = SQL_NEW_DOTLISTRULE;
            (yyval.pParseNode)->append ((yyvsp[0].pParseNode));
        }
    break;

  case 348: /* property_path: property_path '.' property_path_entry  */
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

  case 349: /* property_path_entry: SQL_TOKEN_NAME opt_column_array_idx  */
        {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
			(yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 350: /* property_path_entry: '*'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("*", SQL_NODE_PUNCTUATION));
        }
    break;

  case 351: /* property_path_entry: SQL_TOKEN_DOLLAR  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("$", SQL_NODE_PUNCTUATION));
        }
    break;

  case 352: /* column_ref: property_path opt_extract_value  */
                {
			(yyval.pParseNode) = SQL_NEW_RULE;
			(yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
		}
    break;

  case 357: /* simple_case: SQL_TOKEN_CASE case_operand simple_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-4].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 358: /* searched_case: SQL_TOKEN_CASE searched_when_clause_list else_clause SQL_TOKEN_END  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 359: /* simple_when_clause_list: simple_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 360: /* simple_when_clause_list: searched_when_clause_list simple_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 361: /* simple_when_clause: SQL_TOKEN_WHEN when_operand_list SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 362: /* when_operand_list: when_operand  */
        {(yyval.pParseNode) = SQL_NEW_COMMALISTRULE;
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));}
    break;

  case 363: /* when_operand_list: when_operand_list ',' when_operand  */
        {(yyvsp[-2].pParseNode)->append((yyvsp[0].pParseNode));
        (yyval.pParseNode) = (yyvsp[-2].pParseNode);}
    break;

  case 370: /* searched_when_clause_list: searched_when_clause  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 371: /* searched_when_clause_list: searched_when_clause_list searched_when_clause  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 372: /* searched_when_clause: SQL_TOKEN_WHEN search_condition SQL_TOKEN_THEN result  */
    {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-3].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-2].pParseNode));
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 373: /* else_clause: %empty  */
        {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 374: /* else_clause: SQL_TOKEN_ELSE result  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 378: /* parameter: ':' SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode) = CREATE_NODE(":", SQL_NODE_PUNCTUATION));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 379: /* parameter: '?'  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE; // test
            (yyval.pParseNode)->append((yyvsp[0].pParseNode) = CREATE_NODE("?", SQL_NODE_PUNCTUATION));
        }
    break;

  case 380: /* range_variable: %empty  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
        }
    break;

  case 381: /* range_variable: opt_as SQL_TOKEN_NAME  */
        {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 382: /* opt_ecsqloptions_clause: %empty  */
                            {(yyval.pParseNode) = SQL_NEW_RULE;}
    break;

  case 384: /* ecsqloptions_clause: SQL_TOKEN_ECSQLOPTIONS ecsqloptions_list  */
        {
        (yyval.pParseNode) = SQL_NEW_RULE;
        (yyval.pParseNode)->append((yyvsp[-1].pParseNode));
        (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 385: /* ecsqloptions_list: ecsqloptions_list ecsqloption  */
        {
            (yyvsp[-1].pParseNode)->append((yyvsp[0].pParseNode));
            (yyval.pParseNode) = (yyvsp[-1].pParseNode);
        }
    break;

  case 386: /* ecsqloptions_list: ecsqloption  */
        {
            (yyval.pParseNode) = SQL_NEW_LISTRULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
        }
    break;

  case 387: /* ecsqloption: SQL_TOKEN_NAME  */
            {
            (yyval.pParseNode) = SQL_NEW_RULE;
            (yyval.pParseNode)->append((yyvsp[0].pParseNode));
            }
    break;

  case 388: /* ecsqloption: SQL_TOKEN_NAME SQL_EQUAL ecsqloptionvalue  */
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
