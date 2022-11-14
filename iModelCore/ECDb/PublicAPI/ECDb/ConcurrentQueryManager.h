/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <chrono>
#include <string>
#include <memory>
#include <functional>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;
typedef uint32_t TaskId;
struct ECDb;
struct ECSqlStatement;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QueryLimit final {
    private:
        static constexpr auto kCount = "count";
        static constexpr auto kOffset = "offset";

        int64_t m_count;
        int64_t m_offset;
    public:
        QueryLimit(int64_t count = -1, int64_t offset = -1) : m_count(count), m_offset(offset) {}
        int64_t GetCount() const noexcept { return m_count; }
        int64_t GetOffset() const noexcept { return m_offset; }
        ECDB_EXPORT void ToJs(BeJsValue&) const;
        ECDB_EXPORT static QueryLimit FromJs(BeJsConst const&) noexcept;

    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QueryQuota final {
    private:
        static constexpr auto kTime = "time";
        static constexpr auto kMemory = "memory";
        std::chrono::seconds m_timeLimit;
        uint32_t m_memoryLimit;
    public:
        QueryQuota() : m_timeLimit(0), m_memoryLimit(0) {}
        explicit QueryQuota(unsigned int timeLimit) : m_timeLimit(timeLimit), m_memoryLimit(0) {}
        QueryQuota(std::chrono::seconds timeLimit, uint32_t sizeInBytes) : m_timeLimit(timeLimit), m_memoryLimit(sizeInBytes) {}
        std::chrono::seconds MaxTimeAllowed() const noexcept { return m_timeLimit; }
        uint32_t MaxMemoryAllowed() const noexcept { return m_memoryLimit; }
        bool IsEmpty() const noexcept { return m_memoryLimit == 0 && m_timeLimit == std::chrono::seconds(0); }
        ECDB_EXPORT void ToJs(BeJsValue&) const;
        ECDB_EXPORT static QueryQuota FromJs(BeJsConst const&) noexcept;

    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QueryRequest {
    using Ptr = std::unique_ptr<QueryRequest>;
    enum class Kind {
        BlobIO=0,
        ECSql=1,
    };
    private:
        static constexpr auto JQuota = "quota";
        static constexpr auto JPriority = "priority";
        static constexpr auto JKind = "kind";
        static constexpr auto JUsePrimaryConn = "usePrimaryConn";
        static constexpr auto JRestartToken = "restartToken";
        static constexpr auto JDelay = "delay";
        QueryQuota m_quota;
        int32_t m_priority;
        Kind m_kind;
        bool m_usePrimaryConn;
        std::string m_restartToken;
        std::chrono::milliseconds m_delay;

    public:
        QueryRequest(Kind kind):m_usePrimaryConn(false), m_priority(0), m_kind(kind), m_delay(0u) {}
        virtual ~QueryRequest(){}
        ECDB_EXPORT QueryRequest(QueryRequest&&) noexcept;
        ECDB_EXPORT QueryRequest(const QueryRequest&) noexcept;
        ECDB_EXPORT QueryRequest& operator = (QueryRequest&&) noexcept;
        ECDB_EXPORT QueryRequest& operator = (const QueryRequest&) noexcept;

        QueryRequest& SetQuota(QueryQuota quota) noexcept { m_quota = quota; return *this;}
        QueryRequest& SetPriority(int32_t priority) noexcept { m_priority = priority;return *this;}
        QueryRequest& SetUsePrimaryConnection(bool usePrimary) { m_usePrimaryConn = usePrimary; return *this;}
        QueryRequest& SetRestartToken(std::string const& token) { m_restartToken = token; return *this;}
        QueryRequest& SetDelay(std::chrono::milliseconds t) noexcept { m_delay = t; return *this;}
        bool UsePrimaryConnection() const noexcept {return m_usePrimaryConn;}
        std::chrono::milliseconds GetDelay() const { return m_delay; }
        QueryQuota const& GetQuota() const noexcept {return m_quota;}
        std::string const& GetRestartToken() const { return m_restartToken; }
        int32_t GetPriority() const noexcept {return m_priority;}
        Kind GetKind() const noexcept {return m_kind;}
        template <class T>
        T const& GetAsConst() const{
            static_assert(std::is_base_of<QueryRequest, T>::value, "T must inherit from QueryRequest");
           // BeAssert(dynamic_cast<T const*>(this) != nullptr);
            return *static_cast<T const*>(this);
        }
        template <class T>
        T& GetAsRef() {
            static_assert(std::is_base_of<QueryRequest, T>::value, "T must inherit from QueryRequest");
            //BeAssert(dynamic_cast<T*>(this) != nullptr);
            return *static_cast<T*>(this);
        }
        ECDB_EXPORT virtual void FromJs(BeJsConst const& val);
        ECDB_EXPORT static Ptr Deserialize(BeJsValue const& val);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BlobIORequest : public QueryRequest{
    using Ptr = std::unique_ptr<BlobIORequest>;
    private:
        static constexpr auto JClassName = "className";
        static constexpr auto JAccessString = "accessString";
        static constexpr auto JInstanceId = "instanceId";
        static constexpr auto JRange = "range";
        std::string m_className;
        std::string m_accessString;
        uint64_t m_ecInstanceId;
        QueryLimit m_range;
    public:
        BlobIORequest(std::string const& className, std::string const& accessString, uint64_t instanceId, QueryLimit const & range)
            :QueryRequest(Kind::BlobIO), m_className(className), m_accessString(accessString), m_ecInstanceId(instanceId), m_range(range) {}
        virtual ~BlobIORequest(){}
        std::string const& GetClassName() const { return m_className; }
        std::string const& GetAccessString() const { return m_accessString; }
        BeInt64Id GetInstanceId() const { return  BeInt64Id(m_ecInstanceId); }
        QueryLimit const& GetRange() const { return m_range;}
        BlobIORequest& SetRange(QueryLimit const& range) { m_range= range; return *this;}
        ECDB_EXPORT static Ptr MakeRequest(std::string const& className, std::string const& propertyName, uint64_t instanceId) {
            return std::make_unique<BlobIORequest>(className, propertyName, instanceId, QueryLimit());
        }
        ECDB_EXPORT static Ptr MakeRequest(std::string const& className, std::string const& propertyName, uint64_t instanceId, QueryLimit const& range) {
            return std::make_unique<BlobIORequest>(className, propertyName, instanceId, range);
        }
        ECDB_EXPORT virtual void FromJs(BeJsConst const& val) override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
class ECSqlParams final {
    public:
    class ECSqlParam final {
        public:
            enum class Type {
                Boolean=0,
                Double=1,
                Id=2,
                IdSet=3,
                Integer=4,
                Long=5,
                Null=6,
                Point2d=7,
                Point3d=8,
                String=9,
                Blob=10,
                Struct=11,
            };
        private:
            static constexpr auto Jx = "x";
            static constexpr auto Jy = "y";
            static constexpr auto Jz = "z";
            Json::Value m_val;
            Type m_type;
            std::string m_name;
        public:
            ECSqlParam(ECSqlParam && rhs):m_val(std::move(rhs.m_val)), m_type(std::move(rhs.m_type)),m_name(std::move(rhs.m_name)){}
            ECDB_EXPORT ECSqlParam& operator = (ECSqlParam && rhs);
            ECSqlParam(const ECSqlParam & rhs):m_val(rhs.m_val), m_type(rhs.m_type),m_name(rhs.m_name){}
            ECDB_EXPORT ECSqlParam& operator = (const ECSqlParam & rhs);
            ECSqlParam():m_type(Type::Null){}
            ECSqlParam(std::string const& name, Type type, Json::Value const& val): m_type(type),m_val(val), m_name(name){}
            ECSqlParam(std::string const& name): m_type(Type::Null),m_val(Json::ValueType::nullValue), m_name(name){}
            ECSqlParam(std::string const& name, BeInt64Id const& val): m_type(Type::Id), m_val(val.ToHexStr()), m_name(name){}
            ECSqlParam(std::string const& name, std::string const& val): m_type(Type::String), m_val(val), m_name(name){}
            ECSqlParam(std::string const& name, double val): m_type(Type::Double), m_val(val), m_name(name){}
            ECSqlParam(std::string const& name, int val): m_type(Type::Integer), m_val(val), m_name(name){}
            ECSqlParam(std::string const& name, bool val): m_type(Type::Boolean), m_val(val), m_name(name){}
            ECSqlParam(std::string const& name, int64_t val): m_type(Type::Long), m_val(val), m_name(name){}
            ECSqlParam(std::string const& name, BeIdSet const& val): m_type(Type::IdSet), m_val(val.ToCompactString()), m_name(name){}
            ECDB_EXPORT ECSqlParam(std::string const& name, DPoint2d const& val);
            ECDB_EXPORT ECSqlParam(std::string const& name, DPoint3d const& val);
            ECDB_EXPORT ECSqlParam(std::string const& name, bvector<Byte> const& val);
            virtual ~ECSqlParam(){}
            ECDB_EXPORT int GetIndex() const;
            bool IsNull() const { return m_type == Type::Null;}
            Json::Value const& GetValue() const { return m_val; }
            Type GetType() const { return m_type;}
            std::string const& GetName() const { return m_name; }
            bool IsNamed() const { return GetIndex() == -1;}
            bool IsIndexed() const { return !IsNamed(); }
            ECDB_EXPORT BeInt64Id GetValueId() const;
            ECDB_EXPORT std::string GetValueString() const;
            ECDB_EXPORT double GetValueDouble() const;
            ECDB_EXPORT int64_t GetValueLong() const;
            ECDB_EXPORT int GetValueInt() const;
            ECDB_EXPORT bool GetValueBool() const;
            ECDB_EXPORT DPoint2d GetValuePoint2d() const;
            ECDB_EXPORT DPoint3d GetValuePoint3d() const;
            ECDB_EXPORT bvector<Byte> GetValueBlob() const;
            ECDB_EXPORT BeIdSet GetValueIdSet() const;
    };
    private:
        static constexpr auto JType = "type";
        static constexpr auto JValue = "value";
        std::map<std::string, ECSqlParam> m_params;
    public:
        ECSqlParams(){}
        ECSqlParams(ECSqlParams && rhs): m_params(std::move(rhs.m_params)) {}
        ECSqlParams(const ECSqlParams& rhs): m_params(rhs.m_params) {}
        ECDB_EXPORT ECSqlParams& operator = (ECSqlParams && rhs);
        ECDB_EXPORT ECSqlParams& operator = (const ECSqlParams & rhs);
        explicit ECSqlParams(Json::Value const& v) { FromJs(v); }
        virtual ~ECSqlParams(){}
        bool IsEmpty() const { return m_params.size() == 0; }
        size_t Count() const { return m_params.size(); }
        auto& GetParam(std::string const& name) { return m_params[name]; }
        auto& GetParam(int index) { return m_params[std::to_string(index)]; }
        ECSqlParams& BindBlob(std::string const& name, bvector<Byte> const& val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindBool(std::string const& name, bool val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindDouble(std::string const& name, double val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindId(std::string const& name, BeInt64Id val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindIdSet(std::string const& name, BeIdSet const& val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindInt(std::string const& name, int val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindLong(std::string const& name, int64_t val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindNull(std::string const& name) { m_params[name] = ECSqlParam(name); return *this;}
        ECSqlParams& BindPoint2d(std::string const& name, DPoint2dCR val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindPoint3d(std::string const& name, DPoint3dCR val) { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindString(std::string const& name, std::string const& val)  { m_params[name] = ECSqlParam(name, val); return *this;}
        ECSqlParams& BindBlob(int index, bvector<Byte> const& val) { BindBlob(std::to_string(index), val); return *this;}
        ECSqlParams& BindBool(int index, bool val) { BindBool(std::to_string(index), val); return *this;}
        ECSqlParams& BindDouble(int index, double val) { BindDouble(std::to_string(index), val); return *this;}
        ECSqlParams& BindId(int index, BeInt64Id val) { BindId(std::to_string(index), val); return *this;}
        ECSqlParams& BindIdSet(int index, BeIdSet const& val) { BindIdSet(std::to_string(index), val); return *this;}
        ECSqlParams& BindInt(int index, int val) { BindInt(std::to_string(index), val); return *this;}
        ECSqlParams& BindLong(int index, int64_t val) { BindLong(std::to_string(index), val); return *this;}
        ECSqlParams& BindNull(int index) { BindNull(std::to_string(index)); return *this;}
        ECSqlParams& BindPoint2d(int index, DPoint2dCR val) { BindPoint2d(std::to_string(index), val); return *this;}
        ECSqlParams& BindPoint3d(int index, DPoint3dCR val) { BindPoint3d(std::to_string(index), val); return *this;}
        ECSqlParams& BindString(int index, std::string const& val) { BindString(std::to_string(index), val); return *this;}
        ECDB_EXPORT void ToJs(Json::Value& val);
        ECDB_EXPORT void FromJs(Json::Value const& val);
        ECDB_EXPORT std::vector<std::string> GetKeys() const;
        ECDB_EXPORT bool TryBindTo(ECSqlStatement& stmt, std::string& err) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECSqlRequest : public QueryRequest{
    using Ptr = std::unique_ptr<ECSqlRequest>;
    enum class ECSqlValueFormat {
        ECSqlNames = 0,
        JsNames = 1
    };
    private:
        static constexpr auto JQuery = "query";
        static constexpr auto JArgs = "args";
        static constexpr auto JSuppressLogErrors = "suppressLogErrors";
        static constexpr auto JIncludeMetaData = "includeMetaData";
        static constexpr auto JAbbreviateBlobs = "abbreviateBlobs";
        static constexpr auto JConvertClassIdsToClassNames = "convertClassIdsToClassNames";
        static constexpr auto JLimit = "limit";
        static constexpr auto JValueFormat = "valueFormat";
        std::string m_query;
        ECSqlParams m_args;
        QueryLimit m_limit;
        bool m_abbreviateBlobs;
        bool m_suppressLogErrors;
        bool m_includeMetaData;
        bool m_convertClassIdsToClassNames;
        ECSqlValueFormat m_valueFmt;
    public:
        ECSqlRequest(std::string const& query, ECSqlParams&& args)
            :QueryRequest(Kind::ECSql), m_query(query), m_args(std::move(args)),m_abbreviateBlobs(false), m_suppressLogErrors(false),m_includeMetaData(true), m_convertClassIdsToClassNames(false),m_valueFmt(ECSqlValueFormat::ECSqlNames){}
        virtual ~ECSqlRequest(){}
        std::string const& GetQuery() const { return m_query; }
        ECSqlParams const& GetArgs() const { return  m_args; }
        ECSqlParams const& GetArgsR() { return  m_args; }
        bool GetAbbreviateBlobs() const {return m_abbreviateBlobs; }
        bool GetSuppressLogErrors() const {return m_suppressLogErrors; }
        bool GetIncludeMetaData() const {return m_includeMetaData; }
        bool GetConvertClassIdsToClassNames() const {return m_convertClassIdsToClassNames; }
        QueryLimit const& GetLimit() const {return m_limit;}
        ECSqlValueFormat GetValueFormat() const { return m_valueFmt; }
        ECSqlRequest& SetValueFmt(ECSqlValueFormat fmt) noexcept { m_valueFmt = fmt; return *this;}
        ECSqlRequest& SetLimit(QueryLimit limit) noexcept { m_limit = limit; return *this;}
        ECSqlRequest& SetAbbreviateBlobs(bool abbreviateBlobs) { m_abbreviateBlobs = abbreviateBlobs; return *this;}
        ECSqlRequest& SetSuppressLogErrors(bool suppressLogErrors) { m_suppressLogErrors = suppressLogErrors; return *this;}
        ECSqlRequest& SetIncludeMetaData(bool includeMetaData) { m_includeMetaData = includeMetaData; return *this;}
        ECSqlRequest& SetConvertClassIdsToClassNames(bool convertClassIdsToClassNames) { m_convertClassIdsToClassNames = convertClassIdsToClassNames; return *this;}
        ECSqlRequest& SetArgs(Json::Value const& args) { m_args.FromJs(args); return *this;}
        ECSqlRequest& SetArgs(ECSqlParams const& args) { m_args = args; return *this;}
        static Ptr MakeRequest(std::string const& query) {
            return std::make_unique<ECSqlRequest>(query, ECSqlParams());
        }
        static Ptr MakeRequest(std::string const& query, ECSqlParams&& args) {
            return std::make_unique<ECSqlRequest>(query, std::move(args));
        }
        static Ptr MakeRequest(std::string const& query, ECSqlParams& args) {
            return std::make_unique<ECSqlRequest>(query, std::move(args));
        }
        ECDB_EXPORT virtual void FromJs(BeJsConst const& val) override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QueryResponse : std::enable_shared_from_this<QueryResponse> {
    using Ptr = std::shared_ptr<QueryResponse>;
    enum class Kind {
        BlobIO=0,
        ECSql=1,
        NoResult=2,
    };
    struct Future final {
        struct Impl; //std::future not compitable with /clr
        private:
            Impl* m_impl;
            Future(Future&) = delete;
            Future& operator =( const Future&) = delete;
        public:
            ECDB_EXPORT Future(Future&&);
            ECDB_EXPORT Future& operator =(Future&&);
            Future(Impl* imp): m_impl(imp){}
            ECDB_EXPORT ~Future();
            ECDB_EXPORT void Cancel();
            ECDB_EXPORT void Wait();
            ECDB_EXPORT bool Valid();
            ECDB_EXPORT Ptr Get();

    };
    struct Stats {
        private:
            static constexpr auto kCpuTime = "cpuTime";
            static constexpr auto kTotalTime = "totalTime";
            static constexpr auto kTimeLimit = "timeLimit";
            static constexpr auto kMemLimit = "memLimit";
            static constexpr auto kMemUsed = "memUsed";
            std::chrono::microseconds m_cpuTime;
            std::chrono::milliseconds m_totalTime;
            std::chrono::milliseconds m_timeLimit;
            uint32_t m_memLimit;
            uint32_t m_memUsed;
        public:
            Stats():m_cpuTime(0ms),m_totalTime(0ms), m_timeLimit(0ms),m_memLimit(0), m_memUsed(0){}
            Stats(std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, uint32_t memUsed, QueryQuota const& quota):
                m_cpuTime(cpuTime), m_totalTime(totalTime),m_memLimit(quota.MaxMemoryAllowed()),m_memUsed(memUsed),
                m_timeLimit(std::chrono::duration_cast<std::chrono::milliseconds>(quota.MaxTimeAllowed())){}
            virtual ~Stats(){}
            std::chrono::microseconds CpuTime() const { return m_cpuTime;}
            std::chrono::milliseconds TotalTime() const { return m_totalTime;}
            std::chrono::milliseconds TimeLimit() const { return m_timeLimit;}
            uint32_t MemLimit() const { return m_memLimit;}
            uint32_t MemUsed() const { return m_memUsed;}
            ECDB_EXPORT void ToJs(BeJsValue&) const;
    };
    enum class Status {
        Done = 1,  // query ran to completion.
        Cancel = 2, // Requested by user.
        Partial = 3, // query was running but ran out of quota.
        Timeout = 4, // query time quota expired while it was in queue.
        QueueFull = 5, // could not submit the query as queue was full.
        Error = 100, // generic error
        Error_ECSql_PreparedFailed = Error + 1, // ecsql prepared failed
        Error_ECSql_StepFailed = Error + 2, // ecsql step failed
        Error_ECSql_RowToJsonFailed = Error + 3, // ecsql failed to serialized row to json.
        Error_ECSql_BindingFailed = Error + 4, // ecsql binding failed.
        Error_BlobIO_OpenFailed = Error + 5, // class or property or instance specified was not found or property as not of type blob.
        Error_BlobIO_OutOfRange = Error + 6, // range specified is invalid based on size of blob.
    };
    private:
        static constexpr auto JStatus = "status";
        static constexpr auto JError = "error";
        static constexpr auto JStats = "stats";
        static constexpr auto JKind = "kind";
        Status m_status;
        std::string m_error;
        Stats m_stats;
        Kind m_kind;
    public:
        QueryResponse(Kind kind, Stats stats, Status status, std::string error): m_status(status),m_kind(kind), m_stats(stats), m_error(error) {}
        virtual ~QueryResponse(){}
        Status GetStatus() const noexcept {return m_status;}
        std::string const& GetError() const noexcept {return m_error;}
        Stats const& GetStats() const { return m_stats; }
        bool IsDone() const noexcept { return m_status == Status::Done;}
        bool IsPartial() const noexcept { return m_status == Status::Partial;}
        bool IsSuccess() const noexcept { return m_status == Status::Done || m_status == Status::Partial;}
        bool IsError() const noexcept {return !IsSuccess();}
        Kind GetKind() const {return m_kind;}
        ECDB_EXPORT static Utf8CP StatusToString(QueryResponse::Status status);
        template <class T>
        T const& GetAsConst() const{
            static_assert(std::is_base_of<QueryResponse, T>::value, "T must inherit from QueryResponse");
           // BeAssert(dynamic_cast<T*>(this) != nullptr);
            return *static_cast<T const*>(this);
        }
        template <class T>
        T& GetAsRef() {
            static_assert(std::is_base_of<QueryResponse, T>::value, "T must inherit from QueryResponse");
            //BeAssert(dynamic_cast<T*>(this) != nullptr);
            return *static_cast<T*>(this);
        }
        ECDB_EXPORT void virtual ToJs(BeJsValue& v, bool includeData) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IJsSerializable {
    public:
        ECDB_EXPORT virtual void ToJs(BeJsValue&) const = 0;
        ECDB_EXPORT std::string Stringify(StringifyFormat format = StringifyFormat::Default) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QueryProperty final: IJsSerializable {
    struct List final: public std::vector<QueryProperty>, IJsSerializable {
        private:
            QueryProperty const& GetPropertyInfo(std::string const& name) const;
        public:
            QueryProperty const& operator [](std::string const& name) const { return GetPropertyInfo(name);}
            QueryProperty const& operator [](int index) const { return at(index); }
            ECDB_EXPORT void ToJs(BeJsValue& val) const override;
            ECDB_EXPORT void append(std::string className, std::string jsonName, std::string name, std::string typeName, bool generated, std::string extendedType, int index);
    };
    private:
        static constexpr auto JClass = "className";
        static constexpr auto JGenerated = "generated";
        static constexpr auto JIndex = "index";
        static constexpr auto JJsonName = "jsonName";
        static constexpr auto JName = "name";
        static constexpr auto JExtendedType = "extendedType";
        static constexpr auto JType = "typeName";
        std::string m_className;
        std::string m_jsonName;
        std::string m_name;
        std::string m_typeName;
        std::string m_extendedType;
        bool m_isGenerated;
        int  m_index;

    public:
        QueryProperty():m_index(-1), m_isGenerated(false){}
        QueryProperty(std::string className, std::string jsonName, std::string name, std::string typeName, bool generated, std::string extendedType, int index)
            :m_index(index), m_extendedType(extendedType),m_className(className), m_jsonName(jsonName),m_name(name), m_typeName(typeName), m_isGenerated(generated){}
        virtual ~QueryProperty(){}
        std::string const& GetClassName() const { return m_className;}
        std::string const& GetJsonName() const { return m_jsonName;}
        std::string const& GetName() const { return m_name;}
        std::string const& GetTypeName() const { return m_typeName;}
        std::string const& GetExtendedType() const { return m_extendedType;}
        bool IsGenerated() const { return m_isGenerated;}
        int GetIndex() const { return m_index;}
        bool IsValid() const {return m_index >=0;}
        ECDB_EXPORT void ToJs(BeJsValue& val) const override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECSqlResponse final : public QueryResponse{
    using Ptr = std::shared_ptr<ECSqlResponse>;
    static constexpr auto JData = "data";
    private:
        static constexpr auto JRowCount = "rowCount";
        static constexpr auto JMeta = "meta";
        std::string m_dataJson;
        uint32_t m_rowCount;
        QueryProperty::List m_properties;
    public:
        ECSqlResponse(Stats stats, Status status, std::string error, std::string & data, QueryProperty::List& meta, uint32_t rowCount)
            :QueryResponse(Kind::ECSql,stats, status, error), m_dataJson(std::move(data)), m_properties(std::move(meta)),m_rowCount(rowCount) {}
        virtual ~ECSqlResponse(){}
        QueryProperty::List const& GetProperties() const { return m_properties; }
        std::string const& asJsonString() const {return m_dataJson; }
        uint32_t GetRowCount() const {return m_rowCount;}
        ECDB_EXPORT void virtual ToJs(BeJsValue& v, bool includeData) const override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BlobIOResponse : public QueryResponse {
    using Ptr = std::shared_ptr<BlobIOResponse>;
    static constexpr auto JData = "data";
    static constexpr auto JBlobSize = "rawBlobSize";
    private:
        std::vector<uint8_t> m_buffer;
        uint32_t m_rawBlobSize;
    public:
        BlobIOResponse(Stats stats, Status status, std::string error, std::vector<uint8_t>& buffer, uint32_t rawBlobSize)
            : QueryResponse(Kind::BlobIO,stats, status, error), m_buffer(std::move(buffer)), m_rawBlobSize(rawBlobSize){}
        virtual ~BlobIOResponse(){}
        uint8_t const* GetData() const { return &m_buffer[0]; }
        uint32_t GetLength() const {return (uint32_t)m_buffer.size(); }
        uint32_t GetRawBlobSize() const {return m_rawBlobSize;}
        ECDB_EXPORT void virtual ToJs(BeJsValue& v, bool includeData) const override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ConcurrentQueryMgr final {
    using OnCompletion=std::function<void(QueryResponse::Ptr)>;
    using Ptr = std::unique_ptr<ConcurrentQueryMgr>;
    enum class ClearCacheOption { Yes, No};
    enum class DetachAttachDbs { Yes, No};
    struct Config final {
         static constexpr auto JThreads = "workerThreads";
         static constexpr auto JQueueSize = "requestQueueSize";
         static constexpr auto JIgnorePriority = "ignorePriority";
         static constexpr auto JQuota = "globalQuota";
        private:
            QueryQuota m_quota;
            uint32_t m_workerThreadCount;
            uint32_t m_requestQueueSize;
            bool m_ignorePriority;
            static Config From(std::string const& json);
        public:
            ECDB_EXPORT Config();
            ECDB_EXPORT bool Equals(Config const& rhs) const;
            bool operator == (Config const& rhs) { return Equals(rhs);}
            QueryQuota const& GetQuota() const { return m_quota;}
            uint32_t GetWorkerThreadCount() const{ return m_workerThreadCount;}
            uint32_t GetRequestQueueSize() const{ return m_requestQueueSize;}
            bool GetIgnorePriority() const {return m_ignorePriority; }
            Config& SetQuota(QueryQuota const& quota) { m_quota = quota; return *this; }
            Config& SetWorkerThreadCount(uint32_t workerThreadCount) { m_workerThreadCount = workerThreadCount; return *this;}
            Config& SetRequestQueueSize(uint32_t requestQueueSize) { m_requestQueueSize = requestQueueSize; return *this;}
            Config& SetIgnorePriority(bool ignorePriority) { m_ignorePriority = ignorePriority; return *this;}
            bool IsDefault() const { return this == &Config::GetDefault() || Config::GetDefault().Equals(*this);}
            ECDB_EXPORT static Config const& GetDefault();
            ECDB_EXPORT static Config GetFromEnv();
            //ECDB_EXPORT static Config& GetInstance();
            ECDB_EXPORT static Config From(BeJsValue);
            void Reset() { *this = GetDefault(); }
    };
    public:
        struct Impl; // prevent circular dependency on ECDb
    private:
        Impl* m_impl;
        ConcurrentQueryMgr(const ConcurrentQueryMgr&) = delete;
        ConcurrentQueryMgr& operator = (const ConcurrentQueryMgr&) = delete;
        ConcurrentQueryMgr(ConcurrentQueryMgr&&) = delete;
        ConcurrentQueryMgr& operator = (ConcurrentQueryMgr&&) = delete;
    public:
        ECDB_EXPORT ConcurrentQueryMgr(ECDb const&);
        ECDB_EXPORT ~ConcurrentQueryMgr();
        ECDB_EXPORT QueryResponse::Future Enqueue(QueryRequest::Ptr);
        ECDB_EXPORT void Enqueue(QueryRequest::Ptr, OnCompletion);
        ECDB_EXPORT bool Suspend(ClearCacheOption clearCache, DetachAttachDbs detachDbs);
        ECDB_EXPORT bool Resume();
        ECDB_EXPORT bool IsSuspended() const;
        // change config
        ECDB_EXPORT void SetWorkerPoolSize(uint32_t);
        ECDB_EXPORT void SetRequestQueueMaxSize(uint32_t);
        ECDB_EXPORT void SetCacheStatementsPerWork(uint32_t);
        ECDB_EXPORT void SetMaxQuota(QueryQuota const&);
        ECDB_EXPORT static ConcurrentQueryMgr& GetInstance(ECDb const&);
        ECDB_EXPORT static void ResetConfig(ECDb const&, Config const&);
        ECDB_EXPORT static Config const& GetConfig(ECDb const&);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECSqlReader {
    struct Row {
        enum class Format {
            UseJsonName,
            UseName
        };
        private:
            Json::Value const& m_row;
            QueryProperty::List const& m_columns;
            ECDB_EXPORT Json::Value const& GetValue(int index) const;
            ECDB_EXPORT Json::Value const& GetValue(std::string const& name) const;
        public:
            Row(Json::Value const& row, QueryProperty::List const& cols):m_row(row), m_columns(cols){}
            QueryProperty const& GetProperty(int index) const { return m_columns[index]; }
            QueryProperty const& GetProperty(std::string const& name) const {return m_columns[name]; }
            //=========
            Json::Value const& operator [] (std::string const& col) const { return GetValue(col);}
            Json::Value const& operator [] (QueryProperty const& col) const { return GetValue(col.GetIndex());}
            Json::Value const& operator [] (int col) const { return GetValue(col); }
            //=========
            size_t Count() const { return m_columns.size();}
            ECDB_EXPORT Json::Value ToJson(Format fmt = Format::UseJsonName) const;
    };
    private:
        ConcurrentQueryMgr& m_mgr;
        int64_t m_globalOffset;
        ECSqlParams m_args;
        Json::Value m_rows;
        QueryProperty::List m_columns;
        std::string m_ecsql;
        bool m_done;
        Json::Value::ArrayIndex m_it;
    private:
        uint32_t Read();
    public:
        ECDB_EXPORT ECSqlReader(ConcurrentQueryMgr& mgr, std::string ecsql, ECSqlParams const& args = ECSqlParams());
        ECSqlParams const& GetArgs() const {return m_args;}
        QueryProperty::List const& GetColumns() const { return m_columns; }
        Row GetRow() const { return Row(m_rows[m_it],m_columns);}
        ECDB_EXPORT bool Next();
};

END_BENTLEY_SQLITE_EC_NAMESPACE

