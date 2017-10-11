/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_visibility.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Vu/VuApi.h>


void PolyfaceHeader__clipByXYVisibility
(
bvector<PolyfaceHeaderPtr> &source,
PolyfaceHeaderPtr &dest,
bool select0,
bool select1
)
    {
    TaggedPolygonVector polygonsIn, polygonsOut;

    for (size_t i = 0; i < source.size (); i++)
        {
        auto visitor = PolyfaceVisitor::Attach (*source[i], false);
        bvector<DPoint3d> &points = visitor->Point ();
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            PolygonVectorOps::AddPolygon (polygonsIn,
                points, i, visitor->GetReadIndex ());
        }
    
    bsiPolygon_clipByXYVisibility (polygonsIn, polygonsOut, select0, select1);
    dest = PolyfaceHeader::CreateFromTaggedPolygons (polygonsOut);

    }


void AddRectangle (PolyfaceHeaderPtr &mesh, double x0, double y0, double z0, double ax, double ay)
    {
    bvector<DPoint3d> xyz
        {
        DPoint3d::From (x0, y0, z0),
        DPoint3d::From (x0 + ax, y0, z0),
        DPoint3d::From (x0 + ax, y0 + ay, z0),
        DPoint3d::From (x0, y0 + ay, z0)
        };
    mesh->AddPolygon (xyz);
    }

void BuildClipTestMeshSet (bvector<PolyfaceHeaderPtr> &meshes)
    {
    meshes.push_back (PolyfaceHeader::CreateVariableSizeIndexed ());
    AddRectangle (meshes.back (), 0,0,0,   10,12);
    AddRectangle (meshes.back (), 1,1,1,   8,6);


    meshes.push_back (PolyfaceHeader::CreateVariableSizeIndexed ());
    AddRectangle (meshes.back (), 2,2,2,   3,4);

    meshes.push_back (CreatePolyface_ExtrudedL (5,5, 7,6, 6, 8, 0.5));
    meshes.back ()->Transform (Transform::From (0,0,0.1));

    meshes.push_back (meshes.back ()->Clone ());
    meshes.back ()->Transform (
        Transform::FromAxisAndRotationAngle (
                DRay3d::FromOriginAndVector (
                        DPoint3d::From (5,5,0),
                        DVec3d::From (1,0.3, 0.2)),
                Angle::DegreesToRadians (15)));
    meshes.back ()->Transform (Transform::From (3.5, 0, 0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Clip,XYVisibilityA)
    {
    // EDL April 6 2017
    // This exercises all the (mysterious) select0,select1 combinations for bsiPolygon_clipByXYVisibility
    // The observed behavior seems to be  . . 
    // true,true -- generates the HIDDEN parts
    // true,false -- generates the VISIBLE parts
    // false,any -- not clear.   output has zinger polygons going down far below the bottom.
    bvector<PolyfaceHeaderPtr> meshA;
    BuildClipTestMeshSet (meshA);
    double a = 20.0;

    bvector<bool> trueFalse {true, false};
    for (auto &mesh : meshA)
        Check::SaveTransformed (*mesh);
    Check::Shift (0,a,0);
    for (bool select0 : trueFalse)
        {
        SaveAndRestoreCheckTransform shift0 (0, a, 0);
        for (bool select1 : trueFalse)
            {
            SaveAndRestoreCheckTransform shift1 (a,0,0);
            PolyfaceHeaderPtr meshB;
            PolyfaceHeader__clipByXYVisibility (meshA, meshB, select0, select1);
            if (meshB.IsValid ())
                Check::SaveTransformed (*meshB);
            }
        }
    Check::Shift (0,a,0);

    Check::ClearGeometry ("Clip.XYVisibilityA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Clip,XYVisibilityB)
    {
    bvector<PolyfaceHeaderPtr> meshA;
    BuildClipTestMeshSet (meshA);
    double a = 20.0;

    bvector<bool> trueFalse {true, false};
    for (auto &mesh : meshA)
        Check::SaveTransformed (*mesh);

    Check::Shift (0,a,0);
    for (DVec3d vectorToEye :
        bvector<DVec3d>
            {
            DVec3d::From (0,0,1),
            DVec3d::From (0,0,-1),
            DVec3d::From (1,1,1)
            }
        )
        {
        SaveAndRestoreCheckTransform shift0 (0, a, 0);
        PolyfaceHeaderPtr meshB;
        Transform localToWorld, worldToLocal;
        PolyfaceHeader::VisibleParts (meshA, vectorToEye, meshB, localToWorld, worldToLocal);
        if (meshB.IsValid ())
            Check::SaveTransformed (*meshB);
        }

    Check::ClearGeometry ("Clip.XYVisibilityB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Clip,XYVisibilityAB)
    {
    bvector<PolyfaceHeaderPtr> meshA;
    BuildClipTestMeshSet (meshA);
    double a = 20.0;

    bvector<bool> trueFalse {true, false};
    for (auto &mesh : meshA)
        Check::SaveTransformed (*mesh);

    Check::Shift (0,a,0);
    for (DVec3d vectorToEye :
        bvector<DVec3d>
            {
            DVec3d::From (0,0,1),
            DVec3d::From (0,0,-1),
            DVec3d::From (1,1,1)
            }
        )
        {
        SaveAndRestoreCheckTransform shift0 (0, a, 0);
        bvector<PolyfaceHeaderPtr> meshB;
        Transform localToWorld, worldToLocal;
        bvector<bvector<SizeSize>> bToA;
        PolyfaceHeader::VisibleParts (meshA, vectorToEye, meshB, &bToA, localToWorld, worldToLocal);
        for (auto m : meshB)
            {
            if (m.IsValid ())
                Check::SaveTransformed (*m);
            }
        }

    Check::ClearGeometry ("Clip.XYVisibilityAB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PartitionReadIndicesByNormal)
    {
    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddSweptNGon (30, 1.0, 0.0, 1.0, false, false);
    auto mesh = builder->GetClientMeshPtr ();
    auto vector = DVec3d::From (-1.0, -2.0, 0.0);
    double a = 7.0;
    double b = 4.0;
    //double r = 3.0;
    DPoint3d origin = DPoint3d::From (0,0,0);
    DVec3d axis = DVec3d::From (0,0,1);
    for (auto degrees : bvector<double> {0.1, 5.0, 15.0, 30.0})
        {
        SaveAndRestoreCheckTransform shift0 (a, 0, 0);
        auto theta = Angle::FromDegrees (90 + degrees);
        auto vectorA = DVec3d::FromRotateVectorAroundVector (vector, axis,  theta);
        auto vectorB = DVec3d::FromRotateVectorAroundVector (vector, axis, -theta);
            bvector<DPoint3d> annotation {
            origin - vectorB, origin + vectorB,
            origin,
            origin + vectorA, origin - vectorA,
            origin,
            origin - vector};
        Check::SaveTransformed (annotation);
                
        bvector<bvector<ptrdiff_t>> readIndices;
        mesh->PartitionReadIndicesByNormal (vector, readIndices, Angle::DegreesToRadians (degrees));
        bvector<PolyfaceHeaderPtr> subMesh;
        mesh->CopyPartitions (readIndices, subMesh);
        Check::SaveTransformed (*mesh);
        for (auto &m : subMesh)
            {
            Check::Shift (0, b, 0);
            Check::SaveTransformed (annotation);
            if (m.IsValid () && m->Point().size () > 0)
                Check::SaveTransformed (*m);
            }
        }
    Check::ClearGeometry ("Polyface.PartitionReadIndicesByNormal");
    }