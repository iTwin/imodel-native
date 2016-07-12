/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceImporter.h $
|    $RCSfile: ScalableMeshCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|    ScalableMeshNewFileCreator.h            (C) Copyright 2001.        |
|                                                BCIVIL Corporation.    |
|                                                All Rights Reserved.   |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

// NTERAY: A whole lot of dependencies for a single header... This is bad. Reduce these to strict minimum.

#include <ctime> //benchmarking
#include <windows.h> //for showing info.


#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshSourceImporter.h>

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

#include "ScalableMeshCoreDefs.h"
#include "ScalableMeshCoreFns.h"

#include <ScalableMesh/GeoCoords/Reprojection.h>
//#include <ImagePP/all/h/ISMStore.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>

#include "ScalableMeshEditListener.h"
#include "ScalableMeshStorage.h"

//#include <HGF3DExtent.h>

/*__PUBLISH_SECTION_END__*/

// NTERAY: Using namespace directive should never happen to be in headers (but inside other namespaces in some cases).
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_START__*/
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Sink;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourcesImporter;
class ClipShapeStorage;

typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;
typedef RefCountedPtr<IStorage>             IStoragePtr;
typedef RefCountedPtr<ClipShapeStorage>     ClipShapeStoragePtr;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceImporter::Impl : public EditListener
    {    
    private :
        
        friend struct                       IScalableMeshSourceImporter;

        IDTMSourceCollection                m_sources;
        GeoCoords::GCS                      m_gcs;
        ScalableMeshCompressionType                m_compressionType;                        
        bool                                m_sourcesDirty;
        bool                                m_gcsDirty;        
        const size_t                        m_workingLayer;
        WritePointsCallbackFP               m_writePointsCallbackFP;
        WriteFeatureCallbackFP              m_writeFeatureCallbackFP;


        virtual void                        _NotifyOfPublicEdit            () override;
        virtual void                        _NotifyOfLastEditUpdate        (Time                                    updatedLastEditTime) override;
        
        int                                 ImportClipMaskSource           (const IDTMSource&                   dataSource,
                                                                            const ClipShapeStoragePtr&              clipShapeStoragePtr) const;

        int                                 TraverseSource                 (SourcesImporter&                        importer,
                                                                            IDTMSource&                             dataSource,
                                                                            const HFCPtr<HVEClipShape>&             clipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS,
                                                                            const Import::ScalableMeshData&         targetScalableMeshData) const;
        
        int                                 TraverseSourceCollection       (SourcesImporter&                        importer,
                                                                            IDTMSourceCollection&                   sources,                                              
                                                                            const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS,
                                                                            const Import::ScalableMeshData&         targetScalableMeshData);        


        StatusInt                           SyncWithSources                     ();

        StatusInt                           ImportSourcesTo                (Import::Sink*                           sinkP);

        /*
        template <typename PointIndex>
        StatusInt                           RemoveSourcesFrom(PointIndex& pointIndex, list<ISMStore::Extent3d64f> listRemoveExtent) const;
        */
/*        
        template <typename PointType, typename PointIndex, typename LinearIndex>
        static StatusInt                    AddPointOverviewOfLinears      (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);
                                                                            */
        template <typename PointIndex, typename LinearIndex>
        static StatusInt                    Filter                         (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex, 
                                                                            int                                     levelToFilter = -1);        
        /*        
        StatusInt                           SaveSources                    ();  
        StatusInt                           SaveSources                    (ISMStore::File&                         file);
        */

        StatusInt                           SaveSources                    (IScalableMeshSourceImporterStoragePtr&                sourceImporterStoragePtr);

    public :  
        explicit                            Impl                           ();
        
                                            ~Impl                          ();

        const GeoCoords::GCS&               GetGCS                         () const;
        StatusInt                           SaveGCS                        (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);
                             
        StatusInt                           UpdateLastModified             ();
        void                                ResetLastModified              ();
        

        StatusInt                           LoadGCS                        (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);        
        StatusInt                           LoadSources                    (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);        

        StatusInt                           LoadFromStorage                (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);        
        StatusInt                           Store                          (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);  
        

        //StatusInt                           CreateScalableMesh                    ();  
        StatusInt                           Import                         ();  
        StatusInt                           Filter                         ();

        StatusInt                           SetFeatureCallback             (WriteFeatureCallbackFP writeFeatureCallbackFP);
        StatusInt                           SetPointsCallback              (WritePointsCallbackFP  writePointsCallbackFP);

        // For debugging / benchmarking
        static void ShowMessageBoxWithTimes(double meshingDuration, double filteringDuration, double stitchingDuration);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
