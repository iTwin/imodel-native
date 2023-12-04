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
struct ValueCreationFuncExp : ValueExp
    {
    public:
        enum class ValueCreationFunctionType
            {
            Nav
            };

    private:
        ValueCreationFunctionType m_valueCreationFunctionType;
        size_t m_columnRefExpIndex = UNSET_CHILDINDEX;

    public:
        ValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, ValueCreationFunctionType type) : ValueExp(Type::ValueCreationFuncExp), m_valueCreationFunctionType(type)
            { m_columnRefExpIndex = AddChild(std::move(columnRefExp)); }

        Exp const* GetColumnRefExp() const { return GetChild<Exp>(m_columnRefExpIndex); }
        ValueCreationFunctionType GetValueCreationFunctionType() const { return m_valueCreationFunctionType; }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct NavValueCreationFuncExp final : ValueCreationFuncExp
    {
    private:
        size_t m_idArgExpIndex = UNSET_CHILDINDEX;
        size_t m_relECClassIdArgExp = UNSET_CHILDINDEX;
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override { return FinalizeParseStatus::Completed; }
        void _ToECSql(ECSqlRenderContext&) const override {}
        void _ToJson(BeJsValue, JsonFormat const&) const {}
        Utf8String _ToString() const override { return "WindowFunctionExp"; }

    public:
        NavValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, std::unique_ptr<ValueExp> idArgExp, std::unique_ptr<ValueExp> relECClassIdArgExp) : ValueCreationFuncExp(std::move(columnRefExp), ValueCreationFunctionType::Nav)
            {
            m_idArgExpIndex = AddChild(std::move(idArgExp));
            
            if (relECClassIdArgExp != nullptr)
                m_relECClassIdArgExp = AddChild(std::move(relECClassIdArgExp));
            }
        
        ValueExp const* GetIdArgExp() const { return GetChild<ValueExp>(m_idArgExpIndex); }
        ValueExp const* GetRelECClassId() const
            {
            if (m_relECClassIdArgExp == UNSET_CHILDINDEX)
                return nullptr;
            return GetChild<ValueExp>(m_relECClassIdArgExp);
            }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE