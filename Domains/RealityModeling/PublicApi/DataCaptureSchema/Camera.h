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
//! Base class for ImageDimensionType 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct ImageDimensionType
    {
private:
    int  m_width;
    int  m_height;

public:
    //! Constructor
    ImageDimensionType(int width, int height):m_width(width),m_height(height) {}

    //! Empty constructor (creates an invalid ImageDimensionType)
    ImageDimensionType() : m_width(0), m_height(0) {}

    //! Copy constructor
    ImageDimensionType(ImageDimensionTypeCR rhs) { *this = rhs; }

    //! Assignment operator
    ImageDimensionType& operator= (ImageDimensionTypeCR rhs)
        {
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        return *this;
        }


    //! Validates 
    bool IsValid() const { return true; }
    bool IsEqual(ImageDimensionTypeCR rhs) const
        {
        if (m_width==rhs.m_width && m_height==rhs.m_height)
            return true;
        return false;
        }

    int GetWidth() const     {return m_width;}
    int GetHeight() const    {return m_height;}
    void SetWidth(int val)   {m_width=val;}
    void SetHeight(int val)  {m_height=val;}

    //! Bind the ImageDimensionType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, ImageDimensionTypeCR val);

    //! Get the ImageDimensionType value at the specified column from a ECSQL statement
    static ImageDimensionType GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex);
    };

//=======================================================================================
//! Base class for CameraDistortionType 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct CameraDistortionType
    {
private:
    double m_k1;
    double m_k2;
    double m_k3;
    double m_p1;
    double m_p2;

public:
    //! Constructor
    CameraDistortionType(double k1, double k2, double k3, double p1, double p2):m_k1(k1),m_k2(k2),m_k3(k3),m_p1(p1),m_p2(p2) {}

    //! Empty constructor 
    CameraDistortionType():m_k1(0),m_k2(0),m_k3(0),m_p1(0),m_p2(0) {}

    //! Copy constructor
    CameraDistortionType(CameraDistortionTypeCR rhs) { *this = rhs; }

    //! Assignment operator
    CameraDistortionType& operator= (CameraDistortionTypeCR rhs)
        {
        m_k1 = rhs.m_k1;
        m_k2 = rhs.m_k2;
        m_k3 = rhs.m_k3;
        m_p1 = rhs.m_p1;
        m_p2 = rhs.m_p2;
        return *this;
        }

    //! Validates 
    bool IsValid() const { return true; }
    bool IsEqual(CameraDistortionTypeCR rhs) const
        {
        if (m_k1 == rhs.m_k1 && m_k2 == rhs.m_k2 && m_k3 == rhs.m_k3 && m_p1 == rhs.m_p1 && m_p2 == rhs.m_p2)
            return true;
        return false;
        }


    double GetK1() const { return m_k1; }
    double GetK2() const { return m_k2; }
    double GetK3() const { return m_k3; }
    double GetP1() const { return m_p1; }
    double GetP2() const { return m_p2; }
    void   SetK1(double val) { m_k1 = val; }
    void   SetK2(double val) { m_k2 = val; }
    void   SetK3(double val) { m_k3 = val; }
    void   SetP1(double val) { m_p1=val; }
    void   SetP2(double val) { m_p2=val; }

    //! Bind the ImageDimensionType field in a ECSQL statement
    static BeSQLite::EC::ECSqlStatus BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, CameraDistortionTypeCR val);

    //! Get the ImageDimensionType value at the specified column from a ECSQL statement
    static CameraDistortionType GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex);
    };

//=======================================================================================
//! Base class for Camera 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Camera : Dgn::PhysicalElement
{
    friend struct CameraHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_Camera, Dgn::PhysicalElement)

public:
    //! Entry in Photo iterator
    struct PhotoEntry : Dgn::ECSqlStatementEntry
    {
        friend struct Dgn::ECSqlStatementIterator < Camera::PhotoEntry >;
        friend struct Camera;
    private:
        PhotoEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
    public:
        PhotoElementId GePhotoElementId() const { return m_statement->GetValueId<PhotoElementId>(0); }
    };

    //! Iterator over timelines
    struct PhotoIterator : Dgn::ECSqlStatementIterator < Camera::PhotoEntry >
    {
    };


private:
    double                  m_focalLenghtPixels;
    ImageDimensionType      m_imageDimension;
    DPoint2d                m_principalPoint;
    CameraDistortionType    m_Distortion;
    double                  m_aspectRatio;
    double                  m_skew;


    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Camera(CreateParams const& params) : T_Super(params) {}


    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    virtual void _CopyFrom(Dgn::DgnElementCR source) override;

#ifdef WIP_MERGE_Donald
    //! Called to bind the parameters when inserting a new Activity into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! And then you @em must call T_Super::_BindInsertParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;

    //! Called to update an Activity in the DgnDb with new values. Override to update subclass properties.
    //! @note If the update fails, the original data will be copied back into this Activity.
    //! @note If you override this method, you @em must call T_Super::_BindUpdateParams, forwarding its status.
    virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
#endif

    virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;


    //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
    //! @note If you override this method, you @em must call T_Super::_OnDeleted.
    virtual Dgn::DgnDbStatus _OnDelete() const override;


public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Camera)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Camera)

    //! Create a new Camera 
    DATACAPTURE_EXPORT static CameraPtr Create(Dgn::SpatialModelR model);

    //! Query for an Camera (Id) by label
    //! @return Id of the Camera or invalid Id if an Camera was not found
    DATACAPTURE_EXPORT static CameraElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    //! Make an iterator over all Photo-s relevant to a Camera
    DATACAPTURE_EXPORT static Camera::PhotoIterator MakePhotoIterator(Dgn::DgnDbCR dgndb, CameraElementId cameraId);


    //! Get the id of this Camera
    DATACAPTURE_EXPORT CameraElementId GetId() const;

    //Properties Get/Set
    DATACAPTURE_EXPORT double                  GetFocalLenghtPixels() const;
    DATACAPTURE_EXPORT ImageDimensionType      GetImageDimension() const;
    DATACAPTURE_EXPORT DPoint2d                GetPrincipalPoint() const;
    DATACAPTURE_EXPORT CameraDistortionType    GetDistortion() const;
    DATACAPTURE_EXPORT double                  GetAspectRatio() const;
    DATACAPTURE_EXPORT double                  GetSkew() const;
    DATACAPTURE_EXPORT void                    SetFocalLenghtPixels(double val);
    DATACAPTURE_EXPORT void                    SetImageDimension(ImageDimensionTypeCR val);
    DATACAPTURE_EXPORT void                    SetPrincipalPoint(DPoint2dCR val);
    DATACAPTURE_EXPORT void                    SetDistortion(CameraDistortionTypeCR val);
    DATACAPTURE_EXPORT void                    SetAspectRatio(double val);
    DATACAPTURE_EXPORT void                    SetSkew(double val);
   
};

//=================================================================================
//! ElementHandler for Camera-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Camera, Camera, CameraHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
#ifdef WIP_MERGE_Donald
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
#endif
};

END_BENTLEY_DATACAPTURE_NAMESPACE

