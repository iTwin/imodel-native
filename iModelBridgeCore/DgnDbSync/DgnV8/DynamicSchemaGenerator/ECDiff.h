/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <stack>
#include <set>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(IECDiffNode);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ECDiffNode);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ECDiffValue);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ECDiff);

typedef RefCountedPtr<ECDiff> ECDiffPtr;
typedef bvector<IECDiffNodeCP> DiffNodeList; 

//======================================================================================
//! Status return by diff operations
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum class DiffStatus
    {
    Success = 0,
    InvalidAccessString,
    Failed
    };

//======================================================================================
//! Provide detail on a diff node in a ECDiff tree.
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
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum ConflictRule
    {
    CONFLICTRULE_TakeLeft, //! Take schema facet value from left schema in diff
    CONFLICTRULE_TakeRight //! Take schema facet value from right schema in diff
    };

//======================================================================================
//! DiffType represent if a node has Conflicting value or left schema have a value that right doesnt or vice versa
//! Equal and Empty are special values and will not be returned currently.
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+=====
enum DiffType
    {
    DIFFTYPE_Conflict,
    DIFFTYPE_Left,
    DIFFTYPE_Right,
    DIFFTYPE_Equal,
    DIFFTYPE_Empty
    };

//======================================================================================
//! DiffNodeId represent meta schema node id for diff nodes.
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+=====
enum class DiffNodeId
    {
    None = 0,
    Root = 1,
    Schema = 2,
    Name = 3,
    DisplayLabel = 4,
    Description = 5,
    VersionRead = 6,
    VersionWrite = 7,
    VersionMinor = 8,
    ECVersionMajor = 9, 
    ECVersionMinor = 10,
    Classes = 11,
    Class = 12,
    References = 13,
    IsCustomAttributeClass = 14,
    IsEntityClass = 15,
    IsStruct = 16,
    BaseClass = 17,
    Reference = 18,
    IsRelationshipClass = 19,
    BaseClasses = 20,
    IsArray = 21,
    Properties = 22,
    Property = 23,
    ArrayInfo = 24,
    IsReadOnly = 25,
    IsPrimitive = 26,
    RelationshipInfo = 27,
    IsOverriden = 28,
    TypeName = 29,
    Strength = 30,
    StrengthDirection = 31,
    Source = 32,
    Target = 33,
    Multiplicity = 34,
    IsPolymorphic = 35,
    RoleLabel = 36,
    MaxOccurs = 37,
    MinOccurs = 38,
    CustomAttributes = 39,
    CustomAttribute = 40,
    ConstraintClasses = 41,
    ConstraintClass = 42,
    Alias = 43,
    IsAbstract = 44,
    IsSealed = 45,
    MinimumValue = 46,
    MaximumValue = 47,
    AbstractConstraint = 48,
    ECVersion = 49,
    ECXmlVersionMajor = 50,
    ECXmlVersionMinor = 51,
    };

//======================================================================================
//! Represent a node in diff tree
//! @bsiclass                                                     Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct IECDiffNode 
    {
public:
    //! Return diff node type. It represent a meta schema id for facet that represent current diff node.
    //! @return return diff node type
    virtual DiffNodeId                  GetId() const = 0;
    //! Return access string for the current node. Access string is a meta schema path for current diff node.
    //! @return access string for current node
    virtual Utf8String                  GetAccessString() const =0;
    //! Return constant string representation of current diff node.
    //! @return name of current node.
    virtual Utf8StringCR                GetName() const = 0;
    //! If current node is a array element e.g. BaseClasses it will also have a index
    //! @return index of current node if its array. Return -1 if if its not.
    virtual int                         GetIndex() const = 0;
    //! Return list of childe node pointers for current diff node. This method computes child nodes so it would nice idea to keep it value in local variable for performance reasons.
    //! @return list of child node pointers
    virtual DiffNodeList                GetChildren() const = 0;
   //! Return child by knownDiffNodeId
    //! @param[in] id The id of child diff node.
    //! @return if successfully will return child node otherwise nullptr
    virtual IECDiffNodeCP               GetChildById(DiffNodeId id) const = 0;
    //! Return child by accessString
    //! @param[in] accessString The accessString of child diff node.
    //! @return if successfully will return child node otherwise nullptr
    virtual IECDiffNodeCP               GetChildByAccessString(Utf8CP accessString) const = 0;
    //! Returns diff type for current diff node
    //! @param[in] bRecursively If true it will compute accumulative diff type of sub tree.
    //! @return DiffType for current node or accumulative one if bRecursively was true.
    virtual DiffType                    GetDiffType(bool bRecursively =false) const = 0;
    //! Return left value for current diff node. GetDiffType() can be use to determine type of diff to see which one (Left or Right) need to be called.
    //! @return ECValue for current node left value from left schema
    virtual ECN::ECValue                GetValueLeft() const = 0;
    //! Return right value for current diff node. GetDiffType() can be use to determine type of diff to see which one (Left or Right) need to be called.
    //! @return ECValue for current node left value from right schema
    virtual ECN::ECValue                GetValueRight() const = 0;

    virtual ~IECDiffNode(){};
    };

