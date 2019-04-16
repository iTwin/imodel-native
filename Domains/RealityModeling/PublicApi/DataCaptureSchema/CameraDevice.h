/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
//! Base class for CameraDevice 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraDeviceModel : Dgn::DefinitionElement
    {
    friend struct CameraDeviceModelHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_CameraDeviceModel, Dgn::DefinitionElement)

    public:
        //! Entry in CameraDevice iterator
        struct CameraDeviceEntry : Dgn::ECSqlStatementEntry
            {
            friend struct Dgn::ECSqlStatementIterator < CameraDeviceModel::CameraDeviceEntry >;
            friend struct CameraDeviceModel;
            private:
                CameraDeviceEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
            public:
                CameraDeviceElementId GeCameraDeviceElementId() const { return m_statement->GetValueId<CameraDeviceElementId>(0); }
            };

        //! Iterator over CameraDevice
        struct CameraDeviceIterator : Dgn::ECSqlStatementIterator < CameraDeviceModel::CameraDeviceEntry >
            {
            };

        enum class ModelType
            {
            Perspective = 0,
            Fisheye = 1,
            };

    private:
        double                  m_focalLength; //Always in meters
        int                     m_imageWidth;
        int                     m_imageHeight;
        DPoint2d                m_principalPoint;   //Always in meters
        double                  m_aspectRatio;
        double                  m_skew;
        ModelType               m_modelType;
        double                  m_sensorSize;

        Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

    protected:

        explicit CameraDeviceModel(CreateParams const& params) : T_Super(params) 
            {
            m_focalLength = 0.0;
            m_imageWidth = 0;
            m_imageHeight = 0;
            m_principalPoint = {0.0, 0.0};
            m_aspectRatio = 0.0;
            m_skew = 0.0;
            m_modelType = ModelType::Perspective;
            m_sensorSize = 0.0;
            }


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

        virtual Dgn::DgnCode _GenerateDefaultCode() const override;


    public:
        DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(CameraDeviceModel)
        DECLARE_DATACAPTURE_QUERYCLASS_METHODS(CameraDeviceModel)

        //! Create a new CameraDevice 
        DATACAPTURE_EXPORT static CameraDeviceModelPtr Create(Dgn::DefinitionModelR model);

        //! Query for an CameraDevice (Id) by label
        //! @return Id of the CameraDevice or invalid Id if an CameraDevice was not found
        DATACAPTURE_EXPORT static CameraDeviceModelElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

        //! Make an iterator over all cameraDevice-s relevant to a CameraDeviceModel
        DATACAPTURE_EXPORT static CameraDeviceModel::CameraDeviceIterator MakeCameraDeviceIterator(Dgn::DgnDbCR dgndb, CameraDeviceModelElementId cameraDeviceModelId);

        DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

        //! Get the id of this CameraDevice
        DATACAPTURE_EXPORT CameraDeviceModelElementId GetId() const;

        //Properties Get/Set
        DATACAPTURE_EXPORT int                      GetImageWidth() const;
        DATACAPTURE_EXPORT void                     SetImageWidth(int val);
        DATACAPTURE_EXPORT int                      GetImageHeight() const;
        DATACAPTURE_EXPORT void                     SetImageHeight(int val);
        //Focal always set/get in meters
        DATACAPTURE_EXPORT double                   GetFocalLength() const;
        //Focal always set/get in meters
        DATACAPTURE_EXPORT void                     SetFocalLength(double val);
        DATACAPTURE_EXPORT void                     SetPrincipalPoint(DPoint2dCR val);
        DATACAPTURE_EXPORT DPoint2d                 GetPrincipalPoint() const;
        DATACAPTURE_EXPORT double                   GetAspectRatio() const;
        DATACAPTURE_EXPORT void                     SetAspectRatio(double val);
        DATACAPTURE_EXPORT double                   GetSkew() const;
        DATACAPTURE_EXPORT void                     SetSkew(double val);
        DATACAPTURE_EXPORT ModelType                GetModelType() const;
        DATACAPTURE_EXPORT void                     SetModelType(ModelType val);
        DATACAPTURE_EXPORT double                   GetSensorSize() const;
        DATACAPTURE_EXPORT void                     SetSensorSize(double val);
    };

//=======================================================================================
//! Base class for CameraDevice 
//! @ingroup DataCaptureGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraDevice : Dgn::PhysicalElement
{
    friend struct CameraDeviceHandler;
    DGNELEMENT_DECLARE_MEMBERS(BDCP_CLASS_CameraDevice, Dgn::PhysicalElement)

public:
    //! Entry in Shot iterator
    struct ShotEntry : Dgn::ECSqlStatementEntry
    {
        friend struct Dgn::ECSqlStatementIterator < CameraDevice::ShotEntry >;
        friend struct CameraDevice;
    private:
        ShotEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
    public:
        ShotElementId GeShotElementId() const { return m_statement->GetValueId<ShotElementId>(0); }
    };

    //! Iterator over timelines
    struct ShotIterator : Dgn::ECSqlStatementIterator < CameraDevice::ShotEntry >
    {
    };

private:
    mutable CameraDeviceModelElementId m_cameraDeviceModel;//Query and cached from DgnDb or given at creation time
    double                  m_focalLength;    //Always in meters
    DPoint2d                m_principalPoint; //Always in meters
    int                     m_imageWidth;
    int                     m_imageHeight;
    double                  m_aspectRatio;
    double                  m_skew;
    //double                  m_pixelToMeterRatio;
    double                  m_sensorSize;

    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);

protected:

    explicit CameraDevice(CreateParams const& params, CameraDeviceModelElementId cameraDeviceModel=CameraDeviceModelElementId()) : T_Super(params), m_cameraDeviceModel(cameraDeviceModel) 
        {
        m_focalLength = 0.0;
        m_imageWidth = 0;
        m_imageHeight = 0;
        m_principalPoint = { 0.0, 0.0 };
        m_aspectRatio = 0.0;
        m_skew = 0.0;
        //m_pixelToMeterRatio = 0.0;
        m_sensorSize = 0.0;
        }

    static BentleyStatus InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb, CameraDeviceElementId cameraDeviceElmId, CameraDeviceModelElementId cameraDeviceModelElmId);
    static CameraDeviceModelElementId QueryCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb, CameraDeviceElementId cameraDeviceElmId);

    void InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb) const;
    void UpdateCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb) const;
    void DeleteCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb) const;


    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    virtual void _CopyFrom(Dgn::DgnElementCR source) override;

    //! Remap any IDs that might refer to elements or resources in the source DgnDb.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    virtual void _RemapIds(Dgn::DgnImportContext&) override;

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

    virtual Dgn::DgnCode _GenerateDefaultCode() const override;


public:
    DECLARE_DATACAPTURE_ELEMENT_BASE_METHODS(CameraDevice)
    DECLARE_DATACAPTURE_QUERYCLASS_METHODS(CameraDevice)

    //! Create a new CameraDevice 
    DATACAPTURE_EXPORT static CameraDevicePtr Create(Dgn::SpatialModelR model, CameraDeviceModelElementId cameraDevice);

    //! Query for an CameraDevice (Id) by label
    //! @return Id of the CameraDevice or invalid Id if an CameraDevice was not found
    DATACAPTURE_EXPORT static CameraDeviceElementId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP label);

    //! Make an iterator over all Shot-s relevant to a CameraDevice
    DATACAPTURE_EXPORT static CameraDevice::ShotIterator MakeShotIterator(Dgn::DgnDbCR dgndb, CameraDeviceElementId cameraDeviceId);

    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR db, Utf8StringCR value);

    //! Get the Field of view of a camera
    DATACAPTURE_EXPORT static DPoint2d ComputeFieldOfView(CameraDeviceCR camDevice);

    //! Get the id of this CameraDevice
    DATACAPTURE_EXPORT CameraDeviceElementId GetId() const;

    //Properties Get/Set
    DATACAPTURE_EXPORT int                      GetImageWidth() const; 
    DATACAPTURE_EXPORT void                     SetImageWidth(int val);
    DATACAPTURE_EXPORT int                      GetImageHeight() const;
    DATACAPTURE_EXPORT void                     SetImageHeight(int val);
    //Focal always set/get in meters
    DATACAPTURE_EXPORT double                   GetFocalLength() const;
    //Focal always set/get in meters
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
    DATACAPTURE_EXPORT double                   GetSensorSize() const;
    DATACAPTURE_EXPORT void                     SetSensorSize(double val);

    //Since everything returned by ContextCapture will be in pixel we need a way to convert that into Meters
    //DATACAPTURE_EXPORT double                   GetPixelToMeterRatio() const;
    //DATACAPTURE_EXPORT void                     SetPixelToMeterRatio(double val);

    DATACAPTURE_EXPORT CameraDeviceModelElementId  GetCameraDeviceModelId() const;
    DATACAPTURE_EXPORT void                 SetCameraDeviceModelId(CameraDeviceModelElementId val);
  
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
//! Singleton CameraDeviceModelHandler for the CameraDeviceModel class
//=======================================================================================
struct CameraDeviceModelHandler : Dgn::dgn_ElementHandler::Element
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_CameraDeviceModel, CameraDeviceModel, CameraDeviceModelHandler, Dgn::dgn_ElementHandler::Element, DATACAPTURE_EXPORT)
    protected:
        virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
    }; // CameraDeviceModelHandler

//=================================================================================
//! ElementHandler for CameraDevice-s
//! @ingroup DataCaptureGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraDeviceHandler : Dgn::dgn_ElementHandler::Geometric3d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BDCP_CLASS_CameraDevice, CameraDevice, CameraDeviceHandler, Dgn::dgn_ElementHandler::Geometric3d, DATACAPTURE_EXPORT)
protected: 
    virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

