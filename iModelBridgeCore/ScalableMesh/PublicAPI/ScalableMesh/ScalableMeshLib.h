/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshLib.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshAdmin.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*=================================================================================**//**
ScalableMeshLib defines interfaces that a "host" application must implement to enable use of ScalableTerrainModel library.
The host must initialize the ScalableMeshLib library by calling ScalableMeshLib::Initialize for each host on a separate thread.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ScalableMeshLib
{
public:

    struct Host : public DgnHost
        {
        protected:

            ScalableMeshAdmin*            m_scalableTerrainModelAdmin;
          
            //! Supply the ScalableTerrainModelAdmin for this session. This method is guaranteed to be called once from ScalableTerrainModelAdmin::Host::Initialize and never again.
            BENTLEYSTM_EXPORT virtual ScalableMeshAdmin& _SupplyScalableMeshAdmin();            

        public:
            Host()
                {
                m_scalableTerrainModelAdmin = 0;                              
                }

            ScalableMeshAdmin&  GetScalableMeshAdmin()             { return *m_scalableTerrainModelAdmin; }
         
        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized () {return 0 != m_scalableTerrainModelAdmin;}

        void Initialize ();        

        //! Terminate this Host. After this method is called, no further ScalableTerrainModel methods can ever be called on this thread again (including Initialize).
        //! This method should be called on thread termination.
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        BENTLEYSTM_EXPORT void Terminate(bool onProgramExit);
        };

    //! Must be called once per Host before calling any method in ScalableTerrainModel. Applications can have more than one Host. 
    BENTLEYSTM_EXPORT static void Initialize (Host& host);

    //! Query if the ScalableTerrainModel library has been initialized on this thread.
    BENTLEYSTM_EXPORT static bool IsInitialized ();

    //! Get the ScalableTerrainModel library host for this thread. 
    BENTLEYSTM_EXPORT static Host& GetHost(); 
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

