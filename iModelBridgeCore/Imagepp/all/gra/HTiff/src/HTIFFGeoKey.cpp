//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFGeoKey.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFGeoKey
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HTIFFGeoKey.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HTIFFTagDefinition.h>



static const int KEYVALUE       = 0;
static const int TAGVALUE       = 1;
static const int COUNTVALUE     = 2;
static const int OFFSETVALUE    = 3;

const HTIFFGeoKey::GeoKeyDefinition HTIFFGeoKey::sGeoKeyInfo[] = {
        {GTModelType              , HTagInfo::SHORT  },
        {GTRasterType             , HTagInfo::SHORT  },
        {GTCitation               , HTagInfo::ASCII  },
        {GeographicType           , HTagInfo::SHORT  },
        {GeogCitation             , HTagInfo::ASCII  },
        {GeogGeodeticDatum        , HTagInfo::SHORT  },
        {GeogPrimeMeridian        , HTagInfo::SHORT  },
        {GeogLinearUnits          , HTagInfo::SHORT  },
        {GeogLinearUnitSize       , HTagInfo::DOUBLE },
        {GeogAngularUnits         , HTagInfo::SHORT  },
        {GeogAngularUnitSize      , HTagInfo::DOUBLE },
        {GeogEllipsoid            , HTagInfo::SHORT  },
        {GeogSemiMajorAxis        , HTagInfo::DOUBLE },
        {GeogSemiMinorAxis        , HTagInfo::DOUBLE },
        {GeogInvFlattening        , HTagInfo::DOUBLE },
        {GeogAzimuthUnits         , HTagInfo::SHORT  },
        {GeogPrimeMeridianLong    , HTagInfo::DOUBLE },
        {ProjectedCSType          , HTagInfo::SHORT  },
        {PCSCitation              , HTagInfo::ASCII  },
        {Projection               , HTagInfo::SHORT  },
        {ProjCoordTrans           , HTagInfo::SHORT  },
        {ProjLinearUnits          , HTagInfo::SHORT  },
        {ProjLinearUnitSize       , HTagInfo::DOUBLE },
        {ProjStdParallel1         , HTagInfo::DOUBLE },
        {ProjStdParallel2         , HTagInfo::DOUBLE },
        {ProjNatOriginLong        , HTagInfo::DOUBLE },
        {ProjNatOriginLat         , HTagInfo::DOUBLE },
        {ProjFalseEasting         , HTagInfo::DOUBLE },
        {ProjFalseNorthing        , HTagInfo::DOUBLE },
        {ProjFalseOriginLong      , HTagInfo::DOUBLE },
        {ProjFalseOriginLat       , HTagInfo::DOUBLE },
        {ProjFalseOriginEasting   , HTagInfo::DOUBLE },
        {ProjFalseOriginNorthing  , HTagInfo::DOUBLE },
        {ProjCenterLong           , HTagInfo::DOUBLE },
        {ProjCenterLat            , HTagInfo::DOUBLE },
        {ProjCenterEasting        , HTagInfo::DOUBLE },
        {ProjCenterNorthing       , HTagInfo::DOUBLE },
        {ProjScaleAtNatOrigin     , HTagInfo::DOUBLE },
        {ProjScaleAtCenter        , HTagInfo::DOUBLE },
        {ProjAzimuthAngle         , HTagInfo::DOUBLE },
        {ProjStraightVertPoleLong , HTagInfo::DOUBLE },
        {ProjRectifiedGridAngle   , HTagInfo::DOUBLE },
        {VerticalCSType           , HTagInfo::SHORT  },
        {VerticalCitation         , HTagInfo::ASCII  },
        {VerticalDatum            , HTagInfo::SHORT  },
        {VerticalUnits            , HTagInfo::SHORT  }

    };

const uint32_t HTIFFGeoKey::sNumberOfDefs = (sizeof (sGeoKeyInfo) / sizeof (sGeoKeyInfo[0]));

//-----------------------------------------------------------------------------
// STATIC METHOD
// RETURNS THE EXPECTED DATA TYPE FOR GEO KEY
//-----------------------------------------------------------------------------
HTagInfo::DataType HTIFFGeoKey::sGetExpectedDataType(GeoKeyID pi_Key)
    {
    for (long Index = 0 ; Index < HTIFFGeoKey::sNumberOfDefs ; ++Index)
        {
        if (sGeoKeyInfo[Index].Key == pi_Key)
            return sGeoKeyInfo[Index].Type;
        }

    return HTagInfo::_NOTYPE;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HTIFFGeoKey::HTIFFGeoKey ()
    {
    m_KeyIsDirty = false;
    m_pError     = 0;
    }

HTIFFGeoKey::HTIFFGeoKey (const unsigned short* pi_pGeoKeyDirectory, uint32_t pi_GeoKeyCount,
                          const double* pi_pGeoDoubleParams, uint32_t pi_GeoDoubleCount,
                          const char* pi_pGeoASCIIParams)
    {
    HPRECONDITION(pi_pGeoKeyDirectory != 0);

    m_KeyIsDirty = false;
    m_pError     = 0;

    // Validate Version.
    // the 3 first value are Version, RevMaj, RevMin
    if ((pi_pGeoKeyDirectory[0] > TIFFGEO_VERSION) ||
        (pi_pGeoKeyDirectory[1] > TIFFGEO_REV_MAJOR))
        {
        ErrorMsg(&m_pError, HTIFFError::UNSUPPORTED_GEOTIFF_VERSION, 0, false);
        }

    // the 4 short is the number of entry.
    unsigned short Count = pi_pGeoKeyDirectory[3];
    uint32_t ASCIIParamsLen = 0;
    if (pi_pGeoASCIIParams != 0)
        ASCIIParamsLen = (uint32_t)strlen(pi_pGeoASCIIParams);

    // Each entry has 4 shorts: GeoKey, [NoTag | 0], Count, [Offset | Value]
    //   NoTag == TIFFTAG_GEODOUBLEPARAMS | TIFFTAG_GEOASCIIPARAMS
    const unsigned short* geoKeyDirectoryEnd = pi_pGeoKeyDirectory + pi_GeoKeyCount;

    for (unsigned short i=0; i < Count; ++i)
        {
        pi_pGeoKeyDirectory += 4;       // Next Entry

        if (pi_pGeoKeyDirectory >= geoKeyDirectoryEnd)
            {
            //TR 312623 : It seems that some software considers the header as an entry, so just pop an HASSERT in
            //            that case and ensure that the code doesn't do some buffer overrun.
            HASSERT(!"Incorrect GeoTIFF directory entry count");
            break;
            }

        // By value ?
        if (pi_pGeoKeyDirectory[TAGVALUE] == 0)
            {
            if (pi_pGeoKeyDirectory[COUNTVALUE] != 1)
                {
                HTIFFError::InvalidGeotiffCountOrIndexErInfo ErInfo;
                ErInfo.m_GeoKey = pi_pGeoKeyDirectory[KEYVALUE];
                ErrorMsg(&m_pError, HTIFFError::INVALID_GEOTIFF_COUNT, &ErInfo, false);
                }
            else
                {
                InsertNewKey((GeoKeyID)pi_pGeoKeyDirectory[KEYVALUE], HTagInfo::SHORT,
                             (Byte*)&(pi_pGeoKeyDirectory[OFFSETVALUE]), 0, pi_pGeoKeyDirectory[COUNTVALUE]);
                }
            }

        else if (pi_pGeoKeyDirectory[TAGVALUE] == HTIFFTAG_GEODOUBLEPARAMS)
            {
            // Validate position and Count
            if (((uint32_t)pi_pGeoKeyDirectory[COUNTVALUE]+(uint32_t)pi_pGeoKeyDirectory[OFFSETVALUE]) > pi_GeoDoubleCount)
                {
                HTIFFError::InvalidGeotiffCountOrIndexErInfo ErInfo;
                ErInfo.m_GeoKey = pi_pGeoKeyDirectory[KEYVALUE];
                ErrorMsg(&m_pError, HTIFFError::INVALID_GEOTIFF_INDEX_OR_COUNT, &ErInfo, false);
                }
            else
                {
                InsertNewKey((GeoKeyID)pi_pGeoKeyDirectory[KEYVALUE], HTagInfo::DOUBLE,
                             (Byte*)&(pi_pGeoDoubleParams[pi_pGeoKeyDirectory[OFFSETVALUE]]), 0, pi_pGeoKeyDirectory[COUNTVALUE]);
                }
            }

        else if (pi_pGeoKeyDirectory[TAGVALUE] == HTIFFTAG_GEOASCIIPARAMS)
            {
            // Validate position and Count
            if (((uint32_t)pi_pGeoKeyDirectory[COUNTVALUE]+(uint32_t)pi_pGeoKeyDirectory[OFFSETVALUE]) > ASCIIParamsLen)
                {
                HTIFFError::InvalidGeotiffCountOrIndexErInfo ErInfo;
                ErInfo.m_GeoKey = pi_pGeoKeyDirectory[KEYVALUE];
                ErrorMsg(&m_pError, HTIFFError::INVALID_GEOTIFF_INDEX_OR_COUNT, &ErInfo, false);
                }
            else
                {
                InsertNewKey((GeoKeyID)pi_pGeoKeyDirectory[KEYVALUE], HTagInfo::ASCII,
                             (Byte*)&(pi_pGeoASCIIParams[pi_pGeoKeyDirectory[OFFSETVALUE]]), 0,
                             pi_pGeoKeyDirectory[COUNTVALUE]);
                }
            }
        else
            {
            HTIFFError::InvalidGeotiffTagErInfo ErInfo;
            ErInfo.m_GeoKey = pi_pGeoKeyDirectory[KEYVALUE];
            ErInfo.m_Tag = pi_pGeoKeyDirectory[TAGVALUE];
            ErrorMsg(&m_pError, HTIFFError::INVALID_GEOTIFF_TAG, &ErInfo, false);
            }
        }
    }



//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HTIFFGeoKey::~HTIFFGeoKey()
    {
    Reset();
    }


//-----------------------------------------------------------------------------
// public
// Reset
//  Clear all geokeys from the list
//-----------------------------------------------------------------------------
void HTIFFGeoKey::Reset()
    {
    // If Key has been modified, need to rewrite the TIFF tag.
    GeoKeyList::iterator Itr;
    for (Itr = m_KeyList.begin(); Itr != m_KeyList.end();)
        {
        GeoKeyList::iterator DeletedItr(Itr);
        if (((*Itr).second).Count > 1)
            {
            if (!( (((*Itr).second).Count == 2) && (((*Itr).second).Type == HTagInfo::ASCII) ))
                delete[] ((*Itr).second).pData;
            }
        ++Itr;
        m_KeyList.erase (DeletedItr);
        }


// No need for this code anymore since the STL map bug
// is corrected (_Lockit)
#if 0
    m_KeyList.clear();
#endif
    }



//-----------------------------------------------------------------------------
// public
// GetGeoParams
//  - Reset the Dirty flags
//  - User must delete the parameters.
//  po_ppGeoDoubleParams and po_ppGeoASCIIParams can be 0 if not present
//-----------------------------------------------------------------------------
void HTIFFGeoKey::GetGeoParams(unsigned short** po_ppGeoKeyDirectory, uint32_t* po_pGeoKeyCount,
                               double** po_ppGeoDoubleParams, uint32_t* po_pGeoDoubleCount,
                               char** po_ppGeoASCIIParams)
    {
    HPRECONDITION(po_ppGeoKeyDirectory != 0);
    HPRECONDITION(po_ppGeoDoubleParams != 0);
    HPRECONDITION(po_ppGeoASCIIParams != 0);
    HPRECONDITION(po_pGeoKeyCount != 0);
    HPRECONDITION(po_pGeoDoubleCount != 0);

    uint32_t    ASCIICount   = 1;                               // terminator

    *po_pGeoDoubleCount     = 0;
    *po_pGeoKeyCount        = (uint32_t)((m_KeyList.size() + 1) * 4);    // +1 Header, *4 -> 4 shorts by entry

    // Count Number of Short, Double, ASCII
    GeoKeyList::iterator Itr;
    for (Itr = m_KeyList.begin(); Itr != m_KeyList.end(); ++Itr)
        {
        switch(((*Itr).second).Type)
            {
            case HTagInfo::ASCII:
                ASCIICount += ((*Itr).second).Count;
                break;

            case HTagInfo::DOUBLE:
                *po_pGeoDoubleCount += ((*Itr).second).Count;
                break;

            default:
                break;
            }
        }

    // Alloc Param...
    *po_ppGeoKeyDirectory   = new unsigned short[*po_pGeoKeyCount];
    unsigned short* pShort         = *po_ppGeoKeyDirectory;

    if (*po_pGeoDoubleCount == 0)
        *po_ppGeoDoubleParams   = 0;
    else
        *po_ppGeoDoubleParams   = new double[*po_pGeoDoubleCount];
    uint32_t DoubleCount      = 0;

    if (ASCIICount <= 1)
        *po_ppGeoASCIIParams = 0;
    else
        {
        *po_ppGeoASCIIParams    = new char[ASCIICount];
        memset(*po_ppGeoASCIIParams, 0, ASCIICount);
        }
    ASCIICount              = 0;

    // Set Header
    pShort[0] = TIFFGEO_VERSION;
    pShort[1] = TIFFGEO_REV_MAJOR;
    pShort[2] = TIFFGEO_REV_MINOR;
    pShort[3] = (unsigned short)m_KeyList.size();

    // Copy in Params
    for (Itr = m_KeyList.begin(); Itr != m_KeyList.end(); ++Itr)
        {
        pShort += 4;        // Next Entry

        // Key value
        pShort[KEYVALUE]   = (*Itr).first;
        pShort[COUNTVALUE] = ((*Itr).second).Count;

        switch(((*Itr).second).Type)
            {
            case HTagInfo::ASCII:
                pShort[TAGVALUE]    = HTIFFTAG_GEOASCIIPARAMS;
                pShort[OFFSETVALUE] = (unsigned short)ASCIICount;
                ASCIICount += ((*Itr).second).Count;

                if (((*Itr).second).Count == 2)
                    strcat(*po_ppGeoASCIIParams, (char*)&(((*Itr).second).DataShort));
                else
                    strcat(*po_ppGeoASCIIParams, (char*)((*Itr).second).pData);

                strcat(*po_ppGeoASCIIParams, "|");
                break;

            case HTagInfo::DOUBLE:
                pShort[TAGVALUE]    = HTIFFTAG_GEODOUBLEPARAMS;
                pShort[OFFSETVALUE] = (unsigned short)(DoubleCount);

                if (((*Itr).second).Count > 1)
                    {
                    memcpy(&((*po_ppGeoDoubleParams)[DoubleCount]), ((*Itr).second).pData,
                           (((*Itr).second).Count * sizeof(double)));
                    }
                else
                    {
                    (*po_ppGeoDoubleParams)[DoubleCount] = ((*Itr).second).DataDouble;
                    }

                DoubleCount += ((*Itr).second).Count;
                break;

            default:
                pShort[TAGVALUE]    = 0;       // By value
                pShort[OFFSETVALUE] = ((*Itr).second).DataShort;
                break;
            }
        }


    m_KeyIsDirty = false;
    }




//-----------------------------------------------------------------------------
// public
// IsValid
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::IsValid(HTIFFError**  po_ppError) const
    {
    if (po_ppError != 0)
        *po_ppError = m_pError;

    return (bool)(m_pError == 0);
    }


//-----------------------------------------------------------------------------
// public
// GetDataType
//
// Returns _NOTYPE if Key doesn't exist.
//-----------------------------------------------------------------------------
HTagInfo::DataType HTIFFGeoKey::GetDataType(GeoKeyID pi_Key) const
    {
    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        return ((*Itr).second).Type;
    else
        return HTagInfo::_NOTYPE;
    }

//-----------------------------------------------------------------------------
// public
// GetCount
//
// Returns 0 if Key doesn't exist.
//-----------------------------------------------------------------------------
uint32_t HTIFFGeoKey::GetCount (GeoKeyID pi_Key) const
    {
    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        return ((*Itr).second).Count;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// public
// GetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::GetValue (GeoKeyID pi_Key, unsigned short* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);

    bool Ret = true;

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::SHORT == ((*Itr).second).Type);

        *po_pVal = ((*Itr).second).DataShort;
        }
    else
        Ret = false;

    return Ret;
    }

//-----------------------------------------------------------------------------
// public
// GetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::GetValues (GeoKeyID pi_Key, double* po_pVal, uint32_t pi_Index, uint32_t pi_Count) const
    {
    HPRECONDITION(po_pVal != 0);

    Byte*  pRetValue   = 0;

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::DOUBLE == ((*Itr).second).Type);
        HASSERT(pi_Index < ((*Itr).second).Count);

        // Default case, only one element in this key
        if (((*Itr).second).Count == 1)
            pRetValue = (Byte*)&(((*Itr).second).DataDouble);
        else
            pRetValue = ((*Itr).second).pData;

        if (pi_Index > 0)
            {
            pRetValue += (HTagInfo::sGetDataLen(HTagInfo::DOUBLE) * pi_Index /
                          sizeof(double));
            }

        memcpy(po_pVal, pRetValue, pi_Count*sizeof(double));
        }

    return (pRetValue != 0);
    }

//-----------------------------------------------------------------------------
// public
// GetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::GetValues (GeoKeyID pi_Key, char* po_pVal, uint32_t pi_Index) const
    {
    HPRECONDITION(po_pVal != 0);

    Byte*  pRetValue   = 0;

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::ASCII == ((*Itr).second).Type);
        HASSERT(pi_Index < ((*Itr).second).Count);

        // Default case, only one element in this key
        if (((*Itr).second).Count == 1)
            pRetValue = (Byte*)&(((*Itr).second).DataShort);
        else
            pRetValue = ((*Itr).second).pData;

        if (pi_Index > 0)
            {
            pRetValue += (HTagInfo::sGetDataLen(HTagInfo::ASCII) * pi_Index /
                          sizeof(char));
            }

        strcpy(po_pVal, (char*)pRetValue);
        }

    return (pRetValue != 0);
    }


//-----------------------------------------------------------------------------
// public
// GetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::GetValues (GeoKeyID pi_Key, char** po_ppVal, uint32_t pi_Index) const
    {
    HPRECONDITION(po_ppVal != 0);

    char*  pRetValue = 0;

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::ASCII == ((*Itr).second).Type);
        HASSERT(pi_Index < ((*Itr).second).Count);

        // Default case, only one element in this key
        if (((*Itr).second).Count == 1)
            pRetValue = (char*)&(((*Itr).second).DataShort);
        else
            pRetValue = (char*)((*Itr).second).pData;

        if (pi_Index > 0)
            {
            pRetValue += (HTagInfo::sGetDataLen(HTagInfo::ASCII) * pi_Index /
                          sizeof(char));
            }

        *po_ppVal = (char*)pRetValue;
        }

    return (pRetValue != 0);
    }

//-----------------------------------------------------------------------------
// public
// SetValues
//  Only one value possibly, for the moment
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::SetValue (GeoKeyID pi_Key, unsigned short pi_Val)
    {
    bool   Ret         = true;

    GeoKeyList::iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::SHORT == ((*Itr).second).Type);

        ((*Itr).second).DataShort = pi_Val;
        }
    else
        InsertNewKey(pi_Key, HTagInfo::SHORT, (Byte*)&pi_Val, 0, 1);

    // If Key changed, set indicator.
    if (Ret)
        m_KeyIsDirty = true;
    return Ret;
    }


//-----------------------------------------------------------------------------
// public
// SetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::SetValues (GeoKeyID pi_Key, const double* pi_pVal, uint32_t pi_Index, uint32_t pi_Count)
    {
    HPRECONDITION(pi_pVal != 0);

    Byte*  pData;
    bool   Ret         = true;

    GeoKeyList::iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::DOUBLE == ((*Itr).second).Type);

        // Check if Key data can receive this value
        if ((pi_Index+pi_Count) > ((*Itr).second).Count)
            {
            // Need to realloc
            size_t PrevCount = ((*Itr).second).Count;

            ((*Itr).second).Count = (unsigned short)(pi_Index+pi_Count);
            pData = (Byte*)new double[((*Itr).second).Count];

            // Copy previous Data
            if (PrevCount == 1)
                memcpy(pData, &(((*Itr).second).DataDouble),  sizeof(double));
            else
                {
                memcpy(pData, ((*Itr).second).pData, PrevCount * sizeof(double));
                delete[] ((*Itr).second).pData;
                }

            ((*Itr).second).pData = pData;
            }

        // If count is 1, stock the value directly
        //      in the same way, the Index must be 0
        if ((((*Itr).second).Count == 1) && (pi_Index == 0))
            ((*Itr).second).DataDouble = *pi_pVal;
        else
            {
            memcpy (((*Itr).second).pData + (HTagInfo::sGetDataLen(HTagInfo::DOUBLE) *
                                             pi_Index / sizeof(double)),
                    pi_pVal, (pi_Count * sizeof(double)));
            }
        }
    else
        InsertNewKey(pi_Key, HTagInfo::DOUBLE, (Byte*)pi_pVal, pi_Index, pi_Count);

    // If Key changed, set indicator.
    if (Ret)
        m_KeyIsDirty = true;
    return Ret;
    }


//-----------------------------------------------------------------------------
// public
// SetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::SetValue (GeoKeyID pi_Key, double pi_Val, uint32_t pi_Index)
    {
    Byte*  pData;
    bool   Ret         = true;

    GeoKeyList::iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::DOUBLE == ((*Itr).second).Type);

        // Check if Key data can receive this value
        if ((pi_Index+((*Itr).second).Count) > ((*Itr).second).Count)
            {
            // Need to realloc
            size_t PrevCount = ((*Itr).second).Count;

            ((*Itr).second).Count = (unsigned short)(pi_Index+((*Itr).second).Count);
            pData = (Byte*)new double[((*Itr).second).Count];

            // Copy previous Data
            if (PrevCount == 1)
                memcpy(pData, &(((*Itr).second).DataDouble), sizeof(double));
            else
                {
                memcpy(pData, ((*Itr).second).pData, PrevCount * sizeof(double));
                delete[] ((*Itr).second).pData;
                }

            ((*Itr).second).pData = pData;
            }

        // If count is 1, stock the value directly
        //      in the same way, the Index must be 0
        if ((((*Itr).second).Count == 1) && (pi_Index == 0))
            ((*Itr).second).DataDouble = pi_Val;
        else
            {
            ((double*)((*Itr).second).pData)[pi_Index] = pi_Val;
            }
        }
    else
        InsertNewKey(pi_Key, HTagInfo::DOUBLE, (Byte*)&pi_Val, pi_Index, 1);

    // If Key changed, set indicator.
    if (Ret)
        m_KeyIsDirty = true;
    return Ret;
    }


//-----------------------------------------------------------------------------
// public
// SetValues
//-----------------------------------------------------------------------------
bool HTIFFGeoKey::SetValues (GeoKeyID pi_Key, const char* pi_pVal, uint32_t pi_Index)
    {
    HPRECONDITION(pi_pVal != 0);

    Byte*  pData;
    bool   Ret         = true;
    uint32_t Count = (uint32_t)strlen(pi_pVal) + 1;

    GeoKeyList::iterator Itr;
    if ((Itr = m_KeyList.find(pi_Key)) != m_KeyList.end())
        {
        HASSERT(HTagInfo::ASCII == ((*Itr).second).Type);

        // Check if Key data can receive this value
        if ((pi_Index+Count) > ((*Itr).second).Count)
            {
            // Need to realloc
            size_t PrevCount = ((*Itr).second).Count;

            ((*Itr).second).Count = (unsigned short)(pi_Index+Count);
            pData = (Byte*)new char[((*Itr).second).Count];

            // Copy previous Data
            if (PrevCount <= 2)
                memcpy(pData, &(((*Itr).second).DataShort), PrevCount * sizeof(char));
            else
                {
                memcpy(pData, ((*Itr).second).pData, PrevCount * sizeof(char));
                delete[] ((*Itr).second).pData;
                }

            ((*Itr).second).pData = pData;
            }

        // If count is 2 (that means string with 1 car. + 0), stock the value directly
        //      in the same way, the Index must be 0
        if ((((*Itr).second).Count == 2) && (pi_Index == 0))
            ((*Itr).second).DataShort = *((unsigned short*)pi_pVal);
        else
            {
            memcpy (((*Itr).second).pData + (HTagInfo::sGetDataLen(HTagInfo::DOUBLE) *
                                             pi_Index / sizeof(char)),
                    pi_pVal, (Count * sizeof(char)));
            }
        }
    else
        InsertNewKey(pi_Key, HTagInfo::ASCII, (Byte*)pi_pVal, pi_Index, Count);

    // If Key changed, set indicator.
    if (Ret)
        m_KeyIsDirty = true;
    return Ret;
    }





// ------------------------------------------------ Privates

void HTIFFGeoKey::InsertNewKey(GeoKeyID pi_Key, HTagInfo::DataType pi_Type, const Byte* pi_pVal, uint32_t pi_Index, uint32_t pi_Count)
    {
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(pi_Index == 0);   // I suppose must be set 0 ???

    bool Status = true;

    // validation....
    switch(pi_Type)
        {
        case HTagInfo::ASCII:
        case HTagInfo::SHORT:
        case HTagInfo::DOUBLE:
            break;

        default:
            HASSERT(0);
            break;
        };

    GeoKeyInfo  KeyData;

    KeyData.Count   = (unsigned short)pi_Count;
    KeyData.Type    = pi_Type;

    if (KeyData.Count == 1 && KeyData.Type != HTagInfo::ASCII)
        {
        switch(pi_Type)
            {
            case HTagInfo::SHORT:
                KeyData.DataShort = *((unsigned short*)pi_pVal);
                break;

            case HTagInfo::DOUBLE:
                KeyData.DataDouble = *((double*)pi_pVal);
                break;
            };
        }
    else if ((KeyData.Count == 2) && (KeyData.Type == HTagInfo::ASCII))
        {
        KeyData.DataShort = *((unsigned short*)pi_pVal);
        }
    else
        {
        size_t DataSize = KeyData.Count * HTagInfo::sGetDataLen(KeyData.Type);

        KeyData.pData = new Byte[DataSize];
        memcpy (KeyData.pData, pi_pVal, DataSize);

        if (KeyData.Type == HTagInfo::ASCII)
            {
            if (DataSize > 1)
                KeyData.pData[DataSize-1] = 0;
            else
                Status = false;
            }
        }

    // Insert entry in the map
    if (Status)
        m_KeyList.insert(GeoKeyList::value_type(pi_Key, KeyData));
    }
