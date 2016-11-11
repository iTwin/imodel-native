/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/UpdateStatementExp.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "ListExp.h"
#include "OptionsExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct UpdateStatementExp : Exp
    {
DEFINE_EXPR_TYPE (Update)
private:
    size_t m_classNameExpIndex;
    size_t m_assignmentListExpIndex;
    int m_whereClauseIndex;
    int m_optionsClauseIndex;

    RangeClasssInfo::List m_finalizeParsingArgCache;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString () const override { return "Update"; }

public:
    UpdateStatementExp(std::unique_ptr<ClassRefExp>, std::unique_ptr<AssignmentListExp>, std::unique_ptr<WhereExp>, std::unique_ptr<OptionsExp>);

    ClassNameExp const* GetClassNameExp () const;
    AssignmentListExp const* GetAssignmentListExp () const;
    WhereExp const* GetWhereClauseExp () const;
    OptionsExp const* GetOptionsClauseExp() const;
    };


//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct AssignmentExp : Exp
    {
DEFINE_EXPR_TYPE (Assignment)
private:
    size_t m_propNameExpIndex;
    size_t m_valueExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "Assignment"; }

public:
    AssignmentExp (std::unique_ptr<PropertyNameExp> propNameExp, std::unique_ptr<ValueExp> valueExp);

    PropertyNameExp const* GetPropertyNameExp () const;
    ValueExp const* GetValueExp () const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

