/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"
#include "Pose.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE


//=======================================================================================
//! Base class for Shot 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Shot : Dgn::SpatialLocationElement
{
    friend struct ShotHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Shot, Dgn::SpatialLocationElement);

private:
    mutable CameraDeviceElementId m_cameraDevice;//Query and cached from DgnDb or given at creation time
    mutable PoseElementId         m_pose;//Query and cached from DgnDb or given at creation time

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    Shot(CreateParams const& params, CameraDeviceElementId cameraDevice=CameraDeviceElementId(), PoseElementId pose=PoseElementId());

    static BentleyStatus InsertShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb, ShotElementId shotElmId, CameraDeviceElementId cameraDeviceElmId);
    static BentleyStatus InsertShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb, ShotElementId shotElmId, PoseElementId poseElmId);
    static CameraDeviceElementId QueryShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb,  ShotElementId shotElmId);
    static PoseElementId QueryShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb, ShotElementId shotElmId);

    void InsertShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;
    void InsertShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteShotIsTakenAtPoseRelationship(Dgn::DgnDbR dgndb) const;

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    virtual void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

    //! Remap any IDs that might refer to elements or resources in the source DgnDb.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    virtual void _RemapIds(Dgn::DgnImportContext&) override;

    //! Called to bind the parameters when inserting a new Shot into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! And then you @em must call T_Super::_BindInsertParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;

    //! Called to update a shot in the DgnDb with new values. Override to update subclass properties.
    //! @note If the update fails, the original data will be copied back into this Activity.
    //! @note If you override this method, you @em must call T_Super::_BindUpdateParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;

    virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    virtual Dgn::DgnDbStatus _OnInsert() override;

    //! Called after a DgnElement was successfully inserted into the database.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    virtual void _OnInserted(Dgn::DgnElementP copiedFrom) const override;

    //! Called after a DgnElement was successfully updated. The element will be in its post-updated state.
    //! @note If you override this method, you @em must call T_Super::_OnUpdated.
    virtual void _OnUpdated(Dgn::DgnElementCR original) const override;

    //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
    //! @note If you override this method, you @em must call T_Super::_OnDeleted.
    virtual void _OnDeleted() const override;

    //! Called when an element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to control when/if their instances are deleted.
    //! @return DgnDbStatus::Success to allow the delete, otherwise the delete will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnDelete, forwarding its status.
    virtual Dgn::DgnDbStatus _OnDelete() const override;

    virtual Dgn::DgnCode _GenerateDefaultCode() const override;

public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Shot)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Shot)

    //! Create a new Shot 
    DATACAPTURE_EXPORT static ShotPtr Create(Dgn::SpatialModelR model, CameraDeviceElementId cameraDevice, PoseElementId pose);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR CameraDeviceValue, Utf8StringCR value);

    //! Query for an Shot (Id) by label
    //! @return Id of the Shot or invalid Id if an Shot was not found
    DATACAPTURE_EXPORT static ShotElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    DATACAPTURE_EXPORT static Dgn::ColorDef GetDefaultColor();
    DATACAPTURE_EXPORT static int           GetDefaultWeight();

    //! Get the id of this Shot element
    DATACAPTURE_EXPORT ShotElementId GetId() const;

    DATACAPTURE_EXPORT CameraDeviceElementId  GetCameraDeviceId() const;
    DATACAPTURE_EXPORT void             SetCameraDeviceId(CameraDeviceElementId val);
    DATACAPTURE_EXPORT PoseElementId    GetPoseId() const;
    DATACAPTURE_EXPORT void             SetPoseId(PoseElementId val);
};

//=================================================================================
//! ElementHandler for Shot-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ShotHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Shot, Shot, ShotHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

