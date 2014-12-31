/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/InsertStatementExp.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "ListExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2013
//+===============+===============+===============+===============+===============+======
struct InsertStatementExp : Exp
    {
DEFINE_EXPR_TYPE(Insert) 
private:
    size_t m_classNameExpIndex;
    size_t m_propertyNameListExpIndex;
    size_t m_valuesExpIndex;
    bool m_isOriginalPropertyNameListUnset;

    std::unique_ptr<RangeClassRefList> m_finalizeParsingArgCache;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    FinalizeParseStatus Validate (ECSqlParseContext& ctx) const;
    void ResolveParameters () const;

    virtual Utf8String _ToString () const override
        {
        return "Insert";
        }

    PropertyNameListExp* GetPropertyNameListExpP () const;

    bool IsOriginalPropertyNameListUnset () const {return m_isOriginalPropertyNameListUnset;}

public :
    InsertStatementExp (std::unique_ptr<ClassNameExp> classNameExp, std::unique_ptr<PropertyNameListExp> propertyNameListExp,
                         std::unique_ptr<RowValueConstructorListExp> valuesExp);

    ClassNameExp const* GetClassNameExp () const;
    PropertyNameListExp const* GetPropertyNameListExp () const;
    RowValueConstructorListExp const* GetValuesExp () const;

    virtual Utf8String ToECSql() const override;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE

