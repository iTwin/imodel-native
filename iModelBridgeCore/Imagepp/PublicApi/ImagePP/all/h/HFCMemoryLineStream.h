//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryLineStream.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMemoryLineStream
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#pragma once

#include "HFCBuffer.h"
#include "HFCMemoryBinStream.h"
BEGIN_IMAGEPP_NAMESPACE
class HFCMemoryLineStream : protected HFCMemoryBinStream
    {
public:

    // Primary methods
    IMAGEPP_EXPORT                  HFCMemoryLineStream(const WString&     pi_Filename,
                                                char              pi_LineDelimiter,
                                                HFCPtr<HFCBuffer>& pi_rpBuffer);

    IMAGEPP_EXPORT virtual          ~HFCMemoryLineStream();

    // Information methods
    HFCPtr<HFCURL>          GetURL() const;

    // Content access
    IMAGEPP_EXPORT virtual size_t   ReadLine(uint32_t pi_LineNb,
                                     WString& po_rLine);

    //Get number of lines
    IMAGEPP_EXPORT uint32_t         GetNbLines() const;

    //Get the position of the line in the file
    uint64_t               GetLinePos(uint32_t pi_LineN) const;

private:
    // Disabled methods
    HFCMemoryLineStream(const HFCMemoryLineStream& pi_rObj);
    HFCMemoryLineStream& operator=(const HFCMemoryLineStream& pi_rObj);

    friend struct MemoryBinStreamCreator;

    typedef vector<uint64_t> MemAddresses;
    // Data members
    HAutoPtr<MemAddresses> m_pLineStartingAddresses;
    };
END_IMAGEPP_NAMESPACE
#include "HFCMemoryLineStream.hpp"