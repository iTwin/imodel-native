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
    private:
        size_t m_columnRefExpIndex = UNSET_CHILDINDEX;
        size_t m_classRefExpIndex = UNSET_CHILDINDEX;

    public:
        ValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, std::unique_ptr<ClassRefExp> classRefExp, Type type) : ValueExp(type)
            { 
            m_columnRefExpIndex = AddChild(std::move(columnRefExp));
            m_classRefExpIndex = AddChild(std::move(classRefExp));
            }

        Exp const* GetColumnRefExp() const { return GetChild<Exp>(m_columnRefExpIndex); }
        ClassRefExp const* GetClassRefExp() const { return GetChild<ClassRefExp>(m_classRefExpIndex); }
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
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const {}
        Utf8String _ToString() const override { return "NavValueCreationFuncExp"; }

    public:
        NavValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, std::unique_ptr<ValueExp> idArgExp, std::unique_ptr<ValueExp> relECClassIdArgExp, std::unique_ptr<ClassRefExp> classRefExp)
            : ValueCreationFuncExp(std::move(columnRefExp), std::move(classRefExp), Type::NavValueCreationFunc)
            {
            m_idArgExpIndex = AddChild(std::move(idArgExp));
            
            if (relECClassIdArgExp != nullptr)
                m_relECClassIdArgExp = AddChild(std::move(relECClassIdArgExp));
            }
        
        ValueExp const* GetIdArgExp() const { return GetChild<ValueExp>(m_idArgExpIndex); }
        ValueExp const* GetRelECClassIdExp() const
            {
            if (m_relECClassIdArgExp == UNSET_CHILDINDEX)
                return nullptr;
            return GetChild<ValueExp>(m_relECClassIdArgExp);
            }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE