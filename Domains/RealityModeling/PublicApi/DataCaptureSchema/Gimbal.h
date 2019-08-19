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
//! Base class for Gimbal 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Gimbal : Dgn::PhysicalElement
{
    friend struct GimbalHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Gimbal, Dgn::PhysicalElement);

public:
    //! Entry in GimbalAngleRange iterator
    struct GimbalAngleRangeEntry : Dgn::ECSqlStatementEntry
        {
            friend struct Dgn::ECSqlStatementIterator < Gimbal::GimbalAngleRangeEntry >;
            friend struct Gimbal;
        private:
            GimbalAngleRangeEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
        public:
            GimbalAngleRangeElementId GetGimbalAngleRangeElementId() const { return m_statement->GetValueId<GimbalAngleRangeElementId>(0); }
        };

    //! Iterator over timelines
    struct GimbalAngleRangeIterator : Dgn::ECSqlStatementIterator < Gimbal::GimbalAngleRangeEntry >
        {
        };

private:
    mutable DgnElementIdSet m_gimbalAngleRangeIdSet;                    //Query and cached from DgnDb or given at creation time
    mutable bool m_gimbalAngleRangeIdSetInit;
    mutable DgnElementIdSet m_cameraIdSet;                    //Query and cached from DgnDb or given at creation time
    mutable bool m_cameraIdSetInit;

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Gimbal(CreateParams const& params);
    explicit Gimbal(CreateParams const& params, DgnElementIdSet cameraIdSet, DgnElementIdSet gimbalAngleRangeIdSet);

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

    void InsertGimbalHasGimbalAngleRangesRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateGimbalHasGimbalAngleRangesRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteGimbalHasGimbalAngleRangesRelationship(Dgn::DgnDbR dgndb) const;
   
    void InsertGimbalHasCamerasRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateGimbalHasCamerasRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteGimbalHasCamerasRelationship(Dgn::DgnDbR dgndb) const;

    static DgnElementIdSet QueryGimbalHasGimbalAngleRangesRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId);
    static BentleyStatus InsertGimbalHasGimbalAngleRangesRelationship(Dgn::DgnDbR dgndb, GimbalElementId gimbalElmId, DgnElementIdSet gimbalAngleRangeIdSet);

    static DgnElementIdSet QueryGimbalHasCamerasRelationship(DgnDbR dgndb, GimbalElementId gimbalElmId);
    static BentleyStatus InsertGimbalHasCamerasRelationship(Dgn::DgnDbR dgndb, GimbalElementId gimbalElmId, DgnElementIdSet cameraIdSet);

public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Gimbal)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Gimbal)

    //! Create a new Gimbal 
    DATACAPTURE_EXPORT static GimbalPtr Create(Dgn::SpatialModelR model);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

    //! Get the id of this Gimbal element
    DATACAPTURE_EXPORT GimbalElementId GetId() const;

    //! Make an iterator over all GimbalAngleRange-s relevant to a PhotoPlan
    DATACAPTURE_EXPORT static Gimbal::GimbalAngleRangeIterator MakeGimbalAngleRangeIterator(Dgn::DgnDbCR dgndb, GimbalElementId gimbalZoneId);

    DATACAPTURE_EXPORT void SetGimbalAngleRangeElementIdSet(DgnElementIdSet gimablAngleRangeIdSet);

    DATACAPTURE_EXPORT DgnElementIdSet GetGimbalAngleRangeElementIdSet() const;

    DATACAPTURE_EXPORT BentleyStatus RemoveAllGimbalAngleRanges();

    DATACAPTURE_EXPORT void SetCameraElementIdSet(DgnElementIdSet cameraIdSet);

    DATACAPTURE_EXPORT DgnElementIdSet GetCameraElementIdSet() const;
};

//=================================================================================
//! ElementHandler for Gimbal-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GimbalHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Gimbal, Gimbal, GimbalHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

