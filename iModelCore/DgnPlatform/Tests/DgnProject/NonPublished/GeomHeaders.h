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

#define MakeTypeInfo(typeName) \
    TypeInfo(sizeof(typeName), __alignof(typeName), WString(#typeName , true))

#define MakeTypeInfoAndAddToList(typeName) \
    list.push_back(MakeTypeInfo(typeName))

//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
void GetGeomStructList(bvector<TypeInfo>& list)
{
    MakeTypeInfoAndAddToList(DPoint2d);
    MakeTypeInfoAndAddToList(DPoint3d);
    MakeTypeInfoAndAddToList(DVec2d);
    MakeTypeInfoAndAddToList(DVec3d);
    MakeTypeInfoAndAddToList(RotMatrix);
    MakeTypeInfoAndAddToList(Transform);
}



