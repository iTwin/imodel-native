/*--------------------------------------------------------------------------------------+
|
|     $Source: formats/bcdtmLidar.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TerrainModel/Formats/Formats.h"
#include "TerrainModel/TerrainModel.h"
#include "TerrainModel/Core/dtmdefs.h"
#include "TerrainModel/Core/dtm2dfns.h"
#include "TerrainModel/Formats/LidarImporter.h"
#include <TerrainModel/Core/bcdtmInlines.h>
#include <GeoCoord\IGeoTiffKeysList.h>
#include <Bentley/BeFile.h>

#define LAS_POINT_DATA_RECORD_FORMAT0 20
#define LAS_PUBLIC_HEADER_BLOCK       227
#define LAS_NUM_FEATURES 256

USING_NAMESPACE_BENTLEY_TERRAINMODEL

#pragma pack(push, 1)

struct BCCIVIL_LASPublicHeaderBlock
    {
    unsigned char FileSignature[4];
    unsigned short FileSourceID;
    unsigned short Reserved;
    unsigned long ProjectIDGUIDdata1;
    unsigned short ProjectIDGUIDdata2;
    unsigned short ProjectIDGUIDdata3;
    unsigned char ProjectIDGUIDdata4[8];
    unsigned char VersionMajor;
    unsigned char VersionMinor;
    unsigned char SystemIdentifier[32];
    unsigned char GeneratingSoftware[32];
    unsigned short FileCreationDayOfYear;
    unsigned short FileCreationYear;
    unsigned short HeaderSize;
    unsigned long OffsetToPointData;
    unsigned long NumberOfVariableLengthRecords;
    unsigned char PointDataFormatID;
    unsigned short PointDataRecordLength;
    unsigned long LegacyNumberOfPointRecords;
    unsigned long LegacyNumberOfPointsByReturn[5];
    double XScaleFactor;
    double YScaleFactor;
    double ZScaleFactor;
    double XOffset;
    double YOffset;
    double ZOffset;
    double MaxX;
    double MinX;
    double MaxY;
    double MinY;
    double MaxZ;
    double MinZ;
    uint64_t StartOfWaveFormData;
    uint64_t StartOfFirstExtendedVariableLenRec;
    uint32_t NumOfExtendedVariableLenRec;
    uint64_t NumberOfPointRecords;
    uint64_t NumberOfPointsByReturn[15];
    };

enum class ClassificationOld
    {
    Created = 0,
    UnClassified = 1,
    Ground = 2,
    LowVegetation = 3,
    MediumVegetation = 4,
    HighVegetation = 5,
    Building = 6,
    LowPoint = 7,
    ModelKeyPoint = 8,
    Water = 9,
    Overlap = 10,
    Others = 11,
    };

struct LASClassificationOld
    {
#if ! defined (BITFIELDS_REVERSED)
    ClassificationOld ClassificationValue : 5;
    unsigned char Synthetic           : 1;
    unsigned char Keypoint            : 1;
    bool Withheld            : 1;
#else
    unsigned char Withheld            : 1;
    unsigned char Keypoint            : 1;
    unsigned char Synthetic           : 1;
    unsigned char ClassificationValue : 5;
#endif

    };

struct LASClassificationFlags
    {
    bool Withheld : 1;
    unsigned char Keypoint : 1;
    unsigned char Synthetic : 1;
    unsigned char Overrlap : 1;
    unsigned char ScannerChannel : 2;
    unsigned char ScannerDirection : 1;
    unsigned char EdgeOfFlight : 1;
    };

struct BCCIVIL_LASPointDataRecord
    {
    long x;
    long y;
    long z;
    unsigned short Intensity;
    union
        {
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            } Format0;
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            double GPSTime;
            } Format1;
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            } Format2;
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            } Format3;
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned char WavePacketDescriptorIndex;
            uint64_t BytOffsetToWaveformData;
            uint32_t WaveFormPackedSize;
            float ReturnPointWaveFormLocation;
            float Xt;
            float Yt;
            float Zt;
            } Format4;
        struct
            {
            unsigned char misc;
            LASClassificationOld Classification;
            unsigned char ScanAngleRank;
            unsigned char UserData;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            unsigned char WavePacketDescriptorIndex;
            uint64_t BytOffsetToWaveformData;
            uint32_t WaveFormPackedSize;
            float ReturnPointWaveFormLocation;
            float Xt;
            float Yt;
            float Zt;
            } Format5;
        struct
            {
            unsigned char misc;
            LASClassificationFlags ClassificationFlags;
            LidarImporter::Classification Classification;
            unsigned char UserData;
            short ScanAngle;
            unsigned short PointSourceID;
            double GPSTime;
            } Format6;
        struct
            {
            unsigned char misc;
            LASClassificationFlags ClassificationFlags;
            LidarImporter::Classification Classification;
            unsigned char UserData;
            short ScanAngle;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            } Format7;
        struct
            {
            unsigned char misc;
            LASClassificationFlags ClassificationFlags;
            LidarImporter::Classification Classification;
            unsigned char UserData;
            short ScanAngle;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            unsigned short NIR;
            } Format8;
        struct
            {
            unsigned char misc;
            LASClassificationFlags ClassificationFlags;
            LidarImporter::Classification Classification;
            unsigned char UserData;
            short ScanAngle;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned char WavePacketDescriptorIndex;
            uint64_t BytOffsetToWaveformData;
            uint32_t WaveFormPackedSize;
            float ReturnPointWaveFormLocation;
            float Xt;
            float Yt;
            float Zt;
            } Format9;
        struct
            {
            unsigned char misc;
            LASClassificationFlags ClassificationFlags;
            LidarImporter::Classification Classification;
            unsigned char UserData;
            short ScanAngle;
            unsigned short PointSourceID;
            double GPSTime;
            unsigned short Red;
            unsigned short Green;
            unsigned short Blue;
            unsigned short NIR;
            unsigned char WavePacketDescriptorIndex;
            uint64_t BytOffsetToWaveformData;
            uint32_t WaveFormPackedSize;
            float ReturnPointWaveFormLocation;
            float Xt;
            float Yt;
            float Zt;
            } Format10;
        };
    };


#pragma pack (pop)

static StatusInt Read (BeFile& file, void* buffer, size_t size)
    {
    uint32_t bytesRead = 0;
    if (file.Read (buffer, &bytesRead, (uint32_t)size) != BeFileStatus::Success || bytesRead != size)
        return DTM_ERROR;
    return DTM_SUCCESS;
    }
DTMStatusInt bcdtmFormatLidar_readHeader (BeFile& lasFile, BCCIVIL_LASPublicHeaderBlock& pPHB)
    {
    memset (&pPHB, 0, sizeof(struct BCCIVIL_LASPublicHeaderBlock));

    if (Read (lasFile, &pPHB, LAS_PUBLIC_HEADER_BLOCK))
        return DTM_ERROR;

    if (LAS_PUBLIC_HEADER_BLOCK != pPHB.HeaderSize)
        {
        char* header = (char*)&pPHB;
        if (Read (lasFile, &header[LAS_PUBLIC_HEADER_BLOCK], pPHB.HeaderSize - LAS_PUBLIC_HEADER_BLOCK))
            return DTM_ERROR;
        }

    if (pPHB.VersionMajor == 1 && pPHB.VersionMinor < 4)
        pPHB.NumberOfPointRecords = pPHB.LegacyNumberOfPointRecords;

    /*
    ** Check For LAS File Signature
    */
    if (memcmp ("LASF", pPHB.FileSignature, 4))
        {
        bcdtmWrite_message (1, 0, 0, "Not A LAS File");
        return DTM_ERROR;
        }
    return DTM_SUCCESS;
    }

struct LasVLRHeader
    {
    uint16_t Reserved;
    char UserID[16];
    uint16_t RecordID;
    uint16_t RecordLengthAfterHeader;
    char Description[32];
    };

struct LasVLR
    {
    LasVLRHeader header;
    char* data;

    LasVLR ()
        {
        data = nullptr;
        }
    ~LasVLR ()
        {
        if (data)
            free (data);
        }

    };

struct GeoTiffKeysList : IGeoTiffKeysList
    {
    struct sGeoKeys
        {
        unsigned short wKeyDirectoryVersion;
        unsigned short wKeyRevision;
        unsigned short wMinorRevision;
        unsigned short wNumberOfKeys;
        struct sKeyEntry
            {
            unsigned short wKeyID;
            unsigned short wTIFFTagLocation;
            unsigned short wCount;
            unsigned short wValue_Offset;
            } pKey[1];
        };

    GeoTiffKeysList (void* geoKeysData, void* geoDoubleParams, void* geoAsciiParams)
        {
        char* asciiParams = (char*)geoAsciiParams;
        double* doubleParams = (double*)geoDoubleParams;
        sGeoKeys* keys = (sGeoKeys*)geoKeysData;

        for (int i = 0; i < keys->wNumberOfKeys; i++)
            {
            IGeoTiffKeysList::GeoKeyItem item;
            item.KeyID = keys->pKey[i].wKeyID;
            if (keys->pKey[i].wTIFFTagLocation == 0)
                {
                item.KeyDataType = LONG;
                item.KeyValue.LongVal = keys->pKey[i].wValue_Offset;
                }
            else if (keys->pKey[i].wTIFFTagLocation == 34736)
                {
                item.KeyDataType = DOUBLE;
                item.KeyValue.DoubleVal = doubleParams[keys->pKey[i].wValue_Offset];
                }
            else if (keys->pKey[i].wTIFFTagLocation == 34737)
                {
                item.KeyDataType = ASCII;
                char* str = new char[keys->pKey[i].wCount + 1];
                memcpy (str, &asciiParams[keys->pKey[i].wValue_Offset], keys->pKey[i].wCount);
                str[keys->pKey[i].wCount] = 0;
                item.KeyValue.StringVal = str;
                }
            m_keys.push_back (item);
            }
        m_index = 0;
        }

    bvector<IGeoTiffKeysList::GeoKeyItem> m_keys;
    mutable int m_index;
    virtual bool            GetFirstKey (GeoKeyItem* po_Key) const
        {
        m_index = 0;
        return GetNextKey (po_Key);
        }
    virtual bool            GetNextKey (GeoKeyItem* po_Key) const
        {
        if (m_index < (int)m_keys.size())
            {
            *po_Key = m_keys[m_index++];
            return true;
            }
        return false;
        }

    virtual void            AddKey (unsigned short pi_KeyID, uint32_t pi_value) {}
    virtual void            AddKey (unsigned short pi_KeyID, double pi_value) {}
    virtual void            AddKey (unsigned short pi_KeyID, const std::string& pi_value) {}
    };

DTMStatusInt bcdtmFormatLidar_getCoordinateSystem (BeFile& lasFile, BCCIVIL_LASPublicHeaderBlock& pPHB, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr& gcs)
    {
    LasVLR* vlrs = new LasVLR[pPHB.NumberOfVariableLengthRecords];

    lasFile.SetPointer (pPHB.HeaderSize, BeFileSeekOrigin::Begin);
    for (int i = 0; i < (int)pPHB.NumberOfVariableLengthRecords; i++)
        {
        if (Read (lasFile, &vlrs[i].header, sizeof (vlrs[i].header)) != DTM_SUCCESS)
            return DTM_ERROR;
        vlrs[i].data = (char*)malloc (vlrs[i].header.RecordLengthAfterHeader);
        Read (lasFile, vlrs[i].data, vlrs[i].header.RecordLengthAfterHeader);
        }

    int geoKeyRecordIndex = -1;
    int geoDoubleParamsTagIndex = -1;
    int geoAsciiParamsTagIndex = -1;

    int wktMathTransformIndex = -1;
    int wktCoordinateSystemIndex = -1;
    for (int i = 0; i < (int)pPHB.NumberOfVariableLengthRecords; i++)
        {
        if (strcmp ("LASF_Projection", vlrs[i].header.UserID) == 0)
            {
            if (vlrs[i].header.RecordID == 2111)
                {
                wktMathTransformIndex = i;
                }
            else if (vlrs[i].header.RecordID == 2111)
                {
                wktCoordinateSystemIndex = i;
                }
            else if (vlrs[i].header.RecordID == 34735)
                {
                geoKeyRecordIndex = i;
                }
            else if (vlrs[i].header.RecordID == 34736)
                {
                geoDoubleParamsTagIndex = i;
                }
            else if (vlrs[i].header.RecordID == 34737)
                {
                geoAsciiParamsTagIndex = i;
                }
            }
        }

    if (wktMathTransformIndex != -1)
        {
        gcs = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS ();
        WString msg;
        StatusInt warning;
        // Not sure what is in here.
        WString wtk(vlrs[wktMathTransformIndex].data, true);
        if (gcs->InitFromWellKnownText (&warning, &msg, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown, wtk.GetWCharCP ()) != SUCCESS)
            {
            WString wtk2 (vlrs[wktCoordinateSystemIndex].data, true);
            if (gcs->InitFromWellKnownText (&warning, &msg, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown, wtk.GetWCharCP ()) != SUCCESS)
                gcs = nullptr;
            }
        }
    else if (geoKeyRecordIndex != -1)
        {
        gcs = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS ();
        GeoTiffKeysList geoTiffKeys (vlrs[geoKeyRecordIndex].data, geoDoubleParamsTagIndex != -1 ? vlrs[geoDoubleParamsTagIndex].data : nullptr, geoAsciiParamsTagIndex != -1 ? vlrs[geoAsciiParamsTagIndex].data : nullptr);
        WString msg;
        StatusInt warning;
        if (gcs->InitFromGeoTiffKeys (&warning, &msg, &geoTiffKeys) != SUCCESS)
            gcs = nullptr;
        }

    if (vlrs != nullptr)
        delete[] vlrs;
    return DTM_SUCCESS;
    }

static LidarImporter::Classification ConvertOldClassification (ClassificationOld oldClassification)
    {
    if (oldClassification == ClassificationOld::Overlap || oldClassification == ClassificationOld::Others)
        return LidarImporter::Classification::Created;
    return (LidarImporter::Classification)oldClassification;
    }
/*-------------------------------------------------------------------+
|                                                                    |
| Greg.Ashe@Bentley.com                                              |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatLidar_importLasFileFeaturesDtmObject
    (
    BC_DTM_OBJ *dtmP,                  // Pointer To DTM Object
    WCharCP lasFileNameP,             // LAS File Name
    const bvector<LidarImporter::Classification>* importFeatures,
    uint64_t& totalNumLidarPoints,     // Number Of Lidar Points Imported For Each Feature
    bvector<long>* numLidarPoints              // Number Of Lidar Points Imported For Each Feature
    )
    {
    DTMStatusInt ret=DTM_SUCCESS;
    int dbg=0 ;
    int    n;
    BeFile lasFile;
    DTMUserTag  userTag=DTM_NULL_USER_TAG ;
    DTMFeatureId featureId=DTM_NULL_FEATURE_ID  ;
    BCCIVIL_LASPublicHeaderBlock pPHB;
    BCCIVIL_LASPointDataRecord format;
    uint64_t ind;
    int numToRead;
    bool extractFeature [LAS_NUM_FEATURES];
    long numberPointsInClassification [LAS_NUM_FEATURES];
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0,0,0,"Importing LAS File To DTM") ;
        bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"lasFileNameP = %s",lasFileNameP) ;
        if (importFeatures)
            {
            for (n = 0; n < (int)importFeatures->size (); n++)
                {
                bcdtmWrite_message (0, 0, 0, "Extracting Las Feature[%2ld] = %2ld", n, (*importFeatures)[n]);
                }
            }
        }
    /*
    ** Initialise
    */
    totalNumLidarPoints = 0;
    for (n = 0; n < LAS_NUM_FEATURES; ++n)
        {
        numberPointsInClassification[n] = 0;
        extractFeature[n] = (importFeatures == nullptr);
        }

    if (importFeatures)
        {
        for (n = 0; n < (int)importFeatures->size (); n++)
            extractFeature[(int)(*importFeatures)[n]] = true;
        }
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
    /*
    ** Open The Files
    */
    if (lasFile.Open (lasFileNameP, BeFileAccess::Read) != BeFileStatus::Success)
        {
        bcdtmWrite_message(1,0,0,"Cannot Open LAS File %s",lasFileNameP) ;
        goto errexit ;
        }
    /*
    ** Read Header
    */
    if (bcdtmFormatLidar_readHeader (lasFile, pPHB)) goto errexit;

    /*
    ** Read Items
    */
    lasFile.SetPointer (pPHB.OffsetToPointData, BeFileSeekOrigin::Begin);
    numToRead = pPHB.PointDataRecordLength;
    if( dbg ) bcdtmWrite_message(0,0,0,"numToRead = %8ld",numToRead) ;

    memset (&format, 0, numToRead + 1);
    /*
     ** Read Point Records
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "numberOfPointRecords = %8ld", pPHB.NumberOfPointRecords);
    for ( ind = 0; ind < pPHB.NumberOfPointRecords &&  ret == DTM_SUCCESS; ind++ )
        {
        if ( Read( lasFile, &format, numToRead))
            break;

        bool isWithheld = false;
        LidarImporter::Classification classification;
        if (pPHB.PointDataFormatID < 6)
            {
            isWithheld = format.Format0.Classification.Withheld;
            classification = ConvertOldClassification(format.Format0.Classification.ClassificationValue);
            }
        else
            {
            isWithheld = format.Format6.ClassificationFlags.Withheld;
            classification = format.Format6.Classification;
            }

        if (!isWithheld && extractFeature[(int)classification])
            {
            DPoint3d pt;
            pt.x = (double)format.x * pPHB.XScaleFactor + pPHB.XOffset;
            pt.y = (double)format.y * pPHB.YScaleFactor + pPHB.YOffset;
            pt.z = (double)format.z * pPHB.ZScaleFactor + pPHB.ZOffset;

            //		  Add Point to DTM
            if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,userTag,1,&featureId,&pt,1)) goto errexit ;
            totalNumLidarPoints++;

            numberPointsInClassification[(int)classification]++;
            }
        }

    if (importFeatures && numLidarPoints)
        {
        for (n = 0; n < (int)importFeatures->size (); n++)
            (*numLidarPoints)[n] = numberPointsInClassification[(int)(*importFeatures)[n]];
        }
    /*
    ** Clean Up
    */
cleanup :
    lasFile.Close ();

    /*
    **  Update Modified Time
    */
    bcdtmObject_updateLastModifiedTime (dtmP) ;
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing LAS File To DTM Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing LAS File To DTM Error") ;
return(ret);
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatLidar_getLasFileFeatureTypes (WCharCP lasFileNameP, bvector<LidarImporter::ClassificationInfo>& classificationInfo)
    {
    DTMStatusInt ret=DTM_SUCCESS;
    int dbg=0 ;
    int    n;
    BeFile lasFile;
    BCCIVIL_LASPointDataRecord format;
    unsigned long ind;
    int numToRead;
    BCCIVIL_LASPublicHeaderBlock pPHB;
    long numberPointsInClassification[LAS_NUM_FEATURES];
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0,0,0,"Getting LAS File Feature Types") ;
        bcdtmWrite_message(0,0,0,"lasFileNameP = %s",lasFileNameP) ;
        }
    /*
    ** Initialise
    */
    for (n = 0; n < LAS_NUM_FEATURES; ++n)
        numberPointsInClassification[n] = 0;
    /*
    ** Open The Files
    */
    if (lasFile.Open (lasFileNameP, BeFileAccess::Read) != BeFileStatus::Success)
        {
        bcdtmWrite_message (1, 0, 0, "Cannot Open LAS File %s", lasFileNameP);
        goto errexit;
        }
    /*
    ** Read Header
    */
    if (bcdtmFormatLidar_readHeader (lasFile, pPHB)) goto errexit;

    /*
    ** Read Items
    */
    lasFile.SetPointer (pPHB.OffsetToPointData, BeFileSeekOrigin::Begin );

    numToRead = pPHB.PointDataRecordLength;
    if( dbg ) bcdtmWrite_message(0,0,0,"numToRead = %8ld",numToRead) ;
    /*
** Read Point Records
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numberOfPointRecords = %8ld",pPHB.NumberOfPointRecords) ;
    for ( ind = 0; ind < pPHB.NumberOfPointRecords &&  ret == DTM_SUCCESS; ind++ )
        {
        if (Read (lasFile, &format, numToRead))
            break;

        bool isWithheld = false;
        LidarImporter::Classification classification;
        if (pPHB.PointDataFormatID < 6)
            {
            isWithheld = format.Format0.Classification.Withheld;
            classification = ConvertOldClassification (format.Format0.Classification.ClassificationValue);
            }
        else
            {
            isWithheld = format.Format6.ClassificationFlags.Withheld;
            classification = format.Format6.Classification;
            }

        if (!isWithheld)
            numberPointsInClassification[(int)classification]++;
        }

    for (n = 0; n < LAS_NUM_FEATURES; n++)
        {
        if (numberPointsInClassification[n])
            {
            LidarImporter::ClassificationInfo info;
            info.classification = (LidarImporter::Classification)n;
            info.count= numberPointsInClassification[n];
            classificationInfo.push_back (info);
            }
        }

    /*
    ** Clean Up
    */
