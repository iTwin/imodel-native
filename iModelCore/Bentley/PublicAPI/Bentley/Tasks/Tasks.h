/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/Tasks.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>

#define BENTLEY_TASKS_NAMESPACE_NAME                    Tasks
#define BEGIN_BENTLEY_TASKS_NAMESPACE                   BEGIN_BENTLEY_NAMESPACE namespace BENTLEY_TASKS_NAMESPACE_NAME {
#define END_BENTLEY_TASKS_NAMESPACE                     END_BENTLEY_NAMESPACE }
#define USING_NAMESPACE_BENTLEY_TASKS                   using namespace BentleyApi::BENTLEY_TASKS_NAMESPACE_NAME;

#define LOGGER_NAMESPACE_BENTLEY_TASKS  "Bentley.Tasks"

BEGIN_BENTLEY_TASKS_NAMESPACE
END_BENTLEY_TASKS_NAMESPACE
