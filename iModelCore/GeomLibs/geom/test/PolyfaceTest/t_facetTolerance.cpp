/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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



static CurveVectorPtr CurveVector100 ()
   {
   auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);


   cv->push_back (ICurvePrimitive::CreateLineString (
           bvector<DPoint3d>{
               DPoint3d::From (-14.168690106104364,-6.7408422780596249,0.0),
               DPoint3d::From (-11.611239192869945,0.64884277566331239,0.0),
               DPoint3d::From (-12.218622861783601,4.2769180500377706,0.0),
               DPoint3d::From (-14.168690106104364,-6.7408422780596249,0.0)}));

   return cv;
   }

static CurveVectorPtr CurveVector101 ()
   {
   auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);


   cv->push_back (ICurvePrimitive::CreateLineString (
           bvector<DPoint3d>{
               DPoint3d::From (-2.6509469088231725,26.539364320285486,0.0),
               DPoint3d::From (-11.611239192869945,0.64884277566331239,0.0),
               DPoint3d::From (-8.1767984141144723,-19.866048072476083,0.0),
               DPoint3d::From (6.0298761823583504,-19.866048072476083,0.0),
               DPoint3d::From (13.723593336266102,5.0562859406072693,0.0),
               DPoint3d::From (-2.6509469088231725,26.539364320285486,0.0)}));

   return cv;
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Builder,JonasBadSolid)
    {
   auto curveA = CurveVector100 ();
   auto curveB = CurveVector101 ();
   Check::SaveTransformed (curveA);
   Check::SaveTransformed (curveB);
   Check::Shift (0,30,0);
   auto region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
   region->Add (curveA);
   region->Add (curveB);
   Check::SaveTransformed (region);

   auto solid = ISolidPrimitive::CreateDgnExtrusion (
        DgnExtrusionDetail (region, DVec3d::From (0,0,3.048), true));
   Check::Shift (0,30,0);
   Check::SaveTransformed (*solid);

    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddSolidPrimitive (*solid);
    auto mesh = builder->GetClientMeshPtr ();

   Check::Shift (0,30,0);
   Check::SaveTransformed (mesh);

   Check::ClearGeometry ("Builder.JonasBadSolid");
   }
// "capped":true "extrusionVector":[0.0 0.0 3.0480]}}
