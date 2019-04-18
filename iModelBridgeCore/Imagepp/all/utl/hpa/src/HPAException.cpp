//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAException
//---------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HPAException.h>
#include <ImagePP/all/h/HPANode.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException(HFCPtr<HPANode>&    pi_rpOffendingNode)
    : HFCException()
    {
    m_pOffendingNode = pi_rpOffendingNode;   
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException()
    : HFCException()
    {
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPAException::HPAException(const HPAException&     pi_rObj) : HFCException(pi_rObj)
 {
     m_pOffendingNode =pi_rObj.m_pOffendingNode;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPAException::~HPAException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
Utf8String HPAException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_pOffendingNode->GetEndPos().m_Line,
                                 m_pOffendingNode->GetEndPos().m_Column);
    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HFCPtr<HPANode>& HPAException::GetOffendingNode() const
    {
    return m_pOffendingNode;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAGenericException::HPAGenericException(HFCPtr<HPANode>&    pi_rpOffendingNode,
                                         const Utf8String&      pi_rMessage)
    : HPAException(pi_rpOffendingNode)
    {
    m_pOffendingNode = pi_rpOffendingNode;
    m_Message = pi_rMessage.c_str();
    }

//---------------------------------------------------------------------------
// This constructor extracts the needed information from a HPAException
//---------------------------------------------------------------------------
HPAGenericException::HPAGenericException(const HPAException* pi_pObj)
    : HPAException()
    {
    HPRECONDITION(pi_pObj != 0);

    m_pOffendingNode = pi_pObj->GetOffendingNode();
    m_Message = pi_pObj->GetExceptionMessage();

    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPAGenericException::HPAGenericException(const HPAGenericException&     pi_rObj) : HPAException(pi_rObj)
 {
     m_Message = pi_rObj.m_Message;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPAGenericException::~HPAGenericException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
Utf8String HPAGenericException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HPAGeneric()).c_str(), m_Message.c_str());
    Utf8String exceptionName(ImagePPExceptions::HPAGeneric().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
Utf8StringCR HPAGenericException::GetMessage() const
    {
    return m_Message;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HPAGenericException::Clone() const
    {
    return new HPAGenericException(*this);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
Utf8String HPAException::MakeErrorMsg() const
    {
    HPRECONDITION(m_pOffendingNode != 0);

    ostringstream OutputMsg;

    if (m_pOffendingNode != 0)
        {
        if (m_pOffendingNode->GetEndPos().m_pURL != 0)
            OutputMsg << m_pOffendingNode ->GetEndPos().m_pURL->GetURL();
        else
            OutputMsg << "????";

        OutputMsg << " at line "
                  << m_pOffendingNode->GetEndPos().m_Line
                  << ", column "
                  << m_pOffendingNode->GetEndPos().m_Column
                  << ": "
                  << GetErrorText();
        }
    else
        OutputMsg << "Invalid program/script";
    return OutputMsg.str().c_str();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
Utf8String HPAException::GetErrorText() const 
    {
        if (dynamic_cast<HPANoTokenException const*>(this) != 0)
           return ImagePPMessages::GetString(ImagePPMessages::PSS_PrematureEndOfFile()); // Can't get token / Premature end of file found.
        else if  (dynamic_cast<HPARecursiveInclusionException const*>(this) != 0)
            return ImagePPMessages::GetString(ImagePPMessages::PSS_FileAlreadyIncluded()); // Can't get token / Premature end of file found.
        else
            {
            HASSERT(0);
            return "";
            }
    }
