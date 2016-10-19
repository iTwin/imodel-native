/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/DataCaptureTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if 0   //NOT NOW


#include "PublicAPI/BackDoor/DataCapture/BackDoor.h"

#define TEST_DB_NAME L"DataCapture.idgndb"
#define TEST_MODEL_NAME "SpatialModel"
#define TEST_DATACAPTURE_MODEL_NAME "DataCaptureModel"

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DataCaptureTestFixture::DataCaptureTestFixture()
    {
    DgnDomains::RegisterDomain(DataCaptureTestDomain::GetDomain());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::SetUp()
    {
    m_testDb = m_testHost.CreateProject(TEST_DB_NAME);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not create test project";

    DgnDbStatus dbStatus = DataCaptureTestDomain::ImportSchema(*m_testDb);
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);

    m_testModel = m_testHost.CreateSpatialModel(*m_testDb, TEST_MODEL_NAME);
    ASSERT_TRUE(m_testModel.IsValid());

    m_testCategoryId = m_testHost.CreatePhysicalCategory(*m_testDb, "DataCapturePhysicalElements");
    ASSERT_TRUE(m_testCategoryId.IsValid());

    m_testDataCaptureModel = DataCaptureModel::Create(DataCaptureModel::CreateParams(*m_testDb, DgnModel::CreateModelCode(TEST_DATACAPTURE_MODEL_NAME)));
    DgnDbStatus status = m_testDataCaptureModel->Insert();
    ASSERT_TRUE(DgnDbStatus::Success == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::CloseDb()
    {
    m_testDb->SaveChanges();
    m_testDb->CloseDb();
    m_testDb = nullptr;
    m_testModel = nullptr;
    m_testDataCaptureModel = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::OpenDb()
    {
    m_testDb = m_testHost.OpenProject(TEST_DB_NAME);
    ASSERT_TRUE(m_testDb.IsValid());

    DgnModelId modelId = m_testDb->Models().QueryModelId(DgnModel::CreateModelCode(TEST_MODEL_NAME));
    ASSERT_TRUE(modelId.IsValid());

    m_testModel = m_testDb->Models().GetModel(modelId);
    ASSERT_TRUE(m_testModel.IsValid());

    DgnModelId dataCaptureModelId = m_testDb->Models().QueryModelId(DgnModel::CreateModelCode(TEST_DATACAPTURE_MODEL_NAME));
    ASSERT_TRUE(dataCaptureModelId.IsValid());

    m_testDataCaptureModel = m_testDb->Models().Get<DataCaptureModel>(dataCaptureModelId);
    ASSERT_TRUE(m_testDataCaptureModel.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnCode DataCaptureTestFixture::GetCode(Utf8CP name, int iFloor, int iQuadrant, int iBlock, PlanId planId)
    {
    return DataCaptureDomain::CreateCode(*m_testDb, Utf8PrintfString("%s:Floor %d, Quadrant %d, Block %d, Plan %llu", name, iFloor, iQuadrant, iBlock, planId.GetValueUnchecked()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::CreateSampleBuilding(int nFloors, int nSquareBlocksPerQuadrant)
    {
    /*
    * Create a building with blocks.
    * + A foundation model with a ground and a foundation element. 
    * + A model for every floor, and a category for every quadrant. 
    * + Height = numFloors
    * + Size of each block = (sizeX, sizeY, sizeZ)
    * + Code for elements is set according to the floor and quadrant
    */

    double blockSizeX = 2.0, blockSizeY = 2.0, blockSizeZ = 2.0;

    /*
    * Create a ground and a foundation
    */
    DPoint3d groundRange = DPoint3d::From(blockSizeX * nSquareBlocksPerQuadrant * 10, blockSizeY * nSquareBlocksPerQuadrant * 10, blockSizeZ * 3);
    InsertBlock(DPoint3d::From(0, 0, -blockSizeZ * 2.5), groundRange, GetCode("Ground", -1, -1, -1));

    DPoint3d foundationRange = DPoint3d::From(blockSizeX * nSquareBlocksPerQuadrant * 5, blockSizeY * nSquareBlocksPerQuadrant * 5, blockSizeZ * 2);
    InsertBlock(DPoint3d::From(0, 0, 0.5), foundationRange, GetCode("Foundation", -1, -1, -1));
    
    /*
     * Create PhysicalElement blocks for each floor, and groups for each quadrant
     */   
    DPoint3d blockSizeRange = DPoint3d::From(blockSizeX, blockSizeY, blockSizeZ);
    DPoint3d winchSizeRange = DPoint3d::From(blockSizeX / 4.0, 2.0 * nSquareBlocksPerQuadrant * blockSizeY, blockSizeZ / 4.0);
    for (int iFloor = 1; iFloor <= nFloors; iFloor++)
        {
        DgnElementIdSet quadElements[4];

        double centerZ = iFloor * blockSizeZ - (blockSizeZ / 2.0);

        int iBlock = 1;
        double limX = nSquareBlocksPerQuadrant* blockSizeX - blockSizeX * 0.5;
        double limY = nSquareBlocksPerQuadrant* blockSizeY - blockSizeY * 0.5;
        for (double centerX = -limX; centerX <= limX; centerX += blockSizeX)
            {
            for (double centerY = -limY; centerY <= limY; centerY += blockSizeY)
                {
                int iQuadrant = (centerX > 0) ? ((centerY > 0) ? 1 : 2) : ((centerY > 0) ? 4 : 3);
                
                DgnElementId elementId = InsertBlock(DPoint3d::From(centerX, centerY, centerZ), blockSizeRange, GetCode("Block", iFloor, iQuadrant, iBlock));
                quadElements[iQuadrant-1].insert(elementId);

                iBlock++;
                }
            }

        for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
            InsertGroup(quadElements[iQuadrant - 1], GetCode("Quadrant", iFloor, iQuadrant, -1));

        // Create a winch at each floor (to be shown temporarily)
        InsertBlock(DPoint3d::From(-limX - blockSizeX, 0.0, centerZ), winchSizeRange, GetCode("Winch", iFloor, -1, -1));
        }

    m_testHost.CreateDefaultView(*m_testModel);

    // Reopen just to flush caches for testing purposes
    CloseDb();
    OpenDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
DgnElementId DataCaptureTestFixture::InsertBlock(DPoint3dCR center, DPoint3d sizeRange, DgnCode const& elementCode)
    {
    SpatialModelP spatialModel = (SpatialModelP) m_testModel.get();
    GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(*spatialModel, m_testCategoryId);
    physicalElementPtr->SetCode(elementCode);
    
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), sizeRange, true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*m_testModel, m_testCategoryId, center, YawPitchRollAngles());
    builder->Append(*geomPtr);
    BentleyStatus status = builder->SetGeomStreamAndPlacement(*physicalElementPtr);
    BeAssert(status == SUCCESS);

    DgnElementId elementId = m_testDb->Elements().Insert(*physicalElementPtr)->GetElementId();
    BeAssert(elementId.IsValid());

    return elementId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
DgnElementId DataCaptureTestFixture::InsertGroup(DgnElementIdSet const& elementIdSet, DgnCode const& elementCode)
    {
    TestElementGroupPtr group = TestElementGroup::Create(*m_testDb, m_testModel->GetModelId(), m_testCategoryId, elementCode);
    DgnElementId groupId = group->Insert()->GetElementId();
    BeAssert(groupId.IsValid());

    for (DgnElementId elementId : elementIdSet)
        {
        PhysicalElementCPtr element = m_testDb->Elements().Get<PhysicalElement>(elementId);
        BeAssert(element.IsValid());

        if (DgnDbStatus::Success != group->AddMember(*element))
            {
            BeAssert(false);
            return DgnElementId();
            }
        }

    return groupId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::AddFoundationToPlan(PlanR plan)
    {
    PlanId planId = plan.GetId();

    WorkBreakdownPtr workBreakdown = WorkBreakdown::Create(plan, "WorkBreakdown Foundation");
    workBreakdown->SetCode(GetCode("FoundationWorkBreakdown", -1, -1, -1, planId));
    WorkBreakdownId workBreakdownId = workBreakdown->Insert();
    ASSERT_TRUE(workBreakdownId.IsValid());

    ActivityPtr activity = Activity::Create(*workBreakdown, "Activity Foundation");
    activity->SetCode(GetCode("FoundationActivity", -1, -1, -1, planId));
    ActivityId activityId = activity->Insert();
    ASSERT_TRUE(activityId.IsValid());

    DgnElementId elementId = m_testDb->Elements().QueryElementIdByCode(GetCode("Foundation", -1, -1, -1));
    ASSERT_TRUE(elementId.IsValid());
    Activity::AssignAffectedElements(*m_testDb, activityId, elementId, Activity::ElementAffectType::Destroy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::AddFloorActivitiesToPlan(int nFloors, PlanR plan)
    {
    PlanId planId = plan.GetId();

    ActivityId foundationId(m_testDb->Elements().QueryElementIdByCode(GetCode("FoundationActivity", -1, -1, -1, planId)).GetValue());
    ASSERT_TRUE(foundationId.IsValid());

    ActivityId previousFloorActivityIds[4];
    DgnElementId previousFloorElementGroupIds[4];
    Duration lagTimeBetweenFloors(1.0, Duration::Format::Weeks);
    for (int iFloor = 1; iFloor <= nFloors; iFloor++)
        {
        Utf8PrintfString modelName("Floor %d", iFloor);
        ActivityId thisFloorActivityIds[4];
        DgnElementId thisFloorElementGroupIds[4];

        for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
            {
            Utf8PrintfString categoryName("Quadrant %d", iQuadrant);

            ActivityPtr activityPtr;
            activityPtr = Activity::Create(plan, "Activity");
            activityPtr->SetCode(GetCode("QuadrantActivity", iFloor, iQuadrant, -1, planId));

            ActivityId activityId = activityPtr->Insert();
            thisFloorActivityIds[iQuadrant - 1] = activityId;

            // Setup "create" element associations with elements in the current floor and quadrant
            DgnElementId groupId = m_testDb->Elements().QueryElementIdByCode(GetCode("Quadrant", iFloor, iQuadrant, -1));
            ASSERT_TRUE(groupId.IsValid());
            thisFloorElementGroupIds[iQuadrant - 1] = groupId;

            Activity::AssignAffectedElements(*m_testDb, activityId, groupId, Activity::ElementAffectType::Create);
            
            // Setup "temporary" element associations with winch elements in the current floor 
            DgnElementId winchId = m_testDb->Elements().QueryElementIdByCode(GetCode("Winch", iFloor, -1, -1));
            ASSERT_TRUE(winchId.IsValid());
            
            Activity::AssignAffectedElements(*m_testDb, activityId, winchId, Activity::ElementAffectType::Temporary);

            if (iFloor > 1)
                {
                // Setup constraints with previous floor
                Activity::InsertActivityConstraint(*m_testDb, previousFloorActivityIds[iQuadrant-1], activityId, Activity::ConstraintType::FinishToStart, &lagTimeBetweenFloors);

                // Setup "maintain" associations with previous floor
                Activity::AssignAffectedElements(*m_testDb, activityId, previousFloorElementGroupIds[iQuadrant - 1], Activity::ElementAffectType::Maintain);
                }
            else
                {
                // Setup constraint with foundation
                Activity::InsertActivityConstraint(*m_testDb, foundationId, activityId, Activity::ConstraintType::FinishToStart, &lagTimeBetweenFloors);
                }
            }

        for (int ii = 0; ii < 4; ii++)
            {
            previousFloorActivityIds[ii] = thisFloorActivityIds[ii];
            previousFloorElementGroupIds[ii] = thisFloorElementGroupIds[ii];
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::AddPlanByFloor(int nFloors)
    {
    PlanPtr plan = Plan::Create(*m_testDataCaptureModel, "By Floor");
    PlanId planId = plan->Insert();
    ASSERT_TRUE(planId.IsValid());

    AddFoundationToPlan(*plan);

    AddFloorActivitiesToPlan(nFloors, *plan);

    // Organize activities into WorkBreakdown-s by floor
    for (int iFloor = 1; iFloor <= nFloors; iFloor++)
        {
        WorkBreakdownPtr wb = WorkBreakdown::Create(*plan, "WorkBreakdown");
        wb->SetCode(GetCode("FloorWorkBreakdown", iFloor, -1, -1, planId));
        WorkBreakdownId wbId = wb->Insert();

        for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
            {
            DgnElementId elementId = m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, iQuadrant, -1, planId));
            ASSERT_TRUE(elementId.IsValid());

            ActivityPtr activity = Activity::GetForEdit(*m_testDb, ActivityId(elementId.GetValue()));
            activity->SetParentId(wbId);
            activity->Update();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::AddPlanByQuadrant(int nFloors)
    {
    PlanPtr plan = Plan::Create(*m_testDataCaptureModel, "By Quadrant");
    PlanId planId = plan->Insert();
    ASSERT_TRUE(planId.IsValid());

    AddFoundationToPlan(*plan);

    AddFloorActivitiesToPlan(nFloors, *plan);

    // Add additional constraints between activities in each quadrant
    Duration lagTimeBetweenQuadrants(0.5, Duration::Format::Weeks);
    for (int iFloor = 1; iFloor <= nFloors; iFloor++)
        {
        ActivityId quad1(m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, 1, -1, planId)).GetValue());
        ASSERT_TRUE(quad1.IsValid());
        ActivityId quad2(m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, 2, -1, planId)).GetValue());
        ASSERT_TRUE(quad2.IsValid());
        ActivityId quad3(m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, 3, -1, planId)).GetValue());
        ASSERT_TRUE(quad3.IsValid());
        ActivityId quad4(m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, 4, -1, planId)).GetValue());
        ASSERT_TRUE(quad4.IsValid());

        Activity::InsertActivityConstraint(*m_testDb, quad1, quad2, Activity::ConstraintType::FinishToStart, &lagTimeBetweenQuadrants);
        Activity::InsertActivityConstraint(*m_testDb, quad2, quad3, Activity::ConstraintType::FinishToStart, &lagTimeBetweenQuadrants);
        Activity::InsertActivityConstraint(*m_testDb, quad3, quad4, Activity::ConstraintType::FinishToStart, &lagTimeBetweenQuadrants);
        }

    // Organize activities into WorkBreakdown-s by quadrant
    for (int iQuadrant = 1; iQuadrant <= 4; iQuadrant++)
        {
        DgnCode wbCode = DataCaptureDomain::CreateCode(*m_testDb, Utf8PrintfString("Quadrant %d", iQuadrant));
        
        WorkBreakdownPtr wb = WorkBreakdown::Create(*plan, "WorkBreakdown");
        wb->SetCode(GetCode("QuadrantWorkBreakdown", -1, iQuadrant, -1, planId));
        WorkBreakdownId wbId = wb->Insert();

        for (int iFloor = 1; iFloor <= nFloors; iFloor++)
            {
            DgnElementId elementId = m_testDb->Elements().QueryElementIdByCode(GetCode("QuadrantActivity", iFloor, iQuadrant, -1, planId));
            ASSERT_TRUE(elementId.IsValid());

            ActivityPtr activity = Activity::GetForEdit(*m_testDb, ActivityId(elementId.GetValue()));
            activity->SetParentId(wbId);
            activity->Update();
            }
        }    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::CreateSampleBuildingPlans(int nFloors)
    {
    AddPlanByFloor(nFloors);
    AddPlanByQuadrant(nFloors);

    // Reopen just to flush caches for testing purposes
    CloseDb();
    OpenDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void DataCaptureTestFixture::CreateSampleBuildingSchedules()
    {
    DateTime planStart = DateTime::GetCurrentTimeUtc();

    for (Plan::Entry const& planEntry : Plan::MakePlanIterator(*m_testDb))
        {
        PlanId planId = planEntry.GetId();
        TimelinePtr timeline = Timeline::Create(*m_testDb, planId, "Primary Timeline");
        TimelineId timelineId = timeline->Insert();

        // Setup durations for all the activities
        for (WorkBreakdown::Entry const& wbEntry : WorkBreakdown::MakeWorkBreakdownIterator(*m_testDb, planId))
            {
            for (Activity::Entry const& activityEntry : Activity::MakeActivityIterator(*m_testDb, wbEntry.GetId()))
                {
                ActivityPtr activity = Activity::GetForEdit(*m_testDb, activityEntry.GetId());

                TimeSpanPtr timeSpan = TimeSpan::Create(*m_testDb, activity->GetId(), timelineId);
                timeSpan->SetPlannedDuration(Duration(0.5, Duration::Format::Weeks));
                TimeSpanId timeSpanId = timeSpan->Insert();

                activity->Update();
                }
            }

        // Setup a start for the plan
        TimeSpanPtr timeSpan = TimeSpan::Create(*m_testDb, planId, timelineId);
        timeSpan->SetPlannedStart(planStart);
        timeSpan->Insert();

        // Schedule all the activities
        Scheduler::Options options;
        Scheduler scheduler(*m_testDb, planId, timelineId, options);
        Scheduler::Status status = scheduler.CreateSchedule();
        ASSERT_TRUE(status == Scheduler::Status::Success);
        }

    // Reopen just to flush caches for testing purposes
    CloseDb();
    OpenDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2015
//---------------------------------------------------------------------------------------
int DataCaptureTestFixture::GetChangeSummaryInstanceCount(ChangeSummaryCR changeSummary, Utf8CP qualifiedClassName) const
    {
    Utf8PrintfString ecSql("SELECT COUNT(*) FROM %s WHERE IsChangedInstance(?, GetECClassId(), ECInstanceId)", qualifiedClassName);

    ECSqlStatement stmt;
    ECSqlStatus ecSqlStatus = stmt.Prepare(*m_testDb, ecSql.c_str());
    BeAssert(ecSqlStatus.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    DbResult ecSqlStepStatus = stmt.Step();
    BeAssert(ecSqlStepStatus == BE_SQLITE_ROW);

    return stmt.GetValueInt(0);
    }

    #endif