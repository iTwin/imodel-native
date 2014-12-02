/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnHandlersAPI.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnCore/DgnCoreAPI.h"
#include <Geom/GeomApi.h>

#include "DgnHandlers.h"
#include "DimensionStyle.h"
#include "Dimension.h"

//__PUBLISH_SECTION_END__
#include "MultilineStyle.h"
#include "BitMaskLinkage.h"
#include "AssociativePoint.h"

//__PUBLISH_SECTION_START__

#include "IAreaFillProperties.h"
#include "IEditActionSource.h"
#include "IManipulator.h"
#include "IMaterialProperties.h"
#include "IModifyElement.h"
#include "ITextEdit.h"
#include "PersistentSnapPath.h"
#include "DropGraphics.h"
#include "IRasterSourceFileQuery.h"
#include "ArcHandlers.h"
#if defined (NEEDS_WORK_DGNITEM)
#include "AssocRegionHandler.h"
#endif
#include "BRepCellHandler.h"
#include "BSplineCurveHandler.h"
#include "BSplineSurfaceHandler.h"
#include "CellHeaderHandler.h"
#include "ChainHeaderHandlers.h"
#include "ComplexHeaderHandler.h"
#include "ConeHandler.h"
#include "CurveHandler.h"
#include "DgnStoreHandlers.h"
#include "ExtendedElementHandler.h"
#include "GroupedHoleHandler.h"
#include "LinearHandlers.h"
#include "NoteHandler.h"
#include "MeshHeaderHandler.h"
#include "MultilineHandler.h"
#include "OleCellHeaderHandler.h"
#include "SharedCellHandler.h"
#include "SurfaceAndSolidHandlers.h"
#include "TextHandlers.h"
#include "DimensionHandler.h"
#include "XMLFragment.h"
#include "TextBlock/TextBlockAPI.h"
#include "RasterHandlers.h"
#include "Locate.h"
#include "AssociativePoint.h"
#include "PointCloudHandler.h"
#include "DgnGraphicsElement.h"

#include "DgnECPersistence.h"

//__PUBLISH_SECTION_END__

#include "SpiralCurveHandler.h"
#include "PickContext.h"
#include "IEcPropertyHandler.h"
#include "IGeoCoordReproject.h"
#include "PersistentElementPathXAttributeHandler.h"
#include "MarkupHandlers.h"
#include "IIModelPublishExtension.h"

//__PUBLISH_SECTION_START__

/** @endcond */
