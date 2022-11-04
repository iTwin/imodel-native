/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

DPoint3d SpherePoint(double radius, double thetaRadians, double phiRadians)
    {
    double cosPhi = cos(phiRadians);
    return DPoint3d::From(radius * cos(thetaRadians) * cosPhi,
        radius * sin(thetaRadians) * cosPhi,
        radius * sin(phiRadians));
    }

DVec3d SphereUnitVector(double thetaRadians, double phiRadians)
    {
    double radius = 1.0;
    double cosPhi = cos(phiRadians);
    return DVec3d::From(radius * cos(thetaRadians) * cosPhi,
        radius * sin(thetaRadians) * cosPhi,
        radius * sin(phiRadians));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LightweightBuilder,HelloWorld)
    {
    auto polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    polyface->Normal().SetActive (true);
    polyface->Param().SetActive(true);
    polyface->NormalIndex().SetActive(true);
    polyface->ParamIndex().SetActive(true);
    auto builder = LightweightPolyfaceBuilder::Create (*polyface,
            0.5, 0.5, 0.5);
    auto pointIndex0 = builder->FindOrAddPoint (DPoint3d::From (0.9, 2, 0));
    auto pointIndex1 = builder->FindOrAddPoint(DPoint3d::From(1.1, 2, 0));
    auto pointIndex2 = builder->FindOrAddPoint(DPoint3d::From(1.2, 2, 0));
    auto pointIndex3 = builder->FindOrAddPoint(DPoint3d::From(1.2, 3.1, 0));
    Check::Size (pointIndex1, pointIndex2, "Expect consolidated point pointIndex");
    Check::True (pointIndex0 != pointIndex1, "integer boundary");
    Check::True(pointIndex3 != pointIndex1, "integer boundary");

    auto paramIndex0 = builder->FindOrAddParam(DPoint2d::From(0.9, 2));
    auto paramIndex1 = builder->FindOrAddParam(DPoint2d::From(1.1, 2));
    auto paramIndex2 = builder->FindOrAddParam(DPoint2d::From(1.2, 2));
    auto paramIndex3 = builder->FindOrAddParam(DPoint2d::From(1.2, 3.1));
    Check::Size(paramIndex1, paramIndex2, "Expect consolidated param paramIndex");
    Check::True(paramIndex0 != paramIndex1, "integer boundary");
    Check::True(paramIndex3 != paramIndex1, "integer boundary");

    double q0 = 0.9;
    double q1 = 1.1;
    double q2 = 1.2;
    double p0 = 0.1;
    double p1 = 0.8;
    auto normalIndex0 = builder->FindOrAddNormal(SphereUnitVector (q0, p0));
    auto normalIndex1 = builder->FindOrAddNormal(SphereUnitVector(q1, p0));
    auto normalIndex2 = builder->FindOrAddNormal(SphereUnitVector(q2, p0));
    auto normalIndex3 = builder->FindOrAddNormal(SphereUnitVector(q2, p1));
    Check::Size(normalIndex1, normalIndex2, "Expect consolidated normalIndex");
    Check::True(normalIndex0 != normalIndex1, "integer boundary");
    Check::True(normalIndex3 != normalIndex1, "integer boundary");

    builder->AddPointIndex (pointIndex0, true);
    builder->AddParamIndex (paramIndex0);
    builder->AddNormalIndex (normalIndex0);
    builder->AddIndexTerminators ();
    }

typedef RefCountedPtr<struct BuilderWrapper>  BuilderWrapperPtr;
struct BuilderWrapper : RefCountedBase
{
virtual void InsertPointTriangle (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, bool terminate) = 0;
virtual char const * Name () = 0;
virtual void Finish () = 0;
virtual PolyfaceHeaderPtr PeekMesh () = 0;
virtual void GetDescription(char buffer[2048]) = 0;
};

struct LWBuilderWrapper : BuilderWrapper
{
LightweightPolyfaceBuilderPtr m_builder;
PolyfaceHeaderPtr m_polyface;
bool m_clusteredSearch;

LWBuilderWrapper(double pointGridSize, double normalGridSize, double paramGridSize, bool clusteredSearch)
    {
    m_clusteredSearch = clusteredSearch;
    m_polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    m_builder = LightweightPolyfaceBuilder::Create(*m_polyface, pointGridSize, normalGridSize, paramGridSize);
    }
static BuilderWrapperPtr Create(double pointGridSize, double normalGridSize, double paramGridSize, bool clusteredSearch)
    {
    return new LWBuilderWrapper (pointGridSize, normalGridSize, paramGridSize, clusteredSearch);
    }
char const * Name() override { return "LightWeightMap";}
void GetDescription(char buffer[2048]) override {
    snprintf(buffer, sizeof(*buffer), "(clustered %d)", m_clusteredSearch ? 1 : 0);
    }
void Finish() override {/* nothing to do */}
PolyfaceHeaderPtr PeekMesh() override {return m_polyface;}

void InsertPointTriangle(DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, bool terminate) override
    {
    size_t index0;
    size_t index1;
    size_t index2;
    if (!m_clusteredSearch)
        {
        index0 = m_builder->FindOrAddPoint(point0);
        index1 = m_builder->FindOrAddPoint(point1);
        index2 = m_builder->FindOrAddPoint(point2);
        }
    else
        {
        index0 = m_builder->FindOrAddPointCluster(point0);
        index1 = m_builder->FindOrAddPointCluster(point1);
        index2 = m_builder->FindOrAddPointCluster(point2);
        }
    m_builder->AddPointIndex (index0, true);
    m_builder->AddPointIndex(index1, true);
    m_builder->AddPointIndex(index2, true);
    if (terminate)
        m_builder->AddIndexTerminators ();
    }
};

struct DirectBuilderWrapper : BuilderWrapper
    {
    PolyfaceHeaderPtr m_polyface;
    double m_compressParam;
    // Compress param ...
    // less than 0 ==> no compress
    // 0 ==> default compress
    // greater than 0 ==> absolute tolerance
    DirectBuilderWrapper(double compressParam)
        {
        m_compressParam = compressParam;
        m_polyface = PolyfaceHeader::CreateVariableSizeIndexed();
        }
    static BuilderWrapperPtr Create(double compressParam)
        {
        return new DirectBuilderWrapper(compressParam);
        }
    char const * Name() override { return "Direct";}
    void GetDescription (char buffer[2048]) override
        {
        if (m_compressParam < 0.0)
            snprintf(buffer, sizeof(*buffer), "(no compress)");
        else if (m_compressParam > 0)
            snprintf(buffer, sizeof(*buffer), "(postcompress %g)", m_compressParam);
        else
            snprintf(buffer, sizeof(*buffer), "(postcompress default)");
        }
    void Finish() override
        {
        if (m_compressParam == 0.0)
            m_polyface->Compress ();
        else if (m_compressParam > 0.0)
            m_polyface->Compress(m_compressParam);
        }
    PolyfaceHeaderPtr PeekMesh() override { return m_polyface; }

    void InsertPointTriangle(DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, bool terminate) override
        {
        size_t index0 = m_polyface->Point().size();
        m_polyface->Point().push_back(point0);
        size_t index1 = m_polyface->Point().size();
        m_polyface->Point().push_back(point1);
        size_t index2 = m_polyface->Point().size();
        m_polyface->Point().push_back(point2);
        m_polyface->PointIndex().push_back((int)index0 + 1);
        m_polyface->PointIndex().push_back((int)index1 + 1);
        m_polyface->PointIndex().push_back((int)index2 + 1);
        if (terminate)
            m_polyface->PointIndex().push_back(0);
        }
};

struct ClassicBuilderWrapped : BuilderWrapper
    {
    IFacetOptionsPtr m_options;
    IPolyfaceConstructionPtr m_builder;
    ClassicBuilderWrapped()
        {
        m_options = IFacetOptions::Create();
        m_builder = IPolyfaceConstruction::Create(*m_options);
        }
    static BuilderWrapperPtr Create()
        {
        return new ClassicBuilderWrapped();
        }
    char const * Name() override { return "Classic";}
    void GetDescription(char buffer[2048]) override
        {
        snprintf(buffer, sizeof(*buffer), "(no params)");
        }
    void Finish() override {}

    PolyfaceHeaderPtr PeekMesh() override { return m_builder->GetClientMeshPtr (); }

    void InsertPointTriangle(DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, bool terminate) override
        {
        size_t index0 = m_builder->FindOrAddPoint(point0);
        size_t index1 = m_builder->FindOrAddPoint(point1);
        size_t index2 = m_builder->FindOrAddPoint(point2);
        m_builder->AddPointIndex(index0, true);
        m_builder->AddPointIndex(index1, true);
        m_builder->AddPointIndex(index2, true);
        if (terminate)
            m_builder->AddPointIndexTerminator();
        }
    };



TEST(LightweightBuilder, PointsAcrossBoundaries)
    {
    double x0 = 10.1;
    // xMid is an exact coordinate in the integer grid . . 
    double xMid = 20.0;
    double x3 = 29.9;
    double yShift = 0.1;
    double yFactor = 3.0;
    double yStep = 2.0;
    // * Build many pairs of triangles that almost (but not quite) share an edge
    //    within each pair, with an exact grid line splitting the gap.
    //    * Lightweight build fails to consolidate
    //    * direct-construction followed by compress consolidates
    //    * Lightweight builder with "test cluster around vertex" corrects that but has
    //         4X performance hit.
    //    * Times:
    //        * Lightweight = 1.0
    //        * Direct construction in polyfae (no compress) = 0.2
    //        * Direct construction followed by compress = 1.2
    //        * Lightweight with "test cluster around vertex" = 4
    // * Timing patterns show clearly at 10000 facets 100K and 1000K are nearly proportional.
    // * doing "reserve" in the Point and PointIndex arrays is of incidental performance benefit.
    for (size_t numTriangle : {10, 1000, 10000})
        {
        bool doOutput = numTriangle < 15;
        double epsilon = numTriangle < 15 ? 0.025 : 1.0e-9;
        double gridSize = 0.25;
        // x1 and x2 are shifted by epsilon to left and right.
        // We expect the Compress() will join them but the integerized map grid will not.
        double x1 = xMid - epsilon;
        double x2 = xMid + epsilon;


        printf("\n *** (numTriangle %d) \n", (int)numTriangle);

        auto lwBuilder1 = LWBuilderWrapper::Create(gridSize, 0.001, 0.1, false);
        auto lwBuilder2 = LWBuilderWrapper::Create (gridSize, 0.001, 0.1, true);
        auto directBuilder = DirectBuilderWrapper::Create(-1.0);
        auto directCompressedBuilderA = DirectBuilderWrapper::Create(0.0);
        auto directCompressedBuilderB = DirectBuilderWrapper::Create(4.0 * epsilon);
        auto directCompressedBuilderC = DirectBuilderWrapper::Create(0.05);
        auto classicBuilder = ClassicBuilderWrapped::Create ();

        for (auto &builder : { lwBuilder1, lwBuilder2,
                directBuilder,
                directCompressedBuilderA, directCompressedBuilderB, directCompressedBuilderC,
                classicBuilder })
            {
            TimeAccumulator timer;
            timer.Reset();
            for (size_t triangleIndex = 0; triangleIndex < numTriangle; triangleIndex++)
                {
                double y0 = yShift + triangleIndex * yFactor;
                double y1 = y0 + yStep;
                DPoint3d pointA0 = DPoint3d::From(x0, y0);
                DPoint3d pointA1 = DPoint3d::From(x1, y0);
                DPoint3d pointA2 = DPoint3d::From(x1, y1);
                DPoint3d pointB0 = DPoint3d::From(x2, y0);
                DPoint3d pointB1 = DPoint3d::From(x3, y0);
                DPoint3d pointB2 = DPoint3d::From(x2, y1);
                DPoint3d pointC1 = DPoint3d::From(x3, y1);
                builder->InsertPointTriangle (pointA0, pointA1, pointA2, true);
                builder->InsertPointTriangle (pointB0, pointB1, pointB2, true);
                builder->InsertPointTriangle (pointC1, pointB2, pointB1, true);
                }
            builder->Finish ();
            timer.AccumulateAndReset();
            double t = timer.Sum();
            char buffer[2048];
            builder->GetDescription(buffer);

            printf ("     %s %s    (time %g)   (#V %d)\n", builder->Name (), buffer, t, (int)builder->PeekMesh()->Point ().size ());
        if (doOutput)
            {
            double xStep = 40.0;
            double yMax = 3.0;
            for (double xC = 0.0; xC < 4.5 * xStep; xC += xStep)
                {
                double xA = xC + xMid - 3.0 * gridSize;
                double xB = xC + xMid + 3.0 * gridSize;
                for (double x = xA; x <= xB; x += gridSize)
                    {
                    Check::SaveTransformed(DSegment3d::From(x, 0, 0, x, yMax, 0));
                    }
                for (double y = 0.0; y <= yMax; y += gridSize)
                    {
                    Check::SaveTransformed(DSegment3d::From(xA, y, 0, xB, y, 0));
                    }
                }
            Check::SaveTransformed(*builder->PeekMesh());
            Check::Shift(40, 0, 0);
            }
            }
        }
    Check::ClearGeometry ("LightweightBuilder.PointsAcrossBoundaries");
    }

TEST(LightweightBuilder, Sphere)
    {
    double gridSize = 0.001;
    for (int numAngle : {4, 8, 20})
        {
        for (double stepFraction : {1.0, 0.999, 0.998})
            {
            SaveAndRestoreCheckTransform shifter(0, 20, 0);
            double dTheta = Angle::Pi () * 0.5 / numAngle;
            double dPhi = dTheta;
            int lastThetaIndex = 4 * numAngle;
            int lastPhiIndex = numAngle - 1;
            int numThetaVert = 4 * numAngle;
            int numPhiVert = 2 * numAngle - 1;
            bool doOutput = numAngle < 10;
            printf("\n\n *** angles per quadrant %d\n", numAngle);
            printf ("     True (#V = %d)\n", numThetaVert * numPhiVert);
            printf("     Step Fraction %g\n", stepFraction);

            auto lwBuilder1 = LWBuilderWrapper::Create(gridSize, 0.001, 0.1, false);
            auto lwBuilder2 = LWBuilderWrapper::Create(gridSize, 0.001, 0.1, true);
            auto directBuilder = DirectBuilderWrapper::Create(-1.0);
            auto directCompressedBuilderA = DirectBuilderWrapper::Create(0.0);
            auto directCompressedBuilderC = DirectBuilderWrapper::Create(0.05);
            auto classicBuilder = ClassicBuilderWrapped::Create();
            double radius = 5.0;
            for (auto &builder : { lwBuilder1, lwBuilder2,
                    directBuilder,
                    directCompressedBuilderA, directCompressedBuilderC,
                    classicBuilder })
                {
                TimeAccumulator timer;
                timer.Reset();
                for (int iPhi = -lastPhiIndex; iPhi + 1 <= lastPhiIndex; iPhi += 1)
                    {
                    for (int iTheta = 0; iTheta <= lastThetaIndex; iTheta++)
                        {
                        double phi0 = iPhi * dPhi;
                        double phi1 = (iPhi + stepFraction) * dPhi;
                        double theta0 = iTheta * dTheta;
                        double theta1 = (iTheta + stepFraction) * dTheta;
                        DPoint3d pointA0 = SpherePoint (radius, theta0, phi0);
                        DPoint3d pointA1 = SpherePoint(radius, theta1, phi0);
                        DPoint3d pointB0 = SpherePoint(radius, theta0, phi1);
                        DPoint3d pointB1 = SpherePoint(radius, theta1, phi1);
                        builder->InsertPointTriangle(pointA0, pointA1, pointB0, true);
                        builder->InsertPointTriangle(pointB0, pointA1, pointB1, true);
                        }
                    }
                builder->Finish();
                timer.AccumulateAndReset();
                double t = timer.Sum();
                char buffer[2048];
                builder->GetDescription(buffer);
                printf("     %s %s    (time %g) (#V %d) \n", builder->Name(), buffer, t, (int)builder->PeekMesh()->Point().size());
                if (doOutput)
                    {
                    Check::SaveTransformed(*builder->PeekMesh());
                    Check::Shift(40, 0, 0);
                    }
                }
            }
        }
    Check::ClearGeometry("LightweightBuilder.Sphere");
    }
