/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ProxyDisplayCore.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <RmgrTools/Tools/DataExternalizer.h>

DGNPLATFORM_TYPEDEFS (HLCurveId)
DGNPLATFORM_TYPEDEFS (HLIdArray)
DGNPLATFORM_TYPEDEFS (ViewHandlerPass)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//typedef std::map <DgnModelP,  struct ProxyDisplayModel*>     T_ProxyModelMap;
typedef std::map <ClipVolumePass, Int32>                        T_ProxyDisplayStyleMap;
typedef bvector</*ElementId*/UInt64>                                      T_ProxyElementIdVector;
//typedef bvector<ProxyDisplayPathP>                              T_ProxyDisplayPathVector;


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
enum HLCurveFlags
{
    HLCurveFlags_None                   = 0,
    HLCurveFlags_Wire                   = 0x0001 << 0,                // Curve is a wire (as opposed to an edge).
    HLCurveFlags_Silhouette             = 0x0001 << 1,                // Curve is a silhouette.
    HLCurveFlags_Boundary               = 0x0001 << 2,                // Curve is a boundary between faces.
    HLCurveFlags_Smooth                 = 0x0001 << 3,                // Curve is "smooth" boundary between faces.
    HLCurveFlags_ChordalApproximation   = 0x0001 << 4,                // Polyline is a choordal appproximation of a curve.
    HLCurveFlags_Intersection           = 0x0001 << 5,                // Surface-Surface intersection curve.
    HLCurveFlags_GeometryMap            = 0x0001 << 6,
    HLCurveFlags_RuleLine               = 0x0001 << 7,
    HLCurveFlags_Hidden                 = 0x0001 << 8,
    };


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
enum    ProxyGraphicsType
    {
    ProxyGraphicsType_Min                    = 0,
    ProxyGraphicsType_VisibleEdge            = 1,
    ProxyGraphicsType_HiddenEdge             = 2,
    ProxyGraphicsType_VisibleWire            = 3,
    ProxyGraphicsType_HiddenWire             = 4,
    ProxyGraphicsType_PassthroughAnnotation  = 5,
    ProxyGraphicsType_CutFill                = 6,
    ProxyGraphicsType_Cut                    = 7,
    ProxyGraphicsType_PassthroughUnderlay    = 8,
    ProxyGraphicsType_Max
    };


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   03/2012
+===============+===============+===============+===============+===============+======*/
//struct ProxyDgnAttachmentHandler
//{
//protected:
//    virtual void                            _Push (ViewContextR viewContext, DgnAttachmentCR dgnAttachment) const  = 0;
//    virtual ProxyDisplayCacheBaseP          _RestoreProxyCache (EditElementHandleR eh, DgnAttachmentR attachment)  const = 0;
//    virtual StatusInt                       _GetReferenceRange (DRange3dR range, DgnAttachmentCR ref) const = 0;
//
//
//public:
//    void                                    Push (ViewContextR viewContext, DgnAttachmentCR dgnAttachment) const          { return _Push (viewContext, dgnAttachment); }
//    ProxyDisplayCacheBaseP                  RestoreProxyCache (EditElementHandleR eh, DgnAttachmentR attachment) const    { return _RestoreProxyCache (eh, attachment); }
//    StatusInt                               GetReferenceRange (DRange3dR range, DgnAttachmentCR ref) const                { return _GetReferenceRange (range,ref); }
//
//
//};  // ProxyDgnAttachmentHandler
//
//
//
//typedef bmap <XAttributeHandlerId, ProxyDgnAttachmentHandlerCP>             T_ProxyDgnAttachmentHandlerMap;
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      10/2011
//*  Manager class for DisplayStyleManager.  A display handler is registered
//*  by calling    ProxyDgnAttachmentHandlerManager::GetManager().RegisterHandler()
//+===============+===============+===============+===============+===============+======*/
//struct ProxyDgnAttachmentHandlerManager
//{
//private:
//        T_ProxyDgnAttachmentHandlerMap     m_map;
//
//public:
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod
//*   Return reference to (singleton) ProxyDgnAttachmentHandlerManager
//+---------------+---------------+---------------+---------------+---------------+------*/
//DGNPLATFORM_EXPORT static ProxyDgnAttachmentHandlerManagerR     GetManager();
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod
//*   Register a ProxyDgnAttachmentHandler manager.   Should be called once per session.
//+---------------+---------------+---------------+---------------+---------------+------*/
//DGNPLATFORM_EXPORT void                                   RegisterHandler (ProxyDgnAttachmentHandlerCR handler, XAttributeHandlerId id);
//DGNPLATFORM_EXPORT ProxyDgnAttachmentHandlerCP            GetHandler (XAttributeHandlerId id); 
//
//};  // DisplayStyleHandlerManager                               
//
//
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     11/2011
+===============+===============+===============+===============+===============+======*/
//struct ProxyDisplayPath
//{
//    DgnModelP                m_rootModel;
//    T_ProxyElementIdVector      m_path;
//
//                   ProxyDisplayPath (DgnModelP rootModel) : m_rootModel (rootModel) { }
//DGNPLATFORM_EXPORT     ProxyDisplayPath (DisplayPath const& displayPath);
//DGNPLATFORM_EXPORT     bool IsSamePath (ProxyDisplayPathCR other) const;
//};

///*=================================================================================**//**
//* @bsiclass                                                     Ray.Bentley   06/2007
//+===============+===============+===============+===============+===============+======*/
//enum ProxySynchStatus
//{
//    ProxySynchStatus_Unchanged          = 0,
//    ProxySynchStatus_Modified           = 1,
//    ProxySynchStatus_Deleted            = 2,
//};
//
//
//
///*=================================================================================**//**
//* @bsiclass                                                     Ray.Bentley   11/2011
//+===============+===============+===============+===============+===============+======*/
struct ProxyDisplayCacheBase // graphite: this class is used only in the importer -- never at run time
{
friend struct ProxyCacheHolder;
       protected:
       DgnModelP                    m_rootModel;

    //virtual double                          _GetRootUnitScaleFactor () const = 0;
    //virtual bool                            _IsObsoleteVersion() const = 0;
    //virtual bool                            _ContainsModel (DgnModelCR) const = 0;
    //virtual StatusInt                       _GetRange (DRange3dR range) const = 0;
    //virtual double                          _GetCreationTime () const = 0;
    //virtual StatusInt                       _SetAlwaysValid (DgnModelP refModel, ElementRefP) = 0;    // used when we're accepting the cache for use in an iModel.
    //virtual bool                            _GetAlwaysValid () const = 0;
    //virtual StatusInt                       _SetAlwaysInvalid (DgnModelP refModel, ElementRefP) = 0;  // used when we're accepting an out-of-date cache for inclusion in an iModel.
    //virtual bool                            _GetAlwaysInvalid () const = 0;
    //virtual bool                            _AnyCachedChildReferencesMissing () const = 0;
    //virtual StatusInt                       _Save (DgnModelP rootModel, ElementRefP) const = 0;
    //virtual ProxyCacheStatus                _GetCacheStatusForViewport (ViewportP viewport) const = 0;
    //virtual void                            _ClearValidityState () = 0;
    //virtual ModelInfoCP                     _GetModelInfoCP (DgnModelCR) const = 0;
    //virtual WStringCP                       _GetLevelName (DgnModelP modelRef, LevelId levelId) const = 0;
    //virtual LevelCacheP                     _GetLevelCache (DgnModelCR) = 0;
    //virtual StatusInt                       _GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, T_ProxyElementIdVector const& attachIds, T_ProxyElementIdVector const& pathIds, ProxyEdgeIdDataCR edgeId, struct GPArrayParam const& edgeParam, ViewHandlerPassCR vhPass) const = 0;

                                            ProxyDisplayCacheBase () : m_rootModel (NULL) { }
                                            ProxyDisplayCacheBase (DgnModelP rootModel) : m_rootModel (rootModel) { }
    virtual                                 ~ProxyDisplayCacheBase () { }

public:

    //DGNPLATFORM_EXPORT bool                     IsObsoleteVersion() const;
    //DGNPLATFORM_EXPORT double                   GetRootUnitScaleFactor () const;
    //DGNPLATFORM_EXPORT bool                     ContainsModel (DgnModelCR model) const;
    //DGNPLATFORM_EXPORT double                   GetCreationTime () const;
    //DGNPLATFORM_EXPORT StatusInt                GetRange (DRange3dR range) const;
    //DGNPLATFORM_EXPORT StatusInt                SetAlwaysValid (DgnModelP refModel, ElementRefP elementRef = NULL);
    //DGNPLATFORM_EXPORT bool                     GetAlwaysValid () const;
    //DGNPLATFORM_EXPORT StatusInt                SetAlwaysInvalid (DgnModelP refModel, ElementRefP elementRef = NULL);
    //DGNPLATFORM_EXPORT bool                     GetAlwaysInvalid () const;
    //DGNPLATFORM_EXPORT bool                     AnyCachedChildReferencesMissing () const;
    //DGNPLATFORM_EXPORT StatusInt                Save (DgnModelP rootModel, ElementRefP = NULL);

    //DGNPLATFORM_EXPORT void                     ClearValidityState ();

    DGNPLATFORM_EXPORT DgnModelP             GetRootModel () const { return m_rootModel; }
    //DGNPLATFORM_EXPORT DgnAttachmentP           GetRootRefFile () const;
    //DGNPLATFORM_EXPORT DgnModelP             GetParentDgnModelP () const;
    //DGNPLATFORM_EXPORT DgnProjectP                 GetParentDgnFile () const;
    //DGNPLATFORM_EXPORT ProxyCacheStatus         GetCacheStatusForViewport (ViewportP viewport) const;
    //DGNPLATFORM_EXPORT bool                     UsableForViewport (ViewportR viewport) const;
    //DGNPLATFORM_EXPORT static bool              IsValidForViewContext (ViewContextR viewContext, bool isTransientDisplay = false);
    //DGNPLATFORM_EXPORT bool                     IsValidForViewport (ViewportR viewport) const;
    //static bool                             OwnsXAttribute (XAttributeHandlerId handlerId, UInt32 xAttrId);
    //DGNPLATFORM_EXPORT ModelInfoCP              GetModelInfoCP (DgnModelCR) const;
    //DGNPLATFORM_EXPORT LevelCacheP              GetLevelCache (DgnModelCR);
    //DGNPLATFORM_EXPORT WStringCP                GetLevelName (DgnModelP modelRef, LevelId levelId) const;
    //DGNPLATFORM_EXPORT StatusInt                GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, T_ProxyElementIdVector const& attachIds, T_ProxyElementIdVector const& pathIds, ProxyEdgeIdDataCR edgeId, struct GPArrayParam const& edgeParam, ViewHandlerPassCR vhPass) const;


};  // ProxyDisplayCacheBase

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   06/2007
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphicsFlags
{

enum
    {
    ProxyGraphicsFlags_ConstructionClass = 0x0001 << 0,
    ProxyGraphicsFlags_SolidFillEdge     = 0x0001 << 1,
    ProxyGraphicsFlags_Precalculated     = 0x0001 << 2,
    };

private:
    UInt16              m_value;

public:
                 ProxyGraphicsFlags () : m_value (0) { }
DGNPLATFORM_EXPORT   ProxyGraphicsFlags (bool constructionClass, bool solidFillEdge, bool isPrecalculated);

    bool         IsConstructionClass ()  { return 0 != (m_value & ProxyGraphicsFlags_ConstructionClass); }
    bool         IsSolidFillEdge ()      { return 0 != (m_value & ProxyGraphicsFlags_SolidFillEdge); }
    bool         IsPrecalculated ()      { return 0 != (m_value & ProxyGraphicsFlags_Precalculated); }

}; //  ProxyGraphicsFlags



/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
struct  ProxyHLEdgeId
{
    UInt16                  m_flags;
    CurveTopologyId         m_id;

                                            ProxyHLEdgeId () :  m_flags (0) { }
                                            ProxyHLEdgeId (CurveTopologyIdCR curveId, UInt32 geometryIndex, UInt16 flags) : m_id (geometryIndex, curveId), m_flags (flags) { }
                                            ProxyHLEdgeId (void const* data, size_t dataBytes) { Init (data, dataBytes); }
                                            ProxyHLEdgeId (ProxyHLEdgeIdCR id, UInt32 segmentIndex) : m_flags (id.m_flags), m_id (id.m_id, segmentIndex) { }

    bool                                    operator == (ProxyHLEdgeIdCR rhs) const { return m_id == rhs.m_id && m_flags == rhs.m_flags; }
    DGNPLATFORM_EXPORT bool                     operator <  (ProxyHLEdgeIdCR rhs) const;
    DGNPLATFORM_EXPORT                          void Dump () const;
    DGNPLATFORM_EXPORT                          void Save (struct ProxyDataBuffer& data) const;

