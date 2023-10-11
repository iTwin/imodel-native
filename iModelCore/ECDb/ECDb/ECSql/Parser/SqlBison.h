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

#ifndef YY_SQLYY_C_REPOS_IMODELNATIVE_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_C_REPOS_IMODELNATIVE_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int SQLyydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    SQL_TOKEN_ACCESS_DATE = 258,   /* SQL_TOKEN_ACCESS_DATE  */
    SQL_TOKEN_REAL_NUM = 259,      /* SQL_TOKEN_REAL_NUM  */
    SQL_TOKEN_INTNUM = 260,        /* SQL_TOKEN_INTNUM  */
    SQL_TOKEN_APPROXNUM = 261,     /* SQL_TOKEN_APPROXNUM  */
    SQL_TOKEN_NOT = 262,           /* SQL_TOKEN_NOT  */
    SQL_TOKEN_NAME = 263,          /* SQL_TOKEN_NAME  */
    SQL_TOKEN_ARRAY_INDEX = 264,   /* SQL_TOKEN_ARRAY_INDEX  */
    SQL_TOKEN_UMINUS = 265,        /* SQL_TOKEN_UMINUS  */
    SQL_TOKEN_WITH = 266,          /* SQL_TOKEN_WITH  */
    SQL_TOKEN_RECURSIVE = 267,     /* SQL_TOKEN_RECURSIVE  */
    SQL_TOKEN_ALL = 268,           /* SQL_TOKEN_ALL  */
    SQL_TOKEN_ANY = 269,           /* SQL_TOKEN_ANY  */
    SQL_TOKEN_AS = 270,            /* SQL_TOKEN_AS  */
    SQL_TOKEN_ASC = 271,           /* SQL_TOKEN_ASC  */
    SQL_TOKEN_AVG = 272,           /* SQL_TOKEN_AVG  */
    SQL_TOKEN_BETWEEN = 273,       /* SQL_TOKEN_BETWEEN  */
    SQL_TOKEN_BY = 274,            /* SQL_TOKEN_BY  */
    SQL_TOKEN_NULLS = 275,         /* SQL_TOKEN_NULLS  */
    SQL_TOKEN_FIRST = 276,         /* SQL_TOKEN_FIRST  */
    SQL_TOKEN_LAST = 277,          /* SQL_TOKEN_LAST  */
    SQL_TOKEN_CAST = 278,          /* SQL_TOKEN_CAST  */
    SQL_TOKEN_COMMIT = 279,        /* SQL_TOKEN_COMMIT  */
    SQL_TOKEN_COUNT = 280,         /* SQL_TOKEN_COUNT  */
    SQL_TOKEN_CROSS = 281,         /* SQL_TOKEN_CROSS  */
    SQL_TOKEN_DELETE = 282,        /* SQL_TOKEN_DELETE  */
    SQL_TOKEN_DESC = 283,          /* SQL_TOKEN_DESC  */
    SQL_TOKEN_DISTINCT = 284,      /* SQL_TOKEN_DISTINCT  */
    SQL_TOKEN_FORWARD = 285,       /* SQL_TOKEN_FORWARD  */
    SQL_TOKEN_BACKWARD = 286,      /* SQL_TOKEN_BACKWARD  */
    SQL_TOKEN_ESCAPE = 287,        /* SQL_TOKEN_ESCAPE  */
    SQL_TOKEN_EXCEPT = 288,        /* SQL_TOKEN_EXCEPT  */
    SQL_TOKEN_EXISTS = 289,        /* SQL_TOKEN_EXISTS  */
    SQL_TOKEN_FALSE = 290,         /* SQL_TOKEN_FALSE  */
    SQL_TOKEN_FROM = 291,          /* SQL_TOKEN_FROM  */
    SQL_TOKEN_FULL = 292,          /* SQL_TOKEN_FULL  */
    SQL_TOKEN_GROUP = 293,         /* SQL_TOKEN_GROUP  */
    SQL_TOKEN_HAVING = 294,        /* SQL_TOKEN_HAVING  */
    SQL_TOKEN_IN = 295,            /* SQL_TOKEN_IN  */
    SQL_TOKEN_INNER = 296,         /* SQL_TOKEN_INNER  */
    SQL_TOKEN_INSERT = 297,        /* SQL_TOKEN_INSERT  */
    SQL_TOKEN_INTO = 298,          /* SQL_TOKEN_INTO  */
    SQL_TOKEN_IS = 299,            /* SQL_TOKEN_IS  */
    SQL_TOKEN_INTERSECT = 300,     /* SQL_TOKEN_INTERSECT  */
    SQL_TOKEN_JOIN = 301,          /* SQL_TOKEN_JOIN  */
    SQL_TOKEN_LIKE = 302,          /* SQL_TOKEN_LIKE  */
    SQL_TOKEN_LEFT = 303,          /* SQL_TOKEN_LEFT  */
    SQL_TOKEN_RIGHT = 304,         /* SQL_TOKEN_RIGHT  */
    SQL_TOKEN_MAX = 305,           /* SQL_TOKEN_MAX  */
    SQL_TOKEN_MIN = 306,           /* SQL_TOKEN_MIN  */
    SQL_TOKEN_NATURAL = 307,       /* SQL_TOKEN_NATURAL  */
    SQL_TOKEN_NULL = 308,          /* SQL_TOKEN_NULL  */
    SQL_TOKEN_TOTAL = 309,         /* SQL_TOKEN_TOTAL  */
    SQL_TOKEN_ON = 310,            /* SQL_TOKEN_ON  */
    SQL_TOKEN_ORDER = 311,         /* SQL_TOKEN_ORDER  */
    SQL_TOKEN_OUTER = 312,         /* SQL_TOKEN_OUTER  */
    SQL_TOKEN_ROLLBACK = 313,      /* SQL_TOKEN_ROLLBACK  */
    SQL_TOKEN_IIF = 314,           /* SQL_TOKEN_IIF  */
    SQL_TOKEN_SELECT = 315,        /* SQL_TOKEN_SELECT  */
    SQL_TOKEN_SET = 316,           /* SQL_TOKEN_SET  */
    SQL_TOKEN_SOME = 317,          /* SQL_TOKEN_SOME  */
    SQL_TOKEN_SUM = 318,           /* SQL_TOKEN_SUM  */
    SQL_TOKEN_TRUE = 319,          /* SQL_TOKEN_TRUE  */
    SQL_TOKEN_UNION = 320,         /* SQL_TOKEN_UNION  */
    SQL_TOKEN_UNIQUE = 321,        /* SQL_TOKEN_UNIQUE  */
    SQL_TOKEN_UNKNOWN = 322,       /* SQL_TOKEN_UNKNOWN  */
    SQL_TOKEN_UPDATE = 323,        /* SQL_TOKEN_UPDATE  */
    SQL_TOKEN_USING = 324,         /* SQL_TOKEN_USING  */
    SQL_TOKEN_VALUE = 325,         /* SQL_TOKEN_VALUE  */
    SQL_TOKEN_VALUES = 326,        /* SQL_TOKEN_VALUES  */
    SQL_TOKEN_WHERE = 327,         /* SQL_TOKEN_WHERE  */
    SQL_TOKEN_DOLLAR = 328,        /* SQL_TOKEN_DOLLAR  */
    SQL_BITWISE_NOT = 329,         /* SQL_BITWISE_NOT  */
    SQL_TOKEN_CURRENT_DATE = 330,  /* SQL_TOKEN_CURRENT_DATE  */
    SQL_TOKEN_CURRENT_TIME = 331,  /* SQL_TOKEN_CURRENT_TIME  */
    SQL_TOKEN_CURRENT_TIMESTAMP = 332, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
    SQL_TOKEN_EVERY = 333,         /* SQL_TOKEN_EVERY  */
    SQL_TOKEN_CASE = 334,          /* SQL_TOKEN_CASE  */
    SQL_TOKEN_THEN = 335,          /* SQL_TOKEN_THEN  */
    SQL_TOKEN_END = 336,           /* SQL_TOKEN_END  */
    SQL_TOKEN_WHEN = 337,          /* SQL_TOKEN_WHEN  */
    SQL_TOKEN_ELSE = 338,          /* SQL_TOKEN_ELSE  */
    SQL_TOKEN_LIMIT = 339,         /* SQL_TOKEN_LIMIT  */
    SQL_TOKEN_OFFSET = 340,        /* SQL_TOKEN_OFFSET  */
    SQL_TOKEN_ONLY = 341,          /* SQL_TOKEN_ONLY  */
    SQL_TOKEN_PRAGMA = 342,        /* SQL_TOKEN_PRAGMA  */
    SQL_TOKEN_FOR = 343,           /* SQL_TOKEN_FOR  */
    SQL_TOKEN_MATCH = 344,         /* SQL_TOKEN_MATCH  */
    SQL_TOKEN_ECSQLOPTIONS = 345,  /* SQL_TOKEN_ECSQLOPTIONS  */
    SQL_TOKEN_INTEGER = 346,       /* SQL_TOKEN_INTEGER  */
    SQL_TOKEN_INT = 347,           /* SQL_TOKEN_INT  */
    SQL_TOKEN_INT64 = 348,         /* SQL_TOKEN_INT64  */
    SQL_TOKEN_LONG = 349,          /* SQL_TOKEN_LONG  */
    SQL_TOKEN_BOOLEAN = 350,       /* SQL_TOKEN_BOOLEAN  */
    SQL_TOKEN_DOUBLE = 351,        /* SQL_TOKEN_DOUBLE  */
    SQL_TOKEN_REAL = 352,          /* SQL_TOKEN_REAL  */
    SQL_TOKEN_FLOAT = 353,         /* SQL_TOKEN_FLOAT  */
    SQL_TOKEN_STRING = 354,        /* SQL_TOKEN_STRING  */
    SQL_TOKEN_VARCHAR = 355,       /* SQL_TOKEN_VARCHAR  */
    SQL_TOKEN_BINARY = 356,        /* SQL_TOKEN_BINARY  */
    SQL_TOKEN_BLOB = 357,          /* SQL_TOKEN_BLOB  */
    SQL_TOKEN_DATE = 358,          /* SQL_TOKEN_DATE  */
    SQL_TOKEN_TIME = 359,          /* SQL_TOKEN_TIME  */
    SQL_TOKEN_TIMESTAMP = 360,     /* SQL_TOKEN_TIMESTAMP  */
    SQL_TOKEN_OVER = 361,          /* SQL_TOKEN_OVER  */
    SQL_TOKEN_ROW_NUMBER = 362,    /* SQL_TOKEN_ROW_NUMBER  */
    SQL_TOKEN_NTILE = 363,         /* SQL_TOKEN_NTILE  */
    SQL_TOKEN_LEAD = 364,          /* SQL_TOKEN_LEAD  */
    SQL_TOKEN_LAG = 365,           /* SQL_TOKEN_LAG  */
    SQL_TOKEN_FIRST_VALUE = 366,   /* SQL_TOKEN_FIRST_VALUE  */
    SQL_TOKEN_LAST_VALUE = 367,    /* SQL_TOKEN_LAST_VALUE  */
    SQL_TOKEN_NTH_VALUE = 368,     /* SQL_TOKEN_NTH_VALUE  */
    SQL_TOKEN_EXCLUDE = 369,       /* SQL_TOKEN_EXCLUDE  */
    SQL_TOKEN_OTHERS = 370,        /* SQL_TOKEN_OTHERS  */
    SQL_TOKEN_TIES = 371,          /* SQL_TOKEN_TIES  */
    SQL_TOKEN_FOLLOWING = 372,     /* SQL_TOKEN_FOLLOWING  */
    SQL_TOKEN_UNBOUNDED = 373,     /* SQL_TOKEN_UNBOUNDED  */
    SQL_TOKEN_PRECEDING = 374,     /* SQL_TOKEN_PRECEDING  */
    SQL_TOKEN_RANGE = 375,         /* SQL_TOKEN_RANGE  */
    SQL_TOKEN_ROWS = 376,          /* SQL_TOKEN_ROWS  */
    SQL_TOKEN_PARTITION = 377,     /* SQL_TOKEN_PARTITION  */
    SQL_TOKEN_WINDOW = 378,        /* SQL_TOKEN_WINDOW  */
    SQL_TOKEN_NO = 379,            /* SQL_TOKEN_NO  */
    SQL_TOKEN_CURRENT = 380,       /* SQL_TOKEN_CURRENT  */
    SQL_TOKEN_ROW = 381,           /* SQL_TOKEN_ROW  */
    SQL_TOKEN_RANK = 382,          /* SQL_TOKEN_RANK  */
    SQL_TOKEN_DENSE_RANK = 383,    /* SQL_TOKEN_DENSE_RANK  */
    SQL_TOKEN_PERCENT_RANK = 384,  /* SQL_TOKEN_PERCENT_RANK  */
    SQL_TOKEN_CUME_DIST = 385,     /* SQL_TOKEN_CUME_DIST  */
    SQL_TOKEN_COLLATE = 386,       /* SQL_TOKEN_COLLATE  */
    SQL_TOKEN_NOCASE = 387,        /* SQL_TOKEN_NOCASE  */
    SQL_TOKEN_RTRIM = 388,         /* SQL_TOKEN_RTRIM  */
    SQL_TOKEN_FILTER = 389,        /* SQL_TOKEN_FILTER  */
    SQL_TOKEN_GROUPS = 390,        /* SQL_TOKEN_GROUPS  */
    SQL_TOKEN_GROUP_CONCAT = 391,  /* SQL_TOKEN_GROUP_CONCAT  */
    SQL_TOKEN_OR = 392,            /* SQL_TOKEN_OR  */
    SQL_TOKEN_AND = 393,           /* SQL_TOKEN_AND  */
    SQL_ARROW = 394,               /* SQL_ARROW  */
    SQL_BITWISE_OR = 395,          /* SQL_BITWISE_OR  */
    SQL_BITWISE_AND = 396,         /* SQL_BITWISE_AND  */
    SQL_BITWISE_SHIFT_LEFT = 397,  /* SQL_BITWISE_SHIFT_LEFT  */
    SQL_BITWISE_SHIFT_RIGHT = 398, /* SQL_BITWISE_SHIFT_RIGHT  */
    SQL_LESSEQ = 399,              /* SQL_LESSEQ  */
    SQL_GREATEQ = 400,             /* SQL_GREATEQ  */
    SQL_NOTEQUAL = 401,            /* SQL_NOTEQUAL  */
    SQL_LESS = 402,                /* SQL_LESS  */
    SQL_GREAT = 403,               /* SQL_GREAT  */
    SQL_EQUAL = 404,               /* SQL_EQUAL  */
    SQL_CONCAT = 405,              /* SQL_CONCAT  */
    SQL_TOKEN_INVALIDSYMBOL = 406  /* SQL_TOKEN_INVALIDSYMBOL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

    connectivity::OSQLParseNode * pParseNode;


};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int SQLyyparse (connectivity::OSQLParser* context);


#endif /* !YY_SQLYY_C_REPOS_IMODELNATIVE_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
