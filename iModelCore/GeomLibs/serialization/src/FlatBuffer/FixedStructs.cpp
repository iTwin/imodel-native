/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include "allcg_generated.h"
#define  BGFB Bentley::Geometry::FB

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static GeometryValidatorPtr s_writeValidator = GeometryValidator::Create();
static GeometryValidatorPtr s_readValidator = GeometryValidator::Create();

/**
Secretly exported entry to suppress write-time validation ....
This is NOT published in an h file.
Unit tests that need to create invalid buffers need to directly replicate the declaration as an import ...
*/
GEOMLIBS_SERIALIZATION_EXPORT GeometryValidatorPtr BentleyGeometryFlatBuffer__SetFBWriteValidation(GeometryValidatorPtr &validator)
    {
    auto oldValidator = s_writeValidator;
    s_writeValidator = validator;
    return oldValidator;
    }

// move blocks of 8 doubles between the flat array  FacetFaceData structure arrays.
// (The flat array is memory compatible for PolyfaceQueryCarrier)
bool unpackFaceData
(
double const*doubles,
size_t numDoubles,
BlockedVector<FacetFaceData> &faceData
)
    {
    faceData.clear ();
    if (numDoubles > 0)
        {
        size_t numFaceData = numDoubles / 8;
        for (size_t i = 0, faceIndex = 0; faceIndex < numFaceData; faceIndex++)
            {
            FacetFaceData data;
            data.m_paramDistanceRange.low.x = doubles[i++];
            data.m_paramDistanceRange.low.y = doubles[i++];
            data.m_paramDistanceRange.high.x = doubles[i++];
            data.m_paramDistanceRange.high.y = doubles[i++];

            data.m_paramRange.low.x = doubles[i++];
            data.m_paramRange.low.y = doubles[i++];
            data.m_paramRange.high.x = doubles[i++];
            data.m_paramRange.high.y = doubles[i++];
            // Face indices filled with zeros by ctor !!!
            faceData.push_back (data);
            }
        return true;
        }
    return false;
    }

// s_prefixBuffer is placed at the front of the flatbuffer block ...
static const Byte s_prefixBuffer[] =
    {
        'b',
        'g',
        '0',
        '0',
        '0',
        '1',
        'f',
        'b',
    };

static const size_t s_prefixBufferSize = sizeof(s_prefixBuffer);

// confirm that allbytes starts with the expected prefix.  If so return pointer to "real" flatbuffer data following ...
static Byte *GetFBStart (bvector<Byte> &allbytes)
    {
    if (allbytes.size () <= s_prefixBufferSize)
        return nullptr;
    for (size_t i = 0; i < s_prefixBufferSize; i++)
        if (s_prefixBuffer[i] != allbytes[i])
            return nullptr;
    return &allbytes[0] + s_prefixBufferSize;
    }

// confirm that allbytes starts with the expected prefix.  If so return pointer to "real" flatbuffer data following ...
static Byte const *GetFBStart (Byte const *allbytes, size_t bufferSize)
    {
    if (bufferSize <= s_prefixBufferSize)
        return nullptr;
    for (size_t i = 0; i < s_prefixBufferSize; i++)
        if (s_prefixBuffer[i] != allbytes[i])
            return nullptr;
    return allbytes + s_prefixBufferSize;
    }

struct FBWriter
{
friend BentleyGeometryFlatBuffer;
private:
flatbuffers::FlatBufferBuilder m_fbb;
public:
FBWriter () : m_fbb ()
    {
    }
private:
static BGFB::DPoint3d FBDPoint3d (DPoint3dCR xyz)
    {
    return BGFB::DPoint3d (xyz.x, xyz.y, xyz.z);
    }

static BGFB::DVector3d FBDVector3d (DVec3dCR xyz)
    {
    return BGFB::DVector3d (xyz.x, xyz.y, xyz.z);
    }

static BGFB::DSegment3d FBDSegment3d (DSegment3dCR segment)
    {
    return BGFB::DSegment3d
            (
            segment.point[0].x, segment.point[0].y, segment.point[0].z,
            segment.point[1].x, segment.point[1].y, segment.point[1].z
            );
    }

static BGFB::DEllipse3d FBDEllipse3d (DEllipse3dCR arc)
    {
    return BGFB::DEllipse3d
            (
            arc.center.x, arc.center.y, arc.center.z,
            arc.vector0.x, arc.vector0.y, arc.vector0.z,
            arc.vector90.x, arc.vector90.y, arc.vector90.z,
            arc.start, arc.sweep
            );
    }

public:
flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (MSBsplineSurfaceCR source)
    {
      bvector<DPoint3d> poles;
      source.GetPoles (poles);
      bvector<double> xyzPoles;
      CurveVectorPtr boundaries = source.GetUVBoundaryCurves (false, true);
      size_t numPoles = poles.size ();
      xyzPoles.reserve (3 * numPoles);
      for (size_t i = 0; i < numPoles; i++)
          {
          xyzPoles.push_back (poles[i].x);
          xyzPoles.push_back (poles[i].y);
          xyzPoles.push_back (poles[i].z);
          }
      auto fbPoles = m_fbb.CreateVector (xyzPoles);

      bvector<double> knots;
      source.GetUKnots (knots);
      auto fbKnotsU = m_fbb.CreateVector (knots);
      knots.clear ();
      source.GetVKnots (knots);
      auto fbKnotsV = m_fbb.CreateVector (knots);

      flatbuffers::Offset<flatbuffers::Vector<double>> fbWeights = 0;

      if (source.rational)
          {
          bvector<double> weights;
          source.GetWeights (weights);
          fbWeights = m_fbb.CreateVector (weights);
          }

      flatbuffers::Offset<BGFB::CurveVector> fbBoundaries = 0;
      if (boundaries.IsValid () && boundaries->size () > 0)
          fbBoundaries = WriteAsFBCurveVector (boundaries.get ());

      BGFB::BsplineSurfaceBuilder builder (m_fbb);
      builder.add_orderU (source.uParams.order);
      builder.add_orderV (source.vParams.order);

      builder.add_numPolesU (source.uParams.numPoles);
      builder.add_numPolesV (source.vParams.numPoles);

      builder.add_numRulesU (source.uParams.numRules);
      builder.add_numRulesV (source.vParams.numRules);

      builder.add_closedU ((uint8_t)source.uParams.closed);
      builder.add_closedV ((uint8_t)source.vParams.closed);
      builder.add_holeOrigin (source.holeOrigin);
      builder.add_poles (fbPoles);
      builder.add_knotsU (fbKnotsU);
      builder.add_knotsV (fbKnotsV);
      if (source.rational)
          builder.add_weights (fbWeights);

      builder.add_boundaries (fbBoundaries);

      auto fbCurve = builder.Finish ();
      return CreateVariantGeometry (m_fbb,
              BGFB::VariantGeometryUnion_BsplineSurface,
              fbCurve.Union ());

    }

public: flatbuffers::Offset<BGFB::CurvePrimitiveId> WriteVariantGeometryTag (CurvePrimitiveIdCP tag)
    {
    if (nullptr != tag)
        {
        flatbuffers::Offset<flatbuffers::Vector<uint8_t>> fbBytes =
            tag->GetIdSize () == 0 ? 0 : m_fbb.CreateVector (tag->PeekId (), tag->GetIdSize ());
        BGFB::CurvePrimitiveIdBuilder builder (m_fbb);
        builder.add_type ((uint16_t)tag->GetType ());
        builder.add_geomIndex ((uint16_t)tag->GetGeometryStreamIndex ());
        builder.add_partIndex ((uint16_t)tag->GetPartGeometryStreamIndex ());
        builder.add_bytes (fbBytes);
        return builder.Finish ();
        }
    return 0;
    }
public: flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (ICurvePrimitiveCR parent)
    {
    if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        DSegment3d segment;
        parent.TryGetLine (segment);
        BGFB::DSegment3d fbSegment = FBDSegment3d (segment);
        //return CreateLineSegment (m_fbb, &dataA, &dataB);
        auto dataC = BGFB::CreateLineSegment (m_fbb, &fbSegment);
        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_LineSegment, dataC.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc)
        {
        DEllipse3d arc;
        parent.TryGetArc (arc);
        BGFB::DEllipse3d fbArc = FBDEllipse3d (arc);
        //return CreateLineEllipse (m_fbb, &dataA, &dataB);
        auto dataC = BGFB::CreateEllipticArc (m_fbb, &fbArc);
        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_EllipticArc, dataC.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve)
        {
        MSBsplineCurveCP source = parent.GetBsplineCurveCP ();


        bvector<DPoint3d> poles;
        source->GetPoles (poles);
        bvector<double> xyzPoles;
        size_t numPoles = poles.size ();
        xyzPoles.reserve (3 * numPoles);
        for (size_t i = 0; i < numPoles; i++)
            {
            xyzPoles.push_back (poles[i].x);
            xyzPoles.push_back (poles[i].y);
            xyzPoles.push_back (poles[i].z);
            }
        auto fbPoles = m_fbb.CreateVector (xyzPoles);

        bvector<double> knots;
        source->GetKnots (knots);
        auto fbKnots = m_fbb.CreateVector (knots);
        flatbuffers::Offset<flatbuffers::Vector<double>> fbWeights = 0;
        if (source->rational)
            {
            bvector<double> weights;
            source->GetWeights (weights);
            fbWeights = m_fbb.CreateVector (weights);
            }
        BGFB::BsplineCurveBuilder builder (m_fbb);

        builder.add_order (source->params.order);
        builder.add_closed ((uint8_t)source->params.closed);
        builder.add_poles (fbPoles);
        builder.add_knots (fbKnots);
        if (source->rational)
            builder.add_weights (fbWeights);
        auto fbCurve = builder.Finish ();

        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_BsplineCurve,
                fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        {
        bvector<DPoint3d> const *sourcePoints = parent.GetLineStringCP ();
        bvector<double> xyzPoints;
        size_t numPoints = sourcePoints->size ();
        xyzPoints.reserve (3 * numPoints);
        for (DPoint3d const & xyz : *sourcePoints)
            {
            xyzPoints.push_back (xyz.x);
            xyzPoints.push_back (xyz.y);
            xyzPoints.push_back (xyz.z);
            }

        auto fbPoints = m_fbb.CreateVector (
                numPoints > 0 ? (double const*)&sourcePoints->front ().x : nullptr,
                3 * numPoints);

        auto fbTag = WriteVariantGeometryTag (parent.GetId ());
        BGFB::LineStringBuilder builder (m_fbb);
        builder.add_points (fbPoints);

        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_LineString,
                builder.Finish ().Union (), fbTag);
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString)
        {
        bvector<DPoint3d> const *sourcePoints = parent.GetPointStringCP ();

        bvector<double> xyzPoints;
        size_t numPoints = xyzPoints.size ();
        xyzPoints.reserve (3 * numPoints);
        for (DPoint3d const & xyz : *sourcePoints)
            {
            xyzPoints.push_back (xyz.x);
            xyzPoints.push_back (xyz.y);
            xyzPoints.push_back (xyz.z);
            }
        auto fbPoints = m_fbb.CreateVector ((double const*)&sourcePoints->front ().x,
                3 * sourcePoints->size ());

        auto fbTag = WriteVariantGeometryTag (parent.GetId ());
        BGFB::PointStringBuilder builder (m_fbb);
        builder.add_points (fbPoints);
        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_PointString,
                builder.Finish ().Union (), fbTag);
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        CurveVectorCP child = parent.GetChildCurveVectorCP ();
        return WriteAsFBVariantGeometry (child);
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve)
        {
        MSInterpolationCurveCP source = parent.GetInterpolationCurveCP ();


        bvector<DPoint3d> poles;
        bvector<double> xyzPoles;
        int numPoles = source->params.numPoints;
        xyzPoles.reserve (3 * numPoles);
        for (int i = 0; i < numPoles; i++)
            {
            xyzPoles.push_back (source->fitPoints[i].x);
            xyzPoles.push_back (source->fitPoints[i].y);
            xyzPoles.push_back (source->fitPoints[i].z);
            }
        auto fbPoles = m_fbb.CreateVector (xyzPoles);

        bvector<double> knots;
        int numKnots = source->params.numKnots;
        for (int i = 0; i < numKnots; i++)
            knots.push_back (source->knots[i]);
        auto fbKnots = m_fbb.CreateVector (knots);

        auto fbStartTangent = FBDPoint3d (source->startTangent);
        auto fbEndTangent = FBDVector3d (DVec3d::From (source->endTangent));

        BGFB::InterpolationCurveBuilder builder (m_fbb);

        builder.add_order (source->params.order);
        builder.add_closed ((uint8_t)source->params.isPeriodic);

        builder.add_startTangent (&fbStartTangent);
        builder.add_endTangent (&fbEndTangent);
        builder.add_isChordLenKnots (source->params.isChordLenKnots);
        builder.add_isColinearTangents (source->params.isColinearTangents);
        builder.add_isChordLenTangents (source->params.isChordLenTangents);
        builder.add_isNaturalTangents (source->params.isNaturalTangents);

        builder.add_fitPoints (fbPoles);
        builder.add_knots (fbKnots);

        auto fbCurve = builder.Finish ();

        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_InterpolationCurve,
                fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve)
        {
        bvector<DPoint3d> const *source = parent.GetAkimaCurveCP ();

        bvector<DPoint3d> poles;
        bvector<double> xyzPoles;
        size_t numPoles = source->size ();
        xyzPoles.reserve (3 * numPoles);
        for (DPoint3d xyz : *source)
            {
            xyzPoles.push_back (xyz.x);
            xyzPoles.push_back (xyz.y);
            xyzPoles.push_back (xyz.z);
            }
        auto fbPoles = m_fbb.CreateVector (xyzPoles);

        BGFB::AkimaCurveBuilder builder (m_fbb);
        builder.add_points (fbPoles);
        auto fbCurve = builder.Finish ();

        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_AkimaCurve,
                fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral)
        {
        DSpiral2dPlacementCP placement = parent.GetSpiralPlacementCP ();
        if (nullptr != placement)
            {
            BGFB::DTransform3d transform
                    (
                    placement->frame.form3d[0][0], placement->frame.form3d[0][1], placement->frame.form3d[0][2], placement->frame.form3d[0][3],
                    placement->frame.form3d[1][0], placement->frame.form3d[1][1], placement->frame.form3d[1][2], placement->frame.form3d[1][3],
                    placement->frame.form3d[2][0], placement->frame.form3d[2][1], placement->frame.form3d[2][2], placement->frame.form3d[2][3]
                    );

            BGFB::TransitionSpiralDetail detail (transform,
                    placement->fractionA, placement->fractionB,
                    placement->spiral->mTheta0, placement->spiral->mTheta1,
                    placement->spiral->mCurvature0, placement->spiral->mCurvature1,
                    placement->spiral->GetTransitionTypeCode (),
                    0);

            auto fbCurve = BGFB::CreateTransitionSpiral (m_fbb, &detail);

            return CreateVariantGeometry (m_fbb,
                    BGFB::VariantGeometryUnion_TransitionSpiral,
                    fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
           }
       }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Catenary)
        {
        DCatenary3dPlacement placement;
        parent.TryGetCatenary (placement);
        double a;
        DPoint3dDVec3dDVec3d basis;
        DSegment1d xLimits;
        placement.Get (a, basis, xLimits);
        auto fbOrigin = FBDPoint3d (basis.origin);
        auto fbVectorU = FBDVector3d (basis.vectorU);
        auto fbVectorV = FBDVector3d (basis.vectorV);
        auto fbCurve = BGFB::CreateCatenaryCurve
            (
            m_fbb,
            a,
            &fbOrigin,
            &fbVectorU,
            &fbVectorV,
            xLimits.GetStart (),
            xLimits.GetEnd ()
            );

        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_CatenaryCurve,
                fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
       }
    else if (parent.GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve)
        {
        PartialCurveDetailCP detail = parent.GetPartialCurveDetailCP ();
        auto fbTarget = WriteAsFBVariantGeometry (*detail->parentCurve);
        auto fbCurve = BGFB::CreatePartialCurve
            (
            m_fbb,
            detail->fraction0, detail->fraction1,
            fbTarget
            );
        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_PartialCurve,
                fbCurve.Union (), WriteVariantGeometryTag (parent.GetId ()));
        }
    return 0;
    }



flatbuffers::Offset<BGFB::CurveVector> WriteAsFBCurveVector (CurveVectorCP parent)
    {
    if (nullptr == parent)
        return 0;

    CurveVectorPtr flattened;
    if (parent->HasNestedUnionRegion())
        {
        // requirement for PowerPlatform and iModel
        flattened = parent->Clone();
        flattened->FlattenNestedUnionRegions();
        if (flattened.IsValid())
            parent = flattened.get();
        }

    CurveVector::BoundaryType type = parent->GetBoundaryType ();
    bvector<flatbuffers::Offset<BGFB::VariantGeometry>> fbCurves;
    for (size_t i = 0; i < parent->size (); i++)
        {
        flatbuffers::Offset<BGFB::VariantGeometry> fbChild = WriteAsFBVariantGeometry (*parent->at(i));
        fbCurves.push_back (fbChild);
        }
    auto fbChildren = m_fbb.CreateVector (fbCurves);
    return BGFB::CreateCurveVector (m_fbb, (int)type, fbChildren);
    }

flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (CurveVectorCP parent)
    {
    auto fbCurveVector = WriteAsFBCurveVector (parent);
        return CreateVariantGeometry (m_fbb,
                BGFB::VariantGeometryUnion_CurveVector,
                fbCurveVector.Union ()
                );
    }

flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (ISolidPrimitiveCR parent)
    {
    SolidPrimitiveType type = parent.GetSolidPrimitiveType ();
    switch (type)
        {
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail detail;
            parent.TryGetDgnBoxDetail (detail);
            auto fbData = BGFB::CreateDgnBox (m_fbb, (BGFB::DgnBoxDetail*)&detail); // YES -- hard case of compatible structure layouts
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnBox,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;
            parent.TryGetDgnConeDetail (detail);
            auto fbData = BGFB::CreateDgnCone (m_fbb, (BGFB::DgnConeDetail*)&detail); // YES -- hard case of compatible structure layouts
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnCone,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;
            parent.TryGetDgnTorusPipeDetail (detail);
            auto fbData = BGFB::CreateDgnTorusPipe (m_fbb, (BGFB::DgnTorusPipeDetail*)&detail); // YES -- hard case of compatible structure layouts
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnTorusPipe,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;
            parent.TryGetDgnSphereDetail (detail);
            auto fbData = BGFB::CreateDgnSphere (m_fbb, (BGFB::DgnSphereDetail*)&detail); // YES -- hard case of compatible structure layouts
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnSphere,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;
            parent.TryGetDgnExtrusionDetail (detail);
            auto fbData = BGFB::CreateDgnExtrusion (m_fbb,
                            WriteAsFBCurveVector (detail.m_baseCurve.get ()),
                            (BGFB::DVector3d*)&detail.m_extrusionVector,
                            detail.m_capped);
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnExtrusion,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;
            parent.TryGetDgnRotationalSweepDetail (detail);
            auto fbData = BGFB::CreateDgnRotationalSweep (m_fbb,
                            WriteAsFBCurveVector (detail.m_baseCurve.get ()),
                            (BGFB::DRay3d*)&detail.m_axisOfRotation,
                            detail.m_sweepAngle,
                            (int32_t)detail.m_numVRules,
                            detail.m_capped);
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnRotationalSweep,
                fbData.Union ()
                );
            }
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
            parent.TryGetDgnRuledSweepDetail (detail);
            bvector<flatbuffers::Offset<BGFB::CurveVector>> fbCurves;
            for (size_t i = 0; i < detail.m_sectionCurves.size (); i++)
                {
                fbCurves.push_back (WriteAsFBCurveVector (detail.m_sectionCurves[i].get ()));
                }
            auto fbChildren = m_fbb.CreateVector (fbCurves);
            auto fbData = BGFB::CreateDgnRuledSweep (m_fbb,
                            fbChildren,
                            detail.m_capped);
            return BGFB::CreateVariantGeometry
                (
                m_fbb,
                BGFB::VariantGeometryUnion_DgnRuledSweep,
                fbData.Union ()
                );
            }
        }
    return 0;
    }

template<typename TBlocked, typename TScalar, int numPerBlock>
flatbuffers::Offset<flatbuffers::Vector<TScalar>> WriteOptionalVector (bvector<TBlocked> const &source)
    {
    if (source.size () > 0)
        {
        size_t flatCount = source.size () * numPerBlock;
        auto fbOffset = m_fbb.CreateVector ((TScalar const*)&source[0], flatCount);
        return fbOffset;
        }
    return 0;
    }

