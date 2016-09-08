#include "ATPDefinitions.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <wtypes.h>
#include <random>
#include <queue>

#include <ScalableMesh/Foundations/Definitions.h>
#undef static_assert

#include "ATPUtils.h"
#include "ATPGeneration.h"
#include "..\TiledTriangulation\MrDTMUtil.h"
#include "..\Tool\DrapeLineTool.h"
#include "..\Tool\VolumeCalculationTool.h"
#include "..\Initialize.h"
#include "..\TiledTriangulation\ITiledTriangulatorValidator.h"



using namespace std;

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshATP.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <TerrainModel/Core/DTMDefs.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <Bentley/BeTimeUtilities.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <Vu/VuApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <ImagePP/h/ImageppAPI.h>

#include <ImagePP\all\h\HFCURLFile.h>
#include <ImagePP\all\h\HRFRasterFileFactory.h>
#include <ImagePP\all\h\HCDCodec.h>
#include <ImagePP\all\h\HRPPixelTypeV24R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP\all\h\HCDCodecIdentity.h>
#include <ImagePP\all\h\HRAClearOptions.h>
#include <ImagePP\all\h\HRACopyFromOptions.h>
#include <ImagePP\all\h\HGFHMRStdWorldCluster.h>

#include <ImagePP/all/h/HIMOnDemandMosaic.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HRSObjectStore.h>

#include <GeoCoord/BaseGeoCoord.h>

#include <BeJpeg\BeJpeg.h>


//#define ABORT(ERROR + 1)

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_IMAGEPP


enum
    {
    ACCELERATOR_CPU = 0,
    ACCELERATOR_GPU
    };

void PerformExportToUnityTest(BeXmlNodeP pTestNode, FILE* pResultFile)
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
		WString outputDir;
		int maxLevel;
		bool exportTexture;

		assert(ParseExportToUnityOptions(outputDir, maxLevel, exportTexture, pTestNode) == true);

		if (status == SUCCESS)
			{
			StatusInt status;
			IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

			//Initialize the origin
			DRange3d range;
			stmFile->GetRange(range);
			DPoint3d translateToOrigin;
			translateToOrigin.x = (range.high.x + range.low.x) / 2;
			translateToOrigin.y = (range.high.y + range.low.y) / 2;
			translateToOrigin.z = (range.high.z + range.low.z) / 2;
			translateToOrigin.Negate();

			if (stmFile != 0)
				{				
				IScalableMeshMeshQueryPtr meshQueryInterface = stmFile->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);

				//the root node
				IScalableMeshNodePtr root;

				//find the root node
				for (int level = 0; level <= maxLevel; level++)
					{
					bvector<IScalableMeshNodePtr> returnedNodes;
					IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
					params->SetLevel(level);
					meshQueryInterface->Query(returnedNodes, 0, 0, params);

					if (returnedNodes.size() == 1)
						{
						root = returnedNodes[0];
						break;
						}
					}

				queue<IScalableMeshNodePtr> nodes;
				nodes.push(root);
				
				//Folder name
				WString folderName = outputDir + L"\\";

				size_t level = root->GetLevel();
				while(level <= maxLevel)
					{
					IScalableMeshNodePtr currentNode = nodes.front();
					
					//Infos for 1 tile
					clock_t nodeClock = clock();
					int64_t pointCount = 0;
					int64_t paramCount = 0;
					int64_t pointIndexCount = 0;
					int64_t nodeId = currentNode->GetNodeId();

					//The node we're at
					WChar numberChar[10];
					swprintf(numberChar, L"%I64d", nodeId);
					WString number(numberChar);

					//File name
					WString materialName = number;
					WString binFileName = folderName + materialName + L".bin";

					//Get mesh
					bvector<bool> clips;
					IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
					if (currentNode->IsTextured())
						flags->SetLoadTexture(true);
					IScalableMeshMeshPtr mesh = currentNode->GetMesh(flags, clips);
					
					if (mesh != NULL)
						{
						//Bin file
						FILE* outBin;
						outBin = _wfopen(binFileName.c_str(), L"wb");

						const PolyfaceQuery* polyface = mesh->GetPolyfaceQuery();

						//Get infos
						pointCount = polyface->GetPointCount();
						paramCount = polyface->GetParamCount();
						pointIndexCount = polyface->GetPointIndexCount();

						IScalableMeshTexturePtr texture = currentNode->GetTexture();
						bool isTextured = currentNode->IsTextured();

						//write node id
						fwrite(&nodeId, sizeof(int64_t), 1, outBin);

						//write if textured
						fwrite(&isTextured, sizeof(bool), 1, outBin);

						//write v
						fwrite(&pointCount, sizeof(int64_t), 1, outBin);
						DPoint3dCP p = polyface->GetPointCP();
						double* points = new double[pointCount * 3];
						int j = 0;
						for (int64_t i = 0; i < pointCount; i++)
							{
							DPoint3d point = p[i];

							point.Add(translateToOrigin);

							points[j] = point.x;
							points[j + 1] = point.z;
							points[j + 2] = -point.y;
							j += 3;
							}
						fwrite(points, sizeof(double), pointCount * 3, outBin);

						//write uv
						if (isTextured)
							{
							fwrite(&paramCount, sizeof(int64_t), 1, outBin);
							DPoint2dCP param = polyface->GetParamCP();
							double* params = new double[paramCount * 2];
							j = 0;
							for (int64_t i = 0; i < paramCount; i++)
								{
								DPoint2d uv = param[i];

								params[j] = uv.x;
								params[j + 1] = uv.y;
								j += 2;
								}
							fwrite(params, sizeof(double), paramCount * 2, outBin);
							}

						//write faces
						fwrite(&pointIndexCount, sizeof(int64_t), 1, outBin);
						//vertices indice
						int32_t* facesV = new int32_t[pointIndexCount];
						for (int64_t i = 0; i < pointIndexCount; i += 3)
							{
							//zero-based index
							facesV[i] = polyface->GetPointIndexCP()[i] - 1;
							facesV[i + 1] = polyface->GetPointIndexCP()[i + 1] - 1;
							facesV[i + 2] = polyface->GetPointIndexCP()[i + 2] - 1;
							}
						fwrite(facesV, sizeof(int32_t), pointIndexCount, outBin);
						//uv indice
						if (isTextured)
							{
							int32_t* facesUV = new int32_t[pointIndexCount];
							for (int64_t i = 0; i < pointIndexCount; i += 3)
								{
								//zero-based index
								facesUV[i] = polyface->GetParamIndexCP()[i] - 1;
								facesUV[i + 1] = polyface->GetParamIndexCP()[i + 1] - 1;
								facesUV[i + 2] = polyface->GetParamIndexCP()[i + 2] - 1;
								}
							fwrite(facesUV, sizeof(int32_t), pointIndexCount, outBin);
							}

						//write texture
						if (isTextured)
							{
							const uint8_t* data = texture->GetData();
							fwrite(data, sizeof(byte), texture->GetSize(), outBin);
							}

						nodeClock = clock() - nodeClock;
						double delay = (double)nodeClock / CLOCKS_PER_SEC;

						fwprintf(pResultFile, L"%s,%I64d,%I64d,%zu,%.5f\n", materialName.c_str(), pointCount, paramCount, level, delay);

						//Close file for this tile
						fclose(outBin);
						}

					//get children nodes
					bvector<IScalableMeshNodePtr> childrenNodes = currentNode->GetChildrenNodes();
					for (auto child : childrenNodes)
						nodes.push(child);
					nodes.pop();
					
					level = currentNode->GetLevel();
					}//end while
				}
			else
				printf("Error loading stm file");
			}
		}
	}

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
        int64_t useCpuVal = 0;
        WString accelerator;
        status = pTestNode->GetAttributeStringValue(accelerator, "accelerator");
        if (status == BEXML_Success && 0 == BeStringUtilities::Wcsicmp(accelerator.c_str(), L"cpu"))
            {
            useCpuVal = 1;
            IScalableMeshATP::StoreInt(L"useCpu", useCpuVal);
            }
        int64_t useThreadInGeneration = 0;
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

        ScalableMeshMesherType mesherType = SCM_MESHER_2D_DELAUNAY;
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
            IScalableMeshATP::StoreDouble(WString(L"nTimeToCreateSeeds"), 0.0);
            IScalableMeshATP::StoreDouble(WString(L"nTimeToEstimateParams"), 0.0);
            IScalableMeshATP::StoreDouble(WString(L"nTimeToFilterGround"), 0.0);
            IScalableMeshATP::StoreInt(L"chosenAccelerator", 1);
            double nTimeToCreateSeeds = 0, nTimeToEstimateParams = 0, nTimeToFilterGround = 0;
            WString gcsKeyName;//

            auto status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

            if (status == BEXML_Success)
                {
                BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
                StatusInt status = creatorPtr->SetBaseGCS(baseGCSPtr);
                assert(status == SUCCESS);
                }//
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

                    int64_t pointCount = 0;
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
                        dgndbFileName.ReplaceAll(L".stm", L".bim");

                        FILE* f = _wfopen(dgndbFileName.c_str(), L"r");
                        assert(f != 0);
                        _fseeki64(f, 0, SEEK_END);
                        fileSize += _ftelli64(f);
                        fclose(f);
                        }

                    WString mesher = GetMesherTypeName(mesherType);
                    WString filter = GetFilterTypeName(filterType);
                    WString trimming = GetTrimmingTypeName(trimmingMethod);
                    int64_t acceleratorUseCpu = 0;
                    IScalableMeshATP::GetDouble(WString(L"nTimeToCreateSeeds"), nTimeToCreateSeeds);
                    IScalableMeshATP::GetDouble(WString(L"nTimeToEstimateParams"), nTimeToEstimateParams);
                    IScalableMeshATP::GetDouble(WString(L"nTimeToFilterGround"), nTimeToFilterGround);
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
                    fwprintf(pResultFile, L"%s,%s,%s,%.5f,%s\n", L"", L"", L"", 0.0, L"ERROR");
                    }
                fflush(pResultFile);
                }
            }
        }
    }

void PerformUpdateTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName;
    ScalableMeshMesherType mesherType = SCM_MESHER_2D_DELAUNAY;
    ScalableMeshFilterType filterType = SCM_FILTER_CGAL_SIMPLIFIER;
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
        WString suffixPartialUpdate(L"_partialUpdate");
        WString stmFileName_PartialUpdateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixPartialUpdate));
        StatusInt status;
        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPartialUpdatePtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status));
        WString suffixGenerate(L"_generate");
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
                            fwprintf(pResultFile, L",%s,%s\n", sourcePath.c_str(), updateType.c_str());
                            if (updateType.Equals(WString(L"Add")))
                                {
                                sourcesPartialUpdate.push_back(UpToDateState::ADD);
                                sourceToAdd.push_back(sourcePath);
                                }
                            else if (updateType.Equals(WString(L"Remove")))
                                sourcesPartialUpdate.push_back(UpToDateState::REMOVE);
                            else if (updateType.Equals(WString(L"Modify")))
                                sourcesPartialUpdate.push_back(UpToDateState::MODIFY);
                            else if (updateType.Equals(WString(L"UpToDate")))
                                sourcesPartialUpdate.push_back(UpToDateState::UP_TO_DATE);
                            if (!updateType.Equals(WString(L"Add")))
                                {
                                //IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                                IDTMSourcePtr srcPtr = IDTMLocalFileSource::Create(type, sourcePath.c_str());
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
                                    wprintf(L"ERROR : cannot add %s\r\n", sourcePath.c_str());
                                    break;
                                    }
                                }
                            if (!updateType.Equals(WString(L"Remove")))
                                {
                                //IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                                IDTMSourcePtr srcPtr = IDTMLocalFileSource::Create(type, sourcePath.c_str());
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
                                    wprintf(L"ERROR : cannot add %s\r\n", sourcePath.c_str());
                                    break;
                                    }
                                }
                            }
                        else
                            {
                            //IDTMSourcePtr srcPtr = CreateSourceFor(sourcePath, type);
                            IDTMSourcePtr srcPtr = IDTMLocalFileSource::Create(type, sourcePath.c_str());
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
                                wprintf(L"ERROR : cannot add %s\r\n", sourcePath.c_str());
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

            creatorPartialUpdatePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status);
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

            for (int i = 0; i < sourceToAdd.size(); i++)
                {
                IDTMSourcePtr srcPtr = IDTMLocalFileSource::Create(DTM_SOURCE_DATA_POINT, sourceToAdd[i].c_str());
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
                    wprintf(L"ERROR : cannot add %s\r\n", sourceToAdd[i].c_str());
                    break;
                    }
                }

            creatorPartialUpdatePtr->SaveToFile();
            creatorPartialUpdatePtr = 0;
            creatorPartialUpdatePtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName_PartialUpdateTest.c_str(), status);
            clock_t t = clock();
            status = creatorPartialUpdatePtr->Create();
            t = clock() - t;
            creatorPartialUpdatePtr->SaveToFile();
            assert(status == 0);
            creatorPartialUpdatePtr = 0;
            int64_t nOfPointsGen, nOfPointsUpdate;
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
    assert(false && "not implemented");
    /*    WString stmFileName;
        ScalableMeshMesherType mesherType = SCM_MESHER_3D_DELAUNAY;
        ScalableMeshFilterType filterType = SCM_FILTER_CGAL_SIMPLIFIER;
        int blossomMatching = 0;
        int indexMethod = 3;
        int trimmingMethod = 5;

        // Parses the test(s) definition:
        if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
            {
            printf("ERROR : stmFileName attribute not found\r\n");
            return;
            }
        else
            {
            StatusInt status;
            BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName.c_str(), status));

            if (creatorPtr == 0 || status != BSISUCCESS)
                {
                printf("ERROR : cannot create STM file\r\n");
                return;
                }
            BeXmlNodeP pSubNode = pTestNode->GetFirstChild();
            while (0 != pSubNode)
                {
                if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "mesher"))
                    {
                    int32_t mesherId;
                    StatusInt status = pSubNode->GetAttributeInt32Value(mesherId, "type");

                    if ((status == BEXML_Success) && (mesherId >= 0) && (mesherId < SCM_MESHER_QTY))
                        {
                        mesherType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesherType)mesherId;
                        }
                    else
                        {
                        printf("ERROR : invalid type attribute for mesher node\r\n");
                        }
                    WString blossomMatch;
                    status = pSubNode->GetAttributeStringValue(blossomMatch, "useBlossomMatching");

                    if (status == BEXML_Success)
                        {
                        if (BeStringUtilities::Wcsicmp(blossomMatch.c_str(), L"none")) {
                            blossomMatching = 0;
                            }
                        else if (BeStringUtilities::Wcsicmp(blossomMatch.c_str(), L"edgeOnly")) {
                            blossomMatching = 1;
                            }
                        else if (BeStringUtilities::Wcsicmp(blossomMatch.c_str(), L"faces")) {
                            blossomMatching = 2;
                            }
                        }

                    WString getIndexMethod;
                    status = pSubNode->GetAttributeStringValue(getIndexMethod, "indexMethod");

                    if (status == BEXML_Success)
                        {
                        if (BeStringUtilities::Wcsicmp(getIndexMethod.c_str(), L"centroids")) {
                            indexMethod = 0;
                            }
                        else if (BeStringUtilities::Wcsicmp(getIndexMethod.c_str(), L"bclib")) {
                            indexMethod = 1;
                            }
                        else if (BeStringUtilities::Wcsicmp(getIndexMethod.c_str(), L"classic")) {
                            indexMethod = 2;
                            }
                        }

                    WString getTrimmingMethod;
                    status = pSubNode->GetAttributeStringValue(getTrimmingMethod, "trimmingMethod");

                    if (status == BEXML_Success)
                        {
                        if (0 == BeStringUtilities::Wcsicmp(getTrimmingMethod.c_str(), L"method5")) {
                            trimmingMethod = 5;
                            }
                        else if (0 == BeStringUtilities::Wcsicmp(getTrimmingMethod.c_str(), L"umbrella")) {
                            trimmingMethod = 7;
                            }
                        }
                    }
                else if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "filter"))
                    {
                    int32_t filterId;
                    StatusInt status = pSubNode->GetAttributeInt32Value(filterId, "type");

                    if ((status == BEXML_Success) && (filterId >= 0) && (filterId < SCM_FILTER_QTY))
                        {
                        filterType = (BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshFilterType)filterId;
                        }
                    else
                        {
                        printf("ERROR : invalid type attribute for filter node\r\n");
                        }
                    }
                else if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "source"))
                    {
                    WString datasetPath;

                    StatusInt status = pSubNode->GetAttributeStringValue(datasetPath, "path");

                    if (status == BEXML_Success)
                        {
                        if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                            (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                            {
                            if (BSISUCCESS != creatorPtr->EditSources().Add(CreateSourceFor(datasetPath, DTM_SOURCE_DATA_POINT)))
                                {
                                wprintf(L"ERROR : cannot add %s\r\n", datasetPath.c_str());
                                break;
                                }
                            }
                        }
                    }
                pSubNode = pSubNode->GetNextSibling();
                }
            WChar mesherTypeChar[10];

            swprintf(mesherTypeChar, L"%i", mesherType);

            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_3D_MESHER_TYPE", mesherTypeChar);

            assert(defineStatus == SUCCESS);

            WChar filterTypeChar[10];

            swprintf(filterTypeChar, L"%i", filterType);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_FILTER_TYPE", filterTypeChar);

            assert(defineStatus == SUCCESS);

            WChar blossomMatchingChar[10];

            swprintf(blossomMatchingChar, L"%i", blossomMatching);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_BLOSSOM_MATCHING", blossomMatchingChar);

            assert(defineStatus == SUCCESS);

            WChar indexMethodChar[10];

            swprintf(indexMethodChar, L"%i", indexMethod);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_INDEX_METHOD", indexMethodChar);

            assert(defineStatus == SUCCESS);

            WChar trimmingMethodChar[10];

            swprintf(trimmingMethodChar, L"%i", trimmingMethod);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_TRIMMING_METHOD", trimmingMethodChar);

            assert(defineStatus == SUCCESS);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_ATP_MESHING_RESULT", L"1");

            assert(defineStatus == SUCCESS);
            // isSingleFile
            status = creatorPtr->Create();

            defineStatus = ConfigurationManager::UndefineVariable(L"SM_ATP_MESHING_RESULT");


            if (status == SUCCESS)
                {

                IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

                int64_t pointCount = 0;
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


                WString mesher = GetMesherTypeName(mesherType);
                WString filter = GetFilterTypeName(filterType);
                int64_t totalTri = 0;
                int64_t totalWrongTri = 0;
                double ratioWrongTri = 0.0;
                double quality = 0.0;
                double ratioMissingTri = 0.0;
                int64_t totalMissingTri = 0;
                IScalableMeshATP::GetInt(WString(L"trimCheck_numTrianglesChecked"), totalTri);
                IScalableMeshATP::GetInt(WString(L"trimCheck_numTrianglesWrong"), totalWrongTri);
                IScalableMeshATP::GetDouble(WString(L"trimCheck_ratioWrongTriangles"), ratioWrongTri);
                IScalableMeshATP::GetInt(WString(L"trimCheck_numTrianglesMissing"), totalMissingTri);
                IScalableMeshATP::GetDouble(WString(L"trimCheck_ratioMissingTriangles"), ratioMissingTri);
                IScalableMeshATP::GetDouble(WString(L"trimCheck_quality"), quality);
                IScalableMeshATP::StoreInt(WString(L"trimCheck_numTrianglesChecked"), 0);
                IScalableMeshATP::StoreInt(WString(L"trimCheck_numTrianglesWrong"), 0);
                IScalableMeshATP::StoreDouble(WString(L"trimCheck_ratioWrongTriangles"), 0.0);
                IScalableMeshATP::StoreInt(WString(L"trimCheck_numTrianglesMissing"), 0);
                IScalableMeshATP::StoreDouble(WString(L"trimCheck_ratioMissingTriangles"), 0.0);
                IScalableMeshATP::StoreDouble(WString(L"trimCheck_quality"), 0.0);
                IScalableMeshATP::StoreInt(WString(L"trimCheck_numGoodTriangles"), 0);
                // fwprintf(pResultFile, L"Test Case,%s,%s\n",stmFileName.c_str(),result.c_str());
                //  fwprintf(pResultFile, L"NOfPoints, Mesher, Filter, Use Blossom?, Index Method, Total Triangles Meshed, Total Wrong Triangles, Wrong Triangles(%%), Total Triangles Missing,Missing Triangles (%%), Quality(higher is better)\n");
                fwprintf(pResultFile, L"%s,%s,%I64d,%s,%s,%s,%s,%s,%I64d,%I64d,%.5f,%I64d,%.5f,%.5f\n",
                         stmFileName.c_str(),
                         result.c_str(),
                         IScalableMeshSourceCreator::GetNbImportedPoints(),
                         mesher.c_str(),
                         filter.c_str(),
                         GetBlossomMatchingValue(blossomMatching).c_str(),
                         GetIndexMethodValue(indexMethod).c_str(),
                         GetTrimmingMethodValue(trimmingMethod).c_str(),
                         totalTri,
                         totalWrongTri,
                         ratioWrongTri * 100,
                         totalMissingTri,
                         ratioMissingTri * 100,
                         quality);
                }
            else
                {
                fwprintf(pResultFile, L"Test Case,%s,%s\n", stmFileName.c_str(), L"ERROR");
                }
            fflush(pResultFile);
            }*/
    }

void Perform2DStitchQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "not implemented");
    /*    WString stmFileName;
        ScalableMeshMesherType mesherType = SCM_MESHER_2D_DELAUNAY;
        ScalableMeshFilterType filterType = SCM_FILTER_CGAL_SIMPLIFIER;

        // Parses the test(s) definition:
        if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
            {
            printf("ERROR : stmFileName attribute not found\r\n");
            return;
            }
        else
            {
            StatusInt status;
            Bentley::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(Bentley::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName.c_str(), status));

            if (creatorPtr == 0 || status != BSISUCCESS)
                {
                printf("ERROR : cannot create STM file\r\n");
                return;
                }
            BeXmlNodeP pSubNode = pTestNode->GetFirstChild();
            while (0 != pSubNode)
                {
                if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "filter"))
                    {
                    Int32 filterId;
                    StatusInt status = pSubNode->GetAttributeInt32Value(filterId, "type");

                    if ((status == BEXML_Success) && (filterId >= 0) && (filterId < SCM_FILTER_QTY))
                        {
                        filterType = (Bentley::ScalableMesh::ScalableMeshFilterType)filterId;
                        }
                    else
                        {
                        printf("ERROR : invalid type attribute for filter node\r\n");
                        }
                    }
                else if (0 == BeStringUtilities::Stricmp(pSubNode->GetName(), "source"))
                    {
                    WString datasetPath;

                    StatusInt status = pSubNode->GetAttributeStringValue(datasetPath, "path");

                    if (status == BEXML_Success)
                        {
                        if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                            (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                            {
                            if (BSISUCCESS != creatorPtr->EditSources().Add(CreateSourceFor(datasetPath, DTM_SOURCE_DATA_POINT)))
                                {
                                wprintf(L"ERROR : cannot add %s\r\n", datasetPath);
                                break;
                                }
                            }
                        }
                    }
                pSubNode = pSubNode->GetNextSibling();
                }

            WChar mesherTypeChar[10];

            swprintf(mesherTypeChar, L"%i", mesherType);

            BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_3D_MESHER_TYPE", mesherTypeChar);

            assert(defineStatus == SUCCESS);
            WChar filterTypeChar[10];

            swprintf(filterTypeChar, L"%i", filterType);

            defineStatus = ConfigurationManager::DefineVariable(L"SM_FILTER_TYPE", filterTypeChar);

            assert(defineStatus == SUCCESS);
            // isSingleFile
            status = creatorPtr->Create();


            if (status == SUCCESS)
                {

                IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

                int64_t pointCount = 0;
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

                // get all points from full res, adapted from MrDTMDataRef::GetDtmForSingleResolution
                IScalableMeshFixResolutionIndexQueryParamsPtr queryParamsPtr(IScalableMeshFixResolutionIndexQueryParams::CreateParams());
                queryParamsPtr->SetResolutionIndex(stmFile->GetNbResolutions() - 1);

                IScalableMeshPointQueryPtr fixResPointQueryPtr(stmFile->GetQueryInterface(SCM_QUERY_FIX_RESOLUTION_VIEW));
                bvector<DPoint3d> points;
                status = fixResPointQueryPtr->Query(points, 0, 0, IScalableMeshQueryParametersPtr(queryParamsPtr));

                BcDTMPtr bcDtmInMemPtr(BcDTM::Create());

                bcDtmInMemPtr->AddPoints(points);
                bcDtmInMemPtr->SetTriangulationParameters(0, 0, 0, 0);
                bcDtmInMemPtr->Triangulate();

                ITiledTriangulatorValidatorPtr tiledTriangulatorValidatorPtr = ITiledTriangulatorValidator::CreateFor(bcDtmInMemPtr);

                //get meshes from all nodes
                IScalableMeshMeshQueryPtr meshQueryInterface = stmFile->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
                bvector<IScalableMeshNodePtr> returnedNodes;
                IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
                meshQueryInterface->Query(returnedNodes, 0, 0, params);
                DRange3d nodeExtent;
                unsigned __int64 totalNbOverlappingTriangles = 0;
                for (auto& node : returnedNodes) {
                    nodeExtent = node->GetNodeExtent();
                    DTMPtr tileDTMPtr;
                    BC_DTM_OBJ* bcDtmP = 0;
                    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
                    if (dtmCreateStatus == 0)
                        {
                        BcDTMPtr bcDtmObjPtr;
                        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
                        tileDTMPtr = bcDtmObjPtr.get();
                        }

                    BC_DTM_OBJ* dtmObjP(tileDTMPtr->GetBcDTM()->GetTinHandle());
                    bvector<bool> clips;
                    IScalableMeshMeshPtr mesh = node->GetMesh(false, clips);
                    const Bentley::PolyfaceQuery* polyface = mesh->GetPolyfaceQuery();
                    DPoint3d triangle[4];
                    int status = SUCCESS;
                    //bvector<DSegment3d> allEdges;
                    for (size_t i = 0; i < polyface->GetPointIndexCount() && status == SUCCESS; i += 3)
                        {
                        triangle[0] = polyface->GetPointCP()[polyface->GetPointIndexCP()[i] - 1];
                        triangle[1] = polyface->GetPointCP()[polyface->GetPointIndexCP()[i + 1] - 1];
                        triangle[2] = polyface->GetPointCP()[polyface->GetPointIndexCP()[i + 2] - 1];
                        triangle[3] = triangle[0];
                        //if(!allTriangles && (!bsiDRange3d_isDPoint3dContained(&nodeExtent, triangle) || !bsiDRange3d_isDPoint3dContained(&nodeExtent, triangle+1)|| !bsiDRange3d_isDPoint3dContained(&nodeExtent, triangle+2))) continue;
                        /*if(!accumulateEdgesAndTestIntersection(allEdges, triangle))
                        {
                        totalNbOverlappingTriangles++;
                        continue;
                        }*/
                        /*                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &triangle[0], 4);
                                            //}
                                            }
                                        bcdtmObject_triangulateStmTrianglesDtmObject(dtmObjP);
                                        tiledTriangulatorValidatorPtr->CompareMemDTMwithTileDTM(tileDTMPtr);
                                        }
                                    unsigned __int64 totalNbComparedTiles;
                                    unsigned __int64 totalNbWrongTiles;
                                    unsigned __int64 totalNbComparedTriangles;
                                    unsigned __int64 totalNbWrongTriangles;

                                    tiledTriangulatorValidatorPtr->GetTotalStat(totalNbComparedTiles,
                                                                                totalNbWrongTiles,
                                                                                totalNbComparedTriangles,
                                                                                totalNbWrongTriangles);

                                    WString filter = GetFilterTypeName(filterType);
                                    double ratioWrongTri = (double)totalNbWrongTriangles / totalNbComparedTriangles;
                                    //Test Case Name, status, n of points, filter, total tiles, total wrong tiles, total triangles, total wrong triangles, %wrong
                                    fwprintf(pResultFile, L"%s,%s,%I64d,%s,%I64u,%I64u,%I64u,%I64u,%.5f,%I64u\n",
                                             stmFileName.c_str(),
                                             result.c_str(),
                                             IScalableMeshSourceCreator::GetNbImportedPoints(),
                                             filter.c_str(),
                                             totalNbComparedTiles,
                                             totalNbWrongTiles,
                                             totalNbComparedTriangles,
                                             totalNbWrongTriangles,
                                             ratioWrongTri * 100.0,
                                             totalNbOverlappingTriangles);

                                    }
                                else
                                    {
                                    fwprintf(pResultFile, L"%s,%s\n", stmFileName.c_str(), L"ERROR");
                                    }
                                fflush(pResultFile);
                                }*/
    }

/*int CollectAllElmsCallback
(
    ElementRefP      elemRef,
    void            *callbackArg,
    ScanCriteria    *scP
    )
    {
    DgnModelRefP modelP(scP->GetModelRef());
    ElementAgenda* agendaP = reinterpret_cast<ElementAgenda*>(callbackArg);

    agendaP->Insert(elemRef, modelP);

    return SUCCESS;
    }*/

struct DPoint3dComparer
    {
    private:
        DPoint3d m_start;
        DPoint3d m_end;
    public:
        DPoint3dComparer()
            {
            }
        DPoint3dComparer(DPoint3d& start, DPoint3d& end)
            {
            SetStart(start); SetEnd(end);
            }
        void SetStart(DPoint3d& start)
            {
            m_start = start;
            }
        void SetEnd(DPoint3d& end)
            {
            m_end = end;
            }
        bool operator() (DPoint3d& X1, DPoint3d& X2)
            {
            DPoint3d direction;
            direction.x = m_end.x - m_start.x;
            direction.y = m_end.y - m_start.y;
            direction.z = m_end.z - m_start.z;
            // First point
            DPoint3d X1direction;
            X1direction.x = X1.x - m_start.x;
            X1direction.y = X1.y - m_start.y;
            X1direction.z = X1.z - m_start.z;
            const double X1RelativePosition = X1direction.x*direction.x + X1direction.y*direction.y + X1direction.y*direction.y;
            // Second point
            DPoint3d X2direction;
            X2direction.x = X2.x - m_start.x;
            X2direction.y = X2.y - m_start.y;
            X2direction.z = X2.z - m_start.z;
            const double X2RelativePosition = X2direction.x*direction.x + X2direction.y*direction.y + X2direction.y*direction.y;
            return X1RelativePosition < X2RelativePosition;
            }
    };

StatusInt DrapeOnScalableMesh(DTMPtr& smPtr, std::vector<std::vector<DPoint3d>>& drapedPoints, std::vector<DPoint3d> origPoints)
    {
    DTMDrapedLinePtr drapedLine;
    auto draping = smPtr->GetDTMDraping();

    draping->DrapeLinear(drapedLine, origPoints.data(), (int)origPoints.size());
    if (drapedLine.IsNull())
        return ERROR;
    //        delete draping;
    unsigned int numPoints = drapedLine->GetPointCount();
    drapedPoints.resize(1);
    for (unsigned int ptNum = 0; ptNum < numPoints; ptNum++)
        {
        DPoint3d pt;
        drapedLine->GetPointByIndex(&pt, nullptr, nullptr, ptNum);
        drapedPoints[0].push_back(pt);
        }

    return SUCCESS;
    }

StatusInt DoBatchDrape(vector<vector<DPoint3d>>& lines, DTMPtr& dtmPtr, vector<vector<DPoint3d>>& drapeLines)
    {
    bool aborded = false;

    int numElemDraped = 0;
    int numElemNOTDraped = 0;
    int numPartial = 0;
    int numPts = 0;
    double drapeLength = 0.0;

    clock_t timer = clock();
    for (auto& origPoints : lines)
        {
        StatusInt status;
        std::vector<std::vector<DPoint3d>> drapedPoints(origPoints.size() - 1);
        status = DrapeOnScalableMesh(dtmPtr, drapedPoints, origPoints);
        if (status == ERROR)
            {
            numElemNOTDraped++;
            continue;
            }
        numElemDraped++;

        vector<DPoint3d> drapeLine;

        // Sort draped points on line and remove duplicates...
        for (unsigned int line = 0; line < origPoints.size() - 1; ++line)
            {
            sort(drapedPoints[line].begin(), drapedPoints[line].end(), DPoint3dComparer(origPoints[line], origPoints[line + 1]));
            std::vector<DPoint3d>::iterator it = unique(drapedPoints[line].begin(), drapedPoints[line].end(), DPoint3dEqualityTest);
            drapedPoints[line].resize(std::distance(drapedPoints[line].begin(), it));
            }

        // Add to final draped points vector
        for (unsigned int line = 0; line < drapedPoints.size(); line++)
            for (unsigned int i = 0; i < drapedPoints[line].size(); i++)
                drapeLine.push_back(drapedPoints[line][i]);

        vector<DPoint3d>::iterator it = unique(drapeLine.begin(), drapeLine.end(), DPoint3dEqualityTest);
        drapeLine.resize(std::distance(drapeLine.begin(), it));

        //reporting of partial drapes
        if (status == SUCCESS && drapeLine.size() == 0)
            {
            numPartial++;
            }
        else if (status == SUCCESS)
            {
            DPoint3d pt = drapeLine.back();
            if (fabs(origPoints[origPoints.size() - 1].x - pt.x) < 1 && fabs(origPoints[origPoints.size() - 1].y - pt.y) < 1)
                {
                drapeLength += 1.0;
                }
            else
                {
                numPartial++;

                DPoint3d intersectPt1 = pt;
                intersectPt1.z = origPoints[origPoints.size() - 1].z;
                drapeLength += fabs(DVec3d::FromStartEnd(origPoints[origPoints.size() - 2], intersectPt1).MagnitudeSquared() / DVec3d::FromStartEnd(origPoints[origPoints.size() - 2], origPoints[origPoints.size() - 1]).MagnitudeSquared());
                }
            }
        numPts += (int)drapeLine.size();
        drapeLines.push_back(drapeLine);
        }

    timer = clock() - timer;

    float secs;
    secs = ((float)timer) / CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(L"drapeTime", secs);
    drapeLength /= numElemDraped;
    IScalableMeshATP::StoreInt(L"nOfLines", numElemDraped + numElemNOTDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesNotDraped", numElemNOTDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesDraped", numElemDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesPartial", numPartial);
    IScalableMeshATP::StoreInt(L"nOfOutputPoints", numPts);
    IScalableMeshATP::StoreDouble(L"lengthOfLinesPartial", drapeLength);
    StatusInt status = aborded;

    return status;
    }

//void ExportDrapeLine(BeXmlNodeP pTestNode, FILE* pResultFile)
//    {
//    WString linesFileName, name, outputname;
//    if (pTestNode->GetAttributeStringValue(outputname, "outputFileName") != BEXML_Success)
//        {
//        printf("ERROR : outputFileName attribute not found\r\n");
//        return;
//        }
//    // parse dgn file name
//    if (pTestNode->GetAttributeStringValue(linesFileName, "linesFileName") != BEXML_Success)
//        {
//        printf("ERROR : linesFileName attribute not found\r\n");
//        return;
//        }
//    if (pTestNode->GetAttributeStringValue(name, "name") != BEXML_Success)
//        {
//        printf("ERROR : name attribute not found\r\n");
//        }
//    DgnFileStatus fileOpenStatus;
//    DgnDocumentPtr lineDoc = DgnDocument::CreateFromFileName(fileOpenStatus, linesFileName.c_str(), NULL, DEFDGNFILE_ID, DgnDocument::FetchMode::Read);
//
//    DgnModelRefP model = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef();
//    DgnAttachment* newAttachment;
//    model->CreateDgnAttachment(newAttachment, *lineDoc->GetMonikerPtr(), L"");
//    ElementAgenda agenda;
//    LevelCache& levelCache = newAttachment->GetLevelCacheR();
//
//    LevelHandle level = levelCache.GetLevelByName(L"toDrape", false);
//    LevelId levelId = level.GetLevelId();
//
//    ScanCriteriaP scP = new ScanCriteria();
//    scP->SetModelRef(newAttachment);
//    scP->SetReturnType(MSSCANCRIT_ITERATE_ELMREF, false, true);
//    scP->SetElemRefCallback(CollectAllElmsCallback, &agenda);
//    BitMaskP levelBitMask = BitMask::Create(false);
//    levelBitMask->SetBit(levelId - 1, 1);
//    scP->SetLevelTest(levelBitMask, false);
//    scP->Scan();
//
//    bvector<bvector<DPoint3d>> pts;
//    bvector<bvector<DPoint3d>> lines;
//    EditElementHandleP curr = agenda.GetFirstP();
//    EditElementHandleP end = curr + agenda.GetCount();
//
//    Json::Value block;
//    for (; curr < end; curr++) // For each valid element we do draping
//        {
//        ElementHandle elemHandle = *curr;
//        bvector<DPoint3d> origPoints;
//        MSElementCP element = elemHandle.GetElementCP();
//        switch (elemHandle.GetElementType())
//            {
//            case LINE_ELM:
//                {
//                origPoints.push_back(element->line_3d.start);
//                origPoints.push_back(element->line_3d.end);
//                break;
//                }
//            case LINE_STRING_ELM:
//                {
//                origPoints.resize(element->point_string_3d.numpts);
//                memcpy(&origPoints[0], &element->point_string_3d.point[0], element->point_string_3d.numpts * sizeof(DPoint3d));
//                break;
//                }
//            default:
//                break;
//            }
//
//        Transform refToActiveTrf;
//        GetFromModelRefToActiveTransform(refToActiveTrf, elemHandle.GetModelRef());
//        bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &origPoints[0], (int)origPoints.size());
//        Transform uorToMeter;
//        Transform meterToUor;
//        GetTransformForPoints(uorToMeter, meterToUor);
//        
//        DPoint3d ptGO;
//        ModelInfoCP modelInfo = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP();
//        if (SUCCESS != modelInfo->GetGlobalOrigin(modelInfo, &ptGO))
//            ptGO.x = ptGO.y = ptGO.z = 0.;
//
//        Transform transTrf;
//        transTrf.InitIdentity();
//        transTrf.SetTranslation(ptGO);
//        transTrf.InverseOf(transTrf);
//
//        bsiTransform_multiplyDPoint3dArrayInPlace(&transTrf, (DPoint3dP)&origPoints[0], (int)origPoints.size());
//
//        // Get the coordinates of the line in meter
//        bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, (DPoint3dP)&origPoints[0], (int)origPoints.size());
//        lines.push_back(origPoints);
//
//        //auto& children = block.append("line");
//        //auto& children = block["lines"];
//        Json::Value children;
//        size_t nbChildren = origPoints.size();
//        for (size_t childInd = 0; childInd < nbChildren; childInd++)
//            {
//            //Json::Value& child = childInd >= nbChildren ? children.append(Json::Value()) : children["point"];
//            //Json::Value& child = children.append("point");
//            //child["x"] = origPoints[childInd].x;
//            //child["y"] = origPoints[childInd].y;
//            //child["z"] = origPoints[childInd].z;
//            Json::Value child;
//            Json::Value points;
//            points["x"] = origPoints[childInd].x;
//            points["y"] = origPoints[childInd].y;
//            points["z"] = origPoints[childInd].z;
//            /*points["x].push_back(origPoints[childInd].x);
//            points.push_back(origPoints[childInd].y);
//            points.push_back(origPoints[childInd].z);*/
//            child["point"] = points;
//            children.append(child);
//            }
//        block["lines"].append(children);
//        }
//
//    uint64_t buffer_size;
//    auto jsonWriter = [&buffer_size](BeFile& file, Json::Value& object)
//        {
//        Json::StyledWriter writer;
//        auto buffer = writer.write(object);
//        buffer_size = buffer.size();
//        file.Write(NULL, buffer.c_str(), buffer_size);
//        };
//    BeFile file;
//    if (BeFileStatus::Success == file.Open(outputname.c_str(), BeFileAccess::Write, BeFileSharing::None))
//        jsonWriter(file, block);
//    else if (BeFileStatus::Success == file.Create(outputname.c_str()))
//        jsonWriter(file, block);
//    else
//        HASSERT(!"Problem creating master header file");
//    file.Close();
//    }

void PerformGroupNodeHeaders(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString scmFileName, outputDir, result;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(scmFileName, "scmFileName") != BEXML_Success)
        {
        printf("ERROR : scmFileName attribute not found\r\n");
        return;
        }
    if (pTestNode->GetAttributeStringValue(outputDir, "outputDir") != BEXML_Success)
        {
        printf("ERROR : outputDir attribute not found\r\n");
        return;
        }

    double t = clock();

    // Check existence of scm file
    StatusInt status;
    IScalableMeshPtr scmFile = IScalableMesh::GetFor(scmFileName.c_str(), false, false, status);

    if (scmFile != 0 && status == SUCCESS)
        {
        status = scmFile->SaveGroupedNodeHeaders(outputDir);
        if (SUCCESS != status) result = L"FAILURE -> could not group node headers";
        }
    else
        {
        result = L"FAILURE -> could not open scm file";
        }
    t = clock() - t;

    fwprintf(pResultFile, L"%s,%s,%0.5f\n",
             scmFileName.c_str(),
             result.c_str(),
             (double)t / CLOCKS_PER_SEC
             );

    fflush(pResultFile);

    }

