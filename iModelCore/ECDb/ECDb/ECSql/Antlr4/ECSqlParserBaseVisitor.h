
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"
#include "ECSqlParserVisitor.h"


/**
 * This class provides an empty implementation of ECSqlParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ECSqlParserBaseVisitor : public ECSqlParserVisitor {
public:

  virtual std::any visitParse(ECSqlParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSql_stmt_list(ECSqlParser::Sql_stmt_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSql_stmt(ECSqlParser::Sql_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAnalyze_stmt(ECSqlParser::Analyze_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttach_stmt(ECSqlParser::Attach_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_name(ECSqlParser::Type_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSigned_number(ECSqlParser::Signed_numberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWith_clause(ECSqlParser::With_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCte_class_name(ECSqlParser::Cte_class_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRecursive_cte(ECSqlParser::Recursive_cteContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCommon_class_expression(ECSqlParser::Common_class_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDelete_stmt(ECSqlParser::Delete_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDetach_stmt(ECSqlParser::Detach_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAccess_string(ECSqlParser::Access_stringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr(ECSqlParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral_value(ECSqlParser::Literal_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_row(ECSqlParser::Value_rowContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValues_clause(ECSqlParser::Values_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsert_stmt(ECSqlParser::Insert_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturning_clause(ECSqlParser::Returning_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexed_property(ECSqlParser::Indexed_propertyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpsert_clause(ECSqlParser::Upsert_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_stmt(ECSqlParser::Pragma_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_value(ECSqlParser::Pragma_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_stmt(ECSqlParser::Select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_clause(ECSqlParser::Join_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_core(ECSqlParser::Select_coreContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFactored_select_stmt(ECSqlParser::Factored_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_select_stmt(ECSqlParser::Simple_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompound_select_stmt(ECSqlParser::Compound_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_or_subquery(ECSqlParser::Class_or_subqueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitResult_property(ECSqlParser::Result_propertyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_operator(ECSqlParser::Join_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_constraint(ECSqlParser::Join_constraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompound_operator(ECSqlParser::Compound_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdate_stmt(ECSqlParser::Update_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualified_class_name(ECSqlParser::Qualified_class_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVacuum_stmt(ECSqlParser::Vacuum_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFilter_clause(ECSqlParser::Filter_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_defn(ECSqlParser::Window_defnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOver_clause(ECSqlParser::Over_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_spec(ECSqlParser::Frame_specContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_clause(ECSqlParser::Frame_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_function_invocation(ECSqlParser::Simple_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_function_invocation(ECSqlParser::Window_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCommon_class_stmt(ECSqlParser::Common_class_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_stmt(ECSqlParser::Order_by_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLimit_stmt(ECSqlParser::Limit_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrdering_term(ECSqlParser::Ordering_termContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsc_desc(ECSqlParser::Asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_left(ECSqlParser::Frame_leftContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_right(ECSqlParser::Frame_rightContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_single(ECSqlParser::Frame_singleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_function(ECSqlParser::Window_functionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOffset(ECSqlParser::OffsetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefault_value(ECSqlParser::Default_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPartition_by(ECSqlParser::Partition_byContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_expr(ECSqlParser::Order_by_exprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr_asc_desc(ECSqlParser::Expr_asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitial_select(ECSqlParser::Initial_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRecursive_select(ECSqlParser::Recursive_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnary_operator(ECSqlParser::Unary_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitError_message(ECSqlParser::Error_messageContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProperty_alias(ECSqlParser::Property_aliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitKeyword(ECSqlParser::KeywordContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName(ECSqlParser::NameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_name(ECSqlParser::Function_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSchema_name(ECSqlParser::Schema_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_name(ECSqlParser::Class_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_or_index_name(ECSqlParser::Class_or_index_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProperty_name(ECSqlParser::Property_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCollation_name(ECSqlParser::Collation_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndex_name(ECSqlParser::Index_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModule_name(ECSqlParser::Module_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_name(ECSqlParser::Pragma_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_alias(ECSqlParser::Class_aliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_name(ECSqlParser::Window_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlias(ECSqlParser::AliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFilename(ECSqlParser::FilenameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBase_window_name(ECSqlParser::Base_window_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_func(ECSqlParser::Simple_funcContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregate_func(ECSqlParser::Aggregate_funcContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_function_name(ECSqlParser::Class_function_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAny_name(ECSqlParser::Any_nameContext *ctx) override {
    return visitChildren(ctx);
  }


};

