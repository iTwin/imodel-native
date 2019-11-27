/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifdef __IMODEL_DMSSUPPORT_BUILD__
#define IMODEL_DMSSUPPORT_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODEL_DMSSUPPORT_EXPORT IMPORT_ATTRIBUTE
#endif

#ifndef BEGIN_BENTLEY_DGN_NAMESPACE
#define BEGIN_BENTLEY_DGN_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
#define END_BENTLEY_DGN_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGN  using namespace BentleyApi::Dgn;
#endif

#include <Logging/bentleylogging.h>