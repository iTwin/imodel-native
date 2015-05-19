/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    struct PropertyRef
        {
        private:
            DerivedPropertyExp const& m_linkedTo;
            NativeSqlBuilder::List m_nativeSqlSnippets;
        public:
            PropertyRef (DerivedPropertyExp const& endPoint)
                :m_linkedTo (endPoint)
                {}

            DerivedPropertyExp const& LinkedTo () const { return m_linkedTo; }
            NativeSqlBuilder::List& GetNativeSqlSnippetsR () { return m_nativeSqlSnippets; }
            NativeSqlBuilder::List const& GetNativeSqlSnippets () const { return m_nativeSqlSnippets; }

            DerivedPropertyExp const& GetEndPointDerivedProperty () const;
            PropertyNameExp const* GetEndPointPropertyNameIfAny () const;
                
        };
public:
    DEFINE_EXPR_TYPE(PropertyName) 
private:
    PropertyPath m_propertyPath;
    bool m_isSystemProperty;
    ECSqlSystemProperty m_systemProperty;
    std::unique_ptr<PropertyRef> m_propertyRef;

    Utf8String m_classAlias;
    RangeClassRefExp const* m_classRefExp;

    ECSqlStatus ResolveColumnRef (ECSqlParseContext& ctx);
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    void SetClassRefExp (RangeClassRefExp const& classRefExp);
    void SetPropertyRef (DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp)
        {
        m_propertyRef = std::unique_ptr<PropertyRef> (new PropertyRef (derivedPropertyExpInSubqueryRefExp));
        }
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
    PropertyRef const* GetPropertyRef () const { return m_propertyRef.get (); }
    PropertyRef* GetPropertyRefP () { return m_propertyRef.get (); }
    Utf8CP GetClassAlias () const;
    bool IsPropertyRef () const
        {
        return m_propertyRef != nullptr;
        }
    virtual Utf8String ToECSql() const override;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE