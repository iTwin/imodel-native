/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshNodeCreator.h $
|    $RCSfile: ScalableMeshNodeCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 22:03:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshNodeCreator.h>

#include "ScalableMeshCreator.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 * @description
 * @bsiclass                                                  Elenie.Godzaridis   07/2015
 +---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshNodeCreator::Impl : public IScalableMeshCreator::Impl
    {
    private:
        friend struct                       IScalableMeshNodeCreator;

    public:
        explicit                            Impl(const WChar*                          scmFileName);
        explicit                            Impl(const IScalableMeshPtr&                        iDTMFilePtr);

        ~Impl();
        IScalableMeshNodeEditPtr AddChildNode(const IScalableMeshNodePtr& parentNode,
                                          DRange3d& childExtent,
                                                       StatusInt&   status);

        IScalableMeshNodeEditPtr AddNode(StatusInt&   status);

        void NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                    StatusInt&                  status);

        virtual StatusInt                           CreateScalableMesh(bool isSingleFile = true) override;

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE