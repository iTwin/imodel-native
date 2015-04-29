/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ValueExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ComputedExp.h"
#include "ClassRefExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ValueExp : ComputedExp
    {
    DEFINE_EXPR_TYPE(Value)
protected:
    ValueExp ()
        : ComputedExp ()
        {}

public:
    virtual ~ValueExp () {}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BetweenRangeValueExp : ValueExp
    {
DEFINE_EXPR_TYPE(BetweenRangeValue) 

private:
    size_t m_lowerBoundOperandExpIndex;
    size_t m_upperBoundOperandExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        return "BetweenRangeValue";
        }

public:
    BetweenRangeValueExp(std::unique_ptr<ValueExp> lowerBound, std::unique_ptr<ValueExp> upperBound)
        : ValueExp ()
        {
        m_lowerBoundOperandExpIndex = AddChild (std::move(lowerBound));
        m_upperBoundOperandExpIndex = AddChild (std::move(upperBound));
        }

    ValueExp const* GetLowerBoundOperand () const {return GetChild<ValueExp> (m_lowerBoundOperandExpIndex);}
    ValueExp const* GetUpperBoundOperand() const {return GetChild<ValueExp> (m_upperBoundOperandExpIndex);}

    virtual Utf8String ToECSql() const override  
        {
        return GetLowerBoundOperand ()->ToECSql() + " AND "  + GetUpperBoundOperand ()->ToECSql(); 
        }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BinaryExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Binary) 

private:
    size_t m_leftOperandExpIndex;
    size_t m_rightOperandExpIndex;
    SqlBinaryOperator m_op;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    BinaryExp(std::unique_ptr<ValueExp> lhs, SqlBinaryOperator op ,std::unique_ptr<ValueExp> rhs)
        : ValueExp (), m_op(op)
        {
        m_leftOperandExpIndex = AddChild (std::move(lhs));
        m_rightOperandExpIndex = AddChild (std::move(rhs));
        }

    ValueExp const* GetLeftOperand() const {return GetChild<ValueExp> (m_leftOperandExpIndex);}
    ValueExp const* GetRightOperand() const {return GetChild<ValueExp> (m_rightOperandExpIndex);}
    SqlBinaryOperator GetOperator() const {return m_op;}

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct CastExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Cast) 
private:
    Utf8String m_castTargetType;
    size_t m_castOperandIndex;
  
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    CastExp (std::unique_ptr<ValueExp> castOperand, Utf8CP castTargetType)
        : ValueExp (), m_castTargetType (castTargetType)
        {
        m_castOperandIndex = AddChild (std::move (castOperand));
        }

    ValueExp const* GetCastOperand() const { return GetChild<ValueExp> (m_castOperandIndex);}
    Utf8String GetCastTargetType() const {return m_castTargetType;}

    bool NeedsCasting () const;

    virtual Utf8String ToECSql () const override;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ConstantValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(ConstantValue) 
public:
    static Utf8CP const CURRENT_DATE;
    static Utf8CP const CURRENT_TIMESTAMP;

private:
    Utf8String m_value;

    ConstantValueExp (Utf8CP value, ECSqlTypeInfo type);

    ECSqlStatus ResolveDataType (ECSqlParseContext& ctx);

    virtual Utf8String _ToString () const override;

