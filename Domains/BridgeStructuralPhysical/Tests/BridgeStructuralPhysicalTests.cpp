/*--------------------------------------------------------------------------------------+
|
|     $Source$
|
|  $Copyright$
|
+--------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalTestFixture.h"
#include "TestHelper.h"
#include "GeometryCreation.h"

#include <BridgeStructuralPhysical/BridgeStructuralPhysicalApi.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomain.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomainUtilities.h>
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>
#include <BridgeStructuralPhysical/BridgeSubstructure.h>
#include <BridgeStructuralPhysical/BridgeSuperstructure.h>

#include <StructuralDomain/StructuralPhysicalApi.h>
#include <FormsDomain/FormsDomainApi.h>
//#include <FormsDomain/CurvedProfiledLoftedExtrusion.h>
//#include <ProfilesDomain/ProfilesDomainApi.h>
//#include <ProfilesDomain/ProfilesModel.h>
//#include <ProfilesDomain/ConstantProfile.h>
//#include <ProfilesDomain/BuiltUpProfileComponent.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley/BeAssert.h>
#include <DgnPlatform/DgnCoreAPI.h>
#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME          "SampleDynamic" 

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, EnsureDomainsAreRegistered)
    {
	// Ensure that Domains were registered in Test Fixture Setup
    DgnDbPtr db = CreateDgnDb();
    ASSERT_TRUE(db.IsValid());
	DgnDomainCP linearReferencingDomain = db->Domains().FindDomain(LinearReferencingDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != linearReferencingDomain);
	DgnDomainCP roadRailAlignmentDomain = db->Domains().FindDomain(RoadRailAlignmentDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != roadRailAlignmentDomain);
	//DgnDomainCP profilesDomain = db->Domains().FindDomain(Profiles::ProfilesDomain::GetDomain().GetDomainName());
	//ASSERT_TRUE(NULL != profilesDomain);
	DgnDomainCP formDomain = db->Domains().FindDomain(Forms::FormsDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != formDomain);
	DgnDomainCP structPhysDomain = db->Domains().FindDomain(StructuralPhysicalDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != structPhysDomain);
    DgnDomainCP bridgeStructPhysDomain = db->Domains().FindDomain(BridgeStructuralPhysicalDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != bridgeStructPhysDomain);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, ValidateSchema)
    {
	// Test validitiy of BridgeStructuralPhysical Schema
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext(true, true);
    context->AddSchemaLocater((*db).GetSchemaLocater());

    ECN::SchemaKey refKey = ECN::SchemaKey(BBP_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema(refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(refSchema.IsValid());

    ASSERT_TRUE(refSchema->Validate());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, CreatePhysicalPartitionMinimalArgs)
    {
	// Testing the minimal arguments for Create PhysicalModels where our structural bridge elements will live
    DgnDbPtr db = CreateDgnDb();
    ASSERT_TRUE(db.IsValid());
    PhysicalModelCPtr  bridgeStructPhysModelCPtr = BridgeStructuralPhysical::BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  06/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, BasicCreateAndDeleteAlignmentTest)
	{
	// Test Alignment Creation and Delete propogation
	DgnDbPtr db = CreateDgnDb(L"BridgeStructPhysAlignment.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	auto horizAlignmPtr = alignmentPtr->QueryHorizontal();
	auto verticalAlignmPtr = alignmentPtr->QueryMainVertical();
	auto verticalModelPtr = verticalAlignmPtr->GetModel();
	
	// Delete-cascade
	ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->Delete());
	ASSERT_TRUE(db->Elements().GetElement(alignmentPtr->GetElementId()).IsNull());
	ASSERT_TRUE(db->Elements().GetElement(horizAlignmPtr->GetElementId()).IsNull());
	ASSERT_TRUE(db->Elements().GetElement(verticalAlignmPtr->GetElementId()).IsNull());
	ASSERT_TRUE(db->Models().GetModel(verticalModelPtr->GetModelId()).IsNull());
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, CreateAlignmentWithSubstructureGeomViewTest)
	{
	// Test Alignment Creation with Geom View
	DgnDbPtr db = CreateDgnDb(L"AlignmentWithSupportsAndView.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	PhysicalModelPtr bridgeStructPhysModelPtr = BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelPtr.IsValid());

	// Create Substructure element	
	GeometricElement3d::CreateParams createParams(*db, bridgeStructPhysModelPtr->GetModelId(), GenericSubstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	auto atDE = DistanceExpression(75);
	ILinearlyLocatedSingleAt::CreateAtParams atParams(*alignmentPtr, atDE);
	GenericSubstructureElementPtr substructureElementPtr = GenericSubstructureElement::Create(createParams, atParams,  0.0);
	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelPtr, "Support1");
	status = TestHelper::SetDgnElementCodeAndInsert(substructureElementPtr, code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == 
		TestHelper::CreateAndPositionMultiColumnPierMembers(substructureElementPtr, bridgeStructPhysModelPtr));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelPtr,
	alignModelPtr);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, CreateAlignmentWithBoxGirderSuperstructureGeomTest)
	{
	// This is an approximation to how the box girder superstructure geometry added to the bim DB,
	// Actual implementation of the forms schema definition of Lofted Extrusion is pending
	//TODO: implement with structural subtraction for the holes
	DgnDbPtr db = CreateDgnDb(L"AlignmentWithBoxGirderSuperstructureGeom.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	PhysicalModelCPtr bridgeStructPhysModelCPtr = BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	GeometricElement3d::CreateParams createParams(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSuperstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	auto atDE = DistanceExpression(0);
	auto superstructureElementPtr = GenericSuperstructureElement::Create(createParams, *alignmentPtr, atDE);
	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "Superstructure1");
	status = TestHelper::SetDgnElementCodeAndInsert(superstructureElementPtr, code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	status = TestHelper::CreateAndPositionBoxGirderSuperstructureMembers(superstructureElementPtr, bridgeStructPhysModelCPtr);
	ASSERT_TRUE(status == Dgn::DgnDbStatus::Success);
	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelCPtr,
		alignModelPtr);
	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, CreateAlignmentWithGirderSlabSuperstructureGeomTest)
	{
	// This is an approximation to how the girder slab superstructure geometry added to the bim DB,
	// Actual implementation of the forms schema definition of Lofted Extrusion is pending
	DgnDbPtr db = CreateDgnDb(L"AlignmentWithGirderSlabSuperstructureGeom.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	PhysicalModelCPtr bridgeStructPhysModelCPtr = BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	GeometricElement3d::CreateParams createParams(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSuperstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	auto atDE = DistanceExpression();
	auto superstructureElementPtr = GenericSuperstructureElement::Create(createParams, *alignmentPtr, atDE);
	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "Superstructure1");
	status = TestHelper::SetDgnElementCodeAndInsert(superstructureElementPtr, code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	status = TestHelper::CreateAndPositionSlabGirderSuperstructureMembers(superstructureElementPtr, bridgeStructPhysModelCPtr);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelCPtr,
		alignModelPtr);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture,
	CreateAlignmentWithGirderSlabSuperstructureAndMultiColPierSubstructureGeomTest)
	{
	// This is an approximation to how the girder slab superstructure geometry added to the bim DB,
	// Actual implementation of the forms schema definition of Lofted Extrusion is pending 
	DgnDbPtr db = CreateDgnDb(L"AlignmentWithGirderSlabSuperstructureMultiColPierSubstructureGeom.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	PhysicalModelCPtr bridgeStructPhysModelCPtr = BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	GeometricElement3d::CreateParams createParams(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSuperstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	auto atDE = DistanceExpression();
	auto superstructureElementPtr = GenericSuperstructureElement::Create(createParams, *alignmentPtr, atDE);
	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "Superstructure1");
	status = TestHelper::SetDgnElementCodeAndInsert(superstructureElementPtr, code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	status = TestHelper::CreateAndPositionSlabGirderSuperstructureMembers(superstructureElementPtr, bridgeStructPhysModelCPtr);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	// Create Substructure elements
	GeometricElement3d::CreateParams createParams2(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSubstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	atDE = DistanceExpression(85);

	ILinearlyLocatedSingleAt::CreateAtParams atParams(*alignmentPtr, atDE);
	auto substructureElementPtr = TestHelper::CreateAndPositionMultiColumnPier(createParams2, atParams,
		bridgeStructPhysModelCPtr, "Support1", .75);
	ASSERT_TRUE(substructureElementPtr != nullptr);

	atDE.SetDistanceAlongFromStart(30);
	ILinearlyLocatedSingleAt::CreateAtParams atParams2(*alignmentPtr, atDE);
	auto substructureElement2Ptr = TestHelper::CreateAndPositionMultiColumnPier(createParams2, atParams2,
		bridgeStructPhysModelCPtr, "Support2", .35);
	ASSERT_TRUE(substructureElement2Ptr != nullptr);

	atDE.SetDistanceAlongFromStart(alignmentPtr->GetLength()-30);
	ILinearlyLocatedSingleAt::CreateAtParams atParams3(*alignmentPtr, atDE);
	auto substructureElement3Ptr = TestHelper::CreateAndPositionMultiColumnPier(createParams2, atParams3,
		bridgeStructPhysModelCPtr, "Support3", -.35);
	ASSERT_TRUE(substructureElement3Ptr != nullptr);
	
	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelCPtr,
		alignModelPtr);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture,
	CreateAlignmentWithBoxGirderSuperstructureAndSingleColumnSubstructureGeomTest)
	{
	// This is an approximation to how the girder slab superstructure geometry added to the bim DB,
	// Actual implementation of the forms schema definition of Lofted Extrusion is pending 
	DgnDbPtr db = CreateDgnDb(L"AlignmentWithBoxGirderSuperstructureAndSingleColumnSubstructureGeom.bim");
	Dgn::DgnDbStatus status;
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = true;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	PhysicalModelCPtr bridgeStructPhysModelCPtr = BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	GeometricElement3d::CreateParams createParams(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSuperstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	auto atDE = DistanceExpression();
	auto superstructureElementPtr = GenericSuperstructureElement::Create(createParams, *alignmentPtr, atDE);
	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "Superstructure1");
	status = TestHelper::SetDgnElementCodeAndInsert(superstructureElementPtr, code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	status = TestHelper::CreateAndPositionBoxGirderSuperstructureMembers(superstructureElementPtr, bridgeStructPhysModelCPtr);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	// Create Substructure elements
	GeometricElement3d::CreateParams createParams2(*db, bridgeStructPhysModelCPtr->GetModelId(), GenericSubstructureElement::QueryClassId(*db), BridgeCategory::Get(*db));
	atDE = DistanceExpression(85);

	ILinearlyLocatedSingleAt::CreateAtParams atParams(*alignmentPtr, atDE);
	auto substructureElementPtr = TestHelper::CreateAndPositionSingleColumnPierSubstructure(createParams2, atParams,
		bridgeStructPhysModelCPtr, "Support1", .35);
	ASSERT_TRUE(substructureElementPtr != nullptr);

	atDE.SetDistanceAlongFromStart(30);
	ILinearlyLocatedSingleAt::CreateAtParams atParams2(*alignmentPtr, atDE);
	auto substructureElement2Ptr = TestHelper::CreateAndPositionSingleColumnPierSubstructure(createParams2, atParams2,
		bridgeStructPhysModelCPtr, "Support2", -.35);
	ASSERT_TRUE(substructureElement2Ptr != nullptr);


	atDE.SetDistanceAlongFromStart(alignmentPtr->GetLength() - 30);
	ILinearlyLocatedSingleAt::CreateAtParams atParams3(*alignmentPtr, atDE);
	auto substructureElement3Ptr = TestHelper::CreateAndPositionSingleColumnPierSubstructure(createParams2, atParams3,
		bridgeStructPhysModelCPtr, "Support3");
	ASSERT_TRUE(substructureElement3Ptr != nullptr);

	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelCPtr,
		alignModelPtr);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  07/2018
//---------------------------------------------------------------------------------------
TEST_F(BridgeStructuralPhysicalTestFixture, PSolidUtilLoftTest)
	{
	// Visual Experimentation test for Lofted extrusion geometries
	DgnDbPtr db = CreateDgnDb(L"PSolidUtilLoft.bim");
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());

	PhysicalModelCPtr  bridgeStructPhysModelCPtr = BridgeStructuralPhysical::BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "PSolidUtilLoftTest");

	auto wallPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
		STRUCTURAL_PHYSICAL_CLASS_Wall, bridgeStructPhysModelCPtr);
	Dgn::DgnDbStatus status;
	status = wallPtr->SetCode(code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	wallPtr->Insert(&status);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	
	IBRepEntityPtr wallBrepPtr;
	bvector<CurveVectorPtr>profiles;
	bvector<DPoint3d> Points;
	Points.push_back(DPoint3d::From(1, 2, 0.5));
	Points.push_back(DPoint3d::From(3, 2, 0.5));
	Points.push_back(DPoint3d::From(3, 2, -0.5));
	Points.push_back(DPoint3d::From(1, 2, -0.5));
	CurveVectorPtr line = CurveVector::CreateLinear(Points, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
	profiles.push_back(line);
	Points.clear();
	Points.push_back(DPoint3d::From(1, 5, 0.5));
	Points.push_back(DPoint3d::From(3, 5, 0.5));
	Points.push_back(DPoint3d::From(3, 5, -0.5));
	Points.push_back(DPoint3d::From(1, 5, -0.5));
	CurveVectorPtr line2 = CurveVector::CreateLinear(Points, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
	profiles.push_back(line2);
	bvector<CurveVectorPtr>guides;
	ICurvePrimitivePtr arc1 = ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(DPoint3d::From(1, 3.5, 0.5), DVec3d::From(0.0, -1.5, 0.0), DVec3d::From(0.0, 0.0, 1.5), 0.0, Angle::Pi()));
	MSInterpolationCurve fitCurve;
	DPoint3d tangents[2];
	tangents[0] = DPoint3d::From(0, 0, 0);
	tangents[1] = DPoint3d::From(0, 0, 0);
	Points.clear();
	Points.push_back(DPoint3d::From(3, 5, 0.5));
	Points.push_back(DPoint3d::From(3, 3.5, 0.6));
	Points.push_back(DPoint3d::From(3, 2, 0.5));
	fitCurve.InitFromPointsAndEndTangents(Points, false, 0.0, tangents, false,
		false, false, false);
	auto arc2 = ICurvePrimitive::CreateInterpolationCurveSwapFromSource(fitCurve);
	Points.clear();
	Points.push_back(DPoint3d::From(3, 5, -0.5));
	Points.push_back(DPoint3d::From(3, 3.5, -0.4));
	Points.push_back(DPoint3d::From(3, 2, -0.5));
	fitCurve.InitFromPointsAndEndTangents(Points, false, 0.0, tangents, false,
		false, false, false);
	auto arc3 = ICurvePrimitive::CreateInterpolationCurveSwapFromSource(fitCurve);

	guides.push_back(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, arc1));
	guides.push_back(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, arc2));
	guides.push_back(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, arc3));

	BentleyStatus bstatus;
	bstatus = BRepUtil::Create::BodyFromLoft(wallBrepPtr, profiles, &guides, false);
	ASSERT_EQ(SUCCESS, bstatus);
	ASSERT_TRUE(!wallBrepPtr.IsNull());

	Placement3d placement;
	placement.GetOriginR() = DPoint3d::From(0, 0, 0);
	GeometryCreation::BuildGeomForGeomElementWithBRep(wallPtr, wallBrepPtr, placement);
	ASSERT_TRUE(wallPtr->HasGeometry());
	TestHelper::CreateAndVerifyViewDefinitionForBridgeModel(db, bridgeStructPhysModelCPtr);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Nick.Purcell                  06/2018
//---------------------------------------------------------------------------------------
/*
TEST_F(BridgeStructuralPhysicalTestFixture, BasicCurvedProfiledLoftedExtrusionTest)
	{
	DgnDbPtr db = CreateDgnDb(L"BasicCurvedProfiledLoftedExtrusionTest.bim");
	ASSERT_TRUE(db.IsValid());
	ASSERT_TRUE(BentleyStatus::SUCCESS == db->Schemas().CreateClassViewsInDb());
	ASSERT_TRUE(DgnDbStatus::Success == RoadRailAlignmentDomain::SetUpModelHierarchy(*db->Elements().GetRootSubject()));

	PhysicalModelCPtr  bridgeStructPhysModelCPtr = BridgeStructuralPhysical::BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(bridgeStructPhysModelCPtr.IsValid());

	AlignmentModelPtr alignModelPtr = TestHelper::GetDefaultAlignmentModel(db);
	bool loadWithArcs = false;
	AlignmentPtr alignmentPtr = TestHelper::GetDefaultAlignment(db, alignModelPtr, "ALG-1", loadWithArcs);
	alignmentPtr->GenerateAprox3dGeom();
	alignmentPtr->Update();

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *bridgeStructPhysModelCPtr, "BasicLoftedProfiledExtrusionTest");

	auto wallPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
		STRUCTURAL_PHYSICAL_CLASS_Wall, bridgeStructPhysModelCPtr);
	Dgn::DgnDbStatus status;
	status = wallPtr->SetCode(code);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	auto profilesModel = Profiles::ProfilesDomainUtilities::CreateProfilesModel(MODEL_TEST_NAME, *db, nullptr);
	ASSERT_TRUE(profilesModel.IsValid());
	Dgn::DgnCode codeConstProfile= Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, "CurvedProfileExtrusionTests_ConstProfile_1");
	Profiles::ConstantProfilePtr constantProfile = Profiles::ConstantProfile::Create(profilesModel);
	ASSERT_TRUE(constantProfile.IsValid());
	constantProfile->SetCode(codeConstProfile);
	constantProfile->AddCardinalPoint("P1", DPoint2d::From(0, 0));
	constantProfile->AddCardinalPoint("P2", DPoint2d::From(1, 0));
	constantProfile->AddCardinalPoint("P3", DPoint2d::From(1, 1));
	constantProfile->AddCardinalPoint("P4", DPoint2d::From(0, 1));

	Dgn::DgnElementCPtr persistentElement = constantProfile->Insert(&status);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	ASSERT_TRUE(persistentElement.IsValid());

	auto cplePtr = CurvedProfiledLoftedExtrusion::Create();
	auto mainPair = alignmentPtr->QueryMainPair();
	CurveVectorPtr horizCurveVector = const_cast<CurveVectorP>(mainPair->GetHorizontalCurveVector());
	CurveVectorPtr verticalCurveVector = const_cast<CurveVectorP>(mainPair->GetVerticalCurveVector());

	cplePtr->SetHorizontalCurve(IGeometry::Create(horizCurveVector));
	cplePtr->SetVerticalCurve(IGeometry::Create(verticalCurveVector));
	cplePtr->SetStartProfile(*constantProfile);
	cplePtr->SetEndProfile(*constantProfile);
	cplePtr->SetStartDistanceAlong(1);
	cplePtr->SetEndDistanceAlong(alignmentPtr->GetLength()-1);
	

	CurvedProfiledLoftedExtrusion::SetAspect(*wallPtr, *cplePtr);
	wallPtr->Insert(&status);
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
	wallPtr->Update();

	auto fetchedAspectP = CurvedProfiledLoftedExtrusion::GetAspectP(*wallPtr);
	ASSERT_TRUE(fetchedAspectP != nullptr);

	CurveVectorCP horizCurveVector2 = &*fetchedAspectP->GetHorizontalCurve()->GetAsCurveVector();
	CurveVectorCP verticalCurveVector2 = &*fetchedAspectP->GetVerticalCurve()->GetAsCurveVector();

	AlignmentPairCPtr createdPair = AlignmentPair::Create(horizCurveVector2, verticalCurveVector2);
	auto alignmnent2Ptr = Alignment::Create(*alignModelPtr);
	auto createdAlignment = alignmnent2Ptr->InsertWithMainPair(*createdPair);
	ILinearElementCR iLinearElementCR = *createdAlignment;
	auto cardinalPoints = TestHelper::GetProfilesCardinalPoints(constantProfile);
	DPoint3d placementPt;
	auto startDist = fetchedAspectP->GetStartDistanceAlong();
	auto endDist = fetchedAspectP->GetEndDistanceAlong();
	//auto profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(cardinalPoints, &iLinearElementCR, placementPt, fetchedAspectP->GetStartDistanceAlong(), fetchedAspectP->GetEndDistanceAlong());
	auto profiles = GeometryCreation::GenerateStartAndEndProfilesForConstantProfilesAlongSLE(cardinalPoints, &iLinearElementCR, bvector<DPoint2d>(),
		startDist, endDist);
	auto guideLines = GeometryCreation::GenerateGuideCurvesAlongILinearElementForConstantProfile(
		cardinalPoints, &iLinearElementCR, startDist, endDist);
	IBRepEntityPtr wallBrepPtr;
	// TODO: fix issues with brep creation failing due to guide lines
	BRepUtil::Create::BodyFromLoft(wallBrepPtr,profiles, &guideLines, false);
	
	ASSERT_TRUE(wallBrepPtr != nullptr);
	Placement3d placement;
	placement.GetOriginR() = DPoint3d::From(0, 0, 0);
	GeometryCreation::BuildGeomForGeomElementWithBRep(wallPtr, wallBrepPtr, placement);
	TestHelper::CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(db, bridgeStructPhysModelCPtr,
		alignModelPtr);

	}
	*/