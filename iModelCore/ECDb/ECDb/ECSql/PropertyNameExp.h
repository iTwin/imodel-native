/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ValueExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct DerivedPropertyExp;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyNameExp final : ValueExp
    {
        enum class SourceType
            {
            ECSql,
            ClassRef,
            SubQuery,
            };
        struct PropertyRef
        {
        private:
            PropertyNameExp const *m_owner;
            DerivedPropertyExp const& m_linkedTo;
            mutable NativeSqlBuilder::List m_nativeSqlSnippets;
            mutable bool m_wasToNativeSqlCalled = false;
            mutable PropertyMap const *m_cachedPropertyMap;
        public:
            explicit PropertyRef(DerivedPropertyExp const& endPoint, PropertyNameExp const* owner) :m_linkedTo(endPoint), m_owner(owner), m_cachedPropertyMap(nullptr) {}

            DerivedPropertyExp const& LinkedTo() const { return m_linkedTo; }
            DerivedPropertyExp const& GetEndPointDerivedProperty() const;
            bool IsPure() const;
            
            bool WasToNativeSqlCalled() const { return m_wasToNativeSqlCalled; }
            NativeSqlBuilder::List const& GetNativeSql() const { return m_nativeSqlSnippets; }
            BentleyStatus ToNativeSql(NativeSqlBuilder::List const&) const;
            BentleyStatus ToNativeSql(NativeSqlBuilder::List const&, std::vector<bool>const&) const;
            ECN::ECPropertyCP TryGetVirtualProperty() const;
            PropertyMap const *TryGetPropertyMap() const;
            PropertyMap const *TryGetPropertyMap(PropertyPath const& testPath) const;
            bool IsComputedExp() const;
            bool ReferToAlias() const;
            bool TryResolvePath(PropertyPath &path) const;
        };
    private:
        ECN::ECPropertyCP m_property;
        PropertyPath m_propertyPath;
        std::unique_ptr<PropertyRef> m_propertyRef;
        SourceType m_sourceType;
        Utf8String m_className;
        RangeClassRefExp const* m_classRefExp = nullptr;
        ECSqlSystemPropertyInfo const* m_sysPropInfo = nullptr; //will never be null, but cannot declare as ref as it is set after construction
        BentleyStatus ResolveUnionOrderByArg(ECSqlParseContext&);
        BentleyStatus ResolveColumnRef(ECSqlParseContext&);
        BentleyStatus ResolveLocalRef(ECSqlParseContext&);

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void SetClassRefExp(RangeClassRefExp const& classRefExp);
        void SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp);
        void SetVirtualProperty(ECN::ECPropertyCR property) { m_property = &property; }
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        explicit PropertyNameExp(PropertyPath const& propPath);
        PropertyNameExp(PropertyPath const& propPath, RangeClassRefExp const& classRefExp, ECN::ECPropertyCR property);
        PropertyNameExp(ECSqlParseContext const&, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap);
        PropertyNameExp(ECSqlParseContext const&, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp);
        Utf8StringCR GetPropertyName() const { return m_propertyPath[0].GetName(); }
        bool HasUserDefinedAlias() const;
        ECN::ECPropertyCP GetVirtualProperty() const;
        bool IsVirtualProperty(bool recursively = true) const { return recursively ? GetVirtualProperty() != nullptr: m_property != nullptr ; }
        PropertyPath const& GetPropertyPath() const { return m_propertyPath; }
        PropertyMap const& GetPropertyMap() const;
        SourceType const GetSourceType() const { return m_sourceType; }
        Utf8CP GetClassName() const { return m_className.c_str(); }
        RangeClassRefExp const* GetClassRefExp() const { return m_classRefExp; }
        PropertyRef const* GetPropertyRef() const { return m_propertyRef.get(); }
        PropertyRef* GetPropertyRefP() { return m_propertyRef.get(); }
        bool IsPropertyRef() const { return m_propertyRef != nullptr; }
        ECSqlSystemPropertyInfo const& GetSystemPropertyInfo() const { BeAssert(m_sysPropInfo != nullptr); return *m_sysPropInfo; }
        bool IsLhsAssignmentOperandExpression() const;
        bool OriginateInASubQuery() const { return nullptr != this->FindParent(Exp::Type::Subquery); }
        bool IsWildCard() const;
 
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct InstanceValueExp : ValueExp {
private:
    PropertyPath m_instancePath;
    size_t m_classIdExpIdx;
    size_t m_instIdExpIdx;
    
public:
    explicit InstanceValueExp(Type, PropertyPath);
    virtual ~InstanceValueExp(){}
    PropertyNameExp const& GetClassIdPropExp() const { return *GetChild<PropertyNameExp>(m_classIdExpIdx);}
    PropertyNameExp const& GetInstanceIdPropExp() const{ return *GetChild<PropertyNameExp>(m_instIdExpIdx);}
    PropertyPath const& GetInstancePath() const {return m_instancePath; }
    static bool IsInstancePath(PropertyPath const& path) {
        return path.Size() == 0 ?  false: path.Last().GetName().Equals("$");
    }
    static bool IsValidSourcePath(PropertyPath const&);
    static Utf8CP GetInstanceAlias(PropertyPath const&);
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ExtractPropertyValueExp final : InstanceValueExp {
    private:
        PropertyPath m_targetPath;
        void _ToECSql(ECSqlRenderContext& ctx) const override{
            ctx.AppendToECSql(GetInstancePath().ToString().c_str());
            ctx.AppendToECSql(" -> ");
            ctx.AppendToECSql(m_targetPath.ToString().c_str());
        }
        Utf8String _ToString() const override { return "";}
    public:
        ExtractPropertyValueExp(
            PropertyPath instancePath, 
            PropertyPath targetPath): 
                InstanceValueExp(Type::ExtractProperty, instancePath), m_targetPath(targetPath) {
            SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String));
        }
        PropertyPath const& GetTargetPath() const { return m_targetPath; }
        virtual ~ExtractPropertyValueExp(){}
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ExtractInstanceValueExp final : InstanceValueExp {
    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override{
            ctx.AppendToECSql(GetInstancePath().ToString().c_str());
        }
        Utf8String _ToString() const override { return ""; }

    public:
        ExtractInstanceValueExp(
            PropertyPath instancePath): 
                InstanceValueExp(Type::ExtractInstance, instancePath) {
                SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String));
            }
        virtual ~ExtractInstanceValueExp(){}
};

END_BENTLEY_SQLITE_EC_NAMESPACE