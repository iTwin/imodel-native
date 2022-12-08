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

#ifndef YY_SQLYY_D_BSW_GIT_NATIVE4_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
# define YY_SQLYY_D_BSW_GIT_NATIVE4_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED
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
    SQL_TOKEN_CAST = 275,          /* SQL_TOKEN_CAST  */
    SQL_TOKEN_COMMIT = 276,        /* SQL_TOKEN_COMMIT  */
    SQL_TOKEN_COUNT = 277,         /* SQL_TOKEN_COUNT  */
    SQL_TOKEN_CROSS = 278,         /* SQL_TOKEN_CROSS  */
    SQL_TOKEN_DEFAULT = 279,       /* SQL_TOKEN_DEFAULT  */
    SQL_TOKEN_DELETE = 280,        /* SQL_TOKEN_DELETE  */
    SQL_TOKEN_DESC = 281,          /* SQL_TOKEN_DESC  */
    SQL_TOKEN_DISTINCT = 282,      /* SQL_TOKEN_DISTINCT  */
    SQL_TOKEN_FORWARD = 283,       /* SQL_TOKEN_FORWARD  */
    SQL_TOKEN_BACKWARD = 284,      /* SQL_TOKEN_BACKWARD  */
    SQL_TOKEN_ESCAPE = 285,        /* SQL_TOKEN_ESCAPE  */
    SQL_TOKEN_EXCEPT = 286,        /* SQL_TOKEN_EXCEPT  */
    SQL_TOKEN_EXISTS = 287,        /* SQL_TOKEN_EXISTS  */
    SQL_TOKEN_FALSE = 288,         /* SQL_TOKEN_FALSE  */
    SQL_TOKEN_FROM = 289,          /* SQL_TOKEN_FROM  */
    SQL_TOKEN_FULL = 290,          /* SQL_TOKEN_FULL  */
    SQL_TOKEN_GROUP = 291,         /* SQL_TOKEN_GROUP  */
    SQL_TOKEN_HAVING = 292,        /* SQL_TOKEN_HAVING  */
    SQL_TOKEN_IN = 293,            /* SQL_TOKEN_IN  */
    SQL_TOKEN_INNER = 294,         /* SQL_TOKEN_INNER  */
    SQL_TOKEN_INSERT = 295,        /* SQL_TOKEN_INSERT  */
    SQL_TOKEN_INTO = 296,          /* SQL_TOKEN_INTO  */
    SQL_TOKEN_IS = 297,            /* SQL_TOKEN_IS  */
    SQL_TOKEN_INTERSECT = 298,     /* SQL_TOKEN_INTERSECT  */
    SQL_TOKEN_JOIN = 299,          /* SQL_TOKEN_JOIN  */
    SQL_TOKEN_LIKE = 300,          /* SQL_TOKEN_LIKE  */
    SQL_TOKEN_LEFT = 301,          /* SQL_TOKEN_LEFT  */
    SQL_TOKEN_RIGHT = 302,         /* SQL_TOKEN_RIGHT  */
    SQL_TOKEN_MAX = 303,           /* SQL_TOKEN_MAX  */
    SQL_TOKEN_MIN = 304,           /* SQL_TOKEN_MIN  */
    SQL_TOKEN_NATURAL = 305,       /* SQL_TOKEN_NATURAL  */
    SQL_TOKEN_NULL = 306,          /* SQL_TOKEN_NULL  */
    SQL_TOKEN_ON = 307,            /* SQL_TOKEN_ON  */
    SQL_TOKEN_ORDER = 308,         /* SQL_TOKEN_ORDER  */
    SQL_TOKEN_OUTER = 309,         /* SQL_TOKEN_OUTER  */
    SQL_TOKEN_ROLLBACK = 310,      /* SQL_TOKEN_ROLLBACK  */
    SQL_TOKEN_IIF = 311,           /* SQL_TOKEN_IIF  */
    SQL_TOKEN_SELECT = 312,        /* SQL_TOKEN_SELECT  */
    SQL_TOKEN_SET = 313,           /* SQL_TOKEN_SET  */
    SQL_TOKEN_SOME = 314,          /* SQL_TOKEN_SOME  */
    SQL_TOKEN_SUM = 315,           /* SQL_TOKEN_SUM  */
    SQL_TOKEN_TRUE = 316,          /* SQL_TOKEN_TRUE  */
    SQL_TOKEN_UNION = 317,         /* SQL_TOKEN_UNION  */
    SQL_TOKEN_UNIQUE = 318,        /* SQL_TOKEN_UNIQUE  */
    SQL_TOKEN_UNKNOWN = 319,       /* SQL_TOKEN_UNKNOWN  */
    SQL_TOKEN_UPDATE = 320,        /* SQL_TOKEN_UPDATE  */
    SQL_TOKEN_USING = 321,         /* SQL_TOKEN_USING  */
    SQL_TOKEN_VALUE = 322,         /* SQL_TOKEN_VALUE  */
    SQL_TOKEN_VALUES = 323,        /* SQL_TOKEN_VALUES  */
    SQL_TOKEN_WHERE = 324,         /* SQL_TOKEN_WHERE  */
    SQL_BITWISE_NOT = 325,         /* SQL_BITWISE_NOT  */
    SQL_TOKEN_CURRENT_DATE = 326,  /* SQL_TOKEN_CURRENT_DATE  */
    SQL_TOKEN_CURRENT_TIME = 327,  /* SQL_TOKEN_CURRENT_TIME  */
    SQL_TOKEN_CURRENT_TIMESTAMP = 328, /* SQL_TOKEN_CURRENT_TIMESTAMP  */
    SQL_TOKEN_EVERY = 329,         /* SQL_TOKEN_EVERY  */
    SQL_TOKEN_CASE = 330,          /* SQL_TOKEN_CASE  */
    SQL_TOKEN_THEN = 331,          /* SQL_TOKEN_THEN  */
    SQL_TOKEN_END = 332,           /* SQL_TOKEN_END  */
    SQL_TOKEN_WHEN = 333,          /* SQL_TOKEN_WHEN  */
    SQL_TOKEN_ELSE = 334,          /* SQL_TOKEN_ELSE  */
    SQL_TOKEN_LIMIT = 335,         /* SQL_TOKEN_LIMIT  */
    SQL_TOKEN_OFFSET = 336,        /* SQL_TOKEN_OFFSET  */
    SQL_TOKEN_ONLY = 337,          /* SQL_TOKEN_ONLY  */
    SQL_TOKEN_PRAGMA = 338,        /* SQL_TOKEN_PRAGMA  */
    SQL_TOKEN_FOR = 339,           /* SQL_TOKEN_FOR  */
    SQL_TOKEN_MATCH = 340,         /* SQL_TOKEN_MATCH  */
    SQL_TOKEN_ECSQLOPTIONS = 341,  /* SQL_TOKEN_ECSQLOPTIONS  */
    SQL_TOKEN_INTEGER = 342,       /* SQL_TOKEN_INTEGER  */
    SQL_TOKEN_INT = 343,           /* SQL_TOKEN_INT  */
    SQL_TOKEN_INT64 = 344,         /* SQL_TOKEN_INT64  */
    SQL_TOKEN_LONG = 345,          /* SQL_TOKEN_LONG  */
    SQL_TOKEN_BOOLEAN = 346,       /* SQL_TOKEN_BOOLEAN  */
    SQL_TOKEN_DOUBLE = 347,        /* SQL_TOKEN_DOUBLE  */
    SQL_TOKEN_REAL = 348,          /* SQL_TOKEN_REAL  */
    SQL_TOKEN_FLOAT = 349,         /* SQL_TOKEN_FLOAT  */
    SQL_TOKEN_STRING = 350,        /* SQL_TOKEN_STRING  */
    SQL_TOKEN_VARCHAR = 351,       /* SQL_TOKEN_VARCHAR  */
    SQL_TOKEN_BINARY = 352,        /* SQL_TOKEN_BINARY  */
    SQL_TOKEN_BLOB = 353,          /* SQL_TOKEN_BLOB  */
    SQL_TOKEN_DATE = 354,          /* SQL_TOKEN_DATE  */
    SQL_TOKEN_TIME = 355,          /* SQL_TOKEN_TIME  */
    SQL_TOKEN_TIMESTAMP = 356,     /* SQL_TOKEN_TIMESTAMP  */
    SQL_TOKEN_OR = 357,            /* SQL_TOKEN_OR  */
    SQL_TOKEN_AND = 358,           /* SQL_TOKEN_AND  */
    SQL_BITWISE_OR = 359,          /* SQL_BITWISE_OR  */
    SQL_BITWISE_AND = 360,         /* SQL_BITWISE_AND  */
    SQL_BITWISE_SHIFT_LEFT = 361,  /* SQL_BITWISE_SHIFT_LEFT  */
    SQL_BITWISE_SHIFT_RIGHT = 362, /* SQL_BITWISE_SHIFT_RIGHT  */
    SQL_LESSEQ = 363,              /* SQL_LESSEQ  */
    SQL_GREATEQ = 364,             /* SQL_GREATEQ  */
    SQL_NOTEQUAL = 365,            /* SQL_NOTEQUAL  */
    SQL_LESS = 366,                /* SQL_LESS  */
    SQL_GREAT = 367,               /* SQL_GREAT  */
    SQL_EQUAL = 368,               /* SQL_EQUAL  */
    SQL_CONCAT = 369,              /* SQL_CONCAT  */
    SQL_TOKEN_INVALIDSYMBOL = 370  /* SQL_TOKEN_INVALIDSYMBOL  */
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


#endif /* !YY_SQLYY_D_BSW_GIT_NATIVE4_SRC_IMODEL_NATIVE_IMODELCORE_ECDB_SCRIPTS_ECDB_ECSQL_PARSER_SQLBISON_H_INCLUDED  */
