/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/DgnPlatform.h>

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
USING_NAMESPACE_BENTLEY_DGN

#define EXPECT_NOT_NULL(EXPR) EXPECT_FALSE(NULL == (EXPR))
#define EXPECT_NULL(EXPR) EXPECT_TRUE(NULL == (EXPR))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectPointsEqual(QPoint3dCR lhs, QPoint3dCR rhs)
    {
    EXPECT_EQ(lhs.x, rhs.x);
    EXPECT_EQ(lhs.y, rhs.y);
    EXPECT_EQ(lhs.z, rhs.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ToString(DPoint3dCR p)
    {
    return Utf8PrintfString("(%f,%f,%f)", p.x, p.y, p.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectPointsEqual(DPoint3dCR lhs, DPoint3dCR rhs, double tolerance)
    {
    EXPECT_TRUE(lhs.IsEqual(rhs, tolerance)) << ToString(lhs).c_str() << " != " << ToString(rhs).c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectQPoint3d(DPoint3dCR dpt, QPoint3d::Params params, QPoint3dCR exp, double tolerance)
    {
    QPoint3d qpt(dpt, params);
    ExpectPointsEqual(qpt, exp);
    ExpectPointsEqual(qpt.Unquantize(params), dpt, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Render_Tests, QPoint)
    {
    DRange3d range = DRange3d::From(DPoint3d::FromXYZ(0.0, -100.0, 200.0), DPoint3d::FromXYZ(50.0, 100.0, 10000.0));
    QPoint3d::Params params(range);
    ExpectPointsEqual(params.origin, range.low, 0.0);

    ExpectQPoint3d(range.low, params, QPoint3d(0,0,0), 0.0);
    ExpectQPoint3d(range.high, params, QPoint3d(0xffff,0xffff,0xffff), 0.0);

    DPoint3d center = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    ExpectQPoint3d(center, params, QPoint3d(0x8000,0x8000,0x8000), 0.08);

    range.low.z = range.high.z = 500.0;
    params = QPoint3d::Params(range);

    ExpectQPoint3d(range.low, params, QPoint3d(0,0,0), 0.0);
    ExpectQPoint3d(range.high, params, QPoint3d(0xffff,0xffff,0), 0.0);

    center.z = 500.0;
    ExpectQPoint3d(center, params, QPoint3d(0x8000,0x8000,0), 0.002);
    }

enum Comparison { kLess, kGreater, kEqual };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectComparison(Comparison exp, VertexKeyCR lhs, VertexKeyCR rhs)
    {
    bool less = lhs < rhs;
    bool grtr = rhs < lhs;
    EXPECT_FALSE(less && grtr);

    switch (exp)
        {
        case kLess:
            EXPECT_TRUE(less);
            EXPECT_FALSE(grtr);
            break;
        case kGreater:
            EXPECT_TRUE(grtr);
            EXPECT_FALSE(less);
            break;
        case kEqual:
            EXPECT_FALSE(grtr);
            EXPECT_FALSE(less);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectSignsEqual(double a, double b)
    {
    if (0.0 != a)
        {
        EXPECT_EQ(a < 0.0, b < 0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectSignsEqual(DVec3dCR a, DVec3dCR b)
    {
    ExpectSignsEqual(a.x, b.x);
    ExpectSignsEqual(a.y, b.y);
    ExpectSignsEqual(a.z, b.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RoundTrip(DVec3d in, bool normalized, double tolerance)
    {
    if (!normalized)
        in.Normalize();

    OctEncodedNormal oen = OctEncodedNormal::From(in);
    DVec3d out = oen.Decode();
    ExpectPointsEqual(in, out, tolerance);
    ExpectSignsEqual(in, out);

    OctEncodedNormal rep = OctEncodedNormal::From(out);
    EXPECT_EQ(oen.Value(), rep.Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RoundTrip(double x, double y, double z, bool normalized=true, double tolerance=0.005)
    {
    RoundTrip(DVec3d::From(x, y, z), normalized, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Render_Tests, OctEncodedNormals)
    {
    RoundTrip(1.0, 0.0, 0.0);
    RoundTrip(0.0, 1.0, 0.0);
    RoundTrip(0.0, 0.0, 1.0);

    RoundTrip(-1.0, 0.0, 0.0);
    RoundTrip(0.0, -1.0, 0.0);
    RoundTrip(0.0, 0.0, -1.0);

    RoundTrip(0.5, 2.5, 0.0, false);
    RoundTrip(-25.0, 25.0, 5.0, false, 0.012);
    RoundTrip(0.0001, -1900.0, 22.5, false, 0.01);
    RoundTrip(-1.0, -1.0, -1.0, false, 0.03);
    }

using MatParams = Material::CreateParams;
using MatColor = MatParams::MatColor;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MatColor makeMatColor(ColorDef color, bool valid, uint8_t alpha = 0)
    {
    color.SetAlpha(alpha);
    MatColor matColor(color);
    matColor.m_valid = valid;
    return matColor;
    }

void expectEqual(MatColor lhs, MatColor rhs) { EXPECT_FALSE(lhs < rhs || rhs < lhs); }
void expectLess(MatColor lhs, MatColor rhs) { EXPECT_TRUE(lhs < rhs); EXPECT_FALSE(rhs < lhs); }
void expectGreater(MatColor lhs, MatColor rhs) { EXPECT_TRUE(rhs < lhs); EXPECT_FALSE(lhs < rhs); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MatColor_Tests, Comparison)
    {
    expectEqual(MatColor(), MatColor());
    expectEqual(makeMatColor(ColorDef::White(), false), makeMatColor(ColorDef::Blue(), false));
    expectGreater(makeMatColor(ColorDef::Black(), true), makeMatColor(ColorDef::White(), false));
    expectLess(makeMatColor(ColorDef::Black(), false), makeMatColor(ColorDef::White(), true));

    // Alpha is ignored
    auto c1 = makeMatColor(ColorDef::Red(), true, 5);
    auto c2 = makeMatColor(ColorDef::Red(), true, 10);
    expectEqual(c1, c2);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct WeightedMatColor : MatColor
{
    double m_weight;

    WeightedMatColor(ColorDef color, double weight = 0.5, bool valid = true) : MatColor(color)
        {
        m_valid = valid;
        m_weight = weight;
        }

    WeightedMatColor() : MatColor(), m_weight(0.5) { }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct Tex : Texture
{
private:
    Tex(CreateParams const& params) : Texture(params) { }

    Dimensions GetDimensions() const { return Dimensions(); }
public:
    static TexturePtr Create()
        {
        return new Tex(Texture::CreateParams());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MatParams makeMatParams(WeightedMatColor diffuse, double transparency, WeightedMatColor specular, double specularExponent, bool textured = false)
    {
    MatParams params;
    params.m_diffuseColor = diffuse;
    params.m_diffuse = diffuse.m_weight;
    params.m_transparency.SetValue(transparency);
    params.m_specularColor = specular;
    params.m_specular = specular.m_weight;
    params.m_specularExponent = specularExponent;

    if (textured)
        {
        auto tex = Tex::Create();
        params.m_textureMapping = TextureMapping(*tex);
        }

    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void expectEqual(MatParams const& lhs, MatParams const& rhs)
    {
    EXPECT_TRUE(MaterialComparison::Equals(lhs, rhs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void expectLess(MatParams const& lhs, MatParams const& rhs)
    {
    EXPECT_TRUE(MaterialComparison::IsLessThan(lhs, rhs));
    EXPECT_FALSE(MaterialComparison::IsLessThan(rhs, lhs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MaterialParams_Tests, Comparison)
    {
    auto a = makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 0.0);
    expectEqual(a, a);

    auto b = makeMatParams(WeightedMatColor(), 1.0, WeightedMatColor(), 0.0);
    expectEqual(b, b);
    expectLess(a, b);

    auto c = makeMatParams(WeightedMatColor(ColorDef::Black(), 0.5), 0.5, WeightedMatColor(ColorDef::Black(), 1.0), 0.5);
    auto d = makeMatParams(WeightedMatColor(ColorDef::Black(), 1.0), 0.5, WeightedMatColor(ColorDef::Black(), 1.0), 0.5);
    expectEqual(c, c);
    expectEqual(d, d);
    expectLess(c, d);
    expectLess(b, c);
    expectLess(a, c);

    auto e = makeMatParams(WeightedMatColor(ColorDef::Black(), 0.5), 0.5, WeightedMatColor(ColorDef::Black(), 1.0), 2.0);
    expectEqual(e, e);
    expectLess(c, e);
    expectLess(d, e);
    expectLess(a, e);
    expectLess(b, e);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct Mat : Material
{
private:
    Mat(MatParams const& params) : Material(params) { }
public:
    static RefCountedPtr<Mat> Create(MatParams const& params) { return new Mat(params); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void mutateColor(MatColor& c)
    {
    c.m_valid = true;
    c.m_value.SetRed(255 - c.m_value.GetRed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void mutateAlpha(MatColor& c)
    {
    c.m_valid = true;
    c.m_value.SetAlpha(255 - c.m_value.GetAlpha());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MaterialComparison_Test, Compare)
    {
    using Params = Material::CreateParams;
    using MutateMaterial = std::function<void(Params& params)>;

    EXPECT_TRUE(MaterialComparison::MatchesDefaults(nullptr));

    MutateMaterial matchDefaults[] =
        {
        [](Params&) { },
        [](Params& p) { mutateColor(p.m_emissiveColor); },
        [](Params& p) { mutateColor(p.m_reflectColor); },
        [](Params& p) { p.m_reflect = 123.456; },
        [](Params& p) { p.m_refract = 123.456; },
        [](Params& p) { p.m_ambient = 123.456; },
        [](Params& p) { p.m_shadows = !p.m_shadows; },
        [](Params& p) { p.m_key = MaterialKey("my_material"); },
        [](Params& p) { mutateAlpha(p.m_emissiveColor); },
        [](Params& p) { mutateAlpha(p.m_reflectColor); },
        };

    for (auto mutate : matchDefaults)
        {
        Params p;
        mutate(p);
        EXPECT_TRUE(MaterialComparison::MatchesDefaults(p));
        EXPECT_FALSE(MaterialComparison::IsLessThan(p, Params()));
        EXPECT_FALSE(MaterialComparison::IsLessThan(Params(), p));

        auto m1 = Mat::Create(Params());
        auto m2 = Mat::Create(p);
        EXPECT_TRUE(MaterialComparison::MatchesDefaults(m2.get()));
        EXPECT_FALSE(MaterialComparison::IsLessThan(m1.get(), m2.get()));
        EXPECT_FALSE(MaterialComparison::IsLessThan(m2.get(), m1.get()));
        EXPECT_FALSE(MaterialComparison::IsLessThan(nullptr, m2.get()));
        EXPECT_FALSE(MaterialComparison::IsLessThan(m2.get(), nullptr));
        }

    MutateMaterial nonDefaults[] =
        {
        [](Params& p) { mutateColor(p.m_diffuseColor); },
        [](Params& p) { mutateColor(p.m_specularColor); },
        [](Params& p) { p.m_transparency.SetValue(0.5); },
        [](Params& p) { p.m_diffuse = 123.456; },
        [](Params& p) { p.m_specular = 123.456; },
        [](Params& p) { p.m_specularExponent = 123.456; },
        [](Params& p) { mutateAlpha(p.m_diffuseColor); },
        [](Params& p) { mutateAlpha(p.m_specularColor); },
        };

    for (auto mutate : nonDefaults)
        {
        Params p;
        mutate(p);
        EXPECT_FALSE(MaterialComparison::MatchesDefaults(p));
        auto lt = MaterialComparison::IsLessThan(p, Params());
        auto gt = MaterialComparison::IsLessThan(Params(), p);
        EXPECT_TRUE(lt || gt);
        EXPECT_FALSE(lt && gt);

        auto m1 = Mat::Create(Params());
        auto m2 = Mat::Create(p);
        EXPECT_FALSE(MaterialComparison::MatchesDefaults(m2.get()));

        lt = MaterialComparison::IsLessThan(m1.get(), m2.get());
        gt = MaterialComparison::IsLessThan(m2.get(), m1.get());
        EXPECT_TRUE(lt || gt);
        EXPECT_FALSE(lt && gt);

        lt = MaterialComparison::IsLessThan(nullptr, m2.get());
        gt = MaterialComparison::IsLessThan(m2.get(), nullptr);
        EXPECT_TRUE(lt || gt);
        EXPECT_FALSE(lt && gt);
        }

    // MatchesDefaults checks texture mapping. IsLessThan does not.
    Params p;
    p.m_textureMapping = TextureMapping(*Tex::Create());
    EXPECT_FALSE(MaterialComparison::MatchesDefaults(p));
    EXPECT_FALSE(MaterialComparison::IsLessThan(p, Params()));
    EXPECT_FALSE(MaterialComparison::IsLessThan(Params(), p));

    auto m1 = Mat::Create(Params());
    auto m2 = Mat::Create(p);
    EXPECT_FALSE(MaterialComparison::MatchesDefaults(m2.get()));
    EXPECT_FALSE(MaterialComparison::IsLessThan(m1.get(), m2.get()));
    EXPECT_FALSE(MaterialComparison::IsLessThan(m2.get(), m1.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MaterialAtlas_Tests, Populate)
    {
    auto p1 = makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 0.0);
    auto m1 = Mat::Create(p1);
    auto m2 = Mat::Create(p1);

    auto a0 = MaterialAtlas::Create(m1.get(), 0);
    EXPECT_TRUE(a0.IsNull());

    auto a1 = MaterialAtlas::Create(m1.get(), 1);
    EXPECT_TRUE(a1.IsValid());
    EXPECT_EQ(a1->NumMaterials(), 1);
    EXPECT_TRUE(a1->IsFull());
    auto i1 = a1->Find(m1.get());
    EXPECT_TRUE(i1.IsValid());
    EXPECT_EQ(i1.Unwrap(), 0);

    // Inserting same material again produces same index
    auto i11 = a1->Insert(m1.get());
    EXPECT_TRUE(i11.IsValid());
    EXPECT_EQ(i11.Unwrap(), i1.Unwrap());
    EXPECT_EQ(a1->NumMaterials(), 1);

    // m2 created from same params therefore maps to same atlas entry
    auto i2 = a1->Find(m2.get());
    EXPECT_TRUE(i2.IsValid());
    EXPECT_EQ(i2.Unwrap(), i1.Unwrap());
    EXPECT_EQ(a1->NumMaterials(), 1);
    i2 = a1->Insert(m2.get());
    EXPECT_TRUE(i2.IsValid());
    EXPECT_EQ(i2.Unwrap(), i1.Unwrap());

    auto p3 = makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 1.0);
    auto m3 = Mat::Create(p3);
    auto i3 = a1->Find(m3.get());
    EXPECT_FALSE(i3.IsValid());
    i3 = a1->Insert(m3.get());
    EXPECT_FALSE(i3.IsValid());

    auto a2 = MaterialAtlas::Create(m1.get(), 2);
    EXPECT_FALSE(a2->IsTranslucent());
    EXPECT_FALSE(m1->GetParams().m_transparency.IsTranslucent());
    auto m4 = Mat::Create(makeMatParams(WeightedMatColor(), 0.5, WeightedMatColor(), 0.0));
    EXPECT_TRUE(m4->GetParams().m_transparency.IsTranslucent());
    auto i4 = a2->Insert(m4.get());
    EXPECT_FALSE(i4.IsValid()); // translucent material can't go into opaque atlas

    auto a3 = MaterialAtlas::Create(m4.get(), 2);
    i4 = a3->Find(m4.get());
    EXPECT_EQ(i4.Unwrap(), 0);
    i1 = a3->Insert(m1.get());
    EXPECT_FALSE(i1.IsValid()); // opaque material can't go into translucent atlas

    auto m5 = Mat::Create(makeMatParams(WeightedMatColor(ColorDef::Black()), 0.5, WeightedMatColor(ColorDef::White()), 0.0));
    auto i5 = a1->Find(m5.get());
    EXPECT_FALSE(i5.IsValid());
    i5 = a3->Insert(m5.get());
    EXPECT_TRUE(i5.IsValid());
    EXPECT_EQ(i5.Unwrap(), 1);
    EXPECT_EQ(a3->NumMaterials(), 2);
    EXPECT_TRUE(a3->IsFull());

    auto m6 = Mat::Create(makeMatParams(WeightedMatColor(ColorDef::Black()), 0.5, WeightedMatColor(ColorDef::White()), 0.0));
    auto i6 = a3->Find(m6.get());
    EXPECT_TRUE(i6.IsValid());
    EXPECT_EQ(i6.Unwrap(), i5.Unwrap());

    auto m7 = Mat::Create(makeMatParams(WeightedMatColor(ColorDef::Black()), 0.5, WeightedMatColor(ColorDef::Blue()), 0.0));
    auto i7 = a3->Insert(m7.get());
    EXPECT_FALSE(i7.IsValid());

    // Textured materials are not permitted in atlas
    auto m8 = Mat::Create(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 0.0, true));
    auto a4 = MaterialAtlas::Create(m8.get(), 5);
    EXPECT_TRUE(a4.IsNull());

    auto m9 = Mat::Create(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 0.0, false));
    a4 = MaterialAtlas::Create(m9.get(), 5);
    EXPECT_TRUE(a4.IsValid());
    auto i8 = a4->Insert(m8.get());
    EXPECT_FALSE(i8.IsValid());

    // nullptr is treated as default material
    Material::CreateParams defaultParams;
    auto ma = Mat::Create(defaultParams);
    auto a5 = MaterialAtlas::Create(ma.get(), 5);
    EXPECT_TRUE(a5.IsValid());
    auto ia = a5->Find(nullptr);
    EXPECT_TRUE(ia.IsValid());
    EXPECT_EQ(ia.Unwrap(), 0);

    auto a6 = MaterialAtlas::Create(nullptr, 5);
    EXPECT_TRUE(a6.IsValid());
    ia = a6->Find(ma.get());
    EXPECT_TRUE(ia.IsValid());
    EXPECT_EQ(ia.Unwrap(), 0);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MatIndices : Mesh::Materials
{
    MatIndices(bool haveAtlas) : Mesh::Materials(haveAtlas) { }

    size_t NumIndices() const
        {
        if (!m_initialized)
            return 0;
        else
            return m_indices.empty() ? 1 : m_indices.size();
        }

    bool IsEmpty() const { return 0 == NumIndices(); }
    bool IsUniform() const { return 1 == NumIndices(); }
    bool IsNonUniform() const { return NumIndices() > 1; }

    bvector<uint8_t> const& Indices() const { return m_indices; }
    uint8_t Uniform() const { return m_uniform; }

    void ExpectIndices(bvector<uint8_t> const& expected) const
        {
        auto const& actual = Indices();
        EXPECT_EQ(actual.size(), expected.size());
        if (actual.size() == expected.size())
            for (size_t i = 0; i < actual.size(); i++)
                EXPECT_EQ(actual[i], expected[i]);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MaterialIndices_Test, Populate)
    {
    // We will never add indices if we don't have a material atlas.
    MatIndices i1(false);
    EXPECT_TRUE(i1.IsEmpty());
    EXPECT_FALSE(i1.IsUniform());
    EXPECT_FALSE(i1.IsNonUniform());

    for (uint8_t i = 0; i < 15; i++)
        i1.Add(i, i);

    EXPECT_TRUE(i1.IsEmpty());
    EXPECT_FALSE(i1.IsUniform());
    EXPECT_FALSE(i1.IsNonUniform());
    EXPECT_TRUE(i1.Indices().empty());

    // We do not add to the vector until we have at least 2 different indices
    MatIndices i2(true);
    EXPECT_TRUE(i2.IsEmpty());
    EXPECT_FALSE(i2.IsUniform());
    EXPECT_FALSE(i2.IsNonUniform());

    bvector<uint8_t> expectedIndices;
    size_t nVerts = 0;
    for ( ; nVerts < 5; nVerts++)
        {
        i2.Add(123, nVerts);
        expectedIndices.push_back(123);
        EXPECT_FALSE(i2.IsEmpty());
        EXPECT_TRUE(i2.IsUniform());
        EXPECT_FALSE(i2.IsNonUniform());
        EXPECT_TRUE(i2.Indices().empty());
        EXPECT_EQ(i2.Uniform(), 123);
        }

    i2.Add(45, ++nVerts);
    expectedIndices.push_back(45);
    EXPECT_FALSE(i2.IsEmpty());
    EXPECT_FALSE(i2.IsUniform());
    EXPECT_TRUE(i2.IsNonUniform());
    i2.ExpectIndices(expectedIndices);

    i2.Add(123, ++nVerts);
    expectedIndices.push_back(123);
    i2.Add(123, ++nVerts);
    expectedIndices.push_back(123);
    i2.Add(0, ++nVerts);
    expectedIndices.push_back(0);
    i2.Add(45, ++nVerts);
    expectedIndices.push_back(45);
    i2.ExpectIndices(expectedIndices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr makeLinearDisplayParams()
    {
    GraphicParams gfParams;
    return DisplayParams::CreateForLinear(gfParams, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr makeTextDisplayParams()
    {
    GraphicParams gfParams;
    return DisplayParams::CreateForText(gfParams, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr makeMeshDisplayParams(Material::CreateParams const& materialParams, uint32_t lineWidth = 1, bool filled = false)
    {
    struct DP : DisplayParams
    {
        DP(SurfaceMaterialCR mat, uint32_t width, FillFlags fill)
            : DisplayParams(ColorDef::White(), ColorDef::White(), width, LinePixels::Solid, mat, fill, DgnCategoryId(), DgnSubCategoryId(), DgnGeometryClass::Primary) { }
    };

    SurfaceMaterial surfaceMaterial(*Mat::Create(materialParams));
    auto fill = filled ? FillFlags::Always : FillFlags::ByView;
    return new DP(surfaceMaterial, lineWidth, fill);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderKey makeBuilderKey(DisplayParamsCR params, Mesh::PrimitiveType type)
    {
    return MeshBuilderKey(params, type, MeshBuilderKey::Flags::None);
    }

//=======================================================================================
// @bsimethod                                                   Paul.Connelly   07/19
//=======================================================================================
TEST(MeshBuilderSet_Test, Populate)
    {
    uint8_t maxMaterials = 2;
    MeshBuilderSet s1(maxMaterials);

    // Linear Mesh does not use material atlas, so only ever has a single builder per key.
    uint32_t numVertices = 1;
    auto k1 = makeBuilderKey(*makeLinearDisplayParams(), Mesh::PrimitiveType::Polyline);
    MeshBuilderP b1 = &s1.GetMeshBuilder(k1, numVertices);
    MeshBuilderListPtr l1 = s1.FindList(k1);
    EXPECT_NOT_NULL(l1.get());
    EXPECT_EQ(s1.size(), 1);
    EXPECT_NULL(b1->GetMesh()->GetMaterialAtlas());

    for (uint8_t i = 0; i < maxMaterials; i++)
        {
        EXPECT_EQ(&s1.GetMeshBuilder(k1, numVertices), b1);
        EXPECT_EQ(l1.get(), s1.FindList(k1).get());
        }

    EXPECT_EQ(l1->size(), 1);
    EXPECT_EQ(s1.size(), 1);

    // Mesh with material must obey max materials per mesh
    MeshBuilderSet s2(maxMaterials);
    auto k2 = makeBuilderKey(*makeMeshDisplayParams(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 1.0)), Mesh::PrimitiveType::Mesh);
    auto b2 = &s2.GetMeshBuilder(k2, numVertices);
    auto l2 = s2.FindList(k2);
    EXPECT_NOT_NULL(l2.get());
    EXPECT_NOT_NULL(b2->GetMesh()->GetMaterialAtlas());
    for (uint8_t i = 0; i < maxMaterials; i++)
        {
        EXPECT_EQ(&s2.GetMeshBuilder(k2, numVertices), b2);
        EXPECT_EQ(s2.FindList(k2).get(), l2.get());
        }

    EXPECT_EQ(l2->size(), 1);
    EXPECT_EQ(s2.size(), 1);

    auto k3 = makeBuilderKey(*makeMeshDisplayParams(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 2.0)), Mesh::PrimitiveType::Mesh);
    auto b3 = &s2.GetMeshBuilder(k3, numVertices);
    EXPECT_EQ(b3, b2);
    EXPECT_EQ(l2->size(), 1);
    EXPECT_EQ(s2.size(), 1);

    auto k4 = makeBuilderKey(*makeMeshDisplayParams(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 3.0)), Mesh::PrimitiveType::Mesh);
    auto b4 = &s2.GetMeshBuilder(k4, numVertices);
    EXPECT_NE(b4, b2);
    EXPECT_EQ(s2.FindList(k4).get(), l2.get());
    EXPECT_EQ(l2->size(), 2);
    EXPECT_EQ(s2.size(), 1);
    EXPECT_NOT_NULL(b4->GetMesh()->GetMaterialAtlas());

    // Second builder in s2 has room in material atlas for another material, but translucent and opaque materials cannot inhabit same atlas
    auto k5 = makeBuilderKey(*makeMeshDisplayParams(makeMatParams(WeightedMatColor(), 0.5, WeightedMatColor(), 4.0)), Mesh::PrimitiveType::Mesh);
    auto b5 = &s2.GetMeshBuilder(k5, numVertices);
    EXPECT_NE(b5, b2);
    EXPECT_NE(b5, b4);
    auto l5 = s2.FindList(k5);
    EXPECT_NE(l5.get(), l2.get());
    EXPECT_EQ(l5->size(), 1);
    EXPECT_EQ(l2->size(), 2);
    EXPECT_EQ(s2.size(), 2);
    EXPECT_NOT_NULL(b5->GetMesh()->GetMaterialAtlas());

    // Linear and Mesh type meshes do not mix
    auto& b = s2.GetMeshBuilder(k1, numVertices);
    EXPECT_EQ(s2.size(), 3);
    EXPECT_NULL(b.GetMesh()->GetMaterialAtlas());

    // Text never has a material, does not use atlas, does not batch with other meshes
    auto textKey = makeBuilderKey(*DisplayParams::CreateForText(GraphicParams(), nullptr), Mesh::PrimitiveType::Mesh);
    MeshBuilderR textBuilder = s2.GetMeshBuilder(textKey, numVertices);
    EXPECT_NULL(textBuilder.GetMesh()->GetMaterialAtlas());
    EXPECT_EQ(s2.size(), 4);

    // Textured materials do not use atlas, do not batch with other meshes
    auto textureKey = makeBuilderKey(*makeMeshDisplayParams(makeMatParams(WeightedMatColor(), 0.0, WeightedMatColor(), 1.0, true)), Mesh::PrimitiveType::Mesh);
    MeshBuilderR textureBuilder = s2.GetMeshBuilder(textureKey, numVertices);
    EXPECT_NULL(textureBuilder.GetMesh()->GetMaterialAtlas());
    EXPECT_EQ(s2.size(), 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MeshBuilderSet_Test, MaxVertexCount)
    {
    MeshBuilderSet s(255, 100);
    auto key = makeBuilderKey(*makeLinearDisplayParams(), Mesh::PrimitiveType::Polyline);
    auto getMeshBuilder = [&](uint32_t numVertices)
        {
        MeshBuilderP builder = &s.GetMeshBuilder(key, numVertices);
        auto& points = builder->GetMesh()->VertsR();
        points.resize(points.size() + numVertices);
        return builder;
        };

    auto countBuilders = [&]()
        {
        auto list = s.FindList(key);
        EXPECT_TRUE(list.IsValid());
        return list->size();
        };

    MeshBuilderP b75 = getMeshBuilder(75);
    EXPECT_NOT_NULL(b75);
    EXPECT_EQ(countBuilders(), 1);

    MeshBuilderP b50 = getMeshBuilder(50);
    EXPECT_NOT_NULL(b50);
    EXPECT_NE(b75, b50);
    EXPECT_EQ(countBuilders(), 2);

    MeshBuilderP b25 = getMeshBuilder(25);
    EXPECT_EQ(b75, b25);
    EXPECT_EQ(countBuilders(), 2);

    MeshBuilderP b1 = getMeshBuilder(1);
    EXPECT_EQ(b50, b1);
    EXPECT_EQ(countBuilders(), 2);

    // If a request for a MeshBuilder exceeds max vertices, we still return a new one and allow it to overflow.
    MeshBuilderP b200 = getMeshBuilder(200);
    EXPECT_NOT_NULL(b200);
    EXPECT_NE(b75, b200);
    EXPECT_NE(b50, b200);
    EXPECT_EQ(countBuilders(), 3);
    }
