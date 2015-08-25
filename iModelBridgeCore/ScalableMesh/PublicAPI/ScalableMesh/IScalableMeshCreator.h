/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshCreator.h $
|    $RCSfile: IScalableMeshCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2012/03/21 18:37:07 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/RefCounted.h>
#include <ScalableMesh/IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshTime.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/GeoCoords/GCS.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshCreator;
typedef RefCountedPtr<IScalableMeshCreator>            IScalableMeshCreatorPtr;
 
/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
typedef StatusInt (*ResolveMrtmFileNameFP)(Bentley::WString& fileName, const Bentley::DgnPlatform::EditElementHandle& elHandle);

struct IScalableMeshCreator : public RefCountedBase                       
    {
private:
/*__PUBLISH_SECTION_END__*/
    friend struct                       IScalableMeshSourceCreator;
    friend struct                       IScalableMeshNodeCreator;
        struct                              Impl;
        std::auto_ptr<Impl>                 m_implP;               
                
        explicit                            IScalableMeshCreator              (Impl*                       implP);

/*__PUBLISH_SECTION_START__*/
      
        // Disable copy
                                            IScalableMeshCreator              (const IScalableMeshCreator&);
        IScalableMeshCreator&                      operator=                  (const IScalableMeshCreator&);

public:
        BENTLEYSTM_EXPORT virtual                 ~IScalableMeshCreator             ();





        //BENTLEYSTM_EXPORT bool                    AreAllSourcesReachable     () const;

        BENTLEYSTM_EXPORT StatusInt               Create                     ();    


        // TDORAY: Rename in GetGCS once GetBaseGCS is used.
        BENTLEYSTM_EXPORT const GeoCoords::GCS&   GetAdvancedGCS             () const;

        BENTLEYSTM_EXPORT const GeoCoords::GCS&   GetGCS                     () const;

        BENTLEYSTM_EXPORT const Bentley::GeoCoordinates::BaseGCSPtr& 
                                                  GetBaseGCS                 () const;

        BENTLEYSTM_EXPORT Time                    GetLastSyncTime            () const;

        BENTLEYSTM_EXPORT Time                    GetLastModified            () const;
        BENTLEYSTM_EXPORT Time                    GetLastModifiedCheckTime   () const;

        //BENTLEYSTM_EXPORT StatusInt               UpdateLastModified         ();
        // TDORAY: Consider adding a versions that takes as argument the minimum last time checked for
        // which it is not worth updating.

       // BENTLEYSTM_EXPORT void                    ResetLastModified          ();
        
        // TDORAY: Temporary way of solving our sources edition problem. This should not be required anymore once we
        // trap all source edit calls via the decorator pattern so that creator is notified of source edit operation. In
        // order to implement this, we'll need to stop dynamic_casting sources.
       /* BENTLEYSTM_EXPORT void                    SetSourcesDirty            ();
        BENTLEYSTM_EXPORT bool                    HasDirtySources            () const;*/

        BENTLEYSTM_EXPORT StatusInt               SaveToFile                 ();        

        BENTLEYSTM_EXPORT StatusInt               SetCompression             (ScalableMeshCompressionType        compressionType);


        BENTLEYSTM_EXPORT StatusInt               SetGCS                     (const GeoCoords::GCS&       gcs);

        BENTLEYSTM_EXPORT StatusInt               SetBaseGCS                 (const Bentley::GeoCoordinates::BaseGCSPtr& 
                                                                                                    gcsPtr);



        BENTLEYSTM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath,
                                                                                     StatusInt&                status);

        BENTLEYSTM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&     scmPtr,
                                                                                     StatusInt&                  status);

       /* BENTLEYSTM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath);

        BENTLEYSTM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&   scmPtr);*/

       /* // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEYSTM_EXPORT const IDTMSourceCollection& GetSources                 () const;
        BENTLEYSTM_EXPORT IDTMSourceCollection&       EditSources                ();*/


#ifdef SCALABLE_MESH_ATP

        BENTLEYSTM_EXPORT static unsigned __int64 GetNbImportedPoints();    

        BENTLEYSTM_EXPORT static double GetImportPointsDuration();

        BENTLEYSTM_EXPORT static double GetLastBalancingDuration();

        BENTLEYSTM_EXPORT static double GetLastMeshingDuration();

        BENTLEYSTM_EXPORT static double GetLastFilteringDuration();

        BENTLEYSTM_EXPORT static double GetLastStitchingDuration();
#endif                           
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
