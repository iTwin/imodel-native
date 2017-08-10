/*--------------------------------------------------------------------------------------+
|
|     $Source: BimCreater/Bimcreater/Geometry.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct GeometricTools
    {
    public:
        static  BentleyStatus CreateDoorGeometry              (Dgn::PhysicalElementPtr door,    BuildingPhysical::BuildingPhysicalModelR model );
        static  BentleyStatus CreateWindowGeometry            (Dgn::PhysicalElementPtr window,  BuildingPhysical::BuildingPhysicalModelR model);
        static  BentleyStatus CreateGeometry                  (Dgn::PhysicalElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);
        static  BentleyStatus CreateFrameGeometry             (Dgn::GeometryBuilderPtr builder, BuildingPhysical::BuildingPhysicalModelR model, double frameDepth, double frameWidth, double height, double width, bool fullFrame);
		static  BentleyStatus CreatePidLineGeometry           (Dgn::DrawingGraphicR element, Dgn::DrawingModelR drawingModel, DPoint2dCP points, int count);
		static  BentleyStatus CreatePidValveGeometry          (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePidPumpGeometry           (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePidTankGeometry           (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePidNozzleGeometry         (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
        static  BentleyStatus CreatePidVirtualNozzleGeometry  (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
        static  BentleyStatus CreatePidReducerGeometry        (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePid3WayValveGeometry      (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePidRoundTankGeometry      (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
		static  BentleyStatus CreatePidVesselGeometry         (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId);
        static  BentleyStatus CreateAnnotationTextGeometry    (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Utf8StringCR textValue);



    };

