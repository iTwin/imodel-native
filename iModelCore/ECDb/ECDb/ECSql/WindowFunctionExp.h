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

struct FilterClauseExp final : Exp
    {
    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "WindowPartitionColumnReferenceList"; }
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

    public:
        FilterClauseExp(std::unique_ptr<WhereExp> whereExp) : Exp(Type::FIlterClause)
        { AddChild(std::move(whereExp)); }

        WhereExp const* GetWhereExp() const { return GetChild<WhereExp>(0); }
    };
struct WindowPartitionColumnReferenceExp final : Exp
    {
    public:
        enum class CollateClauseFunction
            {
            NotSpecified,
            Binary,
            NoCase,
            Rtrim,
            };

    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "WindowPartitionColumnReferenceList"; }
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        CollateClauseFunction m_collateClauseFunction;

    public:
        WindowPartitionColumnReferenceExp(std::unique_ptr<ValueExp> valueExp, CollateClauseFunction collateClauseFunction) :
            Exp(Type::WindowPartitionColumnReference),
            m_collateClauseFunction(collateClauseFunction)
            { AddChild(std::move(valueExp)); }

        CollateClauseFunction GetCollateClauseFunction() const { return m_collateClauseFunction; }
        ValueExp const* GetColumnRef() const { return GetChild<ValueExp>(0); }
    };


struct WindowPartitionColumnReferenceListExp final : Exp
    {
    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "WindowPartitionColumnReferenceList"; }
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    
    public:
        WindowPartitionColumnReferenceListExp(std::vector<std::unique_ptr<WindowPartitionColumnReferenceExp>>& columnRefs) : Exp(Type::WindowPartitionColumnReferenceList)
            {
            for (auto& v : columnRefs)
                {
                AddChild(std::move(v));
                }
            }

    };

struct WindowSpecification final : Exp
    {
    private:
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override;
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        size_t m_partitionByClauseIndex = UNSET_CHILDINDEX;
        size_t m_orderByClauseIndex = UNSET_CHILDINDEX;
    
    public:

        WindowSpecification(std::unique_ptr<WindowPartitionColumnReferenceListExp> partitionByClauseExp, std::unique_ptr<OrderByExp> orderByExp) : Exp(Type::WindowSpecification)
            {
            if (partitionByClauseExp != nullptr)
                m_partitionByClauseIndex = AddChild(std::move(partitionByClauseExp));
            
            if (orderByExp != nullptr)
                m_orderByClauseIndex = AddChild(std::move(orderByExp));
            }

        WindowPartitionColumnReferenceListExp const* GetPartitionBy() const
            {
            if (m_partitionByClauseIndex == UNSET_CHILDINDEX)
                return nullptr;
            return GetChild<WindowPartitionColumnReferenceListExp>(m_partitionByClauseIndex);
            }
        OrderByExp const* GetOrderBy() const
            {
            if (m_orderByClauseIndex == UNSET_CHILDINDEX)
                return nullptr;
            return GetChild<OrderByExp>(m_orderByClauseIndex);
            }
    };

struct WindowFunctionExp final : ValueExp
    {
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override;

        Utf8CP m_windowName = nullptr;
        size_t m_windowFunctionTypeIndex = UNSET_CHILDINDEX;
        size_t m_filterClauseIndex = UNSET_CHILDINDEX;
        size_t m_WindowSpecificationIndex = UNSET_CHILDINDEX;

        WindowFunctionExp(std::unique_ptr<ValueExp> windowFunctionType, std::unique_ptr<FilterClauseExp> filterClauseExp) : ValueExp(Type::WindowFunction)
            {
            m_windowFunctionTypeIndex = AddChild(std::move(windowFunctionType));
            m_filterClauseIndex = AddChild(std::move(filterClauseExp));
            }

    public:
        WindowFunctionExp(std::unique_ptr<ValueExp>windowFunctionType, std::unique_ptr<FilterClauseExp> filterClauseExp, std::unique_ptr<Exp>WindowSpecification) :
            WindowFunctionExp(std::move(windowFunctionType), std::move(filterClauseExp))
            { m_WindowSpecificationIndex = AddChild(std::move(WindowSpecification)); }

        WindowFunctionExp(std::unique_ptr<ValueExp> windowFunctionType, std::unique_ptr<FilterClauseExp> filterClauseExp, Utf8CP windowName) :
            WindowFunctionExp(std::move(windowFunctionType), std::move(filterClauseExp))
            { m_windowName = windowName; }
        
        ValueExp const* GetWindowFunctionType() const { return GetChild<ValueExp>(m_windowFunctionTypeIndex); }
        FilterClauseExp const* GetFilterClauseExp() const { return GetChild<FilterClauseExp>(m_filterClauseIndex); }
        WindowSpecification const* GetWindowSpecification() const 
            {
            if (m_WindowSpecificationIndex == UNSET_CHILDINDEX)
                return nullptr; 
            return GetChild<WindowSpecification>(m_WindowSpecificationIndex);
            }
        Utf8CP const GetWindowName() const { return m_windowName; }

    };

struct WindowFunctionType final : ValueExp
    {
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override;

    public:
        WindowFunctionType(std::unique_ptr<ValueExp> functionCallExp) : ValueExp(Type::WindowFunctionType)
            {
            AddChild(std::move(functionCallExp));
            }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE