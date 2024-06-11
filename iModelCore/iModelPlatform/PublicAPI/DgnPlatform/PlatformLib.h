/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/WString.h>
#include "DgnPlatform.h"
#include "Render.h"
#include "ColorUtil.h"
#include "TxnManager.h"
#include "ViewDefinition.h"
#include "Visualization.h"
#include <Bentley/Logging.h>

typedef struct _EXCEPTION_POINTERS*  LPEXCEPTION_POINTERS;
struct FT_LibraryRec_;

#define T_HOST PlatformLib::GetHost()

DGNPLATFORM_TYPEDEFS(DgnHost)

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Class for initializing the IModelPlatform library.
//=======================================================================================
class PlatformLib {
public:
    struct EXPORT_VTABLE_ATTRIBUTE Host : DgnHost {
    private:
        void InitializeCrtHeap();
        void Initialize();
        void TerminateDgnCore(bool onProgramExit);

        friend class PlatformLib;

    public:
        //! Provides Exception handling capabilities
        struct ExceptionHandler : IHostObject {
            //! Possible outcomes of handling an exception
            enum class WasHandled {
                ContinueSearch = 0,
                ExecuteHandler = 1,
                NonContinuable = 2,
                ContinueExecution = -1,
            };

            enum class TerminationCondition {
                None = 0,
                AfterLogging = 1,
                Immediate = 2
            };

            //! Handle the specified exception
            virtual WasHandled _ProcessException(_EXCEPTION_POINTERS*) { return WasHandled::ContinueSearch; };
            virtual WasHandled _FilterException(_EXCEPTION_POINTERS*, bool onlyWantFloatingPoint) { return WasHandled::ContinueSearch; }
            //! Handle an assertion failure.
            virtual void _HandleAssert(WCharCP message, WCharCP file, unsigned line) {}
            virtual TerminationCondition _SetTerminationCondition(TerminationCondition when) { return when; }
            DGNPLATFORM_EXPORT virtual uint32_t _ResetFloatingPointExceptions(uint32_t newFpuMask);
            virtual uint32_t _QueryUser(bool attemptRecovery, Utf8CP productName) { return 0; }
            virtual uint32_t _EnterCoreCriticalSection(CharCP) { return 0; }   // WIP_CHAR_OK - Just for diagnostic purposes
            virtual uint32_t _ReleaseCoreCriticalSection(CharCP) { return 0; } // WIP_CHAR_OK - Just for diagnostic purposes
            virtual void _RestoreCoreCriticalSection(CharCP, int) {}           // WIP_CHAR_OK - Just for diagnostic purposes
        };

        //! Provides paths to known locations
        struct IKnownLocationsAdmin : IHostObject {
        protected:
            virtual ~IKnownLocationsAdmin() {}
            virtual BeFileNameCR _GetLocalTempDirectoryBaseName() = 0; //!< @see GetLocalTempDirectoryBaseName
            virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() = 0; //!< @see GetDgnPlatformAssetsDirectory
            virtual BeFileNameCR _GetGeoCoordinateDataDirectory() { return _GetDgnPlatformAssetsDirectory(); }

        public:
            //! Get the name of a local directory that can be used for creating temporary files. The directory must have
            //! write access. Subsystems generally create subdirectories in this directory for storing temporary data files.
            //! @return A string that holds the name of the base directory for temporary files. This is expected to always return the same object.
            DGNPLATFORM_EXPORT BeFileNameCR GetLocalTempDirectoryBaseName();

            //! Return a local directory that can be used to store temporary files. This directory can optionally be a subdirectory of #GetLocalTempDirectoryBaseName.
            //! @param[out] tempDir The name of temporary directory. This must be MAX_PATH chars in size.
            //! @param[in]  subDirName Optional subdirectory relative to default temp directory. If non-nullptr, this subdirectory will be created.
            //! @return nullptr if no temporary directory available.
            DGNPLATFORM_EXPORT BentleyStatus GetLocalTempDirectory(BeFileNameR tempDir, WCharCP subDirName);

            //! Return the directory containing the required DgnPlatform assets that must be deployed with any DgnPlatform-based app.
            //! Examples of required assets include fonts, ECSchemas, and localization resources.
            DGNPLATFORM_EXPORT BeFileNameCR GetDgnPlatformAssetsDirectory();

            //! Return the directory containing the required GeoCoordinateData that must be deployed with any DgnPlatform-based app.
            //! The only reason to use this method is when a PowerPlatform-based app is also using these libraries but wants to use the
            //! PowerPlatform based GeoCoordinateData and not the one from DgnClientSdk
            BeFileNameCR GetGeoCoordinateDataDirectory() { return _GetGeoCoordinateDataDirectory(); }
        };

        //=======================================================================================
        // @bsistruct
        //=======================================================================================
        struct VisualizationAdmin : DgnHost::IHostObject {
        protected:
            VisualizationUPtr m_viz;

        public:
            explicit VisualizationAdmin(VisualizationUPtr&& viz) : m_viz(std::move(viz)) {
                BeAssert(nullptr != m_viz);
            }

            Visualization& GetVisualization() { return *m_viz; }
        };

        //! Supervises the processing of GeoCoordination
        struct GeoCoordinationAdmin : IHostObject
            {
            //! Allows the host to provide a path to coordinate system definition files.
            virtual BeFileName _GetDataDirectory() {return BeFileName();}
            virtual IGeoCoordinateServicesP _GetServices() const {return nullptr;}
            };

        //! Support for geometric elements that store their data as Parasolid breps.
        struct BRepGeometryAdmin : IHostObject
            {
            //! Establish solid kernel schema file and temp file locations.
            //! Must be called with asset directory containing parasolid version files and directory to use for partitioned rollback temporary files. See PlatformLib::Host::Initialize.
            virtual void _Initialize(BeFileNameCR assetDir, BeFileNameCR tempDirBaseName) const {}

            //! Start solid kernel session.
            virtual void _StartSession() const {}

            //! Stop solid kernel session.
            virtual void _StopSession() const {}

            //! Return whether solid kernel session is currently started.
            virtual bool _IsSessionStarted() const {return false;}

            //! The MainThreadMark object should be included once in the main thread. It handles setting
            //! up the thread local storage used by all threads and replaces the error handler with
            //! one that handles severe errors and clears exclusions.
            virtual RefCountedPtr<IRefCounted> _CreateMainThreadMark() const {return nullptr;}

            //! The WorkerThreadOuterMark should be included once in each worker thread before any
            //! solid kernel processing is initated. It sets up a local storage for the worker thread
            //! and creates a new lightweight partition in which all solid kernel processing should be
            //! performed. The destructor will delete the lightweight partition created for this thread.
            //! These cannot nest. Do not instantiate on main thread.
            virtual RefCountedPtr<IRefCounted> _CreateWorkerThreadOuterMark() const {return nullptr;}

            //! This is alternative to WorkerThreadOuterMark that does not create a partition or
            //! perform any rollbacks when errors are encountered. It is intended for "read-only"
            //! multi-threaded processing of brep entities, and provides a significant performance
            //! boost for that case (~10x WorkerThreadOuterMark).
            //! These cannot nest. Do not instantiate on main thread.
            virtual RefCountedPtr<IRefCounted> _CreateWorkerThreadErrorHandler() const {return nullptr;}

            //! Receive a part archived in binary format and instantiate an entity in the solid kernel for the current session.
            //! Implementing this method is required to query/facet brep solids and surfaces.
            //! @param[out] out Ref counted pointer to new brep entity.
            //! @param[in] pBuffer The input data (always stored using binary format)
            //! @param[in] bufferSize Size of pBuffer.
            //! @param[in] transform The body to local transform to store on the output IBrepEntity.
            //! @return SUCCESS if out holds a valid brep entity.
            //! @note For Parasolid use PK_PART_RECEIVE.
            virtual BentleyStatus _RestoreEntityFromMemory(IBRepEntityPtr& out, uint8_t const* buffer, size_t bufferSize, TransformCR transform) const {return ERROR;}

