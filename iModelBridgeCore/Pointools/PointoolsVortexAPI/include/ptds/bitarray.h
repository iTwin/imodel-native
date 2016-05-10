#pragma once

#include <PTRMI/DataBuffer.h>

namespace ptds
    {


    template<typename Storage = unsigned char> class BitArray
        {
        public:

            typedef Storage			StorageType;
            typedef unsigned long	BitIndex;
            typedef unsigned long	StorageIndex;
            typedef Storage			BitMask;

        protected:

            Storage				*	bitArray;
            BitIndex				numBits;

            std::recursive_mutex    mutex;

        protected:

            bool					getStorageIndexAndBit(BitIndex bit, StorageIndex &storageIndex, BitMask &bitMask);
            bool					isValidBitIndex(BitIndex bit);

            void					setNumBits(BitIndex numBits);

            void					setBitArray(Storage *initBitArray);
            StorageType			*	getBitArray(void);

            void					setStorageEntry(StorageIndex t, Storage value);
            StorageType				getStorageEntry(StorageIndex t);

        public:
            BitArray(void);
            BitArray(BitIndex initNumBits);
            ~BitArray(void);

            bool					initialize(BitIndex initNumBits);

            bool					isValid(void);

            void					clear(void);
            void					destroy(void);
            void					clearArray(Storage value = 0);

            BitIndex				getNumBits(void);
            StorageIndex			getStorageSize(void);
            StorageIndex			getStorageSizeBytes(void);

            bool					setBit(BitIndex bit, bool value);
            bool					setBit(BitIndex bit);
            bool					clearBit(BitIndex bit);

            bool					getBit(BitIndex bit, bool &value);

            bool					readFromBuffer(PTRMI::DataBuffer &buffer);
            bool					writeToBuffer(PTRMI::DataBuffer &buffer);
        };



    template<typename Storage>
    bool BitArray<Storage>::initialize(BitIndex initNumBits)
        {
        if (initNumBits == 0)
            return false;

        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        setNumBits(initNumBits);

        StorageIndex storageSize = getStorageSize();

        if (storageSize > 0)
            {
            Storage *store;

            if ((store = new Storage[storageSize]) != NULL)
                {
                setBitArray(store);
                return true;
                }
            }

        clear();
        // Return failed
        return false;
        }


    template<typename Storage>
    ptds::BitArray<Storage>::~BitArray(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        destroy();
        }


    template<typename Storage>
    void ptds::BitArray<Storage>::destroy(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        if (isValid())
            {
            delete[]getBitArray();

            clear();
            }
        }


    template<typename Storage>
    ptds::BitArray<Storage>::BitArray(BitIndex initNumBits)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        clear();
        initialize(initNumBits);
        }


    template<typename Storage>
    bool ptds::BitArray<Storage>::isValid(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        return getBitArray() && getNumBits();
        }


    template<typename Storage>
    BitArray<Storage>::BitArray(void)
        {
        clear();
        }


    template<typename Storage>
    void ptds::BitArray<Storage>::clear(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        setNumBits(0);
        setBitArray(NULL);
        }


    template<typename Storage>
    void ptds::BitArray<Storage>::setBitArray(Storage *initBitArray)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        bitArray = initBitArray;
        }


    template<typename Storage>
    typename BitArray<Storage>::StorageType *BitArray<Storage>::getBitArray(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        return bitArray;
        }


    template<typename Storage>
    void BitArray<Storage>::setNumBits(BitIndex initNumBits)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        numBits = initNumBits;
        }


    template<typename Storage>
    typename BitArray<Storage>::BitIndex BitArray<Storage>::getNumBits(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        return numBits;
        }


    template<typename Storage>
    typename BitArray<Storage>::StorageIndex BitArray<Storage>::getStorageSize(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        StorageIndex	storageSizeBits = sizeof(Storage) * 8;
        // Calculate main size of storage required
        StorageIndex size = getNumBits() / storageSizeBits;
        // Include trailing number of bits on the end
        if (getNumBits() % storageSizeBits != 0)
            size++;

        return size;
        }


    template<typename Storage>
    typename BitArray<Storage>::StorageIndex BitArray<Storage>::getStorageSizeBytes(void)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        return getStorageSize() * sizeof(Storage);
        }


    template<typename Storage>
    bool BitArray<Storage>::isValidBitIndex(BitIndex bit)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        return bit < getNumBits();
        }


    template<typename Storage>
    bool BitArray<Storage>::getStorageIndexAndBit(BitIndex bit, StorageIndex &storageIndex, BitMask &bitMask)
        {
        if (isValidBitIndex(bit))
            {
            StorageIndex StorageBits = sizeof(Storage) * 8;
            // Calculate which storage element bit belongs to
            storageIndex = bit / StorageBits;
            // Start with MSB and shift right for each bit
            bitMask = 0x80 >> (bit % StorageBits);
            // Return OK
            return true;
            }
        // Return error
        return false;
        }


    template<typename Storage>
    void BitArray<Storage>::clearArray(Storage value)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        StorageIndex	t;
        StorageIndex	numEntries = getStorageSize();

        if (isValid())
            {
            for (t = 0; t < numEntries; t++)
                {
                setStorageEntry(t, value);
                }
            }
        }

    template<typename Storage>
    bool BitArray<Storage>::setBit(BitIndex bit, bool value)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        if (value)
            {
            return setBit(bit);
            }

        return clearBit(bit);
        }


    template<typename Storage>
    bool BitArray<Storage>::setBit(BitIndex bit)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        StorageIndex	storageIndex;
        BitMask			bitMask;

        if (getStorageIndexAndBit(bit, storageIndex, bitMask))
            {
            // Set bit to one
            setStorageEntry(storageIndex, getStorageEntry(storageIndex) | bitMask);
            return true;
            }

        return false;
        }


    template<typename Storage>
    bool BitArray<Storage>::clearBit(BitIndex bit)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        StorageIndex	storageIndex;
        BitMask			bitMask;

        if (getStorageIndexAndBit(bit, storageIndex, bitMask))
            {
            setStorageEntry(storageIndex, getStorageEntry(bit) & (~bitMask));
            return true;
            }

        return false;
        }


    template<typename Storage>
    bool ptds::BitArray<Storage>::getBit(BitIndex bit, bool &value)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        StorageIndex	storageIndex;
        BitMask			bitMask;

        if (getStorageIndexAndBit(bit, storageIndex, bitMask))
            {
            // Test bit
            value = (getStorageEntry(storageIndex) & bitMask) != 0;
            return true;
            }

        return false;
        }


    template<typename Storage>
    void BitArray<Storage>::setStorageEntry(StorageIndex t, Storage value)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        assert(isValid());

        getBitArray()[t] = value;
        }


    template<typename Storage>
    typename BitArray<Storage>::StorageType BitArray<Storage>::getStorageEntry(StorageIndex t)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        assert(isValid());

        return getBitArray()[t];
        }


    template<typename Storage>
    bool BitArray<Storage>::readFromBuffer(PTRMI::DataBuffer &buffer)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        BitIndex	initNumBits;

        // Read number of bits in bit array
        buffer >> initNumBits;

        if (initNumBits == 0)
            return false;
        // Initialize bit array to handle number of bits
        if (initialize(initNumBits) == false)
            return false;

        StorageIndex sizeBytes = getStorageSizeBytes();
        // Read bit array data into buffer	
        return (buffer.readFromBuffer(reinterpret_cast<PTRMI::DataBuffer::Data *>(getBitArray()), sizeBytes) == sizeBytes);
        }


    template<typename Storage>
    bool BitArray<Storage>::writeToBuffer(PTRMI::DataBuffer &buffer)
        {
        std::lock_guard<std::recursive_mutex> mutexScope(mutex);

        PTRMI::Status	status;

        if (isValid() == false)
            return false;
        // Write number of bits
        buffer << getNumBits();

        return (buffer.writeToBuffer(reinterpret_cast<PTRMI::DataBuffer::Data *>(getBitArray()), getStorageSizeBytes()) == getStorageSizeBytes());
        }


    } // End ptds namespace 


