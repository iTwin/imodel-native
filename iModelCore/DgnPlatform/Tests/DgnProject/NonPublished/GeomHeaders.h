/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/GeomHeaders.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// Geom API Headers
#include <Geom/dpoint2d.h>
#include <Geom/dpoint3d.h>
#include <Geom/dvec2d.h>
#include <Geom/dvec3d.h>
#include <Geom/rotmatrix.h>
#include <Geom/transform.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
void GetGeomStructList(bvector<TypeNamePair>& list)
{
    list.push_back(TypeNamePair(sizeof(DPoint2d), L"DPoint2d"));
    list.push_back(TypeNamePair(sizeof(DPoint3d), L"DPoint3d"));
    list.push_back(TypeNamePair(sizeof(DVec2d), L"DVec2d"));
    list.push_back(TypeNamePair(sizeof(DVec3d), L"DVec3d"));
    list.push_back(TypeNamePair(sizeof(RotMatrix), L"RotMatrix"));
    list.push_back(TypeNamePair(sizeof(Transform), L"Transform"));
}



