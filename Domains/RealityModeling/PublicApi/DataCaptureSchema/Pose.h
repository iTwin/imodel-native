/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/Pose.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE



//=======================================================================================
//! Base class for Pose 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Pose  : Dgn::SpatialLocationElement
    {
    friend struct PoseHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Pose, Dgn::SpatialLocationElement);

private:
    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DPoint3d            m_centerECEF;
    RotMatrix           m_rotationECEF;
    DPoint3d            m_centerLocal;
    RotMatrix           m_rotationLocal;
    bool                m_isECEFSupported;
    Angle m_omega;
    Angle m_phi;
    Angle m_kappa;

    explicit Pose(CreateParams const& params);

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

    virtual Dgn::DgnCode _GenerateDefaultCode() const override;

public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Pose)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Pose)

    //! Create a new Shot 
    DATACAPTURE_EXPORT static PosePtr Create(Dgn::SpatialModelR model);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

    //! Validates 
    DATACAPTURE_EXPORT bool IsEqual(PoseCR rhs) const;


    DATACAPTURE_EXPORT static bool          GetRotationFromRotMatrix(AngleR omega, AngleR phi, AngleR kappa, RotMatrixCR rotation);
    DATACAPTURE_EXPORT static void          FrustumCornersFromCameraPose(DPoint3dP points, PoseCR pose, DPoint2dCR fieldofView, DPoint3dCR target);


    //Position and orientation in current GCS system
    DATACAPTURE_EXPORT bool                 IsECEF() const;
    DATACAPTURE_EXPORT void                 SetIsECEF(bool isECEF);
    DATACAPTURE_EXPORT DPoint3dCR           GetCenter() const;
    DATACAPTURE_EXPORT void                 SetCenter(DPoint3dCR val, bool synchECEF=true);
    DATACAPTURE_EXPORT AngleCR              GetOmega() const;
    DATACAPTURE_EXPORT AngleCR              GetPhi() const;
    DATACAPTURE_EXPORT AngleCR              GetKappa() const;
    DATACAPTURE_EXPORT void                 SetOmega(AngleCR omega,bool synchECEF=true, bool synchRotMatrix=true); 
    DATACAPTURE_EXPORT void                 SetPhi(AngleCR phi, bool synchECEF=true, bool synchRotMatrix=true);
    DATACAPTURE_EXPORT void                 SetKappa(AngleCR kappa, bool synchECEF=true, bool synchRotMatrix=true);
    DATACAPTURE_EXPORT RotMatrix            GetRotMatrix() const;
    DATACAPTURE_EXPORT void                 SetRotMatrix(RotMatrixCR rotation, bool synchECEF=true);
    DATACAPTURE_EXPORT YawPitchRollAngles   GetYawPitchRoll() const;
    DATACAPTURE_EXPORT void                 SetYawPitchRoll(YawPitchRollAnglesCR angles, bool synchECEF=true);

    //Position and orientation in ECEF system
    DATACAPTURE_EXPORT DPoint3dCR           GetCenterECEF() const;
    DATACAPTURE_EXPORT void                 SetCenterECEF(DPoint3dCR val, bool synchLocal=true);
    DATACAPTURE_EXPORT RotMatrix            GetRotMatrixECEF() const;
    DATACAPTURE_EXPORT void                 SetRotMatrixECEF(RotMatrixCR rotation,bool synchLocal=true);

    DATACAPTURE_EXPORT GeoPoint             GetCenterAsLatLongValue() const;
    DATACAPTURE_EXPORT void                 SetCenterFromLatLongValue(GeoPointCR geoPoint);
    //! Get the id of this Shot element
    DATACAPTURE_EXPORT PoseElementId GetId() const;
    };



//=================================================================================
//! ElementHandler for Pose-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PoseHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Pose, Pose, PoseHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

