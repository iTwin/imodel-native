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
    size_t GetSize() const {return _GetSize();}
    Iterator begin() const {return _CreateFrontIterator();}
    Iterator end() const {return _CreateBackIterator();}
};

template<typename T>
using IDataSourcePtr = RefCountedPtr<IDataSource<T>>;

template<typename T>
using IDataSourceCPtr = RefCountedPtr<IDataSource<T> const>;

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
//! An @ref IDataSource driven container.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2015
//=======================================================================================
template<typename T>
struct DataContainer
{
typedef typename IDataSource<T>::Iterator Iterator;

private:
    IDataSourceCPtr<T> m_source;

public:
    //! Constructor. Creates an empty container.
    DataContainer() : m_source(EmptyDataSource<T>::Create()) {}

    //! Constructor. Creates a container using the supplied data source.
    DataContainer(IDataSource<T> const& source) : m_source(&source) {}

    //! Get an item at the supplied index.
    T operator[](size_t index) const {return m_source->Get(index);}
    //! Get an item at the supplied index.
    T Get(size_t index) const {return m_source->Get(index);}

    //! Get the total number of items in this container.
    size_t GetSize() const {return m_source->GetSize();}

    //! Clear this container.
    //! @note The backing data source gets destroyed
    void Clear() {m_source = EmptyDataSource<T>::Create();}

    //! Get the iterator at the start of this container.
    Iterator begin() const {return m_source->begin();}

    //! Get the iterator at the end of this container.
    Iterator end() const {return m_source->end();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
