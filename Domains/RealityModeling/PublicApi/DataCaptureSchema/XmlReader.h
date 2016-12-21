/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/XmlReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//=======================================================================================
//! Utility to read the Data Capture model from an XML file
//! @see XmlWriter
//! @ingroup DataCaptureGroup
//=======================================================================================
struct XmlReader
{

private:
    Dgn::SpatialModelR      m_spatialModel;     //Data capture information can be store on any spatial model
    Dgn::DefinitionModelR   m_definitionModel;  //Data capture definition can be store on any definition model
    Dgn::DgnDbR             m_dgndb;
    long                    m_photoGroupNumber;



    BeXmlStatus ReadBlocks(BeXmlNodeR parentNode);
    BeXmlStatus ReadPhotoGroups(BeXmlNodeR photoGroups);
    BeXmlStatus ReadPhotoGroupNode(BeXmlNodeR photoGroupNode);
    BeXmlStatus ReadCameraDeviceInfo(BeXmlNodeR sourceNodeRef, CameraDeviceR cameraDeviceInfo, long photoGroupNumber);
    BeXmlStatus ReadPhotoNode(BeXmlNodeR sourceNodeRef,  ShotR shot, PoseR pose);
    BeXmlStatus ReadRotationFromCameraDevicePose(BeXmlNodeR photoNode, AngleR omegaAngle, AngleR phiAngle, AngleR kappaAngle);


public:
    //! Constructor
    DATACAPTURE_EXPORT XmlReader(Dgn::SpatialModelR spatialModel, Dgn::DefinitionModelR definitionModel);
    
    //! Read an XML file containing the  Data Capture Model
    DATACAPTURE_EXPORT BentleyStatus ReadXml(BeFileNameCR xmlPathname);
};

END_BENTLEY_DATACAPTURE_NAMESPACE



