#include "ATPUtils.h"
#include "ATPDefinitions.h"
#include "ATPGeneration.h"
#include <sys/stat.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshATP.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <DgnPlatform/Tools/ConfigurationManager.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT


enum
    {
    ACCELERATOR_CPU = 0,
    ACCELERATOR_GPU
    };

void PerformGenerateTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    BeXmlStatus status;
    WString stmFileName;
    status = pTestNode->GetAttributeStringValue(stmFileName, "stmFileName");

    if (status != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        }
    else
        {
        Int64 useCpuVal = 0;
        WString accelerator;
        status = pTestNode->GetAttributeStringValue(accelerator, "accelerator");
        if (status == BEXML_Success && 0 == BeStringUtilities::Wcsicmp(accelerator.c_str(), L"cpu"))
            {
            useCpuVal = 1;
            IScalableMeshATP::StoreInt(L"useCpu", useCpuVal);
            }
        Int64 useThreadInGeneration = 0;
        WString threading;
        status = pTestNode->GetAttributeStringValue(threading, "multithread");
        if (status == BEXML_Success && 0 == BeStringUtilities::Wcsicmp(threading.c_str(), L"true"))
            {
            useThreadInGeneration = 1;
            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_MULTITHREAD_GENERATION", L"1");

            assert(defineStatus == SUCCESS);
            }
        else
            {
            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_MULTITHREAD_GENERATION", L"0");

            assert(defineStatus == SUCCESS);
            }

        WString isSingleFileString;
        bool isSingleFile;

        if (BEXML_Success == pTestNode->GetAttributeStringValue(isSingleFileString, "singleFile"))
            {
            isSingleFile = isSingleFileString.Equals(L"1") ? true : false;
            }
        else
            isSingleFile = true;

        ScalableMeshMesherType mesherType = SCM_MESHER_3D_DELAUNAY;
        ScalableMeshFilterType filterType = SCM_FILTER_DUMB_MESH;
        ScalableMeshSaveType saveType = SCM_SAVE_STMFILE;
        int trimmingMethod = 5;

        if (ParseGenerationOptions(&mesherType, &filterType, &trimmingMethod, &saveType, pTestNode) == true)
            {
            WChar mesherTypeChar[10];

            swprintf(mesherTypeChar, L"%i", mesherType);

            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_3D_MESHER_TYPE", mesherTypeChar);

            assert(defineStatus == SUCCESS);

            WChar filterTypeChar[10];

            swprintf(filterTypeChar, L"%i", filterType);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_FILTER_TYPE", filterTypeChar);

            assert(defineStatus == SUCCESS);

            WChar trimmingMethodChar[10];

            swprintf(trimmingMethodChar, L"%i", trimmingMethod);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_TRIMMING_METHOD", trimmingMethodChar);

            assert(defineStatus == SUCCESS);

            WChar saveTypeChar[10];

            swprintf(saveTypeChar, L"%i", saveType);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_STORE_DGNDB", saveTypeChar);

            assert(defineStatus == SUCCESS);
            }
        StatusInt createStatus;
        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName.c_str(), createStatus));

        if (creatorPtr == 0)
            {
            printf("ERROR : cannot create STM file\r\n");
            }
        else
            {
            IScalableMeshATP::StoreDouble(WString("nTimeToCreateSeeds"), 0.0);
            IScalableMeshATP::StoreDouble(WString("nTimeToEstimateParams"), 0.0);
            IScalableMeshATP::StoreDouble(WString("nTimeToFilterGround"), 0.0);
            IScalableMeshATP::StoreInt(L"chosenAccelerator", 1);
            double nTimeToCreateSeeds = 0, nTimeToEstimateParams = 0, nTimeToFilterGround = 0;
            if (ParseSourceSubNodes(creatorPtr->EditSources(), pTestNode) == true)
                {
                SetGroundDetectionDuration(0.0);
                clock_t t = clock();

                StatusInt status = creatorPtr->Create(isSingleFile);

                t = clock() - t;
                double delay = (double)t / CLOCKS_PER_SEC;
                creatorPtr->SaveToFile();
                creatorPtr = nullptr;
                if (status == SUCCESS)
                    {
                    StatusInt status;

                    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), false, true, status);

                    Int64 pointCount = 0;
                    WString result;

                    if (stmFile != 0)
                        {
                        pointCount = stmFile->GetPointCount();
                        result = L"SUCCESS";
                        }
                    else
                        {
                        result = L"CREATION : SUCCESS | STM FILE OPENING : FAILURE";
                        }

                    stmFile = 0;

                    double minutes = delay / 60.0;
                    double hours = minutes / 60.0;

                    FILE* f = _wfopen(stmFileName.c_str(), L"r");
                    _fseeki64(f, 0, SEEK_END);
                    __int64 fileSize = _ftelli64(f);
                    fclose(f);

                    if (saveType == SCM_SAVE_DGNDB_BLOB)
                        {
                        WString dgndbFileName(stmFileName);
                        dgndbFileName.ReplaceAll(L".stm", L".dgndb");

                        FILE* f = _wfopen(dgndbFileName.c_str(), L"r");
                        assert(f != 0);
                        _fseeki64(f, 0, SEEK_END);
                        fileSize += _ftelli64(f);
                        fclose(f);
                        }

                    WString mesher = GetMesherTypeName(mesherType);
                    WString filter = GetFilterTypeName(filterType);
                    WString trimming = GetTrimmingTypeName(trimmingMethod);
                    Int64 acceleratorUseCpu = 0;
                    IScalableMeshATP::GetDouble(WString("nTimeToCreateSeeds"), nTimeToCreateSeeds);
                    IScalableMeshATP::GetDouble(WString("nTimeToEstimateParams"), nTimeToEstimateParams);
                    IScalableMeshATP::GetDouble(WString("nTimeToFilterGround"), nTimeToFilterGround);
                    IScalableMeshATP::GetInt(L"chosenAccelerator", acceleratorUseCpu);
                    // L"File Name,Mesher,Filter,Nb Input Points,Nb Output Points,Point Kept (%%),File Size (Mb),Accelerator Used,GroundDetection: Time for seeds(s),GroundDetection: Time for Params Estimation (s), GroundDetection: Time for TIN growing (s),GroundDetection (s),GroundDetection(%%), Import Points (%%),Balancing (%%),Meshing (%%),Filtering (%%),Stitching (%%),Duration (minutes),Duration (hours), GroundDetection(minutes), Import Points (minutes),Balancing (minutes),Meshing (minutes),Filtering (minutes),Stitching (minutes),Status\n";

                    fwprintf(pResultFile,
                             L"%s,%s,%s,%s,%I64d,%I64d,%.5f%%,%.5f,%s,%.5f,%.5f,%.5f,%.5f(%.5f s),%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%s\n",
                             stmFileName.c_str(), mesher.c_str(), filter.c_str(), trimming.c_str(), IScalableMeshSourceCreator::GetNbImportedPoints(), pointCount,
                             (double)pointCount / IScalableMeshSourceCreator::GetNbImportedPoints() * 100.0, (double)fileSize / 1024.0 / 1024.0,
                             acceleratorUseCpu == ACCELERATOR_CPU ? L"CPU" : L"GPU",
                             nTimeToCreateSeeds,
                             nTimeToEstimateParams,
                             nTimeToFilterGround,
                             GetGroundDetectionDuration(),
                             GetGroundDetectionDuration() * 60,
                             GetGroundDetectionDuration() / minutes * 100,
                             (IScalableMeshSourceCreator::GetImportPointsDuration() - GetGroundDetectionDuration()) / minutes * 100, //Import points duration includes ground detection duration.
                             IScalableMeshSourceCreator::GetLastBalancingDuration() / minutes * 100,
                             IScalableMeshSourceCreator::GetLastMeshingDuration() / minutes * 100,
                             IScalableMeshSourceCreator::GetLastFilteringDuration() / minutes * 100,
                             IScalableMeshSourceCreator::GetLastStitchingDuration() / minutes * 100,
                             minutes, hours,
                             GetGroundDetectionDuration(),
                             IScalableMeshSourceCreator::GetImportPointsDuration() - GetGroundDetectionDuration(),
                             IScalableMeshSourceCreator::GetLastBalancingDuration(),
                             IScalableMeshSourceCreator::GetLastMeshingDuration(),
                             IScalableMeshSourceCreator::GetLastFilteringDuration(),
                             IScalableMeshSourceCreator::GetLastStitchingDuration(),
                             result.c_str());
                    }
                else
                    {
                    fwprintf(pResultFile, L"%s,%s,%s,%.5f,%s\n", L"", L"", L"", 0, 0, 0, 0, 0, 0, 0, 0, L"ERROR");
                    }
                fflush(pResultFile);
                }
            }
        }
    }

void PerformUpdateTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName;
    ScalableMeshMesherType mesherType = SCM_MESHER_3D_DELAUNAY;
    ScalableMeshFilterType filterType = SCM_FILTER_GARLAND_SIMPLIFIER;
    vector<UpToDateState> sourcesPartialUpdate;
    vector<WString> sourceToAdd;

    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }
    else
        {
        // Create three datasets
        WString suffixPartialUpdate("_partialUpdate");
        WString stmFileName_PartialUpdateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixPartialUpdate));
        StatusInt status;
        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPartialUpdatePtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status));
        WString suffixGenerate("_generate");
        WString stmFileName_GenerateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixGenerate));
        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorGeneratePtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_GenerateTest.c_str(), status));

        fwprintf(pResultFile, L"Initial state\n");
        fwprintf(pResultFile, L",file name, state\n");

        if (creatorPartialUpdatePtr == 0 || creatorGeneratePtr == 0)
            {
            printf("ERROR : cannot create STM file\r\n");
            }
        else
            {
            creatorPartialUpdatePtr->SaveToFile();
            creatorGeneratePtr->SaveToFile();
            // Initialize for the 3 stm generate (initialize state, partial update state, final state (use to compare))

            BeXmlNodeP pSubNode = pTestNode->GetFirstChild();
            while (0 != pSubNode)
                {
                if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "mesher"))
                    {
                    int32_t mesherTypeAttr;
                    StatusInt status = pSubNode->GetAttributeInt32Value(mesherTypeAttr, "type");

                    if ((status == BEXML_Success) && (mesherTypeAttr >= 0) && (mesherTypeAttr < SCM_MESHER_QTY))
                        mesherType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesherType)mesherTypeAttr;
                    else
                        printf("ERROR : invalid type attribute for mesher node\r\n");
                    }
                else if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "filter"))
                    {
                    int32_t filterTypeAttr;

                    StatusInt status = pSubNode->GetAttributeInt32Value(filterTypeAttr, "type");

                    if ((status == BEXML_Success) && (filterTypeAttr >= 0) && (filterTypeAttr < SCM_FILTER_QTY))
                        filterType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshFilterType)filterTypeAttr;
                    else
                        printf("ERROR : invalid type attribute for filter node\r\n");
                    }
                else if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "source"))
                    {
                    WString sourcePath;
                    DTMSourceDataType type = DTM_SOURCE_DATA_POINT;
                    GetSourceDataType(type, pSubNode);
                    if (pSubNode->GetAttributeStringValue(sourcePath, "path") == BEXML_Success)
                        {
                        // Parse the optional attribute "updateType"
                        WString updateType;
                        if (pSubNode->GetAttributeStringValue(updateType, "updateType") == BEXML_Success)
                            {
                            fwprintf(pResultFile, L",%s,%s\n", sourcePath, updateType);
                            if (updateType.Equals(WString("Add")))
                                {
                                sourcesPartialUpdate.push_back(UpToDateState::ADD);
                                sourceToAdd.push_back(sourcePath);
                                }
                            else if (updateType.Equals(WString("Remove")))
                                sourcesPartialUpdate.push_back(UpToDateState::REMOVE);
                            else if (updateType.Equals(WString("Modify")))
                                sourcesPartialUpdate.push_back(UpToDateState::MODIFY);
                            else if (updateType.Equals(WString("UpToDate")))
                                sourcesPartialUpdate.push_back(UpToDateState::UP_TO_DATE);
                            if (!updateType.Equals(WString("Add")))
                                {
                                IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                                struct stat attrib;
                                char fileName[2048];
                                wcstombs(fileName, sourcePath.GetWCharCP(), sourcePath.GetMaxLocaleCharBytes());
                                stat(fileName, &attrib);
                                data.SetTimeFile(attrib.st_mtime);

                                sourceImportConfig.SetReplacementSMData(data);

                                if (BSISUCCESS != creatorPartialUpdatePtr->EditSources().Add(srcPtr))
                                    {
                                    wprintf(L"ERROR : cannot add %s\r\n", sourcePath);
                                    break;
                                    }
                                }
                            if (!updateType.Equals(WString("Remove")))
                                {
                                IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                                struct stat attrib;
                                char fileName[2048];
                                wcstombs(fileName, sourcePath.GetWCharCP(), sourcePath.GetMaxLocaleCharBytes());
                                stat(fileName, &attrib);
                                data.SetTimeFile(attrib.st_mtime);

                                sourceImportConfig.SetReplacementSMData(data);

                                if (BSISUCCESS != creatorGeneratePtr->EditSources().Add(srcPtr))
                                    {
                                    wprintf(L"ERROR : cannot add %s\r\n", sourcePath);
                                    break;
                                    }
                                }
                            }
                        else
                            {
                            IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                            struct stat attrib;
                            char fileName[2048];
                            wcstombs(fileName, sourcePath.GetWCharCP(), sourcePath.GetMaxLocaleCharBytes());
                            stat(fileName, &attrib);
                            data.SetTimeFile(attrib.st_mtime);


                            sourceImportConfig.SetReplacementSMData(data);

                            // Datasource without the attribute "updateType"
                            if (BSISUCCESS != creatorPartialUpdatePtr->EditSources().Add(srcPtr))
                                {
                                wprintf(L"ERROR : cannot add %s\r\n", sourcePath);
                                break;
                                }
                            sourcesPartialUpdate.push_back(UpToDateState::UP_TO_DATE);
                            }
                        }
                    }
                pSubNode = pSubNode->GetNextSibling();
                }

            creatorPartialUpdatePtr->SaveToFile();
            creatorGeneratePtr->SaveToFile();

            WChar mesherTypeChar[10];
            swprintf(mesherTypeChar, L"%i", mesherType);
            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_3D_MESHER_TYPE", mesherTypeChar);
            assert(defineStatus == SUCCESS);
            WChar filterTypeChar[10];
            swprintf(filterTypeChar, L"%i", filterType);
            defineStatus = ConfigurationManager::DefineVariable(L"SM_FILTER_TYPE", filterTypeChar);
            assert(defineStatus == SUCCESS);
            StatusInt status;
            creatorGeneratePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_GenerateTest.c_str(), status);
            clock_t tGen = clock();
            // isSingleFile ?
            status = creatorGeneratePtr->Create();
            tGen = clock() - tGen;
            assert(status == 0);
            creatorGeneratePtr->SaveToFile();


            IDTMSourceCollection sourceCollec = creatorGeneratePtr->GetSources();
            IDTMSourceCollection::iterator srcIt = sourceCollec.BeginEdit();
            for (IDTMSourceCollection::iterator srcIt = sourceCollec.BeginEdit(); srcIt != sourceCollec.EndEdit(); srcIt++)
                {
                IDTMSource& source = *srcIt;
                SourceImportConfig& conf = source.EditConfig();
                ScalableMeshData data = conf.GetReplacementSMData();
                GCS gcs = conf.GetReplacementGCS();
                }


            creatorGeneratePtr = 0;
            // Generate initial state
            // found all sources (all else add sources)
            //creatorPartialUpdatePtr->Release();
            //delete creatorPartialUpdatePtr;

            creatorPartialUpdatePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status);
            // isSingleFile
            status = creatorPartialUpdatePtr->Create();
            assert(status == 0);
            creatorPartialUpdatePtr->SaveToFile();

            creatorPartialUpdatePtr = 0;
            creatorPartialUpdatePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status);
            // Generate partial update
            // use the initial stm and modify the state of source
            IDTMSourceCollection sourceCollection = creatorPartialUpdatePtr->GetSources();
            IDTMSourceCollection::iterator sourceIt = sourceCollection.BeginEdit();

            fwprintf(pResultFile, L"Partial Update\n");

            for (int i = 0; i < sourcesPartialUpdate.size(); i++)
                {
                if (sourcesPartialUpdate[i] != UpToDateState::ADD && sourceIt != sourceCollection.EndEdit())
                    {
                    IDTMSource& source = *sourceIt;
                    SourceImportConfig& conf = source.EditConfig();
                    ScalableMeshData data = conf.GetReplacementSMData();
                    data.SetUpToDateState(sourcesPartialUpdate[i]);
                    conf.SetReplacementSMData(data);
                    sourceIt++;
                    }
                }

            // Remove ????? :(

            for (int i = 0; i < sourceToAdd.size(); i++)
                {

                IDTMSourcePtr srcPtr = CreateSourceFor(sourceToAdd[i], DTM_SOURCE_DATA_POINT);
                SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
                ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

                struct stat attrib;
                char fileName[2048];
                wcstombs(fileName, sourceToAdd[i].GetWCharCP(), sourceToAdd[i].GetMaxLocaleCharBytes());
                stat(fileName, &attrib);

                data.SetTimeFile(attrib.st_mtime);


                sourceImportConfig.SetReplacementSMData(data);

                if (BSISUCCESS != creatorPartialUpdatePtr->EditSources().Add(srcPtr))
                    {
                    wprintf(L"ERROR : cannot add %s\r\n", sourceToAdd[i]);
                    break;
                    }
                }

            //sourceCollection =  creatorPartialUpdatePtr->GetSources();

            creatorPartialUpdatePtr->SaveToFile();
            creatorPartialUpdatePtr = 0;
            creatorPartialUpdatePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status);
            clock_t t = clock();
            status = creatorPartialUpdatePtr->Create();
            t = clock() - t;
            creatorPartialUpdatePtr->SaveToFile();
            assert(status == 0);
            creatorPartialUpdatePtr = 0;
            Int64 nOfPointsGen, nOfPointsUpdate;
            IScalableMeshPtr sm = IScalableMesh::GetFor(stmFileName_GenerateTest.c_str(), true, true);
            nOfPointsGen = sm->GetPointCount();
            sm = 0;
            sm = IScalableMesh::GetFor(stmFileName_PartialUpdateTest.c_str(), true, true);
            nOfPointsUpdate = sm->GetPointCount();
            sm = 0;
            double delay = (double)t / CLOCKS_PER_SEC;
            double minutes = delay / 60.0;
            double hours = minutes / 60.0;
            fwprintf(pResultFile, L", hours, minutes\n");
            fwprintf(pResultFile, L"Time for generation, %.5f, %.5f\n", (double)tGen / CLOCKS_PER_SEC * (1 / 60.0)* (1 / 60.0), (double)tGen / CLOCKS_PER_SEC * (1 / 60.0));
            fwprintf(pResultFile, L"Time for partial update, %.5f, %.5f\n", hours, minutes);
            fwprintf(pResultFile, L"NOfPoints Generated, %I64d\n", nOfPointsGen);
            fwprintf(pResultFile, L"NOfPoints After Update, %I64d\n", nOfPointsUpdate);
            fflush(pResultFile);
            }
        }
    }

void PerformMeshQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "Not ported yet! Perhaps we could use your help?");
    }

void Perform2DStitchQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "Not ported yet! Perhaps we could use your help?");
    }

void PerformDrapeLineTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, linesFileName;
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        assert(false && !"SM not found");
        return;
        }
    if (pTestNode->GetAttributeStringValue(linesFileName, "linesFileName") != BEXML_Success)
        {
        assert(false && !"Test lines not found");
        return;
        }
    IScalableMeshPtr ptr = IScalableMesh::GetFor(stmFileName.c_str(), true, true);
    assert(ptr != 0);
    if (ptr == 0) return;
    std::ifstream linesFile;
    char* nameBuffer = new char[linesFileName.GetMaxLocaleCharBytes()];
    linesFile.open(linesFileName.ConvertToLocaleChars(nameBuffer));
    delete nameBuffer;
    bvector<bvector<DPoint3d>> linesToDrape;
    ReadLinesFile(linesFile, linesToDrape);
    linesFile.close();

    int64_t loadAttempts =1;
    int64_t loadMisses =0;
    int64_t nOutPts = 0;
    int64_t nPartial = 0;
    double drapeLength = 0.0;
    int64_t nOfLinesToDrape = 0, nOfLinesDraped = 0, nOfLinesNotDraped = 0;
    double timeOfDrape = 0.0;

    for (auto line : linesToDrape)
        {
        DTMDrapedLinePtr drapedLine;
        bvector<DPoint3d> drapeLine;
        clock_t timer = clock();
        auto draping = ptr->GetDTMDraping();
        draping->DrapeLinear(drapedLine, line.data(), (int)line.size());
        timeOfDrape += clock() - timer;
        if (drapedLine.IsNull())
            {
            nOfLinesNotDraped++;
            continue;
            }
        nOfLinesDraped++;
        unsigned int numPoints = drapedLine->GetPointCount();
        for (unsigned int ptNum = 0; ptNum < numPoints; ptNum++)
            {
            DPoint3d pt;
            drapedLine->GetPointByIndex(&pt, nullptr, nullptr, ptNum);
            drapeLine.push_back(pt);
            }

        bvector<DPoint3d>::iterator it = unique(drapeLine.begin(), drapeLine.end(), DPoint3dEqualityTest);
        drapeLine.resize(std::distance(drapeLine.begin(), it));
        if (drapeLine.size() == 0) nPartial++;
        else
            {
            DPoint3d pt = drapeLine.back();
            if (fabs(line[line.size() - 1].x - pt.x)<0.001 && fabs(line[line.size() - 1].y - pt.y)<0.001)
                {
                drapeLength += 1.0;
                }
            else
                {
                nPartial++;
                DPoint3d intersectPt1 = pt;
                intersectPt1.z = line[line.size() - 1].z;
                drapeLength += fabs(DVec3d::FromStartEnd(line[line.size() - 2], intersectPt1).MagnitudeSquared() / DVec3d::FromStartEnd(line[line.size() - 2], line[line.size() - 1]).MagnitudeSquared());
                }
            }
        nOutPts += (int)drapeLine.size();
        }

    timeOfDrape /= CLOCKS_PER_SEC;
    nOfLinesToDrape = nOfLinesDraped + nOfLinesNotDraped;
    fwprintf(pResultFile, L"%s,%s,%I64d,%I64d,%I64d,%I64d,%.5f,%.5f,%.5f,%I64d,%.5f,%.5f,%.5f,%I64d, %.5f\n",
             stmFileName.c_str(),
             "SUCCESS",
             ptr->GetPointCount(),
             nOfLinesToDrape,
             nOfLinesDraped,
             nOfLinesDraped - nPartial,
             nPartial * 100.0 / nOfLinesDraped,
             drapeLength * 100.0,
             loadMisses*100.0 / loadAttempts,
             nOfLinesNotDraped,
             (double)nOfLinesNotDraped*100.0 / nOfLinesToDrape,
             timeOfDrape,
             timeOfDrape / nOfLinesToDrape,
             nOutPts,
             1000 * timeOfDrape / nOutPts);
    fflush(pResultFile);
    }

void PerformVolumeTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "Not ported yet! Perhaps we could use your help?");
    }

void PerformSelfContainedImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "Not ported yet! Perhaps we could use your help?");
    }

void PerformGISFeaturesImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "Not ported yet! Perhaps we could use your help?");
    }