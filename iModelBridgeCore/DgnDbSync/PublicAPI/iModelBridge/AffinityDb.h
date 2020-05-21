/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>
#include <iModelBridge/iModelBridgeFwkTypes.h>

#if defined (__IMODEL_BRIDGE_FWK_BUILD__) || defined(__IMODEL_BRIDGE_BUILD__)
    #define AFFINITY_DB_EXPORT EXPORT_ATTRIBUTE
#else
    #define AFFINITY_DB_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Access to affinity database.
//! A bridge uses this class to record information about *source* files and models in 
//! them and reference relationships between them.
//! The affinity database is not an iModel. The affinity database is used by the iModelBridge
//! framework to determine what files are involved in a synchronization and what bridges should
//! be assigned to process them.
// @bsiclass                                                    Sam.Wilson      03/2020
//=======================================================================================
struct iModelBridgeAffinityDb : RefCountedBase
{
private:
    BeSQLite::Db m_db;

    BeSQLite::DbResult CreateSchema();

public:
    BeSQLite::Db& GetDb() {return m_db;}

    AFFINITY_DB_EXPORT static RefCountedPtr<iModelBridgeAffinityDb> Create(BeFileNameCR dbName);

    AFFINITY_DB_EXPORT static RefCountedPtr<iModelBridgeAffinityDb> Open(BeFileNameCR dbName);

    static RefCountedPtr<iModelBridgeAffinityDb> OpenOrCreate(BeFileNameCR dbName) { return dbName.DoesPathExist()? Open(dbName): Create(dbName); }

    //! Look up a previously recorded model
    //! @param description optional. The friendly, human-readable name or description of the model is returned here.
    //! @param jsonData optional. The JSON-encoded data that annotates the model record, if any, is returned here.
    //! @param sourceFileRowId Identifies the source file that contains the model. (See FindOrInsertFile)
    //! @param sourceIdentifier The unique identifier of the model within the source file. This is normally a GUID or a numerical identifier.
    //! @return 0 if the model is not found in the affinity db; otherwise, the non-zero rowid of the model record in the affinity db.
    AFFINITY_DB_EXPORT int64_t FindModel(Utf8StringP description, Utf8StringP jsonData, int64_t sourceFileRowId, Utf8StringCR sourceIdentifier);
    
    //! Record a model in the affinity db.
    //! @param sourceFileRowId Identifies the source file that contains the model. (See FindOrInsertFile)
    //! @param sourceIdentifier The unique identifier of the model within the source file. This is normally a GUID or a numerical identifier.
    //! @param description The friendly, human-readable name or description of the model.
    //! @param jsonData optional. JSON-encoded data that annotates the model record. For example, this could include the transform that that bridge should 
    //! apply when transforming and aligning data from this model into an iModel.
    //! @return The rowid of the new record in the affinity db, or 0 if there was an error writing to the affinity db.
    AFFINITY_DB_EXPORT int64_t InsertModel(int64_t sourceFileRowId, Utf8StringCR sourceIdentifier, Utf8StringCR description, JsonValueCP jsonData);
    
    //! Look up a previously recorded model; if not found, create a record of it. 
    //! @see FindModel, InsertModel 
    AFFINITY_DB_EXPORT int64_t FindOrInsertModel(int64_t sourceFileRowId, Utf8StringCR sourceIdentifier, Utf8StringCR description, JsonValueCP jsonData) { auto rid = FindModel(nullptr,nullptr, sourceFileRowId, sourceIdentifier); if (0 == rid) rid = InsertModel(sourceFileRowId, sourceIdentifier, description, jsonData); return rid; }

    //! Look up a previously recorded bridge.
    //! @param bridgeRegSubKey The registry subkey that identifies the bridge
    //! @return 0 if the bridge is not found in the affinity db; otherwise, the non-zero rowid of the bridge record in the affinity db.
    AFFINITY_DB_EXPORT int64_t FindBridge(Utf8StringCR bridgeRegSubKey);

    //! Look up the name of a bridge by its ROWID.
    AFFINITY_DB_EXPORT Utf8String GetBridgename(int64_t bridgeRowId);

    //! Record a bridge in the affinity db.
    //! @param bridgeRegSubKey The registry subkey that identifies the bridge
    //! @return The rowid of the new record in the affinity db, or 0 if there was an error writing to the affinity db.
    AFFINITY_DB_EXPORT int64_t InsertBridge(Utf8StringCR bridgeRegSubKey);
    
    //! Look up a previously recorded bridge; if not found, create a record of it. 
    //! @see FindBridge, InsertBridge 
    AFFINITY_DB_EXPORT int64_t FindOrInsertBridge(Utf8StringCR bridgeRegSubKey) { auto rid = FindBridge(bridgeRegSubKey); if (0 == rid) rid = InsertBridge(bridgeRegSubKey); return rid; }
    
    //! Look up a previously recorded source file.
    //! @param sourceFileName The full path to the source file.
    //! @return The rowid of the source file record in the affinity db, or 0 if there was an error writing to the affinity db.
    AFFINITY_DB_EXPORT int64_t FindFile(BeFileNameCR sourceFileName);

    //! Look up the filename for the file with the specified rowid
    AFFINITY_DB_EXPORT BeFileName GetFilename(int64_t fileRowId);

    //! Record a source file in the affnity db.
    //! @param sourceFileName The full path to the file.
    //! @return The rowid of the new record in the affinity db, or 0 if there was an error writing to the affinity db.
    AFFINITY_DB_EXPORT int64_t InsertFile(BeFileNameCR sourceFileName);

    //! Look up a previously recorded source file; if not found, create a record of it. 
    //! @see FindFile, InsertFile 
    AFFINITY_DB_EXPORT int64_t FindOrInsertFile(BeFileNameCR sourceFileName) { auto rid = FindFile(sourceFileName); if (0 == rid) rid = InsertFile(sourceFileName); return rid; }

    //! Look up a previously recorded affinity; if not found, create a record of it.
    //! @param affinity Optional. The affinity level is returned here.
    //! @param jsonData optional. The JSON-encoded data that annotates the affinity, if any, is returned here.
    //! @param sourceFileRowId Identifies the source file.
    //! @param bridgeRowId Identifies the bridge.
    //! @return 0 if the affinity is not found in the affinity db; otherwise, the non-zero rowid of the affinity record in the affinity db.
    AFFINITY_DB_EXPORT BentleyStatus FindAffinity(iModelBridgeAffinityLevel* affinity, Utf8StringP jsonData, int64_t sourceFileRowId, int64_t bridgeRowId);

    //! Record an affinity of a bridge to a source file.
    //! @param affinity The affinity of the bridge for the source file.
    //! @param jsonData optional. The JSON-encoded data that annotates the affinity, if any.
    //! @param sourceFileRowId Identifies the source file.
    //! @param bridgeRowId Identifies the bridge.
    //! @return the rowid of the new record in the affinity db, or 0 if there was an error writing to the affinity db.
    AFFINITY_DB_EXPORT BentleyStatus InsertAffinity(int64_t sourceFileRowId, int64_t bridgeRowId, iModelBridgeAffinityLevel affinity, JsonValueCP jsonData);

    //! Look up a previously recorded affinity; if not found, create a record of it. 
    //! @see FindFile, InsertFile 
    AFFINITY_DB_EXPORT BentleyStatus FindOrInsertAffinity(int64_t sourceFileRowId, int64_t bridgeRowId, iModelBridgeAffinityLevel affinity, JsonValueCP jsonData);

    //! Look up a previously recorded reference attachment.
    //! @param jsonData optional. The JSON-encoded data that annotates the attachment, if any, is returned here.
    //! @param parentModelRowId Identifies the referencing model. 
    //! @param childModelRowId Identifies the referencing model. 
    //! @return 0 if the attachment is not found in the affinity db; otherwise, the non-zero rowid of the attachment record in the affinity db.
    AFFINITY_DB_EXPORT BentleyStatus FindAttachment(Utf8StringP jsonData, int64_t parentModelRowId, int64_t childModelRowId);

    //! Record a reference attachment in the affinity db.
    //! @param jsonData optional. The JSON-encoded data that annotates the attachment, if any.
    //! @param parentModelRowId Identifies the referencing model. 
    //! @param childModelRowId Identifies the referencing model. 
    //! @return 0 if the attachment is not found in the affinity db; otherwise, the non-zero rowid of the attachment record in the affinity db.
    AFFINITY_DB_EXPORT BentleyStatus InsertAttachment(int64_t parentModelRowId, int64_t childModelRowId, JsonValueCP jsonData);

    //! Look up a previously recorded attachment; if not found, create a record of it. 
    //! @see FindAttachment, InsertAttachment 
    AFFINITY_DB_EXPORT BentleyStatus FindOrInsertAttachment(int64_t parentModelRowId, int64_t childModelRowId, JsonValueCP jsonData);

    //! Prefer the specified bridge over any other bridge where they have the same affinity
    AFFINITY_DB_EXPORT void DeleteCompetingAffinities(int64_t preferredBridge);

    //! The signature of a function that ComputeAssignments may call.
    //! @param file The file to be bridged
    //! @param bridge The bridge that should be used process this file.
    //! @param affinity the affinity of this bridge for this file.
    typedef void T_ProcessAssignment(BeFileNameCR file, Utf8StringCR bridge, iModelBridgeAffinityLevel affinity);

    //! Compute bridge that should be assigned to each file. A bridge is assigned to a file if it has the highest affinity to it.
    //! In case of equal affinities, the choice of bridge is arbitrary.
    //! @param proc called on each assignment
    AFFINITY_DB_EXPORT void ComputeAssignments(std::function<T_ProcessAssignment> const& proc);

    typedef void T_ProcessFile(int64_t fileRowId);

    //! Query the files that are immediately attached to the specified parent file. The attached files may be thought of as children of the parent.
    //! Note that only the direct attachments are returned. To detect nested attachments, you must call this function on a given child file.
    //! @see InsertAttachment, FindFile
    //! @param parentFileRowId The parent file
    //! @param proc Called on each file that is attached to the parent file.
    AFFINITY_DB_EXPORT void QueryAttachmentsToFile(int64_t parentFileRowId, std::function<T_ProcessFile> const& proc);

    typedef void T_ProcessDuplicateAffinity(int64_t fileRowId, iModelBridgeAffinityLevel affinity, bvector<int64_t> const& bridges);

    AFFINITY_DB_EXPORT void FindDuplicateAffinities(std::function<T_ProcessDuplicateAffinity> const& proc);

};

END_BENTLEY_DGN_NAMESPACE
