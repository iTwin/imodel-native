/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/GeomJsApi/GeomJsApi.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GeomJsApi.h>
#include <DgnPlatform/GeomJsApiProjection.h>

extern Utf8CP geomJsApi_GetBootstrappingSource();

USING_NAMESPACE_BENTLEY_DGNPLATFORM
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
DVec3d GetData (JsDVector3dP vector) {return vector->Get();}
DVec2d GetData (JsDVector2dP vector) {return vector->Get();}

JsDVector3dP CreateJsVector (DVec3dCR data) {return new JsDVector3d (data);}
JsDVector2dP CreateJsVector (DVec2dCR data) {return new JsDVector2d (data);}


//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
bool TryDoubleToIndex (double a, size_t upperBound, size_t &index)
    {
    index = 0;
    if (a < 0.0)
        return false;
    index = (size_t) a;
    if (index < upperBound)
        return true;
    index = SIZE_MAX;
    return false;
    }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
static double SmallDistance ()
    {
    return 1.0e-10;
    }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsDVector3dP CreateJsVector (DVec3dCR data, double length)
        {
        double d = data.Magnitude ();
        if (d < SmallDistance ())
            return nullptr;
        return new JsDVector3d (DVec3d::FromScale (data, length / d));
        }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsDVector2dP CreateJsVector (DVec2dCR data, double length)
        {
        double d = data.Magnitude ();
        if (d < SmallDistance ())
            return nullptr;
        return new JsDVector2d (DVec2d::FromScale (data, length / d));
        }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsSolidPrimitiveP JsSolidPrimitive::Clone ()
    {
    return StronglyTypedJsSolidPrimitive (m_solidPrimitive);
    }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsSolidPrimitiveP JsSolidPrimitive::StronglyTypedJsSolidPrimitive (ISolidPrimitivePtr primitive)
    {
    switch (primitive->GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnBox:
            return new JsDgnBox (primitive);
        case SolidPrimitiveType_DgnSphere:
            return new JsDgnSphere (primitive);
        case SolidPrimitiveType_DgnTorusPipe:
            return new JsDgnTorusPipe (primitive);
        case SolidPrimitiveType_DgnCone:
            return new JsDgnCone (primitive);
        case SolidPrimitiveType_DgnExtrusion:
            return new JsDgnExtrusion (primitive);
        case SolidPrimitiveType_DgnRotationalSweep:
            return new JsDgnRotationalSweep (primitive);
        case SolidPrimitiveType_DgnRuledSweep:
            return new JsDgnRuledSweep (primitive);
        }
    return nullptr;
    }



//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsCurveVectorP JsCurveVector::StronglyTypedJsCurveVector (CurveVectorPtr &data)
    {
    if (!data.IsValid ())
        return nullptr;
    switch (data->GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_None:
            return new JsUnstructuredCurveVector (data);
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            return new JsLoop (data);
        case CurveVector::BOUNDARY_TYPE_Open:
            return new JsPath (data);
        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            return new JsParityRegion (data);
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            return new JsUnionRegion (data);
        }
    // Those are all the types . . . this should never happen, but treat it like BOUNDARY_TYPE_None
    return new JsUnstructuredCurveVector (data);
    }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsCurvePrimitiveP JsCurveVector::MemberAsCurvePrimitive (double doubleIndex) const
    {
    size_t index;
    if (TryDoubleToIndex (doubleIndex, m_curveVector->size (), index)
        && m_curveVector->at (index).IsValid ())
        {
        return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (m_curveVector->at (index), true);
        }
    return nullptr;
    }

//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsCurveVectorP JsCurveVector::MemberAsCurveVector (double doubleIndex) const
    {
    size_t index;
    if (TryDoubleToIndex (doubleIndex, m_curveVector->size (), index)
            && m_curveVector->at (index).IsValid ())
        {
        auto child = m_curveVector->at (index)->GetChildCurveVectorP ();
        if (child != nullptr)
            return StronglyTypedJsCurveVector (child);
        }
    return nullptr;
    }


//=======================================================================================
//                                                                      Eariln.Lutz     12/15
//=======================================================================================
JsCurvePrimitiveP JsCurvePrimitive::StronglyTypedJsCurvePrimitive (ICurvePrimitivePtr const &data, bool nullifyCurveVector)
    {
    if (!data.IsValid ())
        return nullptr;
    switch (data->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
            return new JsLineSegment (data);
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc:
            return new JsEllipticArc (data);
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            return new JsBsplineCurve (data);
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_CurveVector:
            if (nullifyCurveVector)
                return nullptr;
            else
                return new JsCurvePrimitive (data);
        }
    // wrap all Bspline subtypes with no Js support as just BsplineCurve ...
    auto proxy = data->GetProxyBsplineCurvePtr ();
    if (proxy.IsValid ())
        return new JsBsplineCurve (data);
    // wrap everything else as curve primitive base type ....
    return new JsCurvePrimitive (data);
    }

// Default RangeTransformed --- apply transform to simple range.  This is typically an oversized range.
//  Derived classes that implement this will return a tighter range.
JsDRange3dP JsGeometry::RangeAfterTransform (JsTransformP transform)
    {
    JsDRange3dP jsRangeA = Range ();
    if (jsRangeA->IsNull())
        return jsRangeA;
    DRange3d rangeB = jsRangeA->Get ();
    Transform transformB = transform->Get ();
    DRange3d rangeC;
    transformB.Multiply (rangeC, rangeB);
    return new JsDRange3d (rangeC);
    }

JsDRange3dP JsGeometry::Range (){ return new JsDRange3d ();}

JsGeometryP JsGeometry::CreateStronglyTypedJsGeometry (IGeometryPtr geometry)
    {
    switch (geometry->GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            auto cp = geometry->GetAsICurvePrimitive ();
            return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (cp, true);
            }
        case IGeometry::GeometryType::CurveVector:
            {
            auto cv = geometry->GetAsCurveVector ();
            return JsCurveVector::StronglyTypedJsCurveVector  (cv);
            }
        case IGeometry::GeometryType::SolidPrimitive:
            return JsSolidPrimitive::StronglyTypedJsSolidPrimitive  (geometry->GetAsISolidPrimitive ());
        case IGeometry::GeometryType::Polyface:
            return new JsPolyfaceMesh (geometry->GetAsPolyfaceHeader ());
/*
        case IGeometry::GeometryType::BsplineSurface:
            return JsBsplineSurface (geometry->GetAsBsplineSurface ());
*/
        }
    return nullptr;
    }

// Per forward declaration ..
JsCurveLocationDetailP JsCurvePrimitive::ClosestPointBounded (JsDPoint3dP spacePoint)
    {
    CurveLocationDetail detail;
    m_curvePrimitive->ClosestPointBounded (spacePoint->Get (), detail);
    return new JsCurveLocationDetail (detail);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
GeomJsApi::GeomJsApi(BeJsContext& jsContext) : m_context(jsContext)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
GeomJsApi::~GeomJsApi()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void GeomJsApi::_ImportScriptLibrary(BeJsContextR jsContext, Utf8CP)
    {
    jsContext.RegisterProjection<GeomJsApiProjection>(geomJsApi_GetBootstrappingSource(), "file:///GeomJsApi.js");
    }

struct DPoint3dMarshaller : DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller
    {
    void _MarshallNativePointerToJs(BeJsNativePointerR np, BeJsContextR ctx, void* ptr) { np = ctx.ObtainProjectedClassInstancePointer(new JsDPoint3d(*(DPoint3dP)ptr), true); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/16
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller* GeomJsApi::_GetMarshallerForType(Utf8StringCR typeScriptTypeName)
    {
    static DPoint3dMarshaller s_DPoint3dMarshaller;

    if (typeScriptTypeName.EqualsIAscii("DPoint3d"))
        return &s_DPoint3dMarshaller;

    return nullptr;
    }
