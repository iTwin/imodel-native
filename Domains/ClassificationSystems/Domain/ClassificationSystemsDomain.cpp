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
    RegisterHandler(ClassificationSystemClassDefinitionGroupHandler::GetHandler());
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
void ClassificationSystemsDomain::_OnSchemaImported(Dgn::DgnDbR db) const
    {
    ClassificationSystemsDomain::InsertDefinitionSystems(db);
        //TODO call my Generated Method
        //TODO all methods
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemClassDefinitionGroupPtr ClassificationSystemsDomain::InsertGroup
(
    Dgn::DgnDbR db,
    Utf8CP name
) const
    {

    //TODO
    ClassificationSystemClassDefinitionGroupPtr classDefinition = CIBSEClassDefinition::Create(db, name, group);
    if (Dgn::RepositoryStatus::Success == BS::BuildingLocks_LockElementForOperation(*classDefinition.get(), BeSQLite::DbOpcode::Insert, "CIBSEClassDefinition : Insertion"))
        {
        classDefinition->Insert();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertCIBSE 
(
    Dgn::DgnDbR db,
    ClassificationSystemClassDefinitionGroupPtr group,
    Utf8CP name
) const
    {
    CIBSEClassDefinitionPtr classDefinition = CIBSEClassDefinition::Create(db, name, group);
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
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP Category
) const
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertASHRAE2007 
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP Category
) const
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              03/2018
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::InsertASHRAE2010 
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP Category
) const
    {
    }

#include "GeneratedClassificationInserts.cpp"

END_CLASSIFICATIONSYSTEMS_NAMESPACE