/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass
//=======================================================================================
class GeometryStreamTest : public GenericDgnModelTestFixture
{
protected:
    DgnModelPtr                 m_defaultModelP;

public:
    GeometryStreamTest() {  }
    virtual ~GeometryStreamTest () {}

    DgnModelP GetDgnModelP() { return m_defaultModelP.get(); }
    virtual void SetUp()
        {
        DgnDbPtr db = GetDgnDb(WString(TEST_NAME, true).c_str());
        m_defaultModelP = db->Models().GetModel(DgnDbTestUtils::QueryFirstGeometricModelId(*db));
        ASSERT_TRUE(m_defaultModelP.IsValid());
        }
};

static bool hasGeom(GeometryStreamCR el) {return (NULL != el.GetData()) && 3 == el.GetSize();}
static bool sameGeomPtr(GeometryStreamCR el1, GeometryStreamCR el2) {return hasGeom(el1) && (el1.GetData() == el2.GetData());}

//=======================================================================================
// @bsiclass
//=======================================================================================
TEST_F(GeometryStreamTest, DgnElement)
    {
    // allocate a ElementGeom and put some data in its graphics
    GeometryStream eg1;
    ASSERT_TRUE(!hasGeom(eg1));
    eg1.Resize(3);
    memset (eg1.GetDataP(), 5, 3);
    ASSERT_TRUE(hasGeom(eg1));

    // a move constructor should steal the data from the other
    GeometryStream eg2 = std::move(eg1);
    ASSERT_TRUE(hasGeom(eg2));
    ASSERT_TRUE(!hasGeom(eg1));

    // a copy constructor should not steal the data from the other
    GeometryStream eg3 = eg2;
    ASSERT_TRUE(hasGeom(eg2));
    ASSERT_TRUE(hasGeom(eg3));
    ASSERT_TRUE(!sameGeomPtr(eg2,eg3));

    // a move operator should steal
    eg1 = std::move(eg2);
    ASSERT_TRUE(!hasGeom(eg2));
    ASSERT_TRUE(hasGeom(eg1));

    // make sure a move operator with valid data should point to the other side's data and original should be freed.
    void const* g3 = eg3.GetData();
    ASSERT_TRUE(eg1.GetData() != g3);
    eg1 = std::move(eg3);
    ASSERT_TRUE(!hasGeom(eg3));
    ASSERT_TRUE(hasGeom(eg1));
    ASSERT_TRUE(g3 == eg1.GetData());

    // copy operator should make a new copy of the graphics
    eg2 = eg1;
    ASSERT_TRUE(hasGeom(eg2));
    ASSERT_TRUE(hasGeom(eg1));
    ASSERT_TRUE(!sameGeomPtr(eg2,eg1));

    GeometryStream eg4;
    eg4.Resize(10);
    memset (eg4.GetDataP(), 4, 10);

    // an element with a graphics buffer that is large enough to hold the data from a copy operator should not need to realloc.
    void const* g4 = eg4.GetData();
    eg4 = eg1;
    ASSERT_TRUE(hasGeom(eg4));
    ASSERT_TRUE(hasGeom(eg1));
    ASSERT_TRUE(g4 == eg4.GetData()); // pointer should be the same, but buffer should still be allocated to old size
    ASSERT_TRUE(10 == eg4.GetAllocSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryStreamTest, ViewIndependent)
    {
    using Flags = GeometryStreamIO::Header::Flags;

    // An empty geometry stream is not view-independent
    GeometryStream noFlags;
    EXPECT_FALSE(noFlags.IsViewIndependent());

    auto testViewIndependent = [&](GeometryBuilderR builder, bool expectViewIndependent)
        {
        GeometryStream stream;
        EXPECT_EQ(0, builder.GetGeometryStream(stream));
        EXPECT_EQ(stream.IsViewIndependent(), expectViewIndependent);

        auto expectFlags = [&](Flags flags)
            {
            auto expectedFlags = static_cast<uint32_t>(expectViewIndependent ? Flags::ViewIndependent : Flags::None);
            EXPECT_EQ(static_cast<uint32_t>(flags), expectedFlags);
            };

        GeometryStreamIO::Collection geom(stream.data(), stream.size());
        expectFlags(geom.GetHeader().m_flags);

        GeometryCollection collection(stream, *GetDgnDb());
        expectFlags(collection.GetHeaderFlags());
        BeJsDocument json;
        collection.ToJson(json);
        EXPECT_TRUE(json[0].isMember("header"));
        auto const& jsonHeader = json[0]["header"];
        EXPECT_TRUE(jsonHeader.isMember("flags"));
        auto flags = static_cast<Flags>(jsonHeader["flags"].asUInt());
        expectFlags(flags);

        builder.FromJson(json);
        GeometryStream jsonStream;
        EXPECT_EQ(0, builder.GetGeometryStream(jsonStream));
        EXPECT_EQ(jsonStream.IsViewIndependent(), expectViewIndependent);
        };

    DgnCategoryId dummyCategoryId((uint64_t) 1);
    auto builder = GeometryBuilder::CreateWithAutoPlacement(*GetDgnModelP(), dummyCategoryId);
    testViewIndependent(*builder, false);

    builder = GeometryBuilder::CreateWithAutoPlacement(*GetDgnModelP(), dummyCategoryId);
    builder->SetHeaderFlags(GeometryStreamIO::Header::Flags::ViewIndependent);
    testViewIndependent(*builder, true);
    }
