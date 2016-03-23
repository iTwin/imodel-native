#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

namespace ScalableMeshATPexe
{
void InitializeATP(DgnPlatformLib::Host& host);
void CloseATP();
};