/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OptimizedExpressionsParameters
{
private:
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    NavNodeKeyCPtr m_inputNodeKey;
    Utf8String m_contentDisplayType;
public:
    OptimizedExpressionsParameters(IConnectionManagerCR connections, IConnectionCR connection, NavNodeKeyCPtr inputNodeKey, Utf8CP displayType)
        : m_connections(connections), m_connection(connection), m_inputNodeKey(inputNodeKey), m_contentDisplayType(displayType)
        {}
    Utf8StringCR GetContentDisplayType() const {return m_contentDisplayType;}
    NavNodeKeyCPtr GetInputNodeKey() const {return m_inputNodeKey;}
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionCR GetConnection() const {return m_connection;}
};

struct LogicalOptimizedExpression;
struct DisplayTypeOptimizedExpression;
struct IsOfClassOptimizedExpression;
struct IsInstanceNodeOptimizedExpression;
struct IsPropertyGroupingOptimizedExpression;
struct IsECClassGroupingOptimizedExpression;
struct ClassNameOptimizedExpression;
struct InstanceIdOptimizedExpression;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OptimizedExpression : RefCountedBase
{
protected:
    virtual bool _Value(OptimizedExpressionsParameters const& params) = 0;
    virtual bool _IsEqual(OptimizedExpression const& other) const = 0;
    virtual LogicalOptimizedExpression const* _AsLogicalOptimizedExpression() const {return nullptr;}
    virtual DisplayTypeOptimizedExpression const* _AsDisplayTypeOptimizedExpression() const {return nullptr;}
    virtual IsOfClassOptimizedExpression const* _AsIsOfClassOptimizedExpression() const {return nullptr;}
    virtual IsInstanceNodeOptimizedExpression const* _AsIsInstanceNodeOptimizedExpression() const {return nullptr;}
    virtual IsPropertyGroupingOptimizedExpression const* _AsIsPropertyGroupingOptimizedExpression() const {return nullptr;}
    virtual IsECClassGroupingOptimizedExpression const* _AsIsECClassGroupingOptimizedExpression() const {return nullptr;}
    virtual ClassNameOptimizedExpression const* _AsClassNameOptimizedExpression() const {return nullptr;}
    virtual InstanceIdOptimizedExpression const* _AsInstanceIdOptimizedExpression() const {return nullptr;}
public:
    LogicalOptimizedExpression const* AsLogicalOptimizedExpression() const {return _AsLogicalOptimizedExpression();}
    DisplayTypeOptimizedExpression const* AsDisplayTypeOptimizedExpression() const {return _AsDisplayTypeOptimizedExpression();}
    IsOfClassOptimizedExpression const* AsIsOfClassOptimizedExpression() const {return _AsIsOfClassOptimizedExpression();}
    IsInstanceNodeOptimizedExpression const* AsIsInstanceNodeOptimizedExpression() const {return _AsIsInstanceNodeOptimizedExpression();}
    IsPropertyGroupingOptimizedExpression const* AsIsPropertyGroupingOptimizedExpression() const {return _AsIsPropertyGroupingOptimizedExpression();}
    IsECClassGroupingOptimizedExpression const* AsIsECClassGroupingOptimizedExpression() const {return _AsIsECClassGroupingOptimizedExpression();}
    ClassNameOptimizedExpression const* AsClassNameOptimizedExpression() const {return _AsClassNameOptimizedExpression();}
    InstanceIdOptimizedExpression const* AsInstanceIdOptimizedExpression() const {return _AsInstanceIdOptimizedExpression();}
    bool IsEqual(OptimizedExpression const& other) const {return _IsEqual(other);}
    bool Value(OptimizedExpressionsParameters const& params) {return _Value(params);}
};
typedef RefCountedPtr<OptimizedExpression> OptimizedExpressionPtr;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LogicalOptimizedExpression : OptimizedExpression
{
private:
    OptimizedExpressionPtr m_left;
    OptimizedExpressionPtr m_right;
    bool m_useAndOperator;

private:
    LogicalOptimizedExpression(OptimizedExpressionPtr left, OptimizedExpressionPtr right, bool useAnd) : m_left(left), m_right(right), m_useAndOperator(useAnd) {}
protected:
    ECPRESENTATION_EXPORT bool _Value(OptimizedExpressionsParameters const& params) override;
    ECPRESENTATION_EXPORT bool _IsEqual(OptimizedExpression const& other) const override;
    LogicalOptimizedExpression const* _AsLogicalOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<LogicalOptimizedExpression> Create(OptimizedExpressionPtr left, OptimizedExpressionPtr right, bool useAnd)
        {
        return new LogicalOptimizedExpression(left, right, useAnd);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayTypeOptimizedExpression : OptimizedExpression
{
private:
    Utf8String m_preferredDisplayType;
    bool m_expectEqual;

private:
    DisplayTypeOptimizedExpression(Utf8CP type, bool equal) : m_preferredDisplayType(type), m_expectEqual(equal) {}
protected:
    ECPRESENTATION_EXPORT bool _Value(OptimizedExpressionsParameters const& params) override;
    ECPRESENTATION_EXPORT bool _IsEqual(OptimizedExpression const& other) const override;
    DisplayTypeOptimizedExpression const* _AsDisplayTypeOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<DisplayTypeOptimizedExpression> Create(Utf8CP type, bool equal) {return new DisplayTypeOptimizedExpression(type, equal);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IsOfClassOptimizedExpression : OptimizedExpression, IConnectionsListener
{
    struct Cache
        {
        bmap<ECN::ECClassId, bool> m_results;
        ECN::ECClassCP m_expectedClass;
        Cache() : m_expectedClass(nullptr) {}
        Cache(ECN::ECClassCP expectedClass) : m_expectedClass(expectedClass) {}
        };

private:
    Utf8String m_schemaName;
    Utf8String m_className;
    bmap<ECDb const*, Cache> m_cache;
    IConnectionManagerCP m_connections;
    BeMutex& m_cacheMutex;
private:
    IsOfClassOptimizedExpression(Utf8CP schema, Utf8CP className, BeMutex& mutex) : m_schemaName(schema), m_className(className), m_connections(nullptr), m_cacheMutex(mutex) {}
protected:
    ECPRESENTATION_EXPORT bool _Value(OptimizedExpressionsParameters const& params) override;
    ECPRESENTATION_EXPORT bool _IsEqual(OptimizedExpression const& other) const override;
    IsOfClassOptimizedExpression const* _AsIsOfClassOptimizedExpression() const override {return this;}
    ECPRESENTATION_EXPORT void _OnConnectionEvent(ConnectionEvent const&) override;
public:
    static RefCountedPtr<IsOfClassOptimizedExpression> Create(Utf8CP schema, Utf8CP className, BeMutex& mutex) {return new IsOfClassOptimizedExpression(schema, className, mutex);}
    ECPRESENTATION_EXPORT ~IsOfClassOptimizedExpression();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassNameOptimizedExpression : OptimizedExpression, IConnectionsListener
{
private:
    Utf8String m_className;
    bmap<ECDb const*, bmap<ECN::ECClassId, bool>> m_resultsCache;
    IConnectionManagerCP m_connections;
    BeMutex& m_cacheMutex;
private:
    ClassNameOptimizedExpression(Utf8CP className, BeMutex& mutex) : m_className(className), m_connections(nullptr), m_cacheMutex(mutex) {}
protected:
    ECPRESENTATION_EXPORT bool _Value(OptimizedExpressionsParameters const& params) override;
    ECPRESENTATION_EXPORT bool _IsEqual(OptimizedExpression const& other) const override;
    ClassNameOptimizedExpression const* _AsClassNameOptimizedExpression() const override {return this;}
    ECPRESENTATION_EXPORT void _OnConnectionEvent(ConnectionEvent const&) override;
public:
    static RefCountedPtr<ClassNameOptimizedExpression> Create(Utf8CP className, BeMutex& mutex) {return new ClassNameOptimizedExpression(className, mutex);}
    ECPRESENTATION_EXPORT ~ClassNameOptimizedExpression();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceIdOptimizedExpression : OptimizedExpression
{
private:
    BeInt64Id m_instanceId;
private:
    InstanceIdOptimizedExpression(BeInt64Id instanceId) : m_instanceId(instanceId) {}
protected:
    ECPRESENTATION_EXPORT bool _Value(OptimizedExpressionsParameters const& params) override;
    ECPRESENTATION_EXPORT bool _IsEqual(OptimizedExpression const& other) const override;
    InstanceIdOptimizedExpression const* _AsInstanceIdOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<InstanceIdOptimizedExpression> Create(BeInt64Id instanceId) {return new InstanceIdOptimizedExpression(instanceId);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IsInstanceNodeOptimizedExpression : OptimizedExpression
{
private:
    IsInstanceNodeOptimizedExpression() {}
protected:
    bool _Value(OptimizedExpressionsParameters const& params) override {return params.GetInputNodeKey().IsValid() && nullptr != params.GetInputNodeKey()->AsECInstanceNodeKey();}
    bool _IsEqual(OptimizedExpression const& other) const override {return nullptr != other.AsIsInstanceNodeOptimizedExpression();}
    IsInstanceNodeOptimizedExpression const* _AsIsInstanceNodeOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<IsInstanceNodeOptimizedExpression> Create() {return new IsInstanceNodeOptimizedExpression();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IsPropertyGroupingOptimizedExpression : OptimizedExpression
{
private:
    IsPropertyGroupingOptimizedExpression() {}
protected:
    bool _Value(OptimizedExpressionsParameters const& params) override {return params.GetInputNodeKey().IsValid() && params.GetInputNodeKey()->GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode);}
    bool _IsEqual(OptimizedExpression const& other) const override {return nullptr != other.AsIsPropertyGroupingOptimizedExpression();}
    IsPropertyGroupingOptimizedExpression const* _AsIsPropertyGroupingOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<IsPropertyGroupingOptimizedExpression> Create() {return new IsPropertyGroupingOptimizedExpression();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IsECClassGroupingOptimizedExpression : OptimizedExpression
{
private:
    IsECClassGroupingOptimizedExpression() {}
protected:
    bool _Value(OptimizedExpressionsParameters const& params) override {return params.GetInputNodeKey().IsValid() && params.GetInputNodeKey()->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode);}
    bool _IsEqual(OptimizedExpression const& other) const override {return nullptr != other.AsIsECClassGroupingOptimizedExpression();}
    IsECClassGroupingOptimizedExpression const* _AsIsECClassGroupingOptimizedExpression() const override {return this;}
public:
    static RefCountedPtr<IsECClassGroupingOptimizedExpression> Create() {return new IsECClassGroupingOptimizedExpression();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE