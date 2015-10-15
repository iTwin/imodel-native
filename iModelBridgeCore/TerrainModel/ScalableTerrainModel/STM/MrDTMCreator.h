/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMCreator.h $
|    $RCSfile: MrDTMCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMNewFileCreator.h    		  	    		(C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

// NTERAY: A whole lot of dependencies for a single header... This is bad. Reduce these to strict minimum.




#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableTerrainModel/IMrDTMCreator.h>

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/
#include <ImagePP/all/h/HVEClipShape.h>

/*----------------------------------------------------------------------+
| Core DTM Methods                                                      |
+----------------------------------------------------------------------*/

#include "MrDTMCoreDefs.h"
#include "MrDTMCoreFns.h"

#include <ScalableTerrainModel/GeoCoords/Reprojection.h>
#include <STMInternal/Storage/IDTMFile.h>
#include <ScalableTerrainModel/IMrDTMDocumentEnv.h>

#include "MrDTMEditListener.h"
#include "MrDTMStorage.h"

//#include <HGF3DExtent.h>

/*__PUBLISH_SECTION_END__*/

// NTERAY: Using namespace directive should never happen to be in headers (but inside other namespaces in some cases).
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_START__*/
using namespace Bentley::GeoCoordinates;


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct Sink;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct SourcesImporter;
class ClipShapeStorage;

typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;
typedef RefCountedPtr<IStorage>             IStoragePtr;
typedef RefCountedPtr<ClipShapeStorage>     ClipShapeStoragePtr;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMrDTMCreator::Impl : public EditListener
    {    
    private :
        
        friend struct                       IMrDTMCreator;

        IDTMSourceCollection                m_sources;
        GeoCoords::GCS                      m_gcs;
        MrDTMCompressionType                m_compressionType;
        WString                             m_mrDtmFileName;    
        IMrDTMPtr                           m_mrDTMPtr;    
        Time                                m_lastSyncTime;
        Time                                m_lastSourcesModificationTime;
        Time                                m_lastSourcesModificationCheckTime;
        bool                                m_sourcesDirty;
        bool                                m_gcsDirty;
        DocumentEnv                         m_sourceEnv;
        const size_t                        m_workingLayer;


        virtual void                        _NotifyOfPublicEdit            () override;
        virtual void                        _NotifyOfLastEditUpdate        (Time                                    updatedLastEditTime) override;

        static DocumentEnv                  CreateSourceEnvFrom            (const WChar*                          filePath);

        int                                 ImportClipMaskSource           (const IDTMSource&                   dataSource,
                                                                            const ClipShapeStoragePtr&              clipShapeStoragePtr) const;

        int                                 TraverseSource                 (SourcesImporter&                        importer,
                                                                            const IDTMSource&                   dataSource,
                                                                            const HFCPtr<HVEClipShape>&             clipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS) const;
        
        int                                 TraverseSourceCollection       (SourcesImporter&                        importer,
                                                                            const IDTMSourceCollection&             sources,                                              
                                                                            const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS) const;        


        StatusInt                           SyncWithSources                     (const IDTMFile::File::Ptr&              filePtr) const;

        StatusInt                           ImportSourcesTo                (Import::Sink*                           sinkP) const;
        
        template <typename PointType, typename PointIndex, typename LinearIndex>
        static StatusInt                    AddPointOverviewOfLinears      (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);
        template <typename PointIndex, typename LinearIndex>
        static StatusInt                    Filter                         (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);

        bool                                FileExist                      () const;

        IDTMFile::File::Ptr                 GetFile                        (const IDTMFile::AccessMode&             accessMode);

        IDTMFile::File::Ptr                 SetupFileForCreation           ();

        StatusInt                           SaveSources                    ();  
        StatusInt                           SaveSources                    (IDTMFile::File&                         file);

    public :  
        explicit                            Impl                           (const WChar*                          mrDtmFileName);
        explicit                            Impl                           (const IMrDTMPtr&                        iDTMFilePtr);

                                            ~Impl                          ();

        const GeoCoords::GCS&               GetGCS                         () const;
        StatusInt                           SaveGCS                        (IDTMFile::File&                         file);


                     
        StatusInt                           UpdateLastModified             ();
        void                                ResetLastModified              ();

        StatusInt                           LoadGCS                        (const IDTMFile::File&                   file);
        StatusInt                           LoadSources                    (const IDTMFile::File&                   file);

        StatusInt                           LoadFromFile                   ();
        StatusInt                           SaveToFile                     ();  

        StatusInt                           CreateMrDTM                    ();  
        StatusInt                           Filter                         ();

  


    };

END_BENTLEY_MRDTM_NAMESPACE