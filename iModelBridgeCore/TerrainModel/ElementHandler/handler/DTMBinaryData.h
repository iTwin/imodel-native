/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMBinaryData.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

class DTMBinaryData
    {
    public: DTMBinaryData(ElementRefP elRef, UInt32 xAttrId);
    public: static void Initialize();
    public: static bool HasBinaryData(ElementRefP elRef, UInt16 xAttrSubId, UInt32 xAttrId);
    public: static bool HasBinaryData(EditElementHandleR element, UInt16 xAttrSubId, UInt32 xAttrId);
    public: static StatusInt ScheduleReadData(EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void* data, UInt32 offset, UInt32 dataSize);
    public: static void ScheduleWriteData (EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void const* data, UInt32 offset, UInt32 dataSize);
    public: static void ScheduleWriteData (EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void const* data, UInt32 dataSize);
    public: static void ScheduleDeleteData (EditElementHandleR element, UInt16 xAttrSubId, UInt32 xAttrId);
    };

class DTMBinaryDataIter
    {
    private: ElementHandle::XAttributeIter* m_xAttrIter;
    private: ElementRefP m_elRef;
    private: bool m_isReset;
    private: UInt16 m_xAttrSubId;
    public: DTMBinaryDataIter(ElementHandle const& eh, UInt16 xAttrSubId);
    public: ~DTMBinaryDataIter();
    public: void Reset();
    public: bool MoveNext();
    public: void SetxAttrSubId(UInt16 xAttrSubId);
    public: Int32 GetCurrentSize();
    public: void* GetCurrentData();
    public: void* GetWritableData();
    public: UInt32 GetCurrentAttrId();
    public: void SetCurrentAttrId(UInt16 xAttrSubId)
                {
                m_xAttrSubId = xAttrSubId;
                }
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
