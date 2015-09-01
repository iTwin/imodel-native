//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSValueNode.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class HPSValueNode
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HPSValueNode.h>

//---------------------------------------------------------------------------
HPSValueNode::HPSValueNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    m_Type = NONE;
    m_IsOwnerOfValue = true;
    m_Value.m_pObjValue = 0;
    }

//---------------------------------------------------------------------------
HPSValueNode::~HPSValueNode()
    {
    if ((m_Type == STRING) && (m_Value.m_pString != 0))
        {
        delete m_Value.m_pString;
        }

    if ((m_Type == OBJECT) && (m_IsOwnerOfValue == true))
        {
        if (m_Value.m_pObjValue != 0)
            {
            m_Value.m_pObjValue->RemoveHFCPtr();
            }
        }
    }

//---------------------------------------------------------------------------
void HPSValueNode::FreeValue()
    {
    if ((m_Type == OBJECT) && (m_IsOwnerOfValue == true))
        {
        if (m_Value.m_pObjValue != 0)
            {
            m_Value.m_pObjValue->RemoveHFCPtr();
            }
        }

    m_IsOwnerOfValue = false;

    HPANode::FreeValue();
    }

//---------------------------------------------------------------------------
void HPSValueNode::ChangeValueOwnership(HPSValueNode* pi_pOwnerValueNode)
    {
    if (pi_pOwnerValueNode->GetValueOwnership() == true)
        {
        pi_pOwnerValueNode->SetValueOwnership(false);
        SetValueOwnership(true);
        }
    else
        {
        SetValueOwnership(false);
        }
    }
//---------------------------------------------------------------------------
HPSValueNode::ValueType HPSValueNode::GetValueType() const
    {
    return m_Type;
    }

//---------------------------------------------------------------------------
const HPSValueNode::Value& HPSValueNode::GetValue() const
    {
    return m_Value;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValueOwnership(bool pi_IsOwner)
    {
    m_IsOwnerOfValue = pi_IsOwner;
    }

//---------------------------------------------------------------------------
bool HPSValueNode::GetValueOwnership() const
    {
    return m_IsOwnerOfValue;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValue(int32_t pi_Value)
    {
    m_Type           = NUMBER;
    m_Value.m_Number = pi_Value;
    m_IsOwnerOfValue = true;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValue(double pi_Value)
    {
    m_Type = NUMBER;
    m_Value.m_Number = pi_Value;
    m_IsOwnerOfValue = true;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValue(const WString& pi_rString)
    {
    m_Type = STRING;
    m_Value.m_pString = new WString(pi_rString);
    m_IsOwnerOfValue = true;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValue(HPSObjectValue* pi_pObj)
    {
    m_Type = OBJECT;
    m_Value.m_pObjValue = pi_pObj;
    m_IsOwnerOfValue = true;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValueFrom(HPSValueNode* pi_pNode)
    {
    m_Type = pi_pNode->m_Type;
    if (m_Type == STRING)
        {
        if (m_Value.m_pString == 0)
            {
            m_Value.m_pString = new WString(*(pi_pNode->m_Value.m_pString));
            }
        }
    else
        {
        m_Value = pi_pNode->m_Value;
        }

    ChangeValueOwnership(pi_pNode);
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetType(HPSValueNode::ValueType pi_Type)
    {
    m_Type = pi_Type;
    }

//---------------------------------------------------------------------------
void HPSValueNode::SetValue(const HPSValueNode::Value& pi_rValue)
    {
    HPRECONDITION(m_Type != NONE);
    if (m_Type == STRING)
        {
        if (m_Value.m_pString == 0)
            {
            m_Value.m_pString = new WString(*(pi_rValue.m_pString));
            }
        }
    else
        {
        m_Value = pi_rValue;
        }

    m_IsOwnerOfValue = true;
    }