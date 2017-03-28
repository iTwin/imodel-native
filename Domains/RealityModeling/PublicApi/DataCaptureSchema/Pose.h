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
    DPoint3d            m_center;
    RotMatrix           m_rotation;
    bool                m_isECEFSupported;

    //Values below are computed from value above
    DPoint3d            m_centerLocal;
    RotMatrix           m_rotationLocal;
    Angle               m_omega;
    Angle               m_phi;
    Angle               m_kappa;

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

    //! Yaw Pitch Roll angles can be define differently according to convention
    enum class YPRConvention
        {
        // DgnDb convention
        //  The coordinate system orientation has X forward, Y to the left, and Z up.
        //  If only one is nonzero, the meanings are:
        //  
        //  YAW is rotation around Z (X towards Y) (turn left)
        //  PITCH is rotation around negative Y (X towards Z)  (nose up)
        //  ROLL is rotation around X (Y towards Z)    (tip right)
        //
        DgnDb = 0, //East Forward 
          
        // 0,0,0 = looking horizontally in y direction, z=top, x=right (If georeferenced XYZ=ENU, this means looking north horizontally)
        // Order: 
        // 1) turn right yaw radians (z-)
        // 2) turn up pitch radians (x+)
        // 3) tilt roll radians counter clockwise (y-) (NB: might be the opposite, waiting for a non zero roll to check!)
        ENU_NorthForward,
        };

    //! Create a new Shot 
    //! @param[in] IsECEFSupported Set to true if you have a GCS and want to support storing data in 
    //                             geocentric (Earth Centered Earth Fixed) coordinate system. 
    DATACAPTURE_EXPORT static PosePtr Create(Dgn::SpatialModelR model, bool IsECEFSupported);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

    //! Validates 
    DATACAPTURE_EXPORT bool IsEqual(PoseCR rhs) const;


    DATACAPTURE_EXPORT static bool          GetRotationFromRotMatrix(AngleR omega, AngleR phi, AngleR kappa, RotMatrixCR rotation);
    DATACAPTURE_EXPORT static void          FrustumCornersFromCameraPose(DPoint3dP points, PoseCR pose, DPoint2dCR fieldofView, DPoint3dCR target, 
                                            double scaleFactor=1.0, bool calculateAtOrigin=false);

    //Position and orientation in ECEF system OR local if ECEF is not supported (IsECEF is false)
    DATACAPTURE_EXPORT bool                 IsECEF() const;
    DATACAPTURE_EXPORT void                 SetIsECEF(bool isECEF);
    DATACAPTURE_EXPORT DPoint3dCR           GetCenterECEF() const;
    DATACAPTURE_EXPORT void                 SetCenterECEF(DPoint3dCR val);
    DATACAPTURE_EXPORT RotMatrix            GetRotMatrixECEF() const;
    DATACAPTURE_EXPORT void                 SetRotMatrixECEF(RotMatrixCR rotation);

    //Position and orientation in current GCS system
    DATACAPTURE_EXPORT DPoint3dCR           GetCenter() const;
    DATACAPTURE_EXPORT void                 SetCenter(DPoint3dCR val);
    DATACAPTURE_EXPORT AngleCR              GetOmega() const;
    DATACAPTURE_EXPORT AngleCR              GetPhi() const;
    DATACAPTURE_EXPORT AngleCR              GetKappa() const;
    DATACAPTURE_EXPORT void                 SetOmega(AngleCR omega); 
    DATACAPTURE_EXPORT void                 SetPhi(AngleCR phi);
    DATACAPTURE_EXPORT void                 SetKappa(AngleCR kappa);
    DATACAPTURE_EXPORT RotMatrix            GetRotMatrix() const;
    DATACAPTURE_EXPORT void                 SetRotMatrix(RotMatrixCR rotation);
    DATACAPTURE_EXPORT YawPitchRollAngles   GetYawPitchRoll(YPRConvention convention=YPRConvention::DgnDb) const;
    DATACAPTURE_EXPORT void                 SetYawPitchRoll(YawPitchRollAnglesCR angles,YPRConvention convention=YPRConvention::DgnDb);
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