//======================================================================================
//! Provide status of merge operations
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
enum class MergeStatus
    {
    Success = 0, 
    Failed,
    ErrorCreatingMergeSchema,
    ErrorClassNotFound,
    ErrorClassTypeMismatch,
    ErrorMergeClassAlreadyExist,
    ErrorCreatingCopyOfCustomAttribute,
    ErrorParsingRelationshipStrengthType,
    ErrorParsingRelationshipStrengthDirection,
    ErrorParsingMultiplicity,
    };

#define ECDIFF_DEFAULT_TABSIZE 3


//======================================================================================
//! ECDiff can be use to take a difference of two ECSchemas and if required merge them together.
//! @bsiclass                                                     Affan.Khan      02/2013
//+===============+===============+===============+===============+===============+======
struct ECDiff : RefCountedBase
    {
    friend struct ECSchemaMergeTool;
    friend struct ECSchemaDiffTool;
    
private:
    ECDiffNodeP m_diff; 
    ECN::ECSchemaPtr m_leftSchema;
    ECN::ECSchemaPtr m_rightSchema;
    DiffStatus  m_status;

    ECDiff(ECN::ECSchemaR left, ECN::ECSchemaR right, ECDiffNodeP diff, DiffStatus status);
    ECDiffNodeP GetRoot();

public:
    //! Gets left ECSchema as passed at the time of diff.
    //! @return The retrieved ECSchema
    ECN::ECSchemaCR GetLeftSchema() const;
    //! Gets right ECSchema as passed at the time of diff.
    //! @return The retrieved ECSchema
    ECN::ECSchemaCR GetRightSchema() const;
    //! Gets the status of the diff. If its succeed it will return DiffStatus::Success.
    //! @return The status of the diff
    DiffStatus GetStatus() const;
    //! Write the diff to string. Each line is prefix with either 'L" mean appended from left schema, 'R' appended from right schema and '!' means there was conflict between left and right schema.
    //! @param[out] outString WSTring object to which diff will be written.
    //! @param[in] tabSize optional parameter specifying size of the spaces to use for indenting each line in diff.
    //! @return if successfully it will return DiffStatus::Success.
    DiffStatus WriteToString(Utf8StringR outString, int tabSize = ECDIFF_DEFAULT_TABSIZE);
    //! Return if diff is empty. If there is no difference between two schemas. This function will return true.
    //! @return true if its empty
    bool IsEmpty();
    //! Merge the schemas from which diff was created. 
    //! @param[out] mergedSchema The resulting merged schema if merge is successful.
    //! @param[in] conflictRule If there is a conflict in diff then this rule is used to resolve it.
    //! @return if successfully it will return MergeStatus::Success.
    MergeStatus Merge (ECN::ECSchemaPtr& mergedSchema, ConflictRule conflictRule);
    //! Compute a diff of two schemas
    //! @param[in] leftSchema The first or left schema for diff.
    //! @param[in] rightSchema The second or right schema for diff.
    //! @return if successfully it will return DiffStatus::Success.
    static ECDiffPtr Diff (ECN::ECSchemaCR leftSchema, ECN::ECSchemaCR rightSchema);
    //! Return a list of nodes status matching the accessString.
    //! @param[out] nodes The nodes that were found in diff matching accessString.
    //! @param[in] accessString A accessString is a absolute or partial path in a diff tree. Following is few examples
    //!                          *.CustomAttributes.UnitSpecification Find UnitSpecification in diff and its status which ever container it exist on.
    //!                          *.Classes.*.Prop1 It will return Prop1 from diff tree under Classes nodes.
    //! @param[in] bAccumlativeState Accumulative state mean if user need just the state of specific node or all its children.
    //! @return if successfully it will return DiffStatus::Success.
    DiffStatus GetNodesState (bmap<Utf8String, DiffNodeState>& nodes, Utf8StringCR accessString, bool bAccumlativeState = true);
    //! Return top level diff node
    //! @return IECDiffNode that represent a difference between two schemas. nullptr if difference was not found.
    IECDiffNodeCP GetRootNode () const;
    };

