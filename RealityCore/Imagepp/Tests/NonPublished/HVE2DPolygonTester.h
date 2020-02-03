//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DPolygonTester
//-----------------------------------------------------------------------------

#pragma once

// Preparation of required environement
class HVE2DPolygonTester : public EnvironnementTest 
    {   

protected :

    HVE2DPolygonTester();

    HGF2DLocation   PolyClosePoint1A;
    HGF2DLocation   PolyClosePoint1B;
    HGF2DLocation   PolyClosePoint1C;
    HGF2DLocation   PolyClosePoint1D;
    HGF2DLocation   PolyCloseMidPoint1;

    HGF2DLocation   Poly1Point0d0;
    HGF2DLocation   Poly1Point0d1;
    HGF2DLocation   Poly1Point0d5;
    HGF2DLocation   Poly1Point1d0;

    HGF2DLocation   PolyMidPoint1;

    };
