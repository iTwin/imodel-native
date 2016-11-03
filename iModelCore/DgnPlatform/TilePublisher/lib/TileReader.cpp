/*--------------------------------------------------------------------------------------+                                                                                                                                      
|
|     $Source: TilePublisher/lib/TileReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include "Constants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileReader::Status  TileReader::ReadTileFromGLTF (TileMeshList& meshes, BeFileNameCR fileName)
    {
    std::FILE*      file;

    if (nullptr == (file = _wfopen (fileName.c_str(), L"rb")))
        return Status::UnableToOpenFile;

    static      const char s_b3dmMagic[] = "b3dm";
    char        b3dmMagic[4];
    uint32_t    b3dmLength, batchTableStrLen, batchTableBinaryLen, b3dmNumBatches; 

    if (1 != std::fread(&b3dmMagic, 4, 1, file) ||
        0 != memcmp (b3dmMagic, s_b3dmMagic, 4) ||
        1 != std::fread(&b3dmLength, sizeof(b3dmLength), 1, file) ||
        1 != std::fread(&batchTableStrLen, sizeof(batchTableStrLen), 1, file) ||
        1 != std::fread(&batchTableBinaryLen, sizeof(batchTableBinaryLen), 1, file) ||
        1 != std::fread(&b3dmNumBatches, sizeof(b3dmNumBatches), 1, file))
        return Status::InvalidHeader;

    bvector<char>       batchTableData (batchTableStrLen);
    Json::Value         batchTableValue;
    Json::Reader        reader;
    
    if (1 != fread (batchTableData.data(), 1, batchTableStrLen, file))
        return Status::ReadError;
    
    if (! reader.parse (batchTableData.data(), batchTableData.data() + batchTableStrLen, batchTableValue))
        return Status::BatchTableError;

    return Status::Success;     // WIP.
    }
