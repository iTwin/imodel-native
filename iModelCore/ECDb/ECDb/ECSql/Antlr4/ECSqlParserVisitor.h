
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"
#include "ECSqlParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ECSqlParser.
 */
class  ECSqlParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ECSqlParser.
   */
    virtual std::any visitParse(ECSqlParser::ParseContext *context) = 0;

    virtual std::any visitSql_stmt_list(ECSqlParser::Sql_stmt_listContext *context) = 0;

    virtual std::any visitSql_stmt(ECSqlParser::Sql_stmtContext *context) = 0;

    virtual std::any visitAnalyze_stmt(ECSqlParser::Analyze_stmtContext *context) = 0;

    virtual std::any visitAttach_stmt(ECSqlParser::Attach_stmtContext *context) = 0;

    virtual std::any visitType_name(ECSqlParser::Type_nameContext *context) = 0;

    virtual std::any visitSigned_number(ECSqlParser::Signed_numberContext *context) = 0;

    virtual std::any visitWith_clause(ECSqlParser::With_clauseContext *context) = 0;

    virtual std::any visitCte_class_name(ECSqlParser::Cte_class_nameContext *context) = 0;

    virtual std::any visitRecursive_cte(ECSqlParser::Recursive_cteContext *context) = 0;

    virtual std::any visitCommon_class_expression(ECSqlParser::Common_class_expressionContext *context) = 0;

    virtual std::any visitDelete_stmt(ECSqlParser::Delete_stmtContext *context) = 0;

    virtual std::any visitDetach_stmt(ECSqlParser::Detach_stmtContext *context) = 0;

    virtual std::any visitAccess_string(ECSqlParser::Access_stringContext *context) = 0;

    virtual std::any visitExpr(ECSqlParser::ExprContext *context) = 0;

    virtual std::any visitLiteral_value(ECSqlParser::Literal_valueContext *context) = 0;

    virtual std::any visitValue_row(ECSqlParser::Value_rowContext *context) = 0;

    virtual std::any visitValues_clause(ECSqlParser::Values_clauseContext *context) = 0;

    virtual std::any visitInsert_stmt(ECSqlParser::Insert_stmtContext *context) = 0;

    virtual std::any visitReturning_clause(ECSqlParser::Returning_clauseContext *context) = 0;

    virtual std::any visitIndexed_property(ECSqlParser::Indexed_propertyContext *context) = 0;

    virtual std::any visitUpsert_clause(ECSqlParser::Upsert_clauseContext *context) = 0;

    virtual std::any visitPragma_stmt(ECSqlParser::Pragma_stmtContext *context) = 0;

    virtual std::any visitPragma_value(ECSqlParser::Pragma_valueContext *context) = 0;

    virtual std::any visitSelect_stmt(ECSqlParser::Select_stmtContext *context) = 0;

    virtual std::any visitJoin_clause(ECSqlParser::Join_clauseContext *context) = 0;

    virtual std::any visitSelect_core(ECSqlParser::Select_coreContext *context) = 0;

    virtual std::any visitFactored_select_stmt(ECSqlParser::Factored_select_stmtContext *context) = 0;

    virtual std::any visitSimple_select_stmt(ECSqlParser::Simple_select_stmtContext *context) = 0;

    virtual std::any visitCompound_select_stmt(ECSqlParser::Compound_select_stmtContext *context) = 0;

    virtual std::any visitClass_or_subquery(ECSqlParser::Class_or_subqueryContext *context) = 0;

    virtual std::any visitResult_property(ECSqlParser::Result_propertyContext *context) = 0;

    virtual std::any visitJoin_operator(ECSqlParser::Join_operatorContext *context) = 0;

    virtual std::any visitJoin_constraint(ECSqlParser::Join_constraintContext *context) = 0;

    virtual std::any visitCompound_operator(ECSqlParser::Compound_operatorContext *context) = 0;

    virtual std::any visitUpdate_stmt(ECSqlParser::Update_stmtContext *context) = 0;

    virtual std::any visitQualified_class_name(ECSqlParser::Qualified_class_nameContext *context) = 0;

    virtual std::any visitVacuum_stmt(ECSqlParser::Vacuum_stmtContext *context) = 0;

    virtual std::any visitFilter_clause(ECSqlParser::Filter_clauseContext *context) = 0;

    virtual std::any visitWindow_defn(ECSqlParser::Window_defnContext *context) = 0;

    virtual std::any visitOver_clause(ECSqlParser::Over_clauseContext *context) = 0;

    virtual std::any visitFrame_spec(ECSqlParser::Frame_specContext *context) = 0;

    virtual std::any visitFrame_clause(ECSqlParser::Frame_clauseContext *context) = 0;

    virtual std::any visitSimple_function_invocation(ECSqlParser::Simple_function_invocationContext *context) = 0;

    virtual std::any visitAggregate_function_invocation(ECSqlParser::Aggregate_function_invocationContext *context) = 0;

    virtual std::any visitWindow_function_invocation(ECSqlParser::Window_function_invocationContext *context) = 0;

    virtual std::any visitCommon_class_stmt(ECSqlParser::Common_class_stmtContext *context) = 0;

    virtual std::any visitOrder_by_stmt(ECSqlParser::Order_by_stmtContext *context) = 0;

    virtual std::any visitLimit_stmt(ECSqlParser::Limit_stmtContext *context) = 0;

    virtual std::any visitOrdering_term(ECSqlParser::Ordering_termContext *context) = 0;

    virtual std::any visitAsc_desc(ECSqlParser::Asc_descContext *context) = 0;

    virtual std::any visitFrame_left(ECSqlParser::Frame_leftContext *context) = 0;

    virtual std::any visitFrame_right(ECSqlParser::Frame_rightContext *context) = 0;

    virtual std::any visitFrame_single(ECSqlParser::Frame_singleContext *context) = 0;

    virtual std::any visitWindow_function(ECSqlParser::Window_functionContext *context) = 0;

    virtual std::any visitOffset(ECSqlParser::OffsetContext *context) = 0;

    virtual std::any visitDefault_value(ECSqlParser::Default_valueContext *context) = 0;

    virtual std::any visitPartition_by(ECSqlParser::Partition_byContext *context) = 0;

    virtual std::any visitOrder_by_expr(ECSqlParser::Order_by_exprContext *context) = 0;

    virtual std::any visitOrder_by_expr_asc_desc(ECSqlParser::Order_by_expr_asc_descContext *context) = 0;

    virtual std::any visitExpr_asc_desc(ECSqlParser::Expr_asc_descContext *context) = 0;

    virtual std::any visitInitial_select(ECSqlParser::Initial_selectContext *context) = 0;

    virtual std::any visitRecursive_select(ECSqlParser::Recursive_selectContext *context) = 0;

    virtual std::any visitUnary_operator(ECSqlParser::Unary_operatorContext *context) = 0;

    virtual std::any visitError_message(ECSqlParser::Error_messageContext *context) = 0;

    virtual std::any visitProperty_alias(ECSqlParser::Property_aliasContext *context) = 0;

    virtual std::any visitKeyword(ECSqlParser::KeywordContext *context) = 0;

    virtual std::any visitName(ECSqlParser::NameContext *context) = 0;

    virtual std::any visitFunction_name(ECSqlParser::Function_nameContext *context) = 0;

    virtual std::any visitSchema_name(ECSqlParser::Schema_nameContext *context) = 0;

    virtual std::any visitClass_name(ECSqlParser::Class_nameContext *context) = 0;

    virtual std::any visitClass_or_index_name(ECSqlParser::Class_or_index_nameContext *context) = 0;

    virtual std::any visitProperty_name(ECSqlParser::Property_nameContext *context) = 0;

    virtual std::any visitCollation_name(ECSqlParser::Collation_nameContext *context) = 0;

    virtual std::any visitIndex_name(ECSqlParser::Index_nameContext *context) = 0;

    virtual std::any visitModule_name(ECSqlParser::Module_nameContext *context) = 0;

    virtual std::any visitPragma_name(ECSqlParser::Pragma_nameContext *context) = 0;

    virtual std::any visitClass_alias(ECSqlParser::Class_aliasContext *context) = 0;

    virtual std::any visitWindow_name(ECSqlParser::Window_nameContext *context) = 0;

    virtual std::any visitAlias(ECSqlParser::AliasContext *context) = 0;

    virtual std::any visitFilename(ECSqlParser::FilenameContext *context) = 0;

    virtual std::any visitBase_window_name(ECSqlParser::Base_window_nameContext *context) = 0;

    virtual std::any visitSimple_func(ECSqlParser::Simple_funcContext *context) = 0;

    virtual std::any visitAggregate_func(ECSqlParser::Aggregate_funcContext *context) = 0;

    virtual std::any visitClass_function_name(ECSqlParser::Class_function_nameContext *context) = 0;

    virtual std::any visitAny_name(ECSqlParser::Any_nameContext *context) = 0;


};

