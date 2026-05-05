/* =============================================================================
 * ECSql.g4 — ANTLR 4 grammar for ECSQL
 *
 * Generated from the ECSqlRDParser.cpp recursive-descent parser.
 * This grammar describes the syntax accepted by the ECSQL parser.
 *
 * Operator precedence (value expressions, lowest → highest):
 *   |   (bitwise OR)
 *   &   (bitwise AND)
 *   << >>  (shift)
 *   + -    (additive)
 *   * / %  (multiplicative)
 *   ||     (concatenation)
 *   - + ~  (unary prefix)
 *
 * Boolean precedence (lowest → highest):
 *   OR
 *   AND
 *   NOT
 *   predicate
 *
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * =========================================================================== */

grammar ECSql;

/* ==========================================================================
 * Parser Rules
 * ========================================================================== */

// ─── Top-level entry ───

ecsqlStatement
    : cteStatement SEMICOLON? EOF
    | selectStatement SEMICOLON? EOF
    | insertStatement SEMICOLON? EOF
    | updateStatement SEMICOLON? EOF
    | deleteStatement SEMICOLON? EOF
    | pragmaStatement SEMICOLON? EOF
    ;

// ─── INSERT statement ───
// INSERT INTO [ONLY] tableNode [(colList)] VALUES (valList)

insertStatement
    : INSERT INTO (ONLY)? tableNode optColumnRefCommalist
        VALUES LPAREN rowValueConstructorCommalist RPAREN
    ;

// ─── UPDATE statement ───
// UPDATE [ALL|ONLY] tableNode SET assignments [WHERE cond] [OPTIONS ...]

updateStatement
    : UPDATE (ALL | ONLY)? tableNode SET assignmentCommalist
        whereClause? ecsqlOptionsClause?
    ;

// ─── DELETE statement ───
// DELETE FROM [ALL|ONLY] tableNode [WHERE cond] [OPTIONS ...]

deleteStatement
    : DELETE FROM (ALL | ONLY)? tableNode
        whereClause? ecsqlOptionsClause?
    ;

// ─── PRAGMA statement ───
// PRAGMA name [= value | (value)] [FOR path] [OPTIONS ...]

pragmaStatement
    : PRAGMA identifier pragmaValueClause? pragmaForPath? ecsqlOptionsClause?
    ;

pragmaValueClause
    : EQUAL pragmaValue
    | LPAREN pragmaValue RPAREN
    ;

pragmaValue
    : TRUE_
    | FALSE_
    | NULL_
    | INTEGER_LITERAL
    | APPROX_LITERAL
    | STRING_LITERAL
    | identifier
    ;

pragmaForPath
    : FOR pragmaPath
    ;

pragmaPath
    : NAME (( DOT | COLON ) NAME | NAMED_PARAM)*
    ;

// ─── CTE (Common Table Expression) ───
// WITH [RECURSIVE] cteBlock [, ...] selectStatement

cteStatement
    : WITH RECURSIVE? cteBlock (COMMA cteBlock)* selectStatement
    ;

cteBlock
    : identifier (LPAREN columnNameCommalist RPAREN)? AS LPAREN selectStatement RPAREN
    ;

columnNameCommalist
    : identifier (COMMA identifier)*
    ;

// ─── SELECT statement (handles UNION / INTERSECT / EXCEPT) ───

selectStatement
    : valuesStatement
    | singleSelectStatement (compoundOp ALL? selectStatement)?
    ;

valuesStatement
    : VALUES valuesRowList (compoundOp ALL? selectStatement)?
    ;

valuesRowList
    : LPAREN rowValueConstructorCommalist RPAREN
        (COMMA LPAREN rowValueConstructorCommalist RPAREN)*
    ;

compoundOp
    : UNION
    | INTERSECT
    | EXCEPT
    ;

// ─── Single SELECT statement ───

singleSelectStatement
    : SELECT (ALL | DISTINCT)? selection
        fromClause?
        whereClause?
        groupByClause?
        havingClause?
        windowClause?
        orderByClause?
        limitOffsetClause?
        ecsqlOptionsClause?
    ;

// ─── Selection (SELECT clause) ───

selection
    : selectItem (COMMA selectItem)*
    ;

selectItem
    : STAR
    | derivedColumn
    ;

