/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Insert-only hash map: entries in one vector, plus a slot table pointing into it.
// @bsiclass
//+===============+===============+===============+===============+===============+======
template<typename TKey, typename TValue, typename THash>
struct ChangeUnifierFlatMap final {
    using Entry = std::pair<TKey, TValue>;

private:
    struct Slot final {
        uint32_t m_entryPlusOne = 0; //!< 0 = empty, otherwise position in m_entries + 1
        uint32_t m_tag = 0; //!< low bits of the hash, compared before the key
    };

    static constexpr size_t MinSlotCount = 16;

    std::vector<Entry> m_entries;
    std::vector<Slot> m_slots;
    uint32_t m_shift = 64; //!< 64 - log2(slot count)
    THash m_hash;

    static uint64_t Mix(size_t hash) { return static_cast<uint64_t>(hash) * 0x9E3779B97F4A7C15ull; }
    size_t Home(uint64_t mixed) const { return static_cast<size_t>(mixed >> m_shift); }
    size_t Mask() const { return m_slots.size() - 1; }

    void PlaceSlot(uint64_t mixed, uint32_t tag, uint32_t entryPlusOne) {
        size_t pos = Home(mixed);
        while (m_slots[pos].m_entryPlusOne != 0)
            pos = (pos + 1) & Mask();
        m_slots[pos].m_entryPlusOne = entryPlusOne;
        m_slots[pos].m_tag = tag;
    }

    void Rehash(size_t slotCount) {
        m_slots.assign(slotCount, Slot());
        m_shift = 64;
        for (size_t n = slotCount; n > 1; n >>= 1)
            --m_shift;
        for (size_t i = 0; i < m_entries.size(); ++i) {
            const size_t hash = m_hash(m_entries[i].first);
            PlaceSlot(Mix(hash), static_cast<uint32_t>(hash), static_cast<uint32_t>(i + 1));
        }
    }

    //! Keeps the load factor at or below 70%.
    void ReserveForOneMore() {
        const size_t needed = m_entries.size() + 1;
        if (!m_slots.empty() && needed * 10 <= m_slots.size() * 7)
            return;
        size_t slotCount = std::max(MinSlotCount, m_slots.size());
        while (needed * 10 > slotCount * 7)
            slotCount *= 2;
        Rehash(slotCount);
    }

public:
    size_t Size() const { return m_entries.size(); }
    bool IsEmpty() const { return m_entries.empty(); }

    //! Bytes held by the map itself, excluding memory owned by keys and values.
    uint64_t GetMemoryBytes() const { return m_entries.capacity() * sizeof(Entry) + m_slots.capacity() * sizeof(Slot); }

    //! @return the value stored under @p key, or nullptr. Valid until the next Insert or TakeEntries.
    TValue* Find(TKey const& key) {
        if (m_entries.empty())
            return nullptr;
        const size_t hash = m_hash(key);
        const uint32_t tag = static_cast<uint32_t>(hash);
        for (size_t pos = Home(Mix(hash));; pos = (pos + 1) & Mask()) {
            Slot const& slot = m_slots[pos];
            if (slot.m_entryPlusOne == 0)
                return nullptr;
            if (slot.m_tag == tag) {
                Entry& entry = m_entries[slot.m_entryPlusOne - 1];
                if (entry.first == key)
                    return &entry.second;
            }
        }
    }

    //! @p key must not be in the map yet.
    void Insert(TKey const& key, TValue&& value) {
        ReserveForOneMore();
        const size_t hash = m_hash(key);
        m_entries.emplace_back(key, std::move(value));
        PlaceSlot(Mix(hash), static_cast<uint32_t>(hash), static_cast<uint32_t>(m_entries.size()));
    }

    //! Moves all entries out (insertion order) and empties the map.
    std::vector<Entry> TakeEntries() {
        std::vector<Entry> taken;
        taken.swap(m_entries);
        std::fill(m_slots.begin(), m_slots.end(), Slot());
        return taken;
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
