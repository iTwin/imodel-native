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
        size_t m_classNameExpIndex = UNSET_CHILDINDEX;

    protected:
        ValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, std::unique_ptr<ClassRefExp> classRefExp, Type type) : ValueExp(type)
            { 
            m_columnRefExpIndex = AddChild(std::move(columnRefExp));
            m_classNameExpIndex = AddChild(std::move(classRefExp));
            }

    public:
        DerivedPropertyExp const* GetColumnRefExp() const { return GetChild<DerivedPropertyExp>(m_columnRefExpIndex); }
        ClassNameExp const* GetClassNameExp() const { return GetChild<ClassNameExp>(m_classNameExpIndex); }
        PropertyNameExp const* GetPropertyNameExp() const
            {
            if (GetColumnRefExp()->GetExpression()->GetType() != Exp::Type::PropertyName)
                {
                BeAssert(false && "ValueCreationFuncExp column ref expression should be type of PropertyName");
                return nullptr;
                }
    
            return GetColumnRefExp()->GetExpression()->GetAsCP<PropertyNameExp>();
            }
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
        bool _TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const override { parameterExp.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Long)); return true; }
        void _ToECSql(ECSqlRenderContext&) const override;
        void _ToJson(BeJsValue, JsonFormat const&) const override;
        Utf8String _ToString() const override { return "NavValueCreationFuncExp"; }

    public:
        NavValueCreationFuncExp(std::unique_ptr<DerivedPropertyExp> columnRefExp, std::unique_ptr<ValueExp> idArgExp, std::unique_ptr<ValueExp> relECClassIdArgExp, std::unique_ptr<ClassNameExp> classNameExp)
            : ValueCreationFuncExp(std::move(columnRefExp), std::move(classNameExp), Type::NavValueCreationFunc)
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
