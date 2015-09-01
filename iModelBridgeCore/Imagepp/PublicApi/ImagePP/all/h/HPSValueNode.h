//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSValueNode.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPSValueNode.h
//---------------------------------------------------------------------------
// Type of node for rules that, when executed, refer to some value.
//---------------------------------------------------------------------------


#pragma once

#include <Imagepp/all/h/HPANode.h>

BEGIN_IMAGEPP_NAMESPACE

class HPSObjectValue : public HFCShareableObject<HPSObjectValue>
    {
public:
    HPSObjectValue() {};
    virtual ~HPSObjectValue() {};

    void AddHFCPtr()    {_internal_NotifyAdditionOfASmartPointer();}
    void RemoveHFCPtr() {_internal_NotifyRemovalOfASmartPointer();}
    };

class HNOVTABLEINIT HPSValueNode : public HPANode
    {
public:

    enum ValueType
        {
        NONE,
        NUMBER,
        STRING,
        OBJECT
        };

    union Value
        {
        double  m_Number;
        WString* m_pString;
        HPSObjectValue* m_pObjValue;
        };

    HPSValueNode(HPAGrammarObject* pi_pObj,
                 const HPANodeList& pi_rList,
                 const HFCPtr<HPASession>& pi_pSession);

    virtual             ~HPSValueNode();

    ValueType           GetValueType() const;
    const Value&        GetValue() const;

    virtual void        Calculate() = 0;
    virtual void        FreeValue();

    void                SetValueOwnership(bool pi_IsOwner);
    bool               GetValueOwnership() const;
    void                ChangeValueOwnership(HPSValueNode* pi_pOwnerValueNode);

protected:

    void                SetValue(int32_t pi_Value);
    void                SetValue(double pi_Value);
    void                SetValue(const WString& pi_rString);
    void                SetValue(HPSObjectValue* pi_pObj);

    void                SetType(ValueType pi_Type);
    void                SetValue(const Value& pi_rValue);

    void                SetValueFrom(HPSValueNode* pi_pNode);

    bool               m_IsOwnerOfValue;
private:

    ValueType           m_Type;
    Value               m_Value;
    };

END_IMAGEPP_NAMESPACE