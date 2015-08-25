// StandAloneTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>

#include "AppHost.h"

#include <ios>
#include <list>

using namespace std;

#include <Bentley\BeFileName.h>
#include <Bentley\WString.h>
#include <Bentley\WString.h>

#include <DgnPlatform\DgnDocumentManager.h>
#include <DgnPlatform\DgnPlatformErrors.r.h>
#include <GeoCoord\BaseGeoCoord.h>

#include <ImagePP\h\ExportMacros.h>
#include <ImagePP\h\HTypes.h>
#include <ImagePP\h\ImagePPClassId.h>
 
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshSources.h>
#include <ScalableMesh\IScalableMeshSourceImporter.h>



#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <TerrainModel\ElementHandler\TMElementDisplayHandler.h>


#undef fclose
#undef fflush
#undef fopen
#undef fwrite

#include <stdio.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

using namespace Bentley::DgnPlatform;
using namespace Bentley::GeoCoordinates;

/***EXAMPLE CALLBACK CODE********/
static FILE*  s_pPointResultFile = 0;
static FILE*  s_pFeatureResultFile = 0;

bool WritePointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
	{
	char coordinateBuffer[300];   
    int  NbChars;        

    for (size_t PointInd = 0; PointInd < nbOfPoints; PointInd++)
        {                
        NbChars = sprintf(coordinateBuffer, 
                          "%.20f,%.20f,%.20f\n", 
                          points[PointInd].x, 
                          points[PointInd].y, 
                          points[PointInd].z);         

        size_t nbCharsWritten = fwrite (coordinateBuffer, 1, NbChars, s_pPointResultFile);
                
        assert(NbChars == nbCharsWritten);    
        }  

    fflush(s_pPointResultFile);

    return true;
	}

bool WriteFeatureCallback(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d)
    {               
    char outputBuffer[300];   
    int  NbChars;        

    string featureName; 
   
    switch ((DTMFeatureType)featureType)
        {        
        case DTMFeatureType::RandomSpots:
            assert(!"Should be handle by WritePointsCallback");
            featureName = "Point";
            break;
        case DTMFeatureType::GroupSpots:
            assert(!"Should be handle by WritePointsCallback");
            featureName = "PointFeature";
            break;        
        case DTMFeatureType::Breakline:
            featureName = "Breakline";
            break;
        case DTMFeatureType::SoftBreakline:
            featureName = "SoftBreakline";
            break;
        case DTMFeatureType::ContourLine:
            featureName = "Contour";
            break;
        case DTMFeatureType::Void:
            featureName = "Void";
            break;
        case DTMFeatureType::BreakVoid:
            featureName = "BreakVoid";
            break;
        case DTMFeatureType::DrapeVoid:
            featureName = "DrapeVoid";
            break;
        case DTMFeatureType::Island:
            featureName = "Island";
            break;
        case DTMFeatureType::Hole:
            featureName = "Hole";
            break;
        case DTMFeatureType::Hull:
            featureName = "Hull";
            break;
        case DTMFeatureType::DrapeHull:
            featureName = "DrapeHull";
            break;
        case DTMFeatureType::HullLine:
            featureName = "HullLine";
            break;
        case DTMFeatureType::VoidLine:
            featureName = "VoidLine";
            break;
        case DTMFeatureType::HoleLine:
            featureName = "HoleLine";
            break;
        case DTMFeatureType::SlopeToe:
            featureName = "SlopeToe";
            break;
        case DTMFeatureType::GraphicBreak:
            featureName = "GraphicBreak";
            break;
        case DTMFeatureType::Region:
            featureName = "Region";
            break;
        default: 
            assert(!"Unknown feature");
            featureName = "Unknown feature";
        }
 
    NbChars = sprintf(outputBuffer, "FEATURE : %s\n", featureName.c_str());         
    
    size_t nbCharsWritten = fwrite (outputBuffer, 1, NbChars, s_pFeatureResultFile);
                
    assert(NbChars == nbCharsWritten);    
    

    for (size_t featurePointInd = 0; featurePointInd < nbOfFeaturesPoints; featurePointInd++)
        {                
        NbChars = sprintf(outputBuffer, 
                          "%.20g,%.20g,%.20g\n", 
                          featurePoints[featurePointInd].x, 
                          featurePoints[featurePointInd].y, 
                          featurePoints[featurePointInd].z);         

        nbCharsWritten = fwrite (outputBuffer, 1, NbChars, s_pFeatureResultFile);
                
        assert(NbChars == nbCharsWritten);    
        }  

    fflush(s_pFeatureResultFile);        
    
    return true;
    }

/***EXAMPLE CALLBACK CODE END********/

static bool s_tryReprojection = false;


StatusInt    mdlLevel_getIdFromName
(
LevelId*        levelIdOut,
DgnModelRefP    modelRefIn,
LevelId         ,
WCharCP         levelNameIn
)
    {
    // Does not look in level libraries!
    if (NULL == modelRefIn)
        return ERROR;

    LevelHandle level = modelRefIn->GetLevelCacheR().GetLevelByName (levelNameIn, false);
    if (!level.IsValid ())
        {
        if (NULL != levelIdOut)
            *levelIdOut = LEVEL_NULL_ID;

        return static_cast<StatusInt>(level.GetStatus());
        }

    if (NULL != levelIdOut)
        *levelIdOut = level.GetLevelId ();

    return SUCCESS;
    }

/*NEEDS_WORK_SM_IMPORTER - Those environements variables need to be set to ensure the program is working.
PATH=%PATH%;$(OutRoot)Winx64\Product\TerrainDataImporterModule\
*/
static ModelId s_modelId = 0;

//Need to be global to avoid crash during global destruction.
AppHost appHost;


int _tmain(int argc, _TCHAR* argv[])
	{  
    appHost.Startup ();            
   
    DgnFileOpenParams fileOpenParams(L"D:\\MyDoc\\RM - SM - Sprint 3\\Acute3d importer\\MyTestDGN3dStandAloneSeed.dgn", false, DgnFilePurpose::MasterFile);

    DgnFilePtr dgnFilePtr(fileOpenParams.CreateFileAndLoad ());

    assert(dgnFilePtr != 0);

    StatusInt errorDetails;

    DgnModelP modelRef = dgnFilePtr->LoadRootModelById (&errorDetails, s_modelId);    

    MrDTMElementDisplayHandler::OnModelRefActivate(*modelRef, 0);
    //DgnModelP modelRef(dgnFilePtr->GetFirstModelRef ());
  	    
    s_pPointResultFile = fopen("D:\\MyDoc\\RM - SM - Sprint 3\\Acute3d importer\\output\\021l14_0200_dem.xyz", "w+");                

    assert(s_pPointResultFile);

    s_pFeatureResultFile = fopen("D:\\MyDoc\\RM - SM - Sprint 3\\Acute3d importer\\output\\021l14_0200_dem.feature", "w+");                

    assert(s_pPointResultFile);
    
    IDTMSourcePtr sourcePtr;
    
    //WString sourcePath(L"D:\\DEM\\USGS DEM ASCII\\16INT00I0\\021l14_0200_dem.dem");
    //WString sourcePath(L"D:\\Dataset\\PointCloud\\POD\\rock-thing_normal_out.pod");
    //WString sourcePath(L"D:\\Temp\\Ian Rosam's Client Dataset\\Ground_DTM\\228000_7412000_2k_1m_DTM.xyz");
    //WString sourcePath(L"D:\\MyDoc\\RM - SM - Sprint 2\\Hybrid Meshing\\1M.xyz");    
    WString sourcePath(L"D:\\MyDoc\\RM - SM - Sprint 3\\Acute3d importer\\MyTestDGN3dLinearFeature.dgn");
        
    ILocalFileMonikerPtr monikerPtr(ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));
            
    DTMSourceDataType importedType = DTM_SOURCE_DATA_DTM;

    if(0 == _wcsicmp(L"dgn", BeFileName::GetExtension(sourcePath.c_str()).c_str()))
        {
        //assert(!"ToDo: Create IDTMDgnLevelSource with proper IDs");
        sourcePtr = IDTMDgnLevelSource::Create(importedType, monikerPtr, 0, L"Default", 4, L"TM").get();
        }
    else
        {
        sourcePtr = IDTMLocalFileSource::Create(importedType, monikerPtr).get();
        }
               
    IScalableMeshSourceImporterPtr sourceImporterPtr(IScalableMeshSourceImporter::Create());        

    sourceImporterPtr->SetFeatureCallback(WriteFeatureCallback);
    sourceImporterPtr->SetPointsCallback(WritePointsCallback);


    if (s_tryReprojection)
        {
        BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS (L"MTM83-7"));
        StatusInt status = sourceImporterPtr->SetBaseGCS(baseGCSPtr);
        assert(status == SUCCESS);
        }

	sourceImporterPtr->EditSources().Add(sourcePtr);

	StatusInt status = sourceImporterPtr->Import();

	assert(status == SUCCESS);

    fclose(s_pPointResultFile);

    dgnFilePtr = 0;

    appHost.Terminate ();       

	return 0;
	}

