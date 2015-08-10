/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/CrawlerLib.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>


#if defined (__CRAWLERLIB_BUILD__)
#   define CRAWLERLIB_EXPORT EXPORT_ATTRIBUTE
#else
#   define CRAWLERLIB_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE              BEGIN_BENTLEY_NAMESPACE namespace CrawlerLib {
#define END_BENTLEY_CRAWLERLIB_NAMESPACE                } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_CRAWLERLIB              using namespace BentleyApi::CrawlerLib;


#define CRAWLERLIB_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_CRAWLERLIB_NAMESPACE

#define CRAWLERLIB_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_CRAWLERLIB_NAMESPACE

CRAWLERLIB_TYPEDEFS(PageContent)
CRAWLERLIB_REF_COUNTED_PTR(PageContent)
CRAWLERLIB_TYPEDEFS(Url)
CRAWLERLIB_REF_COUNTED_PTR(Url)
CRAWLERLIB_TYPEDEFS(Seed)
CRAWLERLIB_REF_COUNTED_PTR(Seed)
CRAWLERLIB_TYPEDEFS(RobotsTxtContent)
CRAWLERLIB_REF_COUNTED_PTR(RobotsTxtContent)
CRAWLERLIB_TYPEDEFS(DownloadJob)
CRAWLERLIB_REF_COUNTED_PTR(DownloadJob)
CRAWLERLIB_TYPEDEFS(CrawlDelaySleeper)
CRAWLERLIB_REF_COUNTED_PTR(CrawlDelaySleeper)

