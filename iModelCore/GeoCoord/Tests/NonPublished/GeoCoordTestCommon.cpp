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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetRepresentativeMiniListOfGCS()
    {
    GeoCoordTestCommon::Initialize();

    static bvector<Utf8String> s_representativeListOfGCS;

    if(s_representativeListOfGCS.empty())
        {

        const bvector<Utf8String>& listOfGCS = GeoCoordTestCommon::GetListOfGCS();
    
        const bvector<Utf8String>& listOfRepresentativeDatums = GeoCoordTestCommon::GetListOfRepresentativeDatums();
        
        bvector<Utf8String> listOfDatumsFound;
        
        bvector<GeoCoordinates::BaseGCS::ProjectionCodeValue> listOfProjectionMethod;
    
        bvector<Utf8String> listOfUnitsFound;
        
        for (auto GCSName : listOfGCS)
            {
            GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS(GCSName.c_str());
    
            if (currentGCS.IsValid() && currentGCS->IsValid())
                {
                Utf8String datumName = currentGCS->GetDatumName();
                
                // If it is not a datum we already have and the datum is part of the representative datum list ... we add
                if (find(listOfDatumsFound.begin(), listOfDatumsFound.end(), datumName) == listOfDatumsFound.end() && 
                    find(listOfRepresentativeDatums.begin(), listOfRepresentativeDatums.end(), datumName) != listOfRepresentativeDatums.end())
                    {
                    listOfDatumsFound.push_back(datumName);
                    s_representativeListOfGCS.push_back(GCSName);
                    }
                else if (find(listOfProjectionMethod.begin(), listOfProjectionMethod.end(), currentGCS->GetProjectionCode()) == listOfProjectionMethod.end())
                    {
                    // Not a projection we have ...
                    listOfProjectionMethod.push_back(currentGCS->GetProjectionCode());
                    s_representativeListOfGCS.push_back(GCSName);
                    }
                else
                    {
                    // Not a unit we have
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

static bvector<Utf8String> s_listOfRepresentativeDatums = {
"WGS84",
"ADINDAN",
"AINELABD",
"Ascension58",
"ADOS714",
"AZORES-G",
"ASTRLA66-Tasmania",
"AGD84",
"NZGD2000",
"NZGD49",
"MGI-1901",
"NEWISR-7P",
"PULKOVO-KZCSP-7P",
"JPNGSI-Grid",
"NTF-G-Grid",
"ED50-DK34",
"Accra",
"GBK19-IRF",
"HeathrowT5",
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
"CH1903/GSB",
"CH1903Plus_1",
"CH1903Plus_2",
"CHTRF95",
"GDA2020",
"GDA94",
"GDA94/GSB",
"Hartebeesthoek1994",
"ITRF2005-Macau",
"NTF",
"NTF-G-Grid",
"NTF-G-Grid-ClrkIGN",
"NZGD2000",
"NZGD49",
"RGF93",
"SIRGAS2000",
"Slov/JTSK-NULL",
"Slov/JTSK03",
"Karbala79/P",
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& GeoCoordTestCommon::GetListOfRepresentativeDatums()
    {
    return s_listOfRepresentativeDatums;
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
"Hartebeesthoek1994",
"ITRF2005-Macau",
"NTF",
"EPSG:6272",
"NZGD2000",
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
