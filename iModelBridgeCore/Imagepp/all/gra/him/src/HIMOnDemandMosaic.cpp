//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMOnDemandMosaic.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HIMOnDemandMosaic.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLMemFile.h>

#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DExtent.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>

#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRARepPalParms.h>

#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HRPMessages.h>
#include <ImagePP/all/h/HMDContext.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>

#include <Imagepp/all/h/HUTImportFromRasterExportToFile.h>

#include <ImagePP/all/h/HRAMessages.h>

#include <ImagePPInternal/gra/HRAImageNode.h>


#include <BeXml/BeXml.h>


#ifdef __HMR_DEBUG
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#endif

HPM_REGISTER_CLASS(HIMOnDemandMosaic, HRARaster)

HMG_BEGIN_DUPLEX_MESSAGE_MAP(HIMOnDemandMosaic, HRARaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HIMOnDemandMosaic, HGFGeometryChangedMsg, NotifyGeometryChanged)

HMG_REGISTER_MESSAGE(HIMOnDemandMosaic, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_REGISTER_MESSAGE(HIMOnDemandMosaic, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()



static const unsigned short s_MaxNbLoadedRasters = 300;

// Cached bool value macros. 
//
// 1  = True
// 0  = False
// -1 = Unknown value
#define HIMONDEMANDMOSAIC_BOOL_TRUE       1
#define HIMONDEMANDMOSAIC_BOOL_FALSE      0
#define HIMONDEMANDMOSAIC_BOOL_UNKNOWN   -1

//This number should be changed when new information about the on-demand 
//mosaic and its raster are added for performance optimisation purpose.
//0.0 : No version (some the first Beta of Descartes SS3 has no version.
//1.0 : First version - <IDCR> node has been added to the <ODR> node.
//2.0 : Second version - <IUR> node has been added to the <ODR> node.
#define ON_DEMAND_RASTER_INFO_LATEST_VERSION 2.0


/** ---------------------------------------------------------------------------
    Default constructor, for Persistence only.
    ---------------------------------------------------------------------------
*/
HIMOnDemandMosaic::HIMOnDemandMosaic()
    : HRARaster()
    {
    m_pIndex = 0;        
    m_hasLookAhead = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;    
    m_hasUnlimitedRasterSource = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;
    m_isDataChangingWithResolution = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;    
    m_onDemandMosaicCacheInfoVersion = 0.0;
    m_CacheFileDownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;

    
    //Set the pixel type to a fixed value to avoid opening all the rasters for determining the pixel 
    //type of the mosaic.
    SetPixelTypeInfo(TRUE, HFCPtr<HRPPixelType>((HRPPixelType*)new HRPPixelTypeV32R8G8B8A8()));    
    }

/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMOnDemandMosaic::HIMOnDemandMosaic(const HFCPtr<HGF2DCoordSys>&              pi_rpCoordSys,
                                     const HFCPtr<HFCURL>&                     pi_pPSSCacheFileUrl,
                                     const HFCPtr<HRAPyramidRaster>&           pi_rpSubRes, 
                                     HRFDownSamplingMethod::DownSamplingMethod pi_cacheFileDownSamplingMethod)
    : HRARaster(pi_rpCoordSys)
    {
    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(pi_rpCoordSys)));

    m_pPSSCacheFileUrl = pi_pPSSCacheFileUrl;
    m_onDemandMosaicCacheInfoVersion = 0.0;
    m_CacheFileDownSamplingMethod = pi_cacheFileDownSamplingMethod;

    m_hasLookAhead = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;
    m_hasUnlimitedRasterSource = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;
    m_isDataChangingWithResolution = HIMONDEMANDMOSAIC_BOOL_UNKNOWN;
            
    //Ensure that the pyramid representing the lowest resolution is in the same CS than the HIMOnDemandMosaic
    //so that a HRARefenceToRaster can be put over the HIMOnDemandMosaic.
    if (pi_rpSubRes != 0)
        {
        HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_rpSubRes->GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys));
        HFCPtr<HGF2DCoordSys> pDestinationCoordSys(new HGF2DCoordSys(*pTransfoModel, pi_rpCoordSys));
        m_pSubRes = new HRAReferenceToRaster((HFCPtr<HRARaster>&)pi_rpSubRes, pDestinationCoordSys);
        }

    // We are the universe!
    SetShape(HVEShape(HVE2DUniverse(pi_rpCoordSys)));        
    
    //Set the pixel type to a fixed value to avoid opening all the rasters for determining the pixel 
    //type of the mosaic.
    SetPixelTypeInfo(TRUE, HFCPtr<HRPPixelType>((HRPPixelType*)new HRPPixelTypeV32R8G8B8A8()));             
    }

/** ---------------------------------------------------------------------------
    Copy constructor
    ---------------------------------------------------------------------------
*/
HIMOnDemandMosaic::HIMOnDemandMosaic(const HIMOnDemandMosaic& pi_rObj)
    : HRARaster((HRARaster&)pi_rObj)
    {
    //HChk MR
    // Copy constructors on indexes are not implemented!
    // We fill the list one item at a time. We could at least
    // use the batch load operations on indexes (when they
    // are available)

    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

    // Fill the images list using the index's contents
    HAutoPtr< IndexType::IndexableList > pObjects(pi_rObj.m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        m_pIndex->Add((*Itr)->GetObject());
        ++Itr;
        }

    m_hasLookAhead = pi_rObj.m_hasLookAhead;    
    m_hasUnlimitedRasterSource = pi_rObj.m_hasUnlimitedRasterSource;    
    m_isDataChangingWithResolution = pi_rObj.m_isDataChangingWithResolution;        
    m_onDemandMosaicCacheInfoVersion = pi_rObj.m_onDemandMosaicCacheInfoVersion;

    if (pi_rObj.m_pPSSCacheFileUrl != 0)
        {
        m_pPSSCacheFileUrl = HFCURL::Instanciate(pi_rObj.m_pPSSCacheFileUrl->GetURL());
        HASSERT(m_pPSSCacheFileUrl != 0);
        }
    else
        {
        m_pPSSCacheFileUrl = 0;
        }    

    m_CacheFileDownSamplingMethod = pi_rObj.m_CacheFileDownSamplingMethod;
    
    //Set the pixel type to a fixed value to avoid opening all the rasters for determining the pixel 
    //type of the mosaic.
    SetPixelTypeInfo(TRUE, HFCPtr<HRPPixelType>((HRPPixelType*)new HRPPixelTypeV32R8G8B8A8()));        
    }

/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMOnDemandMosaic::~HIMOnDemandMosaic()
    {
    if (m_pIndex)
        {
        // Unlink from all rasters before the quadtree is destroyed
        UnlinkFromAll();

        delete m_pIndex;
        }
    }

/** ---------------------------------------------------------------------------
    Assignment operation
    ---------------------------------------------------------------------------
*/
HIMOnDemandMosaic& HIMOnDemandMosaic::operator=(const HIMOnDemandMosaic& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Copy the HRARaster portion
        HRARaster::operator=(pi_rObj);

        UnlinkFromAll();

        delete m_pIndex;

        m_pEffectiveShape  = 0;
        m_pEffectiveExtent = 0;
        m_RecentlyUsedRasters.clear();

        //HChk MR
        // Assignment operators on indexes are not implemented!
        // We fill the list one item at a time. We could at least
        // use the batch load operations on indexes (when they
        // are available)

        RelativeIndexType::Parameters RelativeParameters;
        m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

        // Fill the images list using the index's contents
        HAutoPtr< IndexType::IndexableList > pObjects(pi_rObj.m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            m_pIndex->Add((*Itr)->GetObject());

            ++Itr;
        	}     

        m_hasLookAhead = pi_rObj.m_hasLookAhead;
        m_hasUnlimitedRasterSource = pi_rObj.m_hasUnlimitedRasterSource;    
        m_isDataChangingWithResolution = pi_rObj.m_isDataChangingWithResolution;
        m_onDemandMosaicCacheInfoVersion = pi_rObj.m_onDemandMosaicCacheInfoVersion;

        if (pi_rObj.m_pPSSCacheFileUrl != 0)
            {
            m_pPSSCacheFileUrl = HFCURL::Instanciate(pi_rObj.m_pPSSCacheFileUrl->GetURL());
            HASSERT(m_pPSSCacheFileUrl != 0);
            }
        else
            {
            m_pPSSCacheFileUrl = 0;
            }    

        m_CacheFileDownSamplingMethod = pi_rObj.m_CacheFileDownSamplingMethod;
        }

    return *this;
    }

/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Since the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------*/
ImagePPStatus HIMOnDemandMosaic::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions)
    {
    HPRECONDITION(pi_rOptions.GetEffectiveCopyRegion() != NULL);

    ImagePPStatus status = COPYFROM_STATUS_VoidRegion;

    // Gather all images
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    for(IndexType::IndexableList::const_iterator Itr(pObjects->begin()); Itr != pObjects->end(); ++Itr)
        {
        HFCPtr<HRARaster> pRaster = GetRaster((*Itr)->GetObject());
        if (pRaster == 0)
            continue;

        // Compute clip shape for the source
        HVEShape VisibleShape(*pi_rOptions.GetEffectiveCopyRegion());
        VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));
        HRACopyFromOptions ThisSourceOptions(pi_rOptions);
        ThisSourceOptions.SetEffectiveCopyRegion(&VisibleShape);
        
        // Copy in this source. Stop on first error or keep going and do what we can? ==> for now stop on first error.
        if(IMAGEPP_STATUS_Success != (status = pRaster->CopyFrom(srcRaster, ThisSourceOptions)))
            break;
        }

    return status;
    }

/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Sicne the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    // Take the mosaic's effective shape
    HFCPtr<HVEShape> pDestShape(new HVEShape(*GetEffectiveShape()));

    // Intersect with source's effective shape
    pDestShape->Intersect(*pi_pSrcRaster->GetEffectiveShape());

    // Intersect with specified "copy from" shape if necessary
    if (pi_rOptions.GetDestShape() != 0)
        pDestShape->Intersect(*(pi_rOptions.GetDestShape()));

    if (!pDestShape->IsEmpty())
        {
        // Gather all images
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            // Compute clip shape for the source
            HVEShape VisibleShape(*pDestShape);
            VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));
            HRACopyFromLegacyOptions ThisSourceOptions(pi_rOptions);
            ThisSourceOptions.SetDestShape(&VisibleShape);

            HFCPtr<HRARaster> pRaster = GetRaster((*Itr)->GetObject());

            // Copy in this source
            if (pRaster != 0)
                {
                pRaster->CopyFromLegacy(pi_pSrcRaster, ThisSourceOptions);
                }

            ++Itr;
            }
        }
    }


/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Sicne the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMOnDemandMosaic::GetRaster(const HFCPtr<HRAOnDemandRaster>& pi_rpOnDemandRaster) const
    {
    OpenRaster             Raster;
    HFCPtr<HRFRasterFile>  pRasterFile;
    bool                  RasterFound    = false;
    RURasterList::iterator pRasterIter    = m_RecentlyUsedRasters.begin();
    RURasterList::iterator pRasterIterEnd = m_RecentlyUsedRasters.end();

    while (pRasterIter != pRasterIterEnd)
        {
        //Move the raster at the top of the list of the most recently used rasters.
        if ((*pRasterIter).m_pOnDemandRaster == pi_rpOnDemandRaster)
            {
            Raster      = *pRasterIter;
            RasterFound = true;
            m_RecentlyUsedRasters.erase(pRasterIter);
            m_RecentlyUsedRasters.push_front(Raster);
            break;
            }
        pRasterIter++;
        }

    if (RasterFound == false)
        {
        if (m_RecentlyUsedRasters.size() == s_MaxNbLoadedRasters)
            {
            UnlinkFrom(m_RecentlyUsedRasters.back().m_pRaster);
            m_RecentlyUsedRasters.pop_back();
            }

        HASSERT(m_RecentlyUsedRasters.size() < s_MaxNbLoadedRasters);

        Raster.m_pOnDemandRaster = pi_rpOnDemandRaster;
        Raster.m_pRaster         = pi_rpOnDemandRaster->LoadRaster();

        if (Raster.m_pRaster != 0)
            {        
            LinkTo(Raster.m_pRaster);

            m_RecentlyUsedRasters.push_front(Raster);
            }
        }

    return Raster.m_pRaster;
    }


/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Sicne the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }


//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------
void HIMOnDemandMosaic::Clear()
    {
    HASSERT(!"HIMOnDemandMosaic is currently used in read only mode.");
    }

//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------
void HIMOnDemandMosaic::Clear(const HRAClearOptions& pi_rOptions)
    {
    HASSERT(!"HIMOnDemandMosaic is currently used in read only mode.");
    }

//-----------------------------------------------------------------------------
// public
// SetContext
//-----------------------------------------------------------------------------
void HIMOnDemandMosaic::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    //HIMOnDemandMosaic is currently used in read only mode.
    HASSERT(0);
    }

//-----------------------------------------------------------------------------
// public
// GetContext
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HIMOnDemandMosaic::GetContext()
    {
    HASSERT(0); //TBD
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// InvalidateRaster
//-----------------------------------------------------------------------------
void HIMOnDemandMosaic::InvalidateRaster()
    {
    RURasterList::iterator rasterIter(m_RecentlyUsedRasters.begin());
    RURasterList::iterator rasterIterEnd(m_RecentlyUsedRasters.end());

    //Only the loaded raster should be invalidated, not all the raster in the mosaic.
    while (rasterIter != rasterIterEnd)
    {
        rasterIter->m_pRaster->InvalidateRaster();
        rasterIter++;
    }
    }

/** ---------------------------------------------------------------------------
    Notification for palette change. We will propagate a ContentChanged.
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // the palette changed become a content change here
    const HRARaster* pSender = (const HRARaster*)pi_rMessage.GetSender();
    Propagate(HRAContentChangedMsg(*(pSender->GetEffectiveShape())));
    
    // do not propagate the old message
    return false;
    }

/** ---------------------------------------------------------------------------
    Add a list of images to the mosaic. This will be faster than adding the
    images one by one since the index will be updated only once.
    @see HIMOnDemandMosaic::Add(const HFCPtr<HRARaster>& pi_pRaster)
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::Add(const HIMOnDemandMosaic::RasterList& pi_rRasters)
    {
    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Create a list of indexables, so we can call AddIndexables directly.
    // We are already iterating on the objects, so we save one pass...
    HAutoPtr<IndexType::IndexableList> pIndexables(new IndexType::IndexableList);

    RasterList::const_iterator Itr(pi_rRasters.begin());
    while (Itr != pi_rRasters.end())
        {
        pIndexables->push_back(new HIDXIndexable< HFCPtr<HRAOnDemandRaster> >(*Itr));
        ++Itr;
        }

    // Add raster
    m_pIndex->AddIndexables(pIndexables);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();     

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Add a list of images to the mosaic from a serialization string.
    @see HIMOnDemandMosaic::GetOnDemandRastersInfo(string& po_rOnDemandRastersInfo)
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::Add(const string&                    pi_rOnDemandRastersInfo, 
                            HPMPool*                         pi_pMemPool, 
                            const HFCPtr<HGF2DWorldCluster>& pi_rAppWorldCluster, 
                            const HFCPtr<HFCURL>&            pi_rpPSSUrl)
{    
    bool                      result = true;
    
    HFCPtr<HRAOnDemandRaster> pOnDemandRaster;
    HFCPtr<HPSWorldCluster>   pWorldCluster;
    HGF2DWorldIdentificator   WorldId;
    RasterList                Rasters;        
    BeXmlStatus               XmlStatus;
    
    try 
    {
        //XMLDocument onDemandMosaicXMLInfo(pi_rOnDemandRastersInfo.c_str(), pi_rOnDemandRastersInfo.size());

        WString TempOnDemandRastersInfo;

        BeStringUtilities::Utf8ToWChar(TempOnDemandRastersInfo, pi_rOnDemandRastersInfo.c_str());

        BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (XmlStatus, TempOnDemandRastersInfo.c_str(), TempOnDemandRastersInfo.length()));

        HASSERT(pXmlDom != 0);

        Utf8String NodeName;

        BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
           
        HASSERT(0 == BeStringUtilities::Stricmp(pRootNode->GetName(), "OnDemandMosaic"));
                            
        if (0 == BeStringUtilities::Stricmp(pRootNode->GetName(), "OnDemandMosaic"))
            {   
            double version;
            
            if (BEXML_Success == pRootNode->GetAttributeDoubleValue(version, "version")) 
                {
                m_onDemandMosaicCacheInfoVersion = version;
                }            
            else
                {
                m_onDemandMosaicCacheInfoVersion = 0.0;
                }
                      
            //Get the minimum and maximum pixel size range.            
            BeXmlNodeP pODMOChildNode = pRootNode->SelectSingleNode("MinimumPixelSize");

            HASSERT(pODMOChildNode != 0);        

            BeXmlStatus isAttributePresent;                
            double xMin, xMax, yMin, yMax;

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(xMin, "xMin"); 
            HASSERT(BEXML_Success == isAttributePresent);

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(xMax, "xMax"); 
            HASSERT(BEXML_Success == isAttributePresent);

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(yMin, "yMin"); 
            HASSERT(BEXML_Success == isAttributePresent);

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(yMax, "yMax"); 
            HASSERT(BEXML_Success == isAttributePresent);    
            
            m_pMinimumPixelSizeRange = new HGF2DExtent(xMin, yMin, xMax, yMax, GetCoordSys());    

            pODMOChildNode = pRootNode->SelectSingleNode("MaximumPixelSize");

            HASSERT(pODMOChildNode != 0);        

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(xMin, "xMin"); 
            HASSERT(BEXML_Success == isAttributePresent);  

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(xMax, "xMax"); 
            HASSERT(BEXML_Success == isAttributePresent);  

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(yMin, "yMin"); 
            HASSERT(BEXML_Success == isAttributePresent);  

            isAttributePresent = pODMOChildNode->GetAttributeDoubleValue(yMax, "yMax"); 
            HASSERT(BEXML_Success == isAttributePresent);  

            m_pMaximumPixelSizeRange = new HGF2DExtent(xMin, yMin, xMax, yMax, GetCoordSys());    
                    
            //Get the represententative PSS containing the world related statements found in the original PSS.
            pODMOChildNode = pRootNode->SelectSingleNode("RepresentativePSSForWorld");
                               
            if (pODMOChildNode != 0)
                {        
                //RepresentativePSS								
                BeXmlStatus XmlStatus = pODMOChildNode->GetContent (m_WorldDescriptivePSS);

                HASSERT(BEXML_Success == XmlStatus);
/*
        #ifdef __HMR_UNICODE    
                m_WorldDescriptivePSS = HARRAYAUTOPTR(HWCHAR,
                                                       HFCUnicodeConverter::FromUTF8ToWideChar(pODMOChildNode->GetValue().c_str()));

                string TempPSS(m_WorldDescriptivePSS.begin(), m_WorldDescriptivePSS.end());
        #else
                m_WorldDescriptivePSS = HARRAYAUTOPTR(HCHAR,
                                                       HFCUnicodeConverter::FromUTF8ToMultiByte(pODMOChildNode->GetValue().c_str()));    

                string TempPSS(m_WorldDescriptivePSS);
        #endif    
        */		   		
                // PSS data must be stored as UTF8.
                Utf8String worldDescriptivePSS_UTF8(m_WorldDescriptivePSS.c_str());
                HFCPtr<HFCBuffer> pBuffer(new HFCBuffer(worldDescriptivePSS_UTF8.size()));
                pBuffer->AddData((Byte const*)worldDescriptivePSS_UTF8.c_str(), worldDescriptivePSS_UTF8.size());
               
		        HFCPtr<HFCURL> pURLMemFile = new HFCURLMemFile(HFCURLMemFile::s_SchemeName() + L"://Raster", pBuffer);
		        HPSObjectStore PSSObjectStore(pi_pMemPool,
		                                      pURLMemFile,
		                                      pi_rAppWorldCluster,
		                                      HFCPtr<HFCURL>());

		        HASSERT(PSSObjectStore.GetWorldCluster()->IsCompatibleWith(HPSWorldCluster::CLASS_ID) == true);

		        //Get the world cluster and the world ID resulting from the parsing of the world statements.
                pWorldCluster = static_cast<HPSWorldCluster*>(PSSObjectStore.GetWorldCluster().GetPtr());

		        WorldId = PSSObjectStore.GetWorldID();
				       
                }
            else
                {
                //If there is no world related statement in the PSS describing 
                //this OnDemandMosaic, just use the default world cluster 
                //and world id.
                pWorldCluster = new HPSWorldCluster();
                WorldId = 0;

                // Link the PSS world cluster to the application world cluster.
                HFCPtr<HGF2DCoordSys> pPSSUnknown(pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                HFCPtr<HGF2DCoordSys> pAppUnknown(pi_rAppWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                pPSSUnknown->SetReference(HGF2DIdentity(), pAppUnknown);
                }    

            SetCoordSys(pWorldCluster->GetCoordSysReference(WorldId));
            
            //Create the HRAOnDemandRaster from the serialization information.        
            //Get the represententative PSS containing the world related statements found in the original PSS.            
            pODMOChildNode = pRootNode->SelectSingleNode("OnDemandRasterList");

            HASSERT(pODMOChildNode != 0);
            
            BeXmlNodeP pOnDemandRasterNode = pODMOChildNode->GetFirstChild();

            Utf8String onDemandRaster; 

            while (0 != pOnDemandRasterNode) 
                {                  
                pOnDemandRaster = new HRAOnDemandRaster(pOnDemandRasterNode, 
                                                        pi_pMemPool, 
                                                        GetCoordSys(), 
                                                        pWorldCluster, 
                                                        WorldId, 
                                                        pi_rpPSSUrl);
                Rasters.push_back(pOnDemandRaster);

                pOnDemandRasterNode = pOnDemandRasterNode->GetNextSibling();
                }

            //Set the pixel type to a fixed value to avoid opening all the rasters for determining the pixel 
            //type of the mosaic.
            SetPixelTypeInfo(true, 
                             HFCPtr<HRPPixelType>((HRPPixelType*)new HRPPixelTypeV32R8G8B8A8()));

            //Add the rasters to the mosaic.
            Add(Rasters);
            }
        else
            {
            result = false;
            }
        }
    catch (...)
        {
        result = false;
        }

    return result;
}     

/** ---------------------------------------------------------------------------
    Set the precomputed pixel type info to avoid loading all the rasters each
    time HasSinglePixelType and GetPixelType methods are called.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::SetPixelTypeInfo(bool                        pi_HasSinglePixelType,
                                         const HFCPtr<HRPPixelType>& pi_rpRepresentativePixelType)
    {
    m_pSetPixelType = pi_rpRepresentativePixelType;

    if (m_pHasSinglePixelType == 0)
        {
        m_pHasSinglePixelType = new bool;
        }

    *m_pHasSinglePixelType = pi_HasSinglePixelType;
    }

/** ---------------------------------------------------------------------------
    Set the precomputed pixel type info to avoid loading all the rasters each
    time method GetPixelSizeRange is called.
    @see HIMOnDemandMosaic::SetPrecomputedPixelSizeRange
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::SetPrecomputedPixelSizeRange(HGF2DExtent& pi_rMinimum,
                                                     HGF2DExtent& pi_rMaximum)
    {
    if (m_pMinimumPixelSizeRange == 0)
        {
        m_pMinimumPixelSizeRange = new HGF2DExtent;
        }

    *m_pMinimumPixelSizeRange = pi_rMinimum;

    m_pMinimumPixelSizeRange->ChangeCoordSys(GetCoordSys());

    if (m_pMaximumPixelSizeRange == 0)
        {
        m_pMaximumPixelSizeRange = new HGF2DExtent;
        }

    *m_pMaximumPixelSizeRange = pi_rMaximum;

    m_pMaximumPixelSizeRange->ChangeCoordSys(GetCoordSys());
    }

/** ---------------------------------------------------------------------------
    Tell if the mosaic has a homogeneous pixel type.
    @see HIMOnDemandMosaic::HasSinglePixelType
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::HasSinglePixelType() const
    {
    bool                 SinglePixelType = true;
    HFCPtr<HRPPixelType> PixelType;
    HFCPtr<HRARaster>    pSource;

    if (m_pHasSinglePixelType == 0)
        {
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        while (SinglePixelType && Itr != pObjects->end())
            {
            // Take a pointer to the raster
            pSource = GetRaster((*Itr)->GetObject());

            if (pSource != 0)
                {        
                SinglePixelType = HasSinglePixelType(pSource, PixelType);
                }

            ++Itr;
            }
        }
    else
        {
        SinglePixelType = *m_pHasSinglePixelType;
        }

    return SinglePixelType;
    }

/** ---------------------------------------------------------------------------
    Determine that the raster passed in parameter has the same pixel type as
    the pixel type passed in parameter.
    @see HIMOnDemandMosaic::HasSinglePixelType
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::HasSinglePixelType(const HFCPtr<HRARaster>& pi_rpSourceRaster,
										   HFCPtr<HRPPixelType>&    pio_rpPrevFoundPixelType)
    {
    bool SinglePixelType = true;

    // If the current source has a unique pixel type
    if (pi_rpSourceRaster->HasSinglePixelType())
        {
        // If it's the first source we're checking
        if (pio_rpPrevFoundPixelType == 0)
            {
            // Keep its pixel type for further tests
            pio_rpPrevFoundPixelType = pi_rpSourceRaster->GetPixelType();
            }
        else
            {
            // Check if same pixel type as previous sources
            SinglePixelType = pio_rpPrevFoundPixelType->HasSamePixelInterpretation(*pi_rpSourceRaster->GetPixelType());
            }
        }
    else
        {
        // Source doesn't has a unique pixel type, so mosaic doesn't either...
        SinglePixelType = false;
        }

    return SinglePixelType;
    }

/** ---------------------------------------------------------------------------
    Get the mosaic's pixel type. If all images inside the mosaic have the
    same pixeltype, this pixeltype is returned. Otherwise, the result will be
    a pixeltype that can contain the colors of all images without loss. For
    example, two 8 bits palette images will force a 24 bits value pixeltype.
    @note The pixeltype of the mosaic can change over time, as we may add
    or remove images.
    @note If the mosaic is empty, the result will be V24R8G8B8.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HIMOnDemandMosaic::GetPixelType() const
    {
    // If a pixel type has been specified, return it now

#ifdef __HMR_DEBUG
    static bool Test = false;

    if (Test == true)
        {
        return new HRPPixelTypeI8R8G8B8();
        //return new HRPPixelTypeV1Gray1();
        }
#endif

    if (m_pSetPixelType != NULL)
        return m_pSetPixelType;
    
    HASSERT(!"The pixel type should be fixed and hardcoded for now.");

    HFCPtr<HRPPixelType> pPixelType;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    if (pObjects->size() == 0)
        {
        // We don't know what pixeltype to use, since we don't have
        // any image. Default to V24RGB.
        pPixelType = new HRPPixelTypeV24R8G8B8();
        }
    else
        {
        if (HasSinglePixelType())
            {
            // Get the pixel type. NULL if no image in mosaic...
            HFCPtr<HRARaster> pRaster(GetRaster(pObjects->front()->GetObject()));

            if (pRaster != 0)
                {
                pPixelType = pRaster->GetPixelType();
                }
            }
        else
            {
            // Mosaic with Different PixelTypes
            // Must create a PixelType to include all mosaic Pixeltypes
            IndexType::IndexableList::const_iterator Itr(pObjects->begin());
            HFCPtr<HRARaster>                        pRaster;
            size_t                                   IndexBits = 0;            
            HRPChannelOrg                            CurrentOrg;            
            bool                                     CanUseIndex = true;
            bool                                     ForceRGB = false;
            bool                                     GrayscalePalette = false;
            bool                                     AddBW = false;

            while (Itr != pObjects->end())
            { 
                pRaster = GetRaster((*Itr)->GetObject());

                if (pRaster == 0)
                    {      
                    Itr++;
                    continue;                   
                    }
                
                if (CurrentOrg.CountChannels() == 0)
                    {
                    CurrentOrg = pRaster->GetPixelType()->GetChannelOrg();
                    }

                if (CanUseIndex)
                    {
                    size_t SourceIndexBits = pRaster->GetPixelType()->CountIndexBits();

                    if (SourceIndexBits > 0)
                        {
                        IndexBits += SourceIndexBits;

                        // Check if there is an alpha channel in the source. If there is, we must
                        // make sure that colors are all opaque or fully transparent. We won't be able
                        // to generate a representative palette for colors that must be blended.
                        bool PartialAlpha = false;
                        HRPChannelOrg SourceOrg(pRaster->GetPixelType()->GetChannelOrg());
                        size_t ChannelIndex = SourceOrg.GetChannelIndex(HRPChannelType::ALPHA, 0);
                        if (ChannelIndex != HRPChannelType::FREE)
                            {
                            HFCPtr<HRPPixelType> pSourceValuePixelType(HRPPixelTypeFactory::GetInstance()->Create(SourceOrg, 0));
                            if (pSourceValuePixelType == 0)
                                {
                                PartialAlpha = true;
                                }
                            else
                                {
                                HRPPixelPalette const& rSrcPalette = pRaster->GetPixelType()->GetPalette();
                                HFCPtr<HRPPixelType> pV32PixelType(new HRPPixelTypeV32R8G8B8A8());
                                HFCPtr<HRPPixelConverter> pConverter(pSourceValuePixelType->GetConverterTo(pV32PixelType));
                                Byte Color[4];

                                for (uint32_t i = 0 ; !PartialAlpha && i < rSrcPalette.CountUsedEntries() ; ++i)
                                    {
                                    pConverter->Convert(rSrcPalette.GetCompositeValue(i), Color);
                                    if (Color[3] > 0 && Color[3] < 255)
                                        PartialAlpha = true;
                                    }
                                }
                            }

                        // Cases where we can't stay in palette mode:
                        //   1) We were already grayscale palette and we're adding index bits
                        //   2) We get a palette with more than 256 entries
                        //   3) We get a 256 entries palette and we must add black and white to it
                        //   4) The pixeltype also has value bits
                        //   5) Some colors have partial alpha values
                        if (GrayscalePalette ||
                            IndexBits > 8   ||
                            (IndexBits == 8 && AddBW) ||
                            pRaster->GetPixelType()->CountValueBits() > 0 ||
                            PartialAlpha)
                            CanUseIndex = false;
                        }
                    else
                        {
                        size_t SourceCompositeValueBits = pRaster->GetPixelType()->GetChannelOrg().CountPixelCompositeValueBits();

                        if (SourceCompositeValueBits == 1)
                            {
                            // We have a black and white image here
                            AddBW = true;

                            // Don't stay in palette mode if we already use the full 256 entries.
                            if (IndexBits >= 8)
                                CanUseIndex = false;
                            }
                        else if (SourceCompositeValueBits <= 8)
                            {
                            if (IndexBits > 0 && !GrayscalePalette)
                                {
                                CanUseIndex = false;
                                }
                            else
                                {
                                GrayscalePalette = true;
                                ForceRGB  = true;
                                IndexBits = 8;
                                }
                            }
                        else
                            {
                            // More than 8 value bits...
                            CanUseIndex = false;
                            }
                        }
                    }

                // Take the most general ChannelOrg
                if (ForceRGB || CurrentOrg != pRaster->GetPixelType()->GetChannelOrg())
                    {
                    HRPChannelOrg TempOrg1(CurrentOrg);
                    HRPChannelOrg TempOrg2(pRaster->GetPixelType()->GetChannelOrg());
                    unsigned short ContainsAlpha = 0;

                    uint32_t ChannelIndex = TempOrg1.GetChannelIndex(HRPChannelType::ALPHA, 0);
                    if (ChannelIndex != HRPChannelType::FREE)
                        {
                        ContainsAlpha = 1;
                        TempOrg1.DeleteChannel((HRPChannelType*)TempOrg1.GetChannelPtr(ChannelIndex));
                        }
                    ChannelIndex = TempOrg2.GetChannelIndex(HRPChannelType::ALPHA, 0);
                    if (ChannelIndex != HRPChannelType::FREE)
                        {
                        ContainsAlpha = 2;

                        TempOrg2.DeleteChannel((HRPChannelType*)TempOrg2.GetChannelPtr(ChannelIndex));
                        }

                    if (!ForceRGB && TempOrg1 == TempOrg2)
                        {
                        if (ContainsAlpha == 2)
                            CurrentOrg = pRaster->GetPixelType()->GetChannelOrg();
                        }
                    else
                        {
                        // Not the same Channel organizations (even without checking alpha).
                        // We will use RGB

                        CurrentOrg = HRPChannelOrgRGB(8,
                                                      8,
                                                      8,
                                                      ContainsAlpha ? 8 : 0,
                                                      HRPChannelType::UNUSED,
                                                      HRPChannelType::INT_CH,
                                                      0);

                        ForceRGB = false;

                        // We use break to save tests. We don't need to continue if
                        // we've hit 32 bits value.
                        if (ContainsAlpha != 0 && (IndexBits == 0 || !CanUseIndex))
                            break;
                        }
                    }

                ++Itr;
                }

            // If there was alpha somewhere, we can't use
            // a pixeltype with a palette
            if (GrayscalePalette)
                {
                size_t ChannelIndex = CurrentOrg.GetChannelIndex(HRPChannelType::ALPHA, 0);
                if (ChannelIndex != HRPChannelType::FREE)
                    IndexBits = 0;
                }

            if (!CanUseIndex)
                IndexBits = 0;

            if (IndexBits > 1)
                IndexBits = 8;

            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(CurrentOrg, (unsigned short)IndexBits);

            if (pPixelType == 0)
                pPixelType = new HRPPixelTypeV32R8G8B8A8();
            else
                {
                if (IndexBits)
                    {
                    // We'll fill our palette with the colors of the palettes of all
                    // our sources since we're sure that we have enough entries.
                    HRPPixelPalette& rPalette = pPixelType->LockPalette();
                    HRPPixelPalette NewPalette(rPalette.GetMaxEntries(), rPalette.GetChannelOrg());
                    bool IndexingWorks = true;

                    if (GrayscalePalette)
                        {
                        Byte Color[4];
                        for (uint32_t i = 0 ; i < NewPalette.GetMaxEntries() ; ++i)
                            {
                            memset(Color, i, 4);    // no trouble, we don't have alpha...
                            NewPalette.AddEntry(Color);
                            }
                        }
                    else
                        {
                        // Need a value pixeltype with the same ChannelOrg for color conversions.
                        HFCPtr<HRPPixelType> pValuePixelType(HRPPixelTypeFactory::GetInstance()->Create(CurrentOrg, 0));
                        if (pValuePixelType == 0)
                            pValuePixelType = new HRPPixelTypeV32R8G8B8A8();

                        // Go through all sources
                        Itr = pObjects->begin();
                        while (IndexingWorks && Itr != pObjects->end())
                            {
                            HASSERT(pRaster->GetPixelType()->CountIndexBits() > 0);
                            HFCPtr<HRPPixelType> pSourceValuePixelType(HRPPixelTypeFactory::GetInstance()->Create(pRaster->GetPixelType()->GetChannelOrg(), 0));
                            if (pSourceValuePixelType == 0)
                                {
                                IndexingWorks = false;
                                }
                            else
                                {
                                HRPPixelPalette const& rSrcPalette = pRaster->GetPixelType()->GetPalette();
                                HFCPtr<HRPPixelConverter> pConverter(pSourceValuePixelType->GetConverterTo(pValuePixelType));
                                Byte Color[4];

                                for (uint32_t i = 0 ; i < rSrcPalette.CountUsedEntries() ; ++i)
                                    {
                                    // We add every color of the source palette (converted)
                                    pConverter->Convert(rSrcPalette.GetCompositeValue(i), Color);
                                    NewPalette.AddEntry(Color);
                                    }
                                }

                            ++Itr;
                            }

                        if (AddBW)
                            {
                            HFCPtr<HRPPixelConverter> pConverter(HRPPixelTypeV24R8G8B8().GetConverterTo(pValuePixelType));
                            Byte Black[4] = {0,0,0,0};
                            Byte White[4] = {255,255,255,255};
                            Byte Color[4];
                            pConverter->Convert(Black, Color);
                            NewPalette.AddEntry(Color);
                            pConverter->Convert(White, Color);
                            NewPalette.AddEntry(Color);
                            }
                        }

                    rPalette = NewPalette;
                    pPixelType->UnlockPalette();

                    if (!IndexingWorks)
                        pPixelType = new HRPPixelTypeV32R8G8B8A8();
                    }
                }
            }
        }

    // Return result
    return pPixelType;
    }

/** ---------------------------------------------------------------------------
    Retrieve the average pixel size of the mosaic.
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMOnDemandMosaic::GetAveragePixelSize () const
    {
    double     XMax = 0.0;
    double     YMax = 0.0;
    uint32_t     UsedSources = 0;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    if (pObjects->size() > 0)
        {
        HGF2DExtent CurrentPixelSize;
        HFCPtr<HRARaster> pRaster;

        while (Itr != pObjects->end())
            {
            // Get pixel size from current source
            pRaster = GetRaster((*Itr)->GetObject());

            if (pRaster != 0)
                {                                                            
                HGF2DExtent TempExtent = pRaster->GetAveragePixelSize();
            
                // Check if this extent is defined
                if (TempExtent.IsDefined())
                    {
                    // Make sure the extent is not null
                    HASSERT(TempExtent.GetWidth() != 0.0);
                    HASSERT(TempExtent.GetHeight() != 0.0);
                
                    HGF2DExtent AdaptedExtent = TempExtent.CalculateApproxExtentIn(GetCoordSys());
                
                    double AverageX = AdaptedExtent.GetWidth();
                    double AverageY = AdaptedExtent.GetHeight();
                
                    // Take current source's pixel size, and add it up...
                    XMax += AverageX;
                    YMax += AverageY;

                    UsedSources++;
                    }
                }

            ++Itr;
            }
        }

    // Return calculated pixel size
    if (UsedSources == 0)
        {
        // No sources ... return undefined extent
        return HGF2DExtent(GetCoordSys());
        }
    else
        {
        return HGF2DExtent(0.0, 0.0, XMax / (double)UsedSources, YMax / (double)UsedSources, GetCoordSys());
        }
    }

/** ---------------------------------------------------------------------------
    Return the pixel size range of the mosaic
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    if (m_pMinimumPixelSizeRange != 0)
        {
        HASSERT(m_pMaximumPixelSizeRange != 0);
        po_rMinimum = *m_pMinimumPixelSizeRange;
        po_rMaximum = *m_pMaximumPixelSizeRange;
        }
    else
        {
        bool   Initialized = false;
        double MinimumArea;
        double MaximumArea;        

        // Initialize return pizel sizes to undefined
        po_rMinimum = HGF2DExtent(GetCoordSys());
        po_rMaximum = po_rMinimum;

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        if (pObjects->size() > 0)
            {
            HFCPtr<HRARaster> pRaster;

            while (Itr != pObjects->end())
                {
                // Get pixel size from current source
                pRaster = GetRaster((*Itr)->GetObject());

                if (pRaster != 0)
                    {                                    
                    GetPixelSizeRange(pRaster, 
                                      GetCoordSys(), 
	                                  Initialized, 
    	                              MinimumArea, 
        	                          MaximumArea, 
            	                      po_rMinimum, 
                					  po_rMaximum);
                    }

                ++Itr;
                }
            }

        po_rMinimum.ChangeCoordSys(GetCoordSys());
        po_rMaximum.ChangeCoordSys(GetCoordSys());
        }

    //Ensure that the extent objects are in the coordinate system of the mosaic.
    HPOSTCONDITION(po_rMinimum.GetCoordSys()->GetTransfoModelTo(GetCoordSys())->IsIdentity() && 
                   po_rMaximum.GetCoordSys()->GetTransfoModelTo(GetCoordSys())->IsIdentity());

    }


/** ---------------------------------------------------------------------------
    Return the pixel size range of the mosaic
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::GetPixelSizeRange(HFCPtr<HRARaster>            pi_pRaster,
                                          const HFCPtr<HGF2DCoordSys>& pi_rpMosaicCoordSys,
                                          bool&                        pio_rInitialized,
                                          double&                      pio_rMinimumArea,
                                          double&                      pio_rMaximumArea,
                                          HGF2DExtent&                 pio_rMinimum,
                                          HGF2DExtent&                 pio_rMaximum)
    {
    HGF2DExtent SourceMinimum;
    HGF2DExtent SourceMaximum;

    // Get pixel size from current source
    pi_pRaster->GetPixelSizeRange(SourceMinimum, SourceMaximum);
    // Get the dimensions in our coordinate system
    HVEShape TempSourceMinimumShape(SourceMinimum);
    HVEShape TempSourceMaximumShape(SourceMaximum);
    TempSourceMinimumShape.ChangeCoordSys(pi_rpMosaicCoordSys);
    TempSourceMaximumShape.ChangeCoordSys(pi_rpMosaicCoordSys);

    // Compute areas of pixel sizes shapes
    double CurrentMinimumArea = TempSourceMinimumShape.GetShapePtr()->CalculateArea();
    double CurrentMaximumArea = TempSourceMaximumShape.GetShapePtr()->CalculateArea();

    if (!HDOUBLE_EQUAL_EPSILON(sqrt(CurrentMinimumArea), 0.0))
        {
        // Check if areas have been initialized
        if (!pio_rInitialized)
            {
            // Check if extents are defined
            if (SourceMinimum.IsDefined() && SourceMaximum.IsDefined())
                {
                // The return values have not been initialized .. we set with current sourcd pixel sizes
                pio_rMinimumArea = CurrentMinimumArea;
                pio_rMaximumArea = CurrentMaximumArea;
                pio_rMinimum = SourceMinimum;
                pio_rMaximum = SourceMaximum;

                // Inidicate it has been initialized
                pio_rInitialized = true;
                }
            }
        else
            {
            // The return values have been initialized ... check if current source minimum is smaller
            // and defined and not null (exact floating point compare)
            if (CurrentMinimumArea < pio_rMinimumArea && SourceMinimum.IsDefined() && CurrentMinimumArea != 0.0)
                {
                // The current source minimum is smaller ... set
                pio_rMinimumArea = CurrentMinimumArea;
                pio_rMinimum     = SourceMinimum;
                }

            // Check if current source maximum is greater
            // and defined and not null (exact floating point compare)
            if (CurrentMaximumArea > pio_rMaximumArea && SourceMaximum.IsDefined() && CurrentMaximumArea != 0.0)
                {
                // The current source minimum is smaller ... set
                pio_rMaximumArea = CurrentMaximumArea;
                pio_rMaximum     = SourceMaximum;
                }
            }
        }
    }

/** ---------------------------------------------------------------------------
    Return the effective shape of the mosaic
    ---------------------------------------------------------------------------
*/
HFCPtr<HVEShape> HIMOnDemandMosaic::GetEffectiveShape () const
    {
    //Currently, for large sparsed mosaic (only a small portion of the mosaic is covered by
    //raster data), the shape library isn't fast enough, so used the extent instead.
    //For now, this should cause no problem since the HIMOnDemandMosaic is only used
    //with PSS as a replacement to the HIMMosaic object for creating a mosaic with
    //many images.
    if (m_pEffectiveShape == 0)
        {
        m_pEffectiveShape = new HVEShape(GetExtent());
        }

    return m_pEffectiveShape;

    //This is the code that allows the computation of the correct effective shape.
#if 0
    if (m_pEffectiveShape == 0)
        {
        // The effective shape is not computed ...

        // Create an empty shape
        m_pEffectiveShape = new HVEShape(GetCoordSys());

        // Query all rasters in mosaic (all encompassing criterium)
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

        // For all rasters ... unify together their effective shape
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            m_pEffectiveShape->Unify(*(*Itr)->GetObject()->GetEffectiveShape());
            ++Itr;
            }

        // Finaly the global shape of all rasters contained herein is cliped to raster shape
        m_pEffectiveShape->Intersect(HRARaster::GetShape());
        }

    return m_pEffectiveShape;
#endif
    }

/** ---------------------------------------------------------------------------
    Get the list of URLs representing the source file composing this mosaic
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::GetSourceFileURLs(ListOfRelatedURLs& po_rRelatedURLs)
    {
    bool areAllSourceFileURLsAvailable = true;

    // Query all rasters in mosaic (all encompassing criterium)
    HAutoPtr<IndexType::IndexableList> pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    // For each raster gets the list of URL composing this raster
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        if ((*Itr)->GetObject()->GetSourceFileURLs(po_rRelatedURLs) == false)
            {
            areAllSourceFileURLsAvailable = false;
            }

        Itr++;
        }

    return areAllSourceFileURLsAvailable;
    }

/** ---------------------------------------------------------------------------
    Check if the a cache file can be used for the HIMOnDemandMosaic
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::IsSupportingCacheFile()
    {    
    return IsDataChangingWithResolution() == false;
    }


/** ---------------------------------------------------------------------------
    Check if the cache file for the HIMOnDemandMosaic is invalid
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::IsCacheFileUpToDate() 
    {
    bool IsValidCache = false;

    if ((HasCache() == true) &&
        (m_onDemandMosaicCacheInfoVersion == ON_DEMAND_RASTER_INFO_LATEST_VERSION))
    {                   
        HASSERT(m_pPSSCacheFileUrl != 0);

        HFCStat           CacheFileStat(m_pPSSCacheFileUrl);
        ListOfRelatedURLs SourceFileURLs;

        if (GetSourceFileURLs(SourceFileURLs) == true)
            {
            ListOfRelatedURLs::const_iterator URLIter    = SourceFileURLs.begin();
            ListOfRelatedURLs::const_iterator URLIterEnd = SourceFileURLs.end();

            while (URLIter != URLIterEnd)
                {
                HFCStat SourceFileStat(*URLIter);

                if (SourceFileStat.GetModificationTime() > CacheFileStat.GetModificationTime())
                    {
                    break;
            	    }         

                URLIter++;
        	    }             

            if (URLIter == URLIterEnd)
                {
            IsValidCache = true;
                }
            }
        }    

    return IsValidCache;
    }

/** ---------------------------------------------------------------------------
    Set a cache on the HIMOnDemandMosaic
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::SetCacheFile(const WString&                   pi_rCacheFileName,
                                     const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                                     HPMPool*                         pi_pMemoryPool)
    {
    bool isCacheFileSet = false;

    m_onDemandMosaicCacheInfoVersion = 0.0;

    HFCPtr<HFCURL>               pCacheFileURL(HFCURL::Instanciate(L"file://" + pi_rCacheFileName));

    if (HRFcTiffCreator::GetInstance()->IsKindOfFile(pCacheFileURL) == true)
        {
        HFCPtr<HRFRasterFile> pCacheFileForPSS(HRFcTiffCreator::GetInstance()->Create(pCacheFileURL));

        HASSERT(pCacheFileForPSS != 0);

        //The cache file is valid if information about the on-demand rasters are found.
        if (pCacheFileForPSS->GetPageDescriptor(0)->HasTag<HRFAttributeOnDemandRastersInfo>())
            {
            HFCPtr<HRSObjectStore> pStore;
            HFCPtr<HRARaster>     pSubResForPSS;
            HFCPtr<HGF2DCoordSys> pCacheFileCoordSys;

            pCacheFileCoordSys = pi_rpWorldCluster->GetCoordSysReference(pCacheFileForPSS->GetWorldIdentificator());

            pStore = new HRSObjectStore(pi_pMemoryPool,
                                        pCacheFileForPSS,
                                        0,
                                        pCacheFileCoordSys);

            // Get the raster from the store
            HFCPtr<HRARaster> pRaster(pStore->LoadRaster());

            HASSERT(pRaster->IsCompatibleWith(HRAPyramidRaster::CLASS_ID) == true);

            HFCPtr<HGF2DTransfoModel> pTransfoModel(pRaster->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));
            HFCPtr<HGF2DCoordSys> pDestinationCoordSys(new HGF2DCoordSys(*pTransfoModel, GetCoordSys()));
            m_pSubRes = new HRAReferenceToRaster(pRaster, pDestinationCoordSys);

            m_pPSSCacheFileUrl = pCacheFileURL;

            isCacheFileSet = true;
            }
        }

    return isCacheFileSet;
    }


/** ---------------------------------------------------------------------------
    Create a cache for the HIMOnDemandMosaic
    ---------------------------------------------------------------------------
*/
#define MAX_NB_CACHED_PIXELS        800000000
#define MIN_GREATEST_DIMENSION_SIZE 512.0 //The minimum size of the greatest dimension size (i.e. : MAX(width, height)) to be allow.

