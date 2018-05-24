#pragma once

#include <list>
#include <iostream>
#include <windows.h> //used for pipes
#include <DgnPlatform/DgnPlatform.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ImagePP\h\ImageppAPI.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DgnPlatformLib.h>

#include <BeXml/BeXml.h>
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE    

struct TaskScheduler
    {

    private:
    
        BeFileName m_taskFolderName;

        void ProcessTask(FILE* file);
    
    public:
    
        TaskScheduler(BeFileName& taskFolderName);

        virtual ~TaskScheduler();

        void Start();

    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE