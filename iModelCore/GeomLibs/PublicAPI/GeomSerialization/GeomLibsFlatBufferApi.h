/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/GeomSerialization/GeomLibsFlatBufferApi.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#if defined(BGFB_INTERNAL)
    #define BGFBIMPEXP EXPORT_ATTRIBUTE
    #define BGFBVIRTUAL EXPORT_ATTRIBUTE virtual
#else
    #define BGFBIMPEXP IMPORT_ATTRIBUTE
    #define BGFBVIRTUAL IMPORT_ATTRIBUTE virtual
#endif

#include <Bentley/Bentley.h>
//#include <Bentley/BeConsole.h>
#include <Geom/GeomApi.h>
#include "GeomLibsSerialization.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

class BentleyGeometryFlatBuffer
{
private:
BentleyGeometryFlatBuffer (){} // no intances.
public:

// test if the bytes have the signature for BentleyGeomFlatBuffer.
static BGFBIMPEXP bool IsFlatBufferFormat (bvector <Byte> &buffer);
static BGFBIMPEXP bool IsFlatBufferFormat (Byte const *buffer);

static BGFBIMPEXP IGeometryPtr BytesToGeometry (bvector <Byte> &buffer);
static BGFBIMPEXP IGeometryPtr BytesToGeometry (Byte const *buffer);

static BGFBIMPEXP ISolidPrimitivePtr BytesToSolidPrimitive (Byte const *buffer);
static BGFBIMPEXP ICurvePrimitivePtr BytesToCurvePrimitive (Byte const *buffer);
static BGFBIMPEXP CurveVectorPtr BytesToCurveVector (Byte const *buffer);
static BGFBIMPEXP PolyfaceHeaderPtr BytesToPolyfaceHeader (Byte const *buffer);
static BGFBIMPEXP MSBsplineSurfacePtr BytesToMSBsplineSurface (Byte const *buffer);


static BGFBIMPEXP bool BytesToPolyfaceQueryCarrier (Byte const *buffer, PolyfaceQueryCarrier &carrier);
static BGFBIMPEXP bool BytesToVectorOfGeometry (bvector <Byte> &buffer, bvector<IGeometryPtr> &dest);

static BGFBIMPEXP void GeometryToBytes (IGeometryCR geometry, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (PolyfaceQueryCR mesh, bvector<Byte>& buffer);

static BGFBIMPEXP void GeometryToBytes (ICurvePrimitiveCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (MSBsplineSurfaceCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (ISolidPrimitiveCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (CurveVectorCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (bvector<IGeometryPtr> &geometry, bvector<Byte>& buffer);
};

END_BENTLEY_GEOMETRY_NAMESPACE

