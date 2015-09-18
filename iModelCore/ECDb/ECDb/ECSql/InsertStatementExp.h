/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/InsertStatementExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

    FinalizeParseStatus Validate (ECSqlParseContext&) const;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "Insert"; }

    PropertyNameListExp* GetPropertyNameListExpP () const;

    bool IsOriginalPropertyNameListUnset () const {return m_isOriginalPropertyNameListUnset;}

public :
    InsertStatementExp (std::unique_ptr<ClassNameExp>& classNameExp, std::unique_ptr<PropertyNameListExp>& propertyNameListExp,
                         std::unique_ptr<ValueExpListExp>& valuesExp);

    ClassNameExp const* GetClassNameExp () const;
    PropertyNameListExp const* GetPropertyNameListExp () const;
    ValueExpListExp const* GetValuesExp() const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE

