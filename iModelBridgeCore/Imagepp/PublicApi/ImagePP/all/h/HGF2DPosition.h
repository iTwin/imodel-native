//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPosition.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DPosition
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

#pragma once


#include "HGF2DCoord.h"
typedef HGF2DCoord<double> HGF2DPosition;
typedef vector<HGF2DCoord<double>, allocator<HGF2DCoord<double> > > HGF2DPositionCollection;

