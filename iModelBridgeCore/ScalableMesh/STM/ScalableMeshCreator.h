/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.h $
|    $RCSfile: ScalableMeshCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

// NTERAY: A whole lot of dependencies for a single header... This is bad. Reduce these to strict minimum.

#include <ctime> //benchmarking
#include <windows.h> //for showing info.


#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>

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
#include "Stores\SMSQLiteStore.h"

//#include <HGF3DExtent.h>

/*__PUBLISH_SECTION_END__*/

// NTERAY: Using namespace directive should never happen to be in headers (but inside other namespaces in some cases).
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_START__*/
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

extern ISMPointIndexFilter<DPoint3d, Extent3dType>* s_filter;

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Sink;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourcesImporter;
class ClipShapeStorage;

typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;
typedef RefCountedPtr<IStorage>             IStoragePtr;
typedef RefCountedPtr<ClipShapeStorage>     ClipShapeStoragePtr;

typedef DPoint3d                          PointType;
//typedef ISMStore::Extent3d64f                         PointIndexExtentType;
typedef DRange3d PointIndexExtentType;
typedef SMMeshIndex <PointType, PointIndexExtentType> MeshIndexType;

void RegisterDelayedImporters();
inline bool fileExist(const WChar* fileName)
    {
    ifstream file(fileName, ios::in | ios::binary);
    return file.good() && (char_traits<char>::eof() != file.peek());
    }



bool DgnDbFilename(BENTLEY_NAMESPACE_NAME::WString& stmFilename);


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                               Elenie.Godzaridis   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScalableMeshProgress : public IScalableMeshProgress
    {

    private:
        std::atomic<bool> m_canceled;
        std::atomic<ScalableMeshStep> m_currentStep;

        std::atomic<float> m_progressInStep;
        std::atomic<int> m_progressStepIndex;
        std::atomic<ScalableMeshStepProcess> m_progressStepProcess;
        int m_totalNSteps;

    protected:
    virtual bool _IsCanceled() const override;
    virtual void _Cancel() override;

    virtual std::atomic<ScalableMeshStep> const& _GetProgressStep() const override;
    virtual int _GetTotalNumberOfSteps() const override { return m_totalNSteps; }

    virtual std::atomic<ScalableMeshStepProcess> const& _GetProgressStepProcess() const override;

    virtual std::atomic<float> const& _GetProgress() const override; //Progress of current step ([0..1])

    virtual std::atomic<float>& _Progress() override;
    virtual std::atomic<ScalableMeshStep>& _ProgressStep() override;


    virtual std::atomic<int> const& _GetProgressStepIndex() const override;

    virtual void _SetTotalNumberOfSteps(int step) override;

    virtual std::atomic<ScalableMeshStepProcess>& _ProgressStepProcess() override;
    virtual std::atomic<int>& _ProgressStepIndex() override;

    public:
        ScalableMeshProgress()
            {
            m_canceled = false;
            m_progressStepIndex = 0;
            m_currentStep = ScalableMeshStep::STEP_NOT_STARTED;
            m_progressInStep = 0;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshCreator::Impl 
    {    
    private :
        
        friend struct                       IScalableMeshCreator;
        friend struct                       IScalableMeshSourceCreator::Impl;
        friend struct                       IScalableMeshNodeCreator::Impl;

        GeoCoords::GCS                      m_gcs;
        ScalableMeshCompressionType         m_compressionType;
        WString                             m_scmFileName;    
        IScalableMeshPtr                    m_scmPtr;  


        SMSQLiteFilePtr                     m_smSQLitePtr;
        bool                                m_isDgnDb;

        Time                                m_lastSyncTime;

        bool                                m_gcsDirty;

        const size_t                        m_workingLayer;


        //CREATOR2

        static DataSourceManager            s_dataSourceManager;


        StatusInt                           CreateDataIndex(HFCPtr<MeshIndexType>&                                    pDataIndex,
                                                            bool needBalancing = false);



        
        template <typename PointType, typename PointIndex, typename LinearIndex>
        static StatusInt                    AddPointOverviewOfLinears      (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);

        bool                                FileExist                      () const;


        //ISMStore::File::Ptr                 GetFile(const ISMStore::AccessMode&             accessMode);
        virtual StatusInt                           Save();
        virtual StatusInt                           Load();
        void                                        SetupFileForCreation();
        SMSQLiteFilePtr                             GetFile(bool fileExists);


        virtual bool                                IsFileDirty();

        ScalableMeshProgress m_progress;

    protected:
        
        HFCPtr<MeshIndexType>               m_dataIndex;

        std::atomic<bool>        m_isCanceled;

        template <typename PointIndex>
          StatusInt                    Filter(PointIndex&                             pointIndex,
                                            int                                     levelToFilter = -1);

        template <typename PointIndex>
         StatusInt                    BalanceDown(PointIndex& pointIndex,
                                                 size_t      depthBeforePartialUpdate);

        template <typename PointIndex>
          StatusInt                    Mesh(PointIndex&                             pointIndex);

        template <typename PointIndex>
          StatusInt                    Stitch(PointIndex&                             pointIndex,
                                                                   int                                     levelToStitch = -1,
                                                                   bool                                    do2_5dStitchFirst = false);


          HFCPtr<MeshIndexType>                   m_pDataIndex;
          WString                             m_baseExtraFilesPath;
    public :  
        explicit                            Impl                           (const WChar*                          scmFileName);
        explicit                            Impl                           (const IScalableMeshPtr&                        iDTMFilePtr);

                                            ~Impl                          ();

        const GeoCoords::GCS&               GetGCS                         () const;


        StatusInt                           LoadGCS();
        StatusInt                           SaveGCS();

        StatusInt                           LoadFromFile                   ();
        StatusInt                           SaveToFile                     ();  

        virtual StatusInt                           CreateScalableMesh                    (bool isSingleFile, bool restrictLevelForPropagation);  

        StatusInt  SetTextureMosaic(HIMMosaic* mosaicP);
        StatusInt  SetTextureProvider(ITextureProviderPtr provider);
        StatusInt  SetTextureStreamFromUrl(WString url);

        StatusInt                           Filter                         ();

        void                               SetBaseExtraFilesPath(const WString& path);

        ScalableMeshProgress*            GetProgress();

        bool                               IsCanceled();

        void                               Cancel();

      //  IScalableMeshNodePtr                AddChildNode (const IScalableMeshNodePtr& parentNode, 
       //                                                   StatusInt&                  status);

        // For debugging / benchmarking
        static void ShowMessageBoxWithTimes(double meshingDuration, double filteringDuration, double stitchingDuration);
    };


    /*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Filter  (PointIndex&     pointIndex,
                                        int             levelToFilter)
    {    
    pointIndex.Filter(levelToFilter);   
    return BSISUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::BalanceDown  (PointIndex& pointIndex, size_t depthBeforePartialUpdate)
    {
    pointIndex.BalanceDown(depthBeforePartialUpdate);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Mesh  (PointIndex& pointIndex)
    {
    pointIndex.Mesh();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Stitch  (PointIndex& pointIndex, 
                                        int         levelToStitch, 
                                        bool        do2_5dStitchFirst)
    {
    pointIndex.Stitch(levelToStitch, do2_5dStitchFirst);
    return BSISUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
