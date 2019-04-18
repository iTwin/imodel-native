/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//@bsistruct                                      Algirdas.Mikoliunas          03/2018
//=======================================================================================
struct PendingChangeSetStorage
{
public:
    static void AddItem(Dgn::DgnDbR db, Utf8String item);
    static void RemoveItem(Dgn::DgnDbR db, Utf8String item);
    static void GetItems(Dgn::DgnDbR db, bvector<Utf8String>& items);
};

END_BENTLEY_IMODELHUB_NAMESPACE

