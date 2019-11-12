/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "IBeStructuredDataWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct BeCGWriter 
    {
    private:
        IBeStructuredDataWriterR m_dest;
        bmap<OrderedIGeometryPtr, BeExtendedData>* m_extendedData;
        bool m_textualizeXYData;  // for compatibility with .net
        int m_depth;
        bool m_compactCurveVectors;
        bool m_preferCGSweeps;
        bool m_preferMostCompactPrimitivesInCGCurveVectors;
        bool PreferCGSweeps () const;
        void WriteXYZ (Utf8CP name, DPoint3dCR xyz, bool shortName = false);
        void WriteText (Utf8CP name, Utf8CP data, bool shortName = false);
        void WriteSetElementStart (Utf8CP name);
        void WriteSetElementEnd (Utf8CP name);
        void WriteArrayElementStart(Utf8CP longName, Utf8CP shortName);
        void WriteArrayElementEnd(Utf8CP longName, Utf8CP shortName);
        void WriteNamedSetStart (Utf8CP name);
        void WriteNamedSetEnd(Utf8CP name);
        void WriteXY (Utf8CP name, double x, double y, bool shortName = false);
        void WriteXYZ (Utf8CP name, double x, double y, double z, bool shortName = false);
        void WriteDouble (Utf8CP name, double data, bool shortName = false);
        void WriteInt (Utf8CP name, int data, bool shortName = false);
        void WriteBool (Utf8CP name, bool data, bool shortName = false);
        void WriteList (bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteList (bvector<DVec3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteList (bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteList (bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteIndexList (bvector<int> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteList (bvector<double> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName);
        void WriteSpiralType (Utf8CP name, int typeCode);
        void WritePlacementZX (DPoint3dCR origin, DVec3dCR vectorZ, DVec3dCR vectorX);
        void WritePlacementZX (DPoint3dCR origin, RotMatrixCR axes);
        void WritePlacementXY (DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY);
        void WriteSegment (DSegment3dCR data);
        void WriteArc (DEllipse3dCR arc);
        void WriteDisk (DEllipse3dCR arc);
        void WritePolyface (PolyfaceHeader &mesh);
        void WriteCurve (MSBsplineCurveCR curve);
        void WriteSurface (MSBsplineSurfaceCR surface);
        void WriteFullSurface (MSBsplineSurfaceCR surface);
        void WriteTextPlacement (IBeStructuredDataWriterR m_dest, DPoint3dCR xyz, Utf8CP data, double charSize);
        void WriteLineString (bvector<DPoint3d> const &points);
        void WriteCoordinate (DPoint3dCR point);
        void WriteSpiral (struct DSpiral2dPlacement const &spiral);
        void WritePartialCurve (PartialCurveDetailCR data);
        void WritePointString (bvector<DPoint3d> const &points, bool preferMostCompactPrimitives);
        void WritePolygon (bvector<DPoint3d> const &points);
        void WriteDgnTorusPipeDetail (DgnTorusPipeDetail data);
        void WriteDgnConeDetail (DgnConeDetail data);
        void WriteDgnBoxDetail (DgnBoxDetail data);
        void WriteDgnSphereDetail (DgnSphereDetail data);
        void WriteDgnExtrusionDetail (DgnExtrusionDetail data);
        void WriteDgnRotationalSweepDetail (DgnRotationalSweepDetail data);
        void WriteDgnRuledSweepDetail (DgnRuledSweepDetail data);
        void WriteExtendedData(BeExtendedData const& extendedData);
        void WriteCurveVectorAsSingleCurveChain (CurveVectorCR curveVector);

    public:
        BeCGWriter (
            IBeStructuredDataWriterR dest,
            bmap<OrderedIGeometryPtr,
            BeExtendedData>* data,
            bool textualizeXYData,
            bool compactCurveVectors,
            bool preferCGSweeps,
            bool preferMostCompactPrimitivesInCGCurveVectors = false
            );
        void Write (ICurvePrimitiveCR curve, bool preferMostCompactPrimitives);
        void Write (ICurvePrimitiveCR curve);
        void Write (CurveVectorCR curveVector);
        void WriteNativeCurveVector (CurveVectorCR curveVector);
        void WriteCGCurveVector (CurveVectorCR curveVector, bool preferMostCompactPrimitives, bool wrapInElement = true, bool preferCurveChain = false);
        void Write (ISolidPrimitiveR primitive);
        void Write (IGeometryCR geometry);
        void Write (bvector<IGeometryPtr> const &geometry);
    };
END_BENTLEY_GEOMETRY_NAMESPACE
