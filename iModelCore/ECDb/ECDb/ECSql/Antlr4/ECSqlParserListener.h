
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"
#include "ECSqlParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by ECSqlParser.
 */
class  ECSqlParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterParse(ECSqlParser::ParseContext *ctx) = 0;
  virtual void exitParse(ECSqlParser::ParseContext *ctx) = 0;

  virtual void enterSql_stmt_list(ECSqlParser::Sql_stmt_listContext *ctx) = 0;
  virtual void exitSql_stmt_list(ECSqlParser::Sql_stmt_listContext *ctx) = 0;

  virtual void enterSql_stmt(ECSqlParser::Sql_stmtContext *ctx) = 0;
  virtual void exitSql_stmt(ECSqlParser::Sql_stmtContext *ctx) = 0;

  virtual void enterAnalyze_stmt(ECSqlParser::Analyze_stmtContext *ctx) = 0;
  virtual void exitAnalyze_stmt(ECSqlParser::Analyze_stmtContext *ctx) = 0;

  virtual void enterAttach_stmt(ECSqlParser::Attach_stmtContext *ctx) = 0;
  virtual void exitAttach_stmt(ECSqlParser::Attach_stmtContext *ctx) = 0;

  virtual void enterType_name(ECSqlParser::Type_nameContext *ctx) = 0;
  virtual void exitType_name(ECSqlParser::Type_nameContext *ctx) = 0;

  virtual void enterSigned_number(ECSqlParser::Signed_numberContext *ctx) = 0;
  virtual void exitSigned_number(ECSqlParser::Signed_numberContext *ctx) = 0;

  virtual void enterWith_clause(ECSqlParser::With_clauseContext *ctx) = 0;
  virtual void exitWith_clause(ECSqlParser::With_clauseContext *ctx) = 0;

  virtual void enterCte_class_name(ECSqlParser::Cte_class_nameContext *ctx) = 0;
  virtual void exitCte_class_name(ECSqlParser::Cte_class_nameContext *ctx) = 0;

  virtual void enterRecursive_cte(ECSqlParser::Recursive_cteContext *ctx) = 0;
  virtual void exitRecursive_cte(ECSqlParser::Recursive_cteContext *ctx) = 0;

  virtual void enterCommon_class_expression(ECSqlParser::Common_class_expressionContext *ctx) = 0;
  virtual void exitCommon_class_expression(ECSqlParser::Common_class_expressionContext *ctx) = 0;

  virtual void enterDelete_stmt(ECSqlParser::Delete_stmtContext *ctx) = 0;
  virtual void exitDelete_stmt(ECSqlParser::Delete_stmtContext *ctx) = 0;

  virtual void enterDetach_stmt(ECSqlParser::Detach_stmtContext *ctx) = 0;
  virtual void exitDetach_stmt(ECSqlParser::Detach_stmtContext *ctx) = 0;

  virtual void enterAccess_string(ECSqlParser::Access_stringContext *ctx) = 0;
  virtual void exitAccess_string(ECSqlParser::Access_stringContext *ctx) = 0;

  virtual void enterExpr(ECSqlParser::ExprContext *ctx) = 0;
  virtual void exitExpr(ECSqlParser::ExprContext *ctx) = 0;

  virtual void enterLiteral_value(ECSqlParser::Literal_valueContext *ctx) = 0;
  virtual void exitLiteral_value(ECSqlParser::Literal_valueContext *ctx) = 0;

  virtual void enterValue_row(ECSqlParser::Value_rowContext *ctx) = 0;
  virtual void exitValue_row(ECSqlParser::Value_rowContext *ctx) = 0;

  virtual void enterValues_clause(ECSqlParser::Values_clauseContext *ctx) = 0;
  virtual void exitValues_clause(ECSqlParser::Values_clauseContext *ctx) = 0;

  virtual void enterInsert_stmt(ECSqlParser::Insert_stmtContext *ctx) = 0;
  virtual void exitInsert_stmt(ECSqlParser::Insert_stmtContext *ctx) = 0;

  virtual void enterReturning_clause(ECSqlParser::Returning_clauseContext *ctx) = 0;
  virtual void exitReturning_clause(ECSqlParser::Returning_clauseContext *ctx) = 0;

  virtual void enterIndexed_property(ECSqlParser::Indexed_propertyContext *ctx) = 0;
  virtual void exitIndexed_property(ECSqlParser::Indexed_propertyContext *ctx) = 0;

  virtual void enterUpsert_clause(ECSqlParser::Upsert_clauseContext *ctx) = 0;
  virtual void exitUpsert_clause(ECSqlParser::Upsert_clauseContext *ctx) = 0;

  virtual void enterPragma_stmt(ECSqlParser::Pragma_stmtContext *ctx) = 0;
  virtual void exitPragma_stmt(ECSqlParser::Pragma_stmtContext *ctx) = 0;

  virtual void enterPragma_value(ECSqlParser::Pragma_valueContext *ctx) = 0;
  virtual void exitPragma_value(ECSqlParser::Pragma_valueContext *ctx) = 0;

  virtual void enterSelect_stmt(ECSqlParser::Select_stmtContext *ctx) = 0;
  virtual void exitSelect_stmt(ECSqlParser::Select_stmtContext *ctx) = 0;

  virtual void enterJoin_clause(ECSqlParser::Join_clauseContext *ctx) = 0;
  virtual void exitJoin_clause(ECSqlParser::Join_clauseContext *ctx) = 0;

  virtual void enterSelect_core(ECSqlParser::Select_coreContext *ctx) = 0;
  virtual void exitSelect_core(ECSqlParser::Select_coreContext *ctx) = 0;

  virtual void enterFactored_select_stmt(ECSqlParser::Factored_select_stmtContext *ctx) = 0;
  virtual void exitFactored_select_stmt(ECSqlParser::Factored_select_stmtContext *ctx) = 0;

  virtual void enterSimple_select_stmt(ECSqlParser::Simple_select_stmtContext *ctx) = 0;
  virtual void exitSimple_select_stmt(ECSqlParser::Simple_select_stmtContext *ctx) = 0;

  virtual void enterCompound_select_stmt(ECSqlParser::Compound_select_stmtContext *ctx) = 0;
  virtual void exitCompound_select_stmt(ECSqlParser::Compound_select_stmtContext *ctx) = 0;

  virtual void enterClass_or_subquery(ECSqlParser::Class_or_subqueryContext *ctx) = 0;
  virtual void exitClass_or_subquery(ECSqlParser::Class_or_subqueryContext *ctx) = 0;

  virtual void enterResult_property(ECSqlParser::Result_propertyContext *ctx) = 0;
  virtual void exitResult_property(ECSqlParser::Result_propertyContext *ctx) = 0;

  virtual void enterJoin_operator(ECSqlParser::Join_operatorContext *ctx) = 0;
  virtual void exitJoin_operator(ECSqlParser::Join_operatorContext *ctx) = 0;

  virtual void enterJoin_constraint(ECSqlParser::Join_constraintContext *ctx) = 0;
  virtual void exitJoin_constraint(ECSqlParser::Join_constraintContext *ctx) = 0;

  virtual void enterCompound_operator(ECSqlParser::Compound_operatorContext *ctx) = 0;
  virtual void exitCompound_operator(ECSqlParser::Compound_operatorContext *ctx) = 0;

  virtual void enterUpdate_stmt(ECSqlParser::Update_stmtContext *ctx) = 0;
  virtual void exitUpdate_stmt(ECSqlParser::Update_stmtContext *ctx) = 0;

  virtual void enterQualified_class_name(ECSqlParser::Qualified_class_nameContext *ctx) = 0;
  virtual void exitQualified_class_name(ECSqlParser::Qualified_class_nameContext *ctx) = 0;

  virtual void enterVacuum_stmt(ECSqlParser::Vacuum_stmtContext *ctx) = 0;
  virtual void exitVacuum_stmt(ECSqlParser::Vacuum_stmtContext *ctx) = 0;

  virtual void enterFilter_clause(ECSqlParser::Filter_clauseContext *ctx) = 0;
  virtual void exitFilter_clause(ECSqlParser::Filter_clauseContext *ctx) = 0;

  virtual void enterWindow_defn(ECSqlParser::Window_defnContext *ctx) = 0;
  virtual void exitWindow_defn(ECSqlParser::Window_defnContext *ctx) = 0;

  virtual void enterOver_clause(ECSqlParser::Over_clauseContext *ctx) = 0;
  virtual void exitOver_clause(ECSqlParser::Over_clauseContext *ctx) = 0;

  virtual void enterFrame_spec(ECSqlParser::Frame_specContext *ctx) = 0;
  virtual void exitFrame_spec(ECSqlParser::Frame_specContext *ctx) = 0;

  virtual void enterFrame_clause(ECSqlParser::Frame_clauseContext *ctx) = 0;
  virtual void exitFrame_clause(ECSqlParser::Frame_clauseContext *ctx) = 0;

  virtual void enterSimple_function_invocation(ECSqlParser::Simple_function_invocationContext *ctx) = 0;
  virtual void exitSimple_function_invocation(ECSqlParser::Simple_function_invocationContext *ctx) = 0;

  virtual void enterAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext *ctx) = 0;
  virtual void exitAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext *ctx) = 0;

  virtual void enterWindow_function_invocation(ECSqlParser::Window_function_invocationContext *ctx) = 0;
  virtual void exitWindow_function_invocation(ECSqlParser::Window_function_invocationContext *ctx) = 0;

  virtual void enterCommon_class_stmt(ECSqlParser::Common_class_stmtContext *ctx) = 0;
  virtual void exitCommon_class_stmt(ECSqlParser::Common_class_stmtContext *ctx) = 0;

  virtual void enterOrder_by_stmt(ECSqlParser::Order_by_stmtContext *ctx) = 0;
  virtual void exitOrder_by_stmt(ECSqlParser::Order_by_stmtContext *ctx) = 0;

  virtual void enterLimit_stmt(ECSqlParser::Limit_stmtContext *ctx) = 0;
  virtual void exitLimit_stmt(ECSqlParser::Limit_stmtContext *ctx) = 0;

  virtual void enterOrdering_term(ECSqlParser::Ordering_termContext *ctx) = 0;
  virtual void exitOrdering_term(ECSqlParser::Ordering_termContext *ctx) = 0;

  virtual void enterAsc_desc(ECSqlParser::Asc_descContext *ctx) = 0;
  virtual void exitAsc_desc(ECSqlParser::Asc_descContext *ctx) = 0;

  virtual void enterFrame_left(ECSqlParser::Frame_leftContext *ctx) = 0;
  virtual void exitFrame_left(ECSqlParser::Frame_leftContext *ctx) = 0;

  virtual void enterFrame_right(ECSqlParser::Frame_rightContext *ctx) = 0;
  virtual void exitFrame_right(ECSqlParser::Frame_rightContext *ctx) = 0;

  virtual void enterFrame_single(ECSqlParser::Frame_singleContext *ctx) = 0;
  virtual void exitFrame_single(ECSqlParser::Frame_singleContext *ctx) = 0;

  virtual void enterWindow_function(ECSqlParser::Window_functionContext *ctx) = 0;
  virtual void exitWindow_function(ECSqlParser::Window_functionContext *ctx) = 0;

  virtual void enterOffset(ECSqlParser::OffsetContext *ctx) = 0;
  virtual void exitOffset(ECSqlParser::OffsetContext *ctx) = 0;

  virtual void enterDefault_value(ECSqlParser::Default_valueContext *ctx) = 0;
  virtual void exitDefault_value(ECSqlParser::Default_valueContext *ctx) = 0;

  virtual void enterPartition_by(ECSqlParser::Partition_byContext *ctx) = 0;
  virtual void exitPartition_by(ECSqlParser::Partition_byContext *ctx) = 0;

  virtual void enterOrder_by_expr(ECSqlParser::Order_by_exprContext *ctx) = 0;
  virtual void exitOrder_by_expr(ECSqlParser::Order_by_exprContext *ctx) = 0;

  virtual void enterOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext *ctx) = 0;
  virtual void exitOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext *ctx) = 0;

  virtual void enterExpr_asc_desc(ECSqlParser::Expr_asc_descContext *ctx) = 0;
  virtual void exitExpr_asc_desc(ECSqlParser::Expr_asc_descContext *ctx) = 0;

  virtual void enterInitial_select(ECSqlParser::Initial_selectContext *ctx) = 0;
  virtual void exitInitial_select(ECSqlParser::Initial_selectContext *ctx) = 0;

  virtual void enterRecursive_select(ECSqlParser::Recursive_selectContext *ctx) = 0;
  virtual void exitRecursive_select(ECSqlParser::Recursive_selectContext *ctx) = 0;

  virtual void enterUnary_operator(ECSqlParser::Unary_operatorContext *ctx) = 0;
  virtual void exitUnary_operator(ECSqlParser::Unary_operatorContext *ctx) = 0;

  virtual void enterError_message(ECSqlParser::Error_messageContext *ctx) = 0;
  virtual void exitError_message(ECSqlParser::Error_messageContext *ctx) = 0;

  virtual void enterProperty_alias(ECSqlParser::Property_aliasContext *ctx) = 0;
  virtual void exitProperty_alias(ECSqlParser::Property_aliasContext *ctx) = 0;

  virtual void enterKeyword(ECSqlParser::KeywordContext *ctx) = 0;
  virtual void exitKeyword(ECSqlParser::KeywordContext *ctx) = 0;

  virtual void enterName(ECSqlParser::NameContext *ctx) = 0;
  virtual void exitName(ECSqlParser::NameContext *ctx) = 0;

  virtual void enterFunction_name(ECSqlParser::Function_nameContext *ctx) = 0;
  virtual void exitFunction_name(ECSqlParser::Function_nameContext *ctx) = 0;

  virtual void enterSchema_name(ECSqlParser::Schema_nameContext *ctx) = 0;
  virtual void exitSchema_name(ECSqlParser::Schema_nameContext *ctx) = 0;

  virtual void enterClass_name(ECSqlParser::Class_nameContext *ctx) = 0;
  virtual void exitClass_name(ECSqlParser::Class_nameContext *ctx) = 0;

  virtual void enterClass_or_index_name(ECSqlParser::Class_or_index_nameContext *ctx) = 0;
  virtual void exitClass_or_index_name(ECSqlParser::Class_or_index_nameContext *ctx) = 0;

  virtual void enterProperty_name(ECSqlParser::Property_nameContext *ctx) = 0;
  virtual void exitProperty_name(ECSqlParser::Property_nameContext *ctx) = 0;

  virtual void enterCollation_name(ECSqlParser::Collation_nameContext *ctx) = 0;
  virtual void exitCollation_name(ECSqlParser::Collation_nameContext *ctx) = 0;

  virtual void enterIndex_name(ECSqlParser::Index_nameContext *ctx) = 0;
  virtual void exitIndex_name(ECSqlParser::Index_nameContext *ctx) = 0;

  virtual void enterModule_name(ECSqlParser::Module_nameContext *ctx) = 0;
  virtual void exitModule_name(ECSqlParser::Module_nameContext *ctx) = 0;

  virtual void enterPragma_name(ECSqlParser::Pragma_nameContext *ctx) = 0;
  virtual void exitPragma_name(ECSqlParser::Pragma_nameContext *ctx) = 0;

  virtual void enterClass_alias(ECSqlParser::Class_aliasContext *ctx) = 0;
  virtual void exitClass_alias(ECSqlParser::Class_aliasContext *ctx) = 0;

  virtual void enterWindow_name(ECSqlParser::Window_nameContext *ctx) = 0;
  virtual void exitWindow_name(ECSqlParser::Window_nameContext *ctx) = 0;

  virtual void enterAlias(ECSqlParser::AliasContext *ctx) = 0;
  virtual void exitAlias(ECSqlParser::AliasContext *ctx) = 0;

  virtual void enterFilename(ECSqlParser::FilenameContext *ctx) = 0;
  virtual void exitFilename(ECSqlParser::FilenameContext *ctx) = 0;

  virtual void enterBase_window_name(ECSqlParser::Base_window_nameContext *ctx) = 0;
  virtual void exitBase_window_name(ECSqlParser::Base_window_nameContext *ctx) = 0;

  virtual void enterSimple_func(ECSqlParser::Simple_funcContext *ctx) = 0;
  virtual void exitSimple_func(ECSqlParser::Simple_funcContext *ctx) = 0;

  virtual void enterAggregate_func(ECSqlParser::Aggregate_funcContext *ctx) = 0;
  virtual void exitAggregate_func(ECSqlParser::Aggregate_funcContext *ctx) = 0;

  virtual void enterClass_function_name(ECSqlParser::Class_function_nameContext *ctx) = 0;
  virtual void exitClass_function_name(ECSqlParser::Class_function_nameContext *ctx) = 0;

  virtual void enterAny_name(ECSqlParser::Any_nameContext *ctx) = 0;
  virtual void exitAny_name(ECSqlParser::Any_nameContext *ctx) = 0;


};

