/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/VersionCompareChangeSummary.h>
#include <ECDb/ECDb.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjects.h>

#define ECSCHEMA_ChangedElements "ChangedElements"
#define ECSCHEMA_ALIAS_ChangedElements "chems"

#define FILEEXT_ChangeCache L".chems"

BEGIN_BENTLEY_DGN_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC

#define OPC_INSERT 1
#define OPC_UPDATE 2
#define OPC_DELETE 4

//=======================================================================================
//! Class used to generate the changed elements ECDb file
//! Used to get changed elements from the ECDb file and accumulate change
//=======================================================================================
struct ChangedElementsManager final {
    private:
        //! Briefcase
        DgnDbPtr    m_db;
        bool        m_filterSpatial;

        DbResult AddMetadataToChangeCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const;
        DbResult InsertEntries(ECDbR cacheDb, DgnRevisionPtr revision, bvector<DgnElementId> const& elementIds, bvector<ECClassId> const& classIds, bvector<BeSQLite::DbOpcode> const& opcodes, bvector<DgnModelId> const& modelIds, bvector<AxisAlignedBox3d> const& bboxes);
        bool HasChangeset(ECDbR cacheDb, DgnRevisionPtr revision);
        DgnDbPtr CloneDb(DgnDbR db);

        bmap<DgnModelId, AxisAlignedBox3d> static ComputeChangedModels(ChangedElementsMap const& changedElements);
        bmap<DgnModelId, AxisAlignedBox3d> static ComputeChangedModels(bvector<DgnModelId> const& modelIds, bvector<AxisAlignedBox3d> const& bboxes);
    public:
        BE_JSON_NAME(elements);
        BE_JSON_NAME(classIds);
        BE_JSON_NAME(opcodes);
        BE_JSON_NAME(modelIds);
        BE_JSON_NAME(bboxes);
        BE_JSON_NAME(changedModels);
        BE_JSON_NAME(changedElements);

        DGNPLATFORM_EXPORT ChangedElementsManager(DgnDbPtr db) : m_db(db), m_filterSpatial(false) {}
        DGNPLATFORM_EXPORT ~ChangedElementsManager();

        //! Only store changes to spatial elements
        void SetFilterSpatial(bool value) { m_filterSpatial = value; }

        //! Creates the changed elements cache
        DGNPLATFORM_EXPORT DbResult CreateChangedElementsCache(ECDbR cacheDb, BeFileNameCR cacheFilePath);
        //! Returns true if the changeset is already in the cache
        DGNPLATFORM_EXPORT bool IsProcessed(ECDbR cacheDb, Utf8String changesetId);
        //! Process changesets and add them to the cache if they don't exist
        DGNPLATFORM_EXPORT DbResult ProcessChangesets(ECDbR cacheDb, Utf8String rulesetId, bvector<DgnRevisionPtr> const& revisions);
        //! Gets the changed elements map based on a range of changesets
        DGNPLATFORM_EXPORT DbResult GetChangedElements(ECDbR cacheDb, ChangedElementsMap& changedElements, Utf8String startChangesetId, Utf8String endChangesetId);
        //! Gets the changed models based on a range of changesets
        DGNPLATFORM_EXPORT DbResult GetChangedModels(ECDbR cacheDb, bmap<DgnModelId, AxisAlignedBox3d>& changedModels, Utf8String startChangesetId, Utf8String endChangesetId);
        //! Parses the map into a JSON object to pass back to the addon
        DGNPLATFORM_EXPORT static void ChangedElementsToJSON(JsonValueR val, ChangedElementsMap const& changedElements);
}; // ChangedElementsManager

END_BENTLEY_DGN_NAMESPACE