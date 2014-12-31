/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ListExp.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************ AssignmentListExp ******************************
struct AssignmentExp;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct AssignmentListExp : Exp
    {
DEFINE_EXPR_TYPE (AssignmentList)
private:
    SystemPropertyExpIndexMap m_specialTokenExpIndexMap;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    virtual Utf8String _ToString () const override
        {
        return "AssignmentListExp";
        }

public:
    AssignmentListExp ()
        : Exp ()
        {}

    void AddAssignmentExp (std::unique_ptr<AssignmentExp> assignmentExp);
    AssignmentExp const* GetAssignmentExp (size_t index) const;
    SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap () const { return m_specialTokenExpIndexMap; }
    virtual Utf8String ToECSql () const override;
    };

//************************ PropertyNameListExp ******************************
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2013
//+===============+===============+===============+===============+===============+======
struct PropertyNameListExp : Exp
    {
DEFINE_EXPR_TYPE(PropertyNameList) 
private:
    SystemPropertyExpIndexMap m_specialTokenExpIndexMap;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    virtual Utf8String _ToString () const override
        {
        return "PropertyNameList";
        }

public :
    PropertyNameListExp ()
        : Exp () 
        {}

    void AddPropertyNameExp (std::unique_ptr<PropertyNameExp>& propertyNameExp);
    PropertyNameExp const* GetPropertyNameExp (size_t index) const;
    SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap () const { return m_specialTokenExpIndexMap; }
    virtual Utf8String ToECSql() const override;
    };

//************************ RowValueConstructorListExp ******************************
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2013
//+===============+===============+===============+===============+===============+======
struct RowValueConstructorListExp : Exp
    {
public:
    DEFINE_EXPR_TYPE(RowValueConstructorList) 

private:
    virtual Utf8String _ToString () const override
        {
        return "RowValueConstructorList";
        }

public:
    RowValueConstructorListExp ();

    void AddRowValueConstructorExp (std::unique_ptr<ValueExp>& valueExp);
    ParameterExp* TryGetAsParameterExpP (size_t index) const;

    virtual Utf8String ToECSql () const override;  
    };


//************************ ValueListExp ******************************
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2013
//+===============+===============+===============+===============+===============+======
struct ValueListExp : ComputedExp
    {
    public:
        DEFINE_EXPR_TYPE(ValueList)

    private:
        virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

        virtual Utf8String _ToString () const override
            {
            return "ValueList";
            }

    public:
        ValueListExp ()
            : ComputedExp ()
            {}

        void AddValueExp (std::unique_ptr<ValueExp>& valueExp);
        virtual Utf8String ToECSql () const override;  
    };

END_BENTLEY_SQLITE_EC_NAMESPACE