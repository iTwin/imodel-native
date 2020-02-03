/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/CreatorParser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "CreatorParser.h"

CreatorParser::CreatorParser(void)
    {
    }

CreatorParser::~CreatorParser(void)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void CreatorParser::SetAccessMode(BooleanMap& accessList, const HFCPtr<HRFCapability>& pi_rpCapability)
{
    if (pi_rpCapability->GetAccessMode().m_HasCreateAccess)
        accessList.insert(BooleanMap::value_type(Create, true));
    if (pi_rpCapability->GetAccessMode().m_HasReadAccess)
        accessList.insert(BooleanMap::value_type(Read, true));
    if (pi_rpCapability->GetAccessMode().m_HasWriteAccess)
        accessList.insert(BooleanMap::value_type(Write, true));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParseBlockTypeCapabilities(HRFRasterFileCreator& rasterCreator, 
                                              const HFCPtr<HRFBlockCapability>& pBlockCapability, 
                                              BlockObject& blockObject)
{
    // Block Initialization
    blockObject.heightIncrement = 0;
    blockObject.widthIncrement = 0;
    blockObject.maximumHeight = 0;
    blockObject.maximumWidth = 0;
    blockObject.minimumHeight = 0;
    blockObject.minimumWidth = 0;
    blockObject.isMultiResolution = false;
    
    HFCPtr<HRFTileCapability> pTileCap;
    HFCPtr<HRFStripCapability> pStripCap;
    
    // Setting up the block object depending on its block type
    blockObject.ID = pBlockCapability->GetBlockType().m_BlockType;
    switch (blockObject.ID)
    {
    case HRFBlockType::AUTO_DETECT:
        blockObject.label = _T("Auto-Detect");
        break;
    case HRFBlockType::LINE:
        blockObject.label = _T("Line");
        break;
    case HRFBlockType::TILE:
        blockObject.label = _T("Tile");
        pTileCap = (const HFCPtr<HRFTileCapability>&) pBlockCapability;
            
        blockObject.heightIncrement = pTileCap->GetHeightIncrement();
        blockObject.widthIncrement  = pTileCap->GetWidthIncrement();
        blockObject.maximumHeight   = pTileCap->GetMaxHeight();
        blockObject.maximumWidth    = pTileCap->GetMaxWidth();
        blockObject.minimumHeight   = pTileCap->GetMinHeight();
        blockObject.minimumWidth    = pTileCap->GetMinWidth();
        break;
    case HRFBlockType::STRIP:
        blockObject.label = _T("Strip");
        pStripCap = (const HFCPtr<HRFStripCapability>&) pBlockCapability;
            
        blockObject.heightIncrement = pStripCap->GetHeightIncrement();
        blockObject.widthIncrement  = 0;
        blockObject.maximumHeight   = pStripCap->GetMaxHeight();
        blockObject.maximumWidth    = 0;
        blockObject.minimumHeight   = pStripCap->GetMinHeight();
        blockObject.minimumWidth    = 0;
        break;
    case HRFBlockType::IMAGE:
        blockObject.label = _T("Image");
        break;
    default:
        blockObject.label = _T("Unknown");
        break;
    }
    
    // Maximum size in bytes
    blockObject.maxSizeInBytes = pBlockCapability->GetMaxSizeInBytes();
    
    // Block Access
    if (pBlockCapability->GetBlockAccess() == HRFBlockAccess::RANDOM)
        blockObject.access = _T("Random");
    else
        blockObject.access = _T("Sequential");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ConvertDownSamplingMethodToString(HRFDownSamplingMethod::DownSamplingMethod method, WString& output)
{
    output.clear();
    switch (method)
    {
    case HRFDownSamplingMethod::AVERAGE:
        output = _T("Average");
        break;
    case HRFDownSamplingMethod::NEAREST_NEIGHBOUR:
        output = _T("Nearest Neighbour");
        break;
    case HRFDownSamplingMethod::NONE:
        output = _T("None");
        break;
    case HRFDownSamplingMethod::ORING4:
        output = _T("OR4");
        break;
    case HRFDownSamplingMethod::UNKOWN:
        output = _T("Unknown");
        break;
    case HRFDownSamplingMethod::VECTOR_AWARENESS:
        output = _T("Vector Awareness");
        break;
    default:
        output = _T("Nearest Neighbor");
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParseImageFormatCapabilities(HRFRasterFileCreator& rasterCreator, ImageFormat& imageFormat)
{
    const HFCPtr<HRFRasterFileCapabilities> capabilities = rasterCreator.GetCapabilities();
    if (!capabilities)
        return;
        
    // Scanline Orientation
    imageFormat.SLO.ID = 0;
    imageFormat.SLO.label = _T("");
    const HFCPtr<HRFScanlineOrientationCapability> sloCap = static_cast<HRFScanlineOrientationCapability*>
        (capabilities->GetCapabilityOfType(HRFScanlineOrientationCapability::CLASS_ID).GetPtr());
    if (sloCap)
    {
        HRFScanlineOrientation slo = sloCap->GetScanlineOrientation();
        imageFormat.SLO.ID = slo.m_ScanlineOrientation;
        imageFormat.SLO.label = WString(HUTClassIDDescriptor::GetInstance()->GetClassLabelSLO(slo).c_str(), BentleyCharEncoding::Utf8);
    }
    
    // Geocoding Capability
    imageFormat.geocodingSupport.isSupported = false;
    const HFCPtr<HRFGeocodingCapability> geoCap = static_cast<HRFGeocodingCapability*>
        (capabilities->GetCapabilityOfType(HRFGeocodingCapability::CLASS_ID).GetPtr());

    if (geoCap)
    {
        imageFormat.geocodingSupport.isSupported = true;
        SetAccessMode(imageFormat.geocodingSupport.accessMode, (HFCPtr<HRFCapability>&)geoCap);
    }
        
    
    // Histogram Capability
    imageFormat.histogramSupport.isSupported = false;
    const HFCPtr<HRFHistogramCapability> histoCap = static_cast<HRFHistogramCapability*>
        (capabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID).GetPtr());

    if (histoCap)
    {
        imageFormat.histogramSupport.isSupported = true;
        SetAccessMode(imageFormat.histogramSupport.accessMode, (HFCPtr<HRFCapability>&)histoCap);
    }
    
    // Interleave Type
    const HFCPtr<HRFInterleaveCapability> interleaveCap = static_cast<HRFInterleaveCapability*>
        (capabilities->GetCapabilityOfType(HRFInterleaveCapability::CLASS_ID).GetPtr());

    if (interleaveCap)
    {
        HRFInterleaveType interleaveType = interleaveCap->GetInterleaveType();
        imageFormat.interleaveType.ID = interleaveType.m_InterleaveType;
        switch (interleaveType.m_InterleaveType)
        {
        case HRFInterleaveType::PIXEL:
            imageFormat.interleaveType.label = _T("Pixel");
            break;
        case HRFInterleaveType::PLANE:
            imageFormat.interleaveType.label = _T("Plane");
            break;
        case HRFInterleaveType::LINE:
            imageFormat.interleaveType.label = _T("Line");
            break;
        }
        SetAccessMode(imageFormat.interleaveType.accessMode, (HFCPtr<HRFCapability>&)interleaveCap);
    }
    
    // Thumbnail support
    imageFormat.thumbnailSupport.isSupported = false;
    const HFCPtr<HRFThumbnailCapability> thumbCap = static_cast<HRFThumbnailCapability*>
        (capabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID).GetPtr());

    if (thumbCap)
    {
        imageFormat.thumbnailSupport.isSupported = true;
        SetAccessMode(imageFormat.thumbnailSupport.accessMode, (HFCPtr<HRFCapability>&)thumbCap);
    }

    // Multipage Support
    imageFormat.multiPageSupport.isSupported = false;
    const HFCPtr<HRFMultiPageCapability> multiPageCap = static_cast<HRFMultiPageCapability*>
        (capabilities->GetCapabilityOfType(HRFMultiPageCapability::CLASS_ID).GetPtr());

    if (multiPageCap)
    {
        imageFormat.multiPageSupport.isSupported = true;
        SetAccessMode(imageFormat.multiPageSupport.accessMode, (HFCPtr<HRFCapability>&)multiPageCap);
    }

    // Unlimited Resolution
    if (_tcsicmp(imageFormat.label.c_str(),_T("Web Map Server")) == 0)
        imageFormat.unlimitedResolution.isUnlimited = true;
    else
        imageFormat.unlimitedResolution.isUnlimited = false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParseTransfoModelCapabilities(HRFRasterFileCreator& rasterCreator, ImageFormat& imageFormat)
{
    // Transformation Model
    const HFCPtr<HRFRasterFileCapabilities> capabilities = 
        rasterCreator.GetCapabilities()->GetCapabilitiesOfType(HRFTransfoModelCapability::CLASS_ID);

    
    if (capabilities)
    {
        for (uint32_t index(0); index < capabilities->CountCapabilities(); ++index)
        {
            // Getting TransfoModel Capability
            const HFCPtr<HRFTransfoModelCapability> transfoCap = 
                (HFCPtr<HRFTransfoModelCapability>&) capabilities->GetCapability(index);
            HCLASS_ID classID = transfoCap->GetTransfoModelClassKey();

            
            // Initializing the TransfoModel object
            TransfoModel transfoModel;
            transfoModel.ID = classID;
            transfoModel.label = WString(HUTClassIDDescriptor::GetInstance()->GetClassLabelTransfoModel(classID).c_str(), BentleyCharEncoding::Utf8);
            SetAccessMode(transfoModel.accessMode, (HFCPtr<HRFCapability>&)transfoCap);
            
            // Adding the TransfoModel object to the list
            imageFormat.transfoModels.insert(TransfoModelMap::value_type(transfoModel.label, transfoModel));
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParsePixelTypeCapabilities(HRFRasterFileCreator& rasterCreator, ImageFormat& imageFormat)
{    
    // Pixel Type Capabilities
    const HFCPtr<HRFRasterFileCapabilities> capabilities =
        rasterCreator.GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID);

        
    for (uint32_t index(0); index < capabilities->CountCapabilities(); ++index)
    {
        // Getting the Pixel type label
        const HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = 
                (const HFCPtr<HRFPixelTypeCapability>& )(capabilities->GetCapability(index));
        HCLASS_ID pixelClassID = pCurrentPixelType->GetPixelTypeClassID();

        WString pixelTypeLabel = WString(HUTClassIDDescriptor::GetInstance()->GetClassLabelPixelType(pixelClassID).c_str(), BentleyCharEncoding::Utf8);
        
        // Label
        ColorSpaceObject colSpace;
        colSpace.label = pixelTypeLabel;
        colSpace.ID = pixelClassID;
        
        // Access Mode
        SetAccessMode(colSpace.accessMode, (HFCPtr<HRFCapability>&)pCurrentPixelType);
        
        // DownSampling Methods
        for (uint32_t i = 0; i < pCurrentPixelType->CountDownSamplingMethod(); ++i)
        {
            HRFDownSamplingMethod dsMethod = pCurrentPixelType->GetDownSamplingMethod(i);
            DownSamplingObject dsObj;
            
            dsObj.ID = dsMethod.m_DownSamplingMethod;
            ConvertDownSamplingMethodToString(dsMethod.m_DownSamplingMethod, dsObj.label);
            
            colSpace.downSamplingMethods.insert(DownSamplingMap::value_type(dsObj.label, dsObj));
        }
        
        // Supported codecs
        for (uint32_t codecIndex(0); codecIndex < pCurrentPixelType->CountCodecs(); ++codecIndex)
        {
            // Getting the codec label
            const HFCPtr<HRFCodecCapability> pCodecCapability = pCurrentPixelType->GetCodecCapabilityByIndex(codecIndex); 
            HCLASS_ID codecClassID = pCodecCapability->GetCodecClassID();

            WString codecLabel = WString(HUTClassIDDescriptor::GetInstance()->GetClassLabelCodec(codecClassID).c_str(), BentleyCharEncoding::Utf8);
            
            // Label
            CodecObject codecObject;
            codecObject.ID = codecClassID;
            codecObject.label = codecLabel;
            
            // Access Mode
            SetAccessMode(codecObject.accessMode, (HFCPtr<HRFCapability>&)pCodecCapability);

            // Block types
            for (uint32_t blockIndex(0); blockIndex < pCodecCapability->CountBlockType(); ++blockIndex)
                {
                    // Adding supported Block Types
                    BlockObject blockObject;
                    const HFCPtr<HRFBlockCapability> pBlockCapability = 
                            (const HFCPtr<HRFBlockCapability>& )(pCodecCapability->GetBlockTypeCapability(blockIndex));
                    
                    // Access Mode
                    SetAccessMode(blockObject.accessMode, (HFCPtr<HRFCapability>&)pBlockCapability);

                    ParseBlockTypeCapabilities(rasterCreator, pBlockCapability, blockObject);
                    
                    // Multi-Resolution support
                    const HFCPtr<HRFMultiResolutionCapability> mresCap = static_cast<HRFMultiResolutionCapability*>
                        (rasterCreator.GetCapabilities()->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID).GetPtr());

                    if (mresCap != NULL)
                    {
                        blockObject.isMultiResolution = true;
                        blockObject.isUnlimitedResolution = mresCap->IsUnlimitedResolution();
                    }
                    
                    //Fix for TR #332696 - Add filters to display the more generic block type for the formats using TIFF block capabilities
                    if (_wcsicmp(imageFormat.label.c_str(), L"Ingr. Tiff") == 0 ||
                        _wcsicmp(imageFormat.label.c_str(), L"SPOT Digital Image Map") == 0)
                    {
                        //Only display the more generic block type
                        if (blockObject.minimumHeight == 1)
                            codecObject.blockTypes.insert(BlockMap::value_type(blockObject.label, blockObject));
                    }
                    else if (_wcsicmp(imageFormat.label.c_str(), L"GEOTIFF") == 0 || 
                        _wcsicmp(imageFormat.label.c_str(), L"Tagged Image File Format") == 0)
                    {
                        //For the TIFF format, only display the less generic block type if it has Read, Write, Create access (all three) overall
                        if (blockObject.minimumHeight == 32)
                        {
                            if (blockObject.accessMode.size() == 3 &&
                                codecObject.accessMode.size() == 3 &&
                                colSpace.accessMode.size() == 3)
                            {
                                codecObject.blockTypes.insert(BlockMap::value_type(blockObject.label, blockObject));
                            }    
                        }
                        //Always display the more generic block type
                        else
                        {
                            codecObject.blockTypes.insert(BlockMap::value_type(blockObject.label, blockObject));
                        }
                    }
                    //No filters applied to the other file formats
                    else
                    {
                        codecObject.blockTypes.insert(BlockMap::value_type(blockObject.label, blockObject));
                    }
                }
            // Adding the Codec to the supported codecs list
            colSpace.supportedCompressions.insert(CodecMap::value_type(codecLabel, codecObject));
        }
        imageFormat.supportedColorSpace.insert(ColorSpaceMap::value_type(colSpace.label, colSpace)) ;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParseTagCapabilities(HRFRasterFileCreator& rasterCreator, ImageFormat& imageFormat)
{
    const HFCPtr<HRFRasterFileCapabilities> capabilities =
        rasterCreator.GetCapabilities()->GetCapabilitiesOfType(HRFTagCapability::CLASS_ID);

    
    if (capabilities)    
    {
        for (uint32_t index(0); index < capabilities->CountCapabilities(); ++index)
        {
            // Tag label
            const HFCPtr<HRFTagCapability> tagCapability = (const HFCPtr<HRFTagCapability>& )(capabilities->GetCapability(index));
            HFCPtr<HPMGenericAttribute> tagAttr = tagCapability->GetTag();
            
            TagObject tag;
            tag.tag = WString(tagAttr->GetName().c_str(), BentleyCharEncoding::Utf8);
                          
            // Access Modes
            SetAccessMode(tag.accessMode, (HFCPtr<HRFCapability>&)tagCapability);
                
            imageFormat.tags.insert(TagMap::value_type(tag.tag, tag));
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Jonathan.Bernier      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ParseGeocodingCapabilities(HRFRasterFileCreator& rasterCreator, ImageFormat& imageFormat)
{
    const HFCPtr<HRFRasterFileCapabilities> capabilities =

    rasterCreator.GetCapabilities()->GetCapabilitiesOfType(HRFGeocodingCapability::CLASS_ID);

    if (capabilities)    
    {
        StringList supportedGeoKeys;

        const HFCPtr<HRFGeocodingCapability> pGeocodingCapability = 
            (const HFCPtr<HRFGeocodingCapability>& )(capabilities->GetCapability(0));

        for (USHORT index(0); index < pGeocodingCapability->GetNbGeotiffKeys(); index++)
        {
            switch(pGeocodingCapability->GetGeotiffKey(index))
            {
            case 1024:
                supportedGeoKeys.push_back(_T("GTModelType"));
                break;
            case 1025:
                supportedGeoKeys.push_back(_T("GTRasterType"));
                break;
            case 1026:
                supportedGeoKeys.push_back(_T("GTCitation"));
                break;
            case 2048:
                supportedGeoKeys.push_back(_T("GeographicType"));
                break;
            case 2049:
                supportedGeoKeys.push_back(_T("GeogCitation"));
                break;
            case 2050:
                supportedGeoKeys.push_back(_T("GeogGeodeticDatum"));
                break;
            case 2051:
                supportedGeoKeys.push_back(_T("GeogPrimeMeridian"));
                break;
            case 2052:
                supportedGeoKeys.push_back(_T("GeogLinearUnits"));
                break;
            case 2053:
                supportedGeoKeys.push_back(_T("GeogLinearUnitSize"));
                break;
            case 2054:
                supportedGeoKeys.push_back(_T("GeogAngularUnits"));
                break;
            case 2055:
                supportedGeoKeys.push_back(_T("GeogAngularUnitSize"));
                break;
            case 2056:
                supportedGeoKeys.push_back(_T("GeogEllipsoid"));
                break;
            case 2057:
                supportedGeoKeys.push_back(_T("GeogSemiMajorAxis"));
                break;
            case 2058:
                supportedGeoKeys.push_back(_T("GeogSemiMinorAxis"));
                break;
            case 2059:
                supportedGeoKeys.push_back(_T("GeogInvFlattening"));
                break;
            case 2060:
                supportedGeoKeys.push_back(_T("GeogAzimuthUnits"));
                break;
            case 2061:
                supportedGeoKeys.push_back(_T("GeogPrimeMeridianLong"));
                break;
            case 3072:
                supportedGeoKeys.push_back(_T("ProjectedCSType"));
                break;
            case 3073:
                supportedGeoKeys.push_back(_T("PCSCitation"));
                break;
            case 3074:
                supportedGeoKeys.push_back(_T("Projection"));
                break;
            case 3075:
                supportedGeoKeys.push_back(_T("ProjCoordTrans"));
                break;
            case 3076:
                supportedGeoKeys.push_back(_T("ProjLinearUnits"));
                break;
            case 3077:
                supportedGeoKeys.push_back(_T("ProjLinearUnitSize"));
                break;
            case 3078:
                supportedGeoKeys.push_back(_T("ProjStdParallel1"));
                break;
            case 3079:
                supportedGeoKeys.push_back(_T("ProjStdParallel2"));
                break;
            case 3080:
                supportedGeoKeys.push_back(_T("ProjNatOriginLong"));
                break;
            case 3081:
                supportedGeoKeys.push_back(_T("ProjNatOriginLat"));
                break;
            case 3082:
                supportedGeoKeys.push_back(_T("ProjFalseEasting"));
                break;
            case 3083:
                supportedGeoKeys.push_back(_T("ProjFalseNorthing"));
                break;
            case 3084:
                supportedGeoKeys.push_back(_T("ProjFalseOriginLong"));
                break;
            case 3085:
                supportedGeoKeys.push_back(_T("ProjFalseOriginLat"));
                break;
            case 3086:
                supportedGeoKeys.push_back(_T("ProjFalseOriginEasting"));
                break;
            case 3087:
                supportedGeoKeys.push_back(_T("ProjFalseOriginNorthing"));
                break;
            case 3088:
                supportedGeoKeys.push_back(_T("ProjCenterLong"));
                break;
            case 3089:
                supportedGeoKeys.push_back(_T("ProjCenterLat"));
                break;
            case 3090:
                supportedGeoKeys.push_back(_T("ProjCenterEasting"));
                break;
            case 3091:
                supportedGeoKeys.push_back(_T("ProjCenterNorthing"));
                break;
            case 3092:
                supportedGeoKeys.push_back(_T("ProjScaleAtNatOrigin"));
                break;
            case 3093:
                supportedGeoKeys.push_back(_T("ProjScaleAtCenter"));
                break;
            case 3094:
                supportedGeoKeys.push_back(_T("ProjAzimuthAngle"));
                break;
            case 3095:
                supportedGeoKeys.push_back(_T("ProjStraightVertPoleLong"));
                break;
            case 3096:
                supportedGeoKeys.push_back(_T("ProjRectifiedGridAngle"));
                break;
            case 4096:
                supportedGeoKeys.push_back(_T("VerticalCSType"));
                break;
            case 4097:
                supportedGeoKeys.push_back(_T("VerticalCitation"));
                break;
            case 4098:
                supportedGeoKeys.push_back(_T("VerticalDatum"));
                break;
            case 4099:
                supportedGeoKeys.push_back(_T("VerticalUnits"));
                break;
            case 32767:
                supportedGeoKeys.push_back(_T("ReservedEndGeoKey"));
                break;
            case 32768:
                supportedGeoKeys.push_back(_T("PrivateBaseGeoKey"));
                break;
            case 60000:
                supportedGeoKeys.push_back(_T("ProjectedCSTypeLong"));
                break;
            case 65535:
                supportedGeoKeys.push_back(_T("PrivateEndGeoKey"));
                break;
            default:
                cout << "Error : Unknown Geokey " << pGeocodingCapability->GetGeotiffKey(index) << endl;
            }
        }

        imageFormat.geokeys = supportedGeoKeys;        
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatorParser::ScanFileCreator(HRFRasterFileCreator& rasterCreator)
{    
    ImageFormat imageFormat;
    imageFormat.label = WString(rasterCreator.GetLabel().c_str(), BentleyCharEncoding::Utf8);
    imageFormat.extensions = WString(rasterCreator.GetExtensions().c_str(), BentleyCharEncoding::Utf8);
    imageFormat.id = rasterCreator.GetRasterFileClassID();
    
    // Parse format capabilities
    ParseImageFormatCapabilities(rasterCreator, imageFormat);
    
    // Transformation Models
    ParseTransfoModelCapabilities(rasterCreator, imageFormat);
    
    // Pixel Types
    ParsePixelTypeCapabilities(rasterCreator, imageFormat);
    
    // Tags
    ParseTagCapabilities(rasterCreator, imageFormat);

    // Geocoding capabilities
    ParseGeocodingCapabilities(rasterCreator, imageFormat);
    
    m_fileFormats.insert(ImageFormatMap::value_type(imageFormat.label, imageFormat));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageFormatMap const& CreatorParser::GetImageFormats(void)
{
    return m_fileFormats;
}