void HIMOnDemandMosaic::CreateCacheFile(const WString&                   		  pi_rCacheFileName, 
                                        const HFCPtr<HGF2DWorldCluster>& 		  pi_pWorldCluster, 
                                        HPMPool*                                  pi_pMemoryPool, 
                                        HRFDownSamplingMethod::DownSamplingMethod pi_DownSamplingMethod)
    {
	HPRECONDITION(IsDataChangingWithResolution() == FALSE);
    
    //All the raster files need to be loadable to ensure the integrity of the cache file.
    if (HasSomeRasterLastLoadFailed() == true)
        {
        //Should eventually have something more meaningful. 
        throw HFCFileNotFoundException(L"");  
        }

    //Verify that a cache is present and is valid                                
    if (IsCacheFileUpToDate() == false)
        {
        //This means that the cache file was found and more recent than the PSS file
        //but that one file composing the mosaic is more recent than the cache file.
        //Thus, remove the cache file from the mosaic to be sure not to use it during
        //the creation of the new cache file.
        if (HasCache() == true)
            {
            RemoveCache();                                               
            }
        }
    /*
        UInt64 mosaicHeightInPixels;
        UInt64 mosaicWidthInPixels;

        GetRasterSizeInPixel(pi_pWorldCluster, mosaicHeightInPixels, mosaicWidthInPixels);
    */
    HAutoPtr<HUTImportFromRasterExportToFile> pImportExport;

    //TDB - Pass the extent as the clip shape instead of the effective shape
    //      because the shape library is too slow for complex shape.                            
    pImportExport = new HUTImportFromRasterExportToFile(this,
                                                        this->GetExtent(),
                                                        pi_pWorldCluster);

    /*
    HASSERT(RasterHeightInPixels == pImportExport->GetImageHeight());
    HASSERT(RasterWidthInPixels == pImportExport->GetImageWidth());
      */

    //MST Hardcoded for now.
    double cacheSizeFactor = 0.1;

    /*  MST : Maybe that should be the default if no file name is provided
        HFCPtr<HFCURL> pDstFileName(HRFiTiffCacheFileCreator::GetInstance()->
                                            GetCacheURLFor((HFCPtr<HFCURL>&)pi_pSrcFilename, 0, 0));
                                            */

    HFCPtr<HFCURL> pDstFileName(HFCURL::Instanciate(L"file://" + pi_rCacheFileName));

    HASSERT((pDstFileName != 0) && (pDstFileName->IsCompatibleWith(HFCURLFile::CLASS_ID) == true));
                       
    pImportExport->SelectExportFileFormat(HRFcTiffCreator::GetInstance());
    pImportExport->SelectExportFilename(pDstFileName);
    pImportExport->SelectPixelType(HRPPixelTypeV32R8G8B8A8::CLASS_ID);

    HRFDownSamplingMethod downSamplingMethod(pi_DownSamplingMethod); 
    pImportExport->SelectDownSamplingMethod(downSamplingMethod); 	    

    int32_t blackTransparent = 0x00000000;
    pImportExport->SetRGBADefaultColor(&blackTransparent);
    pImportExport->SetBlendAlpha(true);    

    Utf8String onDemandRastersInfo;

    GetOnDemandRastersInfo(&onDemandRastersInfo);    

    HFCPtr<HPMGenericAttribute> pTag;

    pTag = new HRFAttributeOnDemandRastersInfo(string(onDemandRastersInfo.c_str()));

    pImportExport->SetTag(pTag);

    /*
        if (pi_pExportProperties->IsPSSCacheDimensionAPercentage == true)
        {
            CacheSizeFactor = pi_pExportProperties->PSSCacheDimension / 100.0;
        }
        else
        {
            UInt32  MaxDimension = MAX(pImportExport->GetImageWidth(),
                                       pImportExport->GetImageHeight());
            CacheSizeFactor = (double)pi_pExportProperties->PSSCacheDimension / MaxDimension;
        }
    */
    uint32_t imageWidth;
    uint32_t imageHeight; 

    if (MAX(pImportExport->GetImageHeight(), pImportExport->GetImageWidth()) < MIN_GREATEST_DIMENSION_SIZE)
        {
        //Don't create a cache greater then the 1:1 representation of the mosaic itself.
        imageWidth = pImportExport->GetImageWidth();
        imageHeight = pImportExport->GetImageHeight();
        }
    else
        {
        imageWidth  = MAX((int32_t)(pImportExport->GetImageWidth() * cacheSizeFactor), 1);
        imageHeight = MAX((int32_t)(pImportExport->GetImageHeight() * cacheSizeFactor), 1);                    
        
        double limitAdjustementFactor = 1.0;
        
        if (MAX(imageHeight, imageWidth) < MIN_GREATEST_DIMENSION_SIZE)
            {
            if (imageHeight > imageWidth)
                {
                limitAdjustementFactor = MIN_GREATEST_DIMENSION_SIZE / (double)imageHeight;
                }
            else
                {
                limitAdjustementFactor = MIN_GREATEST_DIMENSION_SIZE / (double)imageWidth;
                }
            }   
		else        
        if (MAX_NB_CACHED_PIXELS < imageWidth * imageHeight)
            {   //cTIFF is currently a 32 bits file format, so limit the number of pixels cached. 
            limitAdjustementFactor = sqrt((double)MAX_NB_CACHED_PIXELS / (imageWidth * imageHeight));    
            }

        if (limitAdjustementFactor != 1.0)
            {
            imageWidth = (int32_t)(imageWidth * limitAdjustementFactor);
            imageHeight = (int32_t)(imageHeight * limitAdjustementFactor);
            }
        }

    pImportExport->SetImageWidth(imageWidth);
    pImportExport->SetImageHeight(imageHeight);

    /*
    HAutoPtr<ExportProgressListener> pProgressListener;
    if (pi_pExportProperties->FeedbackOn)
        pProgressListener = new ExportProgressListener();

    bool ListenerAdded = false;
    */
    
    /*MST : The listener should probably be passed as a parameter.
    HUTExportProgressIndicator::GetInstance()->AddListener(pProgressListener);
    HRADrawProgressIndicator::GetInstance()->AddListener(pProgressListener);
        ListenerAdded = TRUE;
    */                      

    pImportExport->StartExport();
        
    

    /*
    if (pi_pExportProperties->FeedbackOn)
    {
        // Remove the progression trace
            _tprintf(_TEXT("                                                                    \r"));               
        HUTExportProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
        HRADrawProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
        }*/ 

    if (HasSomeRasterLastLoadFailed() == false)
        {
        bool cacheFileIsSet = SetCacheFile(pi_rCacheFileName, pi_pWorldCluster, pi_pMemoryPool);

        HASSERT(cacheFileIsSet == true);

        m_CacheFileDownSamplingMethod = pi_DownSamplingMethod;
        m_onDemandMosaicCacheInfoVersion = ON_DEMAND_RASTER_INFO_LATEST_VERSION;             
        }
    else
        {
        BeFileName::BeDeleteFile(pi_rCacheFileName.c_str());

        HASSERT(!BeFileName::DoesPathExist(pi_rCacheFileName.c_str()));

        //Should eventually have something more meaningful. 
        throw HFCFileNotFoundException(L"");  
        }       
    }

