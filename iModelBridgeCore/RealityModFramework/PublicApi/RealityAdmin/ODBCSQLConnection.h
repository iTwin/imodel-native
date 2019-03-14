/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/ODBCSQLConnection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntity.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsienum                                   Alain.Robert              11/2019
//! Status codes for ODBCConnection
//=====================================================================================
enum class ODBCConnectionStatus
    {
    Success = SUCCESS,         // The operation was successful.
    RecordDoesNotExistError,   // Record was expected but did not exist
    RecordAlreadyExistsError,  // Record exists but it should not
    // *** Add new here.
    UnknownError = ERROR,   // The operation failed with an unspecified error.
    };


//=======================================================================================
// @bsiclass
// Connection to the SQL Server, handles all transactions
//=======================================================================================
struct ServerConnection
{
private:
    // ODBC Control structures and handles
    SQLHENV     hEnv;
    SQLHDBC     hDbc;
    SQLHSTMT    hStmt;

    // Name of the database
    Utf8String m_dbName;

    static ServerConnection* s_instance;

    void OpenConnection();
    void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

    void TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x);
    RETCODE ExecuteSQL(CHAR* query);
    RETCODE ExecuteSQL(SQLHSTMT stmt);
    RETCODE ExecutePrepared();
    SQLRETURN FetchIdentityFromStringVal(SQLINTEGER &id, const char* tableName, const char* columnName, const char* requiredValue, SQLLEN &len);
    SQLRETURN FetchIdentityFromDualStringVal(SQLINTEGER &id, const char* tableName1, const char* columnName1, const char* requiredValue1, const char* columnName2, const char* requiredValue2, SQLLEN &len);
    SQLRETURN FetchTableIdentity(SQLINTEGER &id, const char* tableName, SQLLEN &len);
    void ReleaseStmt();
    bool IsMirror(Utf8CP file);
    bool HasEntries(CHAR* input);

public:
    REALITYDATAPLATFORM_EXPORT static ServerConnection& GetInstance();
    ServerConnection();
    REALITYDATAPLATFORM_EXPORT void SetStrings(const char* dbName, const char* pwszConnStr);
    REALITYDATAPLATFORM_EXPORT bool IsDuplicate(Utf8CP file);

    SQL_TIMESTAMP_STRUCT PackageDateTime(DateTimeCR date);
    REALITYDATAPLATFORM_EXPORT SQL_DATE_STRUCT PackageDate(DateTimeCR dateTime);
    void Exit();

    //!
    //! This method inserts the 'Spatial Entity' portion in the database.
    //! It does not inject sub-components such as thumbnail, metadata or data sources.
    //! The Id of the new record is returned.
    //! Note that spatial entities using the pair Dataset/Name must be unique and can be use as (dual)key
    //! If the entry already exists then an error is returned; the UpdateBaseSpatialEntity should be used instead.
    ODBCConnectionStatus InsertBareSpatialEntity(SpatialEntityCR data, SQLINTEGER metadataId, SQLINTEGER thumbnailId, SQLINTEGER& entityId);

    ODBCConnectionStatus UpdateBareSpatialEntity(SpatialEntityCR data, SQLINTEGER spatialEntityId);
    ODBCConnectionStatus UpdateDataSource(SpatialEntityDataSourceCR dataSource, SQLINTEGER dataSourceId);
    ODBCConnectionStatus UpdateMetadata(SpatialEntityMetadataCR metadata, SQLINTEGER metadataId);
//    ODBCConnectionStatus UpdateThumbnail(SpatialEntityThumbnailCR dataSource, SQLINTEGER thumbnailId);

    ODBCConnectionStatus GetMetadataIdFromSpatialEntity(SQLINTEGER entityId, SQLINTEGER& metadataId, bool& uniqueConsumer);
//    ODBCConnectionStatus GetThumbnailIdFromSpatialEntity(SQLINTEGER entityId, SQLINTEGER& thumbnailId, bool& uniqueConsumer);



    ODBCConnectionStatus SaveMetadata(SpatialEntityMetadataCR metadata, SQLINTEGER& metadataId);
//    ODBCConnectionStatus SaveThumbnail(SpatialEntityThumbnailCR thumbnail, SQLINTEGER& thumbnailId);
    ODBCConnectionStatus SaveDataSource(SpatialEntityDataSourceCR source, SQLINTEGER dataId, SQLINTEGER serverId, SQLINTEGER& dataSourceId);


    REALITYDATAPLATFORM_EXPORT ODBCConnectionStatus SaveSpatialEntity(SpatialEntityCR data, bool dualMode);
    REALITYDATAPLATFORM_EXPORT ODBCConnectionStatus Update(SpatialEntityCR data);
    REALITYDATAPLATFORM_EXPORT bool CheckExists(Utf8String id);
    REALITYDATAPLATFORM_EXPORT SQLINTEGER SaveServer(SpatialEntityServerCR server);
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE