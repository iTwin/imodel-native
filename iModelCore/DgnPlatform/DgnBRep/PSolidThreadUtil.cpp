/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidThreadUtil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN



//=======================================================================================
// @bsistruct                                                       Ray.Bentley   09/2017
//=======================================================================================
struct PSolidThreadLocalStorage
{
private:
    PK_PMARK_t          m_mark;
    PK_PARTITION_t      m_partition;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
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
* @bsimethod                                                    RayBentley      09/2017
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


static      BeThreadLocalStorage*       s_threadLocalParasolidHandlerStorage;    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::GoToPMark()
    {
    PSolidThreadLocalStorage*     storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());
    
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

    if (NULL != newEntities)
        PK_MEMORY_free(newEntities);
    if (NULL != modEntities)
        PK_MEMORY_free(modEntities);
    if (NULL != delEntities)
        PK_MEMORY_free(delEntities);

    BeAssert(PK_ERROR_no_errors == errorCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::Initialize()
    {
    s_threadLocalParasolidHandlerStorage->SetValueAsPointer(new PSolidThreadLocalStorage());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadLocalStorage::Clear()
    {
    PSolidThreadLocalStorage*     storage = reinterpret_cast<PSolidThreadLocalStorage*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());

    if (nullptr == storage)
        {
        BeAssert(false);
        return;
        }
    delete storage;
    s_threadLocalParasolidHandlerStorage->SetValueAsPointer(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
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
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearExclusions()
    {
    PK_THREAD_exclusion_t       clearedExclusion;
    PK_LOGICAL_t                clearedThisThread;

    PK_THREAD_clear_exclusion (PK_THREAD_exclusion_serious_c, &clearedExclusion, &clearedThisThread);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t threadedParasolidErrorHandler (PK_ERROR_sf_t* errorSf)
    {
    if (errorSf->severity > PK_ERROR_mild)
        {
        switch (errorSf->code)
            {
            case 942:         // Edge crossing (constructing face from curve vector to perform intersections)
            case 547:         // Nonmanifold  (constructing face from curve vector to perform intersections)
            case 1083:        // Degenerate trim loop.
                break;

            default:
                //printf ("Error %d caught in parasolid error handler\n", errorSf->code);
                BeAssert (false && "Severe error during threaded processing");
                break;


            }       

        clearExclusions ();
        
        PK_THREAD_tidy();

        PSolidThreadLocalStorage::GoToPMark ();

        throw PSolidThreadUtil::ParasolidException();
        }
    
    return 0; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::MainThreadMark::MainThreadMark()
    {
    if (nullptr == (m_previousLocalStorage = s_threadLocalParasolidHandlerStorage))
        s_threadLocalParasolidHandlerStorage = new BeThreadLocalStorage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::MainThreadMark::~MainThreadMark()
    { 
    if (nullptr == m_previousLocalStorage) 
        DELETE_AND_CLEAR (s_threadLocalParasolidHandlerStorage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadOuterMark::WorkerThreadOuterMark()
    {
    PSolidThreadLocalStorage::Initialize();

    PK_ERROR_frustrum_t     errorFrustum;

    errorFrustum.handler_fn = threadedParasolidErrorHandler;
    PK_THREAD_register_error_cbs(errorFrustum);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidThreadUtil::WorkerThreadOuterMark::~WorkerThreadOuterMark()
    {
    PSolidThreadLocalStorage::Clear();

    PK_ERROR_frustrum_t     errorFrustum;

    errorFrustum.handler_fn = nullptr;
    PK_THREAD_register_error_cbs(errorFrustum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2017
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
* @bsimethod                                                    RayBentley      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidThreadUtil::SetThreadPartitionMark()
    {
    PSolidThreadLocalStorage::PrepareToWork();
    }
    

     
     
     
