#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <Bentley/Bentley.h>
#include <Bentley\BeStringUtilities.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <BeXml/BeXml.h>
#include <Geom/GeomApi.h>
#include <Mtg/MtgStructs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH

void ParseDTMFeatureType(WString& name, DTMFeatureType& type);

bool ParseExportToUnityOptions(WString& outputDir, int& maxLevel, bool& exportTexture, BeXmlNodeP pTestNode);

bool AddOptionToSource(IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode);

void GetSourceDataType(DTMSourceDataType& dataType, BeXmlNodeP pSourceNode);

bool ParseGenerationOptions(ScalableMeshMesherType* mesherType, ScalableMeshFilterType* filterType, int* trimmingMethod, ScalableMeshSaveType* saveType, BeXmlNodeP pTestNode);

bool ParseSourceSubNodes(IDTMSourceCollection& sourceCollection, BeXmlNodeP pTestNode);

bool ParseBaselineSubNodes(WString& baselinePointFileName, WString& baselineFeatureFileName, BeXmlNodeP pTestNode);

WString GetMesherTypeName(ScalableMeshMesherType mesherType);

WString GetFilterTypeName(ScalableMeshFilterType filterType);

WString GetTrimmingTypeName(int trimmingType);

WString GetBlossomMatchingValue(int blossomMatch);

WString GetIndexMethodValue(int indexMethod);

WString GetTrimmingMethodValue(int trimmingMethod);

BENTLEY_NAMESPACE_NAME::WString UpdateTest_GetStmFileNameWithSuffix(BENTLEY_NAMESPACE_NAME::WString stmFileName, BENTLEY_NAMESPACE_NAME::WString suffix);

