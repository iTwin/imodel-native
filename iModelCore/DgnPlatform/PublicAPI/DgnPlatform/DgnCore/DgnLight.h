/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnLight.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! The DgnLights holds the lights defined for a DgnDb. Each light has a unique name, an
//! options description, and a value encoded in JSON.
//! @see DgnDb::Lights
//=======================================================================================
struct DgnLights : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnLights(DgnDbR db) : DgnDbTable(db) { }

public:
    struct Light
    {
    private:
        friend struct DgnLights;

        DgnLightId  m_id;
        Utf8String  m_name;
        Utf8String  m_descr;
        Utf8String  m_value;

    public:
        //! Constructs an empty, invalid Light
        Light() { }
        //! Construct a Light for insertion into the lights table.
        //! @param[in]      name  The light's name. Must be unique within the DgnDb.
        //! @param[in]      value The JSON representation of the light.
        //! @param[in]      descr The light's optional description
        Light(Utf8CP name, Utf8CP value, Utf8CP descr=nullptr) : m_name(name), m_descr(descr), m_value(value) { }

        DgnLightId GetId() const {return m_id;} //!< The ID of this light.
        Utf8StringCR GetName() const {return m_name;} //!< The name of this light.
        Utf8StringCR GetValue() const {return m_value;} //!< The JSON representation of this light.
        Utf8StringCR GetDescr() const {return m_descr;} //!< The description of this light.
        void SetValue(Utf8StringCR val) {m_value = val;} //!< Sets the JSON representation of this light.
        void SetDescr(Utf8StringCR val) {m_descr = val;} //!< Sets the description of this light.
        bool IsValid() const {return m_id.IsValid();} //!< Test if the light is valid.
    };

    //! An iterator over the lights in a DgnDb
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) { }

        //! An entry in the light table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) { }
        public:
            DGNPLATFORM_EXPORT DgnLightId GetId() const; //!< The light ID.
            DGNPLATFORM_EXPORT Utf8CP GetName() const; //!< The light name.
            DGNPLATFORM_EXPORT Utf8CP GetValue() const; //!< The JSON representation of the light.
            DGNPLATFORM_EXPORT Utf8CP GetDescr() const; //!< The light description.
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT size_t QueryCount() const; //!< The number of entries in the light table.
        DGNPLATFORM_EXPORT Entry begin() const; //!< An iterator to the first entry in the table.
        Entry end() const {return Entry(nullptr, false);} //!< An iterator one beyond the last entry in the table.
    };

    //! Obtain an iterator over the lights in a DgnDb
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Insert a new light into the DgnDb. The name of the light must be unique within the DgnDb.
    //! @param[in]      light       The new light
    //! @param[out]     result      If supplied, holds the result of the insert operation
    //! @return The DgnLightId of the newly created light, or an invalid ID if the light was not created.
    DGNPLATFORM_EXPORT DgnLightId Insert(Light& light, DgnDbStatus* result=nullptr);

    //! Change the properties of the specified light. This method cannot be used to change the light name.
    //! @param[in]      light The modified light.
    //! @return Success if the light was updated, or else an error code.
    DGNPLATFORM_EXPORT DgnDbStatus Update(Light const& light) const;

    //! Look up a light by ID.
    //! @param[in]      id The ID of the desired light.
    //! @return The light with the specified ID, or an invalid light if no such ID exists.
    DGNPLATFORM_EXPORT Light Query(DgnLightId id) const;

    //! Look up the ID of the light with the specified name.
    //! @param[in]      name The name of the desired light.
    //! @return The ID of the specified light, or an invalid ID if no such light exists.
    DGNPLATFORM_EXPORT DgnLightId QueryLightId(Utf8StringCR name) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

