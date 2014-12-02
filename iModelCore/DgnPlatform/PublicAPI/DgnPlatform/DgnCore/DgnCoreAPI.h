/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCoreAPI.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#if !defined (DOCUMENTATION_GENERATOR)

#include "DgnCore.h"
#include "../DgnCore/DgnFileIOApi.h"
#include "AreaPattern.h"
#include "ClipUtil.h"
#include "ClipPrimitive.h"
#include "ClipVector.h"
#include "ColorUtil.h"
#include "DisplayPath.h"
#include "ElementAgenda.h"
#include "TextParam.h"
#include "Material.h"

#include "Handler.h"
#include "DgnDomain.h"
#include <Bentley/CodePages.h>
#include "DisplayHandler.h"
#include "DisplayStyle.h"
#include "ElementHandle.h"
#include "ElementProperties.h"
#include "ElementGeometry.h"
#include "ElementGraphics.h"
#include "FenceParams.h"
#include "DgnFontManager.h"
#include "DgnTextStyle.h"
#include "GPArray.h"
#include "HitPath.h"
#include "IAnnotationHandler.h"
#include "IAuxCoordSys.h"
#include "IPickGeom.h"
#include "ITxnManager.h"
#include "IViewDraw.h"
#include "IViewOutput.h"
#include "IViewTransients.h"
#include "LineStyle.h"
#include "LineStyleManager.h"
#include "DgnFile.h"
#include "DgnColorMap.h"
#include "NotificationManager.h"
#include "PropertyContext.h"
#include "ScanCriteria.h"
#include "SolidKernel.h"
#include "TextString.h"
#include "ValueFormat.h"
#include "ValueParse.h"
#include "ViewContext.h"
#include "ViewController.h"
#include "XAttributeChange.h"
#include "PersistentElementPath.h"

//__PUBLISH_SECTION_END__
#include "DisplayAttribute.h"

#include "AnnotationScale.h"
#include "CompressedXAttribute.h"
#include "DgnTableElementScanner.h"
#include "EldscrFuncs.h"
#include "ElementHandlerManager.h"
#include <DgnPlatform/DgnHandlers/ElementUtil.h>
#include "GeoCoordinationManager.h"
#include "SnapContext.h"
#include "LineStyleApi.h"
#include "Linkage.h"
#include "NullContext.h"
#include "PropertyProcessors.h"
#include "QvViewport.h"
#include "rtypes.r.h"
#include "Sprites.h"
#include "Undo.h"
#include "XAttributeHandler.h"
#include "XGraphics.h"

DGNPLATFORM_EXPORT extern UShort element_drawn[];

//__PUBLISH_SECTION_START__
#endif // DOCUMENTATION_GENERATOR
