//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DUniverseTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DUniverseTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HVE2DUniverseTester : public EnvironnementTest 
    {   
protected :
    
    HVE2DUniverseTester();

    //Default Universe
    HVE2DUniverse DefaultUniverse;

    //With CoordSys
    HVE2DUniverse Universe1;
    HVE2DUniverse Universe2;

    //Rectangle
    HVE2DRectangle Rectangle1;
    HVE2DRectangle Rectangle2;

    //Location
    HGF2DLocation Origin1;
    HGF2DLocation Origin2;

    };