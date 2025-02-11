/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SystemPropertyExpIndexMap
    {
    private:
        bmap<ECSqlSystemPropertyInfo const*, size_t> m_sysPropIndexMap;

        //not copyable
        SystemPropertyExpIndexMap(SystemPropertyExpIndexMap const&) = delete;
        SystemPropertyExpIndexMap& operator=(SystemPropertyExpIndexMap const&) = delete;

    public:
        SystemPropertyExpIndexMap() {}

        bool Contains(ECSqlSystemPropertyInfo const& info) const { return m_sysPropIndexMap.find(&info) != m_sysPropIndexMap.end(); }

        //!@return non-negative index if found. -1 else.
        int GetIndex(ECSqlSystemPropertyInfo const& info) const { auto it = m_sysPropIndexMap.find(&info); return it == m_sysPropIndexMap.end() ? -1 : (int) it->second; }

        void AddIfSystemProperty(PropertyNameExp const&, size_t index);
        void AddIfSystemProperty(SchemaManager const&, ECN::ECPropertyCR, size_t index);
    };

//************************ AssignmentListExp ******************************
struct AssignmentExp;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct AssignmentListExp final : Exp
    {
    private:
        SystemPropertyExpIndexMap m_specialTokenExpIndexMap;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "AssignmentListExp"; }

    public:
        AssignmentListExp() : Exp(Type::AssignmentList) {}

        void AddAssignmentExp(std::unique_ptr<AssignmentExp> assignmentExp);
        AssignmentExp const* GetAssignmentExp(size_t index) const;
        SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap() const { return m_specialTokenExpIndexMap; }
    };

//************************ PropertyNameListExp ******************************
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyNameListExp final : Exp
    {
    private:
        SystemPropertyExpIndexMap m_specialTokenExpIndexMap;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "PropertyNameList"; }

    public:
        PropertyNameListExp() : Exp(Type::PropertyNameList) {}

        void AddPropertyNameExp(std::unique_ptr<PropertyNameExp>& propertyNameExp) { AddChild(std::move(propertyNameExp)); }
        PropertyNameExp const* GetPropertyNameExp(size_t index) const { return GetChild<PropertyNameExp>(index); }
        SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap() const { return m_specialTokenExpIndexMap; }
    };


//************************ ValueExpListExp ******************************
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ValueExpListExp final : ComputedExp
    {
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "ValueExpList"; }

    public:
        ValueExpListExp() : ComputedExp(Type::ValueExpList) {}
        explicit ValueExpListExp(std::vector<std::unique_ptr<ValueExp>>&);

        void AddValueExp(std::unique_ptr<ValueExp>& valueExp) { AddChild(std::move(valueExp)); }
        ValueExp const* GetValueExp(size_t index) const { return GetChild<ValueExp>(index); }
    };

//************************ RowValuesExpListExp ******************************
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SelectClauseExp;
struct RowValueConstructorListExp final : RangeClassRefExp
    {
    ECSqlTypeInfo m_typeInfo;
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "RowValueConstructorList"; }

        void SetTypeInfo(ECSqlTypeInfo const& typeInfo) { m_typeInfo = typeInfo; }
        Utf8StringCR _GetId() const override { static const Utf8String emptryStr = ""; return emptryStr; }
        void _OnAliasChanged() override {}
        void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;

    public:
        explicit RowValueConstructorListExp(std::vector<std::unique_ptr<ValueExpListExp>>&);
        SelectClauseExp const* GetSelection() const { return GetChild<SelectClauseExp>(0); }
        std::vector<ValueExpListExp const*> GetRowValues() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE