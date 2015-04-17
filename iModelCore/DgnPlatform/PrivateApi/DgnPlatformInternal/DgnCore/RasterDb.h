/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/RasterDb.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/DgnDb.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// !Each embedded raster as an entry in the raster table.
// @bsiclass                                                    Donald.Morissette   10/12
//=======================================================================================
struct DgnRasterTable
{
private:
    DgnDbR m_dgndb;

public:
    enum CompressionType
        {
        // *** Persisted in file. Do not change values. ***
        //COMPRESS_Unknown  = 0,
        COMPRESS_None       = 1,
        COMPRESS_RLE        = 2,
        COMPRESS_Deflate    = 3,
        COMPRESS_Jpeg       = 4
        };

    enum PixelType
        {
        // *** Persisted in file. Do not change values. *** 
        //PIXELTYPE_Unknown   =0,
        PIXELTYPE_V24Rgb    = 1,
        PIXELTYPE_V32Rgba   = 2,    
        PIXELTYPE_V1Gray    = 3      // 1-bit.
        };

    struct ResolutionDefinition
        {
        uint32_t width;               
        uint32_t height;
        };

    struct Row
        {
        private:
            friend struct DgnRasterTable;
            friend struct DgnDbRasterSourceQuery;
        
            DgnRasterFileId     m_id;
            Utf8String          m_name;
            uint32_t            m_blockWidth;      // in pixels. 
            uint32_t            m_blockHeight;     // in pixels. 
            CompressionType     m_compressType;
            PixelType           m_pixelType;
            bvector<ResolutionDefinition> m_resolutions;
            
            Row(): m_blockWidth(0), m_blockHeight(0), m_compressType((CompressionType)0), m_pixelType((PixelType)0) {}
        public:
            Row (Utf8CP name, uint32_t blockWidth, uint32_t blockHeight, CompressionType compressType, PixelType pixelType, 
                 bvector<ResolutionDefinition> resolutions, DgnRasterFileId id=DgnRasterFileId())
                : m_id (id), m_name (name), m_blockWidth(blockWidth), m_blockHeight(blockHeight), m_compressType(compressType), m_pixelType(pixelType), 
                  m_resolutions(resolutions)
                {
                }
        
            bool                IsValid() const         {return m_id.IsValid();}
            DgnRasterFileId     GetId() const           {return m_id;}
            Utf8CP              GetName() const         {return m_name.c_str();}
            uint32_t            GetBlockWidth() const    {return m_blockWidth;}
            uint32_t            GetBlockHeight() const   {return m_blockHeight;}
            CompressionType     GetCompressionType() const {return m_compressType;}
            PixelType           GetPixelType() const    {return m_pixelType;}
            bvector<ResolutionDefinition> const& GetResolutions() const {return m_resolutions;}
        };

    DgnRasterTable(DgnDbR project) : m_dgndb(project) {}

    //! Add a new raster.
    //! @param[in] row The definition of the raster to create.
    //! @return BE_SQLITE_OK if the raster was added; non-zero otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertRaster (DgnRasterTable::Row& row);

    //! Remove a raster and all data associated with it.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteRaster(DgnRasterFileId id);

    //! Get the information about a raster from its Id.
    //! @param[in] id The Id of the raster of interest.
    //! @return The Row data for the raster. Call IsValid() on the result to determine whether this method was successful.
    DGNPLATFORM_EXPORT DgnRasterTable::Row QueryRasterById(DgnRasterFileId id);

};

//=======================================================================================
// !Each raster block as an entry in the raster table.
// @bsiclass                                                    Donald.Morissette   10/12
//=======================================================================================
struct DgnRasterDataTable
{
private:
    DgnDbR m_dgndb;
    
public:
    struct RasterBlockData;
    typedef RefCountedPtr<RasterBlockData> RasterBlockDataPtr;
    struct RasterBlockData : RefCountedBase
        {
        private:
            friend struct DgnRasterDataTable;
            BeSQLite::Statement   m_stmt;
            inline BeSQLite::Statement* GetStatement() {return &m_stmt;}
            RasterBlockData() {}

        public:
            static RasterBlockDataPtr Create() {return new RasterBlockData();}
            ~RasterBlockData() {m_stmt.Finalize();}
            void const* GetData(uint32_t& dataSize) {dataSize = m_stmt.GetColumnBytes(0); return m_stmt.GetValueBlob(0);}
        };


    DgnRasterDataTable(DgnDbR project) : m_dgndb(project) {}

    //! Read data from the db
    //! @param[out] data a buffer into which the data content is read. (must be large enough to receive the data, computed from the rasterHeader)
    //! @param[out] outDataSize the number of valid bytes in data. 
    //! @param[in] inDataSize the size in bytes of data. 
    //! @param[in] rasterId the id of the raster file to read.
    //! @param[in] resolutionLevel the pyramid level, 0-->1:1, 1--> 1:2 ...
    //! @param[in] rowBlockId the row Id. Ex. PosY / blockSizeY.
    //! @param[in] columnBlockId the column Id. ex: PosX / blockSizeX.
    //! @return BE_SQLITE_OK if the data was successfully read, and error status otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult ReadData (void* data, uint32_t& outDataSize, uint32_t inDataSize, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId);

    //! Read data from the db
    //! @param[out] data receive the data pointer from the blob, valid until the next call or object destruction.
    //! @param[in] resolutionLevel the pyramid level, 0-->1:1, 1--> 1:2 ...
    //! @param[in] rasterId the id of the raster file to read.
    //! @param[in] resolutionLevel the pyramid level, 0-->1:1, 1--> 1:2 ...
    //! @param[in] rowBlockId the row Id. Ex. PosY / blockSizeY.
    //! @param[in] columnBlockId the column Id. ex: PosX / blockSizeX.
    //! @return BE_SQLITE_OK if the data was successfully read, and error status otherwise.
    //! @note You can use QueryFile get the file's id and to determine how many bytes to allocate for data.
    DGNPLATFORM_EXPORT RasterBlockDataPtr ReadDataPtr (BeSQLite::DbResult& status, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId);

    //! AddData to the db
    //! @param[out] data a buffer into which the data content is read.
    //! @param[in] dataSize the number of bytes in data. 
    //! @param[in] rasterId the id of the raster file to read.
    //! @param[in] resolutionLevel the pyramid level, 0-->1:1, 1--> 1:2 ...
    //! @param[in] rowBlockId the row Id. Ex. PosY / blockSizeY.
    //! @param[in] columnBlockId the column Id. ex: PosX / blockSizeX.
    //! @return BE_SQLITE_OK if the data was successfully read, and error status otherwise.
    //! @note You can use QueryFile get the file's id and to determine how many bytes to allocate for data.
    DGNPLATFORM_EXPORT BeSQLite::DbResult AddData (void const* data, uint32_t datasize, DgnRasterFileId rasterId, uint32_t resolutionLevel, uint32_t rowBlockId, uint32_t columnBlockId);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