derivedColumn
    : valueExp columnAlias?
    ;

columnAlias
    : AS identifier
    | NAME          /* implicit alias */
    ;

// ─── FROM clause ───

fromClause
    : FROM tableRefAndJoins (COMMA tableRefAndJoins)*
    ;

tableRefAndJoins
    : tableRef joinClause*
    ;

// ─── Table reference ───

tableRef
    : polymorphicConstraint? disqualifyPrefix? tableRefBody tableAlias?
    ;

disqualifyPrefix
    : PLUS
    ;

tableRefBody
    : subquery                                                      # subqueryTableRef
    | NAME LPAREN valueExpCommalist? RPAREN                         # tvfNoSchema
    | NAME DOT NAME LPAREN valueExpCommalist? RPAREN                # tvfWithSchema
    | NAME DOT NAME DOT NAME DOT NAME
        LPAREN valueExpCommalist? RPAREN                            # classWithTsAndMemberFunc
    | NAME DOT NAME DOT NAME LPAREN valueExpCommalist? RPAREN      # classWithMemberFunc
    | NAME DOT NAME DOT NAME                                       # classWithTableSpace
    | NAME NAMED_PARAM                                              # classWithColon
    | NAME DOT NAME                                                 # classRef
    | NAME                                                          # cteBlockRef
    ;

tableAlias
    : AS identifier
    | NAME
    ;

// ─── Polymorphic constraint ───

polymorphicConstraint
    : PLUS ALL
    | PLUS ONLY
    | ALL
    | ONLY
    ;

// ─── Table node (class name) — used in INSERT/UPDATE/DELETE ───

tableNode
    : NAME DOT NAME DOT NAME       /* tableSpace.schema.Class */
    | NAME NAMED_PARAM              /* schema:Class */
    | NAME DOT NAME                 /* schema.Class */
    ;

// ─── JOIN clause ───

joinClause
    : NATURAL? joinBody
    ;

joinBody
    : CROSS JOIN tableRef                                           # crossJoin
    | JOIN tableRef joinSpec                                        # simpleJoin
    | joinType OUTER? JOIN tableRef joinSpec                        # qualifiedJoin
    | USING relationshipRef joinDirection?                           # usingRelJoin
    ;

relationshipRef
    : NAME? /* optional RELATIONSHIP keyword */ tableRef
    ;

joinDirection
    : FORWARD
    | BACKWARD
    ;

joinType
    : INNER
    | LEFT
    | RIGHT
    | FULL
    ;

// ─── Join specification ───

joinSpec
    : ON searchCondition
    | USING LPAREN columnNameCommalist RPAREN
    ;

// ─── Subquery ───

subquery
    : LPAREN selectStatement RPAREN
    | LPAREN cteStatement RPAREN
    ;

// ─── WHERE clause ───

whereClause
    : WHERE searchCondition
    ;

// ─── GROUP BY clause ───

groupByClause
    : GROUP BY valueExpCommalist
    ;

// ─── HAVING clause ───

havingClause
    : HAVING searchCondition
    ;

// ─── ORDER BY clause ───

orderByClause
    : ORDER BY orderBySpec (COMMA orderBySpec)*
    ;

orderBySpec
    : orderBySortExpr sortDirection? nullsOrder?
    ;

orderBySortExpr
    : valueExp (comparisonOp valueExp)?
    | valueExp IS NOT? isRhsLiteral
    ;

sortDirection
    : ASC
    | DESC
    ;

nullsOrder
    : NULLS FIRST
    | NULLS LAST
    ;

// ─── LIMIT / OFFSET clause ───

limitOffsetClause
    : LIMIT valueExp (OFFSET valueExp)?
    ;

// ─── ECSQLOPTIONS clause ───

ecsqlOptionsClause
    : ECSQLOPTIONS optionItem (COMMA? optionItem)*
    ;

optionItem
    : NAME (EQUAL optionValue)?
    ;

optionValue
    : TRUE_
    | FALSE_
    | identifier
    | literal
    ;

// ─── WINDOW clause (named window definitions) ───

windowClause
    : WINDOW windowDefinition (COMMA windowDefinition)*
    ;

windowDefinition
    : identifier AS LPAREN windowSpecification RPAREN
    ;

// ─── VALUES helpers ───

