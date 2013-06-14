
#include "ECObjectsPch.h"
#include <stack>
#include <set>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
using namespace std;

#define ID_ROOT L"ECSchemaDiff"
#define ID_NAME L"Name"
#define ID_DISPLAY_LABEL L"DisplayLabel"
#define ID_DESCRIPTION L"Description"
#define ID_VERSION_MAJOR L"VersionMajor"
#define ID_VERSION_MINOR L"VersionMinor"
#define ID_CLASSES L"Classes"
#define ID_REFERENCES L"References"
#define ID_IS_CUSTOMATTRIBUTE_CLASS L"IsCustomAttributeClass"
#define ID_IS_STRUCT L"IsStruct"
#define ID_IS_DOMAIN_CLASS L"IsDomainClass"
#define ID_IS_RELATIONSHIP_CLASS L"IsRelationshipClass"
#define ID_BASECLASSES L"BaseClasses"
#define ID_PROPERTIES L"Properties"
#define ID_IS_ARRAY L"IsArray"
#define ID_TYPENAME L"TypeName"
#define ID_ARRAYINFO L"ArrayInfo"
#define ID_IS_READONLY L"IsReadOnly"
#define ID_IS_PRIMITIVE L"IsPrimitive"
#define ID_RELATIONSHIP_INFO L"RelationshipInfo"
#define ID_IS_OVERRIDEN L"IsOverriden"
#define ID_STRENGTH L"Strength"
#define ID_STRENGTH_DIRECTION L"StrengthDirection"
#define ID_SOURCE L"Source"
#define ID_TARGET L"Target"
#define ID_CARDINALITY L"Cardinality"
#define ID_IS_POLYMORPHIC L"IsPolymorphic"
#define ID_ROLE_LABEL L"RoleLabel"
#define ID_STRENGTH_DIRECTION_BACKWARD L"Backward"
#define ID_STRENGTH_DIRECTION_FORWARD L"Forward"
#define ID_STRENGTH_TYPE_EMBEDDING L"Embedding"
#define ID_STRENGTH_TYPE_HOLDING L"Holding"
#define ID_STRENGTH_TYPE_REFERENCING L"Referencing"
#define ID_MAXOCCURS L"MaxOccurs"
#define ID_MINOCCURS L"MinOccurs"
#define ID_CUSTOMATTRIBUTES L"CustomAttributes"
#define ID_NAMESPACEPREFIX L"NamespacePrefix"
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECDiffNodeCP ECDiff::GetRootNode () const
    {
    return const_cast<ECDiff*>(this)->GetRoot();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiff::ECDiff(ECSchemaR left, ECSchemaR right, ECDiffNodeP diff, DiffStatus status)
    {
    m_leftSchema = &left;
    m_rightSchema = &right;
    m_diff = diff;
    m_status = status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiff::GetRoot()
    {
    return m_diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECDiff::GetLeftSchema() const
    {
    return *m_leftSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECDiff::GetRightSchema() const
    {
    return *m_rightSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DiffStatus ECDiff::GetStatus() const
    {
    return m_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DiffStatus ECDiff::WriteToString(WStringR out, int tabSize)
    {
    BeAssert (GetStatus() == DIFFSTATUS_Success);
    if (GetStatus() == DIFFSTATUS_Success)
        out = GetRoot()->ToString (tabSize);
    return GetStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffPtr ECDiff::Diff (ECN::ECSchemaCR left, ECN::ECSchemaCR right)
    {
    ECDiffNodeP out = ECSchemaDiffTool::Diff (left, right);
    BeAssert(out != NULL);
    return new ECDiff( const_cast<ECSchemaR>(left), const_cast<ECSchemaR>(right), out, DIFFSTATUS_Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus  ECDiff::Merge (ECN::ECSchemaPtr& mergedSchema, ConflictRule rule)
    {
    return ECSchemaMergeTool::Merge (mergedSchema, *this, rule);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECDiff::IsEmpty() 
    {
    BeAssert(GetStatus() == DIFFSTATUS_Success);
    if (GetStatus() == DIFFSTATUS_Success && m_diff)
        return GetRoot()->IsEmpty();
    return true;
    }

DiffStatus ECDiff::GetNodesState (bmap<WString, DiffNodeState>& nodes, WStringCR accessString, bool bAccumlativeState)
    {
    return GetRoot()->GetNodeState (nodes, accessString, bAccumlativeState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue::ECDiffValue()
    : m_type (VALUETYPE_Nil), m_valueInt64(0),m_binarySize(0)
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (WCharCP v) 
    {
    m_valueString.clear();
    if (v == NULL)
       SetNil();
    else
        m_valueString = v ; 
    m_type = VALUETYPE_String; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (byte const* v, size_t size) 
    { 
    m_type = VALUETYPE_Binary;
    m_valueBinary = (byte*)malloc(size);
    memcpy (m_valueBinary, v, size);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (WStringCR v) { m_valueString = v ; m_type = VALUETYPE_String; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (Int32 v) { m_valueInt32 = v ; m_type = VALUETYPE_Int32; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (Int64 v) { m_valueInt64 = v ; m_type = VALUETYPE_Int64; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (double v) { m_valueDouble = v ; m_type = VALUETYPE_Double; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (bool v) { m_valueBool = v ; m_type = VALUETYPE_Boolean; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetDateTimeValue(Int64 v) { m_valueDateTime = v ; m_type = VALUETYPE_DateTimeTicks; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetNil()
    {
    m_valueInt64 = 0 , m_valueString.clear(); 
    m_type = VALUETYPE_Nil;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* ECDiffValue::GetBinary(size_t& size) const
    { 
    BeAssert(m_type == VALUETYPE_Binary); 
    size= m_binarySize;
    return m_valueBinary;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECDiffValue::GetValueWCharCP() const
    { 
    BeAssert(m_type == VALUETYPE_String); 
    return m_valueString.c_str();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 ECDiffValue::GetValueInt32() const
    { 
    BeAssert(m_type == VALUETYPE_Int32); 
    return m_valueInt32;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Int64 ECDiffValue::GetValueInt64() const
    { 
    BeAssert(m_type == VALUETYPE_Int64); 
    return m_valueInt64;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::GetValueBool() const
    { 
    BeAssert(m_type == VALUETYPE_Boolean); 
    return m_valueBool;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
double ECDiffValue::GetValueDouble() const
    { 
    BeAssert(m_type == VALUETYPE_Double); 
    return m_valueDouble;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::IsValueNill() const { return m_type == VALUETYPE_Nil; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::Clear()
    {
    if (m_type == ECDiffValue::VALUETYPE_Binary)
        free(m_valueBinary);
    m_type = VALUETYPE_Nil;
    m_valueString.clear();
    m_valueInt64 = 0;
    m_binarySize = 0;
    m_valueBinary = NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::IsEqual(ECDiffValue const& value)
    {
    if (m_type != value.m_type)
        return false;
    switch(m_type)
        {
    case VALUETYPE_Boolean:
        return m_valueBool == value.m_valueBool;
    case VALUETYPE_Double:
        return m_valueDouble == value.m_valueDouble;
    case VALUETYPE_Int32:
        return m_valueInt32 == value.m_valueInt32;
    case VALUETYPE_Int64:
        return m_valueInt64 == value.m_valueInt64;
    case VALUETYPE_String:
        return m_valueString == value.m_valueString;
    case VALUETYPE_Nil:
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffValue::ToString() const
    {
    WString out;
    switch (m_type)
        {
        case VALUETYPE_Boolean:
            WString::Sprintf (out, L"%ls", m_valueBool?L"true" :L"false"); break;
        case VALUETYPE_Double:
            WString::Sprintf (out, L"%f", m_valueDouble); break;
        case VALUETYPE_Int32:
            WString::Sprintf (out, L"%d", m_valueInt32); break;
        case VALUETYPE_Int64:
            WString::Sprintf (out, L"%ld", m_valueInt64); break;
        case VALUETYPE_String:
            WString::Sprintf (out, L"\"%ls\"", m_valueString.c_str()); break;
        case VALUETYPE_Nil:
            out = L"<nil>"; break;
        }
    return out;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECDiffValue::GetValueAsECValue() const
    {
    switch (m_type)
        {
        case VALUETYPE_Boolean:
            return ECValue(GetValueBool());
        case VALUETYPE_Double:
            return ECValue(GetValueDouble());
        case VALUETYPE_Int32:
            return ECValue(GetValueInt32());
        case VALUETYPE_Int64:
            return ECValue(GetValueInt64());
        case VALUETYPE_String:
            return ECValue(GetValueString().c_str());
        case VALUETYPE_Nil:
            return ECValue();
        }
    BeAssert( false && "Case not handled");
    return ECValue();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::_Write (WStringR out, int indent, WStringCR tab) const
    {
    WString prefix;
    WString eol = L"\r\n";
    int i = indent;
    while (i-- >= 1)
        prefix.append (tab);
    indent = indent + 1;
    //Left value is new
    if (!m_valueLeft.IsValueNill() && m_valueRight.IsValueNill())
        {
        out.append (L"L");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueLeft.ToString());
        out.append (eol);
        }
    //Right value is new
    else if (m_valueLeft.IsValueNill() && !m_valueRight.IsValueNill())
        {
        out.append (L"R");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueRight.ToString());
        out.append (eol);
        }
    //Conflict
    else if (!m_valueLeft.IsValueNill() && !m_valueRight.IsValueNill())
        {
        out.append (L"!");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueLeft.ToString());
        out.append (L" <> ");
        out.append (m_valueRight.ToString());
        out.append (eol);
        }
    else // this is a object output its child nodes
        {
        out.append (L" ");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (eol);
        for (const_iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
            {
            (*itor)->_Write (out, indent, tab);
            }
        //out.append (prefix);
        //out.append (L"}");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Find (WCharCP name)
    {
    ECDiffNodeMap::const_iterator itor = m_childNodeMap.find (name);
    if (itor != m_childNodeMap.end())
        return itor->second;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DiffStatus ECDiffNode::GetNodeState (bmap<WString, DiffNodeState>& nodes, WStringCR accessString, bool bAccumlativeState)
    {
    BeAssert(!accessString.empty());
    bvector<WString> accessors;
    WString delimiter = L".";

    WString::size_type j = 0;
    WString::size_type i = accessString.find (delimiter, j);
 
    do 
        {
        if (i == WString::npos)
            i = accessString.size();

        WString accessor = accessString.substr (j, i - j);
        if (accessor.empty())
            return DIFFSTATUS_InvalidAccessString;

        if (!accessors.empty() && accessor == L"*")
            {
            WStringCR lastAccessor = accessors [accessor.size() - 1];
            if (lastAccessor != accessor)
                accessors.push_back (accessor);
            }
        else
            accessors.push_back (accessor);
        j = i + delimiter.size();
        i = accessString.find (delimiter, j);
        } while (j < accessString.size());
 

    if (accessors.empty())
        return DIFFSTATUS_InvalidAccessString;

    stack<WString> accessorStack;
    for (bvector<WString>::const_reverse_iterator itor = accessors.rbegin() ; itor != accessors.rend() ; ++itor )
        accessorStack.push(*itor);

    _GetNodeState (nodes, accessorStack, bAccumlativeState);
    return DIFFSTATUS_Success;
    }

DiffNodeState ConvertToDiffNodeState(DiffType type)
    {
    if (type == DIFFTYPE_Left)
        return DIFFNODESTATE_AppendLeft;
    if (type == DIFFTYPE_Right)
        return DIFFNODESTATE_AppendRight;
    return DIFFNODESTATE_Conflict;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::_GetNodeState (bmap<WString, DiffNodeState>& nodes, stack<WString> accessors , bool bAccumlativeState)
    {
    if (accessors.empty())
        {
        BeAssert(false);
        return;
        }
    bool wildCard = (accessors.top() == L"*");
    if (wildCard && accessors.size() == 1)
        {
        nodes [GetAccessString()] = ConvertToDiffNodeState (ImplGetDiffType (bAccumlativeState));
        for (const_iterator itor = begin(); itor != end(); ++itor)
            (*itor)->_GetNodeState (nodes, accessors, bAccumlativeState);
        return;
        }

    if (wildCard)
        accessors.pop();

    if (accessors.top() == GetName())
        {
        accessors.pop();
        if (accessors.empty())
            nodes [GetAccessString()] = ConvertToDiffNodeState (ImplGetDiffType (bAccumlativeState));
        else
            for (const_iterator itor = begin(); itor != end(); ++itor)
                (*itor)->_GetNodeState (nodes, accessors, bAccumlativeState);
    
        return;
        } 
    if (wildCard)
        {
        accessors.push(L"*");
        for (const_iterator itor = begin(); itor != end(); ++itor)
            (*itor)->_GetNodeState (nodes, accessors, bAccumlativeState);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::ECDiffNode (WCharCP name, ECDiffNodeP parent, DiffNodeId id, int index) 
    : m_parent(parent), m_id(id), m_index(index),m_cachedAccumlativeType(DIFFTYPE_Empty)
    {
    BeAssert(name != NULL);
    m_name = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffNode::GetAccessString() const
    {
    if (GetParent() == NULL)
        return GetName();
    return GetParent()->GetName() + L"." + GetName();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::ImplGetValueLeft() {return m_valueLeft; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::ImplGetValueRight() {return m_valueRight; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::GetValue (ValueDirection direction) { return direction== DIRECTION_Left ? ImplGetValueLeft() : ImplGetValueRight();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (WCharCP left, WCharCP right)
    {
    if ( left || right )
        {
        m_valueLeft.SetValue (left);
        m_valueRight.SetValue (right);
        return true;
        }
    BeAssert( false && "left and right both values are null");
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (bool left, bool right)
    {
    m_valueLeft.SetValue (left);
    m_valueRight.SetValue (right);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (UInt32 left, UInt32 right)
    {
    m_valueLeft.SetValue ((Int32)left);
    m_valueRight.SetValue ((Int32)right);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::const_iterator ECDiffNode::begin() { return m_childNodeList.begin(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::const_iterator ECDiffNode::end() { return m_childNodeList.end(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::size_type ECDiffNode::size () { return m_childNodeList.size(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeCP ECDiffNode::GetParent() const { return m_parent ; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Add (WCharCP name, DiffNodeId type)
    {
    if (name == NULL)
        name = IdToString (type);
    BeAssert (name != NULL);
    if (m_childNodeMap.find (name) != m_childNodeMap.end())
        {
        BeAssert(false && "ECDiffNode with same name already exist");
        return NULL;
        }

    ECDiffNodeP node = new ECDiffNode (name, this, type, -1);
    m_childNodeMap [node->GetName().c_str()] = node;
    m_childNodeList.push_back(node);
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Add (DiffNodeId type)
    {
    return Add ((WCharCP)NULL, type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::AddWithIndex (int index, DiffNodeId type)
    {
    WString idx;
    WString::Sprintf (idx, L"%ls[%d]", m_name.c_str(), index);
    if (m_childNodeMap.find (idx.c_str()) != m_childNodeMap.end())
        {
        BeAssert(false && "ECDiffNode with same name already exist");
        return NULL;
        }
    ECDiffNodeP node = new ECDiffNode (idx.c_str(), this, type, index);
    m_childNodeMap [node->GetName().c_str()] = node;
    m_childNodeList.push_back(node);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DiffType ECDiffNode::ImplGetDiffType (bool bRecursively) 
    {
    if (m_cachedAccumlativeType != DIFFTYPE_Empty)
        return m_cachedAccumlativeType;
    m_cachedAccumlativeType = _GetDiffType(bRecursively);
    return m_cachedAccumlativeType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DiffType ECDiffNode::_GetDiffType (bool bRecursively) 
    {
    DiffType type;
    if (ImplGetValueLeft().IsValueNill() && ImplGetValueRight().IsValueNill())
        type = DIFFTYPE_Empty;
    else if (!ImplGetValueLeft().IsValueNill() && ImplGetValueRight().IsValueNill())
        type = DIFFTYPE_Left;
    else if (!ImplGetValueRight().IsValueNill() && ImplGetValueLeft().IsValueNill())
        type = DIFFTYPE_Right;
    else
        type = DIFFTYPE_Conflict;

    if (!bRecursively)
        return type;

    for(const_iterator itor = begin(); itor != end(); ++itor)
        {
        DiffType childType = (*itor)->ImplGetDiffType (bRecursively);
        if (type == DIFFTYPE_Empty)
            if (childType != DIFFTYPE_Empty)
                type = childType;
        if (type == childType || childType == DIFFTYPE_Empty)
            continue;
        if (type != childType)
            return DIFFTYPE_Conflict;
        }
    return type;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::GetChild (WStringCR accessString, bool bCreate)
    {
    WString cur, rest;
    size_t n = accessString.find (L".");
    if (n == WString::npos)
        cur = accessString;
    else
        {
        cur = accessString.substr (0, n);
        rest = accessString.substr (n + 1);
        }
    ECDiffNodeMap::const_iterator itor = m_childNodeMap.find (cur.c_str());
    if (itor != m_childNodeMap.end())
        if (rest.empty())
            return itor->second;
        else
            return itor->second->GetChild (rest, bCreate);
    if (bCreate)
        if (rest.empty())
            return Add (cur.c_str(), DiffNodeId::None);
        else
            return Add (cur.c_str(), DiffNodeId::None)->GetChild (rest, bCreate);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::Clear()
    {
    m_childNodeMap.clear();
    for (ECDiffNodeList::const_iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
        delete *itor;
    m_childNodeList.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::RemoveIfEmpty (ECDiffNodeP n)
    {
    BeAssert(n != NULL && n->GetParent() == this);
    if (n->IsEmpty())
        {
        Remove(n);
        return NULL;
        }
    return n;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::Remove (ECDiffNodeP n)
    {
    for (ECDiffNodeList::iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
        if (*itor == n)
            {
                m_childNodeList.erase(itor);
                break;
            }
    m_childNodeMap.erase(n->GetName().c_str());
    delete n;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffNode::ToString(int tabSize) const
    {
    WString out;
    WString tab;
    BeAssert (tabSize > 0);
    if (tabSize <= 0 )
        tabSize = 1;
    do tab.append (L" "); while (--tabSize > 0);
    _Write (out, 0, tab);
    return out;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::~ECDiffNode()
    {
    Clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::IsEmpty()
    {
    return size() == 0 && ImplGetValueLeft().IsValueNill() && ImplGetValueRight().IsValueNill();
    }

WCharCP  ECDiffNode::IdToString (DiffNodeId id)
    {
    switch (id)
    {
       case DiffNodeId::Schema:
       case DiffNodeId::Reference: 
       case DiffNodeId::Class:
       case DiffNodeId::Property: 
       case DiffNodeId::None:
       case DiffNodeId::CustomAttribute:
        case DiffNodeId::ConstraintClass:
           return NULL;

        case DiffNodeId::Root: return ID_ROOT;
        case DiffNodeId::Name: return ID_NAME;
        case DiffNodeId::DisplayLabel: return ID_DISPLAY_LABEL;
        case DiffNodeId::Description: return ID_DESCRIPTION;
        case DiffNodeId::VersionMajor: return ID_VERSION_MAJOR;
        case DiffNodeId::VersionMinor:return ID_VERSION_MINOR;
        case DiffNodeId::ConstraintClasses: 
        case DiffNodeId::Classes: 
            return ID_CLASSES;
        case DiffNodeId::References: return ID_REFERENCES;
        case DiffNodeId::IsCustomAttributeClass: return ID_IS_CUSTOMATTRIBUTE_CLASS;
        case DiffNodeId::IsDomainClass: return ID_IS_DOMAIN_CLASS;
        case DiffNodeId::IsStruct: return ID_IS_STRUCT;
        case DiffNodeId::BaseClass: return ID_BASECLASSES;
        case DiffNodeId::IsRelationshipClass: return ID_IS_RELATIONSHIP_CLASS;
        case DiffNodeId::BaseClasses: return ID_BASECLASSES;
        case DiffNodeId::IsArray: return ID_IS_ARRAY;
        case DiffNodeId::Properties: return ID_PROPERTIES;
        case DiffNodeId::ArrayInfo: return ID_ARRAYINFO;
        case DiffNodeId::IsReadOnly: return ID_IS_READONLY;
        case DiffNodeId::IsPrimitive: return ID_IS_PRIMITIVE;
        case DiffNodeId::RelationshipInfo: return ID_RELATIONSHIP_INFO;
        case DiffNodeId::IsOverriden: return ID_IS_OVERRIDEN;
        case DiffNodeId::TypeName: return ID_TYPENAME;
        case DiffNodeId::Strength: return ID_STRENGTH;
        case DiffNodeId::StrengthDirection:return ID_STRENGTH_DIRECTION;
        case DiffNodeId::Source: return ID_SOURCE;
        case DiffNodeId::Target:return ID_TARGET;
        case DiffNodeId::Cardinality: return ID_CARDINALITY;
        case DiffNodeId::IsPolymorphic: return ID_IS_POLYMORPHIC;
        case DiffNodeId::RoleLabel: return ID_ROLE_LABEL;
        case DiffNodeId::MaxOccurs: return ID_MAXOCCURS;
        case DiffNodeId::MinOccurs: return ID_MINOCCURS;
        case DiffNodeId::CustomAttributes: return ID_CUSTOMATTRIBUTES;
        case DiffNodeId::NamespacePrefix: return ID_NAMESPACEPREFIX;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::Diff(ECSchemaCR left, ECSchemaCR right)
    {
    ECDiffNodeP diff = new ECDiffNode (ID_ROOT, NULL, DiffNodeId::Root);

    if (left.GetName() != right.GetName())
        diff->Add (DiffNodeId::Name)->SetValue (left.GetName().c_str(), right.GetName().c_str());

    if (left.GetVersionMajor() != right.GetVersionMajor())
        diff->Add (DiffNodeId::VersionMajor)->SetValue (left.GetVersionMajor(), right.GetVersionMajor());

    if (left.GetVersionMinor() != right.GetVersionMinor())
        diff->Add (DiffNodeId::VersionMinor)->SetValue (left.GetVersionMinor(), right.GetVersionMinor());

    if (left.GetDisplayLabel() != right.GetDisplayLabel())
        diff->Add (DiffNodeId::DisplayLabel)->SetValue (left.GetIsDisplayLabelDefined()? left.GetDisplayLabel().c_str() : NULL, right.GetIsDisplayLabelDefined()? right.GetDisplayLabel().c_str(): NULL);

    if (left.GetDescription() != right.GetDescription())
        diff->Add (DiffNodeId::Description)->SetValue (left.GetDescription().c_str(), right.GetDescription().c_str());

    if (left.GetNamespacePrefix() != right.GetNamespacePrefix())
        diff->Add (DiffNodeId::NamespacePrefix)->SetValue (left.GetNamespacePrefix().c_str(), right.GetNamespacePrefix().c_str());

    DiffReferences (*diff, left, right);
    DiffCustomAttributes (*diff, left, right);
    set<WCharCP, DiffNameComparer> classes;
    ECClassContainerCR classesLeft = left.GetClasses();
    for (ECClassContainer::const_iterator itor = classesLeft.begin(); itor != classesLeft.end(); ++itor)
        if (classes.find ((*itor)->GetName().c_str()) == classes.end())
            classes.insert ((*itor)->GetName().c_str());

    ECClassContainerCR classesRight = right.GetClasses();
    for (ECClassContainer::const_iterator itor = classesRight.begin(); itor != classesRight.end(); ++itor)
        if (classes.find ((*itor)->GetName().c_str()) == classes.end())
            classes.insert ((*itor)->GetName().c_str());

    ECDiffNodeP diffClasses = diff->Add (DiffNodeId::Classes);
    for (std::set<WCharCP,  DiffNameComparer>::const_iterator itor = classes.begin(); itor != classes.end(); ++itor)
        DiffClass ((*itor), left, right, *diffClasses);

    if (diffClasses->IsEmpty())
        diff->Remove(diffClasses);

    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffReferences(ECDiffNodeR parentDiff, ECSchemaCR schemaLeft, ECSchemaCR schemaRight)
        {
        ECSchemaReferenceListCR left = schemaLeft.GetReferencedSchemas();
        ECSchemaReferenceListCR right = schemaRight.GetReferencedSchemas();
   
        set<WString> referenceSchemas;
        WString namespacePrefix;
        for (ECSchemaReferenceList::const_iterator itor = left.begin(); itor != left.end() ; ++itor)
            if (schemaLeft.ResolveNamespacePrefix (*itor->second, namespacePrefix) == ECOBJECTS_STATUS_Success)
                if (referenceSchemas.find (namespacePrefix) == referenceSchemas.end())
                    referenceSchemas.insert(namespacePrefix);

        for (ECSchemaReferenceList::const_iterator itor = right.begin(); itor != right.end() ; ++itor)
            if (schemaRight.ResolveNamespacePrefix (*itor->second, namespacePrefix) == ECOBJECTS_STATUS_Success)
                if (referenceSchemas.find (namespacePrefix) == referenceSchemas.end())
                    referenceSchemas.insert(namespacePrefix);

        if (referenceSchemas.empty())
            return NULL;
    ECDiffNodeP diff= parentDiff.Add (DiffNodeId::References);
        for(set<WString>::const_iterator itor = referenceSchemas.begin(); itor != referenceSchemas.end(); ++itor)
            {
            ECSchemaCP leftR =  schemaLeft.GetSchemaByNamespacePrefixP (*itor);
            ECSchemaCP rightR =  schemaRight.GetSchemaByNamespacePrefixP (*itor);
            if (leftR && !rightR)
                diff->Add ((*itor).c_str(), DiffNodeId::Reference)->ImplGetValueLeft().SetValue (leftR->GetFullSchemaName().c_str());
            else if (!leftR && rightR)
                diff->Add ((*itor).c_str(), DiffNodeId::Reference)->ImplGetValueRight().SetValue (rightR->GetFullSchemaName().c_str());
            else
                if (leftR->GetSchemaKey() != rightR->GetSchemaKey())
                    diff->Add ((*itor).c_str(), DiffNodeId::Reference)->SetValue (leftR->GetFullSchemaName().c_str(), rightR->GetFullSchemaName().c_str());
            }
        return parentDiff.RemoveIfEmpty (diff);
        }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffClass (WCharCP className, ECSchemaCR schemaLeft, ECSchemaCR schemaRight, ECDiffNodeR parentDiff)
    {
    ECClassCP left = schemaLeft.GetClassCP(className);
    ECClassCP right = schemaRight.GetClassCP(className);
 
    if (left && !right)
        return AppendClass (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    if (right && !left)
        return AppendClass (parentDiff, *right, ECDiffNode::DIRECTION_Right);

    ECDiffNodeP diff = parentDiff.Add (className, DiffNodeId::Class);
    //if (left->GetName() != right->GetName())
    //    diff->Add (ID_NAME)->SetValue (left->GetName().c_str(), right->GetName().c_str());

    if (left->GetDisplayLabel() != right->GetDisplayLabel())
        diff->Add (DiffNodeId::DisplayLabel)->SetValue (left->GetIsDisplayLabelDefined()? left->GetDisplayLabel().c_str() : NULL, right->GetIsDisplayLabelDefined()? right->GetDisplayLabel().c_str(): NULL);

    if (left->GetDescription() != right->GetDescription())
        diff->Add (DiffNodeId::Description)->SetValue (left->GetDescription().c_str(), right->GetDescription().c_str());

    if (left->GetIsCustomAttributeClass() != right->GetIsCustomAttributeClass())
        diff->Add (DiffNodeId::IsCustomAttributeClass)->SetValue (left->GetIsCustomAttributeClass(), right->GetIsCustomAttributeClass());

    if (left->GetIsStruct() != right->GetIsStruct())
        diff->Add (DiffNodeId::IsStruct)->SetValue (left->GetIsStruct(), right->GetIsStruct());

    if (left->GetIsDomainClass() != right->GetIsDomainClass())
        diff->Add (DiffNodeId::IsDomainClass)->SetValue (left->GetIsDomainClass(), right->GetIsDomainClass());

    if (( (left->GetRelationshipClassCP() != NULL))!= (right->GetRelationshipClassCP() != NULL))
        diff->Add (DiffNodeId::IsRelationshipClass)->SetValue (left->GetRelationshipClassCP() != NULL, right->GetRelationshipClassCP() != NULL );

    DiffCustomAttributes (*diff, *left, *right);
    AlignedClasses classes; 
    classes.push_back(left);
    classes.push_back(right);
    DiffBaseClasses(*diff, classes);

    std::set<WCharCP, DiffNameComparer> properties;
    ECPropertyIterableCR propertiesLeft = left->GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = propertiesLeft.begin(); itor != propertiesLeft.end(); ++itor)
        if (properties.find ((*itor)->GetName().c_str()) == properties.end())
            properties.insert ((*itor)->GetName().c_str());

    ECPropertyIterableCR propertiesRight = right->GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = propertiesRight.begin(); itor != propertiesRight.end(); ++itor)
        if (properties.find ((*itor)->GetName().c_str()) == properties.end())
            properties.insert ((*itor)->GetName().c_str());

    ECDiffNodeP diffProperties = diff->Add (DiffNodeId::Properties);
    for (std::set<WCharCP,  DiffNameComparer>::const_iterator itor = properties.begin(); itor != properties.end(); ++itor)
        DiffProperty ((*itor), *left, *right, *diffProperties);
    diff->RemoveIfEmpty (diffProperties);

    DiffRelationship (*left, *right, *diff);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffBaseClasses (ECDiffNodeR parentDiff, AlignedClasses const& classes)
    {
    BeAssert (classes.size() == 2);
    AlignedClassMap baseClassMap;
    bvector<WCharCP> baseClassOrderedList;
    for (int i = 0 ; i < (int)classes.size(); i++)
        {
        ECClassCP ecclass = classes[i];
        if (ecclass == NULL)
            continue;

        ECBaseClassesList baseClassContainer = ecclass->GetBaseClasses();
        for (ECBaseClassesList::const_iterator baseClassItor = baseClassContainer.begin(); baseClassItor != baseClassContainer.end(); ++baseClassItor)
            {
            ECClassCR baseClass = *(*baseClassItor);
            if (baseClassMap.find (baseClass.GetFullName())== baseClassMap.end())
                baseClassOrderedList.push_back (baseClass.GetFullName());
            AlignedClasses& baseClasses = baseClassMap [baseClass.GetFullName()];
            int nNullEntriesRequired = i - (int)baseClasses.size();
            if (nNullEntriesRequired > 0)
                do
                    {
                    baseClasses.push_back (NULL);
                    } while (--nNullEntriesRequired);
                baseClasses.push_back (&baseClass);
            }
        }
    for (bvector<WCharCP>::iterator itor = baseClassOrderedList.begin(); itor != baseClassOrderedList.end(); ++itor)
        if (baseClassMap[*itor].size() < classes.size())
            while (baseClassMap[*itor].size() < classes.size())
                baseClassMap[*itor].push_back(NULL);

    if (baseClassOrderedList.empty())
        return NULL;

    ECDiffNodeP baseClasses = parentDiff.Add (DiffNodeId::BaseClasses);
    int index = 0;
    for (bvector<WCharCP>::iterator itor = baseClassOrderedList.begin(); itor != baseClassOrderedList.end(); ++itor, index++)
        {
        ECClassCP left = baseClassMap[*itor][0];
        ECClassCP right = baseClassMap[*itor][1];
        if (left && !right)
            baseClasses->AddWithIndex (index, DiffNodeId::BaseClass)->ImplGetValueLeft().SetValue(left->GetFullName());
        else if(!left && right)
            baseClasses->AddWithIndex (index, DiffNodeId::BaseClass)->ImplGetValueRight().SetValue(right->GetFullName());
        else
            if (wcscmp (left->GetFullName(), right->GetFullName()) != 0)
                baseClasses->AddWithIndex (index, DiffNodeId::BaseClass)->SetValue(left->GetFullName(), right->GetFullName());
        }
    return parentDiff.RemoveIfEmpty (baseClasses);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffProperty (WCharCP propertyName, ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff)
    {
    ECPropertyCP left = classLeft.GetPropertyP (propertyName);
    ECPropertyCP right = classRight.GetPropertyP (propertyName);
        
    if (left && !right)
        return AppendProperty (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    if (right && !left)
        return AppendProperty (parentDiff, *right, ECDiffNode::DIRECTION_Right);
    ECDiffNodeP diff = parentDiff.Add (propertyName, DiffNodeId::Property);

    //if (left->GetName() != right->GetName())
    //    diff->Add (ID_NAME)->SetValue (left->GetName().c_str(), right->GetName().c_str());

    if (left->GetDisplayLabel() != right->GetDisplayLabel())
        diff->Add (DiffNodeId::DisplayLabel)->SetValue (left->GetIsDisplayLabelDefined()? left->GetDisplayLabel().c_str() : NULL, right->GetIsDisplayLabelDefined()? right->GetDisplayLabel().c_str(): NULL);

    if (left->GetDescription() != right->GetDescription())
        diff->Add (DiffNodeId::Description)->SetValue (left->GetDescription().c_str(), right->GetDescription().c_str());

    if (left->GetIsArray() != right->GetIsArray())
        diff->Add (DiffNodeId::IsArray)->SetValue (left->GetIsArray(), right->GetIsArray());

    if (left->GetIsStruct() != right->GetIsStruct())
        diff->Add (DiffNodeId::IsStruct)->SetValue (left->GetIsStruct(), right->GetIsStruct());

    if (left->GetTypeName() != right->GetTypeName())
        diff->Add (DiffNodeId::TypeName)->SetValue (left->GetTypeName().c_str(), right->GetTypeName().c_str());

    if (left->GetIsReadOnly() != right->GetIsReadOnly())
        diff->Add (DiffNodeId::IsReadOnly)->SetValue (left->GetIsReadOnly(), right->GetIsReadOnly());

    if (left->GetIsPrimitive() != right->GetIsPrimitive())
        diff->Add (DiffNodeId::IsPrimitive)->SetValue (left->GetIsPrimitive(), right->GetIsPrimitive());

    bool leftOverriden = left->GetBaseProperty() != NULL;
    bool rightOverriden = right->GetBaseProperty() != NULL;
    if (leftOverriden != rightOverriden)
        diff->Add (DiffNodeId::IsOverriden)->SetValue (leftOverriden, rightOverriden);

    DiffArrayBounds(*diff, *left, *right);
    DiffCustomAttributes (*diff, *left, *right);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffRelationship (ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff)
    {
    ECRelationshipClassCP left = classLeft.GetRelationshipClassCP();
    ECRelationshipClassCP right = classRight.GetRelationshipClassCP();
    if (!left && !right)
        return NULL;
    else if (left && !right)
        return AppendRelationship(parentDiff, *left, ECDiffNode::DIRECTION_Left );
    else if (!left && right)
        return AppendRelationship(parentDiff, *right, ECDiffNode::DIRECTION_Right );
        
    ECDiffNodeP diff = parentDiff.Add (DiffNodeId::RelationshipInfo);

    if (left->GetStrength() != right->GetStrength())
        diff->Add (DiffNodeId::Strength)->SetValue (ToString (left->GetStrength()), ToString (right->GetStrength()));

    if (left->GetStrengthDirection() != right->GetStrengthDirection())
        diff->Add (DiffNodeId::StrengthDirection)->SetValue (ToString (left->GetStrengthDirection()), ToString (right->GetStrengthDirection()));

    DiffRelationshipConstraint(*diff, left->GetSource(), right->GetSource(), ECRelationshipEnd_Source);
    DiffRelationshipConstraint(*diff, left->GetTarget(), right->GetTarget(), ECRelationshipEnd_Target);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  left, ECRelationshipConstraintCR  right, ECRelationshipEnd endPoint)
    {
    ECDiffNodeP diff = endPoint == ECRelationshipEnd_Source ? parent.Add (DiffNodeId::Source) : parent.Add (DiffNodeId::Target);

    if (left.GetCardinality().ToString() != right.GetCardinality().ToString())
        diff->Add (DiffNodeId::Cardinality)->SetValue (left.GetCardinality().ToString().c_str(), right.GetCardinality().ToString().c_str());

    if (left.GetIsPolymorphic() != right.GetIsPolymorphic())
        diff->Add (DiffNodeId::IsPolymorphic)->SetValue (left.GetIsPolymorphic(), right.GetIsPolymorphic());

    if (left.GetRoleLabel() != right.GetRoleLabel())
        diff->Add (DiffNodeId::RoleLabel)->SetValue (left.GetRoleLabel().c_str(), right.GetRoleLabel().c_str());

    DiffCustomAttributes (*diff, left, right);
    bvector<ECRelationshipConstraintCP> constraints;
    constraints.push_back (&left);
    constraints.push_back (&right);
    DiffRelationshipConstraintClasses(*diff, constraints, endPoint);
    return parent.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffRelationshipConstraintClasses (ECDiffNodeR parentDiff, bvector<ECRelationshipConstraintCP> const& constraints, ECRelationshipEnd endPoint)
    {
    BeAssert (constraints.size() == 2);
    AlignedClassMap constraintClassMap;
    bvector<WCharCP> constraintClassOrderedList;
    for (int i = 0 ; i < (int)constraints.size(); i++)
        {
        ECRelationshipConstraintCP ecRelationshipConstraint = constraints[i];
        if (ecRelationshipConstraint == NULL)
            continue;

        ECBaseClassesList constraintClassContainer = ecRelationshipConstraint->GetClasses();
        for (ECBaseClassesList::const_iterator constraintClassItor = constraintClassContainer.begin(); constraintClassItor != constraintClassContainer.end(); ++constraintClassItor)
            {
            ECClassCR baseClass = *(*constraintClassItor);
            if (constraintClassMap.find (baseClass.GetFullName())== constraintClassMap.end())
                constraintClassOrderedList.push_back (baseClass.GetFullName());
            AlignedClasses& baseClasses = constraintClassMap [baseClass.GetFullName()];
            size_t nNullEntriesRequired = i - baseClasses.size();
            if (nNullEntriesRequired > 0)
                do
                    {
                    baseClasses.push_back (NULL);
                    } while (--nNullEntriesRequired);
                baseClasses.push_back (&baseClass);
            }
        }
    for (bvector<WCharCP>::iterator itor = constraintClassOrderedList.begin(); itor != constraintClassOrderedList.end(); ++itor)
        if (constraintClassMap[*itor].size() < constraints.size())
            while (constraintClassMap[*itor].size() < constraints.size())
                constraintClassMap[*itor].push_back(NULL);
    if (constraintClassOrderedList.empty())
        return NULL;

    ECDiffNodeP constraintClasses = parentDiff.Add (DiffNodeId::Classes);
    int index =0;
    for (bvector<WCharCP>::iterator itor = constraintClassOrderedList.begin(); itor != constraintClassOrderedList.end(); ++itor, index++)
        {
        ECClassCP left = constraintClassMap[*itor][0];
        ECClassCP right = constraintClassMap[*itor][1];
        if (left && !right)
            constraintClasses->AddWithIndex (index, DiffNodeId::ConstraintClasses)->ImplGetValueLeft().SetValue(left->GetFullName());
        else if (!left && right)
            constraintClasses->AddWithIndex (index, DiffNodeId::ConstraintClass)->ImplGetValueRight().SetValue(right->GetFullName());
        else
            if (wcscmp (left->GetFullName(), right->GetFullName()) != 0)
                constraintClasses->AddWithIndex (index, DiffNodeId::ConstraintClass)->SetValue(left->GetFullName(), right->GetFullName());
        }
    return parentDiff.RemoveIfEmpty (constraintClasses);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendRelationship(ECDiffNodeR parent, ECRelationshipClassCR relationship, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (DiffNodeId::RelationshipInfo);
    diff->Add (DiffNodeId::Strength)->GetValue(direction).SetValue (ToString(relationship.GetStrength()));
    diff->Add (DiffNodeId::StrengthDirection)->GetValue(direction).SetValue (ToString(relationship.GetStrengthDirection()));
    AppendRelationshipConstraint (*diff, relationship.GetSource(), ECRelationshipEnd_Source, direction);
    AppendRelationshipConstraint (*diff, relationship.GetTarget(), ECRelationshipEnd_Target, direction);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  relationshipConstraint,ECRelationshipEnd endPoint, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = endPoint == ECRelationshipEnd_Source ? parent.Add (DiffNodeId::Source) : parent.Add (DiffNodeId::Target);
    diff->Add (DiffNodeId::Cardinality)->GetValue(direction).SetValue (relationshipConstraint.GetCardinality().ToString());
    diff->Add (DiffNodeId::IsPolymorphic)->GetValue(direction).SetValue (relationshipConstraint.GetIsPolymorphic());
    diff->Add (DiffNodeId::RoleLabel)->GetValue(direction).SetValue (relationshipConstraint.GetRoleLabel());
    AppendCustomAttributes (*diff, relationshipConstraint, direction);

    if (!relationshipConstraint.GetClasses().empty())
        {
        ECDiffNodeP constraintClasses = diff->Add (DiffNodeId::ConstraintClasses);
        ECConstraintClassesList classes = relationshipConstraint.GetClasses();
        int index = 0;
        for(ECConstraintClassesList::const_iterator itor = classes.begin(); itor != classes.end(); ++itor, index++)
            constraintClasses->AddWithIndex (index, DiffNodeId::ConstraintClass)->GetValue(direction).SetValue ((*itor)->GetFullName());
        }
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECSchemaDiffTool::ToString(ECRelatedInstanceDirection direction)
    {
    switch(direction)
        {
        case ECRelatedInstanceDirection::Backward:
            return ID_STRENGTH_DIRECTION_BACKWARD;
        case ECRelatedInstanceDirection::Forward:
            return ID_STRENGTH_DIRECTION_FORWARD;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECSchemaDiffTool::ToString(StrengthType type)
    {
    switch(type)
        {
        case STRENGTHTYPE_Embedding:
            return ID_STRENGTH_TYPE_EMBEDDING;
        case STRENGTHTYPE_Holding:
            return ID_STRENGTH_TYPE_HOLDING;
        case STRENGTHTYPE_Referencing:
            return ID_STRENGTH_TYPE_REFERENCING;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffArrayBounds (ECDiffNodeR parent , ECPropertyCR left, ECPropertyCR right)
    {
    if (left.GetIsArray() && !right.GetIsArray())
        return AppendArrayBounds(parent, right, ECDiffNode::DIRECTION_Left);
    else if (!left.GetIsArray() && right.GetIsArray())
        return AppendArrayBounds(parent, right, ECDiffNode::DIRECTION_Right);
    else if (left.GetIsArray() && right.GetIsArray())
        {
        ECDiffNodeP arrayBounds = parent.Add (DiffNodeId::ArrayInfo);
        if (left.GetAsArrayProperty()->GetMaxOccurs() != right.GetAsArrayProperty()->GetMaxOccurs())
            arrayBounds->Add(DiffNodeId::MaxOccurs)->SetValue (left.GetAsArrayProperty()->GetMaxOccurs(), right.GetAsArrayProperty()->GetMaxOccurs());
        if (left.GetAsArrayProperty()->GetMinOccurs() != right.GetAsArrayProperty()->GetMinOccurs())
            arrayBounds->Add(DiffNodeId::MinOccurs)->SetValue (left.GetAsArrayProperty()->GetMinOccurs(), right.GetAsArrayProperty()->GetMinOccurs());
        if (arrayBounds->IsEmpty())
            parent.Remove(arrayBounds);
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendClass (ECDiffNodeR parent , ECClassCR ecClass, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (ecClass.GetName().c_str(), DiffNodeId::Class);
    //diff->Add (ID_NAME)->GetValue(direction).SetValue (ecClass.GetName().c_str());
    if (ecClass.GetIsDisplayLabelDefined())
        diff->Add (DiffNodeId::DisplayLabel)->GetValue(direction).SetValue (ecClass.GetDisplayLabel().c_str());
    diff->Add (DiffNodeId::Description)->GetValue(direction).SetValue (ecClass.GetDescription().c_str());
    diff->Add (DiffNodeId::IsCustomAttributeClass)->GetValue(direction).SetValue (ecClass.GetIsCustomAttributeClass());
    diff->Add (DiffNodeId::IsStruct)->GetValue(direction).SetValue (ecClass.GetIsStruct());
    diff->Add (DiffNodeId::IsDomainClass)->GetValue(direction).SetValue (ecClass.GetIsDomainClass());
    diff->Add (DiffNodeId::IsRelationshipClass)->GetValue(direction).SetValue (ecClass.GetRelationshipClassCP() != NULL);
    AppendCustomAttributes (*diff, ecClass, direction);
    if (!ecClass.GetBaseClasses().empty())
        {
        ECDiffNodeP baseClassesNode = diff->Add (DiffNodeId::BaseClasses);
        ECBaseClassesList baseClasses = ecClass.GetBaseClasses();
        int index = 0;
        for(ECBaseClassesList::const_iterator itor = baseClasses.begin(); itor != baseClasses.end(); ++itor, index++)
            baseClassesNode->AddWithIndex (index, DiffNodeId::BaseClass)->GetValue(direction).SetValue ((*itor)->GetFullName());
        }

    ECDiffNodeP propertiesNode = diff->Add (DiffNodeId::Properties);
    ECPropertyIterable properties = ecClass.GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = properties.begin(); itor != properties.end(); ++itor)
        AppendProperty (*propertiesNode, **itor, direction);
    diff->RemoveIfEmpty (propertiesNode);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendProperty (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (ecProperty.GetName().c_str(), DiffNodeId::Property);
    //diff->Add (ID_NAME)->GetValue(direction).SetValue (ecProperty.GetName().c_str());
    if (ecProperty.GetIsDisplayLabelDefined())
        diff->Add (DiffNodeId::DisplayLabel)->GetValue(direction).SetValue (ecProperty.GetDisplayLabel().c_str());

    diff->Add (DiffNodeId::Description)->GetValue(direction).SetValue (ecProperty.GetDescription().c_str());
    diff->Add (DiffNodeId::IsArray)->GetValue(direction).SetValue (ecProperty.GetIsArray());
    diff->Add (DiffNodeId::IsStruct)->GetValue(direction).SetValue (ecProperty.GetIsStruct());
    diff->Add (DiffNodeId::TypeName)->GetValue(direction).SetValue (ecProperty.GetTypeName().c_str());
    diff->Add (DiffNodeId::IsOverriden)->GetValue(direction).SetValue (ecProperty.GetBaseProperty()!= NULL);
    diff->Add (DiffNodeId::IsPrimitive)->GetValue(direction).SetValue (ecProperty.GetIsPrimitive());

    AppendArrayBounds(*diff, ecProperty, direction);
    AppendCustomAttributes (*diff, ecProperty, direction);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendArrayBounds (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction)
    {
    if (ecProperty.GetIsArray())
        {
        ECDiffNodeP arrayBounds = parent.Add (DiffNodeId::ArrayInfo);
        arrayBounds->Add (DiffNodeId::MaxOccurs)->GetValue(direction).SetValue ((Int32)ecProperty.GetAsArrayProperty()->GetMaxOccurs());
        arrayBounds->Add (DiffNodeId::MinOccurs)->GetValue(direction).SetValue ((Int32)ecProperty.GetAsArrayProperty()->GetMinOccurs());
        return arrayBounds;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaDiffTool::CollectInstanceValues (bmap<WString,ECValue>& valueMap, set<WString>& accessStrings, IECInstanceCR instance)
    {
    valueMap.clear();
    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create (instance);
    if (propertyValues.IsNull()) 
        return;
    CollectInstanceValues (valueMap, accessStrings, *propertyValues);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaDiffTool::CollectInstanceValues (bmap<WString,ECValue>& valueMap, set<WString>& accessStrings, ECValuesCollectionCR values)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor ();
        WString accessString = valueAccessor.GetManagedAccessString();
        if ((*itor).HasChildValues())
            CollectInstanceValues (valueMap, accessStrings, *(*itor).GetChildValues());
        else
            {
            ECValueCR v = (*itor).GetValue();
            if (!v.IsPrimitive())
                continue;
            if (v.IsBinary() && !v.IsNull())
                {
                size_t size = 0;
                valueMap [accessString.c_str()].SetBinary (v.GetBinary(size), size, true);
                }
            else
                valueMap [accessString.c_str()].From (v);
            if (accessStrings.find (accessString) == accessStrings.end())
                accessStrings.insert(accessString);
            }
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffCustomAttributes 
    (
    ECDiffNodeR parentDiff, 
    IECCustomAttributeContainerCR leftContainer, 
    IECCustomAttributeContainerCR rightContainer
    )
    {
    bmap<WCharCP, ECClassCP, DiffNameComparer> classes;
    ECCustomAttributeInstanceIterable leftCustomAttributes =  leftContainer.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = leftCustomAttributes.begin(); itor != leftCustomAttributes.end(); ++itor)
        {
        ECClassCR caClass = (*itor)->GetClass();
        if (classes.find ((*itor)->GetClass().GetName().c_str()) == classes.end())
            classes[caClass.GetName().c_str()] = &caClass;
        }
    ECCustomAttributeInstanceIterable rightCustomAttributes =  rightContainer.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = rightCustomAttributes.begin(); itor != rightCustomAttributes.end(); ++itor)
        {
        ECClassCR caClass = (*itor)->GetClass();
        if (classes.find ((*itor)->GetClass().GetName().c_str()) == classes.end())
            classes[caClass.GetName().c_str()] = &caClass;
        }
    if (classes.empty())
        return NULL;

    ECDiffNodeP diff = parentDiff.Add (DiffNodeId::CustomAttributes);
    for (bmap<WCharCP, ECClassCP>::const_iterator itor = classes.begin(); itor != classes.end(); ++itor)
        DiffInstance(*diff, *(itor->second), leftContainer, rightContainer);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::DiffInstance 
    (
    ECDiffNodeR parentDiff, 
    ECClassCR customAttributeClass, 
    IECCustomAttributeContainerCR leftContainer, 
    IECCustomAttributeContainerCR rightContainer
    )
    {
    IECInstanceCP left = leftContainer.GetPrimaryCustomAttribute (customAttributeClass).get();
    IECInstanceCP right = rightContainer.GetPrimaryCustomAttribute (customAttributeClass).get();
    if (!left && !right)
        return NULL;
    else if (left && !right)
        return AppendInstance (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    else if (!left && right)
        return AppendInstance (parentDiff, *right, ECDiffNode::DIRECTION_Right);

    bmap<WString,ECValue> leftValues;
    bmap<WString,ECValue> rightValues;
    set<WString> accessStrings;
    CollectInstanceValues (leftValues, accessStrings, *left);
    CollectInstanceValues (rightValues, accessStrings, *right);

    if (!accessStrings.empty())
        {
        ECDiffNodeP diff = parentDiff.Add (customAttributeClass.GetFullName(), DiffNodeId::CustomAttribute);
        for (set<WString> ::const_iterator itor = accessStrings.begin(); itor != accessStrings.end(); ++itor)
            {
            bmap<WString,ECValue>::const_iterator lItor = leftValues.find (*itor);
            bmap<WString,ECValue>::const_iterator rItor = rightValues.find (*itor);
            if (lItor == leftValues.end() && rItor != rightValues.end())
                SetECValue (*(diff->GetChild ((*itor), true)), rItor->second, ECDiffNode::DIRECTION_Right);
            else if (lItor != leftValues.end() && rItor == rightValues.end())
                SetECValue (*(diff->GetChild ((*itor), true)), lItor->second, ECDiffNode::DIRECTION_Left);
            else
                {
                ECValueCR a = rItor->second;
                ECValueCR b = lItor->second;

                if (!a.Equals (b))
                    {
                    ECDiffNodeP value = diff->GetChild ((*itor), true);
                    SetECValue (*value, rItor->second, ECDiffNode::DIRECTION_Right);
                    SetECValue (*value, lItor->second, ECDiffNode::DIRECTION_Left);
                    }
                }
            }
        return parentDiff.RemoveIfEmpty (diff);
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP  ECSchemaDiffTool::AppendCustomAttributes 
    (
    ECDiffNodeR parentDiff, 
    IECCustomAttributeContainerCR container,
    ECDiffNode::ValueDirection direction
    )
    {
    ECDiffNodeP diff = parentDiff.Add (DiffNodeId::CustomAttributes);
    ECCustomAttributeInstanceIterable customAttributes =  container.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = customAttributes.begin(); itor != customAttributes.end(); ++itor)
        AppendInstance (*diff, *(*itor), direction);

    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendInstance (ECDiffNodeR parentDiff, IECInstanceCR instance, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parentDiff.Add (instance.GetClass().GetFullName(), DiffNodeId::CustomAttribute);
    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create (instance);
    if (propertyValues.IsNull()) 
        return NULL;
    return AppendPropertyValues (*diff, *propertyValues, direction);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiffTool::AppendPropertyValues (ECDiffNodeR parentDiff, ECValuesCollectionCR values, ECDiffNode::ValueDirection direction)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor ();
        const WString propertyName = valueAccessor.GetPropertyName ();
        ECDiffNodeP diff = parentDiff.Add (propertyName.c_str(), DiffNodeId::Property);
        if ((*itor).HasChildValues())
            AppendPropertyValues (*diff, *(*itor).GetChildValues(), direction);
        else
            {
            ECValueCR v = (*itor).GetValue();
            if (v.IsNull())
                continue;
            if (v.IsPrimitive())
                SetECValue(*diff, v, direction);
            }
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaDiffTool::SetECValue (ECDiffNodeR n, ECValueCR v, ECDiffNode::ValueDirection direction)
    {
    if (v.IsNull())
        {
        n.GetValue(direction).SetNil();
        return true;
        }
    if (v.IsArray())
        {
        BeAssert(false);
        return false;
        }
    switch(v.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t size = 0;
            n.GetValue(direction).SetValue (v.GetBinary(size), size);
            break;
            }
        case PRIMITIVETYPE_Boolean:
            n.GetValue (direction).SetValue (v.GetBoolean()); break;
        case PRIMITIVETYPE_DateTime:
            n.GetValue (direction).SetDateTimeValue(v.GetDateTimeTicks()); break;
        case PRIMITIVETYPE_Double:
            n.GetValue (direction).SetValue (v.GetDouble()); break;
        case PRIMITIVETYPE_IGeometry: break;
        case PRIMITIVETYPE_Integer:
            n.GetValue (direction).SetValue (v.GetInteger()); break;
        case PRIMITIVETYPE_Long:
            n.GetValue (direction).SetValue (v.GetLong()); break;
        case PRIMITIVETYPE_Point2D:
            {              
            n.Add (L"x", DiffNodeId::None)->GetValue (direction).SetValue (v.GetPoint2D().x); 
            n.Add (L"y", DiffNodeId::None)->GetValue (direction).SetValue (v.GetPoint2D().y); 
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            n.Add (L"x", DiffNodeId::None)->GetValue (direction).SetValue (v.GetPoint3D().x); 
            n.Add (L"y", DiffNodeId::None)->GetValue (direction).SetValue (v.GetPoint3D().y); 
            n.Add (L"z", DiffNodeId::None)->GetValue (direction).SetValue (v.GetPoint3D().z); 
            break;
            }
        case PRIMITIVETYPE_String:
            n.GetValue (direction).SetValue (v.GetString()); break;
        }
    return true;
    }

//////////////////////////////////////ECSchemaMergeTool////////////////////////////////////////////


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECSchemaMergeTool::ParseStrengthDirection(ECRelatedInstanceDirection& direction, WStringCR directionStr)
    {
    if (directionStr.CompareToI(ID_STRENGTH_DIRECTION_BACKWARD) == 0)
        {
        direction = ECRelatedInstanceDirection::Backward;
        return true;
        }

    if (directionStr.CompareToI(ID_STRENGTH_DIRECTION_FORWARD) == 0)
        {
        direction = ECRelatedInstanceDirection::Forward;
        return true;
        }
    BeAssert (false && "Unknown strength direction value");
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::ParseStrength(StrengthType& type, WStringCR strengthStr)
    {
    if (strengthStr.CompareToI(ID_STRENGTH_TYPE_EMBEDDING) == 0)
        {
        type = STRENGTHTYPE_Embedding;
        return true;
        }
    if (strengthStr.CompareToI(ID_STRENGTH_TYPE_HOLDING) == 0)
        {
        type = STRENGTHTYPE_Holding;
        return true;
        }
    if (strengthStr.CompareToI(ID_STRENGTH_TYPE_REFERENCING) == 0)
        {
        type = STRENGTHTYPE_Referencing;
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECSchemaMergeTool::GetLeft() const { return m_diff.GetLeftSchema(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECSchemaMergeTool::GetRight() const { return m_diff.GetRightSchema(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaR ECSchemaMergeTool::GetMerged() { return *m_mergeSchema; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECSchemaMergeTool::GetDefault() const { return m_defaultConflictRule == CONFLICTRULE_TakeLeft ?  GetLeft(): GetRight(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaMergeTool::EnsureSchemaIsReferenced (ECClassCR referenceClass)
    {
    EnsureSchemaIsReferenced (referenceClass.GetSchema());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaMergeTool::EnsureSchemaIsReferenced (ECSchemaCR reference)
    {
    if (&GetMerged() == &reference)
        return;
    if (!ECSchema::IsSchemaReferenced (GetMerged(), reference))
        GetMerged().AddReferencedSchema (const_cast<ECSchemaR>(reference));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValueP ECSchemaMergeTool::GetMergeValue (ECDiffNodeR n, WCharCP id)
    {
    ECDiffNodeP v;
    if ((v = n.GetChild (id)) != NULL)
        return GetMergeValue(*v);
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValueP ECSchemaMergeTool::GetMergeValue (ECDiffNodeR v)
    {
    DiffType type = v.ImplGetDiffType (false) ;
    if (type == DIFFTYPE_Conflict)
        {
        if (m_defaultConflictRule ==  CONFLICTRULE_TakeLeft)
            return &v.ImplGetValueLeft();
        else
            return &v.ImplGetValueRight();
        }
    else if ( type == DIFFTYPE_Left)
        return &v.ImplGetValueLeft();
    else
        return &v.ImplGetValueRight();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValueP ECSchemaMergeTool::GetMergeValue (ECDiffNodeR n, DiffNodeId id)
    {
    WCharCP name = ECDiffNode::IdToString (id);
    BeAssert(name != NULL);
    return GetMergeValue (n, name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaMergeTool::BuildClassMap (ECSchemaCR schema)
    {
    ECClassContainerCR classes = schema.GetClasses();
    for (ECClassContainer::const_iterator itor = classes.begin(); itor != classes.end(); ++itor)
        if (m_classByNameMap.find((*itor)->GetFullName()) == m_classByNameMap.end())
            m_classByNameMap[(*itor)->GetFullName()] = *itor;
    ECSchemaReferenceListCR references = schema.GetReferencedSchemas();
    for(ECSchemaReferenceList::const_iterator itor = references.begin(); itor != references.end(); ++itor)
        BuildClassMap(*(itor->second));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::IsPartOfMergeSchema(ECClassCR ecClass) const
    {
    return &ecClass.GetSchema() == &GetLeft() || &ecClass.GetSchema() == &GetRight();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaMergeTool::ComputeMergeActions(ClassMergeInfoMap& actions, ECDiffNodeP diffClasses, ECSchemaCR schema)
    {
    ECDiffNodeP diffClass = NULL;
    ECClassContainerCR classList = schema.GetClasses();
    for(ECClassContainer::const_iterator itor =  classList.begin(); itor !=  classList.end(); ++itor)
        {
        WStringCR className = (*itor)->GetName();
        if (actions.find (className.c_str()) != actions.end())
            continue;
        if (diffClasses && ((diffClass = diffClasses->GetChild (className)) != NULL))
            actions[className.c_str()] = ClassMergeInfo (diffClass->ImplGetDiffType (true), diffClass);
        else
            actions[className.c_str()] = ClassMergeInfo (DIFFTYPE_Equal, NULL);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaMergeTool::ComputeMergeActions(PropertyMergeInfoMap& actions, ECDiffNodeP diffProperties, ECClassCR ecClass)
    {
    ECDiffNodeP diffProperty = NULL;
    ECPropertyIterable propertyList =ecClass.GetProperties(false);
    for(ECPropertyIterable::const_iterator itor =  propertyList.begin(); itor !=  propertyList.end(); ++itor)
        {
        WStringCR propertyName = (*itor)->GetName();
        if (actions.find (propertyName.c_str()) != actions.end())
            continue;
        if (diffProperties && ((diffProperty = diffProperties->GetChild (propertyName)) != NULL))
            actions[propertyName.c_str()] = diffProperty->ImplGetDiffType (true);
        else
            actions[propertyName.c_str()] = DIFFTYPE_Equal;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaMergeTool::ECSchemaMergeTool(ECDiffR diff, ConflictRule defaultRule)
    :  m_diff(diff), m_defaultConflictRule(defaultRule)
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::Merge(ECSchemaPtr& mergedSchema, ECDiffR diff, ConflictRule defaultRule)
    {
    ECSchemaMergeTool tool (diff, defaultRule);
    return tool.MergeSchema(mergedSchema);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeSchema (ECSchemaPtr& mergedSchema)
    { 
    if (m_defaultConflictRule == CONFLICTRULE_TakeLeft)
        {
        BuildClassMap (GetLeft());
        BuildClassMap (GetRight());
        }
    else
        {
        BuildClassMap (GetRight());
        BuildClassMap (GetLeft());
        }
    WString schemaName;
    UInt32 versionMajor;
    UInt32 versionMinor;

    ECDiffValueP v;
    ECDiffNodeR r = *m_diff.GetRoot();
    if ((v = GetMergeValue (r, DiffNodeId::Name)) != NULL)
        schemaName = v->GetValueString();
    else
        schemaName = GetDefault().GetName();

    if ((v = GetMergeValue (r, DiffNodeId::VersionMajor)) != NULL)
        versionMajor = (UInt32)v->GetValueInt32();
    else
        versionMajor = GetDefault().GetVersionMajor();;

    if ((v = GetMergeValue (r, DiffNodeId::VersionMinor)) != NULL)
        versionMinor = (UInt32)v->GetValueInt32();
    else
        versionMinor = GetDefault().GetVersionMinor();
    //Create Merge schema 
    if (ECSchema::CreateSchema (m_mergeSchema, schemaName, versionMajor, versionMinor) != ECOBJECTS_STATUS_Success)
        return MERGESTATUS_ErrorCreatingMergeSchema;

    if ((v = GetMergeValue (r, DiffNodeId::DisplayLabel)) == NULL)
        {
        if (GetDefault().GetIsDisplayLabelDefined())
            GetMerged().SetDisplayLabel(GetDefault().GetDescription());
        }
    else
        GetMerged().SetDisplayLabel (v->GetValueString());

    if ((v = GetMergeValue (r, DiffNodeId::Description)) == NULL)
        GetMerged().SetDescription(GetDefault().GetDescription());
    else
        GetMerged().SetDescription (v->GetValueString());

    if ((v = GetMergeValue (r, DiffNodeId::NamespacePrefix)) == NULL)
        GetMerged().SetNamespacePrefix (GetDefault().GetNamespacePrefix());
    else
        GetMerged().SetDescription (v->GetValueString());

    ECDiffNodeP diffClasses = r.ImplGetChildById (DiffNodeId::Classes); 
    ComputeMergeActions (m_classMergeTasks, diffClasses, GetLeft());
    ComputeMergeActions (m_classMergeTasks, diffClasses, GetRight());
    ECClassCP mergedClass;
    for (ClassMergeInfoMap::iterator itor = m_classMergeTasks.begin(); itor != m_classMergeTasks.end(); ++itor)
        {
        if (itor->second.GetClass() != NULL)
            continue;
        MergeStatus classMergeStatus = ResolveClassFromMergeContext (mergedClass, itor->first);
        if (classMergeStatus != MERGESTATUS_Success)
            return classMergeStatus;
        }

    ECDiffNodeP customAttributes = r.ImplGetChildById (DiffNodeId::CustomAttributes);
    if (customAttributes == NULL)
        {
        MergeStatus status = AppendCustomAttributesToMerge(GetMerged(), GetDefault());
        if (status != MERGESTATUS_Success)
            return status;
        }
    else
        {
        MergeStatus status = MergeCustomAttributes (*customAttributes, GetMerged(), &GetDefault());
        if (status != MERGESTATUS_Success)
            return status; 
        }

    mergedSchema = m_mergeSchema;
    return MERGESTATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::ResolveClassFromMergeContext (ECClassCP& mergedClass, WCharCP className)
    {
    ClassMergeInfoMap::iterator itor= m_classMergeTasks.find (className);
    if (itor  == m_classMergeTasks.end())
        {
        BeAssert(false);
        return MERGESTATUS_ErrorClassNotFound;
        }
    ClassMergeInfo& info = itor->second;
    if (info.GetClass() != NULL)
        {
        mergedClass = info.GetClass();
        return MERGESTATUS_Success;
        }

    MergeStatus classMergeStatus = MERGESTATUS_Failed;
    switch (info.GetType())
        {
    case DIFFTYPE_Conflict:
        {
        BeAssert (info.GetNode() != NULL);  
        classMergeStatus = MergeClass(*info.GetNode(), GetDefault().GetClassCP (className), info);
        break;
        }
    case DIFFTYPE_Equal:
        //In case two classes are same they does not appear in diff so we copy it from default schema.
        classMergeStatus = AppendClassToMerge (GetDefault().GetClassCP (className), info); break;
    case DIFFTYPE_Left: 
        //Following is faster then MergeClass()
        classMergeStatus = AppendClassToMerge (GetLeft().GetClassCP (className), info); break;
    case DIFFTYPE_Right:
        //Following is faster then MergeClass()
        classMergeStatus = AppendClassToMerge (GetRight().GetClassCP (className), info); break;
    case DIFFTYPE_Empty:
        {
        BeAssert(false && "type == DIFFTYPE_Empty. Diff is invalid");
        break;
        }

        }
    if (classMergeStatus != MERGESTATUS_Success)
        return classMergeStatus;

    mergedClass = GetMerged().GetClassP (className);
    BeAssert (mergedClass != NULL);
    info.SetClass (*mergedClass);
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeClass (ECDiffNodeR diff, ECClassCP defaultClass, ClassMergeInfo& info)
    {
    BeAssert (!diff.IsEmpty());
    BeAssert (defaultClass != NULL);
    MergeStatus status = MERGESTATUS_Success;
    if(defaultClass == NULL)
        return MERGESTATUS_ErrorClassNotFound;
    bool isRelationshipClass = defaultClass->GetRelationshipClassCP() != NULL;
    ECDiffValueP v = NULL;
    if ((v = GetMergeValue (diff, DiffNodeId::IsRelationshipClass)) != NULL)
        isRelationshipClass = v->GetValueBool();
    ECClassP mergeClass;
    if(isRelationshipClass)
        {
        ECRelationshipClassP newClass;
        if (GetMerged().CreateRelationshipClass(newClass, defaultClass->GetName()) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_ErrorMergeClassAlreadyExist;
        ECDiffNodeP relationshipInfo = diff.ImplGetChildById (DiffNodeId::RelationshipInfo);
        if (relationshipInfo)
            {
            if ((status = MergeRelationship(relationshipInfo, *newClass, *defaultClass)) != MERGESTATUS_Success)
                return status;
            }
        else
            if (defaultClass->GetRelationshipClassCP() != NULL)
                if ((status = AppendRelationshipClassToMerge (*newClass, *defaultClass->GetRelationshipClassCP())) != MERGESTATUS_Success)
                    return status;
        mergeClass = newClass;
        }
    else
        if (GetMerged().CreateClass( mergeClass, defaultClass->GetName()) !=  ECOBJECTS_STATUS_Success)
            return MERGESTATUS_ErrorMergeClassAlreadyExist;

    //Make sure it doest process again
    info.SetClass (*mergeClass);

    if ((v = GetMergeValue (diff, DiffNodeId::DisplayLabel)) == NULL)
        {
        if (mergeClass->GetIsDisplayLabelDefined())
            mergeClass->SetDisplayLabel(defaultClass->GetDescription());
        }
    else
        mergeClass->SetDisplayLabel (v->GetValueString());

    if ((v = GetMergeValue (diff, DiffNodeId::Description)) == NULL)
        mergeClass->SetDescription (defaultClass->GetDescription());
    else
        mergeClass->SetDescription (v->GetValueString());

    if ((v = GetMergeValue (diff, DiffNodeId::IsCustomAttributeClass)) == NULL)
        mergeClass->SetIsCustomAttributeClass(defaultClass->GetIsCustomAttributeClass());
    else
        mergeClass->SetIsCustomAttributeClass (v->GetValueBool());

    if ((v = GetMergeValue (diff, DiffNodeId::IsStruct)) == NULL)
        mergeClass->SetIsStruct(defaultClass->GetIsStruct());
    else
        mergeClass->SetIsStruct (v->GetValueBool());

    if ((v = GetMergeValue (diff, DiffNodeId::IsDomainClass)) == NULL)
        mergeClass->SetIsDomainClass(defaultClass->GetIsDomainClass());
    else
        mergeClass->SetIsDomainClass (v->GetValueBool());
    //Process base classes
    ECDiffNodeP baseClasses = diff.ImplGetChildById (DiffNodeId::BaseClasses);
    if (baseClasses)
        {
        if ((status = MergeBaseClasses(*baseClasses, *mergeClass, *defaultClass)) != MERGESTATUS_Success)
            return status;
        }
    else
        if ((status = AppendBaseClassesToMerge(*mergeClass, *defaultClass)) != MERGESTATUS_Success)
            return status;

    //process properties
    ECDiffNodeP properties = diff.ImplGetChildById (DiffNodeId::Properties);
    if (properties)
        {
        if ((status = MergeProperties (*properties, *mergeClass, *defaultClass)) != MERGESTATUS_Success)
            return status;
        }
    else
        if ((status = AppendPropertiesToMerge(*mergeClass, *defaultClass)) != MERGESTATUS_Success)
            return status;

    ECDiffNodeP customAttributes = diff.ImplGetChildById (DiffNodeId::CustomAttributes);
    if (customAttributes == NULL)
        {
        if ((status = AppendCustomAttributesToMerge (GetMerged(), GetDefault()))!= MERGESTATUS_Success)
            return status;
        }
    else
        {
        if ((status = MergeCustomAttributes (*customAttributes, *mergeClass, defaultClass))!= MERGESTATUS_Success)
            return status; 
        }

    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendRelationshipClassToMerge(ECRelationshipClassR mergedClass, ECRelationshipClassCR defaultClass)
    {
    MergeStatus status;
    mergedClass.SetStrength (defaultClass.GetStrength());
    mergedClass.SetStrengthDirection (defaultClass.GetStrengthDirection());

    if ((status = AppendRelationshipContstraintToMerge (mergedClass.GetSource(), defaultClass.GetSource())) != MERGESTATUS_Success)
        return status;

    if ((status = AppendRelationshipContstraintToMerge (mergedClass.GetTarget(), defaultClass.GetTarget())) != MERGESTATUS_Success)
        return status;

    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeRelationship (ECDiffNodeP diff, ECRelationshipClassR mergedClass, ECClassCR defaultClass)
    {
    BeAssert (diff != NULL);
    BeAssert (!diff->IsEmpty());
    ECDiffValueP v  = NULL;
    MergeStatus status;

    ECRelationshipClassCP defaultRelationshipClass = defaultClass.GetRelationshipClassCP();
    if ((v = GetMergeValue (*diff, DiffNodeId::Strength)) != NULL)
        {
        StrengthType strengthType;
        if (!ParseStrength (strengthType, v->GetValueString()))
            return MERGESTATUS_ErrorParsingRelationshipStrengthType;
        mergedClass.SetStrength (strengthType);
        }
    else
        if (defaultRelationshipClass)
            mergedClass.SetStrength (defaultRelationshipClass->GetStrength());


    if ((v = GetMergeValue (*diff, DiffNodeId::StrengthDirection)) == NULL)
        {
        ECRelatedInstanceDirection strengthDirection;
        if (!ParseStrengthDirection (strengthDirection, v->GetValueString()))
            return MERGESTATUS_ErrorParsingRelationshipStrengthDirection;
        mergedClass.SetStrengthDirection (strengthDirection);
        }
    else
        if (defaultRelationshipClass)
            mergedClass.SetStrengthDirection (defaultRelationshipClass->GetStrengthDirection());

    ECDiffNodeP sourceConstraint = diff->ImplGetChildById( DiffNodeId::Source);
    if (sourceConstraint)
        {
        if ((status = MergeRelationshipConstraint (
            *sourceConstraint, mergedClass.GetSource(), 
            (defaultRelationshipClass ? &defaultRelationshipClass->GetSource(): NULL))) != MERGESTATUS_Success)
            return status;
        }
    else
        if (defaultRelationshipClass)
            if ((status = AppendRelationshipContstraintToMerge(mergedClass.GetSource(), defaultRelationshipClass->GetSource())) != MERGESTATUS_Success)
                return status;

    ECDiffNodeP targetConstraint = diff->ImplGetChildById( DiffNodeId::Target);
    if (targetConstraint)
        {
        if ((status = MergeRelationshipConstraint (
            *targetConstraint, mergedClass.GetTarget(), 
            (defaultRelationshipClass ? &defaultRelationshipClass->GetTarget(): NULL))) != MERGESTATUS_Success)
            return status;
        }
    else
        if (defaultRelationshipClass)
            if ((status = AppendRelationshipContstraintToMerge(mergedClass.GetTarget(), defaultRelationshipClass->GetTarget())) != MERGESTATUS_Success)
                return status;

    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeRelationshipConstraint (ECDiffNodeR diff, ECRelationshipConstraintR mergedConstraint, ECRelationshipConstraintCP defaultContraint)
    {
    BeAssert (!diff.IsEmpty());
    MergeStatus status;
    ECDiffValueP v  = NULL;
    if ((v = GetMergeValue (diff, DiffNodeId::Cardinality)) != NULL)
        {
        if (mergedConstraint.SetCardinality (v->GetValueString().c_str()) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_ErrorParsingCardinality;
        }
    else
        if (defaultContraint)
            mergedConstraint.SetCardinality(defaultContraint->GetCardinality());

    if ((v = GetMergeValue (diff, DiffNodeId::RoleLabel)) != NULL)
        mergedConstraint.SetRoleLabel (v->GetValueString()); 
    else
        if (defaultContraint)
            mergedConstraint.SetRoleLabel (defaultContraint->GetRoleLabel());

    if ((v = GetMergeValue (diff, DiffNodeId::IsPolymorphic)) != NULL)
        mergedConstraint.SetIsPolymorphic (v->GetValueBool()); 
    else
        if (defaultContraint)
            mergedConstraint.SetIsPolymorphic (defaultContraint->GetIsPolymorphic());

    set<WString> constraintClasses;
    if (defaultContraint)
        FOR_EACH (ECClassCP constraintClass, defaultContraint->GetClasses())
        constraintClasses.insert(constraintClass->GetFullName());

    ECDiffNodeP constraintClassDiffNode = diff.ImplGetChildById (DiffNodeId::ConstraintClasses);
    if (constraintClassDiffNode)
        for (ECDiffNode::const_iterator itor = constraintClassDiffNode->begin(); itor != constraintClassDiffNode->end(); ++itor)
            if ((v = GetMergeValue (**itor)) != NULL)
                if (constraintClasses.find(v->GetValueString()) == constraintClasses.end())
                    constraintClasses.insert(v->GetValueString());
    for (set<WString>::const_iterator itor = constraintClasses.begin(); itor != constraintClasses.end(); ++itor)
        {
        ECClassCP resolvedConstraintClass = ResolveClass(*itor);
        if (resolvedConstraintClass == NULL)
            return MERGESTATUS_ErrorClassNotFound;
        mergedConstraint.AddClass (*resolvedConstraintClass);
        }

    ECDiffNodeP customAttributesDiffNode = diff.ImplGetChildById (DiffNodeId::CustomAttributes);
    if (customAttributesDiffNode == NULL)
        {
        if (defaultContraint)
            if ((status = AppendCustomAttributesToMerge (mergedConstraint, *defaultContraint)) != MERGESTATUS_Success)
                return status;
        }
    else
        {
        if ((status = MergeCustomAttributes (*customAttributesDiffNode, mergedConstraint, defaultContraint)) != MERGESTATUS_Success)
            return status; 
        }
    return MERGESTATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::TryGetECClass (ECClassCP& ecClass,  ECDiffNodeCR classNode, ECSchemaCR schema)
    {
    ecClass = NULL;
    BeAssert (classNode.GetId() == DiffNodeId::Class);
    if (classNode.GetId() != DiffNodeId::Class)
        return false;
    ecClass = schema.GetClassCP (classNode.GetName().c_str());
    return ecClass != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::TryGetECProperty (ECPropertyCP& ecProperty,  ECDiffNodeCR propertyNode, ECSchemaCR schema, bool includeBaseClass)
    {
    ecProperty = NULL;
    BeAssert (propertyNode.GetId() == DiffNodeId::Property);
    if (propertyNode.GetId() != DiffNodeId::Property)
        return false;
    ECDiffNodeCP properties = propertyNode.GetParent();
    BeAssert(properties != NULL);
    ECDiffNodeCP ecClassNode = properties->GetParent(); 
    BeAssert(ecClassNode != NULL);

    ECClassCP ecClass;
    if (TryGetECClass (ecClass, *ecClassNode, schema))
        ecProperty = ecClass->GetPropertyP(propertyNode.GetName(), includeBaseClass);
    return ecProperty != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::TryGetECRelationshipConstraint (ECRelationshipConstraintCP & relationshipConstraint,  ECDiffNodeCR relationshipConstraintNode, ECSchemaCR schema)
    {
    relationshipConstraint = NULL;
    BeAssert (relationshipConstraintNode.GetId() == DiffNodeId::Source || relationshipConstraintNode.GetId() == DiffNodeId::Target);
    if (relationshipConstraintNode.GetId() == DiffNodeId::Source || relationshipConstraintNode.GetId() == DiffNodeId::Target)
        return false;
    
    ECDiffNodeCP relationshipInfo = relationshipConstraintNode.GetParent();
    BeAssert(relationshipInfo != NULL);
    ECDiffNodeCP ecClassNode = relationshipInfo->GetParent(); 
    BeAssert(ecClassNode != NULL);

    ECClassCP ecClass;
    if (TryGetECClass (ecClass, *ecClassNode, schema))
        {
        ECRelationshipClassCP relationshipClass = ecClass->GetRelationshipClassCP();
        if (relationshipClass)
            {
            if (relationshipInfo->GetId() == DiffNodeId::Source)
                relationshipConstraint = &relationshipClass->GetSource();
            else
                relationshipConstraint = &relationshipClass->GetTarget();
            }
        }
    return relationshipConstraint != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeCustomAttributes (ECDiffNodeR diff, IECCustomAttributeContainerR mergedContainer , IECCustomAttributeContainerCP defaultConstainer)
    {
    BeAssert(!diff.IsEmpty());
    ECSchemaCR otherSchema = &GetDefault() == &GetLeft() ? GetRight() : GetLeft();
    IECCustomAttributeContainerCP otherContainer = NULL;
    ECDiffNodeCP parent = diff.GetParent();
    if (parent->GetId() == DiffNodeId::Root)
        otherContainer = &otherSchema;
    else if (parent->GetId() == DiffNodeId::Class)
        {
        ECClassCP ecClass;
        if (TryGetECClass(ecClass, *parent, otherSchema))
            otherContainer = ecClass;
        }
    else if (parent->GetId() == DiffNodeId::Property)
        {
        ECPropertyCP ecProperty;
        if (TryGetECProperty(ecProperty, *parent, otherSchema, false))
            otherContainer = ecProperty;
        }
    else if (parent->GetId() == DiffNodeId::Source || parent->GetId() == DiffNodeId::Target)
        {
        ECRelationshipConstraintCP ecRelationshipConstraint;
        if (TryGetECRelationshipConstraint(ecRelationshipConstraint, *parent, otherSchema))
            otherContainer = ecRelationshipConstraint;
        }
    else
        {
        BeAssert(false && "Unknown case");
        return MERGESTATUS_Failed;
        }
    //We do not merge actual instance content rather  just select default one in case of conflict.
    bmap<WString, IECInstancePtr> mergedListOfCustomAttributes;
    if (defaultConstainer)
        FOR_EACH (IECInstancePtr const& ca, defaultConstainer->GetPrimaryCustomAttributes(false))
        mergedListOfCustomAttributes [ca->GetClass().GetFullName()] = ca;

    if (otherContainer)
        FOR_EACH (IECInstancePtr const& ca, otherContainer->GetPrimaryCustomAttributes (false))
        if (mergedListOfCustomAttributes.find(ca->GetClass().GetFullName()) == mergedListOfCustomAttributes.end())
            mergedListOfCustomAttributes [ca->GetClass().GetFullName()] = ca;

    for (bmap<WString, IECInstancePtr>::iterator itor = mergedListOfCustomAttributes.begin(); itor != mergedListOfCustomAttributes.end(); ++itor)
        {
        WStringCR className = itor->first;
        IECInstanceR customAttribute = *(itor->second);

        ECClassCP ecClass = ResolveClass (className);
        if (ecClass ==  NULL)
            return MERGESTATUS_ErrorClassNotFound;

        IECInstancePtr mergedInstance = CreateCopyThroughSerialization (customAttribute, *ecClass );
        if (mergedInstance.IsNull())
            return MERGESTATUS_ErrorCreatingCopyOfCustomAttribute;
        mergedContainer.SetCustomAttribute (*mergedInstance);
        }
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeProperties(ECDiffNodeR diff, ECClassR mergedClass, ECClassCR defaultClass)
    {
    PropertyMergeInfoMap propertyActionMap;
    MergeStatus status;
    ECClassCP left = GetLeft().GetClassCP (mergedClass.GetName().c_str());
    ECClassCP right = GetRight().GetClassCP (mergedClass.GetName().c_str());
    BeAssert(left != NULL && right != NULL);
    if (left == NULL || right == NULL)
        return MERGESTATUS_Failed;
    ComputeMergeActions (propertyActionMap, &diff, *left);
    ComputeMergeActions (propertyActionMap, &diff, *right);
    for (PropertyMergeInfoMap::const_iterator itor = propertyActionMap.begin(); itor != propertyActionMap.end(); ++itor )
        {
        WCharCP propertyName = itor->first;
        DiffType diffType= itor->second;

        if (diffType == DIFFTYPE_Left)
            {
            if ((status = AppendPropertyToMerge (mergedClass, left->GetPropertyP (propertyName, false))) != MERGESTATUS_Success)
                return status;
            }
        else if (diffType == DIFFTYPE_Right)
            {
            if ((status = AppendPropertyToMerge (mergedClass, right->GetPropertyP (propertyName, false))) != MERGESTATUS_Success)
                return status;
            }
        else if (diffType == DIFFTYPE_Equal)
            {
            if ((status = AppendPropertyToMerge (mergedClass, defaultClass.GetPropertyP (propertyName, false))) != MERGESTATUS_Success)
                return status;
            }
        else if (diffType == DIFFTYPE_Conflict)
            {
            diffType  = diff.GetChild(propertyName)->ImplGetDiffType(true);
            if ((status = MergeProperty (diff.GetChild(propertyName), mergedClass, defaultClass)) != MERGESTATUS_Success)
                return status;
            }
        else
            {
            BeAssert(false);
            }
        }
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeProperty (ECDiffNodeP diff, ECClassR mergedClass, ECClassCR defaultClass)
    {
    BeAssert (diff != NULL);
    BeAssert (!diff->IsEmpty());
    ECDiffValueP v = NULL;
    MergeStatus status;
    WStringCR propertyName = diff->GetName();
    ECPropertyCP defaultProperty = defaultClass.GetPropertyP (propertyName, false);
    BeAssert (defaultProperty != NULL);
    if (defaultProperty == NULL)
        return MERGESTATUS_Failed;

    bool isArray = defaultProperty->GetIsArray();
    if ((v = GetMergeValue (*diff, DiffNodeId::IsArray)) != NULL)
        isArray = v->GetValueBool();
    bool isStruct = defaultProperty->GetIsStruct();
    if ((v = GetMergeValue (*diff, DiffNodeId::IsStruct)) != NULL)
        isStruct = v->GetValueBool();
    WString typeName = defaultProperty->GetTypeName();
    if ((v = GetMergeValue (*diff, DiffNodeId::TypeName)) != NULL)
        typeName = v->GetValueString();

    if (isArray && !isStruct && typeName ==L"point3d")
        {
        printf("");
        }
    ECPropertyP mergedProperty = NULL;
    if (isStruct)
        {
        StructECPropertyP newProperty;
        ECClassCP structTypeClass = ResolveClass (typeName);
        if (structTypeClass == NULL)
            return MERGESTATUS_Failed;
        if (mergedClass.CreateStructProperty (newProperty, defaultProperty->GetName(), *structTypeClass) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        mergedProperty = newProperty;
        }
    else if (isArray)
        {
        ArrayECPropertyP newProperty;
        if (typeName.find (L":") == WString::npos)
            {
            PrimitiveType primitiveType;
            if (!ParsePrimitiveType (primitiveType, typeName))
                return MERGESTATUS_Failed;
            if (mergedClass.CreateArrayProperty (newProperty, defaultProperty->GetName(), primitiveType) != ECOBJECTS_STATUS_Success)
                return MERGESTATUS_Failed;
            }
        else
            {
            ECClassCP structTypeClass = ResolveClass (typeName);
            if (structTypeClass == NULL)
                return MERGESTATUS_Failed;
            if (mergedClass.CreateArrayProperty (newProperty, defaultProperty->GetName(), structTypeClass) != ECOBJECTS_STATUS_Success)
                return MERGESTATUS_Failed;
            }

        ECDiffNodeP arrayInfo = diff->ImplGetChildById (DiffNodeId::ArrayInfo);
        if ( arrayInfo && (v = GetMergeValue (*arrayInfo, DiffNodeId::MaxOccurs)) != NULL)
            newProperty->SetMaxOccurs (v->GetValueInt32());
        else
            if (defaultProperty->GetIsArray())
                newProperty->SetMaxOccurs (defaultProperty->GetAsArrayProperty()->GetMaxOccurs());

        if (arrayInfo && (v = GetMergeValue (*arrayInfo, DiffNodeId::MinOccurs)) != NULL)
            newProperty->SetMinOccurs (v->GetValueInt32());
        else
            if (defaultProperty->GetIsArray())
                newProperty->SetMinOccurs (defaultProperty->GetAsArrayProperty()->GetMinOccurs());
        mergedProperty = newProperty;
        }
    else 
        {
        PrimitiveECPropertyP newProperty;
        PrimitiveType primitiveType;
        if (!ParsePrimitiveType (primitiveType, typeName))
            return MERGESTATUS_Failed;
        if (mergedClass.CreatePrimitiveProperty (newProperty, defaultProperty->GetName(), primitiveType) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        mergedProperty = newProperty;
        }
    PRECONDITION(mergedProperty != nullptr, MERGESTATUS_Failed);
    if ((v = GetMergeValue (*diff, DiffNodeId::DisplayLabel)) == NULL)
        {
        if (mergedProperty->GetIsDisplayLabelDefined())
            mergedProperty->SetDisplayLabel(defaultProperty->GetDescription());
        }
    else
        mergedProperty->SetDisplayLabel (v->GetValueString());

    if ((v = GetMergeValue (*diff, DiffNodeId::Description)) == NULL)
        mergedProperty->SetDescription (defaultProperty->GetDescription());
    else
        mergedProperty->SetDescription (v->GetValueString());

    if ((v = GetMergeValue (*diff, DiffNodeId::IsReadOnly)) == NULL)
        mergedProperty->SetIsReadOnly (defaultProperty->GetIsReadOnly());
    else
        mergedProperty->SetIsReadOnly (v->GetValueBool());

    ECDiffNodeP customAttributes = diff->ImplGetChildById (DiffNodeId::CustomAttributes);
    if (customAttributes == NULL)
        {
        if ((status = AppendCustomAttributesToMerge(GetMerged(), GetDefault()))!= MERGESTATUS_Success)
            return status;
        }
    else
        {
        if ((status = MergeCustomAttributes (*customAttributes, *mergedProperty, defaultProperty))!= MERGESTATUS_Success)
            return status;
        }

    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaMergeTool::ParsePrimitiveType (PrimitiveType& primitiveType, WStringCR primitiveTypeString)
    {
    if (primitiveTypeString.CompareToI (L"String") == 0)
        primitiveType = PRIMITIVETYPE_String;
    else if (primitiveTypeString.CompareToI (L"Binary") == 0)
        primitiveType = PRIMITIVETYPE_Binary;
    else if (primitiveTypeString.CompareToI (L"Boolean") == 0)
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (primitiveTypeString.CompareToI (L"DateTime") == 0)
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (primitiveTypeString.CompareToI (L"Double") == 0)
        primitiveType = PRIMITIVETYPE_Double;
    else if (primitiveTypeString.CompareToI (L"IGeometry") == 0)
        primitiveType = PRIMITIVETYPE_IGeometry;
    else if (primitiveTypeString.CompareToI (L"Integer") == 0)
        primitiveType = PRIMITIVETYPE_Integer;
    else if (primitiveTypeString.CompareToI (L"Long") == 0)
        primitiveType = PRIMITIVETYPE_Long;
    else if (primitiveTypeString.CompareToI (L"Point2D") == 0)
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (primitiveTypeString.CompareToI (L"Point3D") == 0)
        primitiveType = PRIMITIVETYPE_Point3D;
    else
        return false;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendPropertyToMerge(ECClassR mergeClass,ECPropertyCP property)
    {
    BeAssert(property != NULL);
    ECPropertyP mergeProperty = NULL;
    if (property == NULL)
        return MERGESTATUS_Failed;
    MergeStatus status;
    if (property->GetIsPrimitive())
        {
        PrimitiveECPropertyCP srcProperty = property->GetAsPrimitiveProperty();
        PrimitiveECPropertyP newProperty;
        if (mergeClass.CreatePrimitiveProperty (newProperty, srcProperty->GetName(), srcProperty->GetType()) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        mergeProperty = newProperty;
        }
    else if (property->GetIsStruct())
        {
        StructECPropertyCP srcProperty = property->GetAsStructProperty();
        StructECPropertyP newProperty;
        ECClassCP resolvedStructType = &srcProperty->GetType();
        if (IsPartOfMergeSchema(*resolvedStructType))
            {
            status = ResolveClassFromMergeContext (resolvedStructType, resolvedStructType->GetName().c_str());
            if (status != MERGESTATUS_Success)
                return status;
            }
        else
            EnsureSchemaIsReferenced (*resolvedStructType);
        BeAssert(resolvedStructType!= NULL);
        if (mergeClass.CreateStructProperty (newProperty, srcProperty->GetName(), *resolvedStructType) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        mergeProperty = newProperty;
        }
    else if (property->GetIsArray())
        {
        ArrayECPropertyCP srcProperty = property->GetAsArrayProperty();
        ArrayECPropertyP newProperty;
        if (srcProperty->GetKind() == ARRAYKIND_Struct)
            {
            ECClassCP resolvedStructType = srcProperty->GetStructElementType();
            if (IsPartOfMergeSchema(*resolvedStructType))
                {
                status = ResolveClassFromMergeContext (resolvedStructType, resolvedStructType->GetName().c_str());
                if (status != MERGESTATUS_Success)
                    return status;
                }
            else
                EnsureSchemaIsReferenced (*resolvedStructType);
            BeAssert(resolvedStructType!= NULL);
            if (mergeClass.CreateArrayProperty(newProperty, srcProperty->GetName(), resolvedStructType) != ECOBJECTS_STATUS_Success)
                return MERGESTATUS_Failed;
            }
        else //primitive
            {
            if (mergeClass.CreateArrayProperty (newProperty, srcProperty->GetName(), srcProperty->GetPrimitiveElementType()) != ECOBJECTS_STATUS_Success)
                return MERGESTATUS_Failed;
            }
        newProperty->SetMinOccurs( srcProperty->GetMinOccurs());
        newProperty->SetMaxOccurs( srcProperty->GetMaxOccurs());
        mergeProperty = newProperty;
        }
    if (property->GetIsDisplayLabelDefined())
        mergeProperty->SetDisplayLabel(property->GetDisplayLabel());
    mergeProperty->SetDescription(property->GetDescription());
    mergeProperty->SetIsReadOnly( property->GetIsReadOnly());

    status = AppendCustomAttributesToMerge (*mergeProperty, *property);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendPropertiesToMerge(ECClassR mergeClass, ECClassCR defaultClass)
    {
    MergeStatus status = MERGESTATUS_Success;
    ECPropertyIterable properties = defaultClass.GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = properties.begin(); itor != properties.end() ; ++itor)
        if ( (status = AppendPropertyToMerge (mergeClass, *itor)) != MERGESTATUS_Success)
            return status;
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::MergeBaseClasses (ECDiffNodeR diff,ECClassR mergedClass, ECClassCR defaultClass)
    {
    BeAssert (!diff.IsEmpty());
    ECDiffValueP v = NULL;
    MergeStatus status = MERGESTATUS_Success;
    //Determine the merge list of baseClasses.
    bvector<WString> baseClassList; 
    set<WString> baseClassMap;
    FOR_EACH (ECClassCP baseClass, defaultClass.GetBaseClasses())
        {
        baseClassList.push_back (baseClass->GetFullName());
        baseClassMap.insert(baseClass->GetFullName());
        }
    for(ECDiffNode::const_iterator itor = diff.begin(); itor != diff.end(); ++itor)
        if ((v = GetMergeValue(**itor)) !=  NULL )
            if (baseClassMap.find (v->GetValueString()) != baseClassMap.end())
                {
                baseClassList.push_back (v->GetValueString());
                baseClassMap.insert (v->GetValueString());
                }
            for(bvector<WString>::const_iterator itor = baseClassList.begin(); itor != baseClassList.end(); ++itor)
                {
                ECClassCP baseClass = ResolveClass (*itor);
                if (baseClass == NULL)
                    return MERGESTATUS_ErrorClassNotFound;
                mergedClass.AddBaseClass (*baseClass);
                }
            return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaMergeTool::ResolveClass (WStringCR classFullName)
    {
    BeAssert (!classFullName.empty());
    //First priority is merged schema see if can find it there
    WString::size_type n = classFullName.find (L":");
    BeAssert (n != WString::npos);
    WString className = classFullName.substr (n + 1);
    ECClassCP ecClass = GetMerged().GetClassCP (className.c_str());
    if (ecClass != NULL)
        return ecClass;

    //Second priority is all classes all primary and reference schema
    ClassByNameMap::const_iterator itor = m_classByNameMap.find (classFullName);
    if (itor == m_classByNameMap.end())
        {
        BeAssert(false && "Failed find class");
        return NULL;
        }

    if (!IsPartOfMergeSchema (*(itor->second)))
        EnsureSchemaIsReferenced (*(itor->second));
    else
        {
        if (ResolveClassFromMergeContext (ecClass, className.c_str()) == MERGESTATUS_Success)
            return ecClass;
        }
    return itor->second;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendClassToMerge (ECClassCP ecClass, ClassMergeInfo& info)
    {
    BeAssert(ecClass != NULL);
    ECClassP mergedClass;
    ECRelationshipClassCP relationshipClass = ecClass->GetRelationshipClassCP();
    if (relationshipClass == NULL)
        {
        if (GetMerged().CreateClass (mergedClass, ecClass->GetName()) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        }
    else
        {
        ECRelationshipClassP newClass;
        if (GetMerged().CreateRelationshipClass (newClass, ecClass->GetName()) != ECOBJECTS_STATUS_Success)
            return MERGESTATUS_Failed;
        mergedClass = newClass;
        }
    info.SetClass (*mergedClass);

    if (ecClass->GetIsDisplayLabelDefined())
        mergedClass->SetDisplayLabel (ecClass->GetDisplayLabel());
    mergedClass->SetDescription (ecClass->GetDescription());
    mergedClass->SetIsCustomAttributeClass (ecClass->GetIsCustomAttributeClass());
    mergedClass->SetIsDomainClass (ecClass->GetIsDomainClass());
    mergedClass->SetIsStruct (ecClass->GetIsStruct());

    MergeStatus status = AppendBaseClassesToMerge (*mergedClass, *ecClass);
    if (status != MERGESTATUS_Success)
        return status;
    status = AppendPropertiesToMerge (*mergedClass, *ecClass);
    if (status != MERGESTATUS_Success)
        return status;

    if (relationshipClass != NULL)
        {
        status = AppendRelationshipToMerge (*(mergedClass->GetRelationshipClassP()), *relationshipClass);
        if (status != MERGESTATUS_Success)
            return status;
        }
    status = AppendCustomAttributesToMerge (*mergedClass, *ecClass);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendRelationshipToMerge(ECRelationshipClassR mergedRelationshipClass, ECRelationshipClassCR defaultRelationshipClass)
    {
    MergeStatus status ;
    mergedRelationshipClass.SetStrength (defaultRelationshipClass.GetStrength());
    mergedRelationshipClass.SetStrengthDirection (defaultRelationshipClass.GetStrengthDirection());

    status = AppendRelationshipContstraintToMerge (defaultRelationshipClass.GetSource(), defaultRelationshipClass.GetSource());
    if (status != MERGESTATUS_Success)
        return status;

    status = AppendRelationshipContstraintToMerge (defaultRelationshipClass.GetTarget(), defaultRelationshipClass.GetTarget());
    if (status != MERGESTATUS_Success)
        return status;

    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendRelationshipContstraintToMerge(ECRelationshipConstraintR mergedRelationshipClassConstraint, ECRelationshipConstraintCR defaultRelationshipClassConstraint)
    {
    MergeStatus status;
    mergedRelationshipClassConstraint.SetCardinality (defaultRelationshipClassConstraint.GetCardinality());
    mergedRelationshipClassConstraint.SetIsPolymorphic (defaultRelationshipClassConstraint.GetIsPolymorphic());
    mergedRelationshipClassConstraint.SetRoleLabel (defaultRelationshipClassConstraint.GetRoleLabel());
    status = AppendCustomAttributesToMerge (mergedRelationshipClassConstraint, defaultRelationshipClassConstraint);
    if (status != MERGESTATUS_Success)
        return status;

    FOR_EACH (ECClassCP constraintClass, defaultRelationshipClassConstraint.GetClasses())
        {
        ECClassCP resolvedClass = ResolveClass (constraintClass->GetFullName());
        BeAssert (resolvedClass != NULL);
        if (resolvedClass == NULL)
            return MERGESTATUS_ErrorClassNotFound;
        mergedRelationshipClassConstraint.AddClass(*resolvedClass);
        }
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendCustomAttributesToMerge (IECCustomAttributeContainerR mergeContainer, IECCustomAttributeContainerCR defaultContainer)
    {
    ECCustomAttributeInstanceIterable list = defaultContainer.GetPrimaryCustomAttributes (false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = list.begin(); itor != list.end(); ++itor)
        {
        IECInstancePtr ca = *itor;
        ECClassCP caClass = ResolveClass (ca->GetClass().GetFullName());
        BeAssert (caClass != NULL);
        if (caClass == NULL)
            return MERGESTATUS_ErrorClassNotFound;
        IECInstancePtr copyInstance = CreateCopyThroughSerialization (*ca, *caClass);
        BeAssert (copyInstance.IsValid());
        if (copyInstance.IsNull())
            return MERGESTATUS_ErrorCreatingCopyOfCustomAttribute;
        mergeContainer.SetCustomAttribute (*copyInstance);
        }
    return MERGESTATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECSchemaMergeTool::CreateCopyThroughSerialization (IECInstanceR instance, ECClassCR ecClass)
    {
    //TODO: if class is from diff schema the xml will have different schema name may be if schema being diff have different names
    BeAssert (instance.GetClass().GetName() == ecClass.GetName());
    WString ecInstanceXml;
    instance.WriteToXmlString(ecInstanceXml, true, false);
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (ecClass.GetSchema());
    IECInstancePtr deserializedInstance;
    IECInstance::ReadFromXmlString (deserializedInstance, ecInstanceXml.c_str(), *instanceContext);
    return deserializedInstance;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeStatus ECSchemaMergeTool::AppendBaseClassesToMerge (ECClassR to, ECClassCR from)
    {
    FOR_EACH (ECClassCP baseClass, from.GetBaseClasses())
        {
        ECClassCP resolveBaseClass = ResolveClass (baseClass->GetFullName());
        BeAssert(resolveBaseClass != NULL);
        if (resolveBaseClass == NULL)
            return MERGESTATUS_ErrorClassNotFound;
        to.AddBaseClass (*resolveBaseClass);
        }
    return MERGESTATUS_Success;
    }
	
END_BENTLEY_ECOBJECT_NAMESPACE