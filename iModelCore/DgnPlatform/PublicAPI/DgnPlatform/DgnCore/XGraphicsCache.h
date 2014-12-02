/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XGraphicsCache.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include "XGraphics.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @bsiclass                                                     RayBentley      06/09
//=======================================================================================
struct XGraphicsSymbol : XGraphicsContainer
{
    DEFINE_T_SUPER(XGraphicsContainer)

private:
    TransformP              m_transform;
    Symbology*              m_symbology;
    ElementRefAppData*      m_qvElemSet;

public:
                                XGraphicsSymbol (TransformCP transform) : m_transform (NULL == transform ? NULL : new Transform (*transform)), m_symbology (NULL), m_qvElemSet (NULL) { }
                                XGraphicsSymbol (XGraphicsContainerR container, TransformCP transform, Symbology const* symbology);
                                ~XGraphicsSymbol ();

    TransformCP                 GetTransform () const { return m_transform; }
    bool                        IsEqual (XGraphicsContainer const& rhs, double distanceTolerance, Symbology const* symbology);
    QvElem*                     GetQvElem (ElementHandleCR eh, ViewContextR viewContext, QvCache* qvCache, double pixelSize);
    static XGraphicsSymbolP     Create (ElementHandleCR eh, IStrokeForCache& stroker, double pixelSize);

    ElementRefAppData*          GetQvElemSet () { return m_qvElemSet; }
    void                        SetQvElemSet (ElementRefAppData* qvElemSet) { m_qvElemSet = qvElemSet; }

};  // XGraphicsSymbol

//=======================================================================================
//! @bsiclass                                                     RayBentley      04/09
//=======================================================================================
struct XGraphicsSymbolId
{
private:
    ElementHandlerId        m_handlerId;
    Int32                   m_symbolId;

public:    
    XGraphicsSymbolId () : m_handlerId(), m_symbolId (0) { }
    XGraphicsSymbolId (ElementHandleCR eh, Int32 symbolId);

    bool operator==(XGraphicsSymbolId const& other) const {return (m_handlerId == other.m_handlerId && m_symbolId == other.m_symbolId);}
    bool operator< (XGraphicsSymbolId const& other) const {return (m_handlerId < other.m_handlerId) || ((m_handlerId == other.m_handlerId) && (m_symbolId < other.m_symbolId));}

};  //  XGraphicsSymbolId

//=======================================================================================
//! @bsiclass                                                     John.Gooding    04/14
//=======================================================================================
struct SymbolUseMapEntry
    {
    DgnModelId  m_firstUseModelId;
    ElementId   m_firstUseEleId;
    UInt32      m_nUses;
    SymbolUseMapEntry (DgnModelId firstUseModelId, ElementId firstUseEleId) : m_firstUseModelId(firstUseModelId), m_firstUseEleId(firstUseEleId), m_nUses(1) {}
    SymbolUseMapEntry () : m_firstUseModelId(DgnModelId()), m_firstUseEleId(ElementId()), m_nUses(0) {}
    };

//=======================================================================================
//! @bsiclass                                                     RayBentley      04/09
//=======================================================================================
struct XGraphicsSymbolCache
{

    typedef bvector<XGraphicsSymbolP>               T_Symbols;
    typedef bmap <size_t, bvector <XGraphicsSymbolP>> XGraphicsSymbolTree;
    //  Maps Symbol ID to SymbolUseMapEntry
    typedef bmap <BeRepositoryBasedId, SymbolUseMapEntry > SymbolDependentMap;

private:
    bool                      m_isDirty;
    T_Symbols                 m_symbols;
    DgnProjectR               m_dgnProject;
    SymbolDependentMap        m_dependentMap;

    // m_symbols hold the pointers. XGraphicsSymbolTree is keyed on symbol data size to speed
    // comparison time when generating symbols during publishing (since that's the only time
    // we load this structure now). Don't want to dig into symbol index etc. now so just keep
    // the two lists...
    XGraphicsSymbolTree       m_symbolsTree;

    void RemoveSymbolFromTree(XGraphicsSymbolP symbol);
    void AddSymbolToTree(XGraphicsSymbolP symbol);
    size_t GetXGraphicsSymbolDependentCount (DgnModelId&dgnModelId, BeRepositoryBasedId&clientId, bool&isStamp, BeRepositoryBasedId const& symbolId);

public:
    XGraphicsSymbolCache (DgnProjectR dgnProject);
    ~XGraphicsSymbolCache ();
    //  ElementRefP GetElementRef (UInt32 index) const { return index >= m_symbols.size() ? NULL : m_symbols[index]->GetElementRef(); }
    StatusInt AddSymbol (TransformR transform, size_t& index, XGraphicsContainerR parent, XGraphicsContainerR symbol, DgnModelR dgnModel, Symbology const* symbology);
    XGraphicsSymbolP FindOrAddMatchingSymbol (XGraphicsSymbolIdCR id, XGraphicsSymbolP symbol);

    DGNPLATFORM_EXPORT static XGraphicsSymbolCacheR Get (DgnProjectR dgnProject);
    static  XGraphicsSymbolP GetSymbol (ElementHandleCR eh, Int32 id, IStrokeForCache& stroker, double pixelSize); 
    static ElementId GetSymbolElementId (ElementHandleCR eh, Int32 id, IStrokeForCache& stroker, double pixelSize);

    DGNPLATFORM_EXPORT void Compress (DgnModelR model, bool expandSingleInstances);
    DGNPLATFORM_EXPORT void CountSymbolDependents (DgnModelR model);
    static size_t Optimize (DgnModelR model, UInt32 options);
    static void Dump (DgnModelP modelRef);
    static bool IsLoaded (DgnProjectR dgnProject);
    //  static void Free (DgnModelR dgnModel);
    static void ExtractSymbolIds (T_XGraphicsSymbolIdsR symbolIds, XGraphicsSymbolStampCR symbolStamp);
    static void ExtractSymbolIds (T_XGraphicsSymbolIdsR symbolIds, ElementHandleCR element);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
