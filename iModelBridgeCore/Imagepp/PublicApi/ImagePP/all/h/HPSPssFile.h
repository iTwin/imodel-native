/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HPSPssToken.h>

BEGIN_IMAGEPP_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class HPSPssFile
    {
public:
    IMAGEPP_EXPORT HPSPssFile();
    IMAGEPP_EXPORT virtual ~HPSPssFile();

    IMAGEPP_EXPORT virtual void     AddToken(HFCPtr<HPSPssToken> pToken);
    IMAGEPP_EXPORT virtual size_t   CountToken() const;
    IMAGEPP_EXPORT virtual HSTATUS  CreatePSSFile(Utf8String const& fileName);

protected:
private:
    typedef std::list<HFCPtr<HPSPssToken> > ListOfTokens;
    ListOfTokens m_list;
    };
END_IMAGEPP_NAMESPACE
