/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/GraphicBuilder_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include "FakeRenderSystem.h"
#include <DgnPlatform/TileIO.h>

using namespace FakeRender;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectEqualDouble(double a, double b)
    {
    EXPECT_TRUE(DoubleOps::AlmostEqual(a, b));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ExpectEqualPoints2d(T const& a, T const& b)
    {
    ExpectEqualDouble(a.x, b.x);
    ExpectEqualDouble(a.y, b.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ExpectEqualPoints(T const& a, T const& b)
    {
    ExpectEqualPoints2d(a, b);
    ExpectEqualDouble(a.z, b.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectEqualRange(DRange3dCR a, DRange3dCR b)
    {
    ExpectEqualPoints(a.low, b.low);
    ExpectEqualPoints(a.high, b.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectEqualBytes(ByteStream const& a, ByteStream const& b)
    {
    EXPECT_EQ(a.size(), b.size());
    if (a.size() == b.size())
        {
        bool equalBytes = 0 == memcmp(a.data(), b.data(), a.size());
        EXPECT_TRUE(equalBytes);

#if defined(DEBUG_EQUAL_BYTES)
        if (!equalBytes)
            {
            size_t lastDifference = -1;
            for (size_t i = 0; i < a.size(); i++)
                {
                auto lhs = a[i], rhs = b[i];
                if (lhs != rhs)
                    {
                    if (lastDifference != i-1)
                        {
                        EXPECT_EQ(lhs, rhs) << "Data differs beginning at position " << i;
                        lastDifference = i;
                        }
                    }
                }
            }
#endif
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct GraphicProcessorTest : DgnDbTestFixture, GraphicProcessor
{
protected:
    GraphicProcessorSystem m_system;

    GraphicProcessorTest() : m_system(*this) { }

    template<typename T> void PopulateGraphic(T populateGraphic)
        {
        // Using view coords because have no viewport from which to determine appropriate facet tolerance
        GraphicBuilderPtr gf = m_system._CreateGraphic(GraphicBuilder::CreateParams::View(GetDgnDb()));
        ActivateGraphicParams(*gf);
        populateGraphic(*gf);
        gf->Finish();
        }

    void ActivateGraphicParams(GraphicBuilderR gf, ColorDef fillColor = ColorDef::Blue())
        {
        GeometryParams geomParams(GetDefaultCategoryId());
        geomParams.Resolve(GetDgnDb(), nullptr);
        GraphicParams gfParams = GraphicParams::FromSymbology(ColorDef::Red(), fillColor, 5, LinePixels::Solid);
        gf.ActivateGraphicParams(gfParams, &geomParams);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct DisjointCurvesTest : GraphicProcessorTest
{
protected:
    uint32_t m_numDisjoint = 0;
    uint32_t m_numContinuous = 0;

    DisjointCurvesTest() { }

    void ExpectDisjoint(uint32_t num) const { EXPECT_EQ(num, m_numDisjoint); }
    void ExpectContinuous(uint32_t num) const { EXPECT_EQ(num, m_numContinuous); }

    void Process(IndexedPolylineArgsCR args) override
        {
        if (args.m_disjoint)
            ++m_numDisjoint;
        else
            ++m_numContinuous;
        }

    template<typename T> void Test(uint32_t numDisjoint, uint32_t numContinuous, T createGraphic)
        {
        m_numDisjoint = m_numContinuous = 0;

        PopulateGraphic(createGraphic);

        EXPECT_EQ(numDisjoint, m_numDisjoint);
        EXPECT_EQ(numContinuous, m_numContinuous);
        }

    template<typename T> void ExpectDisjoint(T createGraphic) { Test(1, 0, createGraphic); }
    template<typename T> void ExpectContinuous(T createGraphic) { Test(0, 1, createGraphic); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisjointCurvesTest, SinglePrimitives)
    {
    SetupSeedProject();

    // Ellipse
    ExpectContinuous([](GraphicBuilderR gf)
        {
        gf.AddArc(DEllipse3d::FromCenterRadiusXY(DPoint3d::FromXYZ(50, 50, 0), 10), false, false);
        });

    // Line string
    ExpectContinuous([](GraphicBuilderR gf)
        {
        DPoint2d pts[3] =
            {
            DPoint2d::From(0,0),
            DPoint2d::From(10, 0),
            DPoint2d::From(10, 10)
            };

        gf.AddLineString2d(3, pts, 0);
        });

    DPoint2d pts[2] = { DPoint2d::From(0, 0), DPoint2d::From(10, 0) };

    // Point String
    ExpectDisjoint([&](GraphicBuilderR gf)
        {
        gf.AddPointString2d(2, pts, 0);
        });

    // zero-length line string
    ExpectDisjoint([](GraphicBuilderR gf)
        {
        DPoint2d pts[2] = { DPoint2d::From(0,0), DPoint2d::From(0,0) };
        gf.AddLineString2d(2, pts, 0);
        });

    // single-point line string
    ExpectDisjoint([&](GraphicBuilderR gf)
        {
        gf.AddLineString2d(1, pts, 0);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisjointCurvesTest, CurveVector)
    {
    SetupSeedProject();

    DPoint3d pts[2] = { DPoint3d::FromXYZ(200, 50, 0), DPoint3d::FromXYZ(250, 50, 0) };
    auto adjustY = [&]() { pts[0].y += 35; pts[1].y = pts[0].y; };

    auto curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    // point string
    curve->push_back(ICurvePrimitive::CreatePointString(pts, 2));

    // line
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(pts[0], pts[1])));

    // zero-length line
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(pts[0], pts[0])));

    // single-point line string
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 1));

    // line string
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 2));

    // zero-length line string with 2 points
    adjustY();
    pts[1] = pts[0];
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 2));

    // zero-length line string with 3 points.
    // As in MicroStation, NOT treated as a point if it contains more than 2 vertices
    adjustY();
    DPoint3d same3Pts[3] = { *pts, *pts, *pts };
    curve->push_back(ICurvePrimitive::CreateLineString(same3Pts, 3));

    bool wantFilled = false;
    auto addCurveVector = [&](GraphicBuilderR gf) { gf.AddCurveVector(*curve, wantFilled); };

    // The disjoint curves get batched into one primitive, the continuous curves into another.
    Test(1, 1, addCurveVector);

    // Filled curve vectors cannot be rendered disjoint...
    wantFilled = true;
    ExpectContinuous(addCurveVector);

    // BOUNDARY_TYPE_None required for disjoint curve primitives...
    wantFilled = false;
    curve->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
    ExpectContinuous(addCurveVector);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct MeshBuilderTest : GraphicProcessorTest
{
    friend struct TestGraphic;
protected:
    FeatureTable    m_features;
    MeshBuilderMap  m_builders;
    uint64_t        m_curElemId = 0ull;

    MeshBuilderTest() : m_features(100) { }

    struct TestContext : ViewContext
    {
        SystemR m_system;

        explicit TestContext(PrimitiveBuilderR gf) : m_system(gf.GetSystem())
            {
            SetDgnDb(gf.GetDgnDb());
            Attach(gf.GetViewport(), DrawPurpose::NotSpecified);
            }

        SystemP _GetRenderSystem() const override { return &m_system; }
        GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) override { BeAssert(false); return nullptr; }
        GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) override { BeAssert(false); return nullptr; }
    };

    // Accumulates geometric primitives into a MeshBuilderMap.
    struct TestGraphic : PrimitiveBuilder
    {
        MeshBuilderTest&    m_test;

        TestGraphic(SystemR sys, CreateParamsCR params, MeshBuilderTest& test, DgnElementId elemId) : PrimitiveBuilder(sys, params, elemId), m_test(test)
            {
            // Using view coords so tolerance is trivially computable...
            BeAssert(params.IsViewCoordinates());
            }

        GraphicPtr _FinishGraphic(GeometryAccumulatorR accum) override
            {
            TestContext context(*this);
            GeometryOptions opts;
            m_test.m_builders = accum.ToMeshBuilders(opts, ComputeTolerance(accum), &m_test.m_features, context);

            return GetSystem()._CreateGraphicList(std::move(m_primitives), GetDgnDb());
            }

        void BeginNewElement() { SetElementId(m_test.GetNextElementId()); }
    };

    GraphicBuilderPtr CreateGraphic(SystemR sys, GraphicBuilder::CreateParamsCR params) override
        {
        m_features.clear();
        return new TestGraphic(sys, params, *this, GetNextElementId());
        }

    DgnElementId GetNextElementId() { return DgnElementId(++m_curElemId); }
    Render::Primitives::GeometryCollection GetGeometryCollection(ElementAlignedBox3dR contentRange, DPoint3dR centroid) const
        {
        contentRange = ElementAlignedBox3d(DRange3d::NullRange());
        Render::Primitives::GeometryCollection geom;
        for (auto const& kvp : m_features)
            geom.Meshes().m_features.Insert(kvp.first, kvp.second);

        for (auto& builder : m_builders)
            {
            MeshP mesh = builder.second->GetMesh();
            if (!mesh->IsEmpty())
                {
                geom.Meshes().push_back(mesh);
                if (contentRange.IsNull())
                    contentRange = ElementAlignedBox3d(mesh->GetRange());
                else
                    contentRange.Extend(mesh->GetRange());
                }
            }

        centroid.Interpolate(contentRange.low, 0.5, contentRange.high);
        return geom;
        }

    void BuildGraphic(GraphicBuilderR gf);
    void BeginNewElement(GraphicBuilderR gf)
        {
        BeAssert(nullptr != dynamic_cast<TestGraphic*>(&gf));
        static_cast<TestGraphic&>(gf).BeginNewElement();
        }

    template<typename T> void RoundTripGeometryCollection(T populateGraphic);
    template<typename T> void RoundTripMeshBuilders(T populateGraphic);

    void RoundTripMeshBuilders(TileTree::StreamBufferR writeBytes, DgnElementIdSet const& skipElems, size_t nTotalElems);

    void ExpectEqualGeometry(TileTree::StreamBufferR baseline, TileTree::StreamBufferR comparand);
    void ExpectEqualGeometry(Render::Primitives::GeometryCollectionR base, Render::Primitives::GeometryCollectionR comp);
    void ExpectEqualMeshLists(MeshListCR, MeshListCR);
    void ExpectEqualFeatureTables(FeatureTableCR, FeatureTableCR, bool expectEqualIndices=false);
    void ExpectEqualMeshes(MeshCR, MeshCR);
    void ExpectEqualColorTables(ColorTableCR, ColorTableCR);
    void ExpectEqualMeshPrimitives(MeshCR, MeshCR);
    void ExpectEqualEdges(MeshCR, MeshCR);
    void ExpectEqualMeshEdgeLists(bvector<MeshEdge> const& base, bvector<MeshEdge> const& comp, MeshCR baseMesh, MeshCR compMesh);
    void ExpectEqualSilhouetteNormals(OctEncodedNormalPairListCR base, OctEncodedNormalPairListCR comp);
    void ExpectEqualPolylineLists(MeshCR, MeshCR);
    void ExpectEqualPolylineLists(PolylineList const&, PolylineList const&, MeshCR, MeshCR);
    void ExpectEqualVertexLists(bvector<uint32_t> const&, bvector<uint32_t> const&, MeshCR, MeshCR);
    void ExpectEqualVertices(MeshCR, MeshCR, uint32_t, uint32_t, FeatureIndex const&, FeatureIndex const&);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualGeometry(TileTree::StreamBufferR base, TileTree::StreamBufferR comp)
    {
    ElementAlignedBox3d baseRange, compRange;
    Render::Primitives::GeometryCollection baseGeom, compGeom;
    bool baseIsLeaf, compIsLeaf;
    auto& model = *GetDefaultPhysicalModel();

    base.SetPos(0);
    comp.SetPos(0);
    EXPECT_TRUE(TileTree::IO::ReadStatus::Success == TileTree::IO::ReadDgnTile(baseRange, baseGeom, base, model, m_system, baseIsLeaf));
    EXPECT_TRUE(TileTree::IO::ReadStatus::Success == TileTree::IO::ReadDgnTile(compRange, compGeom, comp, model, m_system, compIsLeaf));

    ExpectEqualRange(baseRange, compRange);
    EXPECT_EQ(baseIsLeaf, compIsLeaf);
    ExpectEqualGeometry(baseGeom, compGeom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualGeometry(Render::Primitives::GeometryCollectionR base, Render::Primitives::GeometryCollectionR comp)
    {
    EXPECT_EQ(base.IsEmpty(), comp.IsEmpty());
    EXPECT_EQ(base.IsComplete(), comp.IsComplete());
    EXPECT_EQ(base.ContainsCurves(), comp.ContainsCurves());

    ExpectEqualMeshLists(base.Meshes(), comp.Meshes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualMeshLists(MeshListCR base, MeshListCR comp)
    {
    ExpectEqualFeatureTables(base.FeatureTable(), comp.FeatureTable());

    EXPECT_EQ(base.size(), comp.size());
    if (base.size() == comp.size())
        {
        for (size_t i = 0; i < base.size(); i++)
            ExpectEqualMeshes(*base[i], *comp[i]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualFeatureTables(FeatureTableCR base, FeatureTableCR comp, bool expectEqualIndices)
    {
    EXPECT_EQ(base.size(), comp.size());
    EXPECT_EQ(base.GetMaxFeatures(), comp.GetMaxFeatures());
    EXPECT_EQ(base.IsUniform(), comp.IsUniform());

    for (auto kvp : base)
        {
        uint32_t compIndex;
        EXPECT_TRUE(comp.FindIndex(compIndex, kvp.first));
        if (expectEqualIndices)
            EXPECT_EQ(kvp.second, compIndex);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualMeshes(MeshCR base, MeshCR comp)
    {
    EXPECT_EQ(base.IsEmpty(), comp.IsEmpty());
    EXPECT_EQ(base.Is2d(), comp.Is2d());
    EXPECT_EQ(base.IsPlanar(), comp.IsPlanar());
    EXPECT_TRUE(base.GetType() == comp.GetType());

    EXPECT_EQ(base.Triangles().Count(), comp.Triangles().Count());
    EXPECT_EQ(base.Polylines().size(), comp.Polylines().size());
    EXPECT_EQ(base.Colors().size(), comp.Colors().size());
    EXPECT_EQ(base.Params().size(), comp.Params().size());
    EXPECT_EQ(base.Normals().size(), comp.Normals().size());

    ExpectEqualRange(base.GetRange(), comp.GetRange());
    ExpectEqualRange(base.GetUVRange(), comp.GetUVRange());
    EXPECT_TRUE(base.GetDisplayParams().IsEqualTo(comp.GetDisplayParams()));

    ExpectEqualColorTables(base.GetColorTable(), comp.GetColorTable());

    ASSERT_EQ(base.Points().size(), comp.Points().size());
    if (Mesh::PrimitiveType::Mesh == base.GetType())
        ExpectEqualMeshPrimitives(base, comp);
    else
        ExpectEqualPolylineLists(base, comp);

    ASSERT_EQ(base.GetEdges().IsValid(), comp.GetEdges().IsValid());
    if (base.GetEdges().IsValid())
        ExpectEqualEdges(base, comp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualEdges(MeshCR baseMesh, MeshCR compMesh)
    {
    auto const& base = *baseMesh.GetEdges();
    auto const& comp = *compMesh.GetEdges();

    ExpectEqualPolylineLists(base.m_polylines, comp.m_polylines, baseMesh, compMesh);
    ExpectEqualMeshEdgeLists(base.m_visible, comp.m_visible, baseMesh, compMesh);
    ExpectEqualMeshEdgeLists(base.m_silhouette, comp.m_silhouette, baseMesh, compMesh);
    ExpectEqualSilhouetteNormals(base.m_silhouetteNormals, comp.m_silhouetteNormals);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualMeshEdgeLists(bvector<MeshEdge> const& base, bvector<MeshEdge> const& comp, MeshCR baseMesh, MeshCR compMesh)
    {
    ASSERT_EQ(base.size(), comp.size());

    bvector<uint32_t> baseIndices(2);
    bvector<uint32_t> compIndices(2);
    for (size_t i = 0; i < base.size(); i++)
        {
        baseIndices[0] = base[i].m_indices[0];
        baseIndices[1] = base[i].m_indices[1];
        baseIndices[0] = base[i].m_indices[0];
        compIndices[1] = comp[i].m_indices[1];

        ExpectEqualVertexLists(baseIndices, compIndices, baseMesh, compMesh);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualSilhouetteNormals(OctEncodedNormalPairListCR base, OctEncodedNormalPairListCR comp)
    {
    ASSERT_EQ(base.size(), comp.size());
    for (size_t i = 0; i < base.size(); i++)
        {
        EXPECT_TRUE(base[i].first == comp[i].first);
        EXPECT_TRUE(base[i].second == comp[i].second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualColorTables(ColorTableCR base, ColorTableCR comp)
    {
    EXPECT_EQ(base.size(), comp.size());
    EXPECT_EQ(base.HasTransparency(), comp.HasTransparency());
    EXPECT_EQ(base.IsUniform(), comp.IsUniform());

    for (auto kvp : base)
        {
        auto compKvp = comp.find(kvp.first);
        EXPECT_FALSE(comp.end() == compKvp);
        if (comp.end() != compKvp)
            EXPECT_EQ(kvp.second, compKvp->second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualMeshPrimitives(MeshCR base, MeshCR comp)
    {
    ASSERT_EQ(base.Triangles().Count(), comp.Triangles().Count());
    ExpectEqualVertexLists(base.Triangles().Indices(), comp.Triangles().Indices(), base, comp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualPolylineLists(MeshCR baseMesh, MeshCR compMesh)
    {
    ExpectEqualPolylineLists(baseMesh.Polylines(), compMesh.Polylines(), baseMesh, compMesh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualPolylineLists(PolylineList const& basePolylines, PolylineList const& compPolylines, MeshCR baseMesh, MeshCR compMesh)
    {
    for (size_t i = 0; i < basePolylines.size(); i++)
        {
        MeshPolyline const& base = basePolylines[i];
        MeshPolyline const& comp = compPolylines[i];

        ASSERT_EQ(base.GetIndices().size(), comp.GetIndices().size());
        EXPECT_EQ(base.GetStartDistance(), comp.GetStartDistance());
        ExpectEqualPoints(base.GetRangeCenter(), comp.GetRangeCenter());

        ExpectEqualVertexLists(base.GetIndices(), comp.GetIndices(), baseMesh, compMesh);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualVertexLists(bvector<uint32_t> const& base, bvector<uint32_t> const& comp, MeshCR baseMesh, MeshCR compMesh)
    {
    ASSERT_EQ(base.size(), comp.size());

    FeatureIndex baseFeatureIndex, compFeatureIndex;
    baseMesh.ToFeatureIndex(baseFeatureIndex);
    compMesh.ToFeatureIndex(compFeatureIndex);

    ASSERT_EQ(baseFeatureIndex.IsEmpty(), compFeatureIndex.IsEmpty());
    ASSERT_EQ(baseFeatureIndex.IsUniform(), compFeatureIndex.IsUniform());

    for (size_t i = 0; i < base.size(); i++)
        ExpectEqualVertices(baseMesh, compMesh, base[i], comp[i], baseFeatureIndex, compFeatureIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::ExpectEqualVertices(MeshCR base, MeshCR comp, uint32_t baseIndex, uint32_t compIndex, FeatureIndex const& baseFeatIdx, FeatureIndex const& compFeatIdx)
    {
    EXPECT_TRUE(base.Points()[baseIndex] == comp.Points()[compIndex]);

    if (!base.Normals().empty())
        EXPECT_TRUE(base.Normals()[baseIndex] == comp.Normals()[compIndex]);

    if (!base.Params().empty())
        ExpectEqualPoints2d(base.Params()[baseIndex], comp.Params()[compIndex]);

    if (!base.Colors().empty())
        EXPECT_EQ(base.Colors()[baseIndex], comp.Colors()[compIndex]);

    uint32_t baseFeatureId, compFeatureId;
    if (baseFeatIdx.IsUniform())
        {
        baseFeatureId = baseFeatIdx.m_featureID;
        compFeatureId = compFeatIdx.m_featureID;
        }
    else
        {
        baseFeatureId = baseFeatIdx.m_featureIDs[baseIndex];
        compFeatureId = compFeatIdx.m_featureIDs[compIndex];
        }

    Feature baseFeat, compFeat;
    EXPECT_TRUE(base.GetFeatureTable()->FindFeature(baseFeat, baseFeatureId));
    EXPECT_TRUE(comp.GetFeatureTable()->FindFeature(compFeat, compFeatureId));
    EXPECT_TRUE(baseFeat.GetClass() == compFeat.GetClass());
    EXPECT_EQ(baseFeat.GetElementId().GetValueUnchecked(), compFeat.GetElementId().GetValueUnchecked());
    EXPECT_EQ(baseFeat.GetSubCategoryId().GetValueUnchecked(), compFeat.GetSubCategoryId().GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* Test that we can write a GeometryCollection to a binary+json stream, read it back,
* and obtain an equivalent GeometryCollection.
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void MeshBuilderTest::RoundTripGeometryCollection(T populateGraphic)
    {
    PopulateGraphic(populateGraphic);

    // Write the geometry to a stream buffer
    ElementAlignedBox3d contentRange;
    DPoint3d centroid;
    auto geom = GetGeometryCollection(contentRange, centroid);

    GeometricModelR model = *GetDefaultPhysicalModel();
    bool isLeaf = true;
    TileTree::StreamBuffer writeBytes;
    EXPECT_EQ(SUCCESS, TileTree::IO::WriteDgnTile(writeBytes, contentRange, geom, model, centroid, isLeaf));

    // Read the geometry back from the buffer
    ElementAlignedBox3d readContentRange;
    Render::Primitives::GeometryCollection readGeom;
    bool readIsLeaf = false;
    writeBytes.SetPos(0);
    EXPECT_TRUE(TileTree::IO::ReadStatus::Success == TileTree::IO::ReadDgnTile(readContentRange, readGeom, writeBytes, model, m_system, readIsLeaf));

    EXPECT_EQ(isLeaf, readIsLeaf);
    EXPECT_EQ(geom.Meshes().size(), readGeom.Meshes().size());
    ExpectEqualRange(contentRange, readContentRange);

    // Write it back to a stream buffer, confirm same bytes
    TileTree::StreamBuffer roundTripBytes;
    EXPECT_EQ(SUCCESS, TileTree::IO::WriteDgnTile(roundTripBytes, readContentRange, readGeom, model, centroid, readIsLeaf));
    ExpectEqualBytes(writeBytes, roundTripBytes);
    }

/*---------------------------------------------------------------------------------**//**
* Test that we can write a GeometryCollection to a binary+json stream and read it back
* into a MeshBuilderMap, optionally omitting some elements when reading it back.
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void MeshBuilderTest::RoundTripMeshBuilders(T populateGraphic)
    {
    // Populate the graphic with various elements (1 per geometric primitive)
    uint64_t firstElemId = m_curElemId + 1;

    PopulateGraphic(populateGraphic);

    uint64_t lastElemId = m_curElemId;
    size_t nFeatures = m_features.size();
    EXPECT_EQ(nFeatures, lastElemId + 1 - firstElemId);

    DgnElementIdSet allElemIds;
    for (uint64_t id = firstElemId; id <= lastElemId; id++)
        allElemIds.insert(DgnElementId(id));

    // Serialize to stream buffer
    ElementAlignedBox3d contentRange;
    DPoint3d centroid;
    auto geom = GetGeometryCollection(contentRange, centroid);
    TileTree::StreamBuffer writeBytes;
    GeometricModelR model = *GetDefaultPhysicalModel();
    EXPECT_EQ(SUCCESS, TileTree::IO::WriteDgnTile(writeBytes, contentRange, geom, model, centroid, true));

    // Round-trip, omitting no elements
    RoundTripMeshBuilders(writeBytes, DgnElementIdSet(), nFeatures);

    // Round-trip, omitting all elements
    RoundTripMeshBuilders(writeBytes, allElemIds, nFeatures);

    // Round-trip, omitting one element
    DgnElementIdSet skipElems;
    skipElems.insert(*allElemIds.begin());
    RoundTripMeshBuilders(writeBytes, skipElems, nFeatures);

    // Round-trip, omitting all but one element
    skipElems = allElemIds;
    skipElems.erase(*allElemIds.begin());
    RoundTripMeshBuilders(writeBytes, skipElems, nFeatures);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::RoundTripMeshBuilders(TileTree::StreamBufferR writeBytes, DgnElementIdSet const& skipElems, size_t nTotalElems)
    {
    // Read into mesh builder map
    m_builders.clear();
    m_features.clear();
    TileTree::IO::DgnTile::Flags flags;
    writeBytes.SetPos(0);
    GeometricModelR model = *GetDefaultPhysicalModel();
    EXPECT_EQ(TileTree::IO::ReadStatus::Success, TileTree::IO::ReadDgnTile(m_builders, writeBytes, model, m_system, flags, skipElems));

    // Confirm we really skipped the elems specified
    EXPECT_EQ(nTotalElems - skipElems.size(), m_features.size());
    for (auto const& skipElem : skipElems)
        {
        uint32_t featIndex;
        EXPECT_FALSE(m_features.FindIndex(featIndex, Feature(skipElem, DgnCategory::GetDefaultSubCategoryId(GetDefaultCategoryId()), DgnGeometryClass::Primary)));
        }

    ElementAlignedBox3d readContentRange;
    DPoint3d readCentroid;
    auto geom = GetGeometryCollection(readContentRange, readCentroid);
    EXPECT_EQ(nTotalElems == skipElems.size(), geom.Meshes().empty());

    // Serialize the new geometry collection, compare to input
    TileTree::StreamBuffer roundTripBytes;
    EXPECT_EQ(SUCCESS, TileTree::IO::WriteDgnTile(roundTripBytes, readContentRange, geom, model, readCentroid, true));
    if (skipElems.empty())
        ExpectEqualGeometry(writeBytes, roundTripBytes);
    else
        EXPECT_TRUE(roundTripBytes.size() < writeBytes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderTest::BuildGraphic(GraphicBuilderR gf)
    {
    DPoint2d pts[5] =
        {
        DPoint2d::From(0, 0),
        DPoint2d::From(10, 0),
        DPoint2d::From(10, 10),
        DPoint2d::From(0, 10),
        DPoint2d::From(0, 0)
        };

    auto adjustShapePts = [&]()
        {
        for (auto& pt : pts)
            pt.y += 20;
        };

    gf.AddArc(DEllipse3d::FromCenterRadiusXY(DPoint3d::FromXYZ(50, 50, 0), 10), false, false);

    BeginNewElement(gf);
    gf.AddLineString2d(3, pts, 0);

    BeginNewElement(gf);
    gf.AddShape2d(5, pts, true, 0);

    ColorDef fillColors[5] = { ColorDef::Red(), ColorDef::Green(), ColorDef::Blue(), ColorDef::White(), ColorDef::Black() };
    for (uint32_t i = 0; i < 5; i++)
        {
        adjustShapePts();
        BeginNewElement(gf);
        ActivateGraphicParams(gf, fillColors[i]);
        gf.AddShape2d(5, pts, true, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MeshBuilderTest, RoundTripTileIO)
    {
    SetupSeedProject();

    RoundTripGeometryCollection([&](GraphicBuilderR gf) { BuildGraphic(gf); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MeshBuilderTest, RoundTripMeshBuilders)
    {
    SetupSeedProject();

    RoundTripMeshBuilders([&](GraphicBuilderR gf) { BuildGraphic(gf); });
    }

