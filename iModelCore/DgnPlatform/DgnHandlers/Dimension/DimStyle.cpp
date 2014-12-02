/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimStyle.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"


#define     MS_DIMSTYLE_TABLE_LEVEL                             5
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::Create (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }

    return new DimensionStyle (name, file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::GetByName (WCharCP name, DgnProjectR file)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    if (NULL == name)
        { BeAssert (false); return NULL; }

    ElementHandle  styleElem = GetStyleElementByName (name, file);

    if ( ! styleElem.IsValid())
        return NULL;

    return new DimensionStyle (styleElem);
#endif
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::GetByID (UInt64 elemID, DgnProjectR file)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    ElementHandle tableEh = GetTableElement(file);
    if (!tableEh.IsValid())
        return NULL;

    for (ChildElemIter dimStyleEhIter(tableEh, ExposeChildrenReason::Count); dimStyleEhIter.IsValid(); dimStyleEhIter = dimStyleEhIter.ToNext())
        {
        if (DimStyleEntryHandler::IsSettings (dimStyleEhIter))
            continue;

        if (dimStyleEhIter.GetElementId() == elemID)
            return new DimensionStyle (dimStyleEhIter);
        }
#endif

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::FixSettingsObjectFromElement ()
    {
    if (NULL == m_project)
        {
        BeAssert (false && "DimensionStyle::FixSettingsObjectFromElement called but m_file is NULL");
        return;
        }

    // This method must be called whenever a DimensionStyle is created from the
    // Settings element

    // In the settings element, the Name of the active style is stored as a Description.
    // Here we get the name out from the description slot, and use it to fixup with
    // object to accurately represent the settings.
    WString         styleName = m_description;

    DimensionStylePtr namedStyle = GetByName (styleName.c_str(), *m_project);

    if ( ! namedStyle.IsValid())
        {
        // In this case, the settings object represents style:<none>.  If this
        // object is pushed onto a dimension, the element will not have a
        // persistent style reference, and not have any shields.
        m_name.assign(L"");
        m_description.assign(L"");
        m_elemID = 0;
        }
    else
        {
        // The goal is for the settings object to be indistinguishable from the
        // named style.  Of course its properties may be different, but that is
        // just like a named style object with unsaved changes.  If this object
        // is pushed onto a dimension, the element will persistently refer to
        // the named style, with shields as necessary.
        m_name.assign(namedStyle->m_name.c_str());
        m_description.assign(namedStyle->m_description.c_str());
        m_elemID = namedStyle->m_elemID;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::GetSettings (DgnProjectR project)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    ElementHandle  settingsElem = GetSettingsElementByFile (project);

    if ( ! settingsElem.IsValid())
        return NULL;

    DimensionStyleP settingsObj = new DimensionStyle (settingsElem);
    settingsObj->FixSettingsObjectFromElement();

    return settingsObj;
#endif
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::ReplaceSettings (DimensionStyleR settingsObj, DgnProjectR file)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    DimensionStylePtr namedStyle = GetByName (settingsObj.m_name.c_str(), file);

    if (namedStyle.IsValid())
        settingsObj.m_description.assign (settingsObj.m_name.c_str());
    else
        settingsObj.m_description.assign (L"");

    // For the settings element, the Name of the active style is stored as a Description.
    // Here we set the name into the description slot, and empty the name.
    // This is the reverse of FixSettingsObjectFromElement()

    settingsObj.m_name.assign (L"");
    settingsObj.m_elemID = 0; // will be set properly in ReplaceSettingElement

    // Find table element
    EditElementHandle  tableElm (FetchTableElement (file), true);

    // Replace the settings element, possibly cloning into the destination file.
    DimStyleTableEditor editor (tableElm);
    if (SUCCESS != editor.ReplaceSettingsElement (settingsObj))
        return ERROR;

    if (NULL == tableElm.GetElementRef () && SUCCESS != tableElm.AddToModel())
        return ERROR;
    else if (SUCCESS != tableElm.ReplaceInModel(tableElm.GetElementRef ()))
        return ERROR;

    // Replace the input settingsObject with the contents of the table
    ElementHandle newElm = GetSettingsElementByFile (file);
    settingsObj.FromElement (newElm);
    settingsObj.FixSettingsObjectFromElement();

    return SUCCESS;
#endif
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::CreateFromDgnDimStyle (DgnDimStyleCR dgnDimStyle)
    {
    //WIP TODO: Test it for annotation unscaling
    return new DimensionStyle (dgnDimStyle);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleTableEditor::DimStyleTableEditor (EditElementHandleR table)
    :
    m_tableElem (table)
    {
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::_CopyValues (DimensionStyleCR  source)
    {
    /*-----------------------------------------------------------------------------------
        Fixed portion of the object
    -----------------------------------------------------------------------------------*/
    m_elemID      = source.m_elemID;
    m_data        = source.m_data;
    m_extensions  = source.m_extensions;

    m_textStyle->CopyPropertyValuesFrom(*source.m_textStyle);
    m_textStyle->SetId (DgnStyleId(source.m_textStyle->GetId()));
    m_textStyle->SetName (source.m_textStyle->GetName().c_str());
    /*-----------------------------------------------------------------------------------
        Strings
    -----------------------------------------------------------------------------------*/
    m_name.assign           (source.m_name);
    m_description.assign    (source.m_description);
    m_prefixCellName.assign (source.m_prefixCellName);
    m_suffixCellName.assign (source.m_suffixCellName);
    m_arrowCellName.assign  (source.m_arrowCellName);
    m_strokeCellName.assign (source.m_strokeCellName);
    m_originCellName.assign (source.m_originCellName);
    m_dotCellName.assign    (source.m_dotCellName);
    m_noteCellName.assign   (source.m_noteCellName);

    m_primary.m_masterUnitLabel.assign   (source.m_primary.m_masterUnitLabel);
    m_primary.m_subUnitLabel.assign      (source.m_primary.m_subUnitLabel);
    m_secondary.m_masterUnitLabel.assign (source.m_secondary.m_masterUnitLabel);
    m_secondary.m_subUnitLabel.assign    (source.m_secondary.m_subUnitLabel);

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr DimensionStyle::Copy () const
    {
    DimensionStyleP newStyle = new DimensionStyle (m_name.c_str(), *m_project);

    newStyle->CopyValues (*this);

    return newStyle;
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DimStyleTableEditor::ElementFromStyle (EditElementHandleR newElm, DimensionStyleR newStyle)
    {
    newStyle.ToElement (newElm);

    if (m_tableElem.GetDgnProject() == newElm.GetDgnProject())
        return;

    // Convert the element to the context of the destination file
    DgnModelP   srcModel = newElm.GetDgnProject()->GetDictionaryModel();
    DgnModelP   dstModel = m_tableElem.GetDgnProject()->GetDictionaryModel();

    IllegalDependencyRemapper noDependenciesExpected;
    CopyContext copier (dstModel, srcModel, false, false, false, NULL, CopyContext::CLONE_OPTIONS_None, noDependenciesExpected);
    copier.SetWriteElements (false);
    copier.DoCopy (newElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStyleTableEditor::AddEntryElement (DimensionStyleR newStyle)
    {
    // Check to see if the style already exists by name
    ElementHandle  duplicateElm = DimensionStyle::GetStyleElementByName (newStyle.GetName().c_str(), m_tableElem);

    if (duplicateElm.IsValid())
        return ERROR;

    // Create the new element in the context of the table's file
    EditElementHandle  newElm;
    ElementFromStyle (newElm, newStyle);
    MSElementDescrP newEdP   = newElm.ExtractElementDescr();

    // Add it to the table
    return m_tableElem.GetElementDescrP()->AppendDescr (newEdP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStyleTableEditor::RemoveEntryElement (WCharCP name)
    {
    if (NULL == name)
        { BeAssert (false); return ERROR; }

    // Find the style element by name and remove it
    for (ChildEditElemIter iter (m_tableElem, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (DimStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = DimStyleEntryHandler::GetName(*elemCP);

        if (0 != BeStringUtilities::Wcsicmp (name, elemName.c_str()))
            continue;

        if (DimensionStyle::IsDimStyleUsed (elemCP->ehdr.uniqueId, *m_tableElem.GetDgnProject()))
            break;

        iter.GetElementDescrP()->RemoveElement ();

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStyleTableEditor::ReplaceSettingsElement (DimensionStyleR settingsObj)
    {
    // Find the settings element and replace it
    for (ChildEditElemIter iter (m_tableElem, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if ( ! DimStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();

        // Create the new element in the context of the table's file
        EditElementHandle  newElm;
        ElementFromStyle (newElm, settingsObj);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();

        UInt entryID = DgnTableUtilities::GetEntryId (*elemCP);
        DgnTableUtilities::SetEntryId (newEdP->el, entryID);
        // Set the new element to use the ID of the old element
        newEdP->el.ehdr.uniqueId = elemCP->ehdr.uniqueId;

        // Replace the old element with the new one
        return iter.ReplaceElementDescr (newEdP);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimStyleTableEditor::ReplaceEntryElement (DimensionStyleR newStyle, WCharCP oldName)
    {
    if (NULL == oldName)
        { BeAssert (false); return ERROR; }

    // Find the style element with the same name and replace it
    for (ChildEditElemIter iter (m_tableElem, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (DimStyleEntryHandler::IsSettings (iter))
            continue;

        DgnElementCP     elemCP   = iter.GetElementCP ();
        WString         elemName = DimStyleEntryHandler::GetName(*elemCP);

        if (0 != BeStringUtilities::Wcsicmp (oldName, elemName.c_str()))
            continue;

        // Set the newStyle to use the ID of the old element
        newStyle.SetElemID (elemCP->ehdr.uniqueId);

        // Create the new element in the context of the table's file
        EditElementHandle  newElm;
        ElementFromStyle (newElm, newStyle);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();

        // Replace the old element with the new one
        return iter.ReplaceElementDescr (newEdP);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleIterator::DimStyleIterator () { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleIterator::DimStyleIterator (DgnProjectP file)
    {
    if (NULL == file)
        return;
    ElementHandle hdr = DimensionStyle::GetTableElement (*file);
    if (!hdr.IsValid())
        return;

    m_elemIter = ChildElemIter(hdr, ExposeChildrenReason::Count);

    while (m_elemIter.IsValid() && DimStyleEntryHandler::IsSettings (m_elemIter))
        m_elemIter = m_elemIter.ToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleIterator& DimStyleIterator::operator ++ ()
    {
    m_current = NULL;

    if (!m_elemIter.IsValid())
        return *this;

    m_elemIter = m_elemIter.ToNext();

    while (m_elemIter.IsValid() && DimStyleEntryHandler::IsSettings (m_elemIter))
        m_elemIter = m_elemIter.ToNext();

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimStyleIterator::operator == (DimStyleIterator const& rhs) const
    {
    bool lhsValid = m_elemIter.IsValid();
    bool rhsValid = rhs.m_elemIter.IsValid();
    if (!lhsValid && !rhsValid)
        return true;

    if (!lhsValid || !rhsValid)
        return false;

    if (m_elemIter.GetDgnProject() != rhs.m_elemIter.GetDgnProject())
        return false;

    return m_elemIter.GetElementCP() == rhs.m_elemIter.GetElementCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimStyleIterator::operator != (DimStyleIterator const& rhs) const
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStyleP const& DimStyleIterator::operator * () const
    {
    if (!m_current.IsValid())
        m_current = new DimensionStyle (m_elemIter);
    return m_current.GetCR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimStyleIterator::IsValid() const
    {
    return m_elemIter.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleCollection::DimStyleCollection (DgnProjectR file)
 :m_dgnFile(&file)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleIterator DimStyleCollection::begin () const
    {
    return DimStyleIterator (m_dgnFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStyleIterator DimStyleCollection::end () const {return DimStyleIterator (NULL);}
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static int      dgnDimStyle_initializeToDefaultStyle
(
DimStyleSettings  *pDs
)
    {
    int                 i;
    UShort              option = 0xFFFF;

    /*---------------------------------------------------------------
    In version 2, all the doubles were converted when they should
    have been unconverted and vice versa.
    ---------------------------------------------------------------*/

    if (option & 0x1)        /* Initialize all of Autodim4                  */
        {
        /* fill in everythingn with zeroes and set only nonzero values */
        memset (&pDs->ad4, 0, sizeof (pDs->ad4));

        /* Command: 0 - dimension size with arrows */
        pDs->ad4.dim_template[0].left_term      = 1;
        pDs->ad4.dim_template[0].right_term     = 1;
        pDs->ad4.dim_template[0].left_witness   = 1;
        pDs->ad4.dim_template[0].right_witness  = 1;

        /* Command 1 - dimension size with stroke */
        pDs->ad4.dim_template[1].left_term      = 2;
        pDs->ad4.dim_template[1].right_term     = 2;
        pDs->ad4.dim_template[1].left_witness   = 1;
        pDs->ad4.dim_template[1].right_witness  = 1;

        /* Command 2 - dimension single location */
        pDs->ad4.dim_template[2].first_term     = 3;
        pDs->ad4.dim_template[2].right_term     = 1;
        pDs->ad4.dim_template[2].left_witness   = 1;
        pDs->ad4.dim_template[2].right_witness  = 1;

        /* Command 3 - dimension stacked location */
        /* Command 4 - dimension angle size */
        /* Command 5 - dimension arc size */
        /* Command 6 - dimension angle location */
        /* Command 7 - dimension arc location */
        /* Command 8 - dimension angle between lines */
        /* Command 9 - dimension angle from axis */
        /* Command 10 - dimension radius */
        /* Command 11 - dimension diameter */
        /* Command 12 - dimension diameter parallel */
        /* Command 13 - dimension diameter perpendicular */
        /* Command 14 - user defined linear dimension */

        /* user standard boiler template */
        for (i=3; i<16; i++)
            pDs->ad4.dim_template[i] = pDs->ad4.dim_template[0];

        /* dimension angle between lines has no witness lines */
        pDs->ad4.dim_template[8].left_witness  = false;
        pDs->ad4.dim_template[8].right_witness = false;

        /* dimension radius and dimension diameter, angle from x
            and angle from y have no witness lines */
        for (i=8; i<12; i++)
            pDs->ad4.dim_template[i].left_witness =
            pDs->ad4.dim_template[i].right_witness = false;

        /* the diameter commands need the diameter symbol at
            either the beginning or the end of the text string */
        pDs->ad4.dim_template[10].pre_symbol = 2;
        pDs->ad4.dim_template[11].pre_symbol = 1;
        pDs->ad4.dim_template[12].pre_symbol = 1;
        pDs->ad4.dim_template[13].pre_symbol = 1;
        pDs->ad4.ext_dimflg.leading_zero     = true;
        pDs->ad4.ext_dimflg.trailing_zeros   = false;

        /* turn on stacked bit for stack commands */
        pDs->ad4.dim_template[3].stacked =
        pDs->ad4.dim_template[6].stacked =
        pDs->ad4.dim_template[7].stacked = true;
        }

    if (option & 0x2)        /*  Fix screwed up doubles in Autodim4         */
        {
        pDs->ad4.witness_offset  = 0.5;
        pDs->ad4.witness_extend  = 0.5;
        pDs->ad4.text_margin     = 2.0;
        pDs->ad4.toltxt_scale    = 1.0;
        pDs->ad4.dimension_scale = 1.0;
        }

    if (option & 0x4)        /* Set Autodim5 variables                      */
        {
        memset (&pDs->ad5, 0, sizeof (pDs->ad5));
        pDs->ad5.textMarginV  = 0.5;
        pDs->ad5.textMarginH  = 0.5;
        pDs->ad5.tolMarginV   = 0.5;
        pDs->ad5.tolMarginH   = 0.5;
        pDs->ad5.tolSepV      = 0.5;
        pDs->ad5.termHeight   = 0.5;
        pDs->ad5.termWidth    = 1.0;
        }

    if (option & 0x8)        /* Set new templates and Autodim6              */
        {
        memset (&pDs->ad4.dim_template[16], 0, 8 * sizeof(Dim_template));
        pDs->ad4.dim_template[16] = pDs->ad4.dim_template[10];
        pDs->ad4.dim_template[16].first_term = 3;
        pDs->ad4.dim_template[17] = pDs->ad4.dim_template[11];
        pDs->ad4.dim_template[17].first_term = 2;
        memset (&pDs->ad6, 0, sizeof (pDs->ad6));
        }

    pDs->ad1.mode.automan = 1;
    pDs->ad4.ext_dimflg.semiauto = 0;
    pDs->ad1.mode.witness = 1;     /* default witness lines to be on */
    pDs->ad1.mode.just    = 2;     /* set text alignment to center */

    /*-----------------------------------------------------------------------------------
        Initialize units.
    -----------------------------------------------------------------------------------*/
    pDs->ad1.params.overrideWorkingUnits = false;

    pDs->ad7.primaryMaster.flags.base   = static_cast<UInt32>(UnitBase::Meter);
    pDs->ad7.primaryMaster.flags.system = static_cast<UInt32>(UnitSystem::Metric);
    pDs->ad7.primaryMaster.numerator    = 1.0;
    pDs->ad7.primaryMaster.denominator  = 1.0;

    pDs->ad7.primarySub.flags.base      = static_cast<UInt32>(UnitBase::Meter);
    pDs->ad7.primarySub.flags.system    = static_cast<UInt32>(UnitSystem::Metric);
    pDs->ad7.primarySub.numerator       = 1.0;
    pDs->ad7.primarySub.denominator     = 1.0;

    memcpy (&pDs->ad7.secondaryMaster, &pDs->ad7.primaryMaster, sizeof (pDs->ad7.secondaryMaster));
    memcpy (&pDs->ad7.secondarySub,    &pDs->ad7.primarySub,    sizeof (pDs->ad7.secondarySub));

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStyle::DimensionStyle (WCharCP name, DgnProjectR project)
    {
    Clear();
    m_textStyle = DgnTextStyle::Create (project);
    if (NULL == name)
        BeAssert (false);
    else
        m_name.assign (name);

    m_project = &project;

    dgnDimStyle_initializeToDefaultStyle (&m_data);

    DgnFontCR   font = DgnFontManager::GetDefaultTrueTypeFont();
    if (SUCCESS != m_project->Fonts().AcquireFontNumber (m_data.ad4.dimfont, font))
        BeAssert (false);

    m_textStyle->SetPropertyValue(DgnTextStyleProperty::Font, &font);

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::Clear ()
    {
    m_initialized = false;
    m_project = NULL;
    m_elemID = INVALID_ELEMENTID;

    memset (&m_data, 0, sizeof(m_data));
    memset (&m_extensions, 0, sizeof(m_extensions));

    m_name.clear();
    m_description.clear();
    m_prefixCellName.clear();
    m_suffixCellName.clear();
    m_arrowCellName.clear();
    m_strokeCellName.clear();
    m_originCellName.clear();
    m_dotCellName.clear();
    m_noteCellName.clear();

    m_primary.m_masterUnitLabel.clear();
    m_primary.m_subUnitLabel.clear();
    m_secondary.m_masterUnitLabel.clear();
    m_secondary.m_subUnitLabel.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString DimensionStyle::StringFromElement (DgnElementCR elem, DimStringLinkageKey key)
    {
    WChar     value[MAX_LINKAGE_STRING_LENGTH];

    if (SUCCESS == LinkageUtil::ExtractNamedStringLinkageByIndex (value, MAX_LINKAGE_STRING_LENGTH, key, 0, &elem))
        return value;

    return L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::StringToElement (DgnElementR elem, WCharCP name, DimStringLinkageKey key) const
    {
    if (NULL == name || '\0' == *name)
        {
        LinkageUtil::DeleteStringLinkage (&elem, key, 0);
        return;
        }

    LinkageUtil::SetStringLinkage (&elem, key, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionStyle::ToElement (EditElementHandleR out) const
    {
    DgnV8ElementBlank       elem;

    /*-----------------------------------------------------------------------------------
        Element header
    -----------------------------------------------------------------------------------*/
    memset (&elem, 0, sizeof(DgnElementHeader));

    elem.SetLegacyType(TABLE_ENTRY_ELM);
    elem.SetLevel(MS_DIMSTYLE_TABLE_LEVEL);
    elem.SetDictionary(true);
    elem.SetSizeWordsNoAttributes(sizeof(DimStyleEntryElm)/2);
    elem.SetElementId(ElementId(m_elemID));

    /*-----------------------------------------------------------------------------------
        Fixed portion of the element
    -----------------------------------------------------------------------------------*/
    memcpy (&elem.ToDimStyleEntryElmR().data, &m_data, sizeof(elem).ToDimStyleEntryElm().data);
    elem.ToDimStyleEntryElmR().textStyleId = m_textStyle->GetId().GetValue();

    /*-----------------------------------------------------------------------------------
        Linkage data
    -----------------------------------------------------------------------------------*/
    StringToElement (elem,  m_name.c_str(),                         DIMSTYLE_Name);
    StringToElement (elem, m_description.c_str(),                   DIMSTYLE_Description);

    StringToElement (elem,  m_prefixCellName.c_str(),               DIMCELL_Prefix);
    StringToElement (elem,  m_suffixCellName.c_str(),               DIMCELL_Suffix);
    StringToElement (elem,  m_arrowCellName.c_str(),                DIMCELL_Arrow);
    StringToElement (elem,  m_strokeCellName.c_str(),               DIMCELL_Stroke);
    StringToElement (elem,  m_originCellName.c_str(),               DIMCELL_Origin);
    StringToElement (elem,  m_dotCellName.c_str(),                  DIMCELL_Dot);
    StringToElement (elem,  m_noteCellName.c_str(),                 DIMCELL_Note);

    StringToElement (elem,  m_primary.m_masterUnitLabel.c_str(),    DIMUNIT_PrimaryMaster);
    StringToElement (elem,  m_primary.m_subUnitLabel.c_str(),       DIMUNIT_PrimarySub);
    StringToElement (elem,  m_secondary.m_masterUnitLabel.c_str(),  DIMUNIT_SecondayMaster);
    StringToElement (elem,  m_secondary.m_subUnitLabel.c_str(),     DIMUNIT_SecondarySub);

    MSElementDescrP tDscr = new MSElementDescr(elem, *GetDgnProject()->Models().GetDictionaryModel());
    out.SetElementDescr(tDscr, false);

    mdlDim_setStyleExtension (out, &m_extensions);

    LegacyTextStyle textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*m_textStyle);
    ElementUtil::AppendTextStyleAsLinkage (out, textStyle, KEY_TEXTSTYLE_DIMSTYLE);

    // The textstyle override flags on the dimension style are meaningless and unused.
    // Why did they get turned on?  We will not read them back out: see DimensionStyle::FromElement().
    //BeAssert ( ! textStyle->overrideFlags.AreAnyFlagsSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::FromElement (ElementHandleCR styleElement)
    {
    Clear();

    if ( ! styleElement.IsValid())
        return;

    DgnElementCP elemCP = styleElement.GetElementCP();

    if ( ! dimStyleEntry_isStyleElement (elemCP))
        return;

    m_project =  styleElement.GetDgnProject();

    /*-----------------------------------------------------------------------------------
        Fixed portion of the element
    -----------------------------------------------------------------------------------*/
    m_elemID = elemCP->GetElementId().GetValue();
    DgnTextStylePtr fileStyle = styleElement.GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(elemCP->ToDimStyleEntryElm().textStyleId));
    if (fileStyle.IsValid())
        {
        m_textStyle->SetId (DgnStyleId(elemCP->ToDimStyleEntryElm().textStyleId));
        m_textStyle->SetName (fileStyle->GetName().c_str());
        }

    memcpy (&m_data, &elemCP->ToDimStyleEntryElm().data, sizeof(m_data));

    /*-----------------------------------------------------------------------------------
        Linkage data
    -----------------------------------------------------------------------------------*/
    mdlDim_getStyleExtension (&m_extensions, styleElement);

    LegacyTextStyle textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*m_textStyle);
    ElementUtil::ExtractTextStyleFromLinkage (textStyle, styleElement, KEY_TEXTSTYLE_DIMSTYLE);

    // The textstyle override flags on the dimension style are meaningless and unused.
    // Don't read them.
    memset(&textStyle.overrideFlags, 0, sizeof (textStyle.overrideFlags));

    m_name           = StringFromElement (*elemCP, DIMSTYLE_Name);
    m_description    = StringFromElement (*elemCP, DIMSTYLE_Description);
    m_prefixCellName = StringFromElement (*elemCP, DIMCELL_Prefix);
    m_suffixCellName = StringFromElement (*elemCP, DIMCELL_Suffix);
    m_arrowCellName  = StringFromElement (*elemCP, DIMCELL_Arrow);
    m_strokeCellName = StringFromElement (*elemCP, DIMCELL_Stroke);
    m_originCellName = StringFromElement (*elemCP, DIMCELL_Origin);
    m_dotCellName    = StringFromElement (*elemCP, DIMCELL_Dot);
    m_noteCellName   = StringFromElement (*elemCP, DIMCELL_Note);

    m_primary.m_masterUnitLabel   = StringFromElement (*elemCP, DIMUNIT_PrimaryMaster);
    m_primary.m_subUnitLabel      = StringFromElement (*elemCP, DIMUNIT_PrimarySub);
    m_secondary.m_masterUnitLabel = StringFromElement (*elemCP, DIMUNIT_SecondayMaster);
    m_secondary.m_subUnitLabel    = StringFromElement (*elemCP, DIMUNIT_SecondarySub);

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStyle::DimensionStyle (ElementHandleCR styleElement)
    {
    m_textStyle = DgnTextStyle::Create(styleElement.GetDgnModelP()->GetDgnProject());
    FromElement (styleElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr   DimensionStyle::CreateFromElement (ElementHandleCR styleElement)
    {
    if ( ! dimStyleEntry_isStyleElement (styleElement.GetElementCP()))
        return NULL;

    return new DimensionStyle (styleElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::ToDgnDimStyle (DgnDimStyleR dgnDimStyle, DgnModelP dgnCache) const
    {
    dgnDimStyle.initialized = false;

    if ( ! m_initialized)
        return;

    if (&dgnCache->GetDgnProject() != m_project)
        { BeAssert (false); return; }

    /*-----------------------------------------------------------------------------------
        Fixed portion of the object
    -----------------------------------------------------------------------------------*/
    dgnDimStyle.uniqueId    = m_elemID;
    dgnDimStyle.data        = m_data;
    dgnDimStyle.extensions  = m_extensions;

    dgnDimStyle.textStyleId = m_textStyle->GetId().GetValue();
    dgnDimStyle.textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*m_textStyle);

    /*-----------------------------------------------------------------------------------
        Strings
    -----------------------------------------------------------------------------------*/
    BeStringUtilities::Wcsncpy (dgnDimStyle.name,            m_name.c_str()            );
    BeStringUtilities::Wcsncpy (dgnDimStyle.description,     m_description.c_str()     );
    BeStringUtilities::Wcsncpy (dgnDimStyle.prefixCellName,  m_prefixCellName.c_str()  );
    BeStringUtilities::Wcsncpy (dgnDimStyle.suffixCellName,  m_suffixCellName.c_str()  );
    BeStringUtilities::Wcsncpy (dgnDimStyle.arrowCellName,   m_arrowCellName.c_str()   );
    BeStringUtilities::Wcsncpy (dgnDimStyle.strokeCellName,  m_strokeCellName.c_str()  );
    BeStringUtilities::Wcsncpy (dgnDimStyle.originCellName,  m_originCellName.c_str()  );
    BeStringUtilities::Wcsncpy (dgnDimStyle.dotCellName,     m_dotCellName.c_str()     );
    BeStringUtilities::Wcsncpy (dgnDimStyle.noteCellName,    m_noteCellName.c_str()    );

    BeStringUtilities::Wcsncpy (dgnDimStyle.primary.masterUnitLabel,   m_primary.m_masterUnitLabel.c_str()  );
    BeStringUtilities::Wcsncpy (dgnDimStyle.primary.subUnitLabel,      m_primary.m_subUnitLabel.c_str()     );
    BeStringUtilities::Wcsncpy (dgnDimStyle.secondary.masterUnitLabel, m_secondary.m_masterUnitLabel.c_str());
    BeStringUtilities::Wcsncpy (dgnDimStyle.secondary.subUnitLabel,    m_secondary.m_subUnitLabel.c_str()   );

    dgnDimStyle.initialized = true;

    /*-----------------------------------------------------------------------------------
        In a DgnDimStyle, distances are stored in uors defined by the dgnCache
        member.  For the DimensionStyle object we always use uors defined by the
        default model.
    -----------------------------------------------------------------------------------*/
    DgnModelP defaultCache = m_project->Models().GetModelById (m_project->Models().GetFirstModelId());
    dgnDimStyle_scaleBetweenModels (&dgnDimStyle, defaultCache, dgnCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::FromDgnDimStyle (DgnDimStyleCR dgnDimStyleIn)
    {
    Clear();

    if ( ! dgnDimStyleIn.initialized)
        return;

    m_project = &dgnDimStyleIn.dgnCache->GetDgnProject();

    /*-----------------------------------------------------------------------------------
        In a DgnDimStyle, distances are stored in uors defined by the dgnCache
        member.  For the DimensionStyle object we always use uors defined by the
        default model.
    -----------------------------------------------------------------------------------*/
    DgnDimStyle dgnDimStyle = dgnDimStyleIn;

    DgnModelP defaultCache = m_project->Models().GetModelById(m_project->Models().GetFirstModelId());
    dgnDimStyle_scaleBetweenModels (&dgnDimStyle, dgnDimStyleIn.dgnCache, defaultCache);

    /*-----------------------------------------------------------------------------------
        Fixed portion of the object
    -----------------------------------------------------------------------------------*/
    m_elemID      = dgnDimStyle.uniqueId;
    m_data        = dgnDimStyle.data;
    m_extensions  = dgnDimStyle.extensions;


    DgnTextStylePtr fileStyle = GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(dgnDimStyle.textStyleId));
    if (fileStyle.IsValid())
        {
        m_textStyle->SetName (fileStyle->GetName().c_str());
        m_textStyle->SetId (fileStyle->GetId());
        }

    DgnTextStylePersistence::Legacy::FromLegacyStyle(*m_textStyle, dgnDimStyle.textStyle);

    /*-----------------------------------------------------------------------------------
        Strings
    -----------------------------------------------------------------------------------*/
    m_name.assign           (dgnDimStyle.name);
    m_description.assign    (dgnDimStyle.description);
    m_prefixCellName.assign (dgnDimStyle.prefixCellName);
    m_suffixCellName.assign (dgnDimStyle.suffixCellName);
    m_arrowCellName.assign  (dgnDimStyle.arrowCellName);
    m_strokeCellName.assign (dgnDimStyle.strokeCellName);
    m_originCellName.assign (dgnDimStyle.originCellName);
    m_dotCellName.assign    (dgnDimStyle.dotCellName);
    m_noteCellName.assign   (dgnDimStyle.noteCellName);

    m_primary.m_masterUnitLabel.assign   (dgnDimStyle.primary.masterUnitLabel);
    m_primary.m_subUnitLabel.assign      (dgnDimStyle.primary.subUnitLabel);
    m_secondary.m_masterUnitLabel.assign (dgnDimStyle.secondary.masterUnitLabel);
    m_secondary.m_subUnitLabel.assign    (dgnDimStyle.secondary.subUnitLabel);

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStyle::DimensionStyle (DgnDimStyleCR dgnDimStyle)
    {
    AssignTextStyle(*DgnTextStyle::Create(dgnDimStyle.dgnCache->GetDgnProject()));
    FromDgnDimStyle (dgnDimStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionStyle::ProcessProperties (PropertyContextR proc, bool canChange)
    {
    bool        changed = false;

    // Colors
    changed |= proc.DoColorCallback (&m_data.ad4.dim_color,      EachColorArg (m_data.ad4.dim_color,      SETPROPCBEIFLAG(m_data.ad4.ext_dimflg.colorOverride), proc));
    changed |= proc.DoColorCallback (&m_data.ad5.altSymb.color,  EachColorArg (m_data.ad5.altSymb.color,  SETPROPCBEIFLAG(m_data.ad5.flags.altColor), proc));
    changed |= proc.DoColorCallback (&m_data.ad5.termSymb.color, EachColorArg (m_data.ad5.termSymb.color, SETPROPCBEIFLAG(m_data.ad5.flags.termColor), proc));

    // Weights
    changed |= proc.DoWeightCallback (&m_data.ad4.dim_weight,      EachWeightArg (m_data.ad4.dim_weight,      SETPROPCBEIFLAG(m_data.ad4.ext_dimflg.weightOverride), proc));
    changed |= proc.DoWeightCallback (&m_data.ad5.altSymb.weight,  EachWeightArg (m_data.ad5.altSymb.weight,  SETPROPCBEIFLAG(m_data.ad5.flags.altWeight), proc));
    changed |= proc.DoWeightCallback (&m_data.ad5.termSymb.weight, EachWeightArg (m_data.ad5.termSymb.weight, SETPROPCBEIFLAG(m_data.ad5.flags.termWeight), proc));

    // LineStyles
    changed |= proc.DoLineStyleCallback (&m_data.ad4.dim_style,      EachLineStyleArg (m_data.ad4.dim_style,      NULL, SETPROPCBEIFLAG(m_data.ad4.ext_dimflg.styleOverride), proc));
    changed |= proc.DoLineStyleCallback (&m_data.ad5.altSymb.style,  EachLineStyleArg (m_data.ad5.altSymb.style,  NULL, SETPROPCBEIFLAG(m_data.ad5.flags.altStyle), proc));
    changed |= proc.DoLineStyleCallback (&m_data.ad5.termSymb.style, EachLineStyleArg (m_data.ad5.termSymb.style, NULL, SETPROPCBEIFLAG(m_data.ad5.flags.termStyle), proc));

    // Stored in TextStyle - Color, Font, LineStyle
    DgnTextStylePtr     textStyle;

    if (SUCCESS == GetTextStyleProp (textStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE))
        {
        /*----------------------------------------------------------------------
         TR#230774 tricky!

           ad4.ext_dimflg.textColorOverride is a m_property of the dimstyle
           textStyle.flags.color            is a m_property of the textstyle

           textStyle.color is a valid color if either above flag is true
        ----------------------------------------------------------------------*/
        bool oldFlag;
        textStyle->GetPropertyValue (DgnTextStyleProperty::HasColor, oldFlag);

        bool newFlag = oldFlag | m_data.ad4.ext_dimflg.textColorOverride;
        textStyle->SetPropertyValue (DgnTextStyleProperty::HasColor, newFlag);

        // Let the textstyle handler take care of it.
        if (textStyle->ProcessProperties (proc, canChange))
            {
            textStyle->SetPropertyValue (DgnTextStyleProperty::HasColor, oldFlag);

            SetTextStyleProp (*textStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE);
            changed = true;
            }
        }

    // TextStyle
    EachTextStyleArg    textStyleArg (m_textStyle->GetId().GetValue(), PROPSCALLBACK_FLAGS_NoFlagsSet, proc);
    UInt32              newTextStyleID = m_textStyle->GetId().GetValue();

    if (proc.DoTextStyleCallback (&newTextStyleID, textStyleArg))
        {
        DgnTextStylePtr fileStyle = GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(newTextStyleID));
        if (fileStyle.IsValid())
            {
            m_textStyle->SetName (fileStyle->GetName().c_str());
            m_textStyle->SetId (fileStyle->GetId());
            }
        changed = true;
        }

    StyleParamsRemapping    paramsRemapping = textStyleArg.GetRemappingAction ();

    /*--------------------------------------------------------------------------
      The element potentially been assigned to a new style.  This change may
      have an impact on the parameters of elmP that are derived from the style.

         *** This step should be done after all ids are remapped ***
    --------------------------------------------------------------------------*/
    switch (paramsRemapping)
        {
        case StyleParamsRemapping::Override:
            {
            //  If any of the element's properties don't match the style, set up overrides for them.
            // NEEDSWORK

            break;
            }
        case StyleParamsRemapping::ApplyStyle:
            {
            if (0 == m_textStyle->GetId().GetValue())
                break;

            bool            textStyleChange = false;
            DgnModelP    destDgnModel = proc.GetDestinationDgnModel();

            DgnTextStylePtr fileTextStylePtr = destDgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyle->GetId().GetValue()));

            if ( ! fileTextStylePtr.IsValid())
                { BeAssert(0); break; }

            UpdateTextStyleParams (&textStyleChange, *fileTextStylePtr);

            if (textStyleChange)
                changed = true;

            break;
            }
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::ConvertToFile (DgnProjectR dgnFile)
    {
    // NEEDSWORK
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::_SetName (WCharCP newName)
    {
    if (NULL == newName)
        { BeAssert (false); return ERROR; }

    if (SUCCESS != SetStringProp (newName, DIMSTYLE_PROP_General_DimStyleName_MSWCHAR))
        { BeAssert(0); return ERROR; }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR       DimensionStyle::GetName ()        const { return m_name; }
UInt64          DimensionStyle::GetID ()          const { return m_elemID; }
DgnProjectP     DimensionStyle::GetDgnProject() const { return m_project; }
UInt32          DimensionStyle::GetTextStyleId () const { return m_textStyle->GetId().GetValue(); }

void            DimensionStyle::SetElemID (UInt64 elementId)  { m_elemID = elementId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionStyle::IsPropertyLocallyControlled (DimStyleProp prop) const
    {
    bool            inverted;
    DimStyleProp    overrideProp;

    if (SUCCESS != GetOverrideProp (&overrideProp, &inverted, prop))
        return true;

    bool    overrideValue;

    if (SUCCESS != GetBooleanProp (overrideValue, overrideProp))
        { BeAssert (false); return true; }

    return inverted ? ! overrideValue : overrideValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::InitDimText (DimText& dimText, ElementHandleCR eh, int pointNo) const
    {
    adim_initDimText (eh, pointNo, &dimText, *this);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct GetDimStyleTableScanner : DgnTableElementScanner::ScanCallback
{
ElementHandle  m_foundTable;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OnTableElement
(
ElementHandleCR        tableEh,                    /* => input potential table element descriptor for which to build descriptor chain */
ScanCriteria        *scP
)
    {
    /*----------------------------------------------------------------------------------
      Scan the entire dictionary model, and only keep the dimstyle table element with
      the largest (ie. most recent) lastModified time.

      If as a result of a bug, the file contains multiple tables, this procedure will
      guarantee that we always find the same table that the user is working with.
    ----------------------------------------------------------------------------------*/
    if ( ! m_foundTable.IsValid())
        {
        m_foundTable = tableEh;
        return SUCCESS;
        }

    if (tableEh.GetElementCP()->ehdr.lastModified > m_foundTable.GetElementCP()->ehdr.lastModified)
        m_foundTable = tableEh;

    return SUCCESS;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AbeeshBasheer    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      DimensionStyle::CreateTableElement (DgnProjectR file)
    {
    MSElementDescrP tableDescr  = NULL;
    if (SUCCESS != DgnTableUtilities::CreateTable (&tableDescr,  MS_DIMSTYLE_TABLE_LEVEL, sizeof (DimStyleTableElm), 0))
        return ElementHandle();

    EditElementHandle tableElement (tableDescr, true, false, &file.GetDictionaryModel());
    // Create the new settings element in the context of the table's file
    DimStyleTableEditor tableEditor(tableElement);
    if (SUCCESS != tableEditor.AddEntryElement (DimensionStyle (L"", file)))
        return ElementHandle();

    DgnTableUtilities::SetEntryId (tableElement.GetElementDescrP()->h.firstElem->el, DIMSTYLE_ACTIVE_STYLE_ID);
    return tableElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   DimensionStyle::FetchTableElement (DgnProjectR file)
    {
    ElementHandle tableElement = GetTableElement (file);
    if (tableElement.IsValid())
        return tableElement;

    return CreateTableElement (file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle   DimensionStyle::GetTableElement (DgnProjectR file)
    {
    GetDimStyleTableScanner getTableScanner;

    DgnTableElementScanner::ScanTableElements (getTableScanner, file.GetDictionaryModel(), NULL, MS_DIMSTYLE_TABLE_LEVEL);
    return getTableScanner.m_foundTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      DimensionStyle::GetStyleElementByName (WCharCP name, ElementHandleCR table)
    {
    if (!table.IsValid() || NULL == name|| 0 == wcslen(name))
        {return ElementHandle (); }

    for (ChildElemIter iter (table, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (DimStyleEntryHandler::IsSettings (iter))
            continue;

        WString     elemName = DimStyleEntryHandler::GetName(*iter.GetElementCP());

        if (0 == BeStringUtilities::Wcsicmp (name, elemName.c_str()))
            return iter;
        }

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      DimensionStyle::GetStyleElementByName (WCharCP name, DgnProjectR file)
    {
    if (NULL == name)
        { BeAssert (false); return ElementHandle (); }

    ElementHandle  table = GetTableElement (file);

    return GetStyleElementByName (name, table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AbeeshBasheer    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      DimensionStyle::GetSettingsElement (ElementHandleCR tableElem)
    {
    if (!tableElem.IsValid())
        return ElementHandle();

    for (ChildElemIter iter (tableElem, ExposeChildrenReason::Count); iter.IsValid (); iter = iter.ToNext ())
        {
        if (DimStyleEntryHandler::IsSettings (iter))
            return iter;
        }

    return ElementHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle      DimensionStyle::GetSettingsElementByFile (DgnProjectR file)
    {
    ElementHandle  tableElem = GetTableElement (file);
    return GetSettingsElement(tableElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::Replace (WCharCP oldName, DgnProjectP destFile)
    {
    if (NULL == destFile)
        destFile = GetFile();

    // Get the name of the style to replace
    WCharCP       nameToReplace = (NULL != oldName) ? oldName : GetName().c_str();

    if (NULL == nameToReplace || 0 >= wcslen (nameToReplace))
        return ERROR;

    if (oldName && (0 != GetName().CompareToI(oldName)))
        {
        DimensionStylePtr existingNamedStyle = GetByName (GetName().c_str(), *destFile);
        if (existingNamedStyle.IsValid())
            return ERROR;
        }

    // Find table element
    EditElementHandle  tableElm (FetchTableElement (*destFile), true);

    ElementRefP oldElemRef = tableElm.GetElementRef ();

    // Replace the entry element, possibly cloning into the destination file.
    DimStyleTableEditor editor (tableElm);
    if (SUCCESS != editor.ReplaceEntryElement (*this, nameToReplace))
        return ERROR;

    // Rewrite the whole table
    StatusInt status = (NULL != oldElemRef) ? tableElm.ReplaceInModel(oldElemRef) : tableElm.AddToModel();
    if (SUCCESS != status)
        return (BentleyStatus)status;

    // Replace the input style with the contents of the table
    ElementHandle newElm = GetStyleElementByName (GetName().c_str(), tableElm);
    FromElement (newElm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::Add (DgnProjectP destFile)
    {
    if (GetName().empty())
        return ERROR;

    if (NULL == destFile)
        destFile = GetFile();

    // Find destination table element
    EditElementHandle  tableElm (FetchTableElement (*destFile), true);

    ElementRefP oldElemRef = tableElm.GetElementRef ();
    // Add the entry element, possibly cloning into the destination file.
    DimStyleTableEditor editor (tableElm);
    if (SUCCESS != editor.AddEntryElement (*this))
        return ERROR;

    // Rewrite the whole table
    StatusInt status = (NULL != oldElemRef) ? tableElm.ReplaceInModel(oldElemRef) : tableElm.AddToModel();
    if (SUCCESS != status)
        return (BentleyStatus)status;

    // Replace the input style with the contents of the table
    ElementHandle newElm = GetStyleElementByName (GetName().c_str(), tableElm);
    FromElement (newElm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::Delete (WCharCP name, DgnProjectR dgnFile)
    {
    if (NULL == name)
        { BeAssert (false); return ERROR; }

    // Check if it's safe to delete
    DimensionStylePtr fileStyle = GetByName (name, dgnFile);
    if (fileStyle.IsNull ())
        return ERROR;

    if (fileStyle->HasDependants())
        return ERROR;

    // Find table element
    EditElementHandle  tableElm (GetTableElement (dgnFile), true);
    if (!tableElm.IsValid())
        return ERROR;

    // Delete the entry element
    DimStyleTableEditor editor (tableElm);
    if (SUCCESS != editor.RemoveEntryElement (name))
        return ERROR;

    // Rewrite the whole table
    return (SUCCESS == tableElm.ReplaceInModel(tableElm.GetElementRef ())) ? SUCCESS : ERROR;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionStyleIsUsedQuery : IQueryProperties
    {
    bool        m_found;
    UInt64   m_styleElemID;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    08/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    DimensionStyleIsUsedQuery (UInt64 styleElemID)
        :
        m_found (false),
        m_styleElemID (styleElemID)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    08/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementProperties    _GetQueryPropertiesMask () override
        {
        return ELEMENT_PROPERTY_DimStyle;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    08/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachDimStyleCallback        (EachDimStyleArg& arg) override
        {
        if (arg.GetStoredValue() == m_styleElemID)
            m_found = true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionStyle::IsDimStyleUsed (UInt64 elemID, DgnProjectR dgnFile)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    if (0 == elemID)
        return false;

    DimensionStyleIsUsedQuery   queryObj (elemID);

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
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            DimensionStyle::HasDependants () const
//    {
//    return IsDimStyleUsed (GetID(), *GetFile());
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    JoshSchifter    10/13
//+---------------+---------------+---------------+---------------+---------------+------*/
//BentleyStatus DimensionStyle::RemapDependents (WCharCP destName, WCharCP srcName, DgnProjectR file)
//    {
//    DimensionStylePtr srcStyle  = GetByName (srcName, file);
//    DimensionStylePtr destStyle = GetByName (destName, file);
//
//    if (srcStyle.IsNull() || destStyle.IsNull())
//        return ERROR;
//
//    StyleDependantRemapper remapper;
//
//    remapper.AddDimStyleRemap (srcStyle->GetID(), destStyle->GetID());
//    remapper.DoRemapping (file);
//
//    return SUCCESS;
//    }

static DgnHost::Key s_transactionListenerKey;

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::OnDimStyleTransactionEvent (ElementHandleCP entryBefore, ElementHandleCP entryAfter, StyleEventType eventType, StyleEventSource source)
    {
    IDimStyleTransactionListener* listener = (IDimStyleTransactionListener*) T_HOST.GetHostVariable (s_transactionListenerKey);
    if (NULL == listener)
        return;

    DimensionStylePtr styleBefore;
    DimensionStylePtr styleAfter;

    if (entryBefore)
        styleBefore = new DimensionStyle (*entryBefore);

    if (entryAfter)
        styleAfter = new DimensionStyle (*entryAfter);

    listener->_OnDimStyleChange (styleBefore.get(), styleAfter.get(), eventType, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::SetTransactionListener (IDimStyleTransactionListener* obj)
    {
    void* oldListener = T_HOST.GetHostVariable (s_transactionListenerKey);
    BeAssert ((NULL != obj && NULL == oldListener) || (NULL == obj && NULL != oldListener));
    T_HOST.SetHostVariable (s_transactionListenerKey, obj);
    }
#endif

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dgnDimStyle_dump
(
const DgnDimStyleP  dgnDimStyleP,
const char          *pFileNameIn
)
    {
    FILE    *pFile = NULL;

    if (NULL == pFileNameIn || strlen (pFileNameIn) <= 0 || NULL == (pFile = fopen (pFileNameIn, "w")))
        pFile = stdout;

    if (NULL == dgnDimStyleP)
        {
        fprintf (pFile, "========== Start of DgnDimStyleP Dump ==========\n");
        fprintf (pFile, "* Invalid DgnDimStyleP!                        *\n");
        fprintf (pFile, "========== End of DgnDimStyleP Dump ============\n");
        }
    else
        {
        fprintf (pFile, "========== Start of DgnDimStyleP Dump ==========\n");
        fprintf (pFile, "* Name: \'%ls\'\tDescription: \'%ls\'\n", dgnDimStyleP->name, dgnDimStyleP->description);
        fprintf (pFile, "* UniqueID: %lld\n", dgnDimStyleP->uniqueId);
        fprintf (pFile, "========== End of DgnDimStyleP Dump ============\n");
        }

    if (stdout != pFile)
        fclose (pFile);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     dgnDimStyle_init
(
DgnDimStyleP    dgnDimStyleP
)
    {
    if (NULL == dgnDimStyleP)
        return  ;

    memset (dgnDimStyleP, 0, sizeof (*dgnDimStyleP));
    dgnDimStyleP->initialized = true;
    dgnDimStyleP->bUsingAnnotationScale = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        dgnDimStyle_applyUorScale
(
DimStyleSettings   *dsP,
double              uorScale
)
    {
    dsP->ad2.txheight       *= uorScale;
    dsP->ad4.stack_offset   *= uorScale;
    dsP->ad4.dimcen         *= uorScale;
    dsP->ad5.textWidth      *= uorScale;
    dsP->ad2.uptol          *= uorScale;
    dsP->ad2.lowtol         *= uorScale;

    /* these are uor values (stupidly) stored as ULongs */
    dsP->ad6.altFormat.threshold        = (UInt32) (dsP->ad6.altFormat.threshold * uorScale);
    dsP->ad6.secAltFormat.threshold     = (UInt32) (dsP->ad6.secAltFormat.threshold * uorScale);

#if defined (NEEDSWORK_POSTV8_DIMSTYLE)
// Eventually, the roundoff (and possibly other) values in the style extension linkage
// will need to be scaled.  At that time, we will need a full DgnDimStyleP.
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        dgnDimStyle_scaleBetweenModels
(
DgnDimStyleP    dgnDimStyleP,
DgnModelP       srcCache,
DgnModelP       dstCache
)
    {
    StatusInt   status = ERROR;

    if (dgnDimStyleP->initialized)
        {
#if defined (REMOVE_DEAD_CODE)
    double      uorScale;
        if (SUCCESS == modelInfo_getUorScaleBetweenModels (&uorScale, srcCache, dstCache))
            dgnDimStyle_applyUorScale (&dgnDimStyleP->data, uorScale);
#endif

        bool        bHadAnnotationScale = dgnDimStyleP->bUsingAnnotationScale;

        if (bHadAnnotationScale)
            mdlDimStyle_unapplyAnnotationScaleToScaledTextSize (dgnDimStyleP);

        dgnDimStyleP->dgnCache = dstCache;

        if (bHadAnnotationScale)
            mdlDimStyle_applyAnnotationScaleToUnscaledTextSize (dgnDimStyleP);

        status = SUCCESS;
        }

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/04
+---------------+---------------+---------------+---------------+---------------+------*/
double  BentleyApi::mdlDimStyle_getEffectiveAnnotationScale
(
DgnDimStyleCP        dgnDimStyleP
)
    {
//WIP_BEIJING_DIMENSION_AB
// All this annotation scale stuff moves up to ustation, once v7 code conversion is with
//The new dimension style structure
    double      scale = 1.0;
    bool     useStyleScale;

    DimensionStylePtr style = DimensionStyle::CreateFromDgnDimStyle (*dgnDimStyleP);
    style->GetBooleanProp (useStyleScale, DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT);

    if (useStyleScale)
        style->GetDoubleProp (scale, DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE);
    else
#ifdef WIP_V10_MODELINFO
        scale = dgnModel_getEffectiveAnnotationScale (dgnDimStyleP->dgnCache);
#else
        scale = 1.0;
#endif

    if (fabs (scale - 0.0) < mgds_fc_epsilon)
        scale = 1.0;

    return scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void            scaleDimStyleDistances
(
DgnDimStyleP            dgnDimStyleP,
double                  scale
)
    {
    dgnDimStyleP->data.ad2.txheight     *= scale;
    dgnDimStyleP->data.ad5.textWidth    *= scale;
    dgnDimStyleP->data.ad4.dimcen       *= scale;
    dgnDimStyleP->data.ad4.stack_offset *= scale;

    dgnDimStyleP->textStyle.Scale(scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimStyle_applyAnnotationScaleToUnscaledTextSize
(
DgnDimStyleP            dgnDimStyleP
)
    {
    double      scale = 1.0;

    // don't apply twice
    if (dgnDimStyleP->bUsingAnnotationScale)
        return SUCCESS;

    // The functions mdlDimStyle_applyAnnotationScaleToUnscaledTextSize and
    // mdlDimStyle_unapplyAnnotationScaleToScaledTextSize are only used when
    // getting/restoring the active dimstyle from disk or when saving active
    // dimstyle to disk. This is because the in-memory active dimstyle is
    // expected to have it's textheight and textwidth scaled by the effective
    // annotation scale, which can come from the model or be overridden in
    // the dimstyle.

    scale = BentleyApi::mdlDimStyle_getEffectiveAnnotationScale (dgnDimStyleP);
    scaleDimStyleDistances (dgnDimStyleP, scale);
    dgnDimStyleP->bUsingAnnotationScale = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDimStyle_unapplyAnnotationScaleToScaledTextSize
(
DgnDimStyleP            dgnDimStyleP
)
    {
    double      scale = 1.0;

    // don't unapply twice
    if ( ! dgnDimStyleP->bUsingAnnotationScale)
        return SUCCESS;

    // The functions mdlDimStyle_applyAnnotationScaleToUnscaledTextSize and
    // mdlDimStyle_unapplyAnnotationScaleToScaledTextSize are only used when
    // getting/restoring the active dimstyle from disk or when saving active
    // dimstyle to disk. This is because the in-memory active dimstyle is
    // expected to have it's textheight and textwidth scaled by the effective
    // annotation scale, which can come from the model or be overridden in
    // the dimstyle.

    scale = BentleyApi::mdlDimStyle_getEffectiveAnnotationScale (dgnDimStyleP);
    scaleDimStyleDistances (dgnDimStyleP, 1.0 / scale);
    dgnDimStyleP->bUsingAnnotationScale = false;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    unitDefFromDimUnits
(
UnitDefinitionR     unitDef,
DimUnits const&     dimUnit,
WCharCP           label
)
    {
    unitDef.Init (UnitDefinition::BaseFromInt   (dimUnit.flags.base),
                  UnitDefinition::SystemFromInt (dimUnit.flags.system),
                  dimUnit.numerator,
                  dimUnit.denominator,
                  label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimUnitsFromUnitDef
(
DimUnits&           dimUnits,
UnitDefinitionCR    unitDef
)
    {
    memset (&dimUnits, 0, sizeof(dimUnits));
    dimUnits.flags.base    = static_cast<UInt32>(unitDef.GetBase());
    dimUnits.flags.system  = static_cast<UInt32>(unitDef.GetSystem());
    dimUnits.numerator     = unitDef.GetNumerator();
    dimUnits.denominator   = unitDef.GetDenominator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::SetTextStyleByID (UInt32 textStyleID)
    {
    DgnTextStylePtr textStyle;

    if (0 == textStyleID)
        textStyle = DgnTextStyle::Create(*GetDgnProject());
    else
        textStyle = GetDgnProject()->Styles().TextStyles().QueryById(DgnStyleId(textStyleID));

    if ( ! textStyle.IsValid())
        return ERROR;

    UpdateTextStyleParams (NULL, *textStyle);
    m_textStyle->SetId (DgnStyleId(textStyleID));
    m_textStyle->SetName (textStyle->GetName().c_str());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetAccuracyProp (byte& value, DimStyleProp iProperty) const
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_Accuracy_ACCURACY:
            {
            value = m_data.ad1.primaryAccuracy;
            break;
            }
        case DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY:
            {
            value = m_data.ad1.secondaryAccuracy;
            break;
            }
        case DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY:
            {
            value = m_data.ad6.altFormat.accuracy;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY:
            {
            value = m_data.ad6.secAltFormat.accuracy;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY:
            {
            value = (byte) m_extensions.primaryTolAcc;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY:
            {
            value = (byte) m_extensions.secondaryTolAcc;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetAccuracyProp
(
byte            value,
DimStyleProp    iProperty
)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_Accuracy_ACCURACY:
            {
            m_data.ad1.primaryAccuracy = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY:
            {
            m_data.ad1.secondaryAccuracy = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY:
            {
            m_data.ad6.altFormat.accuracy = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY:
            {
            m_data.ad6.secAltFormat.accuracy = value;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY:
            {
            m_extensions.primaryTolAcc = value;

            if (DIMACC_INVALID != value)
                m_extensions.modifiers |= STYLE_Extension_PrimaryToleranceAccuracy;
            else
                m_extensions.modifiers &= ~STYLE_Extension_PrimaryToleranceAccuracy;


            break;
            }
        case DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY:
            {
            m_extensions.secondaryTolAcc = value;

            if (DIMACC_INVALID != value)
                m_extensions.modifiers |= STYLE_Extension_SecondaryToleranceAccuracy;
            else
                m_extensions.modifiers &= ~STYLE_Extension_SecondaryToleranceAccuracy;

            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetBooleanProp (bool& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_IsActive_BOOLINT:
            {
            value = m_data.ad6.bnc.flags.active;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT:
            {
            value = m_data.ad6.bnc.flags.elbow;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT:
            {
            value = m_data.ad6.bnc.flags.noDockOnDimLine;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.joiner;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT:
            {
            value = m_data.ad5.flags.altColor;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT:
            {
            value = m_data.ad5.flags.altStyle;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT:
            {
            value = m_data.ad5.flags.altWeight;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT:
            {
            value = m_data.ad1.mode.witness;
            break;
            }
        case DIMSTYLE_PROP_General_IgnoreLevelSymbology_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.noLevelSymb;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideColor_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.colorOverride;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.styleOverride;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideWeight_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.weightOverride;
            break;
            }
        case DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.relDimLine;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT:
            {
            value = m_data.ad5.flags.multiLeader;
            break;
            }
        case DIMSTYLE_PROP_Placement_CompatibleV3_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.compatible;
            break;
            }
        case DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.levelOverride;
            break;
            }
        case DIMSTYLE_PROP_Placement_UseReferenceScale_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad4.ext_dimflg.useRefUnits;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT:
            {
            value = m_data.ad6.flags.noLineThruArrowTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT:
            {
            value = m_data.ad6.flags.noLineThruDotTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT:
            {
            value = m_data.ad6.flags.noLineThruOriginTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT:
            {
            value = m_data.ad6.flags.noLineThruStrokeTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruArrow_BOOLINT:
            {
            value = !m_data.ad6.flags.noLineThruArrowTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruDot_BOOLINT:
            {
            value = !m_data.ad6.flags.noLineThruDotTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruOrigin_BOOLINT:
            {
            value = !m_data.ad6.flags.noLineThruOriginTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruStroke_BOOLINT:
            {
            value = !m_data.ad6.flags.noLineThruStrokeTerm;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT:
            {
            value = m_data.ad5.flags.termColor;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT:
            {
            value = m_data.ad5.flags.termStyle;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT:
            {
            value = m_data.ad5.flags.termWeight;
            break;
            }
        case DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT:
            {
            value = m_data.ad5.flags.uniformCellScale;
            break;
            }
        case DIMSTYLE_PROP_Text_AutoLift_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad4.ext_dimflg.noAutoTextLift;
            break;
            }
        case DIMSTYLE_PROP_Text_Box_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.boxtext;
            break;
            }
        case DIMSTYLE_PROP_Text_Capsule_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.captext;
            break;
            }
        case DIMSTYLE_PROP_Text_DecimalComma_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.decimal_comma;
            break;
            }
        case DIMSTYLE_PROP_Text_Embed_BOOLINT:
            {
            value = m_data.ad1.params.embed;
            // there is no way of telling the caller to look at the TextLocation flag instead.
            break;
            }
        case DIMSTYLE_PROP_Text_Font_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.fontOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_Horizontal_BOOLINT:
            {
            value = m_data.ad1.params.horizontal;
            break;
            }
        case DIMSTYLE_PROP_Text_LeadingZero_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.leading_zero;
            break;
            }
        case DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.omitLeadingDelimeter;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideColor_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.textColorOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.textSizeOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.textWeightOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT:
            {
            value = m_data.ad5.flags.useWidth;
            break;
            }
        case DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.leading_zero2;
            break;
            }
        case DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT:
            {
            /* These are redundant */
            value = m_data.ad1.params.dual && m_data.ad5.flags.useSecUnits;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractions_BOOLINT:
            {
            value = m_data.ad5.flags.useStackedFractions;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideStackedFractions_BOOLINT:
            {
            value = m_data.ad6.flags.fractionOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT:
            {
            value = m_data.ad6.flags.underlineOverride;
            break;
            }
        case DIMSTYLE_PROP_Text_Underline_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.underlineText;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Mode_BOOLINT:
            {
            value = m_data.ad1.params.tolmode;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Show_BOOLINT:
            {
            value = m_data.ad1.params.tolerance;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT:
            {
            value = m_data.ad1.params.tolStackEqual;
            break;
            }
        case DIMSTYLE_PROP_Value_AltIsActive_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.useAltFmt;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.useAltFmt;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.adp_delimiter;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad6.secAltFormat.flags.adp_nomastunits;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.adp_subunits;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.adp_label;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.equalToThreshold;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.lessThanThreshold;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT:
            {
            value = m_data.ad6.secAltFormat.flags.adp_allowZeroMast;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad6.secAltFormat.flags.adp_hideZeroSub;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.adp_delimiter;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad6.altFormat.flags.adp_nomastunits;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.adp_subunits;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.adp_label;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.equalToThreshold;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.lessThanThreshold;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT:
            {
            value = m_data.ad6.altFormat.flags.adp_allowZeroMast;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad6.altFormat.flags.adp_hideZeroSub;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT:
            {
            value = m_data.ad4.ext_styleflg.angleLeadingZero;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT:
            {
            value = m_data.ad1.mode.labang;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT:
            {
            value = m_data.ad4.ext_styleflg.angleTrailingZero;
            break;
            }
        case DIMSTYLE_PROP_Value_RoundLSD_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.roundLSD;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT:
            {
            value = m_data.ad1.format.adp_delimiter2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT:
            {
            value = !m_data.ad1.format.adp_nomastunits2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT:
            {
            value = m_data.ad1.format.adp_subunits2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.trailing_zeros2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT:
            {
            value = m_data.ad1.format.adp_label2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT:
            {
            value = m_data.ad1.params.adp_allowZeroMast2;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad1.params.adp_hideZeroSub2;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT:
            {
            value = m_data.ad1.format.adp_delimiter;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad1.format.adp_nomastunits;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT:
            {
            value = m_data.ad1.format.adp_subunits;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.trailing_zeros;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT:
            {
            value = m_data.ad1.format.adp_label;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT:
            {
            value = m_data.ad1.params.adp_allowZeroMast;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad1.params.adp_hideZeroSub;
            break;
            }
        case DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT:
            {
            value = m_data.ad4.ext_dimflg.superscriptLSD;
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT:
            {
            value = m_data.ad5.flags.thousandSep;
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT:
            {
            value = m_data.ad5.flags.metricSpc;
            break;
            }
        case DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT:
            {
            // internal value reversed
            value = !m_data.ad1.params.overrideWorkingUnits;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT:
            {
            value = m_extensions.flags.uOrdUseDatumValue;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT:
            {
            value = m_extensions.flags.uOrdDecrementReverse;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT:
            {
            value = m_extensions.flags2.uOrdFreeLocation;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT:
            {
            value = m_extensions.flags.labelLineSupressAngle;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT:
            {
            value = m_extensions.flags.labelLineSupressLength;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT:
            {
            value = m_extensions.flags.labelLineInvertLabels;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT:
            {
            value = m_extensions.flags.uLabelLineAdjacentLabels;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceFraction;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceAltFraction;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceTolFraction;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceSecFraction;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceAltSecFraction;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT:
            {
            value = m_extensions.flags.uNoReduceTolSecFraction;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT:
            {
            value = m_extensions.flags2.uNoteLeaderType;
            break;
            }
        case DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT:
            {
            value = m_extensions.flags2.uNotUseModelAnnotationScale;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT:
            {
            value = m_extensions.flags2.uNoteScaleFrame;
            break;
            }
        case DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT:
            {
            value = m_extensions.flags3.uNoTermsOutside;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT:
            {
            value = m_extensions.flags3.uUseBncElbowLength;
            break;
            }
        case DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT:
            {
            value = m_extensions.flags3.uTightFitTextAbove;
            break;
            }
        case DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT:
            {
            value = m_extensions.flags3.uFitInclinedTextBox;
            break;
            }
        case DIMSTYLE_PROP_General_UseMinLeader_BOOLINT:
            {
            value = !m_extensions.flags3.uIgnoreMinLeader;
            break;
            }
        case DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT:
            {
            // internal value reversed
            value = ! m_data.ad6.flags.noNonStackedSpace;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT:
            {
            value = m_data.ad4.ext_styleflg.tolSignForZero;
            break;
            }
        case DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT:
            {
            value = m_extensions.flags3.uExtendDimLineUnderText;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetBooleanProp (bool value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_IsActive_BOOLINT:
            {
            m_data.ad6.bnc.flags.active = value;
            if (!value)
                m_extensions.flags3.uAutoBallNChain = false;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT:
            {
            m_data.ad6.bnc.flags.elbow = value;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT:
            {
            m_data.ad6.bnc.flags.noDockOnDimLine = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT:
            {
            m_data.ad4.ext_dimflg.joiner = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT:
            {
            m_data.ad5.flags.altColor = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT:
            {
            m_data.ad5.flags.altStyle = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT:
            {
            m_data.ad5.flags.altWeight = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT:
            {
            m_data.ad1.mode.witness = value;
            break;
            }
        case DIMSTYLE_PROP_General_IgnoreLevelSymbology_BOOLINT:
            {
            m_data.ad4.ext_dimflg.noLevelSymb = value;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideColor_BOOLINT:
            {
            m_data.ad4.ext_dimflg.colorOverride = value;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT:
            {
            m_data.ad4.ext_dimflg.styleOverride = value;
            break;
            }
        case DIMSTYLE_PROP_General_OverrideWeight_BOOLINT:
            {
            m_data.ad4.ext_dimflg.weightOverride = value;
            break;
            }
        case DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT:
            {
            m_data.ad4.ext_dimflg.relDimLine = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT:
            {
            m_data.ad5.flags.multiLeader = value;
            break;
            }
        case DIMSTYLE_PROP_Placement_CompatibleV3_BOOLINT:
            {
            m_data.ad4.ext_dimflg.compatible = value;
            break;
            }
        case DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT:
            {
            m_data.ad4.ext_dimflg.levelOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Placement_UseReferenceScale_BOOLINT:
            {
            // internal value reversed
            m_data.ad4.ext_dimflg.useRefUnits = !value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT:
            {
            m_data.ad6.flags.noLineThruArrowTerm = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT:
            {
            m_data.ad6.flags.noLineThruDotTerm = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT:
            {
            m_data.ad6.flags.noLineThruOriginTerm = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT:
            {
            m_data.ad6.flags.noLineThruStrokeTerm = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruArrow_BOOLINT:
            {
            m_data.ad6.flags.noLineThruArrowTerm = !value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruDot_BOOLINT:
            {
            m_data.ad6.flags.noLineThruDotTerm = !value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruOrigin_BOOLINT:
            {
            m_data.ad6.flags.noLineThruOriginTerm = !value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DimLineThruStroke_BOOLINT:
            {
            m_data.ad6.flags.noLineThruStrokeTerm = !value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT:
            {
            m_data.ad5.flags.termColor = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT:
            {
            m_data.ad5.flags.termStyle = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT:
            {
            m_data.ad5.flags.termWeight = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT:
            {
            m_data.ad5.flags.uniformCellScale = value;
            break;
            }
        case DIMSTYLE_PROP_Text_AutoLift_BOOLINT:
            {
            // internal value reversed
            m_data.ad4.ext_dimflg.noAutoTextLift = !value;
            break;
            }
        case DIMSTYLE_PROP_Text_Box_BOOLINT:
            {
            m_data.ad4.ext_dimflg.boxtext = value;
            if (value)
                m_data.ad4.ext_dimflg.captext = false;
            break;
            }
        case DIMSTYLE_PROP_Text_Capsule_BOOLINT:
            {
            m_data.ad4.ext_dimflg.captext = value;
            if (value)
                m_data.ad4.ext_dimflg.boxtext = false;
            break;
            }
        case DIMSTYLE_PROP_Text_DecimalComma_BOOLINT:
            {
            m_data.ad4.ext_dimflg.decimal_comma = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Embed_BOOLINT:
            {
            m_data.ad1.params.embed = value;

            // Set the text location flag to zero which means the location of
            // the text is read from embed.
            m_extensions.flags2.uTextLocation = 0;
            break;
            }
        case DIMSTYLE_PROP_Text_Font_BOOLINT:
            {
            m_data.ad4.ext_dimflg.fontOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Horizontal_BOOLINT:
            {
            m_data.ad1.params.horizontal = value;
            break;
            }
        case DIMSTYLE_PROP_Text_LeadingZero_BOOLINT:
            {
            m_data.ad4.ext_dimflg.leading_zero = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT:
            {
            m_data.ad4.ext_dimflg.omitLeadingDelimeter = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideColor_BOOLINT:
            {
            m_data.ad4.ext_dimflg.textColorOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT:
            {
            m_data.ad4.ext_dimflg.textSizeOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT:
            {
            m_data.ad4.ext_dimflg.textWeightOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT:
            {
            m_data.ad5.flags.useWidth = value;
            break;
            }
        case DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT:
            {
            m_data.ad4.ext_dimflg.leading_zero2 = value;
            break;
            }
        case DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT:
            {
            /* These are redundant */
            m_data.ad1.params.dual = m_data.ad5.flags.useSecUnits = value;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractions_BOOLINT:
            {
            m_data.ad5.flags.useStackedFractions = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideStackedFractions_BOOLINT:
            {
            m_data.ad6.flags.fractionOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT:
            {
            m_data.ad6.flags.underlineOverride = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Underline_BOOLINT:
            {
            m_data.ad4.ext_dimflg.underlineText = value;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Mode_BOOLINT:
            {
            m_data.ad1.params.tolmode = value;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_Show_BOOLINT:
            {
            m_data.ad1.params.tolerance = value;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT:
            {
            m_data.ad1.params.tolStackEqual = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltIsActive_BOOLINT:
            {
            m_data.ad6.altFormat.flags.useAltFmt = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.useAltFmt = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.adp_delimiter = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad6.secAltFormat.flags.adp_nomastunits = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.adp_subunits = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.adp_label = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.equalToThreshold = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.lessThanThreshold = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT:
            {
            m_data.ad6.secAltFormat.flags.adp_allowZeroMast = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad6.secAltFormat.flags.adp_hideZeroSub = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT:
            {
            m_data.ad6.altFormat.flags.adp_delimiter = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad6.altFormat.flags.adp_nomastunits = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT:
            {
            m_data.ad6.altFormat.flags.adp_subunits = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT:
            {
            m_data.ad6.altFormat.flags.adp_label = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT:
            {
            m_data.ad6.altFormat.flags.equalToThreshold = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT:
            {
            m_data.ad6.altFormat.flags.lessThanThreshold = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT:
            {
            m_data.ad6.altFormat.flags.adp_allowZeroMast = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad6.altFormat.flags.adp_hideZeroSub = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT:
            {
            m_data.ad4.ext_styleflg.angleLeadingZero = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT:
            {
            m_data.ad1.mode.labang = value;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT:
            {
            m_data.ad4.ext_styleflg.angleTrailingZero = value;
            break;
            }
        case DIMSTYLE_PROP_Value_RoundLSD_BOOLINT:
            {
            m_data.ad4.ext_dimflg.roundLSD = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT:
            {
            m_data.ad1.format.adp_delimiter2 = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad1.format.adp_nomastunits2 = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT:
            {
            m_data.ad1.format.adp_subunits2 = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT:
            {
            m_data.ad4.ext_dimflg.trailing_zeros2 = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT:
            {
            m_data.ad1.format.adp_label2 = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT:
            {
            m_data.ad1.params.adp_allowZeroMast2 = value;
            break;
            }
        case DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad1.params.adp_hideZeroSub2 = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT:
            {
            m_data.ad1.format.adp_delimiter = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad1.format.adp_nomastunits = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT:
            {
            m_data.ad1.format.adp_subunits = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT:
            {
            m_data.ad4.ext_dimflg.trailing_zeros = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT:
            {
            m_data.ad1.format.adp_label = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT:
            {
            m_data.ad1.params.adp_allowZeroMast = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT:
            {
            // internal value reversed
            m_data.ad1.params.adp_hideZeroSub = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT:
            {
            m_data.ad4.ext_dimflg.superscriptLSD = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT:
            {
            m_data.ad5.flags.thousandSep = value;
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT:
            {
            m_data.ad5.flags.metricSpc = value;
            break;
            }
        case DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT:
            {
            // internal value reversed
            m_data.ad1.params.overrideWorkingUnits = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT:
            {
            m_extensions.flags.uOrdUseDatumValue = value;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT:
            {
            m_extensions.flags.uOrdDecrementReverse = value;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT:
            {
            m_extensions.flags2.uOrdFreeLocation = value;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT:
            {
            m_extensions.flags.labelLineSupressAngle = value;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT:
            {
            m_extensions.flags.labelLineSupressLength = value;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT:
            {
            m_extensions.flags.labelLineInvertLabels = value;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT:
            {
            m_extensions.flags.uLabelLineAdjacentLabels = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceFraction = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceAltFraction = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceTolFraction = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceSecFraction = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceAltSecFraction = value;
            break;
            }
        case DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT:
            {
            m_extensions.flags.uNoReduceTolSecFraction = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT:
            {
            m_extensions.flags2.uNoteLeaderType = value;
            break;
            }
        case DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT:
            {
            m_extensions.flags2.uNotUseModelAnnotationScale = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT:
            {
            m_extensions.flags2.uNoteScaleFrame = value;
            break;
            }
        case DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT:
            {
            m_extensions.flags3.uNoTermsOutside = value;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT:
            {
            m_extensions.flags3.uUseBncElbowLength = value;
            break;
            }
        case DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT:
            {
            m_extensions.flags3.uTightFitTextAbove = value;
            break;
            }
        case DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT:
            {
            m_extensions.flags3.uFitInclinedTextBox = value;
            break;
            }
        case DIMSTYLE_PROP_General_UseMinLeader_BOOLINT:
            {
            m_extensions.flags3.uIgnoreMinLeader = !value;
            break;
            }
        case DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT:
            {
            // internal value reversed
            m_data.ad6.flags.noNonStackedSpace = !value;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT:
            {
            m_data.ad4.ext_styleflg.tolSignForZero = value;
            break;
            }
        case DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT:
            {
            m_extensions.flags3.uExtendDimLineUnderText = value;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetCharProp (UShort& value, DimStyleProp iProperty) const
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Symbol_DiameterChar_CHAR:
            {
            value = m_data.ad2.diam;
            break;
            }
        case DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.lower_prefix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.lower_suffix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.main_prefix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.main_suffix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR:
            {
            value = m_data.ad6.pmChar;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixChar_CHAR:
            {
            value = m_data.ad6.preChar;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixChar_CHAR:
            {
            value = m_data.ad6.sufChar;
            break;
            }
        case DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.tol_prefix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.tol_suffix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.upper_prefix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR:
            {
            value = m_data.ad4.dimtxt.upper_suffix;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowChar_CHAR:
            {
            value = m_data.ad2.arrhead;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotChar_CHAR:
            {
            value = m_data.ad4.bowtie_symbol;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginChar_CHAR:
            {
            value = m_data.ad2.comorg;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeChar_CHAR:
            {
            value = m_data.ad2.oblique;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteChar_CHAR:
            {
            value = m_extensions.iNoteTerminatorChar;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetCharProp
(
UShort          value,
DimStyleProp    iProperty
)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Symbol_DiameterChar_CHAR:
            {
            m_data.ad2.diam = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR:
            {
            m_data.ad4.dimtxt.lower_prefix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR:
            {
            m_data.ad4.dimtxt.lower_suffix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR:
            {
            m_data.ad4.dimtxt.main_prefix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR:
            {
            m_data.ad4.dimtxt.main_suffix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR:
            {
            m_data.ad6.pmChar = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixChar_CHAR:
            {
            m_data.ad6.preChar = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixChar_CHAR:
            {
            m_data.ad6.sufChar = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR:
            {
            m_data.ad4.dimtxt.tol_prefix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR:
            {
            m_data.ad4.dimtxt.tol_suffix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR:
            {
            m_data.ad4.dimtxt.upper_prefix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR:
            {
            m_data.ad4.dimtxt.upper_suffix = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowChar_CHAR:
            {
            m_data.ad2.arrhead = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotChar_CHAR:
            {
            m_data.ad4.bowtie_symbol = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginChar_CHAR:
            {
            m_data.ad2.comorg = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeChar_CHAR:
            {
            m_data.ad2.oblique = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteChar_CHAR:
            {
            m_extensions.iNoteTerminatorChar = value;
            if (0.0 != value)
                m_extensions.modifiers |= STYLE_Extension_NoteTerminatorChar;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteTerminatorChar;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetDoubleProp (double& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE:
            {
            value = m_extensions.dBncElbowLength;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE:
            {
            value = m_extensions.dNoteElbowLength;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE:
            {
            value = m_extensions.dNoteLeftMargin;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE:
            {
            value = m_extensions.dNoteLowerMargin;
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE:
            {
            value = m_extensions.dNoteFrameScale;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE:
            {
            value = m_data.ad4.witness_extend;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE:
            {
            value = m_data.ad4.witness_offset;
            break;
            }
        case DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE:
            {
            value = m_data.ad4.dimcen;
            break;
            }
        case DIMSTYLE_PROP_General_DimensionScale_DOUBLE:
            {
            value = m_data.ad4.dimension_scale;
            break;
            }
        case DIMSTYLE_PROP_General_StackOffset_DOUBLE:
            {
            value = m_data.ad4.stack_offset;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Height_DOUBLE:
            {
            value = m_data.ad5.termHeight;
            break;
            }
        case DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE:
            {
            value = m_data.ad4.text_margin;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Width_DOUBLE:
            {
            value = m_data.ad5.termWidth;
            break;
            }
        case DIMSTYLE_PROP_Text_Height_DOUBLE:
            {
            value = m_data.ad2.txheight;
            break;
            }
        case DIMSTYLE_PROP_Text_Width_DOUBLE:
            {
            value = m_data.ad5.textWidth;
            break;
            }
        case DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE:
            {
            value = m_data.ad5.textMarginH;
            break;
            }
        case DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE:
            {
            value = m_data.ad5.textMarginV;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE:
            {
            if (0.0 >= m_extensions.stackedFractionScale)
                value = 1.0;
            else
                value = m_extensions.stackedFractionScale;

            break;
            }
        case DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE:
            {
            value = m_extensions.dInlineTextLift;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE:
            {
            value = m_data.ad2.lowtol;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE:
            {
            value = m_data.ad5.tolMarginH;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE:
            {
            value = m_data.ad4.toltxt_scale;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextVerticalMargin_DOUBLE:
            {
            value = m_data.ad5.tolMarginV;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE:
            {
            value = m_data.ad5.tolSepV;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE:
            {
            value = m_data.ad2.uptol;
            break;
            }
        case DIMSTYLE_PROP_Value_AltThreshold_DOUBLE:
            {
            value = m_data.ad6.altFormat.threshold;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE:
            {
            value = m_data.ad6.secAltFormat.threshold;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE:
            {
            value = m_extensions.dOrdinateDatumValue;
            break;
            }
        case DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE:
            {
            value = 1.0;

            if ((m_extensions.modifiers & STYLE_Extension_AnnotationScale) &&
                (fabs (m_extensions.dAnnotationScale - 0.0) > mgds_fc_epsilon))
                value = m_extensions.dAnnotationScale;

            break;
            }
        case DIMSTYLE_PROP_Value_RoundOff_DOUBLE:
            {
            value = m_extensions.dRoundOff;
            break;
            }
        case DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE:
            {
            value = m_extensions.dSecondaryRoundOff;
            break;
            }

        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetDoubleProp (double valueIn, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE:
            {
            m_extensions.dBncElbowLength = valueIn;
            if (0.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_BncElbowLength;
            else
                m_extensions.modifiers &= ~STYLE_Extension_BncElbowLength;
            break;
            }
        case DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE:
            {
            m_extensions.dNoteElbowLength = valueIn;
            if (0.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_NoteElbowLength;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteElbowLength;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE:
            {
            m_extensions.dNoteLeftMargin = valueIn;
            if (0.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_NoteLeftMargin;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteLeftMargin;
            break;
            }
        case DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE:
            {
            m_extensions.dNoteLowerMargin = valueIn;
            if (0.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_NoteLowerMargin;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteLowerMargin;
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE:
            {
            m_extensions.dNoteFrameScale = valueIn;
            if (0.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_NoteFrameScale;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteFrameScale;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE:
            {
            m_data.ad4.witness_extend = valueIn;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE:
            {
            m_data.ad4.witness_offset = valueIn;
            break;
            }
        case DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE:
            {
            m_data.ad4.dimcen = valueIn;
            break;
            }
        case DIMSTYLE_PROP_General_DimensionScale_DOUBLE:
            {
            m_data.ad4.dimension_scale = valueIn;
            break;
            }
        case DIMSTYLE_PROP_General_StackOffset_DOUBLE:
            {
            m_data.ad4.stack_offset = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Height_DOUBLE:
            {
            m_data.ad5.termHeight = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE:
            {
            m_data.ad4.text_margin = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Width_DOUBLE:
            {
            m_data.ad5.termWidth = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Text_Height_DOUBLE:
            {
            m_data.ad2.txheight = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE:
            {
            m_data.ad5.textMarginH = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE:
            {
            m_data.ad5.textMarginV = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Text_Width_DOUBLE:
            {
            m_data.ad5.textWidth = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE:
            {
            if (0.0 > valueIn)
                {
                status = DGNHANDLERS_STATUS_BadArg;
                break;
                }

            m_extensions.stackedFractionScale = valueIn;

            if (0.0 != valueIn && 1.0 != valueIn)
                m_extensions.modifiers |= STYLE_Extension_StackedFractionScale;
            else
                m_extensions.modifiers &= ~STYLE_Extension_StackedFractionScale;

            break;
            }
        case DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE:
            {
            m_extensions.dInlineTextLift = valueIn;

            if (!LegacyMath::DEqual(valueIn, 0.0))
                m_extensions.modifiers |= STYLE_Extension_InlineTextLift;
            else
                m_extensions.modifiers &= ~STYLE_Extension_InlineTextLift;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE:
            {
            m_data.ad2.lowtol = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE:
            {
            m_data.ad5.tolMarginH = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE:
            {
            m_data.ad4.toltxt_scale = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextVerticalMargin_DOUBLE:
            {
            m_data.ad5.tolMarginV = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE:
            {
            m_data.ad5.tolSepV = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE:
            {
            m_data.ad2.uptol = valueIn;
            break;
            }
        case DIMSTYLE_PROP_Value_AltThreshold_DOUBLE:
            {
            m_data.ad6.altFormat.threshold = (UInt32) valueIn;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE:
            {
            m_data.ad6.secAltFormat.threshold = (UInt32) valueIn;
            break;
            }
        case DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE:
            {
            m_extensions.dOrdinateDatumValue = valueIn;
            if (valueIn)
                m_extensions.modifiers |= STYLE_Extension_OrdinateDatumValue;
            else
                m_extensions.modifiers &= ~STYLE_Extension_OrdinateDatumValue;
            break;
            }
        case DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE:
            {
            m_extensions.dAnnotationScale = valueIn;
            if (valueIn)
                m_extensions.modifiers |= STYLE_Extension_AnnotationScale;
            else
                m_extensions.modifiers &= ~STYLE_Extension_AnnotationScale;

            break;
            }
        case DIMSTYLE_PROP_Value_RoundOff_DOUBLE:
            {
            m_extensions.dRoundOff = valueIn;
            if (valueIn)
                m_extensions.modifiers |= STYLE_Extension_RoundOff;
            else
                m_extensions.modifiers &= ~STYLE_Extension_RoundOff;
            break;
            }
        case DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE:
            {
            m_extensions.dSecondaryRoundOff = valueIn;
            if (valueIn)
                m_extensions.modifiers |= STYLE_Extension_SecondaryRoundOff;
            else
                m_extensions.modifiers &= ~STYLE_Extension_SecondaryRoundOff;
            break;
            }

        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static  StatusInt  dgnDimStyle_getValueFormatProps
(
DimStyleProp       *pShowMasterProp,
DimStyleProp       *pShowSubProp,
DimStyleProp       *pShowLabelProp,
DimStyleProp       *pShowDelimeterProp,
DimStyleProp        formatProp
)
    {
    StatusInt   status = SUCCESS;

    switch (formatProp)
        {
        case DIMSTYLE_PROP_Value_Format_INTEGER:
            {
            *pShowMasterProp    = DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT;
            *pShowSubProp       = DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT;
            *pShowLabelProp     = DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT;
            *pShowDelimeterProp = DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT;
            break;
            }
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
            {
            *pShowMasterProp    = DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT;
            *pShowSubProp       = DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT;
            *pShowLabelProp     = DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT;
            *pShowDelimeterProp = DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT;
            break;
            }
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
            {
            *pShowMasterProp    = DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT;
            *pShowSubProp       = DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT;
            *pShowLabelProp     = DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT;
            *pShowDelimeterProp = DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            *pShowMasterProp    = DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT;
            *pShowSubProp       = DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT;
            *pShowLabelProp     = DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT;
            *pShowDelimeterProp = DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT;
            break;
            }
        default:
            {
            status = ERROR;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetValueFormat (DimStyleProp_Value_Format& format, DimStyleProp formatProp) const
    {
    bool            showMasterVal,  showSubVal,  showLabelVal,  showDelimeterVal;
    DimStyleProp    showMasterProp, showSubProp, showLabelProp, showDelimeterProp;

    if (SUCCESS != dgnDimStyle_getValueFormatProps (&showMasterProp, &showSubProp, &showLabelProp, &showDelimeterProp,
                                                    formatProp))
        {
        return STYLETABLE_ERROR_BadName;
        }

    GetBooleanProp (showMasterVal,     showMasterProp);
    GetBooleanProp (showSubVal,        showSubProp);
    GetBooleanProp (showLabelVal,      showLabelProp);
    GetBooleanProp (showDelimeterVal,  showDelimeterProp);

    if ( ! showMasterVal)
        {
        if (showLabelVal)
            format = DIMSTYLE_VALUE_Value_Format_SU_Label;
        else
            format = DIMSTYLE_VALUE_Value_Format_SU;
        }
    else
    if ( ! showSubVal)
        {
        if (showLabelVal)
            format = DIMSTYLE_VALUE_Value_Format_MU_Label;
        else
            format = DIMSTYLE_VALUE_Value_Format_MU;
        }
    else
    if (showLabelVal)
        {
        if (showDelimeterVal)
            format = DIMSTYLE_VALUE_Value_Format_MU_Label_dash_SU_Label;
        else
            format = DIMSTYLE_VALUE_Value_Format_MU_Label_SU_Label;
        }
    else
        {
        format = DIMSTYLE_VALUE_Value_Format_MU_dash_SU;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   dgnDimStyle_getComparisonProps
(
DimStyleProp       *pLessThanProp,
DimStyleProp       *pEqualToProp,
DimStyleProp        comparisonProp
)
    {
    StatusInt   status = SUCCESS;

    switch (comparisonProp)
        {
        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
            {
            *pLessThanProp = DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT;
            *pEqualToProp  = DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT;
            break;
            }
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            *pLessThanProp = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT;
            *pEqualToProp  = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT;
            break;
            }
        default:
            {
            status = ERROR;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetComparisonValue (DimStyleProp_Value_Comparison& comparison, DimStyleProp comparisonProp) const
    {
    bool            lessThanVal,  equalToVal;
    DimStyleProp    lessThanProp, equalToProp;

    if (SUCCESS != dgnDimStyle_getComparisonProps (&lessThanProp, &equalToProp, comparisonProp))
        return STYLETABLE_ERROR_BadName;

    GetBooleanProp (lessThanVal, lessThanProp);
    GetBooleanProp (equalToVal,  equalToProp);

    if (lessThanVal)
        {
        if (equalToVal)
            comparison = DIMSTYLE_VALUE_Value_Compare_LessOrEqual;
        else
            comparison = DIMSTYLE_VALUE_Value_Compare_Less;
        }
    else
        {
        if (equalToVal)
            comparison = DIMSTYLE_VALUE_Value_Compare_GreaterOrEqual;
        else
            comparison = DIMSTYLE_VALUE_Value_Compare_Greater;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimensionStyle::GetLabelLineFormat () const
    {
    switch (m_extensions.flags.labelLineInvertLabels)
        {
        case true:
            {
            switch (m_extensions.flags.uLabelLineAdjacentLabels)
                {
                case true:
                    {
                    return DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleBelow;
                    break;
                    }

                case false:
                default:
                    {
                    if (false == m_extensions.flags.labelLineSupressLength &&
                        false == m_extensions.flags.labelLineSupressAngle)
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_AngleOverLength;
                    else if (true == m_extensions.flags.labelLineSupressLength)
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_AngleAbove;
                    else
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_LengthBelow;
                    break;
                    }
                }

            break;
            }

        case false:
        default:
            {
            switch (m_extensions.flags.uLabelLineAdjacentLabels)
                {
                case true:
                    {
                    return DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleAbove;
                    break;
                    }

                case false:
                default:
                    {
                    if (false == m_extensions.flags.labelLineSupressLength &&
                        false == m_extensions.flags.labelLineSupressAngle)
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_Standard;
                    else if (m_extensions.flags.labelLineSupressLength)
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_AngleBelow;
                    else
                        return DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAbove;
                    break;
                    }
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetIntegerProp (int& value, DimStyleProp iProperty) const
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER:
            {
            value = m_data.ad6.bnc.flags.alignment;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER:
            {
            value = m_data.ad6.bnc.flags.terminator;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER:
            {
            value = m_data.ad6.bnc.flags.chainType;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_Mode_INTEGER:
            {
            if (m_data.ad6.bnc.flags.active)
                value = m_extensions.flags3.uAutoBallNChain ? DIMSTYLE_VALUE_BallAndChain_Mode_Auto : DIMSTYLE_VALUE_BallAndChain_Mode_On;
            else
                value = DIMSTYLE_VALUE_BallAndChain_Mode_None;
            break;
            }
        case DIMSTYLE_PROP_General_Alignment_INTEGER:
            {
            value = m_data.ad1.mode.parallel;
            break;
            }
        case DIMSTYLE_PROP_General_RadialMode_INTEGER:
            {
            value = m_data.ad5.flags.radialMode;
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameType_INTEGER:
            {
            value = GetMLNoteFrameType();
            break;
            }
        case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
            {
            value = m_data.ad5.flags.multiJust;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER:
            {
            value = m_extensions.flags.uMultiJustVertical;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Note_INTEGER:
            {
            value = adim_extensionsGetNoteTerminator (m_extensions);
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteType_INTEGER:
            {
            value = m_extensions.flags2.uNoteTerminatorType;
            break;
            }
        case DIMSTYLE_PROP_MLNote_TextRotation_INTEGER:
            {
            value = m_extensions.flags2.uNoteTextRotation;
            break;
            }
        case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
            {
            value = m_extensions.flags2.uNoteHorAttachment;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
            {
            value = 0;
            adim_getMLNoteVerLeftAttachment (&m_extensions, (UInt16 *) &value);
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
            {
            value = 0;
            adim_getMLNoteVerRightAttachment (&m_extensions, (UInt16 *) &value);
            break;
            }
        case DIMSTYLE_PROP_Placement_TextPosition_INTEGER:
            {
            // automan   semiauto
            //  true      true      manual
            //  true      false     manual
            //  false     true      semi-automatic
            //  false     false     automatic

            if (m_data.ad1.mode.automan)
                value = DIMSTYLE_VALUE_Placement_TextPosition_Manual;
            else
            if (m_data.ad4.ext_dimflg.semiauto)
                value = DIMSTYLE_VALUE_Placement_TextPosition_SemiAuto;
            else
                value = DIMSTYLE_VALUE_Placement_TextPosition_Auto;

            break;
            }
        case DIMSTYLE_PROP_Symbol_DiameterType_INTEGER:
            {
            value = m_data.ad6.flags.diameter;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER:
            {
            value = m_data.ad6.flags.plusMinus;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixType_INTEGER:
            {
            value = m_data.ad6.flags.prefix;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixType_INTEGER:
            {
            value = m_data.ad6.flags.suffix;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowType_INTEGER:
            {
            value = m_data.ad6.flags.arrow;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotType_INTEGER:
            {
            value = m_data.ad6.flags.dot;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Mode_INTEGER:
            {
            value = m_data.ad4.ext_dimflg.arrowOut;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginType_INTEGER:
            {
            value = m_data.ad6.flags.origin;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER:
            {
            value = m_data.ad4.ext_dimflg.arrowhead;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeType_INTEGER:
            {
            value = m_data.ad6.flags.stroke;
            break;
            }
        case DIMSTYLE_PROP_Text_Justification_INTEGER:
            {
            if (m_extensions.flags3.uPushTextRight)
                value = DIMSTYLE_VALUE_Text_Justification_CenterRight;
            else
                value = m_data.ad1.mode.just;
            break;
            }
        case DIMSTYLE_PROP_Text_TextStyleID_INTEGER:
            {
            value = GetTextStyleId();
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER:
            {
            value = m_data.ad5.flags.stackedFractionAlign;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionType_INTEGER:
            {
            value = m_data.ad5.flags.stackedFractionType;
            break;
            }
        case DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER:
            {
            value = m_data.ad4.ext_styleflg.superscriptMode;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleFormat_INTEGER:
            {
            value = m_data.ad4.angle;
            break;
            }
        case DIMSTYLE_PROP_Value_AnglePrecision_INTEGER:
            {
            int angleFormat;
            GetIntegerProp (angleFormat, DIMSTYLE_PROP_Value_AngleFormat_INTEGER);
            if (angleFormat == DIMSTYLE_VALUE_Value_AngleFormat_DegMin)
                {
                value = 0;
                }
            else
                {
                value = m_data.ad4.refdispl;
                value = (0 == value) ? 4 : value - 1;
                }
            break;
            }
        case DIMSTYLE_PROP_Text_Location_INTEGER:
            {
            switch (m_extensions.flags2.uTextLocation)
                {
                case 2:
                    value = DIMSTYLE_VALUE_Text_Location_TopLeft;
                    break;
                case 1:
                    value = DIMSTYLE_VALUE_Text_Location_Outside;
                    break;
                case 0:
                default:
                    value = !m_data.ad1.params.embed;
                    break;
                }
            break;
            }
        case DIMSTYLE_PROP_Value_Format_INTEGER:
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            status = GetValueFormat ((DimStyleProp_Value_Format&) value, iProperty);
            break;
            }

        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            status = GetComparisonValue ((DimStyleProp_Value_Comparison&) value, iProperty);
            break;
            }

        case DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER:
            {
            if ( ! m_data.ad5.flags.metricSpc)
                value = DIMSTYLE_VALUE_Value_ThousandsSep_None;
            else
            if (m_data.ad5.flags.thousandSep)
                value = DIMSTYLE_VALUE_Value_ThousandsSep_Comma;
            else
                value = DIMSTYLE_VALUE_Value_ThousandsSep_Space;
            break;
            }

        case DIMSTYLE_PROP_Text_FrameType_INTEGER:
            {
            if (m_data.ad4.ext_dimflg.boxtext)
                value = DIMSTYLE_VALUE_Text_FrameType_Box;
            else
            if (m_data.ad4.ext_dimflg.captext)
                value = DIMSTYLE_VALUE_Text_FrameType_Capsule;
            else
                value = DIMSTYLE_VALUE_Text_FrameType_None;
            break;
            }

        case DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER:
            {
            value = GetLabelLineFormat();
            break;
            }

        case DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER:
            {
            value = m_data.ad6.flags.dmsAccuracyMode;
            break;
            }

        case DIMSTYLE_PROP_General_FitOption_INTEGER:
            {
            value = m_extensions.flags3.uFitOption;
            // must support legacy data
            if (0 == value)
                GetIntegerProp (value, DIMSTYLE_PROP_Terminator_Mode_INTEGER);
            break;
            }

        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   dgnDimStyle_getIntegerPropInfo
(
int                *pMinValue,
int                *pMaxValue,
int                *pStringListID,
DimStyleProp        iProperty
)
    {
    int         minVal = 0, maxVal = 0;
    int         strList = 0;
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_BallAndChain_Alignment_COUNT - 1;
            strList= STRINGID_DSPropVal_BallAndChain_Alignment;

            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Terminator_Type_COUNT - 1;
            strList= STRINGID_DSPropVal_Terminator_TypeLeft;

            break;
            }
        case DIMSTYLE_PROP_BallAndChain_Mode_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_BallAndChain_MODE_COUNT - 1;
            strList= STRINGID_DSPropVal_BallAndChain_Mode;

            break;
            }
        case DIMSTYLE_PROP_Terminator_Note_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Terminator_Type_COUNT - 1;
            strList= STRINGID_DSPropVal_Terminator_TypeNote;

            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_BallAndChain_ChainType_COUNT - 1;
            strList= STRINGID_DSPropVal_BallAndChain_ChainType;

            break;
            }
        case DIMSTYLE_PROP_General_Alignment_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_General_Alignment_COUNT - 1;
            strList= STRINGID_DSPropVal_General_Alignment;
            break;
            }
        case DIMSTYLE_PROP_General_RadialMode_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_General_RadialMode_COUNT - 1;
            strList= STRINGID_DSPropVal_General_RadialMode;
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_FrameType_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_FrameType;
            break;
            }
        case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_Justification_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_Justification;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_VerticalJustification_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_VerticalJustification;
            break;
            }
        case DIMSTYLE_PROP_MLNote_TextRotation_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_TextRotation_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_TextRotation;
            break;
            }
        case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_HorAttachment_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_HorAttachment;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
        case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_MLNote_VerAttachment_COUNT - 1;
            strList= STRINGID_DSPropVal_MLNote_VerAttachment;
            break;
            }
        case DIMSTYLE_PROP_Placement_TextPosition_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Placement_TextPosition_COUNT - 1;
            strList= STRINGID_DSPropVal_Placement_TextPosition;
            break;
            }
        case DIMSTYLE_PROP_Symbol_DiameterType_INTEGER:
        case DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Symbol_CustomType_COUNT - 1;
            strList= STRINGID_DSPropVal_Symbol_CustomType;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixType_INTEGER:
        case DIMSTYLE_PROP_Symbol_SuffixType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Symbol_PreSufType_COUNT - 1;
            strList= STRINGID_DSPropVal_Symbol_PreSufType;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowType_INTEGER:
        case DIMSTYLE_PROP_Terminator_DotType_INTEGER:
        case DIMSTYLE_PROP_Terminator_OriginType_INTEGER:
        case DIMSTYLE_PROP_Terminator_StrokeType_INTEGER:
        case DIMSTYLE_PROP_Terminator_NoteType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Symbol_TermType_COUNT - 1;
            strList= STRINGID_DSPropVal_Symbol_TermType;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Mode_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Terminator_Mode_COUNT - 1;
            strList= STRINGID_DSPropVal_Terminator_Mode;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Terminator_Arrowhead_COUNT - 1;
            strList= STRINGID_DSPropVal_Terminator_Arrowhead;
            break;
            }
        case DIMSTYLE_PROP_Text_Justification_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_Justification_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_Justification;
            break;
            }
        case DIMSTYLE_PROP_Value_AngleFormat_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_AngleFormat_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_AngleFormat;
            break;
            }
        case DIMSTYLE_PROP_Value_AnglePrecision_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_AnglePrecision_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_AnglePrecision;
            break;
            }
        case DIMSTYLE_PROP_Text_TextStyleID_INTEGER:
            {
            maxVal = -1;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_StackedFractionAlignment_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_StackedFractionAlignment;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_StackedFractionType_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_StackedFractionType;
            break;
            }
        case DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_SuperscriptMode_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_SuperScriptMode;
            break;
            }
        case DIMSTYLE_PROP_Text_Location_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_Location_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_Location;
            break;
            }
        case DIMSTYLE_PROP_Value_Format_INTEGER:
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_Format_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_Format;

            break;
            }
        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_Compare_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_Comparison;
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_Thousands_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_ThousandsOpts;
            break;
            }
        case DIMSTYLE_PROP_Text_FrameType_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Text_FrameType_COUNT - 1;
            strList= STRINGID_DSPropVal_Text_FrameType;
            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_LabelLineFormat_COUNT - 1;
            strList= STRINGID_DSPropVal_Value_LabelLineFormat;
            break;
            }
        case DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_Value_DMSPrecisionMode_COUNT - 1;
            strList= 0;  // NEEDSWORK can't make transkit changes now
            break;
            }
        case DIMSTYLE_PROP_General_FitOption_INTEGER:
            {
            maxVal = DIMSTYLE_VALUE_FitOption_COUNT - 1;
            strList = STRINGID_DSPropVal_FitOption;
            break;
            }

        default:
            {
            status = ERROR;
            break;
            }
        }

    if (SUCCESS == status)
        {
        if (NULL != pMinValue)
            *pMinValue = minVal;

        if (NULL != pMaxValue)
            *pMaxValue = maxVal;

        if (NULL != pStringListID)
            *pStringListID = strList;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dgnDimStyle_isValueValidForIntegerProp
(
int                 value,
DimStyleProp        iProperty
)
    {
    int         minVal = 0, maxVal = 0;
    bool        bValueValid = false;
    StatusInt   status = SUCCESS;

    status = dgnDimStyle_getIntegerPropInfo (&minVal, &maxVal, NULL, iProperty);

    if (SUCCESS == status)
        {
        bool    bDoMinMaxTest = (0 <= maxVal);

        if (bDoMinMaxTest)
            bValueValid = (value >= minVal && value <= maxVal);
        else
            bValueValid = true;
        }

    return bValueValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::SetValueFormat (DimStyleProp formatProp, DimStyleProp_Value_Format format)
    {
    bool            showMasterVal,  showSubVal,  showLabelVal,  showDelimeterVal;
    DimStyleProp    showMasterProp, showSubProp, showLabelProp, showDelimeterProp;

    if (SUCCESS != dgnDimStyle_getValueFormatProps (&showMasterProp, &showSubProp, &showLabelProp, &showDelimeterProp,
                                                    formatProp))
        return ERROR;

    showMasterVal = showSubVal = showLabelVal = showDelimeterVal = false;

    switch (format)
        {
        case DIMSTYLE_VALUE_Value_Format_MU:
            {
            showMasterVal    = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_MU_Label:
            {
            showMasterVal    = true;
            showLabelVal     = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_SU:
            {
            showSubVal       = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_SU_Label:
            {
            showSubVal       = true;
            showLabelVal     = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_MU_dash_SU:
            {
            showMasterVal    = true;
            showSubVal       = true;
            showDelimeterVal = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_MU_Label_SU_Label:
            {
            showMasterVal    = true;
            showSubVal       = true;
            showLabelVal     = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Format_MU_Label_dash_SU_Label:
            {
            showMasterVal    = true;
            showSubVal       = true;
            showLabelVal     = true;
            showDelimeterVal = true;
            break;
            }
        default:
            {
            // this case should have been caught by dgnDimStyle_isValueValidForIntegerProp
            BeAssert (false);
            return ERROR;
            }
        }

    SetBooleanProp (showMasterVal,     showMasterProp);
    SetBooleanProp (showSubVal,        showSubProp);
    SetBooleanProp (showLabelVal,      showLabelProp);
    SetBooleanProp (showDelimeterVal,  showDelimeterProp);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::SetComparisonValue (DimStyleProp comparisonProp, DimStyleProp_Value_Format comparison)
    {
    bool            lessThanVal,  equalToVal;
    DimStyleProp    lessThanProp, equalToProp;

    if (SUCCESS != dgnDimStyle_getComparisonProps (&lessThanProp, &equalToProp, comparisonProp))
        return STYLETABLE_ERROR_BadName;

    lessThanVal = equalToVal = false;

    switch (comparison)
        {
        case DIMSTYLE_VALUE_Value_Compare_Greater:
            {
            break;
            }
        case DIMSTYLE_VALUE_Value_Compare_GreaterOrEqual:
            {
            equalToVal       = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Compare_Less:
            {
            lessThanVal      = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_Compare_LessOrEqual:
            {
            lessThanVal      = true;
            equalToVal       = true;
            break;
            }
        default:
            {
            // this case should have been caught by dgnDimStyle_isValueValidForIntegerProp
            BeAssert (false);
            return ERROR;
            }
        }

    SetBooleanProp (lessThanVal, lessThanProp);
    SetBooleanProp (equalToVal,  equalToProp);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::SetLabelLineFormat
(
int             valueIn
)
    {
    switch (valueIn)
        {
        case DIMSTYLE_VALUE_Value_LabelLineFormat_AngleOverLength:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = true;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAbove:
            {
            m_extensions.flags.labelLineSupressAngle    = true;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = false;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_AngleAbove:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = true;
            m_extensions.flags.labelLineInvertLabels    = true;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_LengthBelow:
            {
            m_extensions.flags.labelLineSupressAngle    = true;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = true;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_AngleBelow:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = true;
            m_extensions.flags.labelLineInvertLabels    = false;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleAbove:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = false;
            m_extensions.flags.uLabelLineAdjacentLabels = true;
            break;
            }
        case DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleBelow:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = true;
            m_extensions.flags.uLabelLineAdjacentLabels = true;
            break;
            }
         case DIMSTYLE_VALUE_Value_LabelLineFormat_Standard:
         default:
            {
            m_extensions.flags.labelLineSupressAngle    = false;
            m_extensions.flags.labelLineSupressLength   = false;
            m_extensions.flags.labelLineInvertLabels    = false;
            m_extensions.flags.uLabelLineAdjacentLabels = false;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetIntegerProp (int value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    if (false == dgnDimStyle_isValueValidForIntegerProp (value, iProperty))
        return DGNHANDLERS_STATUS_BadArg;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER:
            {
            m_data.ad6.bnc.flags.alignment = value;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER:
            {
            m_data.ad6.bnc.flags.terminator = value;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER:
            {
            m_data.ad6.bnc.flags.chainType = value;
            break;
            }
        case DIMSTYLE_PROP_BallAndChain_Mode_INTEGER:
            {
            switch (value)
                {
                case DIMSTYLE_VALUE_BallAndChain_Mode_Auto:
                    m_data.ad6.bnc.flags.active = true;
                    m_extensions.flags3.uAutoBallNChain = true;
                    break;
                case DIMSTYLE_VALUE_BallAndChain_Mode_On:
                    m_data.ad6.bnc.flags.active = true;
                    m_extensions.flags3.uAutoBallNChain = false;
                    break;
                case DIMSTYLE_VALUE_BallAndChain_Mode_None:
                    m_data.ad6.bnc.flags.active = false;
                    m_extensions.flags3.uAutoBallNChain = false;
                    break;
                default:
                    status = STYLETABLE_ERROR_BadIndex;
                }
            break;
            }
        case DIMSTYLE_PROP_General_Alignment_INTEGER:
            {
            m_data.ad1.mode.parallel = value;
            break;
            }
        case DIMSTYLE_PROP_General_RadialMode_INTEGER:
            {
            m_data.ad5.flags.radialMode = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameType_INTEGER:
            {
            SetMLNoteFrameType ((UInt16) value);
            break;
            }
        case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
            {
            m_data.ad5.flags.multiJust = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER:
            {
            m_extensions.flags.uMultiJustVertical = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Note_INTEGER:
            {
            adim_extensionsSetNoteTerminator (m_extensions, value);
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteType_INTEGER:
            {
            m_extensions.flags2.uNoteTerminatorType = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_TextRotation_INTEGER:
            {
            m_extensions.flags2.uNoteTextRotation = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
            {
            m_extensions.flags2.uNoteHorAttachment = value;
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
            {
            adim_setMLNoteVerLeftAttachment (&m_extensions, (UInt16) value);
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
            {
            adim_setMLNoteVerRightAttachment (&m_extensions, (UInt16) value);
            break;
            }
        case DIMSTYLE_PROP_Placement_TextPosition_INTEGER:
            {
            // automan   semiauto
            //  true      true      manual
            //  true      false     manual
            //  false     true      semi-automatic
            //  false     false     automatic

            m_data.ad1.mode.automan = false;
            m_data.ad4.ext_dimflg.semiauto = false;

            if (DIMSTYLE_VALUE_Placement_TextPosition_Manual == value)
                m_data.ad1.mode.automan = true;
            else
            if (DIMSTYLE_VALUE_Placement_TextPosition_SemiAuto == value)
                m_data.ad4.ext_dimflg.semiauto = true;

            break;
            }
        case DIMSTYLE_PROP_Symbol_DiameterType_INTEGER:
            {
            m_data.ad6.flags.diameter = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER:
            {
            m_data.ad6.flags.plusMinus = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixType_INTEGER:
            {
            m_data.ad6.flags.prefix = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixType_INTEGER:
            {
            m_data.ad6.flags.suffix = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowType_INTEGER:
            {
            m_data.ad6.flags.arrow = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotType_INTEGER:
            {
            m_data.ad6.flags.dot = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Mode_INTEGER:
            {
            m_data.ad4.ext_dimflg.arrowOut = value;
            m_extensions.flags3.uFitOption = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginType_INTEGER:
            {
            m_data.ad6.flags.origin = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER:
            {
            m_data.ad4.ext_dimflg.arrowhead = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeType_INTEGER:
            {
            m_data.ad6.flags.stroke = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Justification_INTEGER:
            {
            if (DIMSTYLE_VALUE_Text_Justification_CenterRight == value)
                {
                m_extensions.flags3.uPushTextRight = true;
                m_data.ad1.mode.just = DIMSTYLE_VALUE_Text_Justification_CenterLeft;
                }
            else
                {
                m_data.ad1.mode.just = value;
                m_extensions.flags3.uPushTextRight = false;
                }
            break;
            }
        case DIMSTYLE_PROP_Text_TextStyleID_INTEGER:
            {
            status = SetTextStyleByID (value);
            break;
            }
        case DIMSTYLE_PROP_Value_AngleFormat_INTEGER:
            {
            m_data.ad4.angle = static_cast<byte>(value);
            if (value == static_cast<byte>(AngleFormatVals::DegMin))
                {
                SetIntegerProp (DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating, DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER);
                SetIntegerProp (DIMSTYLE_VALUE_Value_AnglePrecision_1_Place, DIMSTYLE_PROP_Value_AnglePrecision_INTEGER);
                }
            else
                {
                SetIntegerProp (DIMSTYLE_VALUE_Value_DMSPrecisionMode_Fixed, DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER);
                }

            break;
            }
        case DIMSTYLE_PROP_Value_AnglePrecision_INTEGER:
            {
            m_data.ad4.refdispl = static_cast<byte>((4 == value) ? 0 : value + 1);
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER:
            {
            m_data.ad5.flags.stackedFractionAlign = value;
            break;
            }
        case DIMSTYLE_PROP_Text_StackedFractionType_INTEGER:
            {
            m_data.ad5.flags.stackedFractionType = value;
            break;
            }
        case DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER:
            {
            m_data.ad4.ext_styleflg.superscriptMode = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Location_INTEGER:
            {
            switch (value)
                {
                case DIMSTYLE_VALUE_Text_Location_TopLeft:
                    m_extensions.flags2.uTextLocation = 2;
                    m_data.ad1.params.embed = 0;
                    break;

                case DIMSTYLE_VALUE_Text_Location_Outside:
                    m_extensions.flags2.uTextLocation = 1;
                    m_data.ad1.params.embed = 0;
                    break;

                case DIMSTYLE_VALUE_Text_Location_Inline:
                    m_extensions.flags2.uTextLocation = 0;
                    m_data.ad1.params.embed = 1;
                    break;

                case DIMSTYLE_VALUE_Text_Location_Above:
                default:
                    m_extensions.flags2.uTextLocation = 0;
                    m_data.ad1.params.embed = 0;
                    break;
                }
            break;
            }
        case DIMSTYLE_PROP_Value_Format_INTEGER:
        case DIMSTYLE_PROP_Value_AltFormat_INTEGER:
        case DIMSTYLE_PROP_Value_SecFormat_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecFormat_INTEGER:
            {
            status = SetValueFormat (iProperty, (DimStyleProp_Value_Format) value);
            break;
            }
        case DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER:
        case DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER:
            {
            status = SetComparisonValue (iProperty, (DimStyleProp_Value_Format)value);
            break;
            }
        case DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER:
            {
            m_data.ad5.flags.metricSpc   = false;
            m_data.ad5.flags.thousandSep = false;

            if (DIMSTYLE_VALUE_Value_ThousandsSep_Space == value)
                {
                m_data.ad5.flags.metricSpc   = true;
                }
            else
            if (DIMSTYLE_VALUE_Value_ThousandsSep_Comma == value)
                {
                m_data.ad5.flags.metricSpc   = true;
                m_data.ad5.flags.thousandSep = true;
                }

            break;
            }
        case DIMSTYLE_PROP_Text_FrameType_INTEGER:
            {
            m_data.ad4.ext_dimflg.boxtext = false;
            m_data.ad4.ext_dimflg.captext = false;

            if (DIMSTYLE_VALUE_Text_FrameType_Box == value)
                m_data.ad4.ext_dimflg.boxtext = true;
            else
            if (DIMSTYLE_VALUE_Text_FrameType_Capsule == value)
                m_data.ad4.ext_dimflg.captext = true;

            break;
            }
        case DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER:
            {
            SetLabelLineFormat(value);
            break;
            }
        case DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER:
            {
            // Accept floating mode only if angle format is DDMM, else, fixed mode is used.
            if (m_data.ad4.angle == static_cast<byte>(AngleFormatVals::DegMin) && value == DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating)
                m_data.ad6.flags.dmsAccuracyMode = DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating;
            else
                m_data.ad6.flags.dmsAccuracyMode = DIMSTYLE_VALUE_Value_DMSPrecisionMode_Fixed;
            break;
            }
        case DIMSTYLE_PROP_General_FitOption_INTEGER:
            {
            m_extensions.flags3.uFitOption = value;
            if (value < DIMSTYLE_VALUE_Terminator_Mode_COUNT)
                SetIntegerProp (value, DIMSTYLE_PROP_Terminator_Mode_INTEGER);
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetLevelProp (LevelId& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Placement_Level_LEVEL:
            {
            value = LevelId(m_data.ad1.level);
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetLevelProp (LevelId value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    DgnLevels::Level const& level = m_project->Levels().QueryLevelById (value);
    if (!level.IsValid ())
        return DGNHANDLERS_STATUS_BadArg;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Placement_Level_LEVEL:
            {
            m_data.ad1.level = value.GetValue();
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetStringProp (WStringR valueOut, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_DimStyleDescription_MSWCHAR:
            {
            valueOut.assign (m_description);
            break;
            }
        case DIMSTYLE_PROP_General_DimStyleName_MSWCHAR:
            {
            valueOut.assign (m_name);
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR:
            {
            valueOut.assign (m_prefixCellName);
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR:
            {
            valueOut.assign (m_suffixCellName);
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR:
            {
            valueOut.assign (m_arrowCellName);
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR:
            {
            valueOut.assign (m_dotCellName);
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR:
            {
            valueOut.assign (m_originCellName);
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR:
            {
            valueOut.assign (m_strokeCellName);
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR:
            {
            valueOut.assign (m_primary.m_masterUnitLabel);
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR:
            {
            valueOut.assign (m_secondary.m_masterUnitLabel);
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR:
            {
            valueOut.assign (m_secondary.m_subUnitLabel);
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR:
            {
            valueOut.assign (m_primary.m_subUnitLabel);
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR:
            {
            valueOut.assign (m_noteCellName);
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetStringProp (WCharCP valueIn, DimStyleProp iProperty)
    {
    if (NULL == valueIn)
        { BeAssert (false); return ERROR; }

    size_t      maxChars = 0;
    WStringP    value;
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_DimStyleDescription_MSWCHAR:
            {
            maxChars = MAX_DIMSTYLE_DESCRIPTION_LENGTH;
            value = &m_description;
            break;
            }
        case DIMSTYLE_PROP_General_DimStyleName_MSWCHAR:
            {
            maxChars = MAX_DIMSTYLE_NAME_LENGTH;
            value = &m_name;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_prefixCellName;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_suffixCellName;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_arrowCellName;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_dotCellName;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_originCellName;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_strokeCellName;
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR:
            {
            maxChars = MAX_UNIT_LABEL_LENGTH;
            value = &m_primary.m_masterUnitLabel;
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR:
            {
            maxChars = MAX_UNIT_LABEL_LENGTH;
            value = &m_secondary.m_masterUnitLabel;
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR:
            {
            maxChars = MAX_UNIT_LABEL_LENGTH;
            value = &m_secondary.m_subUnitLabel;
            break;
            }
        case DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR:
            {
            maxChars = MAX_UNIT_LABEL_LENGTH;
            value = &m_primary.m_subUnitLabel;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR:
            {
            maxChars = MAX_CELLNAME_LENGTH;
            value = &m_noteCellName;
            break;
            }
        default:
            {
            value = NULL;
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    if (SUCCESS == status && NULL != value)
        {
        size_t valueInLen = wcslen (valueIn);
        if (maxChars > valueInLen)
            maxChars = valueInLen + 1;
        value->assign (valueIn, maxChars - 1);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetFontProp (UInt32& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Font_FONT:
            {
            value = m_data.ad4.dimfont;
            break;
            }
        case DIMSTYLE_PROP_Symbol_DiameterFont_FONT:
            {
            value = m_data.ad2.diamfont;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixFont_FONT:
            {
            value = m_data.ad6.preFont;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixFont_FONT:
            {
            value = m_data.ad6.sufFont;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowFont_FONT:
            {
            value = m_data.ad2.arrfont;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotFont_FONT:
            {
            value = m_data.ad4.bowtie_font;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginFont_FONT:
            {
            value = m_data.ad2.cofont;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeFont_FONT:
            {
            value = m_data.ad2.oblqfont;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteFont_FONT:
            {
            value = m_extensions.iNoteTerminatorFont;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetFontProp (UInt32 value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Font_FONT:
            {
            m_data.ad4.dimfont = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_DiameterFont_FONT:
            {
            m_data.ad2.diamfont = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_PrefixFont_FONT:
            {
            m_data.ad6.preFont = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_SuffixFont_FONT:
            {
            m_data.ad6.sufFont = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_ArrowFont_FONT:
            {
            m_data.ad2.arrfont = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_DotFont_FONT:
            {
            m_data.ad4.bowtie_font = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_OriginFont_FONT:
            {
            m_data.ad2.cofont = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_StrokeFont_FONT:
            {
            m_data.ad2.oblqfont = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_NoteFont_FONT:
            {
            m_extensions.iNoteTerminatorFont = value;
            if (0 != value)
                m_extensions.modifiers |= STYLE_Extension_NoteTerminatorFont;
            else
                m_extensions.modifiers &= ~STYLE_Extension_NoteTerminatorFont;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetColorProp (UInt32& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Color_COLOR:
            {
            value = m_data.ad4.dim_color;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Color_COLOR:
            {
            value = m_data.ad5.altSymb.color;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Color_COLOR:
            {
            value = m_data.ad5.termSymb.color;
            break;
            }
        case DIMSTYLE_PROP_Text_Color_COLOR:
            {
            value = m_data.ad4.dimtxt_color;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetColorProp (UInt32 value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Color_COLOR:
            {
            m_data.ad4.dim_color = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Color_COLOR:
            {
            m_data.ad5.altSymb.color = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Color_COLOR:
            {
            m_data.ad5.termSymb.color = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Color_COLOR:
            {
            m_data.ad4.dimtxt_color = value;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetWeightProp (UInt32& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;
    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Weight_WEIGHT:
            {
            value = m_data.ad4.dim_weight;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT:
            {
            value = m_data.ad5.altSymb.weight;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Weight_WEIGHT:
            {
            value = m_data.ad5.termSymb.weight;
            break;
            }
        case DIMSTYLE_PROP_Text_Weight_WEIGHT:
            {
            value = m_data.ad4.dimtxt_weight;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetWeightProp (UInt32 value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_Weight_WEIGHT:
            {
            m_data.ad4.dim_weight = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT:
            {
            m_data.ad5.altSymb.weight = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Weight_WEIGHT:
            {
            m_data.ad5.termSymb.weight = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Weight_WEIGHT:
            {
            m_data.ad4.dimtxt_weight = value;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetLineStyleProp (Int32& value, DimStyleProp iProperty) const
    {
    StatusInt       status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_LineStyle_LINESTYLE:
            {
            value = m_data.ad4.dim_style;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE:
            {
            value = m_data.ad5.altSymb.style;
            break;
            }
        case DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE:
            {
            value = m_data.ad5.termSymb.style;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetLineStyleProp (Int32 value, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_General_LineStyle_LINESTYLE:
            {
            m_data.ad4.dim_style = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE:
            {
            m_data.ad5.altSymb.style = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE:
            {
            m_data.ad5.termSymb.style = value;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetUnitsProp (UnitDefinitionP masterUnit, UnitDefinitionP subUnit, DimStyleProp iProperty) const
    {
    WString             masterLabel, subLabel;
    WStringCP           pMULabel, pSULabel;
    DimUnits const*     pMUUnit;
    DimUnits const*     pSUUnit;
    StatusInt           status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_Unit_UNITS:
            {
            pMUUnit = &m_data.ad7.primaryMaster;
            pSUUnit = &m_data.ad7.primarySub;

            GetStringProp (masterLabel, DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR);
            GetStringProp (subLabel,    DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR);
            pMULabel = &masterLabel;
            pSULabel = &subLabel;

            break;
            }
        case DIMSTYLE_PROP_Value_UnitSec_UNITS:
            {
            pMUUnit = &m_data.ad7.secondaryMaster;
            pSUUnit = &m_data.ad7.secondarySub;

            pMULabel = &m_secondary.m_masterUnitLabel;
            pSULabel = &m_secondary.m_subUnitLabel;

            break;
            }
        default:
            {
            pMUUnit = pSUUnit = NULL;
            pMULabel = pSULabel = NULL;
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    if (SUCCESS == status)
        {
        if (masterUnit)
            unitDefFromDimUnits (*masterUnit, *pMUUnit, pMULabel->c_str());

        if (subUnit)
            unitDefFromDimUnits (*subUnit, *pSUUnit, pSULabel->c_str());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Does NOT set the unit labels
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetUnitsProp (UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit, DimStyleProp iProperty)
    {
    if ( ! masterUnit.IsValid() || ! subUnit.IsValid ())
        return DGNHANDLERS_STATUS_BadArg;

    // Master unit must be larger than sub unit
    int comparison = masterUnit.CompareByScale (subUnit);

    if (ERROR == comparison || 0 < comparison)
        return DGNHANDLERS_STATUS_BadArg;

    DimUnits*   pMUUnit;
    DimUnits*   pSUUnit;
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_Unit_UNITS:
            {
            pMUUnit = &m_data.ad7.primaryMaster;
            pSUUnit = &m_data.ad7.primarySub;
            break;
            }
        case DIMSTYLE_PROP_Value_UnitSec_UNITS:
            {
            pMUUnit = &m_data.ad7.secondaryMaster;
            pSUUnit = &m_data.ad7.secondarySub;
            break;
            }
        default:
            {
            pMUUnit = pSUUnit = NULL;
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    if (SUCCESS == status)
        {
        dimUnitsFromUnitDef (*pMUUnit, masterUnit);
        dimUnitsFromUnitDef (*pSUUnit, subUnit);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetOneUnitProp (UnitDefinitionR unitDef, DimStyleProp iProperty) const
    {
    UnitDefinitionP pMasterUnit = NULL, pSubUnit = NULL;
    StatusInt       status = SUCCESS;

    DimStyleProp    combinedProp = DIMSTYLE_PROP_Invalid;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT:
            {
            pMasterUnit  = &unitDef;
            combinedProp = DIMSTYLE_PROP_Value_Unit_UNITS;

            break;
            }
        case DIMSTYLE_PROP_Value_UnitSub_ONEUNIT:
            {
            pSubUnit     = &unitDef;
            combinedProp = DIMSTYLE_PROP_Value_Unit_UNITS;

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT:
            {
            pMasterUnit  = &unitDef;
            combinedProp = DIMSTYLE_PROP_Value_UnitSec_UNITS;

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT:
            {
            pSubUnit     = &unitDef;
            combinedProp = DIMSTYLE_PROP_Value_UnitSec_UNITS;
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    if (SUCCESS == status)
        status = GetUnitsProp (pMasterUnit, pSubUnit, combinedProp);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Does NOT set the unit labels
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::SetOneUnitProp (UnitDefinitionCR unitDef, DimStyleProp iProperty)
    {
    UnitDefinition  masterUnit, subUnit;
    StatusInt       status = SUCCESS;

    DimStyleProp    combinedProp = DIMSTYLE_PROP_Invalid;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT:
            {
            combinedProp = DIMSTYLE_PROP_Value_Unit_UNITS;

            masterUnit = unitDef;
            unitDefFromDimUnits (subUnit, m_data.ad7.primarySub, m_primary.m_subUnitLabel.c_str());

            break;
            }
        case DIMSTYLE_PROP_Value_UnitSub_ONEUNIT:
            {
            combinedProp = DIMSTYLE_PROP_Value_Unit_UNITS;

            unitDefFromDimUnits (masterUnit, m_data.ad7.primaryMaster, m_primary.m_masterUnitLabel.c_str());
            subUnit = unitDef;

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT:
            {
            combinedProp = DIMSTYLE_PROP_Value_UnitSec_UNITS;

            masterUnit = unitDef;
            unitDefFromDimUnits (subUnit, m_data.ad7.secondarySub, m_secondary.m_subUnitLabel.c_str());

            break;
            }
        case DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT:
            {
            combinedProp = DIMSTYLE_PROP_Value_UnitSec_UNITS;

            unitDefFromDimUnits (masterUnit, m_data.ad7.secondaryMaster, m_secondary.m_subUnitLabel.c_str());
            subUnit = unitDef;

            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    if (SUCCESS == status)
        status = SetUnitsProp (masterUnit, subUnit, combinedProp);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::GetTemplateFlagProp (UShort& value, DimensionType dimensionType, DimStyleProp iProperty) const
    {
    StatusInt   status     = SUCCESS;
    int         intDimType = static_cast<int>(dimensionType);

    if (intDimType <= 0)
        return STYLETABLE_ERROR_BadName;

    if (dimensionType > DimensionType::MaxThatHasTemplate)
        return STYLETABLE_ERROR_BadName;

    int         dimcmd = intDimType - 1;
    switch (iProperty)
        {
        case DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].altExt;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].left_witness;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].right_witness;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].centermark;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkLeft_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].centerLeft;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkRight_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].centerRight;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkTop_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].centerTop;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkBottom_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].centerBottom;
            break;
            }
        case DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].stacked;
            break;
            }
        case DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].pre_symbol;
            break;
            }
        case DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].post_symbol;
            break;
            }
        case DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].first_term;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].bowtie_symbol;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].left_term;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].right_term;
            break;
            }
        case DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].above_symbol;
            break;
            }
        case DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].nofit_vertical;
            break;
            }
        case DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG:
            {
            value = m_data.ad4.dim_template[dimcmd].vertical_text;
            break;
            }
        case DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG:
            {
            if ( m_data.ad4.dim_template[dimcmd].vertical_text)
                value = DIMSTYLE_VALUE_Text_Vertical_Always;
            else
            if (m_data.ad4.dim_template[dimcmd].nofit_vertical)
                value = DIMSTYLE_VALUE_Text_Vertical_NoFit;
            else
                value = DIMSTYLE_VALUE_Text_Vertical_Never;

            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetTemplateFlagProp (UShort value, DimensionType dimensionType, DimStyleProp iProperty)
    {
    StatusInt   status     = SUCCESS;
    int         intDimType = static_cast<int>(dimensionType);

    if (intDimType <= 0)
        return STYLETABLE_ERROR_BadName;

    if (dimensionType > DimensionType::MaxThatHasTemplate)
        return STYLETABLE_ERROR_BadName;

    int         dimcmd = intDimType - 1;
    switch (iProperty)
        {
        case DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].altExt = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].left_witness = value;
            break;
            }
        case DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].right_witness = value;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].centermark = value;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkLeft_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].centerLeft = value;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkRight_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].centerRight = value;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkTop_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].centerTop = value;
            break;
            }
        case DIMSTYLE_PROP_General_ShowCenterMarkBottom_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].centerBottom = value;
            break;
            }
        case DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].stacked = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].pre_symbol = value;
            break;
            }
        case DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].post_symbol = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].first_term = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].bowtie_symbol = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].left_term = value;
            break;
            }
        case DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].right_term = value;
            break;
            }
        case DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].above_symbol = value;
            break;
            }
        case DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].nofit_vertical = value;
            break;
            }
        case DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].vertical_text = value;
            break;
            }
        case DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG:
            {
            m_data.ad4.dim_template[dimcmd].vertical_text  = false;
            m_data.ad4.dim_template[dimcmd].nofit_vertical = false;

            if (DIMSTYLE_VALUE_Text_Vertical_Always == value)
                m_data.ad4.dim_template[dimcmd].vertical_text = true;
            else
            if (DIMSTYLE_VALUE_Text_Vertical_NoFit == value)
                m_data.ad4.dim_template[dimcmd].nofit_vertical = true;

            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_GetTextStyleProp (DgnTextStylePtr& textStyle, DimStyleProp iProperty) const
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE:
            {
            LegacyTextStyle   localTextStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*m_textStyle);

            /* The following properties are stored in the main part of the dimStyle
               because they existed BEFORE the text style was stored in the dimStyle */
            localTextStyle.fontNo                   = m_data.ad4.dimfont;
            localTextStyle.height                   = m_data.ad2.txheight;
            localTextStyle.width                    = m_data.ad5.textWidth;
            localTextStyle.color                    = m_data.ad4.dimtxt_color;
            localTextStyle.flags.underline          = m_data.ad4.ext_dimflg.underlineText;
            localTextStyle.flags.fractions          = m_data.ad5.flags.useStackedFractions;

            localTextStyle.overrideFlags.fontNo     = m_data.ad4.ext_dimflg.fontOverride;
            localTextStyle.overrideFlags.height     = m_data.ad4.ext_dimflg.textSizeOverride;
            localTextStyle.overrideFlags.width      = m_data.ad5.flags.useWidth;
            localTextStyle.overrideFlags.color      = m_data.ad4.ext_dimflg.textColorOverride;
            localTextStyle.overrideFlags.colorFlag  = m_data.ad4.ext_dimflg.textColorOverride;
            localTextStyle.overrideFlags.underline  = m_data.ad6.flags.underlineOverride;
            localTextStyle.overrideFlags.fractions  = m_data.ad6.flags.fractionOverride;

            /*-----------------------------------------------------------------------------
              I added the colorFlag line above for the case were the dimstyle specifies
              a text color, but the TextStyle does not.  In that case, the dimensions need
              to be protected from the textstyle.  If I don't turn on the TSO for colorFlag,
              then saving the text style would turn OFF the pDim->text.color which causes
              the text color to inherit from the dhdr.
            -----------------------------------------------------------------------------*/
            textStyle = DgnTextStyle::Create(*m_project);
            DgnTextStylePersistence::Legacy::FromLegacyStyle(*textStyle, localTextStyle);
            textStyle->SetId (m_textStyle->GetId());
            textStyle->SetName (m_textStyle->GetName().c_str());
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionStyle::_SetTextStyleProp (DgnTextStyleCR textStyle, DimStyleProp iProperty)
    {
    StatusInt   status = SUCCESS;

    switch (iProperty)
        {
        case DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE:
            {
            // Copy the input style into the member variable
            m_textStyle->CopyPropertyValuesFrom(textStyle);

            // Tweak the member variable through this pointer
            LegacyTextStyle  memberTextStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*m_textStyle);

            /* The following properties are stored in the main part of the dimStyle
               because they existed BEFORE the text style was stored in the dimStyle */
            m_data.ad4.dimfont                      = memberTextStyle.fontNo;
            m_data.ad2.txheight                     = memberTextStyle.height;
            m_data.ad5.textWidth                    = memberTextStyle.width;
            m_data.ad4.dimtxt_color                 = memberTextStyle.color;
            m_data.ad4.ext_dimflg.underlineText     = memberTextStyle.flags.underline;
            m_data.ad5.flags.useStackedFractions    = (UShort) memberTextStyle.flags.fractions;

            m_data.ad4.ext_dimflg.fontOverride      = memberTextStyle.overrideFlags.fontNo;
            m_data.ad4.ext_dimflg.textSizeOverride  = memberTextStyle.overrideFlags.height;
            m_data.ad5.flags.useWidth               = memberTextStyle.overrideFlags.width;
            m_data.ad4.ext_dimflg.textColorOverride = memberTextStyle.overrideFlags.color;
            m_data.ad6.flags.underlineOverride      = memberTextStyle.overrideFlags.underline;
            m_data.ad6.flags.fractionOverride       = memberTextStyle.overrideFlags.fractions;

            memberTextStyle.fontNo                 = 0;
            memberTextStyle.height                 = 0.0;
            memberTextStyle.width                  = 0.0;
            memberTextStyle.color                  = 0;
            memberTextStyle.flags.underline        = false;
            memberTextStyle.flags.fractions        = false;

            // The textstyle override flags on the dimension style are meaningless and unused.
            memset(&memberTextStyle.overrideFlags, 0, sizeof (memberTextStyle.overrideFlags));

            DgnTextStylePtr fileStyle = (textStyle.GetName().empty() ? NULL : GetDgnProject()->Styles().TextStyles().QueryByName(textStyle.GetName().c_str()));
            if (fileStyle.IsValid())
                {
                m_textStyle->SetName (textStyle.GetName().c_str());
                m_textStyle->SetId (textStyle.GetId());
                }
            break;
            }
        default:
            {
            status = STYLETABLE_ERROR_BadName;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::SetTemplateData (DimTemplate const& data, DimensionType dimensionType)
    {
    int templateIndex = static_cast<int>(dimensionType) - 1;
    if (0 > templateIndex || 24 <= templateIndex)
        { BeAssert (false); return; }

    memcpy (&m_data.ad4.dim_template[templateIndex], &data, sizeof (DimTemplate));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/11
+---------------+---------------+---------------+---------------+---------------+------*/
UShort          DimensionStyle::GetDwgSpecificFlags (UShort& primaryUnits, UShort& secondaryUnits) const
    {
    primaryUnits = m_extensions.dwgSpecifics.flags.uPrimaryUnit;
    secondaryUnits = m_extensions.dwgSpecifics.flags.uSecondaryUnit;

    UShort      dwgFlags = 0;
    memcpy (&dwgFlags, &m_extensions.dwgSpecifics.flags, sizeof(dwgFlags));

    return  dwgFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::SetDwgSpecificFlags (UShort flags)
    {
    memcpy (&m_extensions.dwgSpecifics.flags, &flags, sizeof(m_extensions).dwgSpecifics.flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::SetDwgSpecificFlags (UShort primaryUnits, UShort secondaryUnits)
    {
    m_extensions.dwgSpecifics.flags.uPrimaryUnit = primaryUnits;
    m_extensions.dwgSpecifics.flags.uSecondaryUnit = secondaryUnits;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public PropToOverrideMap  *BentleyApi::dgnDimStyle_getPropToOverridesMap
(
int    *pNumMaps   // <=
)
    {
    static  int                 s_numMaps = 0;
    static  PropToOverrideMap  *s_propToOverridesMap = NULL;

    if (NULL == s_propToOverridesMap)
        {
#if 0 // WIP_BEFILENAME_PORTABILITY
        RscFileHandle rscFileHandle = g_dgnHandlersResources->GetRscFileHandle();

        DimStylePropToOverrideMapRsc*   pRsc = NULL;

        if (NULL != (pRsc = (DimStylePropToOverrideMapRsc *) RmgrResourceMT::Load (rscFileHandle, RTYPE_DimStylePropToOverrideMap, RSCID_DIMSTYLE_PROPTOOVERRIDES)))
            {
            int mapBytes = pRsc->nMaps * sizeof(pRsc)->map[0];

            s_numMaps            = pRsc->nMaps;
            s_propToOverridesMap = (PropToOverrideMap *) malloc (mapBytes);

            memcpy (s_propToOverridesMap, pRsc->map, mapBytes);
            }
#endif
        }

    if (pNumMaps)
        *pNumMaps = s_numMaps;

    return s_propToOverridesMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::GetOverrideProp (DimStyleProp* pOverrideProp, bool* pInverted, DimStyleProp m_property)
    {
    int                 iProp, numProps;
    BentleyStatus       status = ERROR;
    DimStyleProp        overrideProp = DIMSTYLE_PROP_Invalid;
    bool                inverted = false;
    PropToOverrideMap  *propToOverridesMap = dgnDimStyle_getPropToOverridesMap (&numProps);

    for (iProp = 0; iProp < numProps; iProp++)
        {
        if (propToOverridesMap[iProp].m_property == m_property)
            {
            overrideProp    = propToOverridesMap[iProp].m_override;
            inverted        = TO_BOOL (propToOverridesMap[iProp].inverted);
            status = SUCCESS;
            break;
            }
        }

    if (SUCCESS == status)
        {
        if (NULL != pOverrideProp)
            *pOverrideProp = overrideProp;

        if (NULL != pInverted)
            *pInverted = inverted;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionStyle::GetOverriddenProp (DimStyleProp* pProperty, bool* pInverted, DimStyleProp overrideProp)
    {
    int                 iProp, numProps;
    bool                inverted = false;
    BentleyStatus       status = ERROR;
    DimStyleProp        overriddenProp = DIMSTYLE_PROP_Invalid;
    PropToOverrideMap  *propToOverridesMap = dgnDimStyle_getPropToOverridesMap (&numProps);

    for (iProp = 0; iProp < numProps; iProp++)
        {
        if (propToOverridesMap[iProp].m_override == overrideProp)
            {
            overriddenProp    = propToOverridesMap[iProp].m_property;
            inverted    = TO_BOOL (propToOverridesMap[iProp].inverted);

            status = SUCCESS;
            break;
            }
        }

    if (SUCCESS == status)
        {
        if (NULL != pProperty)
            *pProperty = overriddenProp;

        if (NULL != pInverted)
            *pInverted = inverted;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static  StringList*   loadStringList (UInt32 stringListID)
    {
    /*------------------------------------------------------------------------------------
      This is a very leaky abstraction.  Would be better to either:
        a) Introduce an API to expose the dimstyle property meta-data as collections/iterators
           down here in DgnPlatform, or
        b) Move all the stringlists out of DgnPlatform altogether.

      Option b would work especially well if we convert code that uses the stringlists over
      to using reflection on ECClasses.  But there are no ECClasses for DimStyle properties
      yet.  Maybe we could use the string lists to generate the classes (at build-time)
      similiar to how we generate the command tables in the DimStyle application.  JS 04/12
    ------------------------------------------------------------------------------------*/

#ifdef DGNV10FORMAT_CHANGES_WIP
    RscFileHandle rscFileHandle = g_dgnHandlersResources->GetRscFileHandle();
    return mdlStringList_loadResource (rscFileHandle, stringListID);
#endif
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StringList*     DimensionStyle::GetPropValuesStringList (DimStyleProp_ValueList stringListID)
    {
    return loadStringList (stringListID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StringList*     DimensionStyle::GetPropCategoryStringList (DimStyleProp_Category stringListID)
    {
    return loadStringList (stringListID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionStyle::UpdateTextStyleParams (bool* pAnyChange, DgnTextStyleCR textStyleIn)
    {
    DgnTextStylePtr     textStyleFromDimStyle;
    DgnTextStylePtr     originalTextStyleFromDimStyle;

    // Get the text style out of the dimension style
    GetTextStyleProp (textStyleFromDimStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE);

    // Make a copy of the original text style for later use
    if (pAnyChange)
        originalTextStyleFromDimStyle = textStyleFromDimStyle->Clone();

    double height = 0.0;
    textStyleIn.GetPropertyValue (DgnTextStyleProperty::Height, height);

    bool hasHeight = height > 0.0;
    SetBooleanProp (!hasHeight, DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT);

    if (hasHeight)
        SetDoubleProp (height, DIMSTYLE_PROP_Text_Height_DOUBLE);

    /* update the text style respecting existing overrides */
    LegacyTextStyle v8style = DgnTextStylePersistence::Legacy::ToLegacyStyle(*textStyleFromDimStyle);
    DgnTextStylePropertyMaskPtr overrides = DgnTextStylePersistence::Legacy::FromLegacyMask(v8style.overrideFlags);
    textStyleFromDimStyle->CopyPropertyValuesFrom (textStyleIn, *overrides);

    // If requested, test to see if the text style was changed
    if (pAnyChange)
        {
        DgnTextStylePropertyMaskPtr compareMask = originalTextStyleFromDimStyle->Compare (*textStyleFromDimStyle);

        if (compareMask->AreAnyPropertiesSet())
            *pAnyChange = true;
        }

    // Push the updated text style back into the dimension style
    SetTextStyleProp (*textStyleFromDimStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimStyleDgnCacheLoaderCollection::AddToFile (DgnProjectR file) const
    {
    DgnModelR nonModel = file.GetDictionaryModel();
    if (!nonModel.IsFillInProgress())
        return ERROR;

    MSElementDescrP tableDescr  = NULL;
    if (SUCCESS != DgnTableUtilities::CreateTable (&tableDescr,  MS_DIMSTYLE_TABLE_LEVEL, sizeof (DimStyleTableElm), 0))
        return ERROR;

    EditElementHandle tableElement (tableDescr, true, false, &nonModel);

    for (DimensionStylePtr const& style : m_styles)
        {
        EditElementHandle  newElm;
        style->ToElement (newElm);
        MSElementDescrP newEdP   = newElm.ExtractElementDescr();
        newEdP->el.ehdr.uniqueId = style->GetID();
        if (newEdP->el.ehdr.uniqueId == m_settingsElement)
            DgnTableUtilities::SetEntryId (newEdP->el, DIMSTYLE_ACTIVE_STYLE_ID);

        tableElement.GetElementDescrP()->AppendDescr (newEdP);
        }

    return (BentleyStatus)dgnModel_loadDscrFromFile (&nonModel, tableElement.GetElementDescrP(), 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimStyleDgnCacheLoaderCollection::Add (DimensionStyleR style, ElementId id, bool isSettings)
    {
    style.SetElemID (id);
    if (isSettings)
        {
        if (INVALID_ELEMENTID != m_settingsElement)
            return ERROR;
        m_settingsElement = id;
        }

    m_styles.push_back(&style);
    return SUCCESS;
    }
#endif
