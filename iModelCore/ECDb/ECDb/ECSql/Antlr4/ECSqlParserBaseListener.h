
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"
#include "ECSqlParserListener.h"


/**
 * This class provides an empty implementation of ECSqlParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  ECSqlParserBaseListener : public ECSqlParserListener {
public:

  virtual void enterParse(ECSqlParser::ParseContext * /*ctx*/) override { }
  virtual void exitParse(ECSqlParser::ParseContext * /*ctx*/) override { }

  virtual void enterSql_stmt_list(ECSqlParser::Sql_stmt_listContext * /*ctx*/) override { }
  virtual void exitSql_stmt_list(ECSqlParser::Sql_stmt_listContext * /*ctx*/) override { }

  virtual void enterSql_stmt(ECSqlParser::Sql_stmtContext * /*ctx*/) override { }
  virtual void exitSql_stmt(ECSqlParser::Sql_stmtContext * /*ctx*/) override { }

  virtual void enterAnalyze_stmt(ECSqlParser::Analyze_stmtContext * /*ctx*/) override { }
  virtual void exitAnalyze_stmt(ECSqlParser::Analyze_stmtContext * /*ctx*/) override { }

  virtual void enterAttach_stmt(ECSqlParser::Attach_stmtContext * /*ctx*/) override { }
  virtual void exitAttach_stmt(ECSqlParser::Attach_stmtContext * /*ctx*/) override { }

  virtual void enterType_name(ECSqlParser::Type_nameContext * /*ctx*/) override { }
  virtual void exitType_name(ECSqlParser::Type_nameContext * /*ctx*/) override { }

  virtual void enterSigned_number(ECSqlParser::Signed_numberContext * /*ctx*/) override { }
  virtual void exitSigned_number(ECSqlParser::Signed_numberContext * /*ctx*/) override { }

  virtual void enterWith_clause(ECSqlParser::With_clauseContext * /*ctx*/) override { }
  virtual void exitWith_clause(ECSqlParser::With_clauseContext * /*ctx*/) override { }

  virtual void enterCte_class_name(ECSqlParser::Cte_class_nameContext * /*ctx*/) override { }
  virtual void exitCte_class_name(ECSqlParser::Cte_class_nameContext * /*ctx*/) override { }

  virtual void enterRecursive_cte(ECSqlParser::Recursive_cteContext * /*ctx*/) override { }
  virtual void exitRecursive_cte(ECSqlParser::Recursive_cteContext * /*ctx*/) override { }

  virtual void enterCommon_class_expression(ECSqlParser::Common_class_expressionContext * /*ctx*/) override { }
  virtual void exitCommon_class_expression(ECSqlParser::Common_class_expressionContext * /*ctx*/) override { }

  virtual void enterDelete_stmt(ECSqlParser::Delete_stmtContext * /*ctx*/) override { }
  virtual void exitDelete_stmt(ECSqlParser::Delete_stmtContext * /*ctx*/) override { }

  virtual void enterDetach_stmt(ECSqlParser::Detach_stmtContext * /*ctx*/) override { }
  virtual void exitDetach_stmt(ECSqlParser::Detach_stmtContext * /*ctx*/) override { }

  virtual void enterAccess_string(ECSqlParser::Access_stringContext * /*ctx*/) override { }
  virtual void exitAccess_string(ECSqlParser::Access_stringContext * /*ctx*/) override { }

  virtual void enterExpr(ECSqlParser::ExprContext * /*ctx*/) override { }
  virtual void exitExpr(ECSqlParser::ExprContext * /*ctx*/) override { }

  virtual void enterLiteral_value(ECSqlParser::Literal_valueContext * /*ctx*/) override { }
  virtual void exitLiteral_value(ECSqlParser::Literal_valueContext * /*ctx*/) override { }

  virtual void enterValue_row(ECSqlParser::Value_rowContext * /*ctx*/) override { }
  virtual void exitValue_row(ECSqlParser::Value_rowContext * /*ctx*/) override { }

  virtual void enterValues_clause(ECSqlParser::Values_clauseContext * /*ctx*/) override { }
  virtual void exitValues_clause(ECSqlParser::Values_clauseContext * /*ctx*/) override { }

  virtual void enterInsert_stmt(ECSqlParser::Insert_stmtContext * /*ctx*/) override { }
  virtual void exitInsert_stmt(ECSqlParser::Insert_stmtContext * /*ctx*/) override { }

  virtual void enterReturning_clause(ECSqlParser::Returning_clauseContext * /*ctx*/) override { }
  virtual void exitReturning_clause(ECSqlParser::Returning_clauseContext * /*ctx*/) override { }

  virtual void enterIndexed_property(ECSqlParser::Indexed_propertyContext * /*ctx*/) override { }
  virtual void exitIndexed_property(ECSqlParser::Indexed_propertyContext * /*ctx*/) override { }

  virtual void enterUpsert_clause(ECSqlParser::Upsert_clauseContext * /*ctx*/) override { }
  virtual void exitUpsert_clause(ECSqlParser::Upsert_clauseContext * /*ctx*/) override { }

  virtual void enterPragma_stmt(ECSqlParser::Pragma_stmtContext * /*ctx*/) override { }
  virtual void exitPragma_stmt(ECSqlParser::Pragma_stmtContext * /*ctx*/) override { }

  virtual void enterPragma_value(ECSqlParser::Pragma_valueContext * /*ctx*/) override { }
  virtual void exitPragma_value(ECSqlParser::Pragma_valueContext * /*ctx*/) override { }

  virtual void enterSelect_stmt(ECSqlParser::Select_stmtContext * /*ctx*/) override { }
  virtual void exitSelect_stmt(ECSqlParser::Select_stmtContext * /*ctx*/) override { }

  virtual void enterJoin_clause(ECSqlParser::Join_clauseContext * /*ctx*/) override { }
  virtual void exitJoin_clause(ECSqlParser::Join_clauseContext * /*ctx*/) override { }

  virtual void enterSelect_core(ECSqlParser::Select_coreContext * /*ctx*/) override { }
  virtual void exitSelect_core(ECSqlParser::Select_coreContext * /*ctx*/) override { }

  virtual void enterFactored_select_stmt(ECSqlParser::Factored_select_stmtContext * /*ctx*/) override { }
  virtual void exitFactored_select_stmt(ECSqlParser::Factored_select_stmtContext * /*ctx*/) override { }

  virtual void enterSimple_select_stmt(ECSqlParser::Simple_select_stmtContext * /*ctx*/) override { }
  virtual void exitSimple_select_stmt(ECSqlParser::Simple_select_stmtContext * /*ctx*/) override { }

  virtual void enterCompound_select_stmt(ECSqlParser::Compound_select_stmtContext * /*ctx*/) override { }
  virtual void exitCompound_select_stmt(ECSqlParser::Compound_select_stmtContext * /*ctx*/) override { }

  virtual void enterClass_or_subquery(ECSqlParser::Class_or_subqueryContext * /*ctx*/) override { }
  virtual void exitClass_or_subquery(ECSqlParser::Class_or_subqueryContext * /*ctx*/) override { }

  virtual void enterResult_property(ECSqlParser::Result_propertyContext * /*ctx*/) override { }
  virtual void exitResult_property(ECSqlParser::Result_propertyContext * /*ctx*/) override { }

  virtual void enterJoin_operator(ECSqlParser::Join_operatorContext * /*ctx*/) override { }
  virtual void exitJoin_operator(ECSqlParser::Join_operatorContext * /*ctx*/) override { }

  virtual void enterJoin_constraint(ECSqlParser::Join_constraintContext * /*ctx*/) override { }
  virtual void exitJoin_constraint(ECSqlParser::Join_constraintContext * /*ctx*/) override { }

  virtual void enterCompound_operator(ECSqlParser::Compound_operatorContext * /*ctx*/) override { }
  virtual void exitCompound_operator(ECSqlParser::Compound_operatorContext * /*ctx*/) override { }

  virtual void enterUpdate_stmt(ECSqlParser::Update_stmtContext * /*ctx*/) override { }
  virtual void exitUpdate_stmt(ECSqlParser::Update_stmtContext * /*ctx*/) override { }

  virtual void enterQualified_class_name(ECSqlParser::Qualified_class_nameContext * /*ctx*/) override { }
  virtual void exitQualified_class_name(ECSqlParser::Qualified_class_nameContext * /*ctx*/) override { }

  virtual void enterVacuum_stmt(ECSqlParser::Vacuum_stmtContext * /*ctx*/) override { }
  virtual void exitVacuum_stmt(ECSqlParser::Vacuum_stmtContext * /*ctx*/) override { }

  virtual void enterFilter_clause(ECSqlParser::Filter_clauseContext * /*ctx*/) override { }
  virtual void exitFilter_clause(ECSqlParser::Filter_clauseContext * /*ctx*/) override { }

  virtual void enterWindow_defn(ECSqlParser::Window_defnContext * /*ctx*/) override { }
  virtual void exitWindow_defn(ECSqlParser::Window_defnContext * /*ctx*/) override { }

  virtual void enterOver_clause(ECSqlParser::Over_clauseContext * /*ctx*/) override { }
  virtual void exitOver_clause(ECSqlParser::Over_clauseContext * /*ctx*/) override { }

  virtual void enterFrame_spec(ECSqlParser::Frame_specContext * /*ctx*/) override { }
  virtual void exitFrame_spec(ECSqlParser::Frame_specContext * /*ctx*/) override { }

  virtual void enterFrame_clause(ECSqlParser::Frame_clauseContext * /*ctx*/) override { }
  virtual void exitFrame_clause(ECSqlParser::Frame_clauseContext * /*ctx*/) override { }

  virtual void enterSimple_function_invocation(ECSqlParser::Simple_function_invocationContext * /*ctx*/) override { }
  virtual void exitSimple_function_invocation(ECSqlParser::Simple_function_invocationContext * /*ctx*/) override { }

  virtual void enterAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext * /*ctx*/) override { }
  virtual void exitAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext * /*ctx*/) override { }

  virtual void enterWindow_function_invocation(ECSqlParser::Window_function_invocationContext * /*ctx*/) override { }
  virtual void exitWindow_function_invocation(ECSqlParser::Window_function_invocationContext * /*ctx*/) override { }

  virtual void enterCommon_class_stmt(ECSqlParser::Common_class_stmtContext * /*ctx*/) override { }
  virtual void exitCommon_class_stmt(ECSqlParser::Common_class_stmtContext * /*ctx*/) override { }

  virtual void enterOrder_by_stmt(ECSqlParser::Order_by_stmtContext * /*ctx*/) override { }
  virtual void exitOrder_by_stmt(ECSqlParser::Order_by_stmtContext * /*ctx*/) override { }

  virtual void enterLimit_stmt(ECSqlParser::Limit_stmtContext * /*ctx*/) override { }
  virtual void exitLimit_stmt(ECSqlParser::Limit_stmtContext * /*ctx*/) override { }

  virtual void enterOrdering_term(ECSqlParser::Ordering_termContext * /*ctx*/) override { }
  virtual void exitOrdering_term(ECSqlParser::Ordering_termContext * /*ctx*/) override { }

  virtual void enterAsc_desc(ECSqlParser::Asc_descContext * /*ctx*/) override { }
  virtual void exitAsc_desc(ECSqlParser::Asc_descContext * /*ctx*/) override { }

  virtual void enterFrame_left(ECSqlParser::Frame_leftContext * /*ctx*/) override { }
  virtual void exitFrame_left(ECSqlParser::Frame_leftContext * /*ctx*/) override { }

  virtual void enterFrame_right(ECSqlParser::Frame_rightContext * /*ctx*/) override { }
  virtual void exitFrame_right(ECSqlParser::Frame_rightContext * /*ctx*/) override { }

  virtual void enterFrame_single(ECSqlParser::Frame_singleContext * /*ctx*/) override { }
  virtual void exitFrame_single(ECSqlParser::Frame_singleContext * /*ctx*/) override { }

  virtual void enterWindow_function(ECSqlParser::Window_functionContext * /*ctx*/) override { }
  virtual void exitWindow_function(ECSqlParser::Window_functionContext * /*ctx*/) override { }

  virtual void enterOffset(ECSqlParser::OffsetContext * /*ctx*/) override { }
  virtual void exitOffset(ECSqlParser::OffsetContext * /*ctx*/) override { }

  virtual void enterDefault_value(ECSqlParser::Default_valueContext * /*ctx*/) override { }
  virtual void exitDefault_value(ECSqlParser::Default_valueContext * /*ctx*/) override { }

  virtual void enterPartition_by(ECSqlParser::Partition_byContext * /*ctx*/) override { }
  virtual void exitPartition_by(ECSqlParser::Partition_byContext * /*ctx*/) override { }

  virtual void enterOrder_by_expr(ECSqlParser::Order_by_exprContext * /*ctx*/) override { }
  virtual void exitOrder_by_expr(ECSqlParser::Order_by_exprContext * /*ctx*/) override { }

  virtual void enterOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext * /*ctx*/) override { }
  virtual void exitOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext * /*ctx*/) override { }

  virtual void enterExpr_asc_desc(ECSqlParser::Expr_asc_descContext * /*ctx*/) override { }
  virtual void exitExpr_asc_desc(ECSqlParser::Expr_asc_descContext * /*ctx*/) override { }

  virtual void enterInitial_select(ECSqlParser::Initial_selectContext * /*ctx*/) override { }
  virtual void exitInitial_select(ECSqlParser::Initial_selectContext * /*ctx*/) override { }

  virtual void enterRecursive_select(ECSqlParser::Recursive_selectContext * /*ctx*/) override { }
  virtual void exitRecursive_select(ECSqlParser::Recursive_selectContext * /*ctx*/) override { }

  virtual void enterUnary_operator(ECSqlParser::Unary_operatorContext * /*ctx*/) override { }
  virtual void exitUnary_operator(ECSqlParser::Unary_operatorContext * /*ctx*/) override { }

  virtual void enterError_message(ECSqlParser::Error_messageContext * /*ctx*/) override { }
  virtual void exitError_message(ECSqlParser::Error_messageContext * /*ctx*/) override { }

  virtual void enterProperty_alias(ECSqlParser::Property_aliasContext * /*ctx*/) override { }
  virtual void exitProperty_alias(ECSqlParser::Property_aliasContext * /*ctx*/) override { }

  virtual void enterKeyword(ECSqlParser::KeywordContext * /*ctx*/) override { }
  virtual void exitKeyword(ECSqlParser::KeywordContext * /*ctx*/) override { }

  virtual void enterName(ECSqlParser::NameContext * /*ctx*/) override { }
  virtual void exitName(ECSqlParser::NameContext * /*ctx*/) override { }

  virtual void enterFunction_name(ECSqlParser::Function_nameContext * /*ctx*/) override { }
  virtual void exitFunction_name(ECSqlParser::Function_nameContext * /*ctx*/) override { }

  virtual void enterSchema_name(ECSqlParser::Schema_nameContext * /*ctx*/) override { }
  virtual void exitSchema_name(ECSqlParser::Schema_nameContext * /*ctx*/) override { }

  virtual void enterClass_name(ECSqlParser::Class_nameContext * /*ctx*/) override { }
  virtual void exitClass_name(ECSqlParser::Class_nameContext * /*ctx*/) override { }

  virtual void enterClass_or_index_name(ECSqlParser::Class_or_index_nameContext * /*ctx*/) override { }
  virtual void exitClass_or_index_name(ECSqlParser::Class_or_index_nameContext * /*ctx*/) override { }

  virtual void enterProperty_name(ECSqlParser::Property_nameContext * /*ctx*/) override { }
  virtual void exitProperty_name(ECSqlParser::Property_nameContext * /*ctx*/) override { }

  virtual void enterCollation_name(ECSqlParser::Collation_nameContext * /*ctx*/) override { }
  virtual void exitCollation_name(ECSqlParser::Collation_nameContext * /*ctx*/) override { }

  virtual void enterIndex_name(ECSqlParser::Index_nameContext * /*ctx*/) override { }
  virtual void exitIndex_name(ECSqlParser::Index_nameContext * /*ctx*/) override { }

  virtual void enterModule_name(ECSqlParser::Module_nameContext * /*ctx*/) override { }
  virtual void exitModule_name(ECSqlParser::Module_nameContext * /*ctx*/) override { }

  virtual void enterPragma_name(ECSqlParser::Pragma_nameContext * /*ctx*/) override { }
  virtual void exitPragma_name(ECSqlParser::Pragma_nameContext * /*ctx*/) override { }

  virtual void enterClass_alias(ECSqlParser::Class_aliasContext * /*ctx*/) override { }
  virtual void exitClass_alias(ECSqlParser::Class_aliasContext * /*ctx*/) override { }

  virtual void enterWindow_name(ECSqlParser::Window_nameContext * /*ctx*/) override { }
  virtual void exitWindow_name(ECSqlParser::Window_nameContext * /*ctx*/) override { }

  virtual void enterAlias(ECSqlParser::AliasContext * /*ctx*/) override { }
  virtual void exitAlias(ECSqlParser::AliasContext * /*ctx*/) override { }

  virtual void enterFilename(ECSqlParser::FilenameContext * /*ctx*/) override { }
  virtual void exitFilename(ECSqlParser::FilenameContext * /*ctx*/) override { }

  virtual void enterBase_window_name(ECSqlParser::Base_window_nameContext * /*ctx*/) override { }
  virtual void exitBase_window_name(ECSqlParser::Base_window_nameContext * /*ctx*/) override { }

  virtual void enterSimple_func(ECSqlParser::Simple_funcContext * /*ctx*/) override { }
  virtual void exitSimple_func(ECSqlParser::Simple_funcContext * /*ctx*/) override { }

  virtual void enterAggregate_func(ECSqlParser::Aggregate_funcContext * /*ctx*/) override { }
  virtual void exitAggregate_func(ECSqlParser::Aggregate_funcContext * /*ctx*/) override { }

  virtual void enterClass_function_name(ECSqlParser::Class_function_nameContext * /*ctx*/) override { }
  virtual void exitClass_function_name(ECSqlParser::Class_function_nameContext * /*ctx*/) override { }

  virtual void enterAny_name(ECSqlParser::Any_nameContext * /*ctx*/) override { }
  virtual void exitAny_name(ECSqlParser::Any_nameContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

