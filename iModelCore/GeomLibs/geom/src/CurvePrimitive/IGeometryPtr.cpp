/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/IGeometryPtr.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometry::IGeometry (ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
IGeometry::IGeometry (CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
IGeometry::IGeometry (ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
IGeometry::IGeometry (MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
IGeometry::IGeometry (PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometryPtr IGeometry::Create (ICurvePrimitivePtr const& source) {return new IGeometry (source);}
IGeometryPtr IGeometry::Create (CurveVectorPtr const& source) {return new IGeometry (source);}
IGeometryPtr IGeometry::Create (ISolidPrimitivePtr const& source) {return new IGeometry (source);}
IGeometryPtr IGeometry::Create (MSBsplineSurfacePtr const& source) {return new IGeometry (source);}
IGeometryPtr IGeometry::Create (PolyfaceHeaderPtr const& source) {return new IGeometry (source);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometry::GeometryType IGeometry::GetGeometryType () const {return m_type;}
ICurvePrimitivePtr IGeometry::GetAsICurvePrimitive () const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get ()) : NULL);}
CurveVectorPtr IGeometry::GetAsCurveVector () const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get ()) : NULL);}
ISolidPrimitivePtr IGeometry::GetAsISolidPrimitive () const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get ()) : NULL);}
MSBsplineSurfacePtr IGeometry::GetAsMSBsplineSurface () const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get ()) : NULL);}
PolyfaceHeaderPtr IGeometry::GetAsPolyfaceHeader () const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get ()) : NULL);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IGeometry::TryGetRange (DRange3dR range) const
    {
    if (!m_data.IsValid ())
        return false;

    switch (m_type)
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = GetAsICurvePrimitive ();

            return curvePrimitive->GetRange (range);
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = GetAsCurveVector ();

            return curveVector->GetRange (range);
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            return solidPrimitive->GetRange (range);
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = GetAsMSBsplineSurface ();

            bSurface->GetPoleRange (range);

            return true;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();

            range = polyface->PointRange ();

            return true;
            }

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IGeometry::TryGetRange (DRange3dR range, TransformCR transform) const
    {
    if (!m_data.IsValid ())
        return false;

    switch (m_type)
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = GetAsICurvePrimitive ();

            return curvePrimitive->GetRange (range, transform);
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = GetAsCurveVector ();

            return curveVector->GetRange (range, transform);
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            return solidPrimitive->GetRange (range, transform);
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = GetAsMSBsplineSurface ();

            bSurface->GetPoleRange (range, transform);

            return true;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();
            if (polyface->GetPointCount () == 0)
                range = DRange3d::NullRange ();
            else
                range = DRange3d::From (transform, &polyface->Point()[0], nullptr, (int)polyface->GetPointCount ());

            return true;
            }

        default:
            return false;
        }
    }




//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IGeometry::IsSameStructureAndGeometry (IGeometryCR other, double tolerance) const
    {
    if (!m_data.IsValid ())
        return false;
    if (!other.m_data.IsValid ())
        return false;
    if (m_type != other.m_type)
        return false;

    switch (m_type)
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr dataA = GetAsICurvePrimitive ();
            ICurvePrimitivePtr dataB = other.GetAsICurvePrimitive ();
            return dataA->IsSameStructureAndGeometry (*dataB, tolerance);
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr dataA = GetAsCurveVector ();
            CurveVectorPtr dataB = other.GetAsCurveVector ();
            return dataA->IsSameStructureAndGeometry (*dataB, tolerance);
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr dataA = GetAsISolidPrimitive ();
            ISolidPrimitivePtr dataB = other.GetAsISolidPrimitive ();
            return dataA->IsSameStructureAndGeometry (*dataB, tolerance);
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr dataA = GetAsMSBsplineSurface ();
            MSBsplineSurfacePtr dataB = other.GetAsMSBsplineSurface ();
            return dataA->IsSameStructureAndGeometry (*dataB, tolerance);
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr dataA = GetAsPolyfaceHeader ();
            PolyfaceHeaderPtr dataB = other.GetAsPolyfaceHeader ();
            return dataA->IsSameStructureAndGeometry (*dataB, tolerance);
            }

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IGeometry::TryTransformInPlace (TransformCR transform)
    {
    if (!m_data.IsValid ())
        return false;

    switch (m_type)
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = GetAsICurvePrimitive ();

            return curvePrimitive->TransformInPlace (transform);
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = GetAsCurveVector ();

            return curveVector->TransformInPlace (transform);
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();

            return solidPrimitive->TransformInPlace (transform);
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = GetAsMSBsplineSurface ();

            return (SUCCESS == bSurface->TransformSurface (transform));
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();

            polyface->Transform (transform);

            return true;
            }

        default:
            return false;
        }
    }

bool OrderedIGeometryPtr::operator < (OrderedIGeometryPtr const &other) const
    {
    if (m_geometry.IsValid () && other.m_geometry.IsValid ())
        return (size_t)m_geometry.get ()->m_data.get () < (size_t)other.m_geometry.get ()->m_data.get ();
    else
        return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometryPtr IGeometry::Clone () const
    {
    if (!m_data.IsValid ())
        return nullptr;

    switch (m_type)
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = GetAsICurvePrimitive ();
            return IGeometry::Create (curvePrimitive->Clone ());
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = GetAsCurveVector ();
            return IGeometry::Create (curveVector->Clone ());
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = GetAsISolidPrimitive ();
            return IGeometry::Create (solidPrimitive->Clone ());
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = GetAsMSBsplineSurface ();
            return IGeometry::Create (bSurface->Clone ());
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = GetAsPolyfaceHeader ();
            return IGeometry::Create (polyface->Clone ());
            }

        default:
            return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IGeometryPtr IGeometry::Clone (TransformCR transform) const
    {
    if (!m_data.IsValid ())
        return nullptr;
    auto g = Clone ();
    if (g->TryTransformInPlace (transform))
        return g;
    return nullptr;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
