/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <ctype.h>
#include <ctime>
#include <chrono>
#include <math.h>

#define BEGIN_BENTLEY_FORMATTING_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Formatting {
#define END_BENTLEY_FORMATTING_NAMESPACE } END_BENTLEY_NAMESPACE

#define USING_BENTLEY_FORMATTING using namespace BENTLEY_NAMESPACE_NAME::Formatting;

#define FORMATTING_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_FORMATTING_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_FORMATTING_NAMESPACE
