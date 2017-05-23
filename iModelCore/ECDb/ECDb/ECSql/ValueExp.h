/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ValueExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ComputedExp.h"
#include "ClassRefExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct ParameterExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ValueExp : ComputedExp
    {
    protected:
        bool m_isConstant = false;

        explicit ValueExp(Type type) : ValueExp(type, false) {}
        ValueExp(Type type, bool isConstant) : ComputedExp(type), m_isConstant(isConstant) {}

        void SetIsConstant(bool isConstant) { m_isConstant = isConstant; }
    public:
        virtual ~ValueExp() {}
        bool IsConstant() const { return m_isConstant; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BetweenRangeValueExp final : ValueExp
    {
    private:
        size_t m_lowerBoundOperandExpIndex;
        size_t m_upperBoundOperandExpIndex;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "BetweenRangeValue"; }

    public:
        BetweenRangeValueExp(std::unique_ptr<ValueExp> lowerBound, std::unique_ptr<ValueExp> upperBound) : ValueExp(Type::BetweenRangeValue)
            {
            m_lowerBoundOperandExpIndex = AddChild(std::move(lowerBound));
            m_upperBoundOperandExpIndex = AddChild(std::move(upperBound));
            }

        ValueExp const* GetLowerBoundOperand() const { return GetChild<ValueExp>(m_lowerBoundOperandExpIndex); }
        ValueExp const* GetUpperBoundOperand() const { return GetChild<ValueExp>(m_upperBoundOperandExpIndex); }

    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BinaryValueExp final : ValueExp
    {
    private:
        size_t m_leftOperandExpIndex;
        size_t m_rightOperandExpIndex;
        BinarySqlOperator m_op;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        BinaryValueExp(std::unique_ptr<ValueExp> lhs, BinarySqlOperator op, std::unique_ptr<ValueExp> rhs)
            : ValueExp(Type::BinaryValue, lhs->IsConstant() && rhs->IsConstant()), m_op(op)
            {
            m_leftOperandExpIndex = AddChild(std::move(lhs));
            m_rightOperandExpIndex = AddChild(std::move(rhs));
            }

        ValueExp const* GetLeftOperand() const { return GetChild<ValueExp>(m_leftOperandExpIndex); }
        ValueExp const* GetRightOperand() const { return GetChild<ValueExp>(m_rightOperandExpIndex); }
        BinarySqlOperator GetOperator() const { return m_op; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct CastExp final : ValueExp
    {
    private:
        Utf8String m_castTargetSchemaName;
        Utf8String m_castTargetTypeName;
        bool m_castTargetIsArray;
        size_t m_castOperandIndex;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        CastExp(std::unique_ptr<ValueExp> castOperand, Utf8CP castTargetPrimitiveType, bool castTargetIsArray)
            : ValueExp(Type::Cast, castOperand->IsConstant()), m_castTargetTypeName(castTargetPrimitiveType), m_castTargetIsArray(castTargetIsArray)
            {
            m_castOperandIndex = AddChild(std::move(castOperand));
            }

        CastExp(std::unique_ptr<ValueExp> castOperand, Utf8StringCR castTargetSchemaName, Utf8StringCR castTargetClassName, bool castTargetIsArray)
            : ValueExp(Type::Cast, castOperand->IsConstant()), m_castTargetSchemaName(castTargetSchemaName), m_castTargetTypeName(castTargetClassName), m_castTargetIsArray(castTargetIsArray)
            {
            m_castOperandIndex = AddChild(std::move(castOperand));
            }

        ValueExp const* GetCastOperand() const { return GetChild<ValueExp>(m_castOperandIndex); }
        Utf8StringCR GetCastTargetSchemaName() const { return m_castTargetSchemaName; }
        Utf8StringCR GetCastTargetClassName() const { BeAssert(!m_castTargetSchemaName.empty()); return m_castTargetTypeName; }
        Utf8StringCR GetCastTargetPrimitiveType() const { BeAssert(m_castTargetSchemaName.empty()); return m_castTargetTypeName; }
        bool CastTargetIsArray() const { return m_castTargetIsArray; }
        bool NeedsCasting() const;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct LiteralValueExp final : ValueExp
    {
    public:
        static Utf8CP const CURRENT_DATE;
        static Utf8CP const CURRENT_TIMESTAMP;

    private:
        Utf8String m_value;

        LiteralValueExp(Utf8CP value, ECSqlTypeInfo type);

        BentleyStatus ResolveDataType(ECSqlParseContext&);

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        static BentleyStatus Create(std::unique_ptr<ValueExp>&, ECSqlParseContext&, Utf8CP value, ECSqlTypeInfo type);

        Utf8StringCR GetValue() const { return m_value; }
        int64_t GetValueAsInt64() const;
        bool GetValueAsBoolean() const;

        static Utf8String EscapeStringLiteral(Utf8StringCR constantStringLiteral);
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct FunctionCallExp final : ValueExp
    {
    private:
        Utf8String m_functionName;
        bool m_isStandardSetFunction;
        SqlSetQuantifier m_setQuantifier;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

        static ECN::PrimitiveType DetermineReturnType(ECDbCR, Utf8StringCR functionName, int argCount);

    public:
        explicit FunctionCallExp(Utf8StringCR functionName, SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified, bool isStandardSetFunction = false) : ValueExp(Type::FunctionCall), m_functionName(functionName), m_setQuantifier(setQuantifier), m_isStandardSetFunction(isStandardSetFunction) {}
        virtual ~FunctionCallExp() {}

        Utf8StringCR GetFunctionName() const { return m_functionName; }
        SqlSetQuantifier GetSetQuantifier() const { return m_setQuantifier; }

        BentleyStatus AddArgument(std::unique_ptr<ValueExp>);
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct LikeRhsValueExp final : ValueExp
    {
    private:
        size_t m_rhsExpIndex;
        int m_escapeExpIndex = UNSET_CHILDINDEX;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "LikeRhsValue"; }

    public:
        explicit LikeRhsValueExp(std::unique_ptr<ValueExp> rhsExp, std::unique_ptr<ValueExp> escapeExp = nullptr)
            : ValueExp(Type::LikeRhsValue, rhsExp->IsConstant())
            {
            m_rhsExpIndex = AddChild(std::move(rhsExp));
            if (escapeExp != nullptr)
                m_escapeExpIndex = (int) AddChild(std::move(escapeExp));
            }

        ValueExp const* GetRhsExp() const { return GetChild<ValueExp>(m_rhsExpIndex); }
        bool HasEscapeExp() const { return m_escapeExpIndex >= 0; }
        ValueExp const* GetEscapeExp() const;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ParameterExp final : ValueExp
    {
    private:
        int m_parameterIndex = -1;
        Utf8String m_parameterName;
        ComputedExp const* m_targetExp = nullptr;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        explicit ParameterExp(Utf8CP parameterName) : ValueExp(Type::Parameter), m_parameterName(parameterName) {}

        void SetDefaultTargetExpInfo();
        void SetTargetExpInfo(ComputedExp const& exp);
        void SetTargetExpInfo(ECSqlTypeInfo const& targetTypeInfo);
        ComputedExp const* GetTargetExp() const { return m_targetExp; }
        bool IsNamedParameter() const { return !m_parameterName.empty(); }
        Utf8StringCR GetParameterName() const { return m_parameterName; }
        int GetParameterIndex() const { return m_parameterIndex; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct UnaryValueExp final : ValueExp
    {
    public:
        enum class Operator
            {
            Minus,
            Plus,
            BitwiseNot
            };

    private:
        size_t m_operandExpIndex;
        Operator m_op;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        UnaryValueExp(std::unique_ptr<ValueExp>& operand, Operator op) : ValueExp(Type::UnaryValue, operand->IsConstant()), m_op(op)
            {
            m_operandExpIndex = AddChild(std::move(operand));
            }

        ValueExp const* GetOperand() const { return GetChild<ValueExp>(m_operandExpIndex); }
        Operator GetOperator() const { return m_op; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