rowValueConstructorCommalist
    : valueExp (COMMA valueExp)*
    ;

// ─── Column reference list (INSERT column list) ───

optColumnRefCommalist
    : /* empty */
    | LPAREN propertyNameExp (COMMA propertyNameExp)* RPAREN
    ;

// ─── Assignment list (UPDATE SET) ───

assignmentCommalist
    : assignment (COMMA assignment)*
    ;

assignment
    : propertyNameExp EQUAL valueExp
    ;

// ─── Value expressions — precedence climbing ───

valueExp
    : valueExpOr
    ;

valueExpOr
    : valueExpAnd (BITWISE_OR valueExpAnd)*
    ;

valueExpAnd
    : valueExpShift (BITWISE_AND valueExpShift)*
    ;

valueExpShift
    : valueExpAddSub ((SHIFT_LEFT | SHIFT_RIGHT) valueExpAddSub)*
    ;

valueExpAddSub
    : valueExpMulDiv ((PLUS | MINUS) valueExpMulDiv)*
    ;

valueExpMulDiv
    : valueExpConcat ((STAR | SLASH | PERCENT) valueExpConcat)*
    ;

valueExpConcat
    : valueExpUnary (CONCAT valueExpUnary)*
    ;

valueExpUnary
    : (MINUS | PLUS | BITWISE_NOT) valueExpPrimary
    | valueExpPrimary
    ;

// ─── Primary value expressions (atoms) ───

valueExpPrimary
    : literal                                                       # literalExpr
    | PARAMETER                                                     # positionalParam
    | NAMED_PARAM                                                   # namedParam
    | dollarAccess                                                  # dollarExpr
    | subquery                                                      # subqueryValueExpr
    | LPAREN valueExp RPAREN                                        # parenExpr
    | castSpec                                                      # castExpr
    | caseExp                                                       # caseExpr
    | iifExp                                                        # iifExpr
    | navigationValueExp                                            # navValueExpr
    | currentDatetimeFunc                                           # currentDatetimeExpr
    | datetimeLiteral                                               # datetimeLiteralExpr
    | RTRIM LPAREN valueExpCommalist? RPAREN                        # rtrimExpr
    | columnRefOrFunc                                               # columnRefExpr
    | STAR                                                          # starExpr
    ;

// ─── Literals ───

literal
    : NULL_
    | TRUE_
    | FALSE_
    | INTEGER_LITERAL
    | APPROX_LITERAL
    | STRING_LITERAL
    ;

isRhsLiteral
    : NULL_
    | TRUE_
    | FALSE_
    | UNKNOWN
    ;

// ─── Dollar / instance access ───

dollarAccess
    : DOLLAR (ARROW propertyPath PARAMETER?)?
    ;

// ─── CAST ───

castSpec
    : CAST LPAREN valueExp AS castType arrayMarker? RPAREN
    ;

castType
    : typeKeyword
    | NAME (DOT NAME)?          /* plain or schema.TypeName */
    ;

typeKeyword
    : BINARY | BLOB | BOOLEAN | DATE | DOUBLE | FLOAT
    | INT | INT64 | LONG | REAL | STRING_KW
    | TIME | TIMESTAMP | VARCHAR
    ;

arrayMarker
    : LBRACKET RBRACKET
    ;

// ─── CASE ───

caseExp
    : CASE whenClause+ elseClause? END
    ;

whenClause
    : WHEN searchCondition THEN valueExp
    ;

elseClause
    : ELSE valueExp
    ;

// ─── IIF ───

iifExp
    : IIF LPAREN searchCondition COMMA valueExp COMMA valueExp RPAREN
    ;

// ─── NAVIGATION_VALUE ───

navigationValueExp
    : NAVIGATION_VALUE LPAREN derivedColumn COMMA valueExp (COMMA valueExp)? RPAREN
    ;

// ─── CURRENT_DATE / CURRENT_TIME / CURRENT_TIMESTAMP ───

currentDatetimeFunc
    : CURRENT_DATE
    | CURRENT_TIME
    | CURRENT_TIMESTAMP
    ;

// ─── DATE / TIME / TIMESTAMP literals ───

datetimeLiteral
    : DATE STRING_LITERAL
    | TIME STRING_LITERAL
    | TIMESTAMP STRING_LITERAL
    ;

// ─── Column reference or function call ───

