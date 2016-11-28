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
    Dgn::SpatialModelR m_model;  //Data capture information can be store on any spatial model
    Dgn::DgnDbR        m_dgndb;
    long               m_photoGroupNumber;



    BeXmlStatus ReadBlocks(BeXmlNodeR parentNode);
    BeXmlStatus ReadPhotoGroups(BeXmlNodeR photoGroups);
    BeXmlStatus ReadPhotoGroupNode(BeXmlNodeR photoGroupNode);
    BeXmlStatus ReadCameraInfo(BeXmlNodeR sourceNodeRef, CameraR cameraInfo, long photoGroupNumber);
    BeXmlStatus ReadPhotoNode(BeXmlNodeR sourceNodeRef, PhotoR photo);
    BeXmlStatus ReadRotationFromCameraPose(BeXmlNodeR photoNode, RotationMatrixTypeR rotation);


public:
    //! Constructor
    DATACAPTURE_EXPORT XmlReader(Dgn::SpatialModelR model) : m_model(model), m_dgndb(model.GetDgnDb()),m_photoGroupNumber(0) {}
    
    //! Read an XML file containing the  Data Capture Model
    DATACAPTURE_EXPORT BentleyStatus ReadXml(BeFileNameCR xmlPathname);
};

END_BENTLEY_DATACAPTURE_NAMESPACE