cleanup :
    lasFile.Close();
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting LAS File Feature Types Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting LAS File Feature Types Error") ;
return(ret);
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup;
    }

    /*-------------------------------------------------------------------+
    |                                                                    |
    |                                                                    |
    |                                                                    |
    +-------------------------------------------------------------------*/
    BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatLidar_getGCS (WCharCP lasFileNameP, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr& gcs)
        {
        DTMStatusInt ret = DTM_SUCCESS;
        int dbg = 0;
        BeFile lasFile;
        BCCIVIL_LASPublicHeaderBlock pPHB;

        /*
        ** Write Entry Message
        */
        if (dbg)
            {
            bcdtmWrite_message (0, 0, 0, "Getting LAS GCS");
            bcdtmWrite_message (0, 0, 0, "lasFileNameP = %s", lasFileNameP);
            }
        /*
        ** Open The Files
        */
        if (lasFile.Open (lasFileNameP, BeFileAccess::Read) != BeFileStatus::Success)
            {
            bcdtmWrite_message (1, 0, 0, "Cannot Open LAS File %s", lasFileNameP);
            goto errexit;
            }
        /*
        ** Read Header
        */
        if (bcdtmFormatLidar_readHeader (lasFile, pPHB)) goto errexit;

        if (bcdtmFormatLidar_getCoordinateSystem (lasFile, pPHB, gcs)) goto errexit;

        /*
        ** Clean Up
        */
    cleanup:
        lasFile.Close ();
        /*
        ** Job Completed
        */
        if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Getting LAS File Feature Types Completed");
        if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Getting LAS File Feature Types Error");
        return(ret);
        /*
        ** Error Exit
        */
    errexit:
        if (ret == DTM_SUCCESS) ret = DTM_ERROR;
        goto cleanup;
        }
