/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeSyncInfoFile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <iModelBridge/iModelBridge.h>
#include <Bentley/md5.h>
#include <BeRapidJson/BeRapidJson.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/** @addtogroup GROUP_syncinfo 

"SyncInfo" is a strategy or pattern for saving information to help with change detection. 
The term "syncinfo" is short for "synchronization information." When a bridge converts a data source
to a BIM, it must save information about how the source data was mapped to elements in an iModel. Later,
when called to detect changes in the source and to update the BIM, the bridge must use this saved information
to look up the elements in the BIM that must be updated. 

Syncinfo is specific to a single job.

A bridge may implement syncinfo in whatever way best fits the data source. 

\ref iModelBridgeSyncInfoFile is a general-purpose implementation of syncinfo.

Change detection is closely related to syncinfo. If the datasource itself does not track changes, a bridge 
must detect changes by comparing the current state of an item in a data source with information that it recorded in syncinfo.
Similarly, a bridge must have a way of detecting when source items are deleted.
It is a good design to encapsulate change detection logic in a "change detector" class that is 
tied to its implementation of syncinfo. The bridge then creates and employs an instance of its change detector class 
to detect new or changed items as it traverses the source dataset. The change detector compares the source items 
that it sees with data recorded in syncinfo. After traversing the entire source dataset, the change detector can then 
infer deletions by comparing what was seen to what was previously recorded in syncinfo.

\ref iModelBridgeSyncInfoFile::ChangeDetector is an implementation of a change detector that is tied to the iModelBridgeSyncInfoFile 
implementation of syncinfo.

@anchor ANCHOR_MutiFileTransaction
<h2>Syncinfo Integrity</h2>

A bridge must guarantee that its syncinfo matches its BIM. In particular, the bridge must ensure that it updates
syncinfo when it updates its BIM. If the BIM update fails, then syncinfo should not be updated. One
reliable way to guarantee this is to implement syncinfo as a SQLite database and to @em attach it to the BIM
during a conversion. SQLite will then update the BIM and the attached syncinfo db together or not at all as part 
of an atomic multi-file transaction.

Note that if a bridge attaches a syncinfo db to the BIM, this must be done in the iModelBridge::_OnConvertToBim
method and not in its _ConvertToBim method.

\ref iModelBridgeSyncInfoFile uses the SQLite attachment mechanism to guarantee transactional integrity.

*/

/**
* An implementation of the "syncinfo" pattern, along with a supporting change-detector implementation.
* This implementation stores mappings between "items" in the source data and BIM elements.
* This implementation can be used for many different kinds of source data formats.
* It is most useful for source formats that do not have change tracking of their own.
*
* The key class is iModelBridgeSyncInfoFile::ChangeDetector, which normally handles all access to the
* syncinfo file for the bridge. This is explained in more detail below.
*
* iModelBridgeSyncInfoFile stores mappings in a local SQLite file
* and uses the @ref ANCHOR_MutiFileTransaction "attachment" mechanism to guarantee that it always
* matches the BIM.
* 
* A syncinfo file must be specific to a single @ref ANCHOR_iModelBridgeJobOverview "bridge job".
*
* <h2>Source "Items"</h2>
* 
* The design of iModelBridgeSyncInfoFile is based on the idea that source data files contain "items" that 
* the bridge converts to elements.
*
* The concept of an "item" is very general, so that bridges can present the information stored in many
* different kinds of source data formats as items. The only requirements are that an item:
* -# can be identified uniquely in the source data files over time,
* -# can compute a unique hash value that captures the state of its data.
*
* If a source item has actual stable identifier in the source data that will be used in syncinfo mappings.
* If not, the item's data hash will be used as an identity.
*
* The bridge must implement the iModelBridgeSyncInfoFile::ISourceItem interface in order to
* present source items to iModelBridgeSyncInfoFile and iModelBridgeSyncInfoFile::ChangeDetector.
* The ISourceItem's job is to know how to access the item's data and possibly its native ID and to present
* them to syncinfo as strings.
*
* Here is an example where items do not have stable IDs of their own. In this case, the ISourceItem
* just has to return a hash of the item's data.
* @code
* struct ExampleSourceItem : iModelBridgeSyncInfoFile::ISourceItem
    {
    MySourceData* m_data;       // An item in the source data format.
    ExampleSourceItem(MySourceData* data) : m_data(data) {}

    Utf8String _GetId() override  {return "";}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override
        {
        SHA1 sha1;
        sha1(m_data.GetData(), m_data.GetDataSize()); ... compute a hash of the item's data ...
        return sha1.GetHashString();
        }
    };
* @endcode
* 
* Note that a bridge can have many different implementations of iModelBridgeSyncInfoFile::ISourceItem to match
* different kinds of source data. Or, possibly a single generic implementation could handle the job of computing
* hashes, and the bridge could use the source "kind" parameter to differentiate different kinds of items. It's up
* to the bridge to decide which approach will work best.
*
* <h3>Item Scoping</h3>
*
* iModelBridgeSyncInfoFile also supports the concept that one item can be, logically, contained in another item.
* That concept can be used to capture source format constructs such as a file containing models, or models
* containing objects, or a parent object having children. 
*
* iModelBridgeSyncInfoFile considers an item's scope to be part if its identity. Therefore, as far as iModelBridgeSyncInfoFile
* is concerned, an item's ID (if it has one) only has to be unique within its scope.
*
* Also, ChangeDetector understands that if a parent item has been marked as unchanged, then all of its
* child items are unchanged. Or, if it infers that an item was deleted, then it assumes that all of of the child items were deleted.
*
* @anchor ANCHOR_TypicalBridgeConversionLogic
* <h2>Typical Bridge Conversion Logic</h2>
*
* A bridge will typically use its ChangeDetector to guide conversion and manage SyncInfo.
* - The bridge will call ChangeDetector to detect changes.
* - The bridge will convert items that have changed or are new, and then
* - The bridge will call ChangeDetector to write the conversion results to SyncInfo and the BIM.
*
* <h3>Use iModelBridgeWithSyncInfoBase to make things easy</h3>
* Most bridges will not need to write any custom logic at all to get SyncInfo or a ChangeDetector. They can just derive from 
* iModelBridgeWithSyncInfoBase, which will provide syncinfo. A typical bridge will be defined like this:
* @code
* struct MyBridge : iModelBridgeWithSyncInfoBase
*   {
*   // iModelBridge methods that I must override:
*   BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
*   BentleyStatus _Initialize(int argc, WCharCP argv[]) override; // (not shown)
*   BentleyStatus _OpenSource() override;       // (not shown)
*   void _CloseSource(BentleyStatus) override;  // (not shown)
*   Dgn::SubjectCPtr _InitializeJob() override; // (not shown)
*   Dgn::SubjectCPtr _FindJob() override;       // (not shown)
*
*   // My internal methods
*   BentleyStatus ConvertMyItem(void* nativeItem, iModelBridgeSyncInfoFile::ROWID scopingItem, Utf8CP sourceKind, DgnModelR model, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector); // Convert this item
*   DgnModelPtr DetermineModelForMyItem(Dgn::SubjectCR, ExampleSourceItem&);  // Pick a destination model when converting this item
*   }
* @endcode 
*
* <h3>1:1 Conversion Pattern</h3>
* The conversion logic of a typical 1:1 bridge looks like this:
* @code
* // Convert the native items in source data files to the BIM. Organize all converted content under your job subject.
* BentleyStatus MyBridge::_ConvertToBim(Dgn::SubjectCR jobSubject)
*       {
*       auto changeDetector = GetSyncInfo().GetChangeDetectorFor(*this);    // Ask iModelSyncInfoFile for the appropriate change detector to use.
*       for each "native item" in the source                                // Navigate through the data source files
*           {
*           iModelBridgeSyncInfoFile::ROWID scopeItem = ...;                // @see iModelBridgeSyncInfoFile::SourceIdentity
*           Utf8CP sourceKind = "abc";                                      // @see iModelBridgeSyncInfoFile::SourceIdentity
*           DgnModelPtr model = DetermineModelForMyItem(jobSubject, nativeitem);  // organize your models under your jobSubject
*           ConvertMyItem(nativeItem, scopeItem, sourceKind, *model, *changeDetector); // Convert the item to one or more elements in the BIM
*           // Note that the bridge does not stop if conversion of a single item fails. Instead, it logs the details and keeps on going! 
*           // A bridge should always succeed!
*           }
*       changeDetector->_DeleteElementsNotSeenInScopes(scopes);                     // Infer deletions
*       return BentleyStatus::SUCCESS;  // a bridge should always succeed, even in the face of data errors.
*       }
*  @endcode
*
* The logic to convert an item into one or more elements follows a pattern, too. It begins by asking the ChangeDetector if the source item 
* has changed. If a change is detected, then the bridge runs its conversion logic, but does not write directly to the BIM. Instead, 
* it finished by asking the ChangeDetector to process the results of the conversion. This pattern is 
* typical of all converters. Only the conversion logic will vary. The following is a typical example:
* @code
* // Convert an item to one or more BIM elements.
* // nativeItem is an item from the source data files that is to be converted
* // scopingItem is the parent or container of that item. The parent must have been converted to BIM and written to SyncInfo already.
* // sourceKind is a string that you assign to your items for SyncInfo purposes only.
* // model is the model write to in the BIM
* // changeDetector is the ChangeDetector to use.
* BentleyStatus MyBridge::ConvertMyItem(void* nativeItem, iModelBridgeSyncInfoFile::ROWID scopingItem, Utf8CP sourceKind, DgnModelR model, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
*       {
*       // Wrap the native item in your ISourceItem implementaiton, so that ChangeDetector can deal with it
*       ExampleSourceItem item(nativeItem);
*
*       // See if the item is new or has changed.
*       auto change = changeDetector._DetectChange(scopingItem, sourceKind, item);
*       if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
*           {
*           // the item has already been converted and has not changed in the source
*           changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
*           return BentleyStatus::SUCCESS; 
*           }
*       
*       // The item is new or has changed in the source. Convert it. YOu could also convert this item to more than one element, perhaps as an assembly.
*       DgnElementPtr convertedElement = ConvertMyItemToAnElement(model, item);
*       
*       // write the converted element to the BIM and to syncinfo
*       iModelBridgeSyncInfoFile::ConversionResults results;
*       results.m_element = convertedElement;                           // If this were an assembly, you would put the children in the iModelBridgeSyncInfoFile::ConversionResults::m_childElements member.
*       return changeDetector._UpdateBimAndSyncInfo(results, change);
*       }
*
*  @endcode
*
* Note that the conversion logic can do an early return if ChangeDetector::_DetectChange reports no change.
* If it does, it must call ChangeDetector::_OnElementSeen. That tells the
* change detector that the item was detected in the source. The bridge could also call ChangeDetector::_UpdateBimAndSyncInfo,
* which, in this case, will not actually write anything to the BIM, but will mark the item as seen.
* As noted above, if the bridge skips an unchanged item (not forgetting to call _OnElementSeen), then the ChangeDetector
* infers that the children of the item were also unchanged.
*
* Later, after it has visited all of the source data, the bridge calls ChangeDetector::_DeleteElementsNotSeenInScopes to 
* delete the elements in the BIM that correspond to items that were not processed by the conversion logic. 
* The change detector is making the inference that the only reason why an item was not seen is because it is not there.
*
* <h2>Attach and Detach</h2>
* A bridge must attach syncinfo in iModelBridge::_OnOpenBim and detach syncinfo in iModelBridge::_OnCloseBim. 
* It must not try to do either of these operations in iModelBridge::_ConvertToBim.
* iModelBridgeWithSyncInfoBase overrides _OnOpenBim and _OnCloseBim in order to handle attach and detach for you.
*
* <h2>Complex Mappings</h2>
*
* A single source item can be mapped to one or more DgnElements. To do that, the bridge 
* would convert the same item to multiple elements and then call changeDetector._UpdateBimAndSyncInfo
* multiple times, each time with the same source item and a different resulting element. 
*
* Or, multiple source items can be mapped to a single DgnElement. To do that, the bridge
* would use the items to generate a single element and the call changeDetector._UpdateBimAndSyncInfo
* multiple times, each time with a differnt source item and the same resulting element.
* 
* A mapping with a 0 DgnElementId indicates a source item that was not mapped into the BIM.
* The bridge can create this kind of mapping by setting iModelBridgeSyncInfoFile::ConversionResults::m_element to
* null before calling _UpdateBimAndSyncInfo. This kind of mapping is useful if the bridge wants to keep
* track of items that it has deliberately excluded from the conversion.
*
*  @ingroup GROUP_iModelBridgeSyncInfoFile
* @bsiclass                                    BentleySystems 
*/

//! iModelExternalSourceAspect holds information that identifies a piece of data in a source repository. It is part of a mapping of that data to an element in an iModel. It also
//! contains enough information to enable a bridge to detect changes in the source data and to update the related element.
//! The "kind" property is used by source-specific bridges to distinguish different kinds of data. The meaning of this property is known only to the bridge.
//! The "sourceId" property is source-specific bridges to identify items uniquely in the source repository. The information required to form an identifier is bridge-specific.
//! The "scope" property identifies the element in the iModel (not the source) that corresponds to the scope or container in the source that owns, contains, or qualifies the identity of the source item.
//! Note that the scope property does not hold an identifier of any item in the source repository. Rather, it holds the ElementId of the element in the iModel that represents the source container.
//!
//! Optional properties:
//!
//! SourceState
//! The "lastModHash" - Optional value that is a version number or last modified time -- something quick that will allow the synchronizer to detect that an item is unchanged and avoid computing 
//! a cryptographic hash. If present, this value must be guaranteed to change when *any* of the source item's content changes. If LastModHash is non-empty and if the current value equals the stored 
//! value, then a bridge will conclude that the item is unchanged. Otherwise, the bridge will ask for and use the cryptographic hash.
//! The "hash" property must capture the state of the source item's content. Its value must be guaranteed to be unique to the content. A cryptographic hash or message digest is ideal for this purpose.
//! For very small items, the item's state itself would also work.
//!
//!
//! The "properties" property is for the private use of the bridge to store additional qualifying data on a mapping. This can be used, for example to capture a transform in the 
//! case where a single item in the source is intstanced multiple times in the iModel, each instance being transformed in some way. It is recommended that the bridge store auxilliary properties
//! in JSON format, but that is up to the bridge.
struct iModelExternalSourceAspect
    {
    //! Utility function to convert a fixed-length byte array to its hex string representation. This is useful when 
    //! converting an MD5 or SHA1 hash value to a string in order to store it. This is a common operation.
    template<size_t BYTE_COUNT> static void HexStrFromBytes(Utf8StringR str, const Byte (&bytes)[BYTE_COUNT])
        {
        str.clear();
        str.reserve(2 * BYTE_COUNT);
        for (size_t i = 0; i < BYTE_COUNT; i++)
            {
            static constexpr char* dec2hex = "0123456789abcdef";
            str += dec2hex[(bytes[i] >> 4) & 15];
            str += dec2hex[ bytes[i]       & 15];
            }
        }

    //! Utility function to convert a hex string representation of a byte array to a fixed-length byte array. This is useful when 
    //! converting a stored hash value back to a MD5 or SHA1 binary hash value. This should be rarely needed, as hash values can
    //! be compared for equality in their stringified form.
    template<size_t BYTE_COUNT> static void HextStrToBytes(Byte (&bytes)[BYTE_COUNT], Utf8StringCR str)
        {
        if (str.size() != 2 * BYTE_COUNT)
            {
            BeAssert(false);
            return;
            }
        std::transform(str.begin(), str.end(), bytes, [](Utf8Char ch) {return isdigit(ch) ? ch - '0' : static_cast <char> (tolower(ch)) - 'a' + 10;});
        }

    //! Generate a string representation of a double, without losing any precision
    static void DoubleToString(Utf8StringR str, double v) {str.Sprintf("%.17g", v);}
    //! Reconstruct a double by parsing a string representation
    static double DoubleFromString(Utf8CP str) {return ::atof(str);}

    //! Useful when writing uint64_t to JSON 
    static void UInt64ToString(Utf8StringR str, uint64_t v)
        {
        str.Sprintf("%llu", v);
        }
    static Utf8String UInt64ToString(uint64_t v)
        {
        Utf8String s;
        UInt64ToString(s, v);
        return s;
        }

    //! Useful when deserializing uint64_t previously written as a string
    static uint64_t UInt64FromString(Utf8CP str)
        {
        uint64_t v;
        return (1 == sscanf(str, "%llu", &v))? v: 0;
        }

    struct SourceState
        {
        Utf8String m_checksum;
        Utf8String m_version;
        };

    RefCountedPtr<ECN::IECInstance> m_instance;

    public:

    iModelExternalSourceAspect(ECN::IECInstance* i = nullptr) :m_instance(i) {}
    iModelExternalSourceAspect(ECN::IECInstance const* i) : m_instance(const_cast<ECN::IECInstance*>(i)) {}
 
    bool IsValid() const {return m_instance.IsValid();}
    void Invalidate() {m_instance = nullptr;}

    bool operator!=(iModelExternalSourceAspect const& rhs) const {return m_instance == rhs.m_instance;}
    bool operator==(iModelExternalSourceAspect const& rhs) const {return m_instance != rhs.m_instance;}

    BeSQLite::EC::ECInstanceId GetECInstanceId() const { return BeSQLite::EC::ECInstanceId(BeSQLite::EC::ECInstanceId::FromString(m_instance->GetInstanceId().c_str()).GetValueUnchecked()); }
    IMODEL_BRIDGE_EXPORT Utf8String GetKind() const;
    IMODEL_BRIDGE_EXPORT DgnElementId GetElementId() const;
    IMODEL_BRIDGE_EXPORT DgnElementId GetScope() const;
    IMODEL_BRIDGE_EXPORT Utf8String GetIdentifier() const;
    IMODEL_BRIDGE_EXPORT SourceState GetSourceState() const;
    IMODEL_BRIDGE_EXPORT rapidjson::Document GetProperties() const;

    IMODEL_BRIDGE_EXPORT void SetProperties(rapidjson::Document const&); //!< Update the custom properties
    IMODEL_BRIDGE_EXPORT void SetSourceState(SourceState const& ss) {SetSourceState(*m_instance, ss);} //!< Update the state-tracking properties of the ECInstance
    
    //! Add this aspect to the specified element. (Caller must then call element's Insert or Update method.)
    IMODEL_BRIDGE_EXPORT DgnDbStatus AddAspect(DgnElementR);

    //! Get the ExternalSourceAspect ECClass
    IMODEL_BRIDGE_EXPORT static ECN::ECClassCP GetAspectClass(DgnDbR);

    //! Get an existing ExternalSourceAspect from the specified element in order to view the aspect
    IMODEL_BRIDGE_EXPORT static iModelExternalSourceAspect GetAspect(DgnElementCR el, BeSQLite::EC::ECInstanceId id, ECN::ECClassCP aspectClass = nullptr);
    //! Get an existing ExternalSourceAspect from the specified element in order to update the aspect (and then the element)
    IMODEL_BRIDGE_EXPORT static iModelExternalSourceAspect GetAspectForEdit(DgnElementR el, BeSQLite::EC::ECInstanceId id, ECN::ECClassCP aspectClass = nullptr);

    //! Look up an aspect by Scope, Kind, and SourceId
    IMODEL_BRIDGE_EXPORT static iModelExternalSourceAspect GetAspectBySourceId(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR sourceId);

    //! Create a new ECInstance of the specified provenance aspect class
    //! Bridges will probably want factory methods with signatures that are customized to the bridge.
    IMODEL_BRIDGE_EXPORT static ECN::IECInstancePtr CreateInstance(DgnElementId scope, Utf8CP kind, Utf8StringCR sourceId, SourceState const*, ECN::ECClassCR aspectClass);

    //! Set the SourceState of this aspect in memory
    IMODEL_BRIDGE_EXPORT static void SetSourceState(ECN::IECInstanceR, SourceState const&);

    struct ElementAndAspectId
        {
        DgnElementId elementId;
        BeSQLite::EC::ECInstanceId aspectId;
        };

    //! Look up the ElementId of the element that contains an aspect with the specified Scope, Kind, and Identifier. Also returns the aspect's instanceid.
    IMODEL_BRIDGE_EXPORT static ElementAndAspectId FindElementBySourceId(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR sourceId);

    IMODEL_BRIDGE_EXPORT static ElementAndAspectId FindElementByHash(DgnDbR db, DgnElementId scopeId, Utf8CP kind, Utf8StringCR sourceId);

    //! Look up all aspects on the specified element
    IMODEL_BRIDGE_EXPORT static bvector<iModelExternalSourceAspect> GetAll(DgnElementCR, ECN::ECClassCP aspectClass = nullptr);

    //! Look up all aspects on the specified element by Kind
    IMODEL_BRIDGE_EXPORT static bvector<iModelExternalSourceAspect> GetAllByKind(DgnElementCR, Utf8CP kind, ECN::ECClassCP aspectClass = nullptr);

    //! Return a statement that finds all aspects with the specified scope, identifier, and kind and returns the Element.Id and ECInstanceId of each
    static BeSQLite::EC::CachedECSqlStatementPtr GetSelect(DgnDbR db, Utf8StringCR kind, DgnElementId scope, Utf8StringCR identifier, BeSQLite::EC::IECSqlBinder::MakeCopy cc)
        {
        auto allFromSameSource = db.GetPreparedECSqlStatement("SELECT Element.Id, ECInstanceId FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE " XTRN_SRC_ASPCT_Identifier "=? AND " XTRN_SRC_ASPCT_Scope ".Id=? AND " XTRN_SRC_ASPCT_Kind "=?");
        auto rc = allFromSameSource->BindText(1, identifier.c_str(), cc); BeAssert(rc.IsSuccess());
        rc = allFromSameSource->BindId(2, scope); BeAssert(rc.IsSuccess());
        rc = allFromSameSource->BindText(3, kind.c_str(), cc); BeAssert(rc.IsSuccess());
        return allFromSameSource;
        }

    //! Return a statement that finds all aspects from the same source as the specified aspect and returns the Element.Id and ECInstanceId of each
    static BeSQLite::EC::CachedECSqlStatementPtr GetSelectFromSameSource(DgnDbR db, iModelExternalSourceAspect const& aspect, BeSQLite::EC::IECSqlBinder::MakeCopy cc = BeSQLite::EC::IECSqlBinder::MakeCopy::Yes)
        {
        return GetSelect(db, aspect.GetKind(), aspect.GetScope(), aspect.GetIdentifier(), cc);
        }

    IMODEL_BRIDGE_EXPORT Utf8String FormatForDump(DgnDbR, bool includeProperties, bool includeSourceState) const;
    IMODEL_BRIDGE_EXPORT static Utf8String GetDumpHeaders(bool includeProperties, bool includeSourceState);
    IMODEL_BRIDGE_EXPORT static void Dump(DgnElementCR el, Utf8CP loggingCategory, NativeLogging::SEVERITY, bool includeProperties = true, bool includeSourceState = false);
    };

//=======================================================================================
// Identifies an iModelExternalSourceAspect on an element.
// @bsiclass                                                    
//=======================================================================================
#ifdef COMMENT_OUT_NOT_USED
struct iModelExternalSourceAspectID
    {
    DgnElementId m_elementId;
    BeSQLite::EC::ECInstanceId m_aspectId;

    iModelExternalSourceAspectID()  {}
    iModelExternalSourceAspectID(DgnElementId eid, BeSQLite::EC::ECInstanceId aid) : m_elementId(eid), m_aspectId(aid) {BeAssert(m_elementId.IsValid()); BeAssert(m_aspectId.IsValid());}
    iModelExternalSourceAspectID(iModelExternalSourceAspect const& aspect) : m_elementId(aspect.GetElementId()), m_aspectId(aspect.GetECInstanceId()) {BeAssert(m_elementId.IsValid()); BeAssert(m_aspectId.IsValid());}

    bool IsValid() const {return m_elementId.IsValid();}

    bool operator==(iModelExternalSourceAspectID const& rhs) const {return m_elementId == rhs.m_elementId && m_aspectId == rhs.m_aspectId;}
    bool operator!=(iModelExternalSourceAspectID const& rhs) const {return !(*this == rhs);}
    bool operator <(iModelExternalSourceAspectID const& rhs) const {if (m_elementId.GetValue() < rhs.m_elementId.GetValue()) return true; if (m_elementId.GetValue() > rhs.m_elementId.GetValue()) return false; return m_aspectId < rhs.m_aspectId;}
    };
#endif

//=======================================================================================
//! Helper class for stepping through all ExternalSourceAspects in a specified Scope
//! with optional WHERE clause. 
//! NB: It is generally unsafe to use positional parameter binding. Use named parameters
//! and call GetParameterIndex instead.
// @bsiclass                                                    
//=======================================================================================
template<typename T>
struct ExternalSourceAspectIterator : NonCopyableClass
{
    struct Entry;
    friend struct Entry;

protected:
    DgnDbR m_db;
    BeSQLite::EC::CachedECSqlStatementPtr m_stmt;
    mutable iModelExternalSourceAspect m_aspect;

    BeSQLite::DbResult Step() const
        {
        if (!m_stmt.IsValid())
            return BeSQLite::BE_SQLITE_DONE;

        auto rc = m_stmt->Step();
        if (BeSQLite::BE_SQLITE_ROW !=  rc)
            {
            m_aspect.Invalidate();
            return rc;
            }

        auto el = m_db.Elements().GetElement(m_stmt->GetValueId<DgnElementId>(1));
        if (!el.IsValid())
            {
            BeAssert(false);
            m_aspect.Invalidate();
            return BeSQLite::BE_SQLITE_ERROR;
            }
        m_aspect = iModelExternalSourceAspect::GetAspect(*el, m_stmt->GetValueId<BeSQLite::EC::ECInstanceId>(0));
        return BeSQLite::BE_SQLITE_ROW;
        }

public:
    ExternalSourceAspectIterator(DgnDbR db, DgnElementId scope, Utf8CP kind, Utf8StringCR wh) : m_db(db)
        {
        Utf8String ecsql("SELECT ECInstanceId, Element.Id FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE ( (Scope.Id=?) AND (Kind=?)");
        if (!wh.empty())
            ecsql.append(" AND (").append(wh).append(" )");
        ecsql.append(" )");

        m_stmt = db.GetPreparedECSqlStatement(ecsql.c_str());

        if (!m_stmt.IsValid())
            return;

        m_stmt->BindId(1, scope);
        m_stmt->BindText(2, kind, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        }

    //! Move ctor. ExternalSourceAspectIterator are not copyable, but they are movable.
    ExternalSourceAspectIterator(ExternalSourceAspectIterator&& rhs) : m_stmt(std::move(rhs.m_stmt)), m_aspect(rhs.m_aspect), m_db(rhs.m_db) {}

    struct Entry : std::iterator<std::input_iterator_tag, T const>
    {
        friend struct ExternalSourceAspectIterator;
    protected:
        ExternalSourceAspectIterator const* m_it;
        Entry(ExternalSourceAspectIterator const* it) : m_it(it) {Step();}
        void Step()
            {
            if (m_it) 
                {
                if (BeSQLite::BE_SQLITE_ROW != m_it->Step()) 
                    m_it = nullptr;
                }
            }
    public:
        Entry& operator++() {Step(); return *this;}
        bool operator!=(Entry const& rhs) const {return !(*this == rhs);}
        bool operator==(Entry const& rhs) const {return m_it == rhs.m_it;}
        T const& operator*() const {return (T const&)(m_it->m_aspect);}
        T const* operator->() const {return (T const*)(&m_it->m_aspect);}
    };

    typedef Entry const_iterator;
    typedef const_iterator iterator;
    const_iterator begin() const {return Entry(this);}
    const_iterator end() const {return Entry(nullptr);}

    //! Get the prepared statement for this iterator. This can be used to bind parameters before calling /c begin.
    BeSQLite::EC::CachedECSqlStatement* GetStatement() {return m_stmt.get();}

    //! Get the parameter index of the specified named parameter. Note: you should not include the parameter prefix character in the name of the parameter.
    int GetParameterIndex(Utf8CP pname) {BeAssert((*pname != ':') && (*pname != '@') && (*pname != '$')); auto i = GetStatement()->GetParameterIndex(pname); BeAssert(i != -1); return i;}
};

struct EXPORT_VTABLE_ATTRIBUTE iModelBridgeSyncInfoFile
{
    typedef uint64_t ROWID;

    //! The type of change operation
    enum class ChangeOperation {Create, Update, Delete};

    //=======================================================================================
    // @bsiclass                                                    Sam.Wilson  07/14
    //=======================================================================================
    struct SyncInfoProperty
    {
        struct Spec : BeSQLite::PropertySpec
            {
            Spec(Utf8CP name) : PropertySpec(name, "SyncInfo", PropertySpec::Mode::Normal, PropertySpec::Compress::No) {}
            };

        static Spec ProfileVersion()       {return Spec("SchemaVersion");}
        static Spec DgnDbGuid()            {return Spec("DgnDbGuid");}
        static Spec DbProfileVersion()     {return Spec("DbSchemaVersion");}
        static Spec DgnDbProfileVersion()  {return Spec("DgnDbSchemaVersion");}
    };

    //=======================================================================================
    //! The identity of an item in the source repository
    //! - scope ROWID.    The ROWID in syncinfo of the item that is the scope/container/parent of the item. This is optional. 
    //! - kind.          A mark that is assigned to the item by the caller that helps the caller differentiate different kinds of items and to make IDs unique. This value is specific to this one syncinfo file. This value is meaninful only to the source repository. This is optional. 
    //! - Id.            The ID of the item, unique within its scope and kind. The ID is presumably assigned by the source repository. This is optional. If no ID is available, then the hash is used as an item's ID./
    //! An item @em may be uniquely identified in the source repository by the combination of (SourceROWID, Kind, and ID).
    //! An item @em may have no unique ID in the source repository. In that case, the iModelBridgeSyncInfoFile::SourceState hash value is used to identify the item.
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct SourceIdentity
        {
      private:
        ROWID m_scopeROWID;
        Utf8String m_kind, m_id;

      public:
        SourceIdentity() : m_scopeROWID(0) {}
        SourceIdentity(ROWID srid, Utf8StringCR k, Utf8StringCR i) 
            : m_scopeROWID(srid), m_kind(k), m_id(i) {}

        //! Scope ROWID
        ROWID GetScopeROWID() const {return m_scopeROWID;}
        //! Classification of the source item.
        Utf8StringCR GetKind() const {return m_kind;}
        //! Unique Identity of the source item.
        Utf8StringCR GetId() const {return m_id;}
        //! Compare for equality
        bool operator==(SourceIdentity const& rhs) const {return (GetScopeROWID() == rhs.GetScopeROWID()) && (GetKind() == rhs.GetKind()) && (GetId() == rhs.GetId());}
        };

    //=======================================================================================
    //! The state of the item in the source repository
    //! - hash.                      The cryptographic hash of the item's content. SyncInfo assumes that if the current hash of a
    //!                             source item matches the stored value, then the item's content is unchanged. To compute this value, the implementor can use any algorithm that 
    //!                             does not produce the same value for different contents. <em>This is required.</em>
    //! - Last modified time (LMT).  The last "time" that the item's content was modified, if known. SyncInfo assumes that if the current LMT of a source item
    //!                             is equal to the stored value, then the item's content is unchanged. 
    //!                             The source repository must guarantee this! If in doubt, \ref iModelBridgeSyncInfoFile::ISourceItem should return 0 for LMT. Note that the definition of this value is known only to the source repository.
    //!                             It may be a real timestamp or just a counter. SyncInfo just needs to check for equality.
    //! @note The hash value is the only required field. Hash can serve as the item's identity (relative to scope and kind) in the soure repository if it has no other identity.
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct SourceState
        {
      private:
        Utf8String m_hash;
        double m_lmt;

      public:
        SourceState() : m_lmt(0) {}
        SourceState(double d, Utf8StringCR h) 
            : m_lmt(d), m_hash(h) {}
        
        IMODEL_BRIDGE_EXPORT SourceState(iModelExternalSourceAspect::SourceState aspectState);

        //! The last "time" the source item was created or modified. The definition of this value is known only to source repository.
        double GetLastModifiedTime() const {return m_lmt;}
        //! The cryptographic hash of the contents of the source item. The definition and method of computing this value is known only to source repository.
        Utf8StringCR GetHash() const  {return m_hash;}

        iModelExternalSourceAspect::SourceState GetAspectState() const;
        };

    //=======================================================================================
    //! Interface for presenting an item in the source repository to SyncInfo and its ChangeDetector
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct ISourceItem
        {
        //! Unique Identity of the source item (relative to its scope and kind).
        virtual Utf8String _GetId() = 0;
        //! The last "time" the source item was created or modified. The definition of this value is known only to source repository.
        virtual double _GetLastModifiedTime() = 0;
        //! The cryptographic hash of the contents of the source item. The definition and method of computing this value is known only to source repository.
        virtual Utf8String _GetHash() = 0;
        };

    //=======================================================================================
    //! Information about an item that is stored in SyncInfo
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct Record
        {
      private:
        friend struct iModelBridgeSyncInfoFile;

        ROWID m_ROWID;
        DgnElementId m_elementId;
        SourceIdentity m_sourceId;
        SourceState m_sourceState;

      public:
        Record() : m_ROWID(0), m_sourceState(0, "") {}
        Record(ROWID rid, DgnElementId eid, SourceIdentity const& sid, SourceState const& sstate) : 
            m_ROWID(rid), m_elementId(eid), m_sourceId(sid), m_sourceState(sstate) {}

        //! Query if this record is valid
        bool IsValid() const {return 0 != m_ROWID;}
        //! Item's ROWID in SyncInfo
        ROWID GetROWID() const {return m_ROWID;}
        //! The element to which this record is mapped
        DgnElementId GetDgnElementId() const {return m_elementId;}
        //! SourceIdentity
        SourceIdentity const& GetSourceIdentity() const {return m_sourceId;}
        //! SourceState
        SourceState const& GetSourceState() const {return m_sourceState;}
        };

    //=======================================================================================
    //! Iterates the syncinfo file's records
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Iterator : BeSQLite::DbTableIterator
        {
        Iterator(DgnDbCR db, Utf8CP where);

        //! Information about an item that is stored in syncinfo
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
          private:
            friend struct Iterator;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql,isValid) {}

          public:
            //! Item's ROWID in SyncInfo
            IMODEL_BRIDGE_EXPORT ROWID GetROWID() const;
            //! The element to which this record is mapped
            IMODEL_BRIDGE_EXPORT DgnElementId GetDgnElementId() const;
            //! SourceIdentity
            IMODEL_BRIDGE_EXPORT SourceIdentity GetSourceIdentity() const;
            //! SourceState
            IMODEL_BRIDGE_EXPORT SourceState GetSourceState() const;
            //! Get the item Record as a whole
            Record GetRecord() const {return Record(GetROWID(), GetDgnElementId(), GetSourceIdentity(), GetSourceState());}

            Entry const& operator* () const {return *this;}
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        IMODEL_BRIDGE_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry (NULL, false);}
        };

    //=======================================================================================
    //! The result of converting an Item to a DgnElement (possibly an assembly)
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct ConversionResults
        {
        DgnElementPtr m_element;  //!< The DgnDb element created to represent the source item -- optional
        bvector<ConversionResults> m_childElements; //!< Child elements - optional
        Record m_syncInfoRecord; //!< Output from _ProcessConversionResults
        };

    //=======================================================================================
    //! Detects changes to source items, as compared with info recorded in syncinfo.
    //! @note You should use the same change detector object for an entire update. 
    // @bsiclass                                    BentleySystems 
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE ChangeDetector : RefCountedBase
        {
        enum class ChangeType {Unchanged, New, Changed};

        struct Results
            {
          private:
            ChangeType m_changeType;
            SourceIdentity m_sourceId;  // Copy of ID info obtained from SourceItem
            SourceState m_currentState;
            Record m_record;
            //friend struct iModelBridgeSyncInfoFile;
          public:
            Results() : m_changeType(ChangeType::New) {}
            Results(SourceIdentity const& si, SourceState const& newState) : m_changeType(ChangeType::New), m_sourceId(si), m_currentState(newState) {}
            Results(ChangeType ct, Record const& rec, SourceState const& currentState) : m_changeType(ct), m_record(rec), m_sourceId(rec.GetSourceIdentity()), m_currentState(currentState) {}

            //! Get the SourceIdentity as cached by the change detector. Note that this is just a copy of data that the detector gets from the input SourceItem.
            SourceIdentity const& GetSourceIdentity() const {return m_sourceId;}
            //! Get the change type detected
            ChangeType GetChangeType() const {return m_changeType;}
            //! Get the existing syncinfo record, if any. Caller must check if record is valid. @see Record::IsValid
            Record const& GetSyncInfoRecord() const {return m_record;}
            //! Get the current state of the source data, as cached by the change detector. @note This may be empty if #GetChangeType returns ChangeType::Unchanged.
            SourceState const& GetCurrentState() const {return m_currentState;}
            };

      private:
        iModelBridgeSyncInfoFile* m_si;
      protected:
        uint32_t            m_elementsConverted = 0;
        
        DgnElementIdSet     m_elementsSeen;
        bset<ROWID>         m_scopesSkipped;

        IMODEL_BRIDGE_EXPORT DgnDbStatus InsertResultsIntoBIM(ConversionResults&);;
        IMODEL_BRIDGE_EXPORT DgnElementPtr MakeCopyForUpdate(DgnElementCR newEl, DgnElementCR originalEl);;
        IMODEL_BRIDGE_EXPORT DgnDbStatus UpdateResultsInBIMForOneElement(ConversionResults& conversionResults, DgnElementId existingElementId);;
        IMODEL_BRIDGE_EXPORT DgnDbStatus UpdateResultsInBIMForChildren(ConversionResults& parentConversionResults);;
        

        //! Invoked just before an an element is deleted
        IMODEL_BRIDGE_EXPORT virtual void _OnItemDelete(Record const&);

     public:

        IMODEL_BRIDGE_EXPORT DgnDbStatus UpdateResultsInBIM(ConversionResults& conversionResults, DgnElementId existingElementId);

        DgnDbStatus AddProvenanceAspect(SourceIdentity const& identity, SourceState const &state, DgnElementR element);
      
        //! @private
        //! Construct a change detector object. This will invoke the iModelBridgeSyncInfoFile::_OnNewUpdate method on @a si
        //! @note You should generally call iModelSyncInfoFile::GetChangeDetectorFor to get a change detector.
        //! @note You should use the same change detector object for an entire update.
        IMODEL_BRIDGE_EXPORT ChangeDetector(iModelBridgeSyncInfoFile* si);

        //! Get a reference to the BIM
        virtual DgnDbR GetDgnDb() {return m_si->GetDgnDb();}

        iModelBridgeSyncInfoFile& GetSyncInfo() {return *m_si;}

        //! Used to choose one of many existing entries in SyncInfo
        typedef std::function<bool(Record const&, iModelBridgeSyncInfoFile& bridge)> T_Filter;

        //! Detect if the item has changed or is new.
        //! @param scope The scoping item
        //! @param kind the kind of source item
        //! @param item the source item
        //! @param filter Optional. How to choose among many records for the same item
        //! @param forceChange Optional. If the item exists in syncinfo, then always report it as changed.
        //! return the results of looking in syncinfo and comparing the existing syncinfo record, if any, with the item's current state.
        IMODEL_BRIDGE_EXPORT virtual Results _DetectChange(ROWID scope, Utf8CP kind, ISourceItem& item, T_Filter* filter = nullptr, bool forceChange = false);

        //! Update the BIM and syncinfo with the results of converting an item to one or more DgnElements.
        //! If changeDetectorResults indicates that the item is new or changed, the conversion writes are written to the BIM and syncinfo is updated.
        //! If changeDetectorResults indicates that the item is known and unchanged, then the BIM is not updated.
        //! In either case, this function will call _OnElementSeen on the DgnElementIds in conversionResults.m_element and its children.
        //! @note If you decide not to call this function to process unchanged items, then you must call _OnElementSeen or _OnScopeSkipped directly.
        IMODEL_BRIDGE_EXPORT virtual BentleyStatus _UpdateBimAndSyncInfo(ConversionResults& conversionResults, ChangeDetector::Results const& changeDetectorResults);

        //! @private
        IMODEL_BRIDGE_EXPORT DgnDbStatus GetLocksAndCodes(DgnElementR);

        //! Get the number of elements converted.
        uint32_t GetElementsConverted() const {return m_elementsConverted;}

        //! Keep track of elements reached
        virtual void _OnElementSeen(DgnElementId eid) {if (eid.IsValid()) m_elementsSeen.insert(eid);}

        //! Invoked when an element is inserted, updated, or deleted
        IMODEL_BRIDGE_EXPORT virtual void _OnItemConverted(Record const&, ChangeOperation);

        //! Call this if you decide to skip an item that might be the scope or parent of other items. This allows
        //! ChangeDetector to keep track of item that were deliberately skipped and not mistakenly infer that they were deleted.
        IMODEL_BRIDGE_EXPORT virtual void _OnScopeSkipped(ROWID srid);

        //! Delete all elements in the BIM that are recorded in syncinfo but are not in the set of elements seen and are not the targets of mappings
        //! in scopes that were skipped as unchanged.
        //! @param onlyInScopes The scopes to consider. Items in all other scopes are left intact.
        IMODEL_BRIDGE_EXPORT virtual void _DeleteElementsNotSeenInScopes(bvector<ROWID> const& onlyInScopes);

        //! Delete all elements in the BIM that are recorded in syncinfo but are not in the set of elements seen and are not the targets of mappings
        //! in scopes that were skipped as unchanged.
        //! @param onlyInScope The scope to consider. Items in all other scopes are left intact.
        void DeleteElementsNotSeenInScope(ROWID onlyInScope) {bvector<ROWID> scopes; scopes.push_back(onlyInScope); _DeleteElementsNotSeenInScopes(scopes);}
        };

    //! A ChangeDetector that can be used in the initial conversion, where all source items are new.
    struct InitialConversionChangeDetector : ChangeDetector
        {
        IMODEL_BRIDGE_EXPORT Results _DetectChange(ROWID scope, Utf8CP kind, ISourceItem& item, T_Filter* filter = nullptr, bool forceChange = false) override;
        void _OnScopeSkipped(ROWID srid) override {BeAssert(false);}//! Nothing should be skipped -- all content goes into the intial conversion. 
        void _OnElementSeen(DgnElementId) override {}               //! No need to keep a list of elements 
        void _DeleteElementsNotSeenInScopes(bvector<iModelBridgeSyncInfoFile::ROWID> const&) override {}                   //! There will be no deletions

        //! Construct a change detector that can be used efficiently by a converter that is doing an initial conversion, such that all items are new to the BIM.
        //! @note You should generally call iModelSyncInfoFile::GetChangeDetectorFor to get a change detector.
        InitialConversionChangeDetector(iModelBridgeSyncInfoFile* si) : ChangeDetector(si) {}
        };

    struct iModelBasedChangeDetector : ChangeDetector
        {
        DgnDbR m_dgnDb;
        virtual DgnDbR GetDgnDb() override { return m_dgnDb; }
        iModelBasedChangeDetector(DgnDbR dgnDb) : ChangeDetector(NULL) , m_dgnDb(dgnDb){}
        //! Detect if the item has changed or is new.
        //! @param scope The scoping item
        //! @param kind the kind of source item
        //! @param item the source item
        //! @param filter Optional. How to choose among many records for the same item
        //! @param forceChange Optional. If the item exists in syncinfo, then always report it as changed.
        //! return the results of looking in syncinfo and comparing the existing syncinfo record, if any, with the item's current state.
        IMODEL_BRIDGE_EXPORT virtual Results _DetectChange(ROWID scope, Utf8CP kind, ISourceItem& item, T_Filter* filter = nullptr, bool forceChange = false);

        //! Update the BIM and syncinfo with the results of converting an item to one or more DgnElements.
        //! If changeDetectorResults indicates that the item is new or changed, the conversion writes are written to the BIM and syncinfo is updated.
        //! If changeDetectorResults indicates that the item is known and unchanged, then the BIM is not updated.
        //! In either case, this function will call _OnElementSeen on the DgnElementIds in conversionResults.m_element and its children.
        //! @note If you decide not to call this function to process unchanged items, then you must call _OnElementSeen or _OnScopeSkipped directly.
        IMODEL_BRIDGE_EXPORT virtual BentleyStatus _UpdateBimAndSyncInfo(ConversionResults& conversionResults, ChangeDetector::Results const& changeDetectorResults);

        //! Delete all elements in the BIM that are recorded in syncinfo but are not in the set of elements seen and are not the targets of mappings
        //! in scopes that were skipped as unchanged.
        //! @param onlyInScopes The scopes to consider. Items in all other scopes are left intact.
        IMODEL_BRIDGE_EXPORT virtual void _DeleteElementsNotSeenInScopes(bvector<ROWID> const& onlyInScopes);
        };
    typedef RefCountedPtr<ChangeDetector> ChangeDetectorPtr;

  protected:
    bool                m_isAttached;
    BeSQLite::DbResult  m_lastError;
    Utf8String          m_lastErrorDescription;
    DgnDbPtr            m_bim;

    IMODEL_BRIDGE_EXPORT BeSQLite::DbResult SavePropertyString (BeSQLite::PropertySpec const& spec, Utf8CP stringData, uint64_t majorId=0, uint64_t subId=0);
    IMODEL_BRIDGE_EXPORT BeSQLite::DbResult QueryProperty (Utf8StringR str, BeSQLite::PropertySpec const& spec, uint64_t id=0, uint64_t subId=0) const;
    IMODEL_BRIDGE_EXPORT BentleyStatus PerformVersionChecks();
    IMODEL_BRIDGE_EXPORT BentleyStatus OnAttach();
    IMODEL_BRIDGE_EXPORT BentleyStatus CreateTables();
    IMODEL_BRIDGE_EXPORT Record FindFirstByElementId(DgnElementId eid);

    IMODEL_BRIDGE_EXPORT void SetLastError(BeSQLite::DbResult rc);

    IMODEL_BRIDGE_EXPORT bool IsSourceItemMappedToAnElementThatWasSeen(Iterator::Entry const&, DgnElementIdSet const&);
    IMODEL_BRIDGE_EXPORT BeSQLite::DbResult WriteRecord(Record&);

  public:
    //! Construct a syncinfo object. @see AttachToBIM
    //! @note iModelBridgeWithSyncInfoBase will construct and manage a syncinfo file for you automatically.
    iModelBridgeSyncInfoFile() : m_bim(nullptr) {}
    
    //! Destructor - detaches from m_bim
    //! @note iModelBridgeWithSyncInfoBase will construct and manage a syncinfo file for you automatically.
    IMODEL_BRIDGE_EXPORT ~iModelBridgeSyncInfoFile();
    
    //! Compute the filepath that a syncinfo file should have
    //! @param bimName   The filepath of the BIM
    IMODEL_BRIDGE_EXPORT static BeFileName ComputeSyncInfoFileName(BeFileNameCR bimName);

    //! Delete the existing syncinfo file for the specified BIM, if it exists.
    IMODEL_BRIDGE_EXPORT static BentleyStatus DeleteSyncInfoFileFor(BeFileNameCR BIMName);

    //! Create a new, empty syncinfo file. This should be called <em>only once</em> immediately after acquiring a new BIM.
    IMODEL_BRIDGE_EXPORT static BentleyStatus CreateEmptyFile(BeFileNameCR fileName, bool deleteIfExists);

    //! Open the syncinfo file and attach it to a BIM
    //! @param bim  The BIM
    //! @param createIfNecessary create an empty syncinfo file if it does not exist?
    IMODEL_BRIDGE_EXPORT BentleyStatus AttachToBIM(DgnDbR bim, bool createIfNecessary = true);

    //! Detach from the BIM
    IMODEL_BRIDGE_EXPORT void DetachFromBIM();

    //! Check that syncinfo is open and attached to the BIM
    bool IsAttached() const {return m_isAttached;}

    //! Get a reference to the BIM
    DgnDbR GetDgnDb() const {return *m_bim;}

    IMODEL_BRIDGE_EXPORT void GetLastError (BeSQLite::DbResult&, Utf8String&);

    //! Iterate all records in syncinfo
    IMODEL_BRIDGE_EXPORT Iterator MakeIterator(Utf8CP where = nullptr);

    //! Iterate all records in syncinfo that pertain to the specified BIM element
    IMODEL_BRIDGE_EXPORT Iterator MakeIteratorByElementId(DgnElementId id);

    //! Iterate all records in syncinfo that pertain to the source item that is identified by the specified scope, kind, and id
    IMODEL_BRIDGE_EXPORT Iterator MakeIteratorBySourceId(SourceIdentity const&);

    //! Iterate all records in syncinfo that pertain to the the source item that is identified by the specified hash value.
    IMODEL_BRIDGE_EXPORT Iterator MakeIteratorByHash(ROWID scopeRowId, Utf8StringCR kind, Utf8StringCR hash);
    
    //! Iterate all records in syncinfo that have the specified scope ROWID
    IMODEL_BRIDGE_EXPORT Iterator MakeIteratorByScope(ROWID scopeRowId);

    //! Look up the ROWID of an item by its SourceId
    IMODEL_BRIDGE_EXPORT ROWID FindRowidBySourceId(SourceIdentity const&);

    //! Write item records to syncinfo
    IMODEL_BRIDGE_EXPORT BentleyStatus WriteResults(ROWID rid, ConversionResults&, SourceIdentity const& sid, SourceState const& sstate, ChangeDetector&);

    //! Remove item records from syncinfo that are mapped to the specified element
    IMODEL_BRIDGE_EXPORT BentleyStatus DeleteAllItemsMappedToElement(DgnElementId);

    //! Remove item records from syncinfo that are in the specified scope
    //! @param srid The ROWID of a scope item
    IMODEL_BRIDGE_EXPORT BentleyStatus DeleteAllItemsInScope(ROWID srid);

    //! Remove the specific record.
    //! @param itemrid  The ROWID of the item to delete
    IMODEL_BRIDGE_EXPORT BentleyStatus DeleteItem(ROWID itemrid);

    //! Return the ChangeDetector that this bridge should use.
    IMODEL_BRIDGE_EXPORT ChangeDetectorPtr GetChangeDetectorFor(iModelBridge& bridge);
        
};

//=======================================================================================
//! Base class for iModel bridges that use iModelBridgeSyncInfoFile. This base class implements
//! the bridge methods that must deal with syncinfo, including _OnOpenBim to attach and 
//! _OnCloseBim to detach.
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE iModelBridgeWithSyncInfoBase : iModelBridgeBase
{
    DEFINE_T_SUPER(iModelBridgeBase)
protected:
    iModelBridgeSyncInfoFile m_syncInfo;
    
public:
    //! Returns a reference to the syncinfo
    iModelBridgeSyncInfoFile& GetSyncInfo() {return m_syncInfo;}
    
    //! Attaches syncinfo to the BIM
    IMODEL_BRIDGE_EXPORT BentleyStatus _OnOpenBim(DgnDbR db) override;
    //! Detaches syncinfo from the BIM
    IMODEL_BRIDGE_EXPORT void _OnCloseBim(BentleyStatus, iModelBridge::ClosePurpose purpose) override;

    void _Terminate(BentleyStatus) override {}

    //! Deletes the syncinfo file
    IMODEL_BRIDGE_EXPORT void _DeleteSyncInfo() override;
    //! Detects and cleans up after deleted documents
    IMODEL_BRIDGE_EXPORT BentleyStatus _DetectDeletedDocuments() override {return DetectDeletedDocuments();}

    //! Called when the framework detects that a document has been deleted from the source DMS or at least removed from the job. The subclass should
    //! override this method to delete from the BIM all models and elements that it previously created from data that came from this document.
    //! @param docId Identifies the document in the source DMS. May be a GUID or a local file name.
    //! @param docSyncInfoid Identifies the document in the syncinfo file
    virtual void _OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docSyncInfoid) = 0;

    //! @name Document-tracking Methods
    //! @{

    //! Convenience method to insert or update a RepositoryLink element in the BIM to represent a source document, and to insert or update a record in syncinfo to track it. 
    //! Calls GetSourceItemForDocument and then uses the supplied ChangeDetector to write the item to syncinfo.
    //! @param changeDetector The change detector
    //! @param fileName Optional. The local filename of the document. If not supplied, this defaults to the input filename.
    //! @param sstate Optional. If specified, the state of the document. If not specified, the state defaults to an empty hash and the last modified time of the file.
    //! @param kind Optional. The document kind. Defaults to "DocumentWithBeGuid"
    //! @param srid Optional. The document scope. Defaults to 0
    //! @param knownUrn Optional. The URN of the master document. Defaults to "".
    //! @return the resulting RepositoryLink Element and corresponding SyncInfo record
    //! @see GetAllDocumentGuidsInSyncInfo, DeleteAllItemsFromDocumentInSyncInfo
    IMODEL_BRIDGE_EXPORT iModelBridgeSyncInfoFile::ConversionResults RecordDocument(iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, 
                                                                BeFileNameCR fileName = BeFileName(), iModelBridgeSyncInfoFile::SourceState const* sstate = nullptr,
                                                                Utf8CP kind = "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID srid = iModelBridgeSyncInfoFile::ROWID(),
                                                                Utf8StringCR knownUrn = Utf8String());

    //! Convenience method to detect deleted documents and delete everything that was created from them.
    //! This method takes care of detecting which documents were deleted (in cooperation with iModelBridge::IDocumentPropertiesAccessor).
    //! And, this method takes care of deleting the corresponding records from syncinfo.
    //! This method delegates the task of deleting corresponding models and elements from the BIM to the bridge by invoking the _OnDocumentDeleted method.
    //! @param kind Optional. The document kind. Defaults to "DocumentWithBeGuid"
    //! @param srid Optional. The document scope. Defaults to 0
    //! @see RecordDocumentInSyncInfo, _OnDocumentDeleted
    //! @note _DetectDeletedDocuments automatically calls this function. You only need to call this function directly if you override _DetectDeletedDocuments and do not call super.
    IMODEL_BRIDGE_EXPORT BentleyStatus DetectDeletedDocuments(Utf8CP kind = "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID srid = iModelBridgeSyncInfoFile::ROWID());

    //! @}

    //! Convenience method to detect if the bridge job's spatial data transform has changed and, if so, to update its record syncinfo and return the old and new transforms.
    //! @param[out] newTrans    The new spatial data transform to use.
    //! @param[out] oldTrans    The spatial data transform that was used the last time this bridge ran.
    //! @param changeDetector   The change detector
    //! @param srid             The scope of the spatial data transform syncinfo record. Normally, this should be the document ROWID.
    //! @param kind             The kind of syncinfo record that should hold this transform record. This must be distinct from the kinds of records used by the bridge for normal data.
    //! @param id               The id that the transform record should have in syncinfo. This must be unique relative to @a kind.
    //! @return true if the spatial data transform has changed (or is new).
    IMODEL_BRIDGE_EXPORT bool DetectSpatialDataTransformChange(TransformR newTrans, TransformR oldTrans,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ROWID srid, Utf8CP kind, Utf8StringCR id);
};
//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct DocSourceItem : iModelBridgeSyncInfoFile::ISourceItem
    {
    DgnCode m_linkCode;
    iModelBridgeSyncInfoFile::SourceState m_sstate;

    DocSourceItem(DgnCode const& linkCode, iModelBridgeSyncInfoFile::SourceState const& sstate) : m_linkCode(linkCode), m_sstate(sstate) { ; }

    Utf8String _GetId() override { return m_linkCode.GetValue().GetUtf8CP(); }
    double _GetLastModifiedTime() override { return m_sstate.GetLastModifiedTime(); }
    Utf8String _GetHash() override { return m_sstate.GetHash(); }
    };

END_BENTLEY_DGN_NAMESPACE
