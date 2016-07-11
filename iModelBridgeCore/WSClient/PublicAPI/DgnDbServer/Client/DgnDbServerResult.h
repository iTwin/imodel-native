/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerResult.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/DgnDbServerError.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

template<typename ValueType> using  DgnDbServerResult       = Tasks::AsyncResult<ValueType, DgnDbServerError>;
template<typename ValueType> using  DgnDbServerResultPtr    = std::shared_ptr<DgnDbServerResult<ValueType>>;
template<typename ValueType> using  DgnDbServerTask         = Tasks::PackagedAsyncTask<DgnDbServerResult<ValueType>>;
template<typename ValueType> using  DgnDbServerTaskPtr      = Tasks::AsyncTaskPtr<DgnDbServerResult<ValueType>>;

#define DEFINE_TASK_TYPEDEFS(_type_, _name_) \
    typedef DgnDbServerResult<_type_>           _name_##Result; \
    typedef DgnDbServerResultPtr<_type_>        _name_##ResultPtr; \
    typedef DgnDbServerResult<_type_> const&    _name_##ResultCR; \
    typedef DgnDbServerTask<_type_>             _name_##Task; \
    typedef DgnDbServerTaskPtr<_type_>          _name_##TaskPtr;

DEFINE_TASK_TYPEDEFS(void, DgnDbServerStatus);
END_BENTLEY_DGNDBSERVER_NAMESPACE