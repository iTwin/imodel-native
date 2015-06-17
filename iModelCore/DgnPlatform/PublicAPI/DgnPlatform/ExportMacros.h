/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ExportMacros.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#include <RmgrTools/ExportMacros.h>

#ifndef DGNPLATFORM_EXPORT
#if defined (__DGNPLATFORM_BUILD__)
    #define DGNPLATFORM_EXPORT EXPORT_ATTRIBUTE
#endif
#endif

#if defined (__DGNVIEW_BUILD__)
#define DGNVIEW_EXPORT            EXPORT_ATTRIBUTE
#endif

#if defined (__VISEDGESLIB_BUILD__)
#define VISEDGESLIB_EXPORT            EXPORT_ATTRIBUTE
#endif

#if defined (__DGNTOOLS_BUILD__)
#define DGNTOOLS_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__DGNDBTOXFDLL_BUILD__)
#define DGNDBTOXF_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__MSPSOLID_BUILD__)
#define __DGNHOST_BUILD__
#define MSPSOLID_EXPORT             EXPORT_ATTRIBUTE
#endif

#if defined (__PSOLIDCORE_BUILD__)
#define __DGNHOST_BUILD__
#define PSOLIDCORE_EXPORT           EXPORT_ATTRIBUTE
#endif

#if defined (__SOLIDINTEROP_BUILD__)
#define __DGNHOST_BUILD__
#define SOLIDINTEROP_EXPORT         EXPORT_ATTRIBUTE
#endif

#if defined (__MSACIS_BUILD__)
#define __DGNHOST_BUILD__
#define MSACIS_EXPORT             EXPORT_ATTRIBUTE
#endif

#if defined (__KISOLID_BUILD__)
#define __DGNHOST_BUILD__
#define KISOLID_EXPORT             EXPORT_ATTRIBUTE
#endif

#if defined (__MSPFACET_BUILD__)
#define __DGNHOST_BUILD__
#define MSPFACET_EXPORT             EXPORT_ATTRIBUTE
#endif

#if defined (__DGNHOST_BUILD__)
#define DGNHOST_EXPORT            EXPORT_ATTRIBUTE
#define DGNHOST_IMPLEMENTED
#else
#define DGNHOST_IMPLEMENTED       IMPORT_ATTRIBUTE
#endif

#if defined (__FEDERATIONTEST_BUILD__)
#define FEDERATIONTEST_EXPORT            EXPORT_ATTRIBUTE
#endif

#if defined (__IMAGELIB_BUILD__)
#define IMAGELIB_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__VIDEOLIB_BUILD__)
#define VIDEOLIB_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERLIB_BUILD__)
#define RASTERLIB_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERCORE_BUILD__)
#define RASTERCORE_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTEREXT_BUILD__)
#define RASTEREXT_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERSELECT_BUILD__)
#define RASTERSELECT_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERMANAGER_BUILD__)
#define RASTERMANAGER_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERWINOPEN_BUILD__)
#define RASTERWINOPEN_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__RASTERVALIDATE_BUILD__)
#define RASTERVALIDATE_EXPORT EXPORT_ATTRIBUTE
#endif

#if defined (__ECXDATATREE_BUILD__)
#define ECXDATATREE_EXPORT            EXPORT_ATTRIBUTE
#endif

#if defined (__MARKUPLIB_BUILD__)
#define MARKUPLIB_EXPORT EXPORT_ATTRIBUTE
#endif

//__PUBLISH_SECTION_START__

#if !defined (MSPSOLID_EXPORT)
#define MSPSOLID_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (PSOLIDCORE_EXPORT)
#define PSOLIDCORE_EXPORT   IMPORT_ATTRIBUTE
#endif
#if !defined (SOLIDINTEROP_EXPORT)
#define SOLIDINTEROP_EXPORT   IMPORT_ATTRIBUTE
#endif
#if !defined (MSACIS_EXPORT)
#define MSACIS_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (KISOLID_EXPORT)
#define KISOLID_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (MSPFACET_EXPORT)
#define MSPFACET_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (DGNHOST_EXPORT)
#define DGNHOST_EXPORT      IMPORT_ATTRIBUTE
#endif
#if !defined (DGNDBTOXF_EXPORT)
#define DGNDBTOXF_EXPORT  IMPORT_ATTRIBUTE
#endif
#if !defined (DGNVIEW_EXPORT)
#define DGNVIEW_EXPORT  IMPORT_ATTRIBUTE
#endif
#if !defined (VISEDGESLIB_EXPORT)
#define VISEDGESLIB_EXPORT  IMPORT_ATTRIBUTE
#endif
#if !defined (DGNTOOLS_EXPORT)
#define DGNTOOLS_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (DGNPLATFORM_EXPORT)
#define DGNPLATFORM_EXPORT  IMPORT_ATTRIBUTE
#endif
#if !defined (FOREIGNFORMAT_EXPORT)
#define FOREIGNFORMAT_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RMGRSUBS_EXPORT)
#define RMGRSUBS_EXPORT  IMPORT_ATTRIBUTE
#endif
#if !defined (ECXATTRIBUTES_EXPORT)
#define ECXATTRIBUTES_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (ECXDATATREE_EXPORT)
#define ECXDATATREE_EXPORT       IMPORT_ATTRIBUTE
#endif
#if !defined (FEDERATIONTEST_EXPORT)
#define FEDERATIONTEST_EXPORT    IMPORT_ATTRIBUTE
#endif
#if !defined (IMAGELIB_EXPORT)
#define IMAGELIB_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (VIDEOLIB_EXPORT)
#define VIDEOLIB_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERLIB_EXPORT)
#define RASTERLIB_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERCORE_EXPORT)
#define RASTERCORE_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTEREXT_EXPORT)
#define RASTEREXT_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERSELECT_EXPORT)
#define RASTERSELECT_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERMANAGER_EXPORT)
#define RASTERMANAGER_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERWINOPEN_EXPORT)
#define RASTERWINOPEN_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (RASTERVALIDATE_EXPORT)
#define RASTERVALIDATE_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (MARKUPLIB_EXPORT)
#define MARKUPLIB_EXPORT IMPORT_ATTRIBUTE
#endif
#if !defined (TOOLSUBS_EXPORT)
#define TOOLSUBS_EXPORT  IMPORT_ATTRIBUTE
#endif