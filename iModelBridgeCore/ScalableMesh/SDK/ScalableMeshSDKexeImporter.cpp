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


    Bentley::MrDTM::IDTMSourcePtr CreateSourceFor(const WString&                    sourcePath,
                                                  Bentley::MrDTM::DTMSourceDataType importedType)
        {
        Bentley::MrDTM::ILocalFileMonikerPtr monikerPtr(Bentley::MrDTM::ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));
       
        return Bentley::MrDTM::IDTMLocalFileSource::Create(importedType, monikerPtr).get();
        }

     Bentley::ScalableMesh::IDTMSourcePtr CreateSourceFor(const WString&                          sourcePath,
                                                          Bentley::ScalableMesh::DTMSourceDataType importedType,
                                                          BeXmlNodeP                               pTestChildNode)
        {
        Bentley::ScalableMesh::ILocalFileMonikerPtr monikerPtr(Bentley::ScalableMesh::ILocalFileMonikerFactory::GetInstance().Create(sourcePath.c_str()));

        if (0 == _wcsicmp(L"dgn", BeFileName::GetExtension(sourcePath.c_str()).c_str()))
            {
            assert(pTestChildNode != 0);

            WString model = L"Default";
            WString level = L"Default";

            DgnFileOpenParams fileOpenParams(sourcePath.c_str(), true, DgnFilePurpose::MasterFile);

            Bentley::RefCountedPtr<DgnFile> dgnFilePtr(fileOpenParams.CreateFileAndLoad());

            if (dgnFilePtr == 0)
                return Bentley::ScalableMesh::IDTMSourcePtr();
                                    
            StatusInt status = pTestChildNode->GetAttributeStringValue(model, "model");

            assert(status == SUCCESS);
            
            ModelId modelID = dgnFilePtr->FindModelIdByName (model.c_str());

            StatusInt errorDetails;

            DgnModel* modelRef = dgnFilePtr->LoadRootModelById(&errorDetails, modelID);

            if (modelRef == 0)
                return Bentley::ScalableMesh::IDTMSourcePtr();
            
            status = pTestChildNode->GetAttributeStringValue(level, "level");

            assert(status == SUCCESS);

            LevelId levelId;

            LevelCache& levelCache = modelRef->GetLevelCacheR();

            LevelHandle levelH = levelCache.GetLevelByName(level.c_str(), false);
            levelId = levelH.GetLevelId();


            assert(status == SUCCESS);

            return Bentley::ScalableMesh::IDTMDgnLevelSource::Create(importedType, monikerPtr, modelID, model.c_str(), levelId, level.c_str()).get();
            }

        return Bentley::ScalableMesh::IDTMLocalFileSource::Create(importedType, monikerPtr).get();
        }
    
     bool AddOptionToSource(Bentley::ScalableMesh::IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode)
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

        //NEEDS_WORK_SM
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
  
    void GetSourceDataType(Bentley::ScalableMesh::DTMSourceDataType& dataType, BeXmlNodeP pSourceNode)
        {
        WString dataTypeStr;

        StatusInt status = pSourceNode->GetAttributeStringValue(dataTypeStr, "dataType");

        if (status == BEXML_Success)
            {
            if (dataTypeStr.CompareTo(L"POINT") == 0)
                {
                dataType = Bentley::ScalableMesh::DTM_SOURCE_DATA_POINT;
                }
            else
                if (dataTypeStr.CompareTo(L"DTM") == 0)
                    {
                    dataType = Bentley::ScalableMesh::DTM_SOURCE_DATA_DTM;
                    }
                else
                    if (dataTypeStr.CompareTo(L"BREAKLINE") == 0)
                        {
                        dataType = Bentley::ScalableMesh::DTM_SOURCE_DATA_BREAKLINE;
                        }
                    else
                        {
                        printf("Unsupporter/unknown data type");
                        }
            }
        }
    
    bool ParseSourceSubNodes(Bentley::ScalableMesh::IDTMSourceCollection& sourceCollection, BeXmlNodeP pTestNode)
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
                    Bentley::ScalableMesh::DTMSourceDataType dataType = Bentley::ScalableMesh::DTM_SOURCE_DATA_POINT;

                    GetSourceDataType(dataType, pTestChildNode);

                    if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                        (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                        {
                        Bentley::ScalableMesh::IDTMSourcePtr srcPtr = CreateSourceFor(datasetPath, dataType, pTestChildNode);

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
                            Bentley::ScalableMesh::IDTMSourcePtr srcPtr = CreateSourceFor(firstPath, dataType, pTestChildNode);
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
                }

            pTestChildNode = pTestChildNode->GetNextSibling();
            }

        return isSuccess;
        }
    }