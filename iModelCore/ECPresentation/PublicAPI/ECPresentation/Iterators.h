/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename T>
struct IteratorImpl
{
protected:
    virtual std::unique_ptr<IteratorImpl<T>> _Copy() const = 0;
    virtual bool _Equals(IteratorImpl<T> const& other) const = 0;
    virtual void _Next(size_t count) = 0;
    virtual T _GetCurrent() const = 0;
public:
    virtual ~IteratorImpl() {}
    bool operator==(IteratorImpl<T> const& other) const {return _Equals(other);}
    bool operator!=(IteratorImpl<T> const& other) const {return !_Equals(other);}
    T operator*() const {return _GetCurrent();}
    IteratorImpl<T>& operator++() {_Next(1); return *this;}
    IteratorImpl<T>& operator+=(size_t count) {_Next(count); return *this;}
    std::unique_ptr<IteratorImpl<T>> operator+(size_t count) const
        {
        std::unique_ptr<IteratorImpl<T>> copy = Copy();
        copy->_Next(count);
        return copy;
        }
    std::unique_ptr<IteratorImpl<T>> Copy() const {return _Copy();}
};

/*=================================================================================**//**
* Iterator with no results.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename T>
struct EmptyIteratorImpl : IteratorImpl<T>
{
protected:
    std::unique_ptr<IteratorImpl<T>> _Copy() const override {return std::make_unique<EmptyIteratorImpl<T>>();}
    bool _Equals(IteratorImpl<T> const& other) const override {return true;}
    void _Next(size_t) override {}
    T _GetCurrent() const override {return T();}
};

/*=================================================================================**//**
* Iterator that iterates over another given iterator.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TIterator, typename TValue>
struct IterableIteratorImpl : IteratorImpl<TValue>
{
private:
    TIterator m_iter;
protected:
    std::unique_ptr<IteratorImpl<TValue>> _Copy() const override {return std::make_unique<IterableIteratorImpl<TIterator, TValue>>(m_iter);}
    bool _Equals(IteratorImpl<TValue> const& other) const override {return m_iter == static_cast<IterableIteratorImpl<TIterator, TValue> const&>(other).m_iter;}
    void _Next(size_t count) override {m_iter += count;}
    TValue _GetCurrent() const override {return *m_iter;}
public:
    IterableIteratorImpl(TIterator iter) : m_iter(iter) {}
};

/*=================================================================================**//**
* Similar to `IterableIteratorImpl`, but also transforms the value from wrapped iterator using
* given transformation function.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TWrappedIterator, typename TWrappedValue, typename TOutputValue>
struct TransformingIterableIteratorImpl : IteratorImpl<TOutputValue>
{
typedef TOutputValue(*TTransformFunc)(TWrappedValue const&);
private:
    TWrappedIterator m_wrappedIter;
    TTransformFunc m_transformFunc;
protected:
    std::unique_ptr<IteratorImpl<TOutputValue>> _Copy() const override
        {
        return std::make_unique<TransformingIterableIteratorImpl<TWrappedIterator, TWrappedValue, TOutputValue>>(m_wrappedIter, m_transformFunc);
        }
    bool _Equals(IteratorImpl<TOutputValue> const& other) const override
        {
        return m_wrappedIter == static_cast<TransformingIterableIteratorImpl<TWrappedIterator, TWrappedValue, TOutputValue> const&>(other).m_wrappedIter;
        }
    void _Next(size_t count) override {m_wrappedIter += count;}
    TOutputValue _GetCurrent() const override {return m_transformFunc(*m_wrappedIter);}
public:
    TransformingIterableIteratorImpl(TWrappedIterator wrappedIter, TTransformFunc transformFunc)
        : m_wrappedIter(wrappedIter), m_transformFunc(transformFunc)
        {}
};

/*=================================================================================**//**
* Iterator that uses indexing operator `[]` to iterate over given container.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TContainer, typename TValue>
struct RandomAccessIteratorImpl : IteratorImpl<TValue>
{
private:
    TContainer const& m_container;
    size_t m_currIndex;
protected:
    std::unique_ptr<IteratorImpl<TValue>> _Copy() const override {return std::make_unique<RandomAccessIteratorImpl<TContainer, TValue>>(m_container, m_currIndex);}
    bool _Equals(IteratorImpl<TValue> const& other) const override
        {
        return &m_container == &static_cast<RandomAccessIteratorImpl<TContainer, TValue> const&>(other).m_container
            && m_currIndex == static_cast<RandomAccessIteratorImpl<TContainer, TValue> const&>(other).m_currIndex;
        }
    void _Next(size_t count) override {m_currIndex += count;}
    TValue _GetCurrent() const override {return m_container[m_currIndex];}
public:
    RandomAccessIteratorImpl(TContainer const& container, size_t startIndex = 0)
        : m_container(container), m_currIndex(startIndex)
        {}
};

/*=================================================================================**//**
* A wrapper over `IteratorImpl` to allow using it by value.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename T>
struct IteratorWrapper
{
private:
    std::unique_ptr<IteratorImpl<T>> m_impl;
public:
    IteratorWrapper() : m_impl(nullptr) {}
    IteratorWrapper(std::unique_ptr<IteratorImpl<T>> impl) : m_impl(std::move(impl)) {}
    IteratorWrapper(IteratorWrapper<T>&& other) : m_impl(std::move(other.m_impl)) {}
    IteratorWrapper(IteratorWrapper<T> const& other) : m_impl(other.m_impl ? other.m_impl->Copy() : nullptr) {}
    // Note: Not having this destructor causes a crash when running some of our tests on Windows using an
    // optimized build. That seems like a bug in optimizer, so the destructor might become unnecessary when that's fixed.
    ~IteratorWrapper() {m_impl = nullptr;}
    IteratorWrapper<T>& operator=(IteratorWrapper<T>&& other) {m_impl = std::move(other.m_impl); return *this;}
    IteratorWrapper<T>& operator=(IteratorWrapper<T> const& other) {m_impl = other.m_impl ? other.m_impl->Copy() : nullptr; return *this;}
    bool operator==(IteratorWrapper<T> const& other) const {return !m_impl && !other.m_impl || (m_impl && other.m_impl && *m_impl == *other.m_impl);}
    bool operator!=(IteratorWrapper<T> const& other) const {return !operator==(other);}
    IteratorWrapper<T>& operator++() {if (m_impl) {++(*m_impl);} return *this;}
    IteratorWrapper<T>& operator+=(size_t count) {if (m_impl) {(*m_impl) += count;} return *this;}
    IteratorWrapper<T> operator+(size_t count) const {return m_impl ? IteratorWrapper<T>((*m_impl) + count) : IteratorWrapper<T>();}
    T operator*() const {return m_impl ? **m_impl : T();}
    bool IsValid() const {return nullptr != m_impl;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
