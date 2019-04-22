/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/Error.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

template<typename ValueType> using  Result       = Tasks::AsyncResult<ValueType, Error>;
template<typename ValueType> using  ResultPtr    = std::shared_ptr<Result<ValueType>>;
template<typename ValueType> using  Task         = Tasks::PackagedAsyncTask<Result<ValueType>>;
template<typename ValueType> using  TaskPtr      = Tasks::AsyncTaskPtr<Result<ValueType>>;

#define DEFINE_TASK_TYPEDEFS(_type_, _name_) \
    typedef Result<_type_>           _name_##Result; \
    typedef ResultPtr<_type_>        _name_##ResultPtr; \
    typedef Result<_type_> const&    _name_##ResultCR; \
    typedef Task<_type_>             _name_##Task; \
    typedef TaskPtr<_type_>          _name_##TaskPtr;

DEFINE_TASK_TYPEDEFS(void, Status);
END_BENTLEY_IMODELHUB_NAMESPACE
