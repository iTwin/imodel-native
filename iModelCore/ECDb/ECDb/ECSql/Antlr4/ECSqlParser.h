
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"




class  ECSqlParser : public antlr4::Parser {
public:
  enum {
    SCOL = 1, DOT = 2, OPEN_PAR = 3, CLOSE_PAR = 4, COMMA = 5, ASSIGN = 6,
    STAR = 7, PLUS = 8, MINUS = 9, TILDE = 10, PIPE2 = 11, DIV = 12, MOD = 13,
    LT2 = 14, GT2 = 15, AMP = 16, PIPE = 17, LT = 18, LT_EQ = 19, GT = 20,
    GT_EQ = 21, EQ = 22, NOT_EQ1 = 23, NOT_EQ2 = 24, DOLLAR = 25, ARROW = 26,
    ABORT_ = 27, ACTION_ = 28, ADD_ = 29, AFTER_ = 30, ALL_ = 31, ALTER_ = 32,
    ANALYZE_ = 33, AND_ = 34, AS_ = 35, ASC_ = 36, ATTACH_ = 37, AUTOINCREMENT_ = 38,
    BEFORE_ = 39, BEGIN_ = 40, BETWEEN_ = 41, BY_ = 42, CASCADE_ = 43, CASE_ = 44,
    CAST_ = 45, CHECK_ = 46, COLLATE_ = 47, COLUMN_ = 48, COMMIT_ = 49,
    CONFLICT_ = 50, CONSTRAINT_ = 51, CREATE_ = 52, CROSS_ = 53, CURRENT_DATE_ = 54,
    CURRENT_TIME_ = 55, CURRENT_TIMESTAMP_ = 56, DATABASE_ = 57, DEFAULT_ = 58,
    DEFERRABLE_ = 59, DEFERRED_ = 60, DELETE_ = 61, DESC_ = 62, DETACH_ = 63,
    DISTINCT_ = 64, DROP_ = 65, EACH_ = 66, ELSE_ = 67, END_ = 68, ESCAPE_ = 69,
    EXCEPT_ = 70, EXCLUSIVE_ = 71, EXISTS_ = 72, EXPLAIN_ = 73, FAIL_ = 74,
    FOR_ = 75, FOREIGN_ = 76, FROM_ = 77, FULL_ = 78, GLOB_ = 79, GROUP_ = 80,
    HAVING_ = 81, IF_ = 82, IGNORE_ = 83, IMMEDIATE_ = 84, IN_ = 85, INDEX_ = 86,
    INDEXED_ = 87, INITIALLY_ = 88, INNER_ = 89, INSERT_ = 90, INSTEAD_ = 91,
    INTERSECT_ = 92, INTO_ = 93, IS_ = 94, ISNULL_ = 95, JOIN_ = 96, KEY_ = 97,
    LEFT_ = 98, LIKE_ = 99, LIMIT_ = 100, MATCH_ = 101, NATURAL_ = 102,
    NO_ = 103, NOT_ = 104, NOTNULL_ = 105, NULL_ = 106, OF_ = 107, OFFSET_ = 108,
    ON_ = 109, OR_ = 110, ORDER_ = 111, OUTER_ = 112, PLAN_ = 113, PRAGMA_ = 114,
    PRIMARY_ = 115, QUERY_ = 116, RAISE_ = 117, RECURSIVE_ = 118, REFERENCES_ = 119,
    REGEXP_ = 120, REINDEX_ = 121, RELEASE_ = 122, RENAME_ = 123, REPLACE_ = 124,
    RESTRICT_ = 125, RETURNING_ = 126, RIGHT_ = 127, ROLLBACK_ = 128, ROW_ = 129,
    ROWS_ = 130, SAVEPOINT_ = 131, SELECT_ = 132, SET_ = 133, TABLE_ = 134,
    TEMP_ = 135, TEMPORARY_ = 136, THEN_ = 137, TO_ = 138, TRANSACTION_ = 139,
    TRIGGER_ = 140, UNION_ = 141, UNIQUE_ = 142, UPDATE_ = 143, USING_ = 144,
    VACUUM_ = 145, VALUES_ = 146, VIEW_ = 147, VIRTUAL_ = 148, WHEN_ = 149,
    WHERE_ = 150, WITH_ = 151, WITHOUT_ = 152, FIRST_VALUE_ = 153, OVER_ = 154,
    PARTITION_ = 155, RANGE_ = 156, PRECEDING_ = 157, UNBOUNDED_ = 158,
    CURRENT_ = 159, FOLLOWING_ = 160, CUME_DIST_ = 161, DENSE_RANK_ = 162,
    LAG_ = 163, LAST_VALUE_ = 164, LEAD_ = 165, NTH_VALUE_ = 166, NTILE_ = 167,
    PERCENT_RANK_ = 168, RANK_ = 169, ROW_NUMBER_ = 170, GENERATED_ = 171,
    ALWAYS_ = 172, STORED_ = 173, TRUE_ = 174, FALSE_ = 175, WINDOW_ = 176,
    NULLS_ = 177, FIRST_ = 178, LAST_ = 179, FILTER_ = 180, GROUPS_ = 181,
    EXCLUDE_ = 182, TIES_ = 183, OTHERS_ = 184, DO_ = 185, NOTHING_ = 186,
    IDENTIFIER = 187, NUMERIC_LITERAL = 188, BIND_PARAMETER = 189, STRING_LITERAL = 190,
    BLOB_LITERAL = 191, SINGLE_LINE_COMMENT = 192, MULTILINE_COMMENT = 193,
    SPACES = 194, UNEXPECTED_CHAR = 195
  };

  enum {
    RuleParse = 0, RuleSql_stmt_list = 1, RuleSql_stmt = 2, RuleAnalyze_stmt = 3,
    RuleAttach_stmt = 4, RuleType_name = 5, RuleSigned_number = 6, RuleWith_clause = 7,
    RuleCte_class_name = 8, RuleRecursive_cte = 9, RuleCommon_class_expression = 10,
    RuleDelete_stmt = 11, RuleDetach_stmt = 12, RuleAccess_string = 13,
    RuleExpr = 14, RuleLiteral_value = 15, RuleValue_row = 16, RuleValues_clause = 17,
    RuleInsert_stmt = 18, RuleReturning_clause = 19, RuleIndexed_property = 20,
    RuleUpsert_clause = 21, RulePragma_stmt = 22, RulePragma_value = 23,
    RuleSelect_stmt = 24, RuleJoin_clause = 25, RuleSelect_core = 26, RuleFactored_select_stmt = 27,
    RuleSimple_select_stmt = 28, RuleCompound_select_stmt = 29, RuleClass_or_subquery = 30,
    RuleResult_property = 31, RuleJoin_operator = 32, RuleJoin_constraint = 33,
    RuleCompound_operator = 34, RuleUpdate_stmt = 35, RuleQualified_class_name = 36,
    RuleVacuum_stmt = 37, RuleFilter_clause = 38, RuleWindow_defn = 39,
    RuleOver_clause = 40, RuleFrame_spec = 41, RuleFrame_clause = 42, RuleSimple_function_invocation = 43,
    RuleAggregate_function_invocation = 44, RuleWindow_function_invocation = 45,
    RuleCommon_class_stmt = 46, RuleOrder_by_stmt = 47, RuleLimit_stmt = 48,
    RuleOrdering_term = 49, RuleAsc_desc = 50, RuleFrame_left = 51, RuleFrame_right = 52,
    RuleFrame_single = 53, RuleWindow_function = 54, RuleOffset = 55, RuleDefault_value = 56,
    RulePartition_by = 57, RuleOrder_by_expr = 58, RuleOrder_by_expr_asc_desc = 59,
    RuleExpr_asc_desc = 60, RuleInitial_select = 61, RuleRecursive_select = 62,
    RuleUnary_operator = 63, RuleError_message = 64, RuleProperty_alias = 65,
    RuleKeyword = 66, RuleName = 67, RuleFunction_name = 68, RuleSchema_name = 69,
    RuleClass_name = 70, RuleClass_or_index_name = 71, RuleProperty_name = 72,
    RuleCollation_name = 73, RuleIndex_name = 74, RuleModule_name = 75,
    RulePragma_name = 76, RuleClass_alias = 77, RuleWindow_name = 78, RuleAlias = 79,
    RuleFilename = 80, RuleBase_window_name = 81, RuleSimple_func = 82,
    RuleAggregate_func = 83, RuleClass_function_name = 84, RuleAny_name = 85
  };

  explicit ECSqlParser(antlr4::TokenStream *input);

  ECSqlParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ECSqlParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ParseContext;
  class Sql_stmt_listContext;
  class Sql_stmtContext;
  class Analyze_stmtContext;
  class Attach_stmtContext;
  class Type_nameContext;
  class Signed_numberContext;
  class With_clauseContext;
  class Cte_class_nameContext;
  class Recursive_cteContext;
  class Common_class_expressionContext;
  class Delete_stmtContext;
  class Detach_stmtContext;
  class Access_stringContext;
  class ExprContext;
  class Literal_valueContext;
  class Value_rowContext;
  class Values_clauseContext;
  class Insert_stmtContext;
  class Returning_clauseContext;
  class Indexed_propertyContext;
  class Upsert_clauseContext;
  class Pragma_stmtContext;
  class Pragma_valueContext;
  class Select_stmtContext;
  class Join_clauseContext;
  class Select_coreContext;
  class Factored_select_stmtContext;
  class Simple_select_stmtContext;
  class Compound_select_stmtContext;
  class Class_or_subqueryContext;
  class Result_propertyContext;
  class Join_operatorContext;
  class Join_constraintContext;
  class Compound_operatorContext;
  class Update_stmtContext;
  class Qualified_class_nameContext;
  class Vacuum_stmtContext;
  class Filter_clauseContext;
  class Window_defnContext;
  class Over_clauseContext;
  class Frame_specContext;
  class Frame_clauseContext;
  class Simple_function_invocationContext;
  class Aggregate_function_invocationContext;
  class Window_function_invocationContext;
  class Common_class_stmtContext;
  class Order_by_stmtContext;
  class Limit_stmtContext;
  class Ordering_termContext;
  class Asc_descContext;
  class Frame_leftContext;
  class Frame_rightContext;
  class Frame_singleContext;
  class Window_functionContext;
  class OffsetContext;
  class Default_valueContext;
  class Partition_byContext;
  class Order_by_exprContext;
  class Order_by_expr_asc_descContext;
  class Expr_asc_descContext;
  class Initial_selectContext;
  class Recursive_selectContext;
  class Unary_operatorContext;
  class Error_messageContext;
  class Property_aliasContext;
  class KeywordContext;
  class NameContext;
  class Function_nameContext;
  class Schema_nameContext;
  class Class_nameContext;
  class Class_or_index_nameContext;
  class Property_nameContext;
  class Collation_nameContext;
  class Index_nameContext;
  class Module_nameContext;
  class Pragma_nameContext;
  class Class_aliasContext;
  class Window_nameContext;
  class AliasContext;
  class FilenameContext;
  class Base_window_nameContext;
  class Simple_funcContext;
  class Aggregate_funcContext;
  class Class_function_nameContext;
  class Any_nameContext;

