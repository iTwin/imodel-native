/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include "DTMHelpers.h"
#include "DTM.h"
BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE


struct DTMMeshEnumeratorImpl
    {
    Bentley::TerrainModel::DTMMeshEnumerator&          m_collection;
    Bentley::TerrainModel::DTMMeshEnumerator::iterator m_current;

    DTMMeshEnumeratorImpl (Bentley::TerrainModel::DTMMeshEnumerator& collection) : m_collection (collection), m_current (m_collection.begin ()) {}
    };

public ref class DTMMeshEnumerator : System::Collections::Generic::IEnumerable <BGEO::PolyfaceHeader^>
    {
    ref class Enumerator : System::Collections::Generic::IEnumerator<BGEO::PolyfaceHeader^>
        {
        DTMMeshEnumeratorImpl* m_impl;
        ReleaseMarshaller* m_marshaller;
        Bentley::TerrainModel::DTMMeshEnumerator& m_native;
        internal:
            Enumerator(Bentley::TerrainModel::DTMMeshEnumerator& native, ReleaseMarshaller* marshaller) : m_native(native), m_marshaller(marshaller)
                {
                m_native.AddRef ();
                m_impl = nullptr;
                }
            ~Enumerator()
                {
                if (m_impl)
                    {
                    delete m_impl;
                    m_impl = nullptr;
                    }
                m_native.Release();
                }
            !Enumerator()
                {
                if (m_impl)
                    {
                    delete m_impl;
                    m_impl = nullptr;
                    }

                m_marshaller->QueueEntry(&m_native);
                }
        public:
            virtual bool MoveNext ()
                {
                if (m_impl == nullptr)
                    {
                    m_impl = new DTMMeshEnumeratorImpl (m_native);
                    return m_impl->m_current != m_impl->m_collection.end ();
                    }
                Bentley::TerrainModel::DTMMeshEnumerator::iterator endIterator = m_impl->m_collection.end ();
                if (!(m_impl->m_current != endIterator))
                    return false;

                ++m_impl->m_current;
                return m_impl->m_current != endIterator;
                }
        property BGEO::PolyfaceHeader^ Current
            {
            virtual BGEO::PolyfaceHeader^ get();
            }
        virtual void Reset ()
            {
            if (nullptr != m_impl)
                m_impl->m_current = m_impl->m_collection.begin();
            }

        private:
        property System::Object^ CurrentI
            {
            virtual System::Object^ get () sealed = System::Collections::IEnumerator::Current::get
                {
                return Current;
                }
            }
        };

    private:
        DTM^ m_dtm;
        ReleaseMarshaller* m_marshaller;
        Bentley::TerrainModel::DTMMeshEnumerator* m_native;

    public:
        DTMMeshEnumerator (DTM^ dtm);
        ~DTMMeshEnumerator()
            {
            m_native->Release();
            m_native = nullptr;
            }
        !DTMMeshEnumerator()
            {
            m_marshaller->QueueEntry(m_native);
            }

        property int MaxTriangles
            {
            void set(int);
            }

        virtual System::Collections::Generic::IEnumerator<BGEO::PolyfaceHeader^>^ GetEnumerator ()
            {
            return gcnew Enumerator (*m_native, m_marshaller);
            }

    private:
        virtual System::Collections::IEnumerator^ GetEnumeratorI () sealed = System::Collections::IEnumerable::GetEnumerator
            {
            return GetEnumerator ();
            }
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE