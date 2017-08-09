/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshLib.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshAdmin.h"
#include <ScalableMesh/IScalableMesh.h>

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
            WsgTokenAdmin*                m_wsgTokenAdmin;
            SASTokenAdmin*                m_sasTokenAdmin;
            SSLCertificateAdmin*          m_sslCertificateAdmin;

#ifdef VANCOUVER_API
			STMAdmin*    m_stmAdmin; 
#endif

            bmap<WString, IScalableMesh*>* m_smPaths;
          
            //! Supply the ScalableTerrainModelAdmin for this session. This method is guaranteed to be called once from ScalableTerrainModelAdmin::Host::Initialize and never again.
            BENTLEY_SM_EXPORT virtual ScalableMeshAdmin& _SupplyScalableMeshAdmin();            

            BENTLEY_SM_EXPORT virtual WsgTokenAdmin& _SupplyWsgTokenAdmin();

            BENTLEY_SM_EXPORT virtual SASTokenAdmin& _SupplySASTokenAdmin();

            BENTLEY_SM_EXPORT virtual SSLCertificateAdmin& _SupplySSLCertificateAdmin();

#ifdef VANCOUVER_API
			BENTLEY_SM_EXPORT virtual STMAdmin& _SupplySTMAdmin();
#endif
        public:
            Host()
                {
                m_scalableTerrainModelAdmin = 0;                              
                }

            ScalableMeshAdmin&      GetScalableMeshAdmin()              { return *m_scalableTerrainModelAdmin; }
            WsgTokenAdmin&          GetWsgTokenAdmin()                  { return *m_wsgTokenAdmin; }
            SASTokenAdmin&          GetSASTokenAdmin()                  { return *m_sasTokenAdmin; }
            SSLCertificateAdmin&    GetSSLCertificateAdmin()            { return *m_sslCertificateAdmin; }
#ifdef VANCOUVER_API
			STMAdmin&    GetSTMAdmin() { return *m_stmAdmin; }
#endif           
        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized () {return 0 != m_scalableTerrainModelAdmin;}

        void Initialize ();
        
        //! Terminate this Host. After this method is called, no further ScalableTerrainModel methods can ever be called on this thread again (including Initialize).
        //! This method should be called on thread termination.
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        BENTLEY_SM_EXPORT void Terminate(bool onProgramExit);

        BENTLEY_SM_EXPORT IScalableMeshPtr GetRegisteredScalableMesh(const WString& path);
        BENTLEY_SM_EXPORT void             RemoveRegisteredScalableMesh(const WString& path);
        void RegisterScalableMesh(const WString& path, IScalableMeshPtr& ref);
        };

    //! Must be called once per Host before calling any method in ScalableMesh. Applications can have more than one Host. 
    BENTLEY_SM_EXPORT static void Initialize (Host& host);

    BENTLEY_SM_EXPORT static void Terminate (Host& host);

    //! Query if the ScalableMesh library has been initialized on this thread.
    BENTLEY_SM_EXPORT static bool IsInitialized ();

    //! Get the ScalableMesh library host for this thread. 
    BENTLEY_SM_EXPORT static Host& GetHost(); 
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

