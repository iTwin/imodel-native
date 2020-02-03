//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DPoint
//-----------------------------------------------------------------------------
// Position in three-dimension
//-----------------------------------------------------------------------------

#pragma once

#include "HGF3DCoord.h"
BEGIN_IMAGEPP_NAMESPACE
typedef HGF3DCoord<double> HGF3DPoint;
typedef vector<HGF3DCoord<double>, allocator<HGF3DCoord<double> > > HGF3DPointCollection;
// HPM_DECLARE_TYPE(HGF3DPoint);
END_IMAGEPP_NAMESPACE

