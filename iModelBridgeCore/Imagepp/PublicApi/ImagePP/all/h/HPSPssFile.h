/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HPSPssFile.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HPSPssToken.h>



/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class HPSPssFile
    {
public:
    _HDLLg HPSPssFile();
    _HDLLg virtual ~HPSPssFile();

    _HDLLg virtual void     AddToken(HFCPtr<HPSPssToken> pToken);
    _HDLLg virtual size_t   CountToken() const;
    _HDLLg virtual HSTATUS  CreatePSSFile(WString const& fileName);

protected:
private:
    typedef std::list<HFCPtr<HPSPssToken> > ListOfTokens;
    ListOfTokens m_list;
    };
