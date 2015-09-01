//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAException
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HPANode.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


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
WString HPAException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_pOffendingNode->GetEndPos().m_Line,
                                 m_pOffendingNode->GetEndPos().m_Column);
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
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
                                         const WString&      pi_rMessage)
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
WString HPAGenericException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HPAGeneric()).c_str(), m_Message.c_str());
    WString exceptionName(ImagePPExceptions::HPAGeneric().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HPAGenericException::GetMessage() const
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
WString HPAException::MakeErrorMsg() const
    {
    HPRECONDITION(m_pOffendingNode != 0);

    wostringstream OutputMsg;

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
WString HPAException::GetErrorText() const 
    {
        if (dynamic_cast<HPANoTokenException const*>(this) != 0)
           return ImagePPMessages::GetStringW(ImagePPMessages::PSS_PrematureEndOfFile()); // Can't get token / Premature end of file found.
        else if  (dynamic_cast<HPARecursiveInclusionException const*>(this) != 0)
            return ImagePPMessages::GetStringW(ImagePPMessages::PSS_FileAlreadyIncluded()); // Can't get token / Premature end of file found.
        else
            {
            HASSERT(0);
            return L"";
            }
    }
