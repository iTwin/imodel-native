/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PropertyProcessors.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <set>
#include <map>
#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"
#include "ElementHandle.h"
#include "ElementHandler.h"
#include "PropertyContext.h"
#include "Material.h"

typedef std::set<int32_t>     T_StdInt32Set;
typedef std::set<uint32_t>    T_StdUInt32Set;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class T_Member> struct PropertyTableValue
    {
    T_Member    m_value;
    uint32_t    m_flags;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class T_Key> struct PropertyTable : public RefCountedBase
    {
    private: std::map<T_Key, PropertyTableValue<T_Key> >   m_map;

    public:

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS 
    
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
    public:  void        AddProperty (T_Key key, T_Key value, uint32_t flags)
        {
        PropertyTableValue<T_Key>   entry;

        entry.m_value = value;
        entry.m_flags = flags;

        m_map[key] = entry;
        }

    //! @bsimethod                                                    JoshSchifter    01/07
    public:  StatusInt   FindProperty (T_Key* outValue, uint32_t* outFlags, T_Key key)
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
    uint64_t/*DgnMaterialId*/ m_value;
    uint32_t    m_flags;
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
    
    typedef std::map<uint64_t/*DgnMaterialId*/, MaterialIdPropertyTableValue /*, MaterialIdComparator*/> MaterialIdPropertyMap;
    MaterialIdPropertyMap   m_map;

    public:

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS 
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    PaulChater      10/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:  bool   IsEmpty () { return m_map.empty(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    PaulChater      10/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:  void        AddProperty (uint64_t/*DgnMaterialId*/ key, uint64_t/*DgnMaterialId*/ value, uint32_t flags)
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
    public:  StatusInt   FindProperty (uint64_t/*DgnMaterialId*/* outValue, uint32_t* outFlags, uint64_t/*DgnMaterialId*/ key)
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

typedef PropertyTable<DgnElementId> T_ElementIDTable;
typedef PropertyTable<int32_t>      T_Int32Table;
typedef PropertyTable<uint32_t>     T_UInt32Table;
typedef PropertyTable<double>       T_DoubleTable;

typedef std::map<uint32_t, IRefCountedP>   T_TableCollection;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyCollection
{
    private:    T_TableCollection   m_tableCollection;

    protected:  StatusInt           FindElementIDEntry (DgnElementId* value, uint32_t* flags, DgnElementId key, ElementProperties tableKey);
    protected:  StatusInt           FindInt32Entry     (int32_t*     value, uint32_t* flags, int32_t   key, ElementProperties tableKey);
    protected:  StatusInt           FindUInt32Entry    (uint32_t*    value, uint32_t* flags, uint32_t  key, ElementProperties tableKey);
    protected:  StatusInt           FindDoubleEntry    (double*    value, uint32_t* flags, double    key, ElementProperties tableKey);
    protected:  StatusInt           FindMaterialIdEntry(uint64_t/*DgnMaterialId*/* value, uint32_t* flags, uint64_t/*DgnMaterialId*/ key, ElementProperties tableKey);

    protected:  void                AddElementIDEntry  (DgnElementId key, DgnElementId value, uint32_t flags, ElementProperties tableKey);
    protected:  void                AddInt32Entry      (int32_t   key, int32_t   value, uint32_t flags, ElementProperties tableKey);
    protected:  void                AddUInt32Entry     (uint32_t  key, uint32_t  value, uint32_t flags, ElementProperties tableKey);
    protected:  void                AddDoubleEntry     (double    key, double    value, uint32_t flags, ElementProperties tableKey);
    protected:  void                AddMaterialIdEntry (uint64_t/*DgnMaterialId*/ key, uint64_t/*DgnMaterialId*/ value, uint32_t flags, ElementProperties tableKey);

    public:     DGNPLATFORM_EXPORT                     PropertyCollection () {}
    public:     DGNPLATFORM_EXPORT                     PropertyCollection (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT PropertyCollection& operator= (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT virtual            ~PropertyCollection () { ReleaseAllTables(); }

    public:     DGNPLATFORM_EXPORT void                AddPropertyTableCollection (PropertyCollection const&);
    public:     DGNPLATFORM_EXPORT void                AddPropertyTable    (IRefCountedP table, uint32_t key);
    public:     DGNPLATFORM_EXPORT IRefCountedP        FindPropertyTable   (uint32_t tableKey);
    public:     DGNPLATFORM_EXPORT StatusInt           RemovePropertyTable (uint32_t tableKey);
    public:     DGNPLATFORM_EXPORT void                ReleaseAllTables    ();
    public:     DGNPLATFORM_EXPORT bool                IsEmpty();

    public:     DGNPLATFORM_EXPORT void                AddColor           (uint32_t     key, uint32_t   value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddLineStyle       (int32_t      key, int32_t    value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddCategory        (DgnCategoryId key, DgnCategoryId value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddFont            (uint32_t     key, uint32_t   value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddDimStyle        (DgnElementId    key, DgnElementId  value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddMLineStyle      (DgnElementId    key, DgnElementId  value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddMaterial        (uint64_t/*DgnMaterialId*/ key, uint64_t/*DgnMaterialId*/ value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddWeight          (uint32_t     key, uint32_t   value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddElementClass    (int32_t      key, int32_t    value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddTransparency    (double       key, double     value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddThickness       (double       key, double     value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddDisplayPriority (int32_t      key, int32_t    value, uint32_t flags);
    public:     DGNPLATFORM_EXPORT void                AddElementTemplate (DgnElementId    key, DgnElementId  value, uint32_t flags);

    public:     DGNPLATFORM_EXPORT StatusInt           FindColor          (uint32_t*    value, uint32_t* flags, uint32_t  key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindLineStyle      (int32_t*     value, uint32_t* flags, int32_t   key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindCategory       (DgnCategoryId* value, uint32_t* flags, DgnCategoryId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindFont           (uint32_t*    value, uint32_t* flags, uint32_t  key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindDimStyle       (DgnElementId* value, uint32_t* flags, DgnElementId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindMLineStyle     (DgnElementId* value, uint32_t* flags, DgnElementId key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindMaterial       (uint64_t/*DgnMaterialId*/* value, uint32_t* flags, uint64_t/*DgnMaterialId*/ key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindWeight         (uint32_t*    value, uint32_t* flags, uint32_t  key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindElementClass   (int32_t*     value, uint32_t* flags, int32_t   key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindTransparency   (double*    value, uint32_t* flags, double    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindThickness      (double*    value, uint32_t* flags, double    key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindDisplayPriority(int32_t*     value, uint32_t* flags, int32_t   key);
    public:     DGNPLATFORM_EXPORT StatusInt           FindElementTemplate(DgnElementId* value, uint32_t* flags, DgnElementId key);
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

virtual void _EachCategoryCallback        (EachCategoryArg& arg) override         {m_callbackObj->_EachCategoryCallback (arg);}
virtual void _EachColorCallback           (EachColorArg& arg) override            {m_callbackObj->_EachColorCallback (arg);}
virtual void _EachLineStyleCallback       (EachLineStyleArg& arg) override        {m_callbackObj->_EachLineStyleCallback (arg);}
virtual void _EachFontCallback            (EachFontArg& arg) override             {m_callbackObj->_EachFontCallback (arg);}
virtual void _EachDimStyleCallback        (EachDimStyleArg& arg) override         {m_callbackObj->_EachDimStyleCallback (arg);}
virtual void _EachMLineStyleCallback      (EachMLineStyleArg& arg) override       {m_callbackObj->_EachMLineStyleCallback (arg);}
virtual void _EachMaterialCallback        (EachMaterialArg& arg) override         {m_callbackObj->_EachMaterialCallback (arg);}
virtual void _EachWeightCallback          (EachWeightArg& arg) override           {m_callbackObj->_EachWeightCallback (arg);}
virtual void _EachTransparencyCallback    (EachTransparencyArg& arg) override     {m_callbackObj->_EachTransparencyCallback (arg);}
virtual void _EachThicknessCallback       (EachThicknessArg& arg) override        {m_callbackObj->_EachThicknessCallback (arg);;}
virtual void _EachDisplayPriorityCallback (EachDisplayPriorityArg& arg) override  {m_callbackObj->_EachDisplayPriorityCallback (arg);}
virtual void _EachElementTemplateCallback (EachElementTemplateArg& arg) override  {m_callbackObj->_EachElementTemplateCallback (arg);}

virtual ElementProperties       _GetQueryPropertiesMask      () override {return m_queryObj->_GetQueryPropertiesMask ();}
virtual QueryPropertyPurpose    _GetQueryPropertiesPurpose   () override {return m_queryObj->_GetQueryPropertiesPurpose ();}

virtual ElementProperties       _GetEditPropertiesMask       () override {return m_editObj->_GetEditPropertiesMask ();}
virtual EditPropertyPurpose     _GetEditPropertiesPurpose    () override {return m_editObj->_GetEditPropertiesPurpose ();}
virtual DgnModelP               _GetDestinationDgnModel      () override {return m_editObj->_GetDestinationDgnModel ();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StyleDependantRemapper : public DgnPlatform::IEditProperties
    {
private:
    ElementProperties m_propertyMask;
    PropertyTable<DgnElementId> m_dimStyleMap;
    PropertyTable<DgnElementId> m_mlineStyleMap;
    PropertyTable<DgnCategoryId> m_levelMap;

    virtual ElementProperties   _GetEditPropertiesMask () override;
    virtual EditPropertyPurpose _GetEditPropertiesPurpose () override;

    virtual void    _EachDimStyleCallback   (EachDimStyleArg& arg)   override;
    virtual void    _EachMLineStyleCallback (EachMLineStyleArg& arg) override;
    virtual void    _EachCategoryCallback   (EachCategoryArg& arg)      override;

public:
    DGNPLATFORM_EXPORT   /* ctor */  StyleDependantRemapper ();
    DGNPLATFORM_EXPORT   void        AddDimStyleRemap    (DgnElementId  fromID, DgnElementId  toID);
    DGNPLATFORM_EXPORT   void        AddMLineStyleRemap  (DgnElementId  fromID, DgnElementId  toID);
    DGNPLATFORM_EXPORT   void        AddCategoryRemap    (DgnCategoryId fromID, DgnCategoryId toID);

                         ElementProperties  GetEditPropertiesMask () { return _GetEditPropertiesMask(); }

    DGNPLATFORM_EXPORT   uint32_t           DoRemapping (DgnDbR);
    DGNPLATFORM_EXPORT   uint32_t           DoRemapping (DgnModelR);
    DGNPLATFORM_EXPORT   bool               DoRemapping (EditElementHandleR);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

