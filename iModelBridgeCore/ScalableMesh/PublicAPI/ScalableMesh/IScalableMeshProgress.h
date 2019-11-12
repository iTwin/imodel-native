/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMesh.h>
#include <Bentley/RefCounted.h>

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
    STEP_CONVERT_3DTILES_DATA,
    STEP_GENERATE_3DTILES_HEADERS,
    STEP_SAVEAS_3SM,
    STEP_DETECT_GROUND,
    STEP_GENERATE_TEXTURE,    
    STEP_CREATE_FROM_3MX,
    STEP_CLIPPING,
    STEP_QTY,
    };

enum ScalableMeshStepProcess
    {
    PROCESS_INACTIVE = 0,
    PROCESS_GENERATION,
    PROCESS_TEXTURING,
    PROCESS_DETECT_GROUND,
    PROCESS_CONVERT_3DTILES,
    PROCESS_SAVEAS_3SM,
    PROCESS_CREATION_FROM_3MX,
	PROCESS_TEXTURING_STREAMED
    };

enum ScalableMeshProcessType
    {
    CONVERT_3DTILES,
    SAVEAS_3SM
    };

struct IScalableMeshProgress;
typedef RefCountedPtr<IScalableMeshProgress>            IScalableMeshProgressPtr;

struct IScalableMeshProgressListener
    {
    public:
        virtual void CheckContinueOnProgress(IScalableMeshProgress* progress) const {};
    };

struct IScalableMeshProgress : public RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/

    protected:
        virtual bool _IsCanceled() const =0 ;
        virtual void _Cancel() = 0;

        virtual bool _AddListener(const IScalableMeshProgressListener& listener) = 0;
        virtual void _UpdateListeners() = 0;
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


        /*__PUBLISH_SECTION_START__*/
    public:
    BENTLEY_SM_EXPORT bool IsCanceled() const;
    BENTLEY_SM_EXPORT void Cancel();

    BENTLEY_SM_EXPORT static IScalableMeshProgressPtr Create(const ScalableMeshProcessType& processType, IScalableMeshPtr smPtr);
    BENTLEY_SM_EXPORT static IScalableMeshProgressPtr Create();

    BENTLEY_SM_EXPORT bool AddListener(const IScalableMeshProgressListener& listener);

    BENTLEY_SM_EXPORT void UpdateListeners();

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

 
END_BENTLEY_SCALABLEMESH_NAMESPACE
