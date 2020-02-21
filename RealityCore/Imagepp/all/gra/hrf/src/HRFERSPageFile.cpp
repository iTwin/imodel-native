//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFERSPageFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>
   

#include <ImagePP/all/h/HTIFFTag.h>

#include <ImagePP/all/h/HGFAngle.h>
#include <ImagePP/all/h/HRFERSPageFile.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCStat.h>
#include <ImagePP/all/h/HFCBinStream.h>
#include <ImagePP/all/h/HFCIniFileBrowser.h>

#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HRFCalsFile.h>
#include <ImagePP/all/h/HRFUtility.h>

#include <ImagePP/all/h/HCPGeoTiffKeys.h>

#include <ImagePP/all/h/HRFErMapperSupportedFile.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>



//ERS possible entries

//Block descriptor suffix
#define BEGIN_BLOCK_SUFFIX  " Begin"
#define END_BLOCK_SUFFIX    " End"

//DatasetHeader Block
#define DATASET_HEADER_BLOCK "DatasetHeader"
//Compulsory entries
#define VERSION         "Version"
#define DATASET_TYPE    "DataSetType"
#define DATATYPE        "DataType"
#define BYTEORDER       "ByteOrder"

#define VERSION_FLAG                        0X0001
#define DATASET_TYPE_FLAG                   0X0002
#define DATATYPE_FLAG                       0X0004
#define BYTEORDER_FLAG                      0X0008
#define COORDINATE_SPACE_BLOCK_FLAG         0X0010
#define RASTER_INFO_BLOCK_FLAG              0X0020
#define ALL_DATASET_HEADER_COMPULSORY_EN    0X003F

//CoordinateSpace Block
#define COORDINATE_SPACE_BLOCK "CoordinateSpace"

//Compulsory entries
#define DATUM           "Datum"
#define PROJECTION      "Projection"
#define COORDINATE_TYPE "CoordinateType"

#define DATUM_FLAG                      0X0001
#define PROJECTION_FLAG                 0X0002
#define COORDINATE_TYPE_FLAG            0X0004
#define ALL_COORD_SPACE_COMPULSORY_EN   0X0007

//Optional entries
#define UNITS           "Units"
#define ROTATION        "Rotation"
//End of CoordinateSpace block

//RasterInfo block
#define RASTER_INFO_BLOCK "RasterInfo"

//Compulsory entries
#define CELL_TYPE               "CellType"
#define NR_OF_LINES             "NrOfLines"
#define NR_OF_CELLS_PER_LINE    "NrOfCellsPerLine"
#define NR_OF_BANDS             "NrOfBands"

#define CELL_TYPE_FLAG                  0X0001
#define NR_OF_LINES_FLAG                0X0002
#define NR_OF_CELLS_PER_LINE_FLAG       0X0004
#define NR_OF_BANDS_FLAG                0X0008
#define ALL_RASTER_INFO_COMPULSORY_EN   0X000F

//Optional entries
#define REGISTRATION_CELL_X "RegistrationCellX"
#define REGISTRATION_CELL_Y "RegistrationCellY"

//CellInfo Block
#define CELL_INFO_BLOCK "CellInfo"

//Compulsory entries
#define X_DIMENSION "Xdimension"
#define Y_DIMENSION "Ydimension"
//End of CellInfo block

//RegistrationCoord block
#define REGISTRATION_COORD_BLOCK "RegistrationCoord"

//Compulsory entries are one of the following pair :
//If CoordinateType equals EN
#define EASTINGS    "Eastings"
#define NORTHINGS   "Northings"
//If CoordinateType equals RAW
#define METERSX     "MetersX"
#define METERSY     "MetersY"
//If CoordinateType equals LL
#define LATITUDE    "Latitude"
#define LONGITUDE   "Longitude"
//End of RegistrationCoord block
//End of RasterInfo block

//Optional entries
//None currently used by Imagepp

//End Of DatasetHeader Block

#define SET_REG_COORD_TYPE(pi_pRegCoord, pi_RegParamType, pi_rUrl) \
if (pi_pRegCoord->m_CoordType == ERSCoordinateSpaceInfo::UNDEFINED) \
{ \
    pi_pRegCoord->m_CoordType = pi_RegParamType; \
} \
else \
{ \
    if (pi_pRegCoord->m_CoordType != pi_RegParamType) \
    { \
        throw HRFERSUnmatchRegSpaceCoordTypexception(pi_rUrl); \
    } \
}

//-----------------------------------------------------------------------------
// Class HRFERSCapabilities
//-----------------------------------------------------------------------------
HRFERSCapabilities::HRFERSCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));


    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0)));
    }


// Singleton
HFC_IMPLEMENT_SINGLETON(HRFERSPageFileCreator)

