#include <Bentley/BeFileName.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/ScalableMeshDefs.h>


// Convert a 3MX model to a Scalable Mesh in 3SM format

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

// Scoped enum for the conversion status
enum class SMFrom3MXStatus
{
    Success = 0,
    Canceled,				// Cancelled during processing
    Read3MXError,			// Error while reading the master 3MX file
    Read3MXBError,			// Error while reading a 3MXB file
    ReadJPEGError,			// Error while reading a JPEG texture resource
    GCSError,				// Error related to GCS
    ScalableMeshSDKError,	// Error related to ScalableMeshSDK
    ReprojectionError,		// Error encountered in reprojection (most likely due to a 3D point out of bounds)
    UnsupportedCase			// Special case not handled by the converter
};


// Interface for the GCS handler used during conversion
class EXPORT_VTABLE_ATTRIBUTE IScalableMeshFrom3MXGCSHandler
{
public:
    virtual bool _SetInputGCS(CharCP InputSRS) = 0;	              // Called once to initialize the handler with the WKT string contained in the 3MX file
    virtual bool _Reproject(DPoint3dCR p, DPoint3dR q) const = 0; // Called to transform 3D positions from the 3MX GCS to the output Scalable Mesh GCS
    virtual GeoCoordinates::BaseGCSPtr _GetOutputGCS() const = 0; // Called to retrieve the GCS of the output Scalable Mesh
};


// Interface for the progress handler used during conversion
class EXPORT_VTABLE_ATTRIBUTE IScalableMeshFrom3MXProgressHandler
{
public:
    virtual bool _Progress(float progress) = 0; // Called to indicate the progress in the conversion, between 0.f and 1.f. If false is returned, the conversion is interrupted asap
};


// Conversion function. It has different variants to offer control on the GCS operations and the handling of progress
BENTLEY_SM_EXPORT SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,									// Path to the master 3MX file of the 3MX model
    BeFileNameCR output3SMPath,									// Path to the output 3SM file
    IScalableMeshFrom3MXGCSHandler& gcsHandler,					// Object handling the GCS operations
    IScalableMeshFrom3MXProgressHandler& progressHandler		// Object handling the progress
);

// Reprojection to specified GCS; No progress handling
BENTLEY_SM_EXPORT SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,									// Path to the master 3MX file of the 3MX model
    BeFileNameCR output3SMPath,									// Path to the output 3SM file
    GeoCoordinates::BaseGCSPtr outputGCS,						// Output GCS for reprojection. If nullptr, no reprojection
    IScalableMeshFrom3MXProgressHandler& progressHandler		// Object handling the progress
);

// No progress handling
BENTLEY_SM_EXPORT SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,									// Path to the master 3MX file of the 3MX model
    BeFileNameCR output3SMPath,									// Path to the output 3SM file
    IScalableMeshFrom3MXGCSHandler& gcsHandler					// Object handling the GCS operations
);

// Reprojection to specified GCS; No progress handling
BENTLEY_SM_EXPORT SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,									// Path to the master 3MX file of the 3MX model
    BeFileNameCR output3SMPath,									// Path to the output 3SM file
    GeoCoordinates::BaseGCSPtr outputGCS						// Output GCS for reprojection. If nullptr, no reprojection
);

END_BENTLEY_SCALABLEMESH_NAMESPACE