void AddTexturesToMesh(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString scmFileName, cloudOutDirPath, result;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(scmFileName, "scmFileName") != BEXML_Success)
        {
        printf("ERROR : scmFileName attribute not found\r\n");
        return;
        }

    double t = clock();

    // copy file helper
    auto copy_file = [](const WString& source, const WString& dest)
        {
        _wremove(dest.c_str());
        ifstream ss(source.c_str(), ios::binary);
        ofstream ds(dest.c_str(), ios::binary);
        istreambuf_iterator<char> begin_source(ss);
        istreambuf_iterator<char> end_source;
        ostreambuf_iterator<char> begin_dest(ds);
        copy(begin_source, end_source, begin_dest);
        ss.close();
        ds.close();
        };

    // setup data for adding textures (cleanup, save original, etc)
    //auto position = scmFileName.find_last_of(L".stm");
    //auto filenameWithoutExtension = scmFileName.substr(0, position - 3);
    //WString texturedFileName = filenameWithoutExtension + L"_textured.stm";

    //copy_file(scmFileName, texturedFileName);

    // Check existence of scm file
    StatusInt status;
    IScalableMeshPtr scmFile = IScalableMesh::GetFor(scmFileName.c_str(), false, false, status);

    if (scmFile != 0 && status == SUCCESS)
        {
        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr sourceCreator(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(scmFile, status));

        if (sourceCreator != 0 && status == SUCCESS)
            {
            if (ParseSourceSubNodes(sourceCreator->EditSources(), pTestNode) == true)
                {
                sourceCreator->ImportRastersTo(scmFile);

                result = L"SUCCESS";
                }
            else {
                result = L"FAILURE -> could not parse sources from xml file";
                }
            }
        else
            {
            result = L"FAILURE -> could not get scalable mesh sources";
            }
        }
    else
        {
        result = L"FAILURE -> could not open scm file";
        }
    t = clock() - t;

    fwprintf(pResultFile, L"%s,%s,%0.5f\n",
             scmFileName.c_str(),
             result.c_str(),
             (double)t / CLOCKS_PER_SEC
             );

    fflush(pResultFile);
    }
void PerformDrapeLineTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, linesFileName, name;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }
    StatusInt status;
    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

    uint64_t pointCount = 0;
    WString result;

    if (stmFile != 0)
        {
        pointCount = stmFile->GetPointCount();
        result = L"SUCCESS";
        }
    else
        {
        result = L"FAILURE";
        }
    //parse dgn file name
    if (pTestNode->GetAttributeStringValue(linesFileName, "linesFileName") != BEXML_Success)
        {
        printf("ERROR : linesFileName attribute not found\r\n");
        return;
        }
    IScalableMeshATP::StoreInt(L"nOfGraphLoadAttempts", 0);
    IScalableMeshATP::StoreInt(L"nOfGraphStoreMisses", 0);

    BeFile file;
    if (BeFileStatus::Success != file.Open(linesFileName.c_str(), BeFileAccess::Read))
        {
        return;
        }
    char* linesFileBuffer = nullptr;
    size_t fileSize;
    file.GetSize(fileSize);
    linesFileBuffer = new char[fileSize];
    uint32_t bytes_read;
    file.Read(linesFileBuffer, &bytes_read, (uint32_t)fileSize);
    assert(bytes_read == fileSize);
    file.Close();

    Json::Reader reader;
    Json::Value root;
    reader.parse(linesFileBuffer, linesFileBuffer + bytes_read, root);
    Json::Value jsonLines = root["lines"];

    vector<vector<DPoint3d>> pts;
    vector<vector<DPoint3d>> lines;

    for (const auto& line : jsonLines)
        {
        vector<DPoint3d> origPoints;
        for (const auto& jsonObject : line)
            {
            Json::Value jsonPoint = jsonObject["point"];
            DPoint3d point;
            point.x = jsonPoint["x"].asDouble();
            point.y = jsonPoint["y"].asDouble();
            point.z = jsonPoint["z"].asDouble();
            origPoints.push_back(point);
            }
        if (!origPoints.empty())
            lines.push_back(origPoints);
        }
    DTMPtr dtmP = dynamic_cast<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*>(&*stmFile->GetDTMInterface());

    status = DoBatchDrape(lines, dtmP, pts);

    int64_t nOfLinesToDrape = 0, nOfLinesDraped = 0, nOfLinesNotDraped = 0;
    double timeOfDrape = 0.0;
    IScalableMeshATP::GetInt(L"nOfLines", nOfLinesToDrape);
    IScalableMeshATP::GetInt(L"nOfLinesNotDraped", nOfLinesNotDraped);
    IScalableMeshATP::GetInt(L"nOfLinesDraped", nOfLinesDraped);
    IScalableMeshATP::GetDouble(L"drapeTime", timeOfDrape);
    int64_t loadAttempts;
    int64_t loadMisses;
    int64_t nOutPts = 0;
    int64_t nPartial = 0;
    double drapeLength = 0.0;
    IScalableMeshATP::GetInt(L"nOfGraphLoadAttempts", loadAttempts);
    IScalableMeshATP::GetInt(L"nOfGraphStoreMisses", loadMisses);
    IScalableMeshATP::GetInt(L"nOfLinesPartial", nPartial);
    IScalableMeshATP::GetInt(L"nOfOutputPoints", nOutPts);
    IScalableMeshATP::GetDouble(L"lengthOfLinesPartial", drapeLength);
    fwprintf(pResultFile, L"%s,%s,%I64d,%I64d,%I64d,%I64d,%.5f,%.5f,%.5f,%I64d,%.5f,%.5f,%.5f,%I64d, %.5f\n",
             stmFileName.c_str(),
             result.c_str(),
             stmFile->GetPointCount(),
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
    IScalableMeshATP::StoreInt(L"nOfLines", 0);
    IScalableMeshATP::StoreInt(L"nOfLinesNotDraped", 0);
    IScalableMeshATP::StoreInt(L"nOfLinesDraped", 0);
    IScalableMeshATP::StoreInt(L"nOfOutputPoints", 0);
    IScalableMeshATP::StoreDouble(L"drapeTime", 0.0);
    IScalableMeshATP::StoreDouble(L"lengthOfLinesPartial", 0.0);
    IScalableMeshATP::StoreInt(L"nOfLinesPartial", 0);
    fflush(pResultFile);
    stmFile = nullptr;
    dtmP = nullptr;
    }

//int CollectFirstMeshElement
//    (
//        ElementRefP      elemRef,
//        void            *callbackArg,
//        ScanCriteria    *scP
//        )
//    {
//    DgnModelRefP modelP(scP->GetModelRef());
//    ElementAgenda* agendaP = reinterpret_cast<ElementAgenda*>(callbackArg);
//    //if (MESH_HEADER_ELM == elementRef_getElemType(elemRef))
//    if (MESH_HEADER_ELM == elemRef->GetElementType())
//        agendaP->Insert(elemRef, modelP);
//
//    return SUCCESS;
//    }

//void ImportVolume(BeXmlNodeP pTestNode, FILE* pResultFile)
//    {
//    //assert(false && "Not ported yet! Perhaps we could use your help?");
//    WString stmFileName, importFileName;
//    // Parses the test(s) definition:
//    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
//        {
//        printf("ERROR : stmFileName attribute not found\r\n");
//        return;
//        }
//    StatusInt status;
//    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);
//
//    int64_t pointCount = 0;
//    WString result;
//
//    if (stmFile != 0)
//        {
//        pointCount = stmFile->GetPointCount();
//        result = L"SUCCESS";
//        }
//    else
//        {
//        result = L"FAILURE";
//        }
//    if (pTestNode->GetAttributeStringValue(importFileName, "importFileName") != BEXML_Success)
//        {
//        printf("ERROR : importFileName attribute not found\r\n");
//        return;
//        }
//
//    BeFile file;
//    if (BeFileStatus::Success != OPEN_FILE(file, importFileName.c_str(), BeFileAccess::Read))
//        {
//        //printf("ERROR : can't open file : %s", linesFileName);
//        return;
//        }
//    char* meshFileBuffer = nullptr;
//    size_t fileSize;
//    file.GetSize(fileSize);
//    meshFileBuffer = new char[fileSize];
//    uint32_t bytes_read;
//    file.Read(meshFileBuffer, &bytes_read, (uint32_t)fileSize);
//    assert(bytes_read == fileSize);
//    file.Close();
//
//    Json::Reader reader;
//    Json::Value mesh;
//    reader.parse(meshFileBuffer, meshFileBuffer + bytes_read, mesh);
//
//    DRange3d elemRange;
//    elemRange.high.x = mesh["elemRange"]["high.x"].asDouble();
//    elemRange.high.y = mesh["elemRange"]["high.y"].asDouble();
//    elemRange.high.z = mesh["elemRange"]["high.z"].asDouble();
//    elemRange.low.x = mesh["elemRange"]["low.x"].asDouble();
//    elemRange.low.y = mesh["elemRange"]["low.y"].asDouble();
//    elemRange.low.z = mesh["elemRange"]["low.z"].asDouble();
//
//    size_t pointSize = mesh["mesh"]["pointCount"].asInt();
//    DPoint3dP points = new DPoint3d[pointSize];
//    int i = 0;
//    for (const auto& jsonObject : mesh["mesh"]["Points"])
//        {
//        DPoint3d point;
//        point.x = jsonObject["Point"][0].asDouble();
//        point.y = jsonObject["Point"][1].asDouble();
//        point.z = jsonObject["Point"][2].asDouble();
//        points[i++] = point;
//        }
//
//    size_t pointIndexSize = mesh["mesh"]["pointIndexCount"].asInt();
//    int32_t* pointsIndex = new int32_t[pointIndexSize];
//    for (int i = 0; i < pointIndexSize; i++)
//        {
//        int32_t id = mesh["mesh"]["PointsIndex"][i].asInt();
//        pointsIndex[i] = id;
//        }
//
//    PolyfaceQuery* poly = new PolyfaceQueryCarrier(3, false/*twoSided*/, pointIndexSize, pointSize, points, pointsIndex);
//    PolyfaceHeaderPtr meshData;
//    IFacetOptionsPtr options = IFacetOptions::Create();
//    options->SetMaxPerFace(3);
//    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*options);
//    builder->AddPolyface(*poly);
//    meshData = builder->GetClientMeshPtr();
//
//    double cut = 0.0, fill = 0.0, volume = 0.0;
//    double cutValidate = 0.0, fillValidate = 0.0, volumeValidate = 0.0;
//    double cutError = 0.0, fillError = 0.0, volumeError = 0.0, avgError = 0.0;
//    double secs = 0.0;
//    double   cutConnected, fillConnected, volConnected, stitchCut, stitchFill, stitchVol;
//    cutConnected = fillConnected = volConnected = stitchCut = stitchFill = stitchVol = 0.0;
//
//    {
//    IScalableMeshATP::StoreInt(L"nTiles", 0);
//    IScalableMeshATP::StoreInt(L"nNoCutFillTiles", 0);
//    IScalableMeshATP::StoreInt(L"nSectionsTotal", 0);
//    IScalableMeshATP::StoreInt(L"nFailedComputePrincipalMoments", 0);
//    bvector<PolyfaceHeaderPtr> volumeMesh;
//    status = ComputeVolumeForAgenda(/*elemRange,*/ meshData, stmFile, cut, fill, volume/*, volumeMesh*/);
//
//
//
//    /*    for (int i = 0; i < volumeMesh.size(); i++)
//    {
//    //Transform uorToMeter, meterToUor;
//    //GetTransformForPoints(uorToMeter, meterToUor);
//    //Transform refToActiveTrf;
//    //GetFromModelRefToActiveTransform(refToActiveTrf, ((EditElementHandleP)agenda.GetFirstP())->GetModelRef());
//    //volumeMesh[i]->Transform(meterToUor);
//    //volumeMesh[i]->Transform(refToActiveTrf);
//    //MSElementDescr* desc = NULL;
//    //mdlMesh_newPolyfaceFromEmbeddedArraysExt(&desc, NULL, &volumeMesh[i]->PointIndex(), 0, &volumeMesh[i]->Point(), (0 == volumeMesh[i]->NormalIndex().size() ? NULL : &volumeMesh[i]->NormalIndex()), (0 == volumeMesh[i]->Normal().size() ? NULL : (EmbeddedDPoint3dArray*) &volumeMesh[i]->Normal()), (0 == volumeMesh[i]->ParamIndex().size() ? NULL : &volumeMesh[i]->ParamIndex()), (0 == volumeMesh[i]->Param().size() ? NULL : (EmbeddedDPoint2dArray*)&volumeMesh[i]->Param()), NULL, NULL, NULL, NULL, false, true);
//    //mdlMesh_newPolyfaceFromEmbeddedArraysExt(&desc, NULL, &volumeMesh[i]->PointIndex(), 0, &volumeMesh[i]->Point(), (0 == volumeMesh[i]->NormalIndex().size() ? NULL : &volumeMesh[i]->NormalIndex()), (0 == volumeMesh[i]->Normal().size() ? NULL : (EmbeddedDPoint3dArray*) &volumeMesh[i]->Normal()), (0 == volumeMesh[i]->ParamIndex().size() ? NULL : &volumeMesh[i]->ParamIndex()), (0 == volumeMesh[i]->Param().size() ? NULL : (EmbeddedDPoint2dArray*)&volumeMesh[i]->Param()), NULL, NULL, NULL, NULL, false, true);
//    //                mdlElmdscr_add(desc);
//    //EditElementHandle handle(desc, true, true, mdlModelRef_getActive());
//    //handle.AddToModel();
//    /*mdlElmdscr_add(desc);
//    EditElementHandle handle(pNewElmDsc, true, true, model); 
//    handle.AddToModel();*/
//    //        }
//
//    if (status != SUCCESS)
//        {
//        result = L"FAILED TO COMPUTE";
//        }
//    else
//        {
//        clock_t timer = clock();
//        IScalableMeshMeshQueryPtr meshQueryInterface = ((IScalableMesh*)stmFile.get())->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
//        bvector<IScalableMeshNodePtr> returnedNodes;
//        IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
//        DRange3d fileRange;
//        stmFile->GetRange(fileRange);
//        //Transform uorToMeter, meterToUor;
//
//        DPoint3d box[4] = {
//            DPoint3d::From(elemRange.low.x, elemRange.low.y, fileRange.low.z),
//            DPoint3d::From(elemRange.low.x, elemRange.high.y, fileRange.low.z),
//            DPoint3d::From(elemRange.high.x, elemRange.low.y, fileRange.high.z),
//            DPoint3d::From(elemRange.high.x, elemRange.high.y, fileRange.high.z)
//            };
//        meshQueryInterface->Query(returnedNodes, box, 4, params);
//        // DPoint3d triangle[4];
//        // int status = SUCCESS;
//        PolyfaceHeaderPtr terrainMesh;
//        IFacetOptionsPtr  options = IFacetOptions::Create();
//        options->SetMaxPerFace(3);
//        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
//        bvector<DPoint3d> allPts;
//        for (auto& node : returnedNodes)
//            {
//            bvector<bool> clips;
//            IScalableMeshMeshPtr scalableMesh = node->GetMesh(false, clips);
//            const PolyfaceQuery* polyface = scalableMesh->GetPolyfaceQuery();
//            builder->AddPolyface(*polyface);
//            allPts.insert(allPts.end(), polyface->GetPointCP(), polyface->GetPointCP() + polyface->GetPointCount());
//            }
//        //IMeshQuery*  meshQuery = dynamic_cast <IMeshQuery*> (&((EditElementHandleP)agenda.GetFirstP())->GetHandler());
//        //PolyfaceHeaderPtr mesh;
//        //meshQuery->GetMeshData(*agenda.GetFirstP(), mesh);
//        //mesh->Transform(uorToMeter);
//        //mesh->Transform(refToActiveTrf);
//        bvector<PolyfaceHeaderPtr> cutSections, fillSections;
//        terrainMesh = builder->GetClientMeshPtr();
//        PolyfaceQuery::ComputeCutAndFill(*terrainMesh, *meshData, cutSections, fillSections);
//        for (auto& polyfaceP : cutSections)
//            {
//            double sectionCut = 0.0;
//            DPoint3d centroid;
//            RotMatrix axes;
//            DVec3d moments;
//            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionCut, centroid, axes, moments, true);
//            cutValidate += fabs(sectionCut);
//            }
//        for (auto& polyfaceP : fillSections)
//            {
//            double sectionFill = 0.0;
//            DPoint3d centroid;
//            RotMatrix axes;
//            DVec3d moments;
//            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true);
//            fillValidate += fabs(sectionFill);
//            }
//
//        volumeValidate = cutValidate - fillValidate;
//        cutError = cutValidate == 0 ? 0 : 100.0*(cut - cutValidate) / cutValidate;
//        fillError = fillValidate == 0 ? 0 : 100.0*(fill - fillValidate) / fillValidate;
//        volumeError = volumeValidate == 0 ? 0 : 100.0*(volume - volumeValidate) / volumeValidate;
//        avgError = (fabs(cutError) + fabs(fillError) + fabs(volumeError)) / 3.0;
//        timer = clock() - timer;
//        secs = ((float)timer) / CLOCKS_PER_SEC;
//        terrainMesh->ClearAllVectors();
//        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
//        int status = CreateBcDTM(dtmPtr);
//        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
//        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &allPts[0], (long)allPts.size());
//        status = bcdtmObject_triangulateDtmObject(dtmObjP);
//        builder = IPolyfaceConstruction::New(*options);
//        BcDTMMeshPtr meshP = dtmPtr->GetBcDTM()->GetMesh((long)true, 0, NULL, 0);
//        DPoint3d triangle[4];
//        for (long i = 0; i < meshP->GetFaceCount(); ++i)
//            {
//            triangle[0] = meshP->GetFace(i)->GetCoordinates(0);
//            triangle[1] = meshP->GetFace(i)->GetCoordinates(1);
//            triangle[2] = meshP->GetFace(i)->GetCoordinates(2);
//            builder->AddTriStrip(triangle, NULL, NULL, 3, true);
//            }
//        terrainMesh = builder->GetClientMeshPtr();
//        cutSections.clear();
//        fillSections.clear();
//        PolyfaceQuery::ComputeCutAndFill(*terrainMesh, *meshData, cutSections, fillSections);
//        for (auto& polyfaceP : cutSections)
//            {
//            double sectionCut = 0.0;
//            DPoint3d centroid;
//            RotMatrix axes;
//            DVec3d moments;
//            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionCut, centroid, axes, moments, true);
//            cutConnected += fabs(sectionCut);
//            }
//        for (auto& polyfaceP : fillSections)
//            {
//            double sectionFill = 0.0;
//            DPoint3d centroid;
//            RotMatrix axes;
//            DVec3d moments;
//            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true);
//            fillConnected += fabs(sectionFill);
//            }
//        volConnected = cutConnected - fillConnected;
//        stitchCut = cutConnected == 0 ? 0 : 100.0*(cut - cutConnected) / cutConnected;
//        stitchFill = fillConnected == 0 ? 0 : 100.0*(fill - fillConnected) / fillConnected;
//        stitchVol = volConnected == 0 ? 0 : 100.0*(volume - volConnected) / volConnected;
//        //meshQuery = nullptr;
//        }
//    }
//
//    //agenda.Clear();
//    //model->DeleteDgnAttachment(newAttachment);
//    //write out results
//    double timeToCompute = 0.0;
//    int64_t nOfTriangles = 0, nTiles = 0, nFailedTiles = 0, nSections = 0, nSectionErrors = 0;
//    IScalableMeshATP::GetInt(L"nTrianglesInCorridor", nOfTriangles);
//    IScalableMeshATP::GetDouble(L"volumeTime", timeToCompute);
//    IScalableMeshATP::GetInt(L"nTiles", nTiles);
//    IScalableMeshATP::GetInt(L"nNoCutFillTiles", nFailedTiles);
//    IScalableMeshATP::GetInt(L"nSectionsTotal", nSections);
//    IScalableMeshATP::GetInt(L"nFailedComputePrincipalMoments", nSectionErrors);
//    fwprintf(pResultFile, L"%s,%s,%I64d,%I64d,%.5f,%.5f,%.5f,%I64d,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f\n",
//             stmFileName.c_str(),
//             result.c_str(),
//             stmFile->GetPointCount(),
//             nOfTriangles,
//             cut,
//             fill,
//             volume,
//             nTiles,
//             cutValidate,
//             fillValidate,
//             volumeValidate,
//             cutError,
//             fillError,
//             volumeError,
//             timeToCompute,
//             avgError,
//             secs,
//             cutConnected,
//             fillConnected,
//             volConnected,
//             stitchCut,
//             stitchFill,
//             stitchVol);
//    IScalableMeshATP::StoreDouble(L"volumeTime", 0.0);
//    IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", 0);
//    fflush(pResultFile);
//    }

