/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDataRefXAttribute.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <Bentley/BeTimeUtilities.h>
#include <TerrainModel/Core/DTMIterators.h>
#include "time.h"

#define FILETIME_1_1_1970  116444736000000000LL           // Win32 file time of midnight 1/1/70
#define UMILLIS_TO_FTI     10000LL                        // milliseconds -> 100-nanosecond interval

// COMPARE_DTM will only work if you load up 1 DTM and you want to check persistency.
//#define COMPARE_DTM

#ifdef COMPARE_DTM
bool remembered = false;
BC_DTM_OBJ stored_headerData;
bvector<void*> stored_featureArrays;
bvector<void*> stored_pointArrays;
bvector<void*> stored_nodeArrays;
bvector<void*> stored_cListArrays;
bvector<void*> stored_fListArrays;

void freeStored (bvector<void*>& list)
    {
    if (list.size() == 0)
        return;

    for (size_t i = 0; i < list.size(); i++)
        {
        free (list[i]);
        }
    }

void compare (bvector<void*>& list1, bvector<void*>& list2, int num, int partitionSize, int size)
    {
    if (list1.size() != list2.size())
        return;

    for (size_t i = 0; i < list1.size(); i++)
        {
        int cmp;
        if (num < partitionSize)
            cmp = memcmp (list1[i], list2[i], size * num);
        else
            cmp = memcmp (list1[i], list2[i], size * partitionSize);

        if (cmp != 0)
            return;
        num -= partitionSize;
        }
    }

void remember(bvector<void*>& list1, void** list2, int num, int partitionSize, int size)
    {
    int i = 0;
    freeStored (list1);

    while (num > 0)
        {
        int pSize = 0;
        if (num < partitionSize)
            pSize = size * num;
        else
            pSize = size * partitionSize;

        void *data = malloc (pSize);
        memcpy (data, list2[i++], pSize);
        list1.push_back (data);
        num -= partitionSize;
        }
    }

void RememberDTM (
            void* headerP,
            void** featuresArrayP,
            void** pointArrayP,
            void** nodeArrayP,
            void** clistArrayP,
            void** flistArrayP)
    {
    memcpy (&stored_headerData, headerP, sizeof (BC_DTM_OBJ));
    remember (stored_featureArrays, featuresArrayP, stored_headerData.memFeatures, stored_headerData.featurePartitionSize, sizeof (BC_DTM_FEATURE));
    remember (stored_pointArrays, pointArrayP, stored_headerData.memPoints, stored_headerData.pointPartitionSize, sizeof (DPoint3d));
    remember (stored_nodeArrays, nodeArrayP, stored_headerData.memNodes, stored_headerData.nodePartitionSize, sizeof (DTM_TIN_NODE));
    remember (stored_cListArrays, clistArrayP, stored_headerData.memClist, stored_headerData.clistPartitionSize, sizeof (DTM_CIR_LIST));
    remember (stored_fListArrays, flistArrayP, stored_headerData.memFlist, stored_headerData.flistPartitionSize, sizeof (DTM_FEATURE_LIST));
    remembered = true;
    }

void CompareDTM (
    void* headerData,
    bvector<void*>& featureArrays,
    bvector<void*>& pointArrays,
    bvector<void*>& nodeArrays,
    bvector<void*>& cListArrays,
    bvector<void*>& fListArrays)
        {
        if (!remembered)
            return;

        BC_DTM_OBJ* dtm = (BC_DTM_OBJ*)headerData;
        compare (stored_featureArrays, featureArrays, dtm->memFeatures, dtm->featurePartitionSize, sizeof (BC_DTM_FEATURE));
        compare (stored_pointArrays, pointArrays, dtm->memPoints, dtm->pointPartitionSize, sizeof (DPoint3d));
        compare (stored_nodeArrays, nodeArrays, dtm->memNodes, dtm->nodePartitionSize, sizeof (DTM_TIN_NODE));
        compare (stored_cListArrays, cListArrays, dtm->memClist, dtm->clistPartitionSize, sizeof (DTM_CIR_LIST));
        compare (stored_fListArrays, fListArrays, dtm->memFlist, dtm->flistPartitionSize, sizeof (DTM_FEATURE_LIST));
        }
#endif

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/13
//=======================================================================================
bool DTMDataRefXAttribute::HasScheduledDTMHeader (ElementHandleCR element)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER);
    if (element.AnyXAttributeChanges ())
        {
        XAttributeChangeSet::T_Iterator it = element.GetXAttributeChangeSet ()->Find (handlerId, 0);
        
        if (it != element.GetXAttributeChangeSet ()->End () && it->GetChangeType () != XAttributeChange::CHANGETYPE_Delete)
            return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
RefCountedPtr<DTMDataRef> DTMDataRefXAttribute::FromElemHandle (ElementHandleCR graphicElement, ElementHandleCR element)
    {
    if (DTMDataRefXAttribute::HasScheduledDTMHeader (element))
        return new DTMDataRefXAttribute (graphicElement, element);

    if (DTMXAttributeHandler::HasDTMData (element))
        {
        DTMDataRefXAttribute* dtmDataRef = new DTMDataRefXAttribute (graphicElement, element);

        BeAssert(DTMDataRef::GetDTMAppData (element) == nullptr);

        DTMDataRef::AddDTMAppData (element, dtmDataRef);
        return dtmDataRef;
        }
    return nullptr;

    //DTMDataRefCache* cache = dynamic_cast<DTMDataRefCache*>(DTMDataRefCachingManager::Get (dataEl));

    //if (elemHandle.GetElementRef() != nullptr && elemHandle.GetElementCP()->ehdr.uniqueId != 0)
    //    {
    //    if (cache)
    //        {
    //        // Shouldn't happen unless called straight from interface.
    //        return cache->GetDataRef(elemHandle, dataEl);
    //        }
    //    }

    //if (HasDTMData(dataEl))
    //    {
    //    if (dataEl.GetElementRef() != nullptr && dataEl.GetElementCP()->ehdr.uniqueId != 0)
    //        {
    //        cache = new DTMDataRefXAttributeCache();
    //        DTMDataRefCachingManager::Add(dataEl, cache);
    //        return cache->GetDataRef(elemHandle, dataEl);
    //        }
    //    return new DTMDataRefXAttribute(elemHandle, dataEl);
    //    }
    //return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefXAttribute::DTMDataRefXAttribute (ElementHandleCR graphicalElement, ElementHandleCR dataEl)
    {
    m_allocator = nullptr;
    m_graphicalElement = graphicalElement;
    m_element = dataEl;
    m_dtmChanged = false;
    m_disposed = false;
    m_tileLastModified = 0;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefXAttribute::~DTMDataRefXAttribute()
    {
    Dispose();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDataRefXAttribute::Dispose()
    {
    if (m_disposed)
        return;

#ifdef DEBUG_MEMCHECK
        if (m_dtm.IsValid())
            {
        BC_DTM_OBJ* tinP = (BC_DTM_OBJ*)m_dtm.get()->GetTinHandle();
        if (bcdtmCheck_tinComponentDtmObject(tinP) != DTM_SUCCESS)
            {
            BeAssert(false);
            }
        }
#endif

    if (m_allocator)
        delete m_allocator;
    m_element = ElementHandle();
    m_graphicalElement = ElementHandle();
    m_disposed = true;

    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
StatusInt DTMDataRefXAttribute::ScheduleFromDtm (EditElementHandleR element, ElementHandleCP templateElement, BcDTMR bcDTM, TransformCR trsf, DgnModelRefR modelRef, bool disposeDTM)
    {
    EditElementHandle elemHandle(element, false);
    DTMElementHandlerManager::CheckAndCreateElementDescr (elemHandle, templateElement, DTMElementHandler::GetElemHandlerId(), trsf, modelRef);
    int status = DTMXAttributeHandler::ScheduleDtmData (elemHandle, bcDTM, disposeDTM);
    if (status == SUCCESS)
        element.SetElementDescr (elemHandle.ExtractElementDescr(), true, false);
    return status;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  07/10
//=======================================================================================
StatusInt DTMDataRefXAttribute::ReplaceDTM (BcDTMR bcDTM, bool disposeDTM)
    {
    if (nullptr != m_allocator &&  &bcDTM == m_allocator->GetDTM ())
        return SUCCESS;

    EditElementHandle element (this->GetElement(), false);
    StatusInt status = DTMXAttributeHandler::ScheduleDtmData (element, bcDTM, disposeDTM, m_allocator);
    return status;
    }

//=======================================================================================
// @bsiclass                                                    Daryl.Holmwood  08/11
//=======================================================================================
double DTMDataRefXAttribute::_GetLastModified()
   {
   Int64 time;
   GetDTMStorage (Bentley::TerrainModel::Element::GetRange)->GetBcDTM()->GetLastModifiedTime(time);

   time -= FILETIME_1_1_1970;   // re-base
   time /= UMILLIS_TO_FTI;      // 100-nanosecond interval -> millisecond
   return (double)time;
   }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/12
//=======================================================================================
void DTMDataRefXAttribute::ProcessAddedElementWithReference (EditElementHandleR element)
    {
    ElementHandle::XAttributeIter xAttrHandle(element, TMReferenceXAttributeHandler::GetXAttributeHandlerId (), TMReferenceXAttributeHandler::GetXAttributeId());

    // Get the Reference data if there is one.
    void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);

    if (data)
        {
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        pep.Load (source);
        DgnModelRef* model = GetModelRef(element);
        if (model)
            {
            // Get the element ref of the DTM Data, note we need to use _DisclosePointers as this allows getting of deleted Element Refs.
            T_StdElementRefSet refs;
            pep.DisclosePointers (&refs, model);

            if (refs.size() != 0)
                {
                ElementRefP ref = *refs.begin();

                // If it is deleted then we can just undelete it.
                if (!ref)
                    {
                    }
                else if (ref->IsDeleted ())
                    {
                    ref->UndeleteElement();
                    ElementHandle element (ref, NULL);
                    TMReferenceXAttributeHandler::AddReferenceCount (element);
                    }
                else
                    {
                    // Otherwise we need to copy the data, as we now only have one dtm element to dtm data..
                    ElementHandle originalDataEl (ref, NULL);
                    ElementHandle::XAttributeIter xAttrdataHandle (originalDataEl);

                    // copy the dtm element and attach it.
                    Transform trsf;
                    EditElementHandle dtmDataEl;

                    ElementHandle::XAttributeIter xAttrRefCount (originalDataEl, TMReferenceXAttributeHandler::GetRefCountXAttributeHandlerId (), TMReferenceXAttributeHandler::GetRefCountXAttributeId());

                    if (xAttrRefCount.IsValid())
                        {
                        dtmDataEl.SetModelRef (element.GetModelRef());
                        DTMElementHandlerManager::GetStorageToUORMatrix (trsf, element);
                        DTMElementHandlerManager::CheckAndCreateElementDescr107 (dtmDataEl, DTMElement107Handler::GetElemHandlerId(), trsf, *element.GetModelRef());
                        DTMElementHandlerManager::AddToModelInOwnBlock (dtmDataEl, element.GetModelRef());

                        while (xAttrdataHandle.IsValid())
                            {
                            if (xAttrdataHandle.GetHandlerId() != XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_TRANSLATION) && 
                                xAttrdataHandle.GetHandlerId() != XAttributeHandlerId (ElementHandlerXAttribute::XATTRIBUTEHANDLERID,0))
                                {
                                const void* data = xAttrdataHandle.PeekData ();
                                UInt32 size = xAttrdataHandle.GetSize ();

                                ITxnManager::GetCurrentTxn().AddXAttribute (dtmDataEl.GetElementRef(), xAttrdataHandle.GetHandlerId(), xAttrdataHandle.GetId(), data, size);
                                }
                            xAttrdataHandle.ToNext();
                            }
                        // If this already exists on the element ref then delete it, but ignore the delete XAttribute change.
                        if (xAttrHandle.GetElementXAttributeIter())
                            {
                            ElementRefP elRef;
                            TMReferenceXAttributeHandler::SetIgnoreXAttributeHandlerDelete (elRef = element.GetElementRef () == nullptr ? element.GetElementDescrCP()->h.elementRef : element.GetElementRef());
                            XAttributeHandle xAttribHandle (elRef, TMReferenceXAttributeHandler::GetXAttributeHandlerId (), TMReferenceXAttributeHandler::GetXAttributeId());
                            ITxnManager::GetCurrentTxn().DeleteXAttribute (xAttribHandle);
                            TMReferenceXAttributeHandler::SetIgnoreXAttributeHandlerDelete (nullptr);
                            }
                        TMReferenceXAttributeHandler::SetDTMDataReference (element, dtmDataEl);
                        }
                    }
                }
            }
        }
    }

//=======================================================================================
// @bsiclass                                                    Daryl.Holmwood  04/10
//=======================================================================================
struct DTMQvCacheTileDetails : public DTMQvCacheDetailsRange
    {
    public: DRange3d m_expandedTileRange;
    public: DTMQvCacheTileDetails (int index, DRange3dCR range, DRange3dCR expandedTileRange) : DTMQvCacheDetailsRange (range), m_expandedTileRange (expandedTileRange)
        {
        m_useContainedRange = false;
        m_tileIndex = index;
        }

    public: virtual bool UseCache (DTMQvCacheDetails* details) override
        {
        DTMQvCacheTileDetails* details2 = dynamic_cast<DTMQvCacheTileDetails*>(details);
        if (!details2) return false;
        if (!DTMQvCacheDetailsRange::UseCache (details))
            return false;
        return m_tileIndex == details2->m_tileIndex;
        }
    };

//=======================================================================================
// @bsiclass                                                    Daryl.Holmwood  04/10
//=======================================================================================
struct DTMQvCacheDetailsRangeTiling : public DTMQvCacheDetailsRange, public DTMQvCacheTilingDetails
    {
private:
    DRange3d m_fullRange;
    bvector<RefCountedPtr<DTMQvCacheTileDetails>> m_tiles;

    public: DTMQvCacheDetailsRangeTiling (DRange3d range, DRange3d fullRange, bvector<RefCountedPtr<DTMQvCacheTileDetails>> const & tiles) : DTMQvCacheDetailsRange (range)
        {
        m_fullRange = fullRange;
        if (tiles.size() == 1)
            range = fullRange;

        CalcTiles (tiles);
        }

    public: ~DTMQvCacheDetailsRangeTiling()
        {
        }

    public: virtual DTMQvCacheTilingDetails* GetTilingDetails() override
        {
        m_useContainedRange = false;
        return this;
        }

    public: virtual size_t GetNumberOfTiles()
        {
        return m_tiles.size();
        }
    public: virtual DTMQvCacheDetails* GetTileDetail(unsigned int index)
        {
        return m_tiles[index].get();
        }
    public: virtual void GetTileFence(unsigned index, DPoint3d& lowPt, DPoint3d& highPt)
        {
        lowPt = m_tiles[index]->range.low;
        highPt = m_tiles[index]->range.high;
        }
    private: void CalcTiles (bvector<RefCountedPtr<DTMQvCacheTileDetails>> const & tiles)
        {
        for (size_t i = 0; i < tiles.size(); i++)
            {
            DRange3d* tileRange = &tiles[i]->m_expandedTileRange;

            if (range.high.x >= tileRange->low.x && range.low.x <= tileRange->high.x && range.high.y >= tileRange->low.y && range.low.y <= tileRange->high.y)
                m_tiles.push_back (tiles[i]);
            }
        }
    };

#ifndef GRAPHICCACHE
//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMQvCacheDetails* DTMDataRefXAttribute::_GetDTMDetails(ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo)
    {
    DRange3d drange;
    DRange3d retRange;

    Bentley::TerrainModel::IDTM* dtm = GetDTMStorage (Bentley::TerrainModel::Element::GetRange);

    if (dtm)
        {
        dtm->GetRange (drange);
        if (drawingInfo.GetFence().numPoints)
            retRange.initFrom (drawingInfo.GetFence().points, drawingInfo.GetFence().numPoints);
        else
            retRange = drange;

        //DTMDataRefXAttributeCache* cache = dynamic_cast<DTMDataRefXAttributeCache*>(DTMDataRefCachingManager::Get (m_element));

        //if (cache)
            return new DTMQvCacheDetailsRangeTiling (retRange, drange, GetTiles (dtm->GetBcDTM ()));            

//        return new DTMQvCacheDetailsRangeTiling (drange/*retRange*/, drange, dtm->GetPointCount());
        }
    return nullptr;
    }
#endif

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
bool DTMDataRefXAttribute::_GetExtents (DRange3dR range)
    {
    IDTM* dtm = GetDTMStorage (Bentley::TerrainModel::Element::GetRange);

    if (dtm->GetRange (range) != DTM_SUCCESS)
        return false;

    return !range.IsEmpty ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
IDTM* DTMDataRefXAttribute::_GetDTMStorage (DTMDataRefPurpose purpose, ViewContextR context)
    {
    if (m_disposed)
        return nullptr;

    return GetDTMStorage(purpose);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
IDTM* DTMDataRefXAttribute::_GetDTMStorage (DTMDataRefPurpose purpose)
    {
    if (m_disposed)
        return nullptr;

    if (!m_allocator)
        {
        // Load the DTM
        m_allocator = DTMXAttributeHandler::LoadDTM (m_element);

        if (!m_allocator)
            return nullptr;
        }

    return m_allocator->GetDTM();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMDataRefXAttribute::_GetDTMReferenceStorage(RefCountedPtr<IDTM>& outDtm)
    {
    outDtm = GetDTMStorage (None);
    return outDtm.IsValid() ? SUCCESS : ERROR;
    }

int addToRange (DTMFeatureType dtmFeatureType,long numTriangles,long numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,long numMeshFaces, long *meshFacesP,void *userP)
    {
    DRange3d* range = (DRange3d*)userP;
    range->Extend (meshPtsP, numMeshPts);
    return DTM_SUCCESS;
    }

bvector<RefCountedPtr<DTMQvCacheTileDetails>> const& DTMDataRefXAttribute::GetTiles (BcDTMP dtm) const
    {
    Int64 lastModified;
    dtm-> GetLastModifiedTime (lastModified);

    if (m_tileLastModified != lastModified)
        {
        m_tileLastModified = lastModified;
        m_tiles.clear();
        DRange3d fullRange;
        static const int s_tilePointSize = 25000;
        int m_numTilesX;
        int m_numTilesY;
        Int64 numberOfPoints = dtm->GetPointCount ();

        dtm->GetRange (fullRange);
        if (numberOfPoints > s_tilePointSize)
            {
            BeAssert(numberOfPoints <= INT_MAX);
            double numOfTiles = (int)numberOfPoints / s_tilePointSize;
            int iNumOfTiles = (int)(sqrt(numOfTiles) + 0.5);

            m_numTilesX = iNumOfTiles;
            m_numTilesY = iNumOfTiles;
            }
        else
            {
            m_numTilesX = 1;
            m_numTilesY = 1;
            }

            double gapX = fullRange.high.x - fullRange.low.x;
            double gapY = fullRange.high.y - fullRange.low.y;

            gapX /= m_numTilesX;
            gapY /= m_numTilesY;
            double x = fullRange.low.x;
            DRange3d tileRange;
            tileRange.low.z = 0;
            tileRange.high.z = 0;
            for (int dx = 0; dx < m_numTilesX; dx++)
                {
                tileRange.low.x = x;
                tileRange.high.x = x + gapX;
                double y = fullRange.low.y;
                for (int dy = 0; dy < m_numTilesY; dy++)
                    {
                    tileRange.low.y = y;
                    tileRange.high.y = y + gapY;
                    DRange3d expandedTileRange = tileRange;

                    DPoint3d fencePts[5];
                    fencePts[0].x = tileRange.low.x; fencePts[0].y = tileRange.low.y;
                    fencePts[1].x = tileRange.high.x; fencePts[1].y = tileRange.low.y;
                    fencePts[2].x = tileRange.high.x; fencePts[2].y = tileRange.high.y;
                    fencePts[3].x = tileRange.low.x; fencePts[3].y = tileRange.high.y;
                    fencePts[4] = fencePts[0];

                    DTMFenceParams fence (DTMFenceType::Block, DTMFenceOption::Overlap, (DPoint3d*)fencePts, 5);
                    DTMMeshEnumeratorPtr en = DTMMeshEnumerator::Create (*dtm);
                    en->SetFence (fence);
                    en->SetMaxTriangles (126000 / 3);
                    en->SetTilingMode (true);
                    expandedTileRange = en->GetRange ();
                    m_tiles.push_back (new DTMQvCacheTileDetails ((dx * 0x10000) + dy, tileRange, expandedTileRange));
                    y += gapY;
                    }
                x += gapX;
                }
        }
    return m_tiles;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
