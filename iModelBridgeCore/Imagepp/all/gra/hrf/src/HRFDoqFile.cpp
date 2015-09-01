//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFDoqFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFDoqFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>




#include <Imagepp/all/h/HRFDoqFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFDoqEditor.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>



//--------------------------------------------------
// Definition
//--------------------------------------------------


enum DoqColorType {DoqUndefinedColor, DoqBlackAndWhite, DoqRGB };





string StringToLower(string strToConvert)
    {   //change each element of the string to lower case
    for(unsigned int i=0; i<strToConvert.length(); i++)
        {
        strToConvert[i] = (char)tolower(strToConvert[i]);
        }
    return strToConvert;//return the converted string
    }


//-----------------------------------------------------------------------------
// HRFDoqBlockCapabilities
//-----------------------------------------------------------------------------
class HRFDoqBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFDoqBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add (new HRFLineCapability (HFC_READ_ONLY,
                                    ULONG_MAX,
                                    HRFBlockAccess::RANDOM));

        }
    };

//-----------------------------------------------------------------------------
// HRFDoqCodecCapabilities
//-----------------------------------------------------------------------------
class HRFDoqCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFDoqCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFDoqBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFDoqCapabilities
//-----------------------------------------------------------------------------
HRFDoqCapabilities::HRFDoqCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV8Gray8::CLASS_ID,
                                     new HRFDoqCodecCapabilities()));

    // PixelTypeV24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                     new HRFDoqCodecCapabilities()));

    // PixelTypeV16Gray16
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFDoqCodecCapabilities()));

    // HRPPixelTypeV16R5G6B5
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16R5G6B5::CLASS_ID,
                                   new HRFDoqCodecCapabilities()));



    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::LINE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCopyright));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFDoqCreator)

//-----------------------------------------------------------------------------
// HRFDoqCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFDoqCreator::HRFDoqCreator()
    : HRFRasterFileCreator(HRFDoqFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFDoqCreator::GetLabel() const
    {
    // DOQ File Format
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_USGS_DOQ()); // DOQ File Format
    }

// Identification information
WString HRFDoqCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFDoqCreator::GetExtensions() const
    {
    return WString(L"*.doq");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFDoqCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFDoqFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFDoqCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                       Result = false;
    HAutoPtr<HFCBinStream>      pFile;
    HArrayAutoPtr<char>        pLine(new char[DOQ_HEADER_LINE_LENGTH]);

    (const_cast<HRFDoqCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the Doq File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        pFile->Read(pLine, DOQ_HEADER_LINE_LENGTH);

        if(strncmp(pLine,"BEGIN_USGS_DOQ_HEADER",21)==0)
            {
            Result = true;
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFDoqCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFDoqCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }





//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of Doq File
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFDoqCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFDoqCapabilities();

    return m_pCapabilities;
    }



//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRFDoqFile::~HRFDoqFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Save
// This method saves the file
//-----------------------------------------------------------------------------
void HRFDoqFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pDoqFile->GetCurrentPos();

    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        // TODO : Update the file if necessary
        m_pDoqFile->Flush();
        }

    //Set back position
    m_pDoqFile->SeekToPos(CurrentPos);

    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFDoqFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pDoqFile);
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file
//-----------------------------------------------------------------------------
bool HRFDoqFile::Open()
    {


    if (!m_IsOpen)
        {
        m_pDoqFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        //Put the header in a buffer
        //TODO : GetHeader();

        //Read Header
        ReadHeader();


        CreatePixelType();



        m_IsOpen = true;
        }


    return true;


    }

//-----------------------------------------------------------------------------
// Protected
// GetHeader
// Put the header in a buffer
//-----------------------------------------------------------------------------
void HRFDoqFile::GetHeader()
    {

    uint64_t   PosBufferEnd;

    m_pDoqFile->SeekToBegin();
    m_pDoqFile->Read(m_pLine,DOQ_HEADER_LINE_LENGTH);

    while(strncmp(m_pLine, "     ", 5 )!=0)
        {
        m_pDoqFile->Read(m_pLine,DOQ_HEADER_LINE_LENGTH);
        }

    PosBufferEnd = m_pDoqFile->GetCurrentPos();
    HPOSTCONDITION(PosBufferEnd < SIZE_MAX);

    m_pHeaderBuffer = new char[(size_t)PosBufferEnd];
    m_pDoqFile->SeekToBegin();
    m_pDoqFile->Read(m_pHeaderBuffer, (size_t)PosBufferEnd);

    }

//-----------------------------------------------------------------------------
// Protected
// ReadBufferLine
// Put one header line from the buffer to the line array
//-----------------------------------------------------------------------------
void HRFDoqFile::ReadBufferLine()const
    {
    m_pLine = &(m_pHeaderBuffer[m_CurrentBufferPos]);
    m_CurrentBufferPos += DOQ_HEADER_LINE_LENGTH;
    }

//-----------------------------------------------------------------------------
// Protected
// ReadHeader
// This method reads the file header
//-----------------------------------------------------------------------------
void HRFDoqFile::ReadHeader()
    {
    HPRECONDITION (m_pDoqFile != 0);


    string CurrentArgument;

    CurrentArgument = GetField(BAND_CONTENT, 1);


    while((CurrentArgument.length() > 0))
        {
        m_BandContent.push_back(CurrentArgument);
        (m_KeywordMap[BAND_CONTENT]).m_NextLineToRead++;
        CurrentArgument = GetField(BAND_CONTENT, 1);

        }

    if(!GetField(BITS_PER_PIXEL,1,&m_BitsPerPixel))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    if(!GetField(SAMPLES_AND_LINES, 1, &m_ImageWidth))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    if(!GetField(SAMPLES_AND_LINES, 2, &m_ImageHeight))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    if(!GetField(BYTE_COUNT, 1, &m_HeaderSize))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    m_SLO = TranslateScanlineOrientation(GetField(RASTER_ORDER, 1));

    }

//-----------------------------------------------------------------------------
// Protected
// GetField
// Find and stores the Field associated with the Keyword
//-----------------------------------------------------------------------------
bool HRFDoqFile::GetField(KeywordName pi_Keyword, uint32_t pi_ArgNb, unsigned short* po_pValReturn )const
    {
    HPRECONDITION (m_pDoqFile != 0);



    bool FoundKeyword = false;
    uint32_t KeywordLength = 0;

    KeywordMap::const_iterator loc = m_KeywordMap.find(pi_Keyword);

    if(loc == m_KeywordMap.end() || pi_ArgNb > loc->second.m_NbArgs)
        {
        //...
        }
    else
        {
        FoundKeyword = true;
        KeywordLength = (uint32_t)strlen(loc->second.m_Keyword);
        if(GetKeywordLine(pi_Keyword))
            FoundKeyword = true;
        else
            FoundKeyword = false;
        *po_pValReturn = (unsigned short) atoi(GetFieldString(KeywordLength, pi_ArgNb).c_str());

        }

    return FoundKeyword;
    }

//-----------------------------------------------------------------------------
// Protected
// GetField
// Find and stores the Keyword
//-----------------------------------------------------------------------------
bool HRFDoqFile::GetField(KeywordName pi_Keyword, uint32_t pi_ArgNb, uint32_t* po_pValReturn )const
    {
    HPRECONDITION (m_pDoqFile != 0);


    bool FoundKeyword = false;
    uint32_t KeywordLength = 0;

    KeywordMap::const_iterator loc = m_KeywordMap.find(pi_Keyword);

    if(loc == m_KeywordMap.end() || pi_ArgNb > loc->second.m_NbArgs)
        {
        //...
        }
    else
        {
        FoundKeyword = true;
        KeywordLength = (uint32_t)strlen(loc->second.m_Keyword);
        if(GetKeywordLine(pi_Keyword))
            FoundKeyword = true;
        else
            FoundKeyword = false;
        *po_pValReturn = (uint32_t) atoi(GetFieldString(KeywordLength, pi_ArgNb).c_str());

        }

    return FoundKeyword;
    }

//-----------------------------------------------------------------------------
// Protected
// GetField
// Find and stores the Keyword
//-----------------------------------------------------------------------------
string HRFDoqFile::GetField(KeywordName pi_Keyword, uint32_t pi_ArgNb )const
    {
    HPRECONDITION (m_pDoqFile != 0);


    uint32_t KeywordLength = 0;
    KeywordMap::const_iterator loc = m_KeywordMap.find(pi_Keyword);

    if(loc == m_KeywordMap.end() || pi_ArgNb > loc->second.m_NbArgs)
        {
        return "";
        }
    else
        {
        KeywordLength = (uint32_t)strlen(loc->second.m_Keyword);
        if(GetKeywordLine(pi_Keyword))
            {
            return GetFieldString(KeywordLength, pi_ArgNb);
            }
        else
            return "";
        }
    }


//-----------------------------------------------------------------------------
// Protected
// GetFieldString
//
//-----------------------------------------------------------------------------
string HRFDoqFile::GetFieldString(int pi_Offset, uint32_t pi_ArgNb)const
    {
    HPRECONDITION (m_pDoqFile != 0);



    size_t FirstSpaceLocation=0;
    size_t SecondSpaceLocation;
    string Line;

    Line =  string(m_pLine, DOQ_HEADER_LINE_LENGTH );
    Line = StringToLower(Line);

    SecondSpaceLocation = Line.find_first_not_of(" ",pi_Offset);
    if(SecondSpaceLocation > DOQ_HEADER_LINE_LENGTH)
        {
        //cant find the right argument
        Line = "";
        }
    else
        {
        uint32_t i=0;
        while(i<pi_ArgNb)
            {
            FirstSpaceLocation = SecondSpaceLocation;
            if(i>0)
                {
                FirstSpaceLocation = Line.find_first_not_of(" ",FirstSpaceLocation);
                }
            SecondSpaceLocation = Line.find(" ",FirstSpaceLocation);


            if(SecondSpaceLocation != string::npos && SecondSpaceLocation < DOQ_HEADER_LINE_LENGTH)
                {
                i++;
                }
            else
                {
                throw HFCCorruptedFileException(m_pURL->GetURL());
                }
            }

        }
    return Line.substr(FirstSpaceLocation, SecondSpaceLocation - FirstSpaceLocation);


    }


//-----------------------------------------------------------------------------
// Protected
// GetField
// Find and stores the Keyword
//-----------------------------------------------------------------------------
bool HRFDoqFile::GetField(KeywordName pi_Keyword, uint32_t pi_ArgNb, double* po_pValReturn)const
    {
    HPRECONDITION (m_pDoqFile != 0);


    bool FoundKeyword = false;
    uint32_t KeywordLength = 0;

    KeywordMap::const_iterator loc = m_KeywordMap.find(pi_Keyword);

    if(loc == m_KeywordMap.end() || pi_ArgNb > loc->second.m_NbArgs)
        {
        //...
        }
    else
        {
        KeywordLength = (uint32_t)strlen(loc->second.m_Keyword);
        if(GetKeywordLine(pi_Keyword))
            FoundKeyword = true;
        else
            FoundKeyword = false;
        *po_pValReturn = (double) atof(GetFieldString(KeywordLength, pi_ArgNb).c_str());
        }

    return FoundKeyword;
    }


//-----------------------------------------------------------------------------
// Protected
// GetKeywordLine
// Finds the 80-char string beginning with that keyword
//---------------------------------------------------------------------------

bool HRFDoqFile::GetKeywordLine(KeywordName pi_Keyword)const
    {
    bool FoundKeyword = false;
    bool ReachEndHeader = false;
    uint32_t KeywordLength = 0;
    unsigned short CurrentKeywordNb = 0;

    KeywordMap::const_iterator loc_end = m_KeywordMap.find(END);
    KeywordMap::const_iterator loc_keyword = m_KeywordMap.find(pi_Keyword);


    KeywordLength = (uint32_t)strlen(loc_keyword->second.m_Keyword);
    m_pDoqFile->SeekToBegin();
    //TODO : tko m_CurrentBufferPos = 0;

    m_pDoqFile->Read(m_pLine,DOQ_HEADER_LINE_LENGTH);
    //TODO : tko ReadBufferLine();


    while(FoundKeyword == false && ReachEndHeader == false)
        {
        if(strncmp(m_pLine, loc_keyword->second.m_Keyword, KeywordLength)==0)
            {
            CurrentKeywordNb++;
            if(CurrentKeywordNb == loc_keyword->second.m_NextLineToRead)
                {
                FoundKeyword = true;
                }
            else
                {
                m_pDoqFile->Read(m_pLine,DOQ_HEADER_LINE_LENGTH);
                //TODO : tko ReadBufferLine();
                }

            }
        else
            {
            m_pDoqFile->Read(m_pLine,DOQ_HEADER_LINE_LENGTH);
            //TODO : tko ReadBufferLine();
            }

        if(strncmp(m_pLine, " ", 1 )==0)
            {
            ReachEndHeader = true;
            }
        }


    return FoundKeyword;


    }



