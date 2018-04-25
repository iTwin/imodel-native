/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/ClassificationSystemsDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnClientFx/DgnClientApp.h>
#include <BuildingShared\BuildingSharedApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE


//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(ClassificationSystemsDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2018
//---------------------------------------------------------------------------------------
ClassificationSystemsDomain::ClassificationSystemsDomain () : Dgn::DgnDomain(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, "ClassificationSystems Domain", 1)
    {
    RegisterHandler(ClassificationSystemHandler::GetHandler());
    RegisterHandler(ClassificationGroupHandler::GetHandler());
    RegisterHandler(CIBSEClassDefinitionHandler::GetHandler());
    RegisterHandler(OmniClassClassDefinitionHandler::GetHandler());
    RegisterHandler(ASHRAE2004ClassDefinitionHandler::GetHandler());
    RegisterHandler(ASHRAE2010ClassDefinitionHandler::GetHandler());
    RegisterHandler(MasterFormatClassDefinitionHandler::GetHandler());
    RegisterHandler(UniFormatClassDefinitionHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2018
//---------------------------------------------------------------------------------------
ClassificationSystemsDomain::~ClassificationSystemsDomain ()
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::_OnSchemaImported
(
Dgn::DgnDbR db
) const
    {
    ClassificationSystemsDomain::InsertStandardDefinitionSystems(db);
    //ClassificationSystemsDomain::InsertMasterFormatDefinition(db);
    ClassificationSystemsDomain::InsertUniFormatDefinitions(db);
    /*ClassificationSystemsDomain::InsertOmniClass11Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass12Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass13_2006Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass13_2010Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass14Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass21Definitions(db);
    ClassificationSystemsDomain::InsertOmniClass22_2006Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass22_2010Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass23Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass32Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass33Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass34Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass36_04Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass36_24Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass41Definitions(Dgn::DgnDbR db);
    ClassificationSystemsDomain::InsertOmniClass49Definitions(Dgn::DgnDbR db);*/
        //TODO call my Generated Method
        //TODO all methods
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemPtr ClassificationSystemsDomain::InsertSystem
(
Dgn::DgnDbR db,
Utf8CP name
) const
    {
    Dgn::DgnCode code = Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), Dgn::DgnElementId(), name);
    ClassificationSystemPtr classSystem = ClassificationSystem::Create(db, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classSystem.get(), BeSQLite::DbOpcode::Insert, "ClassificationSystem : Insertion"))
        {
        classSystem->SetCode(code);
        classSystem->Insert();
        return classSystem;
        }
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr ClassificationSystemsDomain::TryAndGetSystem
(
Dgn::DgnDbR db,
Utf8CP name
) const
    {
    Dgn::DgnCode code = Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), Dgn::DgnElementId() , name);
    Dgn::DgnElementId id = db.Elements().QueryElementIdByCode(code);
    if (id.IsValid()) 
        {
        ClassificationSystemCPtr system = db.Elements().Get<ClassificationSystem>(id);
        return system;
        }
    else {
        return InsertSystem(db, name);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationGroupPtr ClassificationSystemsDomain::InsertGroup
(
ClassificationSystemCR system,
Utf8CP name
) const
    {
    ClassificationGroupPtr classDefinitionGroup = ClassificationGroup::Create(system, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classDefinitionGroup.get(), BeSQLite::DbOpcode::Insert, "ClassificationGroup : Insertion"))
        {
        classDefinitionGroup->Insert();
        return classDefinitionGroup;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertCIBSE 
(
ClassificationSystemCR system,
ClassificationGroupCR group,
Utf8CP name
) const
    {
    CIBSEClassDefinitionPtr classDefinition = CIBSEClassDefinition::Create(system, group, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classDefinition.get(), BeSQLite::DbOpcode::Insert, "CIBSEClassDefinition : Insertion"))
        {
        classDefinition->Insert();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertASHRAE2004 
(
ClassificationSystemCR system,
ClassificationGroupCR group,
Utf8CP name
) const
    {
    ASHRAE2004ClassDefinitionPtr classDefinition = ASHRAE2004ClassDefinition::Create(system, group, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classDefinition.get(), BeSQLite::DbOpcode::Insert, "ASHRAE2004ClassDefinition : Insertion"))
        {
        classDefinition->Insert();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertASHRAE2007 
(
ClassificationSystemCR system,
ClassificationGroupCR group,
Utf8CP name
) const
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertASHRAE2010 
(
ClassificationSystemCR system,
ClassificationGroupCR group,
Utf8CP name
) const
    {
    ASHRAE2010ClassDefinitionPtr classDefinition = ASHRAE2010ClassDefinition::Create(system, group, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classDefinition.get(), BeSQLite::DbOpcode::Insert, "ASHRAE2010ClassDefinition : Insertion"))
        {
        classDefinition->Insert();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
UniFormatClassDefinitionPtr ClassificationSystemsDomain::InsertUniFormat
(
ClassificationSystemCR system, 
Utf8CP code, 
Utf8CP name, 
UniFormatClassDefinitionCP specializes
) const 
    {
        return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
MasterFormatClassDefinitionPtr ClassificationSystemsDomain::InsertMasterFormat
(
    ClassificationSystemCR system,
    Utf8CP code,
    Utf8CP name,
    MasterFormatClassDefinitionCP specializes
) const
    {
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
OmniClassClassDefinitionPtr ClassificationSystemsDomain::InsertOmniClass
(
    ClassificationSystemCR system,
    Utf8CP code,
    Utf8CP name,
    OmniClassClassDefinitionCP specializes
) const
    {
    return nullptr;
    }


END_CLASSIFICATIONSYSTEMS_NAMESPACE