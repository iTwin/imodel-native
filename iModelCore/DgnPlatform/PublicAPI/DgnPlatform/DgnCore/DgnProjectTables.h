/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnProjectTables.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define DGNECSCHEMA_SchemaName          "dgn"
#define DGNPREFIX DGNECSCHEMA_SchemaName "_"

#define DGN_TABLE_Color           DGNPREFIX "Color"
#define DGN_TABLE_Domain          DGNPREFIX "Domain"
#define DGN_TABLE_Element         DGNPREFIX "Element"
#define DGN_TABLE_ElmXAtt         DGNPREFIX "ElmXAtt"
#define DGN_TABLE_Font            DGNPREFIX "Font"
#define DGN_TABLE_GeneratedModel  DGNPREFIX "GeneratedModel"
#define DGN_TABLE_Handler         DGNPREFIX "Handler"
#define DGN_TABLE_Item            DGNPREFIX "Item"
#define DGN_TABLE_KeyStr          DGNPREFIX "KeyStr"
#define DGN_TABLE_Level           DGNPREFIX "Level"
#define DGN_TABLE_SubLevel        DGNPREFIX "SubLevel"
#define DGN_TABLE_Link            DGNPREFIX "Link"
#define DGN_TABLE_Material        DGNPREFIX "Material"
#define DGN_TABLE_Model           DGNPREFIX "Model"
#define DGN_TABLE_ModelSelector   DGNPREFIX "ModelSel"
#define DGN_TABLE_ProvElem        DGNPREFIX "ProvElem"
#define DGN_TABLE_ProvFile        DGNPREFIX "ProvFile"
#define DGN_TABLE_ProvModel       DGNPREFIX "ProvModel"
#define DGN_TABLE_RasterData      DGNPREFIX "RasterData"
#define DGN_TABLE_RasterFile      DGNPREFIX "RasterFile"
#define DGN_TABLE_Session         DGNPREFIX "Session"
#define DGN_TABLE_SetEntry        DGNPREFIX "SetEntry"
#define DGN_TABLE_Stamp           DGNPREFIX "Stamp"
#define DGN_TABLE_Style           DGNPREFIX "Style"
#define DGN_TABLE_Tag             DGNPREFIX "Tag"
#define DGN_TABLE_View            DGNPREFIX "View"
#define DGN_VTABLE_PrjRTree       DGNPREFIX "PrjRTree"

#define DGNECSCHEMA_CLASSNAME_Element   L"Element"

#include "DgnCore.h"
#include "DgnColorMap.h"
#include <DgnPlatform/Tools/BitMask.h>
#include <DgnPlatform/DgnProperties.h>
#include "DgnFontManager.h"
#include "UnitDefinition.h"

//__PUBLISH_SECTION_END__
#include <RmgrTools/Tools/HeapZone.h>
#define DGN_SETTYPE_ModelSelector   DGNPREFIX "ModSel"

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! Map of ID to DgnFont object. See method FontNumberMap.
typedef bmap<UInt32, DgnFontCP> T_FontNumberMap;

//=======================================================================================
//! A base class for api's that access information in a table in a DgnProject
//=======================================================================================
struct DgnProjectTable
{
protected:
    friend struct DgnProject;
    DgnProjectR m_project;
    explicit DgnProjectTable(DgnProjectR project) : m_project(project) {}

public:
    DgnProjectR GetDgnProject() {return m_project;}

    //! Determine whether the supplied name contains any of the invalid characters.
    //! @param[in] name the name to check
    //! @param[in] invalidChars the list of character that may NOT appear in name.
    //! @note names may also not start or end with a space.
    DGNPLATFORM_EXPORT static bool IsValidName(Utf8StringCR name, Utf8CP invalidChars);

    //! Replace any instances of the invalid characters in the supplied name with a replacement character.
    //! @param[in] name the name to validate
    //! @param[in] invalidChars the list of invalid characters. All instances of these characters are replaced.
    //! @param[in] replacement the character to substitute for any invalid characters in name.
    DGNPLATFORM_EXPORT static void ReplaceInvalidCharacters(Utf8StringR name, Utf8CP invalidChars, Utf8Char replacement);
};

//=======================================================================================
//! Each Level has an entry in the Level table.
//=======================================================================================
struct DgnLevels : DgnProjectTable, NonCopyableClass
    {
    //! The Rank of a level indicates who created it and who may use it for elements
    enum class Rank
    {
        User        = 0,    //!< This level was created by the user and may contain user created elements
        System      = 1,    //!< This level is predefined by the system and may only contain system-type elements
        Application = 2,    //!< This level was crated by an application and may only be contain elements from that application
    };

    //! The Scope of a level determines what types of models may use it.
    enum class Scope
    {
        Any         = 0,    //!< This level may be used in any type of model. Generally, this means it came from some external source (e.g. a dgn or dwg file)
        Physical    = 1,    //!< This level may only be used in physical models
        Annotation  = 2,    //!< This level may only be used in annotation models (e.g. sheets or drawings)
        Analytical  = 3,    //!< This level may only be used in analytical models
    };

    //! A sub-level of a level.
    struct SubLevel
    {
        //! The parameters that can determine how graphics on a SubLevel appear when drawn.
        struct Appearance
        {
        private:
            bool        m_invisible;    //!< Graphics on this SubLevel should not be visible
            bool        m_dontPlot;     //!< Graphics on this SubLevel should not be plotted
            bool        m_dontSnap;     //!< Graphics on this SubLevel should not be snappable
            bool        m_dontLocate;   //!< Graphics on this SubLevel should not be locatable
            UInt32      m_color;
            UInt32      m_weight;
            Int32       m_style;
            Int32       m_displayPriority; // only valid for SubLevels of 2d levels
            DgnMaterialId m_material;
            double      m_transparency;

        public:
            void Init() {memset(this, 0, sizeof(*this)); m_material.Invalidate();}
            Appearance() {Init();}
            explicit Appearance(Utf8StringCR val) {FromJson(val);}

            void SetInvisible(bool val) {m_invisible=val;}
            void SetDontPlot(bool val) {m_dontPlot=val;}
            bool GetDontSnap() const {return m_dontSnap;}
            void SetDontSnap(bool val) {m_dontSnap=val;}
            bool GetDontLocate() const {return m_dontLocate;}
            void SetDontLocate(bool val) {m_dontLocate=val;}
            void SetColor(UInt32 val) {m_color=val;}
            void SetWeight(UInt32 val) {m_weight=val;}
            void SetStyle(UInt32 val) {m_style=val;}
            void SetDisplayPriority(Int32 val) {m_displayPriority=val;}
            void SetMaterial(DgnMaterialId val) {m_material=val;}
            void SetTransparency(double val) {m_transparency=val;}
            bool IsInvisible() const {return m_invisible;}
            bool IsVisible() const {return !m_invisible;}
            UInt32 GetColor() const {return m_color;}
            UInt32 GetWeight() const {return m_weight;}
            UInt32 GetStyle() const {return m_style;}
            Int32 GetDisplayPriority() const {return m_displayPriority;}
            DgnMaterialId GetMaterial() const {return m_material;}
            double GetTransparency() const {return m_transparency;}
            DGNPLATFORM_EXPORT bool operator==(Appearance const& other) const;
            bool IsEqual(Appearance const& other) const {return *this==other;}
            void FromJson(Utf8StringCR); //!< initialize this appearance from a previously saved json string
            Utf8String ToJson() const;   //!< convert this appearance to a json string
        };// Appearance

        //! View-specific overrides of the appearance of a SubLevel
        struct Override
        {
        private:
            union
                {
                UInt32 m_int32;
                struct
                    {
                    UInt32 m_invisible:1;
                    UInt32 m_color:1;
                    UInt32 m_weight:1;
                    UInt32 m_style:1;
                    UInt32 m_material:1;
                    UInt32 m_priority:1;
                    UInt32 m_transparency:1;
                    };
                } m_flags;

            Appearance m_value;

        public:
            Override() {Init();}
            explicit Override(JsonValueCR val) {Init(); FromJson(val);}
            void Init() {m_flags.m_int32 = 0; m_value.Init();}
            void SetInvisible(bool val) {m_flags.m_invisible=true; m_value.SetInvisible(val);}
            void SetColor(UInt32 val) {m_flags.m_color=true; m_value.SetColor(val);}
            void SetWeight(UInt32 val) {m_flags.m_weight=true; m_value.SetWeight(val);}
            void SetStyle(UInt32 val) {m_flags.m_style=true; m_value.SetStyle(val);}
            void SetDisplayPriority(Int32 val) {m_flags.m_priority=true; m_value.SetDisplayPriority(val);}
            void SetMaterial(DgnMaterialId val) {m_flags.m_material=true; m_value.SetMaterial(val);}
            void SetTransparency(double val) {m_flags.m_transparency=true; m_value.SetTransparency(val);}
            void ToJson(JsonValueR outValue) const;
            void FromJson (JsonValueCR inValue);
            void ApplyTo(Appearance&) const;
        }; // Override

    private:
        friend struct DgnLevels;
        SubLevelId m_id;
        Utf8String m_name;
        Utf8String m_description;
        Appearance m_appearance;

    public:
        SubLevel() {} //!< Construct an invalid SubLevel.
        SubLevel(SubLevelId id, Utf8CP name, Appearance const& appearance, Utf8CP descr=NULL)
                : m_id(id), m_name(name), m_appearance(appearance) {m_description.AssignOrClear(descr);}

        Utf8CP GetName() const {return m_name.c_str();} //!< The SubLevel name. Unique per level. Default SubLevels will return an empty string.
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : NULL;} //!< The SubLevel description. May be empty.
        SubLevelId GetId() const {return m_id;} //!< The LevelSubLevel id.
        Appearance const& GetAppearance() const {return m_appearance;} //!< The Appearance for this SubLevel.
        Appearance& GetAppearanceR() {return m_appearance;} //!< Get a writeable reference to the Appearance for this SubLevel.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the level description. @param val the new description. May be NULL.
        void SetName(Utf8CP val) {m_name = val;} //!< Set the SubLevel name. @param val the new name for this SubLevel. Must not be NULL. Must be unique per level. Default SubLevels may not be renamed.
        bool IsValid() const {return m_id.IsValid();} //!< Test if the SubLevel is valid. A failed query will return an invalid SubLevel.
        bool IsDefaultSubLevel() const {return DgnSubLevelId(0) == m_id.GetSubLevel();} //!< Determine if this is the default SubLevel for its level.
    }; // SubLevel

    //! A Level in the level table
    struct Level
        {
    private:
        friend struct DgnLevels;
        LevelId     m_levelId;
        Rank        m_rank;
        Scope       m_scope;
        Utf8String  m_name;
        Utf8String  m_description;

        void Init (LevelId id, Utf8CP name, Scope scope, Utf8CP descr, Rank rank)
            {m_levelId=id; m_name=name; m_scope=scope; m_description.AssignOrClear(descr); m_rank=rank;}

    public:
        Level() {}// so that we can put Levels in a bmap

        //! Ctor for a Level in the level table.
        //! @param[in] id The level's Id. Must be unique.
        //! @param[in] name The level's name. Must be unique.
        //! @param[in] scope The Scope of this level.
        //! @param[in] descr The level's description. May be NULL.
        //! @param[in] rank The level's rank
        Level (LevelId id, Utf8CP name, Scope scope, Utf8CP descr=NULL, Rank rank=Rank::User) {Init(id, name, scope, descr, rank);}

        //! Copy data for a level
        Level (LevelId levelId, Utf8CP name, Level const& row) {Init (levelId, name, row.GetScope(), row.GetDescription(), row.GetRank());}

        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : NULL;} //!< The level description. May be empty.
        Utf8CP GetName() const {return m_name.c_str();} //!< The level name. Never empty. Unique.
        LevelId GetLevelId() const {return m_levelId;} //!< The level id. Unique. This is an internal identifier and is not displayed in the GUI.
        Scope GetScope() const {return m_scope;} //!< the level's scope.
        Rank GetRank () const {return m_rank;} //!< the level's rank.
        bool IsSystemLevel() const {return GetRank()==Rank::System;}
        bool IsUserLevel() const {return GetRank()==Rank::User;}
        bool IsValid() const {return m_levelId.IsValid();} //!< Test if the Level is valid. A failed query will return an invalid Level. @see DgnLevels::QueryLevelById.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the level description. @param val the new description. May be NULL.
        void SetName(Utf8CP val) {m_name = val;} //!< Set the level name. @param val the new name. Must not be NULL. Must be unique.
        void SetScope(Scope val) {m_scope= val;} //!< Set the level's scope. @param[in] val the new level scope.
        void SetRank(Rank val) {m_rank=val;} //!< Change the Rank of this level.
        }; // Level

    //! An iterator over the levels in the DgnProject.
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {

        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT LevelId GetLevelId() const; //!< The level id. Unique. This is an internal identifier and is not displayed in the GUI.
            DGNPLATFORM_EXPORT Rank GetRank() const; //!< The level's rank
            DGNPLATFORM_EXPORT Utf8CP GetName() const; //!< The level's name. Never NULL. Unique.
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const; //!< The level's description, if any. May be NULL.
            DGNPLATFORM_EXPORT Scope GetScope() const; //!< The level's type.
            Entry const& operator*() const {return *this;}
            Level ToLevel() const {return Level(GetLevelId(), GetName(), GetScope(), GetDescription(), GetRank());}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (m_stmt.get(), false);}
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // Iterator

    //! An iterator over the SubLevels of a level
    struct SubLevelIterator : BeSQLite::DbTableIterator
    {
    private:
        LevelId m_levelId;
    public:

        //! construct a SubLevelIterator
        //! @param[in] project The project for the SubLevel table
        //! @param[in] level Limit the iterator to SubLevels of this level. If invalid, iterate all SubLevels of all levels.
        //! @param[in] whereClause an optional where clause
        SubLevelIterator (DgnProjectCR project, LevelId level) : DbTableIterator ((BeSQLiteDbCR)project), m_levelId(level) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct SubLevelIterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            SubLevelId GetId() const {return SubLevelId(GetLevelId(), GetSubLevelId());}
            DGNPLATFORM_EXPORT LevelId GetLevelId() const; //! The Level id.
            DGNPLATFORM_EXPORT DgnSubLevelId GetSubLevelId() const; //!< The SubLevel id.
            DGNPLATFORM_EXPORT SubLevel::Appearance GetAppearance() const; //!< The Appearance for this SubLevel
            DGNPLATFORM_EXPORT Utf8CP GetName() const; //!< The SubLevel's name. Never NULL. Unique.
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const; //!< The SubLevel's description, if any. May be NULL.
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
    }; // SubLevelIterator

