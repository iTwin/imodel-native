/*--------------------------------------------------------------------------------------+
|
|     $Source: BimCreater/Bimcreater/Geometry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      
#define PID_CATEGORY               "PID Drawing Categories"
#define PID_EQUIP_SUBCATEGORY      "Equipment"
#define PID_PIPING_SUBCATEGORY     "Piping"
#define PID_ANNOTATION_SUBCATEGORY "Annotation"


struct GeometricTools
    {
    public:
        static  BentleyStatus CreateDoorGeometry              (Dgn::PhysicalElementPtr door,    BuildingPhysical::BuildingPhysicalModelR model );
        static  BentleyStatus CreateWindowGeometry            (Dgn::PhysicalElementPtr window,  BuildingPhysical::BuildingPhysicalModelR model);
        static  BentleyStatus CreateGeometry                  (Dgn::PhysicalElementPtr element, BuildingPhysical::BuildingPhysicalModelR model, Dgn::DgnCategoryId categoryId);
        static  BentleyStatus CreateFrameGeometry             (Dgn::GeometryBuilderPtr builder, BuildingPhysical::BuildingPhysicalModelR model, double frameDepth, double frameWidth, double height, double width, bool fullFrame);
		static  BentleyStatus CreatePidLineGeometry           (Dgn::DrawingGraphicR element, Dgn::DrawingModelR drawingModel, DPoint2dCP points, int count, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidValveGeometry          (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidPumpGeometry           (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidTankGeometry           (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidNozzleGeometry         (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
        static  BentleyStatus CreatePidVirtualNozzleGeometry  (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
        static  BentleyStatus CreatePidReducerGeometry        (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePid3WayValveGeometry      (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidRoundTankGeometry      (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
		static  BentleyStatus CreatePidVesselGeometry         (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId);
        static  BentleyStatus CreateAnnotationTextGeometry    (Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Utf8StringCR textValue, Dgn::DgnSubCategoryId subCategoryId);

        static  BentleyStatus Create3dPipeGeometry            (Dgn::PhysicalElementR element, Dgn::DgnCategoryId categoryId, double length, double diameter);

        static  ICurvePrimitivePtr CreateContainmentBuildingGeometry (/*Dgn::DgnCategoryId categoryId,*/ double radius, double height);


    };

