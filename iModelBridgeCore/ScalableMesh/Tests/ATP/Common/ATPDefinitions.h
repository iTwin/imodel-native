#pragma once
#include "ATPUtils.h"

void PerformExportToUnityTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGenerateTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformUpdateTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformMeshQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void Perform2DStitchQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformDrapeLineTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformVolumeTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformSelfContainedImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGISFeaturesImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformClassificationCompareTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGroundParametersTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformNodeCreationTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformLoadingTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformDrapeBaselineTest(BeXmlNodeP pTestNode, FILE* pResultFile, BeXmlNodeP pRootNode);
void PerformConstraintTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformStreaming(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformSMSaveAs(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformSMToCloud(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformCloudTests(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformTestDrapeRandomLines(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformSqlFileUpdateTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformMapboxTest(BeXmlNodeP pTestNode, FILE* pResultFile);
//void ExportDrapeLine(BeXmlNodeP pTestNode, FILE* pResultFile);
//void ExportVolume(BeXmlNodeP pTestNode, FILE* pResultFile);
//void ImportVolume(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGroupNodeHeaders(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformDcGroundDetectionTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void AddTexturesToMesh(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformDrapeTestLnsFileCreation(BeXmlNodeP pTestNode, FILE* pResultFile);
void Perform3MxTo3SmTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGroundExtractionTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformEditTest(BeXmlNodeP pTestNode, FILE* pResultFile);




