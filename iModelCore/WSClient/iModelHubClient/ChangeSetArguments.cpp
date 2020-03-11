/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ChangeSetArguments.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
StatusResult PushChangeSetArguments::Validate(Dgn::DgnRevisionPtr changeSet, Dgn::DgnDbR dgndb)
    {
    if (m_containingChanges == ChangeSetKind::NotSpecified)
        {
        m_containingChanges = changeSet->ContainsSchemaChanges(dgndb) ? ChangeSetKind::Schema : ChangeSetKind::Regular;
        }
    else
        {
        if (changeSet->ContainsSchemaChanges(dgndb) && m_containingChanges != ChangeSetKind::Schema)
            return StatusResult::Error(Error::Id::InvalidContainingChangesValue);

        if (m_containingChanges != ChangeSetKind::Schema &&
            ((m_containingChanges & ChangeSetKind::Schema) == ChangeSetKind::Schema))
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