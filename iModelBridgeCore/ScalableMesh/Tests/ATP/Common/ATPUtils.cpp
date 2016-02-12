#include "ATPUtils.h"
#include "ATPDefinitions.h"
#include "ATPGeneration.h"


WString GetHeaderForTestType(TestType t)
    {
    switch (t)
        {
        case TEST_GENERATION:
            return  L"File Name,Mesher,Filter,Trimming,Nb Input Points,Nb Output Points,Point Kept (%%), GroundDetection (%%), Import Points (%%),Balancing (%%),Meshing (%%),Filtering (%%),Stitching (%%),Duration (minutes),Duration (hours), Ground Duration (minutes), Import Points (minutes),Balancing (minutes),Meshing (minutes),Filtering (minutes),Stitching (minutes),Status\n";
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
            return L"Test Case, Result, NOfPoints, NOfTriangleInCorridor, Cut, Fill, Volume, NOfTilesUsed, Cut (all tiles), Fill(all tiles), Volume (all tiles), Cut Error (%%), Fill Error (%%), Volume Error (%%), Total time (s), Avg Error (%%), Time for cut+fill (all tiles)(s),Cut (fully connected), Fill(fully connected), Volume (fully connected), Cut [Stitching] Error (%%), Fill [Stitching] Error (%%), Volume [Stitching] Error (%%) \n";
            break;
        case TEST_SELF_CONTAINED_IMPORTER:
            return  L"Nb Imported Points,Duration (minutes)\n";
            break;
        case TEST_IMPORT_FEATURES_GIS:
            return  L"Test Case,Result,NOfBreaklines In Base STM, NOfBreaklines In Created STM, NOfBreaklines Different In New STM, Duration (minutes)\n";
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
        if (t == TEST_PARTIAL_UPDATE)
            {
            WString suffixPartialUpdate("_partialUpdate");
            WString stmFileName_PartialUpdateTest(UpdateTest_GetStmFileNameWithSuffix(stmFileName, suffixPartialUpdate));
            WString suffixGenerate("_generate");
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
        if (linePts.size()>0)lineDefs.push_back(linePts);
        }
    }


bool DPoint3dEqualityTest(const DPoint3d& point1, const DPoint3d& point2)
    {
    return point1.x== point2.x && point1.y == point2.y;
    }