//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DPoint.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DPoint
//-----------------------------------------------------------------------------
// Position in three-dimension
//-----------------------------------------------------------------------------

#pragma once

#include "HGF3DCoord.h"
typedef HGF3DCoord<double> HGF3DPoint;
typedef vector<HGF3DCoord<double>, allocator<HGF3DCoord<double> > > HGF3DPointCollection;
// HPM_DECLARE_TYPE(HGF3DPoint);

