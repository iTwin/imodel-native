/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudDataQuery.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BePointCloudInternal.h"

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudDataQuery : public RefCounted<IPointCloudDataQuery>
    {
    public:
        enum QueryType
            {
            Visible = 0,
            Selected,
            };

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  07/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
        static PointCloudDataQuery* Create(ElementHandleCR eh, DPoint3dCR origin, DPoint3dCR corner) { return new PointCloudDataQuery(eh, origin, corner); }
        static PointCloudDataQuery* Create(ElementHandleCR eh, DPoint3dCR center, double radius) { return new PointCloudDataQuery(eh, center, radius); }
        static PointCloudDataQuery* Create(ElementHandleCR eh, OrientedBoxCR box) { return new PointCloudDataQuery(eh, box); }
        static PointCloudDataQuery* Create(ElementHandleCR eh, QueryType type) { return new PointCloudDataQuery(eh, type); }
*/

    protected:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _Reset () override
            {
            ptResetQuery (m_queryHandle->GetHandle());
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
        void CreateFromBox (OrientedBoxCR clipBox)
            {
            Transform inverseTrans;
            inverseTrans.inverseOf (&m_trans); 

            DPoint3d localOrigin;
            inverseTrans.multiply(&localOrigin, &clipBox.GetOrigin (), 1);
            DVec3d localXVec;
            inverseTrans.multiplyMatrixOnly(&localXVec, &clipBox.GetXVec ());
            DVec3d localYVec;
            inverseTrans.multiplyMatrixOnly(&localYVec, &clipBox.GetYVec ());
            DVec3d localZVec;
            inverseTrans.multiplyMatrixOnly(&localZVec, &clipBox.GetZVec ());

            // build a rotation matrix
            DVec3d u = localXVec;
            u.normalize();
            DVec3d v = localYVec;
            v.normalize();
            DVec3d w;
            w.crossProduct(&u, &v);

            RotMatrix boxRotation;
            boxRotation.initFromColumnVectors(&u, &v, &w);

            DVec3d box[2];
            box[0].init(0.0, 0.0, 0.0);
            box[1].sumOf(&localXVec, &localYVec);
            box[1].sumOf(&box[1], &localZVec);

            // unrotate
            boxRotation.multiplyTranspose(&box[1], &box[1]);

            // find min max of the clip box
            double boxLower[3], boxUpper[3];
            boxLower[0] = std::min(box[0].x, box[1].x);
            boxLower[1] = std::min(box[0].y, box[1].y);
            boxLower[2] = std::min(box[0].z, box[1].z);

            boxUpper[0] = std::max(box[0].x, box[1].x);
            boxUpper[1] = std::max(box[0].y, box[1].y);
            boxUpper[2] = std::max(box[0].z, box[1].z);

            PThandle handle = ptCreateOrientedBoundingBoxQuery (
                boxLower[0], boxLower[1], boxLower[2], 
                boxUpper[0], boxUpper[1], boxUpper[2],  
                localOrigin.x, localOrigin.y, localOrigin.z,
                u.x, u.y, u.z, v.x, v.y, v.z);

            m_queryHandle = PointCloudQueryHandle::Create (handle);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual IPointCloudQueryBuffersPtr  _CreateBuffers(uint32_t capacity, uint32_t channelFlags, IPointCloudChannelVectorCR inChannels) override
            {
            channelFlags = m_caller->CombineFlags(channelFlags);

            // loop on all channels that implement onQuery, add them if not already in inChannels
            PointCloudChannelVector onQueryChannels = m_caller->GetChannelVector();


            for (IPointCloudChannelVector::const_iterator itr (inChannels.begin()); itr != inChannels.end(); ++itr)
                {
                bool found = false;
                //try to find it
                for (PointCloudChannelVector::const_iterator itr2 (onQueryChannels.begin()); !found && itr2 != onQueryChannels.end(); ++itr2)
                    {
                    if ((*itr2)->GetPersistentName () == (*itr)->GetPersistentName ())
                        found = true;
                    }
                if (!found)
                    onQueryChannels.push_back (dynamic_cast<PointCloudChannelP>(*itr));
                }

            return PointCloudQueryBuffers::Create(capacity, channelFlags, onQueryChannels);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual unsigned int _GetPoints (IPointCloudQueryBuffersR pPointCloudChannelBuffers) override
            {
            PointCloudQueryBuffersP queryBuffers = dynamic_cast<PointCloudQueryBuffersP>(&pPointCloudChannelBuffers);

            // we need to get the classification to apply it even if the caller is not interested in filling the buffer
            unsigned int numberPoints = queryBuffers->GetPoints(m_queryHandle->GetHandle());

            if (!m_ignoreTransform)
                m_trans.multiply (queryBuffers->GetXyzBuffer(), numberPoints);

            if (numberPoints && m_caller.IsValid())
                m_caller->Call (*queryBuffers);

            return numberPoints;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   09/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        double _GetFitCylinder (DVec3dR axis, DPoint3dR base, double& radius, double& height, bool constrainToAxis, bool constrainToRadius)
            {
/* POINTCLOUD_WIP_PtFeatureExtract
            double res = PtFeatureExtract::FitCylinderToPoints (m_queryHandle->GetHandle (), axis, base, radius, height, constrainToAxis, constrainToRadius);
*/
double res = 0;
            
            DVec3d heightPt; heightPt.init (height,0,0);
            m_trans.multiplyMatrixOnly (&heightPt, &heightPt);
            height = heightPt.magnitude ();

            DVec3d radPt; radPt.init (radius,0,0);
            m_trans.multiplyMatrixOnly (&radPt, &radPt);
            radius = radPt.magnitude ();


            m_trans.multiply (&base, &base, 1);
            return res;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   09/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        double _GetFitPlanarRectangle (DPoint3d corners[4], bool constrainToNormal)
            {
// POINTCLOUD_WIP_PtFeatureExtract  double res = PtFeatureExtract::FitPlanarRectangleToPoints (m_queryHandle->GetHandle (), corners, constrainToNormal);
double res = 0;

            m_trans.multiply (corners, corners, 4);
            return res;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   09/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        double  _GetFitPlane (DVec3dR planeNormal, DPoint3dR planeOrigin, bool constrainToNormal)
            {
// POINTCLOUD_WIP_PtFeatureExtract            double res =  PtFeatureExtract::FitPlaneToPoints (m_queryHandle->GetHandle (), planeNormal, planeOrigin, constrainToNormal);
double res = 0;

            m_trans.multiplyMatrixOnly (&planeNormal, &planeNormal);
            m_trans.multiply (&planeOrigin, &planeOrigin);

            return res;
            }


        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void _SetChannelHandlerFilter(IPointCloudChannelHandlerFilterP pFilter) 
            {
            m_caller->SetChannelHandlerFilter(pFilter);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void _GetAvailableChannelFlags (uint32_t& channelFlags) const
            {
/* POINTCLOUD_WIP_GR06 - This is probably useless in graphite06
            channelFlags= (uint32_t)PointCloudChannelId::Xyz;
            IPointCloudFileQueryPtr pPCFile = IPointCloudFileQuery::CreateFileQuery(m_eh);

            if (pPCFile == NULL)
                return;

            if (pPCFile->HasRGBChannel ())
                channelFlags |= (uint32_t)PointCloudChannelId::Rgb;
            if (pPCFile->HasClassificationChannel ())
                channelFlags |= (uint32_t)PointCloudChannelId::Classification;
            if (pPCFile->HasNormalChannel ())
                channelFlags |= (uint32_t)PointCloudChannelId::Normal;
            if (pPCFile->HasIntensityChannel ())
                channelFlags |= (uint32_t)PointCloudChannelId::Intensity;

            m_caller->AddModifyingChannelFlags(channelFlags);
*/
throw;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void _GetChannelHandlers (PointCloudChannelHandlers& handlers) const
            {
            m_caller->GetChannelHandlers(handlers);
            }


        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _SetDensity (QUERY_DENSITY type, float densityValue) override
            {
            m_densityType = type;
            m_densityValue = densityValue;

            ptSetQueryDensity (m_queryHandle->GetHandle(), m_densityType, m_densityValue);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _GetDensity (QUERY_DENSITY& type, float& densityValue) const override
            {
            type = m_densityType;
            densityValue = m_densityValue;
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   12/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _SetIgnoreTransform (bool ignore) override
            {
            m_ignoreTransform = ignore;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _GetIgnoreTransform () const  override {return m_ignoreTransform;}


        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   02/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _GetMode  (QUERY_MODE& mode, int& viewNum) const override
            {
            mode = m_mode;
            viewNum = m_viewNum;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   07/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
        StatusInt _SubmitUpdate (IPointCloudChannelPtr channelPtr)
            {
            PointCloudChannel* pChannel (static_cast<PointCloudChannel*>(channelPtr.get()));

            try
                {
                if (NULL != pChannel &&
                    PTV_SUCCESS == ptSubmitPointChannelUpdate (m_queryHandle->GetHandle (), pChannel->GetHandle ()))
                    return SUCCESS;

                }
            catch (...)
                {
                return ERROR;
                }

            return ERROR;
            }


        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual  TransformR _GetUORToNativeTransform (TransformR trans)
            {
            trans.inverseOf (&m_trans);
            return trans;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Daniel.McKenzie                 09/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_Reprojection
        virtual StatusInt _ReprojectUOR  (EditElementHandleR eeh, DPoint3dP pt, int nPoints, DgnModelP modelRef)  override
            {
            StatusInt returnCode;

            //NEW EXPORT
            DgnGCSPtr pSrcMstnGcs = DgnGCS::CreateGCS(modelRef);
            DgnGCSPtr pFinalMstnDestGcs = DgnGCS::CreateGCS(modelRef);

            if (PointCloudMstnGCSData::GetInstance()->GetSourceGCSMstn() == NULL)
                return ERROR;           

            if (PointCloudMstnGCSData::GetInstance()->GetDestinationGCSMstn() == NULL)
                return ERROR;

            pSrcMstnGcs = PointCloudMstnGCSData::GetInstance()->GetSourceGCSMstn()->GetMstnGCSP();
            pFinalMstnDestGcs = PointCloudMstnGCSData::GetInstance()->GetDestinationGCSMstn()->GetMstnGCSP();

            if (pSrcMstnGcs == NULL)
                return ERROR;

            if (!pFinalMstnDestGcs->IsValid())
                return ERROR;

            DPoint3d* outUorsDest = new DPoint3d[nPoints];

            returnCode = pSrcMstnGcs->ReprojectUors(&outUorsDest[0], NULL, NULL, &pt[0], nPoints, *pFinalMstnDestGcs.get());

            if (returnCode != SUCCESS) //Convert the points
                return returnCode;

            memcpy(&pt[0], &outUorsDest[0], nPoints * sizeof(DPoint3d));

            delete []outUorsDest;
            return SUCCESS;
            }
*/

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
        PointCloudDataQuery (ElementHandleCR eh, DPoint3dCR origin, DPoint3dCR corner)
            : m_eh (eh)
            {
            Initialize ();

            Transform inverseTrans;
            inverseTrans.inverseOf (&m_trans);

            DPoint3d localOrigin;
            inverseTrans.multiply (&localOrigin, &origin);

            DPoint3d localCorner;
            inverseTrans.multiply (&localCorner, &corner);

            DPoint3d lower;
            lower.x = std::min(localCorner.x, localOrigin.x);
            lower.y = std::min(localCorner.y, localOrigin.y);
            lower.z = std::min(localCorner.z, localOrigin.z);

            DPoint3d upper;
            upper.x = std::max(localCorner.x, localOrigin.x);
            upper.y = std::max(localCorner.y, localOrigin.y);
            upper.z = std::max(localCorner.z, localOrigin.z);

            PThandle handle = ptCreateBoundingBoxQuery (lower.x, lower.y, lower.z, upper.x, upper.y, upper.z);
            m_queryHandle = PointCloudQueryHandle::Create(handle);

            SetDefaultQuerySettings ();
            }
*/

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
        PointCloudDataQuery (ElementHandleCR eh, DPoint3dCR center, double radius)
            : m_eh (eh)
            {
            Initialize ();

            Transform inverseTrans;
            inverseTrans.inverseOf (&m_trans);

            DPoint3d localCenter;
            inverseTrans.multiply (&localCenter, &center);

            m_queryHandle = PointCloudQueryHandle::Create (ptCreateBoundingSphereQuery ((double *)&localCenter, radius / UOR_PER_METER));

            SetDefaultQuerySettings ();
            }
*/

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2010
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
        PointCloudDataQuery (ElementHandleCR eh, OrientedBoxCR box)
            :m_eh (eh)
            {
            Initialize ();

            CreateFromBox (box);

            SetDefaultQuerySettings ();
            }
*/

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
        PointCloudDataQuery (ElementHandleCR eh, QueryType type)
            : m_eh (eh)
            {
            Initialize ();

            if (type == Visible)
                m_queryHandle = PointCloudQueryHandle::Create (ptCreateVisPointsQuery ());
            else
                m_queryHandle = PointCloudQueryHandle::Create (ptCreateSelPointsQuery ());

            SetDefaultQuerySettings ();
            }
*/

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void Initialize ()
            {
            }

        /*---------------------------------------------------------------------------------**//**
        * Sets standard options in the query handle
        * @bsimethod                                    Simon.Normand                   10/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetDefaultQuerySettings ()
            {

            m_ignoreTransform = false;
            // always return the points using their color, not as they are displayed (filtered)
            ptSetQueryRGBMode (m_queryHandle->GetHandle(), PT_QUERY_RGB_MODE_ACTUAL);

            // limit query to this element
            ptSetQueryScope (m_queryHandle->GetHandle(), m_pScene != NULL ? m_pScene->GetSceneHandle () : 0);

            // default is to always select points at full density
            _SetDensity (QUERY_DENSITY_FULL, 1.0f);
            }

    private:

    private:
        RefCountedPtr<PointCloudQueryHandle>    m_queryHandle;
        Transform                                   m_trans;

/* POINTCLOUD_WIP_GR06_ElementHandle
        ElementHandleCR                              m_eh;
*/

        bvector<unsigned char>                       m_classificationChannel;
        bvector<void*>                       m_channelBuffer;  //this needs to be a member for pointools to work
        
        RefCountedPtr<PointCloudScene>           m_pScene;
        bool                                     m_ignoreTransform;
        QUERY_DENSITY                            m_densityType;
        float                                    m_densityValue;
        QUERY_MODE                               m_mode;
        int                                      m_viewNum;
        ChannelHandlerOnQueryCallerPtr           m_caller;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudDataQueryPtr IPointCloudDataQuery::CreateBoundingBoxQuery(ElementHandleCR eh, DPoint3dCR origin, DPoint3dCR corner)
    {
    return PointCloudDataQuery::Create (eh, origin, corner);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudDataQueryPtr IPointCloudDataQuery::CreateOrientedBoxQuery(ElementHandleCR eh, OrientedBoxCR box)
    {
    return PointCloudDataQuery::Create (eh, box);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudDataQueryPtr IPointCloudDataQuery::CreateBoundingSphereQuery(ElementHandleCR eh, DPoint3dCR center, double radius)
    {
    return PointCloudDataQuery::Create (eh, center, radius);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudDataQueryPtr IPointCloudDataQuery::CreateVisiblePointsQuery(ElementHandleCR eh)
    {
    return PointCloudDataQuery::Create (eh, PointCloudDataQuery::Visible);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudDataQueryPtr IPointCloudDataQuery::CreateSelectedPointsQuery(ElementHandleCR eh)
    {
    return PointCloudDataQuery::Create (eh, PointCloudDataQuery::Selected);
    }




/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
void        IPointCloudDataQuery::Reset () {_Reset ();}
unsigned int IPointCloudDataQuery::GetPoints (IPointCloudQueryBuffersR pPointCloudChannelBuffers) { return _GetPoints(pPointCloudChannelBuffers); }
void        IPointCloudDataQuery::SetDensity (QUERY_DENSITY type, float densityValue) {_SetDensity (type, densityValue);}
void        IPointCloudDataQuery::GetMode (QUERY_MODE& mode, int& viewNum) const {_GetMode (mode, viewNum);}
void        IPointCloudDataQuery::GetDensity (QUERY_DENSITY& type, float& densityValue) const {_GetDensity (type, densityValue);}
bool        IPointCloudDataQuery::GetIgnoreTransform () const {return _GetIgnoreTransform ();}
void        IPointCloudDataQuery::SetIgnoreTransform (bool ignore) {_SetIgnoreTransform (ignore);}
StatusInt   IPointCloudDataQuery::SubmitUpdate (IPointCloudChannelPtr pChannel) {return _SubmitUpdate (pChannel);}
TransformR  IPointCloudDataQuery::GetUORToNativeTransform (TransformR trans)  {return _GetUORToNativeTransform  (trans);}
double      IPointCloudDataQuery::GetFitPlanarRectangle (DPoint3d corners[4], bool constrainToNormal) {return _GetFitPlanarRectangle  (corners, constrainToNormal);}
double      IPointCloudDataQuery::GetFitPlane (DVec3dR planeNormal, DPoint3dR planeOrigin, bool constrainToNormal) {return _GetFitPlane (planeNormal, planeOrigin, constrainToNormal);}
double      IPointCloudDataQuery::GetFitCylinder (DVec3dR axis, DPoint3dR base, double& radius, double& height, bool constrainToAxis, bool constrainToRadius) {return _GetFitCylinder (axis, base, radius, height, constrainToAxis, constrainToRadius);}
StatusInt   IPointCloudDataQuery::ReprojectUOR  (EditElementHandleR eeh, DPoint3dP pt, int nPoints, DgnModelP modelRef) { return _ReprojectUOR(eeh, pt, nPoints, modelRef); }
IPointCloudQueryBuffersPtr  IPointCloudDataQuery::CreateBuffers(uint32_t capacity, uint32_t channelFlags, IPointCloudChannelVectorCR channels) {return _CreateBuffers (capacity, channelFlags, channels);}
void        IPointCloudDataQuery::SetChannelHandlerFilter(IPointCloudChannelHandlerFilterP pFilter) {_SetChannelHandlerFilter (pFilter);}
void        IPointCloudDataQuery::GetAvailableChannelFlags(uint32_t& channelFlags) const {_GetAvailableChannelFlags (channelFlags);}
void        IPointCloudDataQuery::GetChannelHandlers(PointCloudChannelHandlers& handlers) const {_GetChannelHandlers (handlers);}
