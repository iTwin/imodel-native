/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RasterDb.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

USING_NAMESPACE_BENTLEY_SQLITE


// DM&& Note 
//   Performance
//      Check to use GetCachedStatement
//      Try to read all tile in a transaction
//      Is it a good idea to keep a statement for each data tile (ReadDataPtr)?
//
//

// For now we have only one type of header. If we expand that, all blob specific attributes should be hidden into 
// a header class that could of type DgnDbHeader or whatever. 
enum HeaderBlobType
    {
    HEADERBLOBTYPE_DgnDb = 1
    };

static Utf8CP RASTERHEADERBLOB_Type             = "type";              // the blob type. See HeaderBlobType.
static Utf8CP RASTERHEADERBLOB_BlockWidth       = "blockWidth";        // in pixels (X)
static Utf8CP RASTERHEADERBLOB_BlockHeight      = "blockHeight";       // in pixels (Y)
static Utf8CP RASTERHEADERBLOB_PixelType        = "pixelType";         // one of DbRasterTable::PixelType
static Utf8CP RASTERHEADERBLOB_CompressionType  = "compressionType";   // one of DbRasterTable::CompressionType
static Utf8CP RASTERHEADERBLOB_Resolution       = "resolution";

static Utf8CP RASTERRESOLUTIONBLOB_Width        = "width";        // in pixels (X)
static Utf8CP RASTERRESOLUTIONBLOB_Height       = "height";       // in pixels (Y)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRasterTable::Row DgnRasterTable::QueryRasterById(DgnRasterFileId id)
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "SELECT Name,Header FROM " DGN_TABLE_RasterFile " WHERE RasterId=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return DgnRasterTable::Row();

    Json::Value jsonObj (Json::objectValue);
    if (!Json::Reader::Parse ((Utf8CP)stmt.GetValueBlob(1), jsonObj))
        return DgnRasterTable::Row(); // invalid row

    // For now this is the only thing we support.
    if(jsonObj[RASTERHEADERBLOB_Type].asUInt() != HEADERBLOBTYPE_DgnDb)
        return DgnRasterTable::Row(); // invalid row

    DgnRasterTable::Row entry; 

    entry.m_id = id;
    entry.m_name.assign(stmt.GetValueText(0));
    entry.m_blockWidth    = jsonObj[RASTERHEADERBLOB_BlockWidth].asUInt();
    entry.m_blockHeight   = jsonObj[RASTERHEADERBLOB_BlockHeight].asUInt();
    entry.m_pixelType    = (PixelType)jsonObj[RASTERHEADERBLOB_PixelType].asUInt();
    entry.m_compressType = (CompressionType)jsonObj[RASTERHEADERBLOB_CompressionType].asUInt();

    Json::Value& allResolutions = jsonObj[RASTERHEADERBLOB_Resolution];

    for(Json::ArrayIndex i=0; i < allResolutions.size(); ++i)
        {
        Json::Value& oneRes = allResolutions[i];
        ResolutionDefinition resDef;
        resDef.width = oneRes[RASTERRESOLUTIONBLOB_Width].asUInt();
        resDef.height = oneRes[RASTERRESOLUTIONBLOB_Height].asUInt();
        entry.m_resolutions.push_back(resDef);
        }    

    BeAssert(entry.GetResolutions().size() > 0);

    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnRasterTable::DeleteRaster(DgnRasterFileId id)
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "DELETE FROM " DGN_TABLE_RasterFile " WHERE RasterId=?");
    stmt.BindId(1, id);
    
    return stmt.Step() == BE_SQLITE_DONE ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnRasterTable::InsertRaster (DgnRasterTable::Row& row)
    {
    BeAssert (m_dgndb.IsTransactionActive());

    // Do not accept invalid header
    if(row.GetPixelType() == 0 || row.GetCompressionType() == 0 || row.GetResolutions().empty())
        {
        BeAssert(!"DgnRasterTable: Invalid raster header");
        return BE_SQLITE_ERROR;
        }       

    DbResult status = m_dgndb.GetNextRepositoryBasedId(row.m_id, DGN_TABLE_RasterFile, "RasterId");
    if(BE_SQLITE_OK != status)
        {
        BeAssert (!"DgnRasterTable: failed to GetNextRepositoryBasedId"); 
        return status;
        }

    Json::Value headerBlob;
    headerBlob[RASTERHEADERBLOB_Type]        = (uint32_t)HEADERBLOBTYPE_DgnDb;        // The only one we support for now.
    headerBlob[RASTERHEADERBLOB_BlockWidth]  = row.GetBlockWidth();
    headerBlob[RASTERHEADERBLOB_BlockHeight] = row.GetBlockHeight();
    headerBlob[RASTERHEADERBLOB_PixelType]  = (uint32_t)row.GetPixelType();
    headerBlob[RASTERHEADERBLOB_CompressionType] = (uint32_t)row.GetCompressionType();

    Json::Value allResolutions;//(Json::arrayValue);
    for(uint32_t i=0; i < row.GetResolutions().size(); ++i)
        {
        Json::Value oneRes(Json::objectValue);
        oneRes[RASTERRESOLUTIONBLOB_Width]  = row.GetResolutions()[i].width;
        oneRes[RASTERRESOLUTIONBLOB_Height] = row.GetResolutions()[i].height;
        allResolutions.append(oneRes);
        }

    headerBlob[RASTERHEADERBLOB_Resolution] = allResolutions;
    
    Utf8String  HeaderBlobStr = Json::FastWriter::ToString(headerBlob);

    Statement stmt;
    stmt.Prepare (m_dgndb, "INSERT INTO " DGN_TABLE_RasterFile " (RasterID, Name, Header) VALUES(?,?,?)");
    stmt.BindId (1, row.GetId());
    stmt.BindText (2, row.GetName(), Statement::MakeCopy::No);
    stmt.BindBlob (3, HeaderBlobStr.c_str(), (int)HeaderBlobStr.length(), Statement::MakeCopy::No);
    status = stmt.Step();
    
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnRasterDataTable::ReadData (void* data, uint32_t& outDataSize, uint32_t inDataSize, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId)
    {
    BeAssert (m_dgndb.IsTransactionActive());

    DbResult status;
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != (status = m_dgndb.GetCachedStatement (stmt, "SELECT Data FROM " DGN_TABLE_RasterData " WHERE RasterId=? AND ResolutionLevel=? AND RowBlockId=? AND ColumnBlockId=?")))
        return status;

    stmt->BindId (1, rasterId);
    stmt->BindInt (2, resolutionLevel);
    stmt->BindInt (3, rowBlockId);
    stmt->BindInt (4, columnBlockId);
    status = stmt->Step();
    BeAssert (status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW)
        return status;

    void const* blobdata = stmt->GetValueBlob(0);
    outDataSize = stmt->GetColumnBytes(0);
    if(outDataSize > inDataSize)
        return BE_SQLITE_ERROR;
    
    memcpy(data, blobdata, outDataSize);
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRasterDataTable::RasterBlockDataPtr DgnRasterDataTable::ReadDataPtr (BeSQLite::DbResult& status, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId)
    {
    BeAssert (m_dgndb.IsTransactionActive());

    RasterBlockDataPtr pBlockData = RasterBlockData::Create();
    Statement* pStmt(pBlockData->GetStatement());

    pStmt->Prepare (m_dgndb, "SELECT Data FROM " DGN_TABLE_RasterData " WHERE RasterId=? AND ResolutionLevel=? AND RowBlockId=? AND ColumnBlockId=?");
    pStmt->BindId (1, rasterId);
    pStmt->BindInt (2, resolutionLevel);
    pStmt->BindInt (3, rowBlockId);
    pStmt->BindInt (4, columnBlockId);
    status = pStmt->Step();
    if (status != BE_SQLITE_ROW)
        return NULL;

    return  pBlockData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnRasterDataTable::AddData (void const* data, uint32_t datasize, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId)
    {
    BeAssert (m_dgndb.IsTransactionActive());

    Statement stmt;
    stmt.Prepare (m_dgndb, "INSERT INTO " DGN_TABLE_RasterData " (RasterId, ResolutionLevel, RowBlockId, ColumnBlockId, Data) VALUES(?,?,?,?,?)");
    stmt.BindId   (1, rasterId);
    stmt.BindInt  (2, resolutionLevel);
    stmt.BindInt  (3, rowBlockId);
    stmt.BindInt  (4, columnBlockId);
    stmt.BindBlob (5, data, datasize, Statement::MakeCopy::No);
    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;  
    }
