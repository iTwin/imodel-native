/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
            mutable NativeSqlBuilder::List m_nativeSqlSnippets;
            mutable bool m_wasToNativeSqlCalled = false;

        public:
            explicit PropertyRef(DerivedPropertyExp const& endPoint) :m_linkedTo(endPoint) {}

            DerivedPropertyExp const& LinkedTo() const { return m_linkedTo; }
            DerivedPropertyExp const& GetEndPointDerivedProperty() const;

            bool WasToNativeSqlCalled() const { return m_wasToNativeSqlCalled; }
            NativeSqlBuilder::List const& GetNativeSql() const { return m_nativeSqlSnippets; }
            BentleyStatus ToNativeSql(NativeSqlBuilder::List const&) const;
        };
    private:
        PropertyPath m_propertyPath;
        std::unique_ptr<PropertyRef> m_propertyRef;

        Utf8String m_className;
        RangeClassRefExp const* m_classRefExp = nullptr;
        ECSqlSystemPropertyInfo const* m_sysPropInfo = nullptr; //will never be null, but cannot declare as ref as it is set after construction
        BentleyStatus ResolveUnionOrderByArg(ECSqlParseContext&);
        BentleyStatus ResolveColumnRef(ECSqlParseContext&);
        BentleyStatus ResolveColumnRef(Utf8StringR error, RangeClassRefExp const&, PropertyPath&);

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void SetClassRefExp(RangeClassRefExp const& classRefExp);
        void SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp);
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        explicit PropertyNameExp(PropertyPath const& propPath);
        PropertyNameExp(ECSqlParseContext const&, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap);
        PropertyNameExp(ECSqlParseContext const&, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp);

        Utf8StringCR GetPropertyName() const { return m_propertyPath[0].GetName(); }

        PropertyPath const& GetPropertyPath() const { return m_propertyPath; }
        PropertyMap const& GetPropertyMap() const;

        Utf8CP GetClassName() const { return m_className.c_str(); }
        RangeClassRefExp const* GetClassRefExp() const { return m_classRefExp; }
        PropertyRef const* GetPropertyRef() const { return m_propertyRef.get(); }
        PropertyRef* GetPropertyRefP() { return m_propertyRef.get(); }
        bool IsPropertyRef() const { return m_propertyRef != nullptr; }
        ECSqlSystemPropertyInfo const& GetSystemPropertyInfo() const { BeAssert(m_sysPropInfo != nullptr); return *m_sysPropInfo; }
        bool IsLhsAssignmentOperandExpression() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE