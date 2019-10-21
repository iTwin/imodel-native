/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define EXTENDEDDATA_RAPIDJSON_CHUNK_SIZE 256

//=======================================================================================
//! An interface for a class that contains rapidjson extended data.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct IRapidJsonExtendedDataHolder
{
protected:
    virtual ~IRapidJsonExtendedDataHolder() {}
    virtual RapidJsonValueR _GetExtendedData() const = 0;
    virtual rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const = 0;
public:
    RapidJsonValueCR GetExtendedData() const {return _GetExtendedData();}
    RapidJsonValueR GetExtendedDataR() {return _GetExtendedData();}
    rapidjson::MemoryPoolAllocator<>& GetExtendedDataAllocator() {return _GetExtendedDataAllocator();}
};

//=======================================================================================
//! The default implementation of @ref IRapidJsonExtendedDataHolder which stores the
//! extended data as a private member.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
template<typename TBase = IRapidJsonExtendedDataHolder>
struct RapidJsonExtendedDataHolder : TBase
{
private:
    rapidjson::MemoryPoolAllocator<> m_allocator;
    mutable rapidjson::Document m_extendedData;
protected:
    RapidJsonValueR _GetExtendedData() const override {return m_extendedData;}
    virtual rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const override {return m_extendedData.GetAllocator();}
public:
    RapidJsonExtendedDataHolder() : m_allocator(EXTENDEDDATA_RAPIDJSON_CHUNK_SIZE), m_extendedData(&m_allocator) {m_extendedData.SetObject();}
};

//=======================================================================================
//! A helper class used to easily handle rapidjson extended data.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct RapidJsonAccessor
{
private:
    rapidjson::MemoryPoolAllocator<>* m_allocator;
    rapidjson::Value* m_writableData;
    rapidjson::Value const* m_readonlyData;
    bool m_ownsData;

private:
    void InitWritable(RapidJsonValueCR json)
        {
        Cleanup();
        m_allocator = new rapidjson::MemoryPoolAllocator<>(EXTENDEDDATA_RAPIDJSON_CHUNK_SIZE);
        m_writableData = new rapidjson::Value(json, *m_allocator);
        if (!m_writableData->IsObject())
            m_writableData->SetObject();
        m_readonlyData = m_writableData;
        m_ownsData = true;
        }
    void Cleanup()
        {
        if (m_ownsData)
            {
            DELETE_AND_CLEAR(m_writableData);
            DELETE_AND_CLEAR(m_allocator);
            }
        }

protected:
    //! Get the rapidjson allocator used by the wrapped extended data object.
    rapidjson::MemoryPoolAllocator<>& GetAllocator() const {return *m_allocator;}

    //! Get writable json object.
    //! @warning Only valid if this object is created writable.
    rapidjson::Value& GetJsonR() {return *m_writableData;}

    //! Add a json member into the wrapped extended data object.
    //! @param[in] name The name of the json member.
    //! @param[in] value The value to add.
    //! @note If a member with the supplied name already exists in the wrapped
    //! extended data object, it gets replaced.
    void AddMember(Utf8CP name, rapidjson::Value&& value)
        {
        rapidjson::Value::MemberIterator iterator = m_writableData->FindMember(name);
        if (iterator == m_writableData->MemberEnd())
            m_writableData->AddMember(rapidjson::GenericStringRef<Utf8Char>(name), value, *m_allocator);
        else
            iterator->value = value;
        }

public:
    //! Initializes an empty read-write accessor.
    RapidJsonAccessor() : m_ownsData(false), m_allocator(nullptr), m_writableData(nullptr) {InitWritable(rapidjson::Value(rapidjson::kObjectType));}

    //! Copy constructor.
    RapidJsonAccessor(RapidJsonAccessor const& other)
        : m_ownsData(other.m_ownsData), m_allocator(nullptr), m_writableData(nullptr)
        {
        if (m_ownsData)
            {
            InitWritable(rapidjson::Value(rapidjson::kObjectType));
            MergeWith(other);
            }
        else
            {
            m_readonlyData = other.m_readonlyData;
            }
        }

    //! Move constructor.
    RapidJsonAccessor(RapidJsonAccessor&& other)
        : m_allocator(other.m_allocator), m_writableData(other.m_writableData), m_readonlyData(other.m_readonlyData), m_ownsData(other.m_ownsData)
        {
        other.m_ownsData = false;
        }

    //! Initializes a read-only accessor with the specified JSON. Does not copy the supplied JSON.
    RapidJsonAccessor(RapidJsonValueCR data) : m_ownsData(false), m_allocator(nullptr), m_writableData(nullptr), m_readonlyData(&data) {}

    //! Initializes a read-only accessor with the JSON contained in the specified IRapidJsonExtendedDataHolder. Does not copy the supplied JSON.
    RapidJsonAccessor(IRapidJsonExtendedDataHolder const& holder) : m_ownsData(false), m_allocator(nullptr), m_writableData(nullptr), m_readonlyData(&holder.GetExtendedData()) {}

    //! Initializes a read-write accessor with the given JSON. Does not copy the supplied JSON
    RapidJsonAccessor(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator)
        : m_ownsData(false), m_allocator(&allocator), m_writableData(&data), m_readonlyData(m_writableData)
        {
        if (!m_writableData->IsObject())
            m_writableData->SetObject();
        }

    //! Initializes a read-write accessor with the JSON contained in the specified IRapidJsonExtendedDataHolder. Does not copy the supplied JSON.
    RapidJsonAccessor(IRapidJsonExtendedDataHolder& holder)
        : m_ownsData(false), m_allocator(&holder.GetExtendedDataAllocator()), m_writableData(&holder.GetExtendedDataR()), m_readonlyData(m_writableData)
        {
        if (!m_writableData->IsObject())
            m_writableData->SetObject();
        }

    //! Initializes a read-write accessor with a copy of the supplied RapidJsonAccessor object.
    // RapidJsonAccessor(RapidJsonAccessor const& other) {InitWritable(*other.m_readonlyData);}

    //! Destructor
    ~RapidJsonAccessor() {Cleanup();}

    //! Compares whether the given RapidJsonAccessor is equal to this one.
    bool operator==(RapidJsonAccessor const& other) const {return m_readonlyData == other.m_readonlyData || *m_readonlyData == *other.m_readonlyData;}

    //! Initializes a read-write accessor with a copy of the supplied RapidJsonAcccessor object.
    RapidJsonAccessor& operator=(RapidJsonAccessor const& other) {InitWritable(*other.m_readonlyData); return *this;}

    //! Returns the contained JSON.
    RapidJsonValueCR GetJson() const {return *m_readonlyData;}

    //! Re-initializes this object using the given JSON.
    //! @param[in] json     The JSON to initialize from.
    //! @param[in] makeCopy Should the given JSON be copied. If true, this object becomes a writable accessor.
    //!                     If false, it becomes a read-only accessor.
    void FromJson(RapidJsonValueCR json, bool makeCopy)
        {
        Cleanup();
        m_ownsData = makeCopy;
        if (makeCopy)
            InitWritable(json);
        else
            m_readonlyData = &json;
        }

    //! Merges the given RapidJsonAccessor object into this one.
    void MergeWith(RapidJsonAccessor const& other)
        {
        if (nullptr == other.m_readonlyData)
            return;

        if (nullptr == m_writableData || nullptr == m_allocator)
            {
            BeAssert(false && "This is read-only extended data");
            return;
            }

        for (auto iter = other.m_readonlyData->MemberBegin(); iter != other.m_readonlyData->MemberEnd(); ++iter)
            {
#ifdef WIP
            if (m_readonlyData->HasMember(iter->name.GetString()))
                BeAssert(*m_readonlyData == iter->value);
#endif
            AddMember(iter->name.GetString(), rapidjson::Value(iter->value, *m_allocator));
            }
        }
};

//=======================================================================================
//! An interface for a class that contains jsoncpp extended data.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct IJsonCppExtendedDataHolder
{
protected:
    virtual ~IJsonCppExtendedDataHolder() {}
    virtual JsonValueR _GetExtendedData() const = 0;
public:
    JsonValueCR GetExtendedData() const {return _GetExtendedData();}
    JsonValueR GetExtendedDataR() {return _GetExtendedData();}
};

//=======================================================================================
//! The default implementation of @ref IJsonCppExtendedDataHolder which stores the
//! extended data as a private member.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
template<typename TBase = IJsonCppExtendedDataHolder>
struct JsonCppExtendedDataHolder : TBase
{
private:
    mutable Json::Value m_extendedData;
protected:
    JsonValueR _GetExtendedData() const override {return m_extendedData;}
public:
    JsonCppExtendedDataHolder() : m_extendedData(Json::objectValue) {}
};

//=======================================================================================
//! A helper class used to easily handle jsoncpp extended data.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct JsonCppAccessor
{
private:
    Json::Value* m_writableData;
    Json::Value const* m_readonlyData;
    bool m_ownsData;

private:
    void InitWritable(JsonValueCR json)
        {
        m_writableData = new Json::Value(json);
        m_readonlyData = m_writableData;
        m_ownsData = true;
        }
    void Cleanup()
        {
        if (m_ownsData)
            DELETE_AND_CLEAR(m_writableData);
        }

protected:
    //! Get writable json object.
    //! @warning Only valid if this object is created writable.
    JsonValueR GetJsonR() {return *m_writableData;}

    //! Add a json member into the wrapped extended data object.
    //! @param[in] name The name of the json member.
    //! @param[in] value The value to add.
    //! @note If a member with the supplied name already exists in the wrapped
    //! extended data object, it gets replaced.
    void AddMember(Utf8CP name, Json::Value value) {GetJsonR()[name] = value;}

public:
    //! Initializes an empty read-write accessor.
    JsonCppAccessor() {InitWritable(Json::Value(Json::objectValue));}

    //! Initializes a read-only accessor with the specified JSON. Does not copy the supplied JSON.
    JsonCppAccessor(JsonValueCR data) : m_ownsData(false),m_writableData(nullptr), m_readonlyData(&data) {}

    //! Initializes a read-only accessor with the JSON contained in the specified IJsonCppExtendedDataHolder. Does not copy the supplied JSON.
    JsonCppAccessor(IJsonCppExtendedDataHolder const& holder) : m_ownsData(false), m_writableData(nullptr), m_readonlyData(&holder.GetExtendedData()) {}

    //! Initializes a read-write accessor with the given JSON. Does not copy the supplied JSON
    JsonCppAccessor(JsonValueR data) : m_ownsData(false), m_writableData(&data), m_readonlyData(m_writableData) {}

    //! Initializes a read-write accessor with the given JSON.
    JsonCppAccessor(Json::Value&& data) {InitWritable(Json::Value()); m_writableData->swap(data);}

    //! Initializes a read-write accessor with the JSON contained in the specified IJsonCppExtendedDataHolder. Does not copy the supplied JSON.
    JsonCppAccessor(IJsonCppExtendedDataHolder& holder) : m_ownsData(false), m_writableData(&holder.GetExtendedDataR()), m_readonlyData(m_writableData) {}

    //! Initializes a read-write accessor with a copy of the supplied JsonCppAccessor object.
    JsonCppAccessor(JsonCppAccessor const& other) {InitWritable(*other.m_readonlyData);}

    //! Destructor
    ~JsonCppAccessor() {Cleanup();}

    //! Compares whether the given JsonCppAccessor is equal to this one.
    bool operator==(JsonCppAccessor const& other) const {return m_readonlyData == other.m_readonlyData || 0 == m_readonlyData->compare(*other.m_readonlyData);}

    //! Returns the contained JSON.
    JsonValueCR GetJson() const {return *m_readonlyData;}

    //! Re-initializes this object using the given JSON.
    //! @param[in] json     The JSON to initialize from.
    //! @param[in] makeCopy Should the given JSON be copied. If true, this object becomes a writable accessor.
    //!                     If false, it becomes a read-only accessor.
    void FromJson(JsonValueCR json, bool makeCopy)
        {
        Cleanup();
        m_ownsData = makeCopy;
        if (makeCopy)
            InitWritable(json);
        else
            m_readonlyData = &json;
        }

    //! Merges the given JsonCppAccessor object into this one.
    void MergeWith(JsonCppAccessor const& other)
        {
        if (nullptr == other.m_readonlyData)
            return;

        if (nullptr == m_writableData)
            {
            BeAssert(false && "This is read-only extended data");
            return;
            }

        for (auto iter = other.m_readonlyData->begin(); iter != other.m_readonlyData->end(); ++iter)
            AddMember(iter.memberName(), *iter);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
