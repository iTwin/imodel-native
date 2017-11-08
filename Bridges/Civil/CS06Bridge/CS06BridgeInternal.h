/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/CS06BridgeInternal.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __CS06BRIDGEINTERNAL_H__
#define __CS06BRIDGEINTERNAL_H__

//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyB0200".
#define NO_USING_NAMESPACE_BENTLEY 1

#include <DgnDb06BimTeleporter/DgnDb06.h>
#include <CS06Bridge/CS06BridgeApi.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include <VersionedDgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06BimTeleporter/DgnDb06BimTeleporterApi.h>
#include "MarshalHelper.h"

namespace Dgn06 = BentleyG06::Dgn;
namespace BeSQLite06 = BentleyG06::BeSQLite;

namespace Teleporter = BENTLEY_NAMESPACE_NAME::DgnDb06BimTeleporter;

namespace AlignmentBim = BENTLEY_NAMESPACE_NAME::RoadRailAlignment;
namespace RoadRailBim = BENTLEY_NAMESPACE_NAME::RoadRailPhysical;

USING_NAMESPACE_BENTLEY_DGN

#endif
