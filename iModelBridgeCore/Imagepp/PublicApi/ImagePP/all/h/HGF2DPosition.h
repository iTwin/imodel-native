//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DPosition
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DCoord.h"

// Moved to HGF2DCoord.h to remove circular reference because HGF2DCoord<DataType>::CalculateLengthTo requires 
// HGF2DDisplacement which requires HGF2DPosition.
//
// BEGIN_IMAGEPP_NAMESPACE
// 
// typedef HGF2DCoord<double> HGF2DPosition;
// 
// typedef std::vector<HGF2DCoord<double>, std::allocator<HGF2DCoord<double> > > HGF2DPositionCollection;
// 
// END_IMAGEPP_NAMESPACE

