/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaDiff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "ECSchema.h"
#include <set>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// ###TODO? is it necessary to use strings for this?
struct SchemaDiffType
    {
    static const Utf8CP ECDIFF;
    static const Utf8CP NAME;
    static const Utf8CP DISPLAY_LABEL;
    static const Utf8CP DESCRIPTION;
    static const Utf8CP VERSION_MAJOR;
    static const Utf8CP VERSION_MINOR;
    static const Utf8CP CLASSES;
    static const Utf8CP REFERENCES;
    static const Utf8CP IS_CUSTOMATTRIBUTE_CLASS;
    static const Utf8CP IS_STRUCT;
    static const Utf8CP IS_DOMAIN_CLASS;
    static const Utf8CP IS_RELATIONSHIP_CLASS;
    static const Utf8CP BASECLASSES;
    static const Utf8CP PROPERTIES;
    static const Utf8CP IS_ARRAY;
    static const Utf8CP TYPENAME;
    static const Utf8CP ARRAYINFO;
    static const Utf8CP IS_READONLY;
    static const Utf8CP IS_PRIMITIVE;
    static const Utf8CP RELATIONSHIP_INFO;
    static const Utf8CP IS_OVERRIDEN;
    static const Utf8CP STRENGTH;
    static const Utf8CP STRENGTH_DIRECTION;
    static const Utf8CP SOURCE;
    static const Utf8CP TARGET;
    static const Utf8CP CARDINALITY;
    static const Utf8CP IS_POLYMORPHIC;
    static const Utf8CP ROLE_LABEL;
    static const Utf8CP STRENGTH_DIRECTION_BACKWARD;
    static const Utf8CP STRENGTH_DIRECTION_FORWARD;
    static const Utf8CP STRENGTH_TYPE_EMBEDDING;
    static const Utf8CP STRENGTH_TYPE_HOLDING;
    static const Utf8CP STRENGTH_TYPE_REFERENCING;
    static const Utf8CP MAXOCCURS;
    static const Utf8CP MINOCCURS;
    static const Utf8CP CUSTOMATTRIBUTES;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef bvector<ECSchemaCP> AlignedSchemas;
typedef bmap<Utf8CP,AlignedSchemas, CompareIWChar> AlignedSchemaMap;
typedef bvector<ECClassCP> AlignedClasses;
typedef bmap<Utf8CP,AlignedClasses, CompareIWChar> AlignedClassMap;
typedef bvector<ECPropertyCP> AlignedProperties;
typedef bmap<Utf8CP,AlignedProperties, CompareIWChar> AlignedPropertyMap;
typedef bvector<IECCustomAttributeContainerCP> AlignedCustomAttributeContainers;
typedef bvector<IECInstanceCP> AlignedCustomAttributes;
typedef bmap<Utf8CP,AlignedCustomAttributes, CompareIWChar> AlignedCustomAttributeMap;

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
    Utf8String m_valueString;
    size_t m_binarySize;
    ValueType m_type;
    union 
        {
        int32_t m_valueInt32;
        double m_valueDouble;
        int64_t m_valueInt64;
        bool m_valueBool;
        Byte* m_valueBinary;
        int64_t m_valueDateTime;
        };
public:
    ECDiffValue();
    void SetValue(Utf8CP v) ;
    void SetValue(Byte const* v, size_t size) ;
    void SetValue(Utf8StringCR v);
    void SetValue(int32_t v);
    void SetValue(int64_t v);
    void SetValue(double v);
    void SetValue(bool v);
    void SetDateTimeValue(int64_t v);
    void SetNil();
    Byte const* GetBinary(size_t& size) const;
    Utf8CP GetValueUtf8CP() const;
    int32_t GetValueInt32() const;
    int64_t GetValueInt64() const;
    bool  GetValueBool() const;
    double GetValueDouble() const;
    bool IsValueNill() const;
    void Clear();
    bool IsEqual(ECDiffValue const& value);
    Utf8String ToString() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffNode 
    {
    typedef bvector<ECDiffNodeP> ECDiffNodeList;
    typedef bmap<Utf8CP, ECDiffNodeP, CompareUtf8> ECDiffNodeMap;
    typedef ECDiffNodeList::const_iterator const_iterator;
    typedef ECDiffNodeList::iterator iterator;
    typedef ECDiffNodeList::size_type size_type;
private:
    ECDiffNodeP m_parent;
    Utf8String m_name;
    ECDiffNodeList m_childNodeList;
    ECDiffNodeMap  m_childNodeMap;
    ECDiffValue m_valueLeft;
    ECDiffValue m_valueRight;
private:
    void Write (Utf8StringR out, int indent) const;
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
    ECDiffNode (Utf8CP name, ECDiffNodeP parent = NULL) ;

    ECDiffNodeP Find (Utf8CP name);
    Utf8String GetAccessString() const;
    ECDiffValue& GetValueLeft();
    ECDiffValue& GetValueRight();
    ECDiffValue& GetValue (ValueDirection direction);
    bool SetValue (Utf8CP left, Utf8CP right);
    bool SetValue (bool left, bool right);
    bool SetValue (uint32_t left, uint32_t right);
    const_iterator begin();
    const_iterator end();
    ECDiffNodeList::size_type size ();
    ECDiffNodeCP GetParent() const;
    Utf8StringCR GetName() const;
    ECDiffNodeP Add (Utf8CP name);
    ECDiffNodeP GetChild (Utf8StringCR accessString, bool bCreate);
    ECDiffNodeP Add (int index);
    void Clear();
    ECDiffNodeP RemoveIfEmpty(ECDiffNodeP n);
    void Remove (ECDiffNodeP n);
    Utf8String ToString() const;
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
    static ECDiffNodeP DiffClass (Utf8CP className, ECSchemaCR schemaLeft, ECSchemaCR schemaRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffBaseClasses (ECDiffNodeR parentDiff, AlignedClasses const& classes);
    static ECDiffNodeP DiffProperty (Utf8CP propertyName, ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff);
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

    static Utf8CP ToString(ECRelatedInstanceDirection direction);
    static Utf8CP ToString(StrengthType type);
    static void CollectInstanceValues (bmap<Utf8String,ECValue>& valueMap, bset<Utf8String>& accessStrings, IECInstanceCR instance);
    static void CollectInstanceValues (bmap<Utf8String,ECValue>& valueMap, bset<Utf8String>& accessStrings, ECValuesCollectionCR values);
    static bool SetECValue (ECDiffNodeR n, ECValueCR v, ECDiffNode::ValueDirection direction);

public:
    static bool DiffSchema (ECDiffNode& diffRoot, ECSchemaCR left, ECSchemaCR right);
    };

    
END_BENTLEY_ECOBJECT_NAMESPACE

