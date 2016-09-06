/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceImporter.h $
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
//#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/RefCounted.h>
//#include <ScalableMesh/IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshTime.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh\IScalableMeshSourceImporter.h>
#include <ScalableMesh/GeoCoords/GCS.h>

#ifndef VANCOUVER_API
#include "HPUPacket.h"
#else
#include <ImagePP/all/h/HPUPacket.h>
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshSourceImporter;
struct IScalableMeshSourceImporterStorage;
typedef RefCountedPtr<IScalableMeshSourceImporter>         IScalableMeshSourceImporterPtr;
typedef RefCountedPtr<IScalableMeshSourceImporterStorage>  IScalableMeshSourceImporterStoragePtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
typedef bool (*WriteFeatureCallbackFP)(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d);
typedef bool (*WritePointsCallbackFP)(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d);

#define NO_GROUP_ID UINT_MAX



struct IScalableMeshSourceImporterStorage : public RefCountedBase
    {
    public : 

        typedef unsigned int GroupId;
               
    private : 

            virtual StatusInt _AddSource(Time::TimeType     sourceLastModifiedTime, 
                                         const HPU::Packet& serializedSourcePacket, 
                                         const HPU::Packet& serializedContentConfigPacket, 
                                         const HPU::Packet& serializedImportSequencePacket, 
                                         GroupId            groupId) = 0;

            virtual StatusInt _StoreGcs(const WString& wkt) = 0;


            virtual bool _ReadFirstSource(StatusInt* status) = 0;

            virtual StatusInt _GetSourceInfo(Time::TimeType& sourceLastModifiedTime, 
                                             HPU::Packet&    serializedSourcePacket, 
                                             HPU::Packet&    serializedContentConfigPacket, 
                                             HPU::Packet&    serializedImportSequencePacket, 
                                             GroupId&        groupId) const = 0;

            virtual bool _ReadNextSource(StatusInt* status) = 0;

            virtual StatusInt _ReadGcs(WString& wkt) = 0;

                            
    public :                         

            StatusInt AddSource(Time::TimeType     sourceLastModifiedTime, 
                                const HPU::Packet& serializedSourcePacket, 
                                const HPU::Packet& serializedContentConfigPacket, 
                                const HPU::Packet& serializedImportSequencePacket, 
                                GroupId            groupId);    

            StatusInt StoreGcs(const WString& wkt);
            
            bool ReadFirstSource(StatusInt* status = 0);

            StatusInt GetSourceInfo(Time::TimeType& sourceLastModifiedTime, 
                                    HPU::Packet&    serializedSourcePacket, 
                                    HPU::Packet&    serializedContentConfigPacket, 
                                    HPU::Packet&    serializedImportSequencePacket, 
                                    GroupId&        groupId);

            bool ReadNextSource(StatusInt* status = 0);

            StatusInt ReadGcs(WString& wkt);
    };

struct IScalableMeshSourceImporter : public RefCountedBase
    {

private:
/*__PUBLISH_SECTION_END__*/
        struct                              Impl;
        std::auto_ptr<Impl>                 m_implP;               
                
        explicit                            IScalableMeshSourceImporter              (Impl*                       implP);

/*__PUBLISH_SECTION_START__*/
      
        // Disable copy
                                            IScalableMeshSourceImporter              (const IScalableMeshSourceImporter&);
        IScalableMeshSourceImporter&                      operator=                  (const IScalableMeshSourceImporter&);

public:
        BENTLEY_SM_EXPORT virtual                 ~IScalableMeshSourceImporter             ();

        BENTLEY_SM_EXPORT bool                    AreAllSourcesReachable     () const;

        BENTLEY_SM_EXPORT StatusInt               Import                     ();    


        // TDORAY: Rename in GetGCS once GetBaseGCS is used.
        BENTLEY_SM_EXPORT const GeoCoords::GCS&   GetAdvancedGCS             () const;

        BENTLEY_SM_EXPORT const GeoCoords::GCS&   GetGCS                     () const;

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& 
                                            GetBaseGCS                 () const;
                                        
        
        // TDORAY: Temporary way of solving our sources edition problem. This should not be required anymore once we
        // trap all source edit calls via the decorator pattern so that creator is notified of source edit operation. In
        // order to implement this, we'll need to stop dynamic_casting sources.
        BENTLEY_SM_EXPORT void                    SetSourcesDirty            ();
        BENTLEY_SM_EXPORT bool                    HasDirtySources            () const;

        BENTLEY_SM_EXPORT StatusInt               Store                     (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);        

        BENTLEY_SM_EXPORT StatusInt               SetCompression             (ScalableMeshCompressionType        compressionType);

        BENTLEY_SM_EXPORT StatusInt               SetGCS                     (const GeoCoords::GCS&       gcs);

        BENTLEY_SM_EXPORT StatusInt               SetBaseGCS                 (const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& gcsPtr);
        
        BENTLEY_SM_EXPORT StatusInt               SetFeatureCallback         (WriteFeatureCallbackFP writeFeatureCallbackFP);

        BENTLEY_SM_EXPORT StatusInt               SetPointsCallback          (WritePointsCallbackFP  writePointsCallbackFP);

        BENTLEY_SM_EXPORT static IScalableMeshSourceImporterPtr Create ();        

        BENTLEY_SM_EXPORT static IScalableMeshSourceImporterPtr Create (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr, 
                                                                 StatusInt&                      status);    

        
        // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEY_SM_EXPORT const IDTMSourceCollection&
                                            GetSources                 () const;
        BENTLEY_SM_EXPORT IDTMSourceCollection&   EditSources                ();
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