//-----------------------------------------------------------------------------
// Protected
// CreatePixelType
//
//-----------------------------------------------------------------------------
void HRFDoqFile::CreatePixelType()
    {
    HPRECONDITION (m_pDoqFile != 0);
    HPRECONDITION (m_BitsPerPixel !=0);


    DoqColorType ColorType = DoqUndefinedColor;
    string ColorTypeString;

    if(m_BandContent.size() == 1)
        {
        ColorTypeString = m_BandContent.at(0);
        if(ColorTypeString.compare(0, 11, "black&white") == 0)
            {
            ColorType = DoqBlackAndWhite;

            }
        }
    else if(m_BandContent.size() == 3)
        {
        if( m_BandContent.at(0).compare(0,3, "red") == 0 &&
            m_BandContent.at(1).compare(0,5, "green") == 0 &&
            m_BandContent.at(2).compare(0,4, "blue") == 0)
            {
            ColorType = DoqRGB;

            }
        }

    if(ColorType != DoqUndefinedColor)
        {
        switch(m_BitsPerPixel)
            {
            case 8:

                if(ColorType == DoqBlackAndWhite)
                    {
                    m_pPixelType =  new HRPPixelTypeV8Gray8();

                    }
                else if(ColorType == DoqRGB)
                    {
                    m_pPixelType = new HRPPixelTypeV24R8G8B8();
                    }

                break;

            case 16:

                if(ColorType == DoqBlackAndWhite)
                    {
                    m_pPixelType =  new HRPPixelTypeV16Gray16();
                    }
                else if(ColorType == DoqRGB)
                    {
                    m_pPixelType = new HRPPixelTypeV16R5G6B5();
                    }

                break;


            default:
                throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                break;
            }
        }
    else
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    HPOSTCONDITION (m_pPixelType != 0);
    }

//-----------------------------------------------------------------------------
// Protected
// MapKeyword
// Associates Keyword with ID
//-----------------------------------------------------------------------------
void HRFDoqFile::MapHeader()
    {
    m_KeywordMap[BITS_PER_PIXEL].m_Keyword = "BITS_PER_PIXEL";
    m_KeywordMap[BITS_PER_PIXEL].m_NbArgs = 1;
    m_KeywordMap[BITS_PER_PIXEL].m_MultipleKeyword = false;
    m_KeywordMap[BITS_PER_PIXEL].m_NextLineToRead = 1;
    m_KeywordMap[SAMPLES_AND_LINES].m_Keyword = "SAMPLES_AND_LINES";
    m_KeywordMap[SAMPLES_AND_LINES].m_NbArgs = 2;
    m_KeywordMap[SAMPLES_AND_LINES].m_MultipleKeyword = false;
    m_KeywordMap[SAMPLES_AND_LINES].m_NextLineToRead = 1;
    m_KeywordMap[DATA_FILE_SIZE].m_Keyword = "DATA_FILE_SIZE";
    m_KeywordMap[DATA_FILE_SIZE].m_NbArgs = 1;
    m_KeywordMap[DATA_FILE_SIZE].m_MultipleKeyword = false;
    m_KeywordMap[DATA_FILE_SIZE].m_NextLineToRead = 1;
    m_KeywordMap[BYTE_COUNT].m_Keyword = "BYTE_COUNT";
    m_KeywordMap[BYTE_COUNT].m_NbArgs = 1;
    m_KeywordMap[BYTE_COUNT].m_MultipleKeyword = false;
    m_KeywordMap[BYTE_COUNT].m_NextLineToRead = 1;
    m_KeywordMap[BAND_ORGANIZATION].m_Keyword = "BAND_ORGANIZATION";
    m_KeywordMap[BAND_ORGANIZATION].m_NbArgs = 1;
    m_KeywordMap[BAND_ORGANIZATION].m_MultipleKeyword = false;
    m_KeywordMap[BAND_ORGANIZATION].m_NextLineToRead = 1;
    m_KeywordMap[BAND_CONTENT].m_Keyword = "BAND_CONTENT";
    m_KeywordMap[BAND_CONTENT].m_NbArgs = 1;
    m_KeywordMap[BAND_CONTENT].m_MultipleKeyword = true;
    m_KeywordMap[BAND_CONTENT].m_NextLineToRead = 1;
    m_KeywordMap[BEGIN].m_Keyword = "BEGIN_USGS_DOQ_HEADER";
    m_KeywordMap[BEGIN].m_NbArgs = 0;
    m_KeywordMap[BEGIN].m_MultipleKeyword = false;
    m_KeywordMap[BEGIN].m_NextLineToRead = 1;
    m_KeywordMap[END].m_Keyword = "END_DOQ_HEADER";
    m_KeywordMap[END].m_NbArgs = 0;
    m_KeywordMap[END].m_MultipleKeyword = false;
    m_KeywordMap[END].m_NextLineToRead = 1;
    m_KeywordMap[RASTER_ORDER].m_Keyword = "RASTER_ORDER";
    m_KeywordMap[RASTER_ORDER].m_NbArgs = 1;
    m_KeywordMap[RASTER_ORDER].m_MultipleKeyword = false;
    m_KeywordMap[RASTER_ORDER].m_NextLineToRead = 1;
    m_KeywordMap[H_RESOLUTION].m_Keyword = "HORIZONTAL_RESOLUTION";
    m_KeywordMap[H_RESOLUTION].m_NbArgs = 1;
    m_KeywordMap[H_RESOLUTION].m_MultipleKeyword = false;
    m_KeywordMap[H_RESOLUTION].m_NextLineToRead = 1;
    m_KeywordMap[H_RESOLUTION_UNITS].m_Keyword = "HORIZONTAL_UNITS";
    m_KeywordMap[H_RESOLUTION_UNITS].m_NbArgs = 1;
    m_KeywordMap[H_RESOLUTION_UNITS].m_MultipleKeyword = false;
    m_KeywordMap[H_RESOLUTION_UNITS].m_NextLineToRead = 1;
    m_KeywordMap[XY_ORIGIN].m_Keyword = "XY_ORIGIN";
    m_KeywordMap[XY_ORIGIN].m_NbArgs = 2;
    m_KeywordMap[XY_ORIGIN].m_MultipleKeyword = false;
    m_KeywordMap[XY_ORIGIN].m_NextLineToRead = 1;
    m_KeywordMap[HORIZONTAL_DATUM].m_Keyword = "HORIZONTAL_DATUM";
    m_KeywordMap[HORIZONTAL_DATUM].m_NbArgs = 1;
    m_KeywordMap[HORIZONTAL_DATUM].m_MultipleKeyword = false;
    m_KeywordMap[HORIZONTAL_DATUM].m_NextLineToRead = 1;
    m_KeywordMap[COORDINATE_ZONE].m_Keyword = "COORDINATE_ZONE";
    m_KeywordMap[COORDINATE_ZONE].m_NbArgs = 1;
    m_KeywordMap[COORDINATE_ZONE].m_MultipleKeyword = false;
    m_KeywordMap[COORDINATE_ZONE].m_NextLineToRead = 1;
    m_KeywordMap[HORIZONTAL_COORDINATE_SYSTEM].m_Keyword = "HORIZONTAL_COORDINATE_SYSTEM";
    m_KeywordMap[HORIZONTAL_COORDINATE_SYSTEM].m_NbArgs = 1;
    m_KeywordMap[HORIZONTAL_COORDINATE_SYSTEM].m_MultipleKeyword = false;
    m_KeywordMap[HORIZONTAL_COORDINATE_SYSTEM].m_NextLineToRead = 1;




    }
//-----------------------------------------------------------------------------
// Protected
// TranslateScanlineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFDoqFile::TranslateScanlineOrientation(const string& pi_rString)
    {
    HRFScanlineOrientation Orientation;



    if(pi_rString.compare(0,21,"left_right/top_bottom")==0 )
        Orientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    else if(pi_rString.compare(0,21,"left_right/bottom_top")==0)
        Orientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
    else
        {
        HASSERT(0);
        }


    return Orientation;

    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFDoqFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);
    HPRECONDITION (m_pDoqFile != 0);
    HPRECONDITION (m_pPixelType != 0);
    HPRECONDITION (m_ImageWidth > 0);
    HPRECONDITION (m_ImageHeight > 0);

    // Create Page and resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;
    HPMAttributeSet                         TagList;
    HFCPtr<HPMGenericAttribute>             pTag;

    //TODO : check for compression

    string ValueString;
    string ValueString2;
    unsigned short GeoShortValue;
    unsigned short GeoShortValue2;

    // RESOLUTIONUNIT Tag

    ValueString = GetField(H_RESOLUTION_UNITS, 1);
    if(ValueString.compare(0, 6, "meters")==0)
        {
        GeoShortValue = 3;
        m_RatioToMeter = 1.0;
        }
    else if(ValueString.compare(0, 4, "feet")==0)
        {
//        HGFDistanceUnit     Inches(0.0254);
//        HGFDistanceUnit     Meters;
//        HGFDistanceUnit     Feet(12, Inches);
//        Meters.ConvertFrom(Inches, 1.0);
        GeoShortValue = 2;
        m_RatioToMeter = 0.3048;

        }
    else
        {
        GeoShortValue = 1;
        m_RatioToMeter = 1.0;
        }
    pTag = new HRFAttributeResolutionUnit(GeoShortValue);
    TagList.Set(pTag);

    // XRESOLUTION Tag
    double Resolution;
    if(GetField(H_RESOLUTION, 1, &Resolution))
        {
        pTag = new HRFAttributeXResolution(Resolution);
        TagList.Set(pTag);
        pTag = new HRFAttributeYResolution(Resolution);
        TagList.Set(pTag);
        }

    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys(new HCPGeoTiffKeys());

    //Projection Type
    pGeoTiffKeys->AddKey(GTModelType, (uint32_t)TIFFGeo_ModelTypeGeographic);

    //Raster type : pixel is area, can it be otherwise for a DOQ ?
    pGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);

    //HORIZONTAL_DATUM = GEOG_CITATION
    ValueString = GetField(HORIZONTAL_DATUM, 1);

    pGeoTiffKeys->AddKey(GeogCitation, ValueString);

    if(!GetField(COORDINATE_ZONE, 1, &GeoShortValue2))
        {
        //undefined zone
        GeoShortValue2 = 0;
        }

    //GE0GRAPHIC TYPE
    bool IsUTM = (ValueString2.compare(0, 3, "utm") == 0);
    if(ValueString.compare(0, 5, "nad83")==0)
        {
        GeoShortValue = 4269;
        if( 3 <= GeoShortValue2 && GeoShortValue2 <=23 && IsUTM)
            {
            //store the Projected CS Type Codes in the same variable
            GeoShortValue2 = 26900 + GeoShortValue2;
            }
        else
            GeoShortValue2 = 0;
        }
    else if(ValueString.compare(0, 5, "nad27")==0)
        {
        GeoShortValue = 4267;
        if( 3 <= GeoShortValue2 && GeoShortValue2 <=23 && IsUTM )
            {
            //store the Projected CS Type Codes in the same variable
            GeoShortValue2 = 26700 + GeoShortValue2;
            }
        else
            GeoShortValue2 = 0;
        }
    else
        {
        //undefined
        GeoShortValue = 0;
        GeoShortValue2 = 0;
        }

    pGeoTiffKeys->AddKey(GeographicType, (uint32_t)GeoShortValue);
    pGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)GeoShortValue2);

    if(IsUTM)
        {   //UTM
        pGeoTiffKeys->AddKey(ProjCoordTrans, (uint32_t)1);
        }

    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        Resolution,                                     // XResolutionRatio,
        Resolution,                                     // YResolutionRatio,
        m_pPixelType,                                   // PixelType,
        new HCDCodecIdentity(),                         // Codec,
        HRFBlockAccess::RANDOM,                         // RBlockAccess,
        HRFBlockAccess::RANDOM,                         // WBlockAccess,
        GetScanLineOrientation(),                       // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        m_ImageWidth,                                   // Width image ,
        m_ImageHeight,                                  // Height image,
        m_ImageWidth,                                   // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);



    //warning : GetGeoRefInfo needs to be called before IsValidGeoRefInfo

    if(GetGeoRefInfo() && IsValidGeoRefInfo())
        {


        pPage = new HRFPageDescriptor ( GetAccessMode(),
                                        GetCapabilities(),                       // Capabilities,
                                        pResolution,                             // ResolutionDescriptor,
                                        0,                                       // RepresentativePalette,
                                        0,                                       // Histogram,
                                        0,                                       // Thumbnail,
                                        0,                                       // ClipShape,
                                        BuildTransfoModel(),                     // TransfoModel,
                                        0,                                       // Filters
                                        &TagList);                               // Attribute set

        }

    else
        {

        pPage = new HRFPageDescriptor ( GetAccessMode(),         // AccessMode
                                        GetCapabilities(),       // Capabilities,
                                        pResolution,             // ResolutionDescriptor,
                                        0,                       // RepresentativePalette,
                                        0,                       // Histogram,
                                        0,                       // Thumbnail,
                                        0,                       // ClipShape,
                                        0,                       // TransfoModel,
                                        0,                       // Filters
                                        &TagList);               // Attribute set
        }

    pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys));

    m_ListOfPageDescriptor.push_back(pPage);
    }
//-----------------------------------------------------------------------------
// Protected
// GetGeoRefInfo
//-----------------------------------------------------------------------------

bool HRFDoqFile::GetGeoRefInfo()
    {

    bool     ReturnBool = true;
    double  Resolution;
    if(GetField(H_RESOLUTION, 1, &Resolution))
        {
        m_DoqGeoRefInfo.m_A00 = Resolution;
        m_DoqGeoRefInfo.m_A11 = Resolution;
        }

    m_DoqGeoRefInfo.m_A10 = m_DoqGeoRefInfo.m_A01 = 0.0;

    if(!GetField(XY_ORIGIN, 1, &m_DoqGeoRefInfo.m_Tx ))
        {
        ReturnBool = false;
        }

    if(!GetField(XY_ORIGIN, 2, &m_DoqGeoRefInfo.m_Ty ))
        {
        ReturnBool = false;
        }
    else
        {
        //we inverse the Y-translation
        m_DoqGeoRefInfo.m_Ty =  - m_DoqGeoRefInfo.m_Ty;
        }


    return ReturnBool;


    }

//-----------------------------------------------------------------------------
// Protected
// IsValidGeoRefInfo
// Multiple returns for simplicity
//-----------------------------------------------------------------------------
bool HRFDoqFile::IsValidGeoRefInfo() const
    {


    if((HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_DoqGeoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR)))
        return false;

    // Origin must be greater that -2e294
    if (m_DoqGeoRefInfo.m_Tx < -HMAX_EPSILON || m_DoqGeoRefInfo.m_Ty < -HMAX_EPSILON)
        return false;

    // Origin must be smaller than 2e294
    if (m_DoqGeoRefInfo.m_Tx >= HMAX_EPSILON || m_DoqGeoRefInfo.m_Ty >= HMAX_EPSILON)
        return false;

    // Limit the pixelsize (?) inside [-2e294, 2e294]
    if (m_DoqGeoRefInfo.m_A00 > HMAX_EPSILON || m_DoqGeoRefInfo.m_A00 < -HMAX_EPSILON ||
        m_DoqGeoRefInfo.m_A11 > HMAX_EPSILON || m_DoqGeoRefInfo.m_A11 < -HMAX_EPSILON)
        return false;



    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
// Inspired form HRFBilFile::BuildTransfoModel
// The translation for doq files are defined from the center of the first pixel
// in the upper left corner.
//
//  x' = Ax + By + C
//  y' = Dx + Ey + F
//
//  where x'  = calculated x-coordinate of the pixel on the map
//        y'  = calculated y-coordinate of the pixel on the map
//        x   = column number of the pixel in the image
//        y   = row number of the pixel in the image
//        A   = x-scale
//        B,D = rotation term
//        C   = translation term; x-origin (x-coordinate of the center of the
//              upper left corner)
//        E   = y-scale
//        F   = translation term; y-origin (y-coordinate of the center of the
//              upper left corner)
//
// Change axes x,y by i,j
// where i = x - 0.5
//       j = y - 0.5
//
// Now, use the i and j axes instead of x and y
// x' = Ai + Bj + C  => x' = A(x - 0.5) + B(y - 0.5) + C  => x' = Ax + By + C - 0.5(A + B)
// y' = Di + Ej + F  => y' = D(x - 0.5) + E(y - 0.5) + F  => y' = Dx + Ey + F - 0.5(D + E)
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFDoqFile::BuildTransfoModel() const
    {
    // Build the transformation model.
    HFCPtr<HGF2DAffine> pModel;
    HFCPtr<HGF2DTransfoModel> pFinalTransfo;


    double FactorModelToMeter = CalculateFactorModelToMeter();


    pModel = new HGF2DAffine();
    ((HFCPtr<HGF2DAffine>&)pModel)->SetByMatrixParameters(m_DoqGeoRefInfo.m_Tx ,
                                                          m_DoqGeoRefInfo.m_A00,
                                                          m_DoqGeoRefInfo.m_A01,
                                                          m_DoqGeoRefInfo.m_Ty,
                                                          m_DoqGeoRefInfo.m_A10,
                                                          m_DoqGeoRefInfo.m_A11);

    // If we are in creationMode, create a new file, we don't apply the Factor, it will be
    // apply only at save time. Because if we apply the Factor here, and remove at the save time
    // the factor will be 1.0
    if ((FactorModelToMeter != 1.0) && !GetAccessMode().m_HasCreateAccess)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(FactorModelToMeter);
        pScaleModel->SetYScaling(FactorModelToMeter);

        pFinalTransfo = pModel->ComposeInverseWithDirectOf(*pScaleModel);
        }
    else
        pFinalTransfo = pModel;



    if (GetScanLineOrientation() == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
        {
        HFCPtr<HGF2DStretch> pSLOConverterModel = new HGF2DStretch();
        pSLOConverterModel->SetYScaling(-1.0);
        pSLOConverterModel->SetTranslation(HGF2DDisplacement(0.0, m_ImageHeight));

        pFinalTransfo = ((HFCPtr<HGF2DTransfoModel>&)pSLOConverterModel)->ComposeInverseWithDirectOf(*pFinalTransfo);
        }



    return pFinalTransfo;
    }
