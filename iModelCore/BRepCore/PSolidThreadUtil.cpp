/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsistruct
//=======================================================================================
struct PSolidThreadLocalStorage
{
private:
    PK_PMARK_t          m_mark;
    PK_PARTITION_t      m_partition;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadLocalStorage()
    {
    if (SUCCESS != PK_PARTITION_create_empty(&m_partition))
        {
        BeAssert(false);
        return;
        }

    PK_PARTITION_set_type(m_partition, PK_PARTITION_type_light_c);
    PK_PARTITION_make_pmark(m_partition, &m_mark);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~PSolidThreadLocalStorage()
    {
    PK_PARTITION_delete_o_t options;

    PK_PARTITION_delete_o_m (options);

    options.delete_non_empty = true;
    PK_PARTITION_delete (m_partition, &options);
    }

public:

PK_PARTITION_t  GetPartition() const { return m_partition; }
static void  GoToPMark ();
static void  Initialize();
static void  Clear();
static void  PrepareToWork();
};

static BeThreadLocalStorage*    s_threadLocalParasolidHandlerStorage;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::GoToPMark()
    {
    PSolidThreadLocalStorage*   storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());

    PK_PMARK_goto_o_t   opts;
    int                 numNew, numMod, numDel;
    PK_ENTITY_t*        newEntities = nullptr;
    PK_ENTITY_t*        modEntities = nullptr;
    int*                delEntities = nullptr;

    PK_PMARK_goto_o_m(opts);
    opts.want_attrib_mod = FALSE;
    opts.want_new_entities = FALSE;
    opts.want_mod_entities = FALSE;
    opts.want_del_entities = FALSE;

    PK_ERROR_code_t errorCode = PK_PMARK_goto_2(storage->m_mark, &opts, &numNew, &newEntities, &numMod, &modEntities, &numDel, &delEntities);
    UNUSED_VARIABLE(errorCode);

    if (NULL != newEntities)
        PK_MEMORY_free(newEntities);
    if (NULL != modEntities)
        PK_MEMORY_free(modEntities);
    if (NULL != delEntities)
        PK_MEMORY_free(delEntities);

    BeAssert(PK_ERROR_no_errors == errorCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::Initialize()
    {
    s_threadLocalParasolidHandlerStorage->SetValueAsPointer(new PSolidThreadLocalStorage());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::Clear()
    {
    PSolidThreadLocalStorage*   storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());

    if (nullptr == storage)
        {
        BeAssert(false);
        return;
        }
    delete storage;
    s_threadLocalParasolidHandlerStorage->SetValueAsPointer(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::PrepareToWork()
    {
    PSolidThreadLocalStorage*     storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());

    if (nullptr == storage)
        {
        BeAssert(false);
        return;
        }

    PK_PARTITION_advance_pmark_o_t advanceMarkOption;
    PK_PARTITION_advance_pmark_o_m(advanceMarkOption);

    PK_PARTITION_advance_pmark(storage->m_partition, &advanceMarkOption, &storage->m_mark);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearExclusions()
    {
    PK_THREAD_exclusion_t       clearedExclusion;
    PK_LOGICAL_t                clearedThisThread;

    PK_THREAD_clear_exclusion (PK_THREAD_exclusion_serious_c, &clearedExclusion, &clearedThisThread);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t threadedErrorHandler(PK_ERROR_sf_t* errorSf, bool hasMark)
    {
    if (errorSf->severity <= PK_ERROR_mild)
        return 0;

    bool throwException = true;

    switch (errorSf->code)
        {
        case PK_ERROR_crossing_edge:
        case PK_ERROR_non_manifold:
        case PK_ERROR_trim_loop_degenerate:
        case PK_ERROR_failed_to_transform:
            throwException = false; // These errors aren't unexpected and should be handled by failure status...
            break;

        case PK_ERROR_tag_limit_exceeded:
            LOG_BREPCORE.error("Solid kernel tag limit exceeded");
            break;

        default:
            LOG_BREPCORE.errorv("Solid kernel severe error: %d", errorSf->code);
            break;
        }

    clearExclusions();
    PK_THREAD_tidy();

    if (hasMark)
        PSolidThreadLocalStorage::GoToPMark();

    if (!throwException)
        return 0;

    throw std::runtime_error(std::string("Solid kernel severe error: ") + std::to_string(errorSf->code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t threadedParasolidErrorHandlerWithoutMark(PK_ERROR_sf_t* errorSf)
    {
    return threadedErrorHandler(errorSf, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t threadedParasolidErrorHandler (PK_ERROR_sf_t* errorSf)
    {
    return threadedErrorHandler(errorSf, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::MainThreadMark::MainThreadMark()
    {
    if (nullptr == (m_previousLocalStorage = s_threadLocalParasolidHandlerStorage))
        s_threadLocalParasolidHandlerStorage = new BeThreadLocalStorage;

    // Must register error handler in order to clear exclusions if Parasolid encounters any
    // errors on main thread. Otherwise anything that runs Parasolid on worker threads (tile
    // generation, etc.) will hang. After discussion with Brien, don't set mark unless we find
    // it's necessary - has significant performance cost.
    PK_ERROR_frustrum_t errorFrustum;
    errorFrustum.handler_fn = threadedParasolidErrorHandlerWithoutMark;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::MainThreadMark::~MainThreadMark()
    {
    if (nullptr == m_previousLocalStorage)
        DELETE_AND_CLEAR (s_threadLocalParasolidHandlerStorage);

    // PK_ERROR_frustrum_t intentionally not unregistered. Leaving up provides extra bomb guard
    // and subsequent caller can still replace without issue.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadOuterMark::WorkerThreadOuterMark()
    {
    PSolidThreadLocalStorage::Initialize();

    PK_ERROR_frustrum_t     errorFrustum;

    errorFrustum.handler_fn = threadedParasolidErrorHandler;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadOuterMark::~WorkerThreadOuterMark()
    {
    PSolidThreadLocalStorage::Clear();

    PK_ERROR_frustrum_t     errorFrustum;

    errorFrustum.handler_fn = nullptr;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadErrorHandler::WorkerThreadErrorHandler()
    {
    PK_ERROR_frustrum_t errorFrustum;
    errorFrustum.handler_fn = threadedParasolidErrorHandlerWithoutMark;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadErrorHandler::~WorkerThreadErrorHandler()
    {
    PK_ERROR_frustrum_t errorFrustum;
    errorFrustum.handler_fn = nullptr;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_PARTITION_t  PSolidThreadUtil::GetThreadPartition()
    {
    PSolidThreadLocalStorage*     storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());

    if (nullptr == storage)
        {
        BeAssert(false);
        return 0;
        }

    return storage->GetPartition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadUtil::SetThreadPartitionMark()
    {
    PSolidThreadLocalStorage::PrepareToWork();
    }
