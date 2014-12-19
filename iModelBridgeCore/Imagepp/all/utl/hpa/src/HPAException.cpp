//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAException.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAException
//---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HPANode.h>

#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPAException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPAGenericException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException(HFCPtr<HPANode>&    pi_rpOffendingNode)
    : HFCException(HPA_EXCEPTION)
    {
    m_pInfo = new HPAExInfo;
    ((HPAExInfo*)m_pInfo)->m_pOffendingNode = pi_rpOffendingNode;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException(ExceptionID        pi_ExceptionID,
                           bool               pi_CreateInfoStruct)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION(((pi_ExceptionID >= HPA_BASE) && (pi_ExceptionID < HPA_SEPARATOR_ID)) ||
                  ((pi_ExceptionID >= HPS_BASE) && (pi_ExceptionID < HPS_SEPARATOR_ID)));

    if (pi_CreateInfoStruct)
        {
        m_pInfo = new HPAExInfo;
        ((HPAExInfo*)m_pInfo)->m_pOffendingNode = HFCPtr<HPANode>();

        if (GetID() != HPA_CANNOT_RESOLVE_START_RULE_EXCEPTION)
            {
            GENERATE_FORMATTED_EXCEPTION_MSG()
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException(ExceptionID        pi_ExceptionID,
                           HFCPtr<HPANode>&   pi_rpOffendingNode,
                           bool               pi_CreateInfoStruct)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION(((pi_ExceptionID >= HPA_BASE) && (pi_ExceptionID < HPA_SEPARATOR_ID)) ||
                  ((pi_ExceptionID >= HPS_BASE) && (pi_ExceptionID < HPS_SEPARATOR_ID)));

    if (pi_CreateInfoStruct)
        {
        m_pInfo = new HPAExInfo;
        ((HPAExInfo*)m_pInfo)->m_pOffendingNode = pi_rpOffendingNode;

        if (GetID() != HPA_CANNOT_RESOLVE_START_RULE_EXCEPTION)
            {
            GENERATE_FORMATTED_EXCEPTION_MSG()
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPAException::HPAException(const HPAException& pi_rObj)
    : HFCException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPAExInfo)
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
void HPAException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);

    if (GetID() != HPA_CANNOT_RESOLVE_START_RULE_EXCEPTION)
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage,
                             ((HPAExInfo*)m_pInfo)->m_pOffendingNode->GetEndPos().m_Line,
                             ((HPAExInfo*)m_pInfo)->m_pOffendingNode->GetEndPos().m_Column)
        }
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPAException& HPAException::operator=(const HPAException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPAExInfo)
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPAGenericException::HPAGenericException(HFCPtr<HPANode>&    pi_rpOffendingNode,
                                         const WString&      pi_rMessage)
    : HPAException(HPA_GENERIC_EXCEPTION, pi_rpOffendingNode, false)
    {
    m_pInfo = new HPAGenericExInfo;
    ((HPAGenericExInfo*)m_pInfo)->m_pOffendingNode = pi_rpOffendingNode;
    ((HPAGenericExInfo*)m_pInfo)->m_Message = pi_rMessage.c_str();

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }



//---------------------------------------------------------------------------
// This constructor extracts the needed information from a HPAException
//---------------------------------------------------------------------------
HPAGenericException::HPAGenericException(const HPAException* pi_pObj)
    : HPAException(HPA_GENERIC_EXCEPTION, false)
    {
    HPRECONDITION(pi_pObj != 0);

    HPAExInfo* pExInfo = (HPAExInfo*)pi_pObj->GetInfo();

    m_pInfo = new HPAGenericExInfo();
    ((HPAGenericExInfo*)m_pInfo)->m_pOffendingNode = pExInfo->m_pOffendingNode;
    ((HPAGenericExInfo*)m_pInfo)->m_Message = pi_pObj->GetExceptionMessage();

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPAGenericException::HPAGenericException(const HPAGenericException& pi_rObj)
    : HPAException(HPA_GENERIC_EXCEPTION, false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPAGenericExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
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
void HPAGenericException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HPAGenericExInfo*)m_pInfo)->m_Message.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPAGenericException& HPAGenericException::operator=(const HPAGenericException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPAGenericExInfo)
        }

    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
WString HPAException::MakeErrorMsg() const
    {
    HPRECONDITION(m_pInfo != 0);
    HPRECONDITION(((HPAExInfo*)m_pInfo)->m_pOffendingNode != 0);

    wostringstream OutputMsg;

    HPAExInfo* pInfo = (HPAExInfo*)m_pInfo;

    if (pInfo->m_pOffendingNode != 0)
        {
        if (pInfo->m_pOffendingNode->GetEndPos().m_pURL != 0)
            OutputMsg << pInfo->m_pOffendingNode ->GetEndPos().m_pURL->GetURL();
        else
            OutputMsg << "????";

        OutputMsg << " at line "
                  << pInfo->m_pOffendingNode->GetEndPos().m_Line
                  << ", column "
                  << pInfo->m_pOffendingNode->GetEndPos().m_Column
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
    switch (GetID())
        {
        case HPA_EXCEPTION :
            return HFCResourceLoader::GetInstance()->GetString(IDS_PSS_SyntaxError);  //Syntax error.
        case HPA_NO_TOKEN_EXCEPTION :
            return HFCResourceLoader::GetInstance()->GetString(IDS_PSS_PrematureEndOfFile); // Can't get token / Premature end of file found.
        case HPA_RECURSIVE_INCLUSION_EXCEPTION :
            return HFCResourceLoader::GetInstance()->GetString(IDS_PSS_FileAlreadyIncluded); // Can't get token / Premature end of file found.
        default :
            HASSERT(0);
            return L"";
        }
    }