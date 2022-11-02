/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDb.h"
#include <Bentley/btree/btree_map.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Utility to iterate over the change state of entities by id in multiple changesets
//! Effectively does the same thing as a @ref ChangeGroup, but ignores non-id changes
//! Built on top of @ref ChangedIdsIterator
//! @ingroup ECDbGroup
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EntityIdsChangeGroup
    {
    bmap<BeInt64Id, BeSQLite::DbOpcode> elementOps;
    bmap<BeInt64Id, BeSQLite::DbOpcode> aspectOps;
    bmap<BeInt64Id, BeSQLite::DbOpcode> modelOps;
    bmap<BeInt64Id, BeSQLite::DbOpcode> relationshipOps;
    bmap<BeInt64Id, BeSQLite::DbOpcode> codeSpecOps;
    bmap<BeInt64Id, BeSQLite::DbOpcode> fontOps;

    DGNPLATFORM_EXPORT DgnDbStatus ExtractChangedInstanceIdsFromChangeSets(Dgn::DgnDbR db, const bvector<BeFileName>& changeSetFiles);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE