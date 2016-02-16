#include "ATPUtils.h"
#include "ATPDefinitions.h"
#include "ATPGeneration.h"
#include "ATPFileFinder.h"
#include <DgnPlatform/DgnFile.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <DgnPlatform\DgnDocumentManager.h>
#include <DgnPlatform\LevelCache.h>
#include <DgnPlatform\DgnPlatformErrors.r.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
//USING_NAMESPACE_SCALABLEMESH

StatusInt    mdlLevel_getIdFromName
(
    LevelId*        levelIdOut,
    DgnModelRefP    modelRefIn,
    LevelId,
    WCharCP         levelNameIn
    )
    {
    // Does not look in level libraries!
    if (NULL == modelRefIn)
        return ERROR;

    LevelHandle level = modelRefIn->GetLevelCacheR().GetLevelByName(levelNameIn, false);
    if (!level.IsValid())
        {
        if (NULL != levelIdOut)
            *levelIdOut = LEVEL_NULL_ID;

        return static_cast<StatusInt>(level.GetStatus());
        }

    if (NULL != levelIdOut)
        *levelIdOut = level.GetLevelId();

    return SUCCESS;
    }

IDTMSourcePtr CreateSourceFor(const WString&          sourcePath,
                                    DTMSourceDataType importedType, 
                                    BeXmlNodeP        pTestChildNode)
    {
    ILocalFileMonikerPtr monikerPtr(ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));
            
    if(0 == _wcsicmp(L"dgn", BeFileName::GetExtension(sourcePath.c_str()).c_str()))
        {
        assert(pTestChildNode != 0);        

        WString model = L"Default";
        WString level = L"Default";

        DgnFileOpenParams fileOpenParams(sourcePath.c_str(), false, DgnFilePurpose::MasterFile);

        DgnFilePtr dgnFilePtr(fileOpenParams.CreateFileAndLoad ());

        assert(dgnFilePtr != 0);

        StatusInt errorDetails;
        //Only supporting DGN with one model with ID 0.
        ModelId modelID = 0;

        DgnModelP modelRef = dgnFilePtr->LoadRootModelById (&errorDetails, modelID);   
                
        assert(modelRef != 0);

        StatusInt status = pTestChildNode->GetAttributeStringValue(model, "model"); 

        assert(status == SUCCESS);        

#ifndef NDEBUG
        WCharCP modelName;// [3000];
        //mdlModelRef_getModelName(modelRef, modelName);
        modelName = modelRef->GetModelName();
        
        assert(0 == wcscmp(modelName, model.c_str()));            
#endif
                           
        status = pTestChildNode->GetAttributeStringValue(level, "level"); 

        assert(status == SUCCESS);

        LevelId levelId;

        status = mdlLevel_getIdFromName(&levelId, modelRef, 0, level.c_str());
        //modelRef->GetLevelID;

        assert(status == SUCCESS);
        
        return IDTMDgnLevelSource::Create(importedType, monikerPtr, modelID, model.c_str(), levelId, level.c_str()).get();
        }
            
    return IDTMLocalFileSource::Create(importedType, monikerPtr).get();
    }

void ParseDTMFeatureType(WString& name, DTMFeatureType& type)
    {
    if (0 == _wcsicmp(L"Void", name.c_str())) type = DTMFeatureType::Void;
    else if (0 == _wcsicmp(L"DrapeVoid", name.c_str())) type = DTMFeatureType::DrapeVoid;
    else if (0 == _wcsicmp(L"Breakline", name.c_str())) type = DTMFeatureType::Breakline;
    else if (0 == _wcsicmp(L"Hole", name.c_str())) type = DTMFeatureType::Hole;
    else if (0 == _wcsicmp(L"Island", name.c_str())) type = DTMFeatureType::Island;
    else if (0 == _wcsicmp(L"ContourLine", name.c_str())) type = DTMFeatureType::ContourLine;
    }

bool AddOptionToSource(IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode)
    {
    WString datasetIs3D;
    WString datasetIsGround;

    StatusInt status = pTestChildNode->GetAttributeStringValue(datasetIs3D, "is3D");

    if (status == BEXML_Success)
        {
        if (datasetIs3D.Equals(L"1"))
            {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetRepresenting3dData(true);

            sourceImportConfig.SetReplacementSMData(data);
            }
        else
            {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetRepresenting3dData(false);

            sourceImportConfig.SetReplacementSMData(data);
            }
        }

    status = pTestChildNode->GetAttributeStringValue(datasetIsGround, "grounddetection");


    if (status == BEXML_Success)
        {
        if (datasetIsGround.Equals(L"1"))
            {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetIsGroundDetection(true);
            data.SetRepresenting3dData(false);

            sourceImportConfig.SetReplacementSMData(data);
            }
        }
    WString propertyName;

    status = pTestChildNode->GetAttributeStringValue(propertyName, "elevationProperty");

    if (status == BEXML_Success)
        {
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetElevationPropertyName(propertyName);

        sourceImportConfig.SetReplacementSMData(data);
        }
    WString importType;
    status = pTestChildNode->GetAttributeStringValue(importType, "polygonImport");

    if (status == BEXML_Success)
        {
        DTMFeatureType type = DTMFeatureType::Breakline;
        ParseDTMFeatureType(importType, type);
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetPolygonFeatureType(type);

        sourceImportConfig.SetReplacementSMData(data);
        }

    WString linearImportType;
    status = pTestChildNode->GetAttributeStringValue(importType, "linearImport");

    if (status == BEXML_Success)
        {
        DTMFeatureType type = DTMFeatureType::ContourLine;
        ParseDTMFeatureType(linearImportType, type);
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetLinearFeatureType(type);

        sourceImportConfig.SetReplacementSMData(data);
        }

    return true;
    }

void GetSourceDataType(DTMSourceDataType& dataType, BeXmlNodeP pSourceNode)
    {
    WString dataTypeStr;

    StatusInt status = pSourceNode->GetAttributeStringValue(dataTypeStr, "dataType");

    if (status == BEXML_Success)
        {
        if (dataTypeStr.CompareTo(L"POINT") == 0)
            {
            dataType = DTM_SOURCE_DATA_POINT;
            }
        else
            if (dataTypeStr.CompareTo(L"DTM") == 0)
                {
                dataType = DTM_SOURCE_DATA_DTM;
                }
            else
                if (dataTypeStr.CompareTo(L"BREAKLINE") == 0)
                    {
                    dataType = DTM_SOURCE_DATA_BREAKLINE;
                    }
                else
                    if (dataTypeStr.CompareTo(L"RASTER") == 0)
                        {
                        dataType = DTM_SOURCE_DATA_IMAGE;
                        }
                    else
                        if (dataTypeStr.CompareTo(L"CLIP") == 0)
                            {
                            dataType = DTM_SOURCE_DATA_CLIP;
                            }
                        else
                            {
                            printf("Unsupporter/unknown data type");
                            }
        }
    }

enum
    {
    ACCELERATOR_CPU = 0,
    ACCELERATOR_GPU
    };

bool ParseGenerationOptions(ScalableMeshMesherType* mesherType, ScalableMeshFilterType* filterType, int* trimmingMethod, ScalableMeshSaveType* saveType, BeXmlNodeP pTestNode)
    {
    bool isSuccess = true;

    BeXmlNodeP pTestChildNode = pTestNode->GetFirstChild();

    while ((0 != pTestChildNode) && (isSuccess == true))
        {
        if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "mesher"))
            {
            if (mesherType != 0)
                {
                int32_t mesherTypeAttr;

                StatusInt status = pTestChildNode->GetAttributeInt32Value(mesherTypeAttr, "type");

                if ((status == BEXML_Success) && (mesherTypeAttr >= 0) && (mesherTypeAttr < SCM_MESHER_QTY))
                    {
                    *mesherType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesherType)mesherTypeAttr;
                    }
                else
                    {
                    printf("ERROR : invalid type attribute for mesher node\r\n");
                    }

                if (trimmingMethod != 0)
                    {
                    WString getTrimmingMethod;
                    status = pTestChildNode->GetAttributeStringValue(getTrimmingMethod, "trimmingMethod");

                    if (status == BEXML_Success)
                        {
                        if (0 == BeStringUtilities::Wcsicmp(getTrimmingMethod.c_str(), L"method5")) {
                            *trimmingMethod = 5;
                            }
                        else if (0 == BeStringUtilities::Wcsicmp(getTrimmingMethod.c_str(), L"umbrella")) {
                            *trimmingMethod = 7;
                            }
                        }
                    }
                }
            else
                {
                printf("ERROR : unexpected mesher node\r\n");
                }

            }
        else
            if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "filter"))
                {
                if (filterType != 0)
                    {
                    int32_t filterTypeAttr;

                    StatusInt status = pTestChildNode->GetAttributeInt32Value(filterTypeAttr, "type");

                    if ((status == BEXML_Success) && (filterTypeAttr >= 0) && (filterTypeAttr < SCM_FILTER_QTY))
                        {
                        *filterType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshFilterType)filterTypeAttr;
                        }
                    else
                        {
                        printf("ERROR : invalid type attribute for filter node\r\n");
                        }
                    }
                else
                    {
                    printf("ERROR : unexpected filter node\r\n");
                    }
                }
            else
                if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "save"))
                    {
                    if (saveType != 0)
                        {
                        int32_t saveTypeAttr;

                        StatusInt status = pTestChildNode->GetAttributeInt32Value(saveTypeAttr, "type");

                        if ((status == BEXML_Success) && (saveTypeAttr >= 0) && (saveTypeAttr < SCM_SAVE_QTY))
                            {
                            *saveType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshSaveType)saveTypeAttr;
                            }
                        else
                            {
                            printf("ERROR : invalid type attribute for save node\r\n");
                            }
                        }
                    else
                        {
                        printf("ERROR : unexpected save node\r\n");
                        }
                    }
        pTestChildNode = pTestChildNode->GetNextSibling();
        }
    return isSuccess;
    }

