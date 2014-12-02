/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MultilineStyle.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define MLINE_STYLE_VERSION         4
#define     MS_MLINESTYLE_TABLE_LEVEL                           6

#define SHIELD_BIT_SEGMENT          16      /* Length of a segment of shield bits.  Each section (cap, profile, etc) is this long */

static DgnHost::Key s_inUpdateKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbology::MultilineSymbology ()
    {
    Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbology::MultilineSymbology (MultilineSymbologyCR symb)
    {
    Copy (symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbology::MultilineSymbology (MlineSymbology const * symb, LineStyleParamsCP params)
    {
    Clear ();
    if (NULL != symb)
        SetSymbology (*symb);
    if (NULL != params)
        SetLinestyleParams (*params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineSymbology::SetCapLine (bool val)
    {
    m_profile.symb.capLine = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineSymbology::UseCapLine () const
    {
    return m_profile.symb.capLine;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineSymbology::SetCapInnerArc (bool val)
    {
    m_profile.symb.capInArc = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineSymbology::UseCapInnerArc () const
    {
    return m_profile.symb.capInArc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineSymbology::SetCapOuterArc (bool val)
    {
    m_profile.symb.capOutArc = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineSymbology::UseCapOuterArc () const
    {
    return m_profile.symb.capOutArc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineSymbology::SetCapColorFromSegment (bool val)
    {
    m_profile.symb.capColorFromSeg = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineSymbology::UseCapColorFromSegment () const
    {
    return m_profile.symb.capColorFromSeg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfile::MultilineProfile ()
    {
    Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfile::MultilineProfile (MultilineProfileCR msprofile)
    {
    Copy (msprofile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfile::MultilineProfile (MlineProfile const * profile, LineStyleParams const * params)
    {
    Clear ();
    if (NULL != profile)
        SetProfile (*profile);
    if (NULL != params)
        SetLinestyleParams (*params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfilePtr  MultilineProfile::Create (MlineProfile const * profile, LineStyleParams const * params)
    {
    return new MultilineProfile (profile, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfilePtr  MultilineProfile::Create ()
    {
    return new MultilineProfile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineSymbology::Clear ()
    {
    memset (&m_profile, 0, sizeof(m_profile));
    memset (&m_lineStyleInfo, 0, sizeof(m_lineStyleInfo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineSymbology::Copy (MultilineSymbologyCR mstyleProfile)
    {
    m_profile = mstyleProfile.m_profile;  // Direct copy so it works for profile or symb.
    SetLinestyleParams (mstyleProfile.m_lineStyleInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineProfile*       MultilineProfile::GetProfile ()
    {
    return &m_profile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineProfile const &       MultilineProfile::GetProfileCR () const
    {
    return m_profile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineProfile::SetProfile (MlineProfile const & profile) 
    {
    m_profile = profile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleParamsP    MultilineSymbology::GetLinestyleParams () 
    {
    return &m_lineStyleInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleParamsCR    MultilineSymbology::GetLinestyleParamsCR () const 
    {
    return m_lineStyleInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetLinestyleParams (LineStyleParams const & params) 
    {
    m_lineStyleInfo = params;
    // Set customStyle for backward compatability
    if (0 != params.modifiers)
        m_profile.symb.customStyle = 1;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetUseLinestyle (bool val)
    {
    m_profile.symb.useStyle = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::UsesLinestyle () const
    {
    return m_profile.symb.useStyle;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetLinestyle (Int32 val)
    {
    m_profile.symb.style = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
Int32                MultilineSymbology::GetLinestyle () const
    {
    return m_profile.symb.style;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetCustomLinestyleBit (bool val)
    {
    m_profile.symb.customStyle = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::GetCustomLinestyleBit () const
    {
    return m_profile.symb.customStyle;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::UsesLevel () const
    {
    return m_profile.symb.level != 0;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetLevel (LevelId val)
    {
    m_profile.symb.level = val.GetValue();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId               MultilineSymbology::GetLevel () const
    {
    return LevelId(m_profile.symb.level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetUseColor (bool val)
    {
    m_profile.symb.useColor = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::UsesColor () const
    {
    return m_profile.symb.useColor;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetColor (UInt32 val)
    {
    m_profile.symb.color = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32                MultilineSymbology::GetColor () const
    {
    return m_profile.symb.color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetUseWeight (bool val)
    {
    m_profile.symb.useWeight = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::UsesWeight () const
    {
    return m_profile.symb.useWeight;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetWeight (UInt32 val)
    {
    m_profile.symb.weight = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32                MultilineSymbology::GetWeight () const
    {
    return m_profile.symb.weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetUseClass(bool val)
    {
    m_profile.symb.useClass = val;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MultilineSymbology::UsesClass () const
    {
    return m_profile.symb.useClass;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineSymbology::SetClass (DgnElementClass val)
    {
    m_profile.symb.conClass = DgnElementClass::Construction == static_cast<DgnElementClass>(val);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementClass     MultilineSymbology::GetClass () const
    {
    return m_profile.symb.conClass ? DgnElementClass::Construction : DgnElementClass::Primary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineProfile::ScaleDistance (double scale)
    {
    m_profile.dist *= scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double                MultilineProfile::GetDistance () const
    {
    return m_profile.dist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MultilineProfile::SetDistance (double dist)
    {
    m_profile.dist = dist;
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineStyleElementIterator::MlineStyleElementIterator (DgnProjectR file)
    :
    m_dgnFile (file)
    {
    ToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MlineStyleElementIterator::MlineStyleElementIterator (ElementHandleCR tableElm)
    :
    m_elemIter (tableElm, ExposeChildrenReason::Count),
    m_dgnFile (*tableElm.GetDgnProject())
    {
    ToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MlineStyleElementIterator::ToNext ()
    {
    if (m_elemIter.IsValid())
        {
        m_elemIter = m_elemIter.ToNext();
        }
    else
        {
        ElementHandle  tableElm = MultilineStyle::GetTableElement (m_dgnFile);

        if ( ! tableElm.IsValid())
            return;

        m_elemIter = ChildElemIter (tableElm, ExposeChildrenReason::Count);
        }

    while (m_elemIter.IsValid() && MlineStyleEntryHandler::IsSettings (m_elemIter))
        m_elemIter = m_elemIter.ToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatform::ElementHandle const& MlineStyleElementIterator::GetCurrent () const
    {
    return m_elemIter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MlineStyleElementIterator::IsValid () const
    {
    return m_elemIter.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::const_iterator::const_iterator (DgnProjectP file)
    {
    if (NULL == file)
        return;
    ElementHandle hdr = MultilineStyle::GetTableElement (*file);
    if (!hdr.IsValid())
        return;

    m_elemIter = ChildElemIter(hdr, ExposeChildrenReason::Count);

    while (m_elemIter.IsValid() && MlineStyleEntryHandler::IsSettings (m_elemIter))
        m_elemIter = m_elemIter.ToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::const_iterator::const_iterator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineStyleCollection::const_iterator::IsValid() const
    {
    return m_elemIter.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::const_iterator& MultilineStyleCollection::const_iterator::operator ++ ()
    {
    m_current = NULL;

    if (!m_elemIter.IsValid())
        return *this;

    m_elemIter = m_elemIter.ToNext();

    while (m_elemIter.IsValid() && MlineStyleEntryHandler::IsSettings (m_elemIter))
        m_elemIter = m_elemIter.ToNext();

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineStyleCollection::const_iterator::operator != (MultilineStyleCollection::const_iterator const& rhs) const
    {
    bool lhsValid = m_elemIter.IsValid();
    bool rhsValid = rhs.m_elemIter.IsValid();
    if (!lhsValid && !rhsValid)
        return false;

    if (!lhsValid || !rhsValid)
        return true;

    if (m_elemIter.GetDgnProject() != rhs.m_elemIter.GetDgnProject())
        return true;
    
    return m_elemIter.GetElementCP() != rhs.m_elemIter.GetElementCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleP const& MultilineStyleCollection::const_iterator::operator * () const
    {
    if (!m_current.IsValid())
        m_current = MultilineStyle::CreateFromElement (m_elemIter);
    return m_current.GetCR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::MultilineStyleCollection (DgnProjectR file)
 :m_dgnFile(&file)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::const_iterator MultilineStyleCollection::begin () const 
    {
    return MultilineStyleCollection::const_iterator (m_dgnFile.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyleCollection::const_iterator MultilineStyleCollection::end () const 
    {
    return MultilineStyleCollection::const_iterator (NULL);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr  MultilineStyle::CreateFromName (WCharCP name, DgnProjectR file)
    {
    MultilineStyle *newStyle = new MultilineStyle();
    newStyle->Clear ();
    
    if (NULL == name)
        BeAssert (false);
    else
        newStyle->m_name = name;
    
    newStyle->m_file = &file;
    newStyle->m_initialized = true;
    return newStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr  MultilineStyle::CreateFromElement (ElementHandleCR styleElement)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    MultilineStyle *newStyle = new MultilineStyle();
    if (SUCCESS != newStyle->FromElement (styleElement))
        {
        delete newStyle;
        return NULL;
        }
    return newStyle;
#endif
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStyle::Clear ()
    {
    m_initialized = false;
    m_file = NULL;
    m_elemID.Invalidate();
    
    m_name.clear();

    m_orgAngle      = 90.0;
    m_endAngle      = 90.0;
    m_numProfiles   = 0;
    m_fillColor     = 0;
    m_isFilled      = false;

    for (UInt32 iProfile=0; iProfile< m_profiles.size(); iProfile++)
        m_profiles[iProfile]->Clear ();
    
    m_orgCap.Clear ();
    m_orgCap.SetCapLine (true);
    m_endCap.Clear ();
    m_endCap.SetCapLine (true);
    m_midCap.Clear ();

    m_isDefault = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 09/10
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStyle::MultilineStyle ()
    {
    Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString   MultilineStyle::GetName () const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId    MultilineStyle::GetID () const
    {
    return m_elemID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineStyle::StyleExistsInFile (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return false; }
    
    ElementHandle elem = GetStyleElementByName (name, file);
    return elem.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectR  MultilineStyle::GetFile () const
    {
    return *m_file;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus      MultilineStyle::SetName (WCharCP name)
    {
    if (NULL == name)
        { BeAssert (false); return ERROR; }
    
    if (wcslen (name) > MAX_LINKAGE_STRING_LENGTH-1)
        return ERROR;

    m_name = name;
    m_elemID.Invalidate();   // Have to clear the id if we change names
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/10
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePropMaskPtr         MultilineStyle::Compare (MultilineStyleCR style) const
    {
    MultilineStylePropMaskPtr resultPropMask = MultilineStylePropMask::Create ();
    resultPropMask->CompareStyles (*this, style);
    return resultPropMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool  MultilineStyle::IsSettings () const
    {
    return m_isDefault;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void  MultilineStyle::SetAsSettings (bool value)
    {
    m_isDefault = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr      MultilineStyle::Create (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }
    
    return MultilineStyle::CreateFromName (name, file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      MultilineStyle::CopyValues (MultilineStyleCR other)
    {
    WString name = m_name;
    Clear ();

    m_file = other.m_file;
    m_elemID = other.m_elemID;
    m_name = name;  // This method is called after user has already created a style with this name; hang onto name and put it back in.
    m_initialized = other.m_initialized;

    m_orgAngle = other.m_orgAngle;
    m_endAngle = other.m_endAngle;
    m_numProfiles = other.m_numProfiles;
    m_fillColor = other.m_fillColor;
    m_isFilled = other.m_isFilled;

    for (UInt32 iProfile=0; iProfile<m_numProfiles; iProfile++)
        {
        MultilineProfileP profile = new MultilineProfile (*other.m_profiles[iProfile]);
        m_profiles.insert (m_profiles.begin()+iProfile, profile);
        }
    
    m_orgCap.Copy (other.m_orgCap);
    m_endCap.Copy (other.m_endCap);
    m_midCap.Copy (other.m_midCap);
    m_isDefault = other.m_isDefault;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr      MultilineStyle::GetByID (ElementId id, DgnProjectR file)
    {
    ElementHandle elem = GetStyleElementById (id, file);
    return MultilineStyle::CreateFromElement (elem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr      MultilineStyle::GetByName (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }
    
    ElementHandle elem = GetStyleElementByName (name, file);
    return MultilineStyle::CreateFromElement (elem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/10
+---------------+---------------+---------------+---------------+---------------+------*/
double MultilineStyle::GetUorScaleToModel (DgnModelP destCache) const
    {
#if defined (REMOVE_DEAD_CODE)
    if (NULL == destCache)
        return 1.0;
    
    ModelId     defaultModelId  = GetFile().GetDefaultModelId();
    DgnModelPtr defaultCache    = GetFile().LoadModelById(defaultModelId);
    double      uorScale        = 1.0;
    
    if (SUCCESS != modelInfo_getUorScaleBetweenModels (&uorScale, defaultCache.get(), destCache))
        return 1.0;
    
    return uorScale;
#endif
    return 1.0;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      MultilineStyle::GetStyleElementByName (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return ElementHandle (); }
    
#ifdef DGN_IMPORTER_REORG_WIP
    for (MlineStyleElementIterator iter (file); iter.IsValid (); iter.ToNext ())
        {
        ElementHandleCR elem = iter.GetCurrent ();
        DgnElementCP elemCP   = elem.GetElementCP ();
        WString     elemName = MlineStyleEntryHandler::GetName (*elemCP);

        if (0 == BeStringUtilities::Wcsicmp (name, elemName.c_str()))
            return elem;
        }
#endif

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* Used when extracting a style for DWG when nothing exists in the file.
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      MultilineStyle::GetFirstNamedStyleElement (DgnProjectR file)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    for (MlineStyleElementIterator iter (file); iter.IsValid (); iter.ToNext ())
        {
        ElementHandleCR elem = iter.GetCurrent ();
        DgnElementCP elemCP   = elem.GetElementCP ();
        WString     elemName = MlineStyleEntryHandler::GetName (*elemCP);

        if (elemName.length() > 0)
            return elem;
        }
#endif

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      MultilineStyle::GetStyleElementById (ElementId id, DgnProjectR file)
    {
    if (!id.IsValid())
        return ElementHandle();
    
#ifdef DGN_IMPORTER_REORG_WIP
    for (MlineStyleElementIterator iter (file); iter.IsValid (); iter.ToNext ())
        {
        ElementHandleCR elem = iter.GetCurrent ();
        if (id == elem.GetElementId())
            return elem;
        }
#endif

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      MultilineStyle::GetSettingsElement (DgnProjectR file)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    ElementHandle  tableElm = MultilineStyle::GetTableElement (file);

    if (!tableElm.IsValid())
        return ElementHandle();

    DgnPlatform::ChildElemIter elemIter = ChildElemIter (tableElm, ExposeChildrenReason::Count);

    if (!elemIter.IsValid())
        return ElementHandle();

    while (elemIter.IsValid())
        {
        if (MlineStyleEntryHandler::IsSettings (elemIter))
            return elemIter;
        elemIter = elemIter.ToNext();
        }
#endif

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString      MultilineStyle::GetActiveStyleName (ElementHandleCR  tableElm)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    WChar activeName[MAX_LINKAGE_STRING_LENGTH];
    if (SUCCESS == MlineStyleTableHandler::ExtractName (activeName, _countof(activeName), *tableElm.GetElementCP()))
        return activeName;
#endif

    return WString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      MultilineStyle::SetActiveStyleName (EditElementHandleR tableElm, WCharCP activeName)
    {
    if (NULL == activeName)
        { BeAssert (false); return; }
    
    DgnV8ElementBlank   tmpElm;

    tableElm.GetElementCP ()->CopyTo (tmpElm);
    if (SUCCESS == LinkageUtil::SetStringLinkage (&tmpElm, STRING_LINKAGE_KEY_Name, activeName))
        tableElm.ReplaceElement (&tmpElm);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePtr      MultilineStyle::GetSettings (DgnProjectR file)
    {
    ElementHandle  activeStyleElem = GetSettingsElement (file);
    return MultilineStyle::CreateFromElement (activeStyleElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus      MultilineStyle::ReplaceSettings (MultilineStyleR mlineStyle, DgnProjectR file)
    {
    // Find table element
    EditElementHandle   tableElm;
    if (SUCCESS != mlineStyle.GetTableElementForWrite (tableElm, mlineStyle.GetFile()) || !tableElm.IsValid())
        return ERROR;
    
    ElementRefP         oldRef = tableElm.GetElementRef ();

    bool found = false;
    // Replace the entry element
    for (ChildEditElemIter iter (tableElm, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (!MlineStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();

        // Create the new element
        EditElementHandle  newElm;
        mlineStyle.ToElement (newElm);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();
        // Mark it as settings
        newEdP->el.ToMlineStyleEntry().entryFlags.isDefault = true;
        
        // Set the newStyle to use the ID of the old element
        newEdP->el.ehdr.uniqueId = elemCP->ehdr.uniqueId;

        // Replace the old element with the new one
        iter.ReplaceElementDescr (newEdP);
        found = true;
        break;
        }

    if (!found)  // Add to table
        {
        MSElementDescrP tableEdP = tableElm.ExtractElementDescr ();

        EditElementHandle newElm;
        mlineStyle.ToElement (newElm);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();
        newEdP->el.ehdr.uniqueId = 0;
        // Mark it as settings
        newEdP->el.ToMlineStyleEntry().entryFlags.isDefault = true;
        tableEdP->AppendDescr (newEdP);
        
        tableElm.SetElementDescr (tableEdP, true, false);
        }

    SetActiveStyleName (tableElm, (mlineStyle.GetName().c_str()));

    // Rewrite the whole table
    return (SUCCESS == tableElm.ReplaceInModel(oldRef) ? BSISUCCESS : BSIERROR);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineStyle::IsMlineStyleElement (DgnElementCP pCandidate)
    {
    return  (pCandidate &&
             pCandidate->GetLegacyType()  == TABLE_ENTRY_ELM &&
             pCandidate->GetLevelValue() == MS_MLINESTYLE_TABLE_LEVEL);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStyle::FromElement (ElementHandleCR inStyleElement)
    {
    Clear();

    if (!inStyleElement.IsValid())
        return BSIERROR;

    EditElementHandle styleElement (inStyleElement, true);

    DgnElementCP elemCP = styleElement.GetElementCP();

    if (!IsMlineStyleElement (elemCP))
        return BSIERROR;

    m_file      = styleElement.GetDgnProject();
    m_elemID    = elemCP->GetID ();

    // Coming from DWG or wherever we want the suspenders to upgrade this.
    if (elemCP->mlineStyleEntry.version < MLINE_STYLE_VERSION)
        {
        BeAssert (false); // This should happen in upgrade hook.
        }

    m_name = MlineStyleEntryHandler::GetName (*elemCP);

    m_isDefault = elemCP->mlineStyleEntry.entryFlags.isDefault;

    m_orgAngle = elemCP->mlineStyleEntry.orgAngle;
    m_endAngle = elemCP->mlineStyleEntry.endAngle;
    m_numProfiles = elemCP->mlineStyleEntry.nLines;
    m_fillColor = elemCP->mlineStyleEntry.fillColor;
    m_isFilled = elemCP->mlineStyleEntry.flags.filled;
    m_orgCap.SetSymbology (elemCP->mlineStyleEntry.orgCap);
    m_endCap.SetSymbology (elemCP->mlineStyleEntry.endCap);
    m_midCap.SetSymbology (elemCP->mlineStyleEntry.midCap);

    for (UInt32 iProfile=0; iProfile<m_numProfiles; iProfile++)
        {
        MultilineProfileP profile = new MultilineProfile();
        profile->SetProfile (elemCP->mlineStyleEntry.profile[iProfile]);
        m_profiles.insert (m_profiles.begin()+iProfile, profile);
        }

    for (ConstElementLinkageIterator li = styleElement.BeginElementLinkages(); li.IsValid() && li != styleElement.EndElementLinkages(); ++li)
        {
        if (!li->user)
            continue;

        if (STYLELINK_ID != li->primaryID)
            continue;

        UInt16* data = (UInt16 *) li.GetData ();
        LineStyleParams      params;

        LineStyleLinkageUtil::ExtractRawLinkage (&params, (StyleLink *)data, elemCP->Is3d());
        
        if (params.modifiers == 0 || params.lineMask == 0)
            continue;

        for (UShort which=0; which < MULTILINE_MAX; which++)
            {
            if (0 == (((UInt32)(1)<<which) & params.lineMask))
                continue;

            if (params.mlineFlags & MLSFLAG_CAP)
                {
                switch  (which)
                    {
                    case 0:
                        m_orgCap.SetLinestyleParams (params);
                        break;
                    case 1:
                        m_endCap.SetLinestyleParams (params);
                        break;
                    case 2:
                        m_midCap.SetLinestyleParams (params);
                        break;
                    }
                }
            else
                {
                if (which < m_profiles.size()) // We have some garbage linkages out there.
                    m_profiles[which]->SetLinestyleParams (params);
                }
            }
        }

    m_initialized = true;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineStyle::ToElement (EditElementHandleR out) const
    {
    // Set up the element to fill.  Easier to create my own than worry about linkage sizes later
    int elmsize = offsetof (MlineStyleEntryElm, profile) +
                  m_numProfiles*sizeof (MlineProfile);

    DgnElement       mlineStyleElm;
    memset (&mlineStyleElm, 0, sizeof(MlineStyleEntryElm) + 16*sizeof(MlineProfile));

    // Set up table defaults
    MSElementDescrP tempEdP = NULL;
    DgnTableUtilities::CreateEntry (&tempEdP, MS_MLINESTYLE_TABLE_LEVEL, elmsize, 0);
    tempEdP->el.CopyTo (mlineStyleElm);
    RELEASE_AND_CLEAR (tempEdP);

    /* Convert name to linkage */
    WChar         nameBuf[MAX_LINKAGE_STRING_LENGTH];

    wcsncpy (nameBuf, m_name.c_str(), MAX_LINKAGE_STRING_LENGTH);
    nameBuf[MAX_LINKAGE_STRING_LENGTH-1] = 0;
    strutil_wstrpwspc (nameBuf);  

    if (wcslen (nameBuf))
        LinkageUtil::SetStringLinkage (&mlineStyleElm, STRING_LINKAGE_KEY_Name, nameBuf);
    else
        LinkageUtil::DeleteStringLinkage (&mlineStyleElm, STRING_LINKAGE_KEY_Name, 0);

    mlineStyleElm.ToMlineStyleEntry().version = MLINE_STYLE_VERSION;
    mlineStyleElm.ToMlineStyleEntry().entryFlags.isDefault = 0;  // No one is default normally...

    mlineStyleElm.ToMlineStyleEntry().orgAngle    = m_orgAngle;
    mlineStyleElm.ToMlineStyleEntry().endAngle    = m_endAngle;
    mlineStyleElm.ToMlineStyleEntry().nLines      = static_cast<byte>(m_numProfiles);
    mlineStyleElm.ToMlineStyleEntry().fillColor   = m_fillColor;
    mlineStyleElm.ToMlineStyleEntry().flags.filled= m_isFilled;
    mlineStyleElm.ToMlineStyleEntry().orgCap      = m_orgCap.GetSymbologyCR ();
    mlineStyleElm.ToMlineStyleEntry().endCap      = m_endCap.GetSymbologyCR ();
    mlineStyleElm.ToMlineStyleEntry().midCap      = m_midCap.GetSymbologyCR ();

    for (UInt32 iProfile=0; iProfile<m_numProfiles; iProfile++)
        {
        mlineStyleElm.ToMlineStyleEntry().profile[iProfile] = m_profiles[iProfile]->GetProfileCR();
        }

    /* Clear all line style linkages */
    LineStyleLinkageUtil::ClearElementStyle (&mlineStyleElm, true, LS_ALL_LINES, LS_ALL_MLINE_TYPES);

    for (UInt32 iProfile=0; iProfile<m_numProfiles; iProfile++)
        {
        if (m_profiles[iProfile]->UsesLinestyle())
            MlineStyleEntryHandler::AddLinestyleLinkage (mlineStyleElm, m_profiles[iProfile]->GetLinestyleParamsCR());
        }

    /* Also add cap linkages */
    if (m_orgCap.UsesLinestyle())
        MlineStyleEntryHandler::AddLinestyleLinkage (mlineStyleElm, m_orgCap.GetLinestyleParamsCR());
    if (m_endCap.UsesLinestyle())
        MlineStyleEntryHandler::AddLinestyleLinkage (mlineStyleElm, m_endCap.GetLinestyleParamsCR());
    if (m_midCap.UsesLinestyle())
        MlineStyleEntryHandler::AddLinestyleLinkage (mlineStyleElm, m_midCap.GetLinestyleParamsCR());

    MSElementDescrP tDscr = MSElementDescr::Allocate (mlineStyleElm, &GetFile().GetDictionaryModel());
    out.SetElementDescr (tDscr, true, false, NULL);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStyle::Scale (const double scale)
    {
    for (UInt32 iProfile=0; iProfile<m_numProfiles; iProfile++)
        m_profiles[iProfile]->ScaleDistance (scale);
    }

/*---------------------------------------------------------------------------------**//**
* Upgrade the style element to the current version
*
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStyle::UpgradeElement (EditElementHandleR styleElement, bool& wasChanged)
    {
    wasChanged = true;

    DgnElementCP elemCP = styleElement.GetElementCP();
    if (elemCP->ToMlineStyleEntryElm().version == MLINE_STYLE_VERSION)
        {
        wasChanged = false;
        return BSISUCCESS;
        }

    MSElementDescrP edP = styleElement.GetElementDescrP();

    switch  (elemCP->ToMlineStyleEntryElm().version)
        {
        /* 0 -> 1 The version 0 elements had some extra linkages on them due to V7 conversion that should be removed. */
        case 0:
            for (ElementLinkageIterator li = styleElement.BeginElementLinkages(); li != styleElement.EndElementLinkages(); /* incremented below */)
                {
                if (!li->user)
                    styleElement.RemoveElementLinkage (li);
                else
                    ++li;    
                }
            /* FALLTHROUGH - keep on upgrading */

        /* 1 -> 2 pulls out the flags that belong in the TCB (should never have been in the style). */
        case 1:
            edP = styleElement.GetElementDescrP();
            if (edP->Element().ToMlineStyleEntryElm().entryFlags.isDefault)
                {
                /* I doubt there are any version 1 mline styles out there, and certainly these can be reset.  Turning this off now.
                if (0 == tcb->mlineFlags.compatible)
                    tcb->mlineFlags.compatible      = edP->el.ToMlineStyleEntry().flags.deprecated_compatible;
                if (0 == tcb->mlineFlags.offsetMode)
                    tcb->mlineFlags.offsetMode      = edP->el.ToMlineStyleEntry().flags.deprecated_offsetMode;
                if (0 == tcb->mlineFlags.scaleOffsets)
                    tcb->mlineFlags.scaleOffsets    = edP->el.ToMlineStyleEntry().flags.deprecated_scaleOffsets;
                if (0 == tcb->mlineFlags.mirrorOffsets)
                    tcb->mlineFlags.mirrorOffsets   = edP->el.ToMlineStyleEntry().flags.deprecated_mirrorOffsets;
                */
                }

            edP->ElementR().ToMlineStyleEntryElmR().flags.deprecated_compatible     = 0;
            edP->ElementR().ToMlineStyleEntryElmR().flags.deprecated_offsetMode     = 0;
            edP->ElementR().ToMlineStyleEntryElmR().flags.deprecated_scaleOffsets   = 0;
            edP->ElementR().ToMlineStyleEntryElmR().flags.deprecated_mirrorOffsets  = 0;
            /* FALLTHROUGH - keep on upgrading */

        /* 2 -> 3 resets the name of the default style to be blank.  This is to differentiate it from named styles.
           "Default" is stored in here during the element save.  */
        case 2:
            edP = styleElement.GetElementDescrP();
            if (edP->Element().ToMlineStyleEntryElm().entryFlags.isDefault)
                memset (edP->ElementR().ToMlineStyleEntryElmR().deprecatedName, 0, sizeof(edP)->Element().ToMlineStyleEntryElm().deprecatedName);
            /* FALLTHROUGH - keep on upgrading */

        /* 3 -> 4 again resets the name to blank, but now it is stored as a string linkage. This should really only
           affect mline styles that are Default; the first bit should never be hit. */
        case 3:
            {
            edP = styleElement.GetElementDescrP();
            if (!edP->Element().ToMlineStyleEntryElm().entryFlags.isDefault)
                {
                WString nameBuf;
                BeStringUtilities::Utf16ToWChar (nameBuf, edP->Element().ToMlineStyleEntryElm().deprecatedName);
                nameBuf.Trim();

                if (!nameBuf.empty())
                    {
                    DgnV8ElementBlank   elm;

                    edP->Element().CopyTo (elm);
                    LinkageUtil::SetStringLinkage (&elm, STRING_LINKAGE_KEY_Name, nameBuf.c_str());
                    styleElement.ReplaceElement (&elm);
                    edP = styleElement.GetElementDescrP();
                    }
                else
                    LinkageUtil::DeleteStringLinkage (&edP->ElementR(), STRING_LINKAGE_KEY_Name, 0);
                }
            memset (edP->ElementR().ToMlineStyleEntryElmR().deprecatedName, 0, sizeof(edP)->Element().ToMlineStyleEntryElm().deprecatedName);

            break;
            }
        }
    edP->ElementR().ToMlineStyleEntryElmR().version = MLINE_STYLE_VERSION;

    return (SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* Upgrade the style element to the current version
*
* @bsimethod                    ChuckKirschman                  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStyle::UpgradeTable (EditElementHandleR styleElement, bool& wasChanged)
    {
    wasChanged = false;

    for (ChildEditElemIter childIter (styleElement, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        if (!IsMlineStyleElement (childIter.GetElementCP()))
            continue;

        bool    elmChanged = false;
        UpgradeElement (childIter, elmChanged);
        if (elmChanged)
            wasChanged = true;
        }

    return SUCCESS;
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct MlineStyleVersionChangeListener : DgnFileEvents
{
private:
    
    static      MlineStyleVersionChangeListener  s_listener;
public:

    MlineStyleVersionChangeListener () {;}

    static  MlineStyleVersionChangeListener* GetInstance () {return &s_listener;}
    void    Register () {DgnFile::AddListener (this);}
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 07/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _OnVersionChange (DgnModelR cache, UInt32 oldMajorVer, UInt32 oldMinorVer) override
        {
        if (!cache.IsDictionaryModel ())
            return;

        EditElementHandle  tableElm (MultilineStyle::GetTableElement (*cache.GetDgnProject()), true);
        if (!tableElm.IsValid())
            return;
        
        ElementRefP oldElement = tableElm.GetElementRef ();

        bool wasChanged;
        if (SUCCESS != MultilineStyle::UpgradeTable (tableElm, wasChanged))
            return;
        
        // write the settings to table header element in an non undoable fashion
        if (wasChanged)
            {
            T_HOST.SetHostBoolVariable (s_inUpdateKey, true);
            TxnElementWriteOptions writeOpts = DgnPlatform::TxnElementWriteOptions();
            writeOpts.SetSaveInUndo (false);
            writeOpts.SetCallAsynchs (false);  // Otherwise StyleChanged gets called
            ITxnManager::GetCurrentTxn().ReplaceElement (tableElm, oldElement, writeOpts);
            T_HOST.SetHostBoolVariable (s_inUpdateKey, false);
            }
        }

}; // MlineStyleVersionChangeListener

MlineStyleVersionChangeListener MlineStyleVersionChangeListener::s_listener;
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct  MultilineStyleUpdater : IEditProperties 
    {
    protected:
    UInt64   m_styleElemID;
    UInt32      m_numChanged;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    MultilineStyleUpdater (UInt64 styleElemID)
        :
        m_styleElemID (styleElemID)
        {
        m_numChanged = 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementProperties    _GetEditPropertiesMask () override
        {
        return ELEMENT_PROPERTY_MLineStyle;
        }

public:
    void        ElementChanged () {m_numChanged++;}
    UInt32      GetNumElementsChanged () {return m_numChanged;}

    static void     UpdateWholeFile (MultilineStyleUpdater& updater, DgnProjectR dgnFile);
    static void     UpdateModel (MultilineStyleUpdater& updater, DgnModelP modelRef);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineStyleUpdater::UpdateWholeFile (MultilineStyleUpdater& updater, DgnProjectR dgnFile)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    DgnFile::AllElementsCollection collection  = dgnFile.GetAllElementsCollection();
    for (PersistentElementRefP const& elemRef : collection)
        {
        EditElementHandle eeh (elemRef, elemRef->GetDgnModelP());

        if (PropertyContext::EditElementProperties (eeh, &updater))
            {
            updater.ElementChanged();
            eeh.ReplaceInModel(elemRef);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineStyleUpdater::UpdateModel (MultilineStyleUpdater& updater, DgnModelP modelRef)
    {
#if defined DGNV10FORMAT_CHANGES_WIP
    DgnModel::ElementsCollection collection  = modelRef->GetRoot()->GetElementsCollection();
    for (PersistentElementRefP const& elemRef : collection)
        {
        EditElementHandle eeh (elemRef, elemRef->GetDgnModelP());

        if (PropertyContext::EditElementProperties (eeh, &updater))
            {
            updater.ElementChanged();
            eeh.ReplaceInModel(elemRef);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultilineStyleDependentUpdater : MultilineStyleUpdater
    {
    MultilineStyleDependentUpdater (UInt64 styleId)
        :MultilineStyleUpdater (styleId)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachMLineStyleCallback        (EachMLineStyleArg& arg) override
        {
        if (arg.GetStoredValue() == m_styleElemID)
            arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultilineStyleDropDependantsEditor : MultilineStyleUpdater
    {
    MultilineStyleDropDependantsEditor (UInt64 styleId)
        :MultilineStyleUpdater (styleId)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 11/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachMLineStyleCallback        (EachMLineStyleArg& arg) override
        {
        if (arg.GetStoredValue() == m_styleElemID)
            arg.SetStoredValue (ElementId().GetValue());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultilineStyleReplacer : MultilineStyleUpdater
    {
    UInt64   m_newStyleId;
    
    MultilineStyleReplacer (UInt64 oldStyleId, UInt64 newStyleId)
        :MultilineStyleUpdater (oldStyleId),
        m_newStyleId (newStyleId)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachMLineStyleCallback        (EachMLineStyleArg& arg) override
        {
        if (arg.GetStoredValue() == m_styleElemID)
            {
            arg.SetStoredValue (m_newStyleId);
            if (ElementId(m_newStyleId).IsValid())
                arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
            }
        }
    };

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void       MultilineStyle::StaticInitialize ()
    {
    BeDebugLog ("MultilineStyle::StaticInitialize");
    MlineStyleVersionChangeListener::GetInstance()->Register();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineStyle::Replace (WCharCP oldName, DgnProjectP destFile)
    {
    if (NULL == destFile)
        destFile = &(GetFile());

    // Get the name of the style to replace
    WString         nameToReplace = (NULL != oldName) ? oldName : m_name;

    if (nameToReplace.length() == 0)
        return ERROR;

    if (oldName && (0 != m_name.CompareToI(oldName))) 
        {
        MultilineStylePtr existingNamedStyle = GetByName (m_name.c_str(), GetFile());
        if (existingNamedStyle.IsValid())
            return ERROR;
        }

    ConvertToFile (*destFile);

    // Find table element
    EditElementHandle   tableElm (GetTableElement (GetFile()), true);
    if (!tableElm.IsValid())
        return BSIERROR;
    ElementRefP         oldRef = tableElm.GetElementRef ();

    // Replace the entry element
    for (ChildEditElemIter iter (tableElm, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (MlineStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = MlineStyleEntryHandler::GetName (*elemCP);

        if (0 != BeStringUtilities::Wcsicmp (nameToReplace.c_str(), elemName.c_str()))
            continue;

        // Set the newStyle to use the ID of the old element
        m_elemID = elemCP->ehdr.uniqueId;

        // Create the new element
        EditElementHandle  newElm;
        ToElement (newElm);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();

        // Replace the old element with the new one, linking it into the chain.
        iter.ReplaceElementDescr (newEdP);
        break;
        }

    // Rewrite the whole table
    if (SUCCESS != tableElm.ReplaceInModel(oldRef))
        return BSIERROR;
    
    // Need to update all the elements that use the style
    MultilineStyleDependentUpdater  updater (m_elemID);
    MultilineStyleUpdater::UpdateWholeFile (updater, GetFile());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineStyle::Add (DgnProjectP file)
    {
    if ( ! m_initialized || NULL == m_file || m_numProfiles == 0)
        { BeAssert (false); return ERROR; }
    
    DgnProjectR        dgnFile = file ? *file : GetFile();
    
    ConvertToFile (dgnFile);  // Clone all the bits of the element

    // Find table element
    EditElementHandle   tableElm;
    if (SUCCESS != GetTableElementForWrite (tableElm, dgnFile) || !tableElm.IsValid())
        return ERROR;
    ElementRefP         oldRef = tableElm.GetElementRef ();

    // Add the entry element
    WString   styleName = GetName();

    // Check to see if the style already exists by name
    for (ChildEditElemIter iter (tableElm, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = MlineStyleEntryHandler::GetName (*elemCP);
        
        if (0 == BeStringUtilities::Wcsicmp (styleName.c_str(), elemName.c_str()))
            return ERROR;
        }

    // Create the new element
    EditElementHandle  newElm;
    ToElement (newElm);
    MSElementDescrP newEdP   = newElm.ExtractElementDescr();

    // Add it to the table
    if (SUCCESS != tableElm.GetElementDescrP()->AppendDescr (newEdP))
        return ERROR;

    // Rewrite the whole table
    BentleyStatus status = (BentleyStatus)tableElm.ReplaceInModel(oldRef);
    
    // Update the style because it can change; levels change on write...
    for (ChildEditElemIter iter (tableElm, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = MlineStyleEntryHandler::GetName (*elemCP);
        
        if (0 == BeStringUtilities::Wcsicmp (styleName.c_str(), elemName.c_str()))
            {
            newEdP = MSElementDescr::Allocate (*elemCP, tableElm.GetDgnModel());
            newElm.SetElementDescr (newEdP, true, false, tableElm.GetDgnModel());
            FromElement (newElm);
            m_file = &dgnFile;  // Don't want to mess with setting modelRef in elmdscr before this
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineStyle::Delete (WCharCP name, DgnProjectR dgnFile)
    {
    if (NULL == name)
        { BeAssert (false); return BSIERROR; }
    
    // Find table element
    EditElementHandle   tableElm (GetTableElement (dgnFile), true);
    if (!tableElm.IsValid())
        return BSIERROR;
    ElementRefP         oldRef = tableElm.GetElementRef ();

    // Delete the entry element
    bool found = false;
    for (ChildEditElemIter iter (tableElm, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (MlineStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = MlineStyleEntryHandler::GetName (*elemCP);

        if (0 != BeStringUtilities::Wcsicmp (name, elemName.c_str()))
            continue;
        
        // Make sure it's not used; reject if it is.
        if (IsMlineStyleUsed (elemCP->ehdr.uniqueId, dgnFile))
            return BSIERROR; //STYLETABLE_ERROR_StyleIsUsed;

        iter.GetElementDescrP()->RemoveElement ();

        found = true;
        break;
        }

    if (!found)
        return BSIERROR;

    // Rewrite the whole table
    return (BentleyStatus)tableElm.ReplaceInModel(oldRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString MultilineStyle::GetNameFromId (ElementId styleId, DgnProjectR file)
    {
    ElementHandle styleElm (styleId, file.GetDictionaryModel());
    if (!styleElm.IsValid())
        return L"";
    return MlineStyleEntryHandler::GetName (*styleElm.GetElementCP());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       MultilineStyle::RemapDependents (WCharCP destName, WCharCP srcName, DgnProjectR file)
    {
    MultilineStylePtr srcStyle  = GetByName (srcName, file);
    MultilineStylePtr destStyle = GetByName (destName, file);

    if (srcStyle.IsNull() || destStyle.IsNull())
        return ERROR;

    StyleDependantRemapper remapper;

    remapper.AddMLineStyleRemap (srcStyle->GetID(), destStyle->GetID());
    remapper.DoRemapping (file);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultilineStyleIsUsedQuery : IQueryProperties
    {
    bool        m_found;
    ElementId   m_styleElemID;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    MultilineStyleIsUsedQuery (ElementId styleElemID)
        :
        m_found (false),
        m_styleElemID (styleElemID)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementProperties    _GetQueryPropertiesMask () override
        {
        return ELEMENT_PROPERTY_MLineStyle;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachMLineStyleCallback        (EachMLineStyleArg& arg) override
        {
        if (arg.GetStoredValue() == m_styleElemID.GetValue())
            m_found = true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineStyle::IsMlineStyleUsed (ElementId styleId, DgnProjectR dgnFile)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    if (0 == styleId)
        return false;

    MultilineStyleIsUsedQuery   queryObj (styleId);

    DgnFile::AllElementsCollection collection  = dgnFile.GetAllElementsCollection();
    for (PersistentElementRefP const& elemRef : collection)
        {
        EditElementHandle eeh (elemRef, elemRef->GetDgnModelP());
        PropertyContext::QueryElementProperties (eeh, &queryObj);

        if (queryObj.m_found)
            return true;
        }
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MultilineStyle::HasDependants () const
    {
    return IsMlineStyleUsed (GetID(), GetFile());
    }

static DgnHost::Key s_transactionListenerKey;

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStyle::OnMlineStyleTransactionEvent (ElementHandleCP entryBefore, ElementHandleCP entryAfter, StyleEventType eventType, StyleEventSource source)
    {
    IMlineStyleTransactionListener* listener = (IMlineStyleTransactionListener*) T_HOST.GetHostVariable (s_transactionListenerKey);
    if (NULL == listener)
        return;

    MultilineStylePtr styleBefore;
    MultilineStylePtr styleAfter;
    
    if (entryBefore)
        styleBefore = MultilineStyle::CreateFromElement (*entryBefore);

    if (entryAfter)
        styleAfter = MultilineStyle::CreateFromElement (*entryAfter);

    listener->_OnMlineStyleChange (styleBefore.get(), styleAfter.get(), eventType, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStyle::SetTransactionListener (IMlineStyleTransactionListener* obj)
    {
    BeAssert (NULL == obj || NULL == T_HOST.GetHostVariable (s_transactionListenerKey));
    T_HOST.SetHostVariable (s_transactionListenerKey, obj);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MultilineStyle::GetFillColor () const
    {
    return m_fillColor;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void           MultilineStyle::SetFillColor (UInt32 value)
    {
    m_fillColor = value;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool          MultilineStyle::GetFilled () const
    {
    return m_isFilled;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void           MultilineStyle::SetFilled (bool value)
    {
    m_isFilled = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MultilineStyle::GetProfileLineCount () const
    {
    return m_numProfiles;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double          MultilineStyle::GetOriginAngle () const
    {
    // Org angle and end angle in styles were originally stored as degrees because
    // that's how they were stored in the TCB for GUI reasons.  Can't change it
    // because older versions won't know how to read the style:None.
    // Most API's return radians, so we will go with that.
    return  m_orgAngle * msGeomConst_radiansPerDegree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineStyle::SetOriginAngle (double value) 
    {
    m_orgAngle = value * msGeomConst_degreesPerRadian;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
double          MultilineStyle::GetEndAngle () const
    {
    // Org angle and end angle in styles were originally stored as degrees because
    // that's how they were stored in the TCB for GUI reasons.  Can't change it
    // because older versions won't know how to read the style:None.
    // Most API's return radians, so we will go with that.
    return  m_endAngle * msGeomConst_radiansPerDegree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MultilineStyle::SetEndAngle (double value) 
    {
    m_endAngle = value * msGeomConst_degreesPerRadian;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfilePtr           MultilineStyle::GetProfile (UInt32 profileNum) const
    {
    if (profileNum >= m_numProfiles)
        return NULL;

    return new MultilineProfile(*m_profiles[profileNum]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineProfileCP           MultilineStyle::GetProfileCP (UInt32 profileNum) const
    {
    if (profileNum >= m_numProfiles)
        return NULL;

    return m_profiles[profileNum].get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MultilineStyle::ReplaceProfile (MultilineProfileCR profile, UInt32 profileNum)
    {
    if (profileNum >= m_numProfiles) // Outside of existing profiles
        return BSIERROR;
    
    m_profiles[profileNum]->Copy (profile);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* Insert a profile at the specifed index.  All following lines will be moved up one index.  
*
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           MultilineStyle::InsertProfile (MultilineProfileCR profile, UInt32 profileNum)
    {
    if (MULTILINE_MAX == m_numProfiles) // Structure is full
        return BSIERROR;
        
    if (profileNum == -1 || profileNum > m_numProfiles)  // -1 means append it
        profileNum = m_numProfiles;

    if (profileNum >= MULTILINE_MAX)
        return BSIERROR;

    MultilineProfileP newProfile = new MultilineProfile (profile);
    m_profiles.insert (m_profiles.begin()+profileNum, newProfile);
    m_numProfiles++;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* Delete a profile line.  All folowing lines will be moved down one index.
*
* @bsimethod                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           MultilineStyle::RemoveProfile (UInt32 profileNum)
    {
    if (profileNum >= m_numProfiles)
        return BSIERROR;
        
    m_profiles.erase (m_profiles.begin()+profileNum);
    m_numProfiles--;
    return BSISUCCESS;
    }
    
/*----------------------------------------------------------------------------------*//**
* Empty the profile list by setting the number of lines to 0.
*
* @bsimethod                    Chuck.Kirschman                 08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void           MultilineStyle::ClearProfiles ()
    {
    m_profiles.clear();
    m_numProfiles = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr        MultilineStyle::GetOrgCap () const
    {
    return new MultilineSymbology (m_orgCap);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineStyle::SetOrgCap (MultilineSymbologyCR symb)
    {
    m_orgCap.Copy (symb);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr        MultilineStyle::GetEndCap () const
    {
    return new MultilineSymbology (m_endCap);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineStyle::SetEndCap (MultilineSymbologyCR symb)
    {
    m_endCap.Copy (symb);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyPtr        MultilineStyle::GetMidCap () const
    {
    return new MultilineSymbology (m_midCap);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultilineStyle::SetMidCap (MultilineSymbologyCR symb)
    {
    m_midCap.Copy (symb);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyCR       MultilineStyle::GetOrgCapCR () const
    {
    return m_orgCap;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyCR       MultilineStyle::GetEndCapCR () const
    {
    return m_endCap;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineSymbologyCR       MultilineStyle::GetMidCapCR () const
    {
    return m_midCap;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStyle::ConvertToFile (DgnProjectR destFile)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    if (destFile.GetDocument().IsSameFile (GetFile().GetDocument()))
        return SUCCESS;

    EditElementHandle element;
    ToElement (element);
    if (element.IsValid())
        {
        ElementCopyContext copier (&destFile.GetDictionaryModel(), &GetFile().GetDictionaryModel(), false, false, false, NULL, ElementCopyContext::CLONE_OPTIONS_None);
        copier.SetWriteElements (false);
        copier.DoCopy (element);

        FromElement (element);
        }

    return SUCCESS;
#endif
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePropMask::MultilineStylePropMask ()
    {
    m_pBitMask = BitMask::Create ( false);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePropMask::~MultilineStylePropMask ()
    {
    m_pBitMask->Free ();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
MultilineStylePropMaskPtr    MultilineStylePropMask::Create ()
    {
    return new MultilineStylePropMask ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStylePropMask::ClearAllBits ()
    {
    m_pBitMask->SetAll ( false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineStylePropMask::AnyBitSet () const
    {
    return TO_BOOL(m_pBitMask->AnyBitSet ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStylePropMask::CopyValues (MultilineStylePropMaskCR other)
    {
    m_pBitMask->Free ();

    m_pBitMask = BitMask::Clone (*(other.m_pBitMask));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MultilineStylePropMask::AreDoublesEqual (double value1, double value2, double tol)
    {
    return  ! (fabs (value1 - value2) > tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineStylePropMask::GetGeneralBit (UInt32 item) const
    {
    BeAssert (item < SHIELD_BIT_SEGMENT);
    if (item >= SHIELD_BIT_SEGMENT)
        return false;

    return m_pBitMask->Test (item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStylePropMask::SetGeneralBit (UInt32 item, bool bitValue)
    {
    BeAssert (item < SHIELD_BIT_SEGMENT);
    if (item >= SHIELD_BIT_SEGMENT)
        return ERROR;

    return (BentleyStatus)m_pBitMask->SetBits (1, &item, bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineStylePropMask::GetCapBit (MultilineCapType capIndex, UInt32 item) const
    {
    BeAssert (capIndex >= MULTILINE_ORG_CAP && capIndex <= MULTILINE_MID_CAP);
    UInt32       index = ((capIndex+1) * SHIELD_BIT_SEGMENT) + item;

    BeAssert (item < SHIELD_BIT_SEGMENT && capIndex <= MULTILINE_MID_CAP);
    if (item >= SHIELD_BIT_SEGMENT || capIndex > MULTILINE_MID_CAP)
        return false;

    return m_pBitMask->Test (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStylePropMask::SetCapBit (MultilineCapType capIndex, UInt32 item, bool bitValue)
    {
    BeAssert (capIndex >= MULTILINE_ORG_CAP && capIndex <= MULTILINE_MID_CAP);
    UInt32         index = ((capIndex+1) * SHIELD_BIT_SEGMENT) + item;

    BeAssert (item < SHIELD_BIT_SEGMENT && capIndex <= MULTILINE_MID_CAP);
    if (item >= SHIELD_BIT_SEGMENT || capIndex > MULTILINE_MID_CAP)
        return ERROR;

    return (BentleyStatus)m_pBitMask->SetBits (1, &index, bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineStylePropMask::GetProfileBit (UInt32 profileIndex, UInt32 item) const
    {
    UInt32       index = ((profileIndex+4) * SHIELD_BIT_SEGMENT) + item;  /* 4 = General + 3 caps */

    BeAssert (item < SHIELD_BIT_SEGMENT && profileIndex < MULTILINE_MAX);
    if (item >= SHIELD_BIT_SEGMENT || profileIndex >= MULTILINE_MAX)
        return false;

    return m_pBitMask->Test (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStylePropMask::SetProfileBit (UInt32 profileIndex, UInt32 item, bool bitValue)
    {
    UInt32       index = ((profileIndex+4) * SHIELD_BIT_SEGMENT) + item;  /* 4 = General + 3 caps */

    BeAssert (item < SHIELD_BIT_SEGMENT && profileIndex < MULTILINE_MAX);
    if (item >= SHIELD_BIT_SEGMENT || profileIndex >= MULTILINE_MAX)
        return ERROR;

    return (BentleyStatus)m_pBitMask->SetBits (1, &index, bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStylePropMask::SetCapOrProfileBit (bool isCap, UInt32 index, UInt32 item, bool bitValue)
    {
    if (isCap)
        return SetCapBit ((MultilineCapType)index, item, bitValue);
    else
        return SetProfileBit (index, item, bitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultilineStylePropMask::GetCapOrProfileBit (bool isCap, UInt32 index, UInt32 item) const
    {
    if (isCap)
        return GetCapBit ((MultilineCapType)index, item);
    else
        return GetProfileBit (index, item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineStylePropMask::CompareSymbology (MultilineSymbologyCR pSymb1, MultilineSymbologyCR pSymb2, bool isCap, UInt32 capIndex)
    {
    if (pSymb1.UsesClass() && pSymb2.UsesClass())
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Class, pSymb1.GetClass() != pSymb2.GetClass());
    else
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Class, pSymb1.UsesClass() != pSymb2.UsesClass());

    if (pSymb1.UsesColor() && pSymb2.UsesColor())
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Color, pSymb1.GetColor() != pSymb2.GetColor());
    else
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Color, pSymb1.UsesColor() != pSymb2.UsesColor());

    if (pSymb1.UsesWeight() && pSymb2.UsesWeight())
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Weight, pSymb1.GetWeight() != pSymb2.GetWeight());
    else
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Weight, pSymb1.UsesWeight() != pSymb2.UsesWeight());

    if (pSymb1.UsesLinestyle() && pSymb2.UsesLinestyle())
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_LineStyle, pSymb1.GetLinestyle() != pSymb2.GetLinestyle());
        // Also compare custom style data ?!
    else
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_LineStyle, pSymb1.UsesLinestyle() != pSymb2.UsesLinestyle());

    SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_Level, pSymb1.GetLevel() != pSymb2.GetLevel());

    if (isCap)
        {
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_CapColorFromSeg, pSymb1.UseCapColorFromSegment() != pSymb2.UseCapColorFromSegment());
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_CapLine,         pSymb1.UseCapLine() != pSymb2.UseCapLine());
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_CapOutArc,       pSymb1.UseCapOuterArc() != pSymb2.UseCapOuterArc());
        SetCapOrProfileBit (isCap, capIndex, MLINESTYLE_PROP_CapInArc,        pSymb1.UseCapInnerArc() != pSymb2.UseCapInnerArc());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MultilineStylePropMask::CompareProfiles (MultilineProfileCR pProfile1, MultilineProfileCR pProfile2, UInt32 profileIndex, double doubleTol)
    {
    CompareSymbology (pProfile1, pProfile2, false, profileIndex);

    bool offsetFlag = !AreDoublesEqual (pProfile1.GetDistance(), pProfile2.GetDistance(), doubleTol);
    SetCapOrProfileBit (false, profileIndex, MLINESTYLE_PROP_Offset, offsetFlag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultilineStylePropMask::CompareStyles (MultilineStyleCR pStyle1, MultilineStyleCR pStyle2)
    {
    const double    doubleTol = mgds_fc_epsilon;

    // Clear the bit mask
    ClearAllBits ();

    // General
    SetGeneralBit (MLINESTYLE_PROP_OrgAngle, !AreDoublesEqual (pStyle1.GetOriginAngle(), pStyle2.GetOriginAngle(), doubleTol));
    SetGeneralBit (MLINESTYLE_PROP_EndAngle, !AreDoublesEqual (pStyle1.GetEndAngle(), pStyle2.GetEndAngle(), doubleTol));
    SetGeneralBit (MLINESTYLE_PROP_NumLines, pStyle1.GetProfileLineCount() != pStyle2.GetProfileLineCount());

    /* If either the fill types don't match, or the fill colors don't match, mark fill as override. */
    if (pStyle1.GetFilled() && pStyle2.GetFilled())
        {
        SetGeneralBit (MLINESTYLE_PROP_Fill, pStyle1.GetFillColor() != pStyle2.GetFillColor());
        }
    else
        {
        SetGeneralBit (MLINESTYLE_PROP_Fill, pStyle1.GetFilled() != pStyle2.GetFilled());
        }

    // Caps
    CompareSymbology (pStyle1.GetOrgCapCR(), pStyle2.GetOrgCapCR(), true, MULTILINE_ORG_CAP);
    CompareSymbology (pStyle1.GetEndCapCR(), pStyle2.GetEndCapCR(), true, MULTILINE_END_CAP);
    CompareSymbology (pStyle1.GetMidCapCR(), pStyle2.GetMidCapCR(), true, MULTILINE_MID_CAP);

    // Profiles
    UInt32 maxProfile = pStyle1.GetProfileLineCount() < pStyle2.GetProfileLineCount() ? pStyle1.GetProfileLineCount() : pStyle2.GetProfileLineCount();
    for (UInt32 iProfile=0; iProfile<maxProfile; iProfile++)
        {
        CompareProfiles (*pStyle1.GetProfileCP(iProfile), *pStyle2.GetProfileCP(iProfile), iProfile, doubleTol);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStylePropMask::LogicalOperation (MultilineStylePropMaskCR propMask1, MultilineStylePropMaskCR propMask2, BitMaskOperation operation)
    {
    if (this != &propMask1)
        CopyValues (propMask1);

    return (BentleyStatus) (m_pBitMask->LogicalOperation(*( propMask2.m_pBitMask),  operation), SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStylePropMask::FromElement (ElementHandleR element)
    {
    BitMaskP        pBitMask = NULL;
    StatusInt status = BitMaskLinkage::ExtractBitMask (&pBitMask, element.GetElementCP(), BITMASK_LINKAGE_KEY_MlineOverrideFlags, 0);

    if (SUCCESS == status)
        {
        m_pBitMask->Free ();
        m_pBitMask = pBitMask;
        return BSISUCCESS;
        }
    else if (DGNHANDLERS_STATUS_LinkageNotFound == status)
        {
        m_pBitMask->Free ();
        m_pBitMask = BitMask::Create ( 0);
        return BSISUCCESS;
        }
    else
        {
        return (BentleyStatus)status;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStylePropMask::RemoveFromElement (EditElementHandleR element)
    {
    DgnV8ElementBlank   elm;
    element.GetElementCP ()->CopyTo (elm);
    BitMaskLinkage::DeleteBitMask (&elm, BITMASK_LINKAGE_KEY_MlineOverrideFlags, 0);
    element.ReplaceElement (&elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    MultilineStylePropMask::AddToElement (EditElementHandleR element) const
    {
    RemoveFromElement (element);
    DgnV8ElementBlank   elm;
    element.GetElementCP ()->CopyTo (elm);
    BentleyStatus status = (BentleyStatus)BitMaskLinkage::SetBitMask (&elm, BITMASK_LINKAGE_KEY_MlineOverrideFlags, m_pBitMask);
    element.ReplaceElement (&elm);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultilineStylePropMask::Dump () const
    {
    UInt        size;
    bool        bit;
    char        message[512];

    message[0] = '\0';
    size = m_pBitMask->GetCapacity ();

    for (Int32 i=0; ((i-1)*SHIELD_BIT_SEGMENT)<=(Int32)size; i++)
        {
        int segment = i;
        UInt curBit = segment * SHIELD_BIT_SEGMENT;

        if (segment == 0)
            {
            sprintf (message, "General: ");
            for (int j=0; j<4; j++)
                {
                bit = m_pBitMask->Test(j);
                strcat (message, bit ? "1 " : "0 ");
                if (++curBit > size)
                    break;
                }
            }

        else if (segment > 0 && segment < 4)
            {
            sprintf (message, "CAP %d : ", segment-1);
            for (int j=0; j<12; j++)
                {
                bit = m_pBitMask->Test((segment*SHIELD_BIT_SEGMENT)+j);
                strcat (message, bit ? "1 " : "0 ");
                if (++curBit > size)
                    break;
                }
            }

        else
            {
            sprintf (message, "Profile %d : ", segment-4);
            for (int j=0; j<9; j++)
                {
                bit = m_pBitMask->Test((segment*SHIELD_BIT_SEGMENT)+j);
                strcat (message, bit ? "1 " : "0 ");
                if (++curBit > size)
                    break;
                }
            }
        puts(message);
        printf ("\n");
        }
    }

// MlineStyleTableHandler - gutted and moved to ForeignFormat/TableHandlers.cpp