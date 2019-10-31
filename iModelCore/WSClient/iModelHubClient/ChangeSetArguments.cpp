/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ChangeSetArguments.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
StatusResult PushChangeSetArguments::Validate(Dgn::DgnRevisionPtr changeSet, Dgn::DgnDbR dgndb)
    {
    if (m_containingChanges == ChangeSetInfo::ContainingChanges::NotSpecified)
        {
        m_containingChanges = changeSet->ContainsSchemaChanges(dgndb) ? ChangeSetInfo::ContainingChanges::Schema : ChangeSetInfo::ContainingChanges::Regular;
        }
    else
        {
        if (changeSet->ContainsSchemaChanges(dgndb) && m_containingChanges != ChangeSetInfo::ContainingChanges::Schema)
            return StatusResult::Error(Error::Id::InvalidContainingChangesValue);

        if (m_containingChanges != ChangeSetInfo::ContainingChanges::Schema && 
            ((m_containingChanges & ChangeSetInfo::ContainingChanges::Schema) == ChangeSetInfo::ContainingChanges::Schema))
            return StatusResult::Error(Error::Id::InvalidContainingChangesValue);
        }

    if (m_bridgeProperties.IsValid())
        {
        StatusResult bridgePropertiesValidationResult = m_bridgeProperties->Validate();
        if (!bridgePropertiesValidationResult.IsSuccess())
            return bridgePropertiesValidationResult;
        }

    return StatusResult::Success();
    }