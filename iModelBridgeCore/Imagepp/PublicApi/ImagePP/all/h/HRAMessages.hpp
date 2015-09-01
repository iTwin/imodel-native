//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAMessages.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRA message classes
//-----------------------------------------------------------------------------
// Inline methods for Message classes used in HRA.
//-----------------------------------------------------------------------------


////////////////////////////////
// HRAEffectiveShapeChangedMsg
////////////////////////////////

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAEffectiveShapeChangedMsg::HRAEffectiveShapeChangedMsg()
    : HGFGeometryChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAEffectiveShapeChangedMsg::HRAEffectiveShapeChangedMsg(const HRAEffectiveShapeChangedMsg& pi_rObj)
    : HGFGeometryChangedMsg(pi_rObj),
      m_Shape(pi_rObj.m_Shape)
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAEffectiveShapeChangedMsg::HRAEffectiveShapeChangedMsg(const HVEShape& pi_rShape)
    : HGFGeometryChangedMsg(),
      m_Shape(pi_rShape)
    {
    }


//-----------------------------------------------------------------------------
// Get the message's shape
//-----------------------------------------------------------------------------
inline const HVEShape& HRAEffectiveShapeChangedMsg::GetShape() const
    {
    return m_Shape;
    }


///////////////////////////
// HRAContentChangedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAContentChangedMsg::HRAContentChangedMsg()
    : HMGAsynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAContentChangedMsg::HRAContentChangedMsg(const HRAContentChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj),
      m_Shape(pi_rObj.m_Shape)
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAContentChangedMsg::HRAContentChangedMsg(const HVEShape& pi_rShape)
    : HMGAsynchronousMessage(),
      m_Shape(pi_rShape)
    {
    }


//-----------------------------------------------------------------------------
// Get the message's shape
//-----------------------------------------------------------------------------
inline const HVEShape& HRAContentChangedMsg::GetShape() const
    {
    return m_Shape;
    }


///////////////////////////
// HRAProgressImageChangedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAProgressImageChangedMsg::HRAProgressImageChangedMsg()
    : HRAContentChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAProgressImageChangedMsg::HRAProgressImageChangedMsg(const HRAProgressImageChangedMsg& pi_rObj)
    : HRAContentChangedMsg(pi_rObj)
    {
    m_Ended = pi_rObj.m_Ended;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAProgressImageChangedMsg::HRAProgressImageChangedMsg(const HVEShape&   pi_rShape,
                                                              bool             pi_Ended)
    : HRAContentChangedMsg(pi_rShape)
    {
    m_Ended = pi_Ended;
    }


//-----------------------------------------------------------------------------
// Indicates if the total request as ended
//-----------------------------------------------------------------------------
inline bool HRAProgressImageChangedMsg::IsEnded() const
    {
    return (m_Ended);
    }


///////////////////////////
// HRAPyramidRasterClosingMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAPyramidRasterClosingMsg::HRAPyramidRasterClosingMsg()
    : HMGSynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAPyramidRasterClosingMsg::HRAPyramidRasterClosingMsg(const HRAPyramidRasterClosingMsg& pi_rObj)
    : HMGSynchronousMessage(pi_rObj)
    {
    }


///////////////////////////
// HRALookAheadMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRALookAheadMsg::HRALookAheadMsg()
    : HMGSynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRALookAheadMsg::HRALookAheadMsg(const HRALookAheadMsg& pi_rObj)
    : HMGSynchronousMessage(pi_rObj),
      m_TileIDList(pi_rObj.m_TileIDList),
      m_Shape(pi_rObj.m_Shape)
    {
    m_ConsumerID   = pi_rObj.m_ConsumerID;
    m_Asynchronous = pi_rObj.m_Asynchronous;
    m_Resolution   = pi_rObj.m_Resolution;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRALookAheadMsg::HRALookAheadMsg(const HGFTileIDList& pi_rTileIDList,
                                        unsigned short      pi_Resolution,
                                        uint32_t             pi_ConsumerID,
                                        bool                pi_Async)
    : m_TileIDList(pi_rTileIDList)
    {
    HPRECONDITION(!pi_rTileIDList.empty());

    m_ConsumerID   = pi_ConsumerID;
    m_Asynchronous = pi_Async;
    m_Resolution   = pi_Resolution;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRALookAheadMsg::HRALookAheadMsg(const HVEShape& pi_rShape,
                                        unsigned short pi_Resolution,
                                        uint32_t        pi_ConsumerID,
                                        bool           pi_Async)
    : m_Shape(pi_rShape)
    {
    HPRECONDITION(!pi_rShape.IsEmpty());

    m_ConsumerID   = pi_ConsumerID;
    m_Resolution   = pi_Resolution;
    m_Asynchronous = pi_Async;
    }


//-----------------------------------------------------------------------------
// Indicates if the look ahead is shape based or tile id list based
//-----------------------------------------------------------------------------
inline bool HRALookAheadMsg::UseShape() const
    {
    return (m_TileIDList.empty());
    }


//-----------------------------------------------------------------------------
// get the extent in the message
//-----------------------------------------------------------------------------
inline const HGFTileIDList& HRALookAheadMsg::GetTileIDList() const
    {
    HPRECONDITION(!UseShape());

    return (m_TileIDList);
    }


//-----------------------------------------------------------------------------
// get the extent in the message
//-----------------------------------------------------------------------------
inline const HVEShape& HRALookAheadMsg::GetShape() const
    {
    HPRECONDITION(UseShape());

    return (m_Shape);
    }


//-----------------------------------------------------------------------------
// Get the resolution in the message
//-----------------------------------------------------------------------------
inline unsigned short HRALookAheadMsg::GetResolution() const
    {
    HPRECONDITION(UseShape());

    return (m_Resolution);
    }


//-----------------------------------------------------------------------------
// Get the Consumer ID in the message
//-----------------------------------------------------------------------------
inline uint32_t HRALookAheadMsg::GetConsumerID() const
    {
    return (m_ConsumerID);
    }


//-----------------------------------------------------------------------------
// Get the asynchronous state of the LookAhead Handling
//-----------------------------------------------------------------------------
inline bool HRALookAheadMsg::IsAsynchronous() const
    {
    return (m_Asynchronous);
    }

///////////////////////////
// HRAModifiedTileNotSavedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAModifiedTileNotSavedMsg::HRAModifiedTileNotSavedMsg()
    : HMGSynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HRAModifiedTileNotSavedMsg::HRAModifiedTileNotSavedMsg(const HRAModifiedTileNotSavedMsg& pi_rObj)
    : HMGSynchronousMessage(pi_rObj),
      m_Resolution(pi_rObj.m_Resolution),
      m_TileIndex(pi_rObj.m_TileIndex)
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HRAModifiedTileNotSavedMsg::HRAModifiedTileNotSavedMsg(unsigned short pi_Resolution,
                                                              uint64_t pi_TileIndex)
    : m_Resolution(pi_Resolution),
      m_TileIndex(pi_TileIndex)
    {
    }



//-----------------------------------------------------------------------------
// get the resolution in the message
//-----------------------------------------------------------------------------
inline unsigned short HRAModifiedTileNotSavedMsg::GetResolution() const
    {
    return m_Resolution;
    }


//-----------------------------------------------------------------------------
// get the tile index in the message
//-----------------------------------------------------------------------------
inline uint64_t HRAModifiedTileNotSavedMsg::GetTileIndex() const
    {
    return m_TileIndex;
    }

END_IMAGEPP_NAMESPACE