//-----------------------------------------------------------------------------
// Class HRFERSPageFileCreator
//
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 @return true if the file sister file ERS associate with the Raster file
         is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true : Try to find an associate ERS sister file with
                                  all types of files(ITiff, GeoTiff, etc)
                     false(default) : Try to find an associate sister file ERS
                     only with the files don't support Georeference.
-----------------------------------------------------------------------------*/
bool HRFERSPageFileCreator::HasFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                    bool                         pi_ApplyonAllFiles) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    bool HasPageFile = false;

    // TR 108925 HFC_READ_ONLY must be used because we don't want sister of georeferenced file like sid and ecw if pi_ApplyonAllFiles is OFF.
    //(pi_ApplyonAllFiles || !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_CREATE_ONLY))) )

    // We added pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) to fix a problem caused by the TR 162805
    //  This TR adds the capability of TransfoModel on CAL file only to support SLO.

    // Check only the first page in the File.
    if ((pi_rpForRasterFile->GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID)) &&
        (pi_rpForRasterFile->CountPages() <= 1) &&  // don't support the multipage
        (pi_ApplyonAllFiles || pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) ||
         !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_READ_ONLY))) )
        {
        HFCPtr<HFCURL>  URLForPageFile = ComposeURLFor(pi_rpForRasterFile->GetURL());
        if (URLForPageFile != 0)
            {
            HFCStat PageFileStat(URLForPageFile);

            // Check if the decoration file exist and the time stamp.
            if (PageFileStat.IsExistent())
                HasPageFile = true;
            }
        }

    return HasPageFile;
    }

//-----------------------------------------------------------------------------
// public
// CreateFor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageFile> HRFERSPageFileCreator::CreateFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    //Supported only in read-only
    HPRECONDITION(pi_rpForRasterFile->GetAccessMode().m_HasCreateAccess == false);

    // Multi-page not supported
    if (pi_rpForRasterFile->CountPages() > 1)
        throw HRFMultiPageNotSupportedException(pi_rpForRasterFile->GetURL()->GetURL());

    HFCPtr<HRFPageFile> pPageFile;

    pPageFile = new HRFERSPageFile(ComposeURLFor(pi_rpForRasterFile->GetURL()),
                                   pi_rpForRasterFile->GetAccessMode());

    return pPageFile;
    }

//-----------------------------------------------------------------------------
// public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFERSPageFileCreator::ComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName) const
    {
    HPRECONDITION(pi_rpURLFileName != 0);

    HFCPtr<HFCURL> URLForPageFile;

    if (!pi_rpURLFileName->IsCompatibleWith(HFCURLFile::CLASS_ID))
        throw HFCInvalidUrlForSisterFileException(pi_rpURLFileName->GetURL());

    // Extract the Path
    Utf8String Path(((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetAbsoluteFileName());

    // Find the file extension
    Utf8String::size_type DotPos = Path.rfind('.');

    // Extract the extension and the drive dir name
    if (DotPos != Utf8String::npos)
        {
        // Compose the decoration file name
        Utf8String DriveDirName = Path.substr(0, DotPos);
        URLForPageFile = new HFCURLFile(Utf8String(HFCURLFile::s_SchemeName() + "://") + DriveDirName + Utf8String(".ers"));
        }
    else
        URLForPageFile = new HFCURLFile(Utf8String(HFCURLFile::s_SchemeName() + "://") + Path + Utf8String(".ers"));

    return URLForPageFile;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFERSPageFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFERSCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Class HRFERSPageFile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFERSPageFile::HRFERSPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                               HFCAccessMode           pi_AccessMode)
    : HRFPageFile(pi_rpURL, pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);

    m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode, 0, true);

    ReadFile();

    CreateDescriptor();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFERSPageFile::~HRFERSPageFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFERSPageFile::GetCapabilities () const
    {
    return HRFERSPageFileCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFERSPageFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }


void HRFERSPageFile::WriteToDisk()
    {
    HASSERT(0); //Only supported in read-only mode

    }

//-----------------------------------------------------------------------------
// Private
// ReadEntryValue
//-----------------------------------------------------------------------------
bool HRFERSPageFile::ReadEntryValue(const string& pi_rStringToParse, AStringR po_rValue) const
    {
    bool IsEntryFound = false;

    const char* pEntryValue = strchr(pi_rStringToParse.c_str(), '=');

    if (pEntryValue != 0)
        {
        pEntryValue++;

        string EntryValue = pEntryValue;

        CleanUpString(EntryValue);

        po_rValue = EntryValue.c_str();

        IsEntryFound = true;
        }

    return IsEntryFound;
    }

//-----------------------------------------------------------------------------
// Private
// ReadLine
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadLine(string& po_rString)
    {
    HPRECONDITION(m_pFile != 0);

    const int32_t BufferSize = 64;
    char      Buffer[BufferSize+1];
    string    CurrentLine;

    bool EndOfLine = false;
    po_rString.erase();
    while (!EndOfLine)
        {
        memset(Buffer, 0, BufferSize+1);
        for (uint16_t i = 0; i < BufferSize && !EndOfLine; i++)
            {
            m_pFile->Read(&Buffer[i], 1);
            EndOfLine = Buffer[i] == '\n' || m_pFile->EndOfFile();
            }

        po_rString += Buffer;
        }

    CleanUpString(po_rString);
    }

//-----------------------------------------------------------------------------
// Private
// CleanUpString
//
// Remove SPACE/TAB/ENTER from the begin and end of the file.
//-----------------------------------------------------------------------------
void HRFERSPageFile::CleanUpString(string& pio_rString) const
    {
    int32_t Pos = 0;

    // Remove the SPACE/TAB at the begin of the string.
    while (Pos < (int32_t) pio_rString.size() && !IsValidChar(pio_rString[Pos]))
        Pos++;

    pio_rString = pio_rString.substr(Pos);

    // Remove the SPACE/TAB/ENTER at the end of the string.
    if (pio_rString.size() > 0)
        {
        Pos = (int32_t) pio_rString.size() - 1;
        while(Pos >= 0 && !IsValidChar(pio_rString[Pos]))
            Pos--;

        pio_rString = pio_rString.substr(0, Pos+1);
        }
    }

//-----------------------------------------------------------------------------
// Private
// IsValidChar
//
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER.
//-----------------------------------------------------------------------------
bool HRFERSPageFile::IsValidChar(const char pi_Char) const
    {
    bool IsValid = true;

    switch (pi_Char)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
        case '"' :
            IsValid = false;
            break;
        }

    return IsValid;
    }

