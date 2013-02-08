/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaDiff.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "ECSchema.h"
#include <set>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
// For case-insensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIWChar
    {
    bool operator()(WCharCP s1, WCharCP s2) const { return (BeStringUtilities::Wcsicmp(s1, s2) < 0);}
    };

//=======================================================================================
// For case-sensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareWChar
    {
    bool operator()(WCharCP s1, WCharCP s2) const { return (wcscmp(s1, s2) < 0);}
    };

// ###TODO? is it necessary to use strings for this?
struct SchemaDiffType
    {
    static const WCharCP	ECDIFF;
    static const WCharCP	NAME;
    static const WCharCP	DISPLAY_LABEL;
    static const WCharCP	DESCRIPTION;
    static const WCharCP	VERSION_MAJOR;
    static const WCharCP	VERSION_MINOR;
    static const WCharCP	CLASSES;
    static const WCharCP	REFERENCES;
    static const WCharCP	IS_CUSTOMATTRIBUTE_CLASS;
    static const WCharCP	IS_STRUCT;
    static const WCharCP	IS_DOMAIN_CLASS;
    static const WCharCP	IS_RELATIONSHIP_CLASS;
    static const WCharCP	BASECLASSES;
    static const WCharCP	PROPERTIES;
    static const WCharCP	IS_ARRAY;
    static const WCharCP	TYPENAME;
    static const WCharCP	ARRAYINFO;
    static const WCharCP	IS_READONLY;
    static const WCharCP	IS_PRIMITIVE;
    static const WCharCP	RELATIONSHIP_INFO;
    static const WCharCP	IS_OVERRIDEN;
    static const WCharCP	STRENGTH;
    static const WCharCP	STRENGTH_DIRECTION;
    static const WCharCP	SOURCE;
    static const WCharCP	TARGET;
    static const WCharCP	CARDINALITY;
    static const WCharCP	IS_POLYMORPHIC;
    static const WCharCP	ROLE_LABEL;
    static const WCharCP	STRENGTH_DIRECTION_BACKWARD;
    static const WCharCP	STRENGTH_DIRECTION_FORWARD;
    static const WCharCP	STRENGTH_TYPE_EMBEDDING;
    static const WCharCP	STRENGTH_TYPE_HOLDING;
    static const WCharCP	STRENGTH_TYPE_REFERENCING;
    static const WCharCP	MAXOCCURS;
    static const WCharCP	MINOCCURS;
    static const WCharCP	CUSTOMATTRIBUTES;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef bvector<ECSchemaCP> AlignedSchemas;
typedef bmap<WCharCP,AlignedSchemas, CompareIWChar> AlignedSchemaMap;
typedef bvector<ECClassCP> AlignedClasses;
typedef bmap<WCharCP,AlignedClasses, CompareIWChar> AlignedClassMap;
typedef bvector<ECPropertyCP> AlignedProperties;
typedef bmap<WCharCP,AlignedProperties, CompareIWChar> AlignedPropertyMap;
typedef bvector<IECCustomAttributeContainerCP> AlignedCustomAttributeContainers;
typedef bvector<IECInstanceCP> AlignedCustomAttributes;
typedef bmap<WCharCP,AlignedCustomAttributes, CompareIWChar> AlignedCustomAttributeMap;

struct ECDiffNode;
typedef ECDiffNode*       ECDiffNodeP;
typedef ECDiffNode&       ECDiffNodeR;
typedef ECDiffNode const& ECDiffNodeCR;
typedef ECDiffNode const* ECDiffNodeCP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffValue
    {
    enum ValueType
        {
        VALUETYPE_Nil,
        VALUETYPE_String,
        VALUETYPE_Int32,
        VALUETYPE_Int64,
        VALUETYPE_Double,
        VALUETYPE_Boolean, 
        VALUETYPE_Binary,
        VALUETYPE_DateTimeTicks
        };
private:
    WString m_valueString;
    size_t m_binarySize;
    ValueType m_type;
    union 
        {
        Int32 m_valueInt32;
        double m_valueDouble;
        Int64 m_valueInt64;
        bool m_valueBool;
        byte* m_valueBinary;
        Int64 m_valueDateTime;
        };
public:
    ECDiffValue();
    void SetValue(WCharCP v) ;
    void SetValue(byte const* v, size_t size) ;
    void SetValue(WStringCR v);
    void SetValue(Int32 v);
    void SetValue(Int64 v);
    void SetValue(double v);
    void SetValue(bool v);
    void SetDateTimeValue(Int64 v);
    void SetNil();
    Byte const* GetBinary(size_t& size) const;
    WCharCP GetValueWCharCP() const;
    Int32 GetValueInt32() const;
    Int64 GetValueInt64() const;
    Int64 GetValueBool() const;
    Int64 GetValueDouble() const;
    bool IsValueNill() const;
    void Clear();
    bool IsEqual(ECDiffValue const& value);
    WString ToString() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffNode 
    {
    typedef bvector<ECDiffNodeP> ECDiffNodeList;
    typedef bmap<WCharCP, ECDiffNodeP, CompareWChar> ECDiffNodeMap;
    typedef ECDiffNodeList::const_iterator const_iterator;
    typedef ECDiffNodeList::iterator iterator;
    typedef ECDiffNodeList::size_type size_type;
private:
    ECDiffNodeP m_parent;
    WString m_name;
    ECDiffNodeList m_childNodeList;
    ECDiffNodeMap  m_childNodeMap;
    ECDiffValue m_valueLeft;
    ECDiffValue m_valueRight;
private:
    void Write (WStringR out, int indent) const;
public:
    enum KindOf
        {
        KINDOF_Primitive,
        KINDOF_ArrayType,
        };
    enum ValueDirection
        {
        DIRECTION_Left,
        DIRECTION_Right,
        };
public:
    ECDiffNode();
    ECDiffNode (WCharCP name, ECDiffNodeP parent = NULL) ;

    ECDiffNodeP Find (WCharCP name);
    WString GetAccessString() const;
    ECDiffValue& GetValueLeft();
    ECDiffValue& GetValueRight();
    ECDiffValue& GetValue (ValueDirection direction);
    bool SetValue (WCharCP left, WCharCP right);
    bool SetValue (bool left, bool right);
    bool SetValue (UInt32 left, UInt32 right);
    const_iterator begin();
    const_iterator end();
    ECDiffNodeList::size_type size ();
    ECDiffNodeCP GetParent() const;
    WStringCR GetName() const;
    ECDiffNodeP Add (WCharCP name);
    ECDiffNodeP GetChild (WStringCR accessString, bool bCreate);
    ECDiffNodeP Add (int index);
    void Clear();
    ECDiffNodeP RemoveIfEmpty(ECDiffNodeP n);
    void Remove (ECDiffNodeP n);
    WString ToString() const;
    ~ECDiffNode();
    bool IsEmpty();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaDiff
    {
private:
    static ECDiffNodeP DiffReferences(ECDiffNodeR parentDiff, ECSchemaCR schemaLeft, ECSchemaCR schemaRight);
    static ECDiffNodeP DiffClass (WCharCP className, ECSchemaCR schemaLeft, ECSchemaCR schemaRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffBaseClasses (ECDiffNodeR parentDiff, AlignedClasses const& classes);
    static ECDiffNodeP DiffProperty (WCharCP propertyName, ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffRelationship (ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  left, ECRelationshipConstraintCR  right, ECRelationshipEnd endPoint);
    static ECDiffNodeP DiffRelationshipConstraintClasses (ECDiffNodeR parentDiff, bvector<ECRelationshipConstraintCP> const& constraints, ECRelationshipEnd endPoint);
    static ECDiffNodeP DiffArrayBounds (ECDiffNodeR parent , ECPropertyCR left, ECPropertyCR right);
    static ECDiffNodeP DiffCustomAttributes (ECDiffNodeR parentDiff, IECCustomAttributeContainerCR leftContainer, IECCustomAttributeContainerCR rightContainer);
    static ECDiffNodeP DiffInstance (ECDiffNodeR parentDiff, ECClassCR customAttributeClass, IECCustomAttributeContainerCR leftContainer, IECCustomAttributeContainerCR rightContainer);

    static ECDiffNodeP AppendRelationship(ECDiffNodeR parent, ECRelationshipClassCR relationship, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  relationshipConstraint,ECRelationshipEnd endPoint, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendClass (ECDiffNodeR parent , ECClassCR ecClass, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendProperty (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendArrayBounds (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendCustomAttributes (ECDiffNodeR parentDiff, IECCustomAttributeContainerCR container, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendInstance (ECDiffNodeR parentDiff, IECInstanceCR instance, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendPropertyValues (ECDiffNodeR parentDiff, ECValuesCollectionCR values, ECDiffNode::ValueDirection direction);

    static WCharCP ToString(ECRelatedInstanceDirection direction);
    static WCharCP ToString(StrengthType type);
    static void CollectInstanceValues (bmap<WString,ECValue>& valueMap, bset<WString>& accessStrings, IECInstanceCR instance);
    static void CollectInstanceValues (bmap<WString,ECValue>& valueMap, bset<WString>& accessStrings, ECValuesCollectionCR values);
    static bool SetECValue (ECDiffNodeR n, ECValueCR v, ECDiffNode::ValueDirection direction);

public:
    static bool DiffSchema (ECDiffNode& diffRoot, ECSchemaCR left, ECSchemaCR right);
    };

    
END_BENTLEY_ECOBJECT_NAMESPACE

