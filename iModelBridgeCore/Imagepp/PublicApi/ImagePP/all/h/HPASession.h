//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPASession
//---------------------------------------------------------------------------
// Ancestor class for parsing context descriptors
//---------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>

BEGIN_IMAGEPP_NAMESPACE
class HPAParser;
class HFCBinStream;

class HPASession : public HFCShareableObject<HPASession>
    {
public:
    HPASession(HFCBinStream* pi_pStream);
    IMAGEPP_EXPORT virtual             ~HPASession();

    void                SetParser(HPAParser* pi_pParser);
    HPAParser*          GetParser() const;
    HFCBinStream*       GetSrcStream() const;

protected:

private:

    // Disabled methods

    HPASession(const HPASession&);
    HPASession& operator=(const HPASession&);

    // Data

    HPAParser*          m_pParser;
    HFCBinStream*       m_pStream;
    };

END_IMAGEPP_NAMESPACE
#include "HPASession.hpp"
