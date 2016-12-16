/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ListExp.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct SystemPropertyExpIndexMap : NonCopyableClass
    {
    private:
        bmap<ECSqlSystemPropertyInfo, size_t, ECSqlSystemPropertyInfo::LessThan> m_sysPropIndexMap;

    public:
        SystemPropertyExpIndexMap() {}

        template<typename TSysProp>
        bool Contains(TSysProp info) const { return m_sysPropIndexMap.find(ECSqlSystemPropertyInfo(info)) != m_sysPropIndexMap.end(); }
        
        //!@return non-negative index if found. -1 else.
        template<typename TSysProp>
        int GetIndex(TSysProp info) const { auto it = m_sysPropIndexMap.find(ECSqlSystemPropertyInfo(info)); return it == m_sysPropIndexMap.end() ? -1 : (int) it->second; }

        void AddIfSystemProperty(PropertyNameExp const&, size_t index);
        void AddIfSystemProperty(ECDbSchemaManager const&, ECN::ECPropertyCR, size_t index);
    };

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

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "AssignmentListExp"; }

public:
    AssignmentListExp () : Exp () {}

    void AddAssignmentExp (std::unique_ptr<AssignmentExp> assignmentExp);
    AssignmentExp const* GetAssignmentExp (size_t index) const;
    SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap () const { return m_specialTokenExpIndexMap; }
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

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "PropertyNameList"; }

public :
    PropertyNameListExp () : Exp () {}

    void AddPropertyNameExp (std::unique_ptr<PropertyNameExp>& propertyNameExp);
    PropertyNameExp const* GetPropertyNameExp (size_t index) const;
    SystemPropertyExpIndexMap const& GetSpecialTokenExpIndexMap () const { return m_specialTokenExpIndexMap; }
    };


//************************ ValueExpListExp ******************************
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2013
//+===============+===============+===============+===============+===============+======
struct ValueExpListExp : ComputedExp
    {
public:
    DEFINE_EXPR_TYPE(ValueExpList)

private:
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString () const override { return "ValueExpList"; }

public:
    ValueExpListExp () : ComputedExp () {}

    void AddValueExp (std::unique_ptr<ValueExp>& valueExp);
    ParameterExp* TryGetAsParameterExpP(size_t index) const;
    ValueExp const* GetValueExp(size_t index) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE