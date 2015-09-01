//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/ImageppLib.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HFCException.h>

#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFIntergraphTileEditor.h>
#include <Imagepp/all/h/HRFIntergraphLineEditor.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFTypes.h>

#include <Imagepp/all/h/HCDCodecCCITTFax4.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecCRL8.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>

//-----------------------------------------------------------------------------
// Static initialization
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::m_sIntergraphLUT_ApplyReset = false;

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

HRFIntergraphFile::Creator::Creator(HCLASS_ID pi_ClassID)
    : HRFRasterFileCreator(pi_ClassID)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

WString HRFIntergraphFile::Creator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::Creator::IsMultiPage(HFCBinStream& pi_rSrcFile, uint32_t pi_HeaderBlockcount) const
    {
    bool MutliPageRaster = false;

    // Backup the current file cursor pos.
    uint32_t CursorFilePosition = (uint32_t)pi_rSrcFile.GetCurrentPos();

    if (pi_HeaderBlockcount > 1)
        {
        unsigned short ConcatenateFilePtr;

        // The position of the cfp field (concatenate file ptr) is always 528 if there is
        // more than one block.
        pi_rSrcFile.SeekToPos(528);
        pi_rSrcFile.Read(&ConcatenateFilePtr, sizeof(unsigned short));

        // Replace the file cursor where it was.
        pi_rSrcFile.SeekToPos(CursorFilePosition);

        // A value greater than zero mean we have a multipage file.
        MutliPageRaster = (ConcatenateFilePtr != 0);
        }
    return MutliPageRaster;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphFile::HRFIntergraphFile(const HFCPtr<HFCURL>& pi_rURL,
                                     HFCAccessMode         pi_AccessMode,
                                     uint64_t             pi_Offset)
    :HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_pApplicationPacket  = 0;
    m_BitPerPixel         = 0;
    m_BlockHeaderAddition = 0;
    m_CurentPageIndex     = 0;

    m_pRedLUTColorTable   = 0;
    m_pGreenLUTColorTable = 0;
    m_pBlueLUTColorTable  = 0;

    m_IsOpen               = false;
    m_HasHeaderFilled      = false;
    m_OverviewCountChanged = false;
    m_LUTColorCorrected    = false;
    m_LUTColorPacketOffset = 0;

    m_DataTypeCode          = -1;
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFIntergraphFile::HRFIntergraphFile(const HFCPtr<HFCURL>& pi_rURL,
                                     HFCAccessMode         pi_AccessMode,
                                     uint64_t             pi_Offset,
                                     bool                 pi_DontOpenFile)
    :HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    // To do ....
    m_BitPerPixel         = 0;
    m_pApplicationPacket  = 0;
    m_BlockHeaderAddition = 0;
    m_CurentPageIndex     = 0;

    m_pRedLUTColorTable   = 0;
    m_pGreenLUTColorTable = 0;
    m_pBlueLUTColorTable  = 0;

    m_IsOpen               = false;
    m_HasHeaderFilled      = false;
    m_OverviewCountChanged = false;
    m_LUTColorCorrected    = false;
    m_LUTColorPacketOffset = 0;

    m_DataTypeCode         = -1;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRFIntergraphFile::~HRFIntergraphFile()
    {
    // Empty de stl vector of IntergraphBlockSupp
    while (m_pIntergraphBlockSupp.size() > 0)
        {
        IntergraphHeaderBlockP pIntergraphBlockSupp = m_pIntergraphBlockSupp.front();
        delete []pIntergraphBlockSupp;
        m_pIntergraphBlockSupp.erase(m_pIntergraphBlockSupp.begin());
        }

    // Empty the vector of IntergraphResolutionDescriptors
    while (m_IntergraphResDescriptors.size() > 0)
        {
        IntergraphResolutionDescriptor* pIntergraphResolutionDescriptor = m_IntergraphResDescriptors.front();
        delete[] pIntergraphResolutionDescriptor->pOverview;
        delete[] pIntergraphResolutionDescriptor->pOverviewEntry;
        delete[] pIntergraphResolutionDescriptor->pTileDirectoryEntry;
        delete   pIntergraphResolutionDescriptor;
        m_IntergraphResDescriptors.erase(m_IntergraphResDescriptors.begin());
        }

    delete m_pApplicationPacket;

    delete[] m_pRedLUTColorTable;
    delete[] m_pGreenLUTColorTable;
    delete[] m_pBlueLUTColorTable;
    }

//-----------------------------------------------------------------------------
// protected
// CalcNumberOfPage: Look in the raster file how many page are stored
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::CalcNumberOfPage () const
    {
    HPRECONDITION(m_pIntergraphFile != 0);

    // At this time we only support one page per file but this format can support
    // multiple page, see CFP (concatenate file ptr) field in the Header_Block_2
    // return(1);

    // Backup the current file cursor pos.
    uint32_t CursorFilePosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();
    uint32_t NextPageOffset     = 0;
    uint32_t PageCount          = 0;
    int32_t BlockNumInHeader;

    IntergraphHeaderBlocks TempHeaderBlocks;

    do
        {
        m_pIntergraphFile->SeekToPos(NextPageOffset);

        // Read the Header, and check if we have read it at all.
        m_pIntergraphFile->Read(&TempHeaderBlocks.IBlock1, sizeof(IntergraphHeaderBlock1));
        BlockNumInHeader = (TempHeaderBlocks.IBlock1.wtf + 2) / 256;

        if (((TempHeaderBlocks.IBlock1.wtf + 2) % 256) == 0)
            {
            // We always supposed to have at least two block
            // but there is always an exception..
            if (BlockNumInHeader > 1)
                m_pIntergraphFile->Read(&TempHeaderBlocks.IBlock2, sizeof(IntergraphHeaderBlock2));
            else
                CreateHeaderBlock2(TempHeaderBlocks);  // Set the Block2 as empty in a valid state.
            }

        NextPageOffset = TempHeaderBlocks.IBlock2.cfp;
        PageCount++;
        }
    while (NextPageOffset != 0 && !m_pIntergraphFile->EndOfFile());

    // Get back where we are before scanning the file.
    m_pIntergraphFile->SeekToPos(CursorFilePosition);

    HASSERT(PageCount > 0);

    return PageCount;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor: File manipulation
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFIntergraphFile::CreateResolutionEditor(uint32_t       pi_PageIndex,
                                                               unsigned short pi_Resolution,
                                                               HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_PageIndex) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_PageIndex)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_PageIndex)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pIntgResEditor = 0;

    if (GetAccessMode().m_HasCreateAccess)
        {
        // Update the packet overview content...
        if (pi_Resolution)
            {
            // Seek at the end of the file...
            //m_pIntergraphFile->SeekToEnd();
            uint32_t CursorFilePosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();

            // Lock the sister file.
            HFCLockMonitor SisterFileLock (GetLockManager());

            // Get the cursor pos, now we have the next offset overview...
            // Be sure to update the packet overview before writing the tile directory
            UpdatePacketOverview(CursorFilePosition, pi_Resolution);
            if (GetPageDescriptor(pi_PageIndex)->GetResolutionDescriptor(pi_Resolution)->GetBlockType()
                == HRFBlockType::TILE)
                WriteTileDirectory(pi_Resolution);

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }
        }

    if (GetPageDescriptor(pi_PageIndex)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE)
        {
        pIntgResEditor = new HRFIntergraphTileEditor(this,
                                                     pi_PageIndex,
                                                     pi_Resolution,
                                                     pi_AccessMode,
                                                     *m_IntergraphResDescriptors[pi_Resolution],
                                                     m_ListOfFreeBlock);
        }
    else
        {
        pIntgResEditor = new HRFIntergraphLineEditor(this,
                                                     pi_PageIndex,
                                                     pi_Resolution,
                                                     pi_AccessMode,
                                                     *m_IntergraphResDescriptors[pi_Resolution]);
        }
    return pIntgResEditor;
    }

//-----------------------------------------------------------------------------
// Public
// Save
//-----------------------------------------------------------------------------

void HRFIntergraphFile::Save()
    {
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            {
            // Update the modification to the file
            for (uint32_t Page=0; Page < CountPages(); Page++)
                {
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

                // Lock the sister file.
                HFCLockMonitor SisterFileLock (GetLockManager());

                // Update the palette
                if (pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
                    {
                    if (pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType()->CountPixelRawDataBits() == 8)
                        SetPalette(pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType()->GetPalette());
                    }

                IntergraphTagUpdate(pPageDescriptor);

                // Update the TransfoModel
                if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                    WriteTransfoModel(pPageDescriptor->GetTransfoModel());

                // If the packet has changed...
                if (m_OverviewCountChanged)
                    {
                    UpdatePacketOverview(0, 0);
                    }

                // Unlock the sister file
                SisterFileLock.ReleaseKey();
                }
            m_pIntergraphFile->Flush();
            }
        }
    }
//-----------------------------------------------------------------------------
// Protected
// Open:
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pIntergraphFile);

    // Be sure the Intergraph raster file is NOT already open.
    m_pIntergraphFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// private
// Create
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::Create()
    {
    // Be sure the Intergraph raster file is NOT already open.
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pIntergraphFile);

    m_pIntergraphFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Private
// Close
//-----------------------------------------------------------------------------

void HRFIntergraphFile::Close()
    {
    //HPRECONDITION(m_pIntergraphFile != 0);

    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.

    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        try
            {
            Save();

            // Set the open flag to false
            m_IsOpen = false;
            }
        catch(...)
            {
            // Simply stop exceptions in the destructor
            // We want to known if a exception is throw.
            HASSERT(0);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// IntergraphTagUpdate
//-----------------------------------------------------------------------------

void HRFIntergraphFile::IntergraphTagUpdate(HFCPtr<HRFPageDescriptor> pi_pPageDescriptor)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool RasterFileNeedUpdate = false;

    // Unkown unit
    unsigned short Unit = 2;
    double XResolution = 0;
    double YResolution = 0;

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;

    // Tag rewrite...
    for (TagIterator  = pi_pPageDescriptor->GetTags().begin();
         TagIterator != pi_pPageDescriptor->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

        // Update the Tag if changed...
        if (pi_pPageDescriptor->TagHasChanged(*pTag)) 
            {
            RasterFileNeedUpdate = true;

            // X Resolution Tag
            if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
                {
                XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
                }
            // Y Resolution Tag
            else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
                {
                YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
                }
            // Resolution Unit Tag
            else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
                {
                Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();
                }

            // Set the resolution (in meter)
            if(XResolution !=0 || YResolution !=0)
                {
                // Unit is centimeter
                if (Unit == 3)
                    {
                    // Write in micron.
                    m_IntergraphHeader.IBlock1.drs = (short) (1.0 / XResolution / 0.0001);
                    }
                // Unit is inch
                else if (Unit == 2)
                    {
                    // Write dpi.
                    m_IntergraphHeader.IBlock1.drs = (short)(-XResolution);
                    }
                else
                    {
                    // Write dpi.
                    m_IntergraphHeader.IBlock1.drs = (short) 0;
                    }
                }
            }
        }

    // Update the raster file...
    if (RasterFileNeedUpdate)
        {
        // Backup the last cursor position...
        int32_t CurrentCursorPosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();

        if (CurrentCursorPosition >= 0)
            {
            // Be sure to be at the beginning of the file.
            m_pIntergraphFile->SeekToPos(GetpageOffset(m_CurentPageIndex));

            // Afterall flush theses HeaderBlocks on the disk...
            m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock1,
                                     sizeof(Byte) * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

            // Reset the cursor to it's old position.
            m_pIntergraphFile->SeekToPos(CurrentCursorPosition);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// WriteTransfoModel
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION(pi_rpTransfoModel != 0);
    HPRECONDITION(SharingControlIsLocked());

    bool Status = false;

    Status = SetGlobalTransfoModel(pi_rpTransfoModel);

    // Check if transfo model could be set
    if (Status)
        {
        // Transfomodel could be set ...
        // Backup the last cursor position...
        int32_t CurrentCursorPosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();
        if (CurrentCursorPosition >= 0)
            {
            // Be sure to be at the beginning of the file.
            m_pIntergraphFile->SeekToPos(GetpageOffset(m_CurentPageIndex));

            // Afterall flush theses HeaderBlocks on the disk...
            m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock1,
                                     sizeof(Byte) * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

            // Reset the cursor to it's old position.
            m_pIntergraphFile->SeekToPos(CurrentCursorPosition);

            }
        else
            Status = false;
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::GetWidth (unsigned short pi_SubImage) const
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    uint32_t ImageWidth = m_IntergraphHeader.IBlock1.ppl;

    // Check if we want a sub-resolution width instead.
    if (pi_SubImage != 0)
        ImageWidth = m_IntergraphResDescriptors[pi_SubImage]->pOverview->NumberPixels;

    // Assure a minimum width
    HPOSTCONDITION(ImageWidth > 0);

    return ImageWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::GetHeight (unsigned short pi_SubImage) const
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    uint32_t ImageHeight = m_IntergraphHeader.IBlock1.nol;

    // Check if we want a sub-resolution height instead.
    if (pi_SubImage != 0)
        ImageHeight = m_IntergraphResDescriptors[pi_SubImage]->pOverview->NumberLines;

    // Assure a minimum height
    HPOSTCONDITION(ImageHeight > 0);

    return ImageHeight;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::Set8BitsPalette(uint32_t          pi_PaletteType,
                                         HRPPixelPalette* po_pPalette)
    {
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_PaletteType <= 3);
    HPRECONDITION(po_pPalette != 0);

    bool  Status = true;
    Byte RGBValue[3];
    Byte IGDSPaletteBuffer[768];
    uint32_t ColorEntry;
    uint32_t index;
    uint32_t i;

    EnvironV* PaletteEnvironV;

    // In intergraph file format we can have three kind a palette in 8 bit format.
    // So, build each palette with the appropriate method.
    switch(pi_PaletteType)
        {
            // We do not have to treat the first case (Grayscale palette)
            // because it is automatically construct by it's pixeltype : HRPPixelTypeV8Gray8
//      case NONE :          // Grayscale palette
//          break;
        case IGDS :          // IGDS Color Table (standard indexed 8 bit palette)
            memcpy (IGDSPaletteBuffer, m_IntergraphHeader.IBlock2.use, 256);
            if (m_BlockNumInHeader > 2)
                memcpy (&IGDSPaletteBuffer[256], m_pIntergraphBlockSupp[0], 512);
            else
                memset(&IGDSPaletteBuffer[256], 0, 512);
            // Build a 256 color palette from header in the raster file
            for (i=0; i<256; i++)
                {
                RGBValue[0] = IGDSPaletteBuffer[i*3];
                RGBValue[1] = IGDSPaletteBuffer[i*3 + 1];
                RGBValue[2] = IGDSPaletteBuffer[i*3 + 2];
                po_pPalette->SetCompositeValue(i, RGBValue);
                }
            break;
        case ENVIRON_V :     // Environ-V Color Table
            // Initialize color palette.
            for (i=0 ; i<256; i++ )
                {
                RGBValue[0] = 0;
                RGBValue[1] = 0;
                RGBValue[2] = 0;
                po_pPalette->SetCompositeValue(i, RGBValue);
                }
            ColorEntry = m_IntergraphHeader.IBlock2.cte;
            index      = 0;
            // Fill all entry with the specific value contained in the header.
            for ( i=0; i < ColorEntry; i++)
                {
                if ((i > 1) && ((i % 64) == 0))
                    index++;
                PaletteEnvironV = (EnvironV*)m_pIntergraphBlockSupp[index];
                if (PaletteEnvironV[i - (index * 64)].Slot < 256)
                    {
                    RGBValue[0] = PaletteEnvironV[i - (index * 64)].Red   >> 8;
                    RGBValue[1] = PaletteEnvironV[i - (index * 64)].Green >> 8;
                    RGBValue[2] = PaletteEnvironV[i - (index * 64)].Blue  >> 8;
                    po_pPalette->SetCompositeValue(PaletteEnvironV[i - (index * 64)].Slot, RGBValue);
                    }
                }
            break;
        default :
            // If we go here, it's because we've got an invalide pi_PaletteType parameter.
            Status = false;
            break;
        }
    // Be sure to have a valid palette type
    HPOSTCONDITION(Status);
    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// Return the height of a tile.
//-----------------------------------------------------------------------------
uint32_t HRFIntergraphFile::GetTileHeight(unsigned short pi_SubImage) const
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    // Assure a minimum tile height
    HASSERT(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize > 0);

    return m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize;
    }

//-----------------------------------------------------------------------------
// Public
// Return the width of a tile.
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::GetTileWidth(unsigned short pi_SubImage) const
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    // Assure a minimum tile height
    HASSERT(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize > 0);

    return m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize;
    }

//-----------------------------------------------------------------------------
// Public
// Return the width of a tile.
//-----------------------------------------------------------------------------

void HRFIntergraphFile::CheckNotNullTransfoModel()
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    bool NonZeroFound = false;

    for (int i=0; i< 16; i++)
        {
        if (!HDOUBLE_EQUAL_EPSILON(m_IntergraphHeader.IBlock1.trn[i],0.0))
            NonZeroFound = true;
        }
    if (!NonZeroFound)
        m_ZeroMatrixFound = true;
    else
        m_ZeroMatrixFound = false;
    }


#if (0)
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

void HRFIntergraphFile::GetTransfoModel()
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    unsigned short SubImage = 0;

    // Delete the previous transformation.
    HFCPtr<HGF2DTransfoModel> pTransfo;

    CheckNotNullTransfoModel();

    HFCPtr<HGF2DTransfoModel> pRotTransfo = new HGF2DIdentity();

    // The stored matrix in the header was valid only if it's not null and
    // if the file version is 3.0 or higher...
    // !!!! Intergraph Patch : Intergraph do some strange thing... At this time any
    // binary raster file dont use the TRN matrix. So check if the raster is bi-level.
    if (!m_ZeroMatrixFound && (m_IntergraphHeader.IBlock1.ver >= 3) && m_BitPerPixel > 1)
        {

        double Degree = PI/180;
        // we can enter here only if we have a non-null transfo modele
        // or not an identity matrix
        HFCMatrix<3, 3> MyMatrix;
        pRotTransfo = new HGF2DProjective();

        MyMatrix[0][0] = m_IntergraphHeader.IBlock1.trn[0];
        MyMatrix[0][1] = m_IntergraphHeader.IBlock1.trn[1];
        MyMatrix[0][2] = 0.0;
        MyMatrix[1][0] = m_IntergraphHeader.IBlock1.trn[4];
        MyMatrix[1][1] = m_IntergraphHeader.IBlock1.trn[5];
        MyMatrix[1][2] = 0.0;
        MyMatrix[2][0] = m_IntergraphHeader.IBlock1.trn[12];
        MyMatrix[2][1] = m_IntergraphHeader.IBlock1.trn[13];
        MyMatrix[2][2] = m_IntergraphHeader.IBlock1.trn[15];

        //static_cast<HFCPtr<HGF2DProjective> >(pRotTransfo)->SetByMatrix(MyMatrix);
        ((HFCPtr<HGF2DProjective> &)pRotTransfo)->SetByMatrix(MyMatrix);

        // Translation transfo model
        pTransfo = new HGF2DTranslation(HGF2DDisplacement(m_IntergraphHeader.IBlock1.trn[3]),
                                                          m_IntergraphHeader.IBlock1.trn[7])));
        }
    else
        {
        pTransfo = new HGF2DAffine();

        if (!m_ZeroMatrixFound)
            {
            // There is a matrix, however we are in version 2
            // therefore some of it is no good
            // static_cast<HFCPtr<HGF2DAffine> >(pTransfo)
            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetByMatrixParameters( m_IntergraphHeader.IBlock1.trn[3],
                                                                      m_IntergraphHeader.IBlock1.trn[0],
                                                                      m_IntergraphHeader.IBlock1.trn[1],
                                                                      m_IntergraphHeader.IBlock1.trn[7],
                                                                      m_IntergraphHeader.IBlock1.trn[4],
                                                                      m_IntergraphHeader.IBlock1.trn[5]);

            // Nullify rotation
            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetRotation(0.0));

            // Make sure scales are positive
            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetXScaling(fabs(((HFCPtr<HGF2DAffine> &)pTransfo)->GetXScaling()));
            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetYScaling(fabs(((HFCPtr<HGF2DAffine> &)pTransfo)->GetYScaling()));

            }
        else
            {
            // No matrix provided
            double Scale;
            double Degree = (PI/180);

            ((HFCPtr<HGF2DAffine> &)pTransfo)->AddTranslation(
                HGF2DDisplacement(m_IntergraphHeader.IBlock1.trn[3]),
                                  m_IntergraphHeader.IBlock1.trn[7])));

            // We dont set any rotation on 1 bit raster to follow the Intergraph product IRasB
            // functionnality.
            if (m_BitPerPixel != 1)
                ((HFCPtr<HGF2DAffine> &)pTransfo)->AddRotation(m_IntergraphHeader.IBlock1.rot * Degree);

            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetAnorthogonality(m_IntergraphHeader.IBlock1.skw * Degree);

            // Set the x scaling factor and be sure to not have a null factor.
            Scale = sqrt(pow(m_IntergraphHeader.IBlock1.trn[0], 2.0) + pow(m_IntergraphHeader.IBlock1.trn[4], 2.0));
            if (HDOUBLE_EQUAL_EPSILON(Scale, 0))
                Scale = m_IntergraphHeader.IBlock1.xdl;
            if (HDOUBLE_EQUAL_EPSILON(Scale, 0))
                Scale = 1.0;
            ((HFCPtr<HGF2DAffine> &)pTransfo)->SetXScaling(Scale);

            // Set the y scaling factor and be sure to not have a null factor.
            Scale = sqrt(pow(m_IntergraphHeader.IBlock1.trn[1], 2.0) + pow(m_IntergraphHeader.IBlock1.trn[5], 2.0));
            if (HDOUBLE_EQUAL_EPSILON(Scale, 0))
                Scale = m_IntergraphHeader.IBlock1.ydl;
            if (HDOUBLE_EQUAL_EPSILON(Scale, 0))
                Scale = 1.0;
            ((HFCPtr<HGF2DAffine> &) pTransfo)->SetYScaling(Scale);
            }
        }

    m_pTransfoModel = new HGF2DAffine();

    //static_cast<HFCPtr<HGF2DAffine> >(m_pTransfoModel)
    ((HFCPtr<HGF2DAffine> &)(m_pTransfoModel))->SetYScaling(-1);

    // Compose rotation transformation matrix (Applicable only for COT file since all others
    // Have identity
    m_pTransfoModel = ((HFCPtr<HGF2DAffine> &)m_pTransfoModel)->ComposeInverseWithDirectOf(*pRotTransfo);

    InitScanlineOrientation();

    if (m_BitPerPixel > 1)
        {
        // IRas Translation Model....
        int32_t IrasTX = 0;
        int32_t IrasTY = 0;

        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            IrasTY += GetHeight(SubImage);
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            IrasTX -= GetWidth(SubImage);
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            IrasTY += GetHeight(SubImage);
            IrasTX -= GetWidth(SubImage);
            }

        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            IrasTX -= GetWidth(SubImage);
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            IrasTY += GetHeight(SubImage);
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            IrasTY += GetHeight(SubImage);
            IrasTX -= GetWidth(SubImage);
            }

        HGF2DTranslation IRasTranslationModel(HGF2DDisplacement(IrasTX),
                                                                IrasTY)));

        IRasTranslationModel.Reverse();
        m_pTransfoModel = ((HFCPtr<HGF2DAffine> &)m_pTransfoModel)->ComposeInverseWithDirectOf(IRasTranslationModel);

        // SLO Transfo Model
        HRFSLOModelComposer SLOComposer;
        HGF2DPosition Origin(0,0);

        // create the Scanline Model
        m_SLOModel = SLOComposer.GetIntergraphTransfoModelFrom(m_ScanlineOrientation,
                                                               GetWidth(SubImage),
                                                               GetHeight(SubImage),
                                                               Origin);

        ((HFCPtr<HGF2DAffine> &)(m_SLOModel))->Reverse();
        m_pTransfoModel = m_pTransfoModel->ComposeInverseWithDirectOf(*m_SLOModel);
        }

    // Add translation model of COT and everything else for other image types.
    m_pTransfoModel = ((HFCPtr<HGF2DAffine> &)m_pTransfoModel)->ComposeInverseWithDirectOf(*pTransfo);
    }

#endif

//-----------------------------------------------------------------------------
// Protected
// CreateScanlineOrentationModel
//-----------------------------------------------------------------------------

void HRFIntergraphFile::InitScanlineOrientation()
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    // In the switch case bellow, we MUST use the hard coded number instead of the
    // define enum to be sure that nobody will change the order in the enum structure.
    // These value were provide from Intergraph File Format.

    switch (m_IntergraphHeader.IBlock1.slo)
        {

        case 0:            // Origin : Upper Left      Line Orentation : Vertical
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_VERTICAL;
            break;

        case 1:            // Origin : Upper Right     Line Orentation : Vertical
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_VERTICAL;
            break;

        case 2:            // Origin : Lower Left      Line Orentation : Vertical
            m_ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_VERTICAL;
            break;

        case 3:            // Origin : Lower Right     Line Orentation : Vertical
            m_ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_VERTICAL;
            break;

        case 4:            // Origin : Upper Left      Line Orentation : Horizontal
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            break;

        case 5:            // Origin : Upper Right     Line Orentation : Horizontal
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL;
            break;

        case 6:            // Origin : Lower Left      Line Orentation : Horizontal
            m_ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
            break;

        case 7:            // Origin : Lower Right     Line Orentation : Horizontal
            m_ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL;
            break;
        default :          // Invalid Scan Line Orientation. In release keep the  default : SLO 4
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            HASSERT(false);
            break;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Return the width of a tile.
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::InitOpenedFile(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_IsOpen);
    HRPPixelPalette* pPalette;
    bool Status = false;
    uint32_t i;

    // Be sure to intialise all members on each call of this method.
    m_pApplicationPacket   =  0;
    m_pJpegPacketPacket    =  0;
    m_SubResolution        =  0;
    m_CurrentSubImage      = -1;
    m_BitPerPixel          =  0;

    // Check if the file has been open in read and/or write mode...
    if (pi_pPage != 0)
        {
        HPRECONDITION(GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess);

        m_SubResolution   = pi_pPage->CountResolutions() - 1;
        CreateFileHeader(pi_pPage);
        }
    else
        {
        m_DataTypeCode    =  0;
        FillFileHeader();
        }

    // We cannot use this method if the file header is not properly filled.
    if (m_HasHeaderFilled)
        {
        InitScanlineOrientation();
        // Determine the data type code (compression sheme) of the raster file.
        if (m_HasTileAccess)
            m_DataTypeCode = m_IntergraphResDescriptors[0]->TileDirectory.DataTypeCode;
        else
            m_DataTypeCode = m_IntergraphHeader.IBlock1.dtc;

        switch(GetDatatypeCode())
            {
            case  COT :                                              // COT
                // We have different pixel type according the palette type
                // defined in the header block 2.
                if (m_IntergraphHeader.IBlock2.ctv > 0)
                    {
                    m_pPixelType = new HRPPixelTypeI8R8G8B8();
                    pPalette = (HRPPixelPalette*)&(m_pPixelType->LockPalette());
                    Set8BitsPalette(m_IntergraphHeader.IBlock2.ctv, pPalette);
                    m_pPixelType->UnlockPalette();
                    }
                else
                    m_pPixelType = new HRPPixelTypeV8Gray8();
                // Dont build a Codec because we dont have any compression in this file format.
                for (i = 0; i <= m_SubResolution; i++)
                    m_IntergraphResDescriptors[i]->pCodec = 0;

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

                m_BitPerPixel = 8;
                Status = true;
                break;

            case RLE :                                              // RLE
                // Set a pixelType...
                m_pPixelType = new HRPPixelTypeV1Gray1();
                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        {
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecHMRRLE1(GetTileWidth((unsigned short)i), GetTileHeight((unsigned short)i));
                        // We set the padding bit if necessary...
                        if ((GetTileWidth((unsigned short)i) % 8) != 0)
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(8 - (GetTileWidth((unsigned short)i) % 8));
                        else
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(0);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    else
                        {
                        // Instead of giving the width of the line, we give the new width
                        // previously calculated.
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecHMRRLE1(GetWidth((unsigned short)i), GetHeight((unsigned short)i));
                        m_IntergraphResDescriptors[i]->pCodec->SetSubset(GetWidth((unsigned short)i),1);

                        // We set the padding bit if necessary...
                        if ((GetWidth((unsigned short)i) % 8) != 0)
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(8 - (GetWidth((unsigned short)i) % 8));
                        else
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(0);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    }
                m_BitPerPixel = 1;
                Status = true;
                break;

            case CRL   :
                // We have different pixel type according the palette type
                // defined in the header block 2.
                if (m_IntergraphHeader.IBlock2.ctv > 0)
                    {
                    m_pPixelType = new HRPPixelTypeI8R8G8B8();
                    pPalette = (HRPPixelPalette*)&(m_pPixelType->LockPalette());
                    Set8BitsPalette(m_IntergraphHeader.IBlock2.ctv, pPalette);
                    m_pPixelType->UnlockPalette();
                    }
                else
                    m_pPixelType = new HRPPixelTypeV8Gray8();

                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        {
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecCRL8(GetTileWidth((unsigned short)i), GetTileHeight((unsigned short)i));
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    else
                        {
                        // Instead of giving the width of the line, we give the new width
                        // previously calculated.
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecCRL8(GetWidth((unsigned short)i), GetHeight((unsigned short)i));
                        m_IntergraphResDescriptors[i]->pCodec->SetSubset(GetWidth((unsigned short)i),1);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    }

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

                m_BitPerPixel = 8;
                Status = true;
                break;

            case CCITT :                                              // CIT (ligned only) or TG4 (Tiled version...)
                // Set a pixelType...
                m_pPixelType = new HRPPixelTypeV1Gray1();
                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        {
                        uint32_t IntergraphRasterFileWidth = GetTileWidth((unsigned short)i);
                        if ((IntergraphRasterFileWidth % 32) != 0)
                            IntergraphRasterFileWidth += 32 - (IntergraphRasterFileWidth % 32);

                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecCCITTFax4(IntergraphRasterFileWidth, GetTileHeight((unsigned short)i));

                        ((HFCPtr<HCDCodecCCITTFax4> &)m_IntergraphResDescriptors[i]->pCodec)->SetBitRevTable(true);
                        }
                    else
                        {
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecCCITTFax4(GetWidth((unsigned short)i), GetHeight((unsigned short)i));
                        m_IntergraphResDescriptors[i]->pCodec->SetSubset(GetWidth((unsigned short)i),1);

                        // We set the padding bit if necessary...
                        if ((GetWidth((unsigned short)i) % 8) != 0)
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(8 - (GetWidth((unsigned short)i) % 8));
                        else
                            m_IntergraphResDescriptors[i]->pCodec->SetLinePaddingBits(0);

                        ((HFCPtr<HCDCodecCCITTFax4> &)m_IntergraphResDescriptors[i]->pCodec)->SetBitRevTable(true);
                        }
                    }
                m_BitPerPixel = 1;
                Status = true;
                break;

            case COMPRESS_8BITS :                                              // COT Compress 8 Bits
                // We have different pixel type according the palette type
                // defined in the header block 2.
                if (m_IntergraphHeader.IBlock2.ctv > 0)
                    {
                    m_pPixelType = new HRPPixelTypeI8R8G8B8();
                    pPalette = (HRPPixelPalette*)&(m_pPixelType->LockPalette());
                    Set8BitsPalette(m_IntergraphHeader.IBlock2.ctv, pPalette);
                    m_pPixelType->UnlockPalette();
                    }
                else
                    m_pPixelType = new HRPPixelTypeV8Gray8();
                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        {
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecRLE8(GetTileWidth((unsigned short)i), GetTileHeight((unsigned short)i), 8);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecRLE8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    else
                        {
                        // Instead of giving the width of the line, we give the new width
                        // previously calculated.
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecRLE8(GetWidth((unsigned short)i), GetHeight((unsigned short)i), 8);
                        m_IntergraphResDescriptors[i]->pCodec->SetSubset(GetWidth((unsigned short)i),1);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecRLE8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    }

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

                m_BitPerPixel = 8;
                Status = true;
                break;
            case  RBG_UNCOMPRESS :                                             // RBG uncompress type 28

                m_pPixelType = new HRPPixelTypeV24R8G8B8();

                // Dont build a Codec because we dont have any compression in this file format.
                for (i = 0; i <= m_SubResolution; i++)
                    m_IntergraphResDescriptors[i]->pCodec = 0;

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

                m_BitPerPixel = 24;
                Status = true;
                break;
            case  RBG_COMPRESS :                                             // RBG compress RLE8 type 27

                m_pPixelType = new HRPPixelTypeV24R8G8B8();

                // Build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        {
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecRLE8(GetTileWidth((unsigned short)i), GetTileHeight((unsigned short)i), 24);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecRLE8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    else
                        {
                        // Instead of giving the width of the line, we give the new width
                        // previously calculated.
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecRLE8(GetWidth((unsigned short)i), GetHeight((unsigned short)i), 24);
                        m_IntergraphResDescriptors[i]->pCodec->SetSubset(GetWidth((unsigned short)i),1);
                        // Check if we have a scanline header before the data. If it's the case
                        // inform the codec of this particularity.
                        if (m_IntergraphHeader.IBlock1.scn == 1)
                            ((HFCPtr<HCDCodecRLE8> &)m_IntergraphResDescriptors[i]->pCodec)->SetLineHeader(true);
                        }
                    }

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

                m_BitPerPixel = 24;
                Status = true;
                break;

            case  JPEG_GRAYSCALE :                            // Type 30 : 8 Bits JPeg gray scale

                m_pPixelType = new HRPPixelTypeV8Gray8();

                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecIJG(GetTileWidth ((unsigned short)i),
                                                                                GetTileHeight((unsigned short)i),
                                                                                8);
                    else
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecIJG(GetWidth ((unsigned short)i),
                                                                                GetHeight((unsigned short)i),
                                                                                8);

                    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResDescriptors[i]->pCodec)->SetBitsPerPixel(8);
                    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResDescriptors[i]->pCodec)->SetColorMode(HCDCodecIJG::GRAYSCALE);
                    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResDescriptors[i]->pCodec)->SetAbbreviateMode(true);
                    }

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
                m_BitPerPixel = 8;
                Status = true;
                break;

            case  JPEG_RBG       :                        // Type 31 : 24 Bits JPeg true color

                m_pPixelType = new HRPPixelTypeV24R8G8B8();

                // build a Codec
                for (i = 0; i <= m_SubResolution; i++)
                    {
                    if (HasTileAccess((unsigned short)i))
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecIJG(GetTileWidth ((unsigned short)i),
                                                                                GetTileHeight((unsigned short)i),
                                                                                24);
                    else
                        m_IntergraphResDescriptors[i]->pCodec = new HCDCodecIJG(GetWidth ((unsigned short)i),
                                                                                GetHeight((unsigned short)i),
                                                                                24);

                    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResDescriptors[i]->pCodec)->SetBitsPerPixel(24);
                    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResDescriptors[i]->pCodec)->SetColorMode(HCDCodecIJG::RGB);
                    }

                // !!!! Because Iras/B and Iras/C does not work identically, we can't take
                // !!!! the same care of the SLO at this level, supposed always an UPPER_LEFT_HORIZONTAL
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
                m_BitPerPixel = 24;
                Status = true;
                break;

            default :
                // This file type is not support yet...
                throw(new HRFPixelTypeCodecNotSupportedException(GetURL()->GetURL())); 
                break;
            }
        }
    // Be sure to have pass across this method correctly.
    HPOSTCONDITION(m_BitPerPixel != 0);

    if (m_LUTColorCorrected)
        {
        // The color correction given by an LUT should be applied by the HRFIntergraph???Editor
        // only in 24 bits or in grayscale. IRasC don't support LUT color correction for indexed
        // color.

        if (!m_pPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID) &&
            !m_pPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
            {
            m_LUTColorCorrected = false;
            }

        }
    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// HasTileAccess
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::HasTileAccess(unsigned short pi_SubImage) const
    {
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    bool IsAcess = false;

    if (pi_SubImage == 0)
        IsAcess = m_HasTileAccess;
    else
        {
        if (m_IntergraphResDescriptors[pi_SubImage]->pOverview->Flag == 1)
            IsAcess = true;
        }

    return IsAcess;
    }

//-----------------------------------------------------------------------------
// Public
// HasLineAccess
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::HasLineAccess(unsigned short pi_SubImage) const
    {
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    bool IsAcess = false;

    if (pi_SubImage == 0)
        IsAcess = m_HasLineAcess;
    else
        {
        if (m_IntergraphResDescriptors[pi_SubImage]->pOverview->Flag == 0)
            IsAcess = true;
        }
    return IsAcess;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pIntergraphFile);
    }

//-----------------------------------------------------------------------------
// Publics
// Read the header and fill the structure. On error return false.
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::FillFileHeader()
    {
    HPRECONDITION(m_IsOpen);

    m_HasHeaderFilled = false;

    // Lock the sister file.
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Be sure the stream pointer are at the begining of the file
    m_pIntergraphFile->SeekToPos(GetpageOffset(m_CurentPageIndex));

    // Read the Header, and check if we have read it at all.
    m_pIntergraphFile->Read(&m_IntergraphHeader.IBlock1, sizeof(IntergraphHeaderBlock1));

    m_BlockNumInHeader = (m_IntergraphHeader.IBlock1.wtf + 2) / 256;

    // The number of block MUST be an integer.
    if (((m_IntergraphHeader.IBlock1.wtf + 2) % 256) == 0)
        {
        // We always supposed to have at least two block
        // but there is always an exception..
        if (m_BlockNumInHeader > 1)
            m_pIntergraphFile->Read(&m_IntergraphHeader.IBlock2, sizeof(IntergraphHeaderBlock2));
        else
            CreateHeaderBlock2(m_IntergraphHeader);  // Set the Block2 as empty in a valid state.

        if (m_BlockNumInHeader - 2 > 0)
            {
            m_BlockHeaderAddition = m_BlockNumInHeader - 2;

            // If other block present read all of them...
            // Firstly allocate right memory space to hold the data
            for (int32_t BlockIndex = 0; BlockIndex < m_BlockNumInHeader - 2; BlockIndex++)
                {
                IntergraphHeaderBlockP pIntergraphHeaderBlock = new IntergraphHeaderBlockN;
                m_pIntergraphFile->Read(pIntergraphHeaderBlock, sizeof(IntergraphHeaderBlockN));
                m_pIntergraphBlockSupp.push_back(pIntergraphHeaderBlock);
                }
            }
        if (m_IntergraphHeader.IBlock1.dtc == HRF_INTERGRAPH_TILE_CODE)
            {
            m_HasTileAccess = true;
            m_HasLineAcess  = false;
            }
        else
            {
            m_HasTileAccess = false;
            m_HasLineAcess  = true;
            }

        IntergraphResolutionDescriptor* pNewIntergraphResDescriptor = new IntergraphResolutionDescriptor;

        pNewIntergraphResDescriptor->pOverview           = 0;
        pNewIntergraphResDescriptor->pOverviewEntry      = 0;
        pNewIntergraphResDescriptor->pCodec              = 0;
        pNewIntergraphResDescriptor->pTileDirectoryEntry = 0;

        // We need at least one Intergraph descriptor...
        m_IntergraphResDescriptors.push_back(pNewIntergraphResDescriptor);

        // Read Application Packet if we have it.
        if (m_IntergraphHeader.IBlock2.app != 0)
            ReadAllApplicationPacket();

        // Set the value to gain method HasTileAccess valid access
        m_HasHeaderFilled = true;

        for (uint32_t ImageRes=0; ImageRes <= m_SubResolution; ImageRes++)
            {
            if (HasTileAccess((unsigned short)ImageRes))
                ReadTileDirectory((unsigned short)ImageRes);
            }
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

void HRFIntergraphFile::ReadTileDirectory(unsigned short pi_SubImage)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    uint32_t HorzTileNumber;
    uint32_t VertTileNumber;
    uint32_t TotalTileNumber;
    uint32_t FileOffset;

    FileOffset = GetpageOffset(m_CurentPageIndex) + (m_BlockNumInHeader * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
    if (pi_SubImage != 0)
        FileOffset += m_IntergraphResDescriptors[pi_SubImage]->pOverviewEntry->S;

    // Lock the sister file
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Be sure to be at the right place in the raster
    m_pIntergraphFile->SeekToPos(FileOffset);

    // Read the Tile directory
    m_pIntergraphFile->Read(&m_IntergraphResDescriptors[pi_SubImage]->TileDirectory, sizeof(TileDirectoryInfo));

    // Calculate the number of tile required for the raster.
    HorzTileNumber  = (uint32_t)ceil((double)(GetWidth (pi_SubImage)) / (double)(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize));
    VertTileNumber  = (uint32_t)ceil((double)(GetHeight(pi_SubImage)) / (double)(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize));
    TotalTileNumber = HorzTileNumber * VertTileNumber;

    HASSERT(TotalTileNumber > 0);

    // Allocate memory for each tile entry
    if (m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry == 0)
        m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry = new TileEntry[TotalTileNumber];

    // Read all tile entry.
    m_pIntergraphFile->Read(m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry, sizeof(TileEntry) * TotalTileNumber);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// public
// CountSubResolution
//-----------------------------------------------------------------------------
unsigned short HRFIntergraphFile::CountSubResolution() const
    {
    HPRECONDITION(m_HasHeaderFilled);

    // NOTE : Some Intergraph file have very bad sub-resolution quality.
    //        So that we do is: we support multi resolution file (we can read file with
    //        multi-res), but do not take any account of the sub-resolution data.
    //        Which mean that we re-calculate the sub-resolution and store it in a cache file
    return (unsigned short)m_SubResolution;
    // return 0;
    }

//-----------------------------------------------------------------------------
// public
// GetResolution
//-----------------------------------------------------------------------------
double HRFIntergraphFile::GetResolution(unsigned short pi_SubImage)
    {
    // We assume that a Intergraph file as only one image
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    double resolutionImage = 1.0;

    if (pi_SubImage != 0)
        resolutionImage = HRFResolutionDescriptor::RoundResolutionRatio(m_IntergraphHeader.IBlock1.ppl,
                                                                        m_IntergraphResDescriptors[pi_SubImage]->pOverview->NumberPixels);

    return resolutionImage;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HRFIntergraphFile::GetPixelType() const
    {
    HPRECONDITION(m_HasHeaderFilled);

    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void HRFIntergraphFile::SetPalette (const HRPPixelPalette& pi_rPalette)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION(SharingControlIsLocked());

    // Lock the palette and unlock it after the replace value
    HRPPixelPalette& rPalette = GetPixelType()->LockPalette();
    rPalette = pi_rPalette;
    GetPixelType()->UnlockPalette();

    uint32_t PixelRawDataBits = pi_rPalette.GetChannelOrg().CountPixelCompositeValueBits();
    HASSERT(PixelRawDataBits == 24);

    if (PixelRawDataBits == 24)     // The palette will always have a 24 bit rep. (R,G,B);
        {
        int32_t oldCursorPosition;

        m_IntergraphHeader.IBlock2.cte = pi_rPalette.CountUsedEntries();
        GenerateHeader8BitsPalette(IGDS, pi_rPalette);

        // Backup the last cursor position...
        oldCursorPosition = (int32_t)m_pIntergraphFile->GetCurrentPos();
        if (oldCursorPosition >= 0)
            {
            // Be sure to be at the beginning of the file.
            m_pIntergraphFile->SeekToPos(GetpageOffset(m_CurentPageIndex));

            // Afterall flush theses HeaderBlocks on the disk...
            m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock1,
                                     sizeof(Byte) * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
            m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock2,
                                     sizeof(Byte) * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

            for (int32_t BlockInHeader = 2; BlockInHeader < m_BlockNumInHeader; BlockInHeader++)
                m_pIntergraphFile->Write(m_pIntergraphBlockSupp[BlockInHeader - 2], sizeof(Byte) * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

            // Reset the cursor to it's old position.
            m_pIntergraphFile->SeekToPos(oldCursorPosition);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void HRFIntergraphFile::GenerateHeader8BitsPalette(IntergraphColorTable    ColorTableValue,
                                                   const HRPPixelPalette&  po_pPalette)
    {
    // Must be a valid color table value
    HPRECONDITION((ColorTableValue >= 0) && (ColorTableValue < 3));
    // Be sure to have max 256 Colors
    HPRECONDITION(m_IntergraphHeader.IBlock2.cte * 3 <= 768);

    Byte* pTempPalette;
    HRPChannelOrg ChannelOrg(po_pPalette.GetChannelOrg());
    size_t BytePerEntry = ChannelOrg.CountPixelCompositeValueBits() / 8;

    pTempPalette = new Byte[m_IntergraphHeader.IBlock2.cte * BytePerEntry];
    memset(pTempPalette, 0, m_IntergraphHeader.IBlock2.cte * BytePerEntry);
    m_IntergraphHeader.IBlock2.ctv = (unsigned short)ColorTableValue;
    if (ColorTableValue == IGDS)                       // We've got an IGDS color table
        {
        // If we create a palette but the palette given by parameter does not contain any entry...
        if (po_pPalette.CountUsedEntries() == 0)
            {
            memset(m_IntergraphHeader.IBlock2.use, 0, 256);
            if ((m_IntergraphHeader.IBlock2.cte * BytePerEntry) > 256)
                {
                HASSERT ( m_pIntergraphBlockSupp[0] != 0);

                if (m_BlockNumInHeader < 3)
                    m_BlockNumInHeader++;
                if (m_pIntergraphBlockSupp[0] != 0)
                    memset(m_pIntergraphBlockSupp[0], 0, HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
                }
            }
        else
            {
            for (uint32_t i=0; i< m_IntergraphHeader.IBlock2.cte; i++)
                memcpy(&pTempPalette[i * BytePerEntry], po_pPalette.GetCompositeValue(i), BytePerEntry);

            if ((m_IntergraphHeader.IBlock2.cte * BytePerEntry) < 256)
                memcpy(m_IntergraphHeader.IBlock2.use, pTempPalette, m_IntergraphHeader.IBlock2.cte * BytePerEntry);
            else
                {
                HASSERT ( m_pIntergraphBlockSupp[0] != 0);

                memcpy(m_IntergraphHeader.IBlock2.use, pTempPalette, 256);
                if (m_BlockNumInHeader < 3)
                    m_BlockNumInHeader++;
                if (m_pIntergraphBlockSupp[0] != 0)
                    memcpy(m_pIntergraphBlockSupp[0], pTempPalette + 256, (m_IntergraphHeader.IBlock2.cte * BytePerEntry) - 256);
                }
            }
        }
    else if (ColorTableValue == ENVIRON_V)             // We've got an EnvironV color table
        {
        // We dont support the creation of an EnvironV Palette at this time.
        HASSERT(false);
        }
    delete pTempPalette;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

HRFIntergraphFile::IntergraphColorTable HRFIntergraphFile::AnalysePalette(const HRPPixelPalette& po_pPalette)
    {
    HPRECONDITION(po_pPalette.CountUsedEntries() <= 256);

    HRFIntergraphFile::IntergraphColorTable ReturnValue = NONE;
    Byte* CompositeValue;
    uint32_t ColorTableEntry = po_pPalette.CountUsedEntries();

    m_IntergraphHeader.IBlock2.cte = ColorTableEntry;
    for (uint32_t i=0; i < ColorTableEntry; i++)
        {
        CompositeValue = (Byte*)(po_pPalette.GetCompositeValue(i));

        if (!((CompositeValue[0] == i) && (CompositeValue[1] == i) && (CompositeValue[2]== i)))
            ReturnValue = IGDS;
        if (ReturnValue)
            {
            //if (ColorTableEntry < 32)                          // If we have less than 32 color
            //    ReturnValue = ENVIRON_V;
            // To be here we must have NOT a standard grey scale palette.
            i = po_pPalette.CountUsedEntries();                // Break the analyse loop...
            }
        /*
        Note : If we have less than 96 diff. colors we could suggest to use
               an EnvironV palette type to save space in the file header...

               EnvironV entries = 8 Bytes
               IGDS Space / EnvironV Space = 96 Colors for the same use space.

        Idea : If the real goal of an EnvironV was really to save a block in the header
               we must create an EnvironV only if we have less than 32 Color.In this ways,
               we will only use the second half of the second block.

        N.B. The writing of a EnvironV is not yet implement in GenerateHeader8BitsPalette(...
        */
        }
    return ReturnValue;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation in WRITE MODE
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);

    // Validation with the capabilities if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);

    bool Status = false;

    IntergraphResolutionDescriptor* NewIntergraphResolutionDescriptor;
    for (uint32_t Index = 0; Index < pi_pPage->CountResolutions(); Index++)
        {
        // We need an Intergraph descriptor for each raster resolution
        NewIntergraphResolutionDescriptor = new IntergraphResolutionDescriptor;
        NewIntergraphResolutionDescriptor->pOverview           = 0;
        NewIntergraphResolutionDescriptor->pOverviewEntry      = 0;
        NewIntergraphResolutionDescriptor->pCodec              = 0;
        NewIntergraphResolutionDescriptor->pTileDirectoryEntry = 0;
        m_IntergraphResDescriptors.push_back(NewIntergraphResolutionDescriptor);
        }

    // Add the page descriptor to the list
    if (HRFRasterFile::AddPage(pi_pPage))
        {
        // Add the main resolution for the specified page
        // For main resolution
        Status = InitOpenedFile(pi_pPage);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ResizePage
// File manipulation in WRITE MODE
//-----------------------------------------------------------------------------
bool HRFIntergraphFile::ResizePage(uint32_t pi_Page,
                                    uint64_t pi_NewWidth,
                                    uint64_t pi_NewHeight)
    {
    bool Result = T_Super::ResizePage(pi_Page,
                                       pi_NewWidth,
                                       pi_NewHeight);

    if (Result)
        {
        m_pIntergraphFile->SeekToBegin();
        m_IntergraphResDescriptors.clear();
        m_pIntergraphBlockSupp.clear();
        m_ListOfFreeBlock.clear();
        m_BlockNumInHeader = 0;
        m_BlockHeaderAddition = 0;
        m_OverviewCountChanged = false;

        IntergraphResolutionDescriptor* NewIntergraphResolutionDescriptor;
        HFCPtr<HRFPageDescriptor> pPageDesc(GetPageDescriptor(pi_Page));
        for (uint32_t Index = 0; Index < pPageDesc->CountResolutions(); Index++)
            {
            // We need an Intergraph descriptor for each raster resolution
            NewIntergraphResolutionDescriptor = new IntergraphResolutionDescriptor;
            NewIntergraphResolutionDescriptor->pOverview           = 0;
            NewIntergraphResolutionDescriptor->pOverviewEntry      = 0;
            NewIntergraphResolutionDescriptor->pCodec              = 0;
            NewIntergraphResolutionDescriptor->pTileDirectoryEntry = 0;
            m_IntergraphResDescriptors.push_back(NewIntergraphResolutionDescriptor);
            }

        // Add the main resolution for the specified page
        // For main resolution
        Result = InitOpenedFile(pPageDesc);
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::CreateFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_IsOpen);

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);
    HFCPtr<HRPPixelType>            pPixelType            = pResolutionDescriptor->GetPixelType();
    const HRPPixelPalette&          pPalette              = pPixelType->GetPalette();
    uint32_t                         PixelRawDataBits      = pPixelType->CountPixelRawDataBits();

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;

    m_HasHeaderFilled  = true;
    m_BlockNumInHeader = 2;                                    // Set the member to it's minimal value

    CreateHeaderBlock1(pResolutionDescriptor);   // Create a standard header block 1
    CreateHeaderBlock2(m_IntergraphHeader);      // Create a standard header block 2

    if (pi_pPage->HasTransfoModel())
        SetGlobalTransfoModel(pi_pPage->GetTransfoModel());

    for (TagIterator  = pi_pPage->GetTags().begin(); 
         TagIterator != pi_pPage->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);
            
        unsigned short Unit = 2; // Unkown unit
        double XResolution = 0;
        double YResolution = 0;

        // X Resolution Tag
        if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
            {
            XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
            }
        // Y Resolution Tag
        else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
            {
            YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
            }
        // Resolution Unit Tag
        else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
            {
            Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();
            }

        // Set the resolution (in meter)
        if(XResolution !=0 || YResolution !=0)
            {
            // Unit is centimeter
            if (Unit == 3)
                {
                // Write in micron.
                m_IntergraphHeader.IBlock1.drs = (short) (1.0 / XResolution / 0.0001);
                }
            // Unit is inch
            else if (Unit == 2)
                {
                // Write dpi.
                m_IntergraphHeader.IBlock1.drs = (short)(-XResolution);
                }
            else
                {
                // Write dpi.
                m_IntergraphHeader.IBlock1.drs = (short) 0;
                }
            }
        }

    if (PixelRawDataBits == 8 && pPixelType->CountIndexBits() == 8)
        m_BlockNumInHeader++;

    // The m_SubResolution must be set before creation of the TileDirectoryInfo
    // We must know at this time how many m_SubResolution we want...
    m_BlockHeaderAddition = 0;
    if (pi_pPage->CountResolutions() > 1)
        CreatePacketOverview(pi_pPage);

    // Used for debugging purpose...
    HASSERT(m_pIntergraphBlockSupp.size() == 0);

    // Create, Allocated and add Supplementary Header Block to the list container
    // At this level we must know and be sure how many block will be need.
    if (m_BlockNumInHeader > 2)
        {
        for (int32_t BlockIndex = 0; BlockIndex < m_BlockNumInHeader - 2; BlockIndex++)
            {
            m_pIntergraphBlockSupp.push_back(new IntergraphHeaderBlockN);
            memset(m_pIntergraphBlockSupp[BlockIndex], 0, sizeof(IntergraphHeaderBlockN));
            }
        }

    if (PixelRawDataBits == 8 && pPixelType->CountIndexBits() == 8)
        {
        m_IntergraphHeader.IBlock2.cte = 256;
        GenerateHeader8BitsPalette(IGDS, pPalette);
        //GenerateHeader8BitsPalette(AnalysePalette(pPalette), pPalette);
        }

    // Lock the sister file.
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Write freshly created header physically into the file...
    m_HasHeaderFilled = WriteFileHeader(pi_pPage);
    if (m_HasHeaderFilled)
        {
        if (m_IntergraphHeader.IBlock1.dtc == HRF_INTERGRAPH_TILE_CODE)
            {
            m_HasTileAccess = true;
            m_HasLineAcess  = false;

            // If we create a tiled raster, create at least the first tile directory.
            WriteTileDirectory(0);

            // Init Sub-resolution info, fill the vector m_IntergraphResDescriptors[]
            // THe res 0 is done by WriteTileDirectory(0);
            for (unsigned short i=1; i<pi_pPage->CountResolutions(); ++i)
                {
                HFCPtr<HRFResolutionDescriptor> pResDescriptor = pi_pPage->GetResolutionDescriptor(i);
                CreateTileDirectory(i, pResDescriptor);
                }
            }
        else
            {
            m_HasTileAccess = false;
            m_HasLineAcess  = true;
            }
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::WriteFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION (SharingControlIsLocked());

    // Copy the packet overview if present into the right m_pIntergraphBlockSupp...
    uint32_t NumberOfSubImage = pi_pPage->CountResolutions() - 1;

    if (NumberOfSubImage || m_OverviewCountChanged)
        {
        HPRECONDITION ((m_IntergraphHeader.IBlock2.app / HRF_INTERGRAGH_HEADER_BLOCK_LENGTH) >= HRF_INTERGRAPH_STANDARD_HEADER_BLOCK_QTY);

        m_OverviewCountChanged = false;

        // If we have a NumberOfSubImage, m_BlockHeaderAddition cannot be == 0.
        // Compute it.
        if ( m_BlockHeaderAddition > m_BlockNumInHeader || m_BlockHeaderAddition <= 0)
            {
            // Calculate needed supp. block for holding packet overview.
            m_BlockHeaderAddition = (int32_t)ceil((double) (m_IntergraphHeader.IBlock2.apl * 2) /
                                                (double) (HRF_INTERGRAGH_HEADER_BLOCK_LENGTH));

            m_BlockHeaderAddition = MAX( m_BlockHeaderAddition, 0);
            }

        // TR #242927/TR #278636:
        // The palette was overwritten because the index of the  block where to write was not correctly computed. We
        // now compute it by dividing the file position of the application packet by the block length which is always
        // the same for all blocks also considering that the file starts by the sequence of blocks. To this quantity, we
        // subtract the standard block quantity.
        uint32_t BlockSuppIndex  =    (m_IntergraphHeader.IBlock2.app / HRF_INTERGRAGH_HEADER_BLOCK_LENGTH) -
                                    HRF_INTERGRAPH_STANDARD_HEADER_BLOCK_QTY;
        uint32_t BlockSuppOffset =    0;


        HASSERT(m_BlockHeaderAddition >= 0 && m_BlockHeaderAddition <= 40);
        HASSERT(m_pIntergraphBlockSupp[BlockSuppIndex] != 0);
        HASSERT(m_pApplicationPacket != 0);


        // Copy the application packet into its supplementary block
        memcpy(m_pIntergraphBlockSupp[BlockSuppIndex],
               m_pApplicationPacket,
               sizeof(ApplicationPacket));
        BlockSuppOffset += sizeof(ApplicationPacket);


        // Copy all subresolutions overviews in the same block
        uint32_t SubImageIndex;
        for (SubImageIndex = 0; SubImageIndex < NumberOfSubImage; SubImageIndex++)
            {
            memcpy(m_pIntergraphBlockSupp[BlockSuppIndex] + BlockSuppOffset,
                   m_IntergraphResDescriptors[SubImageIndex + 1]->pOverview,
                   sizeof(Overview));

            BlockSuppOffset += sizeof(Overview);
            }

        // Copy all subresolutions entries in the same block
        for (SubImageIndex = 0; SubImageIndex < NumberOfSubImage; SubImageIndex++)
            {
            // HChckSebG debugging code..
            //IntergraphResolutionDescriptor* TempResolutionDescriptor = m_IntergraphResDescriptors[SubImageIndex + 1];

            memcpy(m_pIntergraphBlockSupp[BlockSuppIndex] + BlockSuppOffset,
                   m_IntergraphResDescriptors[SubImageIndex + 1]->pOverviewEntry,
                   sizeof(TileEntry));

            BlockSuppOffset += sizeof(TileEntry);
            }

        // Validate that we didn't just write more than our supplementary block can contain.
        HASSERT(BlockSuppOffset <= HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
        }

    // Be sure to be at the beginning of the file.
    m_pIntergraphFile->SeekToPos(GetpageOffset(m_CurentPageIndex));

    // Update the word to follow according newly added BlockNumInHeader.
    m_IntergraphHeader.IBlock1.wtf = (unsigned short)((m_BlockNumInHeader * (HRF_INTERGRAGH_HEADER_BLOCK_LENGTH / 2)) - 2);

    // Afterall flush theses HeaderBlocks on the disk...
    m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock1, sizeof(IntergraphHeaderBlock1));
    m_pIntergraphFile->Write(&m_IntergraphHeader.IBlock2, sizeof(IntergraphHeaderBlock2));

    // Write all m_pIntergraphBlockSupp into the file.
    for (int32_t BlockInHeader = 2; BlockInHeader < m_BlockNumInHeader; BlockInHeader++)
        {
        m_pIntergraphFile->Write(m_pIntergraphBlockSupp[BlockInHeader - 2], sizeof(IntergraphHeaderBlockN));
        }

    // At this time always return true...
    return true;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void HRFIntergraphFile::CreateHeaderBlock1(HRFResolutionDescriptor*  pi_pResolutionDescriptor)
    {
    m_IntergraphHeader.IBlock1.htc = 0x0908;                   // Header type code (Intergraph ident.)
    m_IntergraphHeader.IBlock1.wtf = 510;                      // Word to follow. Must be calulate later...

    if (pi_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE)
        m_IntergraphHeader.IBlock1.dtc = HRF_INTERGRAPH_TILE_CODE; // Data type code
    else
        {
        HASSERT(-1 != m_DataTypeCode);
        m_IntergraphHeader.IBlock1.dtc = m_DataTypeCode;
        }
    m_IntergraphHeader.IBlock1.utc = 0;                        // Application Type

    // At this time, make an identity matrix...
    for (uint32_t i=0; i<16; i++)
        m_IntergraphHeader.IBlock1.trn[i] = 0.0;               // Transformation matrix

    m_IntergraphHeader.IBlock1.trn[0]  = 1;                    // Transformation matrix
    m_IntergraphHeader.IBlock1.trn[5]  = 1;                    // Transformation matrix
    m_IntergraphHeader.IBlock1.trn[10] = 1;                    // Transformation matrix
    m_IntergraphHeader.IBlock1.trn[15] = 1;                    // Transformation matrix

    m_IntergraphHeader.IBlock1.xori = 0;                        // X Origin (X Translation)
    m_IntergraphHeader.IBlock1.yor = 0;                        // Y Origin (Y Translation)
    m_IntergraphHeader.IBlock1.zor = 0;                        // We dont support 3D at this time
    m_IntergraphHeader.IBlock1.xdl = 1;                        // X Pixel size (X Scalling)
    m_IntergraphHeader.IBlock1.ydl = 1;                        // Y Pixel size (Y Scalling)
    m_IntergraphHeader.IBlock1.zdl = 1;                        // We dont support 3D at this time

    m_IntergraphHeader.IBlock1.ppl = (uint32_t)pi_pResolutionDescriptor->GetWidth();  // Pixel per line
    m_IntergraphHeader.IBlock1.nol = (uint32_t)pi_pResolutionDescriptor->GetHeight(); // Number of line

    m_IntergraphHeader.IBlock1.drs = 0;                        // Device resolution     ?????????
    m_IntergraphHeader.IBlock1.slo = 4;                        // Scan line orientation

    if (GetDatatypeCode() == RLE)
        m_IntergraphHeader.IBlock1.scn = 1;                    // Scannable flag
    else
        m_IntergraphHeader.IBlock1.scn = 0;                    // Scannable flag

    m_IntergraphHeader.IBlock1.rot = 0.0;                      // Rotation Angle (obsolete)
    m_IntergraphHeader.IBlock1.skw = 0.0;                      // Skew Angle (obsolete)
    m_IntergraphHeader.IBlock1.dtm = 0;                        // Data type modifier    ?????????

    memset(m_IntergraphHeader.IBlock1.dgn, 0, 66);             // Design file name (FILE.DGN)
    memset(m_IntergraphHeader.IBlock1.dbs, 0, 66);             // Data base file name
    memset(m_IntergraphHeader.IBlock1.prn, 0, 66);             // Parent grid file name
    memset(m_IntergraphHeader.IBlock1.des, 0, 80);             // File description

    m_IntergraphHeader.IBlock1.min = 0.0;                      // Minimum value
    m_IntergraphHeader.IBlock1.max = 0.0;;                     // Maximum value

    memset(m_IntergraphHeader.IBlock1.rv1, 0, 3);              // reserved
    if (pi_pResolutionDescriptor->GetBitsPerPixel() == 1)
        m_IntergraphHeader.IBlock1.ver = 2;                    // Grid file version
    else
        m_IntergraphHeader.IBlock1.ver = 3;                    // Grid file version
    }

//-----------------------------------------------------------------------------
// Publics
//-----------------------------------------------------------------------------

void HRFIntergraphFile::CreateHeaderBlock2(IntergraphHeaderBlocks& po_rIntergraphHeaderBlocks) const
    {
    po_rIntergraphHeaderBlocks.IBlock2.gan = 0;                        // Gain
    po_rIntergraphHeaderBlocks.IBlock2.oft = 0;                        // Offset threshold
    po_rIntergraphHeaderBlocks.IBlock2.vf1 = 0;                        // View # screen 1
    po_rIntergraphHeaderBlocks.IBlock2.vf2 = 0;                        // View # screen 2
    po_rIntergraphHeaderBlocks.IBlock2.vno = 0;                        // View number
    po_rIntergraphHeaderBlocks.IBlock2.rv2 = 0;                        // Reserved
    po_rIntergraphHeaderBlocks.IBlock2.rv3 = 0;                        // Reserved
    po_rIntergraphHeaderBlocks.IBlock2.asr = 1.0;                      // Aspect ratio
    po_rIntergraphHeaderBlocks.IBlock2.cfp = 0;                        // Concatenate file pointer
    po_rIntergraphHeaderBlocks.IBlock2.ctv = NONE;                     // Color table type
    po_rIntergraphHeaderBlocks.IBlock2.rv8 = 0;                        // reserved
    po_rIntergraphHeaderBlocks.IBlock2.cte = 0;                        // Number of color table entries
    po_rIntergraphHeaderBlocks.IBlock2.app = 0;                        // Application packet pointer
    po_rIntergraphHeaderBlocks.IBlock2.apl = 0;                        // Application packet length
    memset(po_rIntergraphHeaderBlocks.IBlock2.rv9, 0, sizeof(po_rIntergraphHeaderBlocks.IBlock2.rv9[0]) * 110);
    memset(po_rIntergraphHeaderBlocks.IBlock2.use, 0, sizeof(po_rIntergraphHeaderBlocks.IBlock2.use[0]) * 128);
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::CreateTileDirectory(unsigned short             pi_SubImage,
                                             HRFResolutionDescriptor*  pi_pResolutionDescriptor)
    {
    HPRECONDITION(pi_SubImage <= m_SubResolution);

    bool Status = false;

    uint32_t HorzTileNumber;
    uint32_t VertTileNumber;
    uint32_t TotalTileNumber;

    if (m_IntergraphResDescriptors.size() > pi_SubImage)
        {
        IntergraphResolutionDescriptor* pIntergraphResolutionDescriptor = m_IntergraphResDescriptors[pi_SubImage];

        // Fill the Tile Directory...
        pIntergraphResolutionDescriptor->TileDirectory.ApplicationType = 1;        // Must be 1
        pIntergraphResolutionDescriptor->TileDirectory.SubTypeCode     = 7;        // Must be 7
        pIntergraphResolutionDescriptor->TileDirectory.WordsToFollow   = 0;        // Must be multiple of 4
        pIntergraphResolutionDescriptor->TileDirectory.PacketVersion   = 1;        // Must be 1
        pIntergraphResolutionDescriptor->TileDirectory.Identifier      = 1;        // Must be 1
        pIntergraphResolutionDescriptor->TileDirectory.Reserved1[0]    = 0;        // Must be 0
        pIntergraphResolutionDescriptor->TileDirectory.Reserved1[1]    = 0;        // Must be 0
        pIntergraphResolutionDescriptor->TileDirectory.Properties      = 1;        // Tile not in order
        if (m_SubResolution > 0)
            pIntergraphResolutionDescriptor->TileDirectory.Properties  = 3;

        pIntergraphResolutionDescriptor->TileDirectory.DataTypeCode    = GetDatatypeCode();
        memset(pIntergraphResolutionDescriptor->TileDirectory.Reserved2, 0, 100);  // Must be 0
        pIntergraphResolutionDescriptor->TileDirectory.TileSize        = pi_pResolutionDescriptor->GetBlockWidth();

        // Have a tile size greater than 32
        if (pIntergraphResolutionDescriptor->TileDirectory.TileSize < 32)
            pIntergraphResolutionDescriptor->TileDirectory.TileSize = 32;
        else
            {
            // Be sure to have a tile size who is a a factor of 32.
            if ((pIntergraphResolutionDescriptor->TileDirectory.TileSize % 32) != 0)
                pIntergraphResolutionDescriptor->TileDirectory.TileSize -= (pIntergraphResolutionDescriptor->TileDirectory.TileSize % 32);
            }
        pIntergraphResolutionDescriptor->TileDirectory.Reserved3       = 0;                      // Must be 0

        // Calculate the number of tile required for the raster.
        HorzTileNumber  = (uint32_t)ceil((double)(pi_pResolutionDescriptor->GetWidth())  /
                                      (double)(pIntergraphResolutionDescriptor->TileDirectory.TileSize));
        VertTileNumber  = (uint32_t)ceil((double)(pi_pResolutionDescriptor->GetHeight()) /
                                      (double)(pIntergraphResolutionDescriptor->TileDirectory.TileSize));
        TotalTileNumber = HorzTileNumber * VertTileNumber;

        // If we have less than one tile, an error has been occured and we cannot continue...
        HASSERT(TotalTileNumber > 0);

        if (TotalTileNumber > 0)
            {
            // Allocate memory for each tile entry
            if (pIntergraphResolutionDescriptor->pTileDirectoryEntry == 0)
                pIntergraphResolutionDescriptor->pTileDirectoryEntry = new TileEntry[TotalTileNumber];
            for (uint32_t TileIndex = 0; TileIndex < TotalTileNumber; TileIndex++)
                {
                // Create at first blank, uninstanciate tile. The correct information
                // will be filled in the directory tile per tile as WriteTile will be called.
                pIntergraphResolutionDescriptor->pTileDirectoryEntry[TileIndex].S = 0;
                pIntergraphResolutionDescriptor->pTileDirectoryEntry[TileIndex].A = 0;
                pIntergraphResolutionDescriptor->pTileDirectoryEntry[TileIndex].U = 0;
                }
            Status = true;
            pIntergraphResolutionDescriptor->TileDirectory.WordsToFollow = ((sizeof(pIntergraphResolutionDescriptor->TileDirectory) +
                                                                             sizeof(pIntergraphResolutionDescriptor->pTileDirectoryEntry[0]) * TotalTileNumber) / 2) - 4;
            }
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::WriteTileDirectory(unsigned short pi_SubImage)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(pi_SubImage <= m_SubResolution);
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION(SharingControlIsLocked());

    bool  Status = false;
    uint32_t FileOffset;
    uint32_t SizeToWrite;
    uint32_t CalculatedSizeOfDirectory = 0;
    uint32_t HorzTileNumber;
    uint32_t VertTileNumber;
    uint32_t TotalTileNumber;

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);   // Page);
    HFCPtr<HRFResolutionDescriptor>  pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor (pi_SubImage);

    if (CreateTileDirectory(pi_SubImage, pResolutionDescriptor))
        {
        // seek to the right place in the file....
        FileOffset = GetpageOffset(m_CurentPageIndex) + (m_BlockNumInHeader * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
        if (pi_SubImage != 0)
            FileOffset += m_IntergraphResDescriptors[pi_SubImage]->pOverviewEntry->S;

        // Be sure to be at the right place into the raster file
        m_pIntergraphFile->SeekToPos(FileOffset);
            {
            // Calculate the number of tile required for the raster.
            HorzTileNumber  = (uint32_t)ceil((double)(pResolutionDescriptor->GetWidth())  / (double)(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize));
            VertTileNumber  = (uint32_t)ceil((double)(pResolutionDescriptor->GetHeight()) / (double)(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory.TileSize));
            TotalTileNumber = HorzTileNumber * VertTileNumber;

            SizeToWrite = sizeof(m_IntergraphResDescriptors[pi_SubImage]->TileDirectory);
            if (m_pIntergraphFile->Write(&m_IntergraphResDescriptors[pi_SubImage]->TileDirectory, sizeof(Byte) * SizeToWrite) == (sizeof(Byte) * SizeToWrite))
                {
                if (m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry != 0)
                    {
                    SizeToWrite = sizeof(m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry[0]);
                    if ((m_pIntergraphFile->Write(m_IntergraphResDescriptors[pi_SubImage]->pTileDirectoryEntry,
                                                  SizeToWrite * TotalTileNumber)) == (SizeToWrite * TotalTileNumber))
                        {
                        CalculatedSizeOfDirectory += (SizeToWrite * TotalTileNumber);
                        // Calculate the data lenght to write and add padding to quad word boundary.
                        CalculatedSizeOfDirectory = CalculatedSizeOfDirectory % 8;
                        if (CalculatedSizeOfDirectory)
                            {
                            HArrayAutoPtr<Byte> EmptyBuffer(new Byte[CalculatedSizeOfDirectory]);
                            memset(EmptyBuffer, 0, CalculatedSizeOfDirectory);
                            m_pIntergraphFile->Write(EmptyBuffer.get(), sizeof(Byte) * CalculatedSizeOfDirectory);
                            }
                        Status = true;
                        }
                    }
                }
            }
        m_pIntergraphFile->Flush();
        }
    HASSERT(Status);

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::HasCompression()
    {
    bool Status = false;

    if (m_IntergraphResDescriptors[0]->pCodec != 0)
        Status = true;

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::CreatePacketOverview(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_BlockNumInHeader == 2 || m_BlockNumInHeader == 3);

    bool  Status = true;
    uint32_t Index;

    uint32_t NumberOfSubImage = pi_pPage->CountResolutions() - 1;
    uint32_t ApplicationPacketBlockPosition = m_BlockNumInHeader;

    m_pApplicationPacket = new ApplicationPacket;
    m_pApplicationPacket->ApplicationType = 1;
    m_pApplicationPacket->SubTypeCode     = 10;
    // The word to Follow MUST be recalculate at the end of this method.
    m_pApplicationPacket->WordsToFollow   = 0;
    m_pApplicationPacket->PacketVersion   = 1;
    m_pApplicationPacket->Identifier      = 1;
    m_pApplicationPacket->Reserved        = 0;
    m_pApplicationPacket->NumberOverview  = NumberOfSubImage;

    for (Index = 0; Index < NumberOfSubImage; Index++)
        {
        m_IntergraphResDescriptors[Index + 1]->pOverview = new Overview;
        m_IntergraphResDescriptors[Index + 1]->pOverview->NumberLines    = 1;   // Height in Line
        m_IntergraphResDescriptors[Index + 1]->pOverview->NumberPixels   = 1;   // Width in pixel
        m_IntergraphResDescriptors[Index + 1]->pOverview->SamplingMethod = 0;   // 0:Subsample, 1:Logical 'Or', 2:Avaraging
        if (pi_pPage->GetResolutionDescriptor((unsigned short)(Index+1))->GetBlockType() == HRFBlockType::TILE)
            m_IntergraphResDescriptors[Index + 1]->pOverview->Flag = 1;
        else
            m_IntergraphResDescriptors[Index + 1]->pOverview->Flag = 0;
        m_IntergraphResDescriptors[Index + 1]->pOverview->Reserved = 0;         // Always 0
        m_IntergraphResDescriptors[Index + 1]->pOverviewEntry = new TileEntry;
        m_IntergraphResDescriptors[Index + 1]->pOverviewEntry->S = 0;
        m_IntergraphResDescriptors[Index + 1]->pOverviewEntry->A = 0;
        m_IntergraphResDescriptors[Index + 1]->pOverviewEntry->U = 0;
        }

    // Pointer to the First Application Pointer, Normally into the third Header Block.
    // Offset of the first "application packet" in byte...
    m_IntergraphHeader.IBlock2.app = ApplicationPacketBlockPosition * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;

    uint32_t PreCalculateWTF = sizeof(ApplicationPacket)            +
                             sizeof(Overview)  * NumberOfSubImage +
                             sizeof(TileEntry) * NumberOfSubImage;

    // Lentgh of the first application packet.
    m_IntergraphHeader.IBlock2.apl = PreCalculateWTF / 2;

    // Calculate needed supp. block for holding packet overview.
    m_BlockHeaderAddition = (int32_t)ceil((double) (PreCalculateWTF) /
                                        (double) (HRF_INTERGRAGH_HEADER_BLOCK_LENGTH));

    // Add needed m_BlockHeaderAddition to the m_BlockNumInHeader.
    m_BlockNumInHeader += m_BlockHeaderAddition;

    // Size of all application packet in word...
    m_pApplicationPacket->WordsToFollow = PreCalculateWTF / 2;
    if (((PreCalculateWTF / 2) % 4) != 0)
        m_pApplicationPacket->WordsToFollow = (PreCalculateWTF / 2) + ((PreCalculateWTF / 2) % 4);

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::UpdatePacketOverview(uint32_t pi_FileCursorPosition, uint32_t pi_Resolution)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool  Status = false;

    // Always work on the Page 0, at this time we don't support multiple pages raster file.
    HRFPageDescriptor* pPage = GetPageDescriptor(0);

    if (pi_Resolution > 0)
        {
        HASSERT(m_IntergraphResDescriptors[pi_Resolution]->pOverview != 0);
        HASSERT(m_IntergraphResDescriptors[pi_Resolution]->pOverviewEntry != 0);

        m_IntergraphResDescriptors[pi_Resolution]->pOverview->NumberLines  =
            (uint32_t)pPage->GetResolutionDescriptor((unsigned short)pi_Resolution)->GetHeight();
        m_IntergraphResDescriptors[pi_Resolution]->pOverview->NumberPixels =
            (uint32_t)pPage->GetResolutionDescriptor((unsigned short)pi_Resolution)->GetWidth();

        // 0:Subsample, 1:Logical 'Or', 2:Avaraging
        m_IntergraphResDescriptors[pi_Resolution]->pOverview->SamplingMethod = 0;

        if (pPage->GetResolutionDescriptor((unsigned short)pi_Resolution)->GetBlockType() == HRFBlockType::TILE)
            m_IntergraphResDescriptors[pi_Resolution]->pOverview->Flag = 1;
        else
            m_IntergraphResDescriptors[pi_Resolution]->pOverview->Flag = 0;

        m_IntergraphResDescriptors[pi_Resolution]->pOverviewEntry->S = pi_FileCursorPosition;
        }

    Status = WriteFileHeader(pPage);

    return Status;
    }
//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------

const HGF2DWorldIdentificator HRFIntergraphFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_INTERGRAPHWORLD;
    }


//-----------------------------------------------------------------------------
// Protected
// GetFullResolutionSize
//-----------------------------------------------------------------------------
uint32_t HRFIntergraphFile::GetFullResolutionSize()
    {
    uint32_t ResolutionSize=0;
    uint32_t RasterOffset = GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;

    if (m_SubResolution > 0)
        {
        ResolutionSize = m_IntergraphResDescriptors[1]->pOverviewEntry->S - RasterOffset;
        }
    else
        ResolutionSize = (uint32_t)(m_pIntergraphFile->GetSize() - RasterOffset);

    return ResolutionSize;
    }


//-----------------------------------------------------------------------------
// Protected
// UpdateOffsetNextResolution
//-----------------------------------------------------------------------------
void HRFIntergraphFile::UpdateOffsetNextResolutions  (unsigned short pi_CurrentSubImage,
                                                      uint32_t pi_NbByteToAdd)
    {
    if (pi_CurrentSubImage <= m_SubResolution)
        {
        for(unsigned short i=pi_CurrentSubImage+1; i<=m_SubResolution; ++i)
            m_IntergraphResDescriptors[i]->pOverviewEntry->S += pi_NbByteToAdd;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// IsSingleColor
//-----------------------------------------------------------------------------
bool HRFIntergraphFile::IsSingleColor()
    {
    bool   Status       = false;
    uint32_t DataTypeCode = 0;
    uint32_t FileOffset    = 0;

    if (m_IntergraphHeader.IBlock1.dtc == HRF_INTERGRAPH_TILE_CODE)
        {
        IntergraphResolutionDescriptor TempResDescriptors;

        // The data type code is hidden into the tile directory..
        FileOffset = GetpageOffset(m_CurentPageIndex) + (m_BlockNumInHeader * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        // Backup the last cursor position...
        int32_t CurrentCursorPosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();

        // Be sure to be at the right place in the raster
        m_pIntergraphFile->SeekToPos(FileOffset);

        // Read the Tile directory
        m_pIntergraphFile->Read(&TempResDescriptors.TileDirectory, sizeof(TileDirectoryInfo));

        // Reset the cursor to it's old position.
        m_pIntergraphFile->SeekToPos(CurrentCursorPosition);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        DataTypeCode = TempResDescriptors.TileDirectory.DataTypeCode;
        }
    else
        {
        DataTypeCode = m_IntergraphHeader.IBlock1.dtc;
        }
    if (DataTypeCode == CCITT || DataTypeCode == RLE)
        Status = true;

    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::GetpageOffset(uint32_t pi_PageIndex)
    {
    HPRECONDITION(m_pIntergraphFile != 0);
    uint32_t NextPageOffset     = 0;

    // Optimization : do not lose any CPU time if the page zero is requested.
    if (pi_PageIndex > 0)
        {
        // Backup the current file cursor pos.
        uint32_t CursorFilePosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();

        uint32_t PageCount          = 0;
        int32_t BlockNumInHeader;

        IntergraphHeaderBlocks TempHeaderBlocks;

        do
            {
            m_pIntergraphFile->SeekToPos(NextPageOffset);

            // Read the Header, and check if we have read it at all.
            m_pIntergraphFile->Read(&TempHeaderBlocks.IBlock1, sizeof(IntergraphHeaderBlock1));
            BlockNumInHeader = (TempHeaderBlocks.IBlock1.wtf + 2) / 256;

            if (((TempHeaderBlocks.IBlock1.wtf + 2) % 256) == 0)
                {
                // We always supposed to have at least two block
                // but there is always an exception..
                if (BlockNumInHeader > 1)
                    m_pIntergraphFile->Read(&TempHeaderBlocks.IBlock2, sizeof(IntergraphHeaderBlock2));
                else
                    CreateHeaderBlock2(TempHeaderBlocks);  // Set the Block2 as empty in a valid state.
                }

            if (PageCount < pi_PageIndex)
                NextPageOffset = TempHeaderBlocks.IBlock2.cfp;
            PageCount++;
            }
        while (PageCount < pi_PageIndex && NextPageOffset != 0 && !m_pIntergraphFile->EndOfFile());

        // Get back where we are before scanning the file.
        m_pIntergraphFile->SeekToPos(CursorFilePosition);
        }
    return NextPageOffset;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::GetLUTColorCorrection(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile     != 0);
    HPRECONDITION(m_pRedLUTColorTable   == 0);
    HPRECONDITION(m_pGreenLUTColorTable == 0);
    HPRECONDITION(m_pBlueLUTColorTable  == 0);
    HPRECONDITION(pi_PacketHeader.WordsToFollow >= 144);

    // Scan the Application packet to find the LUT Packet
    // Be ready for the stored LUT
    m_pIntergraphFile->SeekToPos(pi_FileOffset + sizeof(GenericApplicationPacket) + 24);

    m_LUTColorPacketOffset = pi_FileOffset;     // useful to this offset for the ResetLUT

    // Read the Lookup Table for each channel

    // For grayscale images, there is only one table.
    m_pRedLUTColorTable = new Byte[256];
    m_pIntergraphFile->Read(m_pRedLUTColorTable  , 256);

    // For RGB raster, we need the other stored LUT
    if (pi_PacketHeader.WordsToFollow > 144)
        {
        m_pGreenLUTColorTable = new Byte[256];
        m_pBlueLUTColorTable  = new Byte[256];

        m_pIntergraphFile->Read(m_pGreenLUTColorTable, 256);
        m_pIntergraphFile->Read(m_pBlueLUTColorTable , 256);
        }
    m_LUTColorCorrected = !IsIdentityLUT();

    return true;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::IsIdentityLUT() const
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile  != 0);
    HPRECONDITION(m_pRedLUTColorTable!= 0);

    // Check if the LUT provide some correction.
    bool IsIdentity = true;

    // First, verrify the first LUT
    for (int ColorIndex = 0; ColorIndex < 256 && IsIdentity; ColorIndex++)
        {
        IsIdentity = m_pRedLUTColorTable[ColorIndex] == ColorIndex;
        }

    // If no correction found, be sure the other table do not contain
    // any correction either, if they exist..
    if (IsIdentity && m_pGreenLUTColorTable != 0)
        {
        IsIdentity = memcmp(m_pRedLUTColorTable, m_pGreenLUTColorTable, 256) == 0;
        }

    if (IsIdentity && m_pBlueLUTColorTable != 0)
        {
        IsIdentity = memcmp(m_pRedLUTColorTable, m_pBlueLUTColorTable, 256) == 0;
        }
    return IsIdentity;
    }

//-----------------------------------------------------------------------------
// ResetLUT
// This method must be call just after we open the file, to be sure that the
// offset is still valid.
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::ResetLUT()
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile  != 0);

    if (m_LUTColorCorrected)
        {
        // Reset the LUT
        for (int ColorIndex = 0; ColorIndex < 256; ColorIndex++)
            m_pRedLUTColorTable[ColorIndex] = (Byte)ColorIndex;

        if (GetAccessMode().m_HasWriteAccess)
            {
            m_pIntergraphFile->SeekToPos(m_LUTColorPacketOffset + sizeof(GenericApplicationPacket) + 24);
            m_pIntergraphFile->Write(m_pRedLUTColorTable ,256);
            }

        if (m_pGreenLUTColorTable != 0 && GetAccessMode().m_HasWriteAccess)
            {
            memcpy(m_pGreenLUTColorTable, m_pRedLUTColorTable, 256);
            m_pIntergraphFile->Write(m_pGreenLUTColorTable ,256);

            memcpy(m_pBlueLUTColorTable, m_pRedLUTColorTable, 256);
            m_pIntergraphFile->Write(m_pBlueLUTColorTable ,256);
            }

        m_LUTColorCorrected = false;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Static method
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::GetIntergraphLUTApplyReset()             {return m_sIntergraphLUT_ApplyReset;}
void HRFIntergraphFile::SetIntergraphLUTApplyReset(bool value)   {m_sIntergraphLUT_ApplyReset = value;}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRFIntergraphFile::LUTOverrideAccessLimitationGuard::LUTOverrideAccessLimitationGuard(bool pi_OverrideAccessLimitation)
    {
    m_intergraphLUT_ApplyReset = GetIntergraphLUTApplyReset();
    SetIntergraphLUTApplyReset(pi_OverrideAccessLimitation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRFIntergraphFile::LUTOverrideAccessLimitationGuard::~LUTOverrideAccessLimitationGuard()
    {
    //restore back the original value
    SetIntergraphLUTApplyReset(m_intergraphLUT_ApplyReset);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::GetJpegAppPacket(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile != 0);

    // Scan the Application packet to find the Jpeg Packet
    bool   Status = false;

    // Fill the first packet.
    // m_pIntergraphFile->SeekToPos(pi_FileOffset);
    // m_pApplicationPacket = new ApplicationPacket;
    // m_pIntergraphFile->Read(m_pJpegPacketPacket, sizeof(JpegPacketPacket));

    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::GetPacketOverview(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile    != 0);
    HPRECONDITION(m_pApplicationPacket == 0);

    // Scan the Application packet to find the PacketOverview (sub-resolutions..)
    bool   Status = false;
    //UInt32  Block = m_IntergraphHeader.IBlock2.app / HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;
    Byte* TempBuffer;
    uint32_t NeededMemory;
    uint32_t index;
    uint32_t ImageRes = 0;

    // Fill the first packet.
    m_pIntergraphFile->SeekToPos(pi_FileOffset);

    m_pApplicationPacket = new ApplicationPacket;
    m_pIntergraphFile->Read(m_pApplicationPacket, sizeof(ApplicationPacket));

    // For performance and quality, we would like to ignore the sub-resolution for 1bit images.
    if ((m_pApplicationPacket->NumberOverview > 0) && IsSingleColor())
        {
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            m_OverviewCountChanged = true;

        m_pApplicationPacket->NumberOverview = 0;
        }

    // Get how many sub image we will have.
    m_SubResolution = m_pApplicationPacket->NumberOverview;

    for (ImageRes = 0; ImageRes < m_SubResolution; ImageRes++)
        {
        // Add all needed IntergraphResDescriptors into the STL list.
        IntergraphResolutionDescriptor* pIntergraphResolutionDescriptor = new IntergraphResolutionDescriptor;

        pIntergraphResolutionDescriptor->pOverview            = new Overview;
        pIntergraphResolutionDescriptor->pOverviewEntry       = new TileEntry;
        pIntergraphResolutionDescriptor->pCodec               = 0;
        pIntergraphResolutionDescriptor->pTileDirectoryEntry  = 0;
        m_IntergraphResDescriptors.push_back(pIntergraphResolutionDescriptor);
        }

    // Allocate right memory space for copying the header block information into
    // a single temporary buffer.
    NeededMemory  = m_SubResolution * sizeof(Overview);
    NeededMemory += m_SubResolution * sizeof(TileEntry);
    TempBuffer = new Byte[NeededMemory];
    index = 0;

    // We are not supposed to need more memory than the given application packet size.
    HASSERT(NeededMemory < (pi_PacketHeader.WordsToFollow * 2));
    m_pIntergraphFile->Read(TempBuffer, NeededMemory);

    for (ImageRes = 0; ImageRes < m_SubResolution; ImageRes++)
        {
        // Fill the Overview structure
        memcpy(m_IntergraphResDescriptors[ImageRes + 1]->pOverview, &TempBuffer[index], sizeof(Overview));
        index += sizeof(Overview);
        }

    for (ImageRes=0; ImageRes < m_SubResolution; ImageRes++)
        {
        // File the overview entry.
        memcpy(m_IntergraphResDescriptors[ImageRes + 1]->pOverviewEntry, &TempBuffer[index], sizeof(TileEntry));
        index += sizeof(TileEntry);
        }
    delete []TempBuffer;

    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::GetHuffmanPacket(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile != 0);

    bool   Status = false;

    // Scan the Application packet to find the Huffman Packet
    // Dont know how we should handle this packet, skip it for now...
    HASSERT(false);

    return Status;
    }

//-----------------------------------------------------------------------------
// private HRFIntergraphFile::ReadAllApplicationPacket
//          If there is some application packet, read all of them
//          and process every supported packet.
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::ReadAllApplicationPacket()
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pIntergraphFile != 0);

    bool   Status = false;
    uint32_t MaxOffsetToBeInHeader = (HRF_INTERGRAGH_HEADER_BLOCK_LENGTH * m_BlockNumInHeader) - sizeof(m_pApplicationPacket);

    // We only supoort the application packet contained by the header.
    if (m_IntergraphHeader.IBlock2.app < MaxOffsetToBeInHeader)
        {
        GenericApplicationPacket ApplicationPacketHeader;

        // Backup last cursor position.
        uint32_t OldCursorFilePosition = (uint32_t)m_pIntergraphFile->GetCurrentPos();
        uint32_t CursorFilePosition    = m_IntergraphHeader.IBlock2.app;

        // Process all known application packet.
        do
            {
            // Seek to the first appl. packet.
            m_pIntergraphFile->SeekToPos(CursorFilePosition);

            // Fill the generic part of the application packet.
            m_pIntergraphFile->Read(&ApplicationPacketHeader, sizeof(GenericApplicationPacket));

            // check if the application packet was an overview packet.
            if (ApplicationPacketHeader.ApplicationType == 1 && ApplicationPacketHeader.SubTypeCode == 10)
                {
                Status = GetPacketOverview(CursorFilePosition, ApplicationPacketHeader);
                }

            // check if the application packet was a jpeg packet.
            if (ApplicationPacketHeader.ApplicationType == 2 && ApplicationPacketHeader.SubTypeCode == 12)
                {
                Status = GetJpegAppPacket(CursorFilePosition, ApplicationPacketHeader);

                }
            // check if the application packet was a huff packet.
            if (ApplicationPacketHeader.ApplicationType == 2 && ApplicationPacketHeader.SubTypeCode == 13)
                {
                Status = GetHuffmanPacket(CursorFilePosition, ApplicationPacketHeader);
                }

            if (ApplicationPacketHeader.ApplicationType == 1 && ApplicationPacketHeader.SubTypeCode == 2)
                {
                if (ImageppLib::GetHost().GetImageppLibAdmin()._IsIntergraphLUTColorCorrectionEnable() || m_sIntergraphLUT_ApplyReset)
                    {
                    GetLUTColorCorrection(CursorFilePosition, ApplicationPacketHeader);
                    // if the LUT is not null, we don't allow to open the file in ReadWrite mode normally.
                    // Except if m_sIntergraphLUT_ApplyReset=true to be able to reset or apply the LUT.
                    if (m_LUTColorCorrected)
                        {
                        if ((GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess) && !m_sIntergraphLUT_ApplyReset)
                            {
                            WString CurrentFileName(GetURL()->GetURL());
                            throw HRFIntergraphLutReadOnlyException(CurrentFileName);
                            }
                        }
                    }
                }

            // Prepare for the next app. packet if any
            if (ApplicationPacketHeader.WordsToFollow)
                CursorFilePosition += (ApplicationPacketHeader.WordsToFollow * 2) + sizeof(GenericApplicationPacket);

            }
        while (ApplicationPacketHeader.WordsToFollow != 0 && CursorFilePosition < MaxOffsetToBeInHeader);

        // Restore the ols cursor position.
        m_pIntergraphFile->SeekToPos(OldCursorFilePosition);
        }
    else
        {
        // This is not recommended (and never seen) but can happen...
        // The Application Packet can be also at the end of the file.

        // At this time it is not a supported feature.
        HASSERT(false);
        }
    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// GetBitPerPixel
// File information
//-----------------------------------------------------------------------------

unsigned short HRFIntergraphFile::GetBitPerPixel() const
    {
    return m_BitPerPixel;
    }

//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------

const HFCBinStream* HRFIntergraphFile::GetIntergraphFilePtr() const
    {
    return m_pIntergraphFile;
    }

//-----------------------------------------------------------------------------
// Public
// GetBlockNumInHeader
// File information
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphFile::GetBlockNumInHeader() const
    {
    return m_BlockNumInHeader;
    }


//-----------------------------------------------------------------------------
// Public
// SetBitPerPixel
// File information
//-----------------------------------------------------------------------------

void HRFIntergraphFile::SetBitPerPixel(unsigned short pi_BitPerPixel)
    {
    m_BitPerPixel = pi_BitPerPixel;
    }

//-----------------------------------------------------------------------------
// Public
// SetBitPerPixel
// File information
//-----------------------------------------------------------------------------

void HRFIntergraphFile::SetDatatypeCode(unsigned short pi_DataTypeCode)
    {
    m_DataTypeCode = pi_DataTypeCode;
    }

//-----------------------------------------------------------------------------
// Public
// SetBitPerPixel
// File information
//-----------------------------------------------------------------------------

const unsigned short HRFIntergraphFile::GetDatatypeCode() const
    {
    HPRECONDITION(-1 != m_DataTypeCode);
    return m_DataTypeCode;
    }


//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------

void HRFIntergraphFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                      uint32_t pi_Page,
                                                      bool   pi_CheckSpecificUnitSpec,
                                                      bool   pi_InterpretUnitINTGR)
    {
    //The unit used by Intergraph file formats is always UoR.
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const HRFScanlineOrientation HRFIntergraphFile::GetScanlineOrientation() const
    {
    HPRECONDITION(m_HasHeaderFilled);

    return m_ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// Public
// SetBitPerPixel
// File information
//-----------------------------------------------------------------------------

bool HRFIntergraphFile::HasLUTColorCorrection() const
    {
    return m_LUTColorCorrected;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const Byte* HRFIntergraphFile::GetRedLUTColorTablePtr  () const
    {
    return m_pRedLUTColorTable;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const Byte* HRFIntergraphFile::GetGreenLUTColorTablePtr() const
    {
    return m_pGreenLUTColorTable;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const Byte* HRFIntergraphFile::GetBlueLUTColorTablePtr () const
    {
    return m_pBlueLUTColorTable;
    }