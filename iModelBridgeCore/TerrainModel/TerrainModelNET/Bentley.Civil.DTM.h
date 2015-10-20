/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/Bentley.Civil.DTM.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// It is important that this is the first file included

// Using Statements for Civil assemblies are now in the mke file.
// Add an entry to the DependsOnCivilAssemblies bmake property/macro for any
// Civil assembly that this assembly requires to build

#using <bentley.GeometryNET.Structs.dll>
#using <bentley.GeometryNET.dll>
#using <Bentley.Exceptions.dll>
#using <System.dll>

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/dtm2dfns.h>
#include <TerrainModel/Core/DTMIterators.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

#pragma make_public(BcDTM)
#pragma make_public(BcDTMDrapedLine)
#pragma make_public(BcDTMFeature)

#if !defined (BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE)
#define BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE namespace Bentley { namespace TerrainModelNET { 
#define END_BENTLEY_TERRAINMODELNET_NAMESPACE }}
#endif


// ToDo Look at

#define bcMem_malloc malloc
#define bcMem_free free
#define bcMem_realloc realloc
#define bcMem_calloc calloc
template<class c> void bcMem_freeAndClear (c **a) { if (a != 0 && *a != 0) { free (*a); *a = 0; } }

using namespace Bentley::Exceptions;
using namespace System::ComponentModel;
using namespace System::Reflection;
using namespace System::Collections;
namespace BGEO = Bentley::GeometryNET;
