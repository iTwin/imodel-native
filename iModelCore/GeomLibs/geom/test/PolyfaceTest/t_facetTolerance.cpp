/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_facetTolerance.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>


//static int s_noisy = 0;
void FacetCone (double chordTol, double angleTol, double maxEdgeLength)
    {
    auto cone = ISolidPrimitive::CreateDgnCone (DgnConeDetail
                (
                DPoint3d::From (0, 0, 0),
                DPoint3d::From (0, 0, 1),
                1.0,
                1.0,
                true
                ));

    IFacetOptionsPtr options = IFacetOptions::Create ();
    if (chordTol >= 0.0)
        options->SetChordTolerance (chordTol);
    if (angleTol >= 0.0)
        options->SetAngleTolerance (angleTol);
    if (maxEdgeLength >= 0.0)
        options->SetMaxEdgeLength (maxEdgeLength);

    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddSolidPrimitive (*cone);
    auto mesh = builder->GetClientMeshPtr ();
    Check::PrintIndent (1);
    Check::Print (options->GetChordTolerance (), "ChordTol");
    Check::Print (Angle::RadiansToDegrees (options->GetAngleTolerance ()), "AngleTol");
    Check::Print (options->GetMaxEdgeLength (), "MaxEdgeLength");
    Check::Print ((uint64_t)mesh->Point().size (), "Count");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Builder,MinimalCone)
    {
    double none = -100.0;
    FacetCone (5.0, none, none);
    FacetCone (5.0, Angle::Pi (), none);
    FacetCone (5.0, Angle::Pi (), 0.5);
    FacetCone (5.0, Angle::Pi (), 1.0);
    FacetCone (5.0, Angle::Pi (), 2.0);
    FacetCone (5.0, Angle::Pi (), 3.0);
    }
