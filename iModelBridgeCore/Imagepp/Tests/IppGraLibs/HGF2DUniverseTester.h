//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DUniverseTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DUniverseTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGF2DUniverseTester : public EnvironnementTest 
    {   
protected :
    
    HGF2DUniverseTester();

    //Default Universe
    HGF2DUniverse DefaultUniverse;

    //With CoordSys
    HGF2DUniverse Universe1;
    HGF2DUniverse Universe2;

    //Rectangle
    HGF2DRectangle Rectangle1;
    HGF2DRectangle Rectangle2;

    //Location
    HGF2DPosition Origin1;
    HGF2DPosition Origin2;

    };