/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnLinkTable.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnCore/DgnProjectTables.h>

#define DGNECSCHEMA_CLASSNAME_DgnLink               L"Link"
#define DGNECSCHEMA_CLASSNAME_UrlDgnLink            L"UrlLink"
#define DGNECSCHEMA_CLASSNAME_DgnViewLink           L"ViewLink"
#define DGNECSCHEMA_CLASSNAME_ExternalFileDgnLink   L"ExternalFileLink"
#define DGNECSCHEMA_CLASSNAME_EmbeddedFileDgnLink   L"EmbeddedFileLink"
#define DGNECSCHEMA_CLASSNAME_ElementHasLinks       L"ElementHasLinks"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DgnLinks
//! @brief DgnLinks are links on elements to other entities, such as URLs or files. These encompass V8 Design Links and Engineering Links.
//! @{

//=======================================================================================
//! Describes possible DGN link types. Respective strongly-typed classes are in DgnLinkTable.
// @bsiclass                                                    Jeff.Marker     03/2013
//=======================================================================================
enum struct DgnLinkType
{
    Url,            //!< A link that represents a URL (@see UrlDgnLinkEntry).
    ExternalFile,   //!< A link that represents an external file (@see ExternalFileDgnLinkEntry).
    EmbeddedFile,   //!< A link that represents an embedded file (@see EmbeddedFileDgnLinkEntry).
    View,           //!< A link to a DgnView in the project (@see DgnViewLinkEntry)

    Invalid         //!< Indicates an error or unitialized condition.
}; // DgnLinkType

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
//! Allows management of DGN links in a project. You typically get a reference to this class via DgnProject::DgnLinks.
// @bsiclass                                                    Jeff.Marker     03/2013
//=======================================================================================
struct DgnLinkTable : public DgnProjectTable
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(DgnProjectTable)
//__PUBLISH_SECTION_START__

private:
    friend struct DgnProject;

    DGNPLATFORM_EXPORT DgnLinkTable(DgnProjectR);

public:
    //=======================================================================================
    //! Base class for DGN links. This represents a DGN link in memory, and most data access is on-demand (e.g. only evaluated from the database when an accessor is called). This means the objects are cheap to create, but expensive to continually re-evalute. Link objects are created by the Attach* methods on DgnLinkTable (e.g. AttachUrlLink), and read by using *Iterator classes in DgnLinkTable (e.g. OnElementIterator).
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct DgnLinkEntry : public RefCountedBase, public NonCopyableClass
    {
    //__PUBLISH_SECTION_END__
    protected:
        DgnProjectP m_project;
        DgnLinkId m_id;
        DgnLinkType m_type;
        mutable BeSQLite::Statement m_select;

        DgnLinkEntry(DgnProjectR, DgnLinkId, DgnLinkType);
        bool EnsureSelect() const;
        virtual BentleyStatus _PrepareSelect() const = 0;

    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
    public:
        //! The project-unique ID of this link.
        DGNPLATFORM_EXPORT DgnLinkId const& GetId() const;
        
        //! The type of link.
        DGNPLATFORM_EXPORT DgnLinkType const& GetLinkType() const;
        
        //! The element this link is associated to. Links must be associated to a single element.
        DGNPLATFORM_EXPORT ElementId GetElementId() const;
        
        //! The display label for this link; useful for presenting the link to the user in an interface.
        DGNPLATFORM_EXPORT Utf8String GetDisplayLabel() const;
    }; // DgnLinkEntry

    typedef RefCountedPtr<DgnLinkEntry> DgnLinkEntryPtr;

    //=======================================================================================
    //! Represents a URL link in memory. See DgnLinkEntry for broader discussion.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct UrlDgnLinkEntry : public DgnLinkEntry
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(DgnLinkEntry)

    protected:
        virtual BentleyStatus _PrepareSelect() const override;

    public:
        UrlDgnLinkEntry(DgnProjectR, DgnLinkId);
        static ECN::ECClassP GetECClassP(BeSQLite::EC::ECDbR, ECN::ECSchemaP dgnSchema = NULL);

    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
    public:
        //! Creates a new instance from a persistent link in the database.
        DGNPLATFORM_EXPORT static DgnLinkEntryPtr Create(DgnProjectR, DgnLinkId);
        
        //! The URL stored by this link.
        DGNPLATFORM_EXPORT Utf8String GetUrl() const;
    }; // UrlDgnLinkEntry

    //=======================================================================================
    //! Represents a View link in memory. See DgnLinkEntry for broader discussion.
    // @bsiclass                                                    Sam.Wilson      06/2014
    //=======================================================================================
    struct DgnViewLinkEntry : public DgnLinkEntry
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(DgnLinkEntry)

    protected:
        virtual BentleyStatus _PrepareSelect() const override;

    public:
        DgnViewLinkEntry(DgnProjectR, DgnLinkId);
        static ECN::ECClassP GetECClassP(BeSQLite::EC::ECDbR, ECN::ECSchemaP dgnSchema = NULL);

    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
    public:
        //! Creates a new instance from a persistent link in the database.
        DGNPLATFORM_EXPORT static DgnLinkEntryPtr Create(DgnProjectR, DgnLinkId);
        
        //! The DgnViewId stored by this link.
        DGNPLATFORM_EXPORT DgnViewId GetViewId() const;
    }; // UrlDgnLinkEntry

    //=======================================================================================
    //! Represents an external file link in memory. See DgnLinkEntry for broader discussion.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct ExternalFileDgnLinkEntry : public DgnLinkEntry
    {
        //! Map of Utf8String/Utf8String used to store paths. See PATH_STRING_KEY_ constants for common keys.
        typedef bmap<Utf8String, Utf8String> PathStringMap;
        
        //! Key in dictionary returned by GetPathStrings for the DMS path (e.g. pw:\/\/server\/directory\/fileid).
        DGNPLATFORM_EXPORT static Utf8CP PATH_STRING_KEY_DmsPath;

        //! Key in dictionary returned by GetPathStrings for the portable path (e.g. ..\\directory\\file).
        DGNPLATFORM_EXPORT static Utf8CP PATH_STRING_KEY_PortablePath;
        
        //! Key in dictionary returned by GetPathStrings for the full local path (e.g. C:\\directory\\file).
        DGNPLATFORM_EXPORT static Utf8CP PATH_STRING_KEY_FullPath;
    
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(DgnLinkEntry)

    protected:
        virtual BentleyStatus _PrepareSelect() const override;

    public:
        ExternalFileDgnLinkEntry(DgnProjectR, DgnLinkId);
        static ECN::ECClassP GetECClassP(BeSQLite::EC::ECDbR, ECN::ECSchemaP dgnSchema = NULL);

    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
    public:
        //! Creates a new instance from a persistent link in the database.
        DGNPLATFORM_EXPORT static DgnLinkEntryPtr Create(DgnProjectR, DgnLinkId);
        
        //! Gets the path strings stored by this link. See the PATH_STRING_KEY_ constants in this class for common map keys. It is recommanded you query and process them in the order PATH_STRING_KEY_DmsPath, PATH_STRING_KEY_PortablePath, PATH_STRING_KEY_FullPath.
        DGNPLATFORM_EXPORT PathStringMap GetPathStrings() const;
    }; // ExternalFileDgnLinkEntry

    //=======================================================================================
    //! Represents an embedded file link in memory. See DgnLinkEntry for broader discussion.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct EmbeddedFileDgnLinkEntry : public DgnLinkEntry
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(DgnLinkEntry)

    protected:
        virtual BentleyStatus _PrepareSelect() const override;

    public:
        EmbeddedFileDgnLinkEntry(DgnProjectR, DgnLinkId);
        static ECN::ECClassP GetECClassP(BeSQLite::EC::ECDbR, ECN::ECSchemaP dgnSchema = NULL);

    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
    public:
        //! Creates a new instance from a persistent link in the database.
        DGNPLATFORM_EXPORT static DgnLinkEntryPtr Create(DgnProjectR, DgnLinkId);
        
        //! Gets the name of the embedded document stored by this link.
        DGNPLATFORM_EXPORT Utf8String GetDocumentName() const;
    }; // EmbeddedFileDgnLinkEntry
    
    struct OnElementIterator;
    struct InProjectIterator;

    //=======================================================================================
    //! Used by DGN link iterators to allow creation of the derived instances of DgnLinkEntry (as smart pointers) for each specific type of link encountered.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct EntryFactory : BeSQLite::DbTableIterator::Entry, std::iterator<std::input_iterator_tag, EntryFactory const>
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(Entry)

    private:
        friend struct OnElementIterator;
        friend struct InProjectIterator;

    //__PUBLISH_SECTION_START__
    private:
        DgnProjectP m_project;

        EntryFactory(DgnProjectR, BeSQLiteStatementP, bool isValid);

    public:
        //! Quick access to the type of link this factory will create.
        DGNPLATFORM_EXPORT DgnLinkType GetLinkType() const;
        
        //! Creates a new in-memory representation of the link this factory represents.
        DGNPLATFORM_EXPORT DgnLinkEntryPtr CreateEntry() const;
        
        //! Allows for easy de-reference of this type.
        DGNPLATFORM_EXPORT EntryFactory const& operator*() const;
    }; // EntryFactory
        
    //=======================================================================================
    //! Use this to iterate the links on an element. Links on an element are created in a specific order, which is reflected by this iterator.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct OnElementIterator : public BeSQLite::DbTableIterator
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(BeSQLite::DbTableIterator)

    //__PUBLISH_SECTION_START__
    private:
        DgnProjectP m_project;
        ElementId m_elementID;

    public:
        //! Creates a new instance to iterate the DGN links, in order, on the specified element in the specified project.
        DGNPLATFORM_EXPORT OnElementIterator(DgnProjectR, ElementId);

        //! Supports range-based for loops.
        typedef EntryFactory const_iterator;

        //! Represents the beginning of the element's DGN link collection. This will be equivalent to end() if the element has no DGN links.
        DGNPLATFORM_EXPORT EntryFactory begin() const;
        
        //! Represents the end (one past the last valid) of the element's DGN link collection.
        DGNPLATFORM_EXPORT EntryFactory end() const;
        
        //! Directly queries the number of DGN links on the element without actually iterating.
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // OnElementIterator

    //=======================================================================================
    //! Use this to iterate the links on a project. The order that the links are iterated should not be considered undefined.
    // @bsiclass                                                    Jeff.Marker     03/2013
    //=======================================================================================
    struct InProjectIterator : public BeSQLite::DbTableIterator
    {
    //__PUBLISH_SECTION_END__
        DEFINE_T_SUPER(BeSQLite::DbTableIterator)

    //__PUBLISH_SECTION_START__
    private:
        DgnProjectP m_project;

    public:
        //! Creates a new instance to iterate the DGN links in the specified project.
        DGNPLATFORM_EXPORT InProjectIterator(DgnProjectR);
        
        //! Supports range-based for loops.
        typedef EntryFactory const_iterator;

        //! Represents the beginning of the project's DGN link collection. This will be equivalent to end() if the element has no DGN links.
        DGNPLATFORM_EXPORT EntryFactory begin() const;
        
        //! Represents the end (one past the last valid) of the project's DGN link collection.
        DGNPLATFORM_EXPORT EntryFactory end() const;
        
        //! Directly queries the number of DGN links in the project without actually iterating.
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // InProjectIterator

    //! Creates and persists a new DgnView link.
    //! @param elementID    ID of the element this link will be attached to. The element must already exist in the project.
    //! @param displayLabel Friendly name of the link; typically shown to the user in lists and pickers.
    //! @param viewId       The DgnViewId to store in this link.
    //! @param insertBefore (Optional) Order of links on an element is preserved when iterating; if this parameter is provided, the new link is inserted before the indicated link.
    //! @return An in-memory representation of the newly created link in the project.
    DGNPLATFORM_EXPORT DgnLinkEntryPtr AttachDgnViewLink(ElementId elementID, Utf8CP displayLabel, DgnViewId viewId, DgnLinkId const* insertBefore);

    //! Creates and persists a new URL DGN link.
    //! @param elementID    ID of the element this link will be attached to. The element must already exist in the project.
    //! @param displayLabel Friendly name of the link; typically shown to the user in lists and pickers.
    //! @param url          The URL of this link.
    //! @param insertBefore (Optional) Order of links on an element is preserved when iterating; if this parameter is provided, the new link is inserted before the indicated link.
    //! @return An in-memory representation of the newly created link in the project.
    DGNPLATFORM_EXPORT DgnLinkEntryPtr AttachUrlLink(ElementId elementID, Utf8CP displayLabel, Utf8CP url, DgnLinkId const* insertBefore);
    
    //! Creates and persists a new external file DGN link.
    //! @param elementID        ID of the element this link will be attached to. The element must already exist in the project.
    //! @param displayLabel     Friendly name of the link; typically shown to the user in lists and pickers.
    //! @param pathStringsMap   Dictionary of path strings, typically from the original V8 moniker. For common keys, see the ExternalFileDgnLinkEntry::PATH_STRING_KEY_* constants.
    //! @param insertBefore     (Optional) Order of links on an element is preserved when iterating; if this parameter is provided, the new link is inserted before the indicated link.
    //! @return An in-memory representation of the newly created link in the project.
    DGNPLATFORM_EXPORT DgnLinkEntryPtr AttachExternalFileLink(ElementId elementID, Utf8CP displayLabel, ExternalFileDgnLinkEntry::PathStringMap const& pathStringsMap, DgnLinkId const* insertBefore);
    
    //! Creates and persists a new embedded file DGN link.
    //! @param elementID    ID of the element this link will be attached to. The element must already exist in the project.
    //! @param displayLabel Friendly name of the link; typically shown to the user in lists and pickers.
    //! @param documentName The name of the embedded document this link refers to (@see DbEmbeddedFileTable).
    //! @param insertBefore (Optional) Order of links on an element is preserved when iterating; if this parameter is provided, the new link is inserted before the indicated link.
    //! @return An in-memory representation of the newly created link in the project.
    DGNPLATFORM_EXPORT DgnLinkEntryPtr AttachEmbeddedFileLink(ElementId elementID, Utf8CP displayLabel, Utf8CP documentName, DgnLinkId const* insertBefore);
    
    //! Deletes the indicated link from this project.
    DGNPLATFORM_EXPORT BentleyStatus DeleteLink(DgnLinkId);
}; // DgnLinkTable
#endif

//! @}

END_BENTLEY_DGNPLATFORM_NAMESPACE
