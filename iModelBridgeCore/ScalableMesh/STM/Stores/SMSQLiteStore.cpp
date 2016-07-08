#pragma once

#include <ScalableMeshPCH.h>

#include "SMSQLiteStore.h"
#include "SMSQLiteStore.hpp"

template class SMIndexMasterHeader<DRange3d>;

template class SMIndexNodeHeaderBase<DRange3d>;

template class SMIndexNodeHeader<DRange3d>;

template class SMSQLiteStore<DRange3d>;

template class SMSQLiteNodePointStore<DPoint3d, DRange3d>;