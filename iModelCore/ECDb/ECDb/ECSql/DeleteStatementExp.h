/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DeleteStatementExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct DeleteStatementExp : Exp
    {
DEFINE_EXPR_TYPE (Delete)
private:
    size_t m_classNameExpIndex;
    int m_whereClauseIndex;

    std::unique_ptr<RangeClassRefList> m_finalizeParsingArgCache;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString () const override { return "Delete"; }

public:
    DeleteStatementExp (std::unique_ptr<ClassRefExp> classNameExp, std::unique_ptr<WhereExp> whereClauseExp);

    ClassNameExp const* GetClassNameExp () const;
    WhereExp const* GetOptWhereClauseExp () const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE

