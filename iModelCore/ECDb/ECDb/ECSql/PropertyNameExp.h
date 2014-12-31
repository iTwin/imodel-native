/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ValueExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct DerivedPropertyExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct PropertyNameExp : ValueExp
    {
public:
    DEFINE_EXPR_TYPE(PropertyName) 
private:
    PropertyPath m_propertyPath;
    bool m_isSystemProperty;
    ECSqlSystemProperty m_systemProperty;

    Utf8String m_classAlias;
    RangeClassRefExp const* m_classRefExp;
    DerivedPropertyExp const* m_derivedPropertyExpInSubqueryRefExp;

    ECSqlStatus ResolveColumnRef (ECSqlParseContext& ctx);
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    void SetClassRefExp (RangeClassRefExp const& classRefExp);
    bool HasDerivedPropExpInSubqueryRefExp() const;
    void SetDerivedPropertyExpInSubqueryRefExp (DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp);
    virtual Utf8String _ToString () const override;

public:
    explicit PropertyNameExp (PropertyPath&& propPath);
    explicit PropertyNameExp (Utf8CP propertyName);
    PropertyNameExp (Utf8CP propertyName, RangeClassRefExp const& classRefExp, IClassMap const& classMap);
    PropertyNameExp (RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp);

    Utf8StringCR GetPropertyName() const;

    PropertyPath const& GetPropertyPath () const;
    PropertyMapCR GetPropertyMap () const;

    bool IsSystemProperty () const { return m_isSystemProperty; }
    bool TryGetSystemProperty (ECSqlSystemProperty& systemProperty) const;

    RangeClassRefExp const* GetClassRefExp() const;
    DerivedPropertyExp const* GetDerivedPropertyExpInSubqueryRefExp() const;
    Utf8CP GetClassAlias () const;

    virtual Utf8String ToECSql() const override;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE