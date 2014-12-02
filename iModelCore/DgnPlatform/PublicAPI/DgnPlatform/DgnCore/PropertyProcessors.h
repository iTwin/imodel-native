/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PropertyProcessors.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <set>
#include <map>
#include <Bentley/RefCounted.h>
#include <RmgrTools/Tools/HeapzoneAllocator.h>
#include "../DgnPlatform.h"
#include "ElementHandle.h"
#include "Handler.h"
#include "PropertyContext.h"
#include "Material.h"

typedef std::set<Int32>     T_StdInt32Set;
typedef std::set<UInt32>    T_StdUInt32Set;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class T_Member> struct PropertyTableValue
    {
    T_Member    m_value;
    UInt32      m_flags;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class T_Key> struct PropertyTable : public RefCountedBase
    {
    private: std::map<T_Key, PropertyTableValue<T_Key> >   m_map;

    public:

    // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }
    
    //! @bsimethod                                                    JoshSchifter    01/07
    public:  template <class TRAVERSER> void     TraverseProperties (TRAVERSER& traverser)
        {
        typename std::map<T_Key, PropertyTableValue<T_Key> > :: const_iterator  iter;

        for (iter = m_map.begin(); iter != m_map.end(); iter++)
            {
            traverser.EachProperty (iter->first, iter->second.m_value, iter->second.m_flags);
            }
        }

    //! @bsimethod                                                    JoshSchifter    01/07
    public:  bool   IsEmpty () { return m_map.empty(); }

    //! @bsimethod                                                    JoshSchifter    01/07
    public:  void        AddProperty (T_Key key, T_Key value, UInt32 flags)
        {
        PropertyTableValue<T_Key>   entry;

        entry.m_value = value;
        entry.m_flags = flags;

        m_map[key] = entry;
        }

    //! @bsimethod                                                    JoshSchifter    01/07
    public:  StatusInt   FindProperty (T_Key* outValue, UInt32* outFlags, T_Key key)
        {
        typename std::map<T_Key, PropertyTableValue<T_Key> > :: const_iterator iter;

        iter = m_map.find(key);

        if (iter == m_map.end())
            return ERROR;

        if (outValue)
            *outValue = iter->second.m_value;

        if (outFlags)
            *outFlags = iter->second.m_flags;

        return SUCCESS;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MaterialIdPropertyTableValue
    {
    UInt64/*DgnMaterialId*/ m_value;
    UInt32      m_flags;
    };

//=======================================================================================
// @bsiclass                                                   
//=======================================================================================
//struct MaterialIdComparator
//    {
//    //! Compare material Ids
//    //! @param [in] id1         First material id to compare
//    //! @param [in] id2         Second material id to compare
//    //! @return True if id1 is less than id2
//    bool operator() (DgnMaterialId id1, DgnMaterialId id2) const
//        {
//        return id1.GetValue() < id2.GetValue();
//        }
//    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MaterialIdPropertyTable : public RefCountedBase
    {
    private: 
    
    typedef std::map<UInt64/*DgnMaterialId*/, MaterialIdPropertyTableValue /*, MaterialIdComparator*/> MaterialIdPropertyMap;
    MaterialIdPropertyMap   m_map;

    public:

    // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    PaulChater      10/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:  bool   IsEmpty () { return m_map.empty(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    PaulChater      10/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:  void        AddProperty (UInt64/*DgnMaterialId*/ key, UInt64/*DgnMaterialId*/ value, UInt32 flags)
        {
        MaterialIdPropertyTableValue   entry;

#ifdef WIP_VANCOUVER_MERGE // material
        entry.m_value.Copy (value);
#endif
        entry.m_value = value;
        entry.m_flags = flags;

        m_map[key] = entry;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    PaulChater      10/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:  StatusInt   FindProperty (UInt64/*DgnMaterialId*/* outValue, UInt32* outFlags, UInt64/*DgnMaterialId*/ key)
        {
        MaterialIdPropertyMap::const_iterator iter;

        iter = m_map.find(key);

        if (iter == m_map.end())
            return ERROR;

        if (outValue)
#ifdef WIP_VANCOUVER_MERGE // material
            outValue->Copy (iter->second.m_value);
#else
            *outValue = iter->second.m_value;
#endif

        if (outFlags)
            *outFlags = iter->second.m_flags;

        return SUCCESS;
        }
    };

typedef PropertyTable<ElementId>   T_ElementIDTable;
typedef PropertyTable<Int32>       T_Int32Table;
typedef PropertyTable<UInt32>      T_UInt32Table;
typedef PropertyTable<double>      T_DoubleTable;

typedef std::map<UInt32, IRefCountedP>   T_TableCollection;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyCollection
{
    private:    T_TableCollection   m_tableCollection;

    protected:  StatusInt           FindElementIDEntry (ElementId* value, UInt32* flags, ElementId key, ElementProperties tableKey);
    protected:  StatusInt           FindInt32Entry     (Int32*     value, UInt32* flags, Int32     key, ElementProperties tableKey);
    protected:  StatusInt           FindUInt32Entry    (UInt32*    value, UInt32* flags, UInt32    key, ElementProperties tableKey);
    protected:  StatusInt           FindDoubleEntry    (double*    value, UInt32* flags, double    key, ElementProperties tableKey);
    protected:  StatusInt           FindMaterialIdEntry(UInt64/*DgnMaterialId*/* value, UInt32* flags, UInt64/*DgnMaterialId*/ key, ElementProperties tableKey);

    protected:  void                AddElementIDEntry  (ElementId key, ElementId value, UInt32 flags, ElementProperties tableKey);
    protected:  void                AddInt32Entry      (Int32     key, Int32     value, UInt32 flags, ElementProperties tableKey);
    protected:  void                AddUInt32Entry     (UInt32    key, UInt32    value, UInt32 flags, ElementProperties tableKey);
    protected:  void                AddDoubleEntry     (double    key, double    value, UInt32 flags, ElementProperties tableKey);
    protected:  void                AddMaterialIdEntry (UInt64/*DgnMaterialId*/ key, UInt64/*DgnMaterialId*/ value, UInt32 flags, ElementProperties tableKey);

    public:     DGNPLATFORM_EXPORT                     PropertyCollection () {}
    public:     DGNPLATFORM_EXPORT                     PropertyCollection (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT PropertyCollection& operator= (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT virtual            ~PropertyCollection () { ReleaseAllTables(); }

    public:     DGNPLATFORM_EXPORT void                AddPropertyTableCollection (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT void                AddPropertyTable    (IRefCountedP table, UInt32 key);
    public:     DGNPLATFORM_EXPORT IRefCountedP        FindPropertyTable   (UInt32 tableKey);
    public:     DGNPLATFORM_EXPORT StatusInt           RemovePropertyTable (UInt32 tableKey);
    public:     DGNPLATFORM_EXPORT void                ReleaseAllTables    ();
    public:     DGNPLATFORM_EXPORT bool                IsEmpty();

    public:     DGNPLATFORM_EXPORT void                AddColor           (UInt32    key, UInt32    value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddLineStyle       (Int32     key, Int32     value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddLevel           (LevelId   key, LevelId   value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddFont            (UInt32    key, UInt32    value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddTextStyle       (DgnStyleId   key, DgnStyleId value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddDimStyle        (ElementId key, ElementId value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddMLineStyle      (ElementId key, ElementId value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddMaterial        (UInt64/*DgnMaterialId*/ key, UInt64/*DgnMaterialId*/ value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddWeight          (UInt32    key, UInt32    value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddElementClass    (Int32     key, Int32     value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddTransparency    (double    key, double    value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddThickness       (double    key, double    value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddDisplayPriority (Int32     key, Int32     value, UInt32 flags);
    public:     DGNPLATFORM_EXPORT void                AddElementTemplate (ElementId key, ElementId value, UInt32 flags);

    public:     DGNPLATFORM_EXPORT StatusInt           FindColor          (UInt32*    value, UInt32* flags, UInt32    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindLineStyle      (Int32*     value, UInt32* flags, Int32     key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindLevel          (LevelId*   value, UInt32* flags, LevelId   key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindFont           (UInt32*    value, UInt32* flags, UInt32    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindTextStyle      (DgnStyleId* value, UInt32* flags, DgnStyleId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindDimStyle       (ElementId* value, UInt32* flags, ElementId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindMLineStyle     (ElementId* value, UInt32* flags, ElementId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindMaterial       (UInt64/*DgnMaterialId*/* value, UInt32* flags, UInt64/*DgnMaterialId*/ key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindWeight         (UInt32*    value, UInt32* flags, UInt32    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindElementClass   (Int32*     value, UInt32* flags, Int32     key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindTransparency   (double*    value, UInt32* flags, double    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindThickness      (double*    value, UInt32* flags, double    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindDisplayPriority(Int32*     value, UInt32* flags, Int32     key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindElementTemplate(ElementId* value, UInt32* flags, ElementId key);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ProcessPropertiesFilter : public IQueryProperties, public IEditProperties
{
protected:

IProcessProperties*     m_callbackObj;
IQueryProperties*       m_queryObj;
IEditProperties*        m_editObj;

public:

ProcessPropertiesFilter (IQueryProperties* queryObj) {m_callbackObj = m_queryObj = queryObj; m_editObj  = NULL;}
ProcessPropertiesFilter (IEditProperties*  editObj)  {m_callbackObj = m_editObj  = editObj;  m_queryObj = NULL;}

virtual void _EachLevelCallback           (EachLevelArg& arg) override            {m_callbackObj->_EachLevelCallback (arg);}
virtual void _EachColorCallback           (EachColorArg& arg) override            {m_callbackObj->_EachColorCallback (arg);}
virtual void _EachLineStyleCallback       (EachLineStyleArg& arg) override        {m_callbackObj->_EachLineStyleCallback (arg);}
virtual void _EachFontCallback            (EachFontArg& arg) override             {m_callbackObj->_EachFontCallback (arg);}
virtual void _EachTextStyleCallback       (EachTextStyleArg& arg) override        {m_callbackObj->_EachTextStyleCallback (arg);}
virtual void _EachDimStyleCallback        (EachDimStyleArg& arg) override         {m_callbackObj->_EachDimStyleCallback (arg);}
virtual void _EachMLineStyleCallback      (EachMLineStyleArg& arg) override       {m_callbackObj->_EachMLineStyleCallback (arg);}
virtual void _EachMaterialCallback        (EachMaterialArg& arg) override         {m_callbackObj->_EachMaterialCallback (arg);}
virtual void _EachWeightCallback          (EachWeightArg& arg) override           {m_callbackObj->_EachWeightCallback (arg);}
virtual void _EachElementClassCallback    (EachElementClassArg& arg) override     {m_callbackObj->_EachElementClassCallback (arg);}
virtual void _EachTransparencyCallback    (EachTransparencyArg& arg) override     {m_callbackObj->_EachTransparencyCallback (arg);}
virtual void _EachThicknessCallback       (EachThicknessArg& arg) override        {m_callbackObj->_EachThicknessCallback (arg);;}
virtual void _EachDisplayPriorityCallback (EachDisplayPriorityArg& arg) override  {m_callbackObj->_EachDisplayPriorityCallback (arg);}
virtual void _EachElementTemplateCallback (EachElementTemplateArg& arg) override  {m_callbackObj->_EachElementTemplateCallback (arg);}

virtual ElementProperties       _GetQueryPropertiesMask      () override {return m_queryObj->_GetQueryPropertiesMask ();}
virtual QueryPropertyPurpose    _GetQueryPropertiesPurpose   () override {return m_queryObj->_GetQueryPropertiesPurpose ();}

virtual ElementProperties       _GetEditPropertiesMask       () override {return m_editObj->_GetEditPropertiesMask ();}
virtual EditPropertyPurpose     _GetEditPropertiesPurpose    () override {return m_editObj->_GetEditPropertiesPurpose ();}
virtual DgnModelP            _GetDestinationDgnModel      () override {return m_editObj->_GetDestinationDgnModel ();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StyleDependantRemapper : public DgnPlatform::IEditProperties
    {
private:
    ElementProperties           m_propertyMask;
    PropertyTable<UInt32>       m_textStyleMap;
    PropertyTable<ElementId>    m_dimStyleMap;
    PropertyTable<ElementId>    m_mlineStyleMap;
    PropertyTable<LevelId>      m_levelMap;

    virtual ElementProperties   _GetEditPropertiesMask () override;
    virtual EditPropertyPurpose _GetEditPropertiesPurpose () override;

    virtual void    _EachTextStyleCallback  (EachTextStyleArg& arg)  override;
    virtual void    _EachDimStyleCallback   (EachDimStyleArg& arg)   override;
    virtual void    _EachMLineStyleCallback (EachMLineStyleArg& arg) override;
    virtual void    _EachLevelCallback      (EachLevelArg& arg)      override;

public:
    DGNPLATFORM_EXPORT   /* ctor */  StyleDependantRemapper ();
    DGNPLATFORM_EXPORT   void        AddTextStyleRemap   (UInt32    fromID, UInt32    toID);
    DGNPLATFORM_EXPORT   void        AddDimStyleRemap    (ElementId fromID, ElementId toID);
    DGNPLATFORM_EXPORT   void        AddMLineStyleRemap  (ElementId fromID, ElementId toID);
    DGNPLATFORM_EXPORT   void        AddLevelRemap       (LevelId   fromID, LevelId   toID);

                         ElementProperties  GetEditPropertiesMask () { return _GetEditPropertiesMask(); }

    DGNPLATFORM_EXPORT   UInt32             DoRemapping (DgnProjectR);
    DGNPLATFORM_EXPORT   UInt32             DoRemapping (DgnModelR);
    DGNPLATFORM_EXPORT   bool               DoRemapping (EditElementHandleR);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

