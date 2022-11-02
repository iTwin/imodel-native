/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/VersionCompareChangeSummary.h>
#include <ECDb/ECDb.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjects.h>
#include <ECPresentation/ECPresentationManager.h>

#define ECSCHEMA_ChangedElements "ChangedElements"
#define ECSCHEMA_ALIAS_ChangedElements "chems"

#define FILEEXT_ChangeCache L".chems"

BEGIN_BENTLEY_DGN_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

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
        BeFileName  m_dbFilename;
        bool        m_filterSpatial;
        bool        m_wantPropertyChecksums;
        bool        m_wantParents;
        bool        m_wantBriefcaseRoll;
        bool        m_wantRelationshipCaching;
        bool        m_wantChunkTraversal;
        int         m_relationshipCacheSize;
        BeFileName  m_tempLocation;
        Utf8String  m_rulesetDirectory;
        ECPresentationManager* m_presentationManager;

        DbResult AddMetadataToChangeCacheFile(ECDb& cacheFile) const;
        DbResult InsertEntries(ECDbR cacheDb, DgnRevisionPtr revision, bvector<ChangedElement> const& elements);
        bool HasChangeset(ECDbR cacheDb, DgnRevisionPtr revision);
        BeFileName CloneDb(BeFileNameCR dbFilename);

        bmap<DgnModelId, AxisAlignedBox3d> static ComputeChangedModels(ChangedElementsMap const& changedElements);
        bmap<DgnModelId, AxisAlignedBox3d> static ComputeChangedModels(bvector<ChangedElement> const& elements);
        DGNPLATFORM_EXPORT static ECPresentationManager* CreatePresentationManager();

    public:
        BE_JSON_NAME(elements);
        BE_JSON_NAME(classIds);
        BE_JSON_NAME(opcodes);
        BE_JSON_NAME(modelIds);
        BE_JSON_NAME(bboxes);
        BE_JSON_NAME(type);
        BE_JSON_NAME(changedModels);
        BE_JSON_NAME(changedElements);
        BE_JSON_NAME(properties);
        BE_JSON_NAME(parentIds);
        BE_JSON_NAME(parentClassIds);
        BE_JSON_NAME(oldChecksums);
        BE_JSON_NAME(newChecksums);

        // Maintain a passed db for older function calls
        ChangedElementsManager(DgnDbPtr db) : m_dbFilename(db->GetFileName()), m_filterSpatial(false), m_wantParents(false), m_wantBriefcaseRoll(false), m_wantPropertyChecksums(true), m_wantRelationshipCaching(true), m_wantChunkTraversal(false), m_presentationManager(CreatePresentationManager()) {}

        ChangedElementsManager(BeFileNameCR dbFilename) : m_dbFilename(dbFilename), m_filterSpatial(false), m_wantParents(false), m_wantBriefcaseRoll(false), m_wantPropertyChecksums(true), m_wantRelationshipCaching(true), m_wantChunkTraversal(false), m_presentationManager(CreatePresentationManager()) {}

        DGNPLATFORM_EXPORT ~ChangedElementsManager();

        //! Only store changes to spatial elements
        void SetFilterSpatial(bool value) { m_filterSpatial = value; }
        //! Find top parents when processing
        void SetWantParents(bool value) { m_wantParents = value; }
        //! Whether to use direct queries or presentation rules
        void SetWantChunkTraversal(bool value) { m_wantChunkTraversal = value; }
        //! Compute checksums for old vs new values
        void SetWantPropertyChecksums(bool value) { m_wantPropertyChecksums = value; }
        //! Whether to roll the briefcase as part of the processing. If not and needed, a clone of the db will be used to roll
        void SetWantBriefcaseRoll(bool value) { m_wantBriefcaseRoll = value; }
        //! Whether to use caching for current relationships for faster performance or do direct queries per property edge
        void SetWantRelationshipCaching(bool value) { m_wantRelationshipCaching = value; }
        //! Number of relationship entries allowed in a map per property edge
        void SetRelationshipCacheSize(int size) { m_relationshipCacheSize = size; }
        //! Set presentation manager to use in processing
        DGNPLATFORM_EXPORT void SetPresentationRulesetDirectory(Utf8String rulesetDir);
        //! Set the temp location where the cloned Dbs are stored and cached for processing
        void SetTempLocation(Utf8String path) { m_tempLocation = BeFileName(path); }

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
        DGNPLATFORM_EXPORT static void ChangedElementsToJSON(BeJsValue val, ChangedElementsMap const& changedElements);
}; // ChangedElementsManager

END_BENTLEY_DGN_NAMESPACE
