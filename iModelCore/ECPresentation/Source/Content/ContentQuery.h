/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/PresentationQuery.h"
#include "ContentQueryContracts.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ContentQuery : PresentationQuery<ContentQuery, ContentQueryContract, GenericQueryResultParameters> {};
DEFINE_QUERY_TYPEDEFS(ContentQuery)
INSTANTIATE_QUERY_SUBCLASS(ContentQuery, extern, EXPORT_VTABLE_ATTRIBUTE)
typedef QuerySet<ContentQuery> ContentQuerySet;

END_BENTLEY_ECPRESENTATION_NAMESPACE
