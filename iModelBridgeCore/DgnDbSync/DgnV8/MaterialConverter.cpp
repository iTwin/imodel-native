/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/MaterialConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <windows.h>
#include <rapidxml2json/xml2json.hpp>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFRasterFileBlockAdapter.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HRFFileFormats.h>
#include <BeJpeg/BeJpeg.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_IMAGEPP
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Mathieu.Marchand 08/2015
+===============+===============+===============+===============+===============+======*/
struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                           
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~MyImageppLibAdmin()
        {
        }
    };

/*=================================================================================**//**
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct MyImageppLibHost : ImagePP::ImageppLib::Host 
{
    virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override
        {
        return *new MyImageppLibAdmin();
        }
    virtual void _RegisterFileFormat() override 
        {
        REGISTER_SUPPORTED_FILEFORMAT
        }
};

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.Marchand 08/2015
//----------------------------------------------------------------------------------------
StatusInt readToRgba(Render::ImageR image, BeFileNameCR filename, bool pseudoBackgroundTransparency)
    {
    if (!ImagePP::ImageppLib::IsInitialized())
        ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());
        
    StatusInt status = SUCCESS;

    // these should be constructed only once.
    static HFCPtr<HGF2DWorldCluster> s_pWorldClusterPtr = new HGFHMRStdWorldCluster();
    static HPMPool s_pool(0/*illimited*/); // Memory pool shared by all rasters. (in KB)

    try
        {
        // Assuming that the file is local we need to build an URL.
        Utf8String fileURL = HFCURLFile::s_SchemeName() + "://" + filename.GetNameUtf8();
        HFCPtr<HFCURL> pURL = new HFCURLFile(fileURL);

        // Open up the raster file(headers only).  If an error occurs it throws an exception.
        HFCPtr<HRFRasterFile> pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(pURL, true/*readOnly*/);
        HFCPtr<HRFRasterFile> pAdaptedRasterFile = pRasterFile;

        // Adapt raster file to load pixels in a single chunk of memory
        if (pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE &&
            HRFRasterFileBlockAdapter::CanAdapt(pRasterFile, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT))
            {
            pAdaptedRasterFile = new HRFRasterFileBlockAdapter(pRasterFile, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT);
            }
                
        HFCPtr<HGF2DCoordSys> pLogicalCS = s_pWorldClusterPtr->GetCoordSysReference(pAdaptedRasterFile->GetWorldIdentificator());
        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&s_pool, pAdaptedRasterFile, 0/*page*/, pLogicalCS);
        HFCPtr<HRAStoredRaster> pStoredRaster = pStore->LoadRaster();

        // Can the source hold alpha?
        HFCPtr<ImagePP::HRPPixelType> pOutPixelType;
        if (pStoredRaster->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            pOutPixelType = new HRPPixelTypeV32R8G8B8A8();
            image.SetFormat(Image::Format::Rgba);
            }
        else
            {
            pOutPixelType = new HRPPixelTypeV24R8G8B8();
            image.SetFormat(Image::Format::Rgb);
            }

        // We can safely assume that textures won't go beyond 32 bits in size.
        uint64_t width64, height64;
        pStoredRaster->GetSize(&width64, &height64);
        image.SetSize((uint32_t)width64, (uint32_t)height64);

        HFCPtr<HRABitmap> pBitmap = HRABitmap::Create(image.GetWidth(), image.GetHeight(), NULL, pStoredRaster->GetPhysicalCoordSys(), pOutPixelType);

        // Since we do not want to return IPP objects we need to inject our output buffer in the destination bitmap.
        size_t bytesPerWidth = (pBitmap->GetPixelType()->CountPixelRawDataBits()*image.GetWidth() +7) / 8; // We should export HRABitmap:ComputeBytesPerWidth()
        image.GetByteStreamR().Resize((uint32_t) bytesPerWidth*image.GetHeight());
        HFCPtr<HCDPacket> pPacket(new HCDPacket(image.GetByteStreamR().GetDataP(), image.GetByteStreamR().GetSize(), image.GetByteStreamR().GetSize()));
        pPacket->SetBufferOwnership(false); 
        pBitmap->SetPacket(pPacket);
        
        HRACopyFromOptions copyOts(false/*alphaBlend*/);
        if (IMAGEPP_STATUS_Success != pBitmap->CopyFrom(*pStoredRaster, copyOts))
            status = ERROR;

        if (pseudoBackgroundTransparency && image.GetFormat() == Image::Format::Rgb)
            {
            // This is the crazy "transparency if color matches upper left pixel jive that we inherited from INGR.
            size_t nPixels = image.GetWidth() * image.GetHeight();
            ByteStream rgb = image.GetByteStream();

            image.GetByteStreamR().resize(4 * nPixels);
            uint8_t* inPixel = rgb.data(), *outPixel = image.GetByteStreamR().data(), *firstPixel = inPixel;
            for (size_t i=0; i<nPixels; ++i)
                {
                bool    isTransparent = 0 == memcmp(inPixel, firstPixel, 3);

                *outPixel++ = *inPixel++;
                *outPixel++ = *inPixel++;
                *outPixel++ = *inPixel++;
                *outPixel++ = isTransparent ? 0 : 0xff;
                }
            image.SetFormat(Image::Format::Rgba);
            }
        }

    catch (HFCException&)
        {
        status = ERROR;
        }

    return status;    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void Converter::SetMaterialUsed(RenderMaterialId id)       {m_materialUsed.insert(id);}
bool Converter::GetMaterialUsed(RenderMaterialId id) const {return m_materialUsed.find(id) != m_materialUsed.end();}
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
RenderMaterialId Converter::GetRemappedMaterial(DgnV8Api::Material const* material) 
    {
    auto const& found = m_materialRemap.find((void*) material);                // TBD.   Figure out why using DgnV8Api::Material* as key won't compile.

    if (found != m_materialRemap.end())          // First look for direct mapping to material address.
        {
        SetMaterialUsed(found->second);
        return found->second;
        }

    Utf8String utfMaterialName(material->GetName().c_str()), utfPaletteName(material->GetPalette().GetName().c_str());

    auto const& foundByName = m_materialNameRemap.find(T_MaterialNameKey(utfMaterialName, utfPaletteName));

    if (foundByName != m_materialNameRemap.end())
        {
        // NEEDS_WORK -- Verify that material matches.
        SetMaterialUsed(foundByName->second);
        return foundByName->second;
        }

    return RenderMaterialId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void Converter::AddMaterialMapping(DgnV8Api::Material const* material, Utf8StringCR name, Utf8StringCR palette, RenderMaterialId materialId) 
    {
    m_materialRemap[(void*) material] = materialId; 
    m_materialNameRemap[T_MaterialNameKey(name, palette)] = materialId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static bool extractTriadMember(Utf8CP suffix, Json::Value& output, Json::Value& value, Utf8CP name, Json::ArrayIndex index) 
    {
    Utf8CP found = strstr(name, suffix);
    ptrdiff_t prefixChars = found - name;

    if (prefixChars != strlen(name) - strlen(suffix))
        return false;

    Utf8String prefix(name, prefixChars);

    if (!output.isMember(prefix.c_str()))
        output[prefix.c_str()] = Json::Value(Json::ValueType::arrayValue);

    output[prefix.c_str()][index] = value;

    return true;
    }     

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static bool isInteger(Utf8CP pChar)
    {
    if (0 != *pChar && *pChar == '-')
        pChar++;

    for (; 0 != *pChar; pChar++)
        if (*pChar < '0' || *pChar > '9')
            return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static bool isFloat(Utf8CP pChar)
    {
    size_t  decimalCount = 0;

    if (0 != *pChar && *pChar == '-')      
        pChar++;

    for (; 0 != *pChar; pChar++)
        {
        if ('.' == *pChar)
            {
            decimalCount++;
            }
        else
            {
            if (*pChar < '0' || *pChar > '9')
                return false;
            }
        }
    return 1 == decimalCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static void convertV8MaterialToDgnDb(Json::Value& output, Json::Value& inputValue, Utf8CP name, Json::ArrayIndex* index)
    {
    Json::Value outputValue;

    if (inputValue.isString())
        {
        Int32           intValue;
        double          doubleValue;
        Utf8CP     strValue = inputValue.asCString();

        outputValue = inputValue;

        if ((NULL == name) || (NULL == strstr(name, "Name") && NULL == strstr(name, "name")))
            {
            if (isInteger(strValue) && 1 == sscanf(strValue, "%d", &intValue))                // Integers.
                outputValue = Json::Value(intValue);
            else if (isFloat(strValue) && 1 == sscanf(strValue, "%lf", &doubleValue))           // Doubles.
                outputValue = Json::Value(doubleValue);
            }
 
        if (NULL != name &&
            (extractTriadMember(".r", output, outputValue, name, 0) ||
             extractTriadMember(".g", output, outputValue, name, 1) ||
             extractTriadMember(".b", output, outputValue, name, 2) ||
             extractTriadMember(".x", output, outputValue, name, 0) ||
             extractTriadMember(".y", output, outputValue, name, 1) ||
             extractTriadMember(".z", output, outputValue, name, 2)))
            return; 
        }
    else if (inputValue.isObject())
        {
        outputValue = Json::Value(Json::ValueType::objectValue);

        for (Json::ValueIterator curr = inputValue.begin(); curr != inputValue.end(); curr++)
            convertV8MaterialToDgnDb(outputValue, *curr, curr.memberName(), NULL);
        }
    else if (inputValue.isArray())
        {
        outputValue = Json::Value(Json::ValueType::arrayValue);

        for (Json::ArrayIndex i=0; i<inputValue.size(); i++)
            convertV8MaterialToDgnDb(outputValue, inputValue[i], NULL, &i);
        }
    if (NULL != name)
        output[name] = outputValue;
    else
        output[*index] = outputValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static void parseFlags(Json::Value& renderMaterials)
    {
    Json::Value     flagsValue = renderMaterials["Flags"];

    if (flagsValue.isNull() || !flagsValue.isInt())
        {
        BeAssert(false);
        return;
        }

    renderMaterials.removeMember("Flags");

    // This structure (m_flags) is serialized in V8 DGN files.
    struct
        {
        unsigned int m_participatesInSpotlib            : 1; // Participates in radiosity/particle trace solutions (legacy variable)
        unsigned int m_noShadows                        : 1; // Does not cast shadows

        unsigned int m_hasBaseColor                     : 1;
        unsigned int m_hasSpecularColor                 : 1;
        unsigned int m_hasFinish                        : 1;
        unsigned int m_hasReflect                       : 1;
        unsigned int m_hasTransmit                      : 1;
        unsigned int m_hasDiffuse                       : 1;
        unsigned int m_hasRefract                       : 1;
        unsigned int m_hasSpecular                      : 1;
        unsigned int m_lockSpecularAndReflect           : 1; // Reflect value is same as specular
        unsigned int m_lockEfficiency                   : 1; // Restrict other properties to maintain efficiency value
        unsigned int m_lockSpecularAndBase              : 1; // Specular value is same as base color
        unsigned int m_lockFinishToSpecular             : 1; // Finish is kept in synch with specular
        unsigned int m_customSpecular                   : 1; // Indicates whether a custom specular or a preset has been specified.
        unsigned int m_linkedToLxp                      : 1; // Material linked to external lxp file
        unsigned int m_invisible                        : 1; // Not visible to eye/camera
        unsigned int m_hasTransmitColor                 : 1; // Transmit color is specified
        unsigned int m_hasTranslucencyColor             : 1; // Translucency color is specified
        unsigned int m_lockFresnelToReflect             : 1;
        unsigned int m_lockRefractionRoughnessToFinish  : 1;
        unsigned int m_hasGlowColor                     : 1;
        unsigned int m_hasReflectColor                  : 1;
        unsigned int m_hasExitColor                     : 1;
        unsigned int m_padding                          : 8;
        } v8Flags;

    UInt32 intFlags = flagsValue.asInt();

    memcpy(&v8Flags, &intFlags, sizeof (v8Flags));
    renderMaterials[RENDER_MATERIAL_FlagNoShadows                      ] = (0 != v8Flags.m_noShadows);
    renderMaterials[RENDER_MATERIAL_FlagHasBaseColor                   ] = (0 != v8Flags.m_hasBaseColor);
    renderMaterials[RENDER_MATERIAL_FlagHasSpecularColor               ] = (0 != v8Flags.m_hasSpecularColor);
    renderMaterials[RENDER_MATERIAL_FlagHasFinish                      ] = (0 != v8Flags.m_hasFinish);
    renderMaterials[RENDER_MATERIAL_FlagHasReflect                     ] = (0 != v8Flags.m_hasReflect);
    renderMaterials[RENDER_MATERIAL_FlagHasTransmit                    ] = (0 != v8Flags.m_hasTransmit);
    renderMaterials[RENDER_MATERIAL_FlagHasDiffuse                     ] = (0 != v8Flags.m_hasDiffuse);
    renderMaterials[RENDER_MATERIAL_FlagHasRefract                     ] = (0 != v8Flags.m_hasRefract);
    renderMaterials[RENDER_MATERIAL_FlagHasSpecular                    ] = (0 != v8Flags.m_hasSpecular);
    renderMaterials[RENDER_MATERIAL_FlagLockSpecularAndReflect         ] = (0 != v8Flags.m_lockSpecularAndReflect);    
    renderMaterials[RENDER_MATERIAL_FlagLockEfficiency                 ] = (0 != v8Flags.m_lockEfficiency);            
    renderMaterials[RENDER_MATERIAL_FlagLockSpecularAndBase            ] = (0 != v8Flags.m_lockSpecularAndBase);
    renderMaterials[RENDER_MATERIAL_FlagLockFinishToSpecular           ] = (0 != v8Flags.m_lockFinishToSpecular);      
    renderMaterials[RENDER_MATERIAL_FlagCustomSpecular                 ] = (0 != v8Flags.m_customSpecular);            
    renderMaterials[RENDER_MATERIAL_FlagLinkedToLxp                    ] = (0 != v8Flags.m_linkedToLxp);            
    renderMaterials[RENDER_MATERIAL_FlagInvisible                      ] = (0 != v8Flags.m_invisible);            
    renderMaterials[RENDER_MATERIAL_FlagHasTransmitColor               ] = (0 != v8Flags.m_hasTransmitColor);            
    renderMaterials[RENDER_MATERIAL_FlagHasTranslucencyColor           ] = (0 != v8Flags.m_hasTranslucencyColor);            
    renderMaterials[RENDER_MATERIAL_FlagLockFresnelToReflect           ] = (0 != v8Flags.m_lockFresnelToReflect);            
    renderMaterials[RENDER_MATERIAL_FlagLockRefractionRoughnessToFinish] = (0 != v8Flags.m_lockRefractionRoughnessToFinish);            
    renderMaterials[RENDER_MATERIAL_FlagHasGlowColor                   ] = (0 != v8Flags.m_hasGlowColor);            
    renderMaterials[RENDER_MATERIAL_FlagHasReflectColor                ] = (0 != v8Flags.m_hasReflectColor);            
    renderMaterials[RENDER_MATERIAL_FlagHasExitColor                   ] = (0 != v8Flags.m_hasExitColor); 
    }
 
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
BentleyStatus Converter::ConvertMaterialTextureMapImage(Json::Value& textureMap, DgnV8Api::DgnFile& v8File, bool pseudoBackgroundTransparency)
    {
    JsonValueCR fileNameValue = textureMap[RENDER_MATERIAL_FileName];

    if (!fileNameValue.isString())
        {
        BeAssert(false);
        return ERROR;
        }

    WString fileName(fileNameValue.asCString());

    fileName.ToLower();
    auto const& found = m_textureFileNameRemap.find(fileName);
    if (found != m_textureFileNameRemap.end())      
        {
        textureMap.removeMember(RENDER_MATERIAL_FileName);
        textureMap[RENDER_MATERIAL_TextureId] = Json::Value(found->second.GetValue());
        return SUCCESS;
        }

    StatusInt status;
    auto foundFile  = DgnV8Api::MaterialManager::GetManagerR().FindTexture (fileName.c_str(), &v8File);

    if (foundFile.empty())
        return ERROR;

    DgnDocumentMonikerPtr dgnDocMonikerPtr = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(foundFile.c_str(), nullptr);

    BeFileName fullPath(dgnDocMonikerPtr->ResolveFileName(&status).c_str());
    if (SUCCESS != status)
        return ERROR;       // File can't be found.... leave as filename.

    ByteStream textureBuffer;
    BeFile dataFile;
    if (BeFileStatus::Success != dataFile.Open(fullPath, BeFileAccess::Read))
        return ERROR;

    if (BeFileStatus::Success != dataFile.ReadEntireFile(textureBuffer))
        return ERROR;

    ImageSource imageSource;

    WString extension;
    fullPath.ParseName(NULL, NULL, NULL, &extension);

    if (0 == extension.CompareToI(L"jpg"))
        {
        imageSource = ImageSource(ImageSource::Format::Jpeg, std::move(textureBuffer));
        }
    else if (0 == extension.CompareToI(L"png"))
        {
        imageSource = ImageSource(ImageSource::Format::Png, std::move(textureBuffer));
        }
    else
        {
        Image image;
        if (SUCCESS != readToRgba(image, fullPath, pseudoBackgroundTransparency))
            {
            BeAssert(false);
            return ERROR;
            }
        imageSource = ImageSource(image, ImageSource::Format::Png);
        }

    if (!imageSource.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    Point2d size = imageSource.GetSize();
    DgnTexture texture(DgnTexture::CreateParams(GetDgnDb().GetDictionaryModel(), Utf8String(fileName), imageSource, size.x, size.y));
    texture.Insert();
    DgnTextureId textureId = texture.GetTextureId();

    if (!textureId.IsValid())
        return ERROR;

    textureMap.removeMember(RENDER_MATERIAL_FileName);
    textureMap[RENDER_MATERIAL_TextureId] = textureId.GetValue();
    m_textureFileNameRemap[fileName] = textureId;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
BentleyStatus Converter::ConvertMaterialTextureMap(Json::Value& dbMapsMap, Json::Value const& v8Map, DgnV8Api::DgnFile& v8File, DgnV8Api::DgnModelRef& modelRef)
    {
    Json::Value mapType  = v8Map[RENDER_MATERIAL_Type];

    static char* s_mapNames[] = 
        {
        "None"                    , // = 0,
        "Pattern"                 , // = 1,
        "Bump"                    , // = 2,
        "Specular"                , // = 3,
        "Reflect"                 , // = 4,
        "Transparency"            , // = 5,
        "Translucency"            , // = 6,
        "Finish"                  , // = 7,
        "Diffuse"                 , // = 8,
        "GlowAmount"              , // = 9,
        "ClearcoatAmount"         , // = 10,
        "AnisotropicDirection"    , // = 11,
        "SpecularColor"           , // = 12,
        "TransparentColor"        , // = 13,
        "TranslucencyColor"       , // = 14,
        "Displacement"            , // = 15,
        "Normal"                  , // = 16,
        "FurLength"               , // = 17,
        "FurDensity"              , // = 18,
        "FurJitter"               , // = 19,
        "FurFlex"                 , // = 20,
        "FurClumps"               , // = 21,
        "FurDirection"            , // = 22,
        "FurVector"               , // = 23,
        "FurBump"                 , // = 24,
        "FurCurls"                , // = 25,
        "GlowColor"               , // = 26,
        "ReflectColor"            , // = 27,
        "RefractionRoughness"     , // = 28,
        "SpecularFresnel"         , // = 29,
        "Geometry"                , // = 30,
        };

    if (!mapType.isInt() ||
        mapType.asInt() > sizeof (s_mapNames) / sizeof (s_mapNames[0]))
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8CP mapKey = s_mapNames[mapType.asInt()];
    Json::Value& dbMap = (dbMapsMap[mapKey] = v8Map);

    dbMap.removeMember(RENDER_MATERIAL_Type);         // Remove internal type - that is now the dgnDb mapping key.

    // If the units are either "Master" or "Sub" need to switch to real world units.
    Json::Value& mapUnits = dbMap[RENDER_MATERIAL_PatternScaleMode];

    if (mapUnits.isInt())
        {
        double conversionScale = 1.0;
        switch (mapUnits.asInt())
            {
            case 1:         // Master Units.
                conversionScale = DgnV8Api::ModelInfo::GetUorPerMaster(modelRef.GetModelInfoCP()) / DgnV8Api::ModelInfo::GetUorPerMeter(modelRef.GetModelInfoCP());
                mapUnits = (int) (RenderingAsset::TextureMap::Units::Meters);
                break;

            case 2:         // Sub Units.
               conversionScale = DgnV8Api::ModelInfo::GetUorPerSub(modelRef.GetModelInfoCP()) / DgnV8Api::ModelInfo::GetUorPerMeter(modelRef.GetModelInfoCP());
               mapUnits = (int) (RenderingAsset::TextureMap::Units::Meters);
               break;
             }
        if (1.0 != conversionScale)
            {
            Json::Value&     scaleValue  = dbMap[RENDER_MATERIAL_PatternScale];

            if (scaleValue.size() >= 2)
                {
                scaleValue[0] = scaleValue[0].asDouble() * conversionScale;
                scaleValue[1] = scaleValue[1].asDouble() * conversionScale;
                }
            }
        }
    
    bool pseudoBackgroundTransparency = v8Map.isMember(RENDER_MATERIAL_PatternFlags) && (0 != (0x0001 &  v8Map[RENDER_MATERIAL_PatternFlags].asInt()));

    // This structure (m_basicFlags) is serialized in V8 DGN files.
    struct BasicMapFlags
        {
        unsigned int m_flipV            : 1;
        unsigned int m_lockSize         : 1;
        unsigned int m_capped           : 1;
        unsigned int m_lockProjection   : 1;
        unsigned int m_flipU            : 1;
        unsigned int m_decalU           : 1;
        unsigned int m_decalV           : 1;
        unsigned int m_mirrorU          : 1;
        unsigned int m_mirrorV          : 1;
        unsigned int m_snappable        : 1;
        unsigned int m_useCellColors    : 1;
        unsigned int m_antialiasing     : 1;
        unsigned int m_padding          : 19;
        unsigned int m_mark             : 1;
        };

    if (dbMap.isMember("Flags"))
        {
        // Parse flags into booleans....
        UInt32              intFlags = dbMap["Flags"].asInt();
        BasicMapFlags       v8Flags;
        memcpy(&v8Flags, &intFlags, sizeof (v8Flags));

        dbMap[RENDER_MATERIAL_PatternFlipV] =                 (0 != v8Flags.m_flipV);
        dbMap[RENDER_MATERIAL_PatternLockSize] =              (0 != v8Flags.m_lockSize);
        dbMap[RENDER_MATERIAL_PatternCylCapped] =             (0 != v8Flags.m_capped);
        dbMap[RENDER_MATERIAL_PatternLockProjectionScale] =   (0 != v8Flags.m_lockProjection);
        dbMap[RENDER_MATERIAL_PatternFlipU] =                 (0 != v8Flags.m_flipU);
        dbMap[RENDER_MATERIAL_PatternTileDecalU] =            (0 != v8Flags.m_decalU);
        dbMap[RENDER_MATERIAL_PatternTileDecalV] =            (0 != v8Flags.m_decalV);
        dbMap[RENDER_MATERIAL_PatternTileMirrorU] =           (0 != v8Flags.m_mirrorU);
        dbMap[RENDER_MATERIAL_PatternTileMirrorV] =           (0 != v8Flags.m_mirrorV);
        dbMap[RENDER_MATERIAL_Snappable] =                    (0 != v8Flags.m_snappable);
        dbMap[RENDER_MATERIAL_UseCellColors] =                (0 != v8Flags.m_useCellColors);
        dbMap[RENDER_MATERIAL_Antialiasing] =                 (0 != v8Flags.m_antialiasing);

        dbMap.removeMember("Flags");
        }

    ConvertMaterialTextureMapImage(dbMap, v8File, pseudoBackgroundTransparency);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertMaterialTextureMaps(Json::Value& renderMaterial, DgnV8Api::DgnFile& v8File, DgnV8Api::DgnModelRef& modelRef)
    {
    JsonValueCR v8Maps = renderMaterial[RENDER_MATERIAL_Map];
    if (v8Maps.isNull())
        return;

    // Replace the V8 array with embedded types to a map by type.
    Json::Value dbMapsMap(Json::ValueType::objectValue);

    if (v8Maps.isObject())
        {
        ConvertMaterialTextureMap(dbMapsMap, v8Maps, v8File, modelRef);
        }
    else if (v8Maps.isArray())
        {
        for (Json::ArrayIndex i=0; i<v8Maps.size(); i++)
            ConvertMaterialTextureMap(dbMapsMap, v8Maps[i], v8File, modelRef);
        }
    else
        {
        BeAssert(false);
        }

    if (0 == dbMapsMap.size())
        {
        BeAssert(false);
        renderMaterial.removeMember(RENDER_MATERIAL_Map);
        }
    else
        {
        renderMaterial[RENDER_MATERIAL_Map] = dbMapsMap;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
BentleyStatus Converter::ConvertMaterial(Json::Value& renderMaterial, DgnV8Api::Material const& v8Material, DgnV8Api::DgnFile& v8File)
    {
    auto xmlString = v8Material.GetSettings().ToString(v8File);
    bvector<char> xmlChars(xmlString.GetMaxLocaleCharBytes());

    xmlString.ConvertToLocaleChars(&xmlChars.front());

    Json::Value input(Json::objectValue), output(Json::objectValue);

    if (!Json::Reader::Parse(xml2json(&xmlChars.front()).c_str(), input))
        return ERROR;

    Json::Value& v8MaterialJson = input["Material"];
    if (v8MaterialJson.isNull())
        return ERROR;

    Json::Value rootNode = Json::Value(Json::ValueType::objectValue);

    convertV8MaterialToDgnDb(rootNode, v8MaterialJson, RenderMaterial::json_renderMaterial(), NULL);
   
    if ((renderMaterial = rootNode[RenderMaterial::json_renderMaterial()]).isNull())
        return ERROR;

    parseFlags(renderMaterial);
    ConvertMaterialTextureMaps(renderMaterial, v8File, v8Material.GetModelRefR());

    return SUCCESS;
    }
  
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertModelMaterials(DgnV8ModelR dgnModel)
    {
    DgnV8Api::CachedMaterialConstIterator iter(dgnModel, true), end(dgnModel, false);

    for (;iter != end;++iter)
        {
        DgnV8Api::Material const&   v8Material = *iter;

        if (GetRemappedMaterial(&v8Material).IsValid())
            {
            // BeAssert (false);
            continue;
            }

        Json::Value renderMaterialJson;
        if (SUCCESS != ConvertMaterial(renderMaterialJson, *iter, *dgnModel.GetDgnFileP()))
            {
            BeAssert(false);
            continue;
            }

        Utf8String utfMaterialName(v8Material.GetName().c_str()), utfPaletteName(v8Material.GetPalette().GetName().c_str());
        RenderMaterial material(m_dgndb->GetDictionaryModel(), utfPaletteName, utfMaterialName);
        material.SetRenderingAsset(renderMaterialJson);

        RenderMaterialCPtr dbMaterial = material.Insert();
        if (dbMaterial.IsNull())
            {
            // WIP -- Duplicate material...check same etc.
            // BeAssert (false);
            continue;
            }

        AddMaterialMapping(&v8Material, utfMaterialName, utfPaletteName, dbMaterial->GetMaterialId());
        }
     }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void Converter::RemoveUnusedMaterials()
    {
#ifdef ALLOW_DELETION
    // NEEDSWORK: Textures, Materials, etc cannot be directly deleted; need a "purge unused" operation
    bvector <RenderMaterialId>     unusedMaterials;
    bset <DgnTextureId>         usedTextures;

    // NEEDSWORK: A proper Materials iterator?
    CachedStatementPtr stmt = GetDgnDb().Elements().GetStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ECClassId=?");
    stmt->BindId(1, RenderMaterial::QueryDgnClassId(GetDgnDb()));

    while (BE_SQLITE_ROW == stmt->Step())
        {
        RenderMaterialId materialId = stmt->GetValueId<RenderMaterialId>(0);
        if (GetMaterialUsed(materialId))
            {
            RenderMaterialCPtr material = RenderMaterial::QueryMaterial(materialId, GetDgnDb());
            Json::Value             renderMaterial, mapsMap;

            if (material.IsValid() &&
                SUCCESS == material->GetAsset(renderMaterial, MATERIAL_ASSET_Rendering) &&
                !(mapsMap = renderMaterial[RENDER_MATERIAL_Map]).isNull())
                {
                for (auto& map : mapsMap)
                    {
                    Json::Value     textureIdValue = map[RENDER_MATERIAL_TextureId];

                    if (!textureIdValue.isNull())     
                        usedTextures.insert((DgnTextureId) textureIdValue.asUInt64());
                    }
                }
            }
        else
            {
            unusedMaterials.push_back(materialId);
            }
        }
    
    bvector <DgnTextureId>  unusedTextures;
    for (auto& entry : GetDgnDb().Textures().MakeIterator())
        if (usedTextures.find(entry.GetId()) == usedTextures.end())
            unusedTextures.push_back(entry.GetId());

    for (auto& materialId : unusedMaterials)
        GetDgnDb().Elements().Delete(materialId);

    for (auto& textureId : unusedTextures)
        GetDgnDb().Textures().Delete(textureId);
#endif
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
