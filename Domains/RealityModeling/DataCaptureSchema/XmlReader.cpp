/*--------------------------------------------------------------------------------------+
|
|  Notice that most of the code here cames from or is an adaptation of "src\MstnPlatform\mstn\mdlapps\rmutil\RMUtilImage.cpp"  
|  on vancouver source stream
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
XmlReader::XmlReader(Dgn::SpatialModelR spatialModel, Dgn::DefinitionModelR definitionModel)
:m_spatialModel(spatialModel), m_definitionModel(definitionModel), m_dgndb(spatialModel.GetDgnDb()), m_photoGroupNumber(0) 
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus XmlReader::ReadXml(BeFileNameCR xmlPathname)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, xmlPathname.c_str());
    if (xmlStatus != BEXML_Success)
        {
        BeAssert(false && "Cannot read specified XML file");
        return ERROR;
        }

    BeXmlNodeP rootNode = xmlDom->GetRootElement();
    if (nullptr == rootNode)
        {
        BeAssert(false && "Importing empty file");
        return ERROR;
        }
        
    BeXmlStatus status = ReadPhotoGroups(*rootNode);
    if (BEXML_Success != status)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
BeXmlStatus XmlReader::ReadPhotoGroups(BeXmlNodeR photoGroups)
    {
    BeXmlNodeP photoGroupsNode = photoGroups.SelectSingleNode("//Block/Photogroups");
    if (!photoGroupsNode)
        return BEXML_NodeNotFound;

    BeXmlDom::IterableNodeSet photoGroupList;
    photoGroupsNode->SelectChildNodes(photoGroupList, "Photogroup");

    for (BeXmlNodeP const& PhotoGroupNode : photoGroupList)
        {
        BeXmlStatus status = ReadPhotoGroupNode(*PhotoGroupNode);
        if (BEXML_Success != status)
            return status;
        }

    return BEXML_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlStatus XmlReader::ReadCameraDeviceInfo (BeXmlNodeR sourceNodeRef, CameraDeviceR cameraDeviceInfo, long photoGroupNumber)
    {

    // Photo group name is optional - make up one if not specified
    Utf8String photoGroupName;
    if (BEXML_Success != sourceNodeRef.GetContent(photoGroupName, "Name"))
        {
        // Photo group name is optional - make up one if not specified
        photoGroupName = Utf8PrintfString("Unnamed photo group %d", photoGroupNumber);
        }

    BeXmlStatus status(BEXML_Success);                          
    Utf8String              cameraDeviceModelType;
    if (BEXML_Success == (status = sourceNodeRef.GetContent(cameraDeviceModelType, "CameraDeviceModelType")))
        {
        //NEEDSWORK: Not Yet
        //cameraDeviceInfo.SetCameraDeviceModel(cameraDeviceModelType);
        }
    double                  focalLength;
    if (BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(focalLength, "FocalLength")))
        {
        cameraDeviceInfo.SetFocalLength(focalLength*1000);
        }

    int                     ImgDimWidth(0);
    int                     ImgDimHeight(0);
    if (BEXML_Success == (status =sourceNodeRef.GetContentInt32Value(ImgDimWidth,           "ImageDimensions/Width")) &&
        BEXML_Success == (status =sourceNodeRef.GetContentInt32Value(ImgDimHeight,          "ImageDimensions/Height")))
        {
        cameraDeviceInfo.SetImageWidth(ImgDimWidth);
        cameraDeviceInfo.SetImageHeight(ImgDimHeight);
        }

    // if Principal Point not specified, use center of pixel array
    DPoint2d principalPoint;
    if (BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(principalPoint.x, "PrincipalPoint/x" )) ||
        BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(principalPoint.y, "PrincipalPoint/y" )) )
        {
        principalPoint.x = (double)ImgDimWidth  / 2.0; 
        principalPoint.y = (double)ImgDimHeight / 2.0; 
        }
    cameraDeviceInfo.SetPrincipalPoint(principalPoint);


    // if distortion not specified, set parameters to zero     
    double k1;
    double k2;
    double k3;
    double p1;
    double p2;
    if (BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(k1, "Distortion/K1" )) ||
        BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(k2, "Distortion/K2" )) ||
        BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(k3, "Distortion/K3" )) ||
        BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(p1, "Distortion/P1" )) ||
        BEXML_Success != (status = sourceNodeRef.GetContentDoubleValue(p2, "Distortion/P2" )) )
        {
        // NEEDS_WORK - handle Fisheye lens
        k1 = 0.0;
        k2 = 0.0;
        k3 = 0.0;
        p1 = 0.0;
        p2 = 0.0;
        }
    else
        {
        RadialDistortionPtr  pRadialDistortion = RadialDistortion::Create(k1, k2, k3);
        TangentialDistortionPtr  pTangentialDistortion = TangentialDistortion::Create(p1, p2);
        cameraDeviceInfo.SetRadialDistortion(pRadialDistortion.get());
        cameraDeviceInfo.SetTangentialDistortion(pTangentialDistortion.get());
        }

    return BEXML_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlStatus XmlReader::ReadRotationFromCameraDevicePose(BeXmlNodeR sourceNodeRef, AngleR omegaAngle, AngleR phiAngle, AngleR kappaAngle)
    {
    BeXmlStatus status(BEXML_Success);

    double omega, phi, kappa;
    if (BEXML_Success == (status =  sourceNodeRef.GetContentDoubleValue(omega, "Pose/Rotation/Omega")) &&
        BEXML_Success == (status =  sourceNodeRef.GetContentDoubleValue(phi,   "Pose/Rotation/Phi")) &&
        BEXML_Success == (status =  sourceNodeRef.GetContentDoubleValue(kappa, "Pose/Rotation/Kappa")))
        {
        omegaAngle =  Angle::FromDegrees(omega);
        phiAngle = Angle::FromDegrees(phi);
        kappaAngle = Angle::FromDegrees(kappa);
        
        return BEXML_Success;
        }

    double M_00, M_01, M_02, M_10, M_11, M_12, M_20, M_21, M_22;
    if (BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_00, "Pose/Rotation/M_00")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_01, "Pose/Rotation/M_01")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_02, "Pose/Rotation/M_02")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_10, "Pose/Rotation/M_10")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_11, "Pose/Rotation/M_11")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_12, "Pose/Rotation/M_12")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_20, "Pose/Rotation/M_20")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_21, "Pose/Rotation/M_21")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(M_22, "Pose/Rotation/M_22")))
        {
        RotMatrix rotation = RotMatrix::FromRowValues(M_00, M_01, M_02, M_10, M_11, M_12, M_20, M_21, M_22);
        Pose::GetRotationFromRotMatrix(omegaAngle,phiAngle,kappaAngle,rotation);

        return BEXML_Success;
        }

    return BEXML_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlStatus XmlReader::ReadPhotoNode (BeXmlNodeR sourceNodeRef, ShotR shot, PoseR pose)
    {
    BeXmlStatus status(BEXML_Success);

    int id;
    if (BEXML_Success == (status = sourceNodeRef.GetContentInt32Value(id, "Id")))
        {
        CameraDeviceElementId cameraDeviceId = shot.GetCameraDeviceId();
        BeAssert(cameraDeviceId.IsValid());
        CameraDeviceCPtr cameraDevicePtr = CameraDevice::Get(shot.GetDgnDb(), cameraDeviceId);
        DgnCode shotCode = Shot::CreateCode(shot.GetDgnDb(), cameraDevicePtr->GetCode().GetValue(), Utf8PrintfString("%d", id));
        shot.SetCode(shotCode);
        }

    Utf8String imagePath;
    if (BEXML_Success == (status = sourceNodeRef.GetContent(imagePath,"ImagePath")))
        {
        //NEEDSWORK: not now
        }

    DPoint3d  poseCenter;
    Angle Omega;
    Angle Phi;
    Angle Kappa;
    if (BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(poseCenter.x, "Pose/Center/x")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(poseCenter.y, "Pose/Center/y")) &&
        BEXML_Success == (status = sourceNodeRef.GetContentDoubleValue(poseCenter.z, "Pose/Center/z")) &&
        BEXML_Success == (status = ReadRotationFromCameraDevicePose(sourceNodeRef, Omega, Phi, Kappa)))
        {
        //set pose in photo
        pose.SetCenter(poseCenter);
        pose.SetOmega(Omega);
        pose.SetPhi(Omega);
        pose.SetKappa(Omega);
        }                                                                                                           

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
BeXmlStatus XmlReader::ReadPhotoGroupNode(BeXmlNodeR photoGroupNode)
    {
    BeXmlStatus status(BEXML_Success);

    m_photoGroupNumber++;
    Utf8String photoGroupName;
    if (BEXML_Success != photoGroupNode.GetContent(photoGroupName, "Name"))
        {
        // Photo group name is optional - make up one if not specified
        photoGroupName = Utf8PrintfString("Unnamed photo group %d", m_photoGroupNumber);
        }

    
    //Assume one cameraDevice type by photo group
    CameraDeviceModelPtr pCameraDeviceModelInfo(CameraDeviceModel::Create(m_definitionModel));
    //Insert into the database
    Dgn::DgnDbStatus dbStatus;
    pCameraDeviceModelInfo->Insert(&dbStatus);


    CameraDevicePtr pCameraDeviceInfo(CameraDevice::Create(m_spatialModel,pCameraDeviceModelInfo->GetId()));
    if (BEXML_Success != (status = ReadCameraDeviceInfo(photoGroupNode, *pCameraDeviceInfo, m_photoGroupNumber)))
        {
        return status;
        }

    //Insert into the database
    pCameraDeviceInfo->Insert(&dbStatus);

    BeXmlDom::IterableNodeSet photoList;
    photoGroupNode.SelectChildNodes(photoList, "Photo");

    for (BeXmlNodeP const& photoNode : photoList)
        {
        //Insert into the database
        Dgn::DgnDbStatus dbStatus;
        PosePtr pPose = Pose::Create(m_spatialModel,false);
        pPose->Insert(&dbStatus);
        ShotPtr pShot(Shot::Create(m_spatialModel, pCameraDeviceInfo->GetId(), pPose->GetId()));
        pShot->Insert(&dbStatus);

        if (pPose.IsValid() && pShot.IsValid())
            status = ReadPhotoNode(*photoNode, *pShot, *pPose);
        if (BEXML_Success != status)
            {
            //Delete newly inserted element and return error
            pPose->Delete();
            pShot->Delete();
            return status;
            }

        }

    return BEXML_Success;
    }



END_BENTLEY_DATACAPTURE_NAMESPACE
