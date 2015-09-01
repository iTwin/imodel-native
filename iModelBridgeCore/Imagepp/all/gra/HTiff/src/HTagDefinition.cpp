//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTagDefinition.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFTagDefinition
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/HTagDefinition.h>
#include <ImagePP/all/h/HTIFFUtils.h>

const short HTagInfo::TAG_IO_ANY         = 0;
const short HTagInfo::TAG_IO_VARIABLE    = -1;
const short HTagInfo::TAG_IO_USE_SPP     = -2;

const int   HTagInfo::sDataLen[] =           {1,    // nothing
                                              1,    // BYTE
                                              1,    // ASCII
                                              2,    // SHORT
                                              4,    // LONG
                                              8,    // RATIONAL
                                              1,    // SBYTE
                                              1,    // UNDEFINED
                                              2,    // SSHORT
                                              4,    // SLONG
                                              8,    // SRATIONAL
                                              4,    // FLOAT
                                              8,    // DOUBLE
                                              2,    // ASCIIW
                                              0,    // unknow
                                              0,    // unknow
                                              8,    // LONG64
                                              8,    // SLONG64
                                              8,    // IFD64
                                             };



const char* HTagInfo::sConvertDataTypetoText (DataType pi_DataType)
    {
    switch (pi_DataType)
        {
        case HTagInfo::_NOTYPE:
            return ("NoType");
        case HTagInfo::BYTE:
            return ("Byte");
        case HTagInfo::ASCII:
            return ("ASCII");
        case HTagInfo::SHORT:
            return ("short");
        case HTagInfo::LONG:
            return ("Long");
        case HTagInfo::RATIONAL:
            return ("Rational");
        case HTagInfo::SBYTE:
            return ("SByte");
        case HTagInfo::UNDEFINED:
            return ("Undefined");
        case HTagInfo::SSHORT:
            return ("SShort");
        case HTagInfo::SLONG:
            return ("SLong");
        case HTagInfo::SRATIONAL:
            return ("SRational");
        case HTagInfo::FLOAT:
            return ("Float");
        case HTagInfo::DOUBLE:
            return ("Double");
        case HTagInfo::ASCIIW:
            return ("ASCIIW");
        case HTagInfo::LONG64:
            return ("LONG64");
        case HTagInfo::SLONG64:
            return ("SLONG64");
        case HTagInfo::IFD64:
            return ("IFD64");

        default:
            return ("UnknownType");
        }
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HTagDefinition::HTagDefinition(const HTagInfo& pi_rTagInfo, uint32_t pi_FileTag, bool pi_IsTiff64)

    {
    m_pTagInfo  = 0;
    m_pError    = 0;

    const size_t NumberOfDef = pi_rTagInfo.GetTagDefinitionQty();
    const HTagInfo::Info* pTagInfoArray = pi_rTagInfo.GetTagDefinitionArray();

    size_t Index;

    FindFirstTag (NumberOfDef, pTagInfoArray, pi_FileTag, &Index);

    // BigTiff
    if(pi_IsTiff64)
        {
        if (pi_rTagInfo.IsBigTiffTag(pi_FileTag))
            {
            while (m_pTagInfo->Type != HTagInfo::LONG64)
                FindNextTag (NumberOfDef, pTagInfoArray, pi_FileTag, &Index);
            }
        }
    }


HTagDefinition::HTagDefinition(const HTagInfo& pi_rTagInfo, HTagID pi_Tag, bool pi_IsTiff64)

    {
    m_pTagInfo  = 0;
    m_pError    = 0;

    const size_t NumberOfDef = pi_rTagInfo.GetTagDefinitionQty();
    const HTagInfo::Info* pTagInfoArray = pi_rTagInfo.GetTagDefinitionArray();

    for (size_t i=0; i<NumberOfDef; i++)
        {
        if (pTagInfoArray[i].TagID == pi_Tag)
            {
            // BigTiff
            if(pi_IsTiff64)
                {
                if(pi_rTagInfo.IsBigTiffTag(pTagInfoArray[i].FileTag))
                    {
                    if (pTagInfoArray[i].Type == HTagInfo::LONG64)
                        {
                        m_pTagInfo = &(pTagInfoArray[i]);
                        i = NumberOfDef;
                        }
                    }
                else
                    {
                    m_pTagInfo = &(pTagInfoArray[i]);
                    i = NumberOfDef;
                    }

                }
            else
                {
                m_pTagInfo = &(pTagInfoArray[i]);
                i = NumberOfDef;
                }
            }
        }

    if (m_pTagInfo == 0)
        {
        HTIFFError::UnknownTagEnumErInfo ErInfo;
        ErInfo.m_Tag = pi_Tag;
        ErrorMsg (&m_pError, HTIFFError::UNKNOWN_TAG_ENUM, &ErInfo, false);
        }
    }


HTagDefinition::HTagDefinition(const HTagInfo& pi_rTagInfo, FileDirEntry64& pi_TagDescriptor, bool pi_IsTiff64)
    {
    m_pTagInfo  = 0;
    m_pError    = 0;

    const size_t NumberOfDef = pi_rTagInfo.GetTagDefinitionQty();
    const HTagInfo::Info* pTagInfoArray = pi_rTagInfo.GetTagDefinitionArray();

    size_t Index;
    if (FindFirstTag (NumberOfDef, pTagInfoArray, pi_TagDescriptor.FileTag, &Index))
        {
        do {
            // Check DataType
            if (pi_TagDescriptor.DataType == m_pTagInfo->Type)
                break;

            }
        while (FindNextTag(NumberOfDef, pTagInfoArray, pi_TagDescriptor.FileTag, &Index));
        }

    // Check for error
    if (m_pError == 0)
        {
        // Tag with a compatible DataType found?
        if (pi_TagDescriptor.DataType != m_pTagInfo->Type)
            {
            HTIFFError::WrongTagDataTypeErInfo ErInfo;
            ErInfo.m_Type      = pi_TagDescriptor.DataType;
            string TagName = string(GetTagName());
            ErInfo.m_TagName    = WString(TagName.c_str(),false);
            ErInfo.m_TagFileNb = GetFileTag();
            ErrorMsg (&m_pError, HTIFFError::WRONG_DATA_TYPE_FOR_TAG, &ErInfo, false);
            }
        }
    else
        {
        // Unknown Tag, display Info
        HTIFFError::UnknownTagErInfo ErInfo;
        ErInfo.m_TagFileNb = pi_TagDescriptor.FileTag;
        string Type   = string(HTagInfo::sConvertDataTypetoText(static_cast<HTagInfo::DataType>(pi_TagDescriptor.DataType)));
        ErInfo.m_Type = WString(Type.c_str(),false);

        if (pi_IsTiff64)
            {
            ErInfo.m_Length = pi_TagDescriptor.DirCount64;
            ErInfo.m_Offset = pi_TagDescriptor.Offset64;
            }
        else
            {
            ErInfo.m_Length = pi_TagDescriptor.DirCount32;
            ErInfo.m_Offset = pi_TagDescriptor.Offset32;
            }

        ErrorMsg (&m_pError, HTIFFError::UNKNOWN_TAG, &ErInfo, false);
        }
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HTagDefinition::~HTagDefinition()
    {
    delete m_pError;
    }




// ------------------------------------------------ Privates

//
// Search the specify Tag in the list
//
// po_pIndex : Index in the table.
//
bool HTagDefinition::FindFirstTag (size_t                  pi_TagDefinitionQty,
                                    const HTagInfo::Info*   pi_pTagDefinitionArray,
                                    uint32_t                pi_TagFile,
                                    size_t*                 po_pIndex)
    {
    m_pTagInfo = 0;

    for (size_t i=0; i<pi_TagDefinitionQty; i++)
        {
        if (pi_pTagDefinitionArray[i].FileTag == pi_TagFile)
            {
            m_pTagInfo = &(pi_pTagDefinitionArray[i]);
            *po_pIndex = i;
            break;
            }
        }

    if (m_pTagInfo == 0)
        {
        HTIFFError::UnkwnownFirstTagErInfo ErInfo;
        ErInfo.m_TagFile = pi_TagFile;
        ErrorMsg (&m_pError, HTIFFError::UNKNOWN_FIRST_TAG, &ErInfo, false);
        }

    return (m_pTagInfo != 0);
    }

//
// A tag can have many DataType
// This nethod check the next data type for the specify Tag
//
// pio_pIndex : Index in the table.
//
bool HTagDefinition::FindNextTag (size_t                   pi_TagDefinitionQty,
                                   const HTagInfo::Info*    pi_pTagDefinitionArray,
                                   uint32_t                 pi_TagFile,
                                   size_t*                  pio_pIndex)
    {
    bool Ret = false;

    size_t i = (*pio_pIndex)+1;
    if (i < pi_TagDefinitionQty)
        {
        // Same Tag, then an other Data type
        if (pi_pTagDefinitionArray[i].FileTag == pi_TagFile)
            {
            m_pTagInfo = &(pi_pTagDefinitionArray[i]);
            *pio_pIndex = i;
            Ret = true;
            }
        }

    return (Ret);
    }
