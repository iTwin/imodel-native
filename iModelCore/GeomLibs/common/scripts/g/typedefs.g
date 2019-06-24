! Swallow C comments:
\/\*<u>\*\/=

!Example: typedef struct foo {double a;} Foo;
typedef struct <I>\W\{<ParseToClosingBrace>\W<U>\;=(TaggedStruct $3)\n
typedef struct \{<ParseToClosingBrace>\W<U>\;=(UnTaggedStruct $2)\n

!Example: typedef union {double a;} Foo;
typedef union <I>\W\{<ParseToClosingBrace>\W<U>\;=(TaggedUnion $3)\n
typedef union \{<ParseToClosingBrace>\W<U>\;=(UnTaggedUnion $2)\n

!Example: typedef enum { a = 100} Foo;
typedef enum \{<ParseToClosingBrace>\W<U>\;=(enum $2)\n

!Example: typedef void (*foo)
typedef <I>\W(\W\*<I>\W)=(FunctionType $2)\n
typedef <I>\W\*\W(\W\*<I>\W)=(FunctionType $2)\n

! Example: typedef int StatusInt;
! and also with unsigned
typedef unsigned <I>\I\W<U>\;=(NameReplacement $2)\n
typedef <I>\I\W<U>\;=(NameReplacement $2)\n

! Other typedefs:
typedef<u>\;=(UnparsedTypedef $0)\n

! Swallow everything else in the input:
?=


! Lookahead through balanced braces.
ParseToClosingBrace:\{#\}=$0
ParseToClosingBrace:\}=$0@end
ParseToClosingBrace:?=?