//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPosition.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DPosition
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

//DM-Android #pragma once

#include "HGF2DCoord.h"
BEGIN_IMAGEPP_NAMESPACE
typedef HGF2DCoord<double> HGF2DPosition;
typedef vector<HGF2DCoord<double>, allocator<HGF2DCoord<double> > > HGF2DPositionCollection;
END_IMAGEPP_NAMESPACE

