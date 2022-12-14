/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/VolumeElement.h>
#include <DgnPlatform/GenericDomain.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Dgn"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//! VolumeElementTestFixture
//=======================================================================================
struct VolumeElementTestFixture : public ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    VolumeElementCPtr InsertVolume(DPoint3dCR origin, DPoint2d shapeArr[5], double height, Utf8CP label);
    GenericPhysicalObjectCPtr InsertBlock(DPoint3dCR origin, double dimension);

public:
    VolumeElementTestFixture() {}
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
VolumeElementCPtr VolumeElementTestFixture::InsertVolume(DPoint3dCR origin, DPoint2d shapeArr[5], double height, Utf8CP label)
    {
    bvector<DPoint2d> shape(shapeArr, shapeArr + 5);
    VolumeElementPtr volume = VolumeElement::Create(*(m_defaultModel->ToSpatialModel()), origin, shape, height, label);
    return volume->Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericPhysicalObjectCPtr VolumeElementTestFixture::InsertBlock(DPoint3dCR center, double dimension)
    {
    GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(*(m_defaultModel->ToPhysicalModelP()), m_defaultCategoryId);

    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(dimension, dimension, dimension), true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    GeometryBuilderPtr builder = GeometryBuilder::Create(*m_defaultModel, m_defaultCategoryId, center, YawPitchRollAngles());
    builder->Append(*geomPtr);
    BentleyStatus status = builder->Finish(*physicalElementPtr);
    BeAssert(status == SUCCESS);

    GenericPhysicalObjectCPtr insertedElement = m_db->Elements().Insert<GenericPhysicalObject>(*physicalElementPtr);
    BeAssert(insertedElement.IsValid());

    return insertedElement;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
TEST_F(VolumeElementTestFixture, CrudTest)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"CrudTest.bim");

    DPoint3d origin = {0.0, 0.0, 0.0};
    DPoint2d shapePointsArr[5] = {{0.0, 0.0}, {100.0, 0.0}, {100.0, 100.0}, {0.0, 100.0}, {0.0, 0.0}};
    double height = 100.0;
    Utf8CP name = "CrudTestVolume";
    VolumeElementCPtr volume = InsertVolume(origin, shapePointsArr, height, name);
    ASSERT_TRUE(volume.IsValid());

    DgnElementIdSet volIdSet = VolumeElement::QueryVolumes(*m_db);
    ASSERT_EQ(1, (int) volIdSet.size());

    DgnElementId volId = VolumeElement::QueryVolumeByLabel(*m_db, name);
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

    VolumeElementPtr volumeEdit = VolumeElement::GetForEdit(*m_db, volume->GetElementId());
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
