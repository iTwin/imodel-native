/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <functional>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ECPresentationErrors.h>
#include <ECPresentation/Rules/PresentationRules.h>

// nodes
ECPRESENTATION_TYPEDEFS(NavNodesFactory)

// misc.
ECPRESENTATION_TYPEDEFS(RulesDrivenProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(RulesDrivenProviderContext)

// nav node data sources
ECPRESENTATION_REFCOUNTED_PTR(NavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(CachingNavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(EmptyNavNodesDataSource)

// nav node providers
ECPRESENTATION_TYPEDEFS(INodesProviderContextFactory)
ECPRESENTATION_TYPEDEFS(INodesProviderFactory)
ECPRESENTATION_TYPEDEFS(NavNodesProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(NavNodesProviderContext)
ECPRESENTATION_TYPEDEFS(NavNodesProvider)
ECPRESENTATION_REFCOUNTED_PTR(NavNodesProvider)
ECPRESENTATION_TYPEDEFS(MultiNavNodesProvider)
ECPRESENTATION_TYPEDEFS(EmptyNavNodesProvider)
ECPRESENTATION_TYPEDEFS(SingleNavNodeProvider)

// content data sources
ECPRESENTATION_TYPEDEFS(ContentSetDataSource)
ECPRESENTATION_REFCOUNTED_PTR(ContentSetDataSource)

// content providers
ECPRESENTATION_TYPEDEFS(ContentProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(ContentProviderContext)
ECPRESENTATION_TYPEDEFS(ContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(ContentProvider)
ECPRESENTATION_TYPEDEFS(SpecificationContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(SpecificationContentProvider)
ECPRESENTATION_TYPEDEFS(NestedContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(NestedContentProvider)
// cache
ECPRESENTATION_TYPEDEFS(IHierarchyCache)
ECPRESENTATION_TYPEDEFS(IVirtualNavNodesCache)
ECPRESENTATION_TYPEDEFS(INavNodesCache)
ECPRESENTATION_TYPEDEFS(IHierarchyLevelLocker)

// queries
ECPRESENTATION_TYPEDEFS(PresentationQueryBuilder);
ECPRESENTATION_TYPEDEFS(ComplexQueryBuilder);
ECPRESENTATION_TYPEDEFS(UnionQueryBuilder);
ECPRESENTATION_TYPEDEFS(ExceptQueryBuilder);
ECPRESENTATION_TYPEDEFS(StringQueryBuilder);
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryBuilder)
ECPRESENTATION_REFCOUNTED_PTR(ComplexQueryBuilder)
ECPRESENTATION_REFCOUNTED_PTR(UnionQueryBuilder)
ECPRESENTATION_REFCOUNTED_PTR(ExceptQueryBuilder)
ECPRESENTATION_REFCOUNTED_PTR(StringQueryBuilder)

// contracts
ECPRESENTATION_TYPEDEFS(PresentationQueryContractField)
ECPRESENTATION_TYPEDEFS(PresentationQueryContract)
ECPRESENTATION_TYPEDEFS(NavigationQueryContract)
ECPRESENTATION_TYPEDEFS(ContentQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryContractField)
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(NavigationQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(ContentQueryContract)

// query builder
ECPRESENTATION_TYPEDEFS(NavigationQueryBuilder)
ECPRESENTATION_TYPEDEFS(ContentQueryBuilder)

// update
ECPRESENTATION_REFCOUNTED_PTR(IUpdateTask)
ECPRESENTATION_TYPEDEFS(IUpdateTask)
ECPRESENTATION_TYPEDEFS(UpdateTasksFactory)

/*=================================================================================**//**
* @bsinamespace
+===============+===============+===============+===============+===============+======*/
namespace std
    {
    template<> struct hash<BeSQLite::EC::ECInstanceId>
        {
        size_t operator()(BeSQLite::EC::ECInstanceId const& value) const { return hash<uint64_t>{}(value.GetValueUnchecked()); }
        };
    template<> struct hash<ECN::ECClassId>
        {
        size_t operator()(ECN::ECClassId const& value) const { return hash<uint64_t>{}(value.GetValueUnchecked()); }
        };
    template<> struct hash<BeSQLite::EC::ECInstanceKey>
        {
        size_t operator()(BeSQLite::EC::ECInstanceKey const& value) const
            {
            return hash<ECN::ECClassId>{}(value.GetClassId()) * 31 + hash<BeSQLite::EC::ECInstanceId>{}(value.GetInstanceId());
            }
        };
    }

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

// https://www.sqlite.org/limits.html#max_compound_select
#define MAX_COMPOUND_STATEMENTS_COUNT       500

/*=================================================================================**//**
* A helper for "params" type of structs that want to store a reference to to something, but
* also avoid issues in cases they're passed an rvalue reference.
*
* This is a problematic case:
* ```
* struct Params {
*   MyObj const& m_arg;
*   Params(MyObj const& arg) : m_arg(arg) {}
* }
* ```
* The above works well when given an lvalue arg, but consider this:
* ```
* static void main() {
*   Params p(MyObj());
*   UseArg(p.m_arg); // `m_arg` returns invalid reference!
* }
* ```
*
* One solution to the above is to ask consumers always pass lvalue references, but that's inconvenient
* and is likely to cause trouble in the future.
*
* Another solution is to make a copy, but that could be expensive.
*
* The goal of `ConstRef` class is to offer the third solution:
* ```
* struct Params {
*   ConstRef<MyObj> m_arg;
*   Params(ConstRef<MyObj> arg) : m_arg(arg) {}
* }
* ```
* The above uses `ConstRef<MyObj>` instead of `MyObj const&`. Both conversions from `T -> ConstRef<T>` and `ConstRef<T> -> T`
* are implicit, so consumers are unaffected for the most part, except for cases when consumer uses some nested types of `T`.
* For that case we have the operator `*`:
* ```
* static void main() {
*   Params p(bvector<MyObj>());
*   for (auto const& item : *p.m_arg) {
*     // use item
*   }
* }
* ```
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename T>
struct ConstRef
{
private:
    T const* m_ptr;
    bool m_owns;
public:
    ConstRef(T const& value) : m_ptr(&value), m_owns(false) {}
    ConstRef(T&& value) : m_ptr(new T(std::move(value))), m_owns(true) {}
    ConstRef(ConstRef<T> const& other) : m_ptr(other.m_owns ? new T(*other.m_ptr) : other.m_ptr), m_owns(other.m_owns) {}
    ConstRef(ConstRef<T>&& other) : m_ptr(other.m_ptr), m_owns(other.m_owns) {other.m_owns = false;}
    ConstRef& operator=(ConstRef<T> const& other)
        {
        m_ptr = other.m_owns ? new T(*other.m_ptr) : other.m_ptr;
        m_owns = other.m_owns;
        return *this;
        }
    ConstRef& operator=(ConstRef<T>&& other)
        {
        m_ptr = other.m_ptr;
        m_owns = other.m_owns;
        other.m_owns = false;
        return *this;
        }
    ~ConstRef()
        {
        if (m_owns)
            DELETE_AND_CLEAR(m_ptr);
        m_owns = false;
        }
    operator T const&() const {return *m_ptr;}
    T const& operator*() const {return *m_ptr;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class NavigationQueryResultType
    {
    Invalid,
    ECInstanceNodes,
    MultiECInstanceNodes,
    DisplayLabelGroupingNodes,
    PropertyGroupingNodes,
    ClassGroupingNodes,
    ECRelationshipClassNodes,
    };

/*=================================================================================**//**
//! note: Order of enum values is important - it defines the order of grouping handlers!
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class GroupingType
    {
    SameLabelInstance = 0,
    DisplayLabel      = 1,
    Property          = 2,
    Class             = 3,
    BaseClass         = 4,
    Relationship      = 5,
    };

typedef bmap<Utf8String, ECValue> NodeArtifacts;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NeverCanceledToken : ICancelationToken
    {
    bool _IsCanceled() const override { return false; }
    static RefCountedPtr<NeverCanceledToken> Create() { return new NeverCanceledToken(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void ThrowIfCancelled(ICancelationTokenCR token)
    {
    if (token.IsCanceled())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_INFO, "Cancelled");
        throw CancellationException();
        }
    }
static inline void ThrowIfCancelled(ICancelationTokenCP token) { if (token) { ThrowIfCancelled(*token); } }

/*=================================================================================**//**
* A `less` compare macro for vectors. Vector items must have a less compare operator.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
#define COMPARE_VEC(lhs, rhs) \
    if (lhs.size() < rhs.size()) \
        return true; \
    if (lhs.size() > rhs.size()) \
        return false; \
    for (auto const& lhsMember : lhs) \
        { \
        for (auto const& rhsMember : rhs) \
            { \
            if (lhsMember < rhsMember) \
                return true; \
            if (rhsMember < lhsMember) \
                return false; \
            } \
        }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RuleApplicationInfo
{
private:
    ECClassCP m_ruleClass;
    bool m_isRulePolymorphic;
public:
    RuleApplicationInfo() : m_ruleClass(nullptr) {}
    RuleApplicationInfo(ECClassCR ruleClass, bool isPolymorphic) : m_ruleClass(&ruleClass), m_isRulePolymorphic(isPolymorphic) {}
    ECClassCP GetRuleClass() const {return m_ruleClass;}
    bool IsRulePolymorphic() const {return m_isRulePolymorphic;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IParsedInput
{
protected:
    virtual bvector<ECClassCP> const& _GetClasses() const = 0;
    virtual bool _HasClass(ECClassCR ecClass) const {return _GetClasses().end() != std::find(_GetClasses().begin(), _GetClasses().end(), &ecClass);}
    virtual bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECClassCR) const = 0;
public:
    virtual ~IParsedInput() {}
    bvector<ECClassCP> const& GetClasses() const { return _GetClasses(); }
    bool HasClass(ECClassCR ecClass) const {return _HasClass(ecClass);}
    bvector<BeSQLite::EC::ECInstanceId> const& GetInstanceIds(ECClassCR selectClass) const { return _GetInstanceIds(selectClass); }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContainerHelpers
{
private:
    ContainerHelpers() {}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType& TransformContainer(TTargetContainerType& target, TSourceContainerType const& source, TFunctor&& f)
        {
        std::transform(source.begin(), source.end(), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType TransformContainer(TSourceContainerType const& source, TFunctor&& f)
        {
        TTargetContainerType target;
        std::transform(source.begin(), source.end(), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType TransformContainer(TSourceContainerType const& source)
        {
        TTargetContainerType target;
        std::copy(source.begin(), source.end(), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType& MoveTransformContainer(TTargetContainerType& target, TSourceContainerType&& source, TFunctor&& f)
        {
        std::transform(std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType MoveTransformContainer(TSourceContainerType&& source, TFunctor&& f)
        {
        TTargetContainerType target;
        std::transform(std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType MoveTransformContainer(TSourceContainerType&& source)
        {
        TTargetContainerType target;
        std::move(std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType& Push(TTargetContainerType& target, TSourceContainerType const& source)
        {
        std::copy(source.begin(), source.end(), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType& MovePush(TTargetContainerType& target, TSourceContainerType&& source)
        {
        std::move(source.begin(), source.end(), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType, typename TFunc>
    static TContainerType& RemoveIf(TContainerType& container, TFunc const& pred)
        {
        container.erase(std::remove_if(container.begin(), container.end(), pred), container.end());
        return container;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TKey, typename TValue>
    static bvector<TKey> GetMapKeys(bmap<TKey, TValue> const& map)
        {
        return TransformContainer<bvector<TKey>>(map, [](bpair<TKey, TValue> const& entry){return entry.first;});
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TKey, typename TValue>
    static bvector<TValue> GetMapValues(bmap<TKey, TValue> const& map)
        {
        return TransformContainer<bvector<TValue>>(map, [](bpair<TKey, TValue> const& entry){return entry.second;});
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TKey, typename TValue>
    static bvector<TValue> SerializeMapListValues(bmap<TKey, bvector<TValue>> const& map)
        {
        bvector<TValue> serialized;
        for (bpair<TKey, bvector<TValue>> const& entry : map)
            Push(serialized, entry.second);
        return serialized;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType, typename TItemType>
    static TContainerType Create(TItemType item)
        {
        TContainerType container;
        container.insert(container.end(), item);
        return container;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TItemType, typename TContainerType>
    static TContainerType FindSubset(TContainerType const& container, std::function<bool(TItemType const)> const& pred)
        {
        TContainerType values = TContainerType();
        for (auto const& item : container)
            {
            if (pred(item))
                values.push_back(item);
            }
        return values;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TItemType, typename TContainerType>
    static TItemType FindFirst(TContainerType const& container, std::function<bool(TItemType const&)> const& pred, TItemType const& defaultValue = TItemType())
        {
        for (auto const& item : container)
            {
            if (pred(item))
                return item;
            }
        return defaultValue;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TItemType, typename TContainerType>
    static int FindFirstIndex(TContainerType const& container, std::function<bool(TItemType const&)> const& pred)
        {
        for (size_t i = 0; i < container.size(); ++i)
            {
            if (pred(container[i]))
                return (int)i;
            }
        return -1;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType>
    static bool Contains(TContainerType const& container, typename TContainerType::value_type const& value)
        {
        return container.end() != std::find(container.begin(), container.end(), value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType>
    static bool Contains(TContainerType const& container, std::function<bool(typename TContainerType::value_type const&)> const& pred)
        {
        return container.end() != std::find_if(container.begin(), container.end(), pred);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType>
    static size_t Count(TContainerType const& container, std::function<bool(typename TContainerType::value_type const&)> const& pred)
        {
        return std::count_if(container.begin(), container.end(), pred);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DiagnosticsHelpers
{
private:
    DiagnosticsHelpers() {}
public:
    ECPRESENTATION_EXPORT static Utf8String CreateRuleIdentifier(PresentationKey const& rule);
    ECPRESENTATION_EXPORT static Utf8String CreateNodeKeyIdentifier(NavNodeKeyCR key);
    ECPRESENTATION_EXPORT static Utf8String CreateNodeIdentifier(NavNodeCR node);
    ECPRESENTATION_EXPORT static Utf8String CreateECInstanceKeyStr(ECClassInstanceKeyCR key);
    ECPRESENTATION_EXPORT static Utf8String CreateContentSetItemStr(ContentSetItemCR item);
    ECPRESENTATION_EXPORT static Utf8String CreateRelatedClassPathStr(RelatedClassPathCR path);
    ECPRESENTATION_EXPORT static void ReportRule(PresentationKey const& rule);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
namespace CommonStrings
    {
    constexpr Utf8CP RULESENGINE_NOTSPECIFIED = "@RulesEngine:LABEL_General_NotSpecified@";
    constexpr Utf8CP RULESENGINE_OTHER = "@RulesEngine:LABEL_General_Other@";
    constexpr Utf8CP RULESENGINE_VARIES = "@RulesEngine:LABEL_General_Varies@";
    constexpr Utf8CP RULESENGINE_MULTIPLEINSTANCES = "@RulesEngine:LABEL_General_MultipleInstances@";
    constexpr Utf8CP ECPRESENTATION_LABEL_SELECTEDITEMS = "@ECPresentation:CATEGORY_LABEL_SelectedItems@";
    constexpr Utf8CP ECPRESENTATION_DISPLAYLABEL = "@ECPresentation:FIELD_LABEL_DisplayLabel";
    constexpr Utf8CP ECPRESENTATION_DESCRIPTION_SELECTEDITEMS = "@ECPresentation:CATEGORY_DESCRIPTION_SelectedItems@";
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
