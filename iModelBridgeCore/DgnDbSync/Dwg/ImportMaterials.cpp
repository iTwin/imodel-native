/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportMaterials.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

#include <BeJpeg/BeJpeg.h>
#include <DgnPlatform/DgnTexture.h>
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

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG
USING_NAMESPACE_IMAGEPP

BEGIN_DWG_NAMESPACE

static const double     s_finishExponent = 0.016;
static const double     s_finishFactor = 2.5;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
struct MaterialFactory
    {
private:
    struct TextureImageAdmin : ImagePP::ImageppLibAdmin
        {
        DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)
        virtual ~TextureImageAdmin() {}
        };  // TextureImageAdmin

    struct TextureImageHost : ImagePP::ImageppLib::Host
        {
        virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override
            {
            return *new TextureImageAdmin();
            }
        virtual void _RegisterFileFormat() override
            {
            REGISTER_SUPPORTED_FILEFORMAT
            }
        };  // TextureImageHost

    DwgImporter&                            m_importer;
    DwgDbMaterialPtr&                       m_dwgMaterial;
    Utf8String                              m_paletteName;
    Utf8String                              m_materialName;
    Json::Value                             m_materialJson;
    Json::Value                             m_mapNodeJson;
    RenderingAsset::TextureMap::Units       m_textureMapUnits;
    double                                  m_unitConversionScale;
    uint32_t                                m_mapLayer;

    Json::Value         GetRGBFrom (DwgGiMaterialColorCR materialColor) const;
    double              GetBoundedScaleFrom (double scale) const;
    TextureMapping::Mode GetMapMode (DwgGiMapperCR dwgMapper) const;
    void                ExtractAndConvertMapUnits ();
    size_t              ReadRawBytes (ByteStream& bytesOut, BeFileNameCR fileNameIn) const;
    BentleyStatus       ReadToRgba (Render::ImageR image, BeFileNameCR filename, bool pseudoBackgroundTransparency) const;
    BentleyStatus       CreateTextureFromImageFile (Json::Value& mapJson, DwgGiImageFileTextureCR imageFile, bool inverted);
    BentleyStatus       CreateTextureMap (Json::Value& mapJson, DwgGiMaterialMapCR map, bool isOn);
    bool                FindTextureFile (BeFileNameR filename);
    void                ConvertDiffuse ();
    void                ConvertBump ();
    void                ConvertSpecular ();
    void                ConvertReflection ();
    void                ConvertRefraction ();
    void                ConvertOpacity ();
    void                ConvertTranslucency ();
    void                ConvertIllumination ();
    void                ConvertShinness ();
    void                ConvertAmbient ();
    void                ConvertNormal ();
    void                Convert ();

public:
    // the constucter
    MaterialFactory (DwgImporter& importer, DwgDbMaterialPtr& mat, Utf8StringCR pal, Utf8StringCR mn) : m_importer(importer), m_dwgMaterial(mat), m_paletteName(pal), m_materialName(mn)
        {
        m_textureMapUnits = RenderingAsset::TextureMap::Units::Relative;
        m_unitConversionScale = 1.0;
        m_materialJson = Json::Value (Json::objectValue);
        m_mapNodeJson = Json::Value (Json::objectValue);
        m_mapLayer = 1.0;
        }
    // main method to create the material from DWG
    BentleyStatus       Create (RenderMaterialId& outId);
    // update existing material from DWG
    BentleyStatus       Update (RenderMaterialR out);
    }; // MaterialFactory