template<typename TBlocked, typename TScalar, int numPerBlock>
flatbuffers::Offset<flatbuffers::Vector<TScalar>> WriteOptionalVector (TBlocked const *source, size_t n)
    {
    if (n > 0 && source != nullptr)
        {
        size_t flatCount = n * numPerBlock;
        auto fbOffset = m_fbb.CreateVector ((TScalar const*)source, flatCount);
        return fbOffset;
        }
    return 0;
    }

 template<typename T>
 bool IsWritten (const flatbuffers::Offset<flatbuffers::Vector<T>> offset)
    {
    return offset.o != 0;
    }

 template<typename T>
 bool IsWritten(const flatbuffers::Offset<T> offset)
     {
     return offset.o != 0;
     }

flatbuffers::Offset<BGFB::VectorOfVariantGeometry> WriteAsFBVectorOfVariantGeometry (bvector<IGeometryPtr> const &source)
    {
    bvector<flatbuffers::Offset<Bentley::Geometry::FB::VariantGeometry>> fbItems;
    for (size_t i = 0; i < source.size (); i++)
        {
        auto g = WriteAsFBVariantGeometry (*source[i]);
        fbItems.push_back (g);
        }
    auto fbArray = m_fbb.CreateVector (fbItems);
    return CreateVectorOfVariantGeometry (m_fbb, fbArray);
    }

