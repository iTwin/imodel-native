#include "ScalableMeshSDKexeImporter.h"

#include <ScalableTerrainModel\IMrDTMSources.h>


#include <windows.h>   

USING_NAMESPACE_BENTLEY_DGNPLATFORM
namespace ScalableMeshSDKexe
    {

    /*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline void AddMissingBackslashToFolderPath(WString* pio_pFolderPath)
    {
    assert(0 != pio_pFolderPath);

    if (pio_pFolderPath->at(pio_pFolderPath->size() - 1) != '\\')
        *pio_pFolderPath += L"\\";
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline void AddWildCardToFolderPath(WString* pio_pFolderPath)
    {
    assert(0 != pio_pFolderPath);

    AddMissingBackslashToFolderPath(pio_pFolderPath);
    *pio_pFolderPath += L"*.*";
    }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Hiba.Dorias   	04/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    FileFinder::FileFinder()
        {}

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Hiba.Dorias   	04/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    FileFinder::~FileFinder()
        {}

    /*---------------------------------------------------------------------------------**//**
    * @description  Searches the SourceFolderPath for files.
    *               Returns a AString with the complete path of each file found
    *               separated by a comma.
    * @bsimethod                                                  Hiba.Dorias   	04/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    void FileFinder::FindFiles(const WString& pi_rSourceFolderPath,
                               WString&       pi_FilePaths,  //=>should be initially an empty AString
                               bool        pi_SearchSubFolders) const
        {
        HANDLE hFindFile;
        WIN32_FIND_DATAW FindFileData;
        WString SourceFolderPathWithWildCard = pi_rSourceFolderPath;
        WString SrcName;
        WString SrcNameWithWildcard;

        AddWildCardToFolderPath(&SourceFolderPathWithWildCard);

        hFindFile = ::FindFirstFileW(SourceFolderPathWithWildCard.c_str(), &FindFileData);

        do
            {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                SrcName = pi_rSourceFolderPath + FindFileData.cFileName;

                //it's a file!
                pi_FilePaths = pi_FilePaths + SrcName + L";";
                }
            else
                {
                SrcName = pi_rSourceFolderPath + FindFileData.cFileName;
                SrcNameWithWildcard = SrcName;
                AddWildCardToFolderPath(&SrcNameWithWildcard);

                if (!((wcscmp(FindFileData.cFileName, L".") == 0) ||
                    (wcscmp(FindFileData.cFileName, L"..") == 0)))
                    {
                    if (pi_SearchSubFolders)
                        {
                        WIN32_FIND_DATAW FindFileData2;
                        HANDLE hFindFile2;

                        hFindFile2 = ::FindFirstFileW(SrcNameWithWildcard.c_str(), &FindFileData2);

                        FindFiles(SrcName, pi_FilePaths, pi_SearchSubFolders);
                        }
                    }
                }

            }
        while (::FindNextFileW(hFindFile, &FindFileData));

            ::FindClose(hFindFile);

        }

    /*---------------------------------------------------------------------------------**//**
    * @description  Returns the FilePaths AString, less the first path; which is in the
    *               other AString.
    *               Returns false if the FilePaths AString is empty.
    * @bsimethod                                                  Hiba.Dorias   	04/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool FileFinder::ParseFilePaths(WString& pio_FilePaths,
                                    WString& pio_FirstPath) const
        {
        bool StringNotEmpty = false;
        pio_FirstPath = L"";

        WChar seps[] = L";";

        WChar* token1,
            *next_token1;

        token1 = wcstok_s(const_cast<WChar*>(pio_FilePaths.c_str()), seps, &next_token1);

        if (token1 != NULL)
            {
            pio_FirstPath = token1;
            StringNotEmpty = true;

            pio_FilePaths = next_token1;

            }

        return StringNotEmpty;
        }


    BENTLEY_NAMESPACE_NAME::MrDTM::IDTMSourcePtr CreateSourceFor(const WString&                    sourcePath,
                                                  BENTLEY_NAMESPACE_NAME::MrDTM::DTMSourceDataType importedType)
        {
        BENTLEY_NAMESPACE_NAME::MrDTM::ILocalFileMonikerPtr monikerPtr(BENTLEY_NAMESPACE_NAME::MrDTM::ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));
       
        return BENTLEY_NAMESPACE_NAME::MrDTM::IDTMLocalFileSource::Create(importedType, monikerPtr).get();
        }

     BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr CreateSourceFor(const WString&                          sourcePath,
                                                          BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceDataType importedType,
                                                          BeXmlNodeP                               pTestChildNode)
        {
        BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr monikerPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));

        if (0 == _wcsicmp(L"dgn", BeFileName::GetExtension(sourcePath.c_str()).c_str()))
            {
            assert(pTestChildNode != 0);

            WString model = L"Default";
            WString level = L"Default";

            DgnFileOpenParams fileOpenParams(sourcePath.c_str(), true, DgnFilePurpose::MasterFile);

            BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnFile> dgnFilePtr(fileOpenParams.CreateFileAndLoad());

            if (dgnFilePtr == 0)
                return BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr();
                                    
            StatusInt status = pTestChildNode->GetAttributeStringValue(model, "model");

            assert(status == SUCCESS);
            
            ModelId modelID = dgnFilePtr->FindModelIdByName (model.c_str());

            StatusInt errorDetails;

            DgnModel* modelRef = dgnFilePtr->LoadRootModelById(&errorDetails, modelID);

            if (modelRef == 0)
                return BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr();
            
            status = pTestChildNode->GetAttributeStringValue(level, "level");

            assert(status == SUCCESS);

            LevelId levelId;

            LevelCache& levelCache = modelRef->GetLevelCacheR();

            LevelHandle levelH = levelCache.GetLevelByName(level.c_str(), false);
            levelId = levelH.GetLevelId();


            assert(status == SUCCESS);

            return BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMDgnLevelSource::Create(importedType, monikerPtr, modelID, model.c_str(), levelId, level.c_str()).get();
            }

        return BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMLocalFileSource::Create(importedType, monikerPtr).get();
        }
    
     bool AddOptionToSource(BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode)
        {
        WString datasetIs3D;
        WString datasetIsGround;

        StatusInt status = pTestChildNode->GetAttributeStringValue(datasetIs3D, "is3D");

        if (status == BEXML_Success)
            {
            assert(!"Not supported yet");
            /*
            if (datasetIs3D.Equals(L"1"))
                {
                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                data.SetRepresenting3dData(true);

                sourceImportConfig.SetReplacementSMData(data);
                }
                */
            }

        status = pTestChildNode->GetAttributeStringValue(datasetIsGround, "grounddetection");

        if (status == BEXML_Success)
            {
            assert(!"Not supported yet");
            /*
            if (datasetIsGround.Equals(L"1"))
                {
                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                data.SetIsGroundDetection(true);
                data.SetRepresenting3dData(false);

                sourceImportConfig.SetReplacementSMData(data);
                }
                */
            }       
        
        WString classesToImportAttr;

        status = pTestChildNode->GetAttributeStringValue(classesToImportAttr, "classesToImport");

        if (status == BEXML_Success)
            {           
            bvector<uint32_t> classesToImport;

            size_t startInd = 0;
            size_t endInd = 0;

            for (; endInd < classesToImportAttr.size(); endInd++)
                {
                if ((classesToImportAttr.c_str()[endInd] == ',') && (startInd < endInd - 1))
                    {                    
                    WString classIdStr(classesToImportAttr.substr(startInd, endInd - startInd - 1));
                    int classId = _wtoi(classIdStr.c_str());                                        
                    classesToImport.push_back(classId);
                    startInd = endInd + 1;
                    }
                }

            if (startInd < endInd)
                {
                WString classIdStr(classesToImportAttr.substr(startInd, endInd - startInd));
                int classId = _wtoi(classIdStr.c_str());
                classesToImport.push_back(classId);
                }

            if (classesToImport.size() > 0)
                {
                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();            
                data.SetClassificationToImport(classesToImport);
                sourceImportConfig.SetReplacementSMData(data);
                }
            }

        return true;
        }
  
    void GetSourceDataType(BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceDataType& dataType, BeXmlNodeP pSourceNode)
        {
        WString dataTypeStr;

        StatusInt status = pSourceNode->GetAttributeStringValue(dataTypeStr, "dataType");

        if (status == BEXML_Success)
            {
            if (dataTypeStr.CompareTo(L"POINT") == 0)
                {
                dataType = BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_POINT;
                }
            else
                if (dataTypeStr.CompareTo(L"DTM") == 0)
                    {
                    dataType = BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_DTM;
                    }
                else
                    if (dataTypeStr.CompareTo(L"BREAKLINE") == 0)
                        {
                        dataType = BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_BREAKLINE;
                        }
                    else
                        {
                        printf("Unsupporter/unknown data type");
                        }
            }
        }            

    bool ParseSourceSubNodes(BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourceCollection& sourceCollection,    
                             bvector<DPoint3d>&                           importClipShape,                                                          
                             DRange3d&                                    importRange, 
                             BeXmlNodeP                                   pTestNode)
        {
        bool isSuccess = true;

        importRange.low.x = -numeric_limits<double>::max();
        importRange.high.x = numeric_limits<double>::max();
        importRange.low.y = -numeric_limits<double>::max();
        importRange.high.y = numeric_limits<double>::max();
        importRange.low.z = -numeric_limits<double>::max();
        importRange.high.z = numeric_limits<double>::max();
        
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
                    BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceDataType dataType = BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_POINT;

                    GetSourceDataType(dataType, pTestChildNode);

                    if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                        (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                        {
                        BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr srcPtr = CreateSourceFor(datasetPath, dataType, pTestChildNode);

                        if (srcPtr == 0)
                            return false;

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
                        FileFinder fileFinder;

                        WString filePaths;

                        fileFinder.FindFiles(datasetPath, filePaths, true);

                        WString firstPath;

                        while (fileFinder.ParseFilePaths(filePaths, firstPath))
                            {
                            BeFileName name(firstPath.c_str());
                            WString extension;
                            name.ParseName(NULL, NULL, NULL, &extension);
                            if (0 == BeStringUtilities::Wcsicmp(extension.c_str(), L"classif")) continue;
                            BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr srcPtr = CreateSourceFor(firstPath, dataType, pTestChildNode);
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
            if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "clipshape"))
                {      
                WString clipShapeStr;
                BeXmlStatus status = pTestChildNode->GetContent(clipShapeStr);
                assert(status == BEXML_Success);
                
                size_t startInd = 0;
                size_t endInd = 0;                
                //DPoint3d pt; 
                bvector<double> clipShape2d;                
                DPoint3d pt; 
                pt.z = 0;
                bool isX = true;

                for (; endInd < clipShapeStr.size(); endInd++)
                    {
                    if (((clipShapeStr.c_str()[endInd] == L',') || (clipShapeStr.c_str()[endInd] == L';')) && (startInd < endInd - 1))
                        {                    
                        WString coordStr(clipShapeStr.substr(startInd, endInd - startInd - 1));
                        if (isX)
                            {
                            assert((clipShapeStr.c_str()[endInd] == L','));
                            pt.x = _wtof(coordStr.c_str());                                        
                            isX = false;
                            }
                        else
                            {
                            assert((clipShapeStr.c_str()[endInd] == L';'));
                            pt.y = _wtof(coordStr.c_str());                                        
                            isX = true;
                            importClipShape.push_back(pt);
                            }
                        
                        startInd = endInd + 1;
                        }
                    }

                if (startInd < endInd)
                    {
                    assert(isX == false);
                    WString coordStr(clipShapeStr.substr(startInd, endInd - startInd));
                    pt.y = _wtof(coordStr.c_str());
                    importClipShape.push_back(pt);
                    }

                importClipShape.push_back(importClipShape[0]);
                                                                                                                          
                importRange = DRange3d::From(&importClipShape[0], (int)importClipShape.size());                
                }

            pTestChildNode = pTestChildNode->GetNextSibling();
            }

        return isSuccess;
        }
    }