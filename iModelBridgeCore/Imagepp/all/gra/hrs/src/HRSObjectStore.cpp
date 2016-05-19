//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrs/src/HRSObjectStore.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRSObjectStore
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRSObjectStore.h>

#include <Imagepp/all/h/HRATiledRaster.h>
#include <Imagepp/all/h/HRAStripedRaster.h>
#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRAUnlimitedResolutionRaster.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HFCExceptionHandler.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRABitmapRLE.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <ImagePP/all/h/HRAMessages.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDPacketRLE.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HTiffTag.h>
#include <Imagepp/all/h/HGSTypes.h>
#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRATransactionRecorder.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>

#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/HRFRasterFileResBooster.h>

// Improve
#include <Imagepp/all/h/HRFAdaptStripToImage.h>
#include <Imagepp/all/h/HRFAdaptTileToImage.h>
#include <Imagepp/all/h/HRFAdaptLineToImage.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

// These pixeltypes below are hidden.
//
//    --> to RLE1Bit
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
//
//  --> to I8R8G8B8(A8)
#include <Imagepp/all/h/HRPPixelTypeI2R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
//
//  --> to V24R8G8B8
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>


#include <Imagepp/all/h/HUTExportProgressIndicator.h>

#include <Imagepp/all/h/HMDAnnotationIconsPDF.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>

//-----------------------------------------------------------------------------
// Adds information about the raster file in the raster file tags.
//-----------------------------------------------------------------------------
HCLASS_ID ExtractClassIDFromFile(HFCPtr<HRFRasterFile>& pi_rpFile)
    {
    // by default, we use the direct raster file class id
    HCLASS_ID Result = pi_rpFile->GetClassID();

    // in the case of extenders, we get the class id of the original file
    if (pi_rpFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        Result = ExtractClassIDFromFile(((HFCPtr<HRFRasterFileExtender>&)pi_rpFile)->GetOriginalFile());

    return Result;
    }


//-----------------------------------------------------------------------------
// Adds information about the raster file in the raster file tags.
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile>& GetRealRasterFile(HFCPtr<HRFRasterFile>& pi_rpFile)
    {
    // in the case of extenders, we get the class id of the original file
    if (pi_rpFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        return GetRealRasterFile(((HFCPtr<HRFRasterFileExtender>&)pi_rpFile)->GetOriginalFile());

    return pi_rpFile;
    }


//-----------------------------------------------------------------------------
// Adds information about the raster file in the raster file tags.
//-----------------------------------------------------------------------------
void  ExtendTagInformation(HFCPtr<HRFRasterFile>& pi_rpFile,
                           uint32_t               pi_Page,
                           HPMAttributeSet&       pi_rAttributes)
    {
    HPRECONDITION(pi_rpFile != 0);
    HPRECONDITION(pi_Page < pi_rpFile->CountPages());
    const HRFRasterFileCreator* pCreator;
    HCLASS_ID                   ClassID;

    // keep a reference on the raster file factory's creators map
    const HRFRasterFileFactory::CreatorsMap&
    rCreatorsMap = HRFRasterFileFactory::GetInstance()->GetCreatorsMap();

    // get the raster file page
    HFCPtr<HRFPageDescriptor> pPage(pi_rpFile->GetPageDescriptor(pi_Page));
    HASSERT(pPage != 0);

    // get the raster file creator
    ClassID = ExtractClassIDFromFile(pi_rpFile);
    HRFRasterFileFactory::CreatorsMap::const_iterator Itr = rCreatorsMap.find(ClassID);
    if (Itr != rCreatorsMap.end())
        pCreator = (*Itr).second;
    else
        pCreator = 0;

    // add the file format name as a tag
    if (pCreator)
        {
        if (!pi_rAttributes.HasAttribute<HRFAttributeOriginalFileFormat>()) 
            {
            HFCPtr<HPMGenericAttribute> pAttrib(new HRFAttributeOriginalFileFormat(pCreator->GetLabel()));
            pi_rAttributes.Set(pAttrib);
            }
        }

    // add the compression type (take only the one for the first resolution)
        {
        HASSERT(pPage->CountResolutions() > 0);
        HFCPtr<HRFResolutionDescriptor> pRes(pPage->GetResolutionDescriptor(0));
        HASSERT(pRes != 0);

        if (pRes->GetCodec() != 0)
            {
            if (!pi_rAttributes.HasAttribute<HRFAttributeOriginalFileCompression>())
                {
                HFCPtr<HPMGenericAttribute> pAttrib(new HRFAttributeOriginalFileCompression(pRes->GetCodec()->GetName()));
                pi_rAttributes.Set(pAttrib);
                }
            }
        }
        
        {
        // add the original file size
        HFCStat FileStat(pi_rpFile->GetURL());
        if (!pi_rAttributes.HasAttribute<HRFAttributeOriginalFileSize>())
            {
            HFCPtr<HPMGenericAttribute> pAttrib(new HRFAttributeOriginalFileSize(FileStat.GetSize()));
            pi_rAttributes.Set(pAttrib);
            }

        if (!pi_rAttributes.HasAttribute<HRFAttributeOriginalFileModificationDate>())
            {
            HFCPtr<HPMGenericAttribute> pAttrib(new HRFAttributeOriginalFileModificationDate(FileStat.GetModificationTime()));
            pi_rAttributes.Set(pAttrib);
            }
        }
    }

//-----------------------------------------------------------------------------

// Predefine ID
const uint64_t HRSObjectStore::ID_PyramidRaster              = 0x8000000000000000;
const uint64_t HRSObjectStore::ID_UnlimitedResolutionRaster  = 0x8000000000000001;
const uint64_t HRSObjectStore::ID_ResizableRaster            = 0x8000000000000002;
const uint64_t HRSObjectStore::ID_TiledRaster                = 0x8000000000000003;   // must be at the end of the list


///////////////////////////////////////////////////////////////////////////////
// Methods for class HRSObjectStore

HMG_BEGIN_RECEIVER_MESSAGE_MAP(HRSObjectStore, HMGMessageReceiver, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRSObjectStore, HRAPyramidRasterClosingMsg, NotifyHRAPyramidRasterClosing)
HMG_REGISTER_MESSAGE(HRSObjectStore, HRALookAheadMsg, NotifyHRALookAhead)
HMG_END_MESSAGE_MAP();

//-----------------------------------------------------------------------------
// The constructor.  An object store is always associated with a default object
// log, that log being specified at construction time.
//-----------------------------------------------------------------------------
HRSObjectStore::HRSObjectStore(HPMPool*                      pi_pLog,
                               HFCPtr<HRFRasterFile>&        pi_pRasterFile,
                               uint32_t                     pi_Page,
                               const HFCPtr<HGF2DCoordSys>&  pi_rRefCoordSys)
    : HPMObjectStore (pi_pLog),
      m_LoadCurSubImage(0),
      m_SaveCurSubImage(0),
      m_Resizable(false),
      m_Loaded(false)
    {
    HPRECONDITION (pi_pRasterFile != 0);

    // set the 1 bit background color members
    m_Has1BitBackgroundColor = false;
    m_BackgroundColor1Bit = 0;

    m_MultiPixelType = false;

    // The object does not throw on invalid HRF results
    m_IsThrowable = false;

    m_UseClipShape = true;

    m_ForceReadOnly = false;

    if (pi_pRasterFile->PagesAreRasterFile())
        m_pRasterFile  = pi_pRasterFile->GetPageFile(pi_Page);
    else
        m_pRasterFile = pi_pRasterFile;

    Constructor (pi_Page, pi_rRefCoordSys);
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HRSObjectStore::~HRSObjectStore()
    {
    try
        {
        SaveAll();
        CleanUp();

        for (size_t i=0; i<m_ResolutionEditor.size(); i++)
            delete m_ResolutionEditor[i];
        m_ResolutionEditor.clear();
        }
    catch(...)
        {
        HFCExceptionHandler::HandleException();
        }
    }


//-----------------------------------------------------------------------------
// Public
// Default is true. If false the native clip shape won't be read and nor
// modified by a save operation.
//-----------------------------------------------------------------------------
void HRSObjectStore::SetUseClipShape(bool pi_UseClipShape)
    {
    m_UseClipShape = pi_UseClipShape;
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if the object will throw an HFCException upon a HRF invalid result
// (not H_SUCCESS) in the Load and Save methods of this class.
//-----------------------------------------------------------------------------
void HRSObjectStore::SetThrowable(bool pi_Throwable)
    {
    m_IsThrowable = pi_Throwable;
    }

//-----------------------------------------------------------------------------
// Private
// Verifies the result of the last raster file operation and throws the
// corresponding HFCException
//-----------------------------------------------------------------------------
bool HRSObjectStore::VerifyResult(HSTATUS pi_Result, bool pi_IsThrowable) const
    {
    // No process needed for H_SUCCESS.
    if(H_SUCCESS == pi_Result)
        return false;

    // verify the type of the error and throw the corresponding exception
    switch(pi_Result)
        {
        case H_INVALID:
        case H_ERROR:
            if (pi_IsThrowable)
                throw HFCUnknownException();
            else
                return true;    // Fatal error
            break;

        case H_NOT_ENOUGH_MEMORY:
            if (pi_IsThrowable)
                throw HFCOutOfMemoryException();
            else
                return true;    // Fatal error
            break;

        case H_NO_SPACE_LEFT:
            if (pi_IsThrowable)
                throw HFCNoDiskSpaceLeftException(m_pRasterFile->GetURL()->GetURL());
            else
                return true;    // Fatal error
            break;

        case H_DATA_CORRUPTED:
            if (pi_IsThrowable)
                {
                throw HFCCorruptedFileException((m_pRasterFile->GetURL()->GetURL()));
                }
            else
                return true;    // Fatal error
            break;

        case H_IOERROR:
            if (pi_IsThrowable)
                throw HFCFileIOErrorException(m_pRasterFile->GetURL()->GetURL());
            else
                return true;    // Fatal error
            break;

        case H_WRITE_ERROR:
            if (pi_IsThrowable)
                throw HFCWriteFaultException(m_pRasterFile->GetURL()->GetURL());
            else
                return true;    // Fatal error
            break;

        case H_INTERNET_CONNECTION_LOST:
            if (pi_IsThrowable)
                {
                throw HFCInternetConnectionException(m_pRasterFile->GetURL()->GetURL(), HFCInternetConnectionException::CONNECTION_LOST);
                }
            else
                return true;    // Fatal error
            break;

        case H_INTERNET_CANNOT_CONNECT:
            if (pi_IsThrowable)
                {
                throw HFCInternetConnectionException(m_pRasterFile->GetURL()->GetURL(), HFCInternetConnectionException::CANNOT_CONNECT);
                }
            else
                return true;    // Fatal error
            break;

        case H_INTERNET_UNKNOWN_ERROR:
            if (pi_IsThrowable)
                {
                throw HFCInternetConnectionException(m_pRasterFile->GetURL()->GetURL(), HFCInternetConnectionException::UNKNOWN);
                }
            else
                return true;    // Fatal error
            break;

        case H_OUT_OF_RANGE:
            if (pi_IsThrowable)
                {
                throw HFCFileOutOfRangeException(m_pRasterFile->GetURL()->GetURL());
                }
            else
                return true;    // Fatal error
            break;

            // Anything else is not a fatal error
            //case H_SUCCESS:
            //case H_NOT_FOUND: // This error is returned when we try to read a file in creation, but the tile is not created yet.
        default:
            // throw nothing
            break;
        }

    return false;   // Not fatal error;
    }

//-----------------------------------------------------------------------------
// public
// operator<< HPMPersistentObject&
//-----------------------------------------------------------------------------
void HRSObjectStore::Save(HPMPersistentObject* pi_pObj)
    {
    HCLASS_ID ClassKey;
    HSTATUS Result = H_SUCCESS;
    HFCMonitor RasterFileMonitor;

    // If the Store is not readonly
    if (!IsReadOnly())
        {
        // If ID > ID_TiledRaster then it is a SubResolution in a Pyramid.
        if (pi_pObj->GetID() >= ID_TiledRaster && pi_pObj->GetID() != INVALID_OBJECT_ID)
            ClassKey = HRATiledRaster::CLASS_ID;
        else
            {
            // Check the ID
            switch (pi_pObj->GetID())
                {
                case ID_PyramidRaster:
                    ClassKey = HRAPyramidRaster::CLASS_ID;
                    break;

                case ID_UnlimitedResolutionRaster:
                    ClassKey = HRAUnlimitedResolutionRaster::CLASS_ID;
                    break;

                case INVALID_OBJECT_ID:
                    // Find the ClassKey
                    ClassKey = CheckObjectClassKey (*pi_pObj);
                    break;

                    //The default is a tile
                default:
                    HASSERT(pi_pObj->IsCompatibleWith(HRABitmapBase::CLASS_ID));
                    ClassKey = HRABitmapBase::CLASS_ID;
                    break;
                }
            }

        // Object CLassKey
        //
        if (ClassKey == HRABitmapBase::CLASS_ID)
            {
            Result = SaveBitmap((HRABitmapBase*)pi_pObj);
            }
        else if (ClassKey == HRATiledRaster::CLASS_ID)
            {
            // Dump the TiledRaster
            SaveTiledRaster ((HRATiledRaster*)pi_pObj);

            // save on main image
            if (pi_pObj->GetID() == ID_TiledRaster)
                {
                if (((HRAStoredRaster*)pi_pObj)->GetPixelType()->CountIndexBits() > 0)
                    SavePalette((HRAStoredRaster*)pi_pObj);

                SaveModel ((HRAStoredRaster*)pi_pObj);
                SaveLogicalShape ((HRAStoredRaster*)pi_pObj);
                }
            else
                {
                if (((HRAStoredRaster*)pi_pObj)->GetPixelType()->CountIndexBits() > 0)
                    SavePalette((HRAStoredRaster*)pi_pObj,
                                (uint16_t)(pi_pObj->GetID() - ID_TiledRaster));
                }
            }
        // Here, all pyramid are a TiledRaster.
        else if (ClassKey == HRAPyramidRaster::CLASS_ID)
            {
            // Dump the PyramidRaster

            uint16_t NumberOfImage(((HRAPyramidRaster*)pi_pObj)->CountSubImages());

            for(uint16_t i = 0; i < NumberOfImage; i++)
                ((HRAPyramidRaster*)pi_pObj)->GetSubImage(i)->Save();

            SaveHistogram((HRAPyramidRaster*)pi_pObj);
            SaveResamplingMethod((HRAPyramidRaster*)pi_pObj);
            SaveDataFlag((HRAPyramidRaster*)pi_pObj);

            m_pRasterFile->Save();

            ((HRAStoredRaster*)pi_pObj)->Saved();

            pi_pObj->SetModificationState(false);
            }
        else if (ClassKey == HRAUnlimitedResolutionRaster::CLASS_ID)
            {
            HASSERT(false);
            }

        RasterFileMonitor.ReleaseKey();
        }
    }


//-----------------------------------------------------------------------------
// public
// Load
//-----------------------------------------------------------------------------
HFCPtr<HRAStoredRaster> HRSObjectStore::Load(HPMObjectID pi_ObjectID)
    {
    HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());
    HFCPtr<HRAStoredRaster> pObject = (HRAStoredRaster*)GetLoaded(pi_ObjectID);
    HSTATUS Result = H_SUCCESS;

    if (pObject == 0)
        {
        if (pi_ObjectID == ID_TiledRaster || pi_ObjectID == ID_PyramidRaster ||
            pi_ObjectID == ID_UnlimitedResolutionRaster || pi_ObjectID == ID_ResizableRaster)
            {
            if (pi_ObjectID == ID_UnlimitedResolutionRaster)
                pObject = LoadUnlimitedResolutionRaster(&m_RasterObjectID);
            else if (pi_ObjectID == ID_ResizableRaster)
                pObject = LoadResizableRaster(&m_RasterObjectID);
            else
                pObject = LoadRaster (&m_RasterObjectID);

            // Assign the Store
            pObject->SetID(m_RasterObjectID);
            pObject->SetStore(this);
            RegisterObject(pObject);

            // Pyramid or TiledRaster not modified
            pObject->SetModificationState (false);

            pObject->GetPixelType()->SetModificationState(false);          // PixelType not modified
            const_cast<HVEShape&>(pObject->GetShape()).SetModificationState(false);  // Shape not modified
            pObject->GetPhysicalCoordSys()->SetModificationState(false);   // TransfoModel not modified
            pObject->GetCoordSys()->SetModificationState(false);


            // Link ourselves to the StoredRaster to receive the message HRAStoredRasterClosing
            LinkTo(pObject);

            // Link the Raster with his HRFFile (NO Interthread!)
            pObject->LinkTo(m_pRasterFile);
            }
        else
            {
            // HRATiledRaster load tiles by LoadTile
            HASSERT(0);
            }

        m_Loaded = true;
        }

    // Analyze the result code.  An exception will be thrown if this object
    // was set contructed as throwable
    VerifyResult(Result, m_IsThrowable);

    return pObject;
    }



//-----------------------------------------------------------------------------
// public
// LoadTile
//-----------------------------------------------------------------------------
HFCPtr<HRABitmapBase> HRSObjectStore::LoadTile(uint64_t                  pi_Index,
                                               uint32_t                   pi_Resolution,
                                               uint64_t*                   po_pPosX,
                                               uint64_t*                   po_pPosY,
                                               HPMPool*                   pi_pPool)
    {
    HPRECONDITION(po_pPosX != 0);
    HPRECONDITION(po_pPosY != 0);

    HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());
    HSTATUS Result = H_SUCCESS;

    // Make a tile.
    HFCPtr<HRABitmapBase> pTile = static_cast<HRABitmapBase*>(m_pTileTemplate->Clone(0).GetPtr());
    if(pTile->IsCompatibleWith(HRABitmap::CLASS_ID))
        static_cast<HRABitmap&>(*pTile).SetPool(pi_pPool);
    try
        {
        HFCPtr<HRFResolutionDescriptor> pResDesc = m_ResolutionEditor[pi_Resolution]->GetResolutionDescriptor();
        pTile->InitSize(pResDesc->GetBlockWidth(), pResDesc->GetBlockHeight());

        // Read Tile
        CheckCurrentSubImageForLoad(pi_Resolution);
        m_pLoadTileDescriptor->GetPositionFromIndex(pi_Index, po_pPosX, po_pPosY);

        if (m_ResolutionEditor[m_LoadCurSubImage]->GetAccessMode().m_HasReadAccess)
            {
            if (pTile->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
                {
                HRABitmapRLE* pBitmapRLE = static_cast<HRABitmapRLE*>(pTile.GetPtr());
                Result = ExceptionsSafeReadBlockRLE(m_ResolutionEditor[m_LoadCurSubImage], *po_pPosX, *po_pPosY, pBitmapRLE->GetPacket());
                }
            else if(pTile->IsCompatibleWith(HRABitmap::CLASS_ID))
                {
                HRABitmap* pTileBitmap = static_cast<HRABitmap*>(pTile.GetPtr());
                Result = ExceptionsSafeReadBlock(m_ResolutionEditor[m_LoadCurSubImage], *po_pPosX, *po_pPosY, pTileBitmap->GetPacket()->GetBufferAddress());
                }
            else
                {
                HASSERT(!"Unknown HRABitmap type");
                }
            }

        if (Result != H_SUCCESS && Result != H_DATA_NOT_AVAILABLE)
            {
            // Used to be MakeEmpty() but that will only remove the packet from the the Bitmap.
            // It will be reinstate on the first call to GetPacket but unfortunately with an uninitialized packet 
            // so better to clear if we had a read error.  
            pTile->Clear();
            //pTile->MakeEmpty();
            }

        // Set persistent information.
        pTile->SetID (m_pLoadTileDescriptor->ComputeIDFromIndex (pi_Index,
                                                                 pi_Resolution));
        pTile->SetStore (this);

        // Tile not modified
        pTile->SetModificationState (false);
        }
    catch(...)
        {
        throw;
        }

    // Analyze the result code.  An exception will be thrown if this object
    // was set contructed as throwable
    VerifyResult(Result, m_IsThrowable);

    return pTile;
    }

//-----------------------------------------------------------------------------
// Public
// If required, this method constructs a context and set it to the raster.
//-----------------------------------------------------------------------------
void HRSObjectStore::ConstructContextForRaster(const HFCPtr<HRFRasterFile>& pi_rRasterFile,
                                               uint32_t                     pi_PageInd,
                                               HRARaster*                   po_rRaster)
    {
    HPRECONDITION(pi_rRasterFile != 0);
    HPRECONDITION(po_rRaster != 0);

    HFCPtr<HMDContext>           pContext;
    HFCPtr<HMDMetaDataContainer> pMDContainer;

    pMDContainer = pi_rRasterFile->GetPageDescriptor(pi_PageInd)
                   ->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO);

    //If some layer info have been found, construct the volatile info
    if (pMDContainer != 0)
        {
        HFCPtr<HMDLayers>         pLayers = (HFCPtr<HMDLayers>&)pMDContainer;
        HFCPtr<HMDVolatileLayers> pVolatileLayers(new HMDVolatileLayers(pLayers));

        pContext = new HMDContext();
        pContext->AddMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO,
                                       (HFCPtr<HMDMetaDataContainer>&)pVolatileLayers);
        }

    pMDContainer = pi_rRasterFile->GetPageDescriptor(pi_PageInd)
                   ->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOTATION_INFO);

    if (pMDContainer != 0)
        {
        if (pContext == 0)
        {
            pContext = new HMDContext();
            }

        pContext->AddMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO,
                                       HFCPtr<HMDMetaDataContainer>(new HMDAnnotationIconsPDF()));
        }

    if(pi_rRasterFile->IsCompatibleWith(HRFFileId_PDF/*HRFPDFFile::CLASS_ID*/))
        {
        if (pContext == 0)
            pContext = new HMDContext();

        // Add meta data that will hold draw options. The HRA context is set into the HRF in HRATiledRaster::GetTile(). Search for TR 270043.
        pContext->AddMetaDataContainer(HMDMetaDataContainer::HMD_PDF_DRAW_OPTIONS, HFCPtr<HMDMetaDataContainer>(new HMDDrawOptionsPDF()));
        }

    po_rRaster->SetContext(pContext);
    }

//-----------------------------------------------------------------------------
// Public
// This method gets the current context of the persistent source.
//-----------------------------------------------------------------------------
const HFCPtr<HMDContext> HRSObjectStore::GetContext()
    {
    return m_pRasterFile->GetContext(m_PageIndex);
    }

//-----------------------------------------------------------------------------
// public
// AddResolution
//
// This method is used in conjonction with a HRAUnlimitedResolutionRaster.
//
//
//-----------------------------------------------------------------------------
uint32_t HRSObjectStore::CreateResolution(double pi_Scale)
    {
#if 0
    HPRECONDITION(m_RasterObjectID == ID_UnlimitedResolutionRaster);
    HPRECONDITION(m_ResolutionEditor.size() < 255);    // the tileIDDescriptor use only 8 bit for the sub resolution index
    HPRECONDITION(GetResolution(pi_Scale) ==);

    ListOfResolutionEditor::iterator Itr(m_ResolutionEditor.begin());
    uint32_t ResIndex = 0;
    while (Itr != m_ResolutionEditor.end())
        {
        if (m_ResolutionEditor[ResIndex] == 0)
            Itr = m_ResolutionEditor.end();
        else
            {
            Itr++;
            ResIndex++;
            }
        }
    HPOSTCONDITION(ResIndex != 0);

    HFCAccessMode PageAccessMode = m_pPageDescriptor->GetAccessMode();
    HRFResolutionEditor* pResEditor = m_pRasterFile->CreateResolutionEditor(m_PageIndex,
                                                                            pi_Scale,
                                                                            PageAccessMode);
    HPOSTCONDITION(pResEditor != 0);
    m_ResolutionEditor[ResIndex] = pResEditor;

    return ResIndex;
#endif
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// RemoveResolution
// This method is used in conjonction with a HRAUnlimitedResolutionRaster.
//
//-----------------------------------------------------------------------------
void HRSObjectStore::RemoveResolution(uint16_t pi_Resolution)
    {
    HPRECONDITION(m_RasterObjectID == ID_UnlimitedResolutionRaster);
    HPRECONDITION(pi_Resolution > 0);           // resolution editor 0 cannot be removed
    HPRECONDITION(pi_Resolution < m_ResolutionEditor.size());

    delete m_ResolutionEditor[pi_Resolution];
    m_ResolutionEditor[pi_Resolution] = 0;
    }

//-----------------------------------------------------------------------------
// public
// ChangeResolution
// This method is used in conjunction with a HRAUnlimitedResolutionRaster.
//
//-----------------------------------------------------------------------------
void HRSObjectStore::ChangeResolution(uint16_t pi_Resolution,
                                      double   pi_NewScaling,
                                      uint64_t*   po_pResWidth,
                                      uint64_t*   po_pResHeight)
    {
    HPRECONDITION(m_pPageDescriptor->IsUnlimitedResolution());
    HPRECONDITION(pi_Resolution < m_ResolutionEditor.size());
    HPRECONDITION(po_pResWidth != 0 && po_pResHeight != 0);

    HFCAccessMode PageAccessMode = m_pPageDescriptor->GetAccessMode();
    delete m_ResolutionEditor[pi_Resolution];
    m_ResolutionEditor[pi_Resolution] = m_pRasterFile->CreateUnlimitedResolutionEditor(m_PageIndex,
                                        pi_NewScaling,
                                        PageAccessMode);

    HFCPtr<HRFResolutionDescriptor> pResDesc(m_ResolutionEditor[pi_Resolution]->GetResolutionDescriptor());
    m_pLoadTileDescriptor = new HGFTileIDDescriptor(pResDesc->GetWidth(),
                                                    pResDesc->GetHeight(),
                                                    pResDesc->GetBlockWidth(),
                                                    pResDesc->GetBlockHeight());
    m_pSaveTileDescriptor->ChangeSize(pResDesc->GetWidth(),
                                      pResDesc->GetHeight());

    *po_pResWidth = pResDesc->GetWidth();
    *po_pResHeight = pResDesc->GetHeight();
    }

//-----------------------------------------------------------------------------
// public
// IsReadOnly
//-----------------------------------------------------------------------------
bool HRSObjectStore::IsReadOnly() const
    {
    return (m_ForceReadOnly ||
            (!GetRasterFileAccessMode().m_HasWriteAccess && !GetRasterFileAccessMode().m_HasCreateAccess) ||
            (GetRasterFileAccessMode().m_HasCreateAccess && m_pRasterFile->IsCreateCancel()));
    }


//----------------------------------------------------------------------------
// Make the database writeable or read-only
//----------------------------------------------------------------------------
void HRSObjectStore::ForceReadOnly(bool pi_ReadOnly)
    {
    m_ForceReadOnly = pi_ReadOnly;
    }

//-----------------------------------------------------------------------------
// public
// RasterSizeChanged
//-----------------------------------------------------------------------------
void HRSObjectStore::RasterSizeChanged(int64_t pi_Top,
                                       int64_t pi_Bottom,
                                       int64_t pi_Left,
                                       int64_t pi_Right)
    {
    HPRECONDITION(m_pPageDescriptor->IsResizable());

    if (m_Loaded)
        {
        HPRECONDITION(m_ResolutionEditor.size() == 1);

        m_pRasterFile->ResizePage(m_PageIndex,
                                  m_pResDescriptor->GetWidth() + pi_Left + pi_Right,
                                  m_pResDescriptor->GetHeight() + pi_Top + pi_Bottom);

        // change resolution editor
        HAutoPtr<HRFResolutionEditor> pOldEditor(m_ResolutionEditor[0]);
        m_ResolutionEditor[0] = m_pRasterFile->CreateResolutionEditor(pOldEditor->GetPage(),
                                                                      pOldEditor->GetResolutionIndex(),
                                                                      pOldEditor->GetAccessMode());
        }
    }

//-----------------------------------------------------------------------------
// public
// NotifyContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRSObjectStore::NotifyHRAPyramidRasterClosing (const HMGMessage& pi_rMessage)
    {
    // Generate a save to flush Data only in Sister.
    //  but can't save other information, the user must call save.
    HRAStoredRaster* pRaster = ((HRAStoredRaster*)pi_rMessage.GetSender());

    if (pRaster->ToBeSaved())
        Save(pRaster);

    // Unlink the HRF and the Raster
    pRaster->UnlinkFrom(m_pRasterFile);
        
    return true;
    }


//-----------------------------------------------------------------------------
// public
// NotifyContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRSObjectStore::NotifyHRALookAhead(const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.GetClassID() == HRALookAheadMsg::CLASS_ID);

    bool RetValue = false;

    // Cast to the right class
    HRALookAheadMsg& rMessage = (HRALookAheadMsg&)pi_rMessage;

    // Set look ahead to the HRFRasterFile
    if (m_pRasterFile->HasLookAhead(m_PageIndex))
        {
        if (!rMessage.UseShape())
            {
            //
            HGFTileIDDescriptor TileIDDesc;
            const HGFTileIDList& rHRATileIDList(rMessage.GetTileIDList());
            HGFTileIDList HRFTileIDList;

            HGFTileIDList::const_iterator Itr(rHRATileIDList.begin());
            while (Itr != rHRATileIDList.end())
                {
                HRFTileIDList.push_back(TileIDDesc.ComputeIDFromIndex(TileIDDesc.GetIndex(*Itr),
                                                                      m_ResolutionEditor[TileIDDesc.GetLevel(*Itr)]->GetResolutionIndex()));//resolution index used by the editor
                Itr++;
                }

            m_pRasterFile->SetLookAhead(m_PageIndex,
                                        HRFTileIDList,
                                        rMessage.GetConsumerID(),
                                        rMessage.IsAsynchronous());
            RetValue = true;
            }
        else
            {
            m_pRasterFile->SetLookAhead(m_PageIndex,
                                        m_ResolutionEditor[rMessage.GetResolution()]->GetResolutionIndex(),//resolution index used by the editor
                                        rMessage.GetShape(),
                                        rMessage.GetConsumerID(),
                                        rMessage.IsAsynchronous());
            RetValue = true;
            }
        }
    return RetValue;
    }

//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// private
// Constructor.
//-----------------------------------------------------------------------------
void HRSObjectStore::Constructor (uint32_t pi_Page,
                                  const HFCPtr<HGF2DCoordSys>&  pi_rRefCoordSys)
    {
    try
        {
        // get an editor on the raster file and verify that it is tiled
        // create the editors to query the raster file
        m_PageIndex = pi_Page;
        m_pPageDescriptor = m_pRasterFile->GetPageDescriptor(pi_Page);

        // Keep the first pixel type
        HFCPtr<HRPPixelType> pMainImagePixelType(m_pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType());

        HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
        pMultiResCapability = static_cast<HRFMultiResolutionCapability*>(m_pRasterFile->GetCapabilities()->
                               GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());
        bool SupportMultiPixelType = pMultiResCapability != 0 && !pMultiResCapability->IsSinglePixelType();
        if (m_pPageDescriptor->IsUnlimitedResolution())
            {
            m_Resizable = false;

            // create a resolution editor on the main image
            if (GetRasterFileAccessMode().m_HasWriteAccess)
                {
                m_ResolutionEditor.push_back(m_pRasterFile->CreateUnlimitedResolutionEditor(pi_Page, 1.0, HFC_READ_WRITE));
                }
            else if (GetRasterFileAccessMode().m_HasCreateAccess)
                {
                m_ResolutionEditor.push_back(m_pRasterFile->CreateUnlimitedResolutionEditor(pi_Page, 1.0, HFC_CREATE_ONLY));
                }
            else
                {
                m_ResolutionEditor.push_back(m_pRasterFile->CreateUnlimitedResolutionEditor(pi_Page, 1.0, HFC_READ_ONLY));
                }

            HASSERT(m_ResolutionEditor[0] != 0);
            }
        else
            {
            uint16_t CountRes = m_pPageDescriptor->CountResolutions();
            for (uint16_t i = 0; i < CountRes; i++)
                {
                if (SupportMultiPixelType)
                    m_MultiPixelType = m_MultiPixelType ||
                                       !pMainImagePixelType->HasSamePixelInterpretation(*m_pPageDescriptor->GetResolutionDescriptor(i)->GetPixelType());

                if (GetRasterFileAccessMode().m_HasWriteAccess)
                    {
                    m_ResolutionEditor.push_back(m_pRasterFile->CreateResolutionEditor(pi_Page, i, HFC_READ_WRITE));
                    }
                else if (GetRasterFileAccessMode().m_HasCreateAccess)
                    {
                    m_ResolutionEditor.push_back(m_pRasterFile->CreateResolutionEditor(pi_Page, i, HFC_CREATE_ONLY));
                    }
                else
                    {
                    m_ResolutionEditor.push_back(m_pRasterFile->CreateResolutionEditor(pi_Page, i, HFC_READ_ONLY));
                    }

                HASSERT(m_ResolutionEditor[i] != 0);
                }
            }

        m_pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor(0);

        // re sizable capabilities
        if (m_Resizable)
            {
            HFCPtr<HRFResizableCapability> pResizableCapability;
            pResizableCapability = static_cast<HRFResizableCapability*>(m_pRasterFile->GetCapabilities()->
                                    GetCapabilityOfType(HRFResizableCapability::CLASS_ID, m_pRasterFile->GetAccessMode()).GetPtr());
            if (pResizableCapability == 0 || m_pPageDescriptor->CountResolutions() != 1 || m_pResDescriptor->CountBlocks() != 1)
                m_Resizable = false;
            }

        // Create the TileDescriptor
        m_pLoadTileDescriptor = new HGFTileIDDescriptor (m_pResDescriptor->GetWidth (),
                                                         m_pResDescriptor->GetHeight (),
                                                         m_pResDescriptor->GetBlockWidth(),
                                                         m_pResDescriptor->GetBlockHeight());
        m_pSaveTileDescriptor = new HGFTileIDDescriptor (m_pResDescriptor->GetWidth (),
                                                         m_pResDescriptor->GetHeight (),
                                                         m_pResDescriptor->GetBlockWidth(),
                                                         m_pResDescriptor->GetBlockHeight());


        // Set CoordSys
        // Attach the Logical Coord to the Reference.
        m_pLogicalCoordSys = pi_rRefCoordSys;

        // Validation with the capabilities if it's possible to get a Transfo Model
        if ((m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_READ_ONLY) != 0) &&
            (m_pPageDescriptor->HasTransfoModel()))
            {
            m_pPhysicalCoordSys = new HGF2DCoordSys (*m_pPageDescriptor->GetTransfoModel(), m_pLogicalCoordSys);
            }
        else
            {
            m_pPhysicalCoordSys = m_pLogicalCoordSys;
            }


        // Create a Tile template
        m_pTileTemplate = MakeBitmap (1, 1, m_pResDescriptor->GetPixelType());
        }
    catch(...)
        {
        // Clean up the ResolutionEditor list and continue the exception propagation.
        for (size_t i = 0; i < m_ResolutionEditor.size(); i++)
            delete m_ResolutionEditor[i];
        m_ResolutionEditor.clear();
        throw;
        }
    }



//-----------------------------------------------------------------------------
// private
// MakeBitmap - Create the Raster for the specify PixelType.
//-----------------------------------------------------------------------------
HFCPtr<HRABitmapBase> HRSObjectStore::MakeBitmap(uint32_t                      pi_Width,
                                                 uint32_t                      pi_Height,
                                                 const HFCPtr<HRPPixelType>&   pi_rpPixelType)
    {
    HPRECONDITION(pi_rpPixelType != 0);
    HFCPtr<HRPPixelType>    pPixelType;
    HFCPtr<HRABitmapBase>   pBitmap;

    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    if (m_pPageDescriptor->HasTransfoModel())
        pTransfoModel = m_pPageDescriptor->GetTransfoModel();

    // Check for the predefine pixelType
    pPixelType = pi_rpPixelType;

    switch (pi_rpPixelType->CountPixelRawDataBits ())
        {
        case 1:
            {
            HFCPtr<HRPPixelType> pBitmapPixelType;
            if (pi_rpPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
                pBitmapPixelType = new HRPPixelTypeI1R8G8B8();
            else
                pBitmapPixelType = new HRPPixelTypeI1R8G8B8A8();

            // Put it in a BitmapRLE, normally the raster completely
            pBitmap = HRABitmapRLE::Create (pi_Width,
                                        pi_Height,
                                        pTransfoModel,
                                        m_pLogicalCoordSys,
                                        pBitmapPixelType,
                                        m_Resizable).GetPtr();

            // Set palette
            HRPPixelTypeV32R8G8B8A8   rgbaPixelType;
            HRPPixelPalette&          rPalette = pBitmapPixelType->LockPalette();
            HFCPtr<HRPPixelConverter> pConvertToRGBA = pi_rpPixelType->GetConverterTo(&rgbaPixelType);
            Byte                      rgbaValues[8];
            Byte                      binaryValues = 0x40; // 0100 0000
            
            // Get index 0 and 1 RGBA value.
            pConvertToRGBA->Convert(&binaryValues, rgbaValues, 2);
            
            // Set RGB(alpha ignored) or RGBA palette entries.
            rPalette.SetCompositeValue(0, rgbaValues);
            rPalette.SetCompositeValue(1, rgbaValues+4);
                
            pBitmapPixelType->UnlockPalette();

            // Set default value
            pBitmapPixelType->SetDefaultRawData(pi_rpPixelType->GetDefaultRawData());
            }
        break;

        case 2:
        case 4:
            {
            HPRECONDITION(pi_rpPixelType->CountValueBits() == 0);
            HPRECONDITION(!m_Resizable);    // resizable supported only for 1 bit image for now

            int32_t     ShiftValue(0);
            uint16_t NbColor(0);  
              
            if (pi_rpPixelType->GetClassID() == HRPPixelTypeId_I2R8G8B8)
                {
                ShiftValue = 6;
                NbColor = 4;  
                }
            else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI4R8G8B8::CLASS_ID ||
                    (pi_rpPixelType->GetClassID() == HRPPixelTypeI4R8G8B8A8::CLASS_ID) )
                {
                ShiftValue = 4;
                NbColor = 16;  
                }
            else
                HASSERT(false);

            if (ShiftValue != 0)
                {
                HFCPtr<HRPPixelType> pBitmapPixelType;
                if(pi_rpPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
                    pBitmapPixelType = new HRPPixelTypeI8R8G8B8(pi_rpPixelType->GetPalette());
                else
                    pBitmapPixelType = new HRPPixelTypeI8R8G8B8A8(pi_rpPixelType->GetPalette());

                pBitmap = HRABitmap::Create(pi_Width,
                                        pi_Height,
                                        pTransfoModel,
                                        m_pLogicalCoordSys,
                                        pBitmapPixelType);

                // Set Converters and temporary memory block also used in the load tile.
                m_ConvertNativeToNormalize = pi_rpPixelType->GetConverterTo(pBitmapPixelType);
                m_ConvertNormalizeToNative = pBitmapPixelType->GetConverterTo(pi_rpPixelType);

                Byte normalizeRawValue;
                m_ConvertNativeToNormalize->Convert(pi_rpPixelType->GetDefaultRawData(), &normalizeRawValue, 1);

                // Set default value
                pBitmapPixelType->SetDefaultRawData(&normalizeRawValue);
                }
            }
            break;

        case 16:
            if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16R5G6B5::CLASS_ID ||
                pi_rpPixelType->GetClassID() == HRPPixelTypeV16B5G5R5::CLASS_ID)
                {
                HFCPtr<HRPPixelType> pBitmapPixelType = new HRPPixelTypeV24R8G8B8();
                pBitmap = HRABitmap::Create(pi_Width,
                                        pi_Height,
                                        pTransfoModel,
                                        m_pLogicalCoordSys,
                                        pBitmapPixelType);

                // Set Converters and temporary memory block also used in the load tile.
                m_ConvertNativeToNormalize = pi_rpPixelType->GetConverterTo(pBitmapPixelType);
                m_ConvertNormalizeToNative = pBitmapPixelType->GetConverterTo(pi_rpPixelType);
                pBitmapPixelType->SetDefaultRawData(pi_rpPixelType->GetDefaultRawData());

                Byte normalizeRawValue[3];
                m_ConvertNativeToNormalize->Convert(pi_rpPixelType->GetDefaultRawData(), &normalizeRawValue, 1);

                // Set default value
                pBitmapPixelType->SetDefaultRawData(&normalizeRawValue);
                break;
                }

        default:
            HPRECONDITION(!m_Resizable);    // resizable supported only for 1 bit image for now
            pBitmap = HRABitmap::Create (pi_Width,
                                     pi_Height,
                                     pTransfoModel,
                                     m_pLogicalCoordSys,
                                     (HRPPixelType*)pi_rpPixelType->Clone());
            break;
        }

    return pBitmap;
    }

//-----------------------------------------------------------------------------
// private
// LoadRaster - Create the Raster.
//
//  Presently we must create a PyramidRaster because :
//      1- Message HRAPyramidRasterClosing,
//         I tried with HRAStoredRasterClosing, in the Destructor HRAStoredRaster
//         but --> many bizzar Bugs + propagate many messages, for each tiles, ...
//
//      2- Patch for the LookAhead...
//-----------------------------------------------------------------------------
HFCPtr<HRAPyramidRaster> HRSObjectStore::LoadRaster (HPMObjectID* po_pRasterID)
    {
    HFCPtr<HRAPyramidRaster> pOutputRaster;

    // Check if MultiResolution
    //
    uint16_t NbSubImage;
    if ((NbSubImage = m_pPageDescriptor->CountResolutions() - 1) != 0)
        {
        // when the raster file is read only and contains the original data,
        // don't need to compute sub resolution
        bool EnableComputeSubRes = (m_pRasterFile->GetAccessMode().m_HasWriteAccess ||
                                     m_pRasterFile->GetAccessMode().m_HasCreateAccess) || !m_pRasterFile->IsOriginalRasterDataStorage();

        HArrayAutoPtr<HRAPyramidRaster::SubImageDescription> pSubImageDesc(new HRAPyramidRaster::SubImageDescription[NbSubImage]);

        bool                           HasSubResDirtyFlag = false;
        HFCPtr<HRFResolutionDescriptor> pResDescriptor;

        for (uint16_t i = 0; i < NbSubImage; i++)
            {
            HRFDataFlag DataFlag;
            bool       HasDirtyFlag = false;

            pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor(i);

            if (HasSubResDirtyFlag)
                {
                HasDirtyFlag = true;
                HasSubResDirtyFlag = false;
                }
            else
                {
                if (EnableComputeSubRes && pResDescriptor->HasBlocksDataFlag())
                    {
                    // Look if we have an overwritten block
                    // if we have an overwritten block this resolution must updated to the source file
                    uint64_t FlagCount = pResDescriptor->CountBlocks();
                    for (uint64_t FlagIndex=0; (FlagIndex < FlagCount) && (HasDirtyFlag == false); FlagIndex++)
                        {
                        DataFlag = pResDescriptor->GetBlockDataFlag(FlagIndex);

                        //Those flag shouldn't be set when a raster file is open.
                        //HASSERT(((HRFDATAFLAG_OVERWRITTEN | HRFDATAFLAG_TOBECLEAR) & DataFlag) == 0);

                        if (DataFlag & HRFDATAFLAG_EMPTY)
                            {
                            HasDirtyFlag = true;
                            }

                        if (DataFlag & HRFDATAFLAG_DIRTYFORSUBRES)
                            {
                            HasSubResDirtyFlag = true;
                            }
                        }
                    }
                }

            // Sister file, generate sub-resolution ?
            //
            // If file with TileFlags, mark resolution not computed.
            if ((m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, HFC_READ_WRITE) != 0) &&
                HasDirtyFlag && (!GetRasterFileAccessMode().m_HasCreateAccess))
                pSubImageDesc[i].ResolutionComputed = false;
            else
                // Sister...
                pSubImageDesc[i].ResolutionComputed = !GetRasterFileAccessMode().m_HasCreateAccess;

            // Get Res description
            pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor(i+1);

            pSubImageDesc[i].Resolution = pResDescriptor->GetResolutionXRatio(); // WARNING NOW WE HAVE Y RATIO

            pSubImageDesc[i].UseDimension   = true;

            HASSERT(pResDescriptor->GetWidth() <= UINT32_MAX);
            HASSERT(pResDescriptor->GetHeight() <= UINT32_MAX);

            pSubImageDesc[i].DimX           = (uint32_t)pResDescriptor->GetWidth();
            pSubImageDesc[i].DimY           = (uint32_t)pResDescriptor->GetHeight();
            pSubImageDesc[i].DimBlockX      = pResDescriptor->GetBlockWidth();
            pSubImageDesc[i].DimBlockY      = pResDescriptor->GetBlockHeight();
            pSubImageDesc[i].ResamplingType = ResamplingMethodChangeType(pResDescriptor->GetDownSamplingMethod());

            if (m_MultiPixelType)
                {
                HFCPtr<HRABitmapBase> pTileTemplate(MakeBitmap(1, 1, pResDescriptor->GetPixelType()));

                HRFBlockType BlockType = pResDescriptor->GetBlockType();
                if ((BlockType == HRFBlockType::STRIP) || (BlockType == HRFBlockType::IMAGE))
                    pSubImageDesc[i].pTiledRaster = new HRAStripedRaster(pTileTemplate, pResDescriptor->GetBlockHeight(), 1, 1);
                else
                    pSubImageDesc[i].pTiledRaster = new HRATiledRaster(pTileTemplate, pResDescriptor->GetBlockWidth(), pResDescriptor->GetBlockHeight(), 1, 1);
                }
            }

        // Make TiledRaster or StripedRaster
        // We must do it, because the subResolution raster must ajuste de tileX size to the image
        // width. (the Class Striped do it)
        HFCPtr<HRABitmapBase> pTileTemplate;
        if (m_MultiPixelType)
            pTileTemplate = MakeBitmap(1, 1, m_pResDescriptor->GetPixelType());
        else
            pTileTemplate = m_pTileTemplate;

        HFCPtr<HRATiledRaster> pModel;
        HRFBlockType BlockType = m_pResDescriptor->GetBlockType();

        if ((BlockType == HRFBlockType::STRIP) || (BlockType == HRFBlockType::IMAGE))
            pModel = new HRAStripedRaster(pTileTemplate, m_pResDescriptor->GetBlockHeight(), 1, 1);
        else
            pModel = new HRATiledRaster(m_pTileTemplate.GetPtr(), m_pResDescriptor->GetBlockWidth(), m_pResDescriptor->GetBlockHeight(), 1, 1);

        HASSERT(m_pResDescriptor->GetWidth() <= UINT32_MAX);
        HASSERT(m_pResDescriptor->GetHeight() <= UINT32_MAX);

        pOutputRaster = new HRAPyramidRaster(pModel,
                                             m_pResDescriptor->GetWidth(),
                                             m_pResDescriptor->GetHeight(),
                                             pSubImageDesc,
                                             NbSubImage,
                                             this,
                                             GetPool(),
                                             NULL,
                                             m_pRasterFile->IsCompatibleWith(HRFFileId_VirtualEarth/*HRFVirtualEarthFile::CLASS_ID*/));
        // Disable TilesFlags if VE

        // Tell the pyramid that the RasterFile supports the LookAhead mechanism.
        if (m_pRasterFile->HasLookAheadByBlock(m_PageIndex))
            pOutputRaster->EnableLookAhead(false);
        else if (m_pRasterFile->HasLookAheadByExtent(m_PageIndex))
            pOutputRaster->EnableLookAhead(true);

        // Load Data Flag if present...
        //
        if ((m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, HFC_READ_WRITE) != 0) &&
            m_pResDescriptor->HasBlocksDataFlag() && (!GetRasterFileAccessMode().m_HasCreateAccess))
            LoadDataFlag(pOutputRaster);
        // else we assume the HRATiledRaster TileStatus are set to false

        *po_pRasterID = ID_PyramidRaster;
        //For a binary image loaded completly in memory, use only the principal resolution (optimization).
        if ((m_pResDescriptor->GetBitsPerPixel() == 1) &&
            ((dynamic_cast<HRFAdaptStripToImage*>(m_ResolutionEditor[0]) != 0) ||
             (dynamic_cast<HRFAdaptTileToImage*>(m_ResolutionEditor[0]) != 0) ||
             (dynamic_cast<HRFAdaptLineToImage*>(m_ResolutionEditor[0]) != 0) ||
             (m_pResDescriptor->GetBlockType() == HRFBlockType::IMAGE)))
            {
            pOutputRaster->UseOnlyFirstResolution(true);
            }
        }
    else
        {
        // Make TiledRaster or StripedRaster
        // We must do it, because the subResolution raster must adjust the tileX size to the image
        // width. (the Class Striped do it)
        HFCPtr<HRATiledRaster> pModel;
        HRFBlockType BlockType = m_pResDescriptor->GetBlockType();
        if ((BlockType == HRFBlockType::STRIP) || (BlockType == HRFBlockType::IMAGE))
            pModel = new HRAStripedRaster(m_pTileTemplate.GetPtr(), m_pResDescriptor->GetBlockHeight(), 1, 1);
        else
            pModel = new HRATiledRaster(m_pTileTemplate.GetPtr(), m_pResDescriptor->GetBlockWidth(), m_pResDescriptor->GetBlockHeight(), 1, 1);

        HASSERT(m_pResDescriptor->GetWidth() <= UINT32_MAX);
        HASSERT(m_pResDescriptor->GetHeight() <= UINT32_MAX);

        pOutputRaster = new HRAPyramidRaster (pModel,
                                              m_pResDescriptor->GetWidth(),
                                              m_pResDescriptor->GetHeight(),
                                              0,
                                              0,
                                              this,
                                              GetPool());

        *po_pRasterID = ID_PyramidRaster;

        // Tell the pyramid that the RasterFile supports the LookAhead mechanism.
        if (m_pRasterFile->HasLookAheadByBlock(m_PageIndex))
            pOutputRaster->EnableLookAhead(false);
        else if (m_pRasterFile->HasLookAheadByExtent(m_PageIndex))
            pOutputRaster->EnableLookAhead(true);
        }


    // Set Logical Shape if present
    //
    if (m_UseClipShape && m_pPageDescriptor->HasClipShape ())
        {
        HVEShape Shape(*m_pPageDescriptor->GetClipShape());

        if (m_pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
            Shape.SetCoordSys(pOutputRaster->GetPhysicalCoordSys());
        else
            Shape.SetCoordSys(pOutputRaster->GetCoordSys());

        pOutputRaster->SetShape(Shape);
        }


    // Set Histogram if present in the file.
    //
    if (m_pPageDescriptor->HasHistogram ())
        {
        bool HistoValid = false;
        HFCPtr<HRPHistogram> pHisto = m_pPageDescriptor->GetHistogram();
        for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize() && !HistoValid; i++)
            HistoValid = (pHisto->GetEntryCount(i) != 1L ? true : HistoValid);

        if (HistoValid)
            {
            pOutputRaster->InitializeHistogram(*m_pPageDescriptor->GetHistogram(), *pOutputRaster->GetPixelType());

            if (m_pRasterFile->GetAccessMode().m_HasWriteAccess )
                pOutputRaster->StartHistogramEditMode(); // Will keep histogram up to date
            }
        }

    // Assign ID
    // Check if MultiResolution
    if (*po_pRasterID == ID_PyramidRaster)
        {
        uint16_t NumberOfImage(pOutputRaster->CountSubImages());
        HFCPtr<HRAStoredRaster> pSubImage;

        for (uint16_t i = 0; i < NumberOfImage; i++)
            {
            pSubImage = pOutputRaster->GetSubImage(i);

            // Assign the SubRes with the Store
            pSubImage->SetID(ID_TiledRaster + i);    // each TiledRaster contain a different ID
            pSubImage->SetStore(this);

            //chck GT            RegisterObject(pList[i]);

            // TiledRaster in Pyramid not modified
            pSubImage->SetModificationState (false);
            pSubImage->GetPixelType()->SetModificationState(false);          // PixelType not modified
            ((HVEShape&)pSubImage->GetShape()).SetModificationState(false);  // Shape not modified
            pSubImage->GetPhysicalCoordSys()->SetModificationState(false);   // TransfoModel not modified
            pSubImage->GetCoordSys()->SetModificationState(false);
            }

        // Set resampling method for sub-resolution.
        SetResamplingForDecimationInRaster(pOutputRaster);
        }

    if (m_Resizable)
        m_Resizable = pOutputRaster->IsResizable();

    return pOutputRaster;
    }


