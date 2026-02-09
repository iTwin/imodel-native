/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#pragma once

#include "SelectStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct FilterClauseExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext& ctx) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "FilterClauseExp"; }

   public:
    FilterClauseExp(std::unique_ptr<WhereExp> whereExp) : Exp(Type::FIlterClause) { AddChild(std::move(whereExp)); }

    WhereExp const* GetWhereExp() const { return GetChild<WhereExp>(0); }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowPartitionColumnReferenceExp final : Exp {
   public:
    enum class CollateClauseFunction {
        NotSpecified,
        Binary,
        NoCase,
        Rtrim,
    };

   private:
    void _ToECSql(ECSqlRenderContext& ctx) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowPartitionColumnReferenceExp"; }
    CollateClauseFunction m_collateClauseFunction;

   public:
    WindowPartitionColumnReferenceExp(std::unique_ptr<ValueExp> valueExp, CollateClauseFunction collateClauseFunction) : Exp(Type::WindowPartitionColumnReference),
                                                                                                                         m_collateClauseFunction(collateClauseFunction) { AddChild(std::move(valueExp)); }

    CollateClauseFunction GetCollateClauseFunction() const { return m_collateClauseFunction; }
    ValueExp const* GetColumnRef() const { return GetChild<ValueExp>(0); }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowPartitionColumnReferenceListExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext& ctx) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowPartitionColumnReferenceListExp"; }

   public:
    WindowPartitionColumnReferenceListExp(std::vector<std::unique_ptr<WindowPartitionColumnReferenceExp>>& columnRefs) : Exp(Type::WindowPartitionColumnReferenceList) {
        for (auto& v : columnRefs) {
            AddChild(std::move(v));
        }
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct FirstWindowFrameBoundExp final : Exp {
   public:
    enum class WindowFrameBoundType {
        ValuePreceding,
        ValueFollowing,
        CurrentRow,
        UnboundedPreceding,
    };

   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFrameBoundType"; }

    WindowFrameBoundType m_windowFrameBoundType;
    size_t m_valueExpIndex = UNSET_CHILDINDEX;

   public:
    FirstWindowFrameBoundExp(FirstWindowFrameBoundExp::WindowFrameBoundType windowFrameBoundType)
        : m_windowFrameBoundType(windowFrameBoundType), Exp(Type::FirstWindowFrameBound) {}

    FirstWindowFrameBoundExp(std::unique_ptr<ValueExp> valueExp, FirstWindowFrameBoundExp::WindowFrameBoundType windowFrameBoundType)
        : FirstWindowFrameBoundExp(windowFrameBoundType) { m_valueExpIndex = AddChild(std::move(valueExp)); }

    WindowFrameBoundType GetWindowFrameBoundType() const { return m_windowFrameBoundType; }
    ValueExp const* GetValueExp() const {
        if (m_valueExpIndex == UNSET_CHILDINDEX)
            return nullptr;

        return GetChild<ValueExp>(m_valueExpIndex);
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SecondWindowFrameBoundExp final : Exp {
   public:
    enum class WindowFrameBoundType {
        ValuePreceding,
        ValueFollowing,
        CurrentRow,
        UnboundedFollowing,
    };

   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "SecondWindowFrameBoundExp"; }

    WindowFrameBoundType m_windowFrameBoundType;
    size_t m_valueExpIndex = UNSET_CHILDINDEX;

   public:
    SecondWindowFrameBoundExp(SecondWindowFrameBoundExp::WindowFrameBoundType windowFrameBoundType)
        : m_windowFrameBoundType(windowFrameBoundType), Exp(Type::SecondWindowFrameBound) {}

    SecondWindowFrameBoundExp(std::unique_ptr<ValueExp> valueExp, SecondWindowFrameBoundExp::WindowFrameBoundType windowFrameBoundType)
        : SecondWindowFrameBoundExp(windowFrameBoundType) { m_valueExpIndex = AddChild(std::move(valueExp)); }

    WindowFrameBoundType GetWindowFrameBoundType() const { return m_windowFrameBoundType; }
    ValueExp const* GetValueExp() const {
        if (m_valueExpIndex == UNSET_CHILDINDEX)
            return nullptr;

        return GetChild<ValueExp>(m_valueExpIndex);
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowFrameBetweenExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFrameBetweenExp"; }

    size_t m_firstWindowFrameBoundIndex = UNSET_CHILDINDEX;
    size_t m_secondWindowFrameBoundIndex = UNSET_CHILDINDEX;

   public:
    WindowFrameBetweenExp(std::unique_ptr<FirstWindowFrameBoundExp> first, std::unique_ptr<SecondWindowFrameBoundExp> second) : Exp(Type::WindowFrameBetween) {
        m_firstWindowFrameBoundIndex = AddChild(std::move(first));
        m_secondWindowFrameBoundIndex = AddChild(std::move(second));
    }

    FirstWindowFrameBoundExp const* GetFirstWindowFrameBoundExp() const { return GetChild<FirstWindowFrameBoundExp>(m_firstWindowFrameBoundIndex); }
    SecondWindowFrameBoundExp const* GetSecondWindowFrameBoundExp() const { return GetChild<SecondWindowFrameBoundExp>(m_secondWindowFrameBoundIndex); }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowFrameStartExp final : Exp {
   public:
    enum class WindowFrameStartType {
        UnboundedPreceding,
        CurrentRow,
        ValuePreceding,
    };

   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFrameStartExp"; }

    WindowFrameStartType m_windowFrameStartType;

   public:
    WindowFrameStartExp(WindowFrameStartType windowFrameStartType, std::unique_ptr<ValueExp> valueExp = nullptr) : Exp(Type::WindowFrameStart), m_windowFrameStartType(windowFrameStartType) {
        if (valueExp)
            AddChild(std::move(valueExp));
    }

    WindowFrameStartType GetWindowFrameStartType() const { return m_windowFrameStartType; }
    ValueExp const* GetValueExp() const {
        if (m_windowFrameStartType != WindowFrameStartType::ValuePreceding)
            return nullptr;
        return GetChild<ValueExp>(0);
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowFrameClauseExp final : Exp {
   public:
    enum class WindowFrameUnit {
        Rows,
        Range,
        Groups,
    };

    enum class WindowFrameExclusionType {
        NotSpecified,
        ExcludeCurrentRow,
        ExcludeGroup,
        ExcludeTies,
        ExcludeNoOthers,
    };

   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFrameClauseExp"; }

    WindowFrameUnit m_WindowFrameUnit;
    WindowFrameExclusionType m_windowFrameExclusionType;
    size_t m_windowFrameStartIndex = UNSET_CHILDINDEX;
    size_t m_windowFrameBetweenIndex = UNSET_CHILDINDEX;

    WindowFrameClauseExp(WindowFrameUnit windowFrameUnit, WindowFrameExclusionType windowFrameExclusionType) : m_WindowFrameUnit(windowFrameUnit), m_windowFrameExclusionType(windowFrameExclusionType), Exp(Type::WindowFrameClause) {}

   public:
    WindowFrameClauseExp(WindowFrameUnit windowFrameUnit, WindowFrameExclusionType windowFrameExclusionType, std::unique_ptr<WindowFrameStartExp> windowFrameStartExp) : WindowFrameClauseExp(windowFrameUnit, windowFrameExclusionType) { m_windowFrameStartIndex = AddChild(std::move(windowFrameStartExp)); }

    WindowFrameClauseExp(WindowFrameUnit windowFrameUnit, WindowFrameExclusionType windowFrameExclusionType, std::unique_ptr<WindowFrameBetweenExp> windowFrameBetweenExp) : WindowFrameClauseExp(windowFrameUnit, windowFrameExclusionType) { m_windowFrameBetweenIndex = AddChild(std::move(windowFrameBetweenExp)); }

    WindowFrameUnit GetWindowFrameUnit() const { return m_WindowFrameUnit; }
    WindowFrameExclusionType GetWindowFrameExclusionType() const { return m_windowFrameExclusionType; }

    WindowFrameStartExp const* GetWindowFrameStartExp() const {
        if (m_windowFrameStartIndex == UNSET_CHILDINDEX)
            return nullptr;

        return GetChild<WindowFrameStartExp>(m_windowFrameStartIndex);
    }

    WindowFrameBetweenExp const* GetWindowFrameBetweenExp() const {
        if (m_windowFrameBetweenIndex == UNSET_CHILDINDEX)
            return nullptr;

        return GetChild<WindowFrameBetweenExp>(m_windowFrameBetweenIndex);
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowSpecification final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext& ctx) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowSpecification"; }

    size_t m_partitionByClauseIndex = UNSET_CHILDINDEX;
    size_t m_orderByClauseIndex = UNSET_CHILDINDEX;
    size_t m_windowFrameClauseIndex = UNSET_CHILDINDEX;

    Utf8String m_windowName;

   public:
    WindowSpecification(Utf8String windowName, std::unique_ptr<WindowPartitionColumnReferenceListExp> partitionByClauseExp, std::unique_ptr<OrderByExp> orderByExp, std::unique_ptr<WindowFrameClauseExp> windowFrameClauseExp)
        : m_windowName(windowName), Exp(Type::WindowSpecification) {
        if (partitionByClauseExp != nullptr)
            m_partitionByClauseIndex = AddChild(std::move(partitionByClauseExp));

        if (orderByExp != nullptr)
            m_orderByClauseIndex = AddChild(std::move(orderByExp));

        if (windowFrameClauseExp != nullptr)
            m_windowFrameClauseIndex = AddChild(std::move(windowFrameClauseExp));
    }

    WindowPartitionColumnReferenceListExp const* GetPartitionBy() const {
        if (m_partitionByClauseIndex == UNSET_CHILDINDEX)
            return nullptr;
        return GetChild<WindowPartitionColumnReferenceListExp>(m_partitionByClauseIndex);
    }

    OrderByExp const* GetOrderBy() const {
        if (m_orderByClauseIndex == UNSET_CHILDINDEX)
            return nullptr;
        return GetChild<OrderByExp>(m_orderByClauseIndex);
    }

    WindowFrameClauseExp const* GetWindowFrameClause() const {
        if (m_windowFrameClauseIndex == UNSET_CHILDINDEX)
            return nullptr;
        return GetChild<WindowFrameClauseExp>(m_windowFrameClauseIndex);
    }

    const Utf8String& GetWindowName() const { return m_windowName; }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OrderByExp;
struct WindowFunctionExp final : ValueExp {
   private:
    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFunctionExp"; }

    Utf8String m_windowName = nullptr;
    size_t m_windowFunctionCallExp = UNSET_CHILDINDEX;
    size_t m_filterClauseIndex = UNSET_CHILDINDEX;
    size_t m_WindowSpecificationIndex = UNSET_CHILDINDEX;

    WindowFunctionExp(std::unique_ptr<ValueExp> windowFunctionCallExp, std::unique_ptr<FilterClauseExp> filterClauseExp) : ValueExp(Type::WindowFunction) {
        m_windowFunctionCallExp = AddChild(std::move(windowFunctionCallExp));

        if (filterClauseExp != nullptr)
            m_filterClauseIndex = AddChild(std::move(filterClauseExp));
    }

   public:
    WindowFunctionExp(std::unique_ptr<ValueExp> windowFunctionCallExp, std::unique_ptr<FilterClauseExp> filterClauseExp, std::unique_ptr<Exp> WindowSpecification) : WindowFunctionExp(std::move(windowFunctionCallExp), std::move(filterClauseExp)) { m_WindowSpecificationIndex = AddChild(std::move(WindowSpecification)); }

    WindowFunctionExp(std::unique_ptr<ValueExp> windowFunctionCallExp, std::unique_ptr<FilterClauseExp> filterClauseExp, Utf8CP windowName) : WindowFunctionExp(std::move(windowFunctionCallExp), std::move(filterClauseExp)) { m_windowName = windowName; }

    ValueExp const* GetWindowFunctionCallExp() const { return GetChild<ValueExp>(m_windowFunctionCallExp); }
    FilterClauseExp const* GetFilterClauseExp() const {
        if (m_filterClauseIndex == UNSET_CHILDINDEX)
            return nullptr;

        return GetChild<FilterClauseExp>(m_filterClauseIndex);
    }
    WindowSpecification const* GetWindowSpecification() const {
        if (m_WindowSpecificationIndex == UNSET_CHILDINDEX)
            return nullptr;
        return GetChild<WindowSpecification>(m_WindowSpecificationIndex);
    }
    const Utf8String& GetWindowName() const { return m_windowName; }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowDefinitionExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowDefinitionExp"; }

    Utf8String m_windowName;

   public:
    WindowDefinitionExp(Utf8String windowName, std::unique_ptr<WindowSpecification> windowSpecification) : m_windowName(windowName), Exp(Type::WindowDefinitionExp) { AddChild(std::move(windowSpecification)); }

    Utf8String GetWindowName() const { return m_windowName; }
    WindowSpecification const* GetWindowSpecification() const { return GetChild<WindowSpecification>(0); }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowDefinitionListExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowDefinitionListExp"; }

   public:
    WindowDefinitionListExp(std::vector<std::unique_ptr<WindowDefinitionExp>>& windowDefinitionExpList) : Exp(Type::WindowDefinitionListExpExp) {
        for (auto& windowDefinition : windowDefinitionExpList) {
            AddChild(std::move(windowDefinition));
        }
    }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct WindowFunctionClauseExp final : Exp {
   private:
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "WindowFunctionClauseExp"; }

   public:
    WindowFunctionClauseExp(std::unique_ptr<WindowDefinitionListExp> windowDefinitionListExp) : Exp(Type::WindowFunctionClauseExp) { AddChild(std::move(windowDefinitionListExp)); }

    WindowDefinitionListExp const* GetWindowDefinitionListExp() const { return GetChild<WindowDefinitionListExp>(0); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE