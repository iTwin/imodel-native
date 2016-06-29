//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/HRARasterCopyFromTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"

#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include "NonLinearTransforms.h"

#define TEST_RASTER_WIDTH    531
#define TEST_RASTER_HEIGHT   542

static uint32_t s_defaultRGBAColor1 = 0xFF00FFFF;     // yellow
static uint32_t s_defaultRGBAColor2 = 0xFFFF0000;     // blue
static uint32_t s_defaultRGBAColorBlack = 0xFFFFFFFF;
static uint32_t s_defaultRGBAColorWhite = 0xFF000000;
static uint32_t s_filePrefixNumber = 0;

static HFCPtr<HGF2DWorldCluster> s_pHMRWorld(new HGFHMRStdWorldCluster());

struct RasterCreator;
typedef vector<RasterCreator*> VectorCreators;


/*=================================================================================**//**
* This creator class exist to delay the creation after the ::testing::Combine operation.
* Google test doesn't handle/report exception very well at that stage.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RasterCreator
    {
    static HFCPtr<HGF2DCoordSys> GetUnknownCS() {return s_pHMRWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);}

    virtual HFCPtr<HRARaster> Create(HPMPool& pool) const = 0;

    virtual ~RasterCreator(){};    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
struct BitmapCreator : RasterCreator
    {
    BitmapCreator(HFCPtr<HRPPixelType> pPixelType) {m_pPixelType = pPixelType;}

    virtual ~BitmapCreator(){};
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual HFCPtr<HRARaster> Create(HPMPool& pool) const override
        {
        HFCPtr<HRABitmap> pBitmap = HRABitmap::Create(TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, NULL, GetUnknownCS(), m_pPixelType, 32);
        return pBitmap.GetPtr(); 
        }

    HFCPtr<HRPPixelType> m_pPixelType;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
struct BitmapRleCreator : RasterCreator
    {
    BitmapRleCreator(bool withAlpha) {m_withAlpha = withAlpha;}

    virtual ~BitmapRleCreator(){};

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual HFCPtr<HRARaster> Create(HPMPool& pool) const override
        {
        HFCPtr<HRPPixelType> pPixelType;
        if(m_withAlpha)
            pPixelType = new HRPPixelTypeI1R8G8B8A8();
        else
            pPixelType = new HRPPixelTypeI1R8G8B8();

        HFCPtr<HRABitmapRLE> pBitmap = HRABitmapRLE::Create(TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, NULL, GetUnknownCS(), pPixelType);       

        return pBitmap.GetPtr(); 
        }

    bool m_withAlpha;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
struct cTiffPyramidCreator : RasterCreator
    {
    cTiffPyramidCreator(HFCPtr<HRPPixelType> pixelType, Utf8CP pFilePrefix) { m_pPixelType = pixelType; m_filePrefix = pFilePrefix; }

    virtual ~cTiffPyramidCreator(){};

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual HFCPtr<HRARaster> Create(HPMPool& pool) const override
        {
        HFCPtr<HRARaster> pRaster;
        try
            {
            HFCPtr<HRFRasterFile> pRasterFile = CreateBlankFile();
            if(pRasterFile == NULL)
                return NULL;

            HFCPtr<HGF2DCoordSys> pLogical = s_pHMRWorld->GetCoordSysReference(pRasterFile->GetWorldIdentificator());
            HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (&pool, pRasterFile, 0/*page*/, pLogical);
                      
            pRaster = pStore->LoadRaster();
            }
        catch (...)
            {
            pRaster = NULL;
            }

        return pRaster;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRFRasterFile> CreateBlankFile() const
        {
        HFCPtr<HRFRasterFile> pRasterFile;
        s_filePrefixNumber++;
        Utf8String filePrefixNumber;
        filePrefixNumber.Sprintf("%i", s_filePrefixNumber);
        try
            {
            
            BeFileName filename(m_filePrefix + filePrefixNumber + "cTiffPyramidCreator.ctiff");
            BeFileName outputFilePath;
            BeTest::GetHost().GetOutputRoot (outputFilePath);
            outputFilePath.AppendToPath(filename.c_str());
            HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8());

            pRasterFile = HRFcTiffCreator::GetInstance()->Create(pURL, HFC_READ_WRITE_CREATE); 

            HRFPageDescriptor::ListOfResolutionDescriptor  ResDescList;

            HAutoPtr<HGFResolutionDescriptor> pPyramidDesc(new HGFResolutionDescriptor(TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, 256, 256));   

            HRFBlockType blockType = m_pPixelType->CountPixelRawDataBits() == 1 ? HRFBlockType::STRIP : HRFBlockType::TILE;

            // Create the resolution list.
            for (uint16_t ResCount = 0 ; ResCount < pPyramidDesc->CountResolutions(); ++ResCount)
                {
                // Create a resolution descriptor.
                HFCPtr<HRFResolutionDescriptor> pResDesc = 
                    new HRFResolutionDescriptor(HFC_CREATE_ONLY,
                    pRasterFile->GetCapabilities(),
                    pPyramidDesc->GetResolution(ResCount),
                    pPyramidDesc->GetResolution(ResCount),
                    m_pPixelType,
                    new HCDCodecIdentity(),
                    HRFBlockAccess::RANDOM,
                    HRFBlockAccess::RANDOM,
                    HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,
                    HRFInterleaveType::PIXEL,
                    false,                                          // Interlace
                    pPyramidDesc->GetWidth(ResCount),
                    pPyramidDesc->GetHeight(ResCount),
                    blockType == HRFBlockType::STRIP ? pPyramidDesc->GetWidth(ResCount) : 256,   // TileSizeX
                    256,   // TileSizeY
                    0,
                    blockType,   
                    1,
                    8,  
                    HRFDownSamplingMethod::AVERAGE);   

                ResDescList.push_back(pResDesc);
                } 

            pRasterFile->AddPage(new HRFPageDescriptor (HFC_CREATE_ONLY, pRasterFile->GetCapabilities(), ResDescList, NULL, NULL, NULL, NULL, NULL, NULL));
            }
        catch (...)
            {
            pRasterFile = NULL;
            }

        return pRasterFile;
        }    

    HFCPtr<HRPPixelType> m_pPixelType;
    Utf8String m_filePrefix;
    };
                           
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HRARasterCopyFromBase
    {
protected:
    HRARasterCopyFromBase()
        :m_pool(1 /*use of a small pool to generate tile flush*/) // Memory pool shared by all rasters. (in KB)
        {
        };

    virtual ~HRARasterCopyFromBase()
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRARaster> AdaptPixelType(HFCPtr<HRARaster>& pRaster)
        {
        HFCPtr<HRPPixelType> pNewPixelType;
        if(pRaster->GetPixelType()->CountPixelRawDataBits() == 1)
            {
            if(pRaster->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
                pNewPixelType = new HRPPixelTypeI1R8G8B8();
            else
                pNewPixelType = new HRPPixelTypeI1R8G8B8A8();                
            }
        else if(pRaster->GetPixelType()->CountIndexBits() == 0)
            {
            return pRaster;   // nothing to adapt
            }
        else
            {
            pNewPixelType = (HRPPixelType*)pRaster->GetPixelType()->Clone();
            }      

        // Palette are assumed to always be RGB or RGBA.
        HRPPixelPalette& rPalette = pNewPixelType->LockPalette();
        rPalette.SetCompositeValue(0, &s_defaultRGBAColor1);
        rPalette.SetCompositeValue(1, &s_defaultRGBAColor2);
        pNewPixelType->UnlockPalette();

        return new HRAPixelTypeReplacer(pRaster, pNewPixelType);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRARaster> InitSink(HFCPtr<HRARaster>& pRaster)
        {
        return AdaptPixelType(pRaster);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/    
    void ClearRect(HRARaster& raster, void* pRawDataValue, double x1, double y1, double x2, double y2, HFCPtr<HGF2DCoordSys>& pPhysicalCS)
        {
        HVEShape rect(x1, y1, x2, y2, pPhysicalCS);

        HRAClearOptions clearOpts;
        clearOpts.SetLoadingData(true);
        clearOpts.SetRawDataValue(pRawDataValue);
        clearOpts.SetShape(&rect);
        raster.Clear(clearOpts);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRARaster> InitSource(HFCPtr<HRARaster> pRaster)
        {
        if(!pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
            return NULL;

        HFCPtr<HGF2DCoordSys> pPhysicalCS = static_cast<HRAStoredRaster*>(pRaster.GetPtr())->GetPhysicalCoordSys();
        
        HFCPtr<HRARaster> pSource = AdaptPixelType(pRaster);

        HFCPtr<HRPPixelType> pRGBA = new HRPPixelTypeV32R8G8B8A8();
        HFCPtr<HRPPixelConverter> pV32ToNative = pRGBA->GetConverterTo(pSource->GetPixelType());

        Byte defaultRawData[HRPPixelType::MAX_PIXEL_BYTES];

        HRAClearOptions clearOpts;
        clearOpts.SetLoadingData(true);
        clearOpts.SetRawDataValue(defaultRawData);
        pV32ToNative->Convert(&s_defaultRGBAColor1, defaultRawData, 1);

        // Clear the whole raster with color1
        pSource->Clear(clearOpts);

        // Setup to clear rectangle with color 2.
        pV32ToNative->Convert(&s_defaultRGBAColor2, defaultRawData, 1);
        clearOpts.SetRawDataValue(defaultRawData);

        ClearRect(*pSource, defaultRawData, 240, 0, 270, 40, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 240, 0, 270, 40, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 320, 1, 400, 255, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 513, 0, 530, 1, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 222, 222, 255, 255, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 222, 257, 255, 278, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 257, 222, 278, 255, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 257, 257, 278, 278, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 500, 251, 522, 267, pPhysicalCS);
        ClearRect(*pSource, defaultRawData, 0, TEST_RASTER_HEIGHT-2, 20, TEST_RASTER_HEIGHT-1, pPhysicalCS);

        return pSource;
        }

    virtual HFCPtr<HRARaster> GetSource() = 0;
    virtual HFCPtr<HRARaster> GetSink() = 0;
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRABitmap> ConvertToBitmap(HFCPtr<HRARaster>& pRaster)
        {
        if(pRaster->IsCompatibleWith(HRABitmap::CLASS_ID))
            return static_cast<HRABitmap*>(pRaster.GetPtr());
        
        HFCPtr<HRABitmap> pBitmap = HRABitmap::Create(TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, NULL, pRaster->GetCoordSys(), new HRPPixelTypeV32R8G8B8A8());
        HRACopyFromOptions opts(false/*alphaBlend*/);
        pBitmap->CopyFrom(*pRaster, opts);

        return pBitmap;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CompareResult(HFCPtr<HRARaster>& pRaster1, HFCPtr<HRARaster>& pRaster2)
        {
        HFCPtr<HRABitmap> pBitmap1 = ConvertToBitmap(pRaster1);
        HFCPtr<HRABitmap> pBitmap2 = ConvertToBitmap(pRaster2);            

        return CompareResult(*pBitmap1, *pBitmap2);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CompareResult(HRABitmap& bitmap1, HRABitmap& bitmap2)
        {
        HFCPtr<HRPPixelType> pRGBAPixelType = new HRPPixelTypeV32R8G8B8A8();
        HFCPtr<HRPPixelConverter> pConvertSrc1 = bitmap1.GetPixelType()->GetConverterTo(pRGBAPixelType);
        HFCPtr<HRPPixelConverter> pConvertSrc2 = bitmap2.GetPixelType()->GetConverterTo(pRGBAPixelType);

        uint64_t Width, Height;
        bitmap1.GetSize(&Width, &Height);

        uint64_t Width2, Height2;
        bitmap2.GetSize(&Width2, &Height2);
       
        if(Width != TEST_RASTER_WIDTH || Width2 != Width ||
           Height != TEST_RASTER_HEIGHT || Height2 != Height)
            {
            Utf8String result;
            result.Sprintf("Invalid bitmap length. Expected(%d,%d), Bitmap1(%d,%d), Bitmap2(%d,%d)", 
                TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, Width, Height, Width2, Height2);
            return result;
            }

        if(bitmap1.GetCodec() != NULL && !bitmap1.GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID) ||
           bitmap2.GetCodec() != NULL && !bitmap2.GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
            {
            return "Only HRABitmap identity codec is supported";
            }
           
        Byte* pBufferSrc1 = bitmap1.GetPacket()->GetBufferAddress();
        Byte* pBufferSrc2 = bitmap2.GetPacket()->GetBufferAddress();

        size_t bytesPerRowSrc1 = bitmap1.ComputeBytesPerWidth();
        size_t bytesPerRowSrc2 = bitmap2.ComputeBytesPerWidth();

        Byte pWorkline1[TEST_RASTER_WIDTH*4];  //RGBA
        Byte pWorkline2[TEST_RASTER_WIDTH*4];  //RGBA

        for(uint64_t Line=0; Line < Height; ++Line)
            {
            pConvertSrc1->Convert(pBufferSrc1 + bytesPerRowSrc1*Line, pWorkline1, static_cast<size_t>(Width));
            pConvertSrc2->Convert(pBufferSrc2 + bytesPerRowSrc2*Line, pWorkline2, static_cast<size_t>(Width));

            for(uint64_t Column=0; Column < Width; ++Column)
                {
                if(*(uint32_t*)(pWorkline1 + Column*3) != *(uint32_t*)(pWorkline2 + Column*3))
                    {
                    Utf8String result;
                    result.Sprintf("Difference at pixel(%d,%d).\nBitmap 1 RGBA(%d,%d,%d,%d)\nBitmap 2 RGBA(%d,%d,%d,%d)",
                                   Column, Line, pWorkline1[Column*3], pWorkline1[Column*3 +1], pWorkline1[Column*3 +2], pWorkline1[Column*3 +3],
                                   pWorkline2[Column*3], pWorkline2[Column*3 +1], pWorkline2[Column*3 +2], pWorkline2[Column*3 +3]);
                    return result;
                    }
                }
            }

        return "";
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   
    +---------------+---------------+---------------+---------------+---------------+------*/
    HVEShape GetPacmanShape(HFCPtr<HGF2DCoordSys>& pPhysicalCS)
        {
        HGF2DPolySegment* pacman = new HGF2DPolySegment();
        pacman->AppendPoint(HGF2DPosition(100.25, 160.0));
        pacman->AppendPoint(HGF2DPosition(110.0, 160.0));
        pacman->AppendPoint(HGF2DPosition(110.0, 150.0));
        pacman->AppendPoint(HGF2DPosition(120.0, 150.0));
        pacman->AppendPoint(HGF2DPosition(120.0, 140.0));
        pacman->AppendPoint(HGF2DPosition(130.0, 140.0));
        pacman->AppendPoint(HGF2DPosition(130.0, 130.0));
        pacman->AppendPoint(HGF2DPosition(140.0, 130.0));
        pacman->AppendPoint(HGF2DPosition(140.0, 120.0));
        pacman->AppendPoint(HGF2DPosition(150.0, 120.0));
        pacman->AppendPoint(HGF2DPosition(150.0, 110.0));
        pacman->AppendPoint(HGF2DPosition(160.0, 110.0));
        pacman->AppendPoint(HGF2DPosition(160.0, 100.0));
        pacman->AppendPoint(HGF2DPosition(200.0, 100.0));
        pacman->AppendPoint(HGF2DPosition(200.0, 110.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 110.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 120.0));
        pacman->AppendPoint(HGF2DPosition(220.0, 120.0));
        pacman->AppendPoint(HGF2DPosition(220.0, 130.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 130.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 140.0));
        pacman->AppendPoint(HGF2DPosition(240.0, 140.0));
        pacman->AppendPoint(HGF2DPosition(240.0, 150.0));
        pacman->AppendPoint(HGF2DPosition(250.0, 150.0));
        pacman->AppendPoint(HGF2DPosition(250.0, 160.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 160.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 170.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 170.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 180.0));
        pacman->AppendPoint(HGF2DPosition(190.0, 180.0));
        pacman->AppendPoint(HGF2DPosition(190.0, 190.0));
        pacman->AppendPoint(HGF2DPosition(170.0, 190.0));
        pacman->AppendPoint(HGF2DPosition(170.0, 200.0));
        pacman->AppendPoint(HGF2DPosition(190.0, 200.0));
        pacman->AppendPoint(HGF2DPosition(190.0, 210.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 210.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 220.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 220.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 230.0));
        pacman->AppendPoint(HGF2DPosition(250.0, 230.0));
        pacman->AppendPoint(HGF2DPosition(250.0, 240.0));
        pacman->AppendPoint(HGF2DPosition(240.0, 240.0));
        pacman->AppendPoint(HGF2DPosition(240.0, 250.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 250.0));
        pacman->AppendPoint(HGF2DPosition(230.0, 260.0));
        pacman->AppendPoint(HGF2DPosition(220.0, 260.0));
        pacman->AppendPoint(HGF2DPosition(220.0, 270.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 270.0));
        pacman->AppendPoint(HGF2DPosition(210.0, 280.0));
        pacman->AppendPoint(HGF2DPosition(160.0, 280.0));
        pacman->AppendPoint(HGF2DPosition(160.0, 270.0));
        pacman->AppendPoint(HGF2DPosition(150.0, 270.0));
        pacman->AppendPoint(HGF2DPosition(150.0, 260.0));
        pacman->AppendPoint(HGF2DPosition(140.0, 260.0));
        pacman->AppendPoint(HGF2DPosition(140.0, 250.0));
        pacman->AppendPoint(HGF2DPosition(130.0, 250.0));
        pacman->AppendPoint(HGF2DPosition(130.0, 240.0));
        pacman->AppendPoint(HGF2DPosition(120.0, 240.0));
        pacman->AppendPoint(HGF2DPosition(120.0, 230.0));
        pacman->AppendPoint(HGF2DPosition(110.0, 230.0));
        pacman->AppendPoint(HGF2DPosition(110.0, 220.0));
        pacman->AppendPoint(HGF2DPosition(100.25, 220.0));
        pacman->AppendPoint(HGF2DPosition(100.25, 210.0));
        pacman->AppendPoint(HGF2DPosition(100.25, 200.0));
        pacman->AppendPoint(HGF2DPosition(100.25, 160.0));
        HVEShape pacmanShape = HVEShape(HGF2DPolygonOfSegments(*pacman), pPhysicalCS);

        HGF2DPolySegment* square = new HGF2DPolySegment();
        square->AppendPoint(HGF2DPosition(239.25, 180.25));
        square->AppendPoint(HGF2DPosition(269.75, 180.25));
        square->AppendPoint(HGF2DPosition(269.75, 210.75));
        square->AppendPoint(HGF2DPosition(239.25, 210.75));
        square->AppendPoint(HGF2DPosition(239.25, 180.25));
        HVEShape squareShape = HVEShape(HGF2DPolygonOfSegments(*square), pPhysicalCS);

        HGF2DPolySegment* square2 = new HGF2DPolySegment();
        square2->AppendPoint(HGF2DPosition(290.25, 180.25));
        square2->AppendPoint(HGF2DPosition(320.75, 180.25));
        square2->AppendPoint(HGF2DPosition(320.75, 210.75));
        square2->AppendPoint(HGF2DPosition(290.25, 210.75));
        square2->AppendPoint(HGF2DPosition(290.25, 180.25));
        HVEShape squareShape2 = HVEShape(HGF2DPolygonOfSegments(*square2), pPhysicalCS);

        HGF2DPolySegment* square3 = new HGF2DPolySegment();
        square3->AppendPoint(HGF2DPosition(350.25, 180.25));
        square3->AppendPoint(HGF2DPosition(520.75, 180.25));
        square3->AppendPoint(HGF2DPosition(520.75, 210.75));
        square3->AppendPoint(HGF2DPosition(350.25, 210.75));
        square3->AppendPoint(HGF2DPosition(350.25, 180.25));
        HVEShape squareShape3 = HVEShape(HGF2DPolygonOfSegments(*square3), pPhysicalCS);

        HGF2DPolySegment* square4 = new HGF2DPolySegment();
        square4->AppendPoint(HGF2DPosition(240.25, 480.25));
        square4->AppendPoint(HGF2DPosition(320.75, 480.25));
        square4->AppendPoint(HGF2DPosition(320.75, 510.75));
        square4->AppendPoint(HGF2DPosition(240.25, 510.75));
        square4->AppendPoint(HGF2DPosition(240.25, 480.25));
        HVEShape squareShape4 = HVEShape(HGF2DPolygonOfSegments(*square4), pPhysicalCS);

        HGF2DPolySegment* squareEyes = new HGF2DPolySegment();
        squareEyes->AppendPoint(HGF2DPosition(170.0, 140.0));
        squareEyes->AppendPoint(HGF2DPosition(190.0, 140.0));
        squareEyes->AppendPoint(HGF2DPosition(190.0, 160.0));
        squareEyes->AppendPoint(HGF2DPosition(170.0, 160.0));
        squareEyes->AppendPoint(HGF2DPosition(170.0, 140.0));
        HVEShape squareShapeEyes = HVEShape(HGF2DPolygonOfSegments(*squareEyes), pPhysicalCS);

        pacmanShape.Unify(squareShape);
        pacmanShape.Unify(squareShape2);
        pacmanShape.Unify(squareShape3);
        pacmanShape.Unify(squareShape4);
        pacmanShape.Differentiate(squareShapeEyes);

        return pacmanShape;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRARaster>& GetBase()
        {
        if(m_pBase == NULL)
            m_pBase = InitSource(HRABitmap::Create(TEST_RASTER_WIDTH, TEST_RASTER_HEIGHT, NULL, RasterCreator::GetUnknownCS(), new HRPPixelTypeV32R8G8B8A8()).GetPtr());
            
        return m_pBase;
        }

    HFCPtr<HRARaster> m_pBase;
    HPMPool m_pool;
    };


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HRARasterCopyFromTester : public HRARasterCopyFromBase,
                                public ::testing::TestWithParam< ::std::tr1::tuple<RasterCreator*, RasterCreator*> >
    {
    protected:
    HFCPtr<HRARaster> GetSource() { return ::std::tr1::get<0>(GetParam())->Create(m_pool);}
    HFCPtr<HRARaster> GetSink()   { return ::std::tr1::get<1>(GetParam())->Create(m_pool);}

    virtual void SetUp() override {ImagePPTestConfig::GetConfig().SetUp();}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HRARasterCopyFromTester2 : public HRARasterCopyFromBase,
                                 public ::testing::TestWithParam<RasterCreator*>
    {
    protected:
    HFCPtr<HRARaster> GetSource() { return GetParam()->Create(m_pool);}
    HFCPtr<HRARaster> GetSink()   { return GetParam()->Create(m_pool);} //Not used, but has to be defined

    virtual void SetUp() override { ImagePPTestConfig::GetConfig().SetUp(); }
    };

/*=================================================================================**//**
*           This fixture is only unsed to deal with the initialization of ImagePPLib
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
class HRARasterCopyFromTesterNonLinear : public HRARasterCopyFromBase,
                                         public ::testing::Test
    { 
    protected:
    HFCPtr<HRARaster> GetSource() { return NULL;} //Not used, but has to be defined
    HFCPtr<HRARaster> GetSink()   { return NULL;} //Not used, but has to be defined

    virtual void SetUp() override { ImagePPTestConfig::GetConfig().SetUp(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRARasterCopyFromTester, CopyFromTests)
{
    HFCPtr<HRARaster> pSourceRaster = GetSource();
    ASSERT_TRUE(pSourceRaster != NULL);
    
    pSourceRaster = InitSource(pSourceRaster);
    ASSERT_TRUE(pSourceRaster != NULL);

    bool sourceHasAlpha = pSourceRaster->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;
     
    HFCPtr<HRARaster> pSinkRaster = GetSink();
    ASSERT_TRUE(pSinkRaster != NULL);

    pSinkRaster = InitSink(pSinkRaster);
    ASSERT_TRUE(pSinkRaster != NULL);


    //************* Default CopyFrom and HRAPixelTypeReplacer Tests *******************
    pSinkRaster->Clear();
    
    HRACopyFromOptions copyFromOpts;
    copyFromOpts.SetAlphaBlend(false);
    
    ASSERT_EQ(IMAGEPP_STATUS_Success, pSinkRaster->CopyFrom(*pSourceRaster, copyFromOpts));
    //pSinkRaster->CopyFrom(pSourceRaster, HRACopyFromOptions(false));
    ASSERT_STREQ ("", CompareResult(GetBase(), pSinkRaster).c_str());
    
    // If source has alpha repeat withs alpha blend
    if(sourceHasAlpha)
        {
        pSinkRaster->Clear();

        copyFromOpts.SetAlphaBlend(true);

        ASSERT_EQ(IMAGEPP_STATUS_Success, pSinkRaster->CopyFrom(*pSourceRaster, copyFromOpts));
        ASSERT_STREQ ("", CompareResult(GetBase(), pSinkRaster).c_str());
        }

    //************* HIMMosaic and HRAReferenceToRaster Tests *******************
        {

        HGF2DDisplacement translation(8889.5, -55914.0);
        HFCPtr<HRARaster> pMovedRefToSource = new HRAReferenceToRaster(pSourceRaster);
        pMovedRefToSource->Move(translation);

        ASSERT_EQ(COPYFROM_STATUS_VoidRegion, pSinkRaster->CopyFrom(*pMovedRefToSource, copyFromOpts));
        
        HFCPtr<HRARaster> pMovedRefToSink = new HRAReferenceToRaster(pSinkRaster);
        pMovedRefToSink->Move(translation);
        ASSERT_EQ(IMAGEPP_STATUS_Success, pMovedRefToSink->CopyFrom(*pMovedRefToSource, copyFromOpts));
        //pMovedRefToSink->CopyFrom(pMovedRefToSource, HRACopyFromOptions(false));
        ASSERT_STREQ("", CompareResult(GetBase(), pMovedRefToSink).c_str());

        HFCPtr<HIMMosaic> pMosaic = new HIMMosaic(s_pHMRWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
        pMosaic->Add(pMovedRefToSource);
        
        ASSERT_EQ(COPYFROM_STATUS_VoidRegion, pSinkRaster->CopyFrom(*pMosaic, copyFromOpts));
        ASSERT_EQ(IMAGEPP_STATUS_Success, pMovedRefToSink->CopyFrom(*pMosaic, copyFromOpts));
        ASSERT_STREQ("", CompareResult(GetBase(), pMovedRefToSink).c_str());
                       
        pMovedRefToSink->Clear();
        copyFromOpts.SetAlphaBlend(false);

        ASSERT_EQ(IMAGEPP_STATUS_Success, pMovedRefToSink->CopyFrom(*pMosaic, copyFromOpts));
        ASSERT_STREQ ("", CompareResult(GetBase(), pMovedRefToSink).c_str());

        // If source has alpha repeat withs alpha blend
        if(sourceHasAlpha)
            {
            pMovedRefToSink->Clear();
            copyFromOpts.SetAlphaBlend(true);

            ASSERT_EQ(IMAGEPP_STATUS_Success, pMovedRefToSink->CopyFrom(*pMosaic, copyFromOpts));
            ASSERT_STREQ ("", CompareResult(GetBase(), pMovedRefToSink).c_str());
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRARasterCopyFromTester2, ClearVSCopyFromTester)
{
    
    HFCPtr<HRARaster> pRasterColor1  = GetSource();
    HFCPtr<HRARaster> pRaster1Color2 = GetSource(); // We need 2 output rasters of same color
    HFCPtr<HRARaster> pRaster2Color2 = GetSource();
    HFCPtr<HGF2DCoordSys> pPhysicalCS = static_cast<HRAStoredRaster*>(pRasterColor1.GetPtr())->GetPhysicalCoordSys();

    pRasterColor1 = InitSink(pRasterColor1);
    pRaster1Color2 = InitSink(pRaster1Color2);
    pRaster2Color2 = InitSink(pRaster2Color2);


    HFCPtr<HRPPixelType>  pPixelType = pRasterColor1->GetPixelType();

    HFCPtr<HRPPixelType> pRGBA = new HRPPixelTypeV32R8G8B8A8();
    HFCPtr<HRPPixelConverter> pV32ToNative = pRGBA->GetConverterTo(pPixelType);

    //Clear and background color values
    Byte defaultRawData1[HRPPixelType::MAX_PIXEL_BYTES];
    Byte defaultRawData2[HRPPixelType::MAX_PIXEL_BYTES];
    HRAClearOptions clearOptsColor1, clearOptsColor2;

    pV32ToNative->Convert(&s_defaultRGBAColor1, defaultRawData1, 1);
    clearOptsColor1.SetRawDataValue(defaultRawData1);
    clearOptsColor1.SetLoadingData(true); // MANDATORY for Pyramid Rasters (refer to HRATiledRaster::Clear)
    
    pV32ToNative->Convert(&s_defaultRGBAColor2, defaultRawData2, 1);
    clearOptsColor2.SetRawDataValue(defaultRawData2);
    clearOptsColor2.SetLoadingData(true);

    pRasterColor1->Clear(clearOptsColor1);
    pRaster1Color2->Clear(clearOptsColor2);
    pRaster2Color2->Clear(clearOptsColor2);
    
    //Create shape
    HVEShape shape = GetPacmanShape(pPhysicalCS);
    
    // Take raster1color2 and copyFrom raster color 1
    HRACopyFromOptions copyOptsWithShape(false/*alphaBlend*/);
    copyOptsWithShape.SetDestShape(&shape);
    pRaster1Color2->CopyFrom(*pRasterColor1, copyOptsWithShape);
    
    // Take raster2color2 and clears it with shape of color 1
    HRAClearOptions clearOptsWithShape;
    clearOptsWithShape.SetShape(&shape);
    clearOptsWithShape.SetRawDataValue(defaultRawData1);
    clearOptsWithShape.SetLoadingData(true); // MANDATORY for Pyramid Rasters (refer to HRATiledRaster::Clear)
    pRaster2Color2->Clear(clearOptsWithShape);
    
    //Comparison
    HFCPtr<HRABitmap> pOutCopyFrom  = ConvertToBitmap(pRaster1Color2);
    HFCPtr<HRABitmap> pOutClear = ConvertToBitmap(pRaster2Color2);
    ASSERT_STREQ("", CompareResult(*pOutCopyFrom, *pOutClear).c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static VectorCreators& s_SourceCreator()
    {
    static VectorCreators s_sourceVector;
    if (s_sourceVector.empty())
        {
        s_sourceVector.push_back(new BitmapCreator(new HRPPixelTypeV32R8G8B8A8()));
        s_sourceVector.push_back(new BitmapCreator(new HRPPixelTypeV24B8G8R8()));
        s_sourceVector.push_back(new BitmapCreator(new HRPPixelTypeI8R8G8B8()));
        s_sourceVector.push_back(new BitmapCreator(new HRPPixelTypeV1Gray1()));
        s_sourceVector.push_back(new BitmapCreator(new HRPPixelTypeI1R8G8B8A8()));
        s_sourceVector.push_back(new BitmapRleCreator(false/*withAlpha*/));
        s_sourceVector.push_back(new BitmapRleCreator(true/*withAlpha*/));
        s_sourceVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeV32R8G8B8A8(), "srcV32"));
        s_sourceVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeV24B8G8R8(), "srcV24"));
        s_sourceVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI8R8G8B8(), "srcI8"));
        s_sourceVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI1R8G8B8(), "srcI1"));
        s_sourceVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI1R8G8B8A8(), "srcI1A8"));
        // + OnDemandMosaic.
        }

    return s_sourceVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static VectorCreators& s_SinkCreator()
    {
    static VectorCreators s_sinkVector;
    if (s_sinkVector.empty())
        {
        s_sinkVector.push_back(new BitmapCreator(new HRPPixelTypeV32R8G8B8A8()));
        s_sinkVector.push_back(new BitmapCreator(new HRPPixelTypeV24B8G8R8()));
        s_sinkVector.push_back(new BitmapCreator(new HRPPixelTypeI8R8G8B8()));
        s_sinkVector.push_back(new BitmapCreator(new HRPPixelTypeV1Gray1()));
        s_sinkVector.push_back(new BitmapCreator(new HRPPixelTypeI1R8G8B8A8()));
        s_sinkVector.push_back(new BitmapRleCreator(false/*withAlpha*/));
        s_sinkVector.push_back(new BitmapRleCreator(true/*withAlpha*/));
        s_sinkVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeV32R8G8B8A8(), "sinkV32"));
        s_sinkVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeV24B8G8R8(), "sinkV24"));
        s_sinkVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI8R8G8B8(), "sinkI8"));
        s_sinkVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI1R8G8B8(), "sinkI1"));
        s_sinkVector.push_back(new cTiffPyramidCreator(new HRPPixelTypeI1R8G8B8A8(), "sinkI1A8"));
        }

    return s_sinkVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(HRARasterCopyFromTests, HRARasterCopyFromTester,
    ::testing::Combine(::testing::ValuesIn(s_SourceCreator()), ::testing::ValuesIn(s_SinkCreator())));

