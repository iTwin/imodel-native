//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAModuleCodes.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPAModuleCodes.h>
#include <Imagepp/all/h/HPAModule.h>
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>
#include <Imagepp/all/h/HPANode.h>


//-----------------------------------------------------------------------------
// Implement the constructors, destructors and code IDs here.
//-----------------------------------------------------------------------------
HFC_ERRORCODE_IMPLEMENT_CLASS2(HPAExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HPAException::CLASS_ID)



//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HPAExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HPRECONDITION(pi_rException.GetID() == HPA_EXCEPTION);

    HPAExInfo* pExInfo = (HPAExInfo*)pi_rException.GetInfo();

    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(GetModuleID());

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the parameter
    if (pExInfo->m_pOffendingNode != 0)
        {
        HPRECONDITION(pExInfo->m_pOffendingNode->GetStartPos().m_pURL != 0);
        WString URL(pExInfo->m_pOffendingNode->GetStartPos().m_pURL->GetURL());

        HFCEncodeDecodeASCII::EscapeToASCII(URL);

        pCode->AddParameter(URL);
            {
            wostringstream ttt;
            ttt << pExInfo->m_pOffendingNode->GetStartPos().m_Line;
            pCode->AddParameter(ttt.str().c_str());
            }
            {
            wostringstream ttt;
            ttt << pExInfo->m_pOffendingNode->GetStartPos().m_Column;
            pCode->AddParameter(ttt.str().c_str());
            }
        }
    else
        {
        pCode->AddParameter(L"No URL");
        pCode->AddParameter(L"0");
        pCode->AddParameter(L"0");
        }

    HPAGenericException GenericException((HPAException*)&pi_rException);
    WString Message(GenericException.GetErrorText());

    HFCEncodeDecodeASCII::EscapeToASCII(Message);
    pCode->AddParameter(Message);

    // Set the message
    pCode->SetMessageText(L"HPAException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HPAExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 4);
    HPASourcePos SourcePos;
    SourcePos.m_Line   = 0;
    SourcePos.m_Column = 0;
    wostringstream Message;

    uint32_t ParamNum = 1;
    for (HFCErrorCodeParameters::const_iterator Itr = pi_rCode.GetParameters().begin();
         Itr != pi_rCode.GetParameters().end();
         ++Itr, ++ParamNum)
        {
        switch (ParamNum)
            {
            case 1:
                SourcePos.m_pURL = HFCURL::Instanciate(*Itr);
                break;

            case 2:
                SourcePos.m_Line = BeStringUtilities::Wtoi((*Itr).c_str());
                break;

            case 3:
                SourcePos.m_Column = BeStringUtilities::Wtoi((*Itr).c_str());
                break;

            case 4:
                Message << *Itr;
                break;
            }
        }

    // build the node
    HFCPtr<HPANode> pNode;
    pNode = new HPANode(0, SourcePos, SourcePos, 0);

    // Make a descendant of HPAException that will receive the message.
    throw HPAGenericException(pNode, Message.str().c_str());

    // never called
    // return (H_ERROR);
    }
