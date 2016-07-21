/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SectionClip.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS(IViewClipObject)
DGNPLATFORM_REF_COUNTED_PTR(IViewClipObject)

BEGIN_BENTLEY_DGN_NAMESPACE

/*
 SectionClip holds information associated with the clip volume tool which inturn is
 used to limit the displayed volume for a view to the region within a clipping element.

 Using the APIs here, one can perform various functions like decide the clip volume dimensions, 
 set the points along which clipping can take place and define and set the rotation matrix of the clip element.
*/

/*==============================================**//**
 Possible options to define sizes of the clipped element from the clipped part.
 There are four possible sizes.
=====================================*/

enum ClipVolumeSizeProp
    {
    CLIPVOLUME_SIZE_Invalid             = -1,
    CLIPVOLUME_SIZE_TopHeight           = 0,
    CLIPVOLUME_SIZE_BottomHeight        = 1,
    CLIPVOLUME_SIZE_FrontDepth          = 2,
    CLIPVOLUME_SIZE_BackDepth           = 3,
    };

/* =================================**//**
 Possible options to define the clipped sides of the clip volume.

 Considering the clip element to be having 6 possible sides, 
 each side is identified using integers ranging from 0 to 5.
===================================*/
enum ClipVolumeCropProp
    {
    CLIPVOLUME_CROP_Invalid             = -1,
    CLIPVOLUME_CROP_StartSide           = 0,
    CLIPVOLUME_CROP_EndSide             = 1,
    CLIPVOLUME_CROP_Front               = 2,
    CLIPVOLUME_CROP_Back                = 3,
    CLIPVOLUME_CROP_Bottom              = 4,
    CLIPVOLUME_CROP_Top                 = 5,
    };

typedef bvector<DPoint3d> DPoint3dVector;

//=======================================================================================
//! An IViewClipObject is an interface that can be adopted
//! to access the data which describes a clip volume
//! and perform certain manipulations on the same.
//=======================================================================================
struct IViewClipObject : RefCountedBase
{
    struct Factory : RefCountedBase
    {
    protected:
        virtual IViewClipObjectPtr _FromJson(JsonValueCR) = 0;

    public:
        //! Register an IViewClipObject::Factory to handle FromJson requests for all serialized IViewClipObject objects with the specified factoryId
        //! @param factoryId    The persistent ID of the factory.
        //! @param factory      The factory
        //! @return previously registered factory or NULL if none previously registered
        DGNPLATFORM_EXPORT static RefCountedPtr<Factory> RegisterFactory(Utf8StringCR factoryId, Factory& factory);

        //! Get a registered factory
        //! @param factoryId    The persistent ID of the factory.
        //! @return registered factory or NULL if none registered
        DGNPLATFORM_EXPORT static RefCountedPtr<Factory> GetFactory(Utf8StringCR factoryId);

        //! Deserialize an IViewClipObject instance from stored data
        DGNPLATFORM_EXPORT static IViewClipObjectPtr FromJson(JsonValueCR);

        //! Serialize an IViewClipObject instance to stored data
        DGNPLATFORM_EXPORT static void ToJson(JsonValueR, IViewClipObject const&);
    };

    friend Factory;

protected:
    virtual Utf8String _GetFactoryId() const = 0;
    virtual void _ToJson(JsonValueR) const = 0;
    virtual void _FromJson(JsonValueCR) = 0;
    virtual double _GetSize(ClipVolumeSizeProp clipVolumeSizeProp) const = 0;
    virtual void _SetSize(ClipVolumeSizeProp clipVolumeSizeProp, double size) = 0;
    virtual bool _GetCrop(ClipVolumeCropProp clipVolumeCropProp) const = 0;
    virtual void _SetCrop(ClipVolumeCropProp clipVolumeCropProp, bool crop) = 0;
    virtual RotMatrixCR _GetRotationMatrix() const = 0;
    virtual void _SetRotationMatrix(RotMatrixCR rMatrix) = 0;
    virtual void _SetPoints(size_t numPoints, DPoint3dCP points) = 0;
    virtual size_t _GetNumPoints() const = 0;
    virtual StatusInt _GetPoints(DPoint3dVector& points, size_t iFromPoint, size_t numPoints) const = 0;
    virtual void _CopyCrops(IViewClipObject const* from) = 0;
    virtual void _SetPreserveUp(bool) = 0;
    virtual bool _GetPreserveUp() const = 0;
    virtual double _GetWidth() const = 0;
    virtual void _SetWidth(double newWidth) = 0;
    virtual StatusInt _GetClipBoundary(ClipVectorPtr& clip, DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const = 0; // changed in graphite
    virtual bool _IsClipVolumePassValid(ClipVolumePass) const {return false;}
    virtual StatusInt _GetCuttingPlane(DPlane3dR cutPlane, DVec3dR xDir, DVec3dR yDir, ClipMask& clipMask, DRange2dR clipRange, bool& forwardFacing, int index, ViewContextR) const {return ERROR;}
    virtual bool _GetAuxTransform(TransformR, ClipVolumePass) const {return false;}
    virtual StatusInt _GetTransform(TransformR trans) const = 0;
    virtual size_t _GetPrimaryCutPlaneCount() const = 0;
    virtual StatusInt _ApplyTransform(TransformCR) = 0;
    virtual void _Draw(ViewContextR) = 0;

public:
    //! @brief Gets the size of the clipped volume corresponding to the value of the enumerated variable `clipVolumeSizeProp`.
    //! @param[in]      clipVolumeCropProp      the specific size to be querried.
    //! @returns the dimension of the side to be clipped.
    double GetSize(ClipVolumeSizeProp clipVolumeCropProp) const {return _GetSize(clipVolumeCropProp);}

    //! Assigns the user defined value to the size of the clip element.
    //! @param[in]      clipVolumeSizeProp      the specific size to be querried.
    //! @param[in]      size                    contains the desired value to be clipped to.
    //! @returns the dimension of the side to be clipped.
    void SetSize(ClipVolumeSizeProp clipVolumeSizeProp, double size) {return _SetSize(clipVolumeSizeProp, size);}

    //! Check if a given side of the clip volume is cropped.
    //! @param[in]      clipVolumeCropProp      contains the information as to which particular side should be clipped.
    //! @returns the boolean value against the desired side of the clip element.
     bool GetCrop(ClipVolumeCropProp clipVolumeCropProp) const {return _GetCrop(clipVolumeCropProp);}

    //! Sets the particular side to either of the two conditions : to be cropped or not to be cropped using a boolean variable.
    //! @param[in]      clipVolumeCropProp      contains the particular side and its clip details.
    //! @param[in]      crop                    true would mean clipped and false would mean unclipped.
    //! @returns  the particular side and its clip details
    void SetCrop(ClipVolumeCropProp clipVolumeCropProp, bool crop) {return _SetCrop(clipVolumeCropProp, crop);}

    //! Gets the rotation matrix  that describes how the element is oriented in 3D space.
    //! @returns the orientation details in the 3D space of the element to be clipped, in the form of a matrix.
    RotMatrixCR GetRotationMatrix() const {return _GetRotationMatrix();}

    //! Sets the rotation matrix of the element to a defined matrix and set the orientation in the 3D space.
    //! @returns the orientation details in the 3D space of the element to be clipped, in the form of the defined matrix.
    //! \sa GetRotationMatrix .
    void SetRotationMatrix(RotMatrixCR rMatrix) {return _SetRotationMatrix(rMatrix);}

    //! Sets the points of the clip element.
    //! @param[in]      numPoints      unsigned integer that contains the number of index points of the clip element.
    //! @param[in]      points         the 3D point coordinates of the clip element.
    //! @returns data points of the clip element.
    //! \sa GetNumPoints  \sa  GetPoints.
    void SetPoints(size_t numPoints, DPoint3dCP points) {return _SetPoints(numPoints,points);}

    //! Obtains the number of data points of the clipped volume.
    //! @returns the number of data points of the clipped volume.
    //! \sa SetPoints.
    size_t GetNumPoints() const {return _GetNumPoints();}

    //! Obtains the data points of the clipped volume.
    //! @param[out]      points          reference to the vector containing the 3D points.
    //! @param[in]      iFromPoint      initial index point of the clip volume.
    //! @param[in]      numPoints       number of index points of the clip volume.
    //! @returns the data points of the clipped volume.
    //! \sa SetPoints.
    StatusInt GetPoints(DPoint3dVector& points, size_t iFromPoint, size_t numPoints) const {return _GetPoints(points, iFromPoint, numPoints);}

    //! Makes a copy of the clip details of a given clip object.
    //! @param[in]      from            a const pointer to IViewClipObject .
    //! @returns a  copy of the clip details of a given clip object.
    void CopyCrops(IViewClipObject const* from) {return _CopyCrops(from);}

    //! Sets preserve up flag to indicate that the resulting view of this clip volume should be displayed 'up right'.
    //! param[in] flag pass 1 for clipped and zero for unclipped.
    //! @returns the resulting view of this clip volume which should be displayed 'up right'.
    void SetPreserveUp(bool flag) {return _SetPreserveUp(flag);}

    //! Gets preserve up flag to indicate that the resulting view of this clip volume should be displayed 'up right'.
    //! @returns the  resulting view of this clip volume which should be displayed 'up right'.
    bool GetPreserveUp() const {return _GetPreserveUp();}

    //! Gets width of the clip element.
    //! @returns the width of the clip element.
    double GetWidth() const {return _GetWidth();}

    //! Set the width of the clip element to a user defined value newWidth.
    //! @param[in] newWidth the value to be assigned to the width of the clip element.
    //! @returns width of the clip element to a user defined value newWidth.
    void SetWidth(double newWidth) {_SetWidth(newWidth);}

    StatusInt GetClipBoundary(ClipVectorPtr& clip, DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const {return _GetClipBoundary(clip,maxRange,pass,displayCutGeometry);}
    bool IsClipVolumePassValid(ClipVolumePass pass) const {return _IsClipVolumePassValid(pass);}
    StatusInt GetCuttingPlane(DPlane3dR cutPlane, DVec3dR xDir, DVec3dR yDir, ClipMask& clipMask, DRange2dR clipRange, bool& forwardFacing, int index, ViewContextR context) const {return _GetCuttingPlane(cutPlane,xDir,yDir,clipMask,clipRange,forwardFacing,index,context);}
    bool GetAuxTransform(TransformR t, ClipVolumePass p) const {return _GetAuxTransform(t,p);}
    StatusInt GetTransform(TransformR trans) const {return _GetTransform(trans);}
    size_t GetPrimaryCutPlaneCount() const {return _GetPrimaryCutPlaneCount();}
    StatusInt ApplyTransform(TransformCR t) {return _ApplyTransform(t);}
    void Draw(ViewContextR c) {return _Draw(c);}
};

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* Support for DgnV8ProjectImporter
* @bsiclass                                    Sam.Wilson      06/2014
+===============+===============+===============+===============+===============+======*/
struct SectionClipObjectLegacyData
    {
    bvector<DPoint3d> m_points;

    struct ClipData
        {
        struct Params
            {
            uint32_t cropMask:6;
            uint32_t preserveUp:1;
            uint32_t reserved:25;
            } params;

        int         numPoints;
        double      topHeight;
        double      bottomHeight;
        double      frontDepth;
        double      backDepth;

        RotMatrix   rotMatrix;

        } m_clipData;
    
    DGNPLATFORM_EXPORT IViewClipObjectPtr CreateObject();
    };

/*=================================================================================**//**
* @bsiclass                                    Sam.Wilson      06/2014
+===============+===============+===============+===============+===============+======*/
struct SectionClipObjectFactory : IViewClipObject::Factory
{
protected:
    DGNPLATFORM_EXPORT virtual IViewClipObjectPtr _FromJson(JsonValueCR) override;

public:
    static Utf8String GetFactoryId() {return "SectionClip";}
    static void Register() {RegisterFactory(GetFactoryId(), *new SectionClipObjectFactory());}
};

//__PUBLISH_SECTION_START__

END_BENTLEY_DGN_NAMESPACE

