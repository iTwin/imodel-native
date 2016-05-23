/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/GeomStream_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
class GeometryStreamTest : public testing::Test
{
protected:
ScopedDgnHost       m_host;
DgnDbTestDgnManager m_testDataManager;

public:
    GeometryStreamTest() : m_testDataManager (L"2dMetricGeneral.ibim", "", Db::OpenMode::ReadWrite, false){BeAssert( NULL != GetDgnModelP() );}
    virtual ~GeometryStreamTest () {}

    DgnModelP GetDgnModelP() {return m_testDataManager.GetDgnModelP();}
};
    
static bool hasGeom(GeometryStreamCR el) {return (NULL != el.GetData()) && 3 == el.GetSize();}
static bool sameGeomPtr(GeometryStreamCR el1, GeometryStreamCR el2) {return hasGeom(el1) && (el1.GetData() == el2.GetData());}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
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