//-----------------------------------------------------------------------------
// private
// LoadUnlimitedResolutionRaster - Create the Raster.
//
//      1- Message HRAPyramidRasterClosing,
//-----------------------------------------------------------------------------
HFCPtr<HRAUnlimitedResolutionRaster> HRSObjectStore::LoadUnlimitedResolutionRaster(HPMObjectID* po_pRasterID)
    {
    HFCPtr<HRAUnlimitedResolutionRaster> pOutputRaster;
    HFCPtr<HRATiledRaster> pModel;

    HRFBlockType BlockType = m_pResDescriptor->GetBlockType();
    if ((BlockType == HRFBlockType::STRIP) || (BlockType == HRFBlockType::IMAGE))
        pModel = new HRAStripedRaster (m_pTileTemplate.GetPtr(),
                                       m_pResDescriptor->GetBlockHeight(),
                                       1,
                                       1);
    else
        pModel = new HRATiledRaster (m_pTileTemplate.GetPtr(),
                                     m_pResDescriptor->GetBlockWidth(),
                                     m_pResDescriptor->GetBlockHeight(),
                                     1,
                                     1);


    HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
    pMultiResCapability = static_cast<HRFMultiResolutionCapability*>(m_pRasterFile->GetCapabilities()->
                           GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_READ_ONLY).GetPtr());
    HASSERT(pMultiResCapability != 0);

    pOutputRaster = new HRAUnlimitedResolutionRaster(pModel,
                                                     m_pResDescriptor->GetWidth(),
                                                     m_pResDescriptor->GetHeight(),
                                                     m_pPageDescriptor->GetMinWidth(),
                                                     m_pPageDescriptor->GetMinHeight(),
                                                     m_pPageDescriptor->GetMaxWidth(),
                                                     m_pPageDescriptor->GetMaxHeight(),
                                                     this,
                                                     GetPool());

    *po_pRasterID = ID_UnlimitedResolutionRaster;

    // Tell the pyramid that the RasterFile supports the LookAhead mechanism.
    // LookAHead is not support for multipage
    if (m_pRasterFile->HasLookAheadByBlock(m_PageIndex))
        pOutputRaster->EnableLookAhead(false);
    else if (m_pRasterFile->HasLookAheadByExtent(m_PageIndex))
        pOutputRaster->EnableLookAhead(true);


    // Set Logical Shape if present
    //
    if (m_UseClipShape && m_pPageDescriptor->HasClipShape ())
        {
        HVEShape Shape(*m_pPageDescriptor->GetClipShape());

        if (m_pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
            Shape.SetCoordSys(pOutputRaster->GetPhysicalCoordSys());
        else
            Shape.SetCoordSys(pOutputRaster->GetCoordSys());

        pOutputRaster->SetShape(Shape);
        }

    // Set Histogram if present in the file.
    //
    if (m_pPageDescriptor->HasHistogram ())
        {
        bool HistoValid = false;
        HFCPtr<HRPHistogram> pHisto = m_pPageDescriptor->GetHistogram();
        for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize() && !HistoValid; i++)
            HistoValid = (pHisto->GetEntryCount(i) != 1L ? true : HistoValid);

        if (HistoValid)
            {
            pOutputRaster->InitializeHistogram(*m_pPageDescriptor->GetHistogram(), *pOutputRaster->GetPixelType());

            if (m_pRasterFile->GetAccessMode().m_HasWriteAccess)
                pOutputRaster->StartHistogramEditMode(); // Will keep histogram up to date
            }
        }

    ConstructContextForRaster(m_pRasterFile, m_PageIndex, pOutputRaster);

    return pOutputRaster;
    }

//-----------------------------------------------------------------------------
// private
// LoadResizableRaster - Create the Raster.
//
//-----------------------------------------------------------------------------
HFCPtr<HRABitmapRLE> HRSObjectStore::LoadResizableRaster(HPMObjectID* po_pRasterID)
    {
    HFCPtr<HRABitmapRLE> pOutputRaster;
    *po_pRasterID = ID_ResizableRaster;

    HPRECONDITION(m_RasterWidth <= UINT32_MAX && m_RasterHeight <= UINT32_MAX);
    m_pTileTemplate = MakeBitmap(1,
                                 1,
                                 m_pResDescriptor->GetPixelType());

    HPOSTCONDITION(m_pTileTemplate->IsCompatibleWith(HRABitmapRLE::CLASS_ID));
    pOutputRaster = (HRABitmapRLE*)m_pTileTemplate->Clone();
    pOutputRaster->InitSize(m_RasterWidth, m_RasterHeight);
    pOutputRaster->SetID(0);
    pOutputRaster->SetStore (this);

    uint64_t PosX;
    uint64_t PosY;
    HFCPtr<HRAStoredRaster> pTile;
    pTile = LoadTile(0, 0, &PosX, &PosY);
    HPOSTCONDITION(pTile->IsCompatibleWith(HRABitmapRLE::CLASS_ID));

    // create a surface from the descriptor and toolbox
    HRASurface OutputSurface(pOutputRaster->GetSurfaceDescriptor());

    // now, copy to original data into the resizable raster
    HRAEditor OutputEditor(OutputSurface);

    PosX = (m_RasterWidth - m_pResDescriptor->GetWidth()) / 2;
    PosY = (m_RasterHeight - m_pResDescriptor->GetHeight()) / 2;

    HFCPtr<HCDPacketRLE> pInputBitmap(((const HFCPtr<HRABitmapRLE>&)pTile)->GetPacket());
    for (uint64_t i = 0; i < m_pResDescriptor->GetHeight(); i++)
        OutputEditor.SetRun((uint32_t)PosX,
                              (uint32_t)(PosY + i),
                              (uint32_t)m_pResDescriptor->GetWidth(),
                              pInputBitmap->GetLineBuffer((uint32_t)i));

    // we need to change the physical coord sys
    if (PosX != 0 || PosY != 0)
        {
        HGF2DTranslation TranslateModel (HGF2DDisplacement (-(double)PosX, -(double)PosY));

        pOutputRaster->SetTransfoModel(*TranslateModel.ComposeInverseWithDirectOf (*pOutputRaster->GetTransfoModel()));
        }

    CHECK_HUINT64_TO_HDOUBLE_CONV(PosX + m_pResDescriptor->GetWidth());
    CHECK_HUINT64_TO_HDOUBLE_CONV(PosY + m_pResDescriptor->GetHeight());

    HGF2DExtent RasterExtent((double)PosX,
                             (double)PosY,
                             (double)(PosX + m_pResDescriptor->GetWidth()),
                             (double)(PosY + m_pResDescriptor->GetHeight()),
                             pOutputRaster->GetPhysicalCoordSys());
    pOutputRaster->SetRasterExtent(RasterExtent);

    // Set Logical Shape if present
    //
    if (m_UseClipShape && m_pPageDescriptor->HasClipShape ())
        {
        HVEShape Shape(*m_pPageDescriptor->GetClipShape());

        if (m_pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
            Shape.SetCoordSys(pOutputRaster->GetPhysicalCoordSys());
        else
            Shape.SetCoordSys(pOutputRaster->GetCoordSys());

        pOutputRaster->SetShape(Shape);
        }

    // Set Histogram if present in the file.
    //
    if (m_pPageDescriptor->HasHistogram())
        {
        bool HistoValid = false;
        HFCPtr<HRPHistogram> pHisto = m_pPageDescriptor->GetHistogram();
        for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize() && !HistoValid; i++)
            HistoValid = (pHisto->GetEntryCount(i) != 1L ? true : HistoValid);

        if (HistoValid)
            {
            pOutputRaster->InitializeHistogram(*m_pPageDescriptor->GetHistogram(), *pOutputRaster->GetPixelType());

            if (m_pRasterFile->GetAccessMode().m_HasWriteAccess)
                pOutputRaster->StartHistogramEditMode(); // Will keep histogram up to date
            }
        }

    ConstructContextForRaster(m_pRasterFile, m_PageIndex, pOutputRaster);

    // Tile not modified
    pOutputRaster->SetModificationState (false);

    return pOutputRaster;
    }



