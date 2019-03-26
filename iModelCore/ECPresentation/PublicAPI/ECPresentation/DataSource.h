/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/DataSource.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! An interface for an abstract data source.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2015
//=======================================================================================
template<typename T>
struct IDataSource : RefCountedBase
{
protected:
    //! Virtual destructor.
    virtual ~IDataSource() {}
    //! @see Get
    virtual T _Get(size_t index) const = 0;
    //! @see GetSize
    virtual size_t _GetSize() const = 0;

public:
    //! Get the item at the supplied index.
    T Get(size_t index) const {return _Get(index);}

    //! Get the total number of items in this data source.
    size_t GetSize() const {return _GetSize();}
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
protected:
    EmptyDataSource() {}
    T _Get(size_t index) const override {return T();}
    size_t _GetSize() const override {return 0;}
public:
    //! Creates an empty data source.
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
    //=======================================================================================
    //! An iterator to iterate over the items in the container.
    // @bsiclass                                    Grigas.Petraitis                09/2015
    //=======================================================================================
    struct Iterator
    {
    friend struct DataContainer<T>;
    private:
        size_t m_index;
        DataContainer const& m_container;
        Iterator(DataContainer const& container, size_t index) : m_container(container), m_index(index) {}
    public:
        bool operator!=(Iterator const& other) {return m_index != other.m_index;}
        Iterator const& operator++() {m_index++; return *this;}
        T operator*() const {return m_container.Get(m_index);}
    };

private:
    RefCountedPtr<IDataSource<T> const> m_source;
    
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
    Iterator begin() const {return Iterator(*this, 0);}
    //! Get the iterator at the end of this container.
    Iterator end() const {return Iterator(*this, GetSize());}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
