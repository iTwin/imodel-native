/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ScalableMeshProgress.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool IScalableMeshProgress::IsCanceled() const
    {
    return _IsCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshProgress::Cancel()
    {
    return _Cancel();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool IScalableMeshProgress::AddListener(const IScalableMeshProgressListener& listener)
    {
    return _AddListener(listener);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshProgress::UpdateListeners()
    {
    return _UpdateListeners();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshProgressPtr IScalableMeshProgress::Create(const ScalableMeshProcessType& processType, IScalableMeshPtr smPtr)
    {
    return new ScalableMeshProgress(processType, smPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Jean-Philippe.Pons   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshProgressPtr IScalableMeshProgress::Create()
{
    return new ScalableMeshProgress();
}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStep> const& IScalableMeshProgress::GetProgressStep() const
    {
    return _GetProgressStep();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshProgress::GetTotalNumberOfSteps() const
    {
    return _GetTotalNumberOfSteps();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStepProcess> const& IScalableMeshProgress::GetProgressStepProcess() const
    {
    return _GetProgressStepProcess();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<int> const& IScalableMeshProgress::GetProgressStepIndex() const
    {
    return _GetProgressStepIndex();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshProgress::SetTotalNumberOfSteps(int step)
    {
    return _SetTotalNumberOfSteps(step);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<float> const& IScalableMeshProgress::GetProgress() const
    {
    return _GetProgress();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<float>& IScalableMeshProgress::Progress()
    {
    return _Progress();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStep>& IScalableMeshProgress::ProgressStep()
    {
    return _ProgressStep();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStepProcess>& IScalableMeshProgress::ProgressStepProcess()
    {
    return _ProgressStepProcess();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<int>& IScalableMeshProgress::ProgressStepIndex()
    {
    return _ProgressStepIndex();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshProgress::ScalableMeshProgress()
    {}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshProgress::ScalableMeshProgress(const ScalableMeshProcessType& processType, IScalableMeshPtr smPtr)
    {
    if (smPtr == nullptr)
        {
        BeAssert(!"Should use default constructor instead");
        return;
        }
    switch (processType)
        {
        case CONVERT_3DTILES:
        {
        auto nodeCount = smPtr->GetNodeCount();
        BeAssert(nodeCount > 0);
        m_progressStepProcess = ScalableMeshStepProcess::PROCESS_CONVERT_3DTILES;
        m_totalNSteps = 2; // index + data
        m_totalNIterationsPerStep[0] = m_totalNIterationsPerStep[1] = nodeCount;
        break;
        }
        case SAVEAS_3SM:
        {
        auto nodeCount = smPtr->GetNodeCount();
        BeAssert(nodeCount > 0);
        m_progressStepProcess = ScalableMeshStepProcess::PROCESS_SAVEAS_3SM;
        m_totalNSteps = 1;
        m_totalNIterationsPerStep[0] = nodeCount;
        break;
        }
        default:
        {
        BeAssert(!"Unknown process type for scalable mesh progress");
        }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshProgress::SetCurrentIteration(const uint64_t & iteration)
    {
    m_progressInStep = (float)iteration / m_totalNIterationsPerStep[m_progressStepIndex - 1];
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshProgress::_IsCanceled() const
    {
    return m_canceled;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshProgress::_Cancel()
    {
    m_canceled = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshProgress::_AddListener(const IScalableMeshProgressListener& listener)
    {
    if (m_listener != nullptr)
        {
        assert(!"Only one listener is supported");
        return false;
        }
    m_listener = &listener;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshProgress::_UpdateListeners()
    {
	if(m_listener != nullptr)
		m_listener->CheckContinueOnProgress(this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStep> const& ScalableMeshProgress::_GetProgressStep() const
    {
    return m_currentStep;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<float> const& ScalableMeshProgress::_GetProgress() const
    {
    return m_progressInStep;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<float>& ScalableMeshProgress::_Progress()
    {
    return m_progressInStep;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStep>& ScalableMeshProgress::_ProgressStep()
    {
    return m_currentStep;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<int> const& ScalableMeshProgress::_GetProgressStepIndex() const
    {
    return m_progressStepIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStepProcess> const& ScalableMeshProgress::_GetProgressStepProcess() const
    {
    return m_progressStepProcess;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshProgress::_SetTotalNumberOfSteps(int step)
    {
    m_totalNSteps = step;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<ScalableMeshStepProcess>& ScalableMeshProgress::_ProgressStepProcess()
    {
    return m_progressStepProcess;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Richard.Bois   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::atomic<int>& ScalableMeshProgress::_ProgressStepIndex()
    {
    return m_progressStepIndex;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE