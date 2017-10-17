/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/SelectionManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECPresentation/SelectionManager.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                 Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
namespace BackDoor
    {
    /*-----------------------------------------------------------------------------**//**
    * @bsinamespace                             Grigas.Petraitis                10/2017
    +-----------+---------------+---------------+---------------+---------------+------*/
    namespace SelectionManager
        {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                            Grigas.Petraitis                10/2017
        +-------+---------------+---------------+---------------+---------------+------*/
        void RemapNodeIds(ECPresentation::SelectionManager& manager, bmap<uint64_t, uint64_t> const& remapInfo)
            {
            manager.RemapNodeIds(remapInfo);
            }
        }
    }

END_ECPRESENTATIONTESTS_NAMESPACE