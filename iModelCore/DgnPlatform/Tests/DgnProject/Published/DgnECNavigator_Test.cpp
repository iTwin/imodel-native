/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnECNavigator_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/DgnHandlers/DgnHandlersAPI.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))
#ifndef POSTCONDITION
#define POSTCONDITION(cond, error) if (!(cond)) return (error);
#endif

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern ECInstanceKey CreateAndImportInstance (ECDbR db, ECClassCR ecClass);
extern void DebugDumpJson (const Json::Value& jsonValue);
extern int CountRows (ECSqlStatement& statement);
extern bool WriteJsonToFile (WCharCP path, const Json::Value& jsonValue);
extern bool ReadJsonFromFile (Json::Value& jsonValue, WCharCP path);

    
BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================    
//! @bsiclass                                                Ramanujam.Raman      02/2014
//=======================================================================================   
struct DgnECNavigatorTest : public testing::Test
{
private:
    Dgn::ScopedDgnHost m_host;
    
protected:
    DgnECNavigatorTest() {};
    virtual ~DgnECNavigatorTest () {};
    
    // Utilities
#if defined (NEEDS_WORK_DGNITEM)
    static Json::Value* FindPrimaryInstance (Utf8StringR primaryRelationshipPath, Utf8StringR primaryClassKey, JsonValueCR elementInfo);
#endif
    static StatusInt CreateFindSimilarQuery 
        (
        Utf8StringR ecSql, 
        JsonValueCR elementInfo, 
        Utf8CP selectedClassKey, 
        Utf8CP selectedProperty, 
        Utf8CP selectedValue,
        ECDbR ecDb
        );
    static StatusInt FindInstancesIncludingChildren (ECInstanceKeyMultiMap& instanceKeyMap, ECSqlStatement& statement, ECDbR ecDb);
    static StatusInt GetElementInfo (Json::Value& elementInfo, const DgnElementId& selectedElementId, DgnDbR project);
    
    static void ValidateElementInfo (JsonValueR actualElementInfo, WStringCR expectedFileName);

    /* 
     * Tests
     */
    static void PlantElementInfo_InternalTest (DgnDbTestDgnManager& tdm);
    static void PlantFindSimilar_InternalTest (DgnDbTestDgnManager& tdm);
    static void PlantElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName);
    static void PlantFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm);
    static void PlantFindSimilarOnElementWithNoPrimaryInstance_ExternalTest (DgnDbTestDgnManager& tdm);

    static void DgnLinksElementInfo_InternalTest (DgnDbTestDgnManager& tdm);
    static void DgnLinksFindSimilar_InternalTest (DgnDbTestDgnManager& tdm);
    static void DgnLinksElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName);
    static void DgnLinksFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm);

    static void IfcElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName);
    static void IfcFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm);
};

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, Plant_WithLatestDgnDb)
    {
    DgnDbTestDgnManager tdm (L"79_Main.i.idgndb", __FILE__, OPENMODE_READONLY);

    PlantElementInfo_InternalTest (tdm);
    PlantFindSimilar_InternalTest (tdm);

    PlantElementInfo_ExternalTest (tdm, L"ElementInfo_79_Main.json");
    PlantFindSimilar_ExternalTest (tdm);
    PlantFindSimilarOnElementWithNoPrimaryInstance_ExternalTest (tdm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, Plant_WithOldDgnDb)
    {
    DgnDbTestDgnManager tdm (L"79_Main_Graphite04.i.idgndb", __FILE__, OPENMODE_READONLY, TestDgnManager::DGNINITIALIZEMODE_None, true);

    PlantElementInfo_ExternalTest (tdm, L"ElementInfo_79_Main_Graphite04.json");
    PlantFindSimilar_ExternalTest (tdm);
    PlantFindSimilarOnElementWithNoPrimaryInstance_ExternalTest (tdm);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, DgnLinks_WithLatestDgnDb)
    {
    DgnDbTestDgnManager tdm (L"DgnLinksSample.idgndb", __FILE__, OPENMODE_READONLY, TestDgnManager::DGNINITIALIZEMODE_None, true);

    DgnLinksElementInfo_InternalTest (tdm);
    DgnLinksFindSimilar_InternalTest (tdm);

    DgnLinksElementInfo_ExternalTest (tdm, L"DgnLinksSample.json");
    DgnLinksFindSimilar_ExternalTest (tdm);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, Ifc_WithLatestDgnDb)
    {
    DgnDbTestDgnManager tdm (L"ifc.i.idgndb", __FILE__, OPENMODE_READONLY);

    IfcElementInfo_ExternalTest (tdm, L"ElementInfo_ifc.json");
    IfcFindSimilar_ExternalTest (tdm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::PlantElementInfo_InternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Identify the new element id corresponding with V8 ElementId 1779 (Cell PSEQP).
    // Note: Only the V8 id is stable from the perspective of our test here - the .i.dgn
    // is checked in as part of source, but the .idgndb/.imodel is created as part of the 
    // build.
    int64_t selectedElementId = GetV9ElementId ("EQPM01.dgn.i.dgn", 1779, project); // Returns 747 (or some child of that assembly) in Graphite04, but that's just for reference
    ASSERT_TRUE (selectedElementId > 0);
    DgnElementId shownElementId;
    if (!DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance (shownElementId, DgnElementId (selectedElementId), project))
        shownElementId = DgnElementId (selectedElementId);
    
    // Find level for equipment
    Utf8PrintfString levelFromElement ("SELECT Level.Name FROM ONLY dgn.Level JOIN dgn.Element USING dgn.ElementIsOnLevel WHERE Element.ECInstanceId = %lld", shownElementId.GetValue());
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, levelFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    ECSqlStepStatus stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
    Utf8String levelName = statement.GetValueText(0);
    ASSERT_TRUE (levelName == "Equipment");

    // Find model for equipment
    Utf8PrintfString modelFromElement ("SELECT Model.Name FROM ONLY dgn.Model JOIN dgn.Element USING dgn.ModelContainsElements WHERE Element.ECInstanceId = %lld", shownElementId.GetValue());
    statement.Finalize();
    prepareStatus = statement.Prepare (project, modelFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
    Utf8String modelName = statement.GetValueText(0);
    ASSERT_TRUE (modelName == "Default [EQPM01]");

    // Find primary instance for equipment - method 1 of 3
    Utf8PrintfString instanceFromElement(
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
         JOIN dgn.Element El USING dgn.ElementHasPrimaryInstance \
         WHERE El.ECInstanceId = %lld", shownElementId.GetValue());
    statement.Finalize();
    prepareStatus = statement.Prepare (project, instanceFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);

    // Find primary instance for equipment - method 2 of 3
    instanceFromElement.Sprintf (
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
         JOIN dgn.ElementHasPrimaryInstance Rel ON Inst.ECInstanceId = Rel.TargetECInstanceId \
         JOIN dgn.Element El ON El.ECInstanceId = Rel.SourceECInstanceId \
         WHERE El.ECInstanceId = %lld", shownElementId.GetValue());
    statement.Finalize();
    prepareStatus = statement.Prepare (project, instanceFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);

    // Find primary instance for equipment - method 3 of 3
    instanceFromElement.Sprintf (
        "SELECT ElementHasPrimaryInstance.TargetECClassId, ElementHasPrimaryInstance.TargetECInstanceId \
         FROM dgn.ElementHasPrimaryInstance \
         JOIN dgn.Element ON Element.ECInstanceId = ElementHasPrimaryInstance.SourceECInstanceId \
         WHERE Element.ECInstanceId = %lld", shownElementId.GetValue());
    statement.Finalize();
    prepareStatus = statement.Prepare (project, instanceFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::DgnLinksElementInfo_InternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Identify the new element id corresponding with V8 ElementId 227
    // Note: Only the V8 id is stable from the perspective of our test here - the .dgn
    // is checked in as part of source, but the .idgndb/.imodel is created as part of the 
    // build.
    int64_t v9ElementId = GetV9ElementId ("DgnLinksSample.dgn", 227, project); // Returns 6 in Graphite04, but that's just for reference
    ASSERT_TRUE (v9ElementId > 0);

    Utf8PrintfString instanceFromElement(
        "SELECT Inst.ECInstanceId FROM dgn.DgnLink Inst \
         JOIN dgn.ElementHasSecondaryInstances Rel ON Inst.ECInstanceId = Rel.TargetECInstanceId \
         JOIN dgn.Element El ON El.ECInstanceId = Rel.SourceECInstanceId \
         WHERE El.ECInstanceId = %lld", v9ElementId);

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, instanceFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    int numDgnLinks = CountRows (statement);
    ASSERT_EQ (3, numDgnLinks);

    instanceFromElement.Sprintf (
        "SELECT ElementHasSecondaryInstances.TargetECClassId, ElementHasSecondaryInstances.TargetECInstanceId \
        FROM dgn.ElementHasSecondaryInstances \
        JOIN dgn.Element ON Element.ECInstanceId = ElementHasSecondaryInstances.SourceECInstanceId \
        WHERE Element.ECInstanceId = %lld", v9ElementId);

    statement.Finalize();
    prepareStatus = statement.Prepare (project, instanceFromElement.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    numDgnLinks = CountRows (statement);
    ASSERT_EQ (3, numDgnLinks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::PlantFindSimilar_InternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Find all equipment on some level
    Utf8CP equipmentFromLevel = 
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
        JOIN dgn.Element El USING dgn.ElementHasPrimaryInstance \
        JOIN dgn.Level USING dgn.ElementIsOnLevel \
        WHERE Level.Name = 'Equipment'";

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, equipmentFromLevel);
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    int numEquipment = CountRows (statement);
    ASSERT_EQ (8, numEquipment);

    equipmentFromLevel = 
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
        JOIN dgn.ElementHasPrimaryInstance Rel ON Inst.ECInstanceId = Rel.TargetECInstanceId \
        JOIN dgn.Element El ON El.ECInstanceId = Rel.SourceECInstanceId \
        JOIN dgn.Level USING dgn.ElementIsOnLevel \
        WHERE Level.Name = 'Equipment'";
    statement.Finalize();
    prepareStatus = statement.Prepare (project, equipmentFromLevel);
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    numEquipment = CountRows (statement);
    ASSERT_EQ (8, numEquipment);

    // Find all equipment on some model
    Utf8CP equipmentFromModel =
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
        JOIN dgn.Element El USING dgn.ElementHasPrimaryInstance \
        JOIN dgn.Model USING dgn.ModelContainsElements \
        WHERE Model.Name = 'Default [EQPM01]'";
    statement.Finalize();
    prepareStatus = statement.Prepare (project, equipmentFromModel);
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    numEquipment = CountRows (statement);
    ASSERT_EQ (16, numEquipment);

    equipmentFromModel =
        "SELECT Inst.ECInstanceId FROM ams.EQUIP_MEQP Inst \
        JOIN dgn.ElementHasPrimaryInstance Rel ON Inst.ECInstanceId = Rel.TargetECInstanceId \
        JOIN dgn.Element El ON El.ECInstanceId = Rel.SourceECInstanceId \
        JOIN dgn.Model USING dgn.ModelContainsElements \
        WHERE Model.Name = 'Default [EQPM01]'";
    statement.Finalize();
    prepareStatus = statement.Prepare (project, equipmentFromModel);
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    numEquipment = CountRows (statement);
    ASSERT_EQ (16, numEquipment);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Json::Value* DgnECNavigatorTest::FindPrimaryInstance (Utf8StringR primaryRelationshipPath, Utf8StringR primaryClassKey, JsonValueCR elementInfo)
    {
    primaryRelationshipPath = "";
    primaryClassKey = "";

    Utf8CP primaryRelationshipName = "ElementHasPrimaryInstance";
    JsonValueCR classes = elementInfo["ecDisplayInfo"]["Classes"];
    JsonValueCR instances = elementInfo["ecInstances"];

    // Iterate over all the instances
    for (Json::Value::iterator iter = instances.begin(); iter != instances.end(); iter++)
        {
        Json::Value& instance = *iter;
        Utf8String ecClassKey = instance["$ECClassKey"].asString();

        // Determine if the relationship path holds the "Primary" relationship
        JsonValueCR classInfo = classes[ecClassKey.c_str()];
        Utf8String relationshipPathStr = classInfo["RelationshipPath"].asString();
        if (relationshipPathStr.find (primaryRelationshipName) != std::string::npos)
            {
            primaryRelationshipPath = relationshipPathStr;
            primaryClassKey = ecClassKey;
            return &instance;
            }
        }

    return nullptr;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
// static 
StatusInt DgnECNavigatorTest::GetElementInfo (Json::Value& elementInfo, const DgnElementId& selectedElementId, DgnDbR project)
    {
    /*
     * Determine the element to show ElementInfo on. 
     * Note: We try to identify an element that's associated with a primary instance by walking
     * up the assembly hierarchy starting with the selected element. This is really the information 
     * we think that the user wants in 80% of the cases. 
     * An application should customize this behaviour as it sees appropriate for it's users. 
     */
    DgnElementId shownElementId;
    if (!DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance (shownElementId, selectedElementId, project))
        shownElementId = selectedElementId;
    
    Json::Value jsonInstances, jsonDisplayInfo; // Note: Cannot just pass actualElemtnInfo["ecInstances"], actualElementInfo["ecDisplayInfo"] here. 
    StatusInt status = DgnECPersistence::GetElementInfo (jsonInstances, jsonDisplayInfo, shownElementId, project);
    POSTCONDITION (status == SUCCESS, status);

    elementInfo["ecInstances"] = jsonInstances;
    elementInfo["ecDisplayInfo"] = jsonDisplayInfo;
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::ValidateElementInfo (JsonValueR actualElementInfo, WStringCR expectedFileName)
    {
    BeFileName expectedFile;
    BeTest::GetHost().GetDocumentsRoot (expectedFile);
    expectedFile.AppendToPath (L"DgnDb");
    expectedFile.AppendToPath (expectedFileName.c_str());

    Json::Value expectedElementInfo;
    bool readFileStatus = ReadJsonFromFile (expectedElementInfo, expectedFile.GetName());
    ASSERT_TRUE (readFileStatus);
    
    // Ignore "$ECInstanceId" in comparision - it's too volatile. 
    ASSERT_TRUE (actualElementInfo["ecInstances"].size() == expectedElementInfo["ecInstances"].size());
    for (int ii=0; ii < (int) actualElementInfo["ecInstances"].size(); ii++)
        actualElementInfo["ecInstances"][ii]["$ECInstanceId"] = "*";

    int compare = expectedElementInfo.compare (actualElementInfo);
    if (0 != compare)
        {
        // For convenient android debugging
        //LOG.debugv ("Expected ElementInfo:");
        //DebugDumpJson (expectedElementInfo);
        //LOG.debugv ("Actual ElementInfo:");
        //DebugDumpJson (actualElementInfo);

        BeFileName actualFile;
        BeTest::GetHost().GetOutputRoot (actualFile);

        WString tmpName = expectedFile.GetFileNameWithoutExtension() + L"_Actual.json";
        actualFile.AppendToPath (tmpName.c_str());
        WriteJsonToFile (actualFile.GetName(), actualElementInfo);

        FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::PlantElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    int64_t v9ElementId = GetV9ElementId ("EQPM01.dgn.i.dgn", 1779, project); // Returns 747 (or some child of that assembly) in Graphite04, but that's just for reference
    ASSERT_TRUE (v9ElementId > 0);
 
    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo (actualElementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    // Validate primary instance label and class label
    Utf8String primaryRelationshipPath, primaryClassKey;
    Json::Value* jsonPrimaryInstance = FindPrimaryInstance (primaryRelationshipPath, primaryClassKey, actualElementInfo);
    ASSERT_TRUE (jsonPrimaryInstance != nullptr);
    ASSERT_TRUE ((*jsonPrimaryInstance)["$ECClassKey"].asString() == "ams.EQUIP_MEQP");
    ASSERT_TRUE ((*jsonPrimaryInstance)["$ECClassLabel"].asString() == "EQUIP_MEQP");
    ASSERT_TRUE ((*jsonPrimaryInstance)["$ECInstanceLabel"].asString() == "PSEQP");

    ValidateElementInfo (actualElementInfo, expectedFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::DgnLinksElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Identify the new element id corresponding with V8 ElementId 227
    // Note: Only the V8 id is stable from the perspective of our test here - the .dgn
    // is checked in as part of source, but the .idgndb/.imodel is created as part of the 
    // build.
    int64_t v9ElementId = GetV9ElementId ("DgnLinksSample.dgn", 227, project); // Returns 6 in Graphite04, but that's just for reference
    ASSERT_TRUE (v9ElementId > 0);

    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo (actualElementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    ValidateElementInfo (actualElementInfo, expectedFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/   
// static
StatusInt DgnECNavigatorTest::CreateFindSimilarQuery 
(
Utf8StringR ecSql, 
JsonValueCR elementInfo, 
Utf8CP selectedClassKey, 
Utf8CP selectedProperty, 
Utf8CP selectedValue,
ECDbR ecDb
)
    {
    ecSql = "";

    /*
     * Note: This routine constructs a ECSQL query to find similar instances(keys) of the *primary* class. 
     * e.g., Find Pumps with the same diameter. 
     * 
     * If a primary instance/class is not found, it constructs a query to find similar instances of 
     * the dgn.Element class. e.g., Find Elements on the same level. 
     *
     * "Similar" instances are identified here as Primary/Element instances that are (or are related to) 
     * instances with the selected property value. 
     *
     * An application can choose to provide more custom behavior - perhaps providing an option to the 
     * user to FindSimilar PrimaryClass-es/SecondaryClass-es/Elements. This test simply illustrates 
     * the use of the underlying API. 
     * 
     */

    // Determine the class to find and it's relationship path to dgn.Element
    Utf8String pathFromFindToElementStr, findClassKey;
    Json::Value* jsonPrimaryInstance = FindPrimaryInstance (pathFromFindToElementStr, findClassKey, elementInfo);
    if (jsonPrimaryInstance == nullptr)
        {
        findClassKey = "dgn.Element";
        pathFromFindToElementStr = "dgn:Element"; // Trivial path to itself, for convenient generation of ECSQL
        }

    JsonValueCR selectedClassJson = elementInfo["ecDisplayInfo"]["Classes"][selectedClassKey];
    Utf8String selectedClassName = selectedClassJson["Name"].asString();
    Utf8String selectedSchemaName = selectedClassJson["SchemaName"].asString();

    // Construct relationship path from the class to find to the selected class
    ECRelationshipPath pathFromFindToSelected;
    StatusInt status = SUCCESS;
    if (findClassKey == selectedClassKey)
        {
        // Construct trivial path to itself for convenient generation of ECSQL
        Utf8String pathFromFindToSelectedStr = selectedSchemaName + ":" + selectedClassName;
        pathFromFindToSelected.InitFromString (pathFromFindToSelectedStr.c_str(), ecDb, nullptr);
        }
    else
        {
        // Construct path by combining paths
        // pathFromFindToSelected = pathFromFindToElement + pathFromElementToSelected

        ECRelationshipPath pathFromSelectedToElement;
        status = pathFromSelectedToElement.InitFromString (selectedClassJson["RelationshipPath"].asString(), ecDb, nullptr);
        POSTCONDITION (status == SUCCESS && !pathFromSelectedToElement.IsEmpty(), ERROR);

        ECRelationshipPath pathFromElementToSelected;
        pathFromSelectedToElement.Reverse (pathFromElementToSelected);

        StatusInt status = pathFromFindToSelected.InitFromString (pathFromFindToElementStr.c_str(), ecDb, nullptr);
        POSTCONDITION (status == SUCCESS && !pathFromFindToSelected.IsEmpty(), ERROR);
        status = pathFromFindToSelected.Combine (pathFromElementToSelected);
        POSTCONDITION (status == SUCCESS, ERROR);
        }

    // Generate query
    Utf8String fromClause, joinClause;
    ECRelationshipPath::GeneratedEndInfo rootInfo, leafInfo;
    status = pathFromFindToSelected.GenerateECSql (fromClause, joinClause, rootInfo, leafInfo, false/*isPolymorphic*/);
    if (status != SUCCESS)
        return status;
    ecSql.Sprintf ("SELECT %s, %s %s %s WHERE %s.%s = '%s'", 
        rootInfo.GetClassIdExpression().c_str(), rootInfo.GetInstanceIdExpression().c_str(),
        fromClause.c_str(), joinClause.c_str(), 
        leafInfo.GetAlias().c_str(), selectedProperty, selectedValue);
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
// static
StatusInt DgnECNavigatorTest::FindInstancesIncludingChildren (ECInstanceKeyMultiMap& instanceKeyMap, ECSqlStatement& statement, ECDbR ecDb)
    {
     ECInstanceFinder instanceFinder (ecDb); // Note: Cache the finder for performance if necessary

     // Seed the instance finder with instances from the statement
     ECInstanceKeyMultiMap seedInstanceKeyMap;
     ECSqlStepStatus stepStatus;
     while ((stepStatus = statement.Step()) == ECSqlStepStatus::HasRow)
         {
         ECClassId ecClassId = statement.GetValueInt64 (0);
         ECInstanceId ecInstanceId = statement.GetValueId<ECInstanceId> (1);
         ECInstanceKeyMultiMapPair instanceEntry (ecClassId, ecInstanceId);
         seedInstanceKeyMap.insert (instanceEntry);
         }

    BentleyStatus status = instanceFinder.FindInstances (instanceKeyMap, seedInstanceKeyMap, 
        ECInstanceFinder::FindOptions (ECInstanceFinder::RelatedDirection_EmbeddedChildren | ECInstanceFinder::RelatedDirection_HeldChildren));
    POSTCONDITION (status == SUCCESS, status);

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::PlantFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    int64_t v9ElementId = GetV9ElementId ("EQPM01.dgn.i.dgn", 1779, project); // Returns 747 (or some child of that assembly) in Graphite04, but that's just for reference
    ASSERT_TRUE (v9ElementId > 0);

    Json::Value elementInfo;
    StatusInt elementInfoStatus = GetElementInfo (elementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    Utf8String ecSql;
    ECSqlStatement statement;
    StatusInt status;
    ECSqlStatus prepareStatus;
    ECInstanceKeyMultiMap instanceKeyMap;

    // Test 1: User has selected BUD_TYPE property in the UI (with value = "EQUIP_MEQP")
    status = CreateFindSimilarQuery (ecSql, elementInfo, "ams.EQUIP_MEQP", "BUD_TYPE", "EQUIP_MEQP", project);
    ASSERT_TRUE (status == SUCCESS);
    prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    status = FindInstancesIncludingChildren (instanceKeyMap, statement, project); // Note: This is only used for illustration - doesn't really gather new instances in this example.
    ASSERT_TRUE (status == SUCCESS);
    ASSERT_EQ (16, (int) instanceKeyMap.size());

    statement.Finalize();
    instanceKeyMap.clear();

    // Test 2: User has selected level property in the UI (with value = Equipment)
    Utf8String selectedLevelName = elementInfo["ecInstances"][3]["Name"].asString();
    status = CreateFindSimilarQuery (ecSql, elementInfo, "dgn.Level", "Name", selectedLevelName.c_str(), project);
    ASSERT_TRUE (status == SUCCESS);
    prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    status = FindInstancesIncludingChildren (instanceKeyMap, statement, project); // Note: This is only used for illustration - doesn't really gather new instances in this example.
    ASSERT_TRUE (status == SUCCESS);
    ASSERT_EQ (8, (int) instanceKeyMap.size());

    statement.Finalize();
    instanceKeyMap.clear();

    // Test 3: User has selected model property in the UI (with value = Equipment)
    Utf8String selectedModelName = elementInfo["ecInstances"][2]["Name"].asString();
    status = CreateFindSimilarQuery (ecSql, elementInfo, "dgn.Model", "Name", selectedModelName.c_str(), project);
    ASSERT_TRUE (status == SUCCESS);
    prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    status = FindInstancesIncludingChildren (instanceKeyMap, statement, project); // Note: This is only used for illustration - doesn't really gather new instances in this example.
    ASSERT_TRUE (status == SUCCESS);
    ASSERT_EQ (16, (int) instanceKeyMap.size());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::PlantFindSimilarOnElementWithNoPrimaryInstance_ExternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    int64_t v9ElementId = GetV9ElementId ("STRM01.dgn.i.dgn", 1578, project); // Structural member with no primary instance
    ASSERT_TRUE (v9ElementId > 0);

    Json::Value elementInfo;
    StatusInt elementInfoStatus = GetElementInfo (elementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    // Test 1: User has selected level property in the UI (with value = "Level 1")
    Utf8String ecSql;
    StatusInt status = CreateFindSimilarQuery (ecSql, elementInfo, "dgn.Level", "Name", "Level 1", project);
    ASSERT_TRUE (status == SUCCESS);

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    int numOfElementsOnLevel1 = CountRows (statement);
    ASSERT_EQ (91, numOfElementsOnLevel1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::DgnLinksFindSimilar_InternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Find all elements with same link to 'ReadMe.txt'
    Utf8CP elementFromLink = 
        "SELECT Element.*  \
        FROM dgn.Element  \
        JOIN dgn.ExternalFileDgnLink USING dgn.ElementHasSecondaryInstances  \
        WHERE ExternalFileDgnLink.DisplayLabel = 'ReadMe.txt'";

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, elementFromLink);
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    int numElements = CountRows (statement);
    ASSERT_EQ (3, numElements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::DgnLinksFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();
    DgnModelP model = tdm.GetDgnModelP();
    ASSERT_TRUE (model != NULL);

    // Identify the new element id corresponding with V8 ElementId 227
    // Note: Only the V8 id is stable from the perspective of our test here - the .dgn
    // is checked in as part of source, but the .idgndb/.imodel is created as part of the 
    // build.
    int64_t v9ElementId = GetV9ElementId ("DgnLinksSample.dgn", 227, project); // Returns 6 in Graphite04, but that's just for reference
    ASSERT_TRUE (v9ElementId > 0);

    Json::Value elementInfo;
    StatusInt elementInfoStatus = GetElementInfo (elementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    // Test 1: User has selected level property in the UI (with value = Equipment)
    Utf8String ecSql;
    StatusInt status = CreateFindSimilarQuery (ecSql, elementInfo, "dgn.ExternalFileDgnLink", "DisplayLabel", "ReadMe.txt", project);
    ASSERT_TRUE (status == SUCCESS);

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    int numLinks = CountRows (statement);
    ASSERT_EQ (3, numLinks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::IfcElementInfo_ExternalTest (DgnDbTestDgnManager& tdm, WStringCR expectedFileName)
    {
    DgnDbR project = *tdm.GetDgnProjectP();

    int64_t v9ElementId = GetV9ElementId ("ifc.i.dgn", 1152921505705818095, project); // 10612 in Graphite0501
    ASSERT_TRUE (v9ElementId > 0);

    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo (actualElementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);

    // Ignore "$ECInstanceId" in comparision - it's too volatile. 
    for (int ii=0; ii < (int) actualElementInfo["ecInstances"].size(); ii++)
        {
        // TODO: For some reason the "ThermalTransmittance" property doesn't seem to compare well, even 
        // if the values are exactly same. 
        JsonValueR actualInstance = actualElementInfo["ecInstances"][ii];
        if (actualInstance.isMember ("ThermalTransmittance"))
            actualInstance["ThermalTransmittance"] = "<Modified for comparision>";
        }

    ValidateElementInfo (actualElementInfo, expectedFileName);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECNavigatorTest::IfcFindSimilar_ExternalTest (DgnDbTestDgnManager& tdm)
    {
    DgnDbR project = *tdm.GetDgnProjectP();

    int64_t v9ElementId = GetV9ElementId ("ifc.i.dgn", 1152921505705818095, project); // 10612 in Graphite0501
    ASSERT_TRUE (v9ElementId > 0);
 
    Json::Value elementInfo;
    BeTest::SetFailOnAssert (false);
    StatusInt elementInfoStatus = GetElementInfo (elementInfo, DgnElementId (v9ElementId), project);
    ASSERT_TRUE (elementInfoStatus == SUCCESS);
    BeTest::SetFailOnAssert (true);

    Utf8String ecSql;
    ECSqlStatement statement;
    StatusInt status;
    ECSqlStatus prepareStatus;
    ECInstanceKeyMultiMap instanceKeyMap;

    // Test 1: User has selected BUD_TYPE property in the UI (with value = "EQUIP_MEQP")
    status = CreateFindSimilarQuery (ecSql, elementInfo, "IFC2x3.IfcWindow", "Tag", "427486", project);
    ASSERT_TRUE (status == SUCCESS);
    prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    status = FindInstancesIncludingChildren (instanceKeyMap, statement, project); // Note: This is only used for illustration - doesn't really gather new instances in this example.
    ASSERT_TRUE (status == SUCCESS);
    ASSERT_EQ (1, (int) instanceKeyMap.size());

    statement.Finalize();
    instanceKeyMap.clear();

    // Test 2: User has selected level property in the UI (with value = Equipment)
    status = CreateFindSimilarQuery (ecSql, elementInfo, "IfcSharedFacilitiesElements2x3.Pset_ManufacturerTypeInformation", 
        "Manufacturer", "Revit", project);
    ASSERT_TRUE (status == SUCCESS);
    prepareStatus = statement.Prepare (project, ecSql.c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    status = FindInstancesIncludingChildren (instanceKeyMap, statement, project); // Note: This is only used for illustration - doesn't really gather new instances in this example.
    ASSERT_TRUE (status == SUCCESS);
    ASSERT_EQ (20, (int) instanceKeyMap.size());

    statement.Finalize();
    instanceKeyMap.clear();
    }
#endif

END_DGNDB_UNIT_TESTS_NAMESPACE



