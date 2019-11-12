/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include "allcg_generated.h"
#define  BGFB Bentley::Geometry::FB

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void HelloBGFB ()
    {
//    BeConsole::Printf ("Hello BGFB\n");
    DPoint3d xyz = DPoint3d::From (0,sqrt(2.0),Angle::FromDegrees (90).Radians ());
    printf (" xyz (%g,%g,%g)\n", xyz.x, xyz.y, xyz.z);
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
static Byte const *GetFBStart (Byte const *allbytes)
    {
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

flatbuffers::Offset<BGFB::VectorOfVariantGeometry> WriteAsFBVectorOfVariantGeometry (bvector<IGeometryPtr> &source)
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

flatbuffers::Offset<BGFB::VariantGeometry> WriteAsVariantGeometry (bvector<IGeometryPtr> &source)
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
* @bsimethod                                                      Ray.Bentley      03/2018
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
* @bsimethod                                                      Ray.Bentley      03/2018
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
* @bsimethod                                                      Ray.Bentley      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::PolyfaceAuxData> WriteAsFBPolyfaceAuxData (PolyfaceAuxDataCP pAuxData)
    {
    if (nullptr == pAuxData)
        return 0;

    bvector<flatbuffers::Offset<BGFB::PolyfaceAuxChannel>> fbChannelVector;

    for (auto const& channel : pAuxData->GetChannels())
        fbChannelVector.push_back(WriteAsFBPolyfaceAuxChannel(channel.get()));

    auto    fbIndices = m_fbb.CreateVector(pAuxData->GetIndices());
    auto    fbChannels = m_fbb.CreateVector(fbChannelVector);

    BGFB::PolyfaceAuxDataBuilder builder (m_fbb);
    builder.add_indices(fbIndices);
    builder.add_channels(fbChannels);

    return builder.Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod        
+---------------+---------------+---------------+---------------+---------------+------*/
flatbuffers::Offset<BGFB::Polyface> WriteAsFBPolyface (PolyfaceHeaderR parent)
    {
    int32_t numPerFace = parent.GetNumPerFace ();
    int32_t numPerRow = parent.GetNumPerRow ();
    int32_t meshStyle = parent.GetMeshStyle ();
    bool    twoSided  = parent.GetTwoSided ();

    const flatbuffers::Offset<flatbuffers::Vector<double>> point = WriteOptionalVector<DPoint3d, double, 3>(parent.Point ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> param = WriteOptionalVector<DPoint2d, double, 2>(parent.Param ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> normal = WriteOptionalVector<DVec3d, double, 3>(parent.Normal ());
    const flatbuffers::Offset<flatbuffers::Vector<double>> faceData = WriteOptionalVector<FacetFaceData, double, 8>(parent.FaceData ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> pointIndex = WriteOptionalVector<int, int, 1>(parent.PointIndex ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> paramIndex = WriteOptionalVector<int, int, 1>(parent.ParamIndex ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> normalIndex = WriteOptionalVector<int, int, 1>(parent.NormalIndex ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> colorIndex = WriteOptionalVector<int, int, 1>(parent.ColorIndex ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> faceIndex = WriteOptionalVector<int, int, 1>(parent.FaceIndex ());
    const flatbuffers::Offset<flatbuffers::Vector<int32_t>> intColor = WriteOptionalVector<uint32_t, int, 1>(parent.IntColor ());
    const flatbuffers::Offset<BGFB::PolyfaceAuxData>        auxData = WriteAsFBPolyfaceAuxData(parent.GetAuxDataCP().get());

    BGFB::PolyfaceBuilder builder (m_fbb);
    builder.add_numPerFace (numPerFace);
    builder.add_numPerRow (numPerRow);
    builder.add_meshStyle (meshStyle);
    builder.add_twoSided (twoSided);
    
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

    return builder.Finish ();
    }

flatbuffers::Offset<BGFB::Polyface> WriteAsFBPolyface (PolyfaceQueryCR parent)
    {
    int32_t numPerFace = parent.GetNumPerFace ();
    int32_t numPerRow = parent.GetNumPerRow ();
    int32_t meshStyle = parent.GetMeshStyle ();
    bool    twoSided  = parent.GetTwoSided ();

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


    BGFB::PolyfaceBuilder builder (m_fbb);
    builder.add_numPerFace (numPerFace);
    builder.add_numPerRow (numPerRow);
    builder.add_meshStyle (meshStyle);
    builder.add_twoSided (twoSided);
    
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

    return builder.Finish ();
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
    FBWriter writer;
    flatbuffers::Offset<BGFB::VariantGeometry> g = writer.WriteAsFBVariantGeometry (geometry);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (PolyfaceQueryCR polyfaceQuery, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (polyfaceQuery);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (ICurvePrimitiveCR prim, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (prim);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (MSBsplineSurfaceCR source, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (ISolidPrimitiveCR source, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (CurveVectorCR source, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsFBVariantGeometry (&source);
    writer.FinishAndGetBuffer (g, buffer);
    }

void BentleyGeometryFlatBuffer::GeometryToBytes (bvector<IGeometryPtr> &source, bvector<Byte>& buffer)
    {
    FBWriter writer;
    auto g = writer.WriteAsVariantGeometry (source);    // FB is fine with empty or singleton array.
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
    return cvPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      Ray.Bentley      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static PolyfaceAuxDataPtr ReadPolyfaceAuxData(const BGFB::PolyfaceAuxData* fbPolyfaceAuxData)
    {
    auto                        fbIndices = fbPolyfaceAuxData->indices();
    auto                        fbChannels = fbPolyfaceAuxData->channels();
    PolyfaceAuxData::Channels   channels;
    bvector<int32_t>            indices(fbIndices->Length());

    memcpy(indices.data(), fbIndices->GetStructFromOffset(0), fbIndices->Length()*sizeof(int32_t));
                                                                                      
    for (unsigned int i=0; i<fbChannels->Length(); i++)
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

template <typename BlockedVectorType, typename StructType>
static void LoadBlockedVector (BlockedVectorType dest, StructType const*source, size_t n)
    {
    dest.SetActive (true);
    dest.resize (n);
    memcpy (&dest[0], source, n * sizeof (StructType));
    }
    
static PolyfaceHeaderPtr ReadPolyfaceHeader (const BGFB::VariantGeometry * fbGeometry)
    {
    if (BGFB::VariantGeometryUnion_Polyface != fbGeometry->geometry_type ())
        return nullptr;
    auto fbPolyface = reinterpret_cast <const BGFB::Polyface *> (fbGeometry->geometry ());
    return ReadPolyfaceHeaderDirect (fbPolyface);
    }
    
    
static PolyfaceHeaderPtr ReadPolyfaceHeaderDirect (const BGFB::Polyface *fbPolyface)
    {

    uint32_t numPerFace   = (uint32_t)fbPolyface->numPerFace ();
    int numPerRow       = fbPolyface->numPerRow ();
    uint32_t meshStyle    = (uint32_t)fbPolyface->meshStyle ();
    bool twoSided       = 0 != fbPolyface->twoSided ();


    PolyfaceHeaderPtr polyface = PolyfaceHeader::New ();
    polyface->SetNumPerRow (numPerRow);
    polyface->SetTwoSided (twoSided);
    polyface->ClearTags (numPerFace, meshStyle);
    // Blocked vectors need non-zero numPerRow ...
    if (numPerRow < 1)
        numPerRow = 1;
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
        //int numFaceData = (size_t)(fbFaceData->Length () / 8);
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
        polyface->AuxData() = ReadPolyfaceAuxData(fbPolyface->auxData());

    return polyface;
    }

// Fill the pointers in a carrier.
static bool ReadPolyfaceQueryCarrierDirect (const BGFB::Polyface *fbPolyface, PolyfaceQueryCarrier &carrier)
    {
    uint32_t numPerFace   = (uint32_t)fbPolyface->numPerFace ();
    int numPerRow       = fbPolyface->numPerRow ();
    uint32_t meshStyle    = (uint32_t)fbPolyface->meshStyle ();
    bool twoSided       = 0 != fbPolyface->twoSided ();

    size_t numPoint = 0, numParam = 0, numNormal = 0, numColor = 0, numFace = 0; //numDoubleColor = 0, numIntColor = 0, numColorTable = 0;
    size_t numPointIndex = 0, numParamIndex = 0, numNormalIndex = 0, numColorIndex = 0, numFaceIndex;
    int32_t const * pPointIndex = nullptr;
    int32_t const * pNormalIndex = nullptr;
    int32_t const * pParamIndex = nullptr;
    int32_t const * pColorIndex = nullptr;
    int32_t const * pFaceIndex = nullptr;

    
    DPoint3dCP pPoints = nullptr;
    DPoint2dCP pParams = nullptr;
    DVec3dCP   pNormals = nullptr;
    FacetFaceDataCP pFaceData = nullptr;
//    RgbFactor const* pDoubleColor = nullptr;
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
//        numIntColor = (size_t)fbData->Length ();
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
//    if (fbPolyface->has_colorTable ())
//        {
//        auto fbData = fbPolyface->colorTable ();
//        pColorTable = (uint32_t*)fbData->GetStructFromOffset(0);
//        numColorTable = (size_t)fbData->Length ();
//        }
    
    if (numParamIndex > 0 && numParamIndex != numPointIndex)
        return false;
    if (numNormalIndex > 0 && numNormalIndex != numPointIndex)
        return false;
    if (numColorIndex > 0 && numColorIndex != numPointIndex)
        return false;

//    size_t numColor = 0;

//    if (nullptr != pDoubleColor)
//        numColor = numDoubleColor;
//    else if (nullptr != pIntColor)
//        numColor = numIntColor;
//    else if (nullptr != pColorTable)
//        numColor = numColorTable;
        
    carrier = PolyfaceQueryCarrier (
        numPerFace, twoSided, numPointIndex,
        numPoint,  pPoints, pPointIndex,
        numNormal, pNormals, pNormalIndex,
        numParam,  pParams, pParamIndex,
        numColor,  pColorIndex, pIntColor, nullptr,
        meshStyle, numPerRow
        );

    carrier.SetFacetFaceData (pFaceData, numFace);
    carrier.SetFaceIndex (pFaceIndex);
    if (fbPolyface->has_auxData())
        {
        PolyfaceAuxDataCPtr  auxData = ReadPolyfaceAuxData(fbPolyface->auxData());
        carrier.SetAuxData(auxData);
        }

    return true;
    }




static ICurvePrimitivePtr ReadCurvePrimitive (const BGFB::VariantGeometry * fbGeometry)
    {
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_LineSegment:
            {
            BGFB::LineSegment const *fbLineSegment
                = reinterpret_cast <const BGFB::LineSegment *> (fbGeometry->geometry ());
            BGFB::DSegment3d const *fbStruct = fbLineSegment->segment ();
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
            BGFB::DEllipse3d const *fbStruct = fbEllipticArc->arc ();
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
                        fbStruct->sweepRadians ()
                        ));
            }

        case BGFB::VariantGeometryUnion_LineString:
            {
            BGFB::LineString const *fbLineString
                = reinterpret_cast <const BGFB::LineString *> (fbGeometry->geometry ());
            auto fbPoints = fbLineString->points ();
            size_t numDoubles = (size_t)fbPoints->Length ();
            size_t numPoints = numDoubles /3;
            bvector<DPoint3d> points (numPoints);
            return ICurvePrimitive::CreateLineString (
                    (DPoint3dCP)fbPoints->GetStructFromOffset(0), numPoints);
            }
        case BGFB::VariantGeometryUnion_PointString:
            {
            BGFB::PointString const *fbPointString
                = reinterpret_cast <const BGFB::PointString *> (fbGeometry->geometry ());
            auto fbPoints = fbPointString->points ();
            size_t numDoubles = (size_t)fbPoints->Length ();
            size_t numPoints = numDoubles /3;
            bvector<DPoint3d> points (numPoints);
            return ICurvePrimitive::CreatePointString (
                    (DPoint3dCP)fbPoints->GetStructFromOffset(0), numPoints);
            }

        case BGFB::VariantGeometryUnion_BsplineCurve:
            {
            BGFB::BsplineCurve const *fbBsplineCurve
                = reinterpret_cast <const BGFB::BsplineCurve *> (fbGeometry->geometry ());
            int  order = fbBsplineCurve->order ();
            bool closed = fbBsplineCurve->closed () != 0;
            auto fbPoles = fbBsplineCurve->poles ();
            auto fbKnots = fbBsplineCurve->knots ();
            auto fbWeights = fbBsplineCurve->weights ();

            int numPoles = fbPoles->Length () / 3;
            DPoint3dCP pPoles = numPoles > 0 ? (DPoint3dCP)fbPoles->GetStructFromOffset(0) : nullptr;

            int numKnots = fbKnots ? fbKnots->Length () : 0;
            double const * pKnots = numKnots > 0 ? (double const*)fbKnots->GetStructFromOffset(0) : nullptr;

            int numWeights = fbWeights ? fbWeights->Length () : 0;
            double const * pWeights = numWeights > 0 ? (double const*)fbWeights->GetStructFromOffset(0) : nullptr;

            MSBsplineCurve curve;
            curve.Populate (pPoles, pWeights, numPoles,
                        pKnots, numKnots, order, closed, true);
            return ICurvePrimitive::CreateBsplineCurveSwapFromSource (curve);
            }
        case BGFB::VariantGeometryUnion_InterpolationCurve:
            {
            BGFB::InterpolationCurve const *fbInterpolationCurve
                = reinterpret_cast <const BGFB::InterpolationCurve *> (fbGeometry->geometry ());
            int  order = fbInterpolationCurve->order ();
            bool closed = fbInterpolationCurve->closed () != 0;
            auto fbFitPoints = fbInterpolationCurve->fitPoints ();
            auto fbKnots = fbInterpolationCurve->knots ();
            auto fbStartTangent = fbInterpolationCurve->startTangent ();
            auto fbEndTangent = fbInterpolationCurve->endTangent ();
            int numPoles = fbFitPoints->Length () / 3;
            DPoint3dCP pFitPoints = numPoles > 0 ? (DPoint3dCP)fbFitPoints->GetStructFromOffset(0) : nullptr;

            DVec3d startTangent, endTangent;
            startTangent.Zero ();
            endTangent.Zero ();
            if (nullptr != fbStartTangent)
                startTangent = DVec3d::From (fbStartTangent->x (), fbStartTangent->y (), fbStartTangent->z ());
            if (nullptr != fbEndTangent)
                endTangent = DVec3d::From (fbEndTangent->x (), fbEndTangent->y (), fbEndTangent->z ());

            int numKnots = fbKnots ? fbKnots->Length () : 0;
            double const * pKnots = numKnots > 0 ? (double const*)fbKnots->GetStructFromOffset(0) : nullptr;

            MSInterpolationCurve curve;
            curve.Populate (
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
                    );
            return ICurvePrimitive::CreateInterpolationCurveSwapFromSource (curve);
            }

        case BGFB::VariantGeometryUnion_AkimaCurve:
            {
            BGFB::AkimaCurve const *fbAkimaCurve
                = reinterpret_cast <const BGFB::AkimaCurve *> (fbGeometry->geometry ());
            auto fbPoints = fbAkimaCurve->points ();
            int numPoles = fbPoints->Length () / 3;
            DPoint3dCP pPoints = numPoles > 0 ? (DPoint3dCP)fbPoints->GetStructFromOffset(0) : nullptr;
            return ICurvePrimitive::CreateAkimaCurve (pPoints, numPoles);
            }


        case BGFB::VariantGeometryUnion_TransitionSpiral:
            {
            BGFB::TransitionSpiral const *fbTransitionSpiral
                = reinterpret_cast <const BGFB::TransitionSpiral *> (fbGeometry->geometry ());
            BGFB::TransitionSpiralDetail const *detail = fbTransitionSpiral->detail ();
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

            auto fbOrigin = fbCatenaryCurve->origin ();
            auto fbVectorU = fbCatenaryCurve->vectorU ();
            auto fbVectorV = fbCatenaryCurve->vectorV ();

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

            int numPoles = fbPoles == nullptr ? 0 : fbPoles->Length () / 3;
            bvector<DPoint3d> poles;
            if (numPoles > 0)
                {
                DPoint3dCP pData = (DPoint3dCP)fbPoles->GetStructFromOffset(0);
                for (int i = 0; i < numPoles; i++)
                    poles.push_back (pData[i]);
                }

            int numKnotsU = fbKnotsU  == nullptr ? 0 : fbKnotsU->Length ();
            bvector<double>knotsU, knotsV, weights;
            if (numKnotsU > 0)
                {
                double const * pData = (double const*)fbKnotsU->GetStructFromOffset(0);
                for (int i = 0; i < numKnotsU; i++)
                    knotsU.push_back (pData[i]);
                }

            int numKnotsV = fbKnotsV  == nullptr ? 0 : fbKnotsV->Length ();
            if (numKnotsV > 0)
                {
                double const * pData = (double const*)fbKnotsV->GetStructFromOffset(0);
                for (int i = 0; i < numKnotsV; i++)
                    knotsV.push_back (pData[i]);
                }


            int numWeights = fbWeights  == nullptr ? 0 : fbWeights->Length ();
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
    switch (fbGeometry->geometry_type ())
        {
        case BGFB::VariantGeometryUnion_DgnBox:
            {
            BGFB::DgnBox const *fbDgnBox
                = reinterpret_cast <const BGFB::DgnBox *> (fbGeometry->geometry ());
            BGFB::DgnBoxDetail const *fbDetail = fbDgnBox->detail ();
            return ISolidPrimitive::CreateDgnBox (*(DgnBoxDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnCone:
            {
            BGFB::DgnCone const *fbDgnCone
                = reinterpret_cast <const BGFB::DgnCone *> (fbGeometry->geometry ());
            BGFB::DgnConeDetail const *fbDetail = fbDgnCone->detail ();
            return ISolidPrimitive::CreateDgnCone (*(DgnConeDetail const*)fbDetail);
            }            
        case BGFB::VariantGeometryUnion_DgnSphere:
            {
            BGFB::DgnSphere const *fbDgnSphere
                = reinterpret_cast <const BGFB::DgnSphere *> (fbGeometry->geometry ());
            BGFB::DgnSphereDetail const *fbDetail = fbDgnSphere->detail ();
            return ISolidPrimitive::CreateDgnSphere (*(DgnSphereDetail const*)fbDetail);
            }    
        case BGFB::VariantGeometryUnion_DgnTorusPipe:
            {
            BGFB::DgnTorusPipe const *fbDgnTorusPipe
                = reinterpret_cast <const BGFB::DgnTorusPipe *> (fbGeometry->geometry ());
            BGFB::DgnTorusPipeDetail const *fbDetail = fbDgnTorusPipe->detail ();
            return ISolidPrimitive::CreateDgnTorusPipe (*(DgnTorusPipeDetail const*)fbDetail);
            }
        case BGFB::VariantGeometryUnion_DgnExtrusion:
            {
            BGFB::DgnExtrusion const *fbDgnExtrusion
                = reinterpret_cast <const BGFB::DgnExtrusion *> (fbGeometry->geometry ());
            CurveVectorPtr baseCurve = ReadCurveVectorDirect (fbDgnExtrusion->baseCurve ());
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
            CurveVectorPtr baseCurve = ReadCurveVectorDirect (fbDgnRotationalSweep->baseCurve ());
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
            auto fbChildren = fbDgnRuledSweep->curves ();
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
    
    
};    
    
IGeometryPtr BentleyGeometryFlatBuffer::BytesToGeometry (bvector <Byte> &buffer)
    {
    if (buffer.size () == 0)
        return nullptr;
    Byte* fbStart = GetFBStart (buffer);
    if (nullptr == fbStart)
        return nullptr;        
    auto fbRoot = flatbuffers::GetRoot <BGFB::VariantGeometry>(fbStart);
    if (nullptr != fbRoot
        && fbRoot->has_geometry_type ()
        && fbRoot->has_geometry ()
        )
        {
        return FBReader::ReadGeometry (fbRoot);
        }
    return nullptr;
    }

bool BentleyGeometryFlatBuffer::IsFlatBufferFormat (bvector <Byte> &buffer)
    {
    if (buffer.size () == 0)
        return false;
    Byte* fbStart = GetFBStart (buffer);
    return nullptr != fbStart;
    }

bool BentleyGeometryFlatBuffer::IsFlatBufferFormat(Byte const* buffer)
    {
    if (nullptr == buffer)
        return false;
    Byte const* fbStart = GetFBStart(buffer);
    return nullptr != fbStart;
    }


#define IMPLEMENT_BytesToXXX(PtrName,MethodName,ReaderMethodName) \
PtrName BentleyGeometryFlatBuffer::MethodName (Byte const *buffer) \
    {\
    if (nullptr == buffer)\
        return nullptr;\
    Byte const* fbStart = GetFBStart (buffer);\
    if (nullptr == fbStart)\
        return nullptr;\
    auto fbRoot = flatbuffers::GetRoot <BGFB::VariantGeometry>(fbStart);\
    if (nullptr != fbRoot\
        && fbRoot->has_geometry_type ()\
        && fbRoot->has_geometry ()\
        )\
        {\
        return FBReader::ReaderMethodName (fbRoot);\
        }\
    return nullptr;\
    }


IMPLEMENT_BytesToXXX(IGeometryPtr,BytesToGeometry,ReadGeometry)
IMPLEMENT_BytesToXXX(ISolidPrimitivePtr,BytesToSolidPrimitive,ReadSolidPrimitive)
IMPLEMENT_BytesToXXX(ICurvePrimitivePtr,BytesToCurvePrimitive,ReadCurvePrimitive)
IMPLEMENT_BytesToXXX(CurveVectorPtr,BytesToCurveVector,ReadCurveVector)

IMPLEMENT_BytesToXXX(PolyfaceHeaderPtr,BytesToPolyfaceHeader,ReadPolyfaceHeader)
IMPLEMENT_BytesToXXX(MSBsplineSurfacePtr,BytesToMSBsplineSurface,ReadMSBsplineSurface)

bool BentleyGeometryFlatBuffer::BytesToVectorOfGeometry (bvector <Byte> &buffer, bvector<IGeometryPtr> &dest)
    {
    if (buffer.size () == 0)
        return false;
    Byte* fbStart = GetFBStart (buffer);
    if (nullptr == fbStart)
        return false;
    dest.clear ();
    auto fbRoot = flatbuffers::GetRoot <BGFB::VariantGeometry>(fbStart);
    if (nullptr != fbRoot
        && fbRoot->has_geometry_type ()
        && fbRoot->has_geometry ()
        )
        {
        if (fbRoot->geometry_type () == BGFB::VariantGeometryUnion_VectorOfVariantGeometry)
            FBReader::ReadVectorOfVariantGeometryDirect (
                    reinterpret_cast <const BGFB::VectorOfVariantGeometry *> (fbRoot->geometry ()), dest);
        else
            {
            IGeometryPtr g = FBReader::ReadGeometry (fbRoot);
            }
        return dest.size () > 0;
        }
    return false;
    }



bool BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (Byte const *buffer, PolyfaceQueryCarrier &carrier)
    {
    if (nullptr == buffer)
        return false;
    Byte const* fbStart = GetFBStart (buffer);
    if (nullptr == fbStart)
        return false;        
    auto fbRoot = flatbuffers::GetRoot <BGFB::VariantGeometry>(fbStart);
    if (nullptr != fbRoot
        && fbRoot->has_geometry_type ()
        && fbRoot->has_geometry ()
        )
        {
        return FBReader::ReadPolyfaceQueryCarrierDirect (
                    reinterpret_cast <const BGFB::Polyface *> (fbRoot->geometry ()),
                    carrier);
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
