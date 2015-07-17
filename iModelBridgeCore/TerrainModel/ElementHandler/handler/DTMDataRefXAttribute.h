/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDataRefXAttribute.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DTMXAttributeHandler.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

class DTMXAttributeHandler;
struct DTMQvCacheTileDetails;

class DTMDataRefXAttribute : public DTMDataRef
    {
private:
    DTMXAttributeHandler* m_allocator;
    bool m_dtmChanged;
    ElementHandle m_graphicalElement;
    bool m_disposed;
    bool UseTopLevel (DTMDataRefPurpose purpose, ViewContextR context);

public:
    static bool HasScheduledDTMHeader (ElementHandleCR element);
    void Dispose();
    static StatusInt ScheduleFromDtm (EditElementHandleR element, ElementHandleCP templateElement, BcDTMR bcDTM, TransformCR trfs, DgnModelRefR modelRef, bool disposeDTM);
    static void ProcessAddedElementWithReference (EditElementHandleR element);

protected:
    DTMDataRefXAttribute(ElementHandleCR graphicElement, ElementHandleCR dataEl);
    virtual ~DTMDataRefXAttribute();
public:
    static DTMDataRefXAttribute* Create (ElementHandleCR graphicElement, ElementHandleCR element)
        {
        return new DTMDataRefXAttribute (graphicElement, element);
        }

    static RefCountedPtr<DTMDataRef> FromElemHandle(ElementHandleCR graphicElement, ElementHandleCR element);

    protected: virtual double _GetLastModified() override;
    public: virtual bool _IsReadOnly() override
                {
                return false;
                }
    public: virtual bool _GetExtents (DRange3dR range) override;
    public: virtual IDTM* _GetDTMStorage(DTMDataRefPurpose purpose, ViewContextR context) override;
    public: virtual IDTM* _GetDTMStorage(DTMDataRefPurpose purpose) override;
    public: virtual StatusInt _GetDTMReferenceStorage (RefCountedPtr<IDTM>& outDtm) override;
#ifndef GRAPHICCACHE
    private: mutable bvector<RefCountedPtr<DTMQvCacheTileDetails>> m_tiles;
    private: mutable Int64 m_tileLastModified;
    private: bvector<RefCountedPtr<DTMQvCacheTileDetails>> const& GetTiles (BcDTMP dtm) const;
    public: virtual DTMQvCacheDetails* _GetDTMDetails (ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo) override;
#endif
    public: StatusInt ReplaceDTM(BcDTMR bcDTM, bool disposeDTM);   // Shouldn't be needed
    virtual ElementHandleCR GetGraphicalElement (void) const override
        {
        return m_graphicalElement;
        }
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
