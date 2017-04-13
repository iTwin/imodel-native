/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshCreator.h $
|    $RCSfile: IScalableMeshCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2012/03/21 18:37:07 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
typedef uint8_t byte;

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
#include <ScalableMesh/ITextureProvider.h>

#ifndef VANCOUVER_API
namespace BENTLEY_NAMESPACE_NAME
    {
    namespace ImagePP
        {
#endif
        class HIMMosaic;
#ifndef VANCOUVER_API
        }
    }
#endif
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


enum ScalableMeshStep
    {
    STEP_NOT_STARTED = 0,
    STEP_IMPORT_SOURCE,
    STEP_BALANCE,
    STEP_MESH,
    STEP_GENERATE_LOD,
    STEP_TEXTURE,
    STEP_SAVE,
    STEP_DETECT_GROUND,
    STEP_GENERATE_TEXTURE,
    STEP_QTY
    };

enum ScalableMeshStepProcess
    {
    PROCESS_INACTIVE = 0,
    PROCESS_GENERATION,
    PROCESS_TEXTURING,
    PROCESS_DETECT_GROUND
    };

struct IScalableMeshCreator;
typedef RefCountedPtr<IScalableMeshCreator>            IScalableMeshCreatorPtr;

struct IScalableMeshProgress
    {
    protected:
        virtual bool _IsCanceled() const =0 ;
        virtual void _Cancel() = 0;

        virtual std::atomic<ScalableMeshStep> const& _GetProgressStep() const = 0;
        virtual std::atomic<ScalableMeshStepProcess> const& _GetProgressStepProcess() const = 0;
        virtual int _GetTotalNumberOfSteps() const = 0;
        virtual std::atomic<int> const& _GetProgressStepIndex() const = 0;

        virtual void _SetTotalNumberOfSteps(int step) = 0;

        virtual std::atomic<float> const& _GetProgress() const = 0; //Progress of current step ([0..1])

        virtual std::atomic<float>& _Progress() = 0;
        virtual std::atomic<ScalableMeshStep>& _ProgressStep() = 0;
        virtual std::atomic<ScalableMeshStepProcess>& _ProgressStepProcess() = 0;
        virtual std::atomic<int>& _ProgressStepIndex() = 0;


    public:
    BENTLEY_SM_EXPORT bool IsCanceled() const;
    BENTLEY_SM_EXPORT void Cancel();

    BENTLEY_SM_EXPORT std::atomic<ScalableMeshStep> const& GetProgressStep() const;
    BENTLEY_SM_EXPORT std::atomic<ScalableMeshStepProcess> const& GetProgressStepProcess() const;
    BENTLEY_SM_EXPORT std::atomic<int> const& GetProgressStepIndex() const;
    BENTLEY_SM_EXPORT int GetTotalNumberOfSteps() const;

    BENTLEY_SM_EXPORT void SetTotalNumberOfSteps(int step);

    BENTLEY_SM_EXPORT std::atomic<float> const& GetProgress() const; //Progress of current step ([0..1])
    
    BENTLEY_SM_EXPORT std::atomic<float>& Progress();
    BENTLEY_SM_EXPORT std::atomic<ScalableMeshStep>& ProgressStep();
    BENTLEY_SM_EXPORT std::atomic<ScalableMeshStepProcess>& ProgressStepProcess();
    BENTLEY_SM_EXPORT std::atomic<int>& ProgressStepIndex();
    };

 
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

        BENTLEY_SM_EXPORT StatusInt               Create(bool isSingleFile = true, bool restrictLevelForPropagation = false);

        BENTLEY_SM_EXPORT StatusInt               SetTextureMosaic(MOSAIC_TYPE* mosaicP);
        BENTLEY_SM_EXPORT StatusInt               SetTextureProvider(ITextureProviderPtr texProvider);


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

        BENTLEY_SM_EXPORT  void                   SetBaseExtraFilesPath(const WString& path);

        BENTLEY_SM_EXPORT  IScalableMeshProgress* GetProgress();


        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath,
                                                                                     StatusInt&                status);

        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&     scmPtr,
                                                                                     StatusInt&                  status);

        BENTLEY_SM_EXPORT StatusInt  SetTextureStreamFromUrl(WString url);

       /* BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const WChar*              filePath);

        BENTLEY_SM_EXPORT static IScalableMeshCreatorPtr GetFor                     (const IScalableMeshPtr&   scmPtr);*/



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
