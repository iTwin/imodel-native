//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteStore.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

#include "SMSQLiteStore.h"
#include "SMSQLiteStore.hpp"

template class SMSQLiteStore<DRange3d>;

template class SMSQLiteNodeDataStore<DPoint3d, DRange3d>;