END_DWG_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     MaterialFactory::GetRGBFrom (DwgGiMaterialColorCR materialColor) const
    {
    DwgCmEntityColor    cmColor;
    materialColor.GetColor (cmColor);

    Json::Value rgb(Json::ValueType::arrayValue);

    rgb[0] = static_cast<double> (cmColor.GetRed() / 255.0);
    rgb[1] = static_cast<double> (cmColor.GetGreen() / 255.0);
    rgb[2] = static_cast<double> (cmColor.GetBlue() / 255.0);

    return  rgb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          MaterialFactory::GetBoundedScaleFrom (double scale) const
    {
    if (scale < 0.0)
        return  0.0;
    else if (scale > 1.0)
        return  1.0;
    return  scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TextureMapping::Mode   MaterialFactory::GetMapMode (DwgGiMapperCR dwgMapper) const
    {
    switch (dwgMapper.GetProjection())
        {
        case DwgGiMapper::Planar:       return TextureMapping::Mode::Planar;
        case DwgGiMapper::Box:          return TextureMapping::Mode::Cubic;
        case DwgGiMapper::Cylinder:     return TextureMapping::Mode::Cylindrical;
        case DwgGiMapper::Sphere:       return TextureMapping::Mode::Spherical;
        case DwgGiMapper::InheritProjection:
        default:                        return TextureMapping::Mode::Parametric;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ExtractAndConvertMapUnits ()
    {
    m_unitConversionScale = 1.0;

    DwgDbUnits  scaleUnits = m_dwgMaterial->GetScaleUnits ();
    switch (scaleUnits)
        {
        case DwgDbUnits::Meters:
            m_textureMapUnits = RenderingAsset::TextureMap::Units::Meters;
            break;
        case DwgDbUnits::Millimeters:
            m_textureMapUnits = RenderingAsset::TextureMap::Units::Millimeters;
            break;
        case DwgDbUnits::Feet:
            m_textureMapUnits = RenderingAsset::TextureMap::Units::Feet;
            break;
        case DwgDbUnits::Inches:
            m_textureMapUnits = RenderingAsset::TextureMap::Units::Inches;
            break;
        case DwgDbUnits::Undefined:
            {
            bool    isSupportedUnit = true;
            switch (m_importer.GetModelSpaceUnits())
                {
                case StandardUnit::MetricMeters:
                    m_textureMapUnits = RenderingAsset::TextureMap::Units::Meters;
                    break;
                case StandardUnit::MetricMillimeters:
                    m_textureMapUnits = RenderingAsset::TextureMap::Units::Millimeters;
                    break;
                case StandardUnit::EnglishFeet:
                    m_textureMapUnits = RenderingAsset::TextureMap::Units::Feet;
                    break;
                case StandardUnit::EnglishInches:
                    m_textureMapUnits = RenderingAsset::TextureMap::Units::Inches;
                    break;
                default:
                    isSupportedUnit = false;
                }
            if (isSupportedUnit)
                break;
            // fall through to default converting units to meters:
            }
        default:
            // Convert unsupported units to meters:
            m_textureMapUnits = RenderingAsset::TextureMap::Units::Meters;
            m_unitConversionScale = m_importer.GetScaleToMeters ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          MaterialFactory::ReadRawBytes (ByteStream& bytesOut, BeFileNameCR fileNameIn) const
    {
    FILE*   binaryFile = fopen (fileNameIn.GetNameUtf8().c_str(), "rb");
    if (nullptr == binaryFile)
        return 0;

    fseek (binaryFile, 0, SEEK_END);
    size_t numBytes = ftell (binaryFile);

    bytesOut.Resize ((uint32_t)numBytes);
    fseek (binaryFile, 0, SEEK_SET);

    fread (bytesOut.GetDataP(), 1, numBytes, binaryFile);

    fclose (binaryFile);

    return numBytes;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.Marchand 08/2015
//----------------------------------------------------------------------------------------
BentleyStatus   MaterialFactory::ReadToRgba (Render::ImageR image, BeFileNameCR filename, bool pseudoBackgroundTransparency) const
    {
    if (!ImagePP::ImageppLib::IsInitialized())
        ImagePP::ImageppLib::Initialize (*new TextureImageHost());
        
    BentleyStatus   status = BSISUCCESS;

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
            status = BSIERROR;

        if (pseudoBackgroundTransparency && image.GetFormat() == Image::Format::Rgb)
            {
            // This is the crazy "transparency if color matches upper left pixel jive that we inherited from INGR.
            size_t          nPixels = image.GetWidth() * image.GetHeight();
            ByteStream      rgb = image.GetByteStream();
            ByteStream&     imageData = image.GetByteStreamR();

            imageData.resize (4 * nPixels );
                
            uint8_t       *inPixel = rgb.data(), *outPixel = imageData.data(), *firstPixel = inPixel;
            for (size_t i=0; i<nPixels; i++)
                {
                bool    isTransparent = 0 == memcmp (inPixel, firstPixel, 3);

                *outPixel++ = *inPixel++;
                *outPixel++ = *inPixel++;
                *outPixel++ = *inPixel++;
                *outPixel++ = (isTransparent) ? 0 : 0xff;
                }
            image.SetFormat (Image::Format::Rgba);
            }
        }

    catch (HFCException&)
        {
        status = BSIERROR;
        }

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MaterialFactory::FindTextureFile (BeFileNameR filename)
    {
    if (m_importer._FindTextureFile(filename))
        return  true;

    // feedback about empty file name
    if (filename.empty())
        {
        // except for the default "Global" which is in all DWG files but unlikely used by a user
        if (m_dwgMaterial->GetObjectId() != m_importer.GetDwgDb().GetMaterialGlobalId())
            m_importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::MaterialNoFile(), m_materialName.c_str());
        return  false;
        }

    // warn about the missing material texture file
    Utf8PrintfString    context("Material \"%s\"", m_materialName.c_str());
    m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::FileNotFound(), Utf8String(filename).c_str(), context.c_str());

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MaterialFactory::CreateTextureFromImageFile (Json::Value& mapJson, DwgGiImageFileTextureCR imageFile, bool inverted)
    {
    BeFileName  fileName(imageFile.GetSourceFileName());
    if (!this->FindTextureFile(fileName))
        return  BSIERROR;

    Utf8String      utf8Name = fileName.GetNameUtf8 ();
    DgnTextureId    textureId = m_importer.GetDgnMaterialTextureFor (utf8Name);
    if (textureId.IsValid())
        {
        mapJson[RENDER_MATERIAL_TextureId] = textureId.ToHexStr();
        return  BSISUCCESS;
        }

    ImageSource imageSource;
    Image       image;
    WString     extension = fileName.GetExtension ();

    // extract DWG image from file:
    if (extension.EqualsI(L"jpg"))
        {
        ByteStream  textureBuffer;
        if (this->ReadRawBytes(textureBuffer, fileName) == 0)
            {
            m_importer.ReportError (IssueCategory::MissingData(), Issue::MaterialError(), Utf8PrintfString("cannot read texture image from %s for material %s", utf8Name.c_str(), m_materialName.c_str()).c_str());
            return BSIERROR;
            }
    
        imageSource = ImageSource(ImageSource::Format::Jpeg, std::move(textureBuffer));
        image = Image (imageSource);
        }
    else
        {
        if (BSISUCCESS != this->ReadToRgba(image, fileName, inverted))
            {
            m_importer.ReportError (IssueCategory::MissingData(), Issue::MaterialError(), Utf8PrintfString("cannot texture RGBA from image file %s for material %s", utf8Name.c_str(), m_materialName.c_str()).c_str());
            return BSIERROR;
            }
        
        imageSource = ImageSource (image, ImageSource::Format::Png);
        }

    // create a DGN texture and added to DB:
    DgnDbStatus     status = DgnDbStatus::Success;
    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "DgnTexture");
        model = &m_importer.GetDgnDb().GetDictionaryModel ();
        }
    DgnTexture      texture(DgnTexture::CreateParams(*model, utf8Name, imageSource, image.GetWidth(), image.GetHeight()));

    if (texture.Insert(&status).IsNull() || DgnDbStatus::Success != status)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MaterialError(), Utf8PrintfString("can't create texture from %s for material %s", utf8Name.c_str(), m_materialName.c_str()).c_str());
        return static_cast<BentleyStatus>(status);
        }

    textureId = texture.GetTextureId();
    if (textureId.IsValid())
        {
        mapJson[RENDER_MATERIAL_TextureId] = textureId.ToHexStr();
        m_importer.AddDgnMaterialTexture (utf8Name, textureId);
        }
    else
        {
        BeAssert (false && "Failed creating material image texture!");
        mapJson[RENDER_MATERIAL_FileName] = utf8Name;
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MaterialFactory::CreateTextureMap (Json::Value& mapJson, DwgGiMaterialMapCR dwgMap, bool isOn)
    {
    DwgGiMaterialTextureCP  texture = dwgMap.GetTexture ();
    if (nullptr == texture)
        return  BSIERROR;

    mapJson.clear ();
    mapJson[RENDER_MATERIAL_PatternScaleMode] = static_cast<int> (m_textureMapUnits);

    if (dwgMap.GetSource() == DwgGiMaterialMap::File)
        {
        // extract and convert DWG image texture to DgnDb texture:
        DwgGiImageFileTextureCP imageFile = texture->ToDwgGiImageFileTextureCP ();
        if (nullptr == imageFile)
            return  BSIERROR;

        bool    inverted = dwgMap.GetBlendFactor() < 0.0;

        if (BSISUCCESS != this->CreateTextureFromImageFile(mapJson, *imageFile, inverted))
            return  BSIERROR;
        }
    else
        {
        // WIP - convert procedural textures
        return  BSIERROR;
        }

    DwgGiMapper mapper;
    dwgMap.GetMapper (mapper);

    mapJson[RENDER_MATERIAL_PatternOff] = !isOn;
    mapJson[RENDER_MATERIAL_PatternMapping] = static_cast<int> (this->GetMapMode(mapper));
#ifdef REMOVED
    mapJson[RENDER_MATERIAL_Layer] = m_mapLayer;
    mapJson[RENDER_MATERIAL_AntialiasStrength] = 100;
#endif
    mapJson[RENDER_MATERIAL_Antialiasing] = true;

    Transform   transform;
    mapper.GetTransform (transform);

    double      xScale = transform.ColumnX().Magnitude ();
    double      yScale = transform.ColumnY().Magnitude ();

    // map pattern scales:
    Json::Value patternScale(Json::ValueType::arrayValue);
    patternScale[0] = fabs(xScale) > 1.0-6 ? m_unitConversionScale / xScale : 1.0;
    patternScale[1] = fabs(yScale) > 1.0-6 ? m_unitConversionScale / yScale : 1.0;

    mapJson[RENDER_MATERIAL_PatternScale] = patternScale;

    // map pattern offset:
    DPoint3d    origin = transform.Origin ();
    Json::Value patternOffset(Json::ValueType::arrayValue);
    patternOffset[0] = origin.x;
    patternOffset[1] = origin.y;

    mapJson[RENDER_MATERIAL_PatternOffset] = patternOffset;

    DwgGiMapper::TileBy uTiling = mapper.GetUTiling ();
    DwgGiMapper::TileBy vTiling = mapper.GetVTiling ();

    // map tilings
    if (DwgGiMapper::Mirror == uTiling)
        mapJson[RENDER_MATERIAL_PatternTileMirrorU] = true;
    if (DwgGiMapper::Mirror == vTiling)
        mapJson[RENDER_MATERIAL_PatternTileMirrorV] = true;
    if (DwgGiMapper::Crop == uTiling && DwgGiMapper::Crop == vTiling)
        mapJson[RENDER_MATERIAL_PatternCylCapped] = true;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertDiffuse ()
    {
    DwgGiMaterialColor  dwgColor;
    DwgGiMaterialMap    dwgMap;

    m_dwgMaterial->GetDiffuse (dwgColor, dwgMap);

    m_materialJson[RENDER_MATERIAL_FlagHasBaseColor] = DwgGiMaterialColor::Override == dwgColor.GetMethod();

    if (m_materialJson[RENDER_MATERIAL_FlagHasBaseColor].asBool())
        {
        m_materialJson[RENDER_MATERIAL_Color] = this->GetRGBFrom (dwgColor);
        m_materialJson[RENDER_MATERIAL_Pattern] = this->GetRGBFrom (dwgColor);
        }

    bool        hasPattern = m_materialJson[RENDER_MATERIAL_FlagHasDiffuse].asBool ();
    Json::Value mapJson (Json::ValueType::objectValue);

    if (this->CreateTextureMap(mapJson, dwgMap, hasPattern) == BSISUCCESS)
        m_mapNodeJson[RENDER_MATERIAL_MAP_Pattern] = mapJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertBump ()
    {
    DwgGiMaterialMap    dwgMap;

    m_dwgMaterial->GetBump (dwgMap);

    bool        hasBump = 0 != (m_dwgMaterial->GetChannelFlags() & DwgDbMaterial::UseBump);
    Json::Value mapJson (Json::ValueType::objectValue);

    if (this->CreateTextureMap(mapJson, dwgMap, hasBump) == BSISUCCESS)
        m_mapNodeJson[RENDER_MATERIAL_MAP_Bump] = mapJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertSpecular ()
    {
    DwgGiMaterialColor  dwgColor;
    DwgGiMaterialMap    dwgMap;

    double  scale = m_dwgMaterial->GetSpecular (dwgColor, dwgMap);

    if (DwgGiMaterialColor::Override == dwgColor.GetMethod())
        scale = dwgColor.GetFactor ();
    // WIP - convert specular scale?
    scale /= 100.0;

    m_materialJson[RENDER_MATERIAL_Specular] = scale;
    m_materialJson[RENDER_MATERIAL_FlagHasSpecularColor] = DwgGiMaterialColor::Override == dwgColor.GetMethod();

    bool    hasSpecular = m_materialJson[RENDER_MATERIAL_FlagHasSpecularColor].asBool ();
    if (hasSpecular)
        m_materialJson[RENDER_MATERIAL_SpecularColor] = this->GetRGBFrom (dwgColor);

    m_materialJson[RENDER_MATERIAL_FlagLockFinishToSpecular] = false;

    Json::Value mapJson (Json::ValueType::objectValue);
    if (this->CreateTextureMap(mapJson, dwgMap, hasSpecular) == BSISUCCESS)
        m_mapNodeJson[RENDER_MATERIAL_MAP_Specular] = mapJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertReflection ()
    {
    DwgGiMaterialColor  dwgColor;
    DwgGiMaterialMap    dwgMap;

    double  scale = m_dwgMaterial->GetReflectivity ();

    m_materialJson[RENDER_MATERIAL_Reflect] = scale;

    double  specularScale = m_materialJson[RENDER_MATERIAL_Specular].asDouble ();
    if (scale > 1.e-6 && specularScale > 1.e-6)
        scale /= specularScale;

    bool        hasReflection = m_materialJson[RENDER_MATERIAL_FlagHasReflect].asBool ();
    Json::Value mapJson(Json::ValueType::objectValue);

    if (this->CreateTextureMap(mapJson, dwgMap, hasReflection) == BSISUCCESS)
        m_mapNodeJson[RENDER_MATERIAL_MAP_Reflect] = mapJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertRefraction ()
    {
    DwgGiMaterialMap    dwgMap;

    double  scale = m_dwgMaterial->GetRefraction (dwgMap);

    m_materialJson[RENDER_MATERIAL_Refract] = this->GetBoundedScaleFrom (scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertOpacity ()
    {
    DwgGiMaterialMap    dwgMap;

    double  scale = m_dwgMaterial->GetOpacity (dwgMap);

    m_materialJson[RENDER_MATERIAL_Transmit] = 1.0 - this->GetBoundedScaleFrom(scale);
    m_materialJson[RENDER_MATERIAL_FlagHasTransmitColor] = false;

    bool        hasTransmit = m_materialJson[RENDER_MATERIAL_FlagHasTransmit].asBool ();
    Json::Value mapJson(Json::ValueType::objectValue);

    if (this->CreateTextureMap(mapJson, dwgMap, hasTransmit) == BSISUCCESS)
        m_mapNodeJson[RENDER_MATERIAL_MAP_Transparency] = mapJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertTranslucency ()
    {
    double  scale = m_dwgMaterial->GetTranslucence ();

#ifdef REMOVED
    m_materialJson[RENDER_MATERIAL_Translucency] = scale;
#endif
    m_materialJson[RENDER_MATERIAL_FlagHasTranslucencyColor] = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertIllumination ()
    {
    double  scale = m_dwgMaterial->GetSelfIllumination ();

    m_materialJson[RENDER_MATERIAL_Glow] = 100.0* scale;
    m_materialJson[RENDER_MATERIAL_FlagHasGlowColor] = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertShinness ()
    {
    double      scale = fabs (100 * m_dwgMaterial->GetShininess());

    // apply V8 scale factor conversion:
    if (scale > 2000.0)
        scale = 2000.0;

    scale = ::pow (10.0, s_finishExponent * scale) / s_finishFactor;

    m_materialJson[RENDER_MATERIAL_Finish] = scale;
    m_materialJson[RENDER_MATERIAL_FlagHasFinish] = scale > 1.e-5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertAmbient ()
    {
    DwgGiMaterialColor  dwgColor;
    m_dwgMaterial->GetAmbient (dwgColor);

    m_materialJson[RENDER_MATERIAL_Ambient] = this->GetRGBFrom (dwgColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::ConvertNormal ()
    {
#ifdef RENDER_MATERIAL_MAP_Normal_REMOVED
    if (0 == (m_dwgMaterial->GetChannelFlags() & DwgDbMaterial::UseNormalMap))
        return;

    DwgDbMaterial::NormalMapMethod  method;
    DwgGiMaterialMap                dwgMap;
    double                          scale = 0.0;

    if (DwgDbStatus::Success == m_dwgMaterial->GetNormalMap(dwgMap, method, scale))
        {
        Json::Value mapJson(Json::ValueType::objectValue);

        if (this->CreateTextureMap(mapJson, dwgMap, true) == BSISUCCESS)
            m_mapNodeJson[RENDER_MATERIAL_MAP_Normal] = mapJson;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            MaterialFactory::Convert ()
    {
    // Material flags
    DwgDbMaterial::ChannelFlags     channelFlags = m_dwgMaterial->GetChannelFlags ();

    m_materialJson[RENDER_MATERIAL_FlagHasDiffuse] = 0 != (channelFlags & DwgDbMaterial::UseDiffuse);
    m_materialJson[RENDER_MATERIAL_FlagHasSpecular] = 0 != (channelFlags & DwgDbMaterial::UseSpecular);
    m_materialJson[RENDER_MATERIAL_FlagHasReflect] = 0 != (channelFlags & DwgDbMaterial::UseReflection);
    m_materialJson[RENDER_MATERIAL_FlagHasRefract] = 0 != (channelFlags & DwgDbMaterial::UseRefraction);
    m_materialJson[RENDER_MATERIAL_FlagHasTransmit] = 0 != (channelFlags & DwgDbMaterial::UseOpacity);
    m_materialJson[RENDER_MATERIAL_FlagCustomSpecular] = DwgDbMaterial::MetalShader == m_dwgMaterial->GetIlluminationModel();

    // Extract material scale units from material's xdata:
    this->ExtractAndConvertMapUnits ();

    // Diffuse -> pattern
    this->ConvertDiffuse ();
    // Bump
    this->ConvertBump ();
    // Specular
    this->ConvertSpecular ();
    // Reflection
    this->ConvertReflection ();
    // Refraction
    this->ConvertRefraction ();
    // Opacity/transparency/transmitance
    this->ConvertOpacity ();
    // Translucency
    this->ConvertTranslucency ();
    // Self-illumination/glow
    this->ConvertIllumination ();
    // Shinness/finish
    this->ConvertShinness ();
    // Normal
    this->ConvertNormal ();
    // Ambient
    this->ConvertAmbient ();

    // Append map node to root material:
    if (m_mapNodeJson.size() > 0)
        m_materialJson[RENDER_MATERIAL_Map] = m_mapNodeJson;
    else
        m_materialJson.removeMember (RENDER_MATERIAL_Map);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MaterialFactory::Create (RenderMaterialId& idOut)
    {
    this->Convert ();

    DefinitionModelP model = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == model)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "RenderMaterial");
        model = &m_importer.GetDgnDb().GetDictionaryModel ();
        }

    // create a DGN material
    RenderMaterial dgnMaterial(*model, m_paletteName, m_materialName);

    // WIP - need a description?
    // Utf8String  description (m_dwgMaterial->GetDescription().c_str());
    // dgnMaterial.SetDescription (description.c_str());

    // add the Json value to MATERIAL_ASSET_Rendering
    dgnMaterial.SetRenderingAsset (m_materialJson);

    // add the material to DB
    if (dgnMaterial.Insert().IsNull())
        {
        m_importer.ReportError (IssueCategory::DiskIO(), Issue::MaterialError(), Utf8PrintfString("failed adding material [%ls] into DB", m_dwgMaterial->GetName().c_str()).c_str());
        return BSIERROR;
        }

    idOut = dgnMaterial.GetMaterialId ();

    return  idOut.IsValid() ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MaterialFactory::Update (RenderMaterialR out)
    {
    this->Convert ();
    
    out.SetRenderingAsset (m_materialJson);

    // add the material to DB
    if (out.Update().IsNull())
        {
        m_importer.ReportError (IssueCategory::DiskIO(), Issue::MaterialError(), Utf8PrintfString("failed adding material [%ls] into DB", m_dwgMaterial->GetName().c_str()).c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgImporter::_FindTextureFile (BeFileNameR filename) const
    {
    if (filename.empty())
        return  false;

    // absolute path precedes search paths
    if (filename.DoesPathExist())
        return  true;

    BeFileName basename (BeFileName::GetFileNameAndExtension(filename.c_str()));
    if (basename.empty())
        return  false;

    // search user paths
    for (auto path : this->GetMaterialSearchPaths())
        {
        BeFileName  fullspec (path);
        fullspec.AppendToPath (basename);
        if (fullspec.DoesPathExist())
            {
            filename = fullspec;
            return  true;
            }
        }
    
    // search input DWG path as requested
    if (this->GetOptions().IsDwgPathInMaterialSearch())
        {
        BeFileName  fullspec (this->GetRootDwgFileName().GetDirectoryName());
        fullspec.AppendToPath (basename);
        if (fullspec.DoesPathExist())
            {
            filename = fullspec;
            return  true;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportMaterial (DwgDbMaterialPtr& dwgMaterial, Utf8StringCR paletteName, Utf8StringCR materialName)
    {
    RenderMaterialId   dgnMaterialId;
    MaterialFactory factory(*this, dwgMaterial, paletteName, materialName);
    BentleyStatus   status = factory.Create (dgnMaterialId);

    if (status == BSISUCCESS)
        {
        this->GetSyncInfo().InsertMaterial (dgnMaterialId, *dwgMaterial.get());
        m_importedMaterials.insert (T_DwgRenderMaterialId(dwgMaterial->GetObjectId(), dgnMaterialId));
        }
    
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportMaterialSection ()
    {
    DwgDbDictionaryPtr  materialTable (this->GetDwgDb().GetMaterialDictionaryId(), DwgDbOpenMode::ForRead);
    if (materialTable.IsNull())
        return  BSIERROR;

    DwgDbDictionaryIteratorPtr iter = materialTable->GetIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;
    
    this->SetStepName (ProgressMessage::STEP_IMPORTING_MATERIALS());

    DwgDbObjectId   materialByLayer = this->GetDwgDb().GetMaterialByLayerId ();
    DwgDbObjectId   materialByBlock = this->GetDwgDb().GetMaterialByBlockId ();

    // set palette name from DWG file name:
    Utf8PrintfString  paletteName ("Palette-%ls", this->GetRootDwgFileName().GetFileNameAndExtension().c_str());

    uint32_t    count = 0;
    for (; !iter->Done(); iter->Next())
        {
        // skip ByLayer and ByBlock materials
        if (iter->GetObjectId() == materialByLayer || iter->GetObjectId() == materialByBlock)
            continue;

        DwgDbMaterialPtr    material(iter->GetObjectId(), DwgDbOpenMode::ForRead);
        if (material.IsNull())
            {
            this->ReportError (IssueCategory::Unknown(), Issue::CantOpenObject(), Utf8PrintfString("material ID=%ld", iter->GetObjectId().ToAscii().c_str()).c_str());
            continue;
            }

        // WIP - do we allow annonymous materials(ones that may have dups)?
        Utf8String   materialName (material->GetName().c_str());
        if (materialName.empty())
            {
            materialName.Sprintf ("unnamed_%d", count);
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Error(), Utf8PrintfString("an empty material name is replaced with \"%s\"!", materialName.c_str()).c_str());
            }
        else
            {
            LOG_MATERIAL.tracev ("Processinging DWG Material %s", materialName.c_str());
            }

        if ((count++ % 100) == 0)
            this->Progress ();

        if (this->IsUpdating())
            {
            DwgSyncInfo::Material   oldMaterial;
            if (m_syncInfo.FindMaterial(oldMaterial, material->GetObjectId()))
                {
                // update material & syncInfo as needed
                this->_OnUpdateMaterial (oldMaterial, material);
                // add to processed material list
                m_importedMaterials.insert (T_DwgRenderMaterialId(material->GetObjectId(), oldMaterial.m_id));
                continue;
                }
            }

        this->_ImportMaterial (material, paletteName, materialName);
        }
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId   DwgImporter::GetDgnMaterialFor (DwgDbObjectIdCR materialId)
    {
    auto        found = m_importedMaterials.find (materialId);
    if (found != m_importedMaterials.end())
        return  found->second;

    // default to the first material, usually the Global
    for (auto entry : RenderMaterial::MakeIterator(this->GetDgnDb()))
        return  entry.GetId ();

    return  RenderMaterialId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId    DwgImporter::GetDgnMaterialTextureFor (Utf8StringCR fileName)
    {
    auto        found = m_materialTextures.find (fileName);
    if (found != m_materialTextures.end())
        return  found->second;

    return  DgnTextureId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::AddDgnMaterialTexture (Utf8StringCR fileName, DgnTextureId texture)
    {
    bpair <Utf8String, DgnTextureId>    newEntry(fileName, texture);
    m_materialTextures.insert (newEntry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_OnUpdateMaterial (DwgSyncInfo::Material const& syncMaterial, DwgDbMaterialPtr& dwgMaterial)
    {
    if (!syncMaterial.IsValid() || dwgMaterial.IsNull())
        return  BSIERROR;

    DwgDbDatabasePtr    dwg = dwgMaterial->GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    if (this->_GetChangeDetector()._ShouldSkipFile(*this, *dwg.get()))
        return  BSISUCCESS;
        
    DwgSyncInfo::Material   newSyncMaterial(syncMaterial.m_id, DwgSyncInfo::DwgFileId::GetFrom(*dwg), this->GetCurrentIdPolicy(), *dwgMaterial.get());
    if (!newSyncMaterial.IsValid())
        return  BSIERROR;

    if (newSyncMaterial.IsSame(syncMaterial))
        return  BSISUCCESS;

    RenderMaterialPtr  newElement;
    RenderMaterialCPtr oldElement = RenderMaterial::Get (this->GetDgnDb(), syncMaterial.m_id);
    if (!oldElement.IsValid() || !(newElement = oldElement->MakeCopy<RenderMaterial>()).IsValid())
        return  BSIERROR;

    // re-create the material and update BIM:
    MaterialFactory factory (*this, dwgMaterial, oldElement->GetPaletteName(), newSyncMaterial.m_name);
    BentleyStatus   status = factory.Update (*newElement.get());

    // update syncInfo
    if (BSISUCCESS == status)
        status = m_syncInfo.UpdateMaterial (newSyncMaterial);

    return  status;
    }
