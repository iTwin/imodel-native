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
//! Base class for RadialDistortion 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct RadialDistortion : Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BDCP_SCHEMA_NAME,BDCP_CLASS_RadialDistortion, Dgn::DgnElement::UniqueAspect)

private:
    double m_k1;
    double m_k2;
    double m_k3;

protected:
    //! Constructor
    RadialDistortion(double k1, double k2, double k3) :m_k1(k1), m_k2(k2), m_k3(k3) {}

    //! Copy constructor
    RadialDistortion(RadialDistortionCR rhs);

    //! Empty constructor 
    RadialDistortion() :m_k1(0), m_k2(0), m_k3(0) {}

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;


public:
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(RadialDistortion)

    //! Create a new RadialDistortion 
    DATACAPTURE_EXPORT static RadialDistortionPtr Create();

    //! Create a new RadialDistortion 
    DATACAPTURE_EXPORT static RadialDistortionPtr Create(double k1, double k2, double k3);

    DATACAPTURE_EXPORT RadialDistortionPtr Clone() const;

    //! Assignment operator
    DATACAPTURE_EXPORT RadialDistortion& operator= (RadialDistortionCR rhs);

    //! Validates 
    DATACAPTURE_EXPORT bool IsValid() const;
    DATACAPTURE_EXPORT bool IsEqual(RadialDistortionCR rhs) const;

    DATACAPTURE_EXPORT double GetK1() const;
    DATACAPTURE_EXPORT double GetK2() const;
    DATACAPTURE_EXPORT double GetK3() const;
    DATACAPTURE_EXPORT void   SetK1(double val);
    DATACAPTURE_EXPORT void   SetK2(double val);
    DATACAPTURE_EXPORT void   SetK3(double val);

    };
//=======================================================================================
//! Base class for TangentialDistortion 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct TangentialDistortion: Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BDCP_SCHEMA_NAME, BDCP_CLASS_TangentialDistortion, Dgn::DgnElement::UniqueAspect)

private:
    double m_p1;
    double m_p2;

protected:
    //! Constructor
    TangentialDistortion(double p1, double p2) :m_p1(p1), m_p2(p2) {}

    //! Empty constructor 
    TangentialDistortion() :m_p1(0), m_p2(0) {}

    //! Copy constructor
    TangentialDistortion(TangentialDistortionCR rhs);

    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

public:
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(TangentialDistortion)

    //! Create a new TangentialDistortion 
    DATACAPTURE_EXPORT static TangentialDistortionPtr Create();

    //! Create a new TangentialDistortion 
    DATACAPTURE_EXPORT static TangentialDistortionPtr Create(double p1, double p2);

    DATACAPTURE_EXPORT TangentialDistortionPtr Clone() const;

    //! Assignment operator
    DATACAPTURE_EXPORT TangentialDistortion& operator= (TangentialDistortionCR rhs);

    //! Validates 
    DATACAPTURE_EXPORT bool IsValid() const;
    DATACAPTURE_EXPORT bool IsEqual(TangentialDistortionCR rhs) const;


    DATACAPTURE_EXPORT double GetP1() const;
    DATACAPTURE_EXPORT double GetP2() const;
    DATACAPTURE_EXPORT void   SetP1(double val);
    DATACAPTURE_EXPORT void   SetP2(double val);
    };

//=======================================================================================
//! Base class for Camera 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraType : Dgn::DefinitionElement
    {
    friend struct CameraTypeHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_CameraType, Dgn::DefinitionElement)

    public:
        //! Entry in Camera iterator
        struct CameraEntry : Dgn::ECSqlStatementEntry
            {
            friend struct Dgn::ECSqlStatementIterator < CameraType::CameraEntry >;
            friend struct CameraType;
            private:
                CameraEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
            public:
                CameraElementId GeCameraElementId() const { return m_statement->GetValueId<CameraElementId>(0); }
            };

        //! Iterator over Camera
        struct CameraIterator : Dgn::ECSqlStatementIterator < CameraType::CameraEntry >
            {
            };


    private:
        double                  m_focalLength;
        int                     m_imageWidth;
        int                     m_imageHeight;
        DPoint2d                m_principalPoint;
        double                  m_aspectRatio;
        double                  m_skew;


        Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

    protected:

        explicit CameraType(CreateParams const& params) : T_Super(params) {}


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


        //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
        //! @note If you override this method, you @em must call T_Super::_OnDeleted.
        virtual Dgn::DgnDbStatus _OnDelete() const override;


    public:
        DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(CameraType)
        DECLARE_DATACAPTURE_QUERYCLASS_METHODS(CameraType)

        //! Create a new Camera 
        DATACAPTURE_EXPORT static CameraTypePtr Create(Dgn::DefinitionModelR model);

        //! Query for an Camera (Id) by label
        //! @return Id of the Camera or invalid Id if an Camera was not found
        DATACAPTURE_EXPORT static CameraTypeElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

        //! Make an iterator over all camera-s relevant to a CameraType
        DATACAPTURE_EXPORT static CameraType::CameraIterator MakeCameraIterator(Dgn::DgnDbCR dgndb, CameraTypeElementId cameraTypeId);


        //! Get the id of this Camera
        DATACAPTURE_EXPORT CameraTypeElementId GetId() const;

        //Properties Get/Set
        DATACAPTURE_EXPORT int                      GetImageWidth() const;
        DATACAPTURE_EXPORT void                     SetImageWidth(int val);
        DATACAPTURE_EXPORT int                      GetImageHeight() const;
        DATACAPTURE_EXPORT void                     SetImageHeight(int val);
        DATACAPTURE_EXPORT double                   GetFocalLength() const;
        DATACAPTURE_EXPORT void                     SetFocalLength(double val);
        DATACAPTURE_EXPORT void                     SetPrincipalPoint(DPoint2dCR val);
        DATACAPTURE_EXPORT DPoint2d                 GetPrincipalPoint() const;
        DATACAPTURE_EXPORT double                   GetAspectRatio() const;
        DATACAPTURE_EXPORT void                     SetAspectRatio(double val);
        DATACAPTURE_EXPORT double                   GetSkew() const;
        DATACAPTURE_EXPORT void                     SetSkew(double val);
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
    mutable CameraTypeElementId m_cameraType;//Query and cached from DgnDb or given at creation time
    double                  m_focalLength;
    DPoint2d                m_principalPoint;
    int                     m_imageWidth;
    int                     m_imageHeight;
    double                  m_aspectRatio;
    double                  m_skew;


    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit Camera(CreateParams const& params, CameraTypeElementId cameraType=CameraTypeElementId()) : T_Super(params), m_cameraType(cameraType) {}

    static BentleyStatus InsertCameraIsDefinedByCameraTypeRelationship(Dgn::DgnDbR dgndb, CameraElementId cameraElmId, CameraTypeElementId cameraTypeElmId);
    static CameraTypeElementId QueryCameraIsDefinedByCameraTypeRelationship(Dgn::DgnDbR dgndb, CameraElementId cameraElmId);

    void InsertCameraIsDefinedByCameraTypeRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateCameraIsDefinedByCameraTypeRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteCameraIsDefinedByCameraTypeRelationship(Dgn::DgnDbR dgndb) const;


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

    virtual Dgn::DgnDbStatus _OnInsert() override;

    //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
    //! @note If you override this method, you @em must call T_Super::_OnDeleted.
    virtual Dgn::DgnDbStatus _OnDelete() const override;

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
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(Camera)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(Camera)

    //! Create a new Camera 
    DATACAPTURE_EXPORT static CameraPtr Create(Dgn::SpatialModelR model, CameraTypeElementId camera);

    //! Query for an Camera (Id) by label
    //! @return Id of the Camera or invalid Id if an Camera was not found
    DATACAPTURE_EXPORT static CameraElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    //! Make an iterator over all Photo-s relevant to a Camera
    DATACAPTURE_EXPORT static Camera::PhotoIterator MakePhotoIterator(Dgn::DgnDbCR dgndb, CameraElementId cameraId);


    //! Get the id of this Camera
    DATACAPTURE_EXPORT CameraElementId GetId() const;

    //Properties Get/Set
    DATACAPTURE_EXPORT int                      GetImageWidth() const; 
    DATACAPTURE_EXPORT void                     SetImageWidth(int val);
    DATACAPTURE_EXPORT int                      GetImageHeight() const;
    DATACAPTURE_EXPORT void                     SetImageHeight(int val);
    DATACAPTURE_EXPORT double                   GetFocalLength() const;
    DATACAPTURE_EXPORT void                     SetFocalLength(double val);
    DATACAPTURE_EXPORT void                     SetPrincipalPoint(DPoint2dCR val);
    DATACAPTURE_EXPORT DPoint2d                 GetPrincipalPoint() const;
    DATACAPTURE_EXPORT RadialDistortionCP       GetRadialDistortion() const;
    DATACAPTURE_EXPORT RadialDistortionP        GetRadialDistortionP();
    DATACAPTURE_EXPORT void                     SetRadialDistortion(RadialDistortionP value);
    DATACAPTURE_EXPORT TangentialDistortionCP   GetTangentialDistortion() const;
    DATACAPTURE_EXPORT TangentialDistortionP    GetTangentialDistortionP();
    DATACAPTURE_EXPORT void                     SetTangentialDistortion(TangentialDistortionP value);
    DATACAPTURE_EXPORT double                   GetAspectRatio() const;
    DATACAPTURE_EXPORT void                     SetAspectRatio(double val);
    DATACAPTURE_EXPORT double                   GetSkew() const;
    DATACAPTURE_EXPORT void                     SetSkew(double val);

    DATACAPTURE_EXPORT CameraTypeElementId  GetCameraTypeId() const;
    DATACAPTURE_EXPORT void                 SetCameraTypeId(CameraTypeElementId val);
  
};

//=================================================================================
//! Handler for RadialDistortion
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RadialDistortionHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(BDCP_CLASS_RadialDistortion, RadialDistortionHandler, Dgn::dgn_AspectHandler::Aspect, DATACAPTURE_EXPORT)

    protected:
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };
//=================================================================================
//! Handler for RadialDistortion
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TangentialDistortionHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(BDCP_CLASS_TangentialDistortion, TangentialDistortionHandler, Dgn::dgn_AspectHandler::Aspect, DATACAPTURE_EXPORT)

    protected:
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

//=======================================================================================
//! Singleton CameraTypeHandler for the Cameratype class
//=======================================================================================
struct CameraTypeHandler : Dgn::dgn_ElementHandler::Element
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_CameraType, CameraType, CameraTypeHandler, Dgn::dgn_ElementHandler::Element, DATACAPTURE_EXPORT)
    protected:
        virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
    }; // CameraTypeHandler

//=================================================================================
//! ElementHandler for Camera-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_Camera, Camera, CameraHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

