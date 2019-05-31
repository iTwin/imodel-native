/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshProgress.h,v $
|   $Revision: 1.0 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMeshProgress.h>


/*
#if defined(DGNDB06_API)
    #undef 
#endif*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                               Elenie.Godzaridis   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEY_SM_EXPORT struct ScalableMeshProgress : public IScalableMeshProgress
    {

    private:

        const IScalableMeshProgressListener* m_listener = nullptr;
        std::atomic<bool> m_canceled = {0};
        std::atomic<ScalableMeshStep> m_currentStep = {ScalableMeshStep::STEP_NOT_STARTED};
        std::atomic<float> m_progressInStep = {0};
        std::atomic<int> m_progressStepIndex = {0};
        std::atomic<ScalableMeshStepProcess> m_progressStepProcess = {ScalableMeshStepProcess::PROCESS_INACTIVE};
        int m_totalNSteps = {0};
        bmap<int, int> m_totalNIterationsPerStep;

    protected:

        virtual bool _IsCanceled() const override;
        virtual void _Cancel() override;

        virtual bool _AddListener(const IScalableMeshProgressListener& listener) override;
        virtual void _UpdateListeners() override;

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
        BENTLEY_SM_EXPORT  ScalableMeshProgress();
        BENTLEY_SM_EXPORT ScalableMeshProgress(const ScalableMeshProcessType& processType, IScalableMeshPtr smPtr);

        BENTLEY_SM_EXPORT void SetCurrentIteration(const uint64_t& iteration);
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
