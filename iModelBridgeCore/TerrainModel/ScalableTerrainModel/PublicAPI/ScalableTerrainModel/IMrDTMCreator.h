/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMCreator.h $
|    $RCSfile: IMrDTMCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2012/03/21 18:37:07 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <ScalableTerrainModel/IMrDTM.h>
#include <ScalableTerrainModel/MrDTMDefs.h>
#include <Bentley/RefCounted.h>
#include <ScalableTerrainModel/IMrDTMMoniker.h>
#include <ScalableTerrainModel/IMrDTMTime.h>
#include <ScalableTerrainModel/IMrDTMSources.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>


BEGIN_BENTLEY_MRDTM_NAMESPACE

struct IMrDTMCreator;
typedef RefCountedPtr<IMrDTMCreator>            IMrDTMCreatorPtr;



/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IMrDTMCreator : public RefCountedBase
                       
    {
private:
/*__PUBLISH_SECTION_END__*/
        struct                              Impl;
        std::auto_ptr<Impl>                 m_implP;               
                
        explicit                            IMrDTMCreator              (Impl*                       implP);

/*__PUBLISH_SECTION_START__*/
      
        // Disable copy
                                            IMrDTMCreator              (const IMrDTMCreator&);
        IMrDTMCreator&                      operator=                  (const IMrDTMCreator&);

public:
        BENTLEYSTM_EXPORT virtual                 ~IMrDTMCreator             ();

        BENTLEYSTM_EXPORT bool                    AreAllSourcesReachable     () const;

        BENTLEYSTM_EXPORT StatusInt               Create                     ();    


        // TDORAY: Rename in GetGCS once GetBaseGCS is used.
        BENTLEYSTM_EXPORT const GeoCoords::GCS&   GetAdvancedGCS             () const;

        BENTLEYSTM_EXPORT const GeoCoords::GCS&   GetGCS                     () const;

        BENTLEYSTM_EXPORT const Bentley::GeoCoordinates::BaseGCSPtr& 
                                            GetBaseGCS                 () const;

        BENTLEYSTM_EXPORT Time                    GetLastSyncTime            () const;

        BENTLEYSTM_EXPORT Time                    GetLastModified            () const;
        BENTLEYSTM_EXPORT Time                    GetLastModifiedCheckTime   () const;

        BENTLEYSTM_EXPORT StatusInt               UpdateLastModified         ();
        // TDORAY: Consider adding a versions that takes as argument the minimum last time checked for
        // which it is not worth updating.

        BENTLEYSTM_EXPORT void                    ResetLastModified          ();
        
        // TDORAY: Temporary way of solving our sources edition problem. This should not be required anymore once we
        // trap all source edit calls via the decorator pattern so that creator is notified of source edit operation. In
        // order to implement this, we'll need to stop dynamic_casting sources.
        BENTLEYSTM_EXPORT void                    SetSourcesDirty            ();
        BENTLEYSTM_EXPORT bool                    HasDirtySources            () const;

        BENTLEYSTM_EXPORT StatusInt               SaveToFile                 ();        

        BENTLEYSTM_EXPORT StatusInt               SetCompression             (MrDTMCompressionType        compressionType);




        BENTLEYSTM_EXPORT StatusInt               SetGCS                     (const GeoCoords::GCS&       gcs);

        BENTLEYSTM_EXPORT StatusInt               SetBaseGCS                 (const Bentley::GeoCoordinates::BaseGCSPtr& 
                                                                                                    gcsPtr);



        BENTLEYSTM_EXPORT static IMrDTMCreatorPtr GetFor                     (const WChar*              filePath,
                                                                              StatusInt&                  status);

        BENTLEYSTM_EXPORT static IMrDTMCreatorPtr GetFor                     (const IMrDTMPtr&            mrDTMPtr,
                                                                              StatusInt&                  status);

        BENTLEYSTM_EXPORT static IMrDTMCreatorPtr GetFor                     (const WChar*              filePath);

        BENTLEYSTM_EXPORT static IMrDTMCreatorPtr GetFor                     (const IMrDTMPtr&            mrDTMPtr);

        // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEYSTM_EXPORT const IDTMSourceCollection&
                                            GetSources                 () const;
        BENTLEYSTM_EXPORT IDTMSourceCollection&   EditSources                ();
    };

END_BENTLEY_MRDTM_NAMESPACE