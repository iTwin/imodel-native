#pragma once
#include "ATPUtils.h"

void PerformGenerateTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformUpdateTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformMeshQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void Perform2DStitchQualityTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformDrapeLineTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformVolumeTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformSelfContainedImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile);
void PerformGISFeaturesImporterTest(BeXmlNodeP pTestNode, FILE* pResultFile);