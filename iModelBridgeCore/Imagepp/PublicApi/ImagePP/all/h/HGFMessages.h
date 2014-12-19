//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMessages.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HGF.
//-----------------------------------------------------------------------------

#pragma once



#include "HMGMessage.h"

// Forward declarations
class HMGMessageSender;


///////////////////////////
// HGGeometryChangedMsg
///////////////////////////

class HGFGeometryChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(1094, HMGAsynchronousMessage)

public:
    HGFGeometryChangedMsg();
    _HDLLg virtual ~HGFGeometryChangedMsg();

    _HDLLg virtual HMGMessage* Clone() const override;
protected:
    HGFGeometryChangedMsg(const HGFGeometryChangedMsg& pi_rObj);
    HGFGeometryChangedMsg& operator=(const HGFGeometryChangedMsg& pi_rObj);
    };


///////////////////////////
// HGFBufferContentChangedMsg
///////////////////////////

class HGFBufferContentChangedMsg: public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(1195, HMGAsynchronousMessage)

public:
    HGFBufferContentChangedMsg();
    HGFBufferContentChangedMsg(uint32_t pi_TileIndex, bool  pi_IsEmpty, bool pi_IsOverwritten);
    _HDLLg virtual ~HGFBufferContentChangedMsg();

    // the Tile index
    uint32_t        GetTileIndex() const;

    // Emptiness state of the tile
    bool           IsEmpty() const;
    // if the tile data was overwritten at least once
    bool           IsOverwritten() const;

    _HDLLg virtual HMGMessage* Clone() const override;

protected:
    HGFBufferContentChangedMsg(const HGFBufferContentChangedMsg& pi_rObj);
    HGFBufferContentChangedMsg& operator=(const HGFBufferContentChangedMsg& pi_rObj);

private:
    bool  m_IsOverwritten;
    bool  m_IsEmpty;
    uint32_t m_TileIndex;
    };


#include "HGFMessages.hpp"

