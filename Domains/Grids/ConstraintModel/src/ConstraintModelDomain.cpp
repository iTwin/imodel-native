/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/src/ConstraintModelDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../PublicApi/ConstraintModelDomain.h"
#include "../PublicApi/ElementConstrainsElementHandler.h"
#include "../PublicApi/DimensionHandler.h"
#include "../PublicApi/CoincidenceHandler.h"
#include <DgnClientFx/DgnClientApp.h>

USING_NAMESPACE_BENTLEY_DGN

USING_NAMESPACE_CONSTRAINTMODEL


//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(ConstraintModelDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 10/2016
//---------------------------------------------------------------------------------------
ConstraintModelDomain::ConstraintModelDomain () : DgnDomain(CONSTRAINTMODEL_SCHEMA_NAME, "ConstraintModel Domain", 1)
    {
    RegisterHandler (ElementConstrainsElementHandler::GetHandler ());
    RegisterHandler (DimensionHandler::GetHandler ());
    RegisterHandler (CoincidenceHandler::GetHandler ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 10/2016
//---------------------------------------------------------------------------------------
ConstraintModelDomain::~ConstraintModelDomain ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 10/2016
//---------------------------------------------------------------------------------------
void ConstraintModelDomain::_OnSchemaImported(DgnDbR db) const
    {
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2016
//---------------------------------------------------------------------------------------
bool ConstraintModelDomain::EnsureConstraintModelECSchemaIsLoaded(Dgn::DgnDbR db)
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                 08/2016
//---------------------------------------------------------------------------------------
void ConstraintModelDomain::_OnDgnDbOpened(DgnDbR db) const
    {

    }