//======================================================================================
//! ECDiffValueHelper can be use to convert string value to strong type values
//! @bsiclass                                                     Affan.Khan      10/2013
//+===============+===============+===============+===============+===============+======
struct ECDiffValueHelper
    {
    //! Attempt to parse a string into ECN::StrengthType
    //! @param[out] strengthType return ECN::StrengthType from string representation.
    //! @param[in] strengthTypeValue string representation of strength type value. 
    //! @return if successfully it will return true.
    static bool TryParseRelationshipStrengthType (ECN::StrengthType& strengthType, Utf8StringCR strengthTypeValue);

    //! Attempt to parse a string into ECN::ECRelatedInstanceDirection
    //! @param[out] strengthDirection return ECN::ECRelatedInstanceDirection from string representation.
    //! @param[in] strengthDirectionValue string representation of strength type value. 
    //! @return if successfully it will return true.
    static bool TryParseRelatedStrengthDirection (ECN::ECRelatedInstanceDirection& strengthDirection, Utf8StringCR strengthDirectionValue);

    //! Attempt to parse a string into ECN::PrimitiveType
    //! @param[out] primitiveType return ECN::PrimitiveType from string representation.
    //! @param[in] primtiveTypeValue string representation of strength type value. 
    //! @return if successfully it will return true.
    static bool TryParsePrimitiveType(ECN::PrimitiveType& primitiveType, Utf8StringCR primtiveTypeValue);

    //! Attempt to parse a classkey string into schemaName and className
    //! @param[out] schemaName the parsed schema name
    //! @param[out] className the parsed class name
    //! @param[in] classKey the input class key
    //! @return if successfully it will return true.
    static bool TryParseClassKey(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR classKey);

    };

//=======================================================================================
// For case-sensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct DiffNameComparer
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return (strcmp(s1, s2) < 0);}
    };

//=======================================================================================
// For case-insensitive Utf8CP comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct DiffNameComparerI
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return (BeStringUtilities::StricmpAscii(s1, s2) < 0); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef bvector<ECN::ECClassCP> AlignedClasses;
typedef bmap<Utf8CP,AlignedClasses, DiffNameComparer> AlignedClassMap;


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
    bool GetValueBool() const;
    double GetValueDouble() const;
    Utf8StringCR GetValueString() const {return m_valueString; }
    bool IsValueNill() const;
    void Clear();
    bool IsEqual(ECDiffValue const& value);
    Utf8String ToString() const;
    ECN::ECValue GetValueAsECValue() const; 
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDiffNode : IECDiffNode
    {
    typedef bvector<ECDiffNodeP> ECDiffNodeList;
    typedef bmap<Utf8CP, ECDiffNodeP, DiffNameComparer> ECDiffNodeMap;
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
    DiffNodeId m_id;
    DiffType m_cachedAccumlativeType;
    int m_index;

    void _Write (Utf8StringR out, int indent, Utf8StringCR tab) const;
    void _GetNodeState (bmap<Utf8String, DiffNodeState>& nodes, std::stack<Utf8String> accessors, bool bAccumlativeState);
    DiffType _GetDiffType (bool bRecursively);

public:
    enum ValueDirection
        {
        DIRECTION_Left,
        DIRECTION_Right,
        };
    static Utf8CP IdToString (DiffNodeId id);

    DiffType GetDiffType (bool bRecursively) const override
        {
        return const_cast<ECDiffNode*>(this)->ImplGetDiffType(bRecursively);
        }
    DiffNodeId GetId() const override { return m_id;}
    Utf8String GetAccessString() const override;
    Utf8StringCR GetName() const override { return m_name;}
    int GetIndex() const override {return m_index;}
    DiffNodeList GetChildren() const override 
        {
        DiffNodeList list;
        for(auto& n : m_childNodeList)
            list.push_back((IECDiffNodeP)n);
        return list;
        }
    IECDiffNodeCP GetChildByAccessString(Utf8CP name ) const override;
    IECDiffNodeCP GetChildById (DiffNodeId id) const override
        {
        return (IECDiffNodeCP)(const_cast<ECDiffNode*>(this)->ImplGetChildById (id));
        }
    ECN::ECValue              GetValueRight() const override
        {
        return const_cast<ECDiffNode*>(this)->ImplGetValueRight().GetValueAsECValue();
        }
    ECN::ECValue              GetValueLeft() const override
        {
        return const_cast<ECDiffNode*>(this)->ImplGetValueLeft().GetValueAsECValue();
        }
    DiffType ImplGetDiffType (bool bRecursively =false);
    ECDiffNodeP Find (Utf8CP name);
    ECDiffNode (Utf8CP name, ECDiffNodeP parent = NULL, DiffNodeId id = DiffNodeId::None, int index = -1) ;
    ECDiffValue& ImplGetValueLeft();
    ECDiffValue& ImplGetValueRight();
    ECDiffValue& GetValue (ValueDirection direction);
    bool SetValue (Utf8CP left, Utf8CP right);
    bool SetValue (bool left, bool right);
    bool SetValue (uint32_t left, uint32_t right);
    const_iterator begin();
    const_iterator end();
    ECDiffNodeList::size_type size ();
    ECDiffNodeCP GetParent() const;

    ECDiffNodeP Add (DiffNodeId type);
    ECDiffNodeP Add (Utf8CP name, DiffNodeId type);
    ECDiffNodeP AddWithIndex (int index, DiffNodeId type);
    ECDiffNodeP GetChild (Utf8StringCR accessString, bool bCreate = false);
    ECDiffNodeP ImplGetChildById (DiffNodeId id)
        {
        Utf8CP c = IdToString(id);
        BeAssert (c != NULL);
        return GetChild (c);
        }
    DiffStatus GetNodeState (bmap<Utf8String, DiffNodeState>& nodes, Utf8StringCR accessString, bool bAccumlativeState = true);
    void Clear();
    ECDiffNodeP RemoveIfEmpty(ECDiffNodeP n);
    void Remove (ECDiffNodeP n);
    Utf8String ToString(int tabSize = 2) const;
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
            ECN::ECClassCP m_mergeClass;
        public:
            ClassMergeInfo ()
                :m_diffType(DIFFTYPE_Equal), m_diff (NULL), m_mergeClass (NULL)
                {

                }
            ClassMergeInfo (DiffType type, ECDiffNodeP diff )
                : m_diffType(type), m_diff (diff), m_mergeClass (NULL)
                {
                }
            void SetClass(ECN::ECClassCR ecClass)
                {
                m_mergeClass = &ecClass;
                }
            ECN::ECClassCP GetClass() const { return m_mergeClass; }
            DiffType  GetType() const { return m_diffType; }
            ECDiffNodeP GetNode ()  {return m_diff;}
        };
    typedef bmap<Utf8CP, ClassMergeInfo, DiffNameComparerI> ClassMergeInfoMap;
    typedef bmap<Utf8CP, DiffType, DiffNameComparerI> PropertyMergeInfoMap;
    typedef bmap<Utf8String, ECN::ECClassCP> ClassByNameMap;

    ECN::ECSchemaPtr m_mergeSchema;
    ECDiffR m_diff;
    ConflictRule m_defaultConflictRule;
    ClassMergeInfoMap m_classMergeTasks;
    ClassByNameMap m_classByNameMap;
    bset<Utf8String> m_doneList;

    ECN::ECSchemaCR GetLeft() const;
    ECN::ECSchemaCR GetRight() const;
    ECN::ECSchemaR GetMerged();
    ECN::ECSchemaCR GetDefault() const;
    
    void EnsureSchemaIsReferenced (ECN::ECClassCR referenceClass);
    void EnsureSchemaIsReferenced (ECN::ECSchemaCR reference);
    void BuildClassMap (ECN::ECSchemaCR schema);

    ECN::IECInstancePtr CreateCopyThroughSerialization (ECN::IECInstanceR instance, ECN::ECClassCR ecClass);
    bool IsPartOfMergeSchema(ECN::ECClassCR ecClass) const;
    void ComputeMergeActions(ClassMergeInfoMap& actions, ECDiffNodeP diffClasses, ECN::ECSchemaCR schema);
    void ComputeMergeActions(PropertyMergeInfoMap& actions, ECDiffNodeP diffProperties, ECN::ECClassCR ecClass);
    
    ECDiffValueP GetMergeValue (ECDiffNodeR n, Utf8CP id);
    ECDiffValueP GetMergeValue (ECDiffNodeR v);
    ECDiffValueP GetMergeValue (ECDiffNodeR n, DiffNodeId id);

    MergeStatus MergeSchema (ECN::ECSchemaPtr& mergedSchema);
    MergeStatus MergeRelationship (ECDiffNodeP diff, ECN::ECRelationshipClassR mergedClass, ECN::ECClassCR defaultClass);
    MergeStatus MergeRelationshipConstraint (ECDiffNodeR diff, ECN::ECRelationshipConstraintR mergedConstraint, ECN::ECRelationshipConstraintCP defaultContraint);
    MergeStatus MergeCustomAttributes (ECDiffNodeR diff, ECN::IECCustomAttributeContainerR mergedContainer, ECN::IECCustomAttributeContainerCP defaultConstainer);
    MergeStatus MergeProperties(ECDiffNodeR diff, ECN::ECClassR mergedClass, ECN::ECClassCR defaultClass);
    MergeStatus MergeProperty (ECDiffNodeP diff, ECN::ECClassR mergedClass, ECN::ECClassCR defaultClass);
    MergeStatus MergeClass (ECDiffNodeR diff, ECN::ECClassCP defaultClass, ClassMergeInfo& info);
    MergeStatus MergeBaseClasses (ECDiffNodeR diff, ECN::ECClassR mergedClass, ECN::ECClassCR defaultClass);

    MergeStatus AppendClassToMerge (ECN::ECClassCP ecClass, ClassMergeInfo& info);
    MergeStatus AppendBaseClassesToMerge (ECN::ECClassR to, ECN::ECClassCR from);
    MergeStatus AppendPropertiesToMerge(ECN::ECClassR mergeClass, ECN::ECClassCR defaultClass);
    MergeStatus AppendPropertyToMerge(ECN::ECClassR mergeClass, ECN::ECPropertyCP property);
    MergeStatus AppendRelationshipToMerge(ECN::ECRelationshipClassR mergedRelationshipClass, ECN::ECRelationshipClassCR defaultRelationshipClass);
    MergeStatus AppendRelationshipClassToMerge(ECN::ECRelationshipClassR mergedClass, ECN::ECRelationshipClassCR defaultClass);
    MergeStatus AppendRelationshipConstraintToMerge(ECN::ECRelationshipConstraintR mergedRelationshipClassConstraint, ECN::ECRelationshipConstraintCR defaultRelationshipClassConstraint);
    MergeStatus AppendCustomAttributesToMerge (ECN::IECCustomAttributeContainerR mergeContainer, ECN::IECCustomAttributeContainerCR defaultContainer);

    ECN::ECClassCP ResolveClass (Utf8StringCR classFullName);
    MergeStatus ResolveClassFromMergeContext (ECN::ECClassCP& mergedClass, Utf8CP className);
    bool TryGetECRelationshipConstraint (ECN::ECRelationshipConstraintCP & relationshipConstraint,  ECDiffNodeCR relationshipConstraintNode, ECN::ECSchemaCR schema);
    bool TryGetECProperty (ECN::ECPropertyCP& ecProperty,  ECDiffNodeCR propertyNode, ECN::ECSchemaCR schema, bool includeBaseClass);
    bool TryGetECClass (ECN::ECClassCP& ecclass,  ECDiffNodeCR classNode, ECN::ECSchemaCR schema);

public:
    ECSchemaMergeTool(ECDiffR diff, ConflictRule defaultRule);
    static MergeStatus Merge(ECN::ECSchemaPtr& mergedSchema, ECDiffR diff, ConflictRule defaultRule);
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaDiffTool
    {
private:
    static ECDiffNodeP DiffReferences(ECDiffNodeR parentDiff, ECN::ECSchemaCR schemaLeft, ECN::ECSchemaCR schemaRight);
    static ECDiffNodeP DiffClass (Utf8CP className, ECN::ECSchemaCR schemaLeft, ECN::ECSchemaCR schemaRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffBaseClasses (ECDiffNodeR parentDiff, AlignedClasses const& classes);
    static ECDiffNodeP DiffProperty (Utf8CP propertyName, ECN::ECClassCR classLeft, ECN::ECClassCR classRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffRelationship (ECN::ECClassCR classLeft, ECN::ECClassCR classRight, ECDiffNodeR parentDiff);
    static ECDiffNodeP DiffRelationshipConstraint(ECDiffNodeR parent, ECN::ECRelationshipConstraintCR  left, ECN::ECRelationshipConstraintCR  right, ECN::ECRelationshipEnd endPoint);
    static ECDiffNodeP DiffRelationshipConstraintClasses (ECDiffNodeR parentDiff, bvector<ECN::ECRelationshipConstraintCP> const& constraints, ECN::ECRelationshipEnd endPoint);
    static ECDiffNodeP DiffArrayBounds (ECDiffNodeR parent , ECN::ECPropertyCR left, ECN::ECPropertyCR right);
    static ECDiffNodeP DiffCustomAttributes (ECDiffNodeR parentDiff, ECN::IECCustomAttributeContainerCR leftContainer, ECN::IECCustomAttributeContainerCR rightContainer);
    static ECDiffNodeP DiffInstance (ECDiffNodeR parentDiff, ECN::ECClassCR customAttributeClass, ECN::IECCustomAttributeContainerCR leftContainer, ECN::IECCustomAttributeContainerCR rightContainer);

    static ECDiffNodeP AppendRelationship(ECDiffNodeR parent, ECN::ECRelationshipClassCR relationship, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendRelationshipConstraint(ECDiffNodeR parent, ECN::ECRelationshipConstraintCR  relationshipConstraint, ECN::ECRelationshipEnd endPoint, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendClass (ECDiffNodeR parent , ECN::ECClassCR ecClass, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendProperty (ECDiffNodeR parent , ECN::ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendArrayBounds (ECDiffNodeR parent , ECN::ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendCustomAttributes (ECDiffNodeR parentDiff, ECN::IECCustomAttributeContainerCR container, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendInstance (ECDiffNodeR parentDiff, ECN::IECInstanceCR instance, ECDiffNode::ValueDirection direction);
    static ECDiffNodeP AppendPropertyValues (ECDiffNodeR parentDiff, ECN::ECValuesCollectionCR values, ECDiffNode::ValueDirection direction);

    static Utf8CP ToString(ECN::ECRelatedInstanceDirection direction);
    static Utf8CP ToString(ECN::StrengthType type);
    static void CollectInstanceValues (bmap<Utf8String, ECN::ECValue>& valueMap, std::set<Utf8String>& accessStrings, ECN::IECInstanceCR instance);
    static void CollectInstanceValues (bmap<Utf8String, ECN::ECValue>& valueMap, std::set<Utf8String>& accessStrings, ECN::ECValuesCollectionCR values);
    static bool SetECValue (ECDiffNodeR n, ECN::ECValueCR v, ECDiffNode::ValueDirection direction);
    //Merge
public:
    static ECDiffNodeP Diff (ECN::ECSchemaCR left, ECN::ECSchemaCR right);
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