columnRefOrFunc
    : NAME functionCallTail windowOver?                             # funcCall
    | NAME DOT NAME functionCallTail windowOver?                    # qualifiedFuncCall
    | NAME DOT dollarContinuation                                   # aliasDollarAccess
    | NAME propertyPathTail                                         # propertyRef
    ;

functionCallTail
    : LPAREN (ALL | DISTINCT)? functionArgs? RPAREN
    ;

functionArgs
    : valueExp (COMMA valueExp)*
    ;

dollarContinuation
    : DOLLAR (ARROW propertyPath PARAMETER?)?
    ;

propertyPathTail
    : arrayIndex? (DOT NAME arrayIndex?)*
        (ARROW propertyPath PARAMETER?)?
    ;

arrayIndex
    : LBRACKET INTEGER_LITERAL RBRACKET
    ;

// ─── Property path (used in -> extraction RHS) ───

propertyPath
    : propertyPathEntry (DOT propertyPathEntry)*
    ;

propertyPathEntry
    : identifier arrayIndex?
    ;

// ─── Property name expression (assignment LHS, column lists) ───

propertyNameExp
    : NAME arrayIndex? (DOT NAME arrayIndex?)*
    ;

// ─── Window function ───

windowOver
    : filterClause? OVER windowNameOrSpec
    ;

windowNameOrSpec
    : identifier
    | LPAREN windowSpecification RPAREN
    ;

filterClause
    : FILTER LPAREN whereClause RPAREN
    ;

// ─── Window specification ───

windowSpecification
    : NAME? partitionClause? windowOrderBy? frameClause?
    ;

partitionClause
    : PARTITION BY partitionColumnRef (COMMA partitionColumnRef)*
    ;

partitionColumnRef
    : columnRefOrFunc collateClause?
    ;

collateClause
    : COLLATE (BINARY | NOCASE | RTRIM)
    ;

windowOrderBy
    : ORDER BY orderBySpec (COMMA orderBySpec)*
    ;

// ─── Window frame clause ───

frameClause
    : frameUnit frameExtent frameExclusion?
    ;

frameUnit
    : ROWS
    | RANGE
    | GROUPS
    ;

frameExtent
    : frameStart
    | BETWEEN firstFrameBound AND secondFrameBound
    ;

frameStart
    : UNBOUNDED PRECEDING
    | CURRENT ROW
    | valueExp PRECEDING
    ;

firstFrameBound
    : UNBOUNDED PRECEDING
    | CURRENT ROW
    | valueExp PRECEDING
    | valueExp FOLLOWING
    ;

secondFrameBound
    : UNBOUNDED FOLLOWING
    | CURRENT ROW
    | valueExp PRECEDING
    | valueExp FOLLOWING
    ;

frameExclusion
    : EXCLUDE GROUP
    | EXCLUDE TIES
    | EXCLUDE CURRENT ROW
    | EXCLUDE NO OTHERS
    ;

// ─── Search condition (boolean expressions) ───

searchCondition
    : searchConditionOr
    ;

searchConditionOr
    : searchConditionAnd (OR searchConditionAnd)*
    ;

searchConditionAnd
    : searchConditionNot (AND searchConditionNot)*
    ;

searchConditionNot
    : NOT? predicateExpr
    ;

// ─── Predicate ───

predicateExpr
    : LPAREN searchCondition RPAREN                                 # parenBoolExpr
    | EXISTS subquery                                               # existsPredicate
    | UNIQUE subquery                                               # uniquePredicate
    | valueExp IS NOT? isRhsLiteral                                 # isNullPredicate
    | valueExp IS NOT? typePredicate                                # isTypePredicate
    | valueExp NOT? BETWEEN valueExp AND valueExp                   # betweenPredicate
    | valueExp NOT? LIKE valueExp escapeClause?                     # likePredicate
    | valueExp NOT? IN inRhs                                        # inPredicate
    | valueExp NOT? MATCH functionCallTail                          # matchPredicate
    | valueExp comparisonOp quantifier subquery                     # quantifiedComparison
    | valueExp comparisonOp valueExp                                # comparisonPredicate
    | valueExp                                                      # booleanValuePredicate
    ;

comparisonOp
    : EQUAL | LESS | GREAT | LESS_EQ | GREAT_EQ | NOT_EQUAL
    ;

quantifier
    : ALL | ANY | SOME
    ;

// ─── Type predicate ───

typePredicate
    : LPAREN typeListEntry (COMMA typeListEntry)* RPAREN
    ;

typeListEntry
    : polymorphicConstraint? tableNode
    ;

// ─── IN predicate RHS ───

inRhs
    : subquery
    | LPAREN valueExpCommalist RPAREN
    ;

// ─── LIKE ESCAPE ───

escapeClause
    : ESCAPE STRING_LITERAL
    ;

// ─── Value expression comma list ───

valueExpCommalist
    : valueExp (COMMA valueExp)*
    ;

// ─── Identifier — NAME or keyword used as identifier ───

identifier
    : NAME
    | nonReservedKeyword
    ;

nonReservedKeyword
    : ALL | ANY | ASC | AVG | BACKWARD
    | BINARY | BLOB | BOOLEAN | BY
    | CASE | CAST | COLLATE | COUNT | CROSS
    | CUME_DIST | CURRENT | CURRENT_DATE | CURRENT_TIME | CURRENT_TIMESTAMP
    | DATE | DENSE_RANK | DESC | DISTINCT | DOUBLE
    | ECSQLOPTIONS | ELSE | END | ESCAPE | EVERY | EXCEPT
    | EXCLUDE | EXISTS | FALSE_ | FILTER | FIRST | FIRST_VALUE
    | FLOAT | FOLLOWING | FOR | FORWARD | FULL
    | GROUP | GROUP_CONCAT | GROUPS | HAVING
    | IIF | INNER | INT | INT64 | INTERSECT
    | JOIN
    | LAG | LAST | LAST_VALUE | LEAD | LEFT | LIMIT | LONG
    | MAX | MIN | NATURAL | NAVIGATION_VALUE
    | NO | NOCASE | NTH_VALUE | NTILE | NULL_ | NULLS
    | OFFSET | ON | ONLY | OPTIONS | ORDER | OTHERS | OUTER | OVER
    | PARTITION | PERCENT_RANK | PRAGMA | PRECEDING
    | RANGE | RANK | REAL | RECURSIVE | RIGHT
    | ROW | ROWS | ROW_NUMBER | RTRIM
    | SET | SOME | STRING_KW | SUM
    | THEN | TIES | TIME | TIMESTAMP | TOTAL | TRUE_
    | UNBOUNDED | UNION | UNIQUE | UNKNOWN
    | USING | VALUE | VALUES | VARCHAR
    | WHEN | WINDOW
    ;


/* ==========================================================================
 * Lexer Rules
 * ========================================================================== */

// ─── Keywords ───

