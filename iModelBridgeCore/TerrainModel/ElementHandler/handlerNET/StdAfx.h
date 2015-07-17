/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handlerNET/StdAfx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#pragma once
#pragma unmanaged
#include <Geom/GeomApi.h>
#pragma warning (disable:4393)

#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>
#define REFERENCE_FILE_CLASS
#include <DgnPlatform\DgnPlatformAPI.h>
#include <DgnView\IViewManager.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace std;

#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>
#include <DgnPlatform\TerrainModel\TMReferenceXAttributeHandler.h>
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>
#include <DgnPlatform\TerrainModel\TMSymbologyOverrideManager.h>
#include <TerrainModel\ElementHandler\TMElementDisplayHandler.h>
#include <TerrainModel\Core\bcDTMClass.h>

#pragma managed

#using <bentley.GeometryNET.Structs.dll>
#using <Bentley.Exceptions.dll>
#using <System.dll>
#using <Bentley.TerrainModelNET.dll>
#using <Bentley.DgnPlatformNET.dll>  as_friend

#include <msclr\gcroot.h>
#include <vcclr.h>

using namespace Bentley::Exceptions;
using namespace System::ComponentModel;
using namespace System::Reflection;
using namespace System::Collections;

namespace BGEO = Bentley::GeometryNET;
namespace DGNET = Bentley::DgnPlatformNET;

#define BEGIN_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace TerrainModelNET { namespace Element {
#define END_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE END_BENTLEY_NAMESPACE }}

USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

[assembly: System::Runtime::CompilerServices::InternalsVisibleTo ("Bentley.TerrainModel.ElementTemplate, PublicKey=00240000048000009400000006020000002400005253413100040000010001008b52bb3e32d2a2d9252516a4a9f6bd166f0e1f3edbe0ef8466b15dda2b3c5219539cafe036e6d07373dea23eab1e4479e1f0874894b963a3910fb847c9f7177ce439031100d699613c94e2b8f201c2f2e531211b18910158d50a7ba8353939dbe3ed7b483096b1bd6ed19b413eb2d2f5dd7bec25b810aaafe20d6dc6c0af579b")];