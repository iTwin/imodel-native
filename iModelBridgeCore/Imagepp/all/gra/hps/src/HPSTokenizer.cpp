//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSTokenizer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPSTokenizer
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPSTokenizer.h>
#include <Imagepp/all/h/HPSParser.h>
#include "HPSParserScope.h"
#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>

//---------------------------------------------------------------------------
HPSTokenizer::HPSTokenizer(HPSParser* pi_pParser)
    : HPADefaultTokenizer(false),
      m_pParser(pi_pParser)
    {
    }


//---------------------------------------------------------------------------
HPSTokenizer::~HPSTokenizer()
    {
    }


//---------------------------------------------------------------------------
HPANode* HPSTokenizer::MakeNode(HPAToken* pi_pToken, const WString& pi_rText,
                                const HPASourcePos& pi_rLeft,
                                const HPASourcePos& pi_rRight,
                                HPASession* pi_pSession)
    {
    HPANode* pNode = 0;
    if (pi_pToken == &m_pParser->Identifier_tk)
        {
        StatementDefinitionNode* pFound = ((HPSSession*)pi_pSession)->GetCurrentScope()->FindStatement(pi_rText);
        if (pFound)
            pNode = new StatementTokenNode(&m_pParser->StatementIdentifier_tk, pFound, pi_rLeft, pi_rRight, (HPSSession*)pi_pSession);
        else {
            VariableTokenNode* pFound = ((HPSSession*)pi_pSession)->GetCurrentScope()->FindVariable(pi_rText);
            if (pFound)
                pNode = new VariableRefNode(pFound->GetGrammarObject(), pFound, pi_rLeft, pi_rRight, pi_pSession);
            }
        }
    else if (pi_pToken == &m_pParser->INCLUDE_tk)
        {
        WString FileName;
        HFCPtr<HPANode> pStringNode = (HPATokenNode*)GetToken();

        if (pStringNode->GetGrammarObject() == &m_pParser->String_tk)
            FileName = ((HFCPtr<HPATokenNode>&)pStringNode)->GetText();
        else if (pStringNode->GetGrammarObject() == &m_pParser->VariableIdentifier_tk)
            {
            ((HFCPtr<VariableRefNode>&)pStringNode)->Calculate();
            if (((HFCPtr<VariableRefNode>&)pStringNode)->m_Type != HPSValueNode::STRING)
                throw HPSTypeMismatchException(pStringNode, HPSTypeMismatchException::STRING);
            FileName = *(((HFCPtr<VariableRefNode>&)pStringNode)->m_Value.m_pString);
            }
        else
            throw HPSExpressionExpectedException(pStringNode);

        HFCPtr<HFCURL> pURL = HFCURL::Instanciate(FileName);  // is it a URL?
        if (pURL == 0) // no...  is it a full path name having drive spec?
            {
            if ((FileName.size() > 2) &&   // yes, do the url accordingly
                ((FileName[1] == L':') || (FileName.substr(0, 2) == L"\\\\") || (FileName.substr(0,2) == L"//")))
                pURL = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + FileName);
            else
                pURL = ((HPSSession*)pi_pSession)->GetURL()->MakeURLTo(FileName);  // no, it is a relative path
            }
        if (pURL == 0)
            throw HPSInvalidUrlException( HFCPtr<HPANode>(HPADefaultTokenizer::MakeNode(pi_pToken, pi_rText, pi_rLeft, pi_rRight, pi_pSession)));

        HFCBinStream* pBinStream = 0;
        try
            {
            try
                {
                pBinStream = HFCBinStream::Instanciate(pURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 0, true);
                }
            catch(HFCFileException& )
                {
                throw HPSIncludeNotFoundException(HFCPtr<HPANode>(HPADefaultTokenizer::MakeNode(pi_pToken,
                                                                                 pi_rText,
                                                                                 pi_rLeft,
                                                                                 pi_rRight,
                                                                                 pi_pSession)));
                }

            if (!m_pParser->GetTokenizer()->Include(pBinStream))
                {
                throw HPARecursiveInclusionException(pStringNode);
                }

            pNode = GetToken();
            }
        catch (...)
            {
            if (pBinStream != 0)
                {
                delete pBinStream;
                }

            throw;
            }
        }
    if (!pNode)
        pNode = HPADefaultTokenizer::MakeNode(pi_pToken, pi_rText, pi_rLeft, pi_rRight, pi_pSession);
    return pNode;
    }