ALL             : A L L ;
AND             : A N D ;
ANY             : A N Y ;
AS              : A S ;
ASC             : A S C ;
AVG             : A V G ;
BACKWARD        : B A C K W A R D ;
BETWEEN         : B E T W E E N ;
BINARY          : B I N A R Y ;
BLOB            : B L O B ;
BOOLEAN         : B O O L E A N ;
BY              : B Y ;
CASE            : C A S E ;
CAST            : C A S T ;
COLLATE         : C O L L A T E ;
COUNT           : C O U N T ;
CROSS           : C R O S S ;
CUME_DIST       : C U M E '_' D I S T ;
CURRENT         : C U R R E N T ;
CURRENT_DATE    : C U R R E N T '_' D A T E ;
CURRENT_TIME    : C U R R E N T '_' T I M E ;
CURRENT_TIMESTAMP : C U R R E N T '_' T I M E S T A M P ;
DATE            : D A T E ;
DELETE          : D E L E T E ;
DENSE_RANK      : D E N S E '_' R A N K ;
DESC            : D E S C ;
DISTINCT        : D I S T I N C T ;
DOUBLE          : D O U B L E ;
ECSQLOPTIONS    : E C S Q L O P T I O N S | O P T I O N S ;
ELSE            : E L S E ;
END             : E N D ;
ESCAPE          : E S C A P E ;
EVERY           : E V E R Y ;
EXCEPT          : E X C E P T ;
EXCLUDE         : E X C L U D E ;
EXISTS          : E X I S T S ;
FALSE_          : F A L S E ;
FILTER          : F I L T E R ;
FIRST           : F I R S T ;
FIRST_VALUE     : F I R S T '_' V A L U E ;
FLOAT           : F L O A T ;
FOLLOWING       : F O L L O W I N G ;
FOR             : F O R ;
FORWARD         : F O R W A R D ;
FROM            : F R O M ;
FULL            : F U L L ;
GROUP           : G R O U P ;
GROUP_CONCAT    : G R O U P '_' C O N C A T ;
GROUPS          : G R O U P S ;
HAVING          : H A V I N G ;
IIF             : I I F ;
IN              : I N ;
INNER           : I N N E R ;
INSERT          : I N S E R T ;
INT             : I N T | I N T E G E R ;
INT64           : I N T '6' '4' ;
INTERSECT       : I N T E R S E C T ;
INTO            : I N T O ;
IS              : I S ;
JOIN            : J O I N ;
LAG             : L A G ;
LAST            : L A S T ;
LAST_VALUE      : L A S T '_' V A L U E ;
LEAD            : L E A D ;
LEFT            : L E F T ;
LIKE            : L I K E ;
LIMIT           : L I M I T ;
LONG            : L O N G ;
MATCH           : M A T C H ;
MAX             : M A X ;
MIN             : M I N ;
NATURAL         : N A T U R A L ;
NAVIGATION_VALUE : N A V I G A T I O N '_' V A L U E ;
NO              : N O ;
NOCASE          : N O C A S E ;
NOT             : N O T ;
NTH_VALUE       : N T H '_' V A L U E ;
NTILE           : N T I L E ;
NULL_           : N U L L ;
NULLS           : N U L L S ;
OFFSET          : O F F S E T ;
ON              : O N ;
ONLY            : O N L Y ;
OPTIONS         : O P T I O N S ;
OR              : O R ;
ORDER           : O R D E R ;
OTHERS          : O T H E R S ;
OUTER           : O U T E R ;
OVER            : O V E R ;
PARTITION       : P A R T I T I O N ;
PERCENT_RANK    : P E R C E N T '_' R A N K ;
PRAGMA          : P R A G M A ;
PRECEDING       : P R E C E D I N G ;
RANGE           : R A N G E ;
RANK            : R A N K ;
REAL            : R E A L ;
RECURSIVE       : R E C U R S I V E ;
RIGHT           : R I G H T ;
ROW             : R O W ;
ROWS            : R O W S ;
ROW_NUMBER      : R O W '_' N U M B E R ;
RTRIM           : R T R I M ;
SELECT          : S E L E C T ;
SET             : S E T ;
SOME            : S O M E ;
STRING_KW       : S T R I N G ;
SUM             : S U M ;
THEN            : T H E N ;
TIES            : T I E S ;
TIME            : T I M E ;
TIMESTAMP       : T I M E S T A M P ;
TOTAL           : T O T A L ;
TRUE_           : T R U E ;
UNBOUNDED       : U N B O U N D E D ;
UNION           : U N I O N ;
UNIQUE          : U N I Q U E ;
UNKNOWN         : U N K N O W N ;
UPDATE          : U P D A T E ;
USING           : U S I N G ;
VALUE           : V A L U E ;
VALUES          : V A L U E S ;
VARCHAR         : V A R C H A R ;
WHEN            : W H E N ;
WHERE           : W H E R E ;
WINDOW          : W I N D O W ;
WITH            : W I T H ;

// ─── Operators & Punctuation ───

LESS            : '<' ;
GREAT           : '>' ;
EQUAL           : '=' ;
LESS_EQ         : '<=' ;
GREAT_EQ        : '>=' ;
NOT_EQUAL       : '<>' | '!=' ;
CONCAT          : '||' ;
ARROW           : '->' ;
BITWISE_NOT     : '~' ;
BITWISE_OR      : '|' ;
BITWISE_AND     : '&' ;
SHIFT_LEFT      : '<<' ;
SHIFT_RIGHT     : '>>' ;

MINUS           : '-' ;
PLUS            : '+' ;
STAR            : '*' ;
SLASH           : '/' ;
PERCENT         : '%' ;
COLON           : ':' ;
LPAREN          : '(' ;
RPAREN          : ')' ;
COMMA           : ',' ;
DOT             : '.' ;
SEMICOLON       : ';' ;
LBRACE          : '{' ;
RBRACE          : '}' ;
LBRACKET        : '[' ;
RBRACKET        : ']' ;
DOLLAR          : '$' ;

