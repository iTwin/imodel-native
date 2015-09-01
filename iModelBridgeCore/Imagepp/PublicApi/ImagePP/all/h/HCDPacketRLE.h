//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDPacketRLE.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDPacketRLE
//-----------------------------------------------------------------------------
// Class used by the codec.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HCDCodec;
class HCDCodecHMRRLE1;

// ----------------------------------------------------------------------------
//  HCDPacketRLE
// ----------------------------------------------------------------------------
class HCDPacketRLE : public HFCShareableObject<HCDPacketRLE>
    {
    HDECLARE_BASECLASS_ID(HCDPacketRLEId_Base)

public:

    class RLEScanlineGenerator
        {
    public:
        RLEScanlineGenerator    (const unsigned short* pi_pRun,
                                 HUINTX         pi_Width,
                                 bool          pi_State);
        virtual ~RLEScanlineGenerator   ();

        HUINTX GetFirstScanline     (HUINTX* po_pPosX);
        HUINTX GetNextScanline      (HUINTX* po_pPosX);
        HUINTX GetCurrentScanline   (HUINTX* po_pPosX);

    private:
        const unsigned short*  m_pRun;
        bool           m_State;
        HUINTX          m_PixelCount;
        size_t          m_Index;
        HUINTX          m_Width;
        };

    HCDPacketRLE();
    IMAGEPP_EXPORT HCDPacketRLE(uint32_t pi_WidthPixels, uint32_t pi_HeightPixels);
    IMAGEPP_EXPORT virtual          ~HCDPacketRLE();

    virtual HCDPacketRLE*   Clone() const;

    size_t GetBufferSize() const;

    size_t GetDataSize() const;

    size_t GetLineBufferSize(HUINTX pi_PosY) const;

    size_t GetLineDataSize(HUINTX pi_PosY) const;

    Byte* GetLineBuffer(HUINTX pi_PosY) const;

    IMAGEPP_EXPORT void             SetLineBuffer   (HUINTX         pi_PosY,            // replace the internal line buffer
                                             Byte*        pi_pBuffer,         // by pi_pBuffer
                                             size_t         pi_BufferSize,
                                             size_t         pi_DataSize);

    IMAGEPP_EXPORT void             SetLineData     (HUINTX         pi_PosY,            // change data into the internal line buffer
                                             const Byte*  pi_pData,
                                             size_t         pi_DataSize);
    IMAGEPP_EXPORT void             SetLineDataSize (HUINTX         pi_PosY,
                                             size_t         pi_DataSize);
    IMAGEPP_EXPORT void             ClearAll();

    // memory management
    bool                   HasBufferOwnership() const;
    IMAGEPP_EXPORT void             SetBufferOwnership(bool pi_Owner);

    IMAGEPP_EXPORT HFCPtr<HCDCodecHMRRLE1> const&
    GetCodec() const;


    RLEScanlineGenerator*   GetRLEScanlineGenerator (HUINTX  pi_PosY,
                                                     bool   pi_State) const;


private:
    // Disabled for now.
    HCDPacketRLE(const HCDPacketRLE& pi_rPacket);
    HCDPacketRLE&    operator=(const HCDPacketRLE& pi_rPacket);

    void                    FreeBuffers();

    bool                   ValidateLineIntegrity(HUINTX pi_NoLine) const;
    bool                   ValidateDataIntegrity() const;

    struct RunBuffer
        {
        Byte*     m_pBuffer;
        size_t      m_BufferSize;
        size_t      m_DataSize;
        };

    // For optimization purpose.
    size_t          m_TotalBufferSize;
    size_t          m_TotalDataSize;

    HFCPtr<HCDCodecHMRRLE1> m_pCodec;

    bool                   m_BufferOwner;

    vector<RunBuffer>       m_RunBuffers;
    };

END_IMAGEPP_NAMESPACE

#include "HCDPacketRLE.hpp"

