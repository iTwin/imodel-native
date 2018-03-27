/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/ClassificationSystemsDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnClientFx/DgnClientApp.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE


//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(ClassificationSystemsDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
ClassificationSystemsDomain::ClassificationSystemsDomain () : Dgn::DgnDomain(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, "ClassificationSystems Domain", 1)
    {
    RegisterHandler (CIBSEClassDefinitionHandler::GetHandler ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
ClassificationSystemsDomain::~ClassificationSystemsDomain ()
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 03/2017
//---------------------------------------------------------------------------------------
void ClassificationSystemsDomain::_OnSchemaImported(Dgn::DgnDbR db) const
    {



    }


END_CLASSIFICATIONSYSTEMS_NAMESPACE