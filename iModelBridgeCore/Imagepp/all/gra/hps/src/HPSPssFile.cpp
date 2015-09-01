/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hps/src/HPSPssFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPSPssFile.h>
#include <Imagepp/all/h/HPSPssToken.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HPSPssFile::HPSPssFile()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HPSPssFile::~HPSPssFile()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void HPSPssFile::AddToken(HFCPtr<HPSPssToken> pToken)
    {
    m_list.push_back(pToken);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HPSPssFile::CountToken() const
    {
    return m_list.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HSTATUS HPSPssFile::CreatePSSFile(WString const& fileName)
    {
    AString localeStr;
    BeStringUtilities::WCharToCurrentLocaleChar(localeStr, fileName.c_str());
    ofstream os(localeStr.c_str());

    if (!os.is_open())
        return H_ERROR;

    ListOfTokens::iterator itr (m_list.begin());
    while(itr != m_list.end())
        {
        os << *(*itr) << endl;
        ++itr;
        }

    return H_SUCCESS;
    }

