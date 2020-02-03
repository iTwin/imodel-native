/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ImageppInternal.h>

#include <ImagePP/all/h/HPSPssFile.h>
#include <ImagePP/all/h/HPSPssToken.h>


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
HSTATUS HPSPssFile::CreatePSSFile(Utf8String const& fileName)
    {
    WString filenameW(fileName.c_str(), BentleyCharEncoding::Utf8);
    AString localeStr(filenameW.c_str());
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

