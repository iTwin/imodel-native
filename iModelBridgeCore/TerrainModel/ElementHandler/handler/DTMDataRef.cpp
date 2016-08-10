/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDataRef.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct DTMDataRefElementRefAppData : ElementRefAppData
    {
    private:
        DTMDataRefPtr m_dtmDataRef;
        static Key s_key;
    protected:
        DTMDataRefElementRefAppData (DTMDataRef* dtmDataRef) : m_dtmDataRef (dtmDataRef) {}
    public:
        DTMDataRef* GetDTMDataRef ()
            {
            return m_dtmDataRef.get();
            }
    virtual void _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
        {
        delete this;
        }
    virtual bool _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason) override
        {
        if (reason == ELEMREF_CHANGE_REASON_Delete && GetDTMDataRef()->IsMrDTM())
            return true; // return true to free the data

        return false;
        }

//        DTMDataRefElementRefAppData* Create (DTMDataRef* dtmDataRef) { return new DTMDataRefElementRefAppData (dtmDataRef); }
    static DTMDataRef* GetDTMAppData (ElementHandleCR element)
        {
//        BeAssert (element.IsPersistent() && element.GetElementRef());

        ElementRefP elementRef = element.GetElementRef();

        if (elementRef)
            {
            DTMDataRefElementRefAppData* appData = static_cast<DTMDataRefElementRefAppData*>(elementRef->FindAppData (s_key));
            if (appData)
                return appData->GetDTMDataRef ();
            }
        return nullptr;
        }
    static void AddDTMAppData (ElementHandleCR element, DTMDataRef* dtmDataRef)
        {
        ElementRefP elementRef = element.GetElementRef();

        if (elementRef)
            elementRef->AddAppData (s_key, new DTMDataRefElementRefAppData (dtmDataRef), elementRef->GetHeapZone());
        }

    static void DropDTMAppData (ElementHandleCR element)
        {
        BeAssert (element.IsPersistent() && element.GetElementRef());

        ElementRefP elementRef = element.GetElementRef();

        if (elementRef)
            elementRef->DropAppData (s_key);
        }
    };

ElementRefAppData::Key DTMDataRefElementRefAppData::s_key;

// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRef::DTMDataRef()
    {    
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRef::~DTMDataRef()
    {
    }


DTMDataRef* DTMDataRef::GetDTMAppData (ElementHandleCR element)
    {
    return DTMDataRefElementRefAppData::GetDTMAppData (element);
    }

void DTMDataRef::AddDTMAppData (ElementHandleCR element, DTMDataRef* dtmDataRef)
    {
    DTMDataRefElementRefAppData::AddDTMAppData (element, dtmDataRef);
    }

void DTMDataRef::DropDTMAppData (ElementHandleCR element)
    {
    DTMDataRefElementRefAppData::DropDTMAppData (element);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/11
//=======================================================================================
StatusInt DTMDataRef::GetDTMReferenceDirect (RefCountedPtr<IDTM>& outDtm)
    {
    GetDTMReferenceStorage (outDtm);
    
    if (outDtm.IsValid())
        {
        Transform   trsf;

        DTMElementHandlerManager::GetStorageToUORMatrix (trsf, GetGraphicalElement());
        outDtm->GetTransformDTM (outDtm, trsf);
        }
    return outDtm.IsValid() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/11
//=======================================================================================
StatusInt DTMDataRef::GetDTMReference(RefCountedPtr<IDTM>& outDtm, TransformCR currTrans)
    {
    GetDTMReferenceStorage (outDtm);
    
    if (outDtm.IsValid())
        {
        Transform storageToUOR;
        Transform transformation;
        DTMPtr oldDtm;

        DTMElementHandlerManager::GetStorageToUORMatrix (storageToUOR, GetGraphicalElement());
        transformation.productOf (&currTrans, &storageToUOR);
        oldDtm = outDtm;
        oldDtm->GetTransformDTM (outDtm, transformation);
        }
    return outDtm.IsValid() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/11
//=======================================================================================
IDTM* DTMDataRef::GetDTM (DTMDataRefPurpose purpose, ViewContextR context)
    {
    return GetDTMStorage (purpose, context);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/11
//=======================================================================================
IDTM* DTMDataRef::GetDTM (DTMDataRefPurpose purpose)
    {
    return GetDTMStorage (purpose);
    }

#ifndef GRAPHICCACHE
//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMQvCacheDetails* DTMDataRef::GetDTMDetails(ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo)
    {
    return _GetDTMDetails(element, purpose, context, drawingInfo);
    }
#endif
//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
IDTM* DTMDataRef::GetDTMStorage(DTMDataRefPurpose purpose, ViewContextR context)
    {
    return _GetDTMStorage(purpose, context);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
bool DTMDataRef::GetExtents (DRange3dR range)
    {
    return _GetExtents (range);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
IDTM* DTMDataRef::GetDTMStorage(DTMDataRefPurpose purpose)
    {
    return _GetDTMStorage(purpose);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
StatusInt DTMDataRef::GetDTMReferenceStorage(RefCountedPtr<IDTM>& dtmOut)
    {
    return _GetDTMReferenceStorage (dtmOut);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
bool DTMDataRef::IsReadOnly()
    {
    return _IsReadOnly();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
bool DTMDataRef::IsMrDTM()
    {
    return _IsMrDTM();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
bool DTMDataRef::GetProjectedPointOnDTM (DPoint3d& pointOnDTM, ElementHandleCR thisElm, ViewportP viewport, const DPoint3d& testPoint)
    {
    return GetProjectedPointOnDTM (pointOnDTM, thisElm, viewport->GetRootToViewMap()->M0, testPoint, viewport);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
bool DTMDataRef::GetProjectedPointOnDTM
(
DPoint3dR       pointOnDTM,
ElementHandleCR    thisElm,
DMatrix4dCR     w2vMap,
DPoint3dCR      testPoint
)
    {
    return GetProjectedPointOnDTM (pointOnDTM, thisElm, w2vMap, testPoint, NULL);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/11
//=======================================================================================
double DTMDataRef::GetLastModified()
   {
   return _GetLastModified();
   }

bool DTMDataRef::CanDrapeRasterTexture ()
    {
    return false;
    }

bool DTMDataRef::GetProjectedPointOnDTM
(
DPoint3dR       pointOnDTM,
ElementHandleCR    thisElm,
DMatrix4dCR     w2vMap,
DPoint3dCR      testPoint,
ViewportP      viewport
 )
    {
    DTMPtr dtmPtr(GetDTMStorage(None));
    BcDTMP dtm = 0;

    if (!dtmPtr.IsValid() || NULL == (dtm = dtmPtr->GetBcDTM()))
        { return false;}

    DTMDrawingInfo drawingInfo;
    DTMElementDisplayHandler::GetDTMDrawingInfo (drawingInfo, thisElm, this,nullptr); // ToDo??? LocalToActiveTransContext (viewport, thisElm.GetModelRef()));

    DPoint3d startPt = testPoint;
    DPoint3d endPt;
    DPoint3d pt;
    DMatrix4d invW2vMap;
    DPoint4d pt4;
    DPoint4d endPt4;

    invW2vMap.qrInverseOf (&w2vMap);
    w2vMap.multiplyAndRenormalize (&pt, &testPoint, 1);
    pt.z -= 100;
    invW2vMap.multiplyAndRenormalize (&endPt, &pt, 1);
    pt4.init (&testPoint, 0);
    w2vMap.multiply (&pt4, &pt4, 1);
    pt4.z -= 100;
    invW2vMap.multiply (&endPt4, &pt4, 1);
    endPt.init (endPt4.x, endPt4.y, endPt4.z);

    drawingInfo.RootToStorage (startPt);
    drawingInfo.RootToStorage (endPt);

    if (fabs(startPt.x - endPt.x) > 1e-5 || fabs(startPt.y - endPt.y) > 1e-5)
        {
        // Intersect line with the DTM Range
        DRange3d range;
        DPoint3d sP;
        DPoint3d eP;
        DPoint3d point;

        dtm->GetRange (range);
        DVec3d diagonalVector = DVec3d::FromStartEnd (startPt, endPt);
        if (!range.intersectRay (nullptr, nullptr, &sP, &eP, &startPt, &diagonalVector))
            return false;

        // Non Top View
        DPoint3d trianglePts[4];
        long drapedType;
        BC_DTM_OBJ* bcDTM = dtm->GetTinHandle();
        bool voidFlag;

        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&sP), ((DPoint3d*)&eP), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != false)
            { return false; }

        startPt = point;
        }

    // TopView
    DPoint3d trianglePts[4];
    int drapedType;
    double elevation;

    if (DTM_SUCCESS != dtm->DrapePoint (&elevation, nullptr, nullptr, trianglePts, drapedType, startPt))
        { return false; }
    startPt.z = elevation;
    drawingInfo.StorageToRoot (pointOnDTM = startPt);
    return true;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMQvCacheDetails* DTMDataRef::_GetDTMDetails(ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo)
    {
    DRange3d drange;
    DRange3d retRange;

    GetExtents (drange);
    if (GetUpdateRangeFromRange (context, drawingInfo, drange, retRange, 1, false))
        {
        return new DTMQvCacheDetailsRange(retRange);
        }
    return nullptr;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
