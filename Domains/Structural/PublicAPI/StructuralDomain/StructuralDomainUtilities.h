/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/StructuralDomain/StructuralDomainUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <StructuralDomain/StructuralDomainApi.h>
#include <Json\Json.h>


#ifdef __STRUCTURAL_DOMAIN_BUILD__
#define STRUCTURAL_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define STRUCTURAL_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::STRUCTURAL_DOMAIN %STRUCTURAL_DOMAIN data types */
#define BEGIN_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace StructuralDomain {
#define END_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STRUCTURAL_DOMAIN        using namespace BentleyApi::StructuralDomain;


BEGIN_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE

END_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE
