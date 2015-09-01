/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HPSPssFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HPSPssToken.h>

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
    IMAGEPP_EXPORT virtual HSTATUS  CreatePSSFile(WString const& fileName);

protected:
private:
    typedef std::list<HFCPtr<HPSPssToken> > ListOfTokens;
    ListOfTokens m_list;
    };
END_IMAGEPP_NAMESPACE