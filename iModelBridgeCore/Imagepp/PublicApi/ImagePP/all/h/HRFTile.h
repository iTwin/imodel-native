//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFTile
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCEvent.h"
#include <ImagePP/all/h/HFCException.h>

BEGIN_IMAGEPP_NAMESPACE
class HRFTile : public HFCShareableObject<HRFTile>,
    public HFCExclusiveKey,
    public HFCEvent
    {
public:
    //--------------------------------------
    // Contruction/Destruction
    //--------------------------------------

    HRFTile(uint64_t pi_ID,
            uint64_t pi_Index,
            uint64_t pi_PosX,
            uint64_t pi_PosY,
            unsigned short pi_Resolution);
    virtual         ~HRFTile();


    //--------------------------------------
    // Tile Methods
    //--------------------------------------

    // Tile positionnig info
    uint64_t           GetID() const;
    uint64_t           GetIndex() const;
    uint64_t           GetPosX() const;
    uint64_t           GetPosY() const;
    unsigned short     GetResolution() const;

    // Validity of the tile
    bool               IsValid() const;

    // Tile Data Methods.
    size_t              GetDataSize() const;
    const Byte*        GetData() const;
    void                SetData(HArrayAutoPtr<Byte>& pi_rpData,    // ownership will be transfered
                                size_t                pi_DataSize);
    void                SetData(const Byte*          pi_pData,    // ownership will be transfered
                                size_t                pi_DataSize);

private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Tile positioning info
    uint64_t               m_ID;
    uint64_t               m_Index;
    uint64_t               m_PosX;
    uint64_t               m_PosY;
    unsigned short         m_Resolution;

    // Validity of the tile
    bool                  m_Valid;

    // Tile Data
    HArrayAutoPtr<Byte>    m_pData;
    size_t                  m_DataSize;
    };
END_IMAGEPP_NAMESPACE

#include "HRFTile.hpp"
