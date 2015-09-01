//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVEShapeTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEShapeTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HVEShapeTester : public EnvironnementTest 
    {   

protected :

    HVEShapeTester();

    //Array
    double DblArray[1000];
    size_t MyShapeCount;

    //Extent
    HGF2DExtent Extent;

    //Shape
    HVEShape Shape;
    HVEShape Shape1;    
    HVEShape Shape3;
    HVEShape Shape4;
    HVEShape Shape6;
    HVEShape Shape7;
    HVEShape Shape8;

    };