bool ParseSourceSubNodes(IDTMSourceCollection& sourceCollection, BeXmlNodeP pTestNode)
    {
    bool isSuccess = true;

    BeXmlNodeP pTestChildNode = pTestNode->GetFirstChild();

    while ((0 != pTestChildNode) && (isSuccess == true))
        {
        if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "source"))
            {
            WString datasetPath;
            WString datasetIs3D;
            WString datasetIsGround;

            StatusInt status = pTestChildNode->GetAttributeStringValue(datasetPath, "path");

            if (status == BEXML_Success)
                {
                DTMSourceDataType dataType = DTM_SOURCE_DATA_POINT;

                GetSourceDataType(dataType, pTestChildNode);

                if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                    (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                    {
                    IDTMSourcePtr srcPtr = CreateSourceFor(datasetPath, dataType, pTestChildNode);
                    AddOptionToSource(srcPtr, pTestChildNode);
                    if (BSISUCCESS != sourceCollection.Add(srcPtr))
                        {
                        isSuccess = false;
                        wprintf(L"ERROR : cannot add %s\r\n", datasetPath);
                        break;
                        }
                    }
                else
                    {
                    ATPFileFinder fileFinder;

                    WString filePaths;
                    WString filter;
                    StatusInt status = pTestChildNode->GetAttributeStringValue(filter, "filter");
                    WString ext;
                    status = pTestChildNode->GetAttributeStringValue(ext, "extension");
                    fileFinder.FindFiles(datasetPath, filePaths, true);

                    WString firstPath;

                    while (fileFinder.ParseFilePaths(filePaths, firstPath))
                        {
                        BeFileName name(firstPath.c_str());
                        WString extension;
                        name.ParseName(NULL, NULL, NULL, &extension);
                        if (0 == BeStringUtilities::Wcsicmp(extension.c_str(), L"classif")) continue;
                        if (!ext.empty() && 0 != BeStringUtilities::Wcsicmp(extension.c_str(), ext.c_str())) continue;
                        if (!filter.empty() && !name.ContainsI(filter)) continue;
                        IDTMSourcePtr srcPtr = CreateSourceFor(firstPath, dataType, pTestChildNode);
                        AddOptionToSource(srcPtr, pTestChildNode);
                        if (BSISUCCESS != sourceCollection.Add(srcPtr))
                            {
                            isSuccess = false;
                            wprintf(L"ERROR : cannot add %s\r\n", firstPath);
                            break;
                            }
                        }
                    }
                }
            else
                {
                printf("ERROR : attribute path for mesher node not found\r\n");
                }
            }
        else
            {
            //printf("ERROR : unknown child node for test node\r\n");
            }

        pTestChildNode = pTestChildNode->GetNextSibling();
        }

    return isSuccess;
    }