            //! Transmit a part to a logical block of data (in binary format) using the solid kernel.
            //! Implementing this method is required for creating geometric elements from brep solid and surface geometry.
            //! @param[out] ppBuffer Archived data for brep entity.
            //! @param[out] bufferSize Size of data referenced by ppBuffer.
            //! @param[in] in The brep entity to transmit.
            //! @param[in] checkConsistency The entity must pass curve/edge and surface/face consistency checks before saving.
            //! @return SUCCESS if transmit of brep entity was successful.
            //! @note For Parasolid use PK_PART_transmit.
            //!       The Parasolid data version is 12.0 (transmit_version 120)
            //! @note The output ppBuffer is assumed to be owned by the input IBRepEntity who is responsible for freeing it
            //!       if necessary when its destructor is called.
            virtual BentleyStatus _SaveEntityToMemory(uint8_t** buffer, size_t& bufferSize, IBRepEntityCR in, bool checkConsistency=true) const {return ERROR;}

            //! Return a CurveVector representation for a wire or planar sheet body.
            virtual CurveVectorPtr _BodyToCurveVector(IBRepEntityCR entity) const {return nullptr;}

            //! Return a PolyfaceHeader created by facetting the supplied sheet or solid body using the specified facet options.
            virtual PolyfaceHeaderPtr _FacetEntity(IBRepEntityCR entity, IFacetOptionsCR facetOptions) const {return nullptr;}

            //! Return a PolyfaceHeader and FaceAttachment for each unique symbology by facetting the supplied sheet or solid body using the specified facet options.
            virtual bool _FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsCR facetOptions) const {return false;}

            //! Return length or perimeter properties for a wire or planar sheet body.
            virtual BentleyStatus _GetLengthProperties(IBRepEntityCR entity, double& length, DPoint3dR centroid, DPoint3dR moments, double& iXY, double& iXZ, double& iYZ, double tolerance) const {return ERROR;}

            //! Return surface area properties for sheet or solid body.
            virtual BentleyStatus _GetAreaProperties(IBRepEntityCR entity, double& area, double& perimeter, DPoint3dR centroid, DPoint3dR moments, double& iXY, double& iXZ, double& iYZ, double tolerance) const {return ERROR;}

            //! Return volume properties for a solid body.
            virtual BentleyStatus _GetVolumeProperties(IBRepEntityCR entity, double& volume, double& area, DPoint3dR centroid, DPoint3dR moments, double& iXY, double& iXZ, double& iYZ, double tolerance) const {return ERROR;}

            //! Announce the closest edge or face isoparameter curve to evaluate for the current snap.
            //! @param[in] entity The brep entity to snap to.
            //! @param[in] closePointLocal Local coordinate point from readPixels identifying this element.
            //! @param[in] snapDivisor The number of uv isoparameter curves to extract when snapping to the interior of a non-planar face.
            //! @param[in] processor The snap processor object to announce edge and face isoparameter curve snap candidates to.
            virtual bool _SnapFindClosestCurve(IBRepEntityCR entity, DPoint3dCR closePointLocal, uint32_t snapDivisor, ISnapProcessorR processor) const {return false;}

            //! Evaluate the surface normal at the supplied snap location.
            //! @param[in] entity The brep entity to snap to.
            //! @param[in] snapPointLocal Local coordinate snap point adjusted to face or edge of solid/surface.
            //! @param[in] testPointLocal Local coordinate point identifying the original surface hit, use to disambiguate face for an edge hit.
            //! @param[in] preferredDir Local coordinate vector identifying the viewing direction for choosing the preferred face from a vertex or edge hit.
            //! @param[out] point The coordinates of the point on the surface at the uv parameter of snapPointLocal.
            //! @param[out] normal The normalized surface normal at the uv parameter of snapPointLocal.
            virtual bool _SnapEvaluateInterior(IBRepEntityCR entity, DPoint3dCR snapPointLocal, DPoint3dCR testPointLocal, DVec3dCR preferredDir, DPoint3dR point, DVec3dR normal) const {return false;}

            //! Optional method to implement for supporting creation of brep geometry from flatbuffer geometry opcodes and operation parameters.
            virtual DgnDbStatus _CreateBRepGeometry(DgnDbR db, Napi::Object const& createProps, Napi::Env env) const {return DgnDbStatus::BadRequest;}

            //! Optional method to implement geometry cache for interactive editing tools.
            virtual void _EditGeometryCachePopulate(DgnDbR db, BeJsValue out, BeJsConst input, ICancellableR cancel) const {};

            //! Optional method to implement geometry operations for interactive editing tools that use the geometry cache.
            virtual BentleyStatus _EditGeometryCacheOperation(DgnDbR db, Napi::Object const& input, Napi::Env env) const {return ERROR;};

            //! Create a new IBRepEntity from a json value. Calls _RestoreEntityFromMemory;
            DGNPLATFORM_EXPORT static IBRepEntityPtr BodyFromJson(BeJsConst value);

            //! Represent the supplied IBRepEntity as a json value. Calls _SaveEntityToMemory;
            DGNPLATFORM_EXPORT static void BodyToJson(BeJsValue, IBRepEntityCR entity);
            };

        typedef bvector<DgnDomain*> T_RegisteredDomains;

    protected:
        IKnownLocationsAdmin*   m_knownLocationsAdmin;
        ExceptionHandler*       m_exceptionHandler;
        GeoCoordinationAdmin*   m_geoCoordAdmin;
        BRepGeometryAdmin*      m_bRepGeometryAdmin;
        VisualizationAdmin*     m_visualizationAdmin;
        T_RegisteredDomains     m_registeredDomains;

    public:
        T_RegisteredDomains& RegisteredDomains() {return m_registeredDomains;}

        DGNPLATFORM_EXPORT virtual void _OnAssert(WCharCP _Message, WCharCP _File, unsigned _Line);

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() = 0;

        DGNPLATFORM_EXPORT virtual ExceptionHandler& _SupplyExceptionHandler();

        DGNPLATFORM_EXPORT virtual VisualizationAdmin& _SupplyVisualizationAdmin();

        //! Supply the GeoCoordinationStateAdmin for this session. This method is guaranteed to be called once per thread from PlatformLib::Host::Initialize and never again..
        DGNPLATFORM_EXPORT virtual GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        //! Supply the BRepGeometryAdmin for this session. This method is guaranteed to be called once per thread from PlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual BRepGeometryAdmin& _SupplyBRepGeometryAdmin();

        Host()
            {
            m_knownLocationsAdmin = nullptr;
            m_exceptionHandler = nullptr;
            m_geoCoordAdmin = nullptr;
            m_bRepGeometryAdmin = nullptr;
            m_visualizationAdmin = nullptr;
            };

        virtual ~Host() {}

        IKnownLocationsAdmin&   GetIKnownLocationsAdmin()  {return *m_knownLocationsAdmin;}
        ExceptionHandler&       GetExceptionHandler()      {return *m_exceptionHandler;}
        GeoCoordinationAdmin&   GetGeoCoordinationAdmin()  {return *m_geoCoordAdmin;}
        BRepGeometryAdmin&      GetBRepGeometryAdmin()     {return *m_bRepGeometryAdmin;}
        VisualizationAdmin&     GetVisualizationAdmin()    {return *m_visualizationAdmin;}
        VisualizationR          Visualization()            {return GetVisualizationAdmin().GetVisualization();}

        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized() {return nullptr != m_knownLocationsAdmin;}

        //! Terminate this Host.
        //! This method should be called when the application is done with this host. After this method is called, no further DgnPlatform methods can ever be called on this host again (including Initialize).
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        DGNPLATFORM_EXPORT void Terminate(bool onProgramExit);
        };

    static void StaticInitialize();

public:
    //! Must be called before DgnPlatform services can be used.
    //! @param host The host to associate with this process.
    DGNPLATFORM_EXPORT static void Initialize(Host& host);
    DGNPLATFORM_EXPORT static void Terminate(bool onProgramExit);

    //! Query whether Initialize has been called.
    //! @return nullptr if not Host is associated with the current process. Otherwise, a pointer to the Host object.
    DGNPLATFORM_EXPORT static Host* QueryHost();

    //! Get the Host
    //! @return a reference to the Host object. WARNING: Do not call this function unless you know that there is a Host.
    DGNPLATFORM_EXPORT static Host& GetHost();

    //! Forward assertion failures to the specified handler.
    DGNPLATFORM_EXPORT static void ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler*);
};

END_BENTLEY_DGN_NAMESPACE

