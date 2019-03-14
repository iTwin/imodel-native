/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/DgnDbSync.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/BentleyAllocator.h>
#include <Bentley/WString.h>
#include <DgnPlatform/DgnPlatform.h>

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DgnDb and foreign data formats. */
#define DGNDBSYNC_NAMESPACE_NAME DgnDbSync
#define BEGIN_DGNDBSYNC_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace DGNDBSYNC_NAMESPACE_NAME {
#define END_DGNDBSYNC_NAMESPACE   } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_DGNDBSYNC using namespace BentleyApi::Dgn::DgnDbSync;

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DGNDB and foreign data formats. */
#define DGNDBSYNC_DGNV8_NAMESPACE_NAME DgnV8
#define BEGIN_DGNDBSYNC_DGNV8_NAMESPACE BEGIN_DGNDBSYNC_NAMESPACE namespace DGNDBSYNC_DGNV8_NAMESPACE_NAME {    
#define END_DGNDBSYNC_DGNV8_NAMESPACE   } END_DGNDBSYNC_NAMESPACE
#define USING_NAMESPACE_DGNDBSYNC_DGNV8 using namespace BentleyApi::Dgn::DgnDbSync::DgnV8;

/** @namespace DgnDbSync Contains types defined by %Bentley Systems that are used to synchronize between DGNDB and foreign data formats. */
#define DGNDBSYNC_DWG_NAMESPACE_NAME Dwg
#define BEGIN_DGNDBSYNC_DWG_NAMESPACE BEGIN_DGNDBSYNC_NAMESPACE namespace DGNDBSYNC_DWG_NAMESPACE_NAME {
#define END_DGNDBSYNC_DWG_NAMESPACE   } END_DGNDBSYNC_NAMESPACE
#define USING_NAMESPACE_DGNDBSYNC_DWG using namespace BentleyApi::Dgn::DgnDbSync::Dwg;

//__PUBLISH_SECTION_END__

#ifndef DGNDBSYNC_EXPORT
#if defined (__DGNDBSYNC_BUILD__)
    #define DGNDBSYNC_EXPORT EXPORT_ATTRIBUTE
#endif
#endif

//__PUBLISH_SECTION_START__

#if !defined (DGNDBSYNC_EXPORT)
#define DGNDBSYNC_EXPORT  IMPORT_ATTRIBUTE
#endif

#define DGNDBSYNC_TYPEDEFS(_structname_) \
    BEGIN_DGNDBSYNC_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_DGNDBSYNC_NAMESPACE

#define DGNDBSYNC_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_DGNDBSYNC_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_DGNDBSYNC_NAMESPACE 
