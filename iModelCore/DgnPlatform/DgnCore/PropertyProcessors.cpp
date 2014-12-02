/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PropertyProcessors.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
*
*   PropertyCollection
*
+---------------+---------------+---------------+---------------+---------------+------*/
/*------------------------------------------------------------23---------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddPropertyTableCollection
(
PropertyCollection const&       c2
)
    {
    for (T_TableCollection::const_iterator iter = c2.m_tableCollection.begin(); iter != c2.m_tableCollection.end (); ++iter)
        AddPropertyTable (iter->second, iter->first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCollection::PropertyCollection
(
PropertyCollection const&       c2
)
    {
    AddPropertyTableCollection (c2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCollection& PropertyCollection::operator= (PropertyCollection const& c2)
    {
    if (&c2 == this)
        return *this;

    ReleaseAllTables ();
    AddPropertyTableCollection (c2);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::ReleaseAllTables ()
    {
    T_TableCollection::iterator iter = m_tableCollection.begin();

    for (; iter != m_tableCollection.end (); ++iter)
        iter->second->Release();

    m_tableCollection.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddPropertyTable (IRefCountedP table, UInt32 key)
    {
    table->AddRef();

    m_tableCollection[key] = table;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IRefCountedP PropertyCollection::FindPropertyTable (UInt32 tableKey)
    {
    T_TableCollection::iterator iter = m_tableCollection.find (tableKey);

    if (iter != m_tableCollection.end ())
        return iter->second;

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::RemovePropertyTable (UInt32 tableKey)
    {
    T_TableCollection::iterator iter = m_tableCollection.find (tableKey);
    if (iter == m_tableCollection.end())
        return ERROR;

    iter->second->Release();
    m_tableCollection.erase (tableKey);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    PropertyCollection::IsEmpty ()
    {
    if (m_tableCollection.empty())
        return true;

    T_TableCollection::const_iterator   iter;

    for (iter = m_tableCollection.begin(); iter != m_tableCollection.end(); iter++)
        {
        IRefCountedP    table = iter->second;

        if (NULL == table)
            { BeAssert (0); continue; }

        switch (iter->first)
            {
            case ELEMENT_PROPERTY_Level:
            case ELEMENT_PROPERTY_Color:
            case ELEMENT_PROPERTY_Font:
            case ELEMENT_PROPERTY_TextStyle:
            case ELEMENT_PROPERTY_Weight:
                {
                T_UInt32Table*   propTable = dynamic_cast <T_UInt32Table *> (table);

                if (NULL == propTable)
                    { BeAssert (0); continue; }

                if (false == propTable->IsEmpty())
                    return false;

                break;
                }
            case ELEMENT_PROPERTY_Linestyle:
            case ELEMENT_PROPERTY_ElementClass:
            case ELEMENT_PROPERTY_DisplayPriority:
                {
                T_Int32Table*   propTable = dynamic_cast <T_Int32Table *> (table);

                if (NULL == propTable)
                    { BeAssert (0); continue; }

                if (false == propTable->IsEmpty())
                    return false;

                break;
                }
            case ELEMENT_PROPERTY_DimStyle:
            case ELEMENT_PROPERTY_MLineStyle:
            case ELEMENT_PROPERTY_Material:
            case ELEMENT_PROPERTY_ElementTemplate:
                {
                T_ElementIDTable*   propTable = dynamic_cast <T_ElementIDTable *> (table);

                if (NULL == propTable)
                    { BeAssert (0); continue; }

                if (false == propTable->IsEmpty())
                    return false;

                break;
                }
            case ELEMENT_PROPERTY_Transparency:
                {
                T_DoubleTable*   propTable = dynamic_cast <T_DoubleTable *> (table);

                if (NULL == propTable)
                    { BeAssert (0); continue; }

                if (false == propTable->IsEmpty())
                    return false;

                break;
                }
            default:
                {
                // We don't know the format of other tables, so assume they are not empty
                return false;
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddElementIDEntry (ElementId key, ElementId value, UInt32 flags, ElementProperties tableKey)
    {
    IRefCountedP        table;
    T_ElementIDTable*   propertyTable;

    if (NULL != (table = FindPropertyTable (tableKey)))
        {
        propertyTable = dynamic_cast <T_ElementIDTable *> (table);
        }
    else
        {
        propertyTable = new T_ElementIDTable ();

        AddPropertyTable (propertyTable, tableKey);
        }

    if (NULL == propertyTable)
        { BeAssert (0); return; }

    propertyTable->AddProperty (key, value, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddMaterialIdEntry (UInt64/*DgnMaterialId*/ key, UInt64/*DgnMaterialId*/ value, UInt32 flags, ElementProperties tableKey)
    {
    IRefCountedP                                        table;
    MaterialIdPropertyTable*     propertyTable;

    if (NULL != (table = FindPropertyTable (tableKey)))
        {
        propertyTable = dynamic_cast <MaterialIdPropertyTable *> (table);
        }
    else
        {
        propertyTable = new MaterialIdPropertyTable ();

        AddPropertyTable (propertyTable, tableKey);
        }

    if (NULL == propertyTable)
        { BeAssert (0); return; }

    propertyTable->AddProperty (key, value, flags);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddInt32Entry (Int32 key, Int32 value, UInt32 flags, ElementProperties tableKey)
    {
    IRefCountedP    table;
    T_Int32Table*   propertyTable;

    if (NULL != (table = FindPropertyTable (tableKey)))
        {
        propertyTable = dynamic_cast <T_Int32Table *> (table);
        }
    else
        {
        propertyTable = new T_Int32Table ();

        AddPropertyTable (propertyTable, tableKey);
        }

    if (NULL == propertyTable)
        { BeAssert (0); return; }

    propertyTable->AddProperty (key, value, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddUInt32Entry (UInt32 key, UInt32 value, UInt32 flags, ElementProperties tableKey)
    {
    IRefCountedP    table;
    T_UInt32Table*  propertyTable;

    if (NULL != (table = FindPropertyTable (tableKey)))
        {
        propertyTable = dynamic_cast <T_UInt32Table *> (table);
        }
    else
        {
        propertyTable = new T_UInt32Table ();

        AddPropertyTable (propertyTable, tableKey);
        }

    if (NULL == propertyTable)
        { BeAssert (0); return; }

    propertyTable->AddProperty (key, value, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddDoubleEntry (double key, double value, UInt32 flags, ElementProperties tableKey)
    {
    IRefCountedP    table;
    T_DoubleTable*  propertyTable;

    if (NULL != (table = FindPropertyTable (tableKey)))
        {
        propertyTable = dynamic_cast <T_DoubleTable *> (table);
        }
    else
        {
        propertyTable = new T_DoubleTable ();

        AddPropertyTable (propertyTable, tableKey);
        }

    if (NULL == propertyTable)
        { BeAssert (0); return; }

    propertyTable->AddProperty (key, value, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::FindElementIDEntry (ElementId* value, UInt32* flags, ElementId key, ElementProperties tableKey)
    {
    IRefCountedP    table;

    if (NULL == (table = FindPropertyTable (tableKey)))
        return ERROR;

    T_ElementIDTable *  propertyTable = dynamic_cast <T_ElementIDTable *> (table);

    if (NULL == propertyTable)
        { BeAssert (0); return ERROR; }

    return propertyTable->FindProperty (value, flags, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::FindMaterialIdEntry(UInt64/*DgnMaterialId*/* value, UInt32* flags, UInt64/*DgnMaterialId*/ key, ElementProperties tableKey)
    {
    IRefCountedP    table;

    if (NULL == (table = FindPropertyTable (tableKey)))
        return ERROR;

    MaterialIdPropertyTable*  propertyTable = dynamic_cast <MaterialIdPropertyTable *> (table);

    if (NULL == propertyTable)
        { BeAssert (0); return ERROR; }

    return propertyTable->FindProperty (value, flags, key);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::FindInt32Entry (Int32* value, UInt32* flags, Int32 key, ElementProperties tableKey)
    {
    IRefCountedP    table;

    if (NULL == (table = FindPropertyTable (tableKey)))
        return ERROR;

    T_Int32Table *  propertyTable = dynamic_cast <T_Int32Table *> (table);

    if (NULL == propertyTable)
        { BeAssert (0); return ERROR; }

    return propertyTable->FindProperty (value, flags, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::FindUInt32Entry (UInt32* value, UInt32* flags, UInt32 key, ElementProperties tableKey)
    {
    IRefCountedP    table;

    if (NULL == (table = FindPropertyTable (tableKey)))
        return ERROR;

    T_UInt32Table *  propertyTable = dynamic_cast <T_UInt32Table *> (table);

    if (NULL == propertyTable)
        { BeAssert (0); return ERROR; }

    return propertyTable->FindProperty (value, flags, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::FindDoubleEntry (double* value, UInt32* flags, double key, ElementProperties tableKey)
    {
    IRefCountedP    table;

    if (NULL == (table = FindPropertyTable (tableKey)))
        return ERROR;

    T_DoubleTable *  propertyTable = dynamic_cast <T_DoubleTable *> (table);

    if (NULL == propertyTable)
        { BeAssert (0); return ERROR; }

    return propertyTable->FindProperty (value, flags, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddColor (UInt32 key, UInt32 value, UInt32 flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddLineStyle (Int32 key, Int32 value, UInt32 flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_Linestyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddLevel (LevelId key, LevelId value, UInt32 flags)
    {
    AddUInt32Entry (key.GetValue(), value.GetValue(), flags, ELEMENT_PROPERTY_Level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddFont (UInt32 key, UInt32 value, UInt32 flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Font);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddTextStyle(DgnStyleId key, DgnStyleId value, UInt32 flags)
    {
    AddUInt32Entry(key.GetValue(), value.GetValue(), flags, ELEMENT_PROPERTY_TextStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddDimStyle (ElementId key, ElementId value, UInt32 flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_DimStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddMLineStyle (ElementId key, ElementId value, UInt32 flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_MLineStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddMaterial(UInt64/*DgnMaterialId*/ key, UInt64/*DgnMaterialId*/ value, UInt32 flags)
    {
    AddMaterialIdEntry(key, value, flags, ELEMENT_PROPERTY_Material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddWeight (UInt32 key, UInt32 value, UInt32 flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddElementClass (Int32 key, Int32 value, UInt32 flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_ElementClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddTransparency (double key, double value, UInt32 flags)
    {
    AddDoubleEntry (key, value, flags, ELEMENT_PROPERTY_Transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddThickness (double key, double value, UInt32 flags)
    {
    AddDoubleEntry (key, value, flags, ELEMENT_PROPERTY_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddDisplayPriority (Int32 key, Int32 value, UInt32 flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_DisplayPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddElementTemplate (ElementId key, ElementId value, UInt32 flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_ElementTemplate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindColor (UInt32* value, UInt32* flags, UInt32 key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindLineStyle (Int32* value, UInt32* flags, Int32 key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_Linestyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindLevel (LevelId* value, UInt32* flags, LevelId key)
    {
    return FindUInt32Entry ((UInt32*) value, flags, key.GetValue(), ELEMENT_PROPERTY_Level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindFont (UInt32* value, UInt32* flags, UInt32 key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Font);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindTextStyle(DgnStyleId* value, UInt32* flags, DgnStyleId key)
    {
    UInt32 valueU32 = 0;
    StatusInt status = FindUInt32Entry((value ? &valueU32 : NULL), flags, key.GetValue(), ELEMENT_PROPERTY_TextStyle);

    if ((SUCCESS == status) && (NULL != value))
        *value = DgnStyleId(valueU32);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindDimStyle (ElementId* value, UInt32* flags, ElementId key)
    {
    return FindElementIDEntry (value, flags, key, ELEMENT_PROPERTY_DimStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindMLineStyle (ElementId* value, UInt32* flags, ElementId key)
    {
    return FindElementIDEntry (value, flags, key, ELEMENT_PROPERTY_MLineStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindMaterial (UInt64/*DgnMaterialId*/* value, UInt32* flags, UInt64/*DgnMaterialId*/ key)
    {
    return FindMaterialIdEntry (value, flags, key, ELEMENT_PROPERTY_Material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindWeight (UInt32* value, UInt32* flags, UInt32 key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindElementClass (Int32* value, UInt32* flags, Int32 key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_ElementClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindTransparency (double* value, UInt32* flags, double key)
    {
    return FindDoubleEntry (value, flags, key, ELEMENT_PROPERTY_Transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindThickness (double* value, UInt32* flags, double key)
    {
    return FindDoubleEntry (value, flags, key, ELEMENT_PROPERTY_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindDisplayPriority (Int32* value, UInt32* flags, Int32 key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_DisplayPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindElementTemplate (ElementId* value, UInt32* flags, ElementId key)
    {
    return FindElementIDEntry (value, flags, key, ELEMENT_PROPERTY_ElementTemplate);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */          StyleDependantRemapper::StyleDependantRemapper () : m_propertyMask(ELEMENT_PROPERTY_None) {}
ElementProperties   StyleDependantRemapper::_GetEditPropertiesMask ()    {return m_propertyMask;}
EditPropertyPurpose StyleDependantRemapper::_GetEditPropertiesPurpose () {return EditPropertyPurpose::Change;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachTextStyleCallback (EachTextStyleArg& arg)
    {
    UInt32   newStyleID;

    if (SUCCESS != m_textStyleMap.FindProperty (&newStyleID, NULL, arg.GetStoredValue()))
        return;

    arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
    arg.SetStoredValue (newStyleID);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachDimStyleCallback (EachDimStyleArg& arg)
    {
    ElementId   newStyleID;

    if (SUCCESS != m_dimStyleMap.FindProperty (&newStyleID, NULL, ElementId(arg.GetStoredValue())))
        return;

    arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
    arg.SetStoredValue (newStyleID.GetValue());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachMLineStyleCallback (EachMLineStyleArg& arg)
    {
    ElementId   newStyleID;

    if (SUCCESS != m_mlineStyleMap.FindProperty (&newStyleID, NULL, ElementId(arg.GetStoredValue())))
        return;

    arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
    arg.SetStoredValue (newStyleID.GetValue());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachLevelCallback (EachLevelArg& arg)
    {
    LevelId   newLevelID;

    if (SUCCESS != m_levelMap.FindProperty (&newLevelID, NULL, arg.GetStoredValue()))
        return;

    arg.SetStoredValue (newLevelID);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddTextStyleRemap (UInt32 fromID, UInt32 toID)
    {
    m_textStyleMap.AddProperty (fromID, toID, 0);

    m_propertyMask  = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_TextStyle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddDimStyleRemap (ElementId fromID, ElementId toID)
    {
    m_dimStyleMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_DimStyle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddMLineStyleRemap (ElementId fromID, ElementId toID)
    {
    m_mlineStyleMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_MLineStyle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddLevelRemap (LevelId fromID, LevelId toID)
    {
    m_levelMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_Level);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  StyleDependantRemapper::DoRemapping (DgnProjectR file)
    {
#ifdef WIP_VANCOUVER_MERGE // StyleDependantRemapper
    UInt32  count = 0;

    DgnFile::Collection collection  = file.GetAllElementsCollection();
    for each (PersistentElementRefP const& elemRef in collection)
        {
        EditElementHandle eeh (elemRef, elemRef->GetDgnModelP());
        if (PropertyContext::EditElementProperties (eeh, this))
            {
            eeh.ReplaceInModel (elemRef);
            count++;
            }
        }

    return count;
#endif
    BeAssert(false && "if we really have to iterate all elements, we have a lot of loading to do");
    return 0;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  StyleDependantRemapper::DoRemapping (DgnModelR model)
    {
    UInt32  count = 0;

#if defined (NEEDS_WORK_DGNITEM)
    DgnModel::ElementsCollection collection  = model.GetElementsCollection();
    for (auto elemRef : collection)
        {
        EditElementHandle eeh (elemRef);
        if (PropertyContext::EditElementProperties (eeh, this))
            {
            eeh.ReplaceInModel (elemRef);
            count++;
            }
        }
#endif

    return count;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool  StyleDependantRemapper::DoRemapping (EditElementHandleR eeh)
    {
    return PropertyContext::EditElementProperties (eeh, this);
    }
