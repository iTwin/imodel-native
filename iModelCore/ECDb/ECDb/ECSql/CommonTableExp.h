/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "SelectStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct CommonTableBlockNameExp;
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct CommonTablePropertyNameExp final : ValueExp
    {
    private:
        Utf8String m_name;
        mutable DerivedPropertyExp const* m_target;
        mutable CommonTableBlockNameExp const* m_blockName;
        std::function<ECSqlTypeInfo(Utf8StringCR)> m_typeInfoCallBack;
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext &, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext &ctx) const override { ctx.AppendToECSql(m_name); }
        Utf8String _ToString() const override { return ""; }

    public:
        explicit CommonTablePropertyNameExp(Utf8CP name, DerivedPropertyExp const &target, std::function<ECSqlTypeInfo(Utf8String)> typeInfoCb, CommonTableBlockNameExp const* blockName = nullptr)
            :ValueExp(Exp::Type::CommonTablePropertyName), m_name(name), m_target(&target), m_typeInfoCallBack(typeInfoCb),m_blockName(blockName) {}
        Utf8StringCR GetName() const { return m_name; }
        DerivedPropertyExp const& GetTarget() const { BeAssert(m_target != nullptr); return *m_target; }
        CommonTableBlockNameExp const* GetBlockNameExp() const { return m_blockName;}
    };
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct CommonTableBlockExp: RangeClassRefExp {
    private:
        Utf8String m_name;
        std::vector<Utf8String> m_columnList;
        mutable bool m_deferredExpand;
        // Exp 
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;
        
        // RangeClass
        Utf8StringCR _GetId() const override;
        void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;

        bool ExpandDerivedProperties() const;

    public:
        CommonTableBlockExp(Utf8CP name, std::vector<Utf8String> colList, std::unique_ptr<SelectStatementExp> stmt);
        SelectStatementExp const *GetQuery() const { return GetChild<SelectStatementExp>(0);}
        Utf8StringCR GetName() const { return m_name; }
        ECSqlTypeInfo FindType(Utf8StringCR cl) const;
        std::vector<Utf8String> const &GetColumns() const { return m_columnList; }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct CommonTableExp: Exp {
    private:
        bool m_recursive;

        // Exp
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        CommonTableExp(std::unique_ptr<SelectStatementExp> stmt, std::vector<std::unique_ptr<CommonTableBlockExp>> cteList, bool recursive);
        std::vector<CommonTableBlockExp const *> GetCteList() const;
        SelectStatementExp const *GetQuery() const { return GetChild<SelectStatementExp>(GetChildrenCount() - 1);}
        bool Recursive() const { return m_recursive; }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct CommonTableBlockNameExp final : RangeClassRefExp {
    private:
        Utf8String m_name;
        mutable CommonTableBlockExp const* m_blockExp;
        
        // Exp
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString () const override;

        // RangeClass
        Utf8StringCR _GetId() const override;
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;
        void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
        CommonTableBlockExp const* ResolveBlock(ECSqlParseContext const&,bool) const;
    public:
        explicit CommonTableBlockNameExp(Utf8CP blockName): RangeClassRefExp(Exp::Type::CommonTableBlockName, PolymorphicInfo::Only()), m_name(blockName), m_blockExp(nullptr){}
        Utf8String GetName() const { return m_name; }
        void SetBlock(CommonTableBlockExp const &block) {
            BeAssert(m_blockExp == nullptr);
            m_blockExp = &block;
        }
};

END_BENTLEY_SQLITE_EC_NAMESPACE