// ─── Literals & Identifiers ───

INTEGER_LITERAL
    : DIGIT+
    | '0' [xX] HEX_DIGIT+
    ;

APPROX_LITERAL
    : DIGIT+ '.' DIGIT* EXPONENT?
    | '.' DIGIT+ EXPONENT?
    | DIGIT+ EXPONENT
    ;

STRING_LITERAL
    : '\'' ( ~'\'' | '\'\'' )* '\''
    ;

PARAMETER
    : '?'
    ;

NAMED_PARAM
    : ':' NAME_CHARS+
    ;

NAME
    : NAME_START NAME_CHARS*
    | QUOTED_IDENTIFIER
    ;

// ─── Fragments ───

fragment QUOTED_IDENTIFIER
    : '"'  ( ~'"'  | '""' )*  '"'
    | '`'  ( ~'`'  | '``' )*  '`'
    | '['  ~']'*  ']'
    ;

fragment DIGIT      : [0-9] ;
fragment HEX_DIGIT  : [0-9a-fA-F] ;
fragment EXPONENT   : [eE] [+-]? DIGIT+ ;
fragment NAME_START : [a-zA-Z_] ;
fragment NAME_CHARS : [a-zA-Z0-9_] ;

// Case-insensitive keyword character fragments
fragment A : [aA] ; fragment B : [bB] ; fragment C : [cC] ; fragment D : [dD] ;
fragment E : [eE] ; fragment F : [fF] ; fragment G : [gG] ; fragment H : [hH] ;
fragment I : [iI] ; fragment J : [jJ] ; fragment K : [kK] ; fragment L : [lL] ;
fragment M : [mM] ; fragment N : [nN] ; fragment O : [oO] ; fragment P : [pP] ;
fragment Q : [qQ] ; fragment R : [rR] ; fragment S : [sS] ; fragment T : [tT] ;
fragment U : [uU] ; fragment V : [vV] ; fragment W : [wW] ; fragment X : [xX] ;
fragment Y : [yY] ; fragment Z : [zZ] ;

// ─── Skip whitespace and comments ───

WS              : [ \t\r\n]+ -> skip ;
LINE_COMMENT    : '--' ~[\r\n]* -> skip ;
BLOCK_COMMENT   : '/*' .*? '*/' -> skip ;

/* ==========================================================================
 * Notes on correspondence with ECSqlRDParser.cpp:
 *
 * 1. PRECEDENCE: Value expression precedence is encoded structurally via
 *    the rule chain: valueExpOr → valueExpAnd → valueExpShift →
 *    valueExpAddSub → valueExpMulDiv → valueExpConcat → valueExpUnary →
 *    valueExpPrimary. This matches the ACTUAL precedence in the C++ parser
 *    (not the misleading method names — see BISON grammar notes).
 *
 * 2. AMBIGUITY: ParsePredicate in the C++ parser uses speculative
 *    backtracking for parenthesised expressions. ANTLR 4's ALL(*) parsing
 *    algorithm handles this naturally via lookahead, so no special
 *    annotation is needed. The predicateExpr alternatives are ordered
 *    with specific forms first.
 *
 * 3. NAMED_PARAM DUAL USE: The lexer emits NAMED_PARAM for both :param
 *    (query parameters) and schema:ClassName. The parser rules
 *    disambiguate based on context (e.g., tableNode uses NAMED_PARAM
 *    after a NAME for schema:Class notation).
 *
 * 4. VIEW CLASS EXPANSION: Handled at the semantic level, not in grammar.
 *
 * 5. KEYWORD CONFLICTS: ECSQLOPTIONS matches both "ECSQLOPTIONS" and
 *    "OPTIONS". INT matches both "INT" and "INTEGER". These are handled
 *    in the lexer rule alternatives.
 *
 * 6. AGGREGATE vs WINDOW FUNCTION DISAMBIGUATION: The C++ parser checks
 *    function names to select aggregate vs. general function parsing.
 *    In ANTLR, this is handled uniformly via columnRefOrFunc and the
 *    optional windowOver clause. Semantic validation is deferred to the
 *    listener/visitor.
 * ========================================================================== */