//-----------------------------------------------------------------------------
// Private
// ReadFile
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadFile()
    {
    HPRECONDITION(m_pFile != 0);

    HFCIniFileBrowser InitFile(m_pFile);
    AString           EntryValue;
    string            LineRead;

    uint32_t         DatasetHeadRequiredEntries = 0;
    uint32_t         RasterInfoRequiredEntries = 0;
    uint32_t         CoordSpaceRequiredEntries = 0;

    ReadLine(LineRead);

    while (m_pFile->EndOfFile() == false)
        {
        if (LineRead == (string(DATASET_HEADER_BLOCK) + string(BEGIN_BLOCK_SUFFIX)))
            {
            do
                {
                ReadLine(LineRead);

                if (BeStringUtilities::Strnicmp(LineRead.c_str(), VERSION, strlen(VERSION)) == 0)
                    {
                    DatasetHeadRequiredEntries |= VERSION_FLAG;

                    ReadEntryValue(LineRead.substr(strlen(VERSION)), m_ERSInfo.m_Version);
                    }
                else if (BeStringUtilities::Strnicmp(LineRead.c_str(), DATASET_TYPE, strlen(DATASET_TYPE)) == 0)
                    {
                    DatasetHeadRequiredEntries |= DATASET_TYPE_FLAG;

                    ReadEntryValue(LineRead.substr(strlen(DATASET_TYPE)), EntryValue);

                    if (EntryValue == "ERStorage")
                        {
                        m_ERSInfo.m_DatasetType = ERSDatasetHeaderInfo::ERSTORAGE;
                        }
                    else if (EntryValue == "Translated")
                        {
                        m_ERSInfo.m_DatasetType = ERSDatasetHeaderInfo::TRANSLATED;
                        }
                    else
                        {
                        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), DATASET_TYPE);
                        }
                    }
                else if (BeStringUtilities::Strnicmp(LineRead.c_str(), DATATYPE, strlen(DATATYPE)) == 0)
                    {
                    DatasetHeadRequiredEntries |= DATATYPE_FLAG;

                    ReadEntryValue(LineRead.substr(strlen(DATATYPE)), EntryValue);

                    if (EntryValue == "Raster")
                        {
                        m_ERSInfo.m_DataType = ERSDatasetHeaderInfo::RASTER;
                        }
                    else if (EntryValue == "Vector")
                        {
                        m_ERSInfo.m_DataType = ERSDatasetHeaderInfo::VECTOR;
                        }
                    else
                        {
                        //throw exception
                        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), DATATYPE);
                        }
                    }
                else if (BeStringUtilities::Strnicmp(LineRead.c_str(), BYTEORDER, strlen(BYTEORDER)) == 0)
                    {
                    DatasetHeadRequiredEntries |= BYTEORDER_FLAG;
                    }
                else if (LineRead == (string(COORDINATE_SPACE_BLOCK) + string(BEGIN_BLOCK_SUFFIX)))
                    {
                    DatasetHeadRequiredEntries |= COORDINATE_SPACE_BLOCK_FLAG;

                    ReadCoordinateSpaceBlock(CoordSpaceRequiredEntries);
                    }
                else if (LineRead == (string(RASTER_INFO_BLOCK) + string(BEGIN_BLOCK_SUFFIX)))
                    {
                    DatasetHeadRequiredEntries |= RASTER_INFO_BLOCK_FLAG;
                    ReadRasterInfoBlock(RasterInfoRequiredEntries);
                    }
                }
            while (LineRead != (string(DATASET_HEADER_BLOCK) + string(BEGIN_BLOCK_SUFFIX)) &&
                   (m_pFile->EndOfFile() == false));

            if (LineRead != (string(DATASET_HEADER_BLOCK) + string(END_BLOCK_SUFFIX)))
                {
                Utf8String ParamGroup = Utf8String(DATASET_HEADER_BLOCK) +
                                     Utf8String(END_BLOCK_SUFFIX);

                throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(), ParamGroup.c_str());
                }
            }
        else
            {
            if (LineRead.size() != 0)
                {
                Utf8String ParamGroup(LineRead.c_str());

                throw HRFERSDataFoundOutsideDatasetHeaderException(m_pFile->GetURL()->GetURL(), ParamGroup.c_str());
                }
            }

        ReadLine(LineRead);
        }

    ValidateERSFile(DatasetHeadRequiredEntries,
                    RasterInfoRequiredEntries,
                    CoordSpaceRequiredEntries);
    }

//-----------------------------------------------------------------------------
// Private
// ReadCoordinateSpaceBlock
// Read the coordinate space block in a .ERS file.
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadCoordinateSpaceBlock(uint32_t& po_rCoordSpaceRequiredEntries)
    {
    AString           EntryValue;
    string            LineRead;

    do
        {
        ReadLine(LineRead);

        if (BeStringUtilities::Strnicmp(LineRead.c_str(), DATUM, strlen(DATUM)) == 0)
            {
            po_rCoordSpaceRequiredEntries |= DATUM_FLAG;

            ReadEntryValue(LineRead.substr(strlen(DATUM)), m_ERSInfo.m_CoordSpaceInfo.m_Datum);
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), PROJECTION, strlen(PROJECTION)) == 0)
            {
            po_rCoordSpaceRequiredEntries |= PROJECTION_FLAG;

            ReadEntryValue(LineRead.substr(strlen(PROJECTION)),
                           m_ERSInfo.m_CoordSpaceInfo.m_Projection);
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), COORDINATE_TYPE, strlen(COORDINATE_TYPE)) == 0)
            {
            po_rCoordSpaceRequiredEntries |= COORDINATE_TYPE_FLAG;

            ReadEntryValue(LineRead.substr(strlen(COORDINATE_TYPE)), EntryValue);

            if (EntryValue == "RAW")
                {
                m_ERSInfo.m_CoordSpaceInfo.m_CoordinateType = ERSCoordinateSpaceInfo::RAW;
                }
            else if (EntryValue == "EN")
                {
                m_ERSInfo.m_CoordSpaceInfo.m_CoordinateType = ERSCoordinateSpaceInfo::EN;
                }
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), UNITS, strlen(UNITS)) == 0)
            {
            m_ERSInfo.m_CoordSpaceInfo.m_pUnits = new AString;

            ReadEntryValue(LineRead.substr(strlen(UNITS)), *m_ERSInfo.m_CoordSpaceInfo.m_pUnits);
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), ROTATION, strlen(ROTATION)) == 0)
            {
            m_ERSInfo.m_CoordSpaceInfo.m_pRotation = new double;

            ReadEntryValue(LineRead.substr(strlen(ROTATION)), EntryValue);

            Utf8String valueUtf8(EntryValue.c_str());
            bool ConvSuccess = ConvertDegMinSecToDeg(valueUtf8, *m_ERSInfo.m_CoordSpaceInfo.m_pRotation);

            if (ConvSuccess == false)
                {
                throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), ROTATION);
                }
            }
        }
    while ((LineRead != (string(COORDINATE_SPACE_BLOCK) + string(END_BLOCK_SUFFIX))) &&
           (m_pFile->EndOfFile() == false));

    if (LineRead != (string(COORDINATE_SPACE_BLOCK) + string(END_BLOCK_SUFFIX)))
        {
        Utf8String ParamGroup = Utf8String(COORDINATE_SPACE_BLOCK) + Utf8String(END_BLOCK_SUFFIX);

        throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(),ParamGroup.c_str());
        }
    }

//-----------------------------------------------------------------------------
// Private
// ReadRasterInfoBlock
// Read the raster info block in a .ERS file.
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadRasterInfoBlock(uint32_t& po_rRasterInfoRequiredEntries)
    {
    AString           EntryValue;
    string            LineRead;

    do
        {
        ReadLine(LineRead);

        if (BeStringUtilities::Strnicmp(LineRead.c_str(), CELL_TYPE, strlen(CELL_TYPE)) == 0)
            {
            po_rRasterInfoRequiredEntries |= CELL_TYPE_FLAG;
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), NR_OF_LINES, strlen(NR_OF_LINES)) == 0)
            {
            po_rRasterInfoRequiredEntries |= NR_OF_LINES_FLAG;
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), NR_OF_CELLS_PER_LINE, strlen(NR_OF_CELLS_PER_LINE)) == 0)
            {
            po_rRasterInfoRequiredEntries |= NR_OF_CELLS_PER_LINE_FLAG;
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), NR_OF_BANDS, strlen(NR_OF_BANDS))== 0)
            {
            po_rRasterInfoRequiredEntries |= NR_OF_BANDS_FLAG;
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), REGISTRATION_CELL_X, strlen(REGISTRATION_CELL_X)) == 0)
            {
            m_ERSInfo.m_RasterInfo.m_pRegistrationCellX = new double;

            ReadEntryValue(LineRead.substr(strlen(REGISTRATION_CELL_X)), EntryValue);

            ConvertStringToDouble(EntryValue, m_ERSInfo.m_RasterInfo.m_pRegistrationCellX);
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), REGISTRATION_CELL_Y, strlen(REGISTRATION_CELL_Y)) == 0)
            {
            m_ERSInfo.m_RasterInfo.m_pRegistrationCellY = new double;

            ReadEntryValue(LineRead.substr(strlen(REGISTRATION_CELL_Y)), EntryValue);

            ConvertStringToDouble(EntryValue, m_ERSInfo.m_RasterInfo.m_pRegistrationCellY);
            }
        else if (LineRead == (string(CELL_INFO_BLOCK) + string(BEGIN_BLOCK_SUFFIX)))
            {
            ReadCellInfoBlock();
            }
        else if (LineRead == (string(REGISTRATION_COORD_BLOCK) + string(BEGIN_BLOCK_SUFFIX)))
            {
            ReadRegistrationCoordBlock() ;
            }
        }
    while (LineRead != (string(RASTER_INFO_BLOCK) + string(END_BLOCK_SUFFIX)) &&
           (m_pFile->EndOfFile() == false));

    if (LineRead != (string(RASTER_INFO_BLOCK) + string(END_BLOCK_SUFFIX)))
        {
        Utf8String ParamGroup = Utf8String(RASTER_INFO_BLOCK) + Utf8String(END_BLOCK_SUFFIX);

        throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(), ParamGroup.c_str());
        }
    }

//-----------------------------------------------------------------------------
// Private
// ReadRasterInfoBlock
// Read the cell info block in a .ERS file.
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadCellInfoBlock()
    {
    AString           EntryValue;
    string            LineRead;

    m_ERSInfo.m_RasterInfo.m_pCellInfo = new ERSCellInfo();

    do
        {
        ReadLine(LineRead);

        if (BeStringUtilities::Strnicmp(LineRead.c_str(), X_DIMENSION, strlen(X_DIMENSION)) == 0)
            {
            ReadEntryValue(LineRead.substr(strlen(X_DIMENSION)), EntryValue);

            ConvertStringToDouble(EntryValue, &(m_ERSInfo.m_RasterInfo.m_pCellInfo->m_XDimension));
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), Y_DIMENSION, strlen(Y_DIMENSION)) == 0)
            {
            ReadEntryValue(LineRead.substr(strlen(Y_DIMENSION)), EntryValue);

            ConvertStringToDouble(EntryValue, &(m_ERSInfo.m_RasterInfo.m_pCellInfo->m_YDimension));
            }
        }
    while (LineRead != (string(CELL_INFO_BLOCK) + string(END_BLOCK_SUFFIX)) && (m_pFile->EndOfFile() == false));

    if (LineRead != (string(CELL_INFO_BLOCK) + string(END_BLOCK_SUFFIX)))
        {
        Utf8String ParamGroup = Utf8String(CELL_INFO_BLOCK) + Utf8String(END_BLOCK_SUFFIX);

        throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(), ParamGroup.c_str());
        }
    }

//-----------------------------------------------------------------------------
// Private
// ReadRasterInfoBlock
// Read the registration coordinate block in a .ERS file.
//-----------------------------------------------------------------------------
void HRFERSPageFile::ReadRegistrationCoordBlock()
    {
    AString           EntryValue;
    string            LineRead;
    ERSRegistrationCoord* pRegCoord = new ERSRegistrationCoord;

    m_ERSInfo.m_RasterInfo.m_pRegistrationCoord = pRegCoord;

    do
        {
        ReadLine(LineRead);

        if (BeStringUtilities::Strnicmp(LineRead.c_str(), EASTINGS, strlen(EASTINGS)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::EN,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(EASTINGS)),
                           EntryValue);

            ConvertStringToDouble(EntryValue, &(pRegCoord->m_X));
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), NORTHINGS, strlen(NORTHINGS)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::EN,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(NORTHINGS)),
                           EntryValue);

            ConvertStringToDouble(EntryValue, &(pRegCoord->m_Y));
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), METERSX, strlen(METERSX)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::RAW,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(METERSX)),
                           EntryValue);

            ConvertStringToDouble(EntryValue, &(pRegCoord->m_X));
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), METERSY, strlen(METERSY)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::RAW,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(METERSY)),
                           EntryValue);

            ConvertStringToDouble(EntryValue, &(pRegCoord->m_Y));
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), LONGITUDE, strlen(LONGITUDE)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::LL,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(LONGITUDE)),
                           EntryValue);

            Utf8String valueUtf8(EntryValue.c_str());
            bool ConvSuccess = ConvertDegMinSecToDeg(valueUtf8,
                                                      pRegCoord->m_X);
            if (ConvSuccess == false)
                {
                throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), LONGITUDE);
                }
            }
        else if (BeStringUtilities::Strnicmp(LineRead.c_str(), LATITUDE, strlen(LATITUDE)) == 0)
            {
            SET_REG_COORD_TYPE(pRegCoord,
                               ERSCoordinateSpaceInfo::LL,
                               m_pFile->GetURL()->GetURL())

            ReadEntryValue(LineRead.substr(strlen(LATITUDE)),
                           EntryValue);

            Utf8String valueUtf8(EntryValue.c_str());
            bool ConvSuccess = ConvertDegMinSecToDeg(valueUtf8,
                                                      pRegCoord->m_Y);
            if (ConvSuccess == false)
                {
                throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), LATITUDE);
                }
            }
        }
    while (LineRead != (string(REGISTRATION_COORD_BLOCK) + string(END_BLOCK_SUFFIX)) &&
           (m_pFile->EndOfFile() == false));

    if (LineRead != (string(REGISTRATION_COORD_BLOCK) + string(END_BLOCK_SUFFIX)))
        {
        Utf8String ParamGroup = Utf8String(REGISTRATION_COORD_BLOCK) + Utf8String(END_BLOCK_SUFFIX);

        throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(),
                                        ParamGroup.c_str());
        }
    }

//-----------------------------------------------------------------------------
// Private
// ValidateERSFile
// Validate that all required parameters are presents and the coherence
// of some values.
//-----------------------------------------------------------------------------
void HRFERSPageFile::ValidateERSFile(uint32_t pi_DatasetHeadRequiredEntries,
                                     uint32_t pi_RasterInfoRequiredEntries,
                                     uint32_t pi_CoordSpaceRequiredEntries)
    {
    //Validate
    if (pi_DatasetHeadRequiredEntries != ALL_DATASET_HEADER_COMPULSORY_EN)
        {
        if ((pi_DatasetHeadRequiredEntries & VERSION_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), VERSION);
            }
        else if ((pi_DatasetHeadRequiredEntries & DATASET_TYPE_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), DATASET_TYPE);
            }
        else if ((pi_DatasetHeadRequiredEntries & DATATYPE_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), DATATYPE);
            }
        else if ((pi_DatasetHeadRequiredEntries & BYTEORDER_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), BYTEORDER);
            }
        else if ((pi_DatasetHeadRequiredEntries & COORDINATE_SPACE_BLOCK_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), COORDINATE_SPACE_BLOCK);
            }
        else if ((pi_DatasetHeadRequiredEntries & RASTER_INFO_BLOCK_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(), RASTER_INFO_BLOCK);
            }
        }

    if (pi_CoordSpaceRequiredEntries != ALL_COORD_SPACE_COMPULSORY_EN)
        {
        if ((pi_CoordSpaceRequiredEntries & DATUM_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), DATUM);
            }
        else if ((pi_CoordSpaceRequiredEntries & PROJECTION_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),PROJECTION);
            }
        else if ((pi_CoordSpaceRequiredEntries & COORDINATE_TYPE_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), COORDINATE_TYPE);
            }
        }

    if (pi_RasterInfoRequiredEntries != ALL_RASTER_INFO_COMPULSORY_EN)
        {
        if ((pi_RasterInfoRequiredEntries & CELL_TYPE_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), CELL_TYPE);
            }
        else if ((pi_RasterInfoRequiredEntries & NR_OF_LINES_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), NR_OF_LINES);
            }
        else if ((pi_RasterInfoRequiredEntries & NR_OF_CELLS_PER_LINE_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), NR_OF_CELLS_PER_LINE);
            }
        else if ((pi_RasterInfoRequiredEntries & NR_OF_BANDS_FLAG) == 0)
            {
            throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(), NR_OF_BANDS);
            }
        }

    if (m_ERSInfo.m_DataType != ERSDatasetHeaderInfo::RASTER)
        {
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(), DATATYPE);
        }

    //The registration coordinate type must match the
    //coordinate type of the coordinate space.
    if ((m_ERSInfo.m_RasterInfo.m_pRegistrationCoord != 0) &&
        (m_ERSInfo.m_RasterInfo.m_pRegistrationCoord->m_CoordType !=
         m_ERSInfo.m_CoordSpaceInfo.m_CoordinateType))
        {
        throw HRFERSUnmatchRegSpaceCoordTypexception(m_pFile->GetURL()->GetURL());
        }

    //Validate the pixel resolution
    if (m_ERSInfo.m_RasterInfo.m_pCellInfo != 0)
        {
        if (m_ERSInfo.m_RasterInfo.m_pCellInfo->m_XDimension <= 0)
            {
            Utf8String Param = Utf8String(CELL_INFO_BLOCK) +
                            Utf8String("::") +
                            Utf8String(X_DIMENSION);

            throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                            Param.c_str());
            }

        if (m_ERSInfo.m_RasterInfo.m_pCellInfo->m_YDimension <= 0)
            {
            Utf8String Param = Utf8String(CELL_INFO_BLOCK) +
                            Utf8String("::") +
                            Utf8String(Y_DIMENSION);

            throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                            Param.c_str());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static GeoCoordinates::BaseGCSPtr s_CreateRasterGcsFromERSIDS(CharCP projection, CharCP datum, CharCP unit, double* po_UnitToMeter)
    {
    uint32_t EPSGCodeFomrERLibrary = TIFFGeo_UserDefined;

    #if defined(IPP_HAVE_ERMAPPER_SUPPORT) 
        EPSGCodeFomrERLibrary = HRFErMapperSupportedFile::GetEPSGFromProjectionAndDatum(projection, datum);
    #endif

    return HCPGCoordUtility::CreateRasterGcsFromERSIDS(EPSGCodeFomrERLibrary, projection, datum, unit, po_UnitToMeter);
    }

//-----------------------------------------------------------------------------
// Private
// CreateDescriptor
//-----------------------------------------------------------------------------
void HRFERSPageFile::CreateDescriptor()
    {
    HPMAttributeSet              TagList;
    HFCPtr<HPMGenericAttribute>  pTag;
    double                       OriginX = 0;
    double                       OriginY = 0;
    double                       PixelSizeX = 1;
    double                       PixelSizeY = 1;
    double                       Rotation = 0;
    GeoCoordinates::BaseGCSPtr   pBaseGCS;
    double                       UnitToMeter = 1.0;

    if (m_ERSInfo.m_RasterInfo.m_pCellInfo != 0)
        {
        pTag = new HRFAttributeXResolution(m_ERSInfo.m_RasterInfo.m_pCellInfo->m_XDimension);
        TagList.Set(pTag);

        PixelSizeX = m_ERSInfo.m_RasterInfo.m_pCellInfo->m_XDimension;

        pTag = new HRFAttributeYResolution(m_ERSInfo.m_RasterInfo.m_pCellInfo->m_YDimension);
        TagList.Set(pTag);

        PixelSizeY = m_ERSInfo.m_RasterInfo.m_pCellInfo->m_YDimension;
        }

    if (m_ERSInfo.m_RasterInfo.m_pRegistrationCoord != 0)
        {
        OriginX = m_ERSInfo.m_RasterInfo.m_pRegistrationCoord->m_X;
        OriginY = m_ERSInfo.m_RasterInfo.m_pRegistrationCoord->m_Y;
        }

    if (m_ERSInfo.m_CoordSpaceInfo.m_pRotation != 0)
        {
        Rotation = *m_ERSInfo.m_CoordSpaceInfo.m_pRotation;
        }

    if (m_ERSInfo.m_CoordSpaceInfo.m_Datum != "RAW")
        {
        if (m_ERSInfo.m_RasterInfo.m_pRegistrationCoord == 0)
            {
            throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(), REGISTRATION_COORD_BLOCK);
            }

        // We process RAW as LOCAL
        if (m_ERSInfo.m_CoordSpaceInfo.m_Projection == "RAW")
            {
            m_ERSInfo.m_CoordSpaceInfo.m_Projection = "LOCAL";
            }

        // Sometimes the unit is not set
        if (nullptr == m_ERSInfo.m_CoordSpaceInfo.m_pUnits)
            {
            pBaseGCS = s_CreateRasterGcsFromERSIDS(m_ERSInfo.m_CoordSpaceInfo.m_Projection.c_str(), m_ERSInfo.m_CoordSpaceInfo.m_Datum.c_str(), "", &UnitToMeter);
            }
        else
            {
            pBaseGCS = s_CreateRasterGcsFromERSIDS(m_ERSInfo.m_CoordSpaceInfo.m_Projection.c_str(), m_ERSInfo.m_CoordSpaceInfo.m_Datum.c_str(), m_ERSInfo.m_CoordSpaceInfo.m_pUnits->c_str(), &UnitToMeter);
            }
        }
    else if (nullptr != m_ERSInfo.m_CoordSpaceInfo.m_pUnits && *m_ERSInfo.m_CoordSpaceInfo.m_pUnits.get() != "RAW")
        {
        pBaseGCS = s_CreateRasterGcsFromERSIDS(m_ERSInfo.m_CoordSpaceInfo.m_Projection.c_str(), m_ERSInfo.m_CoordSpaceInfo.m_Datum.c_str(), m_ERSInfo.m_CoordSpaceInfo.m_pUnits->c_str(), &UnitToMeter);
        }

    double RegCellX = 0.0;
    double RegCellY = 0.0;

    if (m_ERSInfo.m_RasterInfo.m_pRegistrationCellX != 0)
        {
        RegCellX = *m_ERSInfo.m_RasterInfo.m_pRegistrationCellX;
        }

    if (m_ERSInfo.m_RasterInfo.m_pRegistrationCellY != 0)
        {
        RegCellY = *m_ERSInfo.m_RasterInfo.m_pRegistrationCellY;
        }

    HFCPtr<HGF2DTransfoModel> pTransfoModel = BuildTransfoModel(OriginX,
                                                                OriginY,
                                                                PixelSizeX,
                                                                PixelSizeY,
                                                                Rotation,
                                                                RegCellX,
                                                                RegCellY,
                                                                pBaseGCS.get(),
                                                                UnitToMeter);

    HRFScanlineOrientation TransfoModelSLO = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

    // Create the Page information and add it to the list of page descriptor
    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor(GetAccessMode(),
                                  GetCapabilities(),    // Capabilities,
                                  0,                    // RepresentativePalette,
                                  0,                    // Histogram,
                                  0,                    // Thumbnail,
                                  0,                    // ClipShape,
                                  pTransfoModel,        // TransfoModel,
                                  &TransfoModelSLO,     // TransfoModelOrientation
                                  0,                    // Filters
                                  &TagList);            // Tag


    pPage->SetGeocoding(pBaseGCS.get());

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Private
// ConvertStringToDouble
//-----------------------------------------------------------------------------
bool HRFERSPageFile::ConvertStringToDouble(AStringCR pi_rString, double* po_pDouble) const
    {
    HPRECONDITION(po_pDouble != 0);

    char* pStopPtr;
    *po_pDouble = strtod(pi_rString.c_str(), &pStopPtr);

    return (pStopPtr - pi_rString.c_str() == pi_rString.length());
    }

//-----------------------------------------------------------------------------
// Private
// BuildTransfoModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFERSPageFile::BuildTransfoModel(double                        pi_OriginX,
                                                            double                        pi_OriginY,
                                                            double                        pi_PixelSizeX,
                                                            double                        pi_PixelSizeY,
                                                            double                        pi_Rotation,
                                                            double                        pi_RegistrationX,
                                                            double                        pi_RegistrationY,
                                                            GeoCoordinates::BaseGCSP      pi_pBaseGCS,
                                                            double                        pi_UnitToMeter) const
    {
    // Transform the rotation to ensure the value lies in the range of [-360,360]
    //if it wasn't the case
    while (pi_Rotation > 360)
        pi_Rotation -= 360;
    while (pi_Rotation < -360)
        pi_Rotation += 360;

    // Build the transformation model.
    double            Degree = (PI/180);

    HGF2DDisplacement Displacement(pi_OriginX + (-pi_RegistrationX * pi_PixelSizeX), 
                                   pi_OriginY + (pi_RegistrationY * pi_PixelSizeY));

    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DAffine(Displacement, pi_Rotation * Degree, pi_PixelSizeX, -pi_PixelSizeY, 0.0);

    if (pi_pBaseGCS != nullptr && pi_pBaseGCS->IsValid())
        pModel = HCPGCoordUtility::TranslateToMeter(pModel, 1.0 / pi_pBaseGCS->UnitsFromMeters());
    else
        { // Apply unit if possible
        pModel = HCPGCoordUtility::TranslateToMeter(pModel, pi_UnitToMeter);
        }

    return pModel->CreateSimplifiedModel();
    }
