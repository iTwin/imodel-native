/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/Shot.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE


//=======================================================================================
//! Base class for RotationMatrixType 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct RotationMatrixType : public RotMatrix
    {
public:
    //! Constructor
    RotationMatrixType(RotMatrixCR in)
        {
        Copy(in);
        }

    //! Empty constructor 
    RotationMatrixType() {}

    //! Copy constructor
    RotationMatrixType(RotationMatrixTypeCR rhs) { *this = rhs; }

    //! Assignment operator
    RotationMatrixType& operator= (RotationMatrixTypeCR rhs)
        {
        Copy(rhs);
        return *this;
        }

    RotationMatrixType& operator= (RotMatrixCR rhs)
        {
        Copy(rhs);
        return *this;
        }

    //! Bind the otationMatrixType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::IECSqlStructBinder& binder, RotationMatrixTypeCR val);

    //! Get the otationMatrixType value at the specified column from a ECSQL statement
    static RotationMatrixType GetValue(BeSQLite::EC::IECSqlStructValue const& structValue);
    };

//=======================================================================================
//! Base class for PoseType 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct PoseType
    {
private:
    DPoint3d            m_center;
    RotationMatrixType  m_rotation;

public:
    //! Constructor
    DATACAPTURE_EXPORT PoseType(DPoint3dCR center, RotationMatrixTypeCR rotation);

    //! Empty constructor (creates an invalid PoseType)
    DATACAPTURE_EXPORT PoseType();

    //! Copy constructor
    DATACAPTURE_EXPORT PoseType(PoseTypeCR rhs);

    //! Assignment operator
    DATACAPTURE_EXPORT PoseType& operator= (PoseTypeCR rhs);

    //! Validates 
    DATACAPTURE_EXPORT bool IsEqual(PoseTypeCR rhs) const;

    DATACAPTURE_EXPORT DPoint3d            GetCenter() const;
    DATACAPTURE_EXPORT RotationMatrixType  GetRotation() const;
    DATACAPTURE_EXPORT void SetCenter(DPoint3dCR val);
    DATACAPTURE_EXPORT void SetRotation(RotationMatrixTypeCR val);

    //! Bind the PoseType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::IECSqlStructBinder& binder, PoseTypeCR val);

    //! Get the PoseType value at the specified column from a ECSQL statement
    static PoseType GetValue(BeSQLite::EC::IECSqlStructValue const& structValue);
    };


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
    int                  m_shotId;
    PoseType             m_pose;

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Shot(CreateParams const& params, CameraDeviceElementId cameraDevice=CameraDeviceElementId()) : T_Super(params), m_cameraDevice(cameraDevice) {}

    static BentleyStatus InsertShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb, ShotElementId shotElmId, CameraDeviceElementId cameraDeviceElmId);
    static CameraDeviceElementId QueryShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb,  ShotElementId shotElmId);

    void InsertShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteShotIsTakenByCameraDeviceRelationship(Dgn::DgnDbR dgndb) const;

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


public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Shot)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Shot)

    //! Create a new Shot 
    DATACAPTURE_EXPORT static ShotPtr Create(Dgn::SpatialModelR model, CameraDeviceElementId cameraDevice);

    //! Query for an Shot (Id) by label
    //! @return Id of the Shot or invalid Id if an Shot was not found
    DATACAPTURE_EXPORT static ShotElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    //! Get the id of this Shot element
    DATACAPTURE_EXPORT ShotElementId GetId() const;

    DATACAPTURE_EXPORT int              GetShotId() const;
    DATACAPTURE_EXPORT PoseType         GetPose() const;
    DATACAPTURE_EXPORT void             SetShotId(int val);
    DATACAPTURE_EXPORT void             SetPose(PoseTypeCR val);

    DATACAPTURE_EXPORT CameraDeviceElementId  GetCameraDeviceId() const;
    DATACAPTURE_EXPORT void             SetCameraDeviceId(CameraDeviceElementId val);

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