  class  ParseContext : public antlr4::ParserRuleContext {
  public:
    ParseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<Sql_stmt_listContext *> sql_stmt_list();
    Sql_stmt_listContext* sql_stmt_list(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  ParseContext* parse();

  class  Sql_stmt_listContext : public antlr4::ParserRuleContext {
  public:
    Sql_stmt_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Sql_stmtContext *> sql_stmt();
    Sql_stmtContext* sql_stmt(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SCOL();
    antlr4::tree::TerminalNode* SCOL(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Sql_stmt_listContext* sql_stmt_list();

  class  Sql_stmtContext : public antlr4::ParserRuleContext {
  public:
    Sql_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Analyze_stmtContext *analyze_stmt();
    Attach_stmtContext *attach_stmt();
    Delete_stmtContext *delete_stmt();
    Detach_stmtContext *detach_stmt();
    Insert_stmtContext *insert_stmt();
    Pragma_stmtContext *pragma_stmt();
    Select_stmtContext *select_stmt();
    Update_stmtContext *update_stmt();
    Vacuum_stmtContext *vacuum_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Sql_stmtContext* sql_stmt();

  class  Analyze_stmtContext : public antlr4::ParserRuleContext {
  public:
    Analyze_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ANALYZE_();
    Schema_nameContext *schema_name();
    Class_or_index_nameContext *class_or_index_name();
    antlr4::tree::TerminalNode *DOT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Analyze_stmtContext* analyze_stmt();

  class  Attach_stmtContext : public antlr4::ParserRuleContext {
  public:
    Attach_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ATTACH_();
    ExprContext *expr();
    antlr4::tree::TerminalNode *AS_();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DATABASE_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Attach_stmtContext* attach_stmt();

  class  Type_nameContext : public antlr4::ParserRuleContext {
  public:
    Type_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<Signed_numberContext *> signed_number();
    Signed_numberContext* signed_number(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Type_nameContext* type_name();

  class  Signed_numberContext : public antlr4::ParserRuleContext {
  public:
    Signed_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMERIC_LITERAL();
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *MINUS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Signed_numberContext* signed_number();

  class  With_clauseContext : public antlr4::ParserRuleContext {
  public:
    With_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WITH_();
    std::vector<Cte_class_nameContext *> cte_class_name();
    Cte_class_nameContext* cte_class_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> AS_();
    antlr4::tree::TerminalNode* AS_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> OPEN_PAR();
    antlr4::tree::TerminalNode* OPEN_PAR(size_t i);
    std::vector<Select_stmtContext *> select_stmt();
    Select_stmtContext* select_stmt(size_t i);
    std::vector<antlr4::tree::TerminalNode *> CLOSE_PAR();
    antlr4::tree::TerminalNode* CLOSE_PAR(size_t i);
    antlr4::tree::TerminalNode *RECURSIVE_();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  With_clauseContext* with_clause();

  class  Cte_class_nameContext : public antlr4::ParserRuleContext {
  public:
    Cte_class_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_nameContext *class_name();
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Cte_class_nameContext* cte_class_name();

  class  Recursive_cteContext : public antlr4::ParserRuleContext {
  public:
    Recursive_cteContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Cte_class_nameContext *cte_class_name();
    antlr4::tree::TerminalNode *AS_();
    antlr4::tree::TerminalNode *OPEN_PAR();
    Initial_selectContext *initial_select();
    antlr4::tree::TerminalNode *UNION_();
    Recursive_selectContext *recursive_select();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    antlr4::tree::TerminalNode *ALL_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Recursive_cteContext* recursive_cte();

  class  Common_class_expressionContext : public antlr4::ParserRuleContext {
  public:
    Common_class_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_nameContext *class_name();
    antlr4::tree::TerminalNode *AS_();
    std::vector<antlr4::tree::TerminalNode *> OPEN_PAR();
    antlr4::tree::TerminalNode* OPEN_PAR(size_t i);
    Select_stmtContext *select_stmt();
    std::vector<antlr4::tree::TerminalNode *> CLOSE_PAR();
    antlr4::tree::TerminalNode* CLOSE_PAR(size_t i);
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Common_class_expressionContext* common_class_expression();

  class  Delete_stmtContext : public antlr4::ParserRuleContext {
  public:
    Delete_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DELETE_();
    antlr4::tree::TerminalNode *FROM_();
    Qualified_class_nameContext *qualified_class_name();
    With_clauseContext *with_clause();
    antlr4::tree::TerminalNode *WHERE_();
    ExprContext *expr();
    Returning_clauseContext *returning_clause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Delete_stmtContext* delete_stmt();

  class  Detach_stmtContext : public antlr4::ParserRuleContext {
  public:
    Detach_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DETACH_();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DATABASE_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Detach_stmtContext* detach_stmt();

  class  Access_stringContext : public antlr4::ParserRuleContext {
  public:
    Access_stringContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    Class_nameContext *class_name();
    std::vector<antlr4::tree::TerminalNode *> DOT();
    antlr4::tree::TerminalNode* DOT(size_t i);
    Schema_nameContext *schema_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Access_stringContext* access_string();

  class  ExprContext : public antlr4::ParserRuleContext {
  public:
    ExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Literal_valueContext *literal_value();
    antlr4::tree::TerminalNode *BIND_PARAMETER();
    Access_stringContext *access_string();
    Unary_operatorContext *unary_operator();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    Function_nameContext *function_name();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    antlr4::tree::TerminalNode *STAR();
    Filter_clauseContext *filter_clause();
    Over_clauseContext *over_clause();
    antlr4::tree::TerminalNode *DISTINCT_();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    antlr4::tree::TerminalNode *CAST_();
    antlr4::tree::TerminalNode *AS_();
    Type_nameContext *type_name();
    Select_stmtContext *select_stmt();
    antlr4::tree::TerminalNode *EXISTS_();
    antlr4::tree::TerminalNode *NOT_();
    antlr4::tree::TerminalNode *CASE_();
    antlr4::tree::TerminalNode *END_();
    std::vector<antlr4::tree::TerminalNode *> WHEN_();
    antlr4::tree::TerminalNode* WHEN_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> THEN_();
    antlr4::tree::TerminalNode* THEN_(size_t i);
    antlr4::tree::TerminalNode *ELSE_();
    antlr4::tree::TerminalNode *PIPE2();
    antlr4::tree::TerminalNode *DIV();
    antlr4::tree::TerminalNode *MOD();
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *MINUS();
    antlr4::tree::TerminalNode *LT2();
    antlr4::tree::TerminalNode *GT2();
    antlr4::tree::TerminalNode *AMP();
    antlr4::tree::TerminalNode *PIPE();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *LT_EQ();
    antlr4::tree::TerminalNode *GT();
    antlr4::tree::TerminalNode *GT_EQ();
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *EQ();
    antlr4::tree::TerminalNode *NOT_EQ1();
    antlr4::tree::TerminalNode *NOT_EQ2();
    antlr4::tree::TerminalNode *IS_();
    antlr4::tree::TerminalNode *IN_();
    antlr4::tree::TerminalNode *LIKE_();
    antlr4::tree::TerminalNode *GLOB_();
    antlr4::tree::TerminalNode *MATCH_();
    antlr4::tree::TerminalNode *REGEXP_();
    antlr4::tree::TerminalNode *AND_();
    antlr4::tree::TerminalNode *OR_();
    antlr4::tree::TerminalNode *BETWEEN_();
    antlr4::tree::TerminalNode *COLLATE_();
    Collation_nameContext *collation_name();
    antlr4::tree::TerminalNode *ESCAPE_();
    antlr4::tree::TerminalNode *ISNULL_();
    antlr4::tree::TerminalNode *NOTNULL_();
    antlr4::tree::TerminalNode *NULL_();
    Class_nameContext *class_name();
    Class_function_nameContext *class_function_name();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DOT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  ExprContext* expr();
  ExprContext* expr(int precedence);
  class  Literal_valueContext : public antlr4::ParserRuleContext {
  public:
    Literal_valueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMERIC_LITERAL();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *BLOB_LITERAL();
    antlr4::tree::TerminalNode *NULL_();
    antlr4::tree::TerminalNode *TRUE_();
    antlr4::tree::TerminalNode *FALSE_();
    antlr4::tree::TerminalNode *CURRENT_TIME_();
    antlr4::tree::TerminalNode *CURRENT_DATE_();
    antlr4::tree::TerminalNode *CURRENT_TIMESTAMP_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Literal_valueContext* literal_value();

  class  Value_rowContext : public antlr4::ParserRuleContext {
  public:
    Value_rowContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Value_rowContext* value_row();

  class  Values_clauseContext : public antlr4::ParserRuleContext {
  public:
    Values_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VALUES_();
    std::vector<Value_rowContext *> value_row();
    Value_rowContext* value_row(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Values_clauseContext* values_clause();

  class  Insert_stmtContext : public antlr4::ParserRuleContext {
  public:
    Insert_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTO_();
    Class_nameContext *class_name();
    antlr4::tree::TerminalNode *INSERT_();
    antlr4::tree::TerminalNode *REPLACE_();
    antlr4::tree::TerminalNode *OR_();
    antlr4::tree::TerminalNode *DEFAULT_();
    antlr4::tree::TerminalNode *VALUES_();
    With_clauseContext *with_clause();
    antlr4::tree::TerminalNode *ROLLBACK_();
    antlr4::tree::TerminalNode *ABORT_();
    antlr4::tree::TerminalNode *FAIL_();
    antlr4::tree::TerminalNode *IGNORE_();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *AS_();
    Class_aliasContext *class_alias();
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    Returning_clauseContext *returning_clause();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    Values_clauseContext *values_clause();
    Select_stmtContext *select_stmt();
    Upsert_clauseContext *upsert_clause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Insert_stmtContext* insert_stmt();

  class  Returning_clauseContext : public antlr4::ParserRuleContext {
  public:
    Returning_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURNING_();
    std::vector<Result_propertyContext *> result_property();
    Result_propertyContext* result_property(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Returning_clauseContext* returning_clause();

  class  Indexed_propertyContext : public antlr4::ParserRuleContext {
  public:
    Indexed_propertyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Property_nameContext *property_name();
    ExprContext *expr();
    antlr4::tree::TerminalNode *COLLATE_();
    Collation_nameContext *collation_name();
    Asc_descContext *asc_desc();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Indexed_propertyContext* indexed_property();

  class  Upsert_clauseContext : public antlr4::ParserRuleContext {
  public:
    Upsert_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ON_();
    antlr4::tree::TerminalNode *CONFLICT_();
    antlr4::tree::TerminalNode *DO_();
    antlr4::tree::TerminalNode *NOTHING_();
    antlr4::tree::TerminalNode *UPDATE_();
    antlr4::tree::TerminalNode *SET_();
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<Indexed_propertyContext *> indexed_property();
    Indexed_propertyContext* indexed_property(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ASSIGN();
    antlr4::tree::TerminalNode* ASSIGN(size_t i);
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    std::vector<antlr4::tree::TerminalNode *> WHERE_();
    antlr4::tree::TerminalNode* WHERE_(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Upsert_clauseContext* upsert_clause();

  class  Pragma_stmtContext : public antlr4::ParserRuleContext {
  public:
    Pragma_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PRAGMA_();
    Pragma_nameContext *pragma_name();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *ASSIGN();
    Pragma_valueContext *pragma_value();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Pragma_stmtContext* pragma_stmt();

  class  Pragma_valueContext : public antlr4::ParserRuleContext {
  public:
    Pragma_valueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Signed_numberContext *signed_number();
    NameContext *name();
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Pragma_valueContext* pragma_value();

  class  Select_stmtContext : public antlr4::ParserRuleContext {
  public:
    Select_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Select_coreContext *> select_core();
    Select_coreContext* select_core(size_t i);
    Common_class_stmtContext *common_class_stmt();
    std::vector<Compound_operatorContext *> compound_operator();
    Compound_operatorContext* compound_operator(size_t i);
    Order_by_stmtContext *order_by_stmt();
    Limit_stmtContext *limit_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Select_stmtContext* select_stmt();

  class  Join_clauseContext : public antlr4::ParserRuleContext {
  public:
    Join_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Class_or_subqueryContext *> class_or_subquery();
    Class_or_subqueryContext* class_or_subquery(size_t i);
    std::vector<Join_operatorContext *> join_operator();
    Join_operatorContext* join_operator(size_t i);
    std::vector<Join_constraintContext *> join_constraint();
    Join_constraintContext* join_constraint(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Join_clauseContext* join_clause();

  class  Select_coreContext : public antlr4::ParserRuleContext {
  public:
    ECSqlParser::ExprContext *whereExpr = nullptr;
    ECSqlParser::ExprContext *exprContext = nullptr;
    std::vector<ExprContext *> groupByExpr;
    ECSqlParser::ExprContext *havingExpr = nullptr;
    Select_coreContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SELECT_();
    std::vector<Result_propertyContext *> result_property();
    Result_propertyContext* result_property(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    antlr4::tree::TerminalNode *FROM_();
    antlr4::tree::TerminalNode *WHERE_();
    antlr4::tree::TerminalNode *GROUP_();
    antlr4::tree::TerminalNode *BY_();
    antlr4::tree::TerminalNode *WINDOW_();
    std::vector<Window_nameContext *> window_name();
    Window_nameContext* window_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> AS_();
    antlr4::tree::TerminalNode* AS_(size_t i);
    std::vector<Window_defnContext *> window_defn();
    Window_defnContext* window_defn(size_t i);
    antlr4::tree::TerminalNode *DISTINCT_();
    antlr4::tree::TerminalNode *ALL_();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<Class_or_subqueryContext *> class_or_subquery();
    Class_or_subqueryContext* class_or_subquery(size_t i);
    Join_clauseContext *join_clause();
    antlr4::tree::TerminalNode *HAVING_();
    Values_clauseContext *values_clause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Select_coreContext* select_core();

  class  Factored_select_stmtContext : public antlr4::ParserRuleContext {
  public:
    Factored_select_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Select_stmtContext *select_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Factored_select_stmtContext* factored_select_stmt();

  class  Simple_select_stmtContext : public antlr4::ParserRuleContext {
  public:
    Simple_select_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Select_coreContext *select_core();
    Common_class_stmtContext *common_class_stmt();
    Order_by_stmtContext *order_by_stmt();
    Limit_stmtContext *limit_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Simple_select_stmtContext* simple_select_stmt();

  class  Compound_select_stmtContext : public antlr4::ParserRuleContext {
  public:
    Compound_select_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Select_coreContext *> select_core();
    Select_coreContext* select_core(size_t i);
    Common_class_stmtContext *common_class_stmt();
    Order_by_stmtContext *order_by_stmt();
    Limit_stmtContext *limit_stmt();
    std::vector<antlr4::tree::TerminalNode *> UNION_();
    antlr4::tree::TerminalNode* UNION_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> INTERSECT_();
    antlr4::tree::TerminalNode* INTERSECT_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> EXCEPT_();
    antlr4::tree::TerminalNode* EXCEPT_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ALL_();
    antlr4::tree::TerminalNode* ALL_(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Compound_select_stmtContext* compound_select_stmt();

  class  Class_or_subqueryContext : public antlr4::ParserRuleContext {
  public:
    Class_or_subqueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_nameContext *class_name();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DOT();
    Class_aliasContext *class_alias();
    antlr4::tree::TerminalNode *INDEXED_();
    antlr4::tree::TerminalNode *BY_();
    Index_nameContext *index_name();
    antlr4::tree::TerminalNode *NOT_();
    antlr4::tree::TerminalNode *AS_();
    Class_function_nameContext *class_function_name();
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    std::vector<Class_or_subqueryContext *> class_or_subquery();
    Class_or_subqueryContext* class_or_subquery(size_t i);
    Join_clauseContext *join_clause();
    Select_stmtContext *select_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Class_or_subqueryContext* class_or_subquery();

  class  Result_propertyContext : public antlr4::ParserRuleContext {
  public:
    Result_propertyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STAR();
    Class_nameContext *class_name();
    antlr4::tree::TerminalNode *DOT();
    ExprContext *expr();
    Property_aliasContext *property_alias();
    antlr4::tree::TerminalNode *AS_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Result_propertyContext* result_property();

  class  Join_operatorContext : public antlr4::ParserRuleContext {
  public:
    Join_operatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMMA();
    antlr4::tree::TerminalNode *JOIN_();
    antlr4::tree::TerminalNode *NATURAL_();
    antlr4::tree::TerminalNode *LEFT_();
    antlr4::tree::TerminalNode *INNER_();
    antlr4::tree::TerminalNode *CROSS_();
    antlr4::tree::TerminalNode *OUTER_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Join_operatorContext* join_operator();

  class  Join_constraintContext : public antlr4::ParserRuleContext {
  public:
    Join_constraintContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ON_();
    ExprContext *expr();
    antlr4::tree::TerminalNode *USING_();
    antlr4::tree::TerminalNode *OPEN_PAR();
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Join_constraintContext* join_constraint();

  class  Compound_operatorContext : public antlr4::ParserRuleContext {
  public:
    Compound_operatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNION_();
    antlr4::tree::TerminalNode *ALL_();
    antlr4::tree::TerminalNode *INTERSECT_();
    antlr4::tree::TerminalNode *EXCEPT_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Compound_operatorContext* compound_operator();

  class  Update_stmtContext : public antlr4::ParserRuleContext {
  public:
    Update_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UPDATE_();
    Qualified_class_nameContext *qualified_class_name();
    antlr4::tree::TerminalNode *SET_();
    std::vector<antlr4::tree::TerminalNode *> ASSIGN();
    antlr4::tree::TerminalNode* ASSIGN(size_t i);
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<Property_nameContext *> property_name();
    Property_nameContext* property_name(size_t i);
    With_clauseContext *with_clause();
    antlr4::tree::TerminalNode *OR_();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    antlr4::tree::TerminalNode *FROM_();
    antlr4::tree::TerminalNode *WHERE_();
    Returning_clauseContext *returning_clause();
    antlr4::tree::TerminalNode *ROLLBACK_();
    antlr4::tree::TerminalNode *ABORT_();
    antlr4::tree::TerminalNode *REPLACE_();
    antlr4::tree::TerminalNode *FAIL_();
    antlr4::tree::TerminalNode *IGNORE_();
    std::vector<Class_or_subqueryContext *> class_or_subquery();
    Class_or_subqueryContext* class_or_subquery(size_t i);
    Join_clauseContext *join_clause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Update_stmtContext* update_stmt();

  class  Qualified_class_nameContext : public antlr4::ParserRuleContext {
  public:
    Qualified_class_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_nameContext *class_name();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *AS_();
    AliasContext *alias();
    antlr4::tree::TerminalNode *INDEXED_();
    antlr4::tree::TerminalNode *BY_();
    Index_nameContext *index_name();
    antlr4::tree::TerminalNode *NOT_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Qualified_class_nameContext* qualified_class_name();

  class  Vacuum_stmtContext : public antlr4::ParserRuleContext {
  public:
    Vacuum_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VACUUM_();
    Schema_nameContext *schema_name();
    antlr4::tree::TerminalNode *INTO_();
    FilenameContext *filename();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Vacuum_stmtContext* vacuum_stmt();

  class  Filter_clauseContext : public antlr4::ParserRuleContext {
  public:
    Filter_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FILTER_();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *WHERE_();
    ExprContext *expr();
    antlr4::tree::TerminalNode *CLOSE_PAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Filter_clauseContext* filter_clause();

  class  Window_defnContext : public antlr4::ParserRuleContext {
  public:
    Window_defnContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    antlr4::tree::TerminalNode *ORDER_();
    std::vector<antlr4::tree::TerminalNode *> BY_();
    antlr4::tree::TerminalNode* BY_(size_t i);
    std::vector<Ordering_termContext *> ordering_term();
    Ordering_termContext* ordering_term(size_t i);
    Base_window_nameContext *base_window_name();
    antlr4::tree::TerminalNode *PARTITION_();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    Frame_specContext *frame_spec();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Window_defnContext* window_defn();

  class  Over_clauseContext : public antlr4::ParserRuleContext {
  public:
    Over_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OVER_();
    Window_nameContext *window_name();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    Base_window_nameContext *base_window_name();
    antlr4::tree::TerminalNode *PARTITION_();
    std::vector<antlr4::tree::TerminalNode *> BY_();
    antlr4::tree::TerminalNode* BY_(size_t i);
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *ORDER_();
    std::vector<Ordering_termContext *> ordering_term();
    Ordering_termContext* ordering_term(size_t i);
    Frame_specContext *frame_spec();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Over_clauseContext* over_clause();

  class  Frame_specContext : public antlr4::ParserRuleContext {
  public:
    Frame_specContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Frame_clauseContext *frame_clause();
    antlr4::tree::TerminalNode *EXCLUDE_();
    antlr4::tree::TerminalNode *NO_();
    antlr4::tree::TerminalNode *OTHERS_();
    antlr4::tree::TerminalNode *CURRENT_();
    antlr4::tree::TerminalNode *ROW_();
    antlr4::tree::TerminalNode *GROUP_();
    antlr4::tree::TerminalNode *TIES_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Frame_specContext* frame_spec();

  class  Frame_clauseContext : public antlr4::ParserRuleContext {
  public:
    Frame_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RANGE_();
    antlr4::tree::TerminalNode *ROWS_();
    antlr4::tree::TerminalNode *GROUPS_();
    Frame_singleContext *frame_single();
    antlr4::tree::TerminalNode *BETWEEN_();
    Frame_leftContext *frame_left();
    antlr4::tree::TerminalNode *AND_();
    Frame_rightContext *frame_right();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Frame_clauseContext* frame_clause();

  class  Simple_function_invocationContext : public antlr4::ParserRuleContext {
  public:
    Simple_function_invocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Simple_funcContext *simple_func();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *STAR();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Simple_function_invocationContext* simple_function_invocation();

  class  Aggregate_function_invocationContext : public antlr4::ParserRuleContext {
  public:
    Aggregate_function_invocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Aggregate_funcContext *aggregate_func();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *STAR();
    Filter_clauseContext *filter_clause();
    antlr4::tree::TerminalNode *DISTINCT_();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Aggregate_function_invocationContext* aggregate_function_invocation();

  class  Window_function_invocationContext : public antlr4::ParserRuleContext {
  public:
    Window_function_invocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Window_functionContext *window_function();
    antlr4::tree::TerminalNode *OPEN_PAR();
    antlr4::tree::TerminalNode *CLOSE_PAR();
    antlr4::tree::TerminalNode *OVER_();
    Window_defnContext *window_defn();
    Window_nameContext *window_name();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *STAR();
    Filter_clauseContext *filter_clause();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Window_function_invocationContext* window_function_invocation();

  class  Common_class_stmtContext : public antlr4::ParserRuleContext {
  public:
    Common_class_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WITH_();
    std::vector<Common_class_expressionContext *> common_class_expression();
    Common_class_expressionContext* common_class_expression(size_t i);
    antlr4::tree::TerminalNode *RECURSIVE_();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Common_class_stmtContext* common_class_stmt();

  class  Order_by_stmtContext : public antlr4::ParserRuleContext {
  public:
    Order_by_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ORDER_();
    antlr4::tree::TerminalNode *BY_();
    std::vector<Ordering_termContext *> ordering_term();
    Ordering_termContext* ordering_term(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Order_by_stmtContext* order_by_stmt();

  class  Limit_stmtContext : public antlr4::ParserRuleContext {
  public:
    Limit_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LIMIT_();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *OFFSET_();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Limit_stmtContext* limit_stmt();

  class  Ordering_termContext : public antlr4::ParserRuleContext {
  public:
    Ordering_termContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    antlr4::tree::TerminalNode *COLLATE_();
    Collation_nameContext *collation_name();
    Asc_descContext *asc_desc();
    antlr4::tree::TerminalNode *NULLS_();
    antlr4::tree::TerminalNode *FIRST_();
    antlr4::tree::TerminalNode *LAST_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Ordering_termContext* ordering_term();

  class  Asc_descContext : public antlr4::ParserRuleContext {
  public:
    Asc_descContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASC_();
    antlr4::tree::TerminalNode *DESC_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Asc_descContext* asc_desc();

  class  Frame_leftContext : public antlr4::ParserRuleContext {
  public:
    Frame_leftContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    antlr4::tree::TerminalNode *PRECEDING_();
    antlr4::tree::TerminalNode *FOLLOWING_();
    antlr4::tree::TerminalNode *CURRENT_();
    antlr4::tree::TerminalNode *ROW_();
    antlr4::tree::TerminalNode *UNBOUNDED_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Frame_leftContext* frame_left();

  class  Frame_rightContext : public antlr4::ParserRuleContext {
  public:
    Frame_rightContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    antlr4::tree::TerminalNode *PRECEDING_();
    antlr4::tree::TerminalNode *FOLLOWING_();
    antlr4::tree::TerminalNode *CURRENT_();
    antlr4::tree::TerminalNode *ROW_();
    antlr4::tree::TerminalNode *UNBOUNDED_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Frame_rightContext* frame_right();

  class  Frame_singleContext : public antlr4::ParserRuleContext {
  public:
    Frame_singleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    antlr4::tree::TerminalNode *PRECEDING_();
    antlr4::tree::TerminalNode *UNBOUNDED_();
    antlr4::tree::TerminalNode *CURRENT_();
    antlr4::tree::TerminalNode *ROW_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Frame_singleContext* frame_single();

  class  Window_functionContext : public antlr4::ParserRuleContext {
  public:
    Window_functionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> OPEN_PAR();
    antlr4::tree::TerminalNode* OPEN_PAR(size_t i);
    ExprContext *expr();
    std::vector<antlr4::tree::TerminalNode *> CLOSE_PAR();
    antlr4::tree::TerminalNode* CLOSE_PAR(size_t i);
    antlr4::tree::TerminalNode *OVER_();
    Order_by_expr_asc_descContext *order_by_expr_asc_desc();
    antlr4::tree::TerminalNode *FIRST_VALUE_();
    antlr4::tree::TerminalNode *LAST_VALUE_();
    Partition_byContext *partition_by();
    Frame_clauseContext *frame_clause();
    antlr4::tree::TerminalNode *CUME_DIST_();
    antlr4::tree::TerminalNode *PERCENT_RANK_();
    Order_by_exprContext *order_by_expr();
    antlr4::tree::TerminalNode *DENSE_RANK_();
    antlr4::tree::TerminalNode *RANK_();
    antlr4::tree::TerminalNode *ROW_NUMBER_();
    antlr4::tree::TerminalNode *LAG_();
    antlr4::tree::TerminalNode *LEAD_();
    OffsetContext *offset();
    Default_valueContext *default_value();
    antlr4::tree::TerminalNode *NTH_VALUE_();
    antlr4::tree::TerminalNode *COMMA();
    Signed_numberContext *signed_number();
    antlr4::tree::TerminalNode *NTILE_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Window_functionContext* window_function();

  class  OffsetContext : public antlr4::ParserRuleContext {
  public:
    OffsetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMMA();
    Signed_numberContext *signed_number();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  OffsetContext* offset();

  class  Default_valueContext : public antlr4::ParserRuleContext {
  public:
    Default_valueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMMA();
    Signed_numberContext *signed_number();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Default_valueContext* default_value();

  class  Partition_byContext : public antlr4::ParserRuleContext {
  public:
    Partition_byContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PARTITION_();
    antlr4::tree::TerminalNode *BY_();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Partition_byContext* partition_by();

  class  Order_by_exprContext : public antlr4::ParserRuleContext {
  public:
    Order_by_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ORDER_();
    antlr4::tree::TerminalNode *BY_();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Order_by_exprContext* order_by_expr();

  class  Order_by_expr_asc_descContext : public antlr4::ParserRuleContext {
  public:
    Order_by_expr_asc_descContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ORDER_();
    antlr4::tree::TerminalNode *BY_();
    Expr_asc_descContext *expr_asc_desc();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Order_by_expr_asc_descContext* order_by_expr_asc_desc();

  class  Expr_asc_descContext : public antlr4::ParserRuleContext {
  public:
    Expr_asc_descContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<Asc_descContext *> asc_desc();
    Asc_descContext* asc_desc(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Expr_asc_descContext* expr_asc_desc();

  class  Initial_selectContext : public antlr4::ParserRuleContext {
  public:
    Initial_selectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Select_stmtContext *select_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Initial_selectContext* initial_select();

  class  Recursive_selectContext : public antlr4::ParserRuleContext {
  public:
    Recursive_selectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Select_stmtContext *select_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Recursive_selectContext* recursive_select();

  class  Unary_operatorContext : public antlr4::ParserRuleContext {
  public:
    Unary_operatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MINUS();
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *TILDE();
    antlr4::tree::TerminalNode *NOT_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Unary_operatorContext* unary_operator();

  class  Error_messageContext : public antlr4::ParserRuleContext {
  public:
    Error_messageContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Error_messageContext* error_message();

  class  Property_aliasContext : public antlr4::ParserRuleContext {
  public:
    Property_aliasContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Property_aliasContext* property_alias();

  class  KeywordContext : public antlr4::ParserRuleContext {
  public:
    KeywordContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ABORT_();
    antlr4::tree::TerminalNode *ACTION_();
    antlr4::tree::TerminalNode *ADD_();
    antlr4::tree::TerminalNode *AFTER_();
    antlr4::tree::TerminalNode *ALL_();
    antlr4::tree::TerminalNode *ALTER_();
    antlr4::tree::TerminalNode *ANALYZE_();
    antlr4::tree::TerminalNode *AND_();
    antlr4::tree::TerminalNode *AS_();
    antlr4::tree::TerminalNode *ASC_();
    antlr4::tree::TerminalNode *ATTACH_();
    antlr4::tree::TerminalNode *AUTOINCREMENT_();
    antlr4::tree::TerminalNode *BEFORE_();
    antlr4::tree::TerminalNode *BEGIN_();
    antlr4::tree::TerminalNode *BETWEEN_();
    antlr4::tree::TerminalNode *BY_();
    antlr4::tree::TerminalNode *CASCADE_();
    antlr4::tree::TerminalNode *CASE_();
    antlr4::tree::TerminalNode *CAST_();
    antlr4::tree::TerminalNode *CHECK_();
    antlr4::tree::TerminalNode *COLLATE_();
    antlr4::tree::TerminalNode *COLUMN_();
    antlr4::tree::TerminalNode *COMMIT_();
    antlr4::tree::TerminalNode *CONFLICT_();
    antlr4::tree::TerminalNode *CONSTRAINT_();
    antlr4::tree::TerminalNode *CREATE_();
    antlr4::tree::TerminalNode *CROSS_();
    antlr4::tree::TerminalNode *CURRENT_DATE_();
    antlr4::tree::TerminalNode *CURRENT_TIME_();
    antlr4::tree::TerminalNode *CURRENT_TIMESTAMP_();
    antlr4::tree::TerminalNode *DATABASE_();
    antlr4::tree::TerminalNode *DEFAULT_();
    antlr4::tree::TerminalNode *DEFERRABLE_();
    antlr4::tree::TerminalNode *DEFERRED_();
    antlr4::tree::TerminalNode *DELETE_();
    antlr4::tree::TerminalNode *DESC_();
    antlr4::tree::TerminalNode *DETACH_();
    antlr4::tree::TerminalNode *DISTINCT_();
    antlr4::tree::TerminalNode *DROP_();
    antlr4::tree::TerminalNode *EACH_();
    antlr4::tree::TerminalNode *ELSE_();
    antlr4::tree::TerminalNode *END_();
    antlr4::tree::TerminalNode *ESCAPE_();
    antlr4::tree::TerminalNode *EXCEPT_();
    antlr4::tree::TerminalNode *EXCLUSIVE_();
    antlr4::tree::TerminalNode *EXISTS_();
    antlr4::tree::TerminalNode *EXPLAIN_();
    antlr4::tree::TerminalNode *FAIL_();
    antlr4::tree::TerminalNode *FOR_();
    antlr4::tree::TerminalNode *FOREIGN_();
    antlr4::tree::TerminalNode *FROM_();
    antlr4::tree::TerminalNode *FULL_();
    antlr4::tree::TerminalNode *GLOB_();
    antlr4::tree::TerminalNode *GROUP_();
    antlr4::tree::TerminalNode *HAVING_();
    antlr4::tree::TerminalNode *IF_();
    antlr4::tree::TerminalNode *IGNORE_();
    antlr4::tree::TerminalNode *IMMEDIATE_();
    antlr4::tree::TerminalNode *IN_();
    antlr4::tree::TerminalNode *INDEX_();
    antlr4::tree::TerminalNode *INDEXED_();
    antlr4::tree::TerminalNode *INITIALLY_();
    antlr4::tree::TerminalNode *INNER_();
    antlr4::tree::TerminalNode *INSERT_();
    antlr4::tree::TerminalNode *INSTEAD_();
    antlr4::tree::TerminalNode *INTERSECT_();
    antlr4::tree::TerminalNode *INTO_();
    antlr4::tree::TerminalNode *IS_();
    antlr4::tree::TerminalNode *ISNULL_();
    antlr4::tree::TerminalNode *JOIN_();
    antlr4::tree::TerminalNode *KEY_();
    antlr4::tree::TerminalNode *LEFT_();
    antlr4::tree::TerminalNode *LIKE_();
    antlr4::tree::TerminalNode *LIMIT_();
    antlr4::tree::TerminalNode *MATCH_();
    antlr4::tree::TerminalNode *NATURAL_();
    antlr4::tree::TerminalNode *NO_();
    antlr4::tree::TerminalNode *NOT_();
    antlr4::tree::TerminalNode *NOTNULL_();
    antlr4::tree::TerminalNode *NULL_();
    antlr4::tree::TerminalNode *OF_();
    antlr4::tree::TerminalNode *OFFSET_();
    antlr4::tree::TerminalNode *ON_();
    antlr4::tree::TerminalNode *OR_();
    antlr4::tree::TerminalNode *ORDER_();
    antlr4::tree::TerminalNode *OUTER_();
    antlr4::tree::TerminalNode *PLAN_();
    antlr4::tree::TerminalNode *PRAGMA_();
    antlr4::tree::TerminalNode *PRIMARY_();
    antlr4::tree::TerminalNode *QUERY_();
    antlr4::tree::TerminalNode *RAISE_();
    antlr4::tree::TerminalNode *RECURSIVE_();
    antlr4::tree::TerminalNode *REFERENCES_();
    antlr4::tree::TerminalNode *REGEXP_();
    antlr4::tree::TerminalNode *REINDEX_();
    antlr4::tree::TerminalNode *RELEASE_();
    antlr4::tree::TerminalNode *RENAME_();
    antlr4::tree::TerminalNode *REPLACE_();
    antlr4::tree::TerminalNode *RESTRICT_();
    antlr4::tree::TerminalNode *RIGHT_();
    antlr4::tree::TerminalNode *ROLLBACK_();
    antlr4::tree::TerminalNode *ROW_();
    antlr4::tree::TerminalNode *ROWS_();
    antlr4::tree::TerminalNode *SAVEPOINT_();
    antlr4::tree::TerminalNode *SELECT_();
    antlr4::tree::TerminalNode *SET_();
    antlr4::tree::TerminalNode *TABLE_();
    antlr4::tree::TerminalNode *TEMP_();
    antlr4::tree::TerminalNode *TEMPORARY_();
    antlr4::tree::TerminalNode *THEN_();
    antlr4::tree::TerminalNode *TO_();
    antlr4::tree::TerminalNode *TRANSACTION_();
    antlr4::tree::TerminalNode *TRIGGER_();
    antlr4::tree::TerminalNode *UNION_();
    antlr4::tree::TerminalNode *UNIQUE_();
    antlr4::tree::TerminalNode *UPDATE_();
    antlr4::tree::TerminalNode *USING_();
    antlr4::tree::TerminalNode *VACUUM_();
    antlr4::tree::TerminalNode *VALUES_();
    antlr4::tree::TerminalNode *VIEW_();
    antlr4::tree::TerminalNode *VIRTUAL_();
    antlr4::tree::TerminalNode *WHEN_();
    antlr4::tree::TerminalNode *WHERE_();
    antlr4::tree::TerminalNode *WITH_();
    antlr4::tree::TerminalNode *WITHOUT_();
    antlr4::tree::TerminalNode *FIRST_VALUE_();
    antlr4::tree::TerminalNode *OVER_();
    antlr4::tree::TerminalNode *PARTITION_();
    antlr4::tree::TerminalNode *RANGE_();
    antlr4::tree::TerminalNode *PRECEDING_();
    antlr4::tree::TerminalNode *UNBOUNDED_();
    antlr4::tree::TerminalNode *CURRENT_();
    antlr4::tree::TerminalNode *FOLLOWING_();
    antlr4::tree::TerminalNode *CUME_DIST_();
    antlr4::tree::TerminalNode *DENSE_RANK_();
    antlr4::tree::TerminalNode *LAG_();
    antlr4::tree::TerminalNode *LAST_VALUE_();
    antlr4::tree::TerminalNode *LEAD_();
    antlr4::tree::TerminalNode *NTH_VALUE_();
    antlr4::tree::TerminalNode *NTILE_();
    antlr4::tree::TerminalNode *PERCENT_RANK_();
    antlr4::tree::TerminalNode *RANK_();
    antlr4::tree::TerminalNode *ROW_NUMBER_();
    antlr4::tree::TerminalNode *GENERATED_();
    antlr4::tree::TerminalNode *ALWAYS_();
    antlr4::tree::TerminalNode *STORED_();
    antlr4::tree::TerminalNode *TRUE_();
    antlr4::tree::TerminalNode *FALSE_();
    antlr4::tree::TerminalNode *WINDOW_();
    antlr4::tree::TerminalNode *NULLS_();
    antlr4::tree::TerminalNode *FIRST_();
    antlr4::tree::TerminalNode *LAST_();
    antlr4::tree::TerminalNode *FILTER_();
    antlr4::tree::TerminalNode *GROUPS_();
    antlr4::tree::TerminalNode *EXCLUDE_();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  KeywordContext* keyword();

  class  NameContext : public antlr4::ParserRuleContext {
  public:
    NameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  NameContext* name();

  class  Function_nameContext : public antlr4::ParserRuleContext {
  public:
    Function_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Function_nameContext* function_name();

  class  Schema_nameContext : public antlr4::ParserRuleContext {
  public:
    Schema_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Schema_nameContext* schema_name();

  class  Class_nameContext : public antlr4::ParserRuleContext {
  public:
    Class_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Class_nameContext* class_name();

  class  Class_or_index_nameContext : public antlr4::ParserRuleContext {
  public:
    Class_or_index_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Class_or_index_nameContext* class_or_index_name();

  class  Property_nameContext : public antlr4::ParserRuleContext {
  public:
    Property_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Property_nameContext* property_name();

  class  Collation_nameContext : public antlr4::ParserRuleContext {
  public:
    Collation_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Collation_nameContext* collation_name();

  class  Index_nameContext : public antlr4::ParserRuleContext {
  public:
    Index_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Index_nameContext* index_name();

  class  Module_nameContext : public antlr4::ParserRuleContext {
  public:
    Module_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Module_nameContext* module_name();

  class  Pragma_nameContext : public antlr4::ParserRuleContext {
  public:
    Pragma_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Pragma_nameContext* pragma_name();

  class  Class_aliasContext : public antlr4::ParserRuleContext {
  public:
    Class_aliasContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Class_aliasContext* class_alias();

  class  Window_nameContext : public antlr4::ParserRuleContext {
  public:
    Window_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Window_nameContext* window_name();

  class  AliasContext : public antlr4::ParserRuleContext {
  public:
    AliasContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  AliasContext* alias();

  class  FilenameContext : public antlr4::ParserRuleContext {
  public:
    FilenameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  FilenameContext* filename();

  class  Base_window_nameContext : public antlr4::ParserRuleContext {
  public:
    Base_window_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Base_window_nameContext* base_window_name();

  class  Simple_funcContext : public antlr4::ParserRuleContext {
  public:
    Simple_funcContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Simple_funcContext* simple_func();

  class  Aggregate_funcContext : public antlr4::ParserRuleContext {
  public:
    Aggregate_funcContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Aggregate_funcContext* aggregate_func();

  class  Class_function_nameContext : public antlr4::ParserRuleContext {
  public:
    Class_function_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Any_nameContext *any_name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Class_function_nameContext* class_function_name();

  class  Any_nameContext : public antlr4::ParserRuleContext {
  public:
    Any_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    KeywordContext *keyword();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *OPEN_PAR();
    Any_nameContext *any_name();
    antlr4::tree::TerminalNode *CLOSE_PAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  Any_nameContext* any_name();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool exprSempred(ExprContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

