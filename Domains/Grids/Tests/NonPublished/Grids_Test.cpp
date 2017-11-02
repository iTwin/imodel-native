/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/Grids_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform\FunctionalDomain.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// Sets up environment for Grids testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct GridsTestFixture : public GridsTestFixtureBase
    {
    protected:
        SpatialLocationModelPtr m_model;
    public:
        GridsTestFixture() {};
        ~GridsTestFixture() {};

        void SetUp() override;
        void TearDown() override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp ();
    DgnDbR db = *DgnClientApp::App ().Project ();
    SubjectCPtr rootSubject = db.Elements ().GetRootSubject ();
    SpatialLocationPartitionCPtr partition = SpatialLocationPartition::CreateAndInsert (*rootSubject, "GridSpatialPartition");
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::TearDown()
    {
    m_model = nullptr;
    GridsTestFixtureBase::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, CreateOrthogonalGrid_Unconstrained)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    DVec3d normal = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From (0.0,0.0,0.0);
    DVec3d vertExtTrans = DVec3d::From (0.0,0.0,0.0);
    OrthogonalGridPortion::StandardCreateParams createParams (m_model.get(), db.Elements ().GetRootSubject ()->GetElementId(), 5, 4, 12.5, 12.5, 50, 50, normal, horizExtTrans, vertExtTrans, false, false, "Unconstrained Grid");

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert (createParams);

    db.SaveChanges ();

    ASSERT_TRUE (orthogonalGridUnconstrained.IsValid()) << "Failed to create orthogonal grid";

    ASSERT_TRUE (orthogonalGridUnconstrained->GetSurfacesModel().IsValid()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrained->MakeIterator ().BuildIdList<DgnElementId> ().size ();
    ASSERT_TRUE (numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    //TODO: check geometry & positions of all gridplaneSurfaces
    //TODO: check other methods - transform rotate etc
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, CreateOrthogonalGrid_Constrained)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();
    DVec3d normal = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d horizExtTrans = DVec3d::From (0.0, 0.0, 0.0);
    DVec3d vertExtTrans = DVec3d::From (0.0, 0.0, 0.0);
    OrthogonalGridPortion::StandardCreateParams createParams (m_model.get (), db.Elements ().GetRootSubject ()->GetElementId (), 5, 4, 12.5, 12.5, 50, 50, normal, horizExtTrans, vertExtTrans, true, false, "Constrained Grid");

    OrthogonalGridPortionPtr orthogonalGridUnconstrained = OrthogonalGridPortion::CreateAndInsert (createParams);

    db.SaveChanges ();

    ASSERT_TRUE (orthogonalGridUnconstrained.IsValid ()) << "Failed to create orthogonal grid";

    ASSERT_TRUE (orthogonalGridUnconstrained->GetSurfacesModel ().IsValid ()) << "Failed to get orthogonal grid surfaces model";

    int numSurfaces = orthogonalGridUnconstrained->MakeIterator ().BuildIdList<DgnElementId> ().size ();
    ASSERT_TRUE (numSurfaces == 9) << "incorrect number of gridPlaneSurfaces in orthogonalGrid";

    //TODO: check geometry & positions of all gridplaneSurfaces
    //TODO: check other methods - transform rotate etc
    }



//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GridsTestFixture, InsertHandlerCreatedElements)
    {
    DgnDbR db = *DgnClientApp::App ().Project ();


    {
    // create new definition model
    GridSurfaceHandler& handler = GridSurfaceHandler::GetHandler ();
    DgnClassId classId = db.Domains ().GetClassId (handler);
    DgnElement::CreateParams params (db, m_model->GetModelId (), classId);

    GeometricElement3dPtr element = dynamic_cast<GeometricElement3d*>(handler.Create (params).get());
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);

    element->SetCategoryId (categoryId);
    element->Insert ();

    ASSERT_TRUE (!element->GetElementId ().IsValid ()) << "should fail to insert surface created via handler";
    }

    {
    // create new definition model
    GridPortionHandler& handler = GridPortionHandler::GetHandler ();
    DgnClassId classId = db.Domains ().GetClassId (handler);
    DgnElement::CreateParams params (db, m_model->GetModelId (), classId);

    GeometricElement3dPtr element = dynamic_cast<GeometricElement3d*>(handler.Create (params).get ());
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (db.GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);

    element->SetCategoryId (categoryId);
    element->Insert ();

    ASSERT_TRUE (!element->GetElementId ().IsValid ()) << "should fail to insert portion created via handler";
    }

    {
    // create new definition model
    GridAxisHandler& handler = GridAxisHandler::GetHandler ();
    DgnClassId classId = db.Domains ().GetClassId (handler);
    DgnElement::CreateParams params (db, m_model->GetModelId (), classId);

    DgnElementPtr element = handler.Create (params);
    element->Insert ();

    ASSERT_TRUE (!element->GetElementId ().IsValid ()) << "should fail to insert axis created via handler";
    }
    db.SaveChanges ();
    //TODO: check geometry & positions of all gridplaneSurfaces
    //TODO: check other methods - transform rotate etc
    }
