/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PropertyProcessors.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
*
*   PropertyCollection
*
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
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
void PropertyCollection::AddPropertyTable (IRefCountedP table, uint32_t key)
    {
    table->AddRef();

    m_tableCollection[key] = table;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IRefCountedP PropertyCollection::FindPropertyTable (uint32_t tableKey)
    {
    T_TableCollection::iterator iter = m_tableCollection.find (tableKey);

    if (iter != m_tableCollection.end ())
        return iter->second;

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PropertyCollection::RemovePropertyTable (uint32_t tableKey)
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
            case ELEMENT_PROPERTY_Category:
            case ELEMENT_PROPERTY_Color:
            case ELEMENT_PROPERTY_Font:
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
void PropertyCollection::AddElementIDEntry (DgnElementId key, DgnElementId value, uint32_t flags, ElementProperties tableKey)
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
void PropertyCollection::AddMaterialIdEntry (uint64_t/*DgnMaterialId*/ key, uint64_t/*DgnMaterialId*/ value, uint32_t flags, ElementProperties tableKey)
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
void PropertyCollection::AddInt32Entry (int32_t key, int32_t value, uint32_t flags, ElementProperties tableKey)
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
void PropertyCollection::AddUInt32Entry (uint32_t key, uint32_t value, uint32_t flags, ElementProperties tableKey)
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
void PropertyCollection::AddDoubleEntry (double key, double value, uint32_t flags, ElementProperties tableKey)
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
StatusInt PropertyCollection::FindElementIDEntry (DgnElementId* value, uint32_t* flags, DgnElementId key, ElementProperties tableKey)
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
StatusInt PropertyCollection::FindMaterialIdEntry(uint64_t/*DgnMaterialId*/* value, uint32_t* flags, uint64_t/*DgnMaterialId*/ key, ElementProperties tableKey)
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
StatusInt PropertyCollection::FindInt32Entry (int32_t* value, uint32_t* flags, int32_t key, ElementProperties tableKey)
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
StatusInt PropertyCollection::FindUInt32Entry (uint32_t* value, uint32_t* flags, uint32_t key, ElementProperties tableKey)
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
StatusInt PropertyCollection::FindDoubleEntry (double* value, uint32_t* flags, double key, ElementProperties tableKey)
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
void PropertyCollection::AddColor (uint32_t key, uint32_t value, uint32_t flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddLineStyle (int32_t key, int32_t value, uint32_t flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_Linestyle);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddCategory (DgnCategoryId key, DgnCategoryId value, uint32_t flags)
    {
    AddUInt32Entry (key.GetValue(), value.GetValue(), flags, ELEMENT_PROPERTY_Category);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddFont (uint32_t key, uint32_t value, uint32_t flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Font);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddDimStyle (DgnElementId key, DgnElementId value, uint32_t flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_DimStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddMLineStyle (DgnElementId key, DgnElementId value, uint32_t flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_MLineStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddMaterial(uint64_t/*DgnMaterialId*/ key, uint64_t/*DgnMaterialId*/ value, uint32_t flags)
    {
    AddMaterialIdEntry(key, value, flags, ELEMENT_PROPERTY_Material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddWeight (uint32_t key, uint32_t value, uint32_t flags)
    {
    AddUInt32Entry (key, value, flags, ELEMENT_PROPERTY_Weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddElementClass (int32_t key, int32_t value, uint32_t flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_ElementClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddTransparency (double key, double value, uint32_t flags)
    {
    AddDoubleEntry (key, value, flags, ELEMENT_PROPERTY_Transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddThickness (double key, double value, uint32_t flags)
    {
    AddDoubleEntry (key, value, flags, ELEMENT_PROPERTY_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddDisplayPriority (int32_t key, int32_t value, uint32_t flags)
    {
    AddInt32Entry (key, value, flags, ELEMENT_PROPERTY_DisplayPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCollection::AddElementTemplate (DgnElementId key, DgnElementId value, uint32_t flags)
    {
    AddElementIDEntry (key, value, flags, ELEMENT_PROPERTY_ElementTemplate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindColor (uint32_t* value, uint32_t* flags, uint32_t key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindLineStyle (int32_t* value, uint32_t* flags, int32_t key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_Linestyle);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindCategory (DgnCategoryId* value, uint32_t* flags, DgnCategoryId key)
    {
    return FindUInt32Entry ((uint32_t*) value, flags, key.GetValue(), ELEMENT_PROPERTY_Category);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindFont (uint32_t* value, uint32_t* flags, uint32_t key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Font);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindDimStyle (DgnElementId* value, uint32_t* flags, DgnElementId key)
    {
    return FindElementIDEntry (value, flags, key, ELEMENT_PROPERTY_DimStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindMLineStyle (DgnElementId* value, uint32_t* flags, DgnElementId key)
    {
    return FindElementIDEntry (value, flags, key, ELEMENT_PROPERTY_MLineStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindMaterial (uint64_t/*DgnMaterialId*/* value, uint32_t* flags, uint64_t/*DgnMaterialId*/ key)
    {
    return FindMaterialIdEntry (value, flags, key, ELEMENT_PROPERTY_Material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindWeight (uint32_t* value, uint32_t* flags, uint32_t key)
    {
    return FindUInt32Entry (value, flags, key, ELEMENT_PROPERTY_Weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindElementClass (int32_t* value, uint32_t* flags, int32_t key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_ElementClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindTransparency (double* value, uint32_t* flags, double key)
    {
    return FindDoubleEntry (value, flags, key, ELEMENT_PROPERTY_Transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindThickness (double* value, uint32_t* flags, double key)
    {
    return FindDoubleEntry (value, flags, key, ELEMENT_PROPERTY_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindDisplayPriority (int32_t* value, uint32_t* flags, int32_t key)
    {
    return FindInt32Entry (value, flags, key, ELEMENT_PROPERTY_DisplayPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PropertyCollection::FindElementTemplate (DgnElementId* value, uint32_t* flags, DgnElementId key)
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
 void    StyleDependantRemapper::_EachDimStyleCallback (EachDimStyleArg& arg)
    {
    DgnElementId   newStyleID;

    if (SUCCESS != m_dimStyleMap.FindProperty (&newStyleID, NULL, DgnElementId ((int64_t) arg.GetStoredValue())))
        return;

    arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
    arg.SetStoredValue (newStyleID.GetValue());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachMLineStyleCallback (EachMLineStyleArg& arg)
    {
    DgnElementId   newStyleID;

    if (SUCCESS != m_mlineStyleMap.FindProperty (&newStyleID, NULL, DgnElementId ((int64_t) arg.GetStoredValue())))
        return;

    arg.SetRemappingAction (StyleParamsRemapping::ApplyStyle);
    arg.SetStoredValue (newStyleID.GetValue());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
 void    StyleDependantRemapper::_EachCategoryCallback (EachCategoryArg& arg)
    {
    DgnCategoryId   newCategoryID;

    if (SUCCESS != m_levelMap.FindProperty (&newCategoryID, NULL, arg.GetStoredValue()))
        return;

    arg.SetStoredValue (newCategoryID);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddDimStyleRemap (DgnElementId fromID, DgnElementId toID)
    {
    m_dimStyleMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_DimStyle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddMLineStyleRemap (DgnElementId fromID, DgnElementId toID)
    {
    m_mlineStyleMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_MLineStyle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    StyleDependantRemapper::AddCategoryRemap (DgnCategoryId fromID, DgnCategoryId toID)
    {
    m_levelMap.AddProperty (fromID, toID, 0);

    m_propertyMask = (ElementProperties) (m_propertyMask | ELEMENT_PROPERTY_Category);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t StyleDependantRemapper::DoRemapping (DgnDbR file)
    {
#ifdef WIP_VANCOUVER_MERGE // StyleDependantRemapper
    uint32_t count = 0;

    DgnFile::Collection collection  = file.GetAllElementsCollection();
    for each (PersistentDgnElementP const& element in collection)
        {
        EditElementHandle eeh (element, element->GetDgnModelP());
        if (PropertyContext::EditElementProperties (eeh, this))
            {
            eeh.ReplaceInModel (element);
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
uint32_t StyleDependantRemapper::DoRemapping (DgnModelR model)
    {
    uint32_t count = 0;

#if defined (NEEDS_WORK_DGNITEM)
    DgnModel::ElementsCollection collection  = model.GetElementsCollection();
    for (auto element : collection)
        {
        EditElementHandle eeh (element);
        if (PropertyContext::EditElementProperties (eeh, this))
            {
            eeh.ReplaceInModel (element);
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
