/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCoreAPI.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#if !defined (DOCUMENTATION_GENERATOR)

#include <DgnPlatform/DgnPlatform.h>
#include "AreaPattern.h"
#include "ClipUtil.h"
#include "ClipPrimitive.h"
#include "ClipVector.h"
#include "ColorUtil.h"
#include "DisplayPath.h"
#include "ElementAgenda.h"
#include "Material.h"
#include "ElementHandler.h"
#include "DgnDomain.h"
#include "ElementHandle.h"
#include "ElementProperties.h"
#include "ElementGeometry.h"
#include "ElementGraphics.h"
#include "FenceParams.h"
#include "DgnFontManager.h"
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
#include "DgnEntity.h"
#include "DgnItem.h"
#include "DgnDb.h"
#include "NotificationManager.h"
#include "PropertyContext.h"
#include "ScanCriteria.h"
#include "SolidKernel.h"
#include "TextString.h"
#include "ValueFormat.h"
#include "ValueParse.h"
#include "ViewContext.h"
#include "ViewController.h"
#include "GeomPart.h"
#include "Annotations/TextAnnotation.h"

//__PUBLISH_SECTION_END__
#include "SnapContext.h"
#include "LineStyleApi.h"
#include "NullContext.h"
#include "PropertyProcessors.h"
#include "QvViewport.h"
#include "Sprites.h"
#include "Undo.h"

//__PUBLISH_SECTION_START__
#endif // DOCUMENTATION_GENERATOR
