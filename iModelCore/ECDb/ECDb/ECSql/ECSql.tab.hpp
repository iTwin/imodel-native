/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_ECSQL_TAB_HPP_INCLUDED
# define YY_YY_ECSQL_TAB_HPP_INCLUDED

/* Pull in all ECDb/ECSql AST types and bring them into scope.
   Required for the ECSqlYYSTYPE union members (Exp*, ValueExp*, etc.)
   and the yyparse() declaration (BisonParseResult*, ECSqlParseContext*).
   %code requires equivalent — keep in sync with ECSql.y %code requires block. */
#include "ECSqlBisonParser.h"
USING_NAMESPACE_BENTLEY_SQLITE_EC

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef ECSQL_YYTOKENTYPE
# define ECSQL_YYTOKENTYPE
  enum ecsql_yytokentype
  {
    ECSQL_YYEMPTY = -2,
    ECSQL_YYEOF = 0,                     /* "end of file"  */
    ECSQL_YYerror = 256,                 /* error  */
    ECSQL_YYUNDEF = 257,                 /* "invalid token"  */
    NAME = 258,                    /* "identifier"  */
    INTEGER_LITERAL = 259,         /* "integer literal"  */
    APPROX_LITERAL = 260,          /* "float literal"  */
    STRING_LITERAL = 261,          /* "string literal"  */
    NAMED_PARAM = 262,             /* "named parameter"  */
    PARAMETER = 263,               /* "?"  */
    DOLLAR = 264,                  /* "$"  */
    LESS = 265,                    /* LESS  */
    GREAT = 266,                   /* GREAT  */
    EQUAL = 267,                   /* EQUAL  */
    LESS_EQ = 268,                 /* LESS_EQ  */
    GREAT_EQ = 269,                /* GREAT_EQ  */
    NOT_EQUAL = 270,               /* NOT_EQUAL  */
    CONCAT = 271,                  /* CONCAT  */
    ARROW = 272,                   /* ARROW  */
    BITWISE_NOT = 273,             /* BITWISE_NOT  */
    BITWISE_OR = 274,              /* BITWISE_OR  */
    BITWISE_AND = 275,             /* BITWISE_AND  */
    SHIFT_LEFT = 276,              /* SHIFT_LEFT  */
    SHIFT_RIGHT = 277,             /* SHIFT_RIGHT  */
    MINUS = 278,                   /* MINUS  */
    PLUS = 279,                    /* PLUS  */
    STAR = 280,                    /* STAR  */
    SLASH = 281,                   /* SLASH  */
    PERCENT = 282,                 /* PERCENT  */
    COLON = 283,                   /* COLON  */
    LPAREN = 284,                  /* LPAREN  */
    RPAREN = 285,                  /* RPAREN  */
    COMMA = 286,                   /* COMMA  */
    DOT = 287,                     /* DOT  */
    SEMICOLON = 288,               /* SEMICOLON  */
    LBRACE = 289,                  /* LBRACE  */
    RBRACE = 290,                  /* RBRACE  */
    LBRACKET = 291,                /* LBRACKET  */
    RBRACKET = 292,                /* RBRACKET  */
    KW_ALL = 293,                  /* KW_ALL  */
    KW_AND = 294,                  /* KW_AND  */
    KW_ANY = 295,                  /* KW_ANY  */
    KW_AS = 296,                   /* KW_AS  */
    KW_ASC = 297,                  /* KW_ASC  */
    KW_AVG = 298,                  /* KW_AVG  */
    KW_BACKWARD = 299,             /* KW_BACKWARD  */
    KW_BETWEEN = 300,              /* KW_BETWEEN  */
    KW_BINARY = 301,               /* KW_BINARY  */
    KW_BLOB = 302,                 /* KW_BLOB  */
    KW_BOOLEAN = 303,              /* KW_BOOLEAN  */
    KW_BY = 304,                   /* KW_BY  */
    KW_CASE = 305,                 /* KW_CASE  */
    KW_CAST = 306,                 /* KW_CAST  */
    KW_COLLATE = 307,              /* KW_COLLATE  */
    KW_COUNT = 308,                /* KW_COUNT  */
    KW_CROSS = 309,                /* KW_CROSS  */
    KW_CUME_DIST = 310,            /* KW_CUME_DIST  */
    KW_CURRENT = 311,              /* KW_CURRENT  */
    KW_CURRENT_DATE = 312,         /* KW_CURRENT_DATE  */
    KW_CURRENT_TIME = 313,         /* KW_CURRENT_TIME  */
    KW_CURRENT_TIMESTAMP = 314,    /* KW_CURRENT_TIMESTAMP  */
    KW_DATE = 315,                 /* KW_DATE  */
    KW_DELETE = 316,               /* KW_DELETE  */
    KW_DENSE_RANK = 317,           /* KW_DENSE_RANK  */
    KW_DESC = 318,                 /* KW_DESC  */
    KW_DISTINCT = 319,             /* KW_DISTINCT  */
    KW_DOUBLE = 320,               /* KW_DOUBLE  */
    KW_ECSQLOPTIONS = 321,         /* KW_ECSQLOPTIONS  */
    KW_ELSE = 322,                 /* KW_ELSE  */
    KW_END = 323,                  /* KW_END  */
    KW_ESCAPE = 324,               /* KW_ESCAPE  */
    KW_EVERY = 325,                /* KW_EVERY  */
    KW_EXCEPT = 326,               /* KW_EXCEPT  */
    KW_EXCLUDE = 327,              /* KW_EXCLUDE  */
    KW_EXISTS = 328,               /* KW_EXISTS  */
    KW_FALSE = 329,                /* KW_FALSE  */
    KW_FILTER = 330,               /* KW_FILTER  */
    KW_FIRST = 331,                /* KW_FIRST  */
    KW_FIRST_VALUE = 332,          /* KW_FIRST_VALUE  */
    KW_FLOAT = 333,                /* KW_FLOAT  */
    KW_FOLLOWING = 334,            /* KW_FOLLOWING  */
    KW_FOR = 335,                  /* KW_FOR  */
    KW_FORWARD = 336,              /* KW_FORWARD  */
    KW_FROM = 337,                 /* KW_FROM  */
    KW_FULL = 338,                 /* KW_FULL  */
    KW_GROUP = 339,                /* KW_GROUP  */
    KW_GROUP_CONCAT = 340,         /* KW_GROUP_CONCAT  */
    KW_GROUPS = 341,               /* KW_GROUPS  */
    KW_HAVING = 342,               /* KW_HAVING  */
    KW_IIF = 343,                  /* KW_IIF  */
    KW_IN = 344,                   /* KW_IN  */
    KW_INNER = 345,                /* KW_INNER  */
    KW_INSERT = 346,               /* KW_INSERT  */
    KW_INT = 347,                  /* KW_INT  */
    KW_INT64 = 348,                /* KW_INT64  */
    KW_INTERSECT = 349,            /* KW_INTERSECT  */
    KW_INTO = 350,                 /* KW_INTO  */
    KW_IS = 351,                   /* KW_IS  */
    KW_JOIN = 352,                 /* KW_JOIN  */
    KW_LAG = 353,                  /* KW_LAG  */
    KW_LAST = 354,                 /* KW_LAST  */
    KW_LAST_VALUE = 355,           /* KW_LAST_VALUE  */
    KW_LEAD = 356,                 /* KW_LEAD  */
    KW_LEFT = 357,                 /* KW_LEFT  */
    KW_LIKE = 358,                 /* KW_LIKE  */
    KW_LIMIT = 359,                /* KW_LIMIT  */
    KW_LONG = 360,                 /* KW_LONG  */
    KW_MATCH = 361,                /* KW_MATCH  */
    KW_MAX = 362,                  /* KW_MAX  */
    KW_MIN = 363,                  /* KW_MIN  */
    KW_NATURAL = 364,              /* KW_NATURAL  */
    KW_NAVIGATION_VALUE = 365,     /* KW_NAVIGATION_VALUE  */
    KW_NO = 366,                   /* KW_NO  */
    KW_NOCASE = 367,               /* KW_NOCASE  */
    KW_NOT = 368,                  /* KW_NOT  */
    KW_NTH_VALUE = 369,            /* KW_NTH_VALUE  */
    KW_NTILE = 370,                /* KW_NTILE  */
    KW_NULL = 371,                 /* KW_NULL  */
    KW_NULLS = 372,                /* KW_NULLS  */
    KW_OFFSET = 373,               /* KW_OFFSET  */
    KW_ON = 374,                   /* KW_ON  */
    KW_ONLY = 375,                 /* KW_ONLY  */
    KW_OPTIONS = 376,              /* KW_OPTIONS  */
    KW_OR = 377,                   /* KW_OR  */
    KW_ORDER = 378,                /* KW_ORDER  */
    KW_OTHERS = 379,               /* KW_OTHERS  */
    KW_OUTER = 380,                /* KW_OUTER  */
    KW_OVER = 381,                 /* KW_OVER  */
    KW_PARTITION = 382,            /* KW_PARTITION  */
    KW_PERCENT_RANK = 383,         /* KW_PERCENT_RANK  */
    KW_PRAGMA = 384,               /* KW_PRAGMA  */
    KW_PRECEDING = 385,            /* KW_PRECEDING  */
    KW_RANGE = 386,                /* KW_RANGE  */
    KW_RANK = 387,                 /* KW_RANK  */
    KW_REAL = 388,                 /* KW_REAL  */
    KW_RECURSIVE = 389,            /* KW_RECURSIVE  */
    KW_RIGHT = 390,                /* KW_RIGHT  */
    KW_ROW = 391,                  /* KW_ROW  */
    KW_ROWS = 392,                 /* KW_ROWS  */
    KW_ROW_NUMBER = 393,           /* KW_ROW_NUMBER  */
    KW_RTRIM = 394,                /* KW_RTRIM  */
    KW_SELECT = 395,               /* KW_SELECT  */
    KW_SET = 396,                  /* KW_SET  */
    KW_SOME = 397,                 /* KW_SOME  */
    KW_STRING_KW = 398,            /* KW_STRING_KW  */
    KW_SUM = 399,                  /* KW_SUM  */
    KW_THEN = 400,                 /* KW_THEN  */
    KW_TIES = 401,                 /* KW_TIES  */
    KW_TIME = 402,                 /* KW_TIME  */
    KW_TIMESTAMP = 403,            /* KW_TIMESTAMP  */
    KW_TOTAL = 404,                /* KW_TOTAL  */
    KW_TRUE = 405,                 /* KW_TRUE  */
    KW_UNBOUNDED = 406,            /* KW_UNBOUNDED  */
    KW_UNION = 407,                /* KW_UNION  */
    KW_UNIQUE = 408,               /* KW_UNIQUE  */
    KW_UNKNOWN = 409,              /* KW_UNKNOWN  */
    KW_UPDATE = 410,               /* KW_UPDATE  */
    KW_USING = 411,                /* KW_USING  */
    KW_VALUE = 412,                /* KW_VALUE  */
    KW_VALUES = 413,               /* KW_VALUES  */
    KW_VARCHAR = 414,              /* KW_VARCHAR  */
    KW_WHEN = 415,                 /* KW_WHEN  */
    KW_WHERE = 416,                /* KW_WHERE  */
    KW_WINDOW = 417,               /* KW_WINDOW  */
    KW_WITH = 418                  /* KW_WITH  */
  };
  typedef enum ecsql_yytokentype ecsql_yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined ECSqlYYSTYPE && ! defined ECSQL_YYSTYPE_IS_DECLARED
union ECSqlYYSTYPE
{
#line 60 "ECSql.y"

    char*  strVal;       /* heap-allocated string — caller frees */
    int    intVal;
    bool   boolVal;

    /* owning raw pointers; BISON does not support unique_ptr in unions.
       Each $$ assignment is immediately wrapped into unique_ptr by the
       receiving rule.  Leaks on parse error are acceptable for this
       reference grammar (production code would use %destructor). */

    Exp*                                exp;
    ValueExp*                           valueExp;
    ComputedExp*                        computedExp;
    BooleanExp*                         booleanExp;
    ClassRefExp*                        classRefExp;
    ClassNameExp*                       classNameExp;
    JoinExp*                            joinExp;
    JoinSpecExp*                        joinSpecExp;
    SubqueryExp*                        subqueryExp;
    SelectStatementExp*                 selectStmtExp;
    SingleSelectStatementExp*           singleSelectExp;
    SelectClauseExp*                    selectClauseExp;
    DerivedPropertyExp*                 derivedPropExp;
    FromExp*                            fromExp;
    WhereExp*                           whereExp;
    GroupByExp*                         groupByExp;
    HavingExp*                          havingExp;
    OrderByExp*                         orderByExp;
    OrderBySpecExp*                     orderBySpecExp;
    LimitOffsetExp*                     limitOffsetExp;
    OptionsExp*                         optionsExp;
    OptionExp*                          optionExp;
    PropertyNameExp*                    propNameExp;
    PropertyNameListExp*                propNameListExp;
    AssignmentListExp*                  assignListExp;
    AssignmentExp*                      assignExp;
    ValueExpListExp*                    valueExpListExp;
    InsertStatementExp*                 insertStmtExp;
    UpdateStatementExp*                 updateStmtExp;
    DeleteStatementExp*                 deleteStmtExp;
    PragmaStatementExp*                 pragmaStmtExp;
    CommonTableExp*                     cteExp;
    CommonTableBlockExp*                cteBlockExp;
    WindowFunctionClauseExp*            windowClauseExp;
    WindowSpecification*                windowSpecExp;
    WindowFunctionExp*                  windowFuncExp;
    WindowFrameClauseExp*               frameClauseExp;
    WindowFrameStartExp*                frameStartExp;
    WindowFrameBetweenExp*              frameBetweenExp;
    FirstWindowFrameBoundExp*           firstBoundExp;
    SecondWindowFrameBoundExp*          secondBoundExp;
    FilterClauseExp*                    filterClauseExp;
    WindowPartitionColumnReferenceListExp* partitionListExp;
    WindowDefinitionExp*                windowDefExp;

    /* enum carriers */
    SqlSetQuantifier                    setQuantifier;
    BinarySqlOperator                   binOp;
    BooleanSqlOperator                  boolOp;
    UnaryValueExp::Operator             unaryOp;
    ECSqlJoinType                       joinType;
    JoinDirection                       joinDir;
    SelectStatementExp::CompoundOperator compoundOp;
    PolymorphicInfo*                    polyInfo;   /* heap-allocated */
    OrderBySpecExp::SortDirection       sortDir;
    OrderBySpecExp::NullsOrder          nullsOrder;
    WindowFrameClauseExp::WindowFrameUnit frameUnit;
    WindowFrameClauseExp::WindowFrameExclusionType frameExcl;
    WindowPartitionColumnReferenceExp::CollateClauseFunction collateFunc;
    SqlCompareListType                  compareListType;

    /* list carriers (raw vectors, transferred into AST nodes) */
    std::vector<std::unique_ptr<CommonTableBlockExp>>*  cteBlockList;
    std::vector<std::unique_ptr<OrderBySpecExp>>*       orderBySpecList;
    std::vector<std::unique_ptr<ValueExp>>*             valueExpVec;
    std::vector<Utf8String>*                            stringVec;
    std::vector<std::unique_ptr<ClassNameExp>>*          classNameVec;
    std::vector<std::unique_ptr<WindowPartitionColumnReferenceExp>>* partColRefVec;
    std::vector<std::unique_ptr<WindowDefinitionExp>>*   windowDefVec;
    std::vector<std::unique_ptr<SearchedWhenClauseExp>>* whenClauseVec;
    PragmaVal*                          pragmaValPtr;

#line 310 "ECSql.tab.hpp"

};
typedef union ECSqlYYSTYPE ECSqlYYSTYPE;
# define ECSQL_YYSTYPE_IS_TRIVIAL 1
# define ECSQL_YYSTYPE_IS_DECLARED 1
#endif




int yyparse (BisonParseResult* result, ECSqlParseContext* ctx, void* scanner);


#endif /* !YY_YY_ECSQL_TAB_HPP_INCLUDED  */
