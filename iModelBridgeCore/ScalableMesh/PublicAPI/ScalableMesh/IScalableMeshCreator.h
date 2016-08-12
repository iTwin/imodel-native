/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshCreator.h $
|    $RCSfile: IScalableMeshCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2012/03/21 18:37:07 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#include <ImagePP\all\h\HIMMosaic.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshCreator;
typedef RefCountedPtr<IScalableMeshCreator>            IScalableMeshCreatorPtr;
 
/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
//typedef StatusInt (*ResolveMrtmFileNameFP)(BENTLEY_NAMESPACE_NAME::WString& fileName, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle);

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
        BENTLEY_SM_EXPORT virtual                 ~IScalableMeshCreator             ();





        //BENTLEY_SM_EXPORT bool                    AreAllSourcesReachable     () const;

        BENTLEY_SM_EXPORT StatusInt               Create                     (bool isSingleFile = true);    

        BENTLEY_SM_EXPORT StatusInt               SetTextureMosaic(MOSAIC_TYPE* mosaicP, Transform unitTransform = Transform::FromIdentity());


        // TDORAY: Rename in GetGCS once GetBaseGCS is used.
        BENTLEY_SM_EXPORT const GeoCoords::GCS&   GetAdvancedGCS             () const;

        BENTLEY_SM_EXPORT const GeoCoords::GCS&   GetGCS                     () const;

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& 
                                                  GetBaseGCS                 () const;

        BENTLEY_SM_EXPORT Time                    GetLastSyncTime            () const;

        BENTLEY_SM_EXPORT Time                    GetLastModified            () const;
        BENTLEY_SM_EXPORT Time                    GetLastModifiedCheckTime   () const;

        //BENTLEY_SM_EXPORT StatusInt               UpdateLastModified         ();
        // TDORAY: Consider adding a versions that takes as argument the minimum last time checked for
        // which it is not worth updating.

       // BENTLEY_SM_EXPORT void                    ResetLastModified          ();
        
        // TDORAY: Temporary way of solving our sources edition problem. This should not be required anymore once we
        // trap all source edit calls via the decorator pattern so that creator is notified of source edit operation. In
        // order to implement this, we'll need to stop dynamic_casting sources.
       /* BENTLEY_SM_EXPORT void                    SetSourcesDirty            ();
        BENTLEY_SM_EXPORT bool                    HasDirtySources            () const;*/

        BENTLEY_SM_EXPORT StatusInt               SaveToFile                 ();        

        BENTLEY_SM_EXPORT StatusInt               SetCompression             (ScalableMeshCompressionType        compressionType);


        BENTLEY_SM_EXPORT StatusInt               SetGCS                     (const GeoCoords::GCS&       gcs);

        BENTLEY_SM_EXPORT StatusInt               SetBaseGCS                 (const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& 
                                                                                                    gcsPtr);



        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath,
                                                                                     StatusInt&                status);

        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&     scmPtr,
                                                                                     StatusInt&                  status);

       /* BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath);

        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&   scmPtr);*/

       /* // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEY_SM_EXPORT const IDTMSourceCollection& GetSources                 () const;
        BENTLEY_SM_EXPORT IDTMSourceCollection&       EditSources                ();*/


#ifdef SCALABLE_MESH_ATP

        BENTLEY_SM_EXPORT static unsigned __int64 GetNbImportedPoints();    

        BENTLEY_SM_EXPORT static double GetImportPointsDuration();

        BENTLEY_SM_EXPORT static double GetLastBalancingDuration();

        BENTLEY_SM_EXPORT static double GetLastMeshingDuration();

        BENTLEY_SM_EXPORT static double GetLastFilteringDuration();

        BENTLEY_SM_EXPORT static double GetLastStitchingDuration();
#endif                           
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