bool ParseBaselineSubNodes(WString& baselinePointFileName, WString& baselineFeatureFileName, BeXmlNodeP pTestNode)
    {
    bool baselineSubNodeFound = false;

    BeXmlNodeP pTestChildNode = pTestNode->GetFirstChild();                    

    while ((0 != pTestChildNode) && (baselineSubNodeFound == false))
        {
        if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "baseline"))
            {                        
            StatusInt status = pTestChildNode->GetAttributeStringValue(baselinePointFileName, "pointFileName");

            assert(status == BEXML_Success);            

            status = pTestChildNode->GetAttributeStringValue(baselineFeatureFileName, "featureFileName");

            assert(status == BEXML_Success);            

            baselineSubNodeFound = true;            

            break;
            }

        pTestChildNode = pTestChildNode->GetNextSibling();                    
        }

    return baselineSubNodeFound;
    }

BENTLEY_NAMESPACE_NAME::WString UpdateTest_GetStmFileNameWithSuffix(BENTLEY_NAMESPACE_NAME::WString stmFileName, BENTLEY_NAMESPACE_NAME::WString suffix)
    {
    // Testfile.stm will become TestFileSuffix.stm.
    // Could be used to create a file like "TestFile_Add.stm"
    // to specify that the file is the result of a partial update of type "Add".

    BENTLEY_NAMESPACE_NAME::WString stmFileExtension(".stm");
    BENTLEY_NAMESPACE_NAME::WString prefix = stmFileName.substr(0, stmFileName.length() - stmFileExtension.length());

    bvector<WString> newPathStrings;
    newPathStrings.push_back(prefix);
    newPathStrings.push_back(suffix);
    newPathStrings.push_back(stmFileExtension);
    WString  newPath = BeStringUtilities::Join(newPathStrings, NULL);

    return newPath;
    }

WString GetMesherTypeName(ScalableMeshMesherType mesherType)
    {
    switch (mesherType)
        {
        case SCM_MESHER_2D_DELAUNAY:
            return WString(L"2.5D_DELAUNAY");
            break;

        case SCM_MESHER_LMS_MARCHING_CUBE:
            return WString(L"LMS_MARCHING_CUBE (i.e. : Merry)");
            break;

        case SCM_MESHER_3D_DELAUNAY:
            return WString(L"3D_DELAUNAY");
            break;

        case SCM_MESHER_TETGEN:
            return WString(L"TETGEN");
            break;

        default:
            assert(!"Unknown mesher type");
        }

    return WString("");
    }

WString GetFilterTypeName(ScalableMeshFilterType filterType)
    {
    switch (filterType)
        {
        case SCM_FILTER_DUMB_MESH:
            return WString(L"Dumb mesh");
            break;
        case SCM_FILTER_GARLAND_SIMPLIFIER:
            return WString(L"Garland simplifier");
            break;
        default:
            assert(!"Unknown filter type");
        }

    return WString("");
    }

WString GetTrimmingTypeName(int trimmingType)
    {
    switch (trimmingType)
        {
        case 5:
            return WString(L"Method 5");
            break;
        case 7:
            return WString(L"Umbrella Filtering");
            break;
        default:
            assert(!"Unknown trimming type");
        }

    return WString("");
    }

WString GetBlossomMatchingValue(int blossomMatch)
    {
    switch (blossomMatch)
        {
        case 0:
            return WString(L"BLOSSOM NOT USED");
            break;

        case 1:
            return WString(L"BLOSSOM-EDGES");
            break;

        case 2:
            return WString(L"BLOSSOM-FACES");
            break;
        default:
            assert(!"Wrong blossom matching value");
        }

    return WString("");
    }

WString GetIndexMethodValue(int indexMethod)
    {
    switch (indexMethod)
        {
        case 0:
            return WString(L"CENTROIDS");
            break;

        case 1:
            return WString(L"BCLIB");
            break;

        case 2:
            return WString(L"CLASSIC");
            break;
        case 3:
            return WString(L"TEST");
            break;
        default:
            assert(!"Wrong index method");
        }

    return WString("");
    }

WString GetTrimmingMethodValue(int trimmingMethod)
    {
    switch (trimmingMethod)
        {
        case 5:
            return WString(L"METHOD5");
            break;

        case 7:
            return WString(L"UMBRELLA");
            break;

        default:
            assert(!"Wrong trimming method");
        }

    return WString("");
    }