flatbuffers::Offset<BGFB::VariantGeometry> WriteAsVariantGeometry (bvector<IGeometryPtr> const &source)
    {
    auto fbChild = WriteAsFBVectorOfVariantGeometry (source);
    return BGFB::CreateVariantGeometry
        (
        m_fbb,
        BGFB::VariantGeometryUnion_VectorOfVariantGeometry,
        fbChild.Union ()
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::PolyfaceAuxChannelData> WriteAsFBPolyfaceAuxChannelData (PolyfaceAuxChannel::DataP pData)
    {
    if (nullptr == pData)
        return 0;

    auto        valuesOffset = WriteOptionalVector<double, double, 1> (pData->GetValues());
    BGFB::PolyfaceAuxChannelDataBuilder builder (m_fbb);
    builder.add_input (pData->GetInput());
    builder.add_values(valuesOffset);

    return builder.Finish();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::PolyfaceAuxChannel> WriteAsFBPolyfaceAuxChannel(PolyfaceAuxChannelCP pChannel)
    {
    if (nullptr == pChannel)
        return 0;

    bvector<flatbuffers::Offset<BGFB::PolyfaceAuxChannelData>> fbDataVector;

    for (auto const& data : pChannel->GetData())
        fbDataVector.push_back(WriteAsFBPolyfaceAuxChannelData(data.get()));

    auto    fbName = m_fbb.CreateString(pChannel->GetName());
    auto    fbInputName = m_fbb.CreateString(pChannel->GetInputName());
    auto    fbData = m_fbb.CreateVector(fbDataVector);

    BGFB::PolyfaceAuxChannelBuilder builder (m_fbb);
    builder.add_dataType(pChannel->GetDataType());
    builder.add_name (fbName);
    builder.add_inputName (fbInputName);
    builder.add_data(fbData);

    return builder.Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::PolyfaceAuxData> WriteAsFBPolyfaceAuxData (PolyfaceAuxDataCP pAuxData)
    {
    if (nullptr == pAuxData)
        return 0;

    bvector<flatbuffers::Offset<BGFB::PolyfaceAuxChannel>> fbChannelVector;

    for (auto const& channel : pAuxData->GetChannels())
        fbChannelVector.push_back(WriteAsFBPolyfaceAuxChannel(channel.get()));

    auto    fbIndices = m_fbb.CreateVector(pAuxData->GetIndices());     // 1-based, 0-terminated/padded
    auto    fbChannels = m_fbb.CreateVector(fbChannelVector);

    BGFB::PolyfaceAuxDataBuilder builder (m_fbb);
    builder.add_indices(fbIndices);
    builder.add_channels(fbChannels);

    return builder.Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const flatbuffers::Offset<BGFB::TaggedNumericData> WriteFBTaggedNumericData(TaggedNumericData const &data)
    {
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> intDataOffset  = WriteOptionalVector<int, int, 1>(data.m_intData);
    const flatbuffers::Offset<flatbuffers::Vector<double>> doubleDataOffset = WriteOptionalVector<double, double, 1>(data.m_doubleData);

    BGFB::TaggedNumericDataBuilder builder(m_fbb);
    builder.add_tagA (data.m_tagA);
    builder.add_tagB (data.m_tagB);
    if (IsWritten(intDataOffset))
        builder.add_intData (intDataOffset);
    if (IsWritten(doubleDataOffset))
        builder.add_doubleData(doubleDataOffset);
    return builder.Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::Polyface> WriteAsFBPolyfaceDirect(PolyfaceQueryCR parent)
    {
    int32_t meshStyle = parent.GetMeshStyle ();
    if (meshStyle != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        // FB mesh index format is 1-based, 0-terminated/padded, variable/fixed-size face loops
        auto indexedMesh = parent.CloneAsVariableSizeIndexed();
        if (indexedMesh.IsValid())
            return WriteAsFBPolyfaceDirect(*indexedMesh);
        return 0;
        }

    int32_t numPerFace = parent.GetNumPerFace ();
    int32_t numPerRow = parent.GetNumPerRow ();
    bool    twoSided  = parent.GetTwoSided ();
    uint32_t expectedClosure = parent.GetExpectedClosure();

    // ASSUME: Polyface/AuxData index arrays are parallel
    const flatbuffers::Offset<flatbuffers::Vector<double>> point = WriteOptionalVector<DPoint3d, double, 3>(parent.GetPointCP (), parent.GetPointCount ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> param = WriteOptionalVector<DPoint2d, double, 2>(parent.GetParamCP (), parent.GetParamCount ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> normal = WriteOptionalVector<DVec3d, double, 3>(parent.GetNormalCP (), parent.GetNormalCount ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> faceData = WriteOptionalVector<FacetFaceData, double, 8>(parent.GetFaceDataCP (), parent.GetFaceCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> pointIndex = WriteOptionalVector<int, int, 1>(parent.GetPointIndexCP (), parent.GetPointIndexCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> paramIndex = WriteOptionalVector<int, int, 1>(parent.GetParamIndexCP (), parent.GetPointIndexCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> normalIndex = WriteOptionalVector<int, int, 1>(parent.GetNormalIndexCP (), parent.GetPointIndexCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> colorIndex = WriteOptionalVector<int, int, 1>(parent.GetColorIndexCP (), parent.GetPointIndexCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> faceIndex = WriteOptionalVector<int, int, 1>(parent.GetFaceIndexCP (), parent.GetPointIndexCount ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> intColor = WriteOptionalVector<uint32_t, int, 1>(parent.GetIntColorCP (), parent.GetColorCount ());
    const flatbuffers::Offset<BGFB::PolyfaceAuxData>        auxData = WriteAsFBPolyfaceAuxData(parent.GetAuxDataCP().get());

    auto numericTags = parent.GetNumericTagsCP ();
    const flatbuffers::Offset<BGFB::TaggedNumericData>
        taggedNumericData = (numericTags == nullptr || numericTags->IsZero()) ? 0 : WriteFBTaggedNumericData(*numericTags);

    BGFB::PolyfaceBuilder builder (m_fbb);
    builder.add_numPerFace (numPerFace);
    builder.add_numPerRow (numPerRow);
    builder.add_meshStyle (meshStyle);
    builder.add_twoSided (twoSided);
    builder.add_expectedClosure(expectedClosure);

    if (IsWritten (point))
        builder.add_point (point);
    if (IsWritten (param))
        builder.add_param (param);
    if (IsWritten (normal))
        builder.add_normal (normal);
    if (IsWritten (pointIndex))
        builder.add_pointIndex (pointIndex);
    if (IsWritten (paramIndex))
        builder.add_paramIndex (paramIndex);
    if (IsWritten (normalIndex))
        builder.add_normalIndex (normalIndex);
    if (IsWritten (colorIndex))
        builder.add_colorIndex (colorIndex);
    if (IsWritten (intColor))
        builder.add_intColor (intColor);
    if (IsWritten (faceIndex))
        builder.add_faceIndex (faceIndex);
    if (IsWritten (faceData))
        builder.add_faceData (faceData);
    if (0 != auxData.o)
        builder.add_auxData(auxData);
    if (IsWritten(taggedNumericData))
        builder.add_taggedNumericData(taggedNumericData);

    return builder.Finish ();
    }

flatbuffers::Offset<BGFB::Polyface> WriteAsFBPolyface (PolyfaceQueryCR parent)
    {
    return WriteAsFBPolyfaceDirect(parent);
    }

flatbuffers::Offset<BGFB::Polyface> WriteAsFBPolyface (PolyfaceHeaderR parent)
    {
    return WriteAsFBPolyfaceDirect(parent);
    }

flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (PolyfaceQueryCR parent)
    {
    auto fbPolyface = WriteAsFBPolyface (parent);
    return CreateVariantGeometry (m_fbb, BGFB::VariantGeometryUnion_Polyface,
            fbPolyface.Union ());
    }

public:
flatbuffers::Offset<BGFB::VariantGeometry> WriteAsFBVariantGeometry (IGeometryCR parent)
    {
    IGeometry::GeometryType type = parent.GetGeometryType ();
    if (type == IGeometry::GeometryType::CurvePrimitive)
        {
        ICurvePrimitivePtr ptr = parent.GetAsICurvePrimitive ();
        if (ptr.IsValid ())
            {
            return WriteAsFBVariantGeometry (*ptr);
            }
        }
    else if (type == IGeometry::GeometryType::CurveVector)
        {
        CurveVectorPtr ptr = parent.GetAsCurveVector ();
        if (ptr.IsValid ())
            {
            return WriteAsFBVariantGeometry(ptr.get ());
            }
        }
    else if (type == IGeometry::GeometryType::SolidPrimitive)
        {
        ISolidPrimitivePtr ptr = parent.GetAsISolidPrimitive ();
        if (ptr.IsValid ())
            {
            return WriteAsFBVariantGeometry (*ptr);
            }
        }
    else if (type == IGeometry::GeometryType::BsplineSurface)
        {
        MSBsplineSurfacePtr ptr = parent.GetAsMSBsplineSurface ();
        if (ptr.IsValid ())
            return WriteAsFBVariantGeometry (*ptr);
        }
    else if (type == IGeometry::GeometryType::Polyface)
        {
        PolyfaceHeaderPtr ptr = parent.GetAsPolyfaceHeader ();
        if (ptr.IsValid ())
            {
            auto fbPolyface = WriteAsFBPolyface (*ptr);
            return CreateVariantGeometry (m_fbb, BGFB::VariantGeometryUnion_Polyface,
                    fbPolyface.Union ());
            }
        }

    return 0;
    }

void FinishAndGetBuffer (flatbuffers::Offset<BGFB::VariantGeometry> g, bvector<Byte> &buffer)
    {
    m_fbb.Finish (g);
    buffer = bvector<Byte>(std::begin(s_prefixBuffer), std::end(s_prefixBuffer));
    buffer.resize(buffer.size () + m_fbb.GetSize());
    Byte *fbDest = GetFBStart (buffer);
    if (nullptr != fbDest)
        memcpy(fbDest, m_fbb.GetBufferPointer(), m_fbb.GetSize());
    else
      buffer.clear ();
    }

    };


void BentleyGeometryFlatBuffer::GeometryToBytes (IGeometryCR geometry, bvector<Byte>& buffer)
    {
    if (!GeometryValidator::IsValidGeometry(s_writeValidator, geometry))
        {
        buffer.clear ();
        return;
        }
    FBWriter writer;
    flatbuffers::Offset<BGFB::VariantGeometry> g = writer.WriteAsFBVariantGeometry (geometry);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (PolyfaceQueryCR polyfaceQuery, bvector<Byte>& buffer)
    {
    if (s_writeValidator.IsValid () && !polyfaceQuery.IsValidGeometry (s_writeValidator))
        {
        buffer.clear();
        return;
        }
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (polyfaceQuery);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (ICurvePrimitiveCR prim, bvector<Byte>& buffer)
    {
    if (s_writeValidator.IsValid() && !prim.IsValidGeometry(s_writeValidator))
        {
        buffer.clear();
        return;
        }
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (prim);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (MSBsplineSurfaceCR source, bvector<Byte>& buffer)
    {
    if (s_writeValidator.IsValid() && !source.IsValidGeometry(s_writeValidator))
        {
        buffer.clear();
        return;
        }
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (ISolidPrimitiveCR source, bvector<Byte>& buffer)
    {
    if (s_writeValidator.IsValid() && !source.IsValidGeometry(s_writeValidator))
        {
        buffer.clear();
        return;
        }
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (CurveVectorCR source, bvector<Byte>& buffer)
    {
    if (s_writeValidator.IsValid() && !source.IsValidGeometry(s_writeValidator))
        {
        buffer.clear();
        return;
        }
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (&source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes(
    bvector<IGeometryPtr> &source,
    bvector<Byte>& buffer,
    bvector<IGeometryPtr> *validGeometry,
    bvector<IGeometryPtr> *invalidGeometry)
    {
    auto validator = s_writeValidator;
    bvector<IGeometryPtr> myValidGeometry;
    if (!validGeometry)
        validGeometry = &myValidGeometry;
    GeometryValidator::ValidateAndAppend (validator, source, validGeometry, invalidGeometry);
    FBWriter writer;
    auto g = writer.WriteAsVariantGeometry (*validGeometry);    // FB is fine with empty or singleton array.
    writer.FinishAndGetBuffer (g, buffer);
    }



struct FBReader
{

static CurveVectorPtr ReadCurveVector (const BGFB::VariantGeometry * fbGeometry)
    {
    if (BGFB::VariantGeometryUnion_CurveVector != fbGeometry->geometry_type ())
        return nullptr;
    auto fbCurveVector = reinterpret_cast <const BGFB::CurveVector *> (fbGeometry->geometry ());
    return ReadCurveVectorDirect (fbCurveVector);
    }

static void ReadVectorOfVariantGeometryDirect (const BGFB::VectorOfVariantGeometry * fbVectorOfVariantGeometry,
            bvector<IGeometryPtr> &members)
    {
    auto fbMembers = fbVectorOfVariantGeometry->members ();
    if (!fbMembers)
        return;
    auto numMembers = fbMembers->Length ();
    for (unsigned int i = 0; i < numMembers; i++)
        {
        auto fbMember = fbMembers->Get (i);
        IGeometryPtr gPtr = ReadGeometry (fbMember);
        if (gPtr.IsValid ())
            members.push_back (gPtr);
        }
    }

static CurveVectorPtr ReadCurveVectorDirect (const BGFB::CurveVector * fbCurveVector)
    {
    if (0 == fbCurveVector)
        return nullptr;
    int type = fbCurveVector->type ();
    auto fbCurves = fbCurveVector->curves ();
    if (!fbCurves)
        return nullptr;
    auto numCurves = fbCurves->Length ();
    CurveVectorPtr cvPtr = CurveVector::Create ((CurveVector::BoundaryType)type);
    for (unsigned int i = 0; i < numCurves; i++)
        {
        auto fbCurve = fbCurves->Get (i);
        IGeometryPtr gPtr = ReadGeometry (fbCurve);
        if (!gPtr.IsValid ())
            continue;

        switch (gPtr->GetGeometryType ())
            {
            case IGeometry::GeometryType::CurvePrimitive:
                {
                cvPtr->push_back (gPtr->GetAsICurvePrimitive ());
                break;
                }

            case IGeometry::GeometryType::CurveVector:
                {
                cvPtr->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*gPtr->GetAsCurveVector ()));
                break;
                }
            }
        }

    cvPtr->FlattenNestedUnionRegions(); // requirement for PowerPlatform and iModel

    return cvPtr;
    }

// Compute the number of logical entries in every flat data array in the AuxData
static uint32_t ChannelDataLength(BGFB::PolyfaceAuxData const& fbAuxData)
    {
    if (!fbAuxData.channels() || !fbAuxData.channels()->size())
        return 0;

    auto fbChannel0 = fbAuxData.channels()->Get(0);
    if (!fbChannel0)
        return 0;

    auto numChannel0Data = fbChannel0->data()->size();
    if (!numChannel0Data)
        return 0;

    auto fbChannel0Data0 = fbChannel0->data()->Get(0);
    if (!fbChannel0Data0)
        return 0;

    auto numChannelDataValues = fbChannel0Data0->values() ? fbChannel0Data0->values()->size() : 0;
    if (!numChannelDataValues)
        return 0;

    return numChannelDataValues / (uint32_t) PolyfaceAuxChannel::GetBlockSize((PolyfaceAuxChannel::DataType) fbChannel0->dataType());
    }

// Examine int array for range and zero count
static uint32_t CountZeroes(int32_t const* ints, uint32_t numInts, int32_t& min, int32_t& max)
    {
    min = std::numeric_limits<int32_t>::max();
    max = std::numeric_limits<int32_t>::lowest();
    uint32_t numZeroes = 0;
    if (ints)
        {
        for (uint32_t i = 0; i < numInts; ++i)
            {
            auto val = ints[i];
            if (min > val)
                min = val;
            if (max < val)
                max = val;
            if (0 == val)
                ++numZeroes;
            }
        }
    return numZeroes;
    }

// Examine int array for zero count
static uint32_t CountZeroes(int32_t const* ints, uint32_t numInts)
    {
    uint32_t numZeroes = 0;
    if (ints)
        {
        for (uint32_t i = 0; i < numInts; ++i)
            {
            auto val = ints[i];
            if (0 == val)
                ++numZeroes;
            }
        }
    return numZeroes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert 0-based indices to 1-based indices with blocking specified by another index array.
* @param destIndices 1-based output indices with same blocking as blockingIndices
* @param sourceIndices 0-based source indices. This array is compressed (has no blocking)
* @param blockingIndices 1-based source indices, with blocking specified by zeroes. Assumed to have length equal to its zero count plus `sourceIndices.length`.
* @param announceIndex callback to receive a 1-based (positive) index
* @param announceZero callback to receive a 0 terminator/pad
+---------------+---------------+---------------+---------------+---------------+------*/
static void WriteOneBasedIndicesFromZeroBasedIndicesWithExternalBlocking(bvector<int32_t>& destIndices, int32_t const* sourceIndices, uint32_t numSourceIndices, int32_t const* blockingIndices, uint32_t numBlockingIndices)
    {
    if (!sourceIndices || !blockingIndices || !numSourceIndices || !numBlockingIndices)
        return;
    auto blockingZeroCount = CountZeroes(blockingIndices, numBlockingIndices);
    if (numSourceIndices + blockingZeroCount != numBlockingIndices)
      return; // invalid input
    uint32_t iSource = 0;
    for (uint32_t iBlocking = 0; iBlocking < numBlockingIndices && iSource < numSourceIndices; iBlocking++)
        {
        if (!blockingIndices[iBlocking])
            destIndices.push_back(0);
        else
            destIndices.push_back(sourceIndices[iSource++] + 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Extract auxData for a mesh.
* Native object format for Polyface/PolyfaceAuxData indices is 1-based, 0-terminated/padded.
* FlatBuffer format for Polyface/PolyfaceAuxData indices is 1-based, 0-terminated/padded.
* Typescript API previously wrote FlatBuffer PolyfaceAuxData indices as 0-based, unterminated;
* heuristics are used herein to identify this legacy format so it can still be read.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PolyfaceAuxDataPtr ReadPolyfaceAuxData(const BGFB::Polyface* fbPolyface, const BGFB::PolyfaceAuxData* fbAuxData)
    {
    if (!fbPolyface || !fbAuxData)
        return nullptr;

    auto fbPointIndices = fbPolyface->pointIndex();
    auto fbAuxIndices = fbAuxData->indices();
    auto fbChannels = fbAuxData->channels();
    auto numChannels = fbChannels ? fbChannels->size() : 0;
    auto fbNumData = ChannelDataLength(*fbAuxData);
    if (!fbPointIndices || !fbPointIndices->size() || !fbAuxIndices || !fbAuxIndices->size() || !numChannels || !fbNumData)
        return nullptr;

    auto numPerFace = fbPolyface->numPerFace();

    // HEURISTICS to identify legacy AuxData indices, mistakenly serialized by Typescript API as 0-based and unterminated
    auto isLegacy = false;
    auto pointIndicesPadCount = CountZeroes((int32_t const*) fbPointIndices->GetStructFromOffset(0), fbPointIndices->size());
    if (numPerFace > 1)
        {
        int32_t auxIndexMin, auxIndexMax;
        auto auxIndexNumZeroes = CountZeroes((int32_t const*) fbAuxIndices->GetStructFromOffset(0), fbAuxIndices->size(), auxIndexMin, auxIndexMax);
        if (auxIndexMax > 0 && (uint32_t) auxIndexMax > fbNumData) // auxIndices invalid
            return nullptr;
        if (auxIndexMax == fbNumData) // auxIndices 1-based
            isLegacy = false;
        else if (auxIndexMax <= 0 || auxIndexMin < 0) // auxIndices 1-based (signed)
            isLegacy = false;
        else if (auxIndexMin == 0) // auxIndices likely legacy 0-based index, but could be modern with padding
            isLegacy = pointIndicesPadCount != auxIndexNumZeroes;
        else if (auxIndexMin > 0) // auxIndices likely modern without padding, but could be legacy if first datum not indexed
            isLegacy = pointIndicesPadCount > 0;
        }
    else
        {
        isLegacy = (fbAuxIndices->size() < fbPointIndices->size()) && (fbAuxIndices->size() + pointIndicesPadCount == fbPointIndices->size());
        }
    if (!isLegacy && fbAuxIndices->size() != fbPointIndices->size())
      return nullptr; // auxIndices invalid

    bvector<int32_t> indices;
    if (isLegacy)
        WriteOneBasedIndicesFromZeroBasedIndicesWithExternalBlocking(indices, (int32_t const*) fbAuxIndices->GetStructFromOffset(0), fbAuxIndices->size(), (int32_t const*) fbPointIndices->GetStructFromOffset(0), fbPointIndices->size());
    else
        {
        indices.resize(fbAuxIndices->size());
        memcpy(indices.data(), fbAuxIndices->GetStructFromOffset(0), fbAuxIndices->size() * sizeof(int32_t));
        }
    if (indices.size() != fbPointIndices->size())
        return nullptr;

    PolyfaceAuxData::Channels channels;
    for (unsigned int i = 0; i < numChannels; i++)
        {
        auto    fbChannel = fbChannels->Get(i);
        auto    fbChannelDataVector = fbChannel->data();
        bvector<PolyfaceAuxChannel::DataPtr>  channelDataVector;

        for (unsigned int j=0; j<fbChannelDataVector->Length(); j++)
            {
            auto            fbChannelData = fbChannelDataVector->Get(j);
            auto            fbChannelDataValues = fbChannelData->values();
            bvector<double> values(fbChannelDataValues->Length());

            memcpy (values.data(), fbChannelDataValues->GetStructFromOffset(0), fbChannelDataValues->Length() * sizeof(double));
            channelDataVector.push_back(new PolyfaceAuxChannel::Data(fbChannelData->input(), std::move(values)));
            }
        channels.push_back(new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType(fbChannel->dataType()), fbChannel->name()->c_str(), fbChannel->inputName()->c_str(), std::move(channelDataVector)));
        }

    return new PolyfaceAuxData(std::move(indices), std::move(channels));
    }

template <typename MemberType>
static void ReadBVector(bvector<MemberType> &dest, const MemberType *source, size_t n)
    {
    dest.resize(n);
    memcpy(&dest[0], source, n * sizeof(MemberType));
    }

template <typename MemberType, typename ScalarType>
static void ReadBVector(bvector<MemberType> &dest, uint32_t numPerMember, const ScalarType*source, size_t n)
    {
    dest.resize(n / numPerMember);
    if (n < numPerMember)
        return;
    memcpy((ScalarType*) &dest[0], source, n * sizeof(ScalarType));
    }

// read from fb into taggedDataDest (which is assumed empty/initialized)
static void ReadTaggedNumericData(
    const BGFB::TaggedNumericData *fbTaggedNumericData,
    TaggedNumericData &taggedDataDest)
    {
    taggedDataDest.m_tagA = fbTaggedNumericData->tagA();
    taggedDataDest.m_tagB = fbTaggedNumericData->tagB();
    auto fbIntData = fbTaggedNumericData->intData ();
    auto fbDoubleData = fbTaggedNumericData->doubleData();

    if (fbIntData != nullptr)
        ReadBVector<int32_t> (taggedDataDest.m_intData, (const int32_t*)fbIntData->GetStructFromOffset(0), fbIntData->Length ());
    if (fbDoubleData != nullptr)
        ReadBVector<double>(taggedDataDest.m_doubleData, (const double*)fbDoubleData->GetStructFromOffset(0), fbDoubleData->Length());
    }


template <typename BlockedVectorType, typename StructType>
static void LoadBlockedVector (BlockedVectorType dest, StructType const*source, size_t n)
    {
    dest.SetActive (true);
    dest.resize (n);
    memcpy (&dest[0], source, n * sizeof (StructType));
    }

static PolyfaceHeaderPtr ReadPolyfaceHeader (const BGFB::VariantGeometry * fbGeometry)
    {
    if (!fbGeometry)
        return nullptr;
    if (BGFB::VariantGeometryUnion_Polyface != fbGeometry->geometry_type ())
        return nullptr;
    auto fbPolyface = reinterpret_cast <const BGFB::Polyface *> (fbGeometry->geometry ());
    return ReadPolyfaceHeaderDirect (fbPolyface);
    }


static PolyfaceHeaderPtr ReadPolyfaceHeaderDirect (const BGFB::Polyface *fbPolyface)
    {
    if (!fbPolyface)
        return nullptr;

    uint32_t numPerFace   = (uint32_t)fbPolyface->numPerFace ();
    int numPerRow       = fbPolyface->numPerRow ();
    uint32_t meshStyle    = (uint32_t)fbPolyface->meshStyle ();
    bool twoSided       = 0 != fbPolyface->twoSided ();

    uint32_t expectedClosure = (uint32_t)fbPolyface->expectedClosure();
    if (expectedClosure >= 3)
        expectedClosure = 0;

    PolyfaceHeaderPtr polyface = PolyfaceHeader::New ();
    polyface->ClearTags (numPerFace, meshStyle);
    polyface->SetNumPerRow (numPerRow);
    polyface->SetTwoSided (twoSided);
    polyface->SetExpectedClosure(expectedClosure);

    if (fbPolyface->has_point ())
        {
        auto fbPoints = fbPolyface->point ();
        LoadBlockedVector<BlockedVectorDPoint3dR, DPoint3d> (
                polyface->Point (),
                ((DPoint3dCP)fbPoints->GetStructFromOffset(0)),
                (size_t)(fbPoints->Length () / 3)
                );
        }

    if (fbPolyface->has_param ())
        {
        auto fbParams = fbPolyface->param ();
        LoadBlockedVector<BlockedVectorDPoint2dR, DPoint2d> (
                polyface->Param (),
                ((DPoint2dCP)fbParams->GetStructFromOffset(0)),
                (size_t)(fbParams->Length () / 2)
                );
        }

    if (fbPolyface->has_normal ())
        {
        auto fbNormals = fbPolyface->normal ();
        LoadBlockedVector<BlockedVectorDVec3dR, DVec3d> (
                polyface->Normal (),
                ((DVec3dCP)fbNormals->GetStructFromOffset(0)),
                (size_t)(fbNormals->Length () / 3)
                );
        }


    if (fbPolyface->has_pointIndex ())
        {
        auto fbData = fbPolyface->pointIndex ();
        LoadBlockedVector<BlockedVectorIntR, int> (
                polyface->PointIndex (),
                (int const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }

    if (fbPolyface->has_paramIndex ())
        {
        auto fbData = fbPolyface->paramIndex ();
        LoadBlockedVector<BlockedVectorIntR, int> (
                polyface->ParamIndex (),
                (int const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }

    if (fbPolyface->has_normalIndex ())
        {
        auto fbData = fbPolyface->normalIndex ();
        LoadBlockedVector<BlockedVectorIntR, int> (
                polyface->NormalIndex (),
                (int const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }

    if (fbPolyface->has_colorIndex ())
        {
        auto fbData = fbPolyface->colorIndex ();
        LoadBlockedVector<BlockedVectorIntR, int> (
                polyface->ColorIndex (),
                (int const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }
    if (fbPolyface->has_faceIndex ())
        {
        auto fbData = fbPolyface->faceIndex ();
        LoadBlockedVector<BlockedVectorIntR, int> (
                polyface->FaceIndex (),
                (int const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }

    if (fbPolyface->has_faceData ())
        {
        auto fbFaceData = fbPolyface->faceData ();
        auto pFaceDataDoubles = (double const*)fbFaceData->GetStructFromOffset(0);
        if (unpackFaceData (pFaceDataDoubles, (size_t)fbFaceData->Length (), polyface->FaceData ()))
            {
            polyface->FaceData().SetTags (1,1,0,0,0, true);
            }
        }

    if (fbPolyface->has_intColor ())
        {
        auto fbData = fbPolyface->intColor();
        LoadBlockedVector<BlockedVectorUInt32R, uint32_t> (
                polyface->IntColor (),
                (uint32_t const*)fbData->GetStructFromOffset(0),
                (size_t)fbData->Length ()
                );
        }

    if (fbPolyface->has_auxData())
        polyface->AuxData() = ReadPolyfaceAuxData(fbPolyface, fbPolyface->auxData());

    if (fbPolyface->has_taggedNumericData())
        {
        TaggedNumericData numericTags;
        ReadTaggedNumericData (fbPolyface->taggedNumericData (), numericTags);
        polyface->SetNumericTags (numericTags);
        }
    return polyface;
    }

// Fill the pointers in a carrier.
static bool ReadPolyfaceQueryCarrierDirect (const BGFB::Polyface *fbPolyface, PolyfaceQueryCarrier &carrier)
    {
    if (!fbPolyface)
        return false;

    uint32_t numPerFace   = (uint32_t)fbPolyface->numPerFace ();
    int numPerRow       = fbPolyface->numPerRow ();
    uint32_t meshStyle    = (uint32_t)fbPolyface->meshStyle ();
    bool twoSided       = 0 != fbPolyface->twoSided ();

    uint32_t expectedClosure = (uint32_t)fbPolyface->expectedClosure();
    if (expectedClosure >= 3)
        expectedClosure = 0;

    size_t numPoint = 0, numParam = 0, numNormal = 0, numColor = 0, numFace = 0;
    size_t numPointIndex = 0, numParamIndex = 0, numNormalIndex = 0, numColorIndex = 0, numFaceIndex = 0;
    int32_t const * pPointIndex = nullptr;
    int32_t const * pNormalIndex = nullptr;
    int32_t const * pParamIndex = nullptr;
    int32_t const * pColorIndex = nullptr;
    int32_t const * pFaceIndex = nullptr;

    DPoint3dCP pPoints = nullptr;
    DPoint2dCP pParams = nullptr;
    DVec3dCP   pNormals = nullptr;
    FacetFaceDataCP pFaceData = nullptr;
    uint32_t const* pIntColor = nullptr;

    // Blocked vectors need non-zero numPerRow ...
    if (numPerRow < 1)
        numPerRow = 1;

    if (fbPolyface->has_point ())
        {
        auto fbPoints = fbPolyface->point ();
        pPoints = (DPoint3dCP)fbPoints->GetStructFromOffset(0);
        numPoint = (size_t)(fbPoints->Length () / 3);
        }

    if (fbPolyface->has_param ())
        {
        auto fbParams = fbPolyface->param ();
        pParams = (DPoint2dCP)fbParams->GetStructFromOffset(0);
        numParam = (size_t)(fbParams->Length () / 2);
        }

    if (fbPolyface->has_normal ())
        {
        auto fbNormals = fbPolyface->normal ();
        pNormals = (DVec3dCP)fbNormals->GetStructFromOffset(0);
        numNormal = (size_t)(fbNormals->Length () / 3);
        }

    if (fbPolyface->has_pointIndex ())
        {
        auto fbData = fbPolyface->pointIndex ();
        pPointIndex = (int const*)fbData->GetStructFromOffset(0);
        numPointIndex = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_paramIndex ())
        {
        auto fbData = fbPolyface->paramIndex ();
        pParamIndex = (int const*)fbData->GetStructFromOffset(0);
        numParamIndex = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_normalIndex ())
        {
        auto fbData = fbPolyface->normalIndex ();
        pNormalIndex = (int const*)fbData->GetStructFromOffset(0);
        numNormalIndex = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_colorIndex ())
        {
        auto fbData = fbPolyface->colorIndex ();
        pColorIndex = (int const*)fbData->GetStructFromOffset(0);
        numColorIndex = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_intColor ())
        {
        auto fbData = fbPolyface->intColor();
        pIntColor = (uint32_t const*)fbData->GetStructFromOffset(0);
        numColor = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_faceIndex ())
        {
        auto fbData = fbPolyface->faceIndex ();
        pFaceIndex = (int const*)fbData->GetStructFromOffset(0);
        numFaceIndex = (size_t)fbData->Length ();
        }

    if (fbPolyface->has_faceData ())
        {
        auto fbData = fbPolyface->faceData();
        pFaceData = (FacetFaceDataCP)fbData->GetStructFromOffset(0);
        numFace = (size_t)fbData->Length () / 8;
        }

    if (numParamIndex > 0 && numParamIndex != numPointIndex)
        return false;
    if (numNormalIndex > 0 && numNormalIndex != numPointIndex)
        return false;
    if (numColorIndex > 0 && numColorIndex != numPointIndex)
        return false;
    if (numFaceIndex > 0 && numFaceIndex != numPointIndex)
        return false;

    carrier = PolyfaceQueryCarrier (
        numPerFace, twoSided, numPointIndex,
        numPoint,  pPoints, pPointIndex,
        numNormal, pNormals, pNormalIndex,
        numParam,  pParams, pParamIndex,
        numColor,  pColorIndex, pIntColor, nullptr,
        meshStyle, numPerRow, expectedClosure
        );

    carrier.SetFacetFaceData (pFaceData, numFace);
    carrier.SetFaceIndex (pFaceIndex);
    if (fbPolyface->has_auxData())
        {
        PolyfaceAuxDataCPtr  auxData = ReadPolyfaceAuxData(fbPolyface, fbPolyface->auxData());
        carrier.SetAuxData(auxData);
        }

    if (fbPolyface->has_taggedNumericData())
        {
        TaggedNumericData numericTags;
        ReadTaggedNumericData(fbPolyface->taggedNumericData(), numericTags);
        carrier.SetNumericTags(numericTags);
        }

    return true;
    }



static ICurvePrimitivePtr ReadCurvePrimitive (const BGFB::VariantGeometry * fbGeometry)
    {
    if (!fbGeometry)
        return nullptr;
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_LineSegment:
            {
            BGFB::LineSegment const *fbLineSegment
                = reinterpret_cast <const BGFB::LineSegment *> (fbGeometry->geometry ());
            if (!fbLineSegment)
                return nullptr;
            BGFB::DSegment3d const *fbStruct = fbLineSegment->segment ();
            if (!fbStruct)
                return nullptr;
            return ICurvePrimitive::CreateLine
                    (DSegment3d::From (
                        fbStruct->point0X (),
                        fbStruct->point0Y (),
                        fbStruct->point0Z (),
                        fbStruct->point1X (),
                        fbStruct->point1Y (),
                        fbStruct->point1Z ()
                        ));
            }

        case BGFB::VariantGeometryUnion_EllipticArc:
            {
            BGFB::EllipticArc const *fbEllipticArc
                = reinterpret_cast <const BGFB::EllipticArc *> (fbGeometry->geometry ());
            if (!fbEllipticArc)
                return nullptr;
            BGFB::DEllipse3d const *fbStruct = fbEllipticArc->arc ();
            if (!fbStruct)
                return nullptr;

	    // Patch for bad VUE data.
	    auto sweepRadians = fbStruct->sweepRadians();
	    if (std::isnan(sweepRadians))
		    sweepRadians = Angle::DegreesToRadians(360.0);

            return ICurvePrimitive::CreateArc
                    (DEllipse3d::From (
                        fbStruct->centerX (),
                        fbStruct->centerY (),
                        fbStruct->centerZ (),
                        fbStruct->vector0X (),
                        fbStruct->vector0Y (),
                        fbStruct->vector0Z (),
                        fbStruct->vector90X (),
                        fbStruct->vector90Y (),
                        fbStruct->vector90Z (),
                        fbStruct->startRadians (),
                        sweepRadians
                        ));
            }

        case BGFB::VariantGeometryUnion_LineString:
            {
            BGFB::LineString const *fbLineString
                = reinterpret_cast <const BGFB::LineString *> (fbGeometry->geometry ());
            if (!fbLineString)
                return nullptr;
            auto fbPoints = fbLineString->points ();
            if (!fbPoints)
                return nullptr;
            size_t numDoubles = (size_t)fbPoints->Length ();
            size_t numPoints = numDoubles /3;
            return ICurvePrimitive::CreateLineString (
                    (DPoint3dCP)fbPoints->GetStructFromOffset(0), numPoints);
            }
        case BGFB::VariantGeometryUnion_PointString:
            {
            BGFB::PointString const *fbPointString
                = reinterpret_cast <const BGFB::PointString *> (fbGeometry->geometry ());
            if (!fbPointString)
                return nullptr;
            auto fbPoints = fbPointString->points ();
            if (!fbPoints)
                return nullptr;
            size_t numDoubles = (size_t)fbPoints->Length ();
            size_t numPoints = numDoubles /3;
            return ICurvePrimitive::CreatePointString (
                    (DPoint3dCP)fbPoints->GetStructFromOffset(0), numPoints);
            }

        case BGFB::VariantGeometryUnion_BsplineCurve:
            {
            BGFB::BsplineCurve const *fbBsplineCurve
                = reinterpret_cast <const BGFB::BsplineCurve *> (fbGeometry->geometry ());
            if (!fbBsplineCurve)
                return nullptr;
            int  order = fbBsplineCurve->order ();
            bool closed = fbBsplineCurve->closed () != 0;
            auto fbPoles = fbBsplineCurve->poles ();
            auto fbKnots = fbBsplineCurve->knots ();
            auto fbWeights = fbBsplineCurve->weights ();
            if (!fbPoles || !fbKnots)
                return nullptr;

            int numPoles = fbPoles->Length() / 3;
            DPoint3dCP pPoles = numPoles > 0 ? (DPoint3dCP)fbPoles->GetStructFromOffset(0) : nullptr;

            int numKnots = fbKnots->Length ();
            double const * pKnots = numKnots > 0 ? (double const*)fbKnots->GetStructFromOffset(0) : nullptr;

            int numWeights = fbWeights ? fbWeights->Length () : 0;
            double const * pWeights = numWeights > 0 ? (double const*)fbWeights->GetStructFromOffset(0) : nullptr;

            MSBsplineCurve curve;
            if (curve.Populate (pPoles, pWeights, numPoles, pKnots, numKnots, order, closed, true) != MSB_SUCCESS)
                return nullptr;
            return ICurvePrimitive::CreateBsplineCurveSwapFromSource (curve);
            }
        case BGFB::VariantGeometryUnion_InterpolationCurve:
            {
            BGFB::InterpolationCurve const *fbInterpolationCurve
                = reinterpret_cast <const BGFB::InterpolationCurve *> (fbGeometry->geometry ());
            if (!fbInterpolationCurve)
                return nullptr;
            int  order = fbInterpolationCurve->order ();
            bool closed = fbInterpolationCurve->closed () != 0;
            auto fbFitPoints = fbInterpolationCurve->fitPoints ();
            auto fbKnots = fbInterpolationCurve->knots ();
            auto fbStartTangent = fbInterpolationCurve->startTangent ();
            auto fbEndTangent = fbInterpolationCurve->endTangent ();
            if (!fbFitPoints)
                return nullptr;
            int numPoles = fbFitPoints->Length() / 3;
            DPoint3dCP pFitPoints = numPoles > 0 ? (DPoint3dCP)fbFitPoints->GetStructFromOffset(0) : nullptr;

            DVec3d startTangent, endTangent;
            startTangent.Zero ();
            endTangent.Zero ();
            if (nullptr != fbStartTangent)
                startTangent = DVec3d::From (fbStartTangent->x (), fbStartTangent->y (), fbStartTangent->z ());
            if (nullptr != fbEndTangent)
                endTangent = DVec3d::From (fbEndTangent->x (), fbEndTangent->y (), fbEndTangent->z ());

            int numKnots = fbKnots ? fbKnots->Length() : 0;
            double const * pKnots = numKnots > 0 ? (double const*)fbKnots->GetStructFromOffset(0) : nullptr;

            MSInterpolationCurve curve;
            if (curve.Populate(
                    order,
                    closed,
                    fbInterpolationCurve->isChordLenKnots (),
                    fbInterpolationCurve->isColinearTangents (),
                    fbInterpolationCurve->isChordLenTangents (),
                    fbInterpolationCurve->isNaturalTangents (),
                    pFitPoints,
                    numPoles,
                    pKnots,
                    numKnots,
                    &startTangent,
                    &endTangent
            ) != MSB_SUCCESS)
                    return nullptr;
            return ICurvePrimitive::CreateInterpolationCurveSwapFromSource (curve);
            }

        case BGFB::VariantGeometryUnion_AkimaCurve:
            {
            BGFB::AkimaCurve const *fbAkimaCurve
                = reinterpret_cast <const BGFB::AkimaCurve *> (fbGeometry->geometry ());
            if (!fbAkimaCurve)
                return nullptr;
            auto fbPoints = fbAkimaCurve->points ();
            if (!fbPoints)
                return nullptr;
            int numPoles = fbPoints->Length () / 3;
            DPoint3dCP pPoints = numPoles > 0 ? (DPoint3dCP)fbPoints->GetStructFromOffset(0) : nullptr;
            return ICurvePrimitive::CreateAkimaCurve (pPoints, numPoles);
            }


        case BGFB::VariantGeometryUnion_TransitionSpiral:
            {
            BGFB::TransitionSpiral const *fbTransitionSpiral
                = reinterpret_cast <const BGFB::TransitionSpiral *> (fbGeometry->geometry ());
            if (!fbTransitionSpiral)
                return nullptr;
            BGFB::TransitionSpiralDetail const *detail = fbTransitionSpiral->detail ();
            if (!detail)
                return nullptr;
            Transform frame = *(Transform const*)&detail->transform ();

            return ICurvePrimitive::CreateSpiralBearingCurvatureBearingCurvature (
                    detail->spiralType (),
                    detail->bearing0Radians (),
                    detail->curvature0 (),
                    detail->bearing1Radians (),
                    detail->curvature1 (),
                    frame,
                    detail->fractionA (),
                    detail->fractionB ()
                    );
            }

        case BGFB::VariantGeometryUnion_CatenaryCurve:
            {
            BGFB::CatenaryCurve const *fbCatenaryCurve
                = reinterpret_cast <const BGFB::CatenaryCurve *> (fbGeometry->geometry ());
            if (!fbCatenaryCurve)
                return nullptr;
            auto fbOrigin = fbCatenaryCurve->origin ();
            auto fbVectorU = fbCatenaryCurve->vectorU ();
            auto fbVectorV = fbCatenaryCurve->vectorV ();
            if (!fbOrigin || !fbVectorU || !fbVectorV)
                return nullptr;
            return ICurvePrimitive::CreateCatenary (
                    fbCatenaryCurve->a(),
                    DPoint3dDVec3dDVec3d (
                        DPoint3d::From (fbOrigin->x (), fbOrigin->y (), fbOrigin->z ()),
                        DVec3d::From (fbVectorU->x (), fbVectorU->y (), fbVectorU->z ()),
                        DVec3d::From (fbVectorV->x (), fbVectorV->y (), fbVectorV->z ())),
                    fbCatenaryCurve->x0 (),
                    fbCatenaryCurve->x1 ()
                    );
            }

        case BGFB::VariantGeometryUnion_PartialCurve:
            {
            BGFB::PartialCurve const *fbPartialCurve
                = reinterpret_cast <const BGFB::PartialCurve*> (fbGeometry->geometry ());
            if (!fbPartialCurve)
                return nullptr;
            double fraction0 = fbPartialCurve->fraction0 ();
            double fraction1 = fbPartialCurve->fraction1 ();
            ICurvePrimitivePtr target = ReadCurvePrimitive (fbPartialCurve->target ());
            return ICurvePrimitive::CreatePartialCurve (target.get (), fraction0, fraction1);
            }
        }
    return nullptr;
    }

static MSBsplineSurfacePtr ReadMSBsplineSurface (const BGFB::VariantGeometry * fbGeometry)
    {
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_BsplineSurface:
            {
            BGFB::BsplineSurface const *fbBsplineSurface
                = reinterpret_cast <const BGFB::BsplineSurface *> (fbGeometry->geometry ());
            if (!fbBsplineSurface)
                return nullptr;
            int  orderU = fbBsplineSurface->orderU ();
            int  orderV = fbBsplineSurface->orderV ();
            int  numPolesU = fbBsplineSurface->numPolesU ();
            int  numPolesV = fbBsplineSurface->numPolesV ();
            int  numRulesU = fbBsplineSurface->numRulesU ();
            int  numRulesV = fbBsplineSurface->numRulesV ();
            bool closedU = fbBsplineSurface->closedU () != 0;
            bool closedV = fbBsplineSurface->closedV () != 0;
            auto fbPoles = fbBsplineSurface->poles ();
            auto fbKnotsU = fbBsplineSurface->knotsU ();
            auto fbKnotsV = fbBsplineSurface->knotsV ();
            auto fbWeights = fbBsplineSurface->weights ();
            int holeOrigin = fbBsplineSurface->holeOrigin ();

            if (!fbPoles || !fbKnotsU || !fbKnotsV)
                return nullptr;

            int numPoles = fbPoles->Length () / 3;
            bvector<DPoint3d> poles;
            if (numPoles > 0)
                {
                DPoint3dCP pData = (DPoint3dCP)fbPoles->GetStructFromOffset(0);
                for (int i = 0; i < numPoles; i++)
                    poles.push_back (pData[i]);
                }

            int numKnotsU = fbKnotsU->Length ();
            bvector<double>knotsU, knotsV, weights;
            if (numKnotsU > 0)
                {
                double const * pData = (double const*)fbKnotsU->GetStructFromOffset(0);
                for (int i = 0; i < numKnotsU; i++)
                    knotsU.push_back (pData[i]);
                }

            int numKnotsV = fbKnotsV->Length ();
            if (numKnotsV > 0)
                {
                double const * pData = (double const*)fbKnotsV->GetStructFromOffset(0);
                for (int i = 0; i < numKnotsV; i++)
                    knotsV.push_back (pData[i]);
                }


            int numWeights = (fbWeights  == nullptr) ? 0 : fbWeights->Length ();
            if (numWeights > 0)
                {
                double const * pData = (double const*)fbWeights->GetStructFromOffset(0);
                for (int i = 0; i < numWeights; i++)
                    weights.push_back (pData[i]);
                }

            auto fbBoundaries = fbBsplineSurface->boundaries ();
            CurveVectorPtr boundaries = ReadCurveVectorDirect (fbBoundaries);

            MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder (
                        poles, &weights,
                        &knotsU, orderU, numPolesU, closedU,
                        &knotsV, orderV, numPolesV, closedV,
                        true);
            if (surface.IsValid ())
                {
                    {
                    surface->SetNumRules (numRulesU, numRulesV);
                    if (boundaries.IsValid ())
                        surface->SetTrim (*boundaries);
                    surface->SetOuterBoundaryActive (holeOrigin == 0);
                    }
                }
            return surface;
            }
        }
    return nullptr;
    }

static ISolidPrimitivePtr ReadSolidPrimitive (const BGFB::VariantGeometry * fbGeometry)
    {
    if (!fbGeometry)
        return nullptr;
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_DgnBox:
            {
            BGFB::DgnBox const *fbDgnBox
                = reinterpret_cast <const BGFB::DgnBox *> (fbGeometry->geometry ());
            if (!fbDgnBox)
                return nullptr;
            BGFB::DgnBoxDetail const *fbDetail = fbDgnBox->detail ();
            if (!fbDetail)
                return nullptr;
            return ISolidPrimitive::CreateDgnBox (*(DgnBoxDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnCone:
            {
            BGFB::DgnCone const *fbDgnCone
                = reinterpret_cast <const BGFB::DgnCone *> (fbGeometry->geometry ());
            if (!fbDgnCone)
                return nullptr;
            BGFB::DgnConeDetail const *fbDetail = fbDgnCone->detail ();
            if (!fbDetail)
                return nullptr;
            return ISolidPrimitive::CreateDgnCone (*(DgnConeDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnSphere:
            {
            BGFB::DgnSphere const *fbDgnSphere
                = reinterpret_cast <const BGFB::DgnSphere *> (fbGeometry->geometry ());
            if (!fbDgnSphere)
                return nullptr;
            BGFB::DgnSphereDetail const *fbDetail = fbDgnSphere->detail ();
            if (!fbDetail)
                return nullptr;
            return ISolidPrimitive::CreateDgnSphere (*(DgnSphereDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnTorusPipe:
            {
            BGFB::DgnTorusPipe const *fbDgnTorusPipe
                = reinterpret_cast <const BGFB::DgnTorusPipe *> (fbGeometry->geometry ());
            if (!fbDgnTorusPipe)
                return nullptr;
            BGFB::DgnTorusPipeDetail const *fbDetail = fbDgnTorusPipe->detail ();
            if (!fbDetail)
                return nullptr;
            return ISolidPrimitive::CreateDgnTorusPipe (*(DgnTorusPipeDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnExtrusion:
            {
            BGFB::DgnExtrusion const *fbDgnExtrusion
                = reinterpret_cast <const BGFB::DgnExtrusion *> (fbGeometry->geometry ());
            if (!fbDgnExtrusion)
                return nullptr;
            CurveVectorPtr baseCurve = ReadCurveVectorDirect (fbDgnExtrusion->baseCurve ());
            if (baseCurve.IsNull())
                return nullptr;
            DgnExtrusionDetail detail (baseCurve,
                    *(DVec3dCP)fbDgnExtrusion->extrusionVector (),
                    0 != fbDgnExtrusion->capped ()
                    );
            return ISolidPrimitive::CreateDgnExtrusion (detail);
            }
        case BGFB::VariantGeometryUnion_DgnRotationalSweep:
            {
            BGFB::DgnRotationalSweep const *fbDgnRotationalSweep
                = reinterpret_cast <const BGFB::DgnRotationalSweep *> (fbGeometry->geometry ());
            if (!fbDgnRotationalSweep)
                return nullptr;
            CurveVectorPtr baseCurve = ReadCurveVectorDirect (fbDgnRotationalSweep->baseCurve ());
            if (baseCurve.IsNull())
                return nullptr;
            DRay3d axis = *(DRay3dCP)fbDgnRotationalSweep->axis ();
            DgnRotationalSweepDetail detail (baseCurve,
                    axis.origin,
                    axis.direction,
                    fbDgnRotationalSweep->sweepRadians (),
                    0 != fbDgnRotationalSweep->capped ()
                    );
            detail.m_numVRules = fbDgnRotationalSweep->numVRules ();
            return ISolidPrimitive::CreateDgnRotationalSweep (detail);
            }
        case BGFB::VariantGeometryUnion_DgnRuledSweep:
            {
            BGFB::DgnRuledSweep const *fbDgnRuledSweep
                = reinterpret_cast <const BGFB::DgnRuledSweep *> (fbGeometry->geometry ());
            if (!fbDgnRuledSweep)
                return nullptr;
            auto fbChildren = fbDgnRuledSweep->curves ();
            if (!fbChildren)
                return nullptr;
            int numCurves = fbChildren->Length ();
            bvector<CurveVectorPtr> sectionCurves;
            for (int i = 0; i < numCurves; i++)
                {
                sectionCurves.push_back (ReadCurveVectorDirect (fbChildren->Get(i)));
                }
            DgnRuledSweepDetail detail (sectionCurves,
                    0 != fbDgnRuledSweep->capped ()
                    );
            return ISolidPrimitive::CreateDgnRuledSweep (detail);
            }

        }
    return nullptr;
    }

static CurvePrimitiveIdPtr ReadCurvePrimitiveId (const BGFB::CurvePrimitiveId *fbTag)
    {
    if (fbTag != nullptr)
        {
        auto tagType = (CurvePrimitiveId::Type)fbTag->type ();
        auto geomIndex  = fbTag->geomIndex ();
        auto partIndex  = fbTag->partIndex ();
        if (fbTag->has_bytes ())
            {
            auto bytes = fbTag->bytes ();
            auto numBytes = bytes->size ();
            bvector<uint8_t> buffer;
            for (unsigned int i = 0; i < numBytes; i++)
                buffer.push_back (bytes->Get (i));
            if (numBytes > 0)
                return CurvePrimitiveId::Create (tagType, &buffer[0], numBytes, geomIndex, partIndex);
            else
                return CurvePrimitiveId::Create (tagType, nullptr, 0, geomIndex, partIndex);
            }
        }
    return nullptr;
    }
static void AddCurvePrimitiveId (const BGFB::VariantGeometry *fbGeometry, IGeometryPtr geometry)
    {
    // only CurvePrimitive can accept the tag. (alas)   Don't bother reading it on others.
    auto cp = geometry->GetAsICurvePrimitive ();
    if (cp.IsValid ())
        {
        auto taggedInfo = ReadCurvePrimitiveId (fbGeometry->tag ());
        if (taggedInfo.IsValid ())
            cp->SetId (taggedInfo.get ());
        }
    }

static IGeometryPtr ReadGeometry (const BGFB::VariantGeometry * fbGeometry)
    {
    if (!fbGeometry)
        return nullptr;
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_LineSegment:
        case BGFB::VariantGeometryUnion_EllipticArc:
        case BGFB::VariantGeometryUnion_LineString:
        case BGFB::VariantGeometryUnion_BsplineCurve:
        case BGFB::VariantGeometryUnion_InterpolationCurve:
        case BGFB::VariantGeometryUnion_AkimaCurve:
        case BGFB::VariantGeometryUnion_TransitionSpiral:
        case BGFB::VariantGeometryUnion_PointString:
        case BGFB::VariantGeometryUnion_CatenaryCurve:
        case BGFB::VariantGeometryUnion_PartialCurve:
            {
            ICurvePrimitivePtr cp = ReadCurvePrimitive(fbGeometry);
            AddCurvePrimitiveId (fbGeometry, IGeometry::Create (cp));
            return cp.IsValid () ? IGeometry::Create (cp) : nullptr;
            }

        case BGFB::VariantGeometryUnion_BsplineSurface:
            {
            MSBsplineSurfacePtr surface = ReadMSBsplineSurface (fbGeometry);
            return surface.IsValid () ? IGeometry::Create (surface) : nullptr;
            }


        case BGFB::VariantGeometryUnion_CurveVector:
            {
            CurveVectorPtr cv = ReadCurveVector (fbGeometry);
            return cv.IsValid () ? IGeometry::Create (cv) : nullptr;
            }

        case BGFB::VariantGeometryUnion_DgnBox:
        case BGFB::VariantGeometryUnion_DgnCone:
        case BGFB::VariantGeometryUnion_DgnSphere:
        case BGFB::VariantGeometryUnion_DgnTorusPipe:
        case BGFB::VariantGeometryUnion_DgnExtrusion:
        case BGFB::VariantGeometryUnion_DgnRotationalSweep:
        case BGFB::VariantGeometryUnion_DgnRuledSweep:
            {
            ISolidPrimitivePtr sp = ReadSolidPrimitive(fbGeometry);
            return sp.IsValid () ? IGeometry::Create (sp) : nullptr;
            }



        case BGFB::VariantGeometryUnion_Polyface:
            {
            PolyfaceHeaderPtr pf = ReadPolyfaceHeader (fbGeometry);
            return pf.IsValid () ? IGeometry::Create (pf) : nullptr;
            }
        }
    return nullptr;
    }
static void ReadVariantGeometry(const BGFB::VariantGeometry * fbGeometry, bvector<IGeometryPtr> &dest)
    {
    if (fbGeometry == nullptr)
        return;
    if (fbGeometry->geometry_type() == BGFB::VariantGeometryUnion_VectorOfVariantGeometry)
        {
        FBReader::ReadVectorOfVariantGeometryDirect(
            reinterpret_cast <const BGFB::VectorOfVariantGeometry*>(fbGeometry->geometry()), dest);
        }
    else
        {
        IGeometryPtr g = FBReader::ReadGeometry(fbGeometry);
        if (g.IsValid())
            dest.push_back(g);
        }
    }
};

bool BentleyGeometryFlatBuffer::IsFlatBufferFormat(bvector<Byte> & buffer)
    {
    if (buffer.size() == 0)
        return false;
    Byte *fbStart = GetFBStart(buffer);
    return nullptr != fbStart;
    }

bool BentleyGeometryFlatBuffer::IsFlatBufferFormat(Byte const *buffer, size_t bufferSize)
    {
    if (nullptr == buffer || bufferSize == 0)
        return false;
    Byte const *fbStart = GetFBStart(buffer, bufferSize);
    return nullptr != fbStart;
    }

template <typename PtrType, typename ReaderMethodType>
PtrType BytesToXXXSafe(Byte const *buffer, size_t bufferSize, bool applyValidation, ReaderMethodType readerMethod)
    {
    if (nullptr == buffer)
        return nullptr;
    Byte const *fbStart = GetFBStart(buffer, bufferSize);
    if (nullptr == fbStart)
        return nullptr;
    auto fbRoot = flatbuffers::GetRoot<BGFB::VariantGeometry>(fbStart);
    if (nullptr == fbRoot)
        return nullptr;
    auto myVerifier = flatbuffers::Verifier(fbStart, bufferSize - s_prefixBufferSize);
    if (!fbRoot->Verify(myVerifier))
        return nullptr;
    if (fbRoot->has_geometry_type() && fbRoot->has_geometry())
        {
        PtrType result = readerMethod(fbRoot);
        if (result.IsValid())
            {
            if (applyValidation && s_readValidator.IsValid())
                {
                if (!result->IsValidGeometry(s_readValidator))
                    return nullptr;
                }
            return result;
            }
        }
    return nullptr;
    }

IGeometryPtr BentleyGeometryFlatBuffer::BytesToGeometrySafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<IGeometryPtr>(buffer, bufferSize, applyValidation, FBReader::ReadGeometry);
    }

IGeometryPtr BentleyGeometryFlatBuffer::BytesToGeometry(bvector<Byte> const&buffer, bool applyValidation)
    {
    return BytesToGeometrySafe(buffer.data(), buffer.size(), applyValidation);
    }

ISolidPrimitivePtr BentleyGeometryFlatBuffer::BytesToSolidPrimitiveSafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<ISolidPrimitivePtr>(buffer, bufferSize, applyValidation, FBReader::ReadSolidPrimitive);
    }

ICurvePrimitivePtr BentleyGeometryFlatBuffer::BytesToCurvePrimitiveSafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<ICurvePrimitivePtr>(buffer, bufferSize, applyValidation, FBReader::ReadCurvePrimitive);
    }

CurveVectorPtr BentleyGeometryFlatBuffer::BytesToCurveVectorSafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<CurveVectorPtr>(buffer, bufferSize, applyValidation, FBReader::ReadCurveVector);
    }

PolyfaceHeaderPtr BentleyGeometryFlatBuffer::BytesToPolyfaceHeaderSafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<PolyfaceHeaderPtr>(buffer, bufferSize, applyValidation, FBReader::ReadPolyfaceHeader);
    }

MSBsplineSurfacePtr BentleyGeometryFlatBuffer::BytesToMSBsplineSurfaceSafe(Byte const *buffer, size_t bufferSize, bool applyValidation)
    {
    return BytesToXXXSafe<MSBsplineSurfacePtr>(buffer, bufferSize, applyValidation, FBReader::ReadMSBsplineSurface);
    }

bool BentleyGeometryFlatBuffer::BytesToVectorOfGeometrySafe
(
    bvector<Byte> &buffer,
    bvector<IGeometryPtr> &dest,
    bool applyValidation,
    bvector<IGeometryPtr> *invalidGeometry
)
    {
    if (buffer.size() == 0)
        return false;
    Byte* fbStart = GetFBStart(buffer);
    if (nullptr == fbStart)
        return false;
    dest.clear ();
    if (invalidGeometry)
        invalidGeometry->clear ();
    auto fbRoot = flatbuffers::GetRoot<BGFB::VariantGeometry>(fbStart);
    if (nullptr == fbRoot)
        return false;
    auto myVerifier = flatbuffers::Verifier(fbStart, buffer.size() - s_prefixBufferSize);
    if (!fbRoot->Verify(myVerifier))
        return false;
    if (fbRoot->has_geometry_type() && fbRoot->has_geometry())
        {
        FBReader::ReadVariantGeometry(fbRoot, dest);
        if (applyValidation && s_readValidator.IsValid())
            GeometryValidator::RemoveInvalidGeometry(s_readValidator, dest, invalidGeometry);
        return dest.size() > 0;
        }
    return false;
    }

bool BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrierSafe
(
    Byte const *buffer,
    size_t bufferSize,
    PolyfaceQueryCarrier &carrier,
    bool applyValidation
)
    {
    if (nullptr == buffer)
        return false;
    Byte const *fbStart = GetFBStart(buffer, bufferSize);
    if (nullptr == fbStart)
        return false;
    auto fbRoot = flatbuffers::GetRoot<BGFB::VariantGeometry>(fbStart);
    if (nullptr == fbRoot)
        return false;
    auto myVerifier = flatbuffers::Verifier(fbStart, bufferSize - s_prefixBufferSize);
    if (!fbRoot->Verify(myVerifier))
        return false;
    if (fbRoot->has_geometry_type() && fbRoot->has_geometry())
        {
        if (BGFB::VariantGeometryUnion_Polyface != fbRoot->geometry_type())
            return false;
        auto result = FBReader::ReadPolyfaceQueryCarrierDirect (
                    reinterpret_cast <const BGFB::Polyface*>(fbRoot->geometry ()),
                    carrier);
        if (result && (!applyValidation || carrier.IsValidGeometry(s_readValidator)))
            return true;
        }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
