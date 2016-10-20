/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/Camera.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//=======================================================================================
//! Base class for Camera 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Camera : Dgn::PhysicalElement
{
    friend struct CameraHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Camera, Dgn::PhysicalElement);

private:
    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Camera(CreateParams const& params) : T_Super(params) {}

    //! Called to bind the parameters when inserting a new Activity into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! And then you @em must call T_Super::_BindInsertParams, forwarding its status.
    DATACAPTURE_EXPORT virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;

    //! Called to update an Activity in the DgnDb with new values. Override to update subclass properties.
    //! @note If the update fails, the original data will be copied back into this Activity.
    //! @note If you override this method, you @em must call T_Super::_BindUpdateParams, forwarding its status.
    DATACAPTURE_EXPORT virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;

    DATACAPTURE_EXPORT virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    DATACAPTURE_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;

public:
    //! Create a new Camera 
    DATACAPTURE_EXPORT static CameraPtr Create(Dgn::SpatialModelR model);

    //! Insert the Camera in the DgnDb
    DATACAPTURE_EXPORT CameraId Insert();

    //! Get a read only copy of the Camera from the DgnDb
    //! @return Invalid if the Camera does not exist
    DATACAPTURE_EXPORT static CameraCPtr Get(Dgn::DgnDbCR dgnDb, Dgn::DgnElementId cameraId);

    //! Get an editable copy of the Camera from the DgnDb
    //! @return Invalid if the Camera does not exist, or if it cannot be edited.
    DATACAPTURE_EXPORT static CameraPtr GetForEdit(Dgn::DgnDbCR dgnDb, Dgn::DgnElementId cameraId);

    //! Update the persistent state of the Camera in the DgnDb from this modified copy of it.
    DATACAPTURE_EXPORT BentleyStatus Update();

    //! Query the DgnClassId of the planning.Camera ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the planning.Camera class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(BDCP_SCHEMA_NAME, BDCP_CLASS_Camera)); }

    //! Get the id of this WorkBreakdown
    CameraId GetId() const { return CameraId(GetElementId().GetValueUnchecked()); }

   
};

//=================================================================================
//! ElementHandler for Camera-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraHandler : Dgn::dgn_ElementHandler::Element
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Camera, Camera, CameraHandler, Dgn::dgn_ElementHandler::Element, DATACAPTURE_EXPORT)
protected: 
    DATACAPTURE_EXPORT virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

