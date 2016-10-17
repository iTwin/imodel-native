/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMesh.h>
#include <Bentley\Bentley.h>
#include <Bentley/RefCounted.h>

#undef static_assert

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshGroundExtractor;

typedef RefCountedPtr<IScalableMeshGroundExtractor> IScalableMeshGroundExtractorPtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshGroundExtractor abstract: virtual public RefCountedBase
    {
    private:        

    protected:                         
             
        //Synchonization with data sources functions
        virtual StatusInt _ExtractAndEmbed() = 0;                

          
    public:
      
        BENTLEY_SM_EXPORT StatusInt ExtractAndEmbed();                
        
        BENTLEY_SM_EXPORT static IScalableMeshGroundExtractorPtr Create(IScalableMeshPtr& scalableMesh);        

    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
