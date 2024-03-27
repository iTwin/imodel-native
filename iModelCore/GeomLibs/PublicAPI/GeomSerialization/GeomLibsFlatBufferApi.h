/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

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
BentleyGeometryFlatBuffer (){} // no instances.

public:

// Test if the bytes have the signature for BentleyGeomFlatBuffer.
static BGFBIMPEXP bool IsFlatBufferFormat(bvector<Byte> &buffer);
static BGFBIMPEXP bool IsFlatBufferFormat(Byte const *buffer);

// Convert flatbuffer bytes to geometry instance. If bytes represent an array of geometries, return nullptr.
static BGFBIMPEXP IGeometryPtr BytesToGeometry(bvector<Byte> &buffer, bool applyValidation = true);
// Convert flatbuffer bytes to geometry instance. If bytes represent an array of geometries, return nullptr.
static BGFBIMPEXP IGeometryPtr BytesToGeometry(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);

static BGFBIMPEXP ISolidPrimitivePtr BytesToSolidPrimitive(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);
static BGFBIMPEXP ICurvePrimitivePtr BytesToCurvePrimitive(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);
static BGFBIMPEXP CurveVectorPtr BytesToCurveVector(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);
static BGFBIMPEXP PolyfaceHeaderPtr BytesToPolyfaceHeader(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);
static BGFBIMPEXP MSBsplineSurfacePtr BytesToMSBsplineSurface(Byte const *buffer, size_t const bufferSize, bool applyValidation = true);

static BGFBIMPEXP bool BytesToPolyfaceQueryCarrier
(
    Byte const *buffer,
    size_t const bufferSize,
    PolyfaceQueryCarrier &carrier,
    bool applyValidation = true
);
//!
//! <ul>
//! <li>Read geometry from the buffer.
//! <li>Optionally apply validation to test each geometry.
//!    <ul>
//!    <li>Invalid geometry is recorded in the (optional) invalidGeometry array, and is NOT written to the FB.
//!    <li>Valid geometry is written to the FB.
//!    <li>Buffer can represent a geometry or an array of geometries.
//!    </ul>
//! </ul>
//!
static BGFBIMPEXP bool BytesToVectorOfGeometry
(
    bvector<Byte> &buffer,
    bvector<IGeometryPtr> &dest,
    bool applyValidation = true,
    bvector<IGeometryPtr> *invalidGeometry = nullptr
);
//! NOTE: GeometryToBytes methods ALWAYS apply the validator (see SetFBWriteValidation)
static BGFBIMPEXP void GeometryToBytes (IGeometryCR geometry, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (PolyfaceQueryCR mesh, bvector<Byte>& buffer);

static BGFBIMPEXP void GeometryToBytes (ICurvePrimitiveCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (MSBsplineSurfaceCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (ISolidPrimitiveCR mesh, bvector<Byte>& buffer);
static BGFBIMPEXP void GeometryToBytes (CurveVectorCR mesh, bvector<Byte>& buffer);
//!
//! <ul>
//! <li>Write geometry to the buffer
//! <li>If a validator is defined (Set SetFBWriteValidation), test each geometry.
//!    <ul>
//!    <li>Invalid geometry is recorded in the (optional) invalidGeometry array, and is NOT written to the FB
//!    <li>Valid geometry is written to the FB and reported in the (optional) validGeometry array.
//!    </ul>
//! <li>If no validator is active, all geometry is written to the FB (dangerous!), and both optional arrays are cleared.
//! </ul>
//!
static BGFBIMPEXP void GeometryToBytes (
    bvector<IGeometryPtr> &geometry,
    bvector<Byte>& buffer,
    bvector<IGeometryPtr> *validGeometry = nullptr,
    bvector<IGeometryPtr> *invalidGeometry = nullptr
);
};

END_BENTLEY_GEOMETRY_NAMESPACE
