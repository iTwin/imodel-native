//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMapBoxFile.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFMapBoxFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFMapboxFile.h>
#include <ImagePP/all/h/HRFMapboxTileEditor.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecJPEG.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

#include <ImagePP/all/h/HGF2DStretch.h>

#include <ImagePP/all/h/HRPPixelTypeFactory.h>
#include <ImagePP/all/h/HRFUtility.h>

#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/ImagePPMessages.xliff.h>
#include <ImagePP/all/h/HFCURLHTTP.h>

#include <ImagePPInternal/gra/Task.h>
#include <ImagePPInternal/HttpConnection.h>


#define MB_MAP_RESOLUTION       19
#define MB_MAP_WIDTH            (256 * (1 << MB_MAP_RESOLUTION))
#define MB_MAP_HEIGHT           (256 * (1 << MB_MAP_RESOLUTION))

#define NB_BLOCK_READER_THREAD 20

//-----------------------------------------------------------------------------//
//                         Extern - MapBoxTileSystem API                 //
// This is an extern API that has been put here instead of in the extern       //
// library because it was more practical.                                      //
//-----------------------------------------------------------------------------//
class MapBoxTileSystem
        {
    private :
        static const double EarthRadius;
        static const double MinLatitude;
        static const double MaxLatitude;        

    public :
        static double  Clip(double n, double minValue, double maxValue);
        static unsigned int MapSize(int levelOfDetail);
        static double  GroundResolution(double latitude, int levelOfDetail);       
        };

const double MapBoxTileSystem::EarthRadius = 6378137;
const double MapBoxTileSystem::MinLatitude = -85.05112878;
const double MapBoxTileSystem::MaxLatitude = 85.05112878;

double MapBoxTileSystem::Clip(double n, double minValue, double maxValue)
    {
    return MIN(MAX(n, minValue), maxValue);
    }

unsigned int MapBoxTileSystem::MapSize(int levelOfDetail)
    {
    return (unsigned int) 256 << levelOfDetail;
    }

double MapBoxTileSystem::GroundResolution(double latitude, int levelOfDetail)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    return cos(latitude * PI / 180) * 2 * PI * EarthRadius /
           MapSize(levelOfDetail);
    }

//-----------------------------------------------------------------------------
// HRFMapBoxBlockCapabilities
//-----------------------------------------------------------------------------
class HRFMapBoxBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMapBoxBlockCapabilities()
        : HRFRasterFileCapabilities()
        {                
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY, // AccessMode
                                  UINT32_MAX,           // MaxSizeInBytes
                                  256,                  // MinWidth
                                  256,                  // MaxWidth
                                  0,                    // WidthIncrement
                                  256,                  // MinHeight
                                  256,                  // MaxHeight
                                  0,                    // HeightIncrement
                                  false));              // Not Square

        }
    };

//-----------------------------------------------------------------------------
// HRFMapBoxBlockCodecTrueColorCapabilities
//-----------------------------------------------------------------------------
class HRFMapBoxBlockCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMapBoxBlockCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFMapBoxBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFMapBoxBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFMapBoxCapabilities
//-----------------------------------------------------------------------------
HRFMapBoxCapabilities::HRFMapBoxCapabilities()
    : HRFRasterFileCapabilities()
    {
            
    // PixelTypeV24B8G8R8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFMapBoxBlockCodecTrueColorCapabilities()));
    
    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single Resolution Capability    
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_ONLY, // AccessMode,
        true,                  // SinglePixelType,
        true,                  // SingleBlockType,
        false,                 // ArbitaryXRatio,
        false);                // ArbitaryYRatio);
    Add(pMultiResolutionCapability);
    
    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Tag capability
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability;

    pGeocodingCapability = new HRFGeocodingCapability(HFC_READ_ONLY);

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogEllipsoid);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogInvFlattening);
    pGeocodingCapability->AddSupportedKey(GeogAzimuthUnits);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridianLong);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel1);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel2);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseNorthing);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginNorthing);
    pGeocodingCapability->AddSupportedKey(ProjCenterLong);
    pGeocodingCapability->AddSupportedKey(ProjCenterLat);
    pGeocodingCapability->AddSupportedKey(ProjCenterEasting);
    pGeocodingCapability->AddSupportedKey(ProjCenterNorthing);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtNatOrigin);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtCenter);
    pGeocodingCapability->AddSupportedKey(ProjAzimuthAngle);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    }

HFC_IMPLEMENT_SINGLETON(HRFMapBoxCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFMapBoxCreator)
// This is the creator to instantiate MapBox format
//-----------------------------------------------------------------------------
HRFMapBoxCreator::HRFMapBoxCreator()
    : HRFRasterFileCreator(HRFMapBoxFile::CLASS_ID)
    {
    // MapBox capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFMapBoxCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFMapBoxCreator::GetLabel() const
    {    
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_MapBox());  // Tagged Image File Format (TIFF)
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFMapBoxCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFMapBoxCreator::GetSchemes() const
    {
    return Utf8String(HFCURLHTTP::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFMapBoxCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFMapBoxCreator::GetExtensions() const
    {
    return Utf8String(L"");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFMapBoxCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFMapBoxCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFMapBoxFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// Is a MapBox URL
//-----------------------------------------------------------------------------
bool IsMapBoxURL(HFCURL const& mapBoxURL)
    {    
    if(!mapBoxURL.IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) || 
        mapBoxURL.GetSchemeType() != HFCURLHTTP::s_SchemeName())
        return false;

    return true;    
    }

//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFMapBoxCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFMapBoxCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    return IsMapBoxURL(*pi_rpURL);
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFMapBoxCreator)
// Create or get the singleton capabilities of MapBox file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFMapBoxCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFMapBoxCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFMapBoxFile::HRFMapBoxFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen                = false;
        
    // Create Page and Res Descriptors.
    Open();
    CreateDescriptors();    
    }


//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy MapBox file object
//-----------------------------------------------------------------------------
HRFMapBoxFile::~HRFMapBoxFile()
    {    
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFMapBoxFile::GetWorldIdentificator () const
    {
    //return HGF2DWorld_HMRWORLD;
    return HGF2DWorld_GEOTIFFUNKNOWN;    
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFMapBoxFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                           unsigned short pi_Resolution,
                                                           HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFMapBoxTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFMapBoxFile::GetFileCurrentSize() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFMapBoxFile::GetCapabilities () const
    {
    return (HRFMapBoxCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFMapBoxFile::Open()
    {  
    m_IsOpen = true;

    return true;
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create MapBox File Descriptors
//-----------------------------------------------------------------------------

void HRFMapBoxFile::CreateDescriptors ()
    {   
    HPRECONDITION (IsMapBoxURL(*GetURL()));
   
    // Pixel Type
    HFCPtr<HRPPixelType> PixelType = new HRPPixelTypeV24R8G8B8();

    // Transfo model
    double Scale = MapBoxTileSystem::GroundResolution(0.0, MB_MAP_RESOLUTION);
    
    // Geocoding and Reference
    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    GeoCoordinates::BaseGCSPtr pBaseGCS;
             
     // Obtain the GCS
     pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();
         
    if (SUCCESS == pBaseGCS->InitFromEPSGCode (NULL, NULL, 900913))
         {                
         GeoPoint geoPoint = {-179.999999999, 85.05112878, 0.0};         
         DPoint3d cartesianPoint;
         pBaseGCS->CartesianFromLatLong (cartesianPoint, geoPoint);
         pTransfoModel = new HGF2DStretch(HGF2DDisplacement(cartesianPoint.x, cartesianPoint.y), Scale, Scale);                  
         }
	             
    if(pTransfoModel == nullptr)
        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), Scale, Scale);
        
    // Flip the Y Axe because the origin of ModelSpace is lower-left    
    HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();
    pFlipModel->SetYScaling(-1.0);
    pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);
   
    // Instantiation of Resolution descriptor
    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;

    // We used to had problems when going larger than INT_MAX. Need to test that if MB_MAP_WIDTH exceed that.
    HASSERT(MB_MAP_WIDTH <= INT_MAX && MB_MAP_HEIGHT <= INT_MAX);

    uint32_t Width = MB_MAP_WIDTH;
    uint32_t Height= MB_MAP_HEIGHT;
    
    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        HFC_READ_ONLY,                                      // AccessMode,
        GetCapabilities(),                                  // Capabilities,
        1,                                                  // XResolutionRatio,
        1,                                                   // YResolutionRatio,
        PixelType,                                          // PixelType,
        new HCDCodecIdentity(),                             // Codecs,
        HRFBlockAccess::RANDOM,                             // RBlockAccess,
        HRFBlockAccess::RANDOM,                             // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                           // InterleaveType
        false,                                              // IsInterlace,
        Width,                                              // Width,
        Height,                                             // Height,
        256,                                                // BlockWidth,
        256,                                                // BlockHeight,
        0,                                                  // BlocksDataFlag
        HRFBlockType::TILE);                                // BlockType

    ListOfResolutionDescriptor.push_back(pResolution);

    for (unsigned short Resolution = 1; Resolution < MB_MAP_RESOLUTION; ++Resolution)
        {
        Width /= 2;
        Height /= 2;               

        double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MB_MAP_WIDTH, Width);

        HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
            HFC_READ_ONLY,                                      // AccessMode,
            GetCapabilities(),                                  // Capabilities,
            Ratio,                                              // XResolutionRatio,
            Ratio,                                              // YResolutionRatio,
            PixelType,                                          // PixelType,
            new HCDCodecIdentity(),                             // Codecs,
            HRFBlockAccess::RANDOM,                             // RBlockAccess,
            HRFBlockAccess::RANDOM,                             // WBlockAccess,
            HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
            HRFInterleaveType::PIXEL,                           // InterleaveType
            false,                                              // IsInterlace,
            Width,                                              // Width,
            Height,                                             // Height,
            256,                                                // BlockWidth,
            256,                                                // BlockHeight,
            0,                                                  // BlocksDataFlag
            HRFBlockType::TILE);                                // BlockType

        ListOfResolutionDescriptor.push_back(pResolution);
        }

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),           // Capabilities,
                                   ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                   0,                           // RepresentativePalette,
                                   0,                           // Histogram,
                                   0,                           // Thumbnail,
                                   0,                           // ClipShape,
                                   pTransfoModel,               // TransfoModel,
                                   0,                           // Filters
                                   0);                           // Defined tag


    if (!pBaseGCS.IsNull() && pBaseGCS->IsValid())
        pPage->SetGeocoding(pBaseGCS.get());

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Protected
// Cancel the current look ahead
//-----------------------------------------------------------------------------
void HRFMapBoxFile::CancelLookAhead(uint32_t pi_Page)
    {
    assert(0);
    HPRECONDITION(pi_Page == 0);

    // I assumed that HRF is not thread safe and that there will be only one thread that will execute LookAHead request and copyFrom(ReadBlock).
    m_tileQueryMap.clear();
    }


//-----------------------------------------------------------------------------
// Indicates that the Virtual Earth file format supports the look ahead by
// block (i.e. tile).
//-----------------------------------------------------------------------------
bool HRFMapBoxFile::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return true;
    }

//-----------------------------------------------------------------------------
// Indicates that the a look ahead can be performed for the Virtual Earth
// file format.
//-----------------------------------------------------------------------------
bool HRFMapBoxFile::CanPerformLookAhead(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page == 0);
    return true;
    }



//-----------------------------------------------------------------------------
// Protected
// Function that is called to request some tiles to be fetch to the server
// before that are actually read by the application. HRFVirtualEarthFile
// supports only look ahead by tiles.
//-----------------------------------------------------------------------------
void HRFMapBoxFile::RequestLookAhead(uint32_t             pi_Page,
                                     const HGFTileIDList& pi_rBlocks,
                                     bool                 pi_Async)
{

    if (pi_rBlocks.size() == 0)
        { 
        m_tileQueryMapMutex.lock();
        m_tileQueryMap.clear();
        m_tileQueryMapMutex.unlock();
        return;
        }

    HGFTileIDList::const_iterator Itr(pi_rBlocks.begin());
    if (Itr != pi_rBlocks.end())
    {
        unsigned short Resolution = (unsigned short)HRFRasterFile::s_TileDescriptor.GetLevel(*Itr);

        //Find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
        HRFMapBoxTileEditor* pResEditor = 0;
        while (pResEditor == 0 && ResItr != m_ResolutionEditorRegistry.end())
        {
            if (((*ResItr)->GetPage() == pi_Page) && ((*ResItr)->GetResolutionIndex() == Resolution))
            {
                pResEditor = (HRFMapBoxTileEditor*)*ResItr;
            }
            else
            {
                ResItr++;
            }
        }
        //The editor must exist
        HASSERT(pResEditor != 0);

        pResEditor->RequestLookAhead(pi_rBlocks);
    }
}


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
struct ThreadLocalHttp : ThreadLocalStorage<HttpSession> {};
END_IMAGEPP_NAMESPACE

WorkerPool& HRFMapBoxFile::GetWorkerPool()
    {
    if (m_pWorkerPool == nullptr)
        {
        m_pWorkerPool.reset(new WorkerPool(NB_BLOCK_READER_THREAD));
        m_threadLocalHttp.reset(new ThreadLocalHttp()); // must be allocated before threads start querying.
        }

    return *m_pWorkerPool;
    }