INSTANTIATE_TEST_CASE_P(HRARasterClearCopyFromTests, HRARasterCopyFromTester2, ::testing::ValuesIn(s_SourceCreator()));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gariepy                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HRARasterCopyFromTesterNonLinear, TestAllTilesAreCopied)
    {
    // Create binary bitmap filled with black. The size of the bitmap is chosen so that 
    // ParabolaTransfoModel creates a 256x256 empty tile in the middle of the strip
    int32_t IMAGE_WIDTH = 2000;
    HFCPtr<HGF2DCoordSys> pCoordSys = s_pHMRWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);
    HFCPtr<HRABitmap> pSourceBitmap = HRABitmap::Create(IMAGE_WIDTH, 300, NULL, pCoordSys, new HRPPixelTypeV8Gray8(), 32).GetPtr();

    HFCPtr<HRPPixelType> pRGBA = new HRPPixelTypeV32R8G8B8A8();
    HFCPtr<HRPPixelConverter> pV32ToNative = pRGBA->GetConverterTo(pSourceBitmap->GetPixelType());

    Byte defaultRawData[HRPPixelType::MAX_PIXEL_BYTES];

    HRAClearOptions clearOptsSource;
    clearOptsSource.SetLoadingData(true);
    clearOptsSource.SetRawDataValue(defaultRawData);
    pV32ToNative->Convert(&s_defaultRGBAColorBlack, defaultRawData, 1);
    pSourceBitmap->Clear(clearOptsSource);

    // Create a transformed coordinate system
    HFCPtr<HGF2DTransfoModel> pNonLinearTransfo = new ParabolaTransfoModel();
    HFCPtr<HGF2DCoordSys> pTransformedCoordSys = new HGF2DCoordSys(*pNonLinearTransfo, pCoordSys);

    // Create a destination bitmap
    HFCPtr<HRABitmap> pDestinationBitmap = HRABitmap::Create(IMAGE_WIDTH, 600, NULL, pTransformedCoordSys, new HRPPixelTypeV8Gray8(), 32).GetPtr();

    HRAClearOptions clearOptsDest;
    clearOptsDest.SetLoadingData(true);
    clearOptsDest.SetRawDataValue(defaultRawData);
    pV32ToNative->Convert(&s_defaultRGBAColorWhite, defaultRawData, 1);
    pDestinationBitmap->Clear(clearOptsDest);

    HRACopyFromOptions opts(false/*alphaBlend*/);
    pDestinationBitmap->CopyFrom(*pSourceBitmap, opts);

    // Verify that all tiles are all copied by checking some pixels in the destination image
    Byte* pBuffer = pDestinationBitmap->GetPacket()->GetBufferAddress();
    // Check some white pixels
    //                      x      y
    ASSERT_EQ(0x00, pBuffer[230  + 10 * IMAGE_WIDTH]);
    ASSERT_EQ(0x00, pBuffer[190  + 420 * IMAGE_WIDTH]);
    ASSERT_EQ(0x00, pBuffer[1000 + 102 * IMAGE_WIDTH]);
    ASSERT_EQ(0x00, pBuffer[1740 + 10 * IMAGE_WIDTH]);
    // Check some white pixels
    //                      x      y
    ASSERT_EQ(0xFF, pBuffer[1990 + 460 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[1830 + 250 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[1618 + 247 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[1480 + 41 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[1132 + 41 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[448  + 41 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[302  + 235 * IMAGE_WIDTH]);
    ASSERT_EQ(0xFF, pBuffer[40   + 400 * IMAGE_WIDTH]);
    }

#else

#pragma message("Warining: Disabling HRARasterCopyFromTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

#endif