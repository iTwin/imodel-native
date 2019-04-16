//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

BEGIN_IMAGEPP_NAMESPACE
// Forward declarations
class HMGMessageSender;
class HRARaster;


////////////////////////////////
// HRAEffectiveShapeChangedMsg
////////////////////////////////

class HRAEffectiveShapeChangedMsg : public HGFGeometryChangedMsg
    {
    HDECLARE_CLASS_ID(HRAEffectiveShapeChangedMsgId, HGFGeometryChangedMsg)

public:
    HRAEffectiveShapeChangedMsg();
    HRAEffectiveShapeChangedMsg(const HRAEffectiveShapeChangedMsg& pi_rObj);
    HRAEffectiveShapeChangedMsg(const HVEShape& pi_rShape);
    IMAGEPP_EXPORT virtual ~HRAEffectiveShapeChangedMsg();

    const HVEShape& GetShape() const;

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;

private:

    HVEShape        m_Shape;
    };


///////////////////////////
// HRAContentChangedMsg
///////////////////////////

class HRAContentChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(HRAContentChangedMsgId, HMGAsynchronousMessage)

public:
    HRAContentChangedMsg();
    HRAContentChangedMsg(const HRAContentChangedMsg& pi_rObj);
    HRAContentChangedMsg(const HVEShape&   pi_rShape);
    IMAGEPP_EXPORT virtual ~HRAContentChangedMsg();

    const HVEShape& GetShape() const;

    IMAGEPP_EXPORT HMGMessage* Clone() const override;

private:

    HVEShape        m_Shape;
    };


///////////////////////////
// HRAProgressImageChangedMsg
///////////////////////////

class HRAProgressImageChangedMsg : public HRAContentChangedMsg
    {
    HDECLARE_CLASS_ID(HRAProgressImageChangedMsgId, HRAContentChangedMsg)

public:
    HRAProgressImageChangedMsg();
    HRAProgressImageChangedMsg(const HRAProgressImageChangedMsg& pi_rObj);
    HRAProgressImageChangedMsg(const HVEShape&   pi_rShape,
                               bool             pi_Ended = false);
    IMAGEPP_EXPORT virtual ~HRAProgressImageChangedMsg();

    bool           IsEnded() const;

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;

private:

    bool           m_Ended;
    };


///////////////////////////
// HRAPyramidRasterClosingMsg
///////////////////////////

class HRAPyramidRasterClosingMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(HRAPyramidRasterClosingMsgId, HMGSynchronousMessage)

public:
    HRAPyramidRasterClosingMsg();
    HRAPyramidRasterClosingMsg(const HRAPyramidRasterClosingMsg& pi_rObj);
    IMAGEPP_EXPORT virtual ~HRAPyramidRasterClosingMsg();

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;

private:
    };



///////////////////////////
// HRALookAheadMsg
///////////////////////////

class HRALookAheadMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(HRALookAheadMsgId, HMGSynchronousMessage)

public:
    HRALookAheadMsg();
    HRALookAheadMsg(const HRALookAheadMsg& pi_rObj);
    HRALookAheadMsg(const HGFTileIDList& pi_rTileIDList,
                    uint16_t      pi_Resolution,
                    uint32_t             pi_ConsumerID,
                    bool                pi_Async);
    HRALookAheadMsg(const HVEShape& pi_rShape,
                    uint16_t pi_Resolution,
                    uint32_t        pi_ConsumerID,
                    bool           pi_Async);
    IMAGEPP_EXPORT virtual ~HRALookAheadMsg();

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;

    bool           UseShape() const;
    const HGFTileIDList&
    GetTileIDList() const;
    const HVEShape& GetShape() const;
    uint16_t GetResolution() const;
    uint32_t        GetConsumerID() const;
    bool           IsAsynchronous() const;

private:

    HGFTileIDList   m_TileIDList;
    HVEShape        m_Shape;
    uint16_t m_Resolution;
    uint32_t        m_ConsumerID;
    bool           m_Asynchronous;
    };


///////////////////////////
// HRAModifiedTileNotSavedMsg
///////////////////////////
class HRAModifiedTileNotSavedMsg : public HMGSynchronousMessage
    {
    HDECLARE_CLASS_ID(HRAModifiedTileNotSavedMsgId, HMGSynchronousMessage)

public:
    HRAModifiedTileNotSavedMsg  ();
    HRAModifiedTileNotSavedMsg  (const HRAModifiedTileNotSavedMsg& pi_rObj);
    HRAModifiedTileNotSavedMsg  (uint16_t           pi_Resolution,
                                 uint64_t                 pi_TileIndex);
    IMAGEPP_EXPORT virtual ~HRAModifiedTileNotSavedMsg();

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;

    uint16_t GetResolution() const;
    uint64_t       GetTileIndex() const;

private:

    uint16_t m_Resolution;
    uint64_t       m_TileIndex;
    };
END_IMAGEPP_NAMESPACE


#include "HRAMessages.hpp"

