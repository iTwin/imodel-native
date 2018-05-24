//#include "ScalableMeshATPPch.h"
#include "ScalableMeshWorker.h"

#include <Bentley\BeDirectoryIterator.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP\all\h\HRFFileFormats.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include "Initialize.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_IMAGEPP


namespace ScalableMeshWorker
    {

    TaskScheduler::TaskScheduler(BeFileName& taskFolderName)
        {
        m_taskFolderName = taskFolderName;
        }

    TaskScheduler::~TaskScheduler()
        {
        }

    void TaskScheduler::Start()
        {

/*
        BENTLEYDLL_EXPORT StatusInt GetCurrentEntry(BeFileName& name, bool& isDir, bool fullPath = true);

        //! Move to the next directory entry
        BENTLEYDLL_EXPORT StatusInt ToNext();
*/            
        bool isThereTaskAvaialble = true;
    
        while (isThereTaskAvaialble)
            { 
            isThereTaskAvaialble = false;

            BeDirectoryIterator dirIter(taskFolderName);
            
            while (dirIter->ToNext() == SUCCESS)
                {
                isThereTaskAvaialble = true; 

                BeFileName name;
                bool isDir;

                StatusInt status = dirIter->GetCurrentEntry(name, isDir);

                if (status == SUCCESS && isDir == false)
                    {
                    FILE* file = fopen(name, "rb+");

                    if (file == nullptr) continue;

                    ProcessTask(file);
                    }
                }                   
            }
        }


    void ProcessTask(FILE* file)
        {
        assert(file != nullptr);

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        BeXmlStatus status;
        WString     errorMsg;

        bvector<char> xmlFileContent(size);

        //BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, testPlanPath.c_str(), &errorMsg); 
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(status, &xmlFileContent[0], xmlFileContent.size(), &errorMsg);

        if (pXmlDom == 0)
            {
            //assert(false && "Invalid test plan filename");
            return false;
            }
        BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
        if (0 != BeStringUtilities::Stricmp(pRootNode->GetName(), "workerTask"))
            {
            //assert(false && "Invalid test plan format");
            return false;
            }

/*
        FILE*          pResultFile = 0;
        if (!OpenResultFile(pRootNode, pResultFile)) return false;
*/
        TestType t;
        if (!ParseTestType(pRootNode, t))
            {
            assert(false && "Invalid test plan type");
            fclose(pResultFile);
            return false;
            }
        fwprintf(pResultFile, GetHeaderForTestType(t).c_str());
        BeXmlNodeP pTestNode = pRootNode->GetFirstChild();

        while (0 != pTestNode)
            {
            switch (t)
                {
            case TEST_GENERATION:
                PerformGenerateTest(pTestNode, pResultFile);
                break;
            case TEST_PARTIAL_UPDATE:
                PerformUpdateTest(pTestNode, pResultFile);
                break;
            case TEST_QUALITY_MESH:
                PerformMeshQualityTest(pTestNode, pResultFile);
                break;
            case TEST_QUALITY_STITCH:
                Perform2DStitchQualityTest(pTestNode, pResultFile);
                break;
            case TEST_DRAPE_LINE:
                PerformDrapeLineTest(pTestNode, pResultFile);
                break;
            case TEST_VOLUME:
                PerformVolumeTest(pTestNode, pResultFile);
                break;
            case TEST_SELF_CONTAINED_IMPORTER:
                PerformSelfContainedImporterTest(pTestNode, pResultFile);
                break;
            case TEST_IMPORT_FEATURES_GIS:
                PerformGISFeaturesImporterTest(pTestNode, pResultFile);
                break;
            case TEST_GROUND_DETECTION_BASELINE:
                PerformClassificationCompareTest(pTestNode, pResultFile);
                break;
            case TEST_OPTIMIZE_GROUND_PARAMETERS:
                PerformGroundParametersTest(pTestNode, pResultFile);
                break;
            case TEST_ADD_NODES:
                PerformNodeCreationTest(pTestNode, pResultFile);
                break;
            case TEST_LOADING:
                PerformLoadingTest(pTestNode, pResultFile);
                break;
            case TEST_DRAPE_BASELINE:
                PerformDrapeBaselineTest(pTestNode, pResultFile, pRootNode);
                break;
            case TEST_CONSTRAINTS:
                PerformConstraintTest(pTestNode, pResultFile);
                break;
            case TEST_STREAMING:
                PerformStreaming(pTestNode, pResultFile);
                break;
            case TEST_SM_TO_CLOUD:
                PerformSMToCloud(pTestNode, pResultFile);
                break;
            case TEST_CLOUD:
                PerformCloudTests(pTestNode, pResultFile);
                break;
            case TEST_RANDOM_DRAPE:
                PerformTestDrapeRandomLines(pTestNode, pResultFile);
                break;
                //case EXPORT_LINE:
                //    ExportDrapeLine(pTestNode, pResultFile);
                //    break;
                //case EXPORT_VOLUME:
                //    ExportVolume(pTestNode, pResultFile);
                //    break;
                //case IMPORT_VOLUME:
                //    ImportVolume(pTestNode, pResultFile);
                //    break;
            case TEST_GROUP_NODE_HEADERS:
                PerformGroupNodeHeaders(pTestNode, pResultFile);
                break;
            case TEST_GROUND_EXTRACTION:
                PerformGroundExtractionTest(pTestNode, pResultFile);
                break;
            case ADD_TEXTURES_TO_MESH:
                AddTexturesToMesh(pTestNode, pResultFile);
                break;
            case EXPORT_TO_UNITY:
                PerformExportToUnityTest(pTestNode, pResultFile);
                break;
            case TEST_SQL_FILE_UPDATE:
                PerformSqlFileUpdateTest(pTestNode, pResultFile);
                break;
            case TEST_MAPBOX:
                PerformMapboxTest(pTestNode, pResultFile);
                break;
            case DRAPE_TEST_LNS_FILE_CREATION:
                PerformDrapeTestLnsFileCreation(pTestNode, pResultFile);
                break;
            case TEST_3MX_TO_3SM_CONVERSION:
                Perform3MxTo3SmTest(pTestNode, pResultFile);
                break;

            default: break;
                }
            pTestNode = pTestNode->GetNextSibling();
            }

        if (pResultFile != 0)
            {
            fclose(pResultFile);
            }
        return true;

        }
    
    }