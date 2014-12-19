//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAMessages.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRA message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HRA.
//-----------------------------------------------------------------------------

#pragma once


#include "HGFMessages.h"
#include "HVEShape.h"
#include "HGFTileIDDescriptor.h"

// Forward declarations
class HMGMessageSender;
class HRARaster;


////////////////////////////////
// HRAEffectiveShapeChangedMsg
////////////////////////////////

class HRAEffectiveShapeChangedMsg : public HGFGeometryChangedMsg
    {
    HDECLARE_CLASS_ID(1095, HGFGeometryChangedMsg)

public:
    HRAEffectiveShapeChangedMsg();
    HRAEffectiveShapeChangedMsg(const HRAEffectiveShapeChangedMsg& pi_rObj);
    HRAEffectiveShapeChangedMsg(const HVEShape& pi_rShape);
    _HDLLg virtual ~HRAEffectiveShapeChangedMsg();

    const HVEShape& GetShape() const;

    _HDLLg virtual HMGMessage* Clone() const override;

private:

    HVEShape        m_Shape;
    };


///////////////////////////
// HRAContentChangedMsg
///////////////////////////

class HRAContentChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(1096, HMGAsynchronousMessage)

public:
    HRAContentChangedMsg();
    HRAContentChangedMsg(const HRAContentChangedMsg& pi_rObj);
    HRAContentChangedMsg(const HVEShape&   pi_rShape);
    _HDLLg virtual ~HRAContentChangedMsg();

    const HVEShape& GetShape() const;

    _HDLLg virtual HMGMessage* Clone() const;

private:

    HVEShape        m_Shape;
    };


///////////////////////////
// HRAProgressImageChangedMsg
///////////////////////////

class HRAProgressImageChangedMsg : public HRAContentChangedMsg
    {
    HDECLARE_CLASS_ID(1088, HRAContentChangedMsg)

public:
    HRAProgressImageChangedMsg();
    HRAProgressImageChangedMsg(const HRAProgressImageChangedMsg& pi_rObj);
    HRAProgressImageChangedMsg(const HVEShape&   pi_rShape,
                               bool             pi_Ended = false);
    _HDLLg virtual ~HRAProgressImageChangedMsg();

    bool           IsEnded() const;

    _HDLLg virtual HMGMessage* Clone() const override;

private:

    bool           m_Ended;
    };


///////////////////////////
// HRAPyramidRasterClosingMsg
///////////////////////////

class HRAPyramidRasterClosingMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(1087, HMGSynchronousMessage)

public:
    HRAPyramidRasterClosingMsg();
    HRAPyramidRasterClosingMsg(const HRAPyramidRasterClosingMsg& pi_rObj);
    _HDLLg virtual ~HRAPyramidRasterClosingMsg();

    _HDLLg virtual HMGMessage* Clone() const override;

private:
    };



///////////////////////////
// HRALookAheadMsg
///////////////////////////

class HRALookAheadMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(1234, HMGSynchronousMessage)

public:
    HRALookAheadMsg();
    HRALookAheadMsg(const HRALookAheadMsg& pi_rObj);
    HRALookAheadMsg(const HGFTileIDList& pi_rTileIDList,
                    unsigned short      pi_Resolution,
                    uint32_t             pi_ConsumerID,
                    bool                pi_Async);
    HRALookAheadMsg(const HVEShape& pi_rShape,
                    unsigned short pi_Resolution,
                    uint32_t        pi_ConsumerID,
                    bool           pi_Async);
    _HDLLg virtual ~HRALookAheadMsg();

    _HDLLg virtual HMGMessage* Clone() const override;

    bool           UseShape() const;
    const HGFTileIDList&
    GetTileIDList() const;
    const HVEShape& GetShape() const;
    unsigned short GetResolution() const;
    uint32_t        GetConsumerID() const;
    bool           IsAsynchronous() const;

private:

    HGFTileIDList   m_TileIDList;
    HVEShape        m_Shape;
    unsigned short m_Resolution;
    uint32_t        m_ConsumerID;
    bool           m_Asynchronous;
    };


///////////////////////////
// HRAModifiedTileNotSavedMsg
///////////////////////////
class HRAModifiedTileNotSavedMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(1364, HMGSynchronousMessage)

public:
    HRAModifiedTileNotSavedMsg  ();
    HRAModifiedTileNotSavedMsg  (const HRAModifiedTileNotSavedMsg& pi_rObj);
    HRAModifiedTileNotSavedMsg  (unsigned short           pi_Resolution,
                                 uint64_t                 pi_TileIndex);
    _HDLLg virtual ~HRAModifiedTileNotSavedMsg();

    _HDLLg virtual HMGMessage* Clone() const override;

    unsigned short GetResolution() const;
    uint64_t       GetTileIndex() const;

private:

    unsigned short m_Resolution;
    uint64_t       m_TileIndex;
    };


#include "HRAMessages.hpp"