//__PUBLISH_SECTION_END__
private:
    friend struct DgnProject;
    explicit DgnLevels(DgnProjectR project) : DgnProjectTable(project) {}

public:
//__PUBLISH_SECTION_START__
private:
    DgnLevels(); // internal only
    DgnLevels(DgnLevels const&); // no copying

public:
    ///@name Querying and manipulating levels
    //@{
    //! Add a new level to the DgnProject.
    //! @param[in] row The definition of the level to create.
    //! @param[in] the appearance for the default SubLevel for the new level
    //! @return BE_SQLITE_OK if the level was added; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified LevelId and/or name is already used.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertLevel (Level const& row, SubLevel::Appearance const&);

    //! Remove a level from the DgnProject.
    //! @param[in] id the id of the level to remove.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the levelId did not exist prior to this call.
    //! @note Deleting a level can result in an inconsistent database. There is no checking that the level to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the level is no longer necessary (for example, on a blank project). Otherwise, avoid using this method.
    //! @note it is illegal to delete the default level. Any attempt to do so will fail.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteLevel(LevelId id);

    //! Change properties of a level.
    //! @param[in] row The new level data to apply.
    //! @return BE_SQLITE_OK if the update was applied; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified name is used
    //! by another level in the table.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateLevel (Level const& row);

    //! Get the Id of a Level from its name.
    //! @param[in] levelName The name of the level of interest.
    //! @return the Id of the level. Will be invalid if no level by the specified name exists in the DgnProject.
    DGNPLATFORM_EXPORT LevelId QueryLevelId (Utf8CP levelName) const;

    //! Get the information about a level from its Id.
    //! @param[in] id The Id of the level of interest.
    //! @return The data for the level. Call IsValid() on the result to determine whether this method was successful.
    DGNPLATFORM_EXPORT Level QueryLevelById (LevelId id) const;

    //! Look up a level by name.
    //! @param[in] levelName the level name to look up
    //! @return The data for the level. Call IsValid() on the result to determine whether this method was successful.
    Level QueryLevelByName (Utf8CP levelName) const {return QueryLevelById(QueryLevelId(levelName));}

    //! Get an iterator over the levels in this DgnProject.
    Iterator MakeIterator () const {return Iterator (m_project);}

    //! Query the highest LevelId used in this table
    DGNPLATFORM_EXPORT LevelId QueryHighestId() const;
    //@}

    ///@name Querying and manipulating SubLevels of a level
    //@{
    //! Add a new SubLevel to a level.
    //! @param[in] subLevel The definition of the SubLevel.
    //! @return BE_SQLITE_OK if the SubLevel was added; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified LevelId does not exist or the SubLevel's name
    //! or SubLevelId is already in use.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertSubLevel (SubLevel& subLevel);

    //! Remove a SubLevel from a level.
    //! @param[in] subLevelId the id of the SubLevel to be deleted.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the subLevelId did not exist prior to this call.
    //! @note Deleting a SubLevel from a level can result in an inconsistent database. There is no checking that the SubLevel to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the level is no longer necessary (for example, on a blank project). Otherwise, avoid using this method.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteSubLevel (SubLevelId subLevelId);

    //! Change properties of a SubLevel.
    //! @param[in] subLevel The new SubLevel data to apply.
    //! @return BE_SQLITE_OK if the update was applied; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified name is used
    //! by another SubLevel for the level.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateSubLevel (SubLevel const& subLevel);

    //! Get the Id of a SubLevel from its name.
    //! @param[in] levelId the level for which the name applies.
    //! @param[in] subLevelName The name of the SubLevel of interest.
    //! @return the Id of the SubLevel. Will be invalid if no SubLevel on that level by the specified name exists
    DGNPLATFORM_EXPORT DgnSubLevelId QuerySubLevelId (LevelId levelId, Utf8CP subLevelName) const;

    //! Get the information about a SubLevel from its LevelId and Id.
    //! @param[in] subLevelId The Id of the SubLevel of interest.
    //! @return The data for the SubLevel. Call IsValid() on the result to determine whether this method was successful.
    DGNPLATFORM_EXPORT SubLevel QuerySubLevelById (SubLevelId subLevelId) const;

    //! Look up a SubLevel by name.
    //! @param[in] levelId the level for which the name applies.
    //! @param[in] subLevelName the SubLevel name to look up
    //! @return The data for the SubLevel. Call IsValid() on the result to determine whether this method was successful.
    SubLevel QuerySubLevelByName (LevelId levelId, Utf8CP subLevelName) const {return QuerySubLevelById (SubLevelId(levelId, QuerySubLevelId(levelId, subLevelName)));}

    //! Get an iterator over the SubLevels of a level or all SubLevels of all levels.
    //! @param[in] levelId Limit the iterator to SubLevels of this level. If invalid, iterate all SubLevels of all levels.
    //! @param[in] whereClause an optional where clause
    SubLevelIterator MakeSubLevelIterator (LevelId levelId=LevelId()) const {return SubLevelIterator (m_project, levelId);}

    //! Query the highest LevelId used in this table
    DGNPLATFORM_EXPORT DgnSubLevelId QueryHighestSubLevelId(LevelId levelId) const;
    //@}

    //! Get a string containing the list of characters that may NOT appear in level names.
    static Utf8CP GetIllegalCharacters() {return "<>\\/.\"?*|,='&\n\t";}

    //! Determine whether the supplied name is a valid level name.
    //! @param[in] name The candidate level name to check
    //! @return true if the level name is valid, false otherwise.
    //! @note Level names may also not start or end with a space.
    static bool IsValidName(Utf8StringCR name) {return DgnProjectTable::IsValidName(name, GetIllegalCharacters());}
};

//=======================================================================================
//! The types of views that can exist in a DgnProject.
//=======================================================================================
enum DgnViewType
{
    DGNVIEW_TYPE_None     = 0,           //!< do not return any views
    DGNVIEW_TYPE_Physical = 1<<0,        //!< a view of physical (3d) project models
    DGNVIEW_TYPE_Drawing  = 1<<1,        //!< a view of a single drawing (2d) model
    DGNVIEW_TYPE_Sheet    = 1<<2,        //!< a view of a sheet.
    DGNVIEW_TYPE_Panorama = 1<<3,        //!< a view of a panorama
    DGNVIEW_TYPE_Component = 1<<4,       //!< a view of a single component model
    DGNVIEW_TYPE_All      = (DGNVIEW_TYPE_Physical | DGNVIEW_TYPE_Drawing | DGNVIEW_TYPE_Sheet | DGNVIEW_TYPE_Panorama | DGNVIEW_TYPE_Component),
};

//=======================================================================================
//! The source for the creation a DgnView.
//=======================================================================================
enum DgnViewSource
{
    DGNVIEW_SOURCE_User      = 1<<0,      //!< created by a user
    DGNVIEW_SOURCE_Generated = 1<<1,      //!< automatically generated by a program, may be relevant to user
    DGNVIEW_SOURCE_Private   = 1<<2,      //!< used internally and should not be presented to user
};

//=======================================================================================
//! Each View has an entry in the View table.
//=======================================================================================
struct DgnViews : DgnProjectTable
{
private:
    friend struct DgnProject;
    explicit DgnViews(DgnProjectR project) : DgnProjectTable(project){}

public:
    //! An object that holds the values for a View in the DgnViews.
    struct View
    {
    private:
        friend struct DgnViews;
        DgnViewId          m_viewId;
        DgnViewType        m_viewType;
        DgnViewSource      m_viewSource;
        DgnModelId         m_baseModelId;
        DgnModelSelectorId m_selectorId;
        Utf8String         m_name;
        Utf8String         m_description;
        Utf8String         m_viewSubType;

    public:
        View() {m_viewType=DGNVIEW_TYPE_Physical;}
        View(DgnViewType viewType, Utf8CP viewSubType, DgnModelId baseId, Utf8CP name, Utf8CP descr=0, DgnModelSelectorId selector=DgnModelSelectorId(), DgnViewSource source=DGNVIEW_SOURCE_User,DgnViewId id=DgnViewId()) : m_viewId(id), m_baseModelId(baseId),m_viewType(viewType), m_name(name), m_selectorId(selector)
            {
            m_description.AssignOrClear(descr);
            m_viewSource = source;
            m_viewSubType = viewSubType;
            }

        DgnViewId GetId() const {return m_viewId;} //!< The DgnViewId of this view.
        //! Get the DgnModelId of the base model for this view. Every view has one DgnModel designated its "base" model. Views that show only one model
        //! have no DgnModelSelector.
        DgnModelId GetBaseModelId() const {return m_baseModelId;}
        DgnViewType GetDgnViewType() const {return m_viewType;} //!< Get the DgnViewType for this view.
        Utf8CP GetViewSubType() const {return m_viewSubType.c_str();} //!< Get the viewcontroller ViewSubType
        DgnViewSource GetDgnViewSource() const {return m_viewSource;} //!< Get the DgnViewSource for this view.
        DgnModelSelectorId GetDgnModelSelectorId() const    {return m_selectorId;} //!< Get the DgnModelSelectorId for this view
        Utf8CP GetName() const {return m_name.c_str();} //!< Get the name for this view.
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : NULL;} //!< Get the description (if present) for this view.

        void SetDgnViewType (DgnViewType val) {m_viewType = val;}    //!< Set the DgnViewType for this view.
        void SetViewSubType(Utf8CP val) {m_viewSubType = val;} //!< Set the viewcontroller ViewSubType
        void SetDgnViewSource (DgnViewSource val) {m_viewSource = val;} //!< Set the DgnViewSource for this view.
        void SetBaseModelId(DgnModelId val) {m_baseModelId = val;} //!< Set the DgnModelId of the base model for this view.
        void SetSelectorId (DgnModelSelectorId val) {m_selectorId = val;} //!< Set the DgnModelSelectorId for this view.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the description for this view.
        void SetName(Utf8CP val) {m_name = val;}//!< Set the name for this view. View names must be unique for a DgnProject.
        bool IsValid() const {return m_viewId.IsValid();}   //!< Determine whether this View is valid or not.
    };

    //! An iterator over the table.
    struct Iterator : BeSQLite::DbTableIterator
    {
    private:
        int  m_typeMask;

    public:
        Iterator (DgnProjectCR project, int viewTypeMask) : DbTableIterator ((BeSQLiteDbCR)project) {m_typeMask = viewTypeMask;}

        //! An entry in the table
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnViewId GetDgnViewId() const;
            DGNPLATFORM_EXPORT DgnModelId GetBaseModelId() const;
            DGNPLATFORM_EXPORT DgnViewType GetDgnViewType() const;
            DGNPLATFORM_EXPORT DgnViewSource GetDgnViewSource() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            DGNPLATFORM_EXPORT DgnModelSelectorId GetSelectorId() const;
            DGNPLATFORM_EXPORT Utf8CP GetViewSubType() const;
            Entry const& operator* () const {return *this;}
        }; // Entry

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount () const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry (NULL, false);}
    }; // Iterator

public:
    //! Add a new view to the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertView (View&);

    //! Delete an existing view from the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteView (DgnViewId viewId);

    //! Change the contents of an existing view in the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateView (View const&);

    //! Get the DgnViewId for a view, by name
    DGNPLATFORM_EXPORT DgnViewId QueryViewId (Utf8CP viewName) const;

    //! Get the data for a view, by DgnViewId
    DGNPLATFORM_EXPORT View QueryViewById (DgnViewId id) const;

    enum class FillModels{No=0, Yes=1};
    DGNPLATFORM_EXPORT ViewControllerPtr LoadViewController(DgnViewId id, FillModels fillModels=FillModels::No) const;

    DGNVIEW_EXPORT BeSQLite::DbResult RenderAndSaveThumbnail(DgnViewId id, int resolution, MSRenderMode renderModeOverride);

    typedef DgnViewProperty::Spec const& DgnViewPropertySpecCR;

    //! Query a view property
    BeSQLite::DbResult QueryProperty (void* value, UInt32 size, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id=0) const;

    //! Query a view property string
    BeSQLite::DbResult QueryProperty (Utf8StringR value, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id=0) const;

    //! Query the size of a view property
    BeSQLite::DbResult QueryPropertySize (UInt32& size, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id=0) const;

    //! Change a view property
    BeSQLite::DbResult SaveProperty (DgnViewId viewId, DgnViewPropertySpecCR spec, void const* value, UInt32 size, UInt64 id=0);

    //! Change a view property string
    BeSQLite::DbResult SavePropertyString (DgnViewId viewId, DgnViewPropertySpecCR spec, Utf8StringCR value, UInt64 id=0);

    //! Delete a view property
    BeSQLite::DbResult DeleteProperty (DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id=0);

    //! Get an iterator over the Views in this project.
    Iterator MakeIterator (int viewTypeMask=DGNVIEW_TYPE_All) const {return Iterator (m_project, viewTypeMask);}

    //=======================================================================================
    //! An interface for handling the mapping of View types/subtypes to ViewControllers. When registered,
    //! instances of this class will be used by DgnViews::LoadViewController to manufacture appropriate instances
    //! of ViewControllers.
    // @bsiclass                                                    Keith.Bentley   07/14
    //=======================================================================================
    struct Factory
    {
        //! Create an instance of a ViewController for a DgnView, if appropriate.
        //! @param[in] project The DgnProject for the view
        //! @param[in] view The DgnViews::View to test.
        //! @return an instance of a ViewController for the supplied DgnViews::View, or NULL if the view is not of interest.
        //! All registered ViewController::Factorys are called, in turn, until one of them returns non-NULL.
        virtual ViewControllerP _SupplyViewController (DgnProjectR project, View const& view) = 0;
    };

    //! Register a DgnViews::Factory. Factories are called in the reverse order that they were registered. In other words,
    //! the last registered factory is called first, and therefore has the highest priority. All registered Factories are called
    //! to create a new instance of a ViewController until one of them returns a non-NULL value.
    //! @param factory the ViewController factory.
    DGNPLATFORM_EXPORT static void Register(Factory& factory);

    //! Remove a DgnViews::Factory from the list of registered factories.
    //! @param factory the ViewController factory to unregiser
    DGNPLATFORM_EXPORT static void UnRegister(Factory& factory);
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// not currently used
//=======================================================================================
struct DgnSessions : DgnProjectTable
{
private:
    friend struct DgnProject;
    explicit DgnSessions(DgnProjectR project) : DgnProjectTable(project){}

public:
    //! An object that holds the values for a Session in the DgnSessions.
    struct Row
    {
    private:
        friend struct DgnSessions;
        DgnSessionId m_sessionId;
        Utf8String   m_name;
        Utf8String   m_description;

        Row() {}

    public:
        explicit Row (Utf8CP name, Utf8CP descr=0, DgnSessionId id=DgnSessionId()) : m_sessionId(id), m_name(name) {m_description.AssignOrClear(descr);}
        DgnSessionId GetId() const {return m_sessionId;}
        Utf8CP      GetName() const        {return m_name.c_str();}
        Utf8CP      GetDescription() const {return m_description.length()>0 ? m_description.c_str() : NULL;}
        bool        IsValid() const        {return m_sessionId.IsValid();}
    };

    //! An iterator over the table
    struct Iterator : BeSQLite::DbTableIterator
    {
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project) {}

        //! An entry in the table
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnSessionId GetSessionId() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            Entry const& operator* () const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry (NULL, false);}
    DGNPLATFORM_EXPORT size_t QueryCount () const;
    };

    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertSession (Row&);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteSession (DgnSessionId SessionId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateSession (Row const&);
    DGNPLATFORM_EXPORT DgnSessionId QuerySessionId (Utf8CP SessionName) const;
    DGNPLATFORM_EXPORT Row QuerySessionById (DgnSessionId id) const;

    Iterator MakeIterator () const {return Iterator (m_project);}
};
//__PUBLISH_SECTION_START__

//=======================================================================================
//! Each DgnModelSelector has an entry in DgnModelSelectors.
//=======================================================================================
struct DgnModelSelectors : DgnProjectTable
{
private:
    friend struct DgnProject;
    explicit DgnModelSelectors(DgnProjectR project) : DgnProjectTable(project){}

public:
    struct Selector
    {
    private:
        friend struct DgnModelSelectors;
        DgnModelSelectorId m_selectorId;
        Utf8String  m_name;
        Utf8String  m_description;
        Selector() {}

    public:
        explicit Selector (Utf8CP name, Utf8CP descr=0, DgnModelSelectorId id=DgnModelSelectorId()) : m_selectorId(id), m_name(name) {m_description.AssignOrClear(descr); }
        DgnModelSelectorId GetId() const {return m_selectorId;}
        Utf8CP GetName() const {return m_name.c_str();}
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : NULL;}
        bool IsValid() const {return m_selectorId.IsValid();}
    };

    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project) {}

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnModelSelectorId GetId() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            Entry const& operator* () const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount () const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry (NULL, false);}
    };

    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertSelector (Selector&);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteSelector (DgnModelSelectorId);
    DGNPLATFORM_EXPORT DgnModelSelectorId QuerySelectorId (Utf8CP selectorName) const;
    DGNPLATFORM_EXPORT Selector QuerySelectorById (DgnModelSelectorId id) const;
    DGNPLATFORM_EXPORT BeSQLite::DbResult AddModelToSelector (DgnModelSelectorId selectorId, DgnModelId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult RemoveModelFromSelector (DgnModelSelectorId selectorId, DgnModelId);

    //! Get an iterator over the DgnModelSelector's in this project
    Iterator MakeIterator () const {return Iterator (m_project);}
};

enum class ModelIterate
    {
    All = 1<<0,   //!< return all iterable models
    Gui = 1<<1,   //!< return only models marked as visible in Model list GUI
    };

//=======================================================================================
//! A "pool" of reference-counted DgnElements. All in-memory DgnElements for a DgnProject are held in its DgnElementPool.
//! When the reference count of an element goes to zero, it is not immediately freed. Instead, it is held by the DgnElementPool
//! and may be "reclaimed" later if/when it is needed again. The memory held by DgnElements is not actually freed until
//! their reference count goes to 0 and the pool is subsequently purged.
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
struct DgnElementPool
{
    //! these values reflect the current state of the elements in the pool
    struct Totals
    {
        UInt32 m_entries;        //! total number of elements (referenced and unreferenced) in the tree
        UInt32 m_unreferenced;   //! total number of unreferenced elements in the tree
        Int64  m_allocedBytes;   //! total number of bytes of data held by elements in the tree
    };

    //! these values can be reset at any point to gauge "element flux"
    //! (note: the same element may become garbage and then be reclaimed, each such occurrence is reflected here.)
    struct Statistics
    {
        UInt32 m_newElements;    //! number of newly created or loaded elements
        UInt32 m_unReferenced;   //! number of elements that became garbage since last reset
        UInt32 m_reReferenced;   //! number of garbage elements that were referenced
        UInt32 m_purged;         //! number of garbage elements that were purged
    };

private:
    DgnElementPool(DgnProjectR);

//__PUBLISH_SECTION_END__
    friend struct DgnElementRef;
    friend struct ElementListHandler;
    friend struct PersistentElementRef;
    friend struct PersistentElementRefList;
    friend struct DbElementReader;
    friend struct DbElementWriter;
    friend struct DgnModels;
    friend struct XAttributeHandle;

    struct ElemIdTree*  m_tree;
    HeapZone            m_heapZone;
    mutable BeSQLite::BeDbMutex m_mutex;

    void OnReclaimed(PersistentElementRefCR);
    void OnUnreferenced(PersistentElementRefCR);
    ~DgnElementPool();
    void Destroy();
    void OnDestroying();
    PersistentElementRefP AllocateElementRef (DgnModelR, ElementId);

public:
    void AllocatedMemory (Int32);
    void ReturnedMemory (Int32);
    void OnChangesetApplied (TxnSummaryCR);
    void OnChangesetCanceled (TxnSummaryCR);
    HeapZone& GetHeapZone() {return m_heapZone;}
    void ReleaseAndCleanup(PersistentElementRefPtr&elementRef);

//__PUBLISH_SECTION_START__
public:

    //! Free unreferenced elements in the pool until the total amount of memory used by the pool is no more than a target number of bytes.
    //! @param[in] memTarget The target number of bytes used by elements in the pool. If the pool is currently using more than this target,
    //! unreferenced elements are freed until the the pool uses no more than targetMem bytes. Least recently used elements are freed first.
    //! If memTarget <= 0, all unreferenced elements are freed.
    //! @note: There is no guarantee that the pool will not actually consume more than memTarget bytes after this call, since elements with
    //! reference counts greater than 0 cannot be purged.
    DGNPLATFORM_EXPORT void Purge (Int64 memTarget);

    //! Get the total counts for the current state of the pool.
    DGNPLATFORM_EXPORT Totals GetTotals() const;

    //! Shortcut to get the Totals.m_allocatedBytes member
    Int64 GetTotalAllocated() const {return GetTotals().m_allocedBytes;}

    //! Get the statistics for the current state of the pool.
    DGNPLATFORM_EXPORT Statistics GetStatistics() const;

    //! Reset the statistics for the pool.
    DGNPLATFORM_EXPORT void ResetStatistics();

    //! Attempt to look up an element in the pool by ElementId.
    //! @param[in] id The Id of the element of interest.
    //! @return the PersistentElementRef of the element or NULL.
    DGNPLATFORM_EXPORT PersistentElementRefP FindElementById (ElementId id) const;
};

typedef bmap<DgnModelId,DgnModelP> T_DgnModelMap;
//=======================================================================================
//! Each DgnModel has an entry in the DgnModels table
//=======================================================================================
struct DgnModels : DgnProjectTable, NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    friend struct DgnProject;
    friend struct DgnFile;
    friend struct DgnV8Models;

    T_DgnModelMap            m_loadedModels;
    ElementId                m_highestElementId;         // 0 means not yet valid. Highest ElementId (for current repositoryId)
    ElementId                m_lowestNewElementId;       // first id created in this session.
    mutable DgnElementPool   m_elements;
    QvCache*                 m_qvCache;
    DgnModelId               m_firstModelId;

    void ClearLoaded();
    DgnModelP NewModelObject (DgnModelId);
    bool FreeQvCache();
    void Empty();
    void OnDbOpened();

private:
    explicit DgnModels(DgnProjectR project);

//__PUBLISH_SECTION_START__
public:
    #if !defined (DOCUMENTATION_GENERATOR)
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   09/10
    //=======================================================================================
    struct XAttributeFlags
    {
        enum class Compress {No=0, Snappy=1, Zip=2,};
        UInt32 m_compress:2;
        UInt32 m_reserved:30;

        XAttributeFlags (Compress compress=Compress::Snappy) {m_compress=(UInt32)compress; m_reserved=0;}
        XAttributeFlags (UInt32 val) {*((UInt32*) this) = val;}
        UInt32 GetUInt32 () const {return *((UInt32*) this);}
    };
    #endif // DOCUMENTATION_GENERATOR

private:
    DgnModels(DgnModels const&); // no copying
    DgnModels& operator= (DgnModels const&);
    ~DgnModels();

public:

    //! An object that holds a row from the DgnModel table.
    struct Model
    {
        enum class CoordinateSpace
        {
            Local   = 0,    // the model has a local coordinate system
            Project = 1,    // the model is in the project coordinate system (only applicable to physical models).
            Aux     = 2,    // the model is in the an auxillary coordinate system (only applicable to physical models).
        };

        friend struct DgnModels;

    private:
        DgnModelId   m_id;
        Utf8String   m_subType;
        Utf8String   m_name;
        Utf8String   m_description;
        DgnModelType m_modelType;
        CoordinateSpace  m_space;
        UInt32       m_visibility; //!< mask of values from ModelIterate

    public:
        Model()
            {
            m_modelType = DgnModelType::Physical;
            m_space = CoordinateSpace::Local;
            m_visibility = 0xff;
            };

        Model(Utf8CP name, DgnModelType modelType, CoordinateSpace space, Utf8CP subType="", DgnModelId id=DgnModelId()) : m_id(id), m_subType(subType), m_name(name)
            {
            m_modelType = modelType;
            m_space = space;
            m_visibility = 0xff;
            }

        Utf8StringR GetNameR() {return m_name;}
        Utf8StringR GetDescriptionR() {return m_description;}
        void SetName(Utf8CP val) {m_name.assign(val);}
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);}
        void SetVisibility(UInt32 val) {m_visibility = val;}
        void SetId(DgnModelId id) {m_id = id;}
        void SetModelType(DgnModelType val) {m_modelType = val;}
        void SetCoordinateSpace(CoordinateSpace val) {m_space = val;}
        Utf8CP GetModelSubType() const {return m_subType.c_str();}

        DgnModelId GetId() const {return m_id;}
        Utf8CP GetNameCP() const {return m_name.c_str();}
        Utf8String GetName() const {return m_name;}
        Utf8CP GetDescription() const {return m_description.c_str();}
        DgnModelType GetModelType() const {return m_modelType;}
        CoordinateSpace GetCoordinateSpace() const {return m_space;}
        UInt32 GetVisibility() const {return m_visibility;}
        bool IsIterable() const {return 0 != m_visibility;}
        bool InModelGui() const {return 0 != ((int) ModelIterate::Gui & m_visibility);}
        bool Is3d() const {return m_modelType==DgnModelType::Physical;}

    }; // Model

    struct Iterator : BeSQLite::DbTableIterator
    {
    private:
        ModelIterate   m_itType;

    public:
        Iterator (DgnProjectCR project, ModelIterate itType) : DbTableIterator ((BeSQLiteDbCR)project) {m_itType = itType;}
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnModelId GetModelId () const;
            DGNPLATFORM_EXPORT Utf8CP GetName () const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription () const;
            DGNPLATFORM_EXPORT DgnModelType GetModelType() const;
            DGNPLATFORM_EXPORT Model::CoordinateSpace GetCoordinateSpace() const;
            DGNPLATFORM_EXPORT UInt32 GetVisibility() const;

            bool Is3d() const {return GetModelType()==DgnModelType::Physical;}
            bool InModelGui() const {return 0 != ((int)ModelIterate::Gui & GetVisibility());}
            Entry const& operator* () const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        void SetIterationType(ModelIterate itType) {m_stmt=0; m_itType = itType;}
        DGNPLATFORM_EXPORT size_t QueryCount () const;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
    };

//__PUBLISH_SECTION_END__
    BeSQLite::DbResult DeleteElementFromDb (ElementId elementId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertXAttribute (Int64& rowid, ElementId elementId, XAttributeHandlerId handlerId, UInt32& xattId, UInt32 xaSize, void const* xaData, XAttributeFlags& flags);
    BeSQLite::DbResult DeleteXAttribute (Int64 rowid);

    QvCache* GetQvCache () {return m_qvCache;}
    void SetQvCache (QvCache* qvCache) {m_qvCache = qvCache;}

    ElementId GetLowestNewElementId ();
    DGNPLATFORM_EXPORT bool IsElementIdUsed (ElementId id) const;
    bool IsValidNewElementId (ElementId elemId);
    DGNPLATFORM_EXPORT ElementId GetHighestElementId();
    DGNPLATFORM_EXPORT ElementId MakeNewElementId();
    HeapZone& GetHeapZone() {return m_elements.GetHeapZone();}

    // WIP_LEGACY_FUNCTION_TO_BE_DELETED
    // Find a model in this file by its ModelId, optionally fill the model.
    // @param[out]     errorDetails    Set to DGNMODEL_STATUS_NotFound if the model could not be found
    // @param          modelID         Identifies the model to load
    // @param          fillCache       If true, calls        FillSectionsInModel with DgnModelSections::Model
    // @return a pointer to the model loaded or NULL if no model by the specified ID was found in the file
    DGNPLATFORM_EXPORT DgnModelP GetAndFillModelById (StatusInt* errorDetails, DgnModelId modelID, bool fillCache=false);

//__PUBLISH_SECTION_START__
    //! Get the ID of the first model in the project.
    DGNPLATFORM_EXPORT DgnModelId GetFirstModelId() const;

    //! Load a DgnModel from this DgnProject. Loading a model does not cause its elements to be filled. Rather, it creates an
    //! instance of the appropriate model type. If the model is already loaded, a pointer to the existing DgnModel is returned.
    //! @param[in] modelId The Id of the model to load.
    DGNPLATFORM_EXPORT DgnModelP GetModelById (DgnModelId modelId);

    //! Query if the specified model has already been loaded.
    //! @return a pointer to the model if found, or NULL if the model has not been loaded.
    //! @see GetLoadedModels
    //! @see LoadModelById
    DGNPLATFORM_EXPORT DgnModelP FindModelById (DgnModelId modelId);

    //! Get the models currently loaded
    //! Example:
    //! \code
    //! for (DgnModelP model : project.Models().GetLoadedModels())
    //!     {
    //!       ...
    //!     }
    //! \endcode
    //! @see LoadModelById
    DGNPLATFORM_EXPORT T_DgnModelMap const& GetLoadedModels() const;

    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertModel (Model & row);
    DGNPLATFORM_EXPORT BentleyStatus DeleteModel (DgnModelId id);
    DGNPLATFORM_EXPORT BentleyStatus QueryModelById (Model* out, DgnModelId id) const;
    DGNPLATFORM_EXPORT BentleyStatus GetModelName (Utf8StringR, DgnModelId id) const;

    //! Find the ModelId of the model with the specified name name.
    //! @return The model's ModelId. Check dgnModelId.IsValid() to see if the DgnModelId was found.
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(Utf8CP name) const;

    //! Make an iterator over the models in this project.
    Iterator MakeIterator (ModelIterate itType=ModelIterate::All) const {return Iterator (m_project, itType);}

    //! Make an iterator over the physical models in this project.
    Iterator MakePhysicalIterator() const {Iterator it(m_project, ModelIterate::All); it.Params().SetWhere("Type=0"); return it;}

    //! Make an iterator over the drawing models in this project.
    Iterator MakeDrawingIterator() const {Iterator it(m_project, ModelIterate::All); it.Params().SetWhere("Type=3"); return it;}

    //! Make an iterator over the sheet models in this project.
    Iterator MakeSheetIterator() const {Iterator it(m_project, ModelIterate::All); it.Params().SetWhere("Type=1"); return it;}

    //! Make an iterator over the Redline models in this project.
    Iterator MakeRedlineIterator() const {Iterator it(m_project, ModelIterate::All); it.Params().SetWhere("Type=2"); return it;}

    //! Make an iterator over the PhysicalRedline models in this project.
    Iterator MakePhysicalRedlineIterator() const {Iterator it(m_project, ModelIterate::All); it.Params().SetWhere("Type=12"); return it;}

    //@}

///@name Finding Models
    //@{

    //! Get the dictionary model for this file.
    //! @see FillDictionaryModelSections
    DGNPLATFORM_EXPORT struct DictionaryModel* GetDictionaryModel();

    //@}

    //! Create a new model.
    //! @param[in,out] modelData The data that defines the model. On input, the name and model type at least must be defined. On output, the id and fileid fields will be set.
    //! @return DGNMODEL_STATUS_Success if the new model was successfully create or non-zero if not.
    //! @Note Only default settings and properties are created for the new model. The caller should update these values.
    DGNPLATFORM_EXPORT DgnModelStatus CreateNewModel (Model& modelData);

    /// @name Finding elements
    //@{
    //! Get a list of elements for a DgnItem
    DGNPLATFORM_EXPORT ElementIdSet GetElementsForItem (DgnItemId id);

    //! Look up and return the ID of the element identified by a DgnResourceURI
    //! @param[in] uri     The URI
    //! @return The ID of the element identified by this URI; else an invalid ElementId if the URI is invalid or if the URI does not identify an element in the file.
    //! @remarks if \a uri contains a target file identifier then this function fails if the GUID part of that identifier does not match the GUID of this file.
    //! @see Handler::CreateDgnResourceURI
    DGNPLATFORM_EXPORT ElementId GetElementByURI (DgnResourceURI const& uri);

    //! Create a DgnResourceURI for the specified ECInstance
    //! @param[out] uri The URI
    //! @param[in] instanceKey Identifies the instance
    //! @param[in] strategy The strategy to use to create the URI
    //! @return non-zero error status if it is not possible to create a URI for this element
    DGNPLATFORM_EXPORT BentleyStatus CreateDgnResourceURI (DgnResourceURI& uri, BeSQLite::EC::ECInstanceKeyCR instanceKey, DgnResourceURICreationStrategy strategy = DGN_RESOURCE_URI_CREATION_STRATEGY_Any);

    //! Look up an ECInstance from a DgnResourceURI
    //! @param[out] instanceKey The instance identified by this URI
    //! @param[in]  uri         The URI
    //! @see CreateDgnResourceURI
    //! @return Non-zero error status if no ECInstance was found.
    DGNPLATFORM_EXPORT BentleyStatus FindECInstanceByURI (BeSQLite::EC::ECInstanceKeyR instanceKey, DgnResourceURI const& uri);
    //@}

    //! Get the DgnElementPool for this DgnProject.
    DGNPLATFORM_EXPORT DgnElementPool& ElementPool() const;

    //! Convenience method to get this DgnProject's DgnElementPool and call Purge.
    //! @see DgnElementPool::Purge
    void PurgeElementPool (Int64 memTarget) {ElementPool().Purge(memTarget);}

    //! Load an element within this project by its Id. @remarks The element is loaded if necessary.
    //! @return NULL if the element does not exist.
    DGNPLATFORM_EXPORT PersistentElementRefPtr GetElementById (ElementId id) const;

    //! Look up an element within this project by its Id.
    //! @return NULL if the element does not exist or is not currently loaded.
    DGNPLATFORM_EXPORT PersistentElementRefP FindElementById(ElementId id) const;

    //! Create a new model
    //! @param result   If not NULL, a non-zero error status if model creation is unsuccessful:
    //!                 DGNMODEL_STATUS_BadSeedModel if the seed is specified but not found;
    //!                 DGNMODEL_STATUS_ModelTableWriteError if the new model cannot be inserted;
    //!                 DGNMODEL_STATUS_InvalidModelName if \a name is invalid;
    //!                 DGNMODEL_STATUS_DuplicateModelName if \a name is already in use.
    //! @param name     The name of the new model.
    //! @param seedModel If valid, this function copies the properties of the the seed model in order to create the new model.
    DGNPLATFORM_EXPORT DgnModelP CreateNewModelFromSeed (DgnModelStatus* result, Utf8CP name, DgnModelId seedModel);

    //! Generate a model name that is not currently in use in this file
    //! @param[in]  baseName base model name to start with (optional)
    //! @return unique name that was generated
    DGNPLATFORM_EXPORT Utf8String GetUniqueModelName(Utf8CP baseName);
    //@}

    typedef DgnModelProperty::Spec const& DgnModelPropertySpecCR;
    BeSQLite::DbResult QueryProperty (void* value, UInt32 size, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult QueryProperty (Utf8StringR value, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult QueryPropertySize (UInt32& size, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult SaveProperty (DgnModelId modelId, DgnModelPropertySpecCR spec, void const* value, UInt32 size, UInt64 id=0);
    BeSQLite::DbResult SavePropertyString (DgnModelId modelId, DgnModelPropertySpecCR spec, Utf8StringCR value, UInt64 id=0);
    BeSQLite::DbResult DeleteProperty (DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id=0);

    //! Get a string containing the list of characters that may NOT appear in model names.
    static Utf8CP GetIllegalCharacters() {return "\\/:*?<>|\"\t\n,=&";}

    //! Determine whether the supplied name is a valid model name.
    //! @param[in] name The candidate model name to check
    //! @return true if the model name is valid, false otherwise.
    //! @note Model names may also not start or end with a space.
    static bool IsValidName(Utf8StringCR name) {return DgnProjectTable::IsValidName(name, GetIllegalCharacters());}

    //=======================================================================================
    //! An interface for handling the mapping of Model types/subtypes to DgnModels. When registered,
    //! instances of this class will be used by DgnModels::GetModelById to manufacture appropriate instances
    //! of DgnModels.
    // @bsiclass                                                    Keith.Bentley   07/14
    //=======================================================================================
    struct Factory
    {
        //! Create an instance of a DgnModel for a Model, if appropriate.
        //! @param[in] project The DgnProject for the model
        //! @param[in] model The DgnModels::Model to test.
        //! @return an instance of a DgnModel for the supplied DgnModels::Model, or NULL if the Model is not of interest.
        //! All registered DgnModel::Factory are called, in turn, until one of them returns non-NULL.
        virtual DgnModelP _SupplyDgnModel (DgnProjectR project, struct Model const& model) = 0;
    };

    //! Register a DgnModels::Factory. Factories are called in the reverse order that they were registered. In other words,
    //! the last registered factory is called first, and therefore has the highest priority. All registered Factories are called
    //! to create a new instance of a DgnModel until one of them returns a non-NULL value.
    //! @param factory the DgnModel factory.
    DGNPLATFORM_EXPORT static void Register(Factory& factory);

    //! Remove a DgnModels::Factory from the list of registered factories.
    //! @param factory the DgnModel factory to unregiser
    DGNPLATFORM_EXPORT static void UnRegister(Factory& factory);
};

//=======================================================================================
//! KeyStrings provide immutable ids (a DgnKeyStringId) that can be used as a persistent reference a string.
//! Once created, a KeyString cannot be removed or changed. In this manner, DgnKeyStringIds may be used as primary keys in other tables,
//! which is valuable since integer primary keys perform better than string primary keys.
//! KeyStrings also allow an optional value, which may be changed, to function as a dictionary.
//=======================================================================================
struct DgnKeyStrings : DgnProjectTable
{
private:
    friend struct DgnProject;
    explicit DgnKeyStrings(DgnProjectR project) : DgnProjectTable(project){}
    DGNPLATFORM_EXPORT BeSQLite::DbResult Delete (Utf8CP name);

public:

    //! Query the DgnKeyStringId for a KeyString, by name.
    //! @param[in] name The name of the KeyString.
    //! @return the DgnKeyStringId for the name. Will be invalid if the name is not found in the KeyString table.
    DGNPLATFORM_EXPORT DgnKeyStringId QueryId (Utf8CP name);

    //! Query the name and (optionally) value for a KeyString by DgnKeyStringId.
    //! @param[in] id the DgnKeyStringId to look up.
    //! @param[out] name On success, the name for the supplied DgnKeyStringId
    //! @param[out] value On success, the value for the supplied DgnKeyStringId. May be NULL.
    //! @return BE_SQLITE_ROW if the id exists in the DgnKeyStrings, error status otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Query (DgnKeyStringId id, Utf8StringP name, Utf8StringP value=0);

    //! Add a new entry in the DgnKeyStrings for this project.
    //! @param[in] name the name for the new KeyString. Must not be NULL.
    //! @param[in] value the value for the new KeyString. May be NULL.
    //! @return the DgnKeyStringId of the new KeyString. Will be invalid if the name already exists in the table.
    DGNPLATFORM_EXPORT DgnKeyStringId Insert (Utf8CP name, Utf8CP value=0);

    //! Change the value for an existing KeyString.
    //! @param[in] name the name of the KeyString whose value is to be modified.
    //! @param[in] value the new value for the keystring.
    //! @return BE_SQLUTE_DONE if the value was changed, error status otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Update (Utf8CP name, Utf8CP value);

    //! Look up a KeyString by name, create it if it doesn't already exist.
    //! @param[in] name the name of the KeyString.
    DgnKeyStringId GetOrInsert (Utf8CP name) {DgnKeyStringId id=QueryId(name); return id.IsValid() ? id : Insert (name);}
};

//=======================================================================================
//! The DgnColors holds the True Colors for a DgnProject. True Colors are RGB values (no transparency) that may optionally
//! be named and from a "color book". The entries in the table are identified by DgnTrueColorId's, that are stored in Dgn element (see discussion of
//! Dgn Elmement Colors at #GetElementColor for the format of the color value on the header of an element.)
//! Once a True Color is defined, it may not be changed or deleted. Note that there may be multiple enties in the table with the same RGB value.
//! However, if a book name is supplied, there may not be two entries with the same name.
//=======================================================================================
struct DgnColors : DgnProjectTable, NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    friend struct DgnProject;
    mutable DgnColorMapP m_colorMap;
    explicit DgnColors(DgnProjectR project) : DgnProjectTable(project){m_colorMap=NULL;}

//__PUBLISH_SECTION_START__
private:
    DgnColors(DgnColors const&); // no copying
    ~DgnColors();

public:
//__PUBLISH_SECTION_END__
    DgnColorMapP GetDgnColorMapP() {return const_cast<DgnColorMapP>(GetDgnColorMap());}
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveDgnColorMap ();
//__PUBLISH_SECTION_START__

    //! Get the DgnColorMap for this DgnProject.
    DGNPLATFORM_EXPORT DgnColorMapCP GetDgnColorMap() const;

    //! Add a new entry to this DgnColors.
    //! @param[in] color The RGB values for the new entry.
    //! @param[in] name The name of the color (or NULL).
    //! @param[in] bookname The name of the colorbook (or NULL).
    //! @note For a given bookname, there may not be more than one color with the same name.
    //! @return colorId The DgnTrueColorId for the newly created entry. Will be invalid if name+bookname is not unique.
    DGNPLATFORM_EXPORT DgnTrueColorId InsertTrueColor(RgbColorDef const& color, Utf8CP name=0, Utf8CP bookname=0);

    //! Find a DgnTrueColorId for the given RGB value.
    //! @return A DgnTrueColorId for the supplied RGB value. If no entry in the table has the given RGB value, the DgnTrueColorId will be invalid.
    //! @note If the table holds more than one entry with the same RGB values, it is not guaranteed which DgnTrueColorId is returned.
    DGNPLATFORM_EXPORT DgnTrueColorId QueryTrueColorId(RgbColorDef const& color) const;

    //! Get content of a DgnTrueColorId.
    //! @param[out] color The RGB value for the colorId. May be NULL.
    //! @param[out] name The name for the colorId. May be NULL.
    //! @param[out] bookname The bookName for the colorId. May be NULL.
    //! @param[in] colorId the true color id to query
    //! @return SUCCESS if colorId was found in the table and the values are valid. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus QueryTrueColorInfo(RgbColorDef* color, Utf8StringP name, Utf8StringP bookname, DgnTrueColorId colorId) const;

    //! Determine whether the a supplied element color is a true color and, if so, its RGB.
    //! @param[out] colorDef the RGB of elementColor.
    //! @param[in] elementColor the integer value stored on a Dgn element in the symb.color field.
    //! @return true if elementColor is a valid true color and colorDef is valid.
    DGNPLATFORM_EXPORT bool IsTrueColor(IntColorDef& colorDef, UInt32 elementColor) const;

    //! Get or find an "encoded true color value" that can be stored on an element from the supplied IntColorDef.
    //! @param[in] colorDef a color value
    //! @param[in] bookName Color book name for the supplied IntColorDef (can be NULL). Used only if no entry with the supplied RGB is found and createIfNotPresent
    //! @param[in] colorName Color name in color book of the supplied IntColorDef (can be NULL).
    //! @param[in] createIfNotPresent  If true, create the color table entry if it does not already exist.
    //! @return the encoded true color value on success, or INVALID_COLOR if colorDef cannot be found in the DgnColors and createIfNotPresent is false.
    //! @note An "encoded true color value" has the top 4 bits zero, the next 20 bits are the DgnTrueColorId and the low 8 bits are the index of the closest
    //! color in the project's DgnColorMap.
    DGNPLATFORM_EXPORT UInt32 GetElementColor(IntColorDef const& colorDef, Utf8CP bookName, Utf8CP colorName, bool createIfNotPresent);

    //! Get or find an "encoded true color value" that can be stored on an element from the supplied IntColorDef.
    //! @param[in] colorDef a color value
    //! @param[in] bookName Color book name for the supplied IntColorDef (can be NULL). Used only if no entry with the supplied RGB is found and createIfNotPresent
    //! @param[in] colorName Color name in color book of the supplied IntColorDef (can be NULL).
    //! @return the encoded true color value
    UInt32 CreateElementColor (IntColorDef const& colorDef, Utf8CP bookName, Utf8CP colorName) {return GetElementColor (colorDef, bookName, colorName, true);}

    //! Find an "encoded true color value" that can be stored on an element from the supplied IntColorDef.
    //! @param[in] colorDef a color value
    //! @return the encoded true color value on success, or INVALID_COLOR if colorDef cannot be found in the DgnColors.
    UInt32 FindElementColor (IntColorDef const& colorDef) {return GetElementColor (colorDef, NULL, NULL, false);}

    //! Get the color information from the supplied element color id.
    //! @param[out]     colorDef        IntColorDef for the supplied element color, can be used to get color values.
    //! @param[out]     colorIndex      The 0 to INDEX_Background index into the file's DgnColorMap. For rgb and book colors this is the closest match from when they were created.
    //! @param[out]     isTrueColor     True if supplied element color is a rgb or book color, false for color index.
    //! @param[out]     bookName        Color book name for the supplied element color (empty string for rgb and indexed colors).
    //! @param[out]     colorName       Color name from color book for the supplied element color (empty string for rgb and indexed colors).
    //! @param[in]      elementColor    The element color to extract the information for.
    //! @return SUCCESS if element color is a valid rgb, book, or DgnColorMap index. COLOR_BYLEVEL or COLOR_BYCELL will return ERROR.
    DGNPLATFORM_EXPORT StatusInt Extract (IntColorDef* colorDef, UInt32* colorIndex, bool* isTrueColor, Utf8StringP bookName, Utf8StringP colorName, UInt32 elementColor) const;

    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project) {}

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnTrueColorId GetId() const;
            DGNPLATFORM_EXPORT RgbColorDef GetColorValue () const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetBookName() const;
            Entry const& operator* () const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount () const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry (NULL, false);}
    };

    //! Make an iterator over the named colors in this DgnProject.
    Iterator MakeIterator() const {return Iterator (m_project);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnFonts : DgnProjectTable, NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    mutable T_FontNumberMap     m_fontNumberMap;
    mutable bool                m_fontNumberMapLoaded;
    mutable bool                m_embeddedFontsLoaded;
    mutable T_FontCatalogMap    m_embeddedFonts;
    mutable T_FontCatalogMap    m_missingFonts;

    friend struct DgnProject;

    // Ensures the font number map is loaded as a side effect.
    T_FontNumberMap const& FontNumberMap() const;
    explicit DgnFonts(DgnProjectR project);

//__PUBLISH_SECTION_START__
private:
    DgnFonts();
    DgnFonts(DgnFonts const&); // no copying

public:
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project){}

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT UInt32 GetFontId() const;
            DGNPLATFORM_EXPORT DgnFontType GetFontType() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            bool IsValid() const {return m_isValid && (NULL!=m_sql->GetSqlStatementP());}
            Entry const& operator*() const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount() const;
    DGNPLATFORM_EXPORT const_iterator begin() const;
    const_iterator end() const {return Entry (NULL, false);}
    };
//__PUBLISH_SECTION_END__
    //! Attempts to find a missing font object by the given type and name.
    DGNPLATFORM_EXPORT DgnFontCP GetMissingFont(DgnFontType fontType, Utf8CP fontName) const;

    //! First attempts to get a missing font object by the given type and name; if it does not exist, one is created.
    //! v8FontNumber is optional, and is only used with DgnFontType is RSC.
    DGNPLATFORM_EXPORT DgnFontCP GetOrCreateMissingFont(DgnFontType, Utf8CP fontName, UInt32 v8FontNumber = (UInt32)-1) const;

    // *** DO NOT USE *** Use AcquireFontNumber instead. This is a low-level method to support DgnV8ProjectImpoter.
    DGNPLATFORM_EXPORT BentleyStatus InsertFontNumberMappingDirect (UInt32 id, DgnFontType, Utf8CP name);

    // *** DO NOT USE *** This is used to corrdinate with the V8 project importer.
    DGNPLATFORM_EXPORT bool IsFontNumberMapLoaded() const;

//__PUBLISH_SECTION_START__

/** @cond BENTLEY_SDK_Publisher */
    //! Embeds the given font in this DgnProject.
    DGNPLATFORM_EXPORT DgnFontCP EmbedFont (DgnFontCR);

    //! Provides a DgnProject-specific font number for the given font. A new number is added if a mapping does not already exist.
    DGNPLATFORM_EXPORT BentleyStatus AcquireFontNumber (UInt32& acquiredID, DgnFontCR);

    //! Queries the project font map to resolve a font to a number. If a mapping does not already exist, one is <b>not</b> added.
    DGNPLATFORM_EXPORT BentleyStatus FindFontNumber (UInt32* foundID, DgnFontCR) const;

    //! Queries the project font map to resolve a number to a font. If a mapping does not already exist, one is <b>not</b> added.
    DGNPLATFORM_EXPORT DgnFontCP FindFont (UInt32 fontNumber) const;

    //! Gets a collection of this DgnProject's embedded fonts. This DgnProject owns these font objects.
    DGNPLATFORM_EXPORT T_FontCatalogMap const& EmbeddedFonts() const;
/** @endcond */

    //! Get an iterator over the fonts in this DgnProject.
    Iterator MakeIterator() const {return Iterator (m_project);}
};

//=======================================================================================
//! DgnStamps are persistent "named chunks of data" that are automatically cached in memory as they're accessed. This is most valuable for
//! values that are large and shared among multiple persistent clients. Clients persist the StampName and then access the StampData through
//! this API. The most common example of DgnStamps are shared XGraphics streams for symbols and DgnComponents.
// @bsiclass                                                    Keith.Bentley   01/14
//=======================================================================================
struct DgnStamps : DgnProjectTable, NonCopyableClass
{
    //! The name for a DgnStamp. StampNames have 4 parts, much like a BeSQLite::Property.
    //! The namespace can be used to define a "type" of DgnStamp. Namespaces should include a leading prefix followed by an underscore (_)
    //! to help avoid collisions with other applications.
    //! The name can be used to identify a specific instance of a DgnStamp or, in combination with ids, a family of related DgnStamps. For
    //! example, DgnComponents define a parameterized component family by name. Various combinations of parameter values generate different
    //! DgnStamps, identified by the same name qualified by Ids.
    //! The numeric part of a StampName is of the form [id1,id2]. Both Id1 and Id2 are 64bit values and neither Id1
    //! nor Id2 may be negative, Id1 may not be 0, and of course the combination of [namespace,name,id1,id2] must be unique. The value of Id1 may be
    //! auto-generated by the #Insert method.
    struct StampName
    {
        Utf8String m_namespace;
        Utf8String m_name;
        DgnStampId m_id1;
        Int64      m_id2;

        //! Construct a blank, invalid StampName
        StampName() {}
        //! Construct a StampName with supplied values
        //! @param[in] ns the Namespace for this StampName. Must not be NULL or blank or the StampName is invalid.
        //! @param[in] name the Name for this StampName. Must not be NULL or blank or the StampName is invalid.
        //! @param[in] id1 The DgnStampId value for this StampName
        //! @param[in] id2 The Id2 value for this StampName. Most often this will be zero.
        StampName(Utf8CP ns, Utf8CP name, DgnStampId id1=DgnStampId(), Int64 id2=0) : m_namespace(ns), m_name(name), m_id1(id1), m_id2(id2) {}

        bool operator==(StampName const& rhs) const {return m_namespace==rhs.m_namespace && m_name==rhs.m_name && m_id1==rhs.m_id1 && m_id2==rhs.m_id2;}
        bool operator!=(StampName const& rhs) const {return !(*this==rhs);}
        bool operator<(StampName const& other) const
            {
            int val = m_namespace.CompareTo(other.m_namespace);
            if (val == 0)
                {
                val = m_name.CompareTo(other.m_name);
                if (val == 0)
                    return (m_id1 != other.m_id1) ? (m_id1 < other.m_id1) : (m_id2 < other.m_id2);
                }
            return val<0;
            }

        //! Get the namespace value
        Utf8StringCR GetNamespace() const {return m_namespace;}
        //! Get the name value
        Utf8StringCR GetName() const {return m_name;}
        //! Get the Id1 value
        DgnStampId GetId1() const {return m_id1;}
        //! Get the Id2 value
        Int64 GetId2() const {return m_id2;}
        //! Determine whether the namespace/name part of this StampName is valid
        bool IsNameValid() const {return !m_namespace.empty() && !m_name.empty();}
        //! Determine whether this StampName is valid
        bool IsValid() const {return IsNameValid() && m_id1.IsValid() && m_id2>=0;}
    }; //StampName

    //! The data for a DgnStamp. StampData are reference-counted and cached.
    struct StampData : RefCountedBase, bvector<UInt8>
    {
        StampData() {}
        StampData (UInt8 const* data, int len) : bvector<UInt8>(data,data+len) {}
    };
    typedef RefCountedPtr<StampData> StampDataPtr;

private:
    DgnStamps(DgnStamps const&); // no copying
    DgnStamps(); // internal only

//__PUBLISH_SECTION_END__
    friend struct DgnProject;
    explicit DgnStamps(DgnProjectR project) : DgnProjectTable(project) {}

    struct StampCache
    {
    private:
        friend struct DgnStamps;
        bmap<StampName,StampDataPtr> m_map;
        void Clear() {m_map.clear();}
        void Add(StampName const& name, StampData& data) {m_map.Insert (name, &data);}
        void Drop(StampName const& name) {m_map.erase (name);}
        StampDataPtr Find(StampName const& name) {auto stamp=m_map.find(name); return stamp!=m_map.end() ? stamp->second : NULL;}
    };

private:
    StampCache m_cache;

//__PUBLISH_SECTION_START__
public:
    //! Add a new stamp to the DgnProject.
    //! @param name[in,out] The name for the new stamp. If the namespace and name parts are not valid, this method will fail.
    //! If the Id1 part of the name is not valid, the next available value for the specified namespace/name will be assigned.
    //! You can retrieve the newly assigned Id1 value from name after a successful call.
    //! If the Id2 part of the name is negative, it is set to 0. Otherwise it is used unchanged (even if Id1 starts out invalid
    //! and is auto-generated.)
    //! @param[in] data The data to store for the newly created stamp. May be empty.
    //! @return BE_SQLITE_OK if the DgnStamp was successfully added, error status otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertStamp(StampName& name, StampData const& data);