//void ExportVolume(BeXmlNodeP pTestNode, FILE* pResultFile)
//    {
//    WString corridorFileName, exportFileName;
//    // Parse the test(s) definition:
//    if (pTestNode->GetAttributeStringValue(corridorFileName, "corridorFileName") != BEXML_Success)
//        {
//        printf("ERROR: corridorFileName attribute not found\r\n");
//        return;
//        }
//
//    if (pTestNode->GetAttributeStringValue(exportFileName, "exportFileName") != BEXML_Success)
//        {
//        printf("ERROR : outputFileName attribute not found\r\n");
//        return;
//        }
//    DgnFileStatus fileOpenStatus;
//    DgnDocumentPtr meshDoc = DgnDocument::CreateFromFileName(fileOpenStatus, corridorFileName.c_str(), NULL, DEFDGNFILE_ID, DgnDocument::FetchMode::Read);
//    DgnModelRefP model = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef();
//    DgnAttachment* newAttachment;
//    model->CreateDgnAttachment(newAttachment, *meshDoc->GetMonikerPtr(), L"");
//
//    ElementAgenda agenda;
//
//    LevelCache& levelCache = newAttachment->GetLevelCacheR();
//
//    LevelHandle level = levelCache.GetLevelByName(L"mesh", false);
//    LevelId levelId = level.GetLevelId();
//
//    ScanCriteriaP scP = new ScanCriteria();
//    BitMaskP  levelBitMask = BitMask::Create(false);
//
//    scP->SetModelRef(newAttachment);
//    scP->SetReturnType(MSSCANCRIT_ITERATE_ELMREF, false, false);
//    scP->SetElemRefCallback(CollectFirstMeshElement, &agenda);
//    levelBitMask->SetBit(levelId - 1, 1);
//    scP->SetLevelTest(levelBitMask, false);
//    scP->Scan();
//
//    if (agenda.GetCount() == 0)
//        {
//        //result = L"MESH NOT FOUND";
//        }
//    else
//        {
//        //DRange3d range;
//        //ScanRangeCP scanRangeP = elemHandle_checkIndexRange(*(EditElementHandleP)agenda.GetFirstP());
//        //DataConvert::ScanRangeToDRange3d(range, *scanRangeP);
//        //bvector<PolyfaceHeaderPtr> volumeMesh;
//        
//        EditElementHandleP meshElement = agenda.GetFirstP();
//
//        Handler&  elmHandler = meshElement->GetHandler();
//        IMeshQuery*  meshQuery = dynamic_cast <IMeshQuery*> (&elmHandler);
//        //clock_t timer = clock();
//        DRange3d elemRange;
//        ScanRangeCP scanRangeP = elemHandle_checkIndexRange(*(EditElementHandleP)agenda.GetFirstP());
//        DataConvert::ScanRangeToDRange3d(elemRange, *scanRangeP);
//
//        Json::Value root;
//        if (NULL != meshQuery)
//            {
//            PolyfaceHeaderPtr vec;
//
//            if (SUCCESS == meshQuery->GetMeshData(*meshElement, vec))
//                {
//
//                //PolyfaceHeaderPtr meshData = PolyfaceHeader::CreateUnifiedIndexMesh(*vec);
//                PolyfaceHeaderPtr meshData = PolyfaceHeader::CreateFixedBlockIndexed(3);
//                meshData->CopyFrom(*vec);
//                meshData->SetNumPerFace(3);
//                meshData->Triangulate();
//
//                Transform uorToMeter, meterToUor;
//                GetTransformForPoints(uorToMeter, meterToUor);
//                meshData->Transform(uorToMeter);
//                bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.low, 1);
//                bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.high, 1);
//                Transform refToActiveTrf;
//                GetFromModelRefToActiveTransform(refToActiveTrf, meshElement->GetModelRef());
//                meshData->Transform(refToActiveTrf);
//                bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.low, 1);
//                bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.high, 1);
//                
//                Json::Value jsonElemRange;
//                jsonElemRange["low.x"] = elemRange.low.x;
//                jsonElemRange["low.y"] = elemRange.low.y;
//                jsonElemRange["low.z"] = elemRange.low.z;
//                jsonElemRange["high.x"] = elemRange.high.x;
//                jsonElemRange["high.y"] = elemRange.high.y;
//                jsonElemRange["high.z"] = elemRange.high.z;
//                root["elemRange"] = jsonElemRange;
//
//                Json::Value jsonMesh;
//                Json::Value jsonPoints;
//                Json::Value jsonFacesIndex;
//                //Json::Value jsonPointsIndex;
//                Json::Value jsonNormalIndex;
//                Json::Value jsonNormal;
//
//                DPoint3dCP ptPoints = meshData->GetPointCP();
//                for (int i = 0; i < meshData->GetPointCount(); i++)
//                    {
//                    Json::Value jsonPoint;
//                    //jsonPoint["point"] = {ptPoints[i].x, ptPoints[i].y, ptPoints[i].z};
//                    jsonPoint["Point"] = Json::arrayValue;
//                    jsonPoint["Point"].append(ptPoints[i].x);
//                    jsonPoint["Point"].append(ptPoints[i].y);
//                    jsonPoint["Point"].append(ptPoints[i].z);
//                    jsonPoints.append(jsonPoint);
//                    }
//
//                const int32_t* ptIndexes = meshData->GetPointIndexCP();
//                jsonMesh["PointsIndex"] = Json::arrayValue;
//                for (int i = 0; i < meshData->GetPointIndexCount(); i++)
//                    {
//                    jsonMesh["PointsIndex"].append(ptIndexes[i]);
//                    }
//
//                FacetFaceDataCP facetFacePtr = meshData->GetFaceDataCP();
//                for (int i = 0; i < meshData->GetFaceCount(); i++)
//                    {
//                    //facetFacePtr[i].
//                    jsonMesh["FaceData"].append(facetFacePtr[i].m_xyzRange.XLength());
//                    }
//
//                const int32_t* faceIndexesPtr = meshData->GetFaceIndexCP();
//                for (int i = 0; i < meshData->GetFaceIndexCount(); i++)
//                    {
//                    jsonMesh["FaceIndex"].append(faceIndexesPtr[i]);
//                    }
//
//                const int32_t* normalIndexPtr = meshData->GetNormalIndexCP();
//                for (int i = 0; i < meshData->GetNormalCount(); i++)
//                    {
//                    jsonMesh["NormalIndex"].append(normalIndexPtr[i]);
//                    }
//
//                DVec3dCP normalPtr = meshData->GetNormalCP();
//                for (int i = 0; i < meshData->GetNormalCount(); i++)
//                    {
//                    jsonMesh["Normal"].append(normalPtr[i].x);
//                    }
//                
//                jsonMesh["numFace"] = meshData->GetNumFacet();
//                jsonMesh["chainCount"] = meshData->GetEdgeChainCount();
//                jsonMesh["faceCount"] = meshData->GetFaceCount();
//                jsonMesh["faceIndexCount"] = meshData->GetFaceIndexCount();
//                jsonMesh["normalCount"] = meshData->GetNormalCount();
//                jsonMesh["pointCount"] = meshData->GetPointCount();
//                jsonMesh["pointIndexCount"] = meshData->GetPointIndexCount();
//                //jsonMesh["FacesIndex"] = jsonFacesIndex;
//                jsonMesh["Points"] = jsonPoints;
//                //jsonMesh["PointsIndex"] = jsonPointsIndex;
//                //jsonMesh["NomalIndex"] = jsonNormalIndex;
//                //jsonMesh["Normal"] = jsonNormal;
//                jsonMesh["MeshStyle"] = meshData->GetMeshStyle();
//                root["mesh"] = jsonMesh;
//
//                /*IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", meshData->GetNumFacet());
//                volume = ComputeVolumeCutAndFill(smPtr->GetDTMInterface(), cut, fill, *meshData, elemRange, volumeMeshVector);
//                timer = clock() - timer;
//                float secs;
//                secs = ((float)timer) / CLOCKS_PER_SEC;
//                IScalableMeshATP::StoreDouble(L"volumeTime", secs);*/
//                uint64_t buffer_size;
//                auto jsonWriter = [&buffer_size](BeFile& file, Json::Value& object)
//                    {
//                    Json::StyledWriter writer;
//                    auto buffer = writer.write(object);
//                    buffer_size = buffer.size();
//                    file.Write(NULL, buffer.c_str(), buffer_size);
//                    };
//                BeFile file;
//                if (BeFileStatus::Success == file.Open(exportFileName.c_str(), BeFileAccess::Write, BeFileSharing::None))
//                    jsonWriter(file, root);
//                else if (BeFileStatus::Success == file.Create(exportFileName.c_str()))
//                    jsonWriter(file, root);
//                else
//                    HASSERT(!"Problem creating master header file");
//                file.Close();
//
//
//
//                //return SUCCESS;
//                }
//            }
//        //return ERROR;
//
//
//
//
//        }
//
//
//
//    /*Json::Value children;
//    size_t nbChildren = origPoints.size();
//    for (size_t childInd = 0; childInd < nbChildren; childInd++)
//        {
//        //Json::Value& child = childInd >= nbChildren ? children.append(Json::Value()) : children["point"];
//        //Json::Value& child = children.append("point");
//        //child["x"] = origPoints[childInd].x;
//        //child["y"] = origPoints[childInd].y;
//        //child["z"] = origPoints[childInd].z;
//        Json::Value child;
//        Json::Value points;
//        points["x"] = origPoints[childInd].x;
//        points["y"] = origPoints[childInd].y;
//        points["z"] = origPoints[childInd].z;
//        /*points["x].push_back(origPoints[childInd].x);
//        points.push_back(origPoints[childInd].y);
//        points.push_back(origPoints[childInd].z);*/
///*        child["point"] = points;
//        children.append(child);
//        }
//    block["lines"].append(children);
//    }*/
//
//
//    }



void PerformVolumeTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, importFileName;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }
    StatusInt status;
    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

    int64_t pointCount = 0;
    WString result;

    if (stmFile != 0)
        {
        pointCount = stmFile->GetPointCount();
        result = L"SUCCESS";
        }
    else
        {
        result = L"FAILURE";
        }
    if (pTestNode->GetAttributeStringValue(importFileName, "importFileName") != BEXML_Success)
        {
        printf("ERROR : importFileName attribute not found\r\n");
        return;
        }

    BeFile file;
    if (BeFileStatus::Success != file.Open(importFileName.c_str(), BeFileAccess::Read))
        {
        return;
        }
    char* meshFileBuffer = nullptr;
    size_t fileSize;
    file.GetSize(fileSize);
    meshFileBuffer = new char[fileSize];
    uint32_t bytes_read;
    file.Read(meshFileBuffer, &bytes_read, (uint32_t)fileSize);
    assert(bytes_read == fileSize);
    file.Close();

    Json::Reader reader;
    Json::Value mesh;
    reader.parse(meshFileBuffer, meshFileBuffer + bytes_read, mesh);

    DRange3d elemRange;
    elemRange.high.x = mesh["elemRange"]["high.x"].asDouble();
    elemRange.high.y = mesh["elemRange"]["high.y"].asDouble();
    elemRange.high.z = mesh["elemRange"]["high.z"].asDouble();
    elemRange.low.x = mesh["elemRange"]["low.x"].asDouble();
    elemRange.low.y = mesh["elemRange"]["low.y"].asDouble();
    elemRange.low.z = mesh["elemRange"]["low.z"].asDouble();

    double pointSize = mesh["mesh"]["pointCount"].asDouble();
    bvector<DPoint3d> points;
    points.resize((uint64_t)pointSize);
    int i = 0;
    for (const auto& jsonObject : mesh["mesh"]["Points"])
        {
        DPoint3d point;
        point.x = jsonObject["Point"][0].asDouble();
        point.y = jsonObject["Point"][1].asDouble();
        point.z = jsonObject["Point"][2].asDouble();
        points[i++] = point;
        }

    double pointIndexSize = mesh["mesh"]["pointIndexCount"].asDouble();
    bvector<int32_t> pointsIndex;
    pointsIndex.resize((uint64_t)pointIndexSize);
    for (int i = 0; i < pointIndexSize; i++)
        {
        int32_t id = mesh["mesh"]["PointsIndex"][i].asInt();
        pointsIndex[i] = id;
        }

    PolyfaceHeaderPtr meshData = PolyfaceHeader::CreateIndexedMesh(4, points, pointsIndex);

    double cut = 0.0, fill = 0.0, volume = 0.0;
    double cutValidate = 0.0, fillValidate = 0.0, volumeValidate = 0.0;
    double cutError = 0.0, fillError = 0.0, volumeError = 0.0, avgError = 0.0;
    double secs = 0.0;
    double   cutConnected, fillConnected, volConnected, stitchCut, stitchFill, stitchVol;
    cutConnected = fillConnected = volConnected = stitchCut = stitchFill = stitchVol = 0.0;

    {
    IScalableMeshATP::StoreInt(L"nTiles", 0);
    IScalableMeshATP::StoreInt(L"nNoCutFillTiles", 0);
    IScalableMeshATP::StoreInt(L"nSectionsTotal", 0);
    IScalableMeshATP::StoreInt(L"nFailedComputePrincipalMoments", 0);
    status = ComputeVolumeForAgenda(meshData, stmFile, cut, fill, volume);


    if (status != SUCCESS)
        {
        result = L"FAILED TO COMPUTE";
        }
    else
        {
        clock_t timer = clock();
        IScalableMeshMeshQueryPtr meshQueryInterface = ((IScalableMesh*)stmFile.get())->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
        bvector<IScalableMeshNodePtr> returnedNodes;
        IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
        DRange3d fileRange;
        stmFile->GetRange(fileRange);

        DPoint3d box[4] = {
            DPoint3d::From(elemRange.low.x, elemRange.low.y, fileRange.low.z),
            DPoint3d::From(elemRange.low.x, elemRange.high.y, fileRange.low.z),
            DPoint3d::From(elemRange.high.x, elemRange.low.y, fileRange.high.z),
            DPoint3d::From(elemRange.high.x, elemRange.high.y, fileRange.high.z)
            };
        meshQueryInterface->Query(returnedNodes, box, 4, params);

        PolyfaceHeaderPtr terrainMesh;
        IFacetOptionsPtr  options = IFacetOptions::Create();
        options->SetMaxPerFace(3);
        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
        bvector<DPoint3d> allPts;
        for (auto& node : returnedNodes)
            {
            bvector<bool> clips;
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            flags->SetLoadGraph(false);
            IScalableMeshMeshPtr scalableMesh = node->GetMesh(flags, clips);
            const PolyfaceQuery* polyface = scalableMesh->GetPolyfaceQuery();
            builder->AddPolyface(*polyface);
            allPts.insert(allPts.end(), polyface->GetPointCP(), polyface->GetPointCP() + polyface->GetPointCount());
            }

        bvector<PolyfaceHeaderPtr> cutSections, fillSections;
        terrainMesh = builder->GetClientMeshPtr();
        PolyfaceQuery::ComputeCutAndFill(*terrainMesh, *meshData, cutSections, fillSections);
        for (auto& polyfaceP : cutSections)
            {
            double sectionCut = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionCut, centroid, axes, moments, true);
            cutValidate += fabs(sectionCut);
            }
        for (auto& polyfaceP : fillSections)
            {
            double sectionFill = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true);
            fillValidate += fabs(sectionFill);
            }

        volumeValidate = cutValidate - fillValidate;
        cutError = cutValidate == 0 ? 0 : 100.0*(cut - cutValidate) / cutValidate;
        fillError = fillValidate == 0 ? 0 : 100.0*(fill - fillValidate) / fillValidate;
        volumeError = volumeValidate == 0 ? 0 : 100.0*(volume - volumeValidate) / volumeValidate;
        avgError = (fabs(cutError) + fabs(fillError) + fabs(volumeError)) / 3.0;
        timer = clock() - timer;
        secs = ((float)timer) / CLOCKS_PER_SEC;
        terrainMesh->ClearAllVectors();
        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
        int status = CreateBcDTM(dtmPtr);
        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &allPts[0], (long)allPts.size());
        status = bcdtmObject_triangulateDtmObject(dtmObjP);
        builder = IPolyfaceConstruction::New(*options);
        BcDTMMeshPtr meshP = dtmPtr->GetBcDTM()->GetMesh((long)true, 0, NULL, 0);
        DPoint3d triangle[4];
        for (long i = 0; i < meshP->GetFaceCount(); ++i)
            {
            triangle[0] = meshP->GetFace(i)->GetCoordinates(0);
            triangle[1] = meshP->GetFace(i)->GetCoordinates(1);
            triangle[2] = meshP->GetFace(i)->GetCoordinates(2);
            builder->AddTriStrip(triangle, NULL, NULL, 3, true);
            }
        terrainMesh = builder->GetClientMeshPtr();
        cutSections.clear();
        fillSections.clear();
        PolyfaceQuery::ComputeCutAndFill(*terrainMesh, *meshData, cutSections, fillSections);
        for (auto& polyfaceP : cutSections)
            {
            double sectionCut = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionCut, centroid, axes, moments, true);
            cutConnected += fabs(sectionCut);
            }
        for (auto& polyfaceP : fillSections)
            {
            double sectionFill = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true);
            fillConnected += fabs(sectionFill);
            }
        volConnected = cutConnected - fillConnected;
        stitchCut = cutConnected == 0 ? 0 : 100.0*(cut - cutConnected) / cutConnected;
        stitchFill = fillConnected == 0 ? 0 : 100.0*(fill - fillConnected) / fillConnected;
        stitchVol = volConnected == 0 ? 0 : 100.0*(volume - volConnected) / volConnected;
        }
    }

    //write out results
    double timeToCompute = 0.0;
    int64_t nOfTriangles = 0, nTiles = 0, nFailedTiles = 0, nSections = 0, nSectionErrors = 0;
    IScalableMeshATP::GetInt(L"nTrianglesInCorridor", nOfTriangles);
    IScalableMeshATP::GetDouble(L"volumeTime", timeToCompute);
    IScalableMeshATP::GetInt(L"nTiles", nTiles);
    IScalableMeshATP::GetInt(L"nNoCutFillTiles", nFailedTiles);
    IScalableMeshATP::GetInt(L"nSectionsTotal", nSections);
    IScalableMeshATP::GetInt(L"nFailedComputePrincipalMoments", nSectionErrors);
    fwprintf(pResultFile, L"%s,%s,%I64d,%I64d,%.5f,%.5f,%.5f,%I64d,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f\n",
             stmFileName.c_str(),
             result.c_str(),
             stmFile->GetPointCount(),
             nOfTriangles,
             cut,
             fill,
             volume,
             nTiles,
             cutValidate,
             fillValidate,
             volumeValidate,
             cutError,
             fillError,
             volumeError,
             timeToCompute,
             avgError,
             secs,
             cutConnected,
             fillConnected,
             volConnected,
             stitchCut,
             stitchFill,
             stitchVol);
    IScalableMeshATP::StoreDouble(L"volumeTime", 0.0);
    IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", 0);
    fflush(pResultFile);
    }

/*---------------------------------------------------------------------------------**//**
* Self-contained importer test.
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t s_nbImportedPoints = 0;
static size_t s_nbImportedFeaturePoints = 0;
static size_t s_nbImportedFeatures = 0;
static FILE*  s_pPointResultFile = 0;
static FILE*  s_pFeatureResultFile = 0;

bool WritePointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
    {
    char coordinateBuffer[300];
    int  NbChars;

    for (size_t pointInd = 0; pointInd < nbOfPoints; pointInd++)
        {
        NbChars = sprintf(coordinateBuffer,
                          "%.20g,%.20g,%.20g\n",
                          points[pointInd].x,
                          points[pointInd].y,
                          points[pointInd].z);

        size_t nbCharsWritten = fwrite(coordinateBuffer, 1, NbChars, s_pPointResultFile);

        assert(NbChars == nbCharsWritten);
        }

    fflush(s_pPointResultFile);

    s_nbImportedPoints += nbOfPoints;

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

    size_t nbCharsWritten = fwrite(outputBuffer, 1, NbChars, s_pFeatureResultFile);

    assert(NbChars == nbCharsWritten);


    for (size_t featurePointInd = 0; featurePointInd < nbOfFeaturesPoints; featurePointInd++)
        {
        NbChars = sprintf(outputBuffer,
                          "%.20g,%.20g,%.20g\n",
                          featurePoints[featurePointInd].x,
                          featurePoints[featurePointInd].y,
                          featurePoints[featurePointInd].z);

        nbCharsWritten = fwrite(outputBuffer, 1, NbChars, s_pFeatureResultFile);

        assert(NbChars == nbCharsWritten);
        }

    fflush(s_pFeatureResultFile);

    s_nbImportedFeaturePoints += nbOfFeaturesPoints;
    s_nbImportedFeatures++;

    return true;
    }

/*class ScalableMeshSourceImporterStorage : public IScalableMeshSourceImporterStorage
    {
    private:

        BeXmlWriterPtr m_pSourceInfoFileWriter;
        BeXmlReaderPtr m_pSourceInfoFileReader;
        bool           m_isReading;
        WString        m_gcsWkt;
        StatusInt      m_gcsStatus;

        struct SourceInfo
            {
            Time::TimeType m_sourceLastModifiedTime;
            HPU::Packet    m_serializedSourcePacket;
            HPU::Packet    m_serializedContentConfigPacket;
            HPU::Packet    m_serializedImportSequencePacket;
            GroupId        m_groupId;
            };

        vector<SourceInfo> m_sources;
        size_t             m_currentSourceIndex;

        void ConvertPacketToHexString(Utf8String& hexString, const HPU::Packet& packet)
            {
            static char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

            for (int i = 0; i < packet.GetPacket().GetDataSize(); ++i)
                {
                const char ch = packet.GetPacket().GetBufferAddress()[i];
                hexString.append(&hex_chars[(ch & 0xF0) >> 4], 1);
                hexString.append(&hex_chars[ch & 0xF], 1);
                }
            }

        void ConvertHexStringPacket(HPU::Packet& packet, const Utf8String& hexString)
            {
            assert(hexString.size() % 2 == 0);
            packet.Reserve(hexString.size() / 2);
            packet.SetSize(hexString.size() / 2);

            char charStr[2];
            for (int i = 0; i < packet.GetPacket().GetDataSize(); ++i)
                {
                charStr[0] = hexString.c_str()[i * 2];
                charStr[1] = hexString.c_str()[i * 2 + 1];

                unsigned long charIntVal = stoul(string(charStr), 0, 16);

                assert(charIntVal < 256);
                packet.GetPacket().GetBufferAddress()[i] = (byte)charIntVal;
                }
            }

        template <typename NodeValueType>
        StatusInt ReadDataSourceNode(NodeValueType& nodeValue, Utf8String& nodeName)
            {
            IBeXmlReader::ReadResult result = m_pSourceInfoFileReader->Read();

            if (result != IBeXmlReader::READ_RESULT_Success)
                return ERROR;

            m_pSourceInfoFileReader->GetCurrentNodeName(nodeName);

            if (nodeName.CompareTo(nodeName.c_str()) != 0)
                return ERROR;

            BeXmlStatus xmlStatus = m_pSourceInfoFileReader->GetCurrentNodeValue(nodeValue);

            if (xmlStatus != BEXML_Success)
                return ERROR;

            return SUCCESS;
            }

        StatusInt ReadDataSource(SourceInfo& sourceInfo)
            {
            StatusInt status;

            WString nodeValueW;

            Utf8String nodeName("Time");

            status = ReadDataSourceNode<WString>(nodeValueW, nodeName);

            if (status != SUCCESS) return status;

            sourceInfo.m_sourceLastModifiedTime = _wtoi(nodeValueW.c_str());

            Utf8String nodeValue;
            nodeName = "SourcePacket";

            status = ReadDataSourceNode<Utf8String>(nodeValue, nodeName);

            if (status != SUCCESS) return status;

            ConvertHexStringPacket(sourceInfo.m_serializedSourcePacket, nodeValue);


            nodeName = "ContentConfigPacket";

            status = ReadDataSourceNode<Utf8String>(nodeValue, nodeName);

            if (status != SUCCESS) return status;

            ConvertHexStringPacket(sourceInfo.m_serializedSourcePacket, nodeValue);


            nodeName = "ImportSequencePacket";

            status = ReadDataSourceNode<Utf8String>(nodeValue, nodeName);

            if (status != SUCCESS) return status;

            ConvertHexStringPacket(sourceInfo.m_serializedSourcePacket, nodeValue);
            }

        StatusInt ReadSourceInfo()
            {
            IBeXmlReader::ReadResult result;
            StatusInt                status = SUCCESS;

            while (((result = m_pSourceInfoFileReader->Read()) == IBeXmlReader::READ_RESULT_Success) && (status == SUCCESS))
                {
                Utf8String nodeName;

                m_pSourceInfoFileReader->GetCurrentNodeName(nodeName);

                if (nodeName.CompareTo("DataSource") == 0)
                    {
                    SourceInfo sourceInfo;
                    status = ReadDataSource(sourceInfo);

                    if (status == SUCCESS)
                        {
                        m_sources.push_back(sourceInfo);
                        }
                    }
                else
                    if (nodeName.CompareTo("GCS") == 0)
                        {
                        BeXmlStatus xmlStatus = m_pSourceInfoFileReader->GetCurrentNodeValue(m_gcsWkt);

                        m_gcsStatus = (xmlStatus == BEXML_Success);
                        }
                    else
                        {
                        assert(!"Unknown node");
                        status = ERROR;
                        }
                }

            assert(result == IBeXmlReader::READ_RESULT_Error);
            }

    protected:

        virtual StatusInt _AddSource(Time::TimeType     sourceLastModifiedTime,
                                     const HPU::Packet& serializedSourcePacket,
                                     const HPU::Packet& serializedContentConfigPacket,
                                     const HPU::Packet& serializedImportSequencePacket,
                                     GroupId            groupId) override
            {
            if (m_isReading) return ERROR;

            m_pSourceInfoFileWriter->WriteElementStart("DataSource");

            m_pSourceInfoFileWriter->WriteElementStart("Time");

            wchar_t buffer[50];
            _itow(sourceLastModifiedTime, buffer, 10);

            m_pSourceInfoFileWriter->WriteText(buffer);
            m_pSourceInfoFileWriter->WriteElementEnd();

            Utf8String hexString;
            ConvertPacketToHexString(hexString, serializedSourcePacket);

            m_pSourceInfoFileWriter->WriteElementStart("SourcePacket");
            m_pSourceInfoFileWriter->WriteText(hexString.c_str());
            m_pSourceInfoFileWriter->WriteElementEnd();

            hexString.clear();
            ConvertPacketToHexString(hexString, serializedContentConfigPacket);

            m_pSourceInfoFileWriter->WriteElementStart("ContentConfigPacket");
            m_pSourceInfoFileWriter->WriteText(hexString.c_str());
            m_pSourceInfoFileWriter->WriteElementEnd();

            hexString.clear();
            ConvertPacketToHexString(hexString, serializedImportSequencePacket);

            m_pSourceInfoFileWriter->WriteElementStart("ImportSequencePacket");
            m_pSourceInfoFileWriter->WriteText(hexString.c_str());
            m_pSourceInfoFileWriter->WriteElementEnd();

            m_pSourceInfoFileWriter->WriteElementEnd();

            return SUCCESS;
            }

        virtual StatusInt _StoreGcs(const WString& wkt) override
            {
            if (m_isReading) return ERROR;

            m_pSourceInfoFileWriter->WriteElementStart("GCS");
            m_pSourceInfoFileWriter->WriteText(wkt.c_str());
            m_pSourceInfoFileWriter->WriteElementEnd();

            return SUCCESS;
            }

        virtual bool _ReadFirstSource(StatusInt* status) override
            {
            if (status != 0)
                {
                *status = SUCCESS;
                }

            if (m_isReading)
                {
                m_currentSourceIndex = 0;

                return m_sources.size() > 0;
                }
            else
                {
                return false;
                }
            }

        virtual StatusInt _GetSourceInfo(Time::TimeType& sourceLastModifiedTime,
                                         HPU::Packet&    serializedSourcePacket,
                                         HPU::Packet&    serializedContentConfigPacket,
                                         HPU::Packet&    serializedImportSequencePacket,
                                         GroupId&        groupId) const override
            {
            if (!m_isReading || m_currentSourceIndex >= m_sources.size())
                return ERROR;

            sourceLastModifiedTime = m_sources[m_currentSourceIndex].m_sourceLastModifiedTime;
            serializedSourcePacket = m_sources[m_currentSourceIndex].m_serializedSourcePacket;
            serializedContentConfigPacket = m_sources[m_currentSourceIndex].m_serializedContentConfigPacket;
            serializedImportSequencePacket = m_sources[m_currentSourceIndex].m_serializedImportSequencePacket;
            groupId = m_sources[m_currentSourceIndex].m_groupId;

            return SUCCESS;
            }

        virtual bool _ReadNextSource(StatusInt* status) override
            {
            if (status != 0)
                {
                *status = SUCCESS;
                }

            if (m_isReading)
                {
                m_currentSourceIndex++;

                return m_currentSourceIndex < m_sources.size();
                }
            else
                {
                return false;
                }
            }

        virtual StatusInt _ReadGcs(WString& wkt) override
            {
            if (!m_isReading) return ERROR;

            wkt = m_gcsWkt;

            return m_gcsStatus;
            }

        ScalableMeshSourceImporterStorage(WString& sourceInfoFileName, bool isReading)
            {
            m_gcsStatus = ERROR;
            m_isReading = isReading;
            m_currentSourceIndex = 0;

            if (!m_isReading)
                {
                m_pSourceInfoFileWriter = BeXmlWriter::CreateFileWriter(sourceInfoFileName.c_str());

                m_pSourceInfoFileWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);

                m_pSourceInfoFileWriter->WriteElementStart("SourceImporter");
                }
            else
                {
                BeXmlStatus status;

                m_pSourceInfoFileReader = BeXmlReader::CreateAndReadFromFile(status, sourceInfoFileName.c_str());

                assert(status == SUCCESS);
                }
            }

        virtual ~ScalableMeshSourceImporterStorage()
            {
            if (!m_isReading)
                m_pSourceInfoFileWriter->WriteElementEnd();
            }

        StatusInt LoadSource()
            {
            assert(!m_isReading);

            StatusInt status = ERROR;

            IBeXmlReader::ReadResult result = m_pSourceInfoFileReader->Read();

            if (result == IBeXmlReader::READ_RESULT_Success)
                {
                Utf8String nodeName;

                m_pSourceInfoFileReader->GetCurrentNodeName(nodeName);

                if (nodeName.CompareTo("SourceImporter") == 0)
                    {
                    status = ReadSourceInfo();
                    }
                }
            }


    public:

        static IScalableMeshSourceImporterStoragePtr Create(WString& sourceInfoFileName, bool isReading)
            {
            return new ScalableMeshSourceImporterStorage(sourceInfoFileName, isReading);
            }
    };
*/
#define BUFFERSIZE 4096

bool CompareFileBinary(FILE* pFile1, FILE* pFile2)
    {
    bool areTheSame = true;

    int status = fseek(pFile1, 0, SEEK_END);
    assert(status == 0);
    long file1Size = ftell(pFile1);

    status = fseek(pFile2, 0, SEEK_END);
    assert(status == 0);
    long file2Size = ftell(pFile2);

    if ((file1Size == file2Size) && (file1Size > 0))
        {
        byte bufferFile1[BUFFERSIZE];
        byte bufferFile2[BUFFERSIZE];

        status = fseek(pFile1, 0, SEEK_SET);
        assert(status == 0);

        status = fseek(pFile2, 0, SEEK_SET);
        assert(status == 0);

        size_t nbBytes1;

        do
            {
            nbBytes1 = fread(bufferFile1, 1, BUFFERSIZE, pFile1);
            size_t nbBytes2 = fread(bufferFile2, 1, BUFFERSIZE, pFile2);

            if ((nbBytes1 == nbBytes2) && (nbBytes1 > 0))
                {
                int result = memcmp(bufferFile1, bufferFile2, BUFFERSIZE);

                if (result != 0)
                    {
                    areTheSame = false;
                    }
                }
            else
                if (nbBytes1 != nbBytes2)
                    {
                    areTheSame = false;
                    }

            } while ((nbBytes1 == BUFFERSIZE) && (areTheSame == true));
        }
    else
        if ((file1Size != 0) || (file2Size != 0))
            {
            areTheSame = false;
            }

    return areTheSame;
    }

bool CompareWithBaseline(FILE* pPointResultFile, FILE* pFeatureResultFile, WString& pointBaseLineFileName, WString& featureBaseLineFileName)
    {
    assert(pPointResultFile != 0);
    assert(pFeatureResultFile != 0);

    bool samePointAsBaseline = false;
    bool sameFeatureAsBaseline = false;

    FILE* pointBaseLineFile = _wfopen(pointBaseLineFileName.c_str(), L"r");

    if (pointBaseLineFile != 0)
        {
        samePointAsBaseline = CompareFileBinary(pPointResultFile, pointBaseLineFile);
        fclose(pointBaseLineFile);
        }

    FILE* featureBaseLineFile = _wfopen(featureBaseLineFileName.c_str(), L"r");

    if (featureBaseLineFile != 0)
        {
        sameFeatureAsBaseline = CompareFileBinary(pFeatureResultFile, featureBaseLineFile);
        fclose(featureBaseLineFile);
        }

    return samePointAsBaseline && sameFeatureAsBaseline;
    }

/*bool PerformStorageRestoreTest(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceImporterPtr&       importerFromStoragePtr,
                               clock_t&                                       storageTime,
                               clock_t&                                       restoreTime,
                               const Bentley::ScalableMesh::IScalableMeshSourceImporterPtr& importerFromSourcesPtr,
                               WString&                                       sourceInfoFileName)
    {
    bool success = false;
    assert(false && "Can't support now");
/*
    storageTime = clock();

    IScalableMeshSourceImporterStoragePtr importerStorage(ScalableMeshSourceImporterStorage::Create(sourceInfoFileName, false));

    StatusInt status = importerFromSourcesPtr->Store(importerStorage);

    storageTime = clock() - storageTime;

    if (status == SUCCESS)
        {
        importerStorage = 0;

        restoreTime = clock();

        IScalableMeshSourceImporterStoragePtr restoreStorage(ScalableMeshSourceImporterStorage::Create(sourceInfoFileName, true));

        StatusInt status;

        importerFromStoragePtr = Bentley::ScalableMesh::IScalableMeshSourceImporter::Create(restoreStorage,
                                                                                            status);

        restoreTime = clock() - restoreTime;

        if (status == SUCCESS)
            {
            success = true;
            }
        }
    */
    /*   return success;
       }*/

void PerformSelfContainedImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "not supported");
    /*BeXmlStatus status;
    WString pointFileName;
    WString featureFileName;
    WString sourceInfoFileName;

    status = pTestNode->GetAttributeStringValue(pointFileName, "pointFileName");
    bool fileNameAttrFound = false;

    if (status != BEXML_Success)
        {
        printf("ERROR : pointFileName attribute not found\r\n");
        }
    else
        {
        status = pTestNode->GetAttributeStringValue(featureFileName, "featureFileName");

        if (status != BEXML_Success)
            {
            printf("ERROR : featureFileName attribute not found\r\n");
            }
        else
            {
            status = pTestNode->GetAttributeStringValue(sourceInfoFileName, "sourceInfoFileName");

            if (status != BEXML_Success)
                {
                printf("ERROR : sourceInfoFileName attribute not found\r\n");
                }
            else
                {
                fileNameAttrFound = true;
                }
            }
        }

    if (fileNameAttrFound)
        {
        s_nbImportedPoints = 0;
        s_nbImportedFeaturePoints = 0;
        s_nbImportedFeatures = 0;

        s_pPointResultFile = _wfopen(pointFileName.c_str(), L"w+");

        assert(s_pPointResultFile);

        s_pFeatureResultFile = _wfopen(featureFileName.c_str(), L"w+");

        assert(s_pFeatureResultFile);

        BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());

        importerPtr->SetFeatureCallback(WriteFeatureCallback);
        importerPtr->SetPointsCallback(WritePointsCallback);

        if (importerPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            }
        else
            {
            WString gcsKeyName;

            status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

            if (status == BEXML_Success)
                {
                BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
                StatusInt status = importerPtr->SetBaseGCS(baseGCSPtr);
                assert(status == SUCCESS);
                }

            if (ParseSourceSubNodes(importerPtr->EditSources(), pTestNode) == true)
                {
                clock_t storageTime;
                clock_t restoreTime;
                Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerFromSourcesPtr;

                bool storeRestoreResult = PerformStorageRestoreTest(importerFromSourcesPtr,
                                                                    storageTime,
                                                                    restoreTime,
                                                                    importerPtr,
                                                                    sourceInfoFileName);

                StatusInt status = ERROR;
                clock_t   importTime = 0;

                if (storeRestoreResult)
                    {
                    importTime = clock();

                    status = importerPtr->Import();

                    importTime = clock() - importTime;
                    }

                if (status == SUCCESS)
                    {
                    assert(status == SUCCESS);

                    WString baselinePointFileName;
                    WString baselineFeatureFileName;

                    bool result = ParseBaselineSubNodes(baselinePointFileName, baselineFeatureFileName, pTestNode);

                    WString baselineComparisonInfo;

                    if (result == true)
                        {
                        bool comparisonResult = CompareWithBaseline(s_pPointResultFile, s_pFeatureResultFile, baselinePointFileName, baselineFeatureFileName);

                        if (comparisonResult)
                            {
                            baselineComparisonInfo = L"Same as baseline";
                            }
                        else
                            {
                            baselineComparisonInfo = L"Different than baseline";
                            }
                        }
                    else
                        {
                        baselineComparisonInfo = L"No baseline specified";
                        }

                    double minutes = (double)importTime / CLOCKS_PER_SEC / 60.0;

                    fwprintf(pResultFile,
                             L"%s,%I64d,%I64d,%I64d,%.5f,%.5f,%.5f,%s\n",
                             sourceInfoFileName.c_str(),
                             s_nbImportedPoints,
                             s_nbImportedFeatures,
                             s_nbImportedFeaturePoints,
                             minutes,
                             (double)storageTime / CLOCKS_PER_SEC,
                             (double)restoreTime / CLOCKS_PER_SEC,
                             baselineComparisonInfo);
                    }
                else
                    {
                    fwprintf(pResultFile, L"%s,%s,%s,%s,%.5f,%.5f,%.5f,%s\n", sourceInfoFileName.c_str(), L"Error", L"Error", L"Error", 0, 0, 0, L"N/A");
                    }
                fflush(pResultFile);
                }
            }

        fclose(s_pPointResultFile);
        s_pPointResultFile = 0;

        fclose(s_pFeatureResultFile);
        s_pFeatureResultFile = 0;
        }*/
    }

std::vector<std::pair<std::vector<DPoint3d>, DTMFeatureType>> s_importedFeatures;
bool ImportedFeaturesCallback(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d)
    {
    std::vector<DPoint3d> vec(nbOfFeaturesPoints);
    memcpy(&vec[0], featurePoints, sizeof(DPoint3d)*nbOfFeaturesPoints);
    s_importedFeatures.push_back(std::make_pair(vec, featureType));
    return true;
    }

bool NoPointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
    {
    return true;
    }

void PerformGISFeaturesImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "not supported");
    /*    WString baselineFileName, result;
        Int64 breaklineCount = 0, createdBreaklineCount = 0, differentBreaklines = 0;
        if (pTestNode->GetAttributeStringValue(baselineFileName, "baseline") != BEXML_Success)
            {
            printf("ERROR : baseline attribute not found\r\n");
            return;
            }
        StatusInt status;
        char* nameBuffer = new char[baselineFileName.GetMaxLocaleCharBytes()];
        std::ifstream featureFile;
        featureFile.open(baselineFileName.ConvertToLocaleChars(nameBuffer));
        std::vector<std::pair<std::vector<DPoint3d>, DTMFeatureType>> oldFeatures;
        ReadFeatureFile(featureFile, oldFeatures);
        Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());

        importerPtr->SetFeatureCallback(ImportedFeaturesCallback);
        importerPtr->SetPointsCallback(NoPointsCallback);

        double elapsed = 0.0;
        featureFile.close();
        breaklineCount = oldFeatures.size();
        if (ParseSourceSubNodes(importerPtr->EditSources(), pTestNode) == true)
            {
            s_importedFeatures.clear();
            clock_t timer = clock();
            status = importerPtr->Import();
            elapsed = clock() - timer;
            if (status == SUCCESS)
                {
                result = L"SUCCESS";
                createdBreaklineCount = s_importedFeatures.size();
                for (auto feature : oldFeatures)
                    {
                    auto found_it = std::find_if(s_importedFeatures.begin(), s_importedFeatures.end(), [&feature](std::pair<std::vector<DPoint3d>, DTMFeatureType>& vec)
                        {
                        if (feature.second != vec.second) return false;
                        if (feature.first.size() != vec.first.size()) return false;
                        for (size_t i = 0; i < feature.first.size(); i++)
                            {
                            if (fabs(feature.first[i].x - vec.first[i].x) > 1e-08 || fabs(feature.first[i].y - vec.first[i].y) > 1e-08 || fabs(feature.first[i].z - vec.first[i].z) > 1e-08) return false;
                            }
                        return true;
                        });
                    if (found_it == s_importedFeatures.end()) differentBreaklines++;
                    }
                }
            else result = L"IMPORT FAILED";
            }
        else result = L"FAILED READING SOURCES";
        //write out results
        baselineFileName.ReplaceAll(L",", L" ");
        fwprintf(pResultFile, L"%s,%s,%I64d, %I64d, %I64d,%.5f\n",
                 baselineFileName.c_str(),
                 result.c_str(),
                 breaklineCount,
                 createdBreaklineCount,
                 differentBreaklines,
                 (elapsed / CLOCKS_PER_SEC) / 60);
        fflush(pResultFile);*/
    }

/*void CreateBuffersAndQueryForPointCloud(EditElementHandleP pcloud, IPointCloudDataQueryPtr query, IPointCloudQueryBuffersPtr buffers, WString& fileName)
    {
        DgnDocumentMonikerPtr monikerPtr = PointCloudDisplay::CreateDocumentMonikerFromFileName(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef(), fileName.c_str());
        PointCloudPropertiesPtr propsP = PointCloudProperties::Create(*monikerPtr, *ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef());
        if (SUCCESS != PointCloudDisplay::CreateElement(*pcloud, *ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef(), *propsP)) assert(false && "Can't create element");
        pcloud->AddToModel();
        IPointCloudChannelP channel = GroundDetectionManager::GetChannelFromPODElement(*pcloud);
        IPointCloudChannelVector queryChannels;
        queryChannels.push_back(channel);

        query = IPointCloudDataQuery::CreateVisiblePointsQuery(*pcloud);
        query->SetDensity(IPointCloudDataQuery::QUERY_DENSITY::QUERY_DENSITY_FULL, 1.0);
        query->SetMode(IPointCloudDataQuery::QUERY_MODE_FILE, -1);
        uint32_t channelFlags = (uint32_t)PointCloudChannelId::Xyz | (uint32_t)PointCloudChannelId::Classification | (uint32_t)PointCloudChannelId::Filter;
        buffers = query->CreateBuffers((uint32_t)400000, channelFlags, queryChannels);
    }

struct CompareClassifResult
    {
    int64_t nTypeIError = 0;
    int64_t nTypeIIError = 0;
    int64_t nTotalPts = 0;
    int64_t nGroundBaseline = 0;
    int64_t nObjectsBaseline = 0;
    int64_t nCorrectGround = 0;
    int64_t nCorrectObjects = 0;
    double nTypeIErrorpercent;
    double nTypeIIErrorpercent;
    double totalError;
    double sensitivity;
    double specificity;
    };

/*void CompareClassifications(WString& testFileName, WString& baselineFileName, CompareClassifResult& res)
    {
    const UChar groundNumber = 2;
    EditElementHandle ehBaseline, ehTest;
    IPointCloudQueryBuffersPtr baselineBuffers, testBuffers;
    IPointCloudDataQueryPtr baselineQuery, testQuery;
    CreateBuffersAndQueryForPointCloud(&ehBaseline, baselineQuery, baselineBuffers, baselineFileName);
    CreateBuffersAndQueryForPointCloud(&ehTest, testQuery, testBuffers, testFileName);
    size_t readPts = baselineQuery->GetPoints(*baselineBuffers.get());
    IPointCloudChannelP channel = GroundDetectionManager::GetChannelFromPODElement(ehTest);
    IPointCloudChannelVector queryChannels;
    queryChannels.push_back(channel);
    size_t testPts = testQuery->GetPoints(*testBuffers.get());
    while (readPts > 0)
        {
        res.nTotalPts += readPts;
        DPoint3d* pointPtr = baselineBuffers->GetXyzBuffer();
        for (size_t idx = 0; idx < readPts; ++idx, ++pointPtr)
            {
            DPoint3d pt = *pointPtr;
            size_t j = idx;
            DPoint3d pt2 = testBuffers->GetXyzBuffer()[j];
            if (fabs(pt2.x - pt.x) < 1.0e-5 && fabs(pt2.y - pt.y) < 1.0e-5 && fabs(pt2.z - pt.z) < 1.0e-5)
                {
                UChar baselineClass = ((UChar*)baselineBuffers->GetClassificationBuffer())[idx];
                UChar testPtClass = ((UChar*)testBuffers->GetClassificationBuffer())[j];
                bool baselineIsGround = (baselineClass == groundNumber);
                bool testPtIsGround = (testPtClass == groundNumber);
                if (baselineIsGround)
                    {
                    res.nGroundBaseline++;
                    if (testPtIsGround) res.nCorrectGround++;
                    else res.nTypeIError++;
                    }
                else
                    {
                    res.nObjectsBaseline++;
                    if (testPtIsGround) res.nTypeIIError++;
                    else res.nCorrectObjects++;
                    }
                //  break;
                //  }
                }
            }
        readPts = baselineQuery->GetPoints(*baselineBuffers.get());
        testPts = testQuery->GetPoints(*testBuffers.get());
        }
    ehBaseline.DeleteFromModel();
    ehTest.DeleteFromModel();
    res.nTypeIErrorpercent = 100.0*res.nTypeIError / res.nTotalPts;
    res.nTypeIIErrorpercent = 100.0*res.nTypeIIError / res.nTotalPts;
    res.totalError = 100.0*(res.nTypeIError + res.nTypeIIError) / res.nTotalPts;
    res.sensitivity = 100.0* res.nCorrectGround / res.nGroundBaseline;
    res.specificity = 100.0*res.nCorrectObjects / res.nObjectsBaseline;
    }*/

void PerformClassificationCompareTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "not supported");
    /*WString baselineFileName, testFileName, result;
    if (pTestNode->GetAttributeStringValue(baselineFileName, "baseline") != BEXML_Success)
        {
        printf("ERROR : baseline attribute not found\r\n");
        return;
        }
    if (pTestNode->GetAttributeStringValue(testFileName, "data") != BEXML_Success)
        {
        printf("ERROR : data attribute not found\r\n");
        return;
        }
    CompareClassifResult res;
    CompareClassifications(baselineFileName, testFileName, res);
    //L"Test Case, Baseline, Type I error [N of ground points classified as objects], Type II error [N of object points classified as ground], Type I error %%, Type II error %%, Total error %%, Sensitivity %%, Specificity %%";
    fwprintf(pResultFile, L"%s,%s,%I64d, %I64d, %I64d,%I64d, %I64d, %.5f,%.5f,%.5f,%.5f,%.5f\n",
             testFileName.c_str(),
             baselineFileName.c_str(),
             res.nTotalPts,
             res.nGroundBaseline,
             res.nObjectsBaseline,
             res.nTypeIError,
             res.nTypeIIError,
             res.nTypeIErrorpercent,
             res.nTypeIIErrorpercent,
             res.totalError,
             res.sensitivity,
             res.specificity);
    fflush(pResultFile);*/
    }

/*void RunGroundDetection(WString& path)
    {
    EditElementHandle pcloud;
    // Replace ACTIVEMODEL by ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()
    DgnDocumentMonikerPtr monikerPtr = PointCloudDisplay::CreateDocumentMonikerFromFileName(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef(), path.c_str());
    PointCloudPropertiesPtr propsP = PointCloudProperties::Create(*monikerPtr, *ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef());
    if (SUCCESS != PointCloudDisplay::CreateElement(pcloud, *ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef(), *propsP)) assert(false && "Can't create element");
    pcloud.AddToModel();
    GroundDetectionParametersPtr paramsP = GroundDetectionParameters::Create();
    GroundDetectionManager::DoGroundDetection(pcloud, *paramsP);
    pcloud.DeleteFromModel();
    }

void ReadFileNames(BeXmlNodeP pFileNode, std::vector<std::string>& attrNames, std::vector<WString>& attrVals)
    {
    for (size_t i = 0; i < attrNames.size(); i++)
        {
        WString val;
        if (pFileNode->GetAttributeStringValue(val, attrNames[i].c_str()) != BEXML_Success)
            {
            printf("ERROR : data attribute not found\r\n");
            assert(false);
            }
        attrVals.push_back(val);
        }
    }

struct ParameterDescription
    {
    float minVal;
    float maxVal;
    float defaultVal;
    bool isInteger;
    float currentVal;
    std::string name;
    };

void ReadParameters(std::ifstream& fileStream, std::vector<ParameterDescription>& params)
    {
    std::string line;
    size_t lineNum = 0;
    while (std::getline(fileStream, line))
        {
        size_t n = 0;
        std::stringstream lineStr(line);
        std::string token;
        while (std::getline(lineStr, token, ','))
            {
            ParameterDescription par;
            par.name = token;
            switch (lineNum)
                {
                case 0:
                    params.push_back(par);
                    break;
                case 1:
                    params[n].defaultVal = std::atof(token.c_str());
                    break;
                case 2:
                    params[n].minVal = std::atof(token.c_str());
                    break;
                case 3:
                    params[n].maxVal = std::atof(token.c_str());
                    break;
                case 4:
                    params[n].isInteger = std::atoi(token.c_str()) != 0;
                    break;
                default:break;
                }
            n++;
            }
        lineNum++;
        }
    }

struct ErrorVariations
    {
    std::array<double, 5> averages;
    std::array<double, 5>  minima;
    std::array<double, 5>  maxima;

    ErrorVariations()
        {
        averages = { { 0, 0, 0, 0, 0 } };
        maxima = minima = { { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() } };
        }
    void ComputeFrom(std::vector<CompareClassifResult>& results)
        {
        for (auto res : results)
            {
            averages[0] += res.nTypeIErrorpercent;
            averages[1] += res.nTypeIIErrorpercent;
            averages[2] += res.totalError;
            averages[3] += res.sensitivity;
            averages[4] += res.specificity;
            minima[0] = _isnan(minima[0]) || res.nTypeIErrorpercent < minima[0] ? res.nTypeIErrorpercent : minima[0];
            minima[1] = _isnan(minima[1]) || res.nTypeIIErrorpercent < minima[1] ? res.nTypeIIErrorpercent : minima[1];
            minima[2] = _isnan(minima[2]) || res.totalError < minima[2] ? res.totalError : minima[2];
            minima[3] = _isnan(minima[3]) || res.sensitivity < minima[3] ? res.sensitivity : minima[3];
            minima[4] = _isnan(minima[4]) || res.specificity < minima[4] ? res.specificity : minima[4];
            maxima[0] = _isnan(maxima[0]) || res.nTypeIErrorpercent > maxima[0] ? res.nTypeIErrorpercent : maxima[0];
            maxima[1] = _isnan(maxima[1]) || res.nTypeIIErrorpercent > maxima[1] ? res.nTypeIIErrorpercent : maxima[1];
            maxima[2] = _isnan(maxima[2]) || res.totalError > maxima[2] ? res.totalError : maxima[2];
            maxima[3] = _isnan(maxima[3]) || res.sensitivity > maxima[3] ? res.sensitivity : maxima[3];
            maxima[4] = _isnan(maxima[4]) || res.specificity > maxima[4] ? res.specificity : maxima[4];
            }
        averages[0] /= results.size();
        averages[1] /= results.size();
        averages[2] /= results.size();
        averages[3] /= results.size();
        averages[4] /= results.size();
        }
    };

void SelectParams(std::vector<ParameterDescription>& params)
    {
    std::default_random_engine generator(time(0));
    std::stringstream configStream;
    for (auto& param : params)
        {
        if (param.isInteger)
            {
            std::uniform_int_distribution<int> distribution(param.minVal, param.maxVal);
            param.currentVal = (float)distribution(generator);
            }
        else
            {
            std::uniform_real_distribution<float> distribution(param.minVal, param.maxVal);
            param.currentVal = distribution(generator);
            }
        configStream << param.currentVal;
        if (&param - &params[0] != params.size()) configStream << ",";
        }
    GroundDetectionManager::SetConfigFromString(configStream.str().c_str());
    }

void FormatParams(std::vector<ParameterDescription>& params, std::string& paramString)
    {
    paramString.clear();
    std::stringstream paramStream;
    for (auto& param : params)
        {
        paramStream << param.name << "-" << std::to_string((long double)param.currentVal) << "--";
        }
    paramString = paramStream.str();
    }*/

void PerformGroundParametersTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    assert(false && "not supported");
    /*BeXmlNodeP pTestChildNode = pTestNode->GetFirstChild();
    std::vector<std::vector<WString>> filePaths;
    WString parameterConfig;
    if (pTestNode->GetAttributeStringValue(parameterConfig, "params") != BEXML_Success)
        {
        printf("ERROR : data attribute not found\r\n");
        assert(false);
        }
    while ((0 != pTestChildNode))
        {
        if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "dataset"))
            {
            std::vector<WString> values;
            std::vector<std::string> attrNames;
            attrNames.push_back("data");
            attrNames.push_back("baseline");
            ReadFileNames(pTestChildNode, attrNames, values);
            filePaths.push_back(values);
            }
        else
            {
            //printf("ERROR : unknown child node for test node\r\n");
            }
        pTestChildNode = pTestChildNode->GetNextSibling();
        }
    char* nameBuffer = new char[parameterConfig.GetMaxLocaleCharBytes()];
    std::ifstream paramFile;
    paramFile.open(parameterConfig.ConvertToLocaleChars(nameBuffer));
    delete[] nameBuffer;
    std::vector<ParameterDescription> params;
    ReadParameters(paramFile, params);
    while (true)
        {
        SelectParams(params);
        std::vector<CompareClassifResult> results;
        for (auto& paths : filePaths)
            {
            RunGroundDetection(paths[0]);
            CompareClassifResult res;
            CompareClassifications(paths[0], paths[1], res);
            results.push_back(res);
            char* nameBuffer = new char[paths[0].GetMaxLocaleCharBytes()];
            std::string pathClassif(paths[0].ConvertToLocaleChars(nameBuffer));
            size_t start_pos = pathClassif.find(".pod");
            pathClassif.replace(start_pos, pathClassif.length(), ".classif");
            std::remove(pathClassif.c_str());
            delete[] nameBuffer;
            }
        //L"Avg Type I error %%, Avg Type II error %%, Avg Total error %%, Avg Sensitivity %%, Avg Specificity %%, Min Type I error %%, Min Type II error %%, Min Total error %%, Min Sensitivity %%, Min Specificity %%, Max Type I error %%, Max Type II error %%, Max Total error %%, Max Sensitivity %%, Max Specificity %%, Starred, Parameters\n";
        ErrorVariations values;
        values.ComputeFrom(results);
        bool starred = false;
        std::string paramString = "abc";
        FormatParams(params, paramString);
        if (values.minima[3] > 40 && values.maxima[3] > 70 && values.minima[4] > 60 && values.maxima[4] > 98) starred = true;
        fwprintf(pResultFile, L"%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%s,%s\n",
                 values.averages[0], values.averages[1], values.averages[2], values.averages[3], values.averages[4],
                 values.minima[0], values.minima[1], values.minima[2], values.minima[3], values.minima[4],
                 values.maxima[0], values.maxima[1], values.maxima[2], values.maxima[3], values.maxima[4],
                 starred ? L"STARRED" : L"",
                 WString(paramString.c_str()).c_str());
        fflush(pResultFile);
        }*/
    }

void PerformNodeCreationTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    BeXmlStatus status;
    WString stmFileName;
    status = pTestNode->GetAttributeStringValue(stmFileName, "stmFileName");

    if (status != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        }
    int openStatus;
    IScalableMeshNodeCreatorPtr ptr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshNodeCreator::GetFor(stmFileName.c_str(), openStatus);
    IScalableMeshNodeEditPtr rootNode = ptr->AddNode(openStatus);
    DRange3d extent = DRange3d::FromMinMax(-1.0, 1.0);
    rootNode->SetNodeExtent(extent);
    rootNode->SetContentExtent(extent);
    DRange3d childExt = DRange3d::From(BENTLEY_NAMESPACE_NAME::DPoint3d::From(-1.0, -1.0), BENTLEY_NAMESPACE_NAME::DPoint3d::From(0.0, 0.0));
    IScalableMeshNodeEditPtr childNode = ptr->AddNode(rootNode, childExt, openStatus);
    childExt = DRange3d::From(BENTLEY_NAMESPACE_NAME::DPoint3d::From(-1.0, 0.0), BENTLEY_NAMESPACE_NAME::DPoint3d::From(0.0, 1.0));
    childNode = ptr->AddNode(rootNode, childExt, openStatus);
    childExt = DRange3d::From(BENTLEY_NAMESPACE_NAME::DPoint3d::From(0.0, -1.0), BENTLEY_NAMESPACE_NAME::DPoint3d::From(1.0, 0.0));
    childNode = ptr->AddNode(rootNode, childExt, openStatus);
    }

void PerformLoadingTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, linesFileName, name;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }

    int level = 0;
    if (pTestNode->GetAttributeInt32Value(level, "maxLevel") != BEXML_Success)
        {
        printf("Using default maxLevel value, all nodes will be loaded\r\n");
        }

    double t = clock();
    StatusInt status;
    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);
    size_t nbLoadedNodes = 0;

    status = stmFile->LoadAllNodeHeaders(nbLoadedNodes, level);
    assert(status == SUCCESS);

    t = clock() - t;

    fwprintf(pResultFile, L"%s,%.5f,%I64d\n",
             stmFileName.c_str(),
             (double)t / CLOCKS_PER_SEC,
             nbLoadedNodes);

    fflush(pResultFile);
    }

