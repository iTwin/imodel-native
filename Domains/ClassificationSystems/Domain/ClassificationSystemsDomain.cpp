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
    RegisterHandler(ClassificationHandler::GetHandler());
    RegisterHandler(ClassificationGroupHandler::GetHandler());
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
    ClassificationSystemsDomain::InsertDomainAuthorities(db);
    ClassificationSystemsDomain::InsertStandardDefinitionSystems(db);
    ClassificationSystemsDomain::InsertMasterFormatDefinitions(db);
    //ClassificationSystemsDomain::InsertUniFormatDefinitions(db);
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
void ClassificationSystemsDomain::InsertDomainAuthorities(Dgn::DgnDbR db) const
    {
    InsertCodeSpec(db, CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2017
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertCodeSpec(Dgn::DgnDbR db, Utf8CP name) const
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, name);
    BeAssert(codeSpec.IsValid());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        BeAssert(codeSpec->GetCodeSpecId().IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode ClassificationSystemsDomain::GetSystemCode
(
    Dgn::DgnDbR db,
    Utf8CP name
) const
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), db.Elements().GetRootSubjectId(), name);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              05/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr ClassificationSystemsDomain::TryAndGetSystem
(
    Dgn::DgnDbR db,
    Utf8CP name
) const
    {
    //TODO Make this static in elements
    ClassificationSystemCPtr system = ClassificationSystem::TryGet(db,name);
    if (system.IsNull())
        {
        return InsertSystem(db, name);
        }
    else {
        return system;
    }
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
    ClassificationSystemPtr classSystem = ClassificationSystem::Create(db, name);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classSystem.get(), BeSQLite::DbOpcode::Insert, "ClassificationSystem : Insertion"))
        {
        classSystem->Insert();
        return classSystem;
        }
    return nullptr;
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
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationPtr ClassificationSystemsDomain::InsertClassification
(
ClassificationSystemCR system, 
Utf8CP name, 
Utf8CP id, 
Utf8CP description, 
ClassificationGroupCP group, 
ClassificationCP specializes
) const
    {
    ClassificationPtr classification = Classification::Create(system, name, id, description, group, specializes);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classification.get(), BeSQLite::DbOpcode::Insert, "Classification : Insertion"))
        {
        classification->Insert();
        return classification;
        }
    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Aurimas.Laureckis              06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystemsDomain::MakeClassificationIterator(Dgn::DgnDbR dgnDbR)
    {
    return dgnDbR.Elements().MakeIterator(CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem));
    }






END_CLASSIFICATIONSYSTEMS_NAMESPACE