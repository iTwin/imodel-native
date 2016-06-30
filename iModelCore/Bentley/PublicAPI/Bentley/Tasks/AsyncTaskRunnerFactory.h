/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Tasks/AsyncTaskRunnerFactory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/ITaskRunner.h>
#include <Bentley/Tasks/AsyncTaskRunner.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                             
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAsyncTaskRunnerFactory
    {
    public:
        virtual std::shared_ptr<ITaskRunner> CreateRunner () = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                       
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename R>
struct AsyncTaskRunnerFactory : public IAsyncTaskRunnerFactory
    {
    public:
        /*--------------------------------------------------------------------------------------+
        * @bsimethod                                      
        +---------------+---------------+---------------+---------------+---------------+------*/
        std::shared_ptr<ITaskRunner> CreateRunner () override
            {
            return R::Create ();
            }
    };

END_BENTLEY_TASKS_NAMESPACE
