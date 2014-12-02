/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsName.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

// this is bogus.
#if defined (DGN_PLATFORM_MT)
#undef DGN_PLATFORM_MT
#include    <RmgrTools/Tools/rmgrstrl.h>
#define DGNPLATFORM_MT
#else
#include    <RmgrTools/Tools/rmgrstrl.h>
#endif

static DgnHost::Key s_allMapsKey;
static DgnHost::Key s_systemLsFileInfoKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsDefinition::SetName
(
Utf8CP          name
)
    {
    if (m_name.m_key)
        FREE_AND_CLEAR (m_name.m_key);

    if (name)
        {
        m_name.m_key = BeStringUtilities::Strdup (name);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::SetHWStyle (UInt32 rscType, UInt64 rscID)
    {
    m_hardwareLineCode = -1;
    if (LsElementType::Internal == (LsElementType)rscType)
        {
        // Linecode only if hardware bit is set and masked value is within range.
        if ( (0 != (rscID & LSID_HARDWARE)) && ((rscID & LSID_HWMASK) <= MAX_LINECODE))
            m_hardwareLineCode = (long)rscID & LSID_HWMASK;
        else
            m_hardwareLineCode = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::Init(Utf8CP name, Json::Value& lsDefinition, Int32 styleId)
    {
    m_isDirty           = false;
    m_lsComp            = NULL;

    m_attributes        = GetAttributes(lsDefinition);
    m_unitDef           = GetUnitDef(lsDefinition);
    m_styleNumber       = styleId;

    m_hardwareLineCode  = -1;
    m_uorsPerMeter      = 0.0;
    m_maxWidth          = 0.0;
    m_componentLoadPostProcessed = false;

    m_name = NULL;
    SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::LsDefinition (Utf8CP name, DgnProjectR project, Json::Value& lsDefinition, Int32 styleId)
    {
    Init (name, lsDefinition, styleId);
    UInt32 compType = GetComponentType(lsDefinition);
    UInt32 compId = GetComponentId(lsDefinition);

    m_location.SetLocation (project, compType, compId);
    SetHWStyle (compType, compId);
    m_componentLookupFailed = false;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::~LsDefinition()
    {
    SetName (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsDefinition::Commit ()
    {
    BeAssert (0 && "unable to commit definition");
    
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 01/06
+---------------+---------------+---------------+---------------+---------------+------*/
double  LsDefinition::GetTrueScale (DgnModelP  dgnCache) const
    {
#if defined (NEEDS_WORK_VIEW_CONTROLLER)
    BeAssert (NULL != dgnCache);
    if (NULL == dgnCache || 0.0 == m_uorsPerMeter || !IsUnitsUOR() || dgnCache->IsDictionaryModel())
        return 1.0;

    return dgnModel_getUorPerMeter (dgnCache) / m_uorsPerMeter;
#endif
    return  1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString         LsDefinition::GetStyleName () const
    {
    Utf8CP   lsName = _GetName();
    if (NULL == lsName)
        return WString (L"");

    WString retval;
    BeStringUtilities::Utf8ToWChar (retval, lsName);
    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::AddIdEntry
(
Int32           id,
LsDefinitionP   nameRec,
bool            resolves
)
    {
    Utf8CP       name = nameRec->_GetName ();

    LsIdNode        idRec (id, name, nameRec, resolves);

    m_idTree.Add (&idRec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::AddIdEntry (Int32 id, DgnProjectR project, CharCP name, bool resolves)
    {
    LsDefinitionP   nameRec = FindInRefOrRsc (&project, name);
    LsIdNode        idRec (id, name, nameRec, resolves);

    m_idTree.Add (&idRec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSystemMap::AddSystemIdEntry
(
Int32           id,
Utf8CP       name,
bool            resolves
)
    {
    LsDefinitionP   nameRec = GetSystemMapP (true)->FindSystemName (name);
    LsIdNode        idRec (id, name, nameRec, resolves);

    m_idTree.Add (&idRec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::AddNameEntry (LsDefinition* nameRec)
    {
    m_nameTree.Add ((NameNode*) &nameRec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsMap::AddNameEntry(Utf8CP name, Int32 styleId, DgnProjectR project, Json::Value& lsDefinition)
    {
    LsDefinition* nameRec = new LsDefinition (name, project, lsDefinition, styleId);
    AddNameEntry (nameRec);
    return  nameRec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsMap::RemoveIdEntry (Int32 id)
    {
    LsIdNode    node (0,0,0, true);

    if (SUCCESS != m_idTree.Remove (id, &node))
        return  ERROR;

    node.Clear();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsMap::RemoveNameEntry (Utf8CP name)
    {
    LsDefinition* node;

    if (SUCCESS != m_nameTree.Remove (name, (NameNode*) &node))
        return  ERROR;

    delete node;
    return  SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDgnProjectMapP  LsMap::GetMapPtr(DgnProjectR project, bool loadIfNotLoaded)
    {
    return  project.Styles().LineStyles().GetMapP (loadIfNotLoaded);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
LsDgnProjectMapP  LsMap::GetProjectMap (DgnProjectR project, bool loadIfNotLoaded)
    {
    return GetMapPtr(project, loadIfNotLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static LsSystemMap* getLsSystemMap (bool createIfNecessary)
    {
    LsSystemMap* lsSystemMap = dynamic_cast<LsSystemMap*> (T_HOST.GetHostObject(s_systemLsFileInfoKey));

    if (NULL == lsSystemMap && createIfNecessary)
        T_HOST.SetHostObject (s_systemLsFileInfoKey, lsSystemMap = new LsSystemMap());

    return lsSystemMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
LsSystemMap*     LsSystemMap::GetSystemMapPtr
(
)
    {
    return getLsSystemMap (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   clearComponentRef
(
NameNode const*     nameNode,
void*                 arg
)
    {
    (const_cast <LsDefinition*> (nameNode->GetValue()))->SetComponent (NULL);
    return  SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::DropAllComponentRefs ()
    {
#ifdef BEIJING_DGNSESSION_WIP
    DgnFileArray& dgnFiles = DgnFile::GetOpenedFileArray (session);

    size_t count = dgnFiles.size();
    for (size_t i=0; i<count; i++)
        {
        DgnProjectP    dgnFile = dgnFiles.at(i);
        if (NULL == dgnFile)
            continue;

        LsMap*     lsMap = LsMap::GetMapPtr (*dgnFile, false);
        if (NULL != lsMap)
            lsMap->ProcessNameMap (clearComponentRef, NULL);
        }
#endif

    Maps_T&      allMaps = *AllMaps ();
    for (MapsIter_T curr = allMaps.begin (); curr != allMaps.end (); curr++)
        {
        LsMap*     lsMap = *curr;
        
        lsMap->ProcessNameMap (clearComponentRef, NULL);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNode::LsIdNode
(
Int32           id,
Utf8CP       name,
LsDefinitionP   nameRec,
bool            resolves
)
    {
    m_id      = id;
    m_name    = NULL;
    m_nameRec = nameRec;
    m_resolves = resolves;

    SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsIdNode::SetName (Utf8CP name)
    {
    if (m_name)
        FREE_AND_CLEAR (m_name);

    if (name)
        m_name = BeStringUtilities::Strdup (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsIdNode::Clear ()
    {
    SetName (NULL);
    m_id      = 0;
    m_nameRec = NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   freeNameRec (NameNodeP node, void* arg)
    {
    delete node->GetValue();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    freeIdRec (LsIdNodeP node, void* arg)
    {
    (const_cast <LsIdNode*> (node))->Clear ();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::EmptyIdMap ()
    {
    // iterate tree, delete all entries
    m_idTree.Process (freeIdRec, NULL);
    m_idTree.Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::EmptyNameMap ()
    {
    // iterate tree, delete all entries
    m_nameTree.Process (freeNameRec, NULL);
    m_nameTree.Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::EmptyMaps()
    {
    EmptyNameMap ();
    EmptyIdMap ();
    m_isLoaded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsMap::LsMap ()
    {
    m_isLoaded = false;
    AllMaps ()->push_back (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsMap::~LsMap ()
    {
    EmptyMaps ();
    
    Maps_T*      allMaps = dynamic_cast<Maps_T*> (T_HOST.GetHostObject(s_allMapsKey));
    if (NULL == allMaps)
        return;

    for (MapsIter_T curr = allMaps->begin (); curr != allMaps->end (); curr++)
        {
        if (*curr == this)
            {
            allMaps->erase (curr);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LsMap::Maps_T* LsMap::AllMaps ()
    {
    Maps_T* all = dynamic_cast<Maps_T*> (T_HOST.GetHostObject(s_allMapsKey));

    if (NULL == all)
        T_HOST.SetHostObject (s_allMapsKey, all = new Maps_T);

    return all;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSystemMap::EmptySystemMap ()
    {
    LsSystemMap* systemMap = GetSystemMapPtr();

    if (NULL != systemMap)
        systemMap->EmptyMaps();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::EmptyAllMaps ()
    {
#ifdef BEIJING_DGNSESSION_WIP
    DgnFileArray& dgnFiles = DgnFile::GetOpenedFileArray (session);

    DgnProjectP  currFile;
    size_t numOpenFiles = dgnFiles.size();
    LsMap*      currMap;

    // first free all
    for (size_t i=0; i<numOpenFiles; i++)
        {
        if (NULL != (currFile = dgnFiles.at (i)) && (NULL != (currMap = GetMapPtr(*currFile))))
            currMap->EmptyMaps();
        }
#endif

    LsSystemMap::EmptySystemMap();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt insertIntoNameMap(LsMap* lsInfo, Utf8CP name, DgnStyleId id, Utf8String& data, DgnProjectR project)
    {
    Json::Value  jsonObj (Json::objectValue);
    if (!Json::Reader::Parse(data, jsonObj))
        return ERROR;

    lsInfo->AddNameEntry (name, (Int32)id.GetValue(), project, jsonObj);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectCR    LsDgnProjectMap::GetDgnProject () const
    {
    return m_dgnProject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectR     LsDgnProjectMap::GetDgnProjectR () const
    {
    return m_dgnProject;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsDgnProjectMap::Load ()
    {
    if (IsLoaded())
        return SUCCESS;

    TreeLoaded ();

    DgnStyles::Iterator lineStyles = m_dgnProject.Styles ().LineStyles().MakeIterator ();
    for (DgnStyles::Iterator::Entry iter = lineStyles.begin (); lineStyles.end () != iter; ++iter)
        {
        Utf8String  data ((Utf8CP)iter.GetData());
        insertIntoNameMap (this, iter.GetName(), iter.GetId(), data, this->m_dgnProject);
        AddIdEntry (iter.GetId().GetValue(), m_dgnProject, iter.GetName(), true);
        }

    // Update the dialog if any names were loaded.  Can't do this until the line style system is loaded
    // since it calls into the level system which updates the local file from dgn libs.
    T_HOST.GetLineStyleAdmin()._LoadedNameMap (m_dgnProject);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
LsDgnProjectMap::~LsDgnProjectMap ()
    {
    LineStyleCacheManager::CacheFree ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsDgnProjectMapPtr LsDgnProjectMap::Create (DgnProjectR project)
    {
    return new LsDgnProjectMap (project); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
LsSystemMapP     LsSystemMap::GetSystemMapP (bool requiresLoadedMap)
    {
    LsSystemMapP lsSystemMap = getLsSystemMap (true);

    //  ReadSystemMap relied on the ability to use the 0 RscFileHandle to find all 
    //  relevant resources, but DgnPlatform does not support 0 RscFileHandle.
    //  Therefore, this method no longer calls ReadSystemMap. 
    if (requiresLoadedMap && !lsSystemMap->IsLoaded())
        LineStyleManager::GetManager().Initialize ();

    return  lsSystemMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSystemMapR  LsSystemMap::GetSystemMapR ()
    {
    return *GetSystemMapP (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSystemMapCR   LsSystemMap::GetSystemMap ()
    {
    return GetSystemMapR ();
    }

/*----------------------------------------------------------------------------------*//**
* Look for the name in the file; if not there, look in the resource map.
* @bsimethod                                                    ChuckKirschman   09/02
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP     LsMap::FindInRefOrRsc (DgnProjectP project, CharCP name)
    {
    LsMap* lsInfo = NULL != project ? GetMapPtr (*project, true) : NULL;

    // if we found an LsMap, look for the name there
    LsDefinitionP nameRec = (NULL != lsInfo) ? lsInfo->Find (name) : NULL;

    // if we don't have a nameRec, look in system map
    return (NULL != nameRec) ? nameRec : LsSystemMap::GetSystemMapP (true)->FindSystemName (name);
    }

/*----------------------------------------------------------------------------------*//**
* Look for the ID in the file; if not there, look in the resource map.
* @bsimethod                                                    ChuckKirschman   09/02
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP     LsMap::FindInRefOrRsc (DgnProjectP project, Int32 styleId)
    {
    LsMap* lsInfo = NULL != project ? GetMapPtr (*project, true) : NULL;

    // if we found an LsMap, look for the name there
    LsDefinitionP nameRec = (NULL != lsInfo) ? lsInfo->Find (styleId) : NULL;

    return (NULL != nameRec) ? nameRec : LsSystemMap::GetSystemMapP (true)->Find (styleId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
                LsMapIterator::LsMapIterator (LsMapCP map, bool wantBegin) 
    {
    m_index = 0;
    m_node = NULL;
    m_auxData = 0;

    if (wantBegin)
        {
        LsMapP              nonConstMap = const_cast <LsMapP> (map);
        T_LsIdIterator      iter = nonConstMap->FirstId();

        m_index = iter.m_index;
        m_node = iter.m_currNode;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsMapIterator&       LsMapIterator::operator++() 
    {
    T_LsIdIterator          iter ((T_LsIdTreeNode const*)m_node, m_index);
    
    iter = iter.Next ();

    m_index = iter.m_index;
    m_node = iter.m_currNode;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsMapEntry const & LsMapIterator::operator* () const 
    {
    T_LsIdIterator          iter ((T_LsIdTreeNode const*)m_node, m_index);

    LsIdNodeP               idNode = iter.Entry ();
    
    return *static_cast <LsMapEntry const *> (idNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        LsMapIterator::operator==(LsMapIterator const& rhs) const 
    {
    return m_index == rhs.m_index && m_node == rhs.m_node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        LsMapIterator::operator!=(LsMapIterator const& rhs) const 
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsMapIterator LsMap::begin () const 
    {
    return LsMapIterator (this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsMapIterator LsMap::end   () const 
    {
    return LsMapIterator (this, false);
    }

/*----------------------------------------------------------------------------------*//**
* Find a style by id. If id > 0, first look in system map. If not there, look in modelref
* @bsimethod                                                    ChuckKirschman   09/02
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP     LsMap::FindInRef (DgnProjectP project, Int32 styleId)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    if (styleId > 0)
        {
        LsDefinitionP  nameRec = LsSystemMap::GetSystemMapP (true)->Find (styleId);
        if (NULL != nameRec)
            return  nameRec;
        }

    LsMap*  lsMap = LsMap::GetMapPtr (dgnFile, true);
    if (NULL == lsMap)
        {
        BeAssert (false);     // Should never occur - but may if modelRef not passed through during cache load (DWG).
        return NULL;
        }

    return  lsMap->Find (styleId);
#endif
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* look up a linestyle in a LsMap by name. Return name record.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsMap::Find (Utf8CP name) const
    {
    NameNode*   nameNode = m_nameTree.Get (name);
    return  (NULL != nameNode) ? nameNode->GetValue() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* look up a linestyle in a LsMap by id. Return LsIdNodeP
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNodeP       LsMap::FindId (Int32 styleId) const
    {
    LsIdNodeP   retval = m_idTree.Get (styleId);
    
    if (NULL != retval && retval->Resolves ())
        return retval;
        
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* look up a linestyle in a LsMap by id. Return name record.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsMap::Find (Int32 styleId) const
    {
    LsIdNodeP   idNode = FindId (styleId);
    return  (NULL != idNode) ? idNode->GetValue() : NULL;
    }

WString         LsMap::GetFileName () const 
    { 
    LsMap* ncThis = const_cast<LsMap*>(this);
    return ncThis->_GetFileName (); 
    }

LsLocationType   LsMap::GetType ()  const 
    { 
    LsMap* ncThis = const_cast<LsMap*>(this);
    return ncThis->_GetType (); 
    }

LsDefinitionP   LsMap::GetLineStyleP (Utf8CP name)  const { return Find (name); }
LsDefinitionP   LsMap::GetLineStyleP (Int32 styleId)   const { return Find (styleId); }
LsDefinitionCP  LsMap::GetLineStyleCP (Utf8CP name) const { return GetLineStyleP (name); }
LsDefinitionCP  LsMap::GetLineStyleCP (Int32 styleId)  const { return GetLineStyleP (styleId); }

static ILineStyleDgnLibIterator* s_iter;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            BentleyApi::LineStyle_setDgnLibIterator
(
ILineStyleDgnLibIterator* iter
)
    {
    s_iter = iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsSystemMap::FindSystemName (Utf8CP  name) const
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    NameNode*       nameNode = m_nameTree.Get (name);
    LsDefinitionP     linFileNameRec = NULL;

    if (NULL != nameNode)
        {
        // Defer lin file nodes until after DGNLIBs.
        if (LsLocationType::LinFile == nameNode->GetValue()->GetLocation()->GetSourceType())
            linFileNameRec = nameNode->GetValue();
        else
            return nameNode->GetValue();
        }

    // Look in DGNLIBs
    LsDefinitionP dgnLibNameRec = findInDgnLib (name);

    return  (NULL != dgnLibNameRec) ? dgnLibNameRec : linFileNameRec;
#endif
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Same as above, but it won't return the name record from the local file, only from RSC,
* LIN, and DGNLIB.
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsSystemMap::FindNameInLibs (Utf8CP  name) const
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    NameNode*       nameNode = m_nameTree.Get (name);
    LsDefinitionP     linFileNameRec = NULL;

    if (NULL != nameNode)
        {
        // Defer lin file nodes until after DGNLIBs.
        if (LsLocationType::LinFile == nameNode->GetValue()->GetLocation()->GetSourceType())
            linFileNameRec = nameNode->GetValue();
        // If it's in a resource, return that
        else if (LsLocationType::ResourceFile == nameNode->GetValue()->GetLocation()->GetSourceType())
            return nameNode->GetValue();
        }

    // Look in DGNLIBs
    LsDefinitionP dgnLibNameRec = findInDgnLib (name);

    return  (NULL != dgnLibNameRec) ? dgnLibNameRec : linFileNameRec;
#endif
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Structure for process name maps in dgnlib list.  The thing this class does is to
* make sure that only element items are passed through.
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            processDgnLibMaps (LsMap::PFNameMapProcessFunc callback, void*arg)
        {
#ifdef DGNV10FORMAT_CHANGES_WIP
        if (NULL == s_iter)
            return;
            
        size_t  iterHandle = s_iter->_Begin ();
        
        for (DgnProjectP  dgnFile = s_iter->_MoveNext (iterHandle); NULL != dgnFile; dgnFile = s_iter->_MoveNext (iterHandle))
            {
            LsMapP     lsMap = LsMap::GetMapPtr (*dgnFile, true);
            lsMap->ProcessNameMap (callback, arg);
            }

        s_iter->_End (iterHandle);
#endif

        return;
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::ProcessNameMap (PFNameMapProcessFunc processFunc, void* arg)
    {
    m_nameTree.Process (processFunc, arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSystemMap::ProcessSystemNameMaps (PFNameMapProcessFunc processFunc, void* arg, bool includeDgnLibs)
    {
    ProcessNameMap (processFunc, arg);
    if (includeDgnLibs)
        processDgnLibMaps (processFunc, arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNodeP       LsMap::SearchIdsForName
(
Utf8CP  name
) const
    {
    LsMapP      ncMap = const_cast <LsMapP> (this);  //  key tree doesn't have a const iterator
    for (T_LsIdIterator curr = ncMap->FirstId(); curr.Entry(); curr = curr.Next())
        {
        Utf8CP currName =curr.Entry()->GetName();
        if (currName && !BeStringUtilities::Stricmp (currName, name))
            return  curr.Entry();
        }

    return  NULL;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      BentleyApi::lineStyle_nameInsert
(
Utf8CP       styleName,     /* => New style name for name map      */
UInt32          rscFile,        /* => Resource file handle             */
UInt32          rscType,        /* => UDLS resource type               */
ElementId       rscID,          /* => UDLS resource ID                 */
UInt32          nameAttributes, /* => Name attributes                  */
long            id,             /* => pass 0 for generated seed id     */
UInt32          option,         /* => Function options                 */
DgnModelP    modelRef
)
    {
    return ERROR;
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    LsRscRecord   rscNameRec;
    DgnProjectR    dgnFile = *modelRef->GetDgnProjectP ();
    
    /*-------------------------------------------------------------------
    Create the name map record.
    -------------------------------------------------------------------*/
    rscNameRec.m_rscType  = rscType;
    rscNameRec.m_id       = rscID;
    rscNameRec.m_flags    = nameAttributes;

    bool    isResource = (false == mdlLineStyle_typeIsElement(rscType));

    LsDefinition const *nameRec = NULL;
    if (isResource)
         nameRec = LsSystemMap::GetSystemMapP (true)->AddNameEntry (styleName, rscFile, &rscNameRec);
    else
        {
        nameRec = LsMap::GetMapPtr (dgnFile, true)->AddNameEntry (styleName, &dgnFile, &rscNameRec);
        }

    // Find a unique ID add to id tree
    Int32   uniqueID = getUniqueLsId ((id != 0 ? id : nameRec->GetLocation()->GetSeedID ()), dgnFile);
    LsDgnFileMap* lsInfo = LsMap::GetMapPtr (dgnFile, true);
    lsInfo->AddIdEntry (uniqueID, dgnFile, styleName, true);

    T_HOST.GetLineStyleAdmin()._AddedNameMapEntry (&dgnFile, uniqueID);

    return  SUCCESS;
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsMap::RenameStyle
(
Utf8CP       oldName,
Utf8CP       newName,
LsLocationType    type        // LsLocationType::Unknown for any
)
    {
    for (T_LsIdIterator curr = FirstId(); curr.Entry(); curr = curr.Next())
        {
        LsLocationType    currType = curr.Entry()->GetValue()->GetLocation()->GetSourceType();

        if (type != LsLocationType::Unknown && type != currType)
            continue;

        Utf8CP currName =curr.Entry()->GetName();

        if (currName && !BeStringUtilities::Stricmp (currName, oldName))
            {
            (const_cast <LsIdNode*>(curr.Entry()))->SetName (newName);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleCacheManager::FreeDgnFileMaps ()
    {
    // Better also clear the cache at this time in case there are any stale pointers.
    // The case we ran into (TR #175621) was a Ref Reload was freeing the maps, but
    //   when the maps were repopulated, the styles were already in the cache.
    LineStyleCacheManager::CacheFree ();
    }
