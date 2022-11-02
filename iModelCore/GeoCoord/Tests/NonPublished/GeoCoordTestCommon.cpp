//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <Bentley/BeTextFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <BeSQLite/BeSQLite.h>
#include "GeoCoordTestCommon.h"

static Utf8String s_initializedPath;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeoCoordTestCommon::Initialize(bool doNotUseAllEarth)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLite::BeSQLiteLib::Initialize(tempDir);
    
    if (!GeoCoordinates::BaseGCS::IsLibraryInitialized())
        {
        BeFileName moduleFileName = Desktop::FileSystem::GetExecutableDir();

        BeFileName path(BeFileName::GetDirectoryName(moduleFileName).c_str());

        path.AppendToPath(L"Assets\\DgnGeoCoord");

        GeoCoordinates::BaseGCS::Initialize(path.GetNameUtf8().c_str());

        s_initializedPath = path.GetNameUtf8();
        
        BeFileName dbName(path);
            
        if (doNotUseAllEarth)
            {
            bvector<Utf8String> listOfWS = {
              "usa",
              "brazil",
              "australia",
              "venezuela",
              "france",
              "germany",
              "japan",
              "newzealand",
              "portugal",
              "slovakia",
              "spain",
              "switzerland",
              "ostn",
              "uk",
            };

            for (auto item: listOfWS) 
                {
                BeFileName theWorkspacePath = path;
                theWorkspacePath.AppendToPath((WString(item.c_str(), BentleyCharEncoding::Utf8) + L".itwin-workspace").c_str());
                GeoCoordinates::BaseGCS::AddWorkspaceDb(theWorkspacePath.GetNameUtf8().c_str(), nullptr, 1);
                }
            }
        else
            {
            BeFileName dbName(path);
            dbName.AppendToPath(L"allEarth.itwin-workspace");
            GeoCoordinates::BaseGCS::AddWorkspaceDb(dbName.GetNameUtf8(), nullptr, 0);
            }
        }

    return GeoCoordinates::BaseGCS::IsLibraryInitialized();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeoCoordTestCommon::Shutdown()
    {
    if (GeoCoordinates::BaseGCS::IsLibraryInitialized())
        GeoCoordinates::BaseGCS::Shutdown();
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const Utf8String& GeoCoordTestCommon::InitializedLibraryPath()
    {
    GeoCoordTestCommon::Initialize();
    return s_initializedPath;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetListOfGCS()
    {
    GeoCoordTestCommon::Initialize();

    static bvector<Utf8String> s_listOfGCS;

    if(s_listOfGCS.empty())
        {
        char        csKeyName[128];

        for (int index = 0; (0 < GeoCoordinates::CSMap::CS_csEnum(index, csKeyName, sizeof(csKeyName))); index++)
            {
            s_listOfGCS.push_back(Utf8String(csKeyName));
            }
        }
    return s_listOfGCS;
    };

static bvector<Utf8String> s_smallListOfGCS = {
"UTM84-12N",
"BritishNatGrid",
"CA-II",
"CA83-II",
"ETRF89.ArticZn4-26",
"PortageWISCRS-A-M",
"ICS83-Metropolis",
"AZ83/2011-WIF"
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetSmallListOfGCS()
    {
    return s_smallListOfGCS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetRepresentativeListOfGCS()
    {

    static bvector<Utf8String> s_representativeListOfGCS;

    if(s_representativeListOfGCS.empty())
        {

        const bvector<Utf8String>& listOfGCS = GeoCoordTestCommon::GetListOfGCS();
    
        bvector<Utf8String> listOfDatumsFound;
        
        bvector<GeoCoordinates::BaseGCS::ProjectionCodeValue> listOfProjectionMethod;
    
        bvector<Utf8String> listOfUnitsFound;
        
        for (auto GCSName : listOfGCS)
            {
            GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS(GCSName.c_str());
    
            if (currentGCS.IsValid() && currentGCS->IsValid())
                {
                Utf8String datumName = currentGCS->GetDatumName();
                
                if (find(listOfDatumsFound.begin(), listOfDatumsFound.end(), datumName) == listOfDatumsFound.end())
                    {
                    listOfDatumsFound.push_back(datumName);
                    s_representativeListOfGCS.push_back(GCSName);
                    }
                else if (find(listOfProjectionMethod.begin(), listOfProjectionMethod.end(), currentGCS->GetProjectionCode()) == listOfProjectionMethod.end())
                    {
                    listOfProjectionMethod.push_back(currentGCS->GetProjectionCode());
                    s_representativeListOfGCS.push_back(GCSName);
                    }
                else
                    {
                    Utf8String unitName;
                    currentGCS->GetUnits(unitName);
                    if (find(listOfUnitsFound.begin(), listOfUnitsFound.end(), unitName) == listOfUnitsFound.end())
                        {
                        listOfUnitsFound.push_back(unitName);
                        s_representativeListOfGCS.push_back(GCSName);
                        }
                    }
                }
            }
        }

    return s_representativeListOfGCS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetListOfDatums()
    {
    GeoCoordTestCommon::Initialize();
    static bvector<Utf8String> s_listOfDatums;

    if(s_listOfDatums.empty())
        {
        GeoCoordinates::DatumEnumeratorP enumerator = GeoCoordinates::Datum::CreateEnumerator();

        if (nullptr != enumerator)
            {
            while (enumerator->MoveNext())
                {
                GeoCoordinates::DatumCP theDatum = enumerator->GetCurrent();

                s_listOfDatums.push_back(theDatum->GetName());
                
                theDatum->Destroy();
                }

            enumerator->Destroy();
            }
        }
    return s_listOfDatums;
    };

// The following datums define grid files based transformation but we do not distribute the grid files or are meant to be overridden.
static bvector<Utf8String> s_listOfNotSupportedDatums = {
"GENGRID-WGS84",
"GENGRID-CLRK-ARC",
"GENGRID-CLRK80",
"GENGRID-CLRK66",
"GENGRID-GRS80",
"GENGRID-BESSEL",
"GENGRID-INTNL",
"GENGRID-AIRY30",
"GENGRID-WGS84-1",
"GENGRID-CLRK-ARC-1",
"GENGRID-CLRK80-1",
"GENGRID-CLRK66-1",
"GENGRID-GRS80-1",
"GENGRID-BESSEL-1",
"GENGRID-INTNL-1",
"GENGRID-AIRY30-1",
"GENGRID-WGS84-2",
"GENGRID-CLRK-ARC-2",
"GENGRID-CLRK80-2",
"GENGRID-CLRK66-2",
"GENGRID-GRS80-2",
"GENGRID-BESSEL-2",
"GENGRID-INTNL-2",
"GENGRID-AIRY30-2",
"GENGRID-WGS84-3",
"GENGRID-CLRK-ARC-3",
"GENGRID-CLRK80-3",
"GENGRID-CLRK66-3",
"GENGRID-GRS80-3",
"GENGRID-BESSEL-3",
"GENGRID-INTNL-3",
"GENGRID-AIRY30-3",
"GENGRID-WGS84-4",
"GENGRID-CLRK-ARC-4",
"GENGRID-CLRK80-4",
"GENGRID-CLRK66-4",
"GENGRID-GRS80-4",
"GENGRID-BESSEL-4",
"GENGRID-INTNL-4",
"GENGRID-AIRY30-4",
"ATS77",
"CSRS",
"EPSG:6140",
"EPSG:6608",
"EPSG:6609",
"EPSG:6122",
"NAD27/CGQ77-83",
"NAD27/CGQ77-98",
"NAD27/1976",
"CAPE/GSB",
"AGD66", // TODO The followings Australian datum are not supported on LINUX because the GSB filenames contains the '(' and ')' characters.
"AGD84",
"ASTRLA66-Grid",
"ASTRLA84-Grid",
"EPSG:6202",
"EPSG:6203"
};    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetListOfUnsupportedDatums()
    {
    return s_listOfNotSupportedDatums;
    };

static bvector<Utf8String> s_listOfDatumsWithAdditionalPaths = {
"NAD83/2011",
"NSRS11",
"NSRS07",
"NAD83/HARN-A",
"NAD83/HARN",
"NAD27",
"PuertoRico",
"NSRS07",
"NSRS11",
"HPGN",
"NAD83",
"EPSG:6269",
"CSRS",
"EPSG:6140",
"EPSG:6267",
"AGD66",
"ASTRLA66-Grid",
"AGD84",
"ASTRLA84-Grid",
"EPSG:6202",
"EPSG:6203",
"JGD2011",
"JGD2000",
"JGD2000-7P",
"JPNGSI-Grid",
"EPSG:6612",
"TOKYO",
"Tokyo-Grid",
"ATS77",
"CH1903/GSB",
"CH1903Plus_1",
"CH1903Plus_2",
"CHTRF95",
"EPSG:6139",
"EPSG:6152",
"EPSG:6171",
"EPSG:6275",
"EPSG:6283",
"GDA2020",
"GDA94",
"GDA94/GSB",
"Hartebeesthoek1994",
"ITRF2005-Macau",
"NTF",
"NTF-G-Grid",
"NTF-G-Grid-ClrkIGN",
"EPSG:6272",
"NZGD2000",
"NZGD49",
"RGF93",
"SIRGAS2000",
"Slov/JTSK-NULL",
"Slov/JTSK03",

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetListOfDatumsWithAdditionalPaths()
    {
    return s_listOfDatumsWithAdditionalPaths;

    }
