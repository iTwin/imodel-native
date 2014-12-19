//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPASession.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPASession
//---------------------------------------------------------------------------
// Ancestor class for parsing context descriptors
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCPtr.h>

class HPAParser;
class HFCBinStream;

class HPASession : public HFCShareableObject<HPASession>
    {
public:
    HPASession(HFCBinStream* pi_pStream);
    _HDLLu virtual             ~HPASession();

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

#include "HPASession.hpp"
