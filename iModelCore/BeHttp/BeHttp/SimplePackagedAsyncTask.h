/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/SimplePackagedAsyncTask.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Tasks/AsyncTask.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/HttpRequest.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                             Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <class D, class R>
struct SimplePackagedAsyncTask : public Tasks::PackagedAsyncTask<R>
    {
private:
    D m_data;

public:
    SimplePackagedAsyncTask (const D& data) :
        Tasks::PackagedAsyncTask<R> (nullptr),
        m_data (data)
        {
        }

    D& GetData ()
        {
        return m_data;
        }

    void OnFinished (const R& result)
        {
        Tasks::PackagedAsyncTask<R>::m_result = result;
        Tasks::PackagedAsyncTask<R>::Execute ();
        }

    virtual void _OnExecute ()
        {
        }
    };

END_BENTLEY_HTTP_NAMESPACE