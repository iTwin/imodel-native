/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECDiff.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
/// @cond BENTLEY_SDK_All
#pragma once

#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECEnabler.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/bset.h>
//__PUBLISH_SECTION_END__
#include <stack>
#include <set>
//__PUBLISH_SECTION_START__



//__PUBLISH_SECTION_END__
EC_TYPEDEFS(ECDiffNode);
EC_TYPEDEFS(ECDiffValue);
//__PUBLISH_SECTION_START__
EC_TYPEDEFS(ECDiff);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECDiff> ECDiffPtr;

//======================================================================================
//! Status return by diff operations
//! @ingroup ECObjectsGroup
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum DiffStatus
    {
    DIFFSTATUS_Success = SUCCESS,
    DIFFSTATUS_InvalidAccessString,
    DIFFSTATUS_Failed
    };

//======================================================================================
//! Provide detail on a diff node in a ECDiff tree.
//! @ingroup ECObjectsGroup
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum DiffNodeState
    {
    DIFFNODESTATE_AppendLeft,
    DIFFNODESTATE_AppendRight,
    DIFFNODESTATE_Conflict,
    };

//======================================================================================
//! In case of conflict during merge where difference contain different values for same 
//! schema facet. A simple rule is use to determine which input schema have higher priority
//! then other and its value would be use for that facet in merged schema.
//! @ingroup ECObjectsGroup
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum ConflictRule
    {
    CONFLICTRULE_TakeLeft, //! Take schema facet value from left schema in diff
    CONFLICTRULE_TakeRight //! Take schema facet value from right schema in diff
    };

//======================================================================================
//! Provide status of merge operations
//! @ingroup ECObjectsGroup
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum MergeStatus
    {
    MERGESTATUS_Success = SUCCESS, 
    MERGESTATUS_Failed,
    MERGESTATUS_ErrorCreatingMergeSchema,
    MERGESTATUS_ErrorClassNotFound,
    MERGESTATUS_ErrorMergeClassAlreadyExist,
    MERGESTATUS_ErrorCreatingCopyOfCustomAttribute,
    MERGESTATUS_ErrorParsingRelationshipStrengthType,
    MERGESTATUS_ErrorParsingRelationshipStrengthDirection,
    MERGESTATUS_ErrorParsingCardinality,
    };

#define ECDIFF_DEFAULT_TABSIZE 3

//======================================================================================
//! ECDiff can be use to take a difference of two ECSchemas and if required merge them together.
//! @ingroup ECObjectsGroup
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
struct ECDiff : RefCountedBase
    {
//__PUBLISH_SECTION_END__
    friend struct ECSchemaMergeTool;
    friend struct ECSchemaDiffTool;
    
private:
    ECDiffNodeP m_diff; 
    ECN::ECSchemaPtr m_leftSchema;
    ECN::ECSchemaPtr m_rightSchema;
    DiffStatus  m_status;
private:
    ECDiff(ECN::ECSchemaR left, ECN::ECSchemaR right, ECDiffNodeP diff, DiffStatus status);
    ECDiffNodeP GetRoot();
//__PUBLISH_SECTION_START__
public:
    //! Gets left ECSchema as passed at the time of diff.
    //! @return The retrieved ECSchema
    ECOBJECTS_EXPORT ECN::ECSchemaCR GetLeftSchema() const;
    //! Gets right ECSchema as passed at the time of diff.
    //! @return The retrieved ECSchema
    ECOBJECTS_EXPORT ECN::ECSchemaCR GetRightSchema() const;
    //! Gets the status of the diff. If its succeed it will return ::DIFFSTATUS_Success.
    //! @return The status of the diff
    ECOBJECTS_EXPORT DiffStatus GetStatus() const;
    //! Write the diff to string. Each line is prefix with either 'L" mean appended from left schema, 'R' appended from right schema and '!' means there was conflict between left and right schema.
    //! @param[out] outString WSTring object to which diff will be written.
    //! @param[in] tabSize optional parameter specifying size of the spaces to use for indenting each line in diff.
    //! @return if successfully it will return ::DIFFSTATUS_Success.
    ECOBJECTS_EXPORT DiffStatus WriteToString(WStringR outString, int tabSize = ECDIFF_DEFAULT_TABSIZE);
    //! Return if diff is empty. If there is no difference between two schemas. This function will return true.
    //! @return true if its empty
    ECOBJECTS_EXPORT bool IsEmpty();
    //! Merge the schemas from which diff was created. 
    //! @param[out] mergedSchema The resulting merged schema if merge is successful.
    //! @param[in] conflictRule If there is a conflict in diff then this rule is used to resolve it.
    //! @return if successfully it will return ::MERGESTATUS_Success.
    ECOBJECTS_EXPORT MergeStatus Merge (ECN::ECSchemaPtr& mergedSchema, ConflictRule conflictRule);
    //! Compute a diff of two schemas
    //! @param[in] leftSchema The first or left schema for diff.
    //! @param[in] rightSchema The second or right schema for diff.
    //! @return if successfully it will return ::DIFFSTATUS_Success.
    ECOBJECTS_EXPORT static ECDiffPtr Diff (ECN::ECSchemaCR leftSchema, ECN::ECSchemaCR rightSchema);
    //! Return a list of nodes status matching the accessString.
    //! @param[out] nodes The nodes that were found in diff matching accessString.
    //! @param[in] accessString A accessString is a absolute or partial path in a diff tree. Following is few examples
    //!                          *.CustomAttributes.UnitSpecification Find UnitSpecification in diff and its status which ever container it exist on.
    //!                          *.Classes.*.Prop1 It will return Prop1 from diff tree under Classes nodes.
    //! @param[in] bAccumlativeState Accumulative state mean if user need just the state of specific node or all its children.
    //! @return if successfully it will return ::DIFFSTATUS_Success.
    ECOBJECTS_EXPORT DiffStatus GetNodesState (bmap<WString, DiffNodeState>& nodes, WStringCR accessString, bool bAccumlativeState = true);
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
// For case-sensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct DiffNameComparer
    {
    bool operator()(WCharCP s1, WCharCP s2) const { return (wcscmp(s1, s2) < 0);}
    };

enum DiffNodeId
    {
    DIFFNODEID_None = 0,
    DIFFNODEID_Root = 1,
    DIFFNODEID_Schema = 2,
    DIFFNODEID_Name = 3,
    DIFFNODEID_DisplayLabel = 4,
    DIFFNODEID_Description = 5,
    DIFFNODEID_VersionMajor = 6,
    DIFFNODEID_VersionMinor = 7,
    DIFFNODEID_Classes = 8,
    DIFFNODEID_Class = 9,
    DIFFNODEID_References = 10,
    DIFFNODEID_IsCustomAttributeClass = 11,
    DIFFNODEID_IsDomainClass = 12,
    DIFFNODEID_IsStruct = 13,
    DIFFNODEID_BaseClass = 14,
    DIFFNODEID_Reference = 15,
    DIFFNODEID_IsRelationshipClass = 16,
    DIFFNODEID_BaseClasses = 17,
    DIFFNODEID_IsArray = 18,
    DIFFNODEID_Properties = 19,
    DIFFNODEID_Property = 20,
    DIFFNODEID_ArrayInfo = 21,
    DIFFNODEID_IsReadOnly = 22,
    DIFFNODEID_IsPrimitive = 23,
    DIFFNODEID_RelationshipInfo = 24,
    DIFFNODEID_IsOverriden = 25,
    DIFFNODEID_TypeName = 26,
    DIFFNODEID_Strength = 27,
    DIFFNODEID_StrengthDirection = 28,
    DIFFNODEID_Source = 29,
    DIFFNODEID_Target = 30,
    DIFFNODEID_Cardinality =31,
    DIFFNODEID_IsPolymorphic = 32,
    DIFFNODEID_RoleLabel = 33,
    DIFFNODEID_MaxOccurs = 34,
    DIFFNODEID_MinOccurs = 35,
    DIFFNODEID_CustomAttributes = 36,
    DIFFNODEID_CustomAttribute = 37,
    DIFFNODEID_ConstraintClasses = 38,
    DIFFNODEID_ConstraintClass = 39,
    DIFFNODEID_NamespacePrefix = 40,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef bvector<ECClassCP> AlignedClasses;
typedef bmap<WCharCP,AlignedClasses, DiffNameComparer> AlignedClassMap;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffValue //Replace it with ECValue 
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
    bool GetValueBool() const;
    double GetValueDouble() const;
    WStringCR GetValueString() const {return m_valueString; }
    bool IsValueNill() const;
    void Clear();
    bool IsEqual(ECDiffValue const& value);
    WString ToString() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/

enum DiffType
    {
    DIFFTYPE_Conflict,
    DIFFTYPE_Left,
    DIFFTYPE_Right,
    DIFFTYPE_Equal,
    DIFFTYPE_Empty
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffNode 
    {
    typedef bvector<ECDiffNodeP> ECDiffNodeList;
    typedef bmap<WCharCP, ECDiffNodeP, DiffNameComparer> ECDiffNodeMap;
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
    DiffNodeId m_id;
    DiffType m_cachedAccumlativeType;
    int m_index;
private:
    void _Write (WStringR out, int indent, WStringCR tab) const;
    void _GetNodeState (bmap<WString, DiffNodeState>& nodes, std::stack<WString> accessors, bool bAccumlativeState);
    DiffType _GetDiffType (bool bRecursively);

public:
   
    enum ValueDirection
        {
        DIRECTION_Left,
        DIRECTION_Right,
        };
    static WCharCP IdToString (DiffNodeId id);
public:
    DiffType GetDiffType (bool bRecursively =false);

    DiffNodeId GetId() const { return m_id;}
    ECDiffNodeP Find (WCharCP name);
    ECDiffNode (WCharCP name, ECDiffNodeP parent = NULL, DiffNodeId id = DIFFNODEID_None, int index = -1) ;
    WString GetAccessString() const;
    ECDiffValue& GetValueLeft();
    ECDiffValue& GetValueRight();
    ECDiffValue& GetValue (ValueDirection direction);
    bool SetValue (WCharCP left, WCharCP right);
    bool SetValue (bool left, bool right);
    bool SetValue (UInt32 left, UInt32 right);
    int GetIndex() const {return m_index;}
    const_iterator begin();
    const_iterator end();
    ECDiffNodeList::size_type size ();
    ECDiffNodeCP GetParent() const;
    WStringCR GetName() const { return m_name;}
    ECDiffNodeP Add (DiffNodeId type);
    ECDiffNodeP Add (WCharCP name, DiffNodeId type);
    ECDiffNodeP AddWithIndex (int index, DiffNodeId type);
    ECDiffNodeP GetChild (WStringCR accessString, bool bCreate = false);
    ECDiffNodeP GetChildById (DiffNodeId id)
        {
        WCharCP c = IdToString(id);
        BeAssert (c != NULL);
        return GetChild (c);
        }
    DiffStatus GetNodeState (bmap<WString, DiffNodeState>& nodes, WStringCR accessString, bool bAccumlativeState = true);
    void Clear();
    ECDiffNodeP RemoveIfEmpty(ECDiffNodeP n);
    void Remove (ECDiffNodeP n);
    WString ToString(int tabSize = 2) const;
    ~ECDiffNode();
    bool IsEmpty();
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaMergeTool
    {
private:
    enum MergeAction
        {
        MERGEACTION_CopyLeft,
        MERGEACTION_CopyRight,
        MERGEACTION_ResolveConflict,
        };
    struct ClassMergeInfo
        {
        private:
            DiffType m_diffType;
            ECDiffNodeP m_diff;
            ECClassCP m_mergeClass;
        public:
            ClassMergeInfo ()
                :m_diffType(DIFFTYPE_Equal), m_diff (NULL), m_mergeClass (NULL)
                {

                }
            ClassMergeInfo (DiffType type, ECDiffNodeP diff )
                : m_diffType(type), m_diff (diff), m_mergeClass (NULL)
                {
                }
            void SetClass(ECClassCR ecClass)
                {
                m_mergeClass = &ecClass;
                }
            ECClassCP GetClass() const { return m_mergeClass; }
            DiffType  GetType() const { return m_diffType; }
            ECDiffNodeP GetNode ()  {return m_diff;}
        };
    typedef bmap<WCharCP, ClassMergeInfo, DiffNameComparer> ClassMergeInfoMap;
    typedef bmap<WCharCP, DiffType, DiffNameComparer> PropertyMergeInfoMap;
    typedef bmap<WString, ECClassCP> ClassByNameMap;

    ECSchemaPtr m_mergeSchema;
    ECDiffR m_diff;
    ConflictRule m_defaultConflictRule;
    ClassMergeInfoMap m_classMergeTasks;
    ClassByNameMap m_classByNameMap;
    std::set<WString> m_doneList;

    ECSchemaCR GetLeft() const;
    ECSchemaCR GetRight() const;
    ECSchemaR GetMerged();
    ECSchemaCR GetDefault() const;
    
    void EnsureSchemaIsReferenced (ECClassCR referenceClass);
    void EnsureSchemaIsReferenced (ECSchemaCR reference);
    void BuildClassMap (ECSchemaCR schema);
    bool ParseStrengthDirection(ECRelatedInstanceDirection& direction, WStringCR directionStr);
    static bool ParseStrength(StrengthType& type, WStringCR strengthStr);
    static bool ParsePrimitiveType (PrimitiveType& primitiveType, WStringCR primitiveTypeString);

    IECInstancePtr CreateCopyThroughSerialization (IECInstanceR instance, ECClassCR ecClass);
    bool IsPartOfMergeSchema(ECClassCR ecClass) const;
    void ComputeMergeActions(ClassMergeInfoMap& actions, ECDiffNodeP diffClasses, ECSchemaCR schema);
    void ComputeMergeActions(PropertyMergeInfoMap& actions, ECDiffNodeP diffProperties, ECClassCR ecClass);
    
    ECDiffValueP GetMergeValue (ECDiffNodeR n, WCharCP id);
    ECDiffValueP GetMergeValue (ECDiffNodeR v);
    ECDiffValueP GetMergeValue (ECDiffNodeR n, DiffNodeId id);

    MergeStatus MergeSchema (ECSchemaPtr& mergedSchema);
    MergeStatus MergeRelationship (ECDiffNodeP diff, ECRelationshipClassR mergedClass, ECClassCR defaultClass);
    MergeStatus MergeRelationshipConstraint (ECDiffNodeR diff, ECRelationshipConstraintR mergedConstraint, ECRelationshipConstraintCP defaultContraint);
    MergeStatus MergeCustomAttributes (ECDiffNodeR diff, IECCustomAttributeContainerR mergedContainer, IECCustomAttributeContainerCP defaultConstainer);
    MergeStatus MergeProperties(ECDiffNodeR diff, ECClassR mergedClass, ECClassCR defaultClass);
    MergeStatus MergeProperty (ECDiffNodeP diff, ECClassR mergedClass, ECClassCR defaultClass);
    MergeStatus MergeClass (ECDiffNodeR diff, ECClassCP defaultClass, ClassMergeInfo& info);
    MergeStatus MergeBaseClasses (ECDiffNodeR diff,ECClassR mergedClass, ECClassCR defaultClass);

    MergeStatus AppendClassToMerge (ECClassCP ecClass, ClassMergeInfo& info);
    MergeStatus AppendBaseClassesToMerge (ECClassR to, ECClassCR from);
    MergeStatus AppendPropertiesToMerge(ECClassR mergeClass, ECClassCR defaultClass);
    MergeStatus AppendPropertyToMerge(ECClassR mergeClass,ECPropertyCP property);
    MergeStatus AppendRelationshipToMerge(ECRelationshipClassR mergedRelationshipClass, ECRelationshipClassCR defaultRelationshipClass);
    MergeStatus AppendRelationshipClassToMerge(ECRelationshipClassR mergedClass, ECRelationshipClassCR defaultClass);
    MergeStatus AppendRelationshipContstraintToMerge(ECRelationshipConstraintR mergedRelationshipClassConstraint, ECRelationshipConstraintCR defaultRelationshipClassConstraint);
    MergeStatus AppendCustomAttributesToMerge (IECCustomAttributeContainerR mergeContainer, IECCustomAttributeContainerCR defaultContainer);

    ECClassCP ResolveClass (WStringCR classFullName);
    MergeStatus ResolveClassFromMergeContext (ECClassCP& mergedClass, WCharCP className);
    bool TryGetECRelationshipConstraint (ECRelationshipConstraintCP & relationshipConstraint,  ECDiffNodeCR relationshipConstraintNode, ECSchemaCR schema);
    bool TryGetECProperty (ECPropertyCP& ecProperty,  ECDiffNodeCR propertyNode, ECSchemaCR schema, bool includeBaseClass);
    bool TryGetECClass (ECClassCP& ecclass,  ECDiffNodeCR classNode, ECSchemaCR schema);

public:
    ECSchemaMergeTool(ECDiffR diff, ConflictRule defaultRule);
    static MergeStatus Merge(ECSchemaPtr& mergedSchema, ECDiffR diff, ConflictRule defaultRule);
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaDiffTool
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
    static void CollectInstanceValues (bmap<WString,ECValue>& valueMap, std::set<WString>& accessStrings, IECInstanceCR instance);
    static void CollectInstanceValues (bmap<WString,ECValue>& valueMap, std::set<WString>& accessStrings, ECValuesCollectionCR values);
    static bool SetECValue (ECDiffNodeR n, ECValueCR v, ECDiffNode::ValueDirection direction);
    //Merge
public:
    static ECDiffNodeP Diff (ECSchemaCR left, ECSchemaCR right);
    };

//__PUBLISH_SECTION_START__
	
END_BENTLEY_ECOBJECT_NAMESPACE
/// @endcond BENTLEY_SDK_All

//__PUBLISH_SECTION_END__

