/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/Photo.h $
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


    //! Bind the ImageDimensionType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, RotationMatrixTypeCR val);

    //! Get the ImageDimensionType value at the specified column from a ECSQL statement
    static RotationMatrixType GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex);
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
    PoseType(DPoint3dCR center, RotationMatrixTypeCR rotation) :m_center(center), m_rotation(rotation) {}

    //! Empty constructor (creates an invalid ImageDimensionType)
    PoseType() {}

    //! Copy constructor
    PoseType(PoseTypeCR rhs) { *this = rhs; }

    //! Assignment operator
    PoseType& operator= (PoseTypeCR rhs)
        {
        m_center = rhs.m_center;
        m_rotation = rhs.m_rotation;
        return *this;
        }


    //! Validates 
    bool IsEqual(PoseTypeCR rhs) const
        {
        if (m_rotation.IsEqual(rhs.m_rotation) && m_center.IsEqual(rhs.m_center))
            return true;
        return false;
        }

    DPoint3d            GetCenter() const   { return m_center; }
    RotationMatrixType  GetRotation() const { return m_rotation; }
    void SetCenter(DPoint3dCR val)          { m_center = val; }
    void SetRotation(RotationMatrixTypeCR val) { m_rotation = val; }

    //! Bind the ImageDimensionType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, PoseTypeCR val);

    //! Get the ImageDimensionType value at the specified column from a ECSQL statement
    static PoseType GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex);
    };


//=======================================================================================
//! Base class for Photo 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Photo : Dgn::SpatialLocationElement
{
    friend struct PhotoHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Photo, Dgn::SpatialLocationElement);

private:
    int                  m_photoId;
    PoseType             m_pose;

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Photo(CreateParams const& params) : T_Super(params) {}

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    virtual void _CopyFrom(Dgn::DgnElementCR source) override;

    //! Called to bind the parameters when inserting a new Activity into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! And then you @em must call T_Super::_BindInsertParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;

    //! Called to update an Activity in the DgnDb with new values. Override to update subclass properties.
    //! @note If the update fails, the original data will be copied back into this Activity.
    //! @note If you override this method, you @em must call T_Super::_BindUpdateParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;

    virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    virtual Dgn::DgnDbStatus _OnInsert() override;

public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Photo)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Photo)

    //! Create a new Photo 
    DATACAPTURE_EXPORT static PhotoPtr Create(Dgn::SpatialModelR model);

    //! Query for an Photo (Id) by label
    //! @return Id of the Photo or invalid Id if an Photo was not found
    DATACAPTURE_EXPORT static PhotoElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    //! Get the id of this Photo
    PhotoElementId GetId() const { return PhotoElementId(GetElementId().GetValueUnchecked()); }

    int         GetPhotoId() const  { return m_photoId; }
    PoseType    GetPose() const     { return m_pose; }
    void        SetPhotoId(int val)  { m_photoId = val; }
    void        SetPose(PoseTypeCR val)  { m_pose = val; }
   
};

//=================================================================================
//! ElementHandler for Photo-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhotoHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Photo, Photo, PhotoHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

