/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshNodeCreator.h $
|    $RCSfile: IScalableMeshNodeCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2015/07/15 10:30:02 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshCreator.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshNodeCreator;
typedef RefCountedPtr<IScalableMeshNodeCreator>            IScalableMeshNodeCreatorPtr;

//This is the creator interface to use when providing a pre-created spatial index to use in the Scalable Mesh. 
//At the moment, it is not possible to import data from source files and also manually create nodes in the index.
struct IScalableMeshNodeCreator : public IScalableMeshCreator
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        friend struct IScalableMeshCreator;
        struct                              Impl;
        //std::auto_ptr<Impl>                 m_implP;

        explicit                            IScalableMeshNodeCreator(Impl*                       implP);

        /*__PUBLISH_SECTION_START__*/

    public:
        BENTLEY_SM_IMPORT_EXPORT virtual                 ~IScalableMeshNodeCreator();

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshNodeCreatorPtr GetFor(const WChar*              filePath,
                                                                StatusInt&                status);

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshNodeCreatorPtr GetFor(const IScalableMeshPtr&     scmPtr,
                                                                StatusInt&                  status);

        BENTLEY_SM_IMPORT_EXPORT IScalableMeshNodeEditPtr AddNode(const IScalableMeshNodePtr& parentNode,
                                                       DRange3d& extent,
                                                            SMStatus&                  status,
                                                            bool computeNodeID = true,
                                                            uint64_t nodeId = 0);

        BENTLEY_SM_IMPORT_EXPORT IScalableMeshNodeEditPtr AddNode(StatusInt&                  status,
                                                           bool computeNodeID = true,
                                                           uint64_t nodeId = 0);

        BENTLEY_SM_IMPORT_EXPORT int64_t AddTexture(int width, int height, int nOfChannels, const byte* texData);
        
        BENTLEY_SM_IMPORT_EXPORT void AddTexture(int width, int height, int nOfChannels, const byte* texData, int64_t texID);

        BENTLEY_SM_IMPORT_EXPORT void SetTextured(SMTextureType textured);

        BENTLEY_SM_IMPORT_EXPORT void SetIsSingleFile(bool isSingleFile);

        BENTLEY_SM_IMPORT_EXPORT void SetIsTerrain(bool isTerrain);

        BENTLEY_SM_IMPORT_EXPORT void SetDataResolution(float resolution);

        BENTLEY_SM_IMPORT_EXPORT void NotifyAllChildrenAdded(const IScalableMeshNodePtr& parentNode,
                                                                          StatusInt&                  status,
                                                                          bool computeNeighbors = true);


    };

END_BENTLEY_SCALABLEMESH_NAMESPACE