public:
    static std::unique_ptr<ConstantValueExp> Create (ECSqlParseContext& ctx, Utf8CP value, ECSqlTypeInfo type);

    Utf8StringCR GetValue () const;
    int64_t GetValueAsInt64() const;
    bool GetValueAsBoolean() const;

    virtual Utf8String ToECSql() const override;
    static Utf8String EscapeStringLiteral (Utf8StringCR constantStringLiteral)
        {
        Utf8String tmp = constantStringLiteral;
        tmp.ReplaceAll ("'", "''");
        return tmp;
        }
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECClassIdFunctionExp : ValueExp
    {
DEFINE_EXPR_TYPE(ECClassIdFunction)

private:
    Utf8String m_classAlias;
    RangeClassRefExp const* m_classRefExp;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    virtual Utf8String _ToString () const override;

    bool HasClassAlias () const { return !m_classAlias.empty ();}

public:
    explicit ECClassIdFunctionExp (Utf8CP classAlias)
        : ValueExp (), m_classAlias (classAlias), m_classRefExp (nullptr)
        {
        }

    ~ECClassIdFunctionExp () {}

    Utf8CP GetClassAlias () const { return m_classAlias.c_str ();}

    RangeClassRefExp const* GetClassRefExp () const { return m_classRefExp;}

    virtual Utf8String ToECSql () const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct FunctionCallExp : ValueExp
    {
    DEFINE_EXPR_TYPE(FunctionCall)
private:
    Utf8String m_functionName;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    
    virtual Utf8String _ToString () const override
        {
        Utf8String str ("FunctionCall [Function: ");
        str.append (m_functionName.c_str ()).append ("]");
        return str;
        }

protected:
    bool IsValidArgCount (ECSqlParseContext& ctx, size_t expectedArgCount, size_t actualArgCount) const;

public:
    explicit FunctionCallExp (Utf8CP functionName)
        : ValueExp (), m_functionName(functionName)
        {
        }

    virtual ~FunctionCallExp () {}

    Utf8StringCR GetFunctionName() const { return m_functionName;}

    void AddArgument (std::unique_ptr<ValueExp> argument);

    virtual Utf8String ToECSql() const override  
        {
        Utf8String tmp = m_functionName + "(";
        bool isFirstItem = true;
        for(auto argExp : GetChildren ())
            {
            if (!isFirstItem)
                tmp.append(", ");

            tmp.append(argExp->ToECSql ());
            isFirstItem = false;
            }
        tmp.append(")");
        return tmp;
        }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct LikeRhsValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(LikeRhsValue) 
private:
    size_t m_rhsExpIndex;
    int m_escapeExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        return "LikeRhsValue";
        }

public:
    explicit LikeRhsValueExp (std::unique_ptr<ValueExp> rhsExp, std::unique_ptr<ValueExp> escapeExp = nullptr)
        : ValueExp (), m_escapeExpIndex (UNSET_CHILDINDEX)
        {
        m_rhsExpIndex = AddChild (std::move(rhsExp));
        if (escapeExp != nullptr)
            m_escapeExpIndex = static_cast<int> (AddChild (std::move(escapeExp)));
        }

    ValueExp const* GetRhsExp () const {return GetChild<ValueExp> (m_rhsExpIndex);}
    bool HasEscapeExp () const { return m_escapeExpIndex >= 0;}
    ValueExp const* GetEscapeExp () const;

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ParameterExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Parameter)
private:
    int m_parameterIndex;
    Utf8String m_parameterName;
    ComputedExp const* m_targetExp;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        Utf8String str ("Parameter [Name: ");
        str.append (m_parameterName).append ("]");
        return str;
        }

public:
    explicit ParameterExp (Utf8CP parameterName)
        : ValueExp (), m_parameterName (parameterName), m_parameterIndex (-1), m_targetExp (nullptr)
        {}

    void SetTargetExp (ComputedExp const& exp);
    void SetTypeInfoFromTarget (ECSqlTypeInfo const& targetTypeInfo);

    ComputedExp const* GetTargetExp () const
        {
        return m_targetExp;
        }

    bool IsNamedParameter () const { return !m_parameterName.empty (); }
    Utf8CP GetParameterName () const { return m_parameterName.c_str (); }
    int GetParameterIndex () const { return m_parameterIndex; }
    virtual Utf8String ToECSql () const override;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SetFunctionCallExp : FunctionCallExp
    {
    DEFINE_EXPR_TYPE(SetFunctionCall)
private:
    SqlSetQuantifier m_setQuantifier;
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;
public:
    explicit SetFunctionCallExp (Utf8CP functionName, SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified)
        : FunctionCallExp (functionName), m_setQuantifier (setQuantifier)
        {
        }

    SqlSetQuantifier GetSetQuantifier() const { return m_setQuantifier; }

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct StringFunctionCallExp : FunctionCallExp
    {
DEFINE_EXPR_TYPE (StringFunctionCall)
private:
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    explicit StringFunctionCallExp (Utf8CP functionName)
        : FunctionCallExp (functionName)
        {
        }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct UnaryExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Unary)

private:
    size_t m_operandExpIndex;
    SqlUnaryOperator m_op;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    UnaryExp(ValueExp* operand, SqlUnaryOperator op)
        : ValueExp (), m_op(op)
        {
        m_operandExpIndex = AddChild (std::unique_ptr<Exp> (operand));
        }

    ValueExp const* GetOperand() const {return GetChild<ValueExp> (m_operandExpIndex);}
    SqlUnaryOperator GetOperator() const {return m_op;}

    virtual Utf8String ToECSql() const override;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