    DGNPLATFORM_EXPORT bool                     IsAssociable () const;
    inline bool                             IsWire() const              { return 0 != (m_flags & HLCurveFlags_Wire); }
    inline bool                             IsEdge() const              { return 0 == (m_flags & HLCurveFlags_Wire); }
    inline bool                             IsVisible () const          { return 0 == (m_flags & HLCurveFlags_Hidden); }
    inline bool                             IsSilhouette ()const        { return 0 != (m_flags & HLCurveFlags_Silhouette); }
    inline bool                             IsApproximate() const       { return 0 != (m_flags & HLCurveFlags_ChordalApproximation); }
    inline bool                             IsIntersection () const     { return 0 != (m_flags & HLCurveFlags_Intersection); }
    inline bool                             IsGeometryMap () const      { return 0 != (m_flags & HLCurveFlags_GeometryMap); }
    inline CurveTopologyIdCR                GetTopologyId () const      { return m_id; }

    DGNPLATFORM_EXPORT void                     Init (void const* data, size_t dataBytes);
    DGNPLATFORM_EXPORT ProxyGraphicsType        GetProxyGraphicsType() const;
    DGNPLATFORM_EXPORT WString                  ToString () const;
};


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
struct  ProxyEdgeIdData
{
private:
    bvector<byte>       m_data;

public:
    byte const*                             GetData() const { return m_data.empty() ? NULL : &m_data[0]; }
    size_t                                  GetSize() const { return m_data.size(); }
    size_t                                  GetAnnounceSize () const { return m_data.empty() ?  0 : m_data.size() - 1; }        // This does not include the first (type) byte.
    byte const*                             GetAnnounceData () const { return m_data.empty() ? NULL : &m_data[1]; }             // This does not include the first (type) byte.
    bool                                    operator == (ProxyEdgeIdDataCR rhs) const { return m_data == rhs.m_data; }

    DGNPLATFORM_EXPORT void                     Store (DataExternalizer& sink) const;
    DGNPLATFORM_EXPORT StatusInt                Load (DataLoader& source);
    DGNPLATFORM_EXPORT bool                     Matches (void const* edgeData, size_t edgeDataBytes) const;
    DGNPLATFORM_EXPORT StatusInt                Init (void const* edgeData, size_t edgeDataBytes);
    DGNPLATFORM_EXPORT StatusInt                Init (ProxyEdgeIdCR edgeId);
    DGNPLATFORM_EXPORT CurvePrimitiveId::Type   GetType() const { return (CurvePrimitiveId::Type) m_data[0]; }

};  // ProxyEdgeIdData


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
struct    ProxyHLEdgeSegmentId
{
    float                   m_startParam;
    float                   m_endParam;
    ProxyHLEdgeId           m_edgeId;

public:                                     ProxyHLEdgeSegmentId () : m_startParam (0.0f), m_endParam (1.0f) { }
    DGNPLATFORM_EXPORT                          ProxyHLEdgeSegmentId (ProxyHLEdgeIdCR, struct GPArrayInterval const& interval);
    DGNPLATFORM_EXPORT StatusInt                Init (void const* edgeData, size_t edgeDataBytes);
    DGNPLATFORM_EXPORT StatusInt                Init (CurvePrimitiveIdCR id);
    DGNPLATFORM_EXPORT StatusInt                InitFromCurveIdData (void const* edgeData, size_t edgeDataBytes);
    DGNPLATFORM_EXPORT StatusInt                Init (ProxyEdgeIdDataCR edgeId);
    bool                                    Equals (ProxyHLEdgeSegmentId rhs) const { return m_edgeId == rhs.m_edgeId && m_startParam == rhs.m_startParam && m_endParam == rhs.m_endParam; }
    DGNPLATFORM_EXPORT void                     Save (ProxyEdgeIdDataR) const;

    inline ProxyHLEdgeIdCR                  GetId ()            { return m_edgeId; }
    inline float                            GetStartParam ()    { return m_startParam; }
    inline float                            GetEndParam ()      { return m_endParam; }
    inline bool                             IsComplete () const { return m_startParam == 0.0f && m_endParam == 1.0f; } 
    DGNPLATFORM_EXPORT GPArrayInterval          GetInterval () const;
    DGNPLATFORM_EXPORT WString                  ToString () const;
    bool                                    operator == (ProxyHLEdgeSegmentIdCR rhs) const { return m_edgeId == rhs.m_edgeId && m_startParam == rhs.m_startParam && m_endParam == rhs.m_endParam; }
    DGNPLATFORM_EXPORT bool                     operator <  (ProxyHLEdgeSegmentIdCR rhs) const;

};  // ProxyHLEdgeSegmentId


#define     ViewHandlerPass_Underlay ClipVolumePass::Maximum

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
+===============+===============+===============+===============+===============+======*/
struct CachedVisibleEdgeHandlerId  : XAttributeHandlerId
{
    DGNPLATFORM_EXPORT CachedVisibleEdgeHandlerId ();
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2011
+===============+===============+===============+===============+===============+======*/
struct CachedVisibleEdgeCacheId  : XAttributeHandlerId
{
    DGNPLATFORM_EXPORT CachedVisibleEdgeCacheId ();
};


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   11/2011
+===============+===============+===============+===============+===============+======*/
struct ViewHandlerPass // graphite: this class is used only the importer, never at run time
{
    ClipVolumePass      m_pass;
    UInt32              m_clipPlaneIndex;

                            ViewHandlerPass () : m_pass (ClipVolumePass::None), m_clipPlaneIndex (0) { }
                            ViewHandlerPass (ClipVolumePass pass, UInt32 planeIndex=0) : m_pass (pass), m_clipPlaneIndex (planeIndex) { }
DGNPLATFORM_EXPORT          ViewHandlerPass (ViewContextR viewContext);
DGNPLATFORM_EXPORT void     Init (ViewContextR viewContext);
inline ClipVolumePass       GetPass() const { return m_pass; }

DGNPLATFORM_EXPORT bool operator == (ViewHandlerPassCR rhs) const;
DGNPLATFORM_EXPORT bool operator < (ViewHandlerPassCR rhs) const;

};  // ViewHandlerPass


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyRestoreUtil
{
class  ReadError { };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static StatusInt CopyData (void* destination, size_t size, byte const*& dataP, byte const* dataEndP);
                    static StatusInt CopyData (TransformR value, byte const*& dataP, byte const* dataEndP)                                         { return CopyData (&value, sizeof (value), dataP, dataEndP); }
                    static StatusInt CopyData (Int32& value, byte const*& dataP, byte const* dataEndP)                                             { return CopyData (&value, sizeof (value), dataP, dataEndP); }
                    static StatusInt CopyData (UInt16& value, byte const*& dataP, byte const* dataEndP)                                            { return CopyData (&value, sizeof (value), dataP, dataEndP); }
                    static StatusInt CopyData (UInt32& value, byte const*& dataP, byte const* dataEndP)                                            { return CopyData (&value, sizeof (value), dataP, dataEndP); }
                    static StatusInt CopyData (double& value, byte const*& dataP, byte const* dataEndP)                                            { return CopyData (&value, sizeof (value), dataP, dataEndP); }
                    static StatusInt CopyData (/*ElementId*/UInt64& value, byte const*& dataP, byte const* dataEndP)                                         { return CopyData (&value, sizeof (value), dataP, dataEndP); }
DGNPLATFORM_EXPORT  static StatusInt CopyString (WStringR string, byte const*& dataP, byte const* dataEndP);


//DGNPLATFORM_EXPORT static DgnModelP FindModelChild (DgnModelP parent, ElementId childId);

};  // ProxyRestoreUtil

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyDataBuffer
{
    bvector<byte>   m_buffer;

    byte*       GetData()       { return m_buffer.empty() ? NULL : &m_buffer[0]; }
    size_t      GetDataSize()   { return m_buffer.size(); }
    void        Clear ()        { m_buffer.resize (0); }

    size_t      Write (byte value)          { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (UInt16 value)        { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (UInt32 value)        { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (Int32 value)         { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (WChar value)         { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (float value)         { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (double value)        { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (/*ElementId*/UInt64 value)     { return Write ((const void*) &value, sizeof (value)); }
    size_t      Write (TransformCR value)   { return Write ((const void*) &value, sizeof (value)); }

    DGNPLATFORM_EXPORT size_t       Write (const void* data, size_t dataSize);
    DGNPLATFORM_EXPORT size_t       Write (WStringCR string);
    DGNPLATFORM_EXPORT void         UpdateBytesToFollow (size_t location);
    DGNPLATFORM_EXPORT StatusInt    SaveToXAttributes (ElementRefP elemRef, XAttributeHandlerId xAttrHandlerId, UInt32 xAttributeIndex);
    DGNPLATFORM_EXPORT StatusInt    SaveCompressedToXAttributes (ElementRefP elemRef, XAttributeHandlerId xAttrHandlerId, UInt32 xAttributeIndex);

};  // ProxyDataBuffer





END_BENTLEY_DGNPLATFORM_NAMESPACE