//-----------------------------------------------------------------------------
// Protected
// CalculateFactorModelToMeter
//
//-----------------------------------------------------------------------------
double HRFDoqFile::CalculateFactorModelToMeter()const
    {
    //for now we only return the XResolutionRatio
    //when others unit swill be supported, we'll have to make the conversion here
    return m_RatioToMeter;


    }



//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFDoqFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFDoqEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFDoqFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HASSERT(false);

    // Assert that no page has allready be entered
    HPRECONDITION (CountPages() == 0);
    HPRECONDITION (pi_pPage != 0);


    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);


    // Create a space at the beginning of the file to add an header
    if (m_Offset != 0)
        {
        HArrayAutoPtr<Byte> pTampon(new Byte[(size_t)m_Offset]);
        memset(pTampon,0x00, (size_t)m_Offset);

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        m_pDoqFile->SeekToBegin();
        m_pDoqFile->Write(pTampon, (size_t)m_Offset);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        }
    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFDoqFile::GetCapabilities () const
    {
    return (HRFDoqCreator::GetInstance()->GetCapabilities());
    }




//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFDoqFile::HRFDoqFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)

    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;

    m_CurrentBufferPos = 0;

    m_DoqGeoRefInfo.m_A00 = 0.0;
    m_DoqGeoRefInfo.m_A01 = 0.0;
    m_DoqGeoRefInfo.m_A10 = 0.0;
    m_DoqGeoRefInfo.m_A11 = 0.0;
    m_DoqGeoRefInfo.m_Tx = 0.0;
    m_DoqGeoRefInfo.m_Ty = 0.0;

    MapHeader();

    m_pLine = new char[DOQ_HEADER_LINE_LENGTH];


    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    else
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }

    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFDoqFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFDoqFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                               uint32_t pi_Page,
                                               bool   pi_CheckSpecificUnitSpec,
                                               bool   pi_InterpretUnitINTGR)
    {
    //There is already a default unit specified by the DOQ file format specification,
    //which is meter.
    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file without open
//-----------------------------------------------------------------------------
HRFDoqFile::HRFDoqFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    }

//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Doq file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFDoqFile::GetFilePtr  () const
    {
    return (m_pDoqFile);
    }
//-----------------------------------------------------------------------------
// Protected
// GetHeaderSize
// Get the header size.
//-----------------------------------------------------------------------------

uint32_t HRFDoqFile::GetHeaderSize() const
    {
    return m_HeaderSize;
    }

//-----------------------------------------------------------------------------
// Protected
// GetHeaderSize
// Get the header size.
//-----------------------------------------------------------------------------

uint32_t HRFDoqFile::GetBitsPerPixel() const
    {
    return m_BitsPerPixel;
    }

//-----------------------------------------------------------------------------
// Protected
// GetScanlineOrientation
// Get the header size.
//-----------------------------------------------------------------------------

HRFScanlineOrientation HRFDoqFile::GetScanLineOrientation() const
    {
    return m_SLO;
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbBands
// Get the number of bands
//-----------------------------------------------------------------------------

uint32_t HRFDoqFile::GetNbBands() const
    {
    return (uint32_t)m_BandContent.size();
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageWidth
// Get the number samples per line
//-----------------------------------------------------------------------------

uint32_t HRFDoqFile::GetImageWidth() const
    {
    return m_ImageWidth;
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageHeight
// Get the number of rows
//-----------------------------------------------------------------------------

uint32_t HRFDoqFile::GetImageHeight() const
    {
    return m_ImageHeight;
    }

//-----------------------------------------------------------------------------
// Protected
// GetTotalRowBytes
// Get the number of bytes per row
//-----------------------------------------------------------------------------

size_t HRFDoqFile::GetTotalRowBytes() const
    {
    return m_BandContent.size() * m_ImageWidth;
    }