//-----------------------------------------------------------------------------
// private
// CheckObjectClassKey
//-----------------------------------------------------------------------------
HCLASS_ID HRSObjectStore::CheckObjectClassKey(HPMPersistentObject& pi_rObj)
    {
    if (pi_rObj.IsCompatibleWith(HRATiledRaster::CLASS_ID))
        return HRATiledRaster::CLASS_ID;
    else if(pi_rObj.IsCompatibleWith(HRAPyramidRaster::CLASS_ID))
        return HRAPyramidRaster::CLASS_ID;
    else if(pi_rObj.IsCompatibleWith(HGF2DCoordSys::CLASS_ID))
        return HGF2DCoordSys::CLASS_ID;
    else if(pi_rObj.IsCompatibleWith(HVEShape::CLASS_ID))
        return HVEShape::CLASS_ID;

    return HRARaster::CLASS_ID;
    }


//-----------------------------------------------------------------------------
// private
// SaveBitmap
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::SaveBitmap(HRABitmapBase* pi_pBitmap)
    {
    HPRECONDITION(pi_pBitmap->IsCompatibleWith(HRABitmapRLE::CLASS_ID) || pi_pBitmap->IsCompatibleWith(HRABitmap::CLASS_ID));

    HSTATUS Result = H_SUCCESS;

    HFCPtr<HRABitmapRLE> pResizableBitmap;
    HRABitmapBase* pBitmap(pi_pBitmap);
    if (pBitmap->IsResizable())
        {
        uint64_t BitmapWidth;
        uint64_t BitmapHeight;
        pBitmap->GetSize(&BitmapWidth, &BitmapHeight);
        uint64_t RasterWidth;
        uint64_t RasterHeight;
        uint64_t RasterPosX;
        uint64_t RasterPosY;
        pBitmap->GetRasterSize(&RasterWidth, &RasterHeight, &RasterPosX, &RasterPosY);

        if (BitmapWidth != RasterWidth || BitmapHeight != RasterHeight)
            {
            HPRECONDITION(m_pTileTemplate->IsCompatibleWith(HRABitmapRLE::CLASS_ID));

            pResizableBitmap = static_cast<HRABitmapRLE*>(m_pTileTemplate->Clone());
            pResizableBitmap->InitSize(RasterWidth, RasterHeight);

            // create a surface from the descriptor and toolbox
            HRASurface RasterSurface(pBitmap->GetSurfaceDescriptor());

            // create the input editor
            HRAEditor rasterEditor(RasterSurface);

            HFCPtr<HCDPacketRLE> pResizablePacket(pResizableBitmap->GetPacket ());
            HFCPtr<HCDCodecHMRRLE1> pCodec(pResizablePacket->GetCodec());
            Byte* pRun;
            size_t RunSize;
            for (uint64_t i = 0; i < RasterHeight; i++)
                {
                pRun = (Byte*)rasterEditor.GetRun((HUINTX)RasterPosX,
                                                     (HUINTX)(RasterPosY + i),
                                                     (size_t)RasterWidth);

                RunSize = pCodec->GetSizeOf(pRun, (uint32_t)RasterWidth, 1);

                // Copy run buffer. Reuse current buffer whenever possible.
                if(pResizablePacket->GetLineBufferSize((uint32_t)i) < RunSize)
                    {
                    HASSERT(pResizablePacket->HasBufferOwnership());
                    pResizablePacket->SetLineBuffer((uint32_t)i, new Byte[RunSize], RunSize, 0/*dataSize*/);
                    pResizablePacket->SetBufferOwnership(true);
                    }
                memcpy(pResizablePacket->GetLineBuffer((uint32_t)i), pRun, RunSize);
                pResizablePacket->SetLineDataSize((uint32_t)i, RunSize);
                }

            pBitmap = pResizableBitmap.GetPtr();
            }
        }

    // A tile
    uint64_t BlockPosX;
    uint64_t BlockPosY;

    if (m_RasterObjectID == ID_PyramidRaster)
        {
        HRAPyramidRaster* pPyramid = (HRAPyramidRaster*)GetLoaded(ID_PyramidRaster);
        HASSERT(pPyramid != 0);
        pPyramid->UpdateNextRes(m_pSaveTileDescriptor->GetLevel(pi_pBitmap->GetID()),
                                m_pSaveTileDescriptor->GetIndex(pi_pBitmap->GetID()),
                                pi_pBitmap);
        }

#if 0
    // Write for the first time ?
    if (!pi_pBitmap->GetStore())
        {
        // In this case, my object already has his ID (setted by pyramid or tiled)
        pi_pBitmap->SetStore (this);
        }
#endif

    HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());
    CheckCurrentSubImageForSave (m_pSaveTileDescriptor->GetLevel (pi_pBitmap->IsResizable() ? 0 : pBitmap->GetID()));

    // Assert that the resolution editor for the current images has write access.
    if (m_ResolutionEditor[m_SaveCurSubImage]->GetAccessMode().m_HasWriteAccess || m_ResolutionEditor[m_SaveCurSubImage]->GetAccessMode().m_HasCreateAccess )
        {
        m_pSaveTileDescriptor->GetPositionFromID (pi_pBitmap->IsResizable() ? 0 : pi_pBitmap->GetID(),
                                                  &BlockPosX,
                                                  &BlockPosY);

        if (pBitmap->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
            {
            HRABitmapRLE* pBitmapRLE = (HRABitmapRLE*)pBitmap;
            Result = ExceptionsSafeWriteBlockRLE(m_ResolutionEditor[m_SaveCurSubImage], BlockPosX, BlockPosY, pBitmapRLE->GetPacket());
            }
        else if(pBitmap->IsCompatibleWith(HRABitmap::CLASS_ID))
            {
            HRABitmap* pBitmapUncompress = (HRABitmap*)pBitmap;
            Result = ExceptionsSafeWriteBlock(m_ResolutionEditor[m_SaveCurSubImage], BlockPosX, BlockPosY, (Byte*)pBitmapUncompress->GetPacket()->GetBufferAddress());
            }
        else
            {
            HASSERT(!"Unknown HRABitmap type");
            }
        }

    // indicate the the tile has been saved
    pi_pBitmap->SetModificationState(false);

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// SaveTiledRaster - Save all tiles in a tiledRaster
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveTiledRaster(HRATiledRaster* pi_pRaster)
    {
    if (pi_pRaster->ToBeSaved())
        {
        pi_pRaster->SaveTiles();
        pi_pRaster->SetModificationState(false);
        }
    }

//-----------------------------------------------------------------------------
// private
// SavePalette - Save the Palette.
//-----------------------------------------------------------------------------
void HRSObjectStore::SavePalette(HRAStoredRaster*   pi_pRaster,
                                 uint16_t    pi_Resolution)
    {
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_Resolution < m_ResolutionEditor.size());
    HPRECONDITION(pi_pRaster->GetPixelType()->CountIndexBits() > 0);

    if (pi_pRaster->GetPixelType()->ToBeSaved())
        {
        HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());

        m_ResolutionEditor[pi_Resolution]->SetPalette(pi_pRaster->GetPixelType()->GetPalette());
        pi_pRaster->GetPixelType()->SetModificationState(false);
        }
    }

//-----------------------------------------------------------------------------
// private
// SaveModel - Save the Model.
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveModel(HRAStoredRaster* pi_pRaster)
    {
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_pRaster->GetID() == ID_TiledRaster); // must be a tiled raster or
    // the first tiled raster of the pyramid

    HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());

    // Update CoordSys in the HRF file.
    if (pi_pRaster->GetPhysicalCoordSys()->ToBeSaved() ||
        pi_pRaster->GetCoordSys()->ToBeSaved())
        {
        m_pPhysicalCoordSys = (HFCPtr<HGF2DCoordSys>&)pi_pRaster->GetPhysicalCoordSys();
        m_pLogicalCoordSys  = (HFCPtr<HGF2DCoordSys>&)pi_pRaster->GetCoordSys();

        if (m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_WRITE_ONLY) != 0)
            m_pPageDescriptor->SetTransfoModel (*pi_pRaster->GetPhysicalCoordSys()->GetTransfoModelTo (
                                                    pi_pRaster->GetCoordSys()));

        pi_pRaster->GetPhysicalCoordSys()->SetModificationState(false);
        pi_pRaster->GetCoordSys()->SetModificationState(false);
        }
    }

//-----------------------------------------------------------------------------
// private
// SaveLogicalShape - Save the Logical shape.
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveLogicalShape(HRAStoredRaster* pi_pRaster)
    {
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_pRaster->GetID() == ID_TiledRaster); // must be a tiled raster or
    // the first tiled raster of the pyramid

    HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());

    if (m_UseClipShape &&
        (m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFClipShapeCapability::CLASS_ID, HFC_WRITE_ONLY) != 0) &&
        pi_pRaster->GetShape().ToBeSaved())
        {
        // Check if the File has a ClipShape
        if (m_pPageDescriptor->HasClipShape ())
            {
            HVEShape Shape(pi_pRaster->GetShape());

            if (m_pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
                Shape.ChangeCoordSys(pi_pRaster->GetPhysicalCoordSys());
            else
                Shape.ChangeCoordSys(pi_pRaster->GetCoordSys());

            HRFClipShape ClipShape(Shape, m_pPageDescriptor->GetClipShape()->GetCoordinateType());
            m_pPageDescriptor->SetClipShape(ClipShape);
            }
        else
            {
            // The file has not a shape then
            // Verify if the logicalShape is egal to Physical extent, if it is,
            // do not add the LogicalShape.
            HVEShape PhysicalShape(pi_pRaster->GetPhysicalExtent());
            PhysicalShape.Differentiate (pi_pRaster->GetShape());

            if (!PhysicalShape.IsEmpty())
                {
                // Physical and Logical shapes are different.
                HFCPtr<HRFCapability> pCapabilityLogical  = new HRFClipShapeCapability(HFC_WRITE_ONLY, HRFCoordinateType::LOGICAL);
                HFCPtr<HRFCapability> pCapabilityPhysical = new HRFClipShapeCapability(HFC_WRITE_ONLY, HRFCoordinateType::PHYSICAL);
                HVEShape Shape(pi_pRaster->GetShape());

                if (m_pRasterFile->GetCapabilities()->Supports(pCapabilityPhysical))
                    {
                    Shape.ChangeCoordSys(pi_pRaster->GetPhysicalCoordSys());
                    HRFClipShape ClipShape(Shape, HRFCoordinateType::PHYSICAL);
                    m_pPageDescriptor->SetClipShape (ClipShape);
                    }
                else if (m_pRasterFile->GetCapabilities()->Supports(pCapabilityLogical))
                    {
                    Shape.ChangeCoordSys(pi_pRaster->GetCoordSys());
                    HRFClipShape ClipShape(Shape, HRFCoordinateType::LOGICAL);
                    m_pPageDescriptor->SetClipShape (ClipShape);
                    }
                }
            }

        ((HVEShape&)pi_pRaster->GetShape()).SetModificationState(false);
        }
    }


//-----------------------------------------------------------------------------
// private
// SaveHistogram - Save the Histogram.
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveHistogram(HRAPyramidRaster* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);

    if (m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID,
                                                              HFC_WRITE_ONLY) != 0)
        {
        HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey()); // ?????

        const HRAHistogramOptions* pHisto = pi_pObj->GetHistogram();

        if ((pHisto) && (pHisto->GetHistogram()) && (pHisto->GetHistogram()->ToBeSaved()))
            {
            // Update Histogram in the HRF file.
            m_pPageDescriptor->SetHistogram (*(pHisto->GetHistogram()));

            pHisto->GetHistogram()->SetModificationState(false);
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// SaveResamplingMethod - Save the resampling method.
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveResamplingMethod(HRAPyramidRaster* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);

    if (m_pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFSubSamplingCapability::CLASS_ID,
                                                              HFC_READ_WRITE) != 0)
        {
        HRAPyramidRaster* pRaster = (HRAPyramidRaster*)pi_pObj;

        HFCMonitor RasterFileMonitor(m_pRasterFile->GetKey());
        HRFDownSamplingMethod DownSamplingMethod;
        HFCPtr<HRFResolutionDescriptor> pResDescriptor;
        for (uint32_t Res = 0; Res < m_pPageDescriptor->CountResolutions(); Res++)
            {
            pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor((uint16_t)Res);
            DownSamplingMethod = ResamplingMethodChangeType(pRaster->GetResamplingForSubResolution(Res));

            if (DownSamplingMethod != pResDescriptor->GetDownSamplingMethod())
                {
                pResDescriptor->SetDownSamplingMethod(DownSamplingMethod);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// SaveDataFlag - Extract Dataflags from ResolutionDescriptor and from
//                TiledRaster and save it in the HRFFile
//-----------------------------------------------------------------------------
void HRSObjectStore::SaveDataFlag(HRAPyramidRaster* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);

    if (m_pPageDescriptor->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID,
                                                                  HFC_READ_WRITE) != 0)
        {
        HFCPtr<HRFResolutionDescriptor> pResDescriptor;
        uint64_t                       NbFlag;

        uint16_t                 NumberOfImage(pi_pObj->CountSubImages());

        HASSERT(m_pPageDescriptor->CountResolutions() == NumberOfImage);

        // For each Resolution...
        for (uint16_t i=0; i<m_pPageDescriptor->CountResolutions(); i++)
            {
            pResDescriptor                  = m_pPageDescriptor->GetResolutionDescriptor(i);
            HRATileStatus& rTileFlags = pi_pObj->GetSubImage(i)->GetInternalTileStatusList(&NbFlag);
            HASSERT(NbFlag == (pResDescriptor->GetBlocksPerWidth() * pResDescriptor->GetBlocksPerHeight()));

            // UPDATE HRFFlags with the HRATileStatus - LIKE a MIRROR
            for (uint32_t j=0; j<NbFlag; j++)
                {
                if (rTileFlags.GetClearFlag(j))
                    // Add the DTO BE CLEAR flag to the HRF
                    pResDescriptor->SetBlockDataFlag(j, HRFDATAFLAG_TOBECLEAR);
                else
                    // Remove the TO BE CLEAR flag from the HRF
                    pResDescriptor->ClearBlockDataFlag(j, HRFDATAFLAG_TOBECLEAR);


                if (rTileFlags.GetDirtyForSubResFlag(j))
                    // Add the DIRTY FOR SUB RES flag to the HRF
                    pResDescriptor->SetBlockDataFlag(j, HRFDATAFLAG_DIRTYFORSUBRES);
                else
                    // Remove the DIRTY FOR SUB RES flag from the HRF
                    pResDescriptor->ClearBlockDataFlag(j, HRFDATAFLAG_DIRTYFORSUBRES);
                }
            // Call this method on the ResolutionEditor to be sure that all possible AdaptedResolutionEditor will
            // be up-to-date.
            m_ResolutionEditor[i]->SaveDataFlag();
            }
        }
    else
        // Update Sub-Resolution.
        pi_pObj->UpdateSubResolution();
    }


//-----------------------------------------------------------------------------
// private
// LoadDataFlag - Load the DataFlags from the HRFFile and set the Dataflags in
//                ResolutionDescriptor and in TiledRaster.
//-----------------------------------------------------------------------------
void HRSObjectStore::LoadDataFlag (HRAPyramidRaster* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);

    HFCPtr<HRFResolutionDescriptor> pResDescriptor;
    uint64_t                       NbFlag;

    uint16_t                 NumberOfImage(pi_pObj->CountSubImages());

    HASSERT(m_pPageDescriptor->CountResolutions() == NumberOfImage);

    // For each Resolution...
    for (uint16_t i=0; i<m_pPageDescriptor->CountResolutions(); i++)
        {
        pResDescriptor                  = m_pPageDescriptor->GetResolutionDescriptor(i);
        const HRFDataFlag* pFlags       = pResDescriptor->GetBlocksDataFlag();
        HRATileStatus& rTileFlags = pi_pObj->GetSubImage(i)->GetInternalTileStatusList(&NbFlag);
        HASSERT(NbFlag == (pResDescriptor->GetBlocksPerWidth() * pResDescriptor->GetBlocksPerHeight()));

        // Convert HRF Flags to HRATileStatus
        for (uint32_t j=0; j<NbFlag; j++)
            {
            if (pFlags[j] & HRFDATAFLAG_TOBECLEAR)
                {
                rTileFlags.SetClearFlag(j, true);

                // Remove the Flag
                //pResDescriptor->ClearBlockDataFlag(j, HRFDATAFLAG_TOBECLEAR);
                }

            if (pFlags[j] & HRFDATAFLAG_DIRTYFORSUBRES)
                {
                rTileFlags.SetDirtyForSubResFlag(j, true);
                // Remove the Flag
                //pResDescriptor->ClearBlockDataFlag(j, HRFDATAFLAG_DIRTYFORSUBRES);
                }
            else
                rTileFlags.SetDirtyForSubResFlag(j, false);
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// GetRasterFileAccessMode -
//-----------------------------------------------------------------------------

HFCAccessMode HRSObjectStore::GetRasterFileAccessMode () const
    {
    HFCAccessMode AccessMode;

    // Get the access from the sister file
    if (m_pRasterFile->IsCompatibleWith(HRFRasterFileResBooster::CLASS_ID))
        AccessMode = ((HFCPtr<HRFRasterFileResBooster>&)m_pRasterFile)->GetBoosterFile()->GetAccessMode();
    else
        AccessMode = m_pRasterFile->GetAccessMode();

    return AccessMode;
    }


//-----------------------------------------------------------------------------
// private
// SetResamplingForDecimationInRaster -
//-----------------------------------------------------------------------------

void HRSObjectStore::SetResamplingForDecimationInRaster (HRAPyramidRaster* pio_pRaster)
    {
    HPRECONDITION(pio_pRaster != 0);

    for (uint16_t i = 0; i < m_pPageDescriptor->CountResolutions(); i++)
        pio_pRaster->SetResamplingForSubResolution(
            ResamplingMethodChangeType(m_pPageDescriptor->GetResolutionDescriptor(i)->
                                       GetDownSamplingMethod()),
            false,
            i);
    }


//-----------------------------------------------------------------------------
// private
// ResamplingMethodChangeType - Convert from HRF to HGS
//-----------------------------------------------------------------------------
HGSResampling::ResamplingMethod HRSObjectStore::ResamplingMethodChangeType(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    HGSResampling::ResamplingMethod Result;

    // Find a name that fit with the given sampling methode.
    if (pi_DownSamplingMethod == HRFDownSamplingMethod::NEAREST_NEIGHBOUR)
        {
        Result = HGSResampling::NEAREST_NEIGHBOUR;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::AVERAGE)
        {
        Result = HGSResampling::AVERAGE;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::VECTOR_AWARENESS)
        {
        Result = HGSResampling::VECTOR_AWARENESS;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::UNKOWN)
        {
        Result = HGSResampling::UNDEFINED;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::ORING4)
        {
        Result = HGSResampling::ORING4;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::AVERAGE)
        {
        Result = HGSResampling::AVERAGE;
        }
    else if (pi_DownSamplingMethod == HRFDownSamplingMethod::NONE)
        {
        Result = HGSResampling::NONE;
        }
    else
        {
        HASSERT(0);
        Result = HGSResampling::UNDEFINED;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// ResamplingMethodChangeType - Convert from HGS to HRF
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRSObjectStore::ResamplingMethodChangeType(HGSResampling::ResamplingMethod pi_ResamplingMethod)
    {
    HRFDownSamplingMethod Result;

    // Find a name that fit with the given sampling methode.
    if (pi_ResamplingMethod == HGSResampling::NEAREST_NEIGHBOUR)
        {
        Result = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
        }
    else if (pi_ResamplingMethod == HGSResampling::AVERAGE)
        {
        Result = HRFDownSamplingMethod::AVERAGE;
        }
    else if (pi_ResamplingMethod == HGSResampling::VECTOR_AWARENESS)
        {
        Result = HRFDownSamplingMethod::VECTOR_AWARENESS;
        }
    else if (pi_ResamplingMethod == HGSResampling::UNDEFINED)
        {
        Result = HRFDownSamplingMethod::UNKOWN;
        }
    else if (pi_ResamplingMethod == HGSResampling::ORING4)
        {
        Result = HRFDownSamplingMethod::ORING4;
        }
    else if (pi_ResamplingMethod == HGSResampling::AVERAGE)
        {
        Result = HRFDownSamplingMethod::AVERAGE;
        }
    else if (pi_ResamplingMethod == HGSResampling::NONE)
        {
        Result = HRFDownSamplingMethod::NONE;
        }
    else
        {
        HASSERT(0);
        Result = HRFDownSamplingMethod::UNKOWN;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// ExceptionsSafeReadBlock
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::ExceptionsSafeReadBlock(HRFResolutionEditor* pi_pEditor,
                                                uint64_t             pi_PosX,
                                                uint64_t             pi_PosY,
                                                Byte*                po_pBuffer)
    {
    HSTATUS Result = H_SUCCESS;

    try
        {
        // If the pixeltype is not supported natively in the  pipeline, we convert it.
        //
        if (m_ConvertNativeToNormalize != 0)
            {
            Result = pi_pEditor->ReadBlock(pi_PosX, pi_PosY, po_pBuffer);

            // Convert in-place, starting by the last line
            // The first line is treating separately, overlap src - dst pixel values.
            uint32_t blockWidth = pi_pEditor->GetResolutionDescriptor()->GetBlockWidth();
            uint32_t blockHeight= pi_pEditor->GetResolutionDescriptor()->GetBlockHeight();
            uint32_t SrcByteByWidth = (m_ConvertNativeToNormalize->GetSourcePixelType()->CountPixelRawDataBits() * blockWidth + 7) / 8;
            uint32_t DstByteByWidth = (m_ConvertNativeToNormalize->GetDestinationPixelType()->CountPixelRawDataBits() * blockWidth + 7) / 8;

            Byte* pNativeData = po_pBuffer + (SrcByteByWidth * (blockHeight-1));
            po_pBuffer += (DstByteByWidth * (blockHeight-1));

            // The last(in fact the first one) line will convert outside of the "for" statement.
            for (uint32_t i=1; i<blockHeight; ++i)
            {
                m_ConvertNativeToNormalize->Convert(pNativeData, po_pBuffer, blockWidth);
                pNativeData -= SrcByteByWidth;
                po_pBuffer  -= DstByteByWidth;
            }

            // Convert the Last line, overlap
            
            HAutoPtr<Byte> pLineNativeData(new Byte[SrcByteByWidth]);
            memcpy(pLineNativeData, po_pBuffer, SrcByteByWidth); 
            m_ConvertNativeToNormalize->Convert(pLineNativeData, po_pBuffer, blockWidth);
            }
        else
            Result = pi_pEditor->ReadBlock(pi_PosX, pi_PosY, po_pBuffer);
        }
    catch(...)
        {
        HASSERT(!"HRSObjectStore::ExceptionsSafeReadBlock() ReadBlock throws an exception, it should not");
        Result = H_ERROR;
        }

    // Is it a fatal error?
    if(VerifyResult(Result, false))
        HRSBlockAccessIndicator::GetInstance()->NotifyReadBlockError(Result, m_pRasterFile->GetURL());

    return Result;
    }


//-----------------------------------------------------------------------------
// private
// ExceptionsSafeReadBlock
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::ExceptionsSafeReadBlock(HRFResolutionEditor* pi_pEditor,
                                                uint64_t             pi_PosX,
                                                uint64_t             pi_PosY,
                                                HFCPtr<HCDPacket>    po_pPacket)
    {
    HSTATUS Result = H_SUCCESS;

    try
        {
        Result = pi_pEditor->ReadBlock(pi_PosX, pi_PosY, po_pPacket);
        }
    catch(...)
        {
        HASSERT(!"HRSObjectStore::ExceptionsSafeReadBlock() ReadBlock throws an exception, it should not");
        Result = H_ERROR;
        }

    // Is it a fatal error?
    if(VerifyResult(Result, false))
        HRSBlockAccessIndicator::GetInstance()->NotifyReadBlockError(Result, m_pRasterFile->GetURL());

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// ExceptionsSafeReadBlockRLE
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::ExceptionsSafeReadBlockRLE(HRFResolutionEditor* pi_pEditor,
                                                   uint64_t             pi_PosX,
                                                   uint64_t             pi_PosY,
                                                   HFCPtr<HCDPacketRLE> po_pPacket)
    {
    HSTATUS Result = H_SUCCESS;

    try
        {
        Result = pi_pEditor->ReadBlockRLE(pi_PosX, pi_PosY, po_pPacket);
        }
    catch(...)
        {
        HASSERT(!"HRSObjectStore::ExceptionsSafeReadBlockRLE() ReadBlockRLE throws an exception, it should not");
        Result = H_ERROR;
        }

    // Is it a fatal error?
    if(VerifyResult(Result, false))
        HRSBlockAccessIndicator::GetInstance()->NotifyReadBlockError(Result, m_pRasterFile->GetURL());

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// ExceptionsSafeWriteBlock
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::ExceptionsSafeWriteBlock(HRFResolutionEditor* pi_pEditor,
                                                 uint64_t             pi_PosX,
                                                 uint64_t             pi_PosY,
                                                 Byte*                pi_pBuffer)
    {
    HPRECONDITION((pi_PosX +
                   pi_pEditor->GetResolutionDescriptor()->GetBlockWidth() <= UINT32_MAX) &&
                  (pi_PosY +
                   pi_pEditor->GetResolutionDescriptor()->GetBlockHeight() <= UINT32_MAX));

    HSTATUS Result = H_SUCCESS;

    try
        {
            // If the pixeltype is not supported natively in the  pipeline, we convert it.
            if (m_ConvertNormalizeToNative != 0)
            {
                uint32_t blockWidth = pi_pEditor->GetResolutionDescriptor()->GetBlockWidth();
                uint32_t SrcByteByWidth = (m_ConvertNormalizeToNative->GetSourcePixelType()->CountPixelRawDataBits() * blockWidth + 7) / 8;
                uint32_t DstByteByWidth = (m_ConvertNormalizeToNative->GetDestinationPixelType()->CountPixelRawDataBits() * blockWidth + 7) / 8;
                HAutoPtr<Byte> pNativeDataBuffer(new Byte[DstByteByWidth*pi_pEditor->GetResolutionDescriptor()->GetBlockHeight()]);
                Byte* pNativeData = pNativeDataBuffer;

                for (uint32_t i=0; i<pi_pEditor->GetResolutionDescriptor()->GetBlockHeight(); ++i)
                {
                    m_ConvertNormalizeToNative->Convert(pi_pBuffer, pNativeData, blockWidth);
                    pNativeData += DstByteByWidth;
                    pi_pBuffer  += SrcByteByWidth;
                }
                Result = pi_pEditor->WriteBlock((uint32_t)pi_PosX, (uint32_t)pi_PosY, pNativeDataBuffer);
            }
            else
              Result = pi_pEditor->WriteBlock((uint32_t)pi_PosX, (uint32_t)pi_PosY, pi_pBuffer);

        //Update the indicator only if an export is in progress.
        HUTExportProgressIndicator::GetInstance()->ContinueIteration(pi_pEditor->GetRasterFile(), m_SaveCurSubImage);
        }
    catch(...)
        {
        HASSERT(!"HRSObjectStore::ExceptionsSafeWriteBlock():WriteBlock throws an exception");
        Result = H_ERROR;
        }

    // Is it a fatal error?
    if (VerifyResult(Result, false))
        HRSBlockAccessIndicator::GetInstance()->NotifyWriteBlockError(Result, m_pRasterFile->GetURL());

    return Result;
    }

//-----------------------------------------------------------------------------
// private
// ExceptionsSafeWriteBlockRLE
//-----------------------------------------------------------------------------
HSTATUS HRSObjectStore::ExceptionsSafeWriteBlockRLE(HRFResolutionEditor* pi_pEditor,
                                                    uint64_t            pi_PosX,
                                                    uint64_t            pi_PosY,
                                                    HFCPtr<HCDPacketRLE> pi_pPacket)
    {
    HPRECONDITION((pi_PosX +
                   pi_pEditor->GetResolutionDescriptor()->GetBlockWidth() <= UINT32_MAX) &&
                  (pi_PosY +
                   pi_pEditor->GetResolutionDescriptor()->GetBlockHeight() <= UINT32_MAX));

    HSTATUS Result = H_SUCCESS;

    try
        {
        Result = pi_pEditor->WriteBlockRLE((uint32_t)pi_PosX, (uint32_t)pi_PosY, pi_pPacket);

        //Update the indicator only if an export is in progress.
        HUTExportProgressIndicator::GetInstance()->ContinueIteration(pi_pEditor->GetRasterFile(), m_SaveCurSubImage);
        }
    catch(...)
        {
        HASSERT(!"HRSObjectStore::ExceptionsSafeWriteBlockRLE():WriteBlock throws an exception");
        Result = H_ERROR;
        }

    // Is it a fatal error?
    if (VerifyResult(Result, false))
        HRSBlockAccessIndicator::GetInstance()->NotifyWriteBlockError(Result, m_pRasterFile->GetURL());

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
// Prepares the current load tile descriptor only if needed
//-----------------------------------------------------------------------------
void HRSObjectStore::CheckCurrentSubImageForLoad (uint32_t pi_SubImage)
    {
    if (pi_SubImage != m_LoadCurSubImage)
        {
        m_LoadCurSubImage = pi_SubImage;

        HFCPtr<HRFResolutionDescriptor> pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor((uint16_t)pi_SubImage);

        m_pLoadTileDescriptor = new HGFTileIDDescriptor (pResDescriptor->GetWidth (),
                                                         pResDescriptor->GetHeight (),
                                                         pResDescriptor->GetBlockWidth(),
                                                         pResDescriptor->GetBlockHeight());

        // change the tile template
        if (m_MultiPixelType)
            m_pTileTemplate = MakeBitmap (1, 1, pResDescriptor->GetPixelType());

        }
    }


//-----------------------------------------------------------------------------
// Private
// Prepares the current save tile descriptor only if needed
//-----------------------------------------------------------------------------
void HRSObjectStore::CheckCurrentSubImageForSave(uint32_t pi_SubImage)
    {
    if (pi_SubImage != m_SaveCurSubImage)
        {
        m_SaveCurSubImage = pi_SubImage;

        HFCPtr<HRFResolutionDescriptor> pResDescriptor = m_pPageDescriptor->GetResolutionDescriptor((uint16_t)pi_SubImage);

        m_pSaveTileDescriptor = new HGFTileIDDescriptor (pResDescriptor->GetWidth (),
                                                         pResDescriptor->GetHeight (),
                                                         pResDescriptor->GetBlockWidth(),
                                                         pResDescriptor->GetBlockHeight());
        }
    }

//----------------------------------------------------------------------------
// Class HRSBlockAccessIndicator
//----------------------------------------------------------------------------
HFC_IMPLEMENT_SINGLETON(HRSBlockAccessIndicator);

//-----------------------------------------------------------------------------
// public
// NotifyWriteBlockError
//-----------------------------------------------------------------------------
void  HRSBlockAccessIndicator::AddListener(HRSBlockAccessListener* pi_pListener)
    {
    HPRECONDITION(pi_pListener != 0);
    HPRECONDITION(find(m_Listeners.begin(), m_Listeners.end(), pi_pListener) == m_Listeners.end());
    m_Listeners.push_back(pi_pListener);
    }

//-----------------------------------------------------------------------------
// public
// NotifyWriteBlockError
//-----------------------------------------------------------------------------
void  HRSBlockAccessIndicator::RemoveListener(HRSBlockAccessListener* pi_pListener)
    {
    HPRECONDITION(pi_pListener != 0);
    HPRECONDITION(find(m_Listeners.begin(), m_Listeners.end(), pi_pListener) != m_Listeners.end());
    m_Listeners.erase(find(m_Listeners.begin(), m_Listeners.end(), pi_pListener));
    }

//-----------------------------------------------------------------------------
// public
// NotifyReadBlockError
//-----------------------------------------------------------------------------
void HRSBlockAccessIndicator::NotifyReadBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL)
    {
    for(ListenerList::const_iterator itr(m_Listeners.begin()); itr != m_Listeners.end(); ++itr)
        {
        (*itr)->OnReadBlockError(pi_Error, pi_pURL);
        }
    }

//-----------------------------------------------------------------------------
// public
// NotifyWriteBlockError
//-----------------------------------------------------------------------------
void HRSBlockAccessIndicator::NotifyWriteBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL)
    {
    for(ListenerList::const_iterator itr(m_Listeners.begin()); itr != m_Listeners.end(); ++itr)
        {
        (*itr)->OnWriteBlockError(pi_Error, pi_pURL);
        }
    }
//-----------------------------------------------------------------------------
// Public
// This method sets the current context that should be used when reading the
// image from a persistent source.
//-----------------------------------------------------------------------------
void HRSObjectStore::SetContext(const HFCPtr<HMDContext>& pi_rContext)
    {
    m_pRasterFile->SetContext(m_PageIndex, pi_rContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRAStoredRaster>  HRSObjectStore::LoadRaster()
    {
    HFCPtr<HRAStoredRaster> pObj;

    if (m_Resizable)
        pObj = Load(HRSObjectStore::ID_ResizableRaster);
    else if (m_pPageDescriptor->IsUnlimitedResolution())
        pObj = Load(HRSObjectStore::ID_UnlimitedResolutionRaster);
    else if (m_pPageDescriptor->CountResolutions() > 1)
        pObj = Load(HRSObjectStore::ID_PyramidRaster);
    else
        pObj = Load(HRSObjectStore::ID_TiledRaster);

    return pObj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRFRasterFile> const& HRSObjectStore::GetRasterFile() const
    {
    return m_pRasterFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t HRSObjectStore::GetPageIndex() const {return m_PageIndex;}
