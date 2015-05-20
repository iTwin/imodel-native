/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudDataQuery.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

struct FilterChannelEditor;
struct IPointCloudSymbologyChannel;
struct IPointCloudDataQuery;
typedef RefCountedPtr<FilterChannelEditor>          FilterChannelEditorPtr;
typedef RefCountedPtr<IPointCloudSymbologyChannel>  IPointCloudSymbologyChannelPtr;
typedef RefCountedPtr<IPointCloudDataQuery>         IPointCloudDataQueryPtr;

/*=================================================================================**//**
* \addtogroup Pointcloud
*/
//@{

/*---------------------------------------------------------------------------------**//**
* FilterChannelEditor
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterChannelEditor : public RefCountedBase
    {
    unsigned char* m_buf;
    FilterChannelEditor();   // disabled
    FilterChannelEditor(unsigned char* selectionBuffer);

    public:
        BEPOINTCLOUD_EXPORT void Show (uint32_t index);
        BEPOINTCLOUD_EXPORT void Hide (uint32_t index);
        BEPOINTCLOUD_EXPORT bool IsVisible (uint32_t index) const;
        BEPOINTCLOUD_EXPORT bool IsHidden (uint32_t index) const;
        BEPOINTCLOUD_EXPORT bool IsSelected (uint32_t index) const;

        static FilterChannelEditorPtr Create(unsigned char* filterBuffer);
    };

/*---------------------------------------------------------------------------------**//**
* Class that associates a symbology to a channel. Mostly used for clash detection.
* IPointCloudSymbologyChannel
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudSymbologyChannel : public RefCountedBase
    {
    protected:
                IPointCloudSymbologyChannel();
        virtual ~IPointCloudSymbologyChannel();
        virtual uint32_t _GetPointSize() const = 0;
        virtual void   _SetPointSize(uint32_t size) = 0;
    
    public:
        BEPOINTCLOUD_EXPORT uint32_t GetPointSize() const;
        BEPOINTCLOUD_EXPORT void   SetPointSize(uint32_t size);

        BEPOINTCLOUD_EXPORT static IPointCloudSymbologyChannelPtr Create();
    };

/*---------------------------------------------------------------------------------**//**
* IPointCloudChannelHandlerFilter
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudChannelHandlerFilter
    {
    public:

        /*---------------------------------------------------------------------------------**//**
        * @param     handler IN PointCloudChannelHandler to filter out
        * @param     pPointCloudChannel IN NULL or channel on which the handler is set on
        * @return    false to ignore this PointCloudChannelHandler
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _Filter (PointCloudChannelHandlerCR handler, IPointCloudChannelCP pPointCloudChannel) const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* IPointCloudQueryBuffers
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudQueryBuffers : RefCountedBase
    {
    protected:
        IPointCloudQueryBuffers();
        virtual ~IPointCloudQueryBuffers();

        virtual DPoint3d*                   _GetXyzBuffer() const = 0;
        virtual PointCloudColorDefP         _GetRgbBuffer() const = 0;
        virtual int16_t*                    _GetIntensityBuffer() const = 0;
        virtual FPoint3d*                   _GetNormalBuffer() const = 0;
        virtual unsigned char*                      _GetFilterBuffer() const = 0;
        virtual unsigned char*                      _GetClassificationBuffer() const  = 0;
        virtual void*                       _GetChannelBuffer(IPointCloudChannelP pChannel) const = 0;
        virtual uint32_t                    _GetNumPoints() const = 0;
        virtual uint32_t                    _GetCapacity() const = 0;

        virtual void                        _SetNumPoints(uint32_t numPoints) = 0;

        virtual void                        _AddChannel(PointCloudChannelId channelId) = 0;
        virtual unsigned char*              _CreatePointCloudSymbologyChannel(IPointCloudSymbologyChannelP symb) = 0;

    public:
        BEPOINTCLOUD_EXPORT DPoint3d*              GetXyzBuffer() const;
        BEPOINTCLOUD_EXPORT PointCloudColorDefP    GetRgbBuffer() const;
        BEPOINTCLOUD_EXPORT int16_t*               GetIntensityBuffer() const;
        BEPOINTCLOUD_EXPORT FPoint3d*              GetNormalBuffer() const;
        BEPOINTCLOUD_EXPORT unsigned char*         GetFilterBuffer() const;
        BEPOINTCLOUD_EXPORT unsigned char*         GetClassificationBuffer() const;
        BEPOINTCLOUD_EXPORT void*                  GetChannelBuffer(IPointCloudChannelP pChannel) const;

        BEPOINTCLOUD_EXPORT uint32_t               GetNumPoints() const;
        BEPOINTCLOUD_EXPORT uint32_t               GetCapacity() const;
        BEPOINTCLOUD_EXPORT void                   SetNumPoints(uint32_t numPoints);
        BEPOINTCLOUD_EXPORT void                   AddChannel(PointCloudChannelId channelId);

        BEPOINTCLOUD_EXPORT unsigned char*                    CreatePointCloudSymbologyChannel(IPointCloudSymbologyChannelP symb);
        BEPOINTCLOUD_EXPORT FilterChannelEditorPtr GetFilterChannelEditor() const;
    };

/*---------------------------------------------------------------------------------**//**
* Interface to query points from in a point cloud element. 
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudDataQuery : public IRefCounted
{
public:
    enum QUERY_MODE
        {
        QUERY_MODE_FILE=1,
        QUERY_MODE_VIEWSETTINGS=2,
        };

    enum QUERY_DENSITY
        {
        QUERY_DENSITY_FULL=1,
        QUERY_DENSITY_VIEW=2,
        QUERY_DENSITY_LIMIT=3,
        };

    enum POINT_FILTER
        {
        POINT_SELECTED = 0x80,
        POINT_HIDDEN   = 0x00,
        };

protected:
    virtual unsigned int _GetPoints (IPointCloudQueryBuffersR pPointCloudChannelBuffers) = 0;
    virtual double      _GetFitCylinder (DVec3dR axis, DPoint3dR base, double& radius, double& height, bool constrainToAxis, bool constrainToRadius) = 0;
    virtual double      _GetFitPlanarRectangle (DPoint3d corners[4], bool constrainToNormal) = 0;
    virtual double      _GetFitPlane (DVec3dR planeNormal, DPoint3dR planeOrigin, bool constrainToNormal) = 0;
    virtual void        _Reset () = 0;    
    virtual void        _SetDensity (QUERY_DENSITY type, float densityValue) = 0;
    virtual void        _SetIgnoreTransform (bool ignore) = 0;
    virtual StatusInt   _SubmitUpdate (IPointCloudChannelPtr pChannel) = 0;
    virtual TransformR  _GetUORToNativeTransform (TransformR trans) = 0;
/* POINTCLOUD_WIP_GR06_ElementHandle)
    virtual StatusInt   _ReprojectUOR  (EditElementHandleR eeh, DPoint3dP pt, int nPoints, DgnModelP modelRef) = 0;
*/
    virtual void        _GetMode (QUERY_MODE& mode, int& viewNum) const = 0;
    virtual void        _GetDensity (QUERY_DENSITY& type, float& densityValue) const = 0;
    virtual bool        _GetIgnoreTransform () const = 0;

    virtual IPointCloudQueryBuffersPtr  _CreateBuffers(uint32_t capacity, uint32_t channelFlags, IPointCloudChannelVectorCR channels) = 0;

    virtual void        _SetChannelHandlerFilter(IPointCloudChannelHandlerFilterP pFilter) = 0;
    virtual void        _GetAvailableChannelFlags (uint32_t& channelFlags) const = 0;
    virtual void        _GetChannelHandlers(PointCloudChannelHandlers& handlers) const = 0;

public:

#if defined (POINTCLOUD_WIP_GR06_ElementHandle)

    /*---------------------------------------------------------------------------------**//**
    * Creates a query for points within the pointcloud's axis aligned bounding box.
    * @param eh IN PointCloud Element to query
    * @param origin IN Minimum UOR extent of bounding box
    * @param corner IN Maximum UOR extent of bounding box
    * @return   Reference counted pointer to an IPointCloudDataQuery instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT static IPointCloudDataQueryPtr CreateBoundingBoxQuery(ElementHandleCR eh, DPoint3dCR origin, DPoint3dCR corner);

    /*---------------------------------------------------------------------------------**//**
    * Creates a query for points within a positioned and oriented bounding box.
    * @param eh IN PointCloud Element to query
    * @param box  IN const reference PointCloudBox.
    * @return   Reference counted pointer to an IPointCloudDataQuery instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT static IPointCloudDataQueryPtr CreateOrientedBoxQuery (ElementHandleCR eh, OrientedBoxCR box);

    /*---------------------------------------------------------------------------------**//**
    * Creates a query for points within a sphere.
    * @param eh IN PointCloud Element to query
    * @param center IN Centre point of the sphere in UOR
    * @param radius IN Radius of sphere in UOR
    * @return   Reference counted pointer to an IPointCloudDataQuery instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT static IPointCloudDataQueryPtr CreateBoundingSphereQuery(ElementHandleCR eh, DPoint3dCR center, double radius);
//NEEDS_WORK &&SN we didn't publish PointCloudEditSelectPoints API yet
    /*---------------------------------------------------------------------------------**//**
    * Creates a query for visible points only.
    * Points that have been hidden will not be returned. 
    * Also note that this may include points that are outside of the viewing extent
    * @param eh IN PointCloud Element to query
    * @return   Reference counted pointer to an IPointCloudDataQuery instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT static IPointCloudDataQueryPtr CreateVisiblePointsQuery(ElementHandleCR eh);

    //&&DM : Might want to change this
    BEPOINTCLOUD_EXPORT StatusInt ReprojectUOR  (EditElementHandleR eeh, DPoint3dP pt, int nPoints, DgnModelP modelRef);


    /*---------------------------------------------------------------------------------**//**
    * Creates a query for selected points.
    * @param eh IN PointCloud Element to query
    * @return   Reference counted pointer to an IPointCloudDataQuery instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT static IPointCloudDataQueryPtr CreateSelectedPointsQuery(ElementHandleCR eh);

#endif


    /*---------------------------------------------------------------------------------**//**
    * Creates a query buffer for the query.
    * @param capacity       IN capacity of the buffers to create
    * @param channelFlags   IN PointCloudChannelId flags to specify which buffers to create
    * @param channels       IN custom point channels to create buffers for
    * @return   Reference counted pointer to an IPointCloudQueryBuffers instance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudQueryBuffersPtr  CreateBuffers(uint32_t capacity, uint32_t channelFlags, IPointCloudChannelVectorCR channels);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void SetChannelHandlerFilter(IPointCloudChannelHandlerFilterP pFilter);
    
    /*---------------------------------------------------------------------------------**//**
    * fills the PointCloudChannelId flags that the query can support
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void GetAvailableChannelFlags (uint32_t& channelFlags) const;

    /*---------------------------------------------------------------------------------**//**
    * fills PointCloudChannelHandlerP that wish to be called by this query
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void GetChannelHandlers(PointCloudChannelHandlers& handlers) const;

    /*---------------------------------------------------------------------------------**//**
    * Sets the detail level for the query allowing quick processing of a view or density based subset of points
    * 
    * @param type IN The level of detail required. This can be one of the following:
    *                   QUERY_DENSITY_FULL      The query returns every point including points that in held in 
    *                                           out-of-core storage useful for algorithms that need to process every point.
    *                                           This is the default behavior, however setting a densityValue less than
    *                                           1 returns a subset of points, ie a percentage of 100 x the density value.
    *                   QUERY_DENSITY_VIEW      A view based optimal point set.
    *                   QUERY_DENSITY_LIMIT     The query returns a subset of points that best represent the entire point set.
    *                                           The number of points to be returned is specified by the densityValue.
    *
    * @param densityValue IN    A coefficient that modulates the density type. This is applied per region and can be used to
    *                           evenly reduce the density of points retrieved. In the case of QUERY_DENSITY_LIMIT, 
    *                           the density value specifies the maximum number of points to be returned. 
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void SetDensity (QUERY_DENSITY type, float densityValue);
    
    /*---------------------------------------------------------------------------------**//**
    * Gets the detail level for the query allowing quick processing of a view or density based subset of points
    * @see SetDensity
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void GetDensity (QUERY_DENSITY& type, float& densityValue) const;

    /*---------------------------------------------------------------------------------**//**
    * Sets the point retrieval mode
    * @param ignore IN true to ignore any transformation to the points. The points will be returned in their native unit (meters).
    *                  By default, the query will always returned points they have been UOR transformed.
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void SetIgnoreTransform (bool ignore);

    /*---------------------------------------------------------------------------------**//**
    * Gets the point retrieval mode
    * @return  returns true to ignore any transformation to the points. The points will be returned in their native unit (meters).
    *                  By default, the query will always returned points they have been UOR transformed.
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT bool GetIgnoreTransform () const;

    /*---------------------------------------------------------------------------------**//**
    * Gets the point retrieval mode
    * @see  SetMode
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void GetMode (QUERY_MODE& mode, int& viewIndex) const;

    /*---------------------------------------------------------------------------------**//**
    * Retrieves query point geometry and optionally rgb, intensity and selection channels
    * into one or more buffers. If the buffers are filled by the retrieval the function returns.
    * To get the remaining points the function should be called until it returns 0 points
    * 
    * @param pPointCloudChannelBuffers IN the query buffer to retrieve point
    *
    * @return   The number of points written to the query buffer by this iteration.
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT unsigned int GetPoints (IPointCloudQueryBuffersR pPointCloudChannelBuffers);

    /*---------------------------------------------------------------------------------**//**
    * Fit a cylinder to points that would be returned by the query. 
    * Note that the queries density settings will be used to extract the points
    * 
    * @param axis OUT cylinder's axis
    * @param base OUT point on cylinder's base
    * @param radius OUT cylinder's radius
    * @param height OUT cylinder's height
    * @param constrainToAxis IN Set to true if the cylinder.axis is a constraint
    * @param constrainToRadius IN Set to true if the cylinder.radius is a constraint
    *
    * @return   Root mean square (rms).
    * @bsimethod  
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT double GetFitCylinder (DVec3dR axis, DPoint3dR base, double& radius, double& height, bool constrainToAxis, bool constrainToRadius);

    /*---------------------------------------------------------------------------------**//**
    * Fit a planar rectangle to that would be returned by the query. 
    * Note that the queries density settings will be used to extract the points
    * 
    * @param corners OUT The resulting 4 corners of the planar rectangle.
    * @param constrainToNormal IN Set to true to constrain to the normal
    *
    * @return   Root mean square (rms).
    * @bsimethod  
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT double GetFitPlanarRectangle (DPoint3d corners[4], bool constrainToNormal);

    /*---------------------------------------------------------------------------------**//**
    * Fit a plane to that would be returned by the query. 
    * Note that the queries density settings will be used to extract the points
    * 
    * @param planeNormal OUT The resulting 4 corners of the planar rectangle.
    * @param planeOrigin OUT The origin of the plane as point
    * @param constrainToNormal IN The normal of the plane will be constrained to the planeNormal
    *
    * @return   measure of error (rms).
    * @bsimethod  
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT double GetFitPlane (DVec3dR planeNormal, DPoint3dR planeOrigin, bool constrainToNormal);

    /*---------------------------------------------------------------------------------**//**
    * Submits changes made to point channel buffer to the internal user data structures. This function should be 
    * called after each call to IPointCloudDataQuery::GetPoints whenever the channel buffer is changed
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT StatusInt SubmitUpdate (IPointCloudChannelPtr pChannel);

    /*---------------------------------------------------------------------------------**//**
    * Returns the transform from point UOR to the native POD point value
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT TransformR GetUORToNativeTransform (TransformR trans);

    /*---------------------------------------------------------------------------------**//**
    * Resets the query to its creation state.
    * Once a query has returned all its points it will not return more points until Reset is called.
    *
    * @bsimethod                                    
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void Reset ();
};

//@}

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
