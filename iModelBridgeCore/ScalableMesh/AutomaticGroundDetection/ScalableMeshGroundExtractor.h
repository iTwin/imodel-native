/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/ScalableMeshGroundExtractor.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshGroundExtractor.h>
#include <ScalableMesh\IScalableMesh.h>

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshGroundExtractor;

typedef RefCountedPtr<ScalableMeshGroundExtractor> ScalableMeshGroundExtractorPtr;

/*----------------------------------------------------------------------------+
|Class ScalableMeshBase
+----------------------------------------------------------------------------*/
struct ScalableMeshGroundExtractor : public RefCounted<IScalableMeshGroundExtractor>
    {
    private: 

        IScalableMeshPtr m_scalableMesh;

    protected:                   

        virtual StatusInt                    _ExtractAndEmbed() override;     

        explicit                            ScalableMeshGroundExtractor(IScalableMeshPtr& scalableMesh);

        virtual                             ~ScalableMeshGroundExtractor();
    
public:

    static ScalableMeshGroundExtractorPtr Create(IScalableMeshPtr& scalableMesh);        
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