/** ---------------------------------------------------------------------------
    Create the PSS file.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::CreatePssFile(const WString& pi_rFileName) const
    {
    // Query all rasters in mosaic (all encompassing criterium)
    HAutoPtr<IndexType::IndexableList> pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    // Should not be called if the mosaic is empty.
    HASSERT(pObjects->size() > 0);

    // For each raster gets the list of URL composing this raster
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    WString representativePSS;
    size_t  pageStatementPos;
    size_t  openParenthesisPos;        

    WString onDemandMosaicStatement(L"m0 = ODMO(");
    WString rasterPSSName;

    HFCLocalBinStream localBinStream(pi_rFileName, HFC_READ_WRITE_CREATE);

    while (Itr != pObjects->end())
        {
        representativePSS = (*Itr)->GetObject()->GetRepresentativePSS();

        pageStatementPos = representativePSS.find(L"PAGE");

        if (pageStatementPos == string::npos)
            {
            pageStatementPos = representativePSS.find(L"PG");
            }

        HASSERT(pageStatementPos != string::npos);

        //Get the raster name.
        openParenthesisPos = representativePSS.find(L"(", pageStatementPos);

        rasterPSSName = representativePSS.substr(openParenthesisPos + 1);

        rasterPSSName.erase(rasterPSSName.find_last_not_of(L" )\n\r\t")+1);

        //Remove the page statement
        representativePSS = representativePSS.substr(0, pageStatementPos);
                                                                                  
        onDemandMosaicStatement += rasterPSSName + L",";

        //Write the representative PSS of the image.                
        representativePSS += L"\r\n";
        
        // PSS is UTF8.
        Utf8String representativePSS_UTF8(representativePSS.c_str());
        size_t nbCharsWritten = localBinStream.Write(representativePSS_UTF8.c_str(), representativePSS_UTF8.size());

        HASSERT(nbCharsWritten == representativePSS_UTF8.size());

        Itr++;
        }
    
    //Write the world descriptive PSS. 
    if (m_WorldDescriptivePSS.empty() == false)
        {
        WString worldDescriptivePSS = m_WorldDescriptivePSS + L"\r\n";
        
        // PSS is UTF8.
        Utf8String worldDescriptivePSS_UTF8(worldDescriptivePSS.c_str());
        size_t nbCharsWritten = localBinStream.Write(worldDescriptivePSS_UTF8.c_str(), worldDescriptivePSS_UTF8.size());

        HASSERT(nbCharsWritten == worldDescriptivePSS_UTF8.size());                    
        }
     
    //Remove the ,
    onDemandMosaicStatement = onDemandMosaicStatement.substr(0, onDemandMosaicStatement.size() - 1);

    onDemandMosaicStatement += L")\r\nPAGE(m0)";

    // PSS is UTF8
    Utf8String onDemandMosaicStatement_UTF8(onDemandMosaicStatement.c_str());
    size_t nbCharsWritten = localBinStream.Write(onDemandMosaicStatement_UTF8.c_str(), onDemandMosaicStatement_UTF8.size());

    HASSERT(nbCharsWritten == onDemandMosaicStatement_UTF8.size());
    }

/** ---------------------------------------------------------------------------
    Get the raster list.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::GetRasterList(HIMOnDemandMosaic::RasterList& po_rRasters) const
    {
    // Query all rasters in mosaic (all encompassing criterium)
    HAutoPtr<IndexType::IndexableList> pOnDemandRasters(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    IndexType::IndexableList::iterator onDemandRasterIter(pOnDemandRasters->begin());
    IndexType::IndexableList::iterator onDemandRasterIterEnd(pOnDemandRasters->end());

    while (onDemandRasterIter != onDemandRasterIterEnd)
        {
        po_rRasters.push_back((*onDemandRasterIter)->GetObject());
        onDemandRasterIter++;
        }
    }

/** ---------------------------------------------------------------------------
    Remove all images from the mosaic
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::RemoveAll()
    {
    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Unlink from all rasters
    UnlinkFromAll();

    delete m_pIndex;
    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    // We are the universe!
//    SetShape(HVEShape(HVE2DUniverse(GetCoordSys())));

    RecalculateEffectiveShape();

    m_RecentlyUsedRasters.clear();

    m_pSubRes = 0;
    m_onDemandMosaicCacheInfoVersion = 0.0;

    //Delete the cache file.
    if (m_pPSSCacheFileUrl != 0)
        {
        HASSERT(m_pPSSCacheFileUrl->IsCompatibleWith(HFCURLFile::CLASS_ID) == true);

        BeFileName::BeDeleteFile(((HFCURLFile*)m_pPSSCacheFileUrl.GetPtr())->GetAbsoluteFileName().c_str());
        }

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }



/** ---------------------------------------------------------------------------
    GetOnDemandRastersInfo

    Information : 
    
        <!--<EffectiveShape></EffectiveShape>-->
        <MinimumPixelSize xmin="" xmax="" ymin="" ymax=""/>                    
        <MaximumPixelSize xmin="" xmax="" ymin="" ymax=""/>
        <RepresentativePSSForWorld></RepresentativePSSForWorld>
        <OnDemandRasterList>
            <!--Info for each OnDemandRaster-->
        </OnDemandRasterList>
    --------------------------        
**/
void HIMOnDemandMosaic::GetOnDemandRastersInfo(Utf8String* po_pOnDemandRastersInfo) const
    {
    HPRECONDITION(po_pOnDemandRastersInfo->empty());

        
    //XMLDocument    onDemandMosaicXMLInfo;
    BeXmlDomPtr pOnDemandMosaicXMLInfo(BeXmlDom::CreateEmpty());
            
    BeXmlNodeP pRootNode = pOnDemandMosaicXMLInfo->AddNewElement("OnDemandMosaic", 0, 0);
       
    WChar doubleValueBuffer[DOUBLE_VALUE_BUFFER_LENGTH];
    WChar format[DOUBLE_FORMATTING_BUFFER_LENGTH];

    GetDoubleFormatting(format, DOUBLE_FORMATTING_BUFFER_LENGTH);
    
    //Precision lost with the AddAttributeDoubleValue, so we are doing our own conversion.
    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, ON_DEMAND_RASTER_INFO_LATEST_VERSION);       
    pRootNode->AddAttributeStringValue("version", doubleValueBuffer);
                                       
    //onDemandMosaicXMLInfo.AddNode(*pOnDemandMosaicNode);
                                   
    if ((m_pMinimumPixelSizeRange == 0) || (m_pMaximumPixelSizeRange == 0))
        {
        //Get pixel size from current source
        bool              initialized = false;
        double            minimumArea;
        double            maximumArea;
        HGF2DExtent       minimum;
        HGF2DExtent       maximum;
        HFCPtr<HRARaster> pRaster;

        pRaster = (HRARaster*)this;

        HIMOnDemandMosaic::GetPixelSizeRange(pRaster,
                                             GetCoordSys(),
                                             initialized,
                                             minimumArea,
                                             maximumArea,
                                             minimum,
                                             maximum);

        m_pMinimumPixelSizeRange = new HGF2DExtent(minimum);
        m_pMaximumPixelSizeRange = new HGF2DExtent(maximum);

        m_pMinimumPixelSizeRange->ChangeCoordSys(GetCoordSys());
        m_pMaximumPixelSizeRange->ChangeCoordSys(GetCoordSys());
        }

    //Minimum pixel size range
    BeXmlNodeP pODMOMinPixelSizeNode = pRootNode->AddEmptyElement("MinimumPixelSize");

    HASSERT(0 != pODMOMinPixelSizeNode);        

    HGF2DExtent extentToSerialize(*m_pMinimumPixelSizeRange);

    extentToSerialize.ChangeCoordSys(GetCoordSys());    
    
    /*MST - Check
    pODMOMinPixelSizeNode->AddAttribute("xMin", extentToSerialize.GetXMin(), ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS);
    pODMOMinPixelSizeNode->AddAttribute("xMax", extentToSerialize.GetXMax(), ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS);
    pODMOMinPixelSizeNode->AddAttribute("yMin", extentToSerialize.GetYMin(), ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS);
    pODMOMinPixelSizeNode->AddAttribute("yMax", extentToSerialize.GetYMax(), ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS);
    */
           
    //Precision lost with the AddAttributeDoubleValue, so we are doing our own conversion.
    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetXMin());             
    pODMOMinPixelSizeNode->AddAttributeStringValue("xMin", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetXMax());             
    pODMOMinPixelSizeNode->AddAttributeStringValue("xMax", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetYMin());    
    pODMOMinPixelSizeNode->AddAttributeStringValue("yMin", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetYMax());    
    pODMOMinPixelSizeNode->AddAttributeStringValue("yMax", doubleValueBuffer);
    
    //Maximum pixel size range
    BeXmlNodeP pODMOMaxPixelSizeNode = pRootNode->AddEmptyElement("MaximumPixelSize");

    HASSERT(0 != pODMOMaxPixelSizeNode);        

    extentToSerialize = *m_pMaximumPixelSizeRange;

    extentToSerialize.ChangeCoordSys(GetCoordSys());    

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetXMin());             
    pODMOMaxPixelSizeNode->AddAttributeStringValue("xMin", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetXMax());             
    pODMOMaxPixelSizeNode->AddAttributeStringValue("xMax", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetYMin());             
    pODMOMaxPixelSizeNode->AddAttributeStringValue("yMin", doubleValueBuffer);

    swprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, extentToSerialize.GetYMax());             
    pODMOMaxPixelSizeNode->AddAttributeStringValue("yMax", doubleValueBuffer);
    
    //Representative PSS of world statements serialized in UTF8 encoding.
    Utf8String utf8Str;

    if (m_WorldDescriptivePSS.size() > 0)
        {            	
        pRootNode->AddElementStringValue("RepresentativePSSForWorld", m_WorldDescriptivePSS.c_str());           
        }
	                   
    //Representative PSS of each raster serialized in UTF8 encoding.        
    BeXmlNodeP pODMORasterListNode = pRootNode->AddEmptyElement("OnDemandRasterList");        

    HASSERT(pODMORasterListNode != 0);

    HAutoPtr<IndexType::IndexableList>       pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator OnDemandRasterItr(pObjects->begin());
    IndexType::IndexableList::const_iterator OnDemandRasterItrEnd(pObjects->end());
               
    while (OnDemandRasterItr != OnDemandRasterItrEnd)
        {        
        (*OnDemandRasterItr)->GetObject()->GetSerializationXMLNode(GetCoordSys(), pODMORasterListNode);
                                              
        OnDemandRasterItr++;
        }    

    pOnDemandMosaicXMLInfo->ToString(*po_pOnDemandRastersInfo, BeXmlDom::TO_STRING_OPTION_Default);
    
    HASSERT(po_pOnDemandRastersInfo->empty() == FALSE);        
    }

/** ---------------------------------------------------------------------------
    Get the mosaic's extent
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMOnDemandMosaic::GetExtent() const
    {
    if (m_pEffectiveShape == 0)
        {
        // Avoid computing the effective shape, if may be a very long task
        return GetExtentInCs(GetCoordSys());
        }
    else
        {
        // The effective shape is already computed, uses it directly
        return m_pEffectiveShape->GetExtent();
        }
    }

/** ---------------------------------------------------------------------------
    Get the mosaic's extent in a specified CS
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMOnDemandMosaic::GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const
    {
    // We can optimise the computation of the extent only if the mosaic is not shaped
    if (m_pEffectiveShape == 0 && GetShape().GetShapePtr()->IsCompatibleWith(HVE2DUniverse::CLASS_ID))
        {
        // The effective shape is alrread computed ...
        if (m_pEffectiveExtent != 0)
            return *m_pEffectiveExtent;

        // Create an empty shape
        m_pEffectiveExtent = new HGF2DExtent(pi_pCoordSys);

        // Query all rasters in mosaic (all encompassing criterium)
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

        // For all rasters ... unify together their effective shape
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            // Take a copy of the raster's shape.
            HFCPtr<HVEShape> pTmpShape = new HVEShape(*(*Itr)->GetObject()->GetEffectiveShape());

            // Bring it into the provided coordsys
            pTmpShape->ChangeCoordSys(pi_pCoordSys);

            m_pEffectiveExtent->Add(pTmpShape->GetExtent());
            ++Itr;
            }

        return *m_pEffectiveExtent;
        }

    // Call the ancestor
    return HRARaster::GetExtentInCs(pi_pCoordSys);
    }

/** ---------------------------------------------------------------------------
    Tell if mosaic contains pixels of the specified channel
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                                  Byte                        pi_Id) const
    {
    bool                                     ContainsSome = false;
    HAutoPtr<IndexType::IndexableList>       pObjects(m_pIndex->
                                                      GetIndex1()->
                                                      QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    HFCPtr<HRARaster> pRaster;

    // Pass through all sources (until we find a source using the channel...)
    while (!ContainsSome && Itr != pObjects->end())
        {
        // Ask source if it has pixels of specified channel
        pRaster = GetRaster((*Itr)->GetObject());

        if (pRaster != 0)
            {
            ContainsSome = pRaster->ContainsPixelsWithChannel(pi_Role, pi_Id);
            }

        ++Itr;
        }

    return ContainsSome;
    }

/** ---------------------------------------------------------------------------
    Create an editor.
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMOnDemandMosaic::CreateEditor(HFCAccessMode pi_Mode)
    {
    return 0;
    }

/** ---------------------------------------------------------------------------
    Create a shaped editor
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMOnDemandMosaic::CreateEditor(const HVEShape& pi_rShape,
                                                 HFCAccessMode   pi_Mode)
    {
    return 0;
    }

/** ---------------------------------------------------------------------------
    Create an unshaped editor
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMOnDemandMosaic::CreateEditorUnShaped(HFCAccessMode pi_Mode)
    {
    return 0;
    }

/** ---------------------------------------------------------------------------
    Check if the mosaic supports the lookahead mechanism.
    @return true if at least one image supports the lookahead.
    @see HIMOnDemandMosaic::SetLookAhead
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::HasLookAhead() const
    {
    if (m_hasLookAhead == HIMONDEMANDMOSAIC_BOOL_UNKNOWN)
        {
        m_hasLookAhead = HIMONDEMANDMOSAIC_BOOL_FALSE;
              
        // try to find at least one LookAheadable source
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        
        while ((m_hasLookAhead == HIMONDEMANDMOSAIC_BOOL_FALSE) && Itr != pObjects->end())
            {            
            // Get the LookAhead capability of the current element
            if ((*Itr)->GetObject()->HasLookAhead())
                m_hasLookAhead = HIMONDEMANDMOSAIC_BOOL_TRUE;

            // Proceed to the next element
			Itr++;
            }        
        }

    return (m_hasLookAhead == HIMONDEMANDMOSAIC_BOOL_TRUE);
    }
        
bool HIMOnDemandMosaic::HasUnlimitedRasterSource() const
    {       
    if (m_hasUnlimitedRasterSource == HIMONDEMANDMOSAIC_BOOL_UNKNOWN)
        {
        m_hasUnlimitedRasterSource = HIMONDEMANDMOSAIC_BOOL_FALSE;
              
        // try to find at least one LookAheadable source
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        
        while ((m_hasUnlimitedRasterSource == HIMONDEMANDMOSAIC_BOOL_FALSE) && Itr != pObjects->end())
            {                       
            // Get the LookAhead capability of the current element
            if ((*Itr)->GetObject()->HasUnlimitedRasterSource())
                m_hasUnlimitedRasterSource = HIMONDEMANDMOSAIC_BOOL_TRUE;

            // Proceed to the next element
            ++Itr;
            }        
        }

    return (m_hasUnlimitedRasterSource == HIMONDEMANDMOSAIC_BOOL_TRUE);
    }

bool HIMOnDemandMosaic::IsDataChangingWithResolution() const
    {
    if (m_isDataChangingWithResolution == HIMONDEMANDMOSAIC_BOOL_UNKNOWN)
        {
        m_isDataChangingWithResolution = HIMONDEMANDMOSAIC_BOOL_FALSE;
              
        // try to find at least one LookAheadable source
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        
        while ((m_isDataChangingWithResolution == HIMONDEMANDMOSAIC_BOOL_FALSE) && Itr != pObjects->end())
            {            
            // Get the LookAhead capability of the current element
            if ((*Itr)->GetObject()->IsDataChangingWithResolution())
                m_isDataChangingWithResolution = HIMONDEMANDMOSAIC_BOOL_TRUE;

            // Proceed to the next element
            ++Itr;
            }        
        }

    return (m_isDataChangingWithResolution == HIMONDEMANDMOSAIC_BOOL_TRUE);
    }


/** ---------------------------------------------------------------------------
    Set a lookahead shape to the mosaic. It will be given to every image
    that supports the lookahead mechanism.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::SetLookAhead(const HVEShape& pi_rShape,
                                     uint32_t       pi_ConsumerID,
                                     bool            pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    // Take the mosaic's effective shape
    HFCPtr<HVEShape> pLookAheadShape(new HVEShape(*GetEffectiveShape()));

    // Intersect with source's effective shape
    pLookAheadShape->Intersect(pi_rShape);

    if (pLookAheadShape->IsEmpty() == false) 
        {
        // Set the lookahead of images that are in the region.
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(HIDXSearchCriteria(m_pIndex->GetIndex2(), 
                                                                                new HGFSpatialCriteria(pLookAheadShape->GetExtent())), true));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());    

        HFCPtr<HRARaster> pRaster;

        while (Itr != pObjects->end())
            {                    
            // Get the LookAhead capability of the current element
            if ((*Itr)->GetObject()->HasLookAhead() == true)
                {
                pRaster = GetRaster((*Itr)->GetObject());
              
                // Calculate the needed visible part of this image
                HVEShape VisibleShape(pi_rShape);
                VisibleShape.Intersect(HRARaster::GetShape());
                VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));

                // Send part to the image, if there is one...                
                if (!VisibleShape.IsEmpty())
                    pRaster->SetLookAhead(VisibleShape, pi_ConsumerID, pi_Async);                    
                }

            // Proceed to the next element
            ++Itr;
            }   
        }
    }

/** ---------------------------------------------------------------------------
    Receive an "effective shape change" notification from one of our images.
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage)
    {
    //This notification isn't supported.
    HASSERT(0);

    // Propagate message
    return true;
    }

/** ---------------------------------------------------------------------------
    Receive a "geometry changed" notification from one of our images.
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    //This notification isn't supported.
    HASSERT(0);

    // Stop the message here!
    return false;
    }

/** ---------------------------------------------------------------------------
    Compute the size of the mosaic in pixels based on the
    HUTImportFromRasterExportToFile constructor.
    ---------------------------------------------------------------------------
*/
/*
void HIMOnDemandMosaic::GetRasterSizeInPixel(const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster,
                                            UInt64&                          po_rHeight,
                                            UInt64&                          po_rWidth)
{
    // A cluster must be provided
    HPRECONDITION(pi_pWorldCluster != 0);

    HFCPtr<HVEShape> pEffectiveShape(GetEffectiveShape());

    HASSERT(!pEffectiveShape->IsEmpty());

    HGF2DPosition OriginalSize;
    bool          SourceHasTransfo = true;
    HVEShape      ClipShape(*pEffectiveShape);

    HIMStoredRasterEquivalentTransfo SRETransfo(this);
    if (SRETransfo.EquivalentTransfoCanBeComputed())
    {
        HVEShape ClipShapeOrig(*pEffectiveShape);
        SRETransfo.TransformLogicalShapeIntoPhysical(ClipShapeOrig);

        HFCGrid OriginalGrid(0.0,
                             0.0,
                             ClipShapeOrig.GetExtent().GetWidth(),
                             ClipShapeOrig.GetExtent().GetHeight());

        // Set Original size in HRFImportExport
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetWidth())
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetHeight())

        OriginalSize.SetX((double)OriginalGrid.GetWidth());
        OriginalSize.SetY((double)OriginalGrid.GetHeight());
    }

    // Calculate the resample size and scale factor.
    ClipShape.ChangeCoordSys(pi_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
    HGF2DExtent TmpExtentMin;
    HGF2DExtent TmpExtentMax;
    GetPixelSizeRange(TmpExtentMin, TmpExtentMax);

    // The minimum pixel size must be defined and its width and height must be greater than 0.0
    HASSERT(TmpExtentMin.IsDefined());
    HASSERT(TmpExtentMin.GetWidth() != 0.0);
    HASSERT(TmpExtentMin.GetHeight() != 0.0);

    HGF2DExtent PixelSize(TmpExtentMin.CalculateApproxExtentIn(pi_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)));

    // PixelSize Area.
    double DefaultResampleScaleFactor = MIN(PixelSize.GetWidth(), PixelSize.GetHeight());

    // The pixel size may not be 0.0 (exact compare)
    HASSERT(DefaultResampleScaleFactor != 0.0);

    // ScaleFactor
    DefaultResampleScaleFactor = 1.0 / DefaultResampleScaleFactor;

    // Exceptionally, we specify a precision. We don't want to create
    // pixels that are not useful. 0.01 is quite arbitrary ;-)
    HFCGrid ResampleGrid(0.0,
                         0.0,
                         ClipShape.GetExtent().GetWidth()  * DefaultResampleScaleFactor,
                         ClipShape.GetExtent().GetHeight() * DefaultResampleScaleFactor,
                         0.01);

    // Set resample size in HRFImportExport.
    po_rWidth = ResampleGrid.GetWidth();
    po_rHeight = ResampleGrid.GetHeight();
}
*/

/** ---------------------------------------------------------------------------
    Check if all raster can be loaded.
    ---------------------------------------------------------------------------
*/
bool HIMOnDemandMosaic::HasSomeRasterLastLoadFailed()
    {
    bool isSomeSourceNotLoadable = false;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    if (pObjects->size() > 0)
        {
        HGF2DExtent CurrentPixelSize;
        HFCPtr<HRARaster> pRaster;

        while (Itr != pObjects->end())
            {
            if ((*Itr)->GetObject()->HasLastLoadFailed() == true)
                {
                isSomeSourceNotLoadable = true;
                break;
                }                  

            ++Itr;
            }
        }

    return isSomeSourceNotLoadable;
    }

/** ---------------------------------------------------------------------------
    Unlink from all rasters composing the mosaic
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::UnlinkFromAll(void)
    {
    RURasterList::iterator pRasterIter    = m_RecentlyUsedRasters.begin();
    RURasterList::iterator pRasterIterEnd = m_RecentlyUsedRasters.end();

    // Pass through all list
    while (pRasterIter != pRasterIterEnd)
        {
        // Unlink ourselves
        UnlinkFrom((*pRasterIter).m_pRaster);
        pRasterIter++;
        }
    }

/** ---------------------------------------------------------------------------
    Move the mosaic. To accomplish this, the mosaic's source images will
    be moved.
    @param pi_rDisplacement The translation to apply, in the mosaic logical
    coordinate system.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    //This operation isn't supported.
    HASSERT(0);
    }

/** ---------------------------------------------------------------------------
    Rotate the mosaic. To accomplish this, the mosaic's source images will
    be rotated.
    @param pi_Angle The angle of rotation, relative to the mosaic's
    logical coordinate system.
    @param pi_rOrigin The rotation's origin point, also in logical system.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin)
    {
    //This operation isn't supported.
    HASSERT(0);
    }

/** ---------------------------------------------------------------------------
    Scale the mosaic. To accomplish this, the mosaic's source images will
    be scaled.
    @param pi_ScaleFactorX The scale on the X axis, relative to the mosaic's
    logical coordinate system.
    @param pi_ScaleFactorY The scale on the Y axis, relative to the mosaic's
    logical coordinate system.
    @param pi_rOrigin The scale's origin point, also in logical system.
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::Scale(double              pi_ScaleFactorX,
                              double              pi_ScaleFactorY,
                              const HGF2DLocation& pi_rOrigin)
    {
    //This operation isn't supported.
    HASSERT(0);
    }

/** ---------------------------------------------------------------------------
    Called when some change necessitates a recalculation of the effective shape
    ---------------------------------------------------------------------------
*/
void HIMOnDemandMosaic::RecalculateEffectiveShape ()
    {
    m_pEffectiveShape  = 0;
    m_pEffectiveExtent = 0;
    }

/** ---------------------------------------------------------------------------
    Draw the mosaic
    ---------------------------------------------------------------------------*/
static bool s_multiThreadTest = false;


