/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMTriangleDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

#include <ScalableTerrainModel/MrDTMUtilityFunctions.h>
#include "MrDTMDataRef.h"
#include <TerrainModel\ElementHandler\IMrDTMProgressiveDisplay.h>
#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>
#include <TerrainModel\Core\DTMIterators.h>

USING_NAMESPACE_RASTER

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForCacheTriangles : IDTMStrokeForCache
    {
    private:
        bool m_doRegions;
        DTMUserTag m_tag;
        BcDTMP m_dtmElement;
        ViewContextP m_context;
        ElementHandleCP m_el;
        BC_DTM_OBJ* m_tinP;
        DTMDrawingInfo& m_drawingInfo;

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        static int LoadFunc (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP);
    protected:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        int Load (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts);
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        void DisplayRegions(DTMUserTag userTag, DTMFeatureId id);

    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMStrokeForCacheTriangles(BcDTMP DTMDataRefXAttribute, DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
            {
            m_dtmElement = DTMDataRefXAttribute;
            m_doRegions = false;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMStrokeForCacheTriangles(BcDTMP DTMDataRefXAttribute, DTMUserTag tag, DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
            {
            m_dtmElement = DTMDataRefXAttribute;
            m_doRegions = true;
            m_tag = tag;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //
        // Strokes the DTM for the cache
        //
        //=======================================================================================
        void _StrokeForCache (ElementHandleCR element, ViewContextR context, double pixelSize)
            {
            if (context.CheckStop())
                return;

            m_context = &context;
            m_el = &element;

            // Get the DTM element
            if (m_dtmElement == nullptr)
                return;

            // Get the unmanaged handle....
            BcDTMP bcDTM = m_dtmElement;

            m_tinP = (BC_DTM_OBJ*)bcDTM->GetTinHandle();

            // Push the transformation matrix to transform the coordinates to UORS.
            DrawSentinel    sentinel (context, m_drawingInfo);
            bcDTM->BrowseFeatures(m_doRegions ? DTMFeatureType::Region : DTMFeatureType::Triangle, m_drawingInfo.GetFence() /* DTMFenceOption::Overlap */, 10000, this, LoadFunc);
            }

    public: static BC_DTM_OBJ* tinP;
    };

BC_DTM_OBJ* DTMStrokeForCacheTriangles::tinP = NULL;

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMStrokeForCacheTriangles::LoadFunc(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP)
    {
    DTMStrokeForCacheTriangles* cache = (DTMStrokeForCacheTriangles*)userP;
    return cache->Load(dtmFeatureType, userTag, id, featurePtsP, numFeaturePts);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMStrokeForCacheTriangles::Load (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts)
    {
    if (m_context->CheckStop())
        return ERROR;

    if (dtmFeatureType == DTMFeatureType::Region)
        {
        DisplayRegions(userTag, id);
        }
    else
        m_context->DrawStyledLineString3d ((int)numFeaturePts, (DPoint3d*)featurePtsP, nullptr);
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMStrokeForCacheTriangles::DisplayRegions(DTMUserTag userTag, DTMFeatureId id)
    {
    if (userTag == m_tag)
       {
        bcdtmLoad_trianglesFromRegionDtmObject(m_tinP, id, LoadFunc, this);
        }
    }

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForCacheShadedTriangles : IDTMStrokeForCache
    {
    private:
        BcDTMP        m_dtmElement;
        ViewContextP  m_context;
        ElementHandleCP   m_el;
        BC_DTM_OBJ*    m_tinP;
        bool           m_doRegions;
        DTMUserTag   m_tag;
        ::DPoint3d*    m_fencePtsP;
        int            m_nbFencePts;
        bool           m_forTiling;
        DTMDrawingInfo& m_drawingInfo;
        //For draping high resolution rasters
        DTMFeatureId     m_textureRegionFeatureId;
        unsigned int       m_nbPointsDrawn;

    private:

        struct UserDataDuringDTMDataLoad
            {
            ViewContextP m_viewContextP;
            unsigned int* m_nbPointsDrawnP;
            };

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        static int LoadFuncTexturing (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, long numFeaturePts,void *userP);

        static int LoadFunc (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP);
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        static int draw(DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP);

        static int drawForTexturing(DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP,void *userP);

    protected:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        int Load (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts);
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        void DisplayRegions(DTMUserTag userTag, DTMFeatureId id);

    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMStrokeForCacheShadedTriangles(BcDTMP        DTMDataRefXAttribute,
                                         DTMDrawingInfo& drawingInfo,
                                         DPoint3d*           fencePts = 0,
                                         int            nbFencePt = 0
                                         ) : m_drawingInfo(drawingInfo)

            {
            m_dtmElement             = DTMDataRefXAttribute;
            m_textureRegionFeatureId = DTMDataRefXAttribute->GetTinHandle()->nullFeatureId;
            m_doRegions              = false;
            m_fencePtsP              = (DPoint3d*)fencePts;
            m_nbFencePts             = nbFencePt;
            m_forTiling              = false;
            m_nbPointsDrawn          = 0;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMStrokeForCacheShadedTriangles(BcDTMP DTMDataRefXAttribute, DTMUserTag tag, DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
            {
            m_dtmElement = DTMDataRefXAttribute;
            m_doRegions = true;
            m_forTiling = false;
            m_fencePtsP = nullptr;
            m_nbFencePts = 0;
            m_tag = tag;
            m_nbPointsDrawn = 0;
            }

        virtual ~DTMStrokeForCacheShadedTriangles()
            {
            if (m_forTiling)
                delete [] m_fencePtsP;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        void SetTextureRegionFeatureId(DTMFeatureId pi_textureRegionFeatureId)
            {
            m_textureRegionFeatureId = pi_textureRegionFeatureId;
            }

        unsigned int GetNbPointsDrawn()
            {
            return m_nbPointsDrawn;
            }

        virtual bool SupportTiling() override
            {
            return !m_doRegions;
            }

        virtual void SetTileFence(DPoint3d& lowPt, DPoint3d& highPt) override
            {
            if (m_forTiling)
                delete [] m_fencePtsP;
            m_forTiling = true;
            m_nbFencePts = 5;
            m_fencePtsP = new DPoint3d[5];
            m_fencePtsP[0] = m_fencePtsP[4] = lowPt;
            m_fencePtsP[1].x = lowPt.x; m_fencePtsP[1].y = highPt.y; m_fencePtsP[1].z = 0;
            m_fencePtsP[2] = highPt;
            m_fencePtsP[3].x = highPt.x; m_fencePtsP[3].y = lowPt.y; m_fencePtsP[3].z = 0;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //
        // Strokes the DTM for the cache
        //
        //=======================================================================================
        void _StrokeForCache (ElementHandleCR element, ViewContextR context, double pixelSize)
            {
            m_nbPointsDrawn = 0;

            if (context.CheckStop())
                return;

            // Get the DTM element
            m_context = &context;
            m_el = &element;

            if (m_dtmElement == nullptr)
                return;

            // Get the unmanaged handle....
            BcDTMP bcDTM = m_dtmElement;

            if (bcDTM->GetTinHandle()->numPoints == 0)
                return;


            DPoint3d fencePt[50];
            const DPoint3d* fencePts = fencePt;
            int nbPts;
            DTMFenceType fenceType = DTMFenceType::Block;

            bool isVisible = true;

            RefCountedPtr<DTMDataRef> ref = m_drawingInfo.GetDTMDataRef();

            if (!m_forTiling)
                {
                if (m_fencePtsP == 0)
                    {
                    if (ref != nullptr && ref->IsMrDTM())
                        {
                        ::DPoint3d*     pTempFencePts = 0;
                        DRange3d drange;

                        bcDTM->GetRange(drange);

                        //Sometime the range is 0???
                        if (memcmp(&drange.low, &drange.high, sizeof(drange.low)) != 0)
                            {
                            if (context.GetViewport() != 0)
                                {
                                isVisible = GetVisibleFencePointsFromContext(pTempFencePts, nbPts, &context, m_drawingInfo, drange);

                                if (isVisible)
                                    {
                                    BeAssert(_isnan(pTempFencePts[0].x) == false);
                                    memcpy(fencePt, pTempFencePts, sizeof(DPoint3d) * nbPts);
                                    }
                                }
                            else
                                {
                                fencePt[0].x = drange.low.x;
                                fencePt[0].y = drange.low.y;
                                fencePt[0].z = 0;
                                fencePt[1].x = drange.high.x;
                                fencePt[1].y = drange.low.y;
                                fencePt[1].z = 0;
                                fencePt[2].x = drange.high.x;
                                fencePt[2].y = drange.high.y;
                                fencePt[2].z = 0;
                                fencePt[3].x = drange.low.x;
                                fencePt[3].y = drange.high.y;
                                fencePt[3].z = 0;
                                fencePt[4].x = drange.low.x;
                                fencePt[4].y = drange.low.y;
                                fencePt[4].z = 0;
                                nbPts = 5;
                                isVisible = true;
                                }
                            }
                        else
                            {
                            isVisible = false;
                            }

                        delete [] pTempFencePts;
                        }
                    else
                        {
                        isVisible = true;
                        fencePts = m_drawingInfo.GetFence().points;
                        nbPts = m_drawingInfo.GetFence().numPoints;
                        fenceType = m_drawingInfo.GetFence().fenceType;
                        }
                    }
                else
                    {
                    isVisible = true;
                    nbPts = m_nbFencePts;
                    const Transform& trsf = *m_drawingInfo.GetStorageToUORTransformation();

                    Transform invertTrsf;

                    bool invert = bsiTransform_invertTransform(&invertTrsf, &trsf);

                    BeAssert(invert != 0);

                    bsiTransform_multiplyDPoint3dArray(&invertTrsf, fencePt, m_fencePtsP, nbPts);

                    }
                }
            else
                {
                isVisible = true;
                nbPts = m_nbFencePts;
                fencePts = m_fencePtsP;
                }

            if (isVisible)
                {
                m_tinP = (BC_DTM_OBJ*)bcDTM->GetTinHandle();

                UserDataDuringDTMDataLoad userData;

                userData.m_viewContextP = &context;
                userData.m_nbPointsDrawnP = &m_nbPointsDrawn;

                // Push the transformation matrix to transform the coordinates to UORS.
                DrawSentinel    sentinel (context, m_drawingInfo);

                if ( context.GetDrawPurpose() == DrawPurpose::Measure )
                    {
                    bcdtmInterruptLoad_triangleShadeMeshFromDtmObject (m_tinP, 65000,2,1,&draw, fencePts != nullptr, fenceType, DTMFenceOption::Overlap, (DPoint3d*)fencePts, nbPts, &userData);
                    }
                else if (m_doRegions)   //Display all regions
                    {
                    bcDTM->BrowseFeatures(DTMFeatureType::Region, DTMFenceParams(fenceType, DTMFenceOption::Overlap, (DPoint3d*)fencePts, nbPts), 10000, this, LoadFunc);
                    }
                else
                    {
                    if (m_textureRegionFeatureId == m_dtmElement->GetTinHandle()->nullFeatureId)
                        {
                        if (m_forTiling)
                            {
                            //bcdtmInterruptLoad_triangleShadeMeshForQVCacheFromDtmObject (m_tinP, 126000 / 3, 2, 1, &draw, true, fenceType, DTMFenceOption::Overlap, (DPoint3d*)fencePts, nbPts, &userData);
                                {
                                DTMFenceParams fence (fenceType, DTMFenceOption::Overlap, (DPoint3d*)fencePts, nbPts);
                                DTMMeshEnumerator en (*bcDTM);
                                en.SetFence (fence);
                                en.SetMaxTriangles (126000 / 3);
                                for (PolyfaceQueryP info : en)
                                    {
                                    context.GetIDrawGeom ().DrawPolyface (*info);
                                    m_nbPointsDrawn += (int)info->GetPointCount ();
                                    }
                                }
                            }
                        else
                            bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(m_tinP, 65000,2,1,&draw, fencePts != nullptr, fenceType, DTMFenceOption::Overlap, (DPoint3d*)fencePts, nbPts, &userData);
                        }
                    else
                        {
                        long maxTriangles = 65000;  // Maximum Triangles To Pass Back
                        long vectorOption = 2;  // Averaged Triangle Surface Normals
                        double zAxisFactor = 1.0;  // Am\ount To Exaggerate Z Axis
                        long regionOption = 1;  // Include Internal Regions Until Fully Tested
                        long indexOption = 2;  // Use Feature Id

                        bcdtmLoad_triangleShadeMeshForRegionDtmObject(m_tinP,
                                                                      maxTriangles,
                                                                      vectorOption,
                                                                      zAxisFactor,
                                                                      regionOption,
                                                                      indexOption,
                                                                      m_textureRegionFeatureId,
                                                                      &drawForTexturing,
                                                                      &userData);
                        }
                    }
                }
            }
    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMStrokeForCacheShadedTriangles::draw(DTMFeatureType dtmFeatureType,int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP)
    {
    UserDataDuringDTMDataLoad* userDataP = (UserDataDuringDTMDataLoad*)userP;

    ViewContextP context = userDataP->m_viewContextP;
    UInt32 numPerFace = 3;
    bool   twoSided = false;
    size_t indexCount = numMeshFaces;
    size_t pointCount = numMeshPts;
    DPoint3dCP pPoint = meshPtsP;
    Int32 const* pPointIndex = (Int32*)meshFacesP;
    size_t normalCount = numMeshPts;
    DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
    Int32 const* pNormalIndex = (Int32*)meshFacesP;

    PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex, normalCount, pNormal, pNormalIndex);
    context->GetIDrawGeom().DrawPolyface (polyCarrier);

    *(userDataP->m_nbPointsDrawnP) += numMeshPts;
    return SUCCESS;
    }

int DTMStrokeForCacheShadedTriangles::drawForTexturing(DTMFeatureType dtmFeatureType,int numTriangles,int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP)
    {
    UserDataDuringDTMDataLoad* userDataP = (UserDataDuringDTMDataLoad*)userP;

    ViewContextP context = userDataP->m_viewContextP;

    UInt32 numPerFace = 3;
    bool   twoSided = false;
    size_t indexCount = numMeshFaces;
    size_t pointCount = numMeshPts;
    DPoint3dCP pPoint = (DPoint3dCP)meshPtsP;
    Int32 const* pPointIndex = (Int32*)meshFacesP;
    size_t normalCount = numMeshPts;
    DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
    Int32 const* pNormalIndex = (Int32*)meshFacesP;

    PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex, normalCount, pNormal, pNormalIndex);
    context->GetIDrawGeom().DrawPolyface (polyCarrier);
    *(userDataP->m_nbPointsDrawnP) += numMeshPts;

    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMStrokeForCacheShadedTriangles::LoadFunc(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP)
    {
    DTMStrokeForCacheShadedTriangles* cache = (DTMStrokeForCacheShadedTriangles*)userP;
    cache->m_nbPointsDrawn += (unsigned int)numFeaturePts;
    return cache->Load (dtmFeatureType, userTag, id, featurePtsP, numFeaturePts);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMStrokeForCacheShadedTriangles::Load (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts)
    {

    if (m_context->CheckStop())
        return ERROR;

    if (dtmFeatureType == DTMFeatureType::Region)
        {
        DisplayRegions(userTag, id);
        }
    else
        {
        int meshFacesP[3] = {1,2,3};
        UInt32 numPerFace = 3;
        bool   twoSided = false;
        size_t indexCount = 3;
        size_t pointCount = 3;
        DPoint3dCP pPoint = featurePtsP;
        Int32 const* pPointIndex = meshFacesP;

        PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex);
        m_context->GetIDrawGeom().DrawPolyface (polyCarrier);
        }
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMStrokeForCacheShadedTriangles::DisplayRegions(DTMUserTag userTag, DTMFeatureId id)
    {
    if (userTag == m_tag)
        {
        bcdtmLoad_trianglesFromRegionDtmObject(m_tinP, id, LoadFunc, this);
        }
    }


//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 06/11
//=======================================================================================
struct PickDraw
    {
    ViewContextP context;
    bool drawnSomething;
    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
static int DrawPickTriangles(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP)
    {
    PickDraw* pd = (PickDraw*)userP;
    if(pd->context->GetViewFlags()->fill)
        pd->context->GetIDrawGeom().DrawShape3d ((int)numFeaturePts, featurePtsP, true, nullptr);
    else
        pd->context->DrawStyledLineString3d ((int)numFeaturePts, featurePtsP, nullptr);
    pd->drawnSomething = true;
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                           Mathieu.St-Pierre
//=======================================================================================
class MrDTMProgressiveStroker : public IMrDTMProgressiveStroker
    {
    private :

        DTMStrokeForCacheShadedTriangles* m_strokeForCacheShadedTrianglesP;

    protected :

        virtual unsigned int _GetNbPointsStroken()
            {
            BeAssert(m_strokeForCacheShadedTrianglesP != 0);
            return m_strokeForCacheShadedTrianglesP->GetNbPointsDrawn();
            }

    public :

        MrDTMProgressiveStroker(DTMStrokeForCacheShadedTriangles* strokeForCacheShadedTrianglesP)
            {
            BeAssert(strokeForCacheShadedTrianglesP != 0);
            m_strokeForCacheShadedTrianglesP = strokeForCacheShadedTrianglesP;
            }

        ~MrDTMProgressiveStroker()
            {
            BeAssert(m_strokeForCacheShadedTrianglesP != 0);
            delete m_strokeForCacheShadedTrianglesP;
            }

        virtual void _StrokeForCache(ElementHandleCR elIter, ViewContextR context, double pixelSize = 0.0)
            {
            BeAssert(m_strokeForCacheShadedTrianglesP != 0);
            m_strokeForCacheShadedTrianglesP->_StrokeForCache(elIter, context, pixelSize);
            }
    };


//=======================================================================================
// @bsimethod                                                           Mathieu.St-Pierre
//=======================================================================================
class MrDTMProgressiveStrokerManager : public IMrDTMProgressiveStrokerManager
    {
    private :

        static IMrDTMProgressiveStrokerManagerPtr m_mrDTMProgressiveStrokerManager;

        explicit MrDTMProgressiveStrokerManager()
            {
            }

        ~MrDTMProgressiveStrokerManager()
            {
            }

    protected :

        virtual int _CreateStroker(IMrDTMProgressiveStrokerPtr& strokerForCache, BcDTMP dtm,  struct DTMDrawingInfo* dtmDrawingIfo, DPoint3d* strokingExtentPts, int nbStrokingExtentPts, DTMFeatureId textureRegionFeatureId)
            {
            DTMStrokeForCacheShadedTriangles* strokeForCacheShadedTrianglesP(new DTMStrokeForCacheShadedTriangles(dtm, *dtmDrawingIfo, strokingExtentPts, nbStrokingExtentPts));

            strokeForCacheShadedTrianglesP->SetTextureRegionFeatureId(textureRegionFeatureId);

            strokerForCache = new MrDTMProgressiveStroker(strokeForCacheShadedTrianglesP);

            return SUCCESS;
            }

    public :

        static IMrDTMProgressiveStrokerManagerPtr GetInstance()
            {
            if (m_mrDTMProgressiveStrokerManager == 0)
                {
                m_mrDTMProgressiveStrokerManager = new MrDTMProgressiveStrokerManager;
                }

            return m_mrDTMProgressiveStrokerManager;
            }
    };

IMrDTMProgressiveStrokerManagerPtr MrDTMProgressiveStrokerManager::m_mrDTMProgressiveStrokerManager;




//=======================================================================================
// @bsiclass                                                   Mathieu.St-Pierre
//=======================================================================================
class TexturedElementPlatform : public ITexturedElement
    {

    private :
        BC_DTM_OBJ*      m_dtmObject;
        DRange3d         m_range3d;
        DTMDrawingInfo&  m_drawingInfo;

    protected :

        virtual const DRange3d& _GetRange() const
            {
            return m_range3d;
            }

        virtual bool _DrapePointOnElement(DPoint3d& pointInUors) const
            {
            long drapeFlag;
            int  status;

            m_drawingInfo.FullUorsToStorage(pointInUors);

            status = bcdtmDrape_pointDtmObject(m_dtmObject,
                                               pointInUors.x,
                                               pointInUors.y,
                                               &pointInUors.z,
                                               &drapeFlag);

            m_drawingInfo.FullStorageToUors(pointInUors);

            return (status == SUCCESS) && (drapeFlag == 1);
            }

    public :

        TexturedElementPlatform(BcDTMP          dtm,
                                DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
            {
            m_dtmObject       = dtm->GetTinHandle();
            dtm->GetRange(m_range3d);

            m_drawingInfo.FullStorageToUors(&m_range3d.low, 2);
            }

        TexturedElementPlatform(BcDTMP         dtm,
                                DTMDrawingInfo& drawingInfo,
                                DRange3d&       effectiveRange)
        : m_drawingInfo(drawingInfo)
            {
            m_dtmObject       = dtm->GetTinHandle();
            dtm->GetRange(m_range3d);

            m_drawingInfo.FullStorageToUors(&m_range3d.low, 2);

            bool result = bsiDRange3d_intersect(&m_range3d, &effectiveRange, &m_range3d);

            if (result == 0)
                {
                memset(&m_range3d, 0, sizeof(DRange3d));
                }
            }

        virtual ~TexturedElementPlatform()
            {
            }
        
        void* operator new(size_t size)
            {
            return __super::operator new(size);
            }

        void operator delete(void *rawMemory, size_t size)
            {
            __super::operator delete(rawMemory, size);
            }


        /*
        TexturedElementPlatform(DRange3d        stmVisibleRange,
                        DTMDrawingInfo& drawingInfo)
                    : m_drawingInfo(drawingInfo)
            {
            m_dtmObject       = dtm->GetTinHandle();
            dtm->getRange(&m_range3d.low, &m_range3d.high);

            m_drawingInfo.FullStorageToUors(&m_range3d.low, 2);
            }
*/
    };

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 06/10
//=======================================================================================
void DTMElementTrianglesDisplayHandler::DrawWithTexture(BcDTMP                           dtm,
                                                 ElementHandleCR                      element,
                                                 ViewContextR                     context,
                                                 RefCountedPtr<DTMDataRef>&        DTMDataRef,
                                                 const ElementHandle::XAttributeIter& xAttr,
                                                 RefCountedPtr<DTMQvCacheDetails>& details,
                                                 DTMDrawingInfo&                   drawingInfo)
    {
    IMultiResolutionGridMaterialManagerPtr multiResolutionGridMatPtr = 0;
    DTMDataRefQvCache*                     qvElem;

    BeAssert((DTMDataRef != 0) && DTMDataRef->IsMrDTM());
    BeAssert(false == IsWireframeRendering(context));

    multiResolutionGridMatPtr = ((MrDTMDataRef*)DTMDataRef.get())->_GetMultiResGridMaterialManager();

    BeAssert(multiResolutionGridMatPtr != 0);

    if (multiResolutionGridMatPtr != 0)
        {
        MRImageTileIdVector visibleTextureTiles;

        ITexturedElementPtr texturedElementPtr;

        if (context.GetDrawPurpose() == DrawPurpose::Plot)
            {
            ViewContext::ModelRefMark modelRefMark(context);

            //During a rasterized print all draw only the tiles which are intersecting the print tile.

            if (modelRefMark.m_useNpcSubRange)
                {
                DRange3d effectiveRange = modelRefMark.m_npcSubRange;

                MRImageTileIdVector::iterator tileIdIter = visibleTextureTiles.begin();

                context.NpcToView((DPoint3dP)&effectiveRange, (DPoint3dCP)&effectiveRange, 2);
                context.ViewToLocal((DPoint3dP)&effectiveRange, (DPoint3dCP)&effectiveRange, 2);

                texturedElementPtr = new TexturedElementPlatform(dtm, drawingInfo, effectiveRange);
                }
            else
                {
                texturedElementPtr = new TexturedElementPlatform(dtm, drawingInfo);
                }
            }
        else
            {
            texturedElementPtr = new TexturedElementPlatform(dtm, drawingInfo);
            }

        double minScreenPixelsPerDrapePixel;

        if ((context.GetDrawPurpose() == DrawPurpose::UpdateDynamic) &&
            (DTMElementHandlerManager::IsDrawForAnimation() == false))
            {
            minScreenPixelsPerDrapePixel = MRDTM_GUI_TO_VIEW_POINT_DENSITY(20);
            }
        else
            {
            //During an animation preview there is only an DrawPurpose::UpdateDynamic draw,
            //with no DrawPurpose::Update draw following.
            if (MrDTMElementDisplayHandler::IsHighQualityDisplayForMrDTM() == true)
                {
                minScreenPixelsPerDrapePixel = MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_HIGH_QUALITY_DISPLAY;
                }
            else
                {
                minScreenPixelsPerDrapePixel = MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_DEFAULT_DISPLAY;
                }
            }

        multiResolutionGridMatPtr->GetVisibleTilesInView(visibleTextureTiles,
                                                         &context,
                                                         minScreenPixelsPerDrapePixel,
                                                         texturedElementPtr);

        if (visibleTextureTiles.size() > 0)
            {        
            multiResolutionGridMatPtr->PrepareLookAhead(visibleTextureTiles, *element.GetModelRef());
            }

        MRImageTileIdVector::const_iterator tileIdIter    = visibleTextureTiles.begin();
        MRImageTileIdVector::const_iterator tileIdIterEnd = visibleTextureTiles.end();
        ICachedMaterialPtr cachedMaterialPtr;

        //The number of different materials (i.e. : one per tile) to be drawn
        //must be equal or greater than the maximum number of materials that
        //can be contained in the cache because no material used during the draw
        //should be deleted before the draw is finished. This might be a bug.

        //BeAssert(visibleTiles.size() <= MAX_NB_MATERIALS_CACHED);

        //TR 352273 - Prioritize STM raster draping over any override.
        OvrMatSymb ovrMatSym(*context.GetOverrideMatSymb());
        UInt32 flag = ovrMatSym.GetFlags();

        if (flag != MATSYMB_OVERRIDE_None)
            {
            flag = flag & ~MATSYMB_OVERRIDE_RenderMaterial;
            flag = flag & ~MATSYMB_OVERRIDE_FillColor;
            flag = flag & ~MATSYMB_OVERRIDE_FillColorTransparency;
            ovrMatSym.SetFlags(flag);
            context.GetIDrawGeom().ActivateOverrideMatSymb(&ovrMatSym);
            }

        int textureId = 0;

        while (tileIdIter != tileIdIterEnd)
            {
            multiResolutionGridMatPtr->SetupLookAheadForTile(*tileIdIter, *element.GetModelRef());

            cachedMaterialPtr = multiResolutionGridMatPtr->GetMaterial(*tileIdIter, *element.GetModelRef());

            BeAssert(cachedMaterialPtr != 0);

            const DRange3d& itemRange = cachedMaterialPtr->GetMaterialEffectiveRange();

            ElemMatSymb elemMatSymb;

            elemMatSymb.SetMaterial(cachedMaterialPtr->GetMaterialProperties());
            
            context.GetIDrawGeom().ActivateMatSymb(&elemMatSymb);

            DPoint3d textureExtent[5];

            textureExtent[0].x = itemRange.low.x;
            textureExtent[0].y = itemRange.low.y;
            textureExtent[0].z = (itemRange.high.z + itemRange.low.z) / 2;

            textureExtent[1].x = itemRange.low.x;
            textureExtent[1].y = itemRange.high.y;
            textureExtent[1].z = textureExtent[0].z;

            textureExtent[2].x = itemRange.high.x;
            textureExtent[2].y = itemRange.high.y;
            textureExtent[2].z = textureExtent[0].z;

            textureExtent[3].x = itemRange.high.x;
            textureExtent[3].y = itemRange.low.y;
            textureExtent[3].z = textureExtent[0].z;

            textureExtent[4].x = itemRange.low.x;
            textureExtent[4].y = itemRange.low.y;
            textureExtent[4].z = textureExtent[0].z;

            DTMFeatureId textureRegionFeatureId;

            drawingInfo.FullUorsToStorage((DPoint3d*)textureExtent, 5);

            int          status;
            DTMUserTag userTag = textureId;
            DTMFeatureId* textureRegionIdsP = 0;
            long            numRegionTextureIds;

            status = bcdtmInsert_internalDtmFeatureMrDtmObject(dtm->GetTinHandle(),
                                                               DTMFeatureType::Region,
                                                               1,
                                                               2,
                                                               userTag,
                                                               &textureRegionIdsP,
                                                               &numRegionTextureIds,
                                                               textureExtent,
                                                               5);

            if ((numRegionTextureIds == 0) || (status != 0))
                {
                textureId++;
                tileIdIter++;
                continue;
                }

            //Around the border of the DTM it is possible that one texture region results
            //in more than one DTM region.
            for (int regionId = 0; regionId < numRegionTextureIds; regionId++)
                {
                textureRegionFeatureId = textureRegionIdsP[regionId];

#ifdef DEBUG
                if (s_dumpMeshLoadingInfo)
                    {
                    DumpDTMInTinFile(dtm,
                                     wstring(L"E:\\MyDoc\\SS3 - Iteration 1\\MrDTM\\Draping Refactorization\\NewMeshLoadingFunction\\Bug\\DTMToClip%I64i.tin"),
                                     &textureRegionFeatureId);

                    DumpPointsInXYZfile(textureExtent,
                                        5,
                                        string("E:\\MyDoc\\SS3 - Iteration 1\\MrDTM\\Draping Refactorization\\NewMeshLoadingFunction\\Bug\\ClipExtent%I64i.xyz"),
                                        &textureRegionFeatureId);
                    }
#endif

                drawingInfo.FullStorageToUors((DPoint3d*)textureExtent, 5);

                DTMStrokeForCacheShadedTriangles trianglesStroker(dtm, drawingInfo,
                                                                  textureExtent,
                                                                  5);

                trianglesStroker.SetTextureRegionFeatureId(textureRegionFeatureId);

                //qvElem = DTMDataRefCachingManager::GetCacheElem (DTMDataRef.get(), element, xAttr.GetId(), context, Triangles, 0, details.get());

                qvElem = DTMDisplayCacheManager::GetCacheElem (element, xAttr.GetId(), context, Triangles, 0, details.get());

                if (qvElem)
                    {
                    qvElem->DrawQVElem (context);
                    }
                else
                    {
                    LogTimeInfo(L"Create Cache");
                    //qvElem = DTMDataRefCachingManager::CreateCacheElemAndDraw(DTMDataRef.get(), element, xAttr.GetId(), context, Triangles, 0, details.get(), trianglesStroker);
                    qvElem = DTMDisplayCacheManager::CreateCacheElemAndDraw (element, xAttr.GetId(), context, Triangles, 0, details.get(), trianglesStroker);
                    }

                if (!qvElem)
                    {
                    trianglesStroker._StrokeForCache(element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                    }
                }

            if (textureRegionIdsP != 0)
                {
                free(textureRegionIdsP);
                textureRegionIdsP = 0;
                }

            textureId++;
            tileIdIter++;
            }
        }

    }

//=======================================================================================
// @bsimethod                                                   Sylvain.Pucci
//=======================================================================================
bool DTMElementTrianglesDisplayHandler::_Draw (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {
    DSHandlerKeyStoragePtr ds = GetCommonHandlerKey (context.GetDisplayStyleHandlerKey ());

    DrawPurpose purpose = context.GetDrawPurpose ();
    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.
    if (purpose == DrawPurpose::ChangedPre)
        {
        context.DrawElementRange(element.GetElementCP());
        }
    else
        {
        //GeomPickModes pickMode =
        //    (DrawPurpose::Pick == purpose && 1 == element.GetElementCP()->hdr.dhdr.props.b.s)
        //    ? PICK_MODE_Cached :
        //context.GetGeomPickModes();

        // Create a DTM element from the XAttributes (this is is a very lightweight operation that
        // just assigns the dtm internal arrays to their addresses inside the XAttributes).

        if (DrawPurpose::FitView == purpose)
            {
            DrawScanRange (context, element, drawingInfo.GetDTMDataRef());
            return false;
            }

        DTMElementTrianglesHandler::DisplayParams params (xAttr);

        if (!SetSymbology (params, drawingInfo, context))
            return false;

        IPickGeom*  pick = context.GetIPickGeom ();
        RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef();
        if (DTMDataRef != nullptr)
            {
            if (DrawPurpose::Pick == context.GetDrawPurpose () || DrawPurpose::Flash == purpose)
                {
                if (CanDoPickFlash(DTMDataRef, purpose) == false)
                    {
                    return false;
                    }

                DTMPtr dtmPtr (DTMDataRef->GetDTMStorage(None, context));
                BcDTMP dtm = 0;

                if (dtmPtr != 0)
                    dtm = dtmPtr->GetBcDTM();

                if (!dtm || dtm->GetDTMState() != DTMState::Tin)
                    return false;

                DPoint3d startPt;
                if (DrawPurpose::Flash == purpose)
                    {
                    // Need to do something else here as they may not be anything visible if we just flash the hull.
                    DisplayPathCP path = context.GetSourceDisplayPath();
                    HitPathCP hitPath = dynamic_cast<HitPathCP>(path);

                    if (!hitPath)
                        return true;

                    hitPath->GetHitPoint (startPt);
                    // point is in Root Coordinates need to convert to Local.
                    drawingInfo.RootToStorage (startPt);
                    }
                else
                    {
                    DPoint3d endPt;
                    if (!GetViewVectorPoints (drawingInfo, context, dtmPtr, startPt, endPt))
                        return true;

                    if (startPt.x != endPt.x || startPt.y != endPt.y)
                        {
                        // Non Top View
                        DPoint3d trianglePts[4];
                        long drapedType;
                        BC_DTM_OBJ* bcDTM = dtm->GetTinHandle();
                        long voidFlag;
                        DPoint3d point;

                        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&startPt), ((DPoint3d*)&endPt), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, &voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != 0)
                            return true;

                        startPt = point;
                        }
                    }

                // TopView
                DPoint3d trianglePts[4];
                int drapedType;

                double elevation;
                if (dtm->DrapePoint (&elevation, nullptr, nullptr, trianglePts, &drapedType, &startPt) == DTM_SUCCESS)
                    {
                    DrawSentinel sentinel (context, drawingInfo);

                    trianglePts[3] = trianglePts[0];
                    if (drapedType == 1)
                        {
                         bool isWireframe = true;

                        if (context.GetViewport() && !IsWireframeRendering (context))
                            isWireframe = false;

                        if (pick)
                            {
                            int meshFacesP[3] = {1,2,3};
                            UInt32 numPerFace = 3;
                            bool   twoSided = false;
                            size_t indexCount = 3;
                            size_t pointCount = 3;
                            DPoint3dCP pPoint = trianglePts;
                            Int32 const* pPointIndex = meshFacesP;

                            PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex);
                            context.GetIDrawGeom().DrawPolyface (polyCarrier);
                            }
                        else
                            {
                            int meshFacesP[3] = {1,2,3};
                            UInt32 numPerFace = 3;
                            bool   twoSided = false;
                            size_t indexCount = 3;
                            size_t pointCount = 3;
                            DPoint3dCP pPoint = trianglePts;
                            Int32 const* pPointIndex = meshFacesP;

                            PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex);
                            context.GetIDrawGeom().DrawPolyface (polyCarrier);
                            }
                        }
                    else if (drapedType == 3)
                        {
                        // This is a fudge as I don't know if this is a point or a line.
                        startPt.z = trianglePts[0].z;
                        if (!trianglePts[0].isEqual(&startPt))
                            context.DrawStyledLineString3d (2, trianglePts, nullptr);
                        else
                            {
                            ::DPoint3d m_fencePt[5];

                            m_fencePt[0] = trianglePts[0];
                            m_fencePt[0].x -= 0.00011;
                            m_fencePt[0].y -= 0.00011;
                            m_fencePt[1] = trianglePts[0];
                            m_fencePt[1].x += 0.00011;
                            m_fencePt[1].y -= 0.00011;
                            m_fencePt[2] = trianglePts[0];
                            m_fencePt[2].x += 0.00011;
                            m_fencePt[2].y += 0.00011;
                            m_fencePt[3] = trianglePts[0];
                            m_fencePt[3].x -= 0.00011;
                            m_fencePt[3].y += 0.00011;
                            m_fencePt[4] = m_fencePt[0];
                            PickDraw pd;
                            pd.context = &context;
                            pd.drawnSomething = false;
                            dtm->BrowseFeatures(DTMFeatureType::Triangle, DTMFenceParams(DTMFenceType::Block, DTMFenceOption::Overlap, (DPoint3d*)m_fencePt, 5), 10000, &pd, DrawPickTriangles);
                            }
                        }
                    if(pick)
                        pick->SetHitPriorityOverride (HitPriority::Highest);
                    }
                return true;
                }

            if (DrawPurpose::RangeCalculation != context.GetDrawPurpose ())
                {
                if (DoProgressiveDraw(DTMDataRef, &context) == true)
                    {
                    LogDebugV(L"Drawing MrDTM Triangles In Progressive Mode DTMDataRef(%x)", DTMDataRef);

                    bool isWireframe = true;

                    if (context.GetViewport() && (false == IsWireframeRendering(context)))
                        isWireframe = false;

                    RefCountedPtr<DTMQvCacheDetails> details = DTMDataRef->GetDTMDetails(element, !isWireframe ? DrawShadedTriangles : DrawTriangles, context, drawingInfo);

                    BeAssert(MrDTMDataRef::GetMrDTMProgressiveDisplayInterface() != 0);

                    Transform storageToUor;
                    Transform uorToStorage;

                    getStorageToUORMatrix(storageToUor, DTMDataRef->GetElement().GetModelRef(), DTMDataRef->GetElement(), false);

                    bool isRasterDrapingRequired(this->GetSubHandlerId() == ELEMENTHANDLER_DTMELEMENTDISPLAYRASTERDRAPING);

                    IMrDTMProgressiveStrokerManagerPtr strokerManagerPtr(MrDTMProgressiveStrokerManager::GetInstance());

                    MrDTMDataRef::GetMrDTMProgressiveDisplayInterface()->SetMrDTMProgressiveStrokerManager(strokerManagerPtr);

                    bool invert = bsiTransform_invertTransform(&uorToStorage, &storageToUor);

                    BeAssert(invert != 0);

                    MrDTMDataRef::GetMrDTMProgressiveDisplayInterface()->Draw(element, context, DTMDataRef, xAttr, uorToStorage, &drawingInfo, isRasterDrapingRequired);
                    }
                else
                    {
                    DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(DrawShadedTriangles, context));
                    BcDTMP dtm = 0;

                    if (dtmPtr != 0)
                        {
                        dtm = dtmPtr->GetBcDTM();
                        }

                    bool isWireframe = true;

                    if (context.GetViewport() && (false == IsWireframeRendering(context)))
                        isWireframe = false;

                    RefCountedPtr<DTMQvCacheDetails> details = DTMDataRef->GetDTMDetails (element, !isWireframe ? DrawShadedTriangles : DrawTriangles, context, drawingInfo); // GRAPHICCACHE


                    if (!dtm || dtm->GetDTMState() != DTMState::Tin)
                        return false;
                    // Need to get graphics cache and details,
                    // Is this view different then previous view?
                    // If so then create
                    LogDebugV(L"Drawing Triangles DTMDataRef(%x)", DTMDataRef);
                    DTMDataRefQvCache* qvElem = DTMDisplayCacheManager::GetCacheElem (element,xAttr.GetId(), context, Triangles, ds, details.get());
                    if (!qvElem)
                        {
                        if (!isWireframe)
                            {
                            try
                                {
                                if (params.HandlerId() == ELEMENTHANDLER_DTMELEMENTDISPLAYRASTERDRAPING)
                                    {
                                    BeAssert(DTMDataRef->CanDrapeRasterTexture() == true);

                                    DrawWithTexture (dtm, element, context, DTMDataRef, xAttr, details, drawingInfo);
                                    }
                                else
                                    {
                                    DTMStrokeForCacheShadedTriangles trianglesStroker (dtm, drawingInfo);

                                    LogTimeInfo(L"Create Cache");
                                    qvElem = DTMDisplayCacheManager::CreateCacheElemAndDraw (element, xAttr.GetId(), context, Triangles, ds, details.get(), trianglesStroker);

                                    if (!qvElem)
                                        {
                                        LogDebug(L"Failed to CreateCache");

                                        trianglesStroker._StrokeForCache (element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                                        }
                                    }
                                }
                            catch(...)
                                {
                                }
                            }
                        else
                            {
                            DTMStrokeForCacheShadedTriangles trianglesStroker(dtm, drawingInfo);

                            LogTimeInfo(L"Create Cache");
                            qvElem = DTMDisplayCacheManager::CreateCacheElemAndDraw (element, xAttr.GetId(), context, Triangles, ds, details.get(), trianglesStroker);

                            if (!qvElem)
                                {
                                LogDebug(L"Failed to CreateCache");

                                trianglesStroker._StrokeForCache (element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                                }
                            }

                        qvElem = nullptr;
                        }

                    if (qvElem)
                        {
                        LogTimeInfo(L"Draw QvElement");
                        qvElem->DrawQVElem (context);
                        }

                    if (details != nullptr)
                        details = nullptr;
                    }
                }
            }
        }
    return true;
    }

/*=================================================================================**//**
* @bsiclass                                                  Mathieu.St-Pierre   08/2011
+===============+===============+===============+===============+===============+======*/
bool DTMElementTrianglesDisplayHandler::_CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const
    {
    bool canDraw = true;
    if (dtmDataRef->IsMrDTM())
        {
        ElementHandle elemHandle(dtmDataRef->GetElement().GetElementRef(),
                              dtmDataRef->GetElement().GetModelRef());

        bool isDrapingOn = DTMElementRasterDrapingHandler::GetInstance()->GetVisibility(elemHandle);

        if ((isDrapingOn == true) &&
            (context.GetViewport() != 0) &&
            (false == IsWireframeRendering(context)) &&
            (((MrDTMDataRef*)dtmDataRef)->GetRasterTextureSource() != 0))
            {
            canDraw = false;
            }


        }
    return canDraw;
    }

void DTMElementTrianglesDisplayHandler::_GetPathDescription
(
ElementHandleCR                        element,
ElementHandle::XAttributeIter const&   xAttr,
LazyDTMDrawingInfoProvider&         ldip,
WStringR                            string,
HitPathCR                           path,
WCharCP                           levelStr,
WCharCP                           modelStr,
WCharCP                           groupStr,
WCharCP                           delimiterStr
)
    {
    double elev;
    double slope;
    double aspect;
    DPoint3d tri[3];
    DPoint3d pt;
    DPoint3d globalOrigin;

    path.GetHitPoint(pt);
    _GetDescription (element, xAttr, string, 255);
    ldip.Get().GetRootToCurrLocalTrans().multiplyAndRenormalize (&pt, &pt, 1);
    ldip.Get().FullUorsToStorage (pt);
    RefCountedPtr<IDTM> dtm;
    RefCountedPtr<DTMDataRef> dtmRef = ldip.Get().GetDTMDataRef();

    if (dtmRef.IsValid())
        dtmRef->GetDTMReferenceStorage (dtm);

    if (dtm == NULL)
        { return; }

    dtm->GetDTMDraping()->DrapePoint(&elev, &slope, &aspect, tri, nullptr, pt);
    WString wElevString;
    WString wSlopeString;
    WString wAspectString;

    const wchar_t*  delim = delimiterStr ? delimiterStr : L",";
    pt.z = elev;
    ldip.Get().FullStorageToUors (pt);


    dgnModel_getGlobalOrigin (path.GetRoot ()->GetDgnModelP(), &globalOrigin);
    wElevString = DistanceFormatter::Create(*path.GetRoot ()->GetDgnModelP())->ToString (pt.z - globalOrigin.z);        // includeUnits ??

    wSlopeString.Sprintf (L"%.2f", slope);
    wAspectString.Sprintf (L"%.2f", aspect);

    WString elevationString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Elevation);
    WString slopeString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Slope);
    WString aspectString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Aspect);

    elevationString.ReplaceAll (L"{1}", wElevString.GetWCharCP());
    slopeString.ReplaceAll (L"{1}", wSlopeString.GetWCharCP());
    aspectString.ReplaceAll (L"{1}", wAspectString.GetWCharCP());

    string.append(delim + elevationString + delim + slopeString + delim + aspectString);
    }

struct RegionDisplayStyleHandler : DisplayStyleHandler
    {
    static RegionDisplayStyleHandler s_instance;
    virtual XAttributeHandlerId _GetHandlerId () const override
        {
        return XAttributeHandlerId (TMElementMajorId, DTMElementRegionsDisplayHandler::GetInstance().GetSubHandlerId());
        }
    virtual WString _GetName () const override
        {
        return L"Region";
        }
    virtual bool   _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override
        {
        return false;
        }
    static DisplayStyleHandlerCR GetInstance()
        {
        return s_instance;
        }
    };

RegionDisplayStyleHandler RegionDisplayStyleHandler::s_instance;

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
+===============+===============+===============+===============+===============+======*/
struct RegionDisplayHandlerKey : public DisplayStyleHandlerKey
{
private:
    Int64 m_region;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    RegionDisplayHandlerKey (Int64 region) : DisplayStyleHandlerKey (RegionDisplayStyleHandler::GetInstance())
        {
        m_region = region;
        }
    virtual ~RegionDisplayHandlerKey ()
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BrandonBohrer   06/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    Matches (DisplayStyleHandlerKey const& other) const override
        {
        RegionDisplayHandlerKey const *     otherKey = NULL;

        if (GetHandlerId() != otherKey->GetHandlerId() || NULL == (otherKey = dynamic_cast <RegionDisplayHandlerKey const *> (&other)))
            return false;

        return  m_region == otherKey->m_region;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BrandonBohrer   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static  DisplayStyleHandlerKeyPtr  Create (Int64 region)
        {
        return new RegionDisplayHandlerKey (region);
        }
    };

//=======================================================================================
// @bsimethod                                                   Sylvain.Pucci
//=======================================================================================
bool DTMElementRegionsDisplayHandler::_Draw (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {
    DrawPurpose purpose = context.GetDrawPurpose ();
    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.
    if (purpose == DrawPurpose::ChangedPre)
        {
        context.DrawElementRange(element.GetElementCP());
        }
    else
        {
        // Create a DTM element from the XAttributes (this is is a very lightweight operation that
        // just assigns the dtm internal arrays to their addresses inside the XAttributes).
        RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef();

        if (DrawPurpose::Flash == purpose && CanDoPickFlash(DTMDataRef, purpose))
            // Need to do something else here as they may not be anything visible if we just flash the hull.
            return false;

        if (DrawPurpose::FitView == purpose)
            {
            DrawScanRange (context, element, drawingInfo.GetDTMDataRef());
            return false;
            }

        if (DrawPurpose::RangeCalculation != purpose)
            {
            DTMElementRegionsHandler::DisplayParams params (xAttr);

            if (!SetSymbology (params, drawingInfo, context))
                return false;

            DSHandlerKeyStoragePtr ds = GetCommonHandlerKey (RegionDisplayHandlerKey::Create (params.GetTag()));
            Bentley::TerrainModel::DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(DrawShadedTriangles, context));
            BcDTMP dtm = 0;

            if (dtmPtr != 0)
                dtm = dtmPtr->GetBcDTM();

            if (!dtm || dtm->GetDTMState() != DTMState::Tin)
                return false;

            bool isWireframe = true;

            if (context.GetViewport() && (false == IsWireframeRendering(context)))
                isWireframe = false;
            LogDebugV(L"Drawing Triangles Regions DTMDataRef(%x)", DTMDataRef);
            RefCountedPtr<DTMQvCacheDetails> details = DTMDataRef->GetDTMDetails (element, !isWireframe ? DrawShadedTriangles : DrawTriangles, context, drawingInfo); // GRAPHICCACHE
//            UInt16 handlerId = *(UInt16*)(((byte*)xAttr.PeekData()) + 4);
            DTMDataRefQvCache* qvElem = DTMDisplayCacheManager::GetCacheElem (element,xAttr.GetId(),context, Region, ds, details.get());
            if (qvElem == nullptr)
                {
                if (false == IsWireframeRendering(context))
                    {
                    try
                        {
                        DTMStrokeForCacheShadedTriangles trianglesStroker(dtm, params.GetTag(), drawingInfo);
                            {
                            LogTimeInfo(L"Create Cache");
                            qvElem = DTMDisplayCacheManager::CreateCacheElemAndDraw (element, xAttr.GetId(), context, Region, ds, details.get(), trianglesStroker);
                            }
                        if (!qvElem)
                            {
                            LogDebug(L"Failed to CreateCache");
                            trianglesStroker._StrokeForCache(element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                            }
                        qvElem = nullptr;
                        }
                    catch(...)
                        {
                        }
                    }
                else
                    {
                    DTMStrokeForCacheTriangles trianglesStroker(dtm, params.GetTag(), drawingInfo);
                        {
                        LogTimeInfo(L"Create Cache");
                        qvElem = DTMDisplayCacheManager::CreateCacheElemAndDraw (element, xAttr.GetId(), context, Region, ds, details.get(), trianglesStroker);
                        }

                    if (!qvElem)
                        {
                        trianglesStroker._StrokeForCache(element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                        }
                    qvElem = nullptr;
                    }
                }
                if (qvElem)
                    {
                    LogTimeInfo(L"Draw QvElement");
                    qvElem->DrawQVElem(context);
                    }
                if (details != nullptr)
                    details = nullptr;
            }
        }
    return true;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
int LoadRegions (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId id, DPoint3d *featurePtsP, size_t numFeaturePts,void *userP)
    {
    bvector<DTMUserTag>& regions = *(bvector<DTMUserTag>*)userP;
    if ( std::find ( regions.begin(), regions.end(), userTag ) != regions.end() )
        return SUCCESS;
    regions.push_back ( userTag );
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 01/11
//=======================================================================================
bool DTMElementRegionsDisplayHandler::_CreateDefaultElements(EditElementHandleR element, bool visible)
    {
    if (!DTMElementRegionsHandler::GetInstance()->HasSubElement (element))
        {
        RefCountedPtr<DTMDataRef> DTMDataRef;
        DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

        if (DTMDataRef.IsValid())
            {
            Bentley::TerrainModel::DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(None));
            BcDTMP dtm = nullptr;

            if (dtmPtr != 0)
                {
                dtm = dtmPtr->GetBcDTM();
                }

            bvector<Int64> regions;
            if (dtm)
                {
                dtm->BrowseFeatures (DTMFeatureType::Region, DTMFenceParams(), 10000, &regions, LoadRegions);

                for (bvector<Int64>::const_iterator iter = regions.begin(); iter < regions.end(); iter++)
                    {
                    DTMSubElementId subElement = DTMElementRegionsHandler::GetInstance()->Create (element, visible);
                    DTMElementRegionsHandler::DisplayParams params (element);
                    params.FromElement (element, subElement);
                    params.SetTag (*iter);
                    params.SetDescription (L"");

                    UInt64 tag = params.GetTag();
                    char* buffer = (char*)&tag;

                    bool valid = true;
                    int length = 8;
                    for(int i = 0; i < 8; i++)
                        {
                        if (!buffer[i])
                            {
                            length = i;
                            break;
                            }

                        if (isalnum (buffer[i]))
                            {
                            valid = true;
                            }
                        }

                    if (valid && length)
                        {
                        char description[9];
                        memcpy (description, buffer, length);
                        description[length] = 0;
                        wchar_t wDescription[9];
                        mbstowcs (wDescription, description, length + 1);

                        params.SetDescription (wDescription);
                        }
                    else
                        {
                        params.SetDescription (L"None");
                        }

                    params.SetElement (element, subElement);
                    }

                return regions.size() != 0;
                }
            }
        // There isn't any regions in this dataset
        return false;
        }
    return false;
    }


SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementTrianglesDisplayHandler);

SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementRegionsDisplayHandler);


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
