//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAMessages.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HRA message classes
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRARaster.h>      // Got to...

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAEffectiveShapeChangedMsg::~HRAEffectiveShapeChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRAEffectiveShapeChangedMsg::Clone() const
    {
    return new HRAEffectiveShapeChangedMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAContentChangedMsg::~HRAContentChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRAContentChangedMsg::Clone() const
    {
    return new HRAContentChangedMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAProgressImageChangedMsg::~HRAProgressImageChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRAProgressImageChangedMsg::Clone() const
    {
    return new HRAProgressImageChangedMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAPyramidRasterClosingMsg::~HRAPyramidRasterClosingMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRAPyramidRasterClosingMsg::Clone() const
    {
    return new HRAPyramidRasterClosingMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRALookAheadMsg::~HRALookAheadMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRALookAheadMsg::Clone() const
    {
    return new HRALookAheadMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAModifiedTileNotSavedMsg::~HRAModifiedTileNotSavedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRAModifiedTileNotSavedMsg::Clone() const
    {
    return new HRAModifiedTileNotSavedMsg(*this);
    }