    //! Delete a DgnStamp from the DgnProject.
    //! @param[in] name the StampName to delete.
    //! @return BE_SQLITE_OK if the DgnStamp was successfully remove, error status otherwise.
    //! @note It is the caller's responsibility to determine that there are no persistent references to the DgnStamp to be deleted.
    //! There are no internal mechanisms to enforce that, since DgnStamp references can be stored in application data.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteStamp(StampName const& name);

    //! Update (change) the value of the StampData for a DgnStamp.
    //! @param[in] name the StampName to update.
    //! @param[in] data the new value for the StampData
    //! @return BE_SQLITE_OK if the stamp was updated, error status otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateStamp(StampName const& name, StampData const& data);

    //! Find the StampData for a DgnStamp by StampName.
    //! @return The StampDataPtr for the requested stamp. If the StampName doesn't specify a valid DgnStamp, it will be invalid.
    //! Callers should release this RefCountedPtr as soon as practical so the stamp is not held in memory unnecessarily.
    //! @note DgnStamps are cached in memory, so very often repeated calls to this method with the same StampName will return
    //! the same value. However, if the DgnStamp cache consumes too much memory, it can be periodically purged, so DgnStamps
    //! sometimes must be reloaded from the database.
    DGNPLATFORM_EXPORT StampDataPtr FindStamp(StampName const& name);
};

//__PUBLISH_SECTION_END__
//=======================================================================================
//  The data in the stamp holds everything that can appear in any of the XAttributes
//  related to XGraphics symbols.  It can have a symbol ID or transform.  It must
//  contain an XGraphics stream.
//
//! @bsiclass                                                     John.Gooding    04/14
//=======================================================================================
struct XGraphicsSymbolStamp : RefCountedBase
{
private:
    friend struct StampQvElemMap;

    DgnStampId      m_symbolId;      //  currently only used for debugging.
    DgnStamps::StampDataPtr m_stampData;
    DgnProjectCR    m_project;
    bool            m_is3d;
    DRange3d        m_range;

    enum OpCode
        {
        OP_XGraphics    = 0,
        OP_SymbolId     = 1,
        OP_Transform    = 2,
        OP_SymbolIds    = 3
        };

    static void Append(DgnStamps::StampData& stampData, void const*data, size_t dataSize);
    static void AppendSection(DgnStamps::StampData& stampData, XGraphicsSymbolStamp::OpCode opCodeIn, size_t dataSize, void const*data);
    void const*GetData(UInt32& streamSize, OpCode testOpCode) const;
    XGraphicsSymbolStamp(DgnStamps::StampData& stampData, DgnStampId symbolId, DgnProjectCR project, bool is3d, DRange3dCR);
    ~XGraphicsSymbolStamp();
    DgnStamps::StampData const& GetStampData() const { return *m_stampData.get(); }

public:
    static Utf8CP GetStampNamespace() { return "Dgn_XGr"; }
    static Utf8CP GetStampName() { return "Symbol"; }

    static BeSQLite::DbResult Create(DgnStampId& stampId, DgnProjectCR project, XGraphicsContainerCR xgraphics, DRange3dCR range, TransformCP transform, XGraphicsSymbolIdCP xGraphicsSymbolId, bool is3d);
    static BeSQLite::DbResult Delete(DgnStampId stampId, DgnProjectR project);
    void GetRange(DRange3d&range) const { range = m_range; }
    DgnProjectCR GetDgnProject() const;
    bool GetIs3d() const { return m_is3d; }
    DGNPLATFORM_EXPORT static XGraphicsSymbolStampPtr Get(DgnProjectCR project, DgnStampId symbolId);
    DGNPLATFORM_EXPORT void const*GetXGraphicStream(UInt32& streamSize) const;
    DGNPLATFORM_EXPORT BentleyStatus GetTransform(TransformR transform) const;

};
//__PUBLISH_SECTION_START__

/** @cond BENTLEY_SDK_Internal */
//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnMaterials : DgnProjectTable
{
private:
    friend struct DgnProject;
    explicit DgnMaterials (DgnProjectR project) : DgnProjectTable (project) { }

public:
    struct Row
    {
    private:
        friend struct DgnMaterials;

        DgnMaterialId       m_materialId;
        Utf8String          m_name;
        Utf8String          m_palette;

    public:
        Row () { }
        Row (Utf8CP name, Utf8CP palette, DgnMaterialId id = DgnMaterialId ()) : m_materialId (id), m_name (name), m_palette (palette) { }

        DgnMaterialId GetId () const { return m_materialId; }
        Utf8CP GetName () const { return m_name.c_str(); }
        Utf8CP GetPalette () const { return m_palette.c_str(); }
        void SetName (Utf8CP val) { m_name = val; }
        void SetPalette (Utf8CP val) { m_palette = val; }
        bool IsValid () const { return m_materialId.IsValid(); }
    };

    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator (DgnProjectCR project) : DbTableIterator ((BeSQLiteDbCR)project) { }

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry (BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnMaterialId GetId () const;
            DGNPLATFORM_EXPORT Utf8CP GetName () const;
            DGNPLATFORM_EXPORT Utf8CP GetPalette () const;
            Entry const& operator* () const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount () const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry (NULL, false);}
    };

    Iterator MakeIterator () const {return Iterator (m_project);}

    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertMaterial (Row& row);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteMaterial (DgnMaterialId materialId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateMaterial (Row const& row);
    DGNPLATFORM_EXPORT Row QueryMaterialById (DgnMaterialId id) const;

    typedef DgnMaterialProperty::Spec const& DgnMaterialPropertySpecCR;
    BeSQLite::DbResult QueryProperty (void* value, UInt32 size, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult QueryProperty (Utf8StringR value, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult QueryPropertySize (UInt32& size, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id=0) const;
    BeSQLite::DbResult SaveProperty (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, void const* value, UInt32 size, UInt64 id=0);
    BeSQLite::DbResult SavePropertyString (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, Utf8StringCR value, UInt64 id=0);
    BeSQLite::DbResult DeleteProperty (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id=0);
};
/** @endcond */

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnUnits : NonCopyableClass
{
    struct GeoTransform
    {
        RotMatrix   m_matrix;
        RotMatrix   m_inverseMatrix;
        double      m_xRadius;
        double      m_yRadius;
        bool        m_isValid;

        GeoTransform() : m_isValid (false) { }
        void Init (DgnUnits const& units);
    };

private:
    friend struct DgnProject;
    DgnProjectR     m_project;
    double          m_azimuth;
    double          m_latitude;
    double          m_longitude;
    DRange3d        m_extent;
    DPoint3d        m_projectOrigin;      //!< Global Origin - in storgae units
    DPoint2d        m_geoOriginBasis;
    bool            m_hasCheckedForGCS;
    bool            m_hasGeoOriginBasis;
    double          m_geoCoordWorkingAreaInMetersSquared;
    double          m_geoCoordWorkingAreaInDegrees;
    mutable GeoTransform m_geoTransform;
    GeoCoordinates::DgnGCS* m_gcs;
    IGeoCoordinateServicesP m_geoServices;

    DgnUnits(DgnProjectR project);
    void LoadProjectExtents();

public:
    void SetGlobalOrigin(DPoint3dCR origin) {m_projectOrigin=origin;}

    DGNPLATFORM_EXPORT void Save();
    DGNPLATFORM_EXPORT DgnFileStatus Load();

    DgnProjectR GetProject() const {return m_project;}

    DPoint3dCR GetProjectOrigin() const {return m_projectOrigin;}

    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProjectExtents (DRange3dCR newExtents);

    //! Get the union of the range (axis-aligned bounding box) of all physical elements in the project
    DGNPLATFORM_EXPORT DRange3d GetProjectExtents();

    //! Convert a GeoPoint to an XYZ point
    //! @param[out] outUors     The output XYZ point
    //! @param[in] inLatLong    The input GeoPoint
    //! @return non-zero error status if the point cannot be converted or if the project is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus UorsFromLatLong (DPoint3dR outUors, GeoPointCR inLatLong);

    //! Convert a an XYZ point to a GeoPoint
    //! @param[out] outLatLong  The output GeoPoint
    //! @param[in] inUors    The input XYZ point
    //! @return non-zero error status if the point cannot be converted or if the project is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus LatLongFromUors (GeoPointR outLatLong, DPoint3dCR inUors);

    //! Query the GCS of the project, if any.
    //! @return the project's GCS or NULL if the project is not geo-located
    DGNPLATFORM_EXPORT GeoCoordinates::DgnGCS* GetDgnGCS();

    //! Get the latitude of the origin in the global coordinate system of the project.
    double GetOriginLatitude() const {return m_latitude;}

    //! Get the longitude of the origin in the global coordinate system of the project.
    double GetOriginLongitude() const {return m_longitude;}

    //! Get the coordinates in DgnCoordSystem::World of the origin latitude and longitude.
    DPoint2d GetGeoOriginBasis() const {return m_geoOriginBasis;}

    //! Get the azimuth (true north offset) of the global coordinate system of the project.
    double GetAzimuth() const {return m_azimuth;}

    //! Set the latitude of the origin in the global coordinate system of the project.
    DGNPLATFORM_EXPORT void SetOriginLatitude (double originLat);

    //! Set the longitude of the origin in the global coordinate system of the project.
    DGNPLATFORM_EXPORT void SetOriginLongitude (double originLong);

    //! Set the coordinates in DgnCoordSystem::World of the origin latitude and longitude.
    DGNPLATFORM_EXPORT void SetGeoOriginBasis (DPoint2dCR basis);

    //! Set the azimuth of the global coordinate system of the project.
    DGNPLATFORM_EXPORT void SetAzimuth (double azimuth);

    //! Utility function to check if this DgnProject contains the necessary information to convert between
    //! geographic and cartesian coordinate systems.
    //! @return true if ConvertToWorldPoint and ConvertToGeoPoint can succeed.
    DGNPLATFORM_EXPORT bool CanConvertBetweenGeoAndWorld() const;

    //! @return true if point is close enough to this coordinate system's origin to be translated to a DPoint3d accurately.
    DGNPLATFORM_EXPORT bool IsGeoPointWithinGeoCoordinateSystemWorkingArea (GeoPointCR point) const;

    //! @return true if point is close enough to this coordinate system's origin to be translated to a GeoPoint accurately.
    DGNPLATFORM_EXPORT bool IsWorldPointWithinGeoCoordinateSystemWorkingArea (DPoint3dCR point) const;

    // Get the GeoCoordinate system working area
    //! @param[out] metersSq The working area in square meters
    //! @param[out] degrees The working area in degrees. This value ignores the effects of changing latitude.
    void GetGeoCoordinateSystemWorkingArea (double& metersSq, double& degrees) {metersSq=m_geoCoordWorkingAreaInMetersSquared; degrees=m_geoCoordWorkingAreaInDegrees;}

    // Set the GeoCoordinate system working area
    //! @param[out] metersSq The working area in square meters
    //! @param[out] degrees The working area in degrees. This value ignores the effects of changing latitude.
    void SetGeoCoordinateSystemWorkingArea (double metersSq, double degrees) {m_geoCoordWorkingAreaInMetersSquared=metersSq; m_geoCoordWorkingAreaInDegrees=degrees;}

    //! Convert from WGS84 to storage units (millimeters).
    //! @return ERROR if the DgnProject does not have a valid latitude and longitude, SUCCESS otherwise
    DGNPLATFORM_EXPORT BentleyStatus ConvertToWorldPoint (DPoint3dR worldPoint, GeoPointCR geoPoint) const;

    //! Convert from Storage units to WGS84.
    //! @return ERROR if the DgnProject does not have a valid latitude and longitude, SUCCESS otherwise
    DGNPLATFORM_EXPORT BentleyStatus ConvertToGeoPoint (GeoPointR geoPoint, DPoint3dCR worldPoint) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DgnStyleType
{
    Display = 1,
    Text = 2,
    Line = 3,
    AnnotationText = 4,
    AnnotationFrame = 5,
    AnnotationLeader = 6,
    TextAnnotationSeed = 7

}; // DgnStyleType

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DgnStyleSort
{
    None = 1,
    NameAsc = 2,
    NameDsc = 3

}; // DgnStyleSort

//=======================================================================================
//! Each DisplayStyle has an entry in the DgnStyles table.
//=======================================================================================
struct DgnStyles : DgnProjectTable, NonCopyableClass
{
private:
    friend struct DgnProject;

    struct DgnDisplayStyles* m_displaySytles;
    struct DgnTextStyles*    m_textStyles;
    struct DgnLineStyles*    m_lineStyles;
    struct DgnAnnotationTextStyles* m_annotationTextStyles;
    struct DgnAnnotationFrameStyles* m_annotationFrameStyles;
    struct DgnAnnotationLeaderStyles* m_annotationLeaderStyles;
    struct DgnTextAnnotationSeeds* m_textAnnotationSeeds;

    explicit DgnStyles(DgnProjectR project);
    DgnStyles(DgnStyles const&);// no copying
    ~DgnStyles();

public:
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Style
    {
    private:
        friend struct DgnStyles;

        DgnStyleId m_id;
        DgnStyleType m_type;
        Utf8String m_name;
        Utf8String m_description;
        bvector<Byte> m_data;

        Style() : m_type((DgnStyleType)0){}

    public:
        Style(DgnStyleId id, DgnStyleType type, Utf8CP name, Utf8CP description, void const* data, size_t dataSize) : m_id(id), m_type(type), m_name(name)
           {
            m_description.AssignOrClear(description);

            if(dataSize > 0)
               {
                m_data.resize(dataSize);
                memcpy(&m_data[0], data, dataSize);
                }
            }

        DgnStyleId GetId() const {return m_id;}
        DgnStyleType GetType() const {return m_type;}
        Utf8CP GetName() const {return m_name.c_str();}
        Utf8CP GetDescription() const{return m_description.c_str();}
        void const* GetData() const {return &m_data[0]; }
        size_t GetDataSize() const {return m_data.size();}
    };

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Iterator : public BeSQLite::DbTableIterator
    {
        explicit Iterator(DgnProjectCR project) : DbTableIterator((BeSQLiteDbCR)project){}

        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct Entry : public DbTableIterator::Entry, public std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLiteStatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid){}

        public:
            DGNPLATFORM_EXPORT DgnStyleId GetId() const;
            DGNPLATFORM_EXPORT DgnStyleType GetType() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            DGNPLATFORM_EXPORT void const* GetData() const;
            DGNPLATFORM_EXPORT int GetDataSize() const;

            Entry const& operator*() const{return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;

        DGNPLATFORM_EXPORT Entry begin() const;
        Entry end() const{return Entry(NULL, false);}
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    };

//__PUBLISH_SECTION_END__
public:

    //! Adds a new style. The ID in the provided style is ignored. If a style already exists by-type/name, no action is performed.
    //! @return BE_SQLITE_DONE if successful.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertStyle(Style&, Int32 minimumStyleId = 0);

    //! Adds a new style with its given ID. If a style already exists by-type/name/id, no action is performed.
    //! @return BE_SQLITE_DONE if successful.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    //! @note This is only intended to support importers that already have unique IDs, and should not normally be used; @see InsertStyle.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertStyleWithId(Style&);

    //! Deletes a style by-ID.
    //! @param[in] type the type of the style to remove.
    //! @param[in] id the id of the style to remove. Must not be used anywhere in the project
    //! @return BE_SQLITE_DONE if successful.
    //! @remarks Use extreme caution when using this method to ensure that the style is not used anywhere in the project.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteStyle(DgnStyleType type, DgnStyleId id);

    //! Updates the name and data of a style by-ID.
    //! @return BE_SQLITE_DONE if successful.
    //! @remarks This will <b>not</b> allow you to change the ID or type.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateStyle(Style const&);

    //! Gets a style ID by type and name.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    DGNPLATFORM_EXPORT DgnStyleId QueryStyleId(DgnStyleType styleType, Utf8CP styleName) const;

    //! Gets a style by-ID.
    //! @param[in] type the style type to find.
    //! @param[in] id the style ID to find.
    //! @note Most style systems require additional management of their respective styles. Always look for a more specific form of this method for each individual style system first.
    DGNPLATFORM_EXPORT Style QueryStyleById(DgnStyleType type, DgnStyleId id) const;

//__PUBLISH_SECTION_START__
public:

    //! Provides accessors for text styles.
    DGNPLATFORM_EXPORT struct DgnTextStyles& TextStyles();

    //! Provides accessors for line styles.
    DGNPLATFORM_EXPORT struct DgnLineStyles& LineStyles();

    //! Provides accessors for annotation text styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationTextStyles& AnnotationTextStyles();

    //! Provides accessors for annotation frame styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationFrameStyles& AnnotationFrameStyles();

    //! Provides accessors for annotation leader styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationLeaderStyles& AnnotationLeaderStyles();

    //! Provides accessors for text annotation seeds.
    DGNPLATFORM_EXPORT struct DgnTextAnnotationSeeds& TextAnnotationSeeds();

}; // DgnStyles

//=======================================================================================
//! The provenance of an element. See DgnProvenances::QueryProvenance.
//=======================================================================================
struct ElementProvenance
{
private:
    Int64 m_fileId;
    Int64 m_elementId;

public:
    //! Constructs an invalid provenance object
    ElementProvenance () : m_fileId(0), m_elementId(0) {;}

    //! Constructs a provenance object from previously derived provenance information.
    //! @remarks This constructor is rarely used, because, normally, the original identifiers
    //! required by this constructor are acquired by calling DgnProvenances::QueryProvenance,
    //! which returns an ElementProvenance object.
    //! @param originalFileid   Identifies the original (external) file.
    //! @param originalElementId   Identifies the original element in the original file.
    ElementProvenance (Int64 originalFileid, Int64 originalElementId) : m_fileId(originalFileid), m_elementId(originalElementId) {;}

    //! Is the provenance defined? If false, that means that we do not have a record of where the element came from.
    bool IsValid() const {return m_elementId != 0;}

    //! Returns the element's identifier in its original source.
    Int64 GetOriginalElementId() const {return m_elementId;}

    //! Returns the identifier of the file that contained the element in its original source.
    Int64 GetOriginalFileId() const {return m_fileId;}
};

//=======================================================================================
//! The provenance of elements in a DgnProject. Provenance describes where an element came
//! from. Provenance in a DgnProject links the elements in the project to the data source
//! from which they were extracted. Provenance is not a complete history. The data source
//! used to create this DgnProject might have been created from some other source. That link
//! is not recorded here. Not all elements have provenance. If an element is created in the
//! DgnProject from user-supplied information, for example, it will have no provenance records
//! in this table.
//! @see DgnProject::Provenance
//=======================================================================================
struct DgnProvenances : public DgnProjectTable
{
private:
    friend struct DgnProject;

    explicit DgnProvenances (DgnProject& p) : DgnProjectTable(p) {;}

public:
    //! Look up an element by its provenance.
    //! @param[out] provenance  The element's provenance.
    //! @return The ID of the element in the DgnProject, if found.
    DGNPLATFORM_EXPORT ElementId GetElement (ElementProvenance const& provenance);

    //! Look up an element's provenance.
    //! @param[in] newElementId The element in the DgnProject to research.
    //! @return ElementProvenance, will be invalid if the element has no provenance.
    DGNPLATFORM_EXPORT ElementProvenance QueryProvenance (ElementId newElementId);

    //! Look up the original name of a file
    //! @param[out] originalFileName  The file's original name, if found.
    //! @param[in] originalFileId     The file's provenance identifier.
    //! @return non-zero error status if the file has no provenance.
    DGNPLATFORM_EXPORT BentleyStatus QueryFileName (Utf8StringR originalFileName, Int64 originalFileId);

    //! Look up the provenance identifier of the file by its original name.
    //! @param[out] originalFileId     The file's provenance identifier.
    //! @param[in] originalFileName  The file's original name, if found.
    //! @return non-zero error status if no file by that name is recorded in the provenance table.
    DGNPLATFORM_EXPORT BentleyStatus QueryFileId (UInt64& originalFileId, Utf8CP originalFileName);

};

//=======================================================================================
//! Utilities for querying the dgnPrj_ViewGeneratedDrawing table. Some drawing models
//! are generated from graphics defined by a view. This table records the relationship
//! between the defining view and the drawing model generated from it.
//=======================================================================================
struct DgnViewGeneratedDrawings : DgnProjectTable
    {
private:
    friend struct DgnProject;

    explicit DgnViewGeneratedDrawings (DgnProject& p) : DgnProjectTable(p) {;}

public:
    //! Record the fact that the specified model was generated by the specified view.
    //! @param modelId          The generated model
    //! @param viewId           The view used to generate the model.
    //! @return BE_SQLITE_ROW if the insert succeeded; non-zero error code otherwise
    DGNPLATFORM_EXPORT BeSQLite::DbResult Insert (DgnModelId modelId, DgnViewId viewId) const;

    //! Query the models that were generated from the specified view.
    //! @param viewId           The view used to generate the models.
    //! @param The view that was used to generate the specified model or an invalid ID if no source view was found.
    DGNPLATFORM_EXPORT bvector<DgnModelId> QueryGeneratedModels (DgnViewId viewId) const;

    //! Query the view that was used to generate the specified model
    //! @param modelId          The generated model
    //! @param The view that was used to generate the specified model or an invalid ID if no source view was found.
    DGNPLATFORM_EXPORT DgnViewId QuerySourceView (DgnModelId modelId) const;

    //! Works backward from physical models to the drawings that were generated from them.
    //! Specifically, this function returns a statement that will return the drawing models that were generated from any model in \a pmodels.
    //! @see GetRelatedDrawingViews
    //! @param[out] dmodelsStmt  The prepared statement
    //! @param pmodels          The set of source models
    //! @param cols             The dgnPrj_View table columns to select in the returned statement
    //! @return non-zero error status if the statement could not be prepared.
    //! @Note The statement returned on success is not guaranteed to return any views.
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryGeneratedModels (BeSQLite::Statement& dmodelsStmt, bset<DgnModelId> const& pmodels, char const* cols) const;

    //! Works backward from physical models to the drawings that were generated from them.
    //! Specifically, this function returns a statement that will return the drawing views that target the drawing models that were generated from any model in \a pmodels.
    //! @see GetRelatedDrawingModels
    //! @param[out] dviewsStmt  The prepared statement
    //! @param pmodels          The set of source models
    //! @param cols             The dgnPrj_View table columns to select in the returned statement
    //! @return non-zero error status if the statement could not be prepared.
    //! @Note The statement returned on success is not guaranteed to return any views.
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryViewsOfGeneratedModels (BeSQLite::Statement& dviewsStmt, bset<DgnModelId> const& pmodels, char const* cols) const;

    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
