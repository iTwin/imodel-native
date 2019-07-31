//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPSTokenizer
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HPATokenizer.h>

BEGIN_IMAGEPP_NAMESPACE

class HPSParser;

class HPSTokenizer : public HPADefaultTokenizer
    {
public:

    HPSTokenizer(HPSParser* pi_pParser);
    virtual                 ~HPSTokenizer();

protected:

    virtual HPANode*        MakeNode(HPAToken* pi_pToken, const Utf8String& pi_rText,
                                     const HPASourcePos& pi_rLeft,
                                     const HPASourcePos& pi_rRight,
                                     HPASession* pi_pSession);

private:

    HPSParser*              m_pParser;
    };

END_IMAGEPP_NAMESPACE
