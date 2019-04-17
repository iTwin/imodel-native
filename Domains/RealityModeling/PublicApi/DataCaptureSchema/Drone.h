/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE


//=======================================================================================
//! Base class for Drone 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Drone : Dgn::PhysicalElement
{
    friend struct DroneHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Drone, Dgn::PhysicalElement);

private:
    mutable GimbalElementId m_gimbalId;       //Query and cached from DgnDb or given at creation time

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Drone(CreateParams const& params);
    explicit Drone(CreateParams const& params, GimbalElementId m_gimbalId);

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    virtual void _CopyFrom(Dgn::DgnElementCR source) override;

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

    void InsertDroneHasGimbalRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateDroneHasGimbalRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteDroneHasGimbalRelationship(Dgn::DgnDbR dgndb) const;
   
    static GimbalElementId QueryDroneHasGimbalRelationship(DgnDbR dgndb, DroneElementId gimbalElmId);
    static BentleyStatus InsertDroneHasGimbalRelationship(Dgn::DgnDbR dgndb, DroneElementId droneElmId, GimbalElementId gimbalId);

public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Drone)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Drone)

    //! Create a new Gimbal 
    DATACAPTURE_EXPORT static DronePtr Create(Dgn::SpatialModelR model, GimbalElementId gimbalElmId);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

    //! Get the id of this Drone element
    DATACAPTURE_EXPORT DroneElementId GetId() const;

    DATACAPTURE_EXPORT void SetGimbalElementId(GimbalElementId gimbalId);

    DATACAPTURE_EXPORT GimbalElementId GetGimbalElementId() const;

    //! Query for an Drone (Id) by label
    //! @return Id of the Drone or invalid Id if an Drone was not found
    DATACAPTURE_EXPORT static DroneElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);
};

//=================================================================================
//! ElementHandler for Drone-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DroneHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Drone, Drone, DroneHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

