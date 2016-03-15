/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Casablanca/CasablancaTaskRunner.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTaskRunner.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

struct ITaskScheduler;
struct HttpRequestTask;
struct CasablancaTaskRunner;

typedef RefCountedPtr<CasablancaTaskRunner> CasablancaTaskRunnerPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CasablancaTaskRunner : Tasks::AsyncTaskRunner
    {
private:
    
protected:
    virtual void _RunAsyncTasksLoop () override;

    BEHTTP_EXPORT CasablancaTaskRunner ();

public:
    BEHTTP_EXPORT static std::shared_ptr<CasablancaTaskRunner> Create ()
        {
        return std::shared_ptr <CasablancaTaskRunner> (new CasablancaTaskRunner ());
        }
    };

END_BENTLEY_HTTP_NAMESPACE
