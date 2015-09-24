/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Bentley.TerrainModelNET.Formats.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#pragma unmanaged

#include <TerrainModel\TerrainModel.h>
#include <TerrainModel\Formats\Inroads.h>
#include <TerrainModel\Formats\InroadsImporter.h>
#include <TerrainModel\Formats\LandXMLImporter.h>
#include <TerrainModel\Formats\LandXMLExporter.h>
#include <TerrainModel\Formats\LidarImporter.h>
#include <TerrainModel\Formats\MX.h>
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\dtm2dfns.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

#pragma managed
// Using Statements for Civil assemblies are now in the mke file.
// Add an entry to the DependsOnCivilAssemblies bmake property/macro for any
// Civil assembly that this assembly requires to build

#using <bentley.GeometryNET.Structs.dll>
#using <Bentley.GeoCoord.dll>
#using <bentley.TerrainModelNET.dll>
#using <Bentley.Exceptions.dll>
#using <System.dll>

#include<vcclr.h>

using namespace Bentley::Exceptions;
using namespace System::ComponentModel;
using namespace System::Reflection;
using namespace System::Collections;

namespace BGEO = Bentley::GeometryNET;


#if !defined (BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE)
#define BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE namespace Bentley { namespace TerrainModelNET { namespace Formats { 
#define END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE }}}
#endif


// ToDo Look at

#define bcMem_malloc malloc
#define bcMem_free free
#define bcMem_realloc realloc
#define bcMem_calloc calloc
template<class c> void bcMem_freeAndClear (c **a) { if (a != 0 && *a != 0) { free (*a); *a = 0; } }

#pragma make_public(::BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporter)
#pragma make_public(::BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainExporter)

#include "TerrainImporter.h"
#include "TerrainExporter.h"