void HIMOnDemandMosaic::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    bool                  DrawDone     = false;
    HRADrawOptions        Options(pi_Options);
    HFCPtr<HGF2DCoordSys> pReplacingCS(Options.GetReplacingCoordSys());
    HVEShape              RegionToDraw(Options.GetShape() != 0 ? *Options.GetShape() : HRARaster::GetShape());
    double                SubResPyramidRes = DBL_MAX;

    //TR 300554 - Temporary fix activation.
    Options.SetDataDimensionFix(true);

    HFCPtr<HGSRegion> pClipRegion(pio_destSurface.GetRegion());

    if (pClipRegion != 0)
        {
        // Intersect it with the destination
        HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());

        if (Options.GetReplacingCoordSys())
            {
            pSurfaceShape->ChangeCoordSys(Options.GetReplacingCoordSys());
            pSurfaceShape->SetCoordSys(GetCoordSys());
            }

        RegionToDraw.Intersect(*pSurfaceShape);
        }
    else
        {
        // Create a rectangular clip region to stay
        // inside the destination surface.
        HVEShape DestSurfaceShape(0.0, 0.0, pio_destSurface.GetSurfaceDescriptor()->GetWidth(), pio_destSurface.GetSurfaceDescriptor()->GetHeight(), pio_destSurface.GetSurfaceCoordSys());

        // Set the stroking tolerance for the surface's shape
        // Set a quarter of a pixel tolerance
        double CenterX = pio_destSurface.GetSurfaceDescriptor()->GetWidth() / 2.0;
        double CenterY = pio_destSurface.GetSurfaceDescriptor()->GetHeight() / 2.0;
        HFCPtr<HGFTolerance> pTol = new HGFTolerance (CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                      pio_destSurface.GetSurfaceCoordSys());

        if (Options.GetReplacingCoordSys())
            {
            DestSurfaceShape.ChangeCoordSys(Options.GetReplacingCoordSys());
            DestSurfaceShape.SetCoordSys(GetCoordSys());
            }

        RegionToDraw.Intersect(DestSurfaceShape);
        }


    if (m_pSubRes != 0)
        {
        HASSERT(IsDataChangingWithResolution() == false);

        SubResPyramidRes = ((HFCPtr<HRAPyramidRaster>&)m_pSubRes->GetSource())->FindTheBestResolution(pio_destSurface.GetCoordSys(),
                                                                                                      pReplacingCS);

        // If the resolution is equal or smaller than the cached sub resolution, use the cached
        // sub-resolution to draw.
        if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(SubResPyramidRes, 1.0))
            {
            // Calculate the needed visible part of this image
            HFCPtr<HVEShape> pVisibleShape(new HVEShape(RegionToDraw));
            pVisibleShape->Intersect(*m_pSubRes->GetEffectiveShape());

            if (pReplacingCS != 0)
                {
                // Adapt replacing CS so that it works on the current image.

                // Extract the transformation we're applying to our source (T1)
                HFCPtr<HGF2DTransfoModel> pRefTransfo(pReplacingCS->GetTransfoModelTo(GetCoordSys()));

                // Calculate the transformation between our source's CS and the CS
                // the object to return uses. (T2)
                HFCPtr<HGF2DTransfoModel> pCurrentToSource(m_pSubRes->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

                // Create the CS by composing everything correctly :)
                // T2 o T1 o -T2
                HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
                HFCPtr<HGF2DCoordSys> pCSToApply = new HGF2DCoordSys(*pToApply, m_pSubRes->GetCoordSys());

                Options.SetReplacingCoordSys(pCSToApply);
                }

            Options.SetShape(pVisibleShape);
            m_pSubRes->Draw(pio_destSurface, Options);
            DrawDone = true;
            }
        }
    
    // Standard draw code, one image at a time, bottom up
    if (!DrawDone)
        {
        // Take a copy of the shape to avoid modifying the original shape
        HVEShape RegionToDrawInCs(RegionToDraw);
        RegionToDrawInCs.ChangeCoordSys(GetCoordSys());

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                          HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(RegionToDrawInCs.GetExtent())), true));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        HFCPtr<HRARaster> pRaster;

        while (Itr != pObjects->end())
            {
            // Calculate the needed visible part of this image
            HFCPtr<HVEShape> pVisibleShape(new HVEShape(RegionToDraw));

            if (pVisibleShape->GetExtent().DoTheyOverlap((*Itr)->GetObject()->GetExtent()) == true)
                {
                pVisibleShape->Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));

                // Send part to the image, if there is one...
                if (!pVisibleShape->IsEmpty())
                    {
                    pRaster = GetRaster((*Itr)->GetObject());

                    if (pRaster == 0)
                        {
                        Itr++;
                        continue;
                        }

                    if (pReplacingCS != 0)
                        {
                        // Adapt replacing CS so that it works on the current image.

                        // Extract the transformation we're applying to our source (T1)
                        HFCPtr<HGF2DTransfoModel> pRefTransfo(pReplacingCS->GetTransfoModelTo(GetCoordSys()));

                        // Calculate the transformation between our source's CS and the CS
                        // the object to return uses. (T2)
                        HFCPtr<HGF2DTransfoModel> pCurrentToSource(pRaster->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

                        // Create the CS by composing everything correctly :)
                        // T2 o T1 o -T2
                        HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
                        HFCPtr<HGF2DCoordSys> pCSToApply = new HGF2DCoordSys(*pToApply, pRaster->GetCoordSys());

                        Options.SetReplacingCoordSys(pCSToApply);
                        }

                    Options.SetShape(pVisibleShape);
                    pRaster->Draw(pio_destSurface, Options);
                    }
                }

            // Proceed to the next element
            ++Itr;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMOnDemandMosaic::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    // Take a copy of the shape to avoid modifying the original shape
    HVEShape RegionToDrawInCs(*options.GetShape());

    // If a replacing CS is provided, we must move the region to draw since this region does not represents the real position of the mosaic.
    // The replacing CS is a replacement of the mosaic logical CS. It represents the position of the mosaic as seen by the destination.
    if (options.GetReplacingCoordSys())
        {
        // Express the region to draw in the replacing CS. Then set the CS of the mosaic. 
        // This will move the region to draw at the right location in the mosaic CS.
        RegionToDrawInCs.ChangeCoordSys(options.GetReplacingCoordSys());
        RegionToDrawInCs.SetCoordSys(GetCoordSys());
        }
    else
        {
        RegionToDrawInCs.ChangeCoordSys(GetCoordSys());
        }

    HGF2DExtent enclosingShape(GetCoordSys());
    std::vector< pair< HFCPtr<HRARaster>, HFCPtr<HVEShape> > > enclosedRasters;

    // Try to use sub-res
    if (m_pSubRes != 0)
        {
        HASSERT(IsDataChangingWithResolution() == false);

        double SubResPyramidRes = ((HFCPtr<HRAPyramidRaster>&)m_pSubRes->GetSource())->FindTheBestResolution(imageNode.GetPhysicalCoordSys(), options.GetReplacingCoordSys());

        // If the resolution is equal or smaller than the cached sub resolution, use the cached
        // sub-resolution to draw.
        if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(SubResPyramidRes, 1.0))
            {
            HFCPtr<HRARaster> pSrcRaster(m_pSubRes.GetPtr());
            HFCPtr<HVEShape> pVisibleSurface = pSrcRaster->GetEffectiveShape();

            if (RegionToDrawInCs.HasIntersect(*pVisibleSurface))
                {
                enclosedRasters.push_back(make_pair(pSrcRaster, pVisibleSurface));
                enclosingShape.Add(pVisibleSurface->GetExtent());
                }
            }
        }

    if (enclosedRasters.empty())
        {
        // Query for visible items
        HAutoPtr<IndexType::IndexableList> pObjects(m_pIndex->QueryIndexables(
            HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(RegionToDrawInCs.GetExtent())), true));

        // Fill the list of rasters that intersect the region to draw
        for (IndexType::IndexableList::const_iterator Itr(pObjects->begin()); Itr != pObjects->end(); ++Itr)
            {
            HFCPtr<HRARaster> pSrcRaster = GetRaster((*Itr)->GetObject());
            if (pSrcRaster == 0)
                continue;
            HFCPtr<HVEShape> pVisibleSurface = m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr);

            if (RegionToDrawInCs.HasIntersect(*pVisibleSurface))
                enclosedRasters.push_back(make_pair(pSrcRaster, pVisibleSurface));
            }

        // Set the enclosing shape as the effective shape's extent. 
        // This may be much larger than the extent of the rasters intersecting the given region to draw.
        // HIMOnDemandMosaic::GetEffectiveShape returns the extent of the mosaic, not the real shape. 
        // When the mosaic sample is copied into the destination, the mosaic's shape (extent) is used to
        // compute the scanlines. This possibly results in more pixels than the effective mosaic can produce and clamping can occur.
        // To avoid clamping, we set the effective shape's extent as the shape of the mosaic node. 
        enclosingShape.Add(GetEffectiveShape()->GetExtent());
        }

    // Intersect with the region to draw
    enclosingShape.Intersect(RegionToDrawInCs.GetExtent());

    // If a replacing CS is provided, move the shape of visible rasters.
    if (options.GetReplacingCoordSys())
        {
        enclosingShape.ChangeCoordSys(GetCoordSys());
        enclosingShape.SetCoordSys(options.GetReplacingCoordSys());
        }

    // Express the shape in the destination node physical CS.
    enclosingShape.ChangeCoordSys(imageNode.GetPhysicalCoordSys());

    if (!enclosingShape.IsDefined())
        return IMAGEPP_STATUS_Success;

    // Take replacing pixeltype into account.
    
    BeAssert(options.GetReplacingPixelType() == NULL || options.GetReplacingPixelType()->CountPixelRawDataBits() == GetPixelType()->CountPixelRawDataBits());
    HFCPtr<HRPPixelType> pEffectivePixelType = (options.GetReplacingPixelType() != NULL) ? options.GetReplacingPixelType() : GetPixelType();

    // If we ware working with binary data we prefer RLE format.
    if (pEffectivePixelType->CountPixelRawDataBits() == 1)
        {
        //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
        HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(pEffectivePixelType);
        if (pRlePixelType == NULL)
            {
            BeAssert(!"Incompatible replacing pixelType");
            return COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
            }
        pEffectivePixelType = pRlePixelType;
        }

    RefCountedPtr<MosaicNode> pMosaicNode = MosaicNode::Create(*this, imageNode.GetPhysicalCoordSys(), enclosingShape, pEffectivePixelType);
    ImagePPStatus buildMosaicStatus = IMAGEPP_STATUS_Success;

    // Process one image at a time, bottom up
    for (auto srcRasterPair : enclosedRasters)
        {
        HFCPtr<HRARaster>& pSrcRaster = srcRasterPair.first;

        // Always setup the clip per raster because the mosaic ::Produce will be called upon the rasters extent and this extent might
        // be beyond this raster effective rectangle. If we do not provide a shape, the outputmerger will clip with the mosaic extent that
        // is somehow generated the Produce iteration.
        HFCPtr<HVEShape> pCurrShape = new HVEShape(*srcRasterPair.second);
        
        // Move the shape as seen by the destination
        if (options.GetReplacingCoordSys())
            {
            pCurrShape->ChangeCoordSys(GetCoordSys());
            pCurrShape->SetCoordSys(options.GetReplacingCoordSys());
            }

        // Convert to destination physical coordinates
        pCurrShape->ChangeCoordSys(pMosaicNode->GetPhysicalCoordSys());

        ImagePPStatus linkToStatus = IMAGEPP_STATUS_UnknownError;
        ImageTransformNodePtr pTransformNode = ImageTransformNode::CreateAndLink(linkToStatus, *pMosaicNode, pCurrShape);
        if (IMAGEPP_STATUS_Success != linkToStatus)
            return linkToStatus;
        
        pTransformNode->SetResamplingMode(options.GetResamplingMode());
        // It is assumed that mosaic rendering always need to blend to produce the right result
        pTransformNode->SetAlphaBlend(enclosedRasters.size() > 1); 
                
        HRACopyToOptions newOptions(options);
        if (options.GetReplacingCoordSys() != NULL)
            {
            // Adapt replacing CS so that it works on the current image.

            // Extract the transformation we're applying to our source (T1)
            HFCPtr<HGF2DTransfoModel> pRefTransfo(options.GetReplacingCoordSys()->GetTransfoModelTo(GetCoordSys()));

            // Calculate the transformation between our source's CS and the CS
            // the object to return uses. (T2)
            HFCPtr<HGF2DTransfoModel> pCurrentToSource(pSrcRaster->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

            // Create the CS by composing everything correctly :)
            // T2 o T1 o -T2
            HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
            HFCPtr<HGF2DCoordSys> pCSToApply = new HGF2DCoordSys(*pToApply, pSrcRaster->GetCoordSys());

            newOptions.SetReplacingCoordSys(pCSToApply);
            }

        ImagePPStatus buildCopyToContextStatus = IMAGEPP_STATUS_Success;
        if (IMAGEPP_STATUS_Success != (buildCopyToContextStatus = pSrcRaster->BuildCopyToContext(*pTransformNode, newOptions)))
            {
            // set global status but keep first error
            if (IMAGEPP_STATUS_Success == buildMosaicStatus)
                buildMosaicStatus = buildCopyToContextStatus;
            }
        }

    if (pMosaicNode->GetChildCount() > 0)
        {
        ImagePPStatus linkToStatus = IMAGEPP_STATUS_UnknownError;
        if (IMAGEPP_STATUS_Success != (linkToStatus = imageNode.LinkTo(*pMosaicNode)))
            return linkToStatus;

        BeAssert(pMosaicNode->GetPhysicalCoordSys() == imageNode.GetPhysicalCoordSys());
        }

    return buildMosaicStatus;
    }

/** ---------------------------------------------------------------------------
Return a new copy of self
---------------------------------------------------------------------------*/
HFCPtr<HRARaster> HIMOnDemandMosaic::Clone(HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HIMOnDemandMosaic(*this);
    }

/** ---------------------------------------------------------------------------
Return a new copy of self
---------------------------------------------------------------------------*/
HPMPersistentObject* HIMOnDemandMosaic::Clone () const
    {
    return new HIMOnDemandMosaic(*this);
    }