/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/Iterators.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! An interface for an abstract data source.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2015
//=======================================================================================
template<typename T>
struct IDataSource : RefCountedBase
{
typedef IteratorWrapper<T> Iterator;

protected:
    virtual ~IDataSource() {}
    virtual T _Get(size_t index) const = 0;
    virtual size_t _GetSize() const = 0;
    virtual Iterator _CreateFrontIterator() const = 0;
    virtual Iterator _CreateBackIterator() const = 0;

public:
    T Get(size_t index) const {return _Get(index);}
    T operator[](size_t index) const {return Get(index);}
    size_t GetSize() const {return _GetSize();}
    Iterator begin() const {return _CreateFrontIterator();}
    Iterator end() const {return _CreateBackIterator();}
};
template<typename T> using IDataSourcePtr = RefCountedPtr<IDataSource<T>>;
template<typename T> using IDataSourceCPtr = RefCountedPtr<IDataSource<T> const>;

//=======================================================================================
//! An empty data source.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2015
//=======================================================================================
template<typename T>
struct EmptyDataSource : IDataSource<T>
{
typedef typename IDataSource<T>::Iterator Iterator;
protected:
    EmptyDataSource() {}
    T _Get(size_t index) const override {return T();}
    size_t _GetSize() const override {return 0;}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<T>>());}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<T>>());}
public:
    static RefCountedPtr<EmptyDataSource> Create() {return new EmptyDataSource();}
};

//=======================================================================================
//! A paged @ref IDataSource.
//! @ingroup GROUP_Presentation
// @bsiclass                                     Grigas.Petraitis                01/2016
//=======================================================================================
template<typename T>
struct PagingDataSource : IDataSource<T>
{
typedef typename IDataSource<T>::Iterator Iterator;

private:
    IDataSourceCPtr<T> m_source;
    size_t m_pageStart;
    size_t m_pageSize;

private:
    PagingDataSource(IDataSource<T> const& source, size_t pageStart, size_t pageSize)
        : m_source(&source), m_pageStart(pageStart), m_pageSize(pageSize)
        {}

protected:
    T _Get(size_t index) const override
        {
        if (0 != m_pageSize && index >= m_pageSize)
            return T();

        size_t actualIndex = m_pageStart + index;
        size_t sourceSize = m_source->GetSize();
        if (actualIndex >= sourceSize)
            return T();

        return m_source->Get(actualIndex);
        }

    size_t _GetSize() const override
        {
        size_t sourceSize = m_source->GetSize();
        if (m_pageStart >= sourceSize)
            return 0;

        if (0 != m_pageSize && sourceSize - m_pageStart > m_pageSize)
            return m_pageSize;

        return sourceSize - m_pageStart;
        }

    Iterator _CreateFrontIterator() const override
        {
        size_t offset = (m_pageStart < m_source->GetSize()) ? m_pageStart : m_source->GetSize();
        return Iterator(std::make_unique<IterableIteratorImpl<Iterator, T>>(m_source->begin() += offset));
        }

    Iterator _CreateBackIterator() const override
        {
        size_t offset = ((0 != m_pageSize) && (m_pageStart + m_pageSize < m_source->GetSize())) ? (m_pageStart + m_pageSize) : m_source->GetSize();
        return Iterator(std::make_unique<IterableIteratorImpl<Iterator, T>>(m_source->begin() += offset));
        }

public:
    static RefCountedPtr<PagingDataSource<T>> Create()
        {
        return new PagingDataSource(*EmptyDataSource<T>::Create(), 0, 0);
        }
    static RefCountedPtr<PagingDataSource<T>> Create(IDataSource<T> const& source, size_t pageStart, size_t pageSize)
        {
        return new PagingDataSource(source, pageStart, pageSize);
        }
    size_t GetTotalSize() const {return m_source->GetSize();}
};
template<typename T> using PagingDataSourcePtr = RefCountedPtr<PagingDataSource<T>>;
template<typename T> using PagingDataSourceCPtr = RefCountedPtr<PagingDataSource<T> const>;

//=======================================================================================
//! An @ref IDataSource based on a given bvector.
//! @ingroup GROUP_Presentation
// @bsiclass                                     Grigas.Petraitis                12/2016
//=======================================================================================
template<typename T>
struct VectorDataSource : IDataSource<T>
{
typedef typename IDataSource<T>::Iterator Iterator;
private:
    bvector<T> m_vec;
    VectorDataSource(bvector<T> vec) : m_vec(vec) {}
protected:
    T _Get(size_t index) const override {return m_vec[index];}
    size_t _GetSize() const override {return m_vec.size();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<typename bvector<T>::const_iterator, T>>(m_vec.begin()));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<typename bvector<T>::const_iterator, T>>(m_vec.end()));}
public:
    static RefCountedPtr<VectorDataSource<T>> Create(bvector<T> vec) {return new VectorDataSource(vec);}
};

//=======================================================================================
//! An @ref IDataSource that loads and caches all items from the given data source.
//! @ingroup GROUP_Presentation
// @bsiclass                                     Grigas.Petraitis                11/2017
//=======================================================================================
template<typename T>
struct PreloadedDataSource : IDataSource<T>
{
typedef typename IDataSource<T>::Iterator Iterator;
private:
    RefCountedPtr<VectorDataSource<T>> m_source;
private:
    PreloadedDataSource(IDataSource<T> const& source)
        {
        bvector<T> items;
        for (auto const& item : source)
            items.push_back(item);
        m_source = VectorDataSource<T>::Create(items);
        }
protected:
    T _Get(size_t index) const override {return m_source->Get(index);}
    size_t _GetSize() const override {return m_source->GetSize();}
    Iterator _CreateFrontIterator() const override {return m_source->begin();}
    Iterator _CreateBackIterator() const override {return m_source->end();}
public:
    static RefCountedPtr<PreloadedDataSource<T>> Create(IDataSource<T> const& source) {return new PreloadedDataSource(source);}
};

//=======================================================================================
//! An @ref IDataSource driven container.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2015
//=======================================================================================
template<typename TItem, typename TDataSource = IDataSource<TItem>, typename TEmptyDataSource = EmptyDataSource<TItem>>
struct DataContainer
{
typedef typename TDataSource::Iterator Iterator;

protected:
    RefCountedPtr<TDataSource const> m_source;

public:
    //! Constructor. Creates an empty container.
    DataContainer() : m_source(TEmptyDataSource::Create()) {}

    //! Constructor. Creates a container using the supplied data source.
    DataContainer(TDataSource const& source) : m_source(&source) {}

    TDataSource const& GetDataSource() const {return *m_source;}

    //! Get an item at the supplied index.
    TItem operator[](size_t index) const {return m_source->Get(index);}
    //! Get an item at the supplied index.
    TItem Get(size_t index) const {return m_source->Get(index);}

    //! Get the total number of items in this container.
    size_t GetSize() const {return m_source->GetSize();}

    //! Get the iterator at the start of this container.
    Iterator begin() const {return m_source->begin();}

    //! Get the iterator at the end of this container.
    Iterator end() const {return m_source->end();}
};

//=======================================================================================
//! A @ref PagingDataSource driven container.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                05/2020
//=======================================================================================
template<typename TItem>
struct PagedDataContainer : DataContainer<TItem, PagingDataSource<TItem>, PagingDataSource<TItem>>
    {
    typedef DataContainer<TItem, PagingDataSource<TItem>, PagingDataSource<TItem>> T_Super;
    PagedDataContainer() : T_Super() {}
    PagedDataContainer(PagingDataSource<TItem> const& source) : T_Super(source) {}
    size_t GetTotalSize() const {return this->m_source->GetTotalSize();}
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