/*bool ReadElementsFromDgnFileAndLevel(ElementAgenda& agenda, DgnAttachment*& attachedDgn, WString dgnFileName, WString levelName)
    {
    DgnFileStatus fileOpenStatus;
    DgnDocumentPtr lineDoc = DgnDocument::CreateFromFileName(fileOpenStatus, dgnFileName.c_str(), NULL, DEFDGNFILE_ID, DgnDocument::FetchMode::Read);
    if (lineDoc == NULL)
        {
        return false;
        }
    //DgnModelRefP model = mdlModelRef_getActive();
    DgnModelRefP model = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef();
    model->CreateDgnAttachment(attachedDgn, *lineDoc->GetMonikerPtr(), L"");
    LevelCache& levelCache = attachedDgn->GetLevelCacheR();

    LevelHandle level = levelCache.GetLevelByName(levelName.c_str(), false);
    LevelId levelId = level.GetLevelId();
    //collect all elements from level
    ScanCriteriaP scP = new ScanCriteria();

    //mdlScanCriteria_setModel(scP, attachedDgn);
    //mdlScanCriteria_setReturnType(scP, MSSCANCRIT_ITERATE_ELMREF, false, true);
    //mdlScanCriteria_setElemRefCallback(scP, CollectAllElmsCallback, &agenda);
    BitMaskP  levelBitMask = BitMask::Create(false);
    //mdlBitMask_create(&levelBitMask, false);
    //mdlBitMask_setBit(levelBitMask, levelId - 1, 1);
    //mdlScanCriteria_setLevelTest(scP, levelBitMask, false, false);

    //mdlScanCriteria_scan(scP, NULL, NULL, NULL);

    //mdlScanCriteria_free(scP);

    scP->SetModelRef(attachedDgn);
    scP->SetReturnType(MSSCANCRIT_ITERATE_ELMREF, false, true);
    scP->SetElemRefCallback(CollectAllElmsCallback, &agenda);
    //levelBitMask->Create(false);
    levelBitMask->SetBit(levelId - 1, 1);
    scP->SetLevelTest(levelBitMask, false);
    scP->Scan();

    return true;
    }

void GatherPointsFromLineAgenda(std::map<DPoint3d, bool, DPoint3dZYXTolerancedSortComparison>& pointsContainer, size_t& number, ElementAgenda& agenda)
    {
    EditElementHandleP    curr = agenda.GetFirstP();
    EditElementHandleP end = curr + agenda.GetCount();
    number = 0;
    for (; curr < end; curr++) //For each valid element we do the draping
        {
        ElementHandle elemHandle = *curr;
        bvector<DPoint3d> origPoints;
        MSElementCP element = elemHandle.GetElementCP();
        switch (elemHandle.GetElementType())
            {
            case LINE_ELM:
                {
                origPoints.push_back(element->line_3d.start);
                origPoints.push_back(element->line_3d.end);
                break;
                }
            case LINE_STRING_ELM:
                {
                origPoints.resize(element->point_string_3d.numpts);
                memcpy(&origPoints[0], &element->point_string_3d.point[0], element->point_string_3d.numpts * sizeof(DPoint3d));
                break;
                }

            case CMPLX_STRING_ELM:
                {
                auto descr = elemHandle.GetElementDescrCP()->h.firstElem;
                while (descr != NULL)
                    {
                    switch (descr->el.ehdr.type)
                        {
                        case LINE_ELM:
                            {
                            origPoints.push_back(descr->el.line_3d.start);
                            origPoints.push_back(descr->el.line_3d.end);
                            break;
                            }
                        case LINE_STRING_ELM:
                            {
                            origPoints.resize(origPoints.size() + descr->el.point_string_3d.numpts);
                            memcpy(&origPoints[0], &descr->el.point_string_3d.point[0], descr->el.point_string_3d.numpts * sizeof(DPoint3d));
                            break;
                            }
                        default:
                            break;
                        }
                    descr = descr->h.next;
                    }

                }
            default:
                break;
            }
        if (origPoints.size() > 0) ++number;
        for (auto pt : origPoints)
            {
            DPoint3d toInsert = pt;
            pointsContainer.insert(make_pair(toInsert, false));
            }
        }
    }


bool FindMatchingElement(size_t& index, bvector<DPoint3d>& query, ElementAgenda& agenda)
    {
    EditElementHandleP    curr = agenda.GetFirstP();
    EditElementHandleP end = curr + agenda.GetCount();
    for (; curr < end; curr++) //For each valid element we do the draping
        {
        ElementHandle elemHandle = *curr;
        bvector<DPoint3d> origPoints;
        MSElementCP element = elemHandle.GetElementCP();
        switch (elemHandle.GetElementType())
            {
            case LINE_ELM:
                {
                origPoints.push_back(element->line_3d.start);
                origPoints.push_back(element->line_3d.end);
                break;
                }
            case LINE_STRING_ELM:
                {
                origPoints.resize(element->point_string_3d.numpts);
                memcpy(&origPoints[0], &element->point_string_3d.point[0], element->point_string_3d.numpts * sizeof(DPoint3d));
                break;
                }
            case CMPLX_STRING_ELM:
                {
                auto descr = elemHandle.GetElementDescrCP()->h.firstElem;
                while (descr != NULL)
                    {
                    switch (descr->el.ehdr.type)
                        {
                        case LINE_ELM:
                            {
                            origPoints.push_back(descr->el.line_3d.start);
                            origPoints.push_back(descr->el.line_3d.end);
                            break;
                            }
                        case LINE_STRING_ELM:
                            {
                            origPoints.resize(origPoints.size() + descr->el.point_string_3d.numpts);
                            memcpy(&origPoints[0], &descr->el.point_string_3d.point[0], descr->el.point_string_3d.numpts * sizeof(DPoint3d));
                            break;
                            }
                        default:
                            break;
                        }
                    descr = descr->h.next;
                    }
                }
            default:
                break;
            }
        if (query.size() != origPoints.size()) continue;
        size_t ptIdx = 0;
        for (; ptIdx < origPoints.size(); ++ptIdx)
            {
            if (!bsiDPoint3d_pointEqualTolerance(&origPoints[ptIdx], &query[ptIdx], 1)) break;
            }
        if (ptIdx == origPoints.size())
            {
            index = curr - (EditElementHandleP)agenda.GetFirstP();
            return true;
            }
        }
    return false;
    }*/

void ReadValuesFromCSV(double& val1, double& val2, WString fileName, WString rowName)
    {
    std::ifstream f;
    char* nameBuffer = new char[fileName.GetMaxLocaleCharBytes()];
    f.open(fileName.ConvertToLocaleChars(nameBuffer));
    std::string line;
    char* rowNameBuffer = new char[rowName.GetMaxLocaleCharBytes()];
    rowName.ConvertToLocaleChars(rowNameBuffer);
    while (std::getline(f, line))
        {
        std::string token;
        std::istringstream str(line);
        bool foundLine = false;
        size_t n = 0;
        while (std::getline(str, token, ','))
            {
            if (0 == BeStringUtilities::Stricmp(token.c_str(), rowNameBuffer))
                {
                foundLine = true;
                }
            else if (foundLine && n == 1) val1 = std::atof(token.c_str());
            else if (foundLine && n == 2) val2 = std::atof(token.c_str());
            ++n;
            }
        if (foundLine) break;
        }
    f.close();
    delete nameBuffer;
    delete rowNameBuffer;
    }

void PerformDrapeBaselineTest(BeXmlNodeP pTestNode, FILE* pResultFile, BeXmlNodeP pRootNode)
    {
    assert(false && "not supported");
    /*    BeXmlStatus status;
        WString stmFileName;
        status = pTestNode->GetAttributeStringValue(stmFileName, "stmFileName");

        if (status != BEXML_Success)
            {
            printf("ERROR : stmFileName attribute not found\r\n");
            }

        WString benchmarksFileName;
        status = pRootNode->GetAttributeStringValue(benchmarksFileName, "benchmarksFile");

        if (status != BEXML_Success)
            {
            printf("ERROR : benchmarksFile attribute not found\r\n");
            }

        WString linesFileName;
        status = pTestNode->GetAttributeStringValue(linesFileName, "lines");

        if (status != BEXML_Success)
            {
            printf("ERROR : lines attribute not found\r\n");
            }

        WString baselineFileName;
        status = pTestNode->GetAttributeStringValue(baselineFileName, "baseline");

        if (status != BEXML_Success)
            {
            printf("ERROR : baseline attribute not found\r\n");
            }

        WString testcaseName;
        status = pTestNode->GetAttributeStringValue(testcaseName, "name");

        if (status != BEXML_Success)
            {
            printf("ERROR : name attribute not found\r\n");
            }
        StatusInt openStatus;
        IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, openStatus);
        bool testcaseOutcome = true;
        if (openStatus != SUCCESS) testcaseOutcome = false;

        DgnAttachment* linesAttachment;
        ElementAgenda agenda;
        if (testcaseOutcome && !ReadElementsFromDgnFileAndLevel(agenda, linesAttachment, linesFileName, L"toDrape")) testcaseOutcome = false;

        bvector<bvector<DPoint3d>> pts;
        DTMPtr dtmP = dynamic_cast<Bentley::TerrainModel::IDTM*>(&*stmFile);
        if (testcaseOutcome && DoBatchDrape(&agenda, dtmP, pts) != SUCCESS) testcaseOutcome = false;
        ElementAgenda baselineLines;
        DgnAttachment* newAttachment;
        if (testcaseOutcome && !ReadElementsFromDgnFileAndLevel(baselineLines, newAttachment, baselineFileName, testcaseName)) testcaseOutcome = false;
        if (testcaseOutcome)
            {
            size_t nTestedOutputPts = 0;
            size_t nMismatchedPoints = 0;
            size_t nMatchedLines = 0;
            std::set<size_t> matchedLines;
            std::map<DPoint3d, bool, DPoint3dZYXTolerancedSortComparison> setOfPointsInOutputLines(DPoint3dZYXTolerancedSortComparison(1, 1));
            size_t nOfBaselines = 0;
            GatherPointsFromLineAgenda(setOfPointsInOutputLines, nOfBaselines, baselineLines);
            for (size_t i = 0; i < pts.size(); i++)
                {
                size_t foundIndex;
                if (FindMatchingElement(foundIndex, pts[i], baselineLines))
                    {
                    matchedLines.insert(foundIndex);
                    nMatchedLines++;
                    }
                for (auto& pt : pts[i])
                    {
                    ++nTestedOutputPts;
                    if (setOfPointsInOutputLines.count(pt) != 0) setOfPointsInOutputLines[pt] = true;
                    else ++nMismatchedPoints;
                    }
                }
            int64_t nOfLinesToDrape;
            int64_t nOfLinesDraped;
            double timeOfDrape1st, timeOfDrapeCached;
            double timeOfDrape1stBaseline, timeOfDrapeCachedBaseline;
            ReadValuesFromCSV(timeOfDrape1stBaseline, timeOfDrapeCachedBaseline, benchmarksFileName, testcaseName);
            IScalableMeshATP::GetInt(L"nOfLines", nOfLinesToDrape);
            IScalableMeshATP::GetInt(L"nOfLinesDraped", nOfLinesDraped);
            IScalableMeshATP::GetDouble(L"drapeTime", timeOfDrape1st);
            pts.clear();
            baselineLines.Clear();
            //mdlModelRef_getActive()->DeleteDgnAttachment(newAttachment);
            ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->DeleteDgnAttachment(newAttachment);
            IScalableMeshATP::StoreInt(L"nOfLines", 0);
            IScalableMeshATP::StoreInt(L"nOfLinesDraped", 0);
            IScalableMeshATP::StoreDouble(L"drapeTime", 0);
            stmFile = nullptr;
            stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, openStatus);
            DoBatchDrape(&agenda, dtmP, pts);
            IScalableMeshATP::GetDouble(L"drapeTime", timeOfDrapeCached);
            IScalableMeshATP::StoreDouble(L"drapeTime", 0);
            agenda.Clear();
            baselineFileName.ReplaceAll(L",", L" ");
            //mdlModelRef_getActive()->DeleteDgnAttachment(linesAttachment);
            ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->DeleteDgnAttachment(linesAttachment);
            //L"Test Case,  Pass/Fail, Baseline, Nb of Lines, Nb of Lines Draped (baseline), Nb of Lines Draped(test), Nb Of Different Lines, %% unmatched points, Time to drape (1st load) (baseline), Time to drape (1st load) (test), Time taken (1st load) variation, Time to drape (cached) (baseline), Time to drape (cached) (test), Time taken (cached) variation\n";
            fwprintf(pResultFile, L"%s,%s,%s,%I64d,%I64d,%I64d,%I64d, %0.5f, %0.5f, %0.5f, %0.5f, %0.5f, %0.5f, %0.5f\n",
                     stmFileName.c_str(),
                     nMismatchedPoints == 0 && nOfLinesDraped == nOfBaselines ? L"PASS" : L"FAIL",
                     baselineFileName.c_str(),
                     nOfLinesToDrape,
                     nOfBaselines,
                     nOfLinesDraped,
                     nOfBaselines - nMatchedLines,
                     (100.0*nMismatchedPoints) / nTestedOutputPts,
                     timeOfDrape1stBaseline,
                     timeOfDrape1st,
                     (timeOfDrape1st - timeOfDrape1stBaseline)*100.0 / timeOfDrape1stBaseline,
                     timeOfDrapeCachedBaseline,
                     timeOfDrapeCached,
                     (timeOfDrapeCached - timeOfDrapeCachedBaseline)*100.0 / timeOfDrapeCachedBaseline);
            }
        else
            {
            fwprintf(pResultFile, L"%s\n",
                     L"ERROR");
            }
        fflush(pResultFile);
        */
    }

bool TriangleWithinClosedFeature(const DPoint3d* triangle, const std::vector<DPoint3d>& feature)
    {
    if (bsiGeom_testXYPolygonConvex(&feature[0], (int)feature.size()))
        {
        for (size_t i = 0; i < 3; ++i)
            if (!bsiGeom_isXYPointInConvexPolygon(&triangle[i], &feature[0], (int)feature.size(), 0)) return false;
        }
    else
        {
        bvector<int> idx;
        vu_triangulateSpacePolygon(&idx, &const_cast<DPoint3d&>(feature[0]), (int)feature.size(), 1e-8, 0, true);
        bvector<bvector<DPoint3d>> convexPolys;
        bvector<DPoint3d> poly;
        for (size_t j = 0; j < idx.size(); ++j)
            {
            if (idx[j] == 0 && poly.size() > 0)
                {
                convexPolys.push_back(poly);
                poly.clear();
                }
            else if (idx[j] > 0) poly.push_back(feature[idx[j] - 1]);
            }
        if (poly.size() > 0)
            {
            convexPolys.push_back(poly);
            poly.clear();
            }
        bool trianglePts[3] = { false, false, false };
        for (auto& polygon : convexPolys)
            {
            for (size_t i = 0; i < 3; ++i)
                if (bsiGeom_isXYPointInConvexPolygon(&triangle[i], &polygon[0], (int)polygon.size(), 0)) trianglePts[i] = true;
            }
        return (trianglePts[0] && trianglePts[1] && trianglePts[2]);
        }
    return true;
    }

bool TriangleWithinClosedFeatureSet(const DPoint3d* triangle, const std::vector<std::vector<DPoint3d>>& features)
    {
    for (size_t i = 0; i < features.size(); ++i)
        {
        if (TriangleWithinClosedFeature(triangle, features[i])) return true;
        }
    return false;
    }

bool ValidateTriangleConstraints(const DPoint3d* triangle, DTMFeatureType feaType, const std::vector<DPoint3d>& featurePts, const std::vector<std::vector<DPoint3d>>& voids, const std::vector<std::vector<DPoint3d>>& islands)
    {
    for (size_t i = 0; i < featurePts.size() - 1; ++i)
        {
        DSegment3d edge = DSegment3d::From(featurePts[i], featurePts[i + 1]);
        for (size_t j = 0; j < 3; ++j)
            {
            DSegment3d triangleEdge = DSegment3d::From(triangle[j], triangle[(j + 1) % 3]);
            double f0, f1;
            DPoint3d pt0, pt1;
            if (DSegment3d::IntersectXY(f0, f1, pt0, pt1, edge, triangleEdge) && f0 > 1.0e-5 && f0 < 1 - 1.0e-5 && f1 > 1.0e-5 && f1 < 1 - 1.0e-5)
                {
                return false;
                }
            }
        }
    if ((feaType == DTMFeatureType::Void || feaType == DTMFeatureType::Hole ||
         feaType == DTMFeatureType::DrapeVoid || feaType == DTMFeatureType::BreakVoid)
        && !TriangleWithinClosedFeatureSet(triangle, islands) && TriangleWithinClosedFeature(triangle, featurePts))
        {
        return false;
        }
    return true;
    }

bool ValidateFeatureDefinition(size_t& nErrors, IScalableMesh* scMeshP, DTMFeatureType feaType, const std::vector<DPoint3d>& featurePts, const std::vector<std::vector<DPoint3d>>& voids, const std::vector<std::vector<DPoint3d>>& islands)
    {
    //types of errors: 
    //a void feature contains triangles that are not also in an island;
    //a mesh triangle intersects the feature edges
    nErrors = 0;
    DRange3d range = DRange3d::From(&featurePts[0], (int)featurePts.size());
    IScalableMeshMeshQueryPtr meshQueryInterface = scMeshP->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    DRange3d fileRange;
    scMeshP->GetRange(fileRange);

    DPoint3d box[4] = {
        DPoint3d::From(range.low.x, range.low.y, fileRange.low.z),
        DPoint3d::From(range.low.x, range.high.y, fileRange.low.z),
        DPoint3d::From(range.high.x, range.low.y, fileRange.high.z),
        DPoint3d::From(range.high.x, range.high.y, fileRange.high.z)
        };
    meshQueryInterface->Query(returnedNodes, box, 4, params);

    for (auto& node : returnedNodes)
        {
        bvector<bool> clips;
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        flags->SetLoadGraph(false);
        auto mesh = node->GetMesh(flags, clips);
        auto polyfaceP = mesh->GetPolyfaceQuery();
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyfaceP, true);
        for (visitor->Reset(); visitor->AdvanceToNextFace();)
            {
            if (!ValidateTriangleConstraints(visitor->GetPointCP(), feaType, featurePts, voids, islands))++nErrors;
            }
        }

    return nErrors == 0;
    }

void PerformConstraintTest(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, featuresFileName;
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }
    if (pTestNode->GetAttributeStringValue(featuresFileName, "featuresFileName") != BEXML_Success)
        {
        printf("ERROR : featuresFileName attribute not found\r\n");
        return;
        }
    StatusInt createStatus;
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(stmFileName.c_str(), createStatus));

    if (creatorPtr == 0)
        {
        printf("ERROR : cannot create STM file\r\n");
        }
    else
        {
        WString gcsKeyName;

        auto status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

        if (status == BEXML_Success)
            {
            BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
            StatusInt status = creatorPtr->SetBaseGCS(baseGCSPtr);
            assert(status == SUCCESS);
            }
        if (ParseSourceSubNodes(creatorPtr->EditSources(), pTestNode) == true)
            {
            StatusInt status = creatorPtr->Create();

            creatorPtr->SaveToFile();
            creatorPtr = nullptr;
            if (status == SUCCESS)
                {
                IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

                if (stmFile != 0)
                    {
                    char* nameBuffer = new char[featuresFileName.GetMaxLocaleCharBytes()];
                    std::ifstream featureFile;
                    featureFile.open(featuresFileName.ConvertToLocaleChars(nameBuffer));
                    std::vector<std::pair<std::vector<DPoint3d>, DTMFeatureType>> featureDefs;
                    ReadFeatureFile(featureFile, featureDefs);
                    size_t featurePoints = 0;
                    size_t validDefs = 0;
                    size_t nTotalErrors = 0;
                    std::vector<std::vector<DPoint3d>> voids;
                    std::vector<std::vector<DPoint3d>> islands;
                    for (auto& def : featureDefs)
                        {
                        if (def.second == DTMFeatureType::Void || def.second == DTMFeatureType::Hole || def.second == DTMFeatureType::DrapeVoid ||
                            def.second == DTMFeatureType::BreakVoid)
                            {
                            voids.push_back(def.first);
                            }
                        else if (def.second == DTMFeatureType::Island)
                            islands.push_back(def.first);
                        }
                    for (auto& def : featureDefs)
                        {
                        size_t nTriangleErrors;
                        featurePoints += def.first.size();
                        bool result = ValidateFeatureDefinition(nTriangleErrors, stmFile.get(), def.second, def.first, voids, islands);
                        if (result) validDefs++;
                        nTotalErrors += nTriangleErrors;
                        }
                    featureFile.close();
                    //L"Test Case,  Pass/Fail, Nb of constraints, Nb of constraint points, Nb of valid constraints, Constraint error rate(%%), Nb of triangles violating constraints\n";
                    fwprintf(pResultFile, L"%s,%s, %I64d, %I64d, %I64d, %.5f, %I64d\n",
                             stmFileName.c_str(),
                             validDefs == featureDefs.size() ? L"PASS" : L"FAIL",
                             featureDefs.size(),
                             featurePoints,
                             validDefs,
                             (featureDefs.size() - validDefs)*100.0 / featureDefs.size(),
                             nTotalErrors);
                    fflush(pResultFile);
                    return;
                    }
                }
            }
        }
    fwprintf(pResultFile, L"ERROR\n");

    fflush(pResultFile);
    }

static HPMPool* s_rasterMemPool = nullptr;
/*
void readJPG(WString texturePath, bvector<Byte>& textureTmp, size_t& sizeTmp)
    {
    int textureWidthInPixels = 1024, textureHeightInPixels = 1024;

    HFCPtr<HRFRasterFile>  pRasterFile;
    HFCPtr<HFCURL>         pRasterFileURL;

    WString fileName = WString(L"file://") + texturePath;

    pRasterFileURL = HFCURL::Instanciate(fileName);

    pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(pRasterFileURL, TRUE);


    //HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));
    HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
    s_rasterMemPool = new HPMPool(300000, HPMPool::KeepLastBlock);
    auto cluster = new HGFHMRStdWorldCluster();
    pLogicalCoordSys = cluster->GetWorldReference(pRasterFile->GetPageWorldIdentificator(0));
    HFCPtr<HRSObjectStore> pObjectStore;
    HFCPtr<HRARaster>      pRaster;
    pObjectStore = new HRSObjectStore(s_rasterMemPool,
                                      pRasterFile,
                                      0,
                                      pLogicalCoordSys);
    // Get the raster from the store
    pRaster = pObjectStore->LoadRaster();
    HASSERT(pRaster != NULL);

    HFCPtr<HRABitmap> pTextureBitmap;
    //HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV24R8G8B8());
    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());
    HFCPtr<HCDCodec> pCodec(new HCDCodecIdentity());
    //int nOfChannels = 3;
    int nOfChannels = 4;
    byte* pixelBufferP = new byte[textureWidthInPixels * textureHeightInPixels * nOfChannels + 3 * sizeof(int)];
    memcpy(pixelBufferP, &textureWidthInPixels, sizeof(int));
    memcpy(pixelBufferP + sizeof(int), &textureHeightInPixels, sizeof(int));
    memcpy(pixelBufferP + 2 * sizeof(int), &nOfChannels, sizeof(int));
    pTextureBitmap = new HRABitmap(textureWidthInPixels,
                                   textureHeightInPixels,
                                   nullptr,
                                   pRaster->GetCoordSys(),//nullptr,//sourceRasterP->GetCoordSys(),
                                   pPixelType,
                                   8,
                                   pCodec);

    pTextureBitmap->GetPacket()->SetBuffer(pixelBufferP + 3 * sizeof(int), textureWidthInPixels * textureHeightInPixels * nOfChannels);
    pTextureBitmap->GetPacket()->SetBufferOwnership(false);

    HRAClearOptions clearOptions;

    //CR 332863 - Quick trick to display the STM outside in smooth outside the area where texture data are available.
    //              Note that this trick will lead to the translucent color shown throughout transparent raster being a shade of gray
    //              instead of the color of the background.
    uint32_t whiteOpaque = 0xFFFFFFFF;

    clearOptions.SetRawDataValue(&whiteOpaque);

    pTextureBitmap->Clear(clearOptions);

    HRACopyFromOptions copyFromOptions;

    //Rasterlib set this option on the last tile of a row or a column to avoid black lines.
    copyFromOptions.SetGridShapeMode(true);
    //copyFromOptions.SetAlphaBlend(true);
    copyFromOptions.SetAlphaBlend(false);

    //pTextureBitmap->CopyFrom(sourceRasterP, copyFromOptions);
    pTextureBitmap->CopyFrom(pRaster, copyFromOptions);

    sizeTmp = textureWidthInPixels * textureHeightInPixels * nOfChannels;
    textureTmp.resize(sizeTmp);
    memcpy(&textureTmp[0], pixelBufferP, sizeTmp);
    /*std::ofstream file_s;
    file_s.open("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\test_SDK\\binary1.txt", ios_base::binary);
    file_s.write((char*)pixelBufferP, sizeTmp);
    file_s.close();*/
    //    }


void PerformStreaming(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString stmFileName, stmStreamFileName, name, result;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(stmFileName, "stmFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }

    if (pTestNode->GetAttributeStringValue(stmStreamFileName, "stmStreamFileName") != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        return;
        }

    // First test if folder / files exists

    StatusInt status;
    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

    IScalableMeshPtr stmStreamFile = IScalableMesh::GetFor(stmStreamFileName.c_str(), true, true, status);

    int64_t pointCount = 0, pointCountStream = 0;
    bool pointCountPass, allTestPass, nodeCountPass, pointCountNodePass;
    pointCountNodePass = true;
    allTestPass = true;
    pointCountPass = false;

    if (stmFile != 0 && stmStreamFile != 0)
        {
        pointCount = stmFile->GetPointCount();
        pointCountStream = stmStreamFile->GetPointCount();
        result = L"SUCCESS";
        pointCountPass = pointCount == pointCountStream;
        }
    else
        {
        result = L"FAILURE";
        allTestPass = false;
        }
    nodeCountPass = false;
    pointCountNodePass = true;


    // TestNodes
    size_t nbLoadedNodes = 0, nbLoadedStreamedNodes = 0;
    double t = clock();
    stmFile->LoadAllNodeHeaders(nbLoadedNodes,0);
    t = clock() - t;
    double tStream = clock();
    stmStreamFile->LoadAllNodeHeaders(nbLoadedStreamedNodes, 0);
    tStream = clock() - tStream;

    if (nbLoadedNodes == nbLoadedStreamedNodes)
        nodeCountPass = true;


    //get meshes from all nodes
    IScalableMeshMeshQueryPtr meshQueryInterface = stmFile->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    meshQueryInterface->Query(returnedNodes, 0, 0, params);

    IScalableMeshMeshQueryPtr meshQueryInterfaceStreaming = stmStreamFile->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodesStreaming;
    IScalableMeshMeshQueryParamsPtr paramsStreaming = IScalableMeshMeshQueryParams::CreateParams();
    meshQueryInterface->Query(returnedNodesStreaming, 0, 0, paramsStreaming);

    assert(returnedNodes.size() == returnedNodesStreaming.size());
    int j = 0;
    for (auto& node : returnedNodes)
        {
        bvector<bool> clips;
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        flags->SetLoadGraph(false);
        IScalableMeshMeshPtr mesh = node->GetMesh(flags, clips);
        const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* polyface = mesh->GetPolyfaceQuery();

        bvector<bool> clipsStreaming;
        IScalableMeshMeshPtr meshStreaming = returnedNodesStreaming[j]->GetMesh(flags, clips);
        const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* polyfaceStreaming = meshStreaming->GetPolyfaceQuery();

        DPoint3d point;
        DPoint3d pointStreaming;
        int status = SUCCESS;

        for (size_t i = 0; i < polyface->GetPointIndexCount() && status == SUCCESS; i++)
            {
            point = polyface->GetPointCP()[polyface->GetPointIndexCP()[i] - 1];
            pointStreaming = polyfaceStreaming->GetPointCP()[polyfaceStreaming->GetPointIndexCP()[i] - 1];

            if (point.x != pointStreaming.x || point.y != pointStreaming.y || point.z != pointStreaming.z)
                {
                pointCountNodePass = false;
                break;
                }
            }
        j++;
        }




    fwprintf(pResultFile, L"%s,%s,%s,%s,%s,%0.5f,%0.5f,%s\n",
             stmFileName.c_str(),
             stmStreamFileName.c_str(),
             allTestPass ? L"true" : L"false",
             pointCountPass ? L"true" : L"false",
             nodeCountPass ? L"true" : L"false",
             (double)t / CLOCKS_PER_SEC,
             (double)tStream / CLOCKS_PER_SEC,
             pointCountNodePass ? L"true" : L"false"
             );

    fflush(pResultFile);
    }

void PerformSCMToCloud(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    WString scmFileName, cloudOutDirPath, result;
    // Parses the test(s) definition:
    if (pTestNode->GetAttributeStringValue(scmFileName, "scmFileName") != BEXML_Success)
        {
        printf("ERROR : scmFileName attribute not found\r\n");
        return;
        }

    if (pTestNode->GetAttributeStringValue(cloudOutDirPath, "cloudOutDirPath") != BEXML_Success || cloudOutDirPath.compare(L"") == 0 || cloudOutDirPath.compare(L"default") == 0)
        {
        // Use default path to output files
        auto position = scmFileName.find_last_of(L".stm");
        cloudOutDirPath = scmFileName.substr(0, position - 3) + L"_stream\\";
        }

    bool allTestPass = true;
    double t = 0;

    // Check existence of scm file
    StatusInt status;
    IScalableMeshPtr scmFile = IScalableMesh::GetFor(scmFileName.c_str(), true, true, status);

    if (scmFile != 0 && status == SUCCESS)
        {
        t = clock();
        status = scmFile->ConvertToCloud(cloudOutDirPath);
        t = clock() - t;
        result = SUCCESS == status ? L"SUCCESS" : L"FAILURE -> could not convert scm file";
        }
    else
        {
        result = L"FAILURE -> could not open scm file";
        allTestPass = false;
        }

    fwprintf(pResultFile, L"%s,%s,%s,%0.5f\n",
             scmFileName.c_str(),
             cloudOutDirPath.c_str(),
             allTestPass ? L"true" : L"false",
             (double)t / CLOCKS_PER_SEC
             );

    fflush(pResultFile);
    }

bool GetMeshAsSingleTileDTM(IScalableMesh* meshP, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& tmP)
    {
    int            status;
    DTMPtr         dtmPtr;
    IScalableMeshFixResolutionIndexQueryParamsPtr queryParamsPtr(IScalableMeshFixResolutionIndexQueryParams::CreateParams());
    queryParamsPtr->SetResolutionIndex(meshP->GetNbResolutions() - 1);
    IScalableMeshPointQueryPtr fixResPointQueryPtr(meshP->GetQueryInterface(SCM_QUERY_FIX_RESOLUTION_VIEW));
    bvector<DPoint3d> points;
    status = fixResPointQueryPtr->Query(points, 0, 0, IScalableMeshQueryParametersPtr(queryParamsPtr));

    BcDTMPtr bcDtmInMemPtr(BcDTM::Create());

    bcDtmInMemPtr->AddPoints(points);
    bcDtmInMemPtr->SetTriangulationParameters(0, 0, 0, 0);
    bcDtmInMemPtr->Triangulate();

    tmP = bcDtmInMemPtr;

    return status == DTM_SUCCESS;
    }

void GenerateLine(bvector<DPoint3d>& polyLine, DRange3d& range)
    {
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);
    std::uniform_int_distribution<int> nOfPts(2, 12);
    int nPts = nOfPts(e1);
    for (int i = 0; i < nPts; ++i)
        {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        polyLine.push_back(pt);
        }

    }

void GenerateLinesToDrapeOnExtent(bvector<bvector<DPoint3d>>& lines, int nLines, DRange3d& range)
    {
    for (int i = 0; i < nLines; i++)
        {
        bvector<DPoint3d> polyLine;
        GenerateLine(polyLine, range);
        lines.push_back(polyLine);
        }
    }

void DrapeLinesOnScalableMesh(bvector<bvector<DPoint3d>>&lines, bvector<bvector<DPoint3d>>& drapedLines, DTMPtr& dtmP2)
    {
    for (auto& line : lines)
        {
        bvector<DPoint3d> lineDraped;
        auto draping = dtmP2->GetDTMDraping();
        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMDrapedLinePtr drapedLine;
        draping->DrapeLinear(drapedLine, line.data(), (int)line.size());
        if (!drapedLine.IsNull())
            {
            unsigned int numPoints = drapedLine->GetPointCount();
            for (unsigned int ptNum = 0; ptNum < numPoints; ptNum++)
                {
                DPoint3d pt;
                drapedLine->GetPointByIndex(&pt, nullptr, nullptr, ptNum);
                lineDraped.push_back(pt);
                }
            bvector<DPoint3d>::iterator it = unique(lineDraped.begin(), lineDraped.end(), DPoint3dEqualityTest);
            lineDraped.resize(std::distance(lineDraped.begin(), it));
            }
        drapedLines.push_back(lineDraped);
        }
    }

void DrapeLinesOnDTM(bvector<bvector<DPoint3d>>&lines, bvector<bvector<DPoint3d>>& drapedLinesTM, DTMPtr& tmPtr)
    {
    for (auto& line : lines)
        {
        bvector<DPoint3d> lineDraped;
        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMDrapedLinePtr drapedLine;
        tmPtr->GetBcDTM()->DrapeLinear(drapedLine, &line[0], (int)line.size());
        if (!drapedLine.IsNull())
            {
            unsigned int numPoints = drapedLine->GetPointCount();
            for (unsigned int ptNum = 0; ptNum < numPoints; ptNum++)
                {
                DPoint3d pt;
                DTMDrapedLineCode code;
                drapedLine->GetPointByIndex(&pt, nullptr, &code, ptNum);
                if (code != DTMDrapedLineCode::External && code != DTMDrapedLineCode::InVoid) lineDraped.push_back(pt);
                }
            bvector<DPoint3d>::iterator it = unique(lineDraped.begin(), lineDraped.end(), DPoint3dEqualityTest);
            lineDraped.resize(std::distance(lineDraped.begin(), it));
            }
        drapedLinesTM.push_back(lineDraped);
        }
    }

void PerformTestDrapeRandomLines(BeXmlNodeP pTestNode, FILE* pResultFile)
    {
    BeXmlStatus status;
    WString stmFileName;
    status = pTestNode->GetAttributeStringValue(stmFileName, "stmFileName");

    if (status != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        }
    WString nLines;
    status = pTestNode->GetAttributeStringValue(nLines, "numberOfLines");

    if (status != BEXML_Success)
        {
        printf("ERROR : stmFileName attribute not found\r\n");
        }
    Utf8String utf8(nLines);
    int nLine = atoi(utf8.c_str());
    StatusInt openStatus;
    IScalableMeshPtr stmFile = IScalableMesh::GetFor(stmFileName.c_str(), true, true, openStatus);
    bvector<bvector<DPoint3d>> lines, drapedLines, drapedLinesTM, drapedLines2, lines2;
    if (openStatus == SUCCESS)
        {
        DTMPtr tmPtr;
        GetMeshAsSingleTileDTM(stmFile.get(), tmPtr);
        WString stmFileName2 = stmFileName + L"2.stm";
        DRange3d range;
        stmFile->GetRange(range);
        range.ScaleAboutCenter(range, 0.80);
        GenerateLinesToDrapeOnExtent(lines, nLine, range);
        lines2 = lines;
        clock_t start = clock();
        double timeToDrapeSM = (double)(clock() - start) / CLOCKS_PER_SEC;
        start = clock();
        DrapeLinesOnDTM(lines, drapedLinesTM, tmPtr);
        double timeToDrapeTM = (double)(clock() - start) / CLOCKS_PER_SEC;
        DTMPtr d = stmFile->GetDTMInterface();
        start = clock();
        DrapeLinesOnScalableMesh(lines, drapedLines, d);
        double timeToDrapeSM2 = (double)(clock() - start) / CLOCKS_PER_SEC;
        size_t nDiffLines = 0;
        WString name = stmFileName;
        name.ReplaceAll(L".stm", L"");
        size_t pos = name.find_last_of(L"\\");
        if (pos != std::string::npos) name = name.substr(pos + 1);
        WString testcaseNameTM = name + L"_tm";
        WString testcaseNameLine = name + L"_line";
        WString testcaseNameOrig = name + L"_orig";

        // Copy all lines in dgn, didn't need this.

        for (auto& line : lines)
            {
            size_t lineN = &line - &lines[0];

            size_t nPtsDraped = drapedLines[lineN].size();
            double length = 0.0;
            for (size_t i = 1; i < drapedLines[lineN].size(); ++i)
                {
                DVec3d aToB = DVec3d::FromStartEnd(drapedLines[lineN][i - 1], drapedLines[lineN][i]);
                length += aToB.Magnitude();
                }
            size_t nPtsDrapedDTM = drapedLinesTM[lineN].size();
            double lengthDTM = 0.0;
            for (size_t i = 1; i < drapedLinesTM[lineN].size(); ++i)
                {
                DVec3d aToB = DVec3d::FromStartEnd(drapedLinesTM[lineN][i - 1], drapedLinesTM[lineN][i]);
                lengthDTM += aToB.Magnitude();
                }

            // L"Test Case, Line Number, N Of Points Draped (SM), N Of Points Draped (Civil), Length (SM), Length (Civil), N Of Points Difference (%%), Length Difference (%%), NDifferentLines Total, Time total(SM) (s), Time total(Civil) (s)\n";
            fwprintf(pResultFile, L"%s,%I64d,%I64d,%I64d,%0.5f,%0.5f,%0.5f, %0.5f\n",
                     stmFileName.c_str(),
                     lineN,
                     nPtsDraped,
                     nPtsDrapedDTM,
                     length,
                     lengthDTM,
                     100.0*(std::max(nPtsDraped, nPtsDrapedDTM) - std::min(nPtsDraped, nPtsDrapedDTM)) / std::max(nPtsDraped, nPtsDrapedDTM),
                     (std::max(length, lengthDTM) - std::min(length, lengthDTM)) / std::max(length, lengthDTM) * 100.0);
            ++lineN;
            if (fabs(lengthDTM - length) > 1e-3 || nPtsDraped != nPtsDrapedDTM) ++nDiffLines;
            }
        fwprintf(pResultFile, L"%s,,,,,,,,%I64d,%0.5f,%0.5f,%0.5f\n",
                 stmFileName.c_str(),
                 nDiffLines,
                 timeToDrapeSM,
                 timeToDrapeTM,
                 timeToDrapeSM2
                 );
        d = stmFile->GetDTMInterface();
        fwprintf(pResultFile, L"Timings for 10 iterations\n");
        start = clock();

        double timeToDrapeSM10 = (double)(clock() - start) / CLOCKS_PER_SEC;
        start = clock();
        for (size_t i = 0; i < 10; ++i)
            {
            DrapeLinesOnDTM(lines2, drapedLinesTM, tmPtr);
            }
        double timeToDrapeTM10 = (double)(clock() - start) / CLOCKS_PER_SEC;
        start = clock();
        for (size_t i = 0; i < 10; ++i)
            {
            DrapeLinesOnScalableMesh(lines2, drapedLines2, d);
            }
        double timeToDrapeSM210 = (double)(clock() - start) / CLOCKS_PER_SEC;
        fwprintf(pResultFile, L"%s,,,,,,,,%I64d,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f\n",
                 stmFileName.c_str(),
                 nDiffLines,
                 timeToDrapeSM10,
                 timeToDrapeTM10,
                 timeToDrapeSM210,
                 timeToDrapeSM10 / 10.0,
                 timeToDrapeTM10 / 10.0,
                 timeToDrapeSM210 / 10.0
                 );
        fwprintf(pResultFile, L"Timings for 100 iterations\n");
        start = clock();

        double timeToDrapeSM100 = (double)(clock() - start) / CLOCKS_PER_SEC;
        start = clock();
        for (size_t i = 0; i < 100; ++i)
            {
            DrapeLinesOnDTM(lines2, drapedLinesTM, tmPtr);
            }
        double timeToDrapeTM100 = (double)(clock() - start) / CLOCKS_PER_SEC;
        start = clock();
        for (size_t i = 0; i < 100; ++i)
            {
            DrapeLinesOnScalableMesh(lines2, drapedLines2, d);
            }
        double timeToDrapeSM2100 = (double)(clock() - start) / CLOCKS_PER_SEC;
        fwprintf(pResultFile, L"%s,,,,,,,,%I64d,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f\n",
                 stmFileName.c_str(),
                 nDiffLines,
                 timeToDrapeSM100,
                 timeToDrapeTM100,
                 timeToDrapeSM2100,
                 timeToDrapeSM100 / 100.0,
                 timeToDrapeTM100 / 100.0,
                 timeToDrapeSM2100 / 100.0
                 );
        }
    else
        {
        fwprintf(pResultFile, L"%s\n",
                 L"ERROR");
        }
    fflush(pResultFile);
    }