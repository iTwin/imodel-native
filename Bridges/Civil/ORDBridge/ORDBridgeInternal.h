/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridgeInternal.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __ORDBRIDGEINTERNAL_H__
#define __ORDBRIDGEINTERNAL_H__

//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyB0200".
#define NO_USING_NAMESPACE_BENTLEY 1

#include <DgnDbSync/DgnV8/DgnV8.h>
#include <CifApi/Cif/SDK/Bentley.Cif.SDK.h>
#include <CifApi/Cif/SDK/ConsensusConnection.h>
#include <CifApi/Cif/SDK/CIFGeometryModelSDK.h>
#include <CifApi/Cif/SDK/GeometryModelDgnECDataBinder.h>
#include <CifApi/Cif/SDK/ConsensusDgnECProvider.h>
#include <CifApi/Cif/SDK/CIFGeometryModelECSchema.h> 
#include <ORDBridge/ORDBridgeApi.h>
#include <DgnDbSync/DgnDbSync.h>
#include <DgnDbSync/DgnV8/Converter.h>

#include "ORDConverter.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
DGNV8_USING_NAMESPACE_BENTLEY_CIF_SDK
DGNV8_USING_NAMESPACE_BENTLEY_CIF_GEOMETRYMODEL_SDK

namespace AlignmentBim = BENTLEY_NAMESPACE_NAME::RoadRailAlignment;
namespace RoadRailBim = BENTLEY_NAMESPACE_NAME::RoadRailPhysical;

#endif
