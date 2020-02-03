/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/


#pragma once
#pragma unmanaged
#include <Geom/GeomApi.h>
#pragma warning (disable:4393)

#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>
#define REFERENCE_FILE_CLASS
#include <DgnPlatform/DgnPlatformAPI.h>
#include <DgnView/IViewManager.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace std;

#include <DgnPlatform/TerrainModel/TMElementHandler.h>
#include <DgnPlatform/TerrainModel/TMElementSubHandler.h>
#include <DgnPlatform/TerrainModel/TMPersistentAppIDs.h>
#include <DgnPlatform/TerrainModel/TMReferenceXAttributeHandler.h>
#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include <DgnPlatform/TerrainModel/TMSymbologyOverrideManager.h>
#include <TerrainModel/ElementHandler/TMElementDisplayHandler.h>
#include <TerrainModel/Core/bcDTMClass.h>

#pragma managed

#using <bentley.GeometryNET.Structs.dll>
#using <Bentley.Exceptions.dll>
#using <System.dll>
#using <Bentley.TerrainModelNET.dll>
#using <Bentley.DgnPlatformNET.dll>  as_friend

#include <msclr/gcroot.h>
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

[assembly: System::Runtime::CompilerServices::InternalsVisibleTo ("Bentley.TerrainModel.ElementTemplate, PublicKey=002400000480000094000000060200000024000052534131000400000100010047f1bc496d5be7f2b61bf2abb530ef55c6a6ff377677780ff34785c98aea2f309cd4556a5ded8ea70af8f623b3f3b6663c547a6a1c68a6146eb871375e3f48b44d02e5ad5c7db943a185fd20a661844a874046e49d2d58a4571b3ddf4606c094a2008aa4be9a8596a42b6ec0f54993f633b71b709d88d6c229499c61729fddba")];
