#include "ATPUtils.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <wtypes.h>

#include "ATPDefinitions.h"
#include "ATPGeneration.h"

WString GetHeaderForTestType(TestType t)
    {
    switch (t)
        {
        case TEST_GENERATION:            
            return  L"File Name,Mesher,Filter,Trimming,Nb Input Points,Nb Output Points,Point Kept (%%),File Size (Mb),Accelerator Used,GroundDetection: Time for seeds(s),GroundDetection: Time for Params Estimation (s), GroundDetection: Time for TIN growing (s),GroundDetection (s),GroundDetection(%%), Import Points (%%),Point Balancing (%%),Meshing (%%),Clipping (%%),Mesh Balancing (%%),Filtering (%%),Stitching (%%),Storing (%%), Duration (minutes),Duration (hours), GroundDetection(minutes), Import Points (minutes),Point Balancing (minutes),Meshing (minutes),Clipping (minutes),Mesh Balancing (minutes),Filtering (minutes),Stitching (minutes),Storing (minutes),Status\n";
            break;
        case TEST_PARTIAL_UPDATE:
            return L"";
            break;
        case TEST_QUALITY_MESH:
            return L"Test Case, Result, NOfPoints, Mesher, Filter, Use Blossom?, Index Method, Trimming Method, Total Triangles Meshed, Total Wrong Triangles, Wrong Triangles(%%), Total Triangles Missing,Missing Triangles (%%), Quality(higher is better)\n";
            break;
        case TEST_QUALITY_STITCH:
            return L"Test Case, Result, NOfPoints, Filter, Total Tiles Stitched, Total Wrong Tiles, Total Triangles Meshed, Total Wrong Triangles, Wrong Triangles(%%)\n";
            break;
        case TEST_DRAPE_LINE:
            return L"Test Case, Result, NOfPoints, NOfLines, NOfLinesSucceeded,NOfFullLines,Rate of partial drapes(%%), Length of partial drapes (%% of full length), Graph Store Misses (%%), NOfFailedDrapes, Error Rate (%%),Total time (s), Time per drape (s), Number of output points in lines, Time per output point (ms)\n";
            break;
        case TEST_VOLUME:
            return L"Test Case, Nb Design Meshes, Result, Cut, Fill, Expected Cut, Expected Fill, Cut Error (%%), Fill Error (%%), Total time (s)\n";
            break;
        case TEST_SELF_CONTAINED_IMPORTER:
            return  L"Nb Imported Points,Duration (minutes)\n";
            break;
        case TEST_IMPORT_FEATURES_GIS:
            return  L"Test Case,Result,NOfBreaklines In Base STM, NOfBreaklines In Created STM, NOfBreaklines Different In New STM, Duration (minutes)\n";
            break;
        case TEST_GROUND_DETECTION_BASELINE:
            return L"Test Case, Baseline, Total points, Ground points (true), Object points (true), Type I error [N of ground points classified as objects], Type II error [N of object points classified as ground], Type I error %%, Type II error %%, Total error %%, Sensitivity %%, Specificity %%\n";
            break;
        case TEST_OPTIMIZE_GROUND_PARAMETERS:
            return L"Avg Type I error %%, Avg Type II error %%, Avg Total error %%, Avg Sensitivity %%, Avg Specificity %%, Min Type I error %%, Min Type II error %%, Min Total error %%, Min Sensitivity %%, Min Specificity %%, Max Type I error %%, Max Type II error %%, Max Total error %%, Max Sensitivity %%, Max Specificity %%, Starred, Parameters\n";
            break;
        case TEST_ADD_NODES:
            return L"";
            break;
        case TEST_LOADING:
            return L"File Name, Time To Load (s), Max depth To Load (if 0 all nodes are loaded), Nb of loaded nodes\n";
            break;
        case TEST_GROUND_EXTRACTION:                       
            return L"InputFileName, Nb Ground Regions, Nb Successful Ground Regions, Result, Duration (minutes)\n";
            break;            
        case TEST_DRAPE_BASELINE:
            return L"Test Case,  Pass/Fail, Baseline, Nb of Lines, Nb of Lines Draped (baseline), Nb of Lines Draped(test), Nb Of Different Lines, %% unmatched points, Time to drape (1st load) (baseline), Time to drape (1st load) (test), Time taken (1st load) variation, Time to drape (cached) (baseline), Time to drape (cached) (test), Time taken (cached) variation\n";
            break;
        case TEST_CONSTRAINTS:
            return L"Test Case,  Pass/Fail, Nb of constraints, Nb of constraint points, Nb of valid constraints, Constraint error rate(%%), Nb of triangles violating constraints\n";
            break;
        case TEST_SDK_MESH:
            return L"";
            break;
        case TEST_STREAMING:
            return L"File Name Original, File Name Streaming, AllTestPass, Point Count Pass, Node Count Pass, time load all node headers, time streaming load all node headers, points Node Pass\n";
            break;
        case TEST_SM_SAVEAS:
            return L"File Name Original, Directory Name Output, Triangle Count Input, Triangle Count Output, AllTestPass, time triangle count, time save as\n";
            break;
        case TEST_SM_TO_CLOUD:
            return L"File Name Original, Directory Name Cloud, AllTestPass, Point Count Pass, Node Count Pass, time load all node headers, time load all node headers, points Node Pass\n";
            break;
        case TEST_CLOUD:
            return L"Container Name Cloud, Directory Name Cloud, AllTestPass, time process cloud\n";
            break;
        case TEST_RANDOM_DRAPE:
            return L"Test Case, Line Number, N Of Points Draped (SM), N Of Points Draped (Civil), Length (SM), Length (Civil), N Of Points Difference (%%), Length Difference (%%), NDifferentLines Total, Time total(SM) (s), Time total(Civil) (s)\n";
            break;
        //case EXPORT_LINE:
        //    return L"\n";
        //    break;
        //case EXPORT_VOLUME:
        //    return L"\n";
        //    break;
        //case IMPORT_VOLUME:
        //    return L"\n";
        //    break;
        case EXPORT_TO_UNITY:
            return L"File name, Nb points, Nb params, Level, Duration (seconds)\n";
            break;
        case TEST_SQL_FILE_UPDATE:
            return L"File name, Duration (seconds)\n";
            break;
        case TEST_3MX_TO_3SM_CONVERSION:
            return L"InputFileName, OutputFileName, Result, Duration (minutes)\n";
            break;
        case TEST_EDIT_MESH:
            return  L"";
            break;
            
        default: break;
        }
    return L"";
    }

bool OpenResultFile(BeXmlNodeP pRootNode, FILE*& pResultFile)
    {
    WString        resultFileName;
    StatusInt status = pRootNode->GetAttributeStringValue(resultFileName, "resultFileName");
    
    if (status == BEXML_Success)
        {        
        WString timestampVal;
        status = pRootNode->GetAttributeStringValue(timestampVal, "timestampResultFile");

        if (status == BEXML_Success && timestampVal.CompareTo(L"yes") == 0)
            {            
            WString dev;
            WString dir; 
            WString name; 
            WString ext;
            BeFileName::ParseName(&dev, &dir, &name, &ext, resultFileName.c_str());

            std::time_t t = std::time(nullptr);
            wchar_t timeStamp[1000];
            std::wcsftime(timeStamp, sizeof(timeStamp), L"_%Hh%Mm%Ss__%dd%mm%yy.", std::localtime(&t));
            
            if (dev.length() != 0)
                resultFileName = dev + WString(L":");
            else
                resultFileName = WString(L"");
                                
            resultFileName += dir + name + WString(timeStamp) + ext;
            }

        pResultFile = _wfopen(resultFileName.c_str(), L"w+");
        }

    if (pResultFile == 0)
        {
        assert(false && "Invalid result filename");
        return false;
        }
    return true;
    }

bool ParseTestType(BeXmlNodeP pRootNode, TestType& t)
    {
    WString testType;
    StatusInt status = pRootNode->GetAttributeStringValue(testType, "type");

    if (status == BEXML_Success)
        {
        
        if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"generation"))
            t = TEST_GENERATION;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"partialUpdate"))
            t = TEST_PARTIAL_UPDATE;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"meshQuality"))
            t = TEST_QUALITY_MESH;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"stitchQuality"))
            t = TEST_QUALITY_STITCH;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"drapeLine"))
            t = TEST_DRAPE_LINE;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"volume"))
            t = TEST_VOLUME;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"selfContainedImporter"))
            t = TEST_SELF_CONTAINED_IMPORTER;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"gisFeatures"))
            t = TEST_IMPORT_FEATURES_GIS;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"compareClassif"))
            t = TEST_GROUND_DETECTION_BASELINE;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"groundDetectionParameters"))
            t = TEST_OPTIMIZE_GROUND_PARAMETERS;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"nodeCreator"))
            t = TEST_ADD_NODES;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"drapeBaseline"))
            t = TEST_DRAPE_BASELINE;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"constraints"))
            t = TEST_CONSTRAINTS;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"meshSDK"))
            t = TEST_SDK_MESH;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"streaming"))
            t = TEST_STREAMING;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"smSaveAs"))
            t = TEST_SM_SAVEAS;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"smToCloud"))
            t = TEST_SM_TO_CLOUD;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"cloud"))
            t = TEST_CLOUD;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"drapeRandom"))
            t = TEST_RANDOM_DRAPE;
        //else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"exportLine"))
        //    t = EXPORT_LINE;
        //else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"exportVolume"))
        //    t = EXPORT_VOLUME;
        //else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"importVolume"))
        //    t = IMPORT_VOLUME;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"loadNodes"))
            t = TEST_LOADING;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"groupNodeHeaders"))
            t = TEST_GROUP_NODE_HEADERS;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"groundExtraction"))
            t = TEST_GROUND_EXTRACTION;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"addTextures"))
            t = ADD_TEXTURES_TO_MESH;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"exportToUnity"))
            t = EXPORT_TO_UNITY;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"sqlFileUpdate"))
            t = TEST_SQL_FILE_UPDATE;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"textureStreaming"))
            t = TEST_MAPBOX;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"drapeTestlnsFileCreation"))
            t = DRAPE_TEST_LNS_FILE_CREATION;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"3mxTo3smConversion"))
            t = TEST_3MX_TO_3SM_CONVERSION;
        else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"smoothingEdit"))
            t = TEST_EDIT_MESH; 
        else return false;
        }
    else return false;
    return true;
    }

bool RemoveStmFiles(BeFileName& inputFileName)
    {
    BeXmlStatus status;
    WString errorMsg;

    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, inputFileName.c_str(), &errorMsg);
    if (pXmlDom == 0)
        {
        assert(false && "Invalid test plan filename");
        return false;
        }
    BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
    if (0 != BeStringUtilities::Stricmp(pRootNode->GetName(), "testplan"))
        {
        assert(false && "Invalid test plan format");
        return false;
        }

    TestType t;
    if (!ParseTestType(pRootNode, t))
        {
        assert(false && "Invalid test plan type");
        return false;
        }

    WString stmFileName;
    BeXmlNodeP pTestNode = pRootNode->GetFirstChild();
    while (0 != pTestNode)
        {
        status = pTestNode->GetAttributeStringValue(stmFileName, "stmFileName");
    
        if (status != BEXML_Success)
            status = pTestNode->GetAttributeStringValue(stmFileName, "smFileName");        

        if (t == TEST_PARTIAL_UPDATE)
            {
            WString suffixPartialUpdate(L"_partialUpdate");
            WString stmFileName_PartialUpdateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixPartialUpdate));
            WString suffixGenerate(L"_generate");
            WString stmFileName_GenerateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixGenerate));
            char* pathPartialUpdate = new char[255];
            wcstombs(pathPartialUpdate, stmFileName_PartialUpdateTest.c_str(), 255);
            if (std::remove(pathPartialUpdate) != 0)
                fwprintf(stderr, L"can't delete file : %ls\n", stmFileName_PartialUpdateTest.c_str());
            char* pathGenerate = new char[255];
            wcstombs(pathGenerate, stmFileName_GenerateTest.c_str(), 255);
            if (std::remove(pathGenerate) != 0)
                fwprintf(stderr, L"can't delete file : %ls\n", stmFileName_GenerateTest.c_str());
            }
        else
            {
            char* path = new char[255];
            wcstombs(path, stmFileName.c_str(), 255);
            if (std::remove(path) != 0)
                fwprintf(stderr, L"can't delete file : %ls\n", stmFileName.c_str());
            }
        pTestNode = pTestNode->GetNextSibling();
        }
    return true;
    }

bool RunTestPlan(BeFileName& testPlanPath)
    {
    BeXmlStatus status;
    WString     errorMsg;

    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, testPlanPath.c_str(), &errorMsg);
    if (pXmlDom == 0)
        {
        assert(false && "Invalid test plan filename");
        return false;
        }
    BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
    if (0 != BeStringUtilities::Stricmp(pRootNode->GetName(), "testplan"))
        {
        assert(false && "Invalid test plan format");
        return false;
        }
    FILE*          pResultFile = 0;
    if (!OpenResultFile(pRootNode, pResultFile)) return false;
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
            case TEST_SM_SAVEAS:
                PerformSMSaveAs(pTestNode, pResultFile);
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
            case TEST_EDIT_MESH:
                PerformEditTest(pTestNode, pResultFile);
            
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

void ReadLinesFile(std::ifstream& file, bvector<bvector<DPoint3d>>& lineDefs)
    {
    std::string line;
    while (std::getline(file, line))
        {
        bvector<DPoint3d> linePts;
        std::string token, ptDef;
        std::istringstream str(line);
        while (std::getline(str, token, ';'))
            {
            size_t n = 0;
            DPoint3d pt;
            std::istringstream ptStr(token);
            while (std::getline(ptStr, ptDef, ' ') && n < 3)
                {
                switch (n)
                    {
                    case 0:
                        pt.x = std::atof(ptDef.c_str());
                        break;
                    case 1:
                        pt.y = std::atof(ptDef.c_str());
                        break;
                    case 2:
                        pt.z = std::atof(ptDef.c_str());
                        break;
                    default:
                        break;
                    }
                n++;
                }
            linePts.push_back(pt);
            }
        if (linePts.size() > 0)lineDefs.push_back(linePts);
        }
    }


bool DPoint3dEqualityTest(const DPoint3d& point1, const DPoint3d& point2)
    {
    return point1.x == point2.x && point1.y == point2.y;
    }

DTMFeatureType ParseFeatureType(std::string& line)
    {
    std::istringstream str(line);
    std::string word;
    std::vector<std::string> tokens;
    while (str >> word) tokens.push_back(word);
    std::string featureText = tokens.back();
    if (stricmp(featureText.c_str(), "Breakline") == 0)
        {
        return DTMFeatureType::Breakline;
        }
    else if (stricmp(featureText.c_str(), "SoftBreakline") == 0)
        {
        return DTMFeatureType::SoftBreakline;
        }
    else if (stricmp(featureText.c_str(), "Contour") == 0)
        {
        return DTMFeatureType::ContourLine;
        }
    else if (stricmp(featureText.c_str(), "Void") == 0)
        {
        return DTMFeatureType::Void;
        }
    else if (stricmp(featureText.c_str(), "BreakVoid") == 0)
        {
        return DTMFeatureType::BreakVoid;
        }
    else if (stricmp(featureText.c_str(), "DrapeVoid") == 0)
        {
        return DTMFeatureType::DrapeVoid;
        }
    else if (stricmp(featureText.c_str(), "Island") == 0)
        {
        return DTMFeatureType::Island;
        }
    else if (stricmp(featureText.c_str(), "Hole") == 0)
        {
        return DTMFeatureType::Hole;
        }
    else if (stricmp(featureText.c_str(), "Hull") == 0)
        {
        return DTMFeatureType::Hull;
        }
    else if (stricmp(featureText.c_str(), "DrapeHull") == 0)
        {
        return DTMFeatureType::DrapeHull;
        }
    else if (stricmp(featureText.c_str(), "HullLine") == 0)
        {
        return DTMFeatureType::HullLine;
        }
    else if (stricmp(featureText.c_str(), "VoidLine") == 0)
        {
        return DTMFeatureType::VoidLine;
        }
    else if (stricmp(featureText.c_str(), "HoleLine") == 0)
        {
        return DTMFeatureType::HoleLine;
        }
    else if (stricmp(featureText.c_str(), "SlopeToe") == 0)
        {
        return DTMFeatureType::SlopeToe;
        }
    else if (stricmp(featureText.c_str(), "GraphicBreak") == 0)
        {
        return DTMFeatureType::GraphicBreak;
        }
    else if (stricmp(featureText.c_str(), "Region") == 0)
        {
        return DTMFeatureType::Region;
        }
    else
        {
        return DTMFeatureType::None;
        }
    }

DPoint3d ParsePoint(std::string& line)
    {
    std::string token;
    size_t n = 0;
    DPoint3d pt;
    pt.Zero();
    std::istringstream str(line);
    while (std::getline(str, token, ','))
        {
        switch (n)
            {
            case 0:
                pt.x = std::atof(token.c_str());
                break;
            case 1:
                pt.y = std::atof(token.c_str());
                break;
            case 2:
                pt.z = std::atof(token.c_str());
                break;
            default:
                return pt;
            }
        n++;
        }
    return pt;
    }

void ReadFeatureFile(std::ifstream& file, std::vector<std::pair<std::vector<DPoint3d>, DTMFeatureType>>& features)
    {
    std::string line;
    std::vector<DPoint3d> currentFeaturePoints;
    DTMFeatureType currentFeatureType;
    while (std::getline(file, line))
        {
        if (line[0] == 'F')
            {
            if (currentFeaturePoints.size() > 0) features.push_back(std::make_pair(currentFeaturePoints, currentFeatureType));
            currentFeaturePoints.clear();
            currentFeatureType = ParseFeatureType(line);
            }
        else
            {
            DPoint3d pt = ParsePoint(line);
            currentFeaturePoints.push_back(pt);
            }
        }
    if (currentFeaturePoints.size() > 0) features.push_back(std::make_pair(currentFeaturePoints, currentFeatureType));
    }

void SetReprojectionMatrixForMeterData(IScalableMeshPtr& stmFile)
    {
    double ratioToMeter = stmFile->GetGCS().GetHorizontalUnit().GetRatioToBase();

    if (ratioToMeter != 1.0)
        {
        GeoCoordinates::BaseGCSPtr targetGcs(GeoCoordinates::BaseGCS::CreateGCS());
        Transform approximateTransform(Transform::FromIdentity());
        approximateTransform.form3d[0][0] = approximateTransform.form3d[1][1] = approximateTransform.form3d[2][2] = ratioToMeter;
        BentleyStatus status = stmFile->SetReprojection(*targetGcs, approximateTransform);
        assert(status == SUCCESS);
        }
    }

bool GetLinePointsFromLnsFile(bvector<bvector<DPoint3d>>& lines, WString& lnsFileName)
    {
    std::wstring line;
    std::wifstream infile(lnsFileName.c_str());
        
    while (std::getline(infile, line))
        {
        bvector<DPoint3d> linePts;
        std::wistringstream iss(line);
        wchar_t semiColon;

        while (true)
            {
            DPoint3d pts;

            if (!(iss >> pts.x >> pts.y >> pts.z >> semiColon)) { break; } // error    
            linePts.push_back(pts);
            }
    
        lines.push_back(linePts);
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertUorPointsToDestUnit(bvector<DPoint3d>& regionPointsMeter, const bvector<DPoint3d>& regionPoints, Transform trans)
    {
    for (int i = 0; i< regionPoints.size(); i++)
        {
        DPoint3d temp;
        trans.Multiply(temp, regionPoints[i]);
        regionPointsMeter.push_back(temp);
        }
    }


void CreateBreaklines(BeFileNameCR extraLinearFeatureAbsFileName, bvector<DPoint3d> const& closedPolygonPoints, ScalableMesh::IScalableMeshPtr& scalableMeshModel, Transform& uorToDestUnits)
    {
    TerrainModel::DTMPtr dtm(scalableMeshModel->GetDTMInterface(DTMAnalysisType::RawDataOnly));

    TerrainModel::BcDTMPtr bcDtmPtr(TerrainModel::BcDTM::Create());
    TerrainModel::DTMDrapedLinePtr drapedLine;
    TerrainModel::IDTMDraping* draping = dtm->GetDTMDraping();
    bool hasAddedBreaklines = false;

    for (size_t segmentInd = 0; segmentInd < closedPolygonPoints.size() - 1; segmentInd++)
        {
        DTMStatusInt status = draping->DrapeLinear(drapedLine, &closedPolygonPoints[segmentInd], 2);
        assert(status == DTMStatusInt::DTM_SUCCESS);

        bvector<DPoint3d> breaklinePts;

        for (size_t ptInd = 0; ptInd < drapedLine->GetPointCount(); ptInd++)
            {
            DPoint3d pt;
            double distance;
            DTMDrapedLineCode code;

            status = drapedLine->GetPointByIndex(pt, &distance, &code, (int)ptInd);
            assert(status == SUCCESS);
            breaklinePts.push_back(pt);
            }

        if (breaklinePts.empty())
            continue;

        DTMFeatureId featureId;

        bvector<DPoint3d> breaklinePtsMeter;
        ConvertUorPointsToDestUnit(breaklinePtsMeter, breaklinePts, uorToDestUnits);

        status = bcDtmPtr->AddLinearFeature(DTMFeatureType::Breakline, &breaklinePtsMeter[0], (int)breaklinePtsMeter.size(), &featureId);
        assert(status == DTMStatusInt::DTM_SUCCESS);
        hasAddedBreaklines = true;
        }

    if (hasAddedBreaklines)
        {
        DTMStatusInt status = bcDtmPtr->SaveAsGeopakDat(extraLinearFeatureAbsFileName.c_str());
        assert(status == DTMStatusInt::DTM_SUCCESS);
        }
    }