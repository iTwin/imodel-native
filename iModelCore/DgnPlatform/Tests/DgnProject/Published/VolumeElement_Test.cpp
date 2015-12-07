/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/VolumeElement_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnHandlersAPI.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Dgn"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
//! VolumeElementTestFixture
//=======================================================================================
struct VolumeElementTestFixture : public ChangeTestFixture
{
protected:
    void CreateSample(WCharCP fileName);
    VolumeElementCPtr InsertVolume(DPoint3dCR origin, DPoint2d shapeArr[5], double height, Utf8CP label);
    PhysicalElementCPtr InsertBlock(DPoint3dCR origin, double dimension);

public:
    virtual void SetUp() override {}
    virtual void TearDown() override { m_testDb->SaveChanges("Saving file at end of test"); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
void VolumeElementTestFixture::CreateSample(WCharCP fileName)
    {
    CreateDgnDb(fileName);
    InsertModel();
    InsertCategory();
    InsertAuthority();
    m_testDb->SaveChanges("Inserted sample");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
VolumeElementCPtr VolumeElementTestFixture::InsertVolume(DPoint3dCR origin, DPoint2d shapeArr[5], double height, Utf8CP label)
    {
    bvector<DPoint2d> shape(shapeArr, shapeArr + 5);

    VolumeElementPtr volume = VolumeElement::Create(*(m_testModel->ToPhysicalModel()), origin, shape, height, label);
    return volume->Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
PhysicalElementCPtr VolumeElementTestFixture::InsertBlock(DPoint3dCR center, double dimension)
    {
    PhysicalElementPtr physicalElementPtr = PhysicalElement::Create(*(m_testModel->ToPhysicalModelP()), m_testCategoryId);

    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(dimension, dimension, dimension), true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*m_testModel, m_testCategoryId, center, YawPitchRollAngles());
    builder->Append(*geomPtr);
    BentleyStatus status = builder->SetGeomStreamAndPlacement(*physicalElementPtr);
    BeAssert(status == SUCCESS);

    PhysicalElementCPtr insertedElement = m_testDb->Elements().Insert<PhysicalElement>(*physicalElementPtr);
    BeAssert(insertedElement.IsValid());

    return insertedElement;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
TEST_F(VolumeElementTestFixture, CrudTest)
    {
    WCharCP fileName = L"VolumeElementCrudTest.dgndb";
    CreateSample(fileName);

    DPoint3d origin = {0.0, 0.0, 0.0};
    DPoint2d shapePointsArr[5] = {{0.0, 0.0}, {100.0, 0.0}, {100.0, 100.0}, {0.0, 100.0}, {0.0, 0.0}};
    double height = 100.0;
    Utf8CP name = "CrudTestVolume";
    VolumeElementCPtr volume = InsertVolume(origin, shapePointsArr, height, name);
    ASSERT_TRUE(volume.IsValid());

    DgnElementIdSet volIdSet = VolumeElement::QueryVolumes(*m_testDb);
    ASSERT_EQ(1, (int) volIdSet.size());

    DgnElementId volId = VolumeElement::QueryVolumeByLabel(*m_testDb, name);
    ASSERT_EQ(volume->GetElementId(), volId);

    bvector<DPoint3d> actualShape;
    DVec3d actualDirection;
    double actualHeight;
    BentleyStatus actualStatus;

    actualStatus = volume->ExtractGeomStream(actualShape, actualDirection, actualHeight);

    /*
    * Validate
    */
    ASSERT_TRUE(SUCCESS == actualStatus);
    ASSERT_EQ(height, actualHeight);
    DVec3d expectedDirection = DVec3d::From(0.0, 0.0, 1.0);
    ASSERT_TRUE(expectedDirection.IsEqual(actualDirection));
    for (int ii = 0; ii < 5; ii++)
        {
        DPoint3d expectedPoint = DPoint3d::FromSumOf(origin, DPoint3d::From(shapePointsArr[ii]));
        ASSERT_TRUE(actualShape[ii].IsEqual(expectedPoint));
        }

    VolumeElementPtr volumeEdit = VolumeElement::GetForEdit(*m_testDb, volume->GetElementId());
    ASSERT_TRUE(volumeEdit.IsValid());

    // Update
    //name = "CrudTest1";
    origin = {1000.0, 1000.0, 1000.0};
    DPoint2d shapePointsArr2[5] = {{0.0, 0.0}, {200.0, 0.0}, {100.0, 200.0}, {0.0, 200.0}, {0.0, 0.0}};
    int numShapePoints2 = (size_t) (sizeof(shapePointsArr2) / sizeof(DPoint2d));
    bvector<DPoint2d> shapePoints2(shapePointsArr2, shapePointsArr2 + numShapePoints2);
    height = 200.0;
    volumeEdit->SetupGeomStream(origin, shapePoints2, height);
    VolumeElementCPtr volumeUpdate = volumeEdit->Update();
    ASSERT_TRUE(volumeUpdate.IsValid());

    actualStatus = volumeUpdate->ExtractGeomStream(actualShape, actualDirection, actualHeight);
    
    /*
    * Validate
    */
    ASSERT_TRUE(SUCCESS == actualStatus);
    ASSERT_EQ(height, actualHeight);
    ASSERT_TRUE(expectedDirection.IsEqual(actualDirection));
    for (int ii = 0; ii < 5; ii++)
        {
        DPoint3d expectedPoint = DPoint3d::FromSumOf(origin, DPoint3d::From(shapePointsArr2[ii]));
        ASSERT_TRUE(actualShape[ii].IsEqual(expectedPoint));
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
TEST_F(VolumeElementTestFixture, QueryTest)
    {
    WCharCP fileName = L"VolumeElementQueryTest.dgndb";
    CreateSample(fileName);

    // Entirely inside
    PhysicalElementCPtr insideEl = InsertBlock(DPoint3d::From(37.5, 37.5, 37.5), 25.0);
    InsertBlock(DPoint3d::From(-37.5, -37.5, -37.5), 25.0);

    // Partly overlaps
    PhysicalElementCPtr overlapEl = InsertBlock(DPoint3d::From(100.0, 100.0, 100.0), 25.0);

    // Entirely outside
    PhysicalElementCPtr outsideEl = InsertBlock(DPoint3d::From(150.0, 150.0, 150.0), 25.0);
    InsertBlock(DPoint3d::From(-150.0, -150.0, -150.0), 25.0);
    InsertBlock(DPoint3d::From(0.0, 0.0, 150.0), 25.0); 
    InsertBlock(DPoint3d::From(0.0, 0.0, -150.0), 25.0);

    // Volume Item
    DPoint3d origin = {0.0, 0.0, -100.0};
    DPoint2d shapePointsArr[5] = {{-100.0, -100.0}, {100.0, -100.0}, {100.0, 100.0}, {-100.0, 100.0}, {-100.0, -100.0}};
    double height = 200.0;
    VolumeElementCPtr volume = InsertVolume(origin, shapePointsArr, height, "QueryTestVolume");
    ASSERT_TRUE(volume.IsValid());

    CreateDefaultView();
    UpdateDgnDbExtents();
    m_testDb->SaveChanges("Finished inserts");
    
    DgnElementIdSet idSet;
    volume->FindElements(idSet, *m_testDb, false);
    ASSERT_EQ(2, (int) idSet.size());

    idSet.clear();
    volume->FindElements(idSet, *m_testDb, true);
    ASSERT_EQ(3, (int) idSet.size());

    ASSERT_TRUE(volume->ContainsElement(*insideEl, false));
    ASSERT_TRUE(volume->ContainsElement(*insideEl, true));

    ASSERT_FALSE(volume->ContainsElement(*overlapEl, false));
    ASSERT_TRUE(volume->ContainsElement(*overlapEl, true));

    ASSERT_FALSE(volume->ContainsElement(*outsideEl, false));
    ASSERT_FALSE(volume->ContainsElement(*outsideEl, true));

    // TODO: SetClip(), ClearClip(), Fit()
    // TODO: Show(), Hide()
    }
