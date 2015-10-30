/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlDeleteStatementCrudTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestFrameworkHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                           Krischan.Eberle            09/13
//=======================================================================================    
struct ECSqlDeleteTestDataset
{
private:
    ECSqlDeleteTestDataset ();
    ~ECSqlDeleteTestDataset ();
public:
    static ECSqlTestDataset FromTests (int rowCountPerClass);
    static ECSqlTestDataset MiscTests (int rowCountPerClass);
    static ECSqlTestDataset PolymorphicTests (int rowCountPerClass);
    };


END_ECDBUNITTESTS_NAMESPACE