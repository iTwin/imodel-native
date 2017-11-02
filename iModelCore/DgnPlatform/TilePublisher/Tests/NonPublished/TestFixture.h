/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include <TilePublisher/CesiumPublisher.h>
#include <DgnPlatform/TileReader.h>
#include <DgnPlatform/TileWriter.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

// Contains many helpful functions for inserting models and elements
// #include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_TILETREE
USING_NAMESPACE_TILETREE_IO

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct TestFixture : public ::testing::Test
{
protected:
    ScopedDgnHost   m_host;
    DgnDbPtr        m_db;

    void TearDown() override { SaveDb(); }
public:

// [ === Managing the DgnDb === ]

    // Create a blank DgnDb to hold data to be published
    void SetupDb(WCharCP filenameWithoutExtension);

    // Get the DgnDb produced by SetupDb()
    DgnDbR GetDb() { BeAssert(m_db.IsValid()); return *m_db; }

    // Close the DgnDb.
    void CloseDb() { GetDb().CloseDb(); }

    // Update the project extents for the DgnDb. You generally want to do this before publishing tiles.
    // It can also be convenient to set a view's frustum to match the project extents.
    AxisAlignedBox3d UpdateProjectExtents() { DgnDbTestUtils::UpdateProjectExtents(GetDb()); return GetDb().GeoLocation().GetProjectExtents(); }

    // Save all changes to the DgnDb. You generally want to do this before publishing tiles.
    void SaveDb()
        {
        if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
            m_db->SaveChanges();
        }

// [ === Executing Tests === ]

    // If a test obtains ref-counted pointers to DgnElements, and does not explicitly set all of them to null before the test terminates,
    // DgnDb will assert. Pass your test function/lambda to this function to ensure any DgnElementPtrs within it are released before termination.
    template<typename T> void ExecuteTest(T testFunc) { testFunc(); }

// [ == Creating definition elements === ]

    // Create a spatial (3d) model
    PhysicalModelPtr InsertSpatialModel(Utf8CP partitionName) { return DgnDbTestUtils::InsertPhysicalModel(GetDb(), partitionName); }

    // Create a spatial category (for 3d elements)
    DgnCategoryId InsertSpatialCategory(Utf8CP name) { return DgnDbTestUtils::InsertSpatialCategory(GetDb(), name); }

    // Spatial (3d) views can display any number of spatial models, defined by a ModelSelector which holds a DgnModelIdSet of the viewed models.
    ModelSelectorCPtr InsertModelSelector(DgnModelId modelId, Utf8CP name="") { DgnModelIdSet modelIds; modelIds.insert(modelId); return InsertModelSelector(modelIds, name); }
    ModelSelectorCPtr InsertModelSelector(DgnModelIdSet const& modelIds, Utf8CP name="");

    // A view can display any number of categories, defined by a CategorySelector which holds a DgnCategoryIdSet of the viewed categories.
    CategorySelectorCPtr InsertCategorySelector(DgnCategoryId catId, Utf8CP name="") { DgnCategoryIdSet catIds; catIds.insert(catId); return InsertCategorySelector(catIds, name); }
    CategorySelectorCPtr InsertCategorySelector(DgnCategoryIdSet const& catIds, Utf8CP name="");

    DisplayStyle3dCPtr InsertDisplayStyle3d(Utf8CP name, ColorDef backgroundColor=ColorDef::Black(), bool groundPlane=false, ViewFlags viewFlags=ViewFlags());

    SpatialViewDefinitionCPtr InsertSpatialView(Utf8CP name, ModelSelectorCR models, CategorySelectorCR categories, DisplayStyle3dCR style, DRange3dCP viewedVolume, SpatialViewDefinition::Camera const* camera=nullptr);

// [ === Defining element geometry === ]

    // Points are specified in local coordinates relative to element origin.

    CurveVectorPtr CreateShape(DPoint3dCP points, size_t numPoints) { return CurveVector::CreateLinear(points, numPoints, CurveVector::BOUNDARY_TYPE_Outer); }
    CurveVectorPtr CreateRectangle(DPoint3dCR lowerLeft, double width, double height);
    CurveVectorPtr CreateTriangle(DPoint3dCR lowerLeft, double width, double height);
    CurveVectorPtr CreateLineString(DPoint3dCP points, size_t numPoints) { return CurveVector::CreateLinear(points, numPoints, CurveVector::BOUNDARY_TYPE_Open); }

    GeometryBuilderPtr CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId) { return GeometryBuilder::Create(model, categoryId, DPoint3d::FromZero()); }
    GeometryBuilderPtr CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId, ColorDef color);

// [ === Creating geometric elements === ]

    PhysicalElementCPtr InsertPhysicalElement(PhysicalModelR model, GeometryBuilderR builder, DPoint3dCR elementOrigin);

// [ === Publishing tiles === ]



};

