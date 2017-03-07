/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct PropertyNameExp final : ValueExp
    {
    struct PropertyRef
        {
        private:
            DerivedPropertyExp const& m_linkedTo;
            NativeSqlBuilder::List m_nativeSqlSnippets;
            bool m_isConverted;
        public:
            explicit PropertyRef(DerivedPropertyExp const& endPoint) :m_linkedTo(endPoint), m_isConverted(false) {}

            DerivedPropertyExp const& LinkedTo() const { return m_linkedTo; }

            NativeSqlBuilder::List const& GetOutSnippets() const { return m_nativeSqlSnippets; }

            DerivedPropertyExp const& GetEndPointDerivedProperty() const;
            PropertyNameExp const* GetEndPointPropertyNameIfAny() const;
            bool IsConverted() const { return m_isConverted; }
            BentleyStatus ToNativeSql(NativeSqlBuilder::List const& snippets);
        };
    private:
        PropertyPath m_propertyPath;
        std::unique_ptr<PropertyRef> m_propertyRef;

        Utf8String m_classAlias;
        RangeClassRefExp const* m_classRefExp;
        ECSqlSystemPropertyInfo const* m_sysPropInfo; //will never be null, but cannot declare as ref as it is set after construction
        BentleyStatus ResolveColumnRef(ECSqlParseContext&);
        BentleyStatus ResolveColumnRef(Utf8StringR error, RangeClassRefExp const&, PropertyPath& propPath);

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void SetClassRefExp(RangeClassRefExp const& classRefExp);
        void SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp);
        void _DoToECSql(Utf8StringR ecsql) const override;
        Utf8String _ToString() const override;

    public:
        explicit PropertyNameExp(PropertyPath&& propPath);
        PropertyNameExp(ECSqlParseContext const&, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap);
        PropertyNameExp(ECSqlParseContext const&, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp);

        Utf8CP GetPropertyName() const { return m_propertyPath[0].GetPropertyName(); }

        PropertyPath const& GetPropertyPath() const { return m_propertyPath; }
        PropertyMap const& GetPropertyMap() const;

        Utf8CP GetClassAlias() const { return m_classAlias.c_str(); }
        RangeClassRefExp const* GetClassRefExp() const { return m_classRefExp; }
        PropertyRef const* GetPropertyRef() const { return m_propertyRef.get(); }
        PropertyRef* GetPropertyRefP() { return m_propertyRef.get(); }
        bool IsPropertyRef() const { return m_propertyRef != nullptr; }
        ECSqlSystemPropertyInfo const& GetSystemPropertyInfo() const { BeAssert(m_sysPropInfo != nullptr); return *m_sysPropInfo; }
        bool IsLhsAssignmentOperandExpression() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE