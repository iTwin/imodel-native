/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/msl_array.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef msl_array_h
#define msl_array_h
#pragma inline_depth(255)
#pragma inline_recursion(on)


#include "assert.h"

// ArrayClass Template.
//
// This Template class will handle array allocation and resizing etc.
//
// This class requires the the template class is able to do TYPE = TYPE (eg copy itself);
// Sample program below!
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayClass
{

    protected:
        TYPE* arrayPtr;                            // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.

            if(newSize > arraySize)                            // If array is not already large enough.
            {
                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }
                if(arrayPtr)
                {
                    TYPE* oldArrayPtr;
                    oldArrayPtr = arrayPtr;                        // Store Keep old pointer.
                    arrayPtr = new TYPE[newSize];            // Allocate new array!
                    int i;                                                            // Loop Counter.
                    for(i = 0; i <= count; i++)                    // Copy old array into new array. (Note a memcpy can't be used here because
                        arrayPtr[i] = oldArrayPtr[i];            // the type might be a class and will delete memory which it is using.
                    delete [] oldArrayPtr;                        // Delete the old array.
                }                                                                                                                        
                else
                    arrayPtr = new TYPE[newSize];            // Allocate new array!
                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)                // Initalize the array.
        {
            arrayPtr = 0;                                            // Allocate first block.
            count = -1;                                                        // Set count to -1 (eg. none allocated)
            arraySize = 0;                                                // Set up initial values.
        }

        void freeArray(void)                                    // Frees the array.
        {
            if(arrayPtr)
                delete [] arrayPtr;                                    // Delete the array.
        }

    public:

        ArrayClass(const int initialSize = 0)// Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up block size.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayClass()                                                    // Deconstructor.
        {
            freeArray();                                                // Free the Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;
            blockSize = 1;
            arraySize = 0;
            if(size <= count)
                count = size - 1;
            expandArray(size - 1);
            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)
        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;
                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }
            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)
        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                            // This function empties the array.
        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
            {
                if(i >= arraySize)                                    // If tried to access pass end of alloacted array.
                    expandArray(i);                                        // Expand the Array to size of i.

                count = i;
            }
            return arrayPtr[i];                                    // Return type at pos i.
        }

        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return arrayPtr[i];                                    // Return type at pos i.
        }

        inline TYPE* getArrayPtr(void)
        {
            return arrayPtr;
        }

        inline const TYPE* getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void) const                                    // Return size of array.
        {
            return count + 1;
        }

        void replace_delete(const int pos)
        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                arrayPtr[pos] = arrayPtr[count];
            }
            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.
        {
            if(arrayPtr)
            {
                int i;
                if(pos > count)
                    return;
                for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                    arrayPtr[i] = arrayPtr[i + 1];
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)
        {
            int i;
            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1);                                    // Expand the Array to size of i.
            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)
        {
            insert_entry(pos);
            arrayPtr[pos] = entry;
        }

        inline void append_entry(const TYPE& entry)                // Appends new element at end.
        {
            operator[](size()) = entry;
        }
    
    ArrayClass(const ArrayClass& tocopy)
        {
            arrayPtr = 0;
            *this = tocopy;
        }

        ArrayClass& operator=(const ArrayClass& tocopy)
        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);
            if(tocopy.arraySize)
            {
                int i;
                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.
                for(i = 0; i <= tocopy.count; i++)            
                    arrayPtr[i] = tocopy.arrayPtr[i];
            }            
            count = tocopy.count;
            return *this;
        }
};

// ArrayMoveClass
//
// When this array class is doing it's move it calls move member of the Template, which will move the data over,
// This can be used when the class has pointer to memory and avoids the class copy constructor which will allocate more memory and
// copy them over.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayMoveClass
{

    protected:
        TYPE* arrayPtr;                            // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.
            if(newSize > arraySize)                            // If array is not already large enough.
            {
                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }
                if(arrayPtr)
                {
                    TYPE* oldArrayPtr;
                    oldArrayPtr = arrayPtr;                        // Store Keep old pointer.
                    arrayPtr = new TYPE[newSize];            // Allocate new array!
                    int i;                                                            // Loop Counter.
                    for(i = 0; i <= count; i++)                    // Copy old array into new array. (Note a memcpy can't be used here because
                        arrayPtr[i].move(oldArrayPtr[i]);    // the type might be a class and will delete memory which it is using.
                    delete [] oldArrayPtr;                        // Delete the old array.
                }                                                                                                                        
                else
                    arrayPtr = new TYPE[newSize];            // Allocate new array!
                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)                // Initalize the array.
        {
            arrayPtr = 0;                                            // Allocate first block.
            count = -1;                                                        // Set count to -1 (eg. none allocated)
            arraySize = 0;                                                // Set up initial values.
        }

        void freeArray(void)                                    // Frees the array.
        {
            if(arrayPtr)
                delete [] arrayPtr;                                    // Delete the array.
        }

    public:

        ArrayMoveClass(const int initialSize = 0)// Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up block size.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayMoveClass()                                            // Deconstructor.
        {
            freeArray();                                                // Free the Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;
            blockSize = 1;
            arraySize = 0;
            if(size <= count)
                count = size - 1;
            expandArray(size - 1);
            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)
        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;
                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }
            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)
        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                            // This function empties the array.
        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
            {
                if(i >= arraySize)                                    // If tried to access pass end of alloacted array.
                    expandArray(i);                                        // Expand the Array to size of i.
                count = i;
            }

            return arrayPtr[i];                                    // Return type at pos i.
        }

        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return arrayPtr[i];                                    // Return type at pos i.
        }

        inline TYPE* getArrayPtr(void)
        {
            return arrayPtr;
        }

        inline const TYPE* getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void) const                                    // Return size of array.
        {
            return count + 1;
        }

        void replace_delete(const int pos)
        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                arrayPtr[pos].move(arrayPtr[count]);
            }
            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.
        {
            if(arrayPtr)
            {
                int i;
                if(pos > count)
                    return;
                for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                    arrayPtr[i].move(arrayPtr[i + 1]);
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)
        {
            int i;
            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1);                                    // Expand the Array to size of i.
            for(i = count; i >= pos; i--)
                arrayPtr[i + 1].move(arrayPtr[i]);
            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)
        {
            insert_entry(pos);
            arrayPtr[pos] = entry;
        }

        inline void append_entry(const TYPE& entry)                // Appends new element at end.
        {
            operator[](size()) = entry;
        }

    ArrayMoveClass(const ArrayMoveClass& tocopy)
        {
            arrayPtr = 0;
            *this = tocopy;
        }

        ArrayMoveClass& operator=(const ArrayMoveClass& tocopy)
        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);
            if(tocopy.arraySize)
            {
                int i;
                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.
                for(i = 0; i <= tocopy.count; i++)            
                    arrayPtr[i] = tocopy.arrayPtr[i];
            }            
            count = tocopy.count;
            return *this;
        }
};


// ArrayCopyClass Template.
//
// This Template class will handle array allocation and resizing etc.
//
// The difference between this class and ArrayClass is that this one will memcpy the
// array during expanding.  Note this might not be used with classes.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayCopyClass
{

    protected:
        TYPE* arrayPtr;                            // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.

            if(newSize > arraySize)                          // If array is not already large enough.
            {

                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }

                if(arrayPtr)
                {
                    TYPE* oldArrayPtr = arrayPtr;            // Store Keep old pointer.
                    arrayPtr = new TYPE[newSize];            // Allocate new array!
                    memcpy(arrayPtr, oldArrayPtr,            // Copy old array into new array.
                        sizeof(TYPE) * (count + 1));
                    delete [] oldArrayPtr;                        // Delete the old array.
                }
                else
                    arrayPtr = new TYPE[newSize];
                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)                // Initalize the array.
        {
            arrayPtr = 0;                                        // Allocate first block.
            count = -1;                                                    // Set count to -1 (eg. none allocated)
            arraySize = 0;                                            // Set up initial values.
        }

        void freeArray(void)                                    // Frees the array.

        {

            if(arrayPtr)
                delete [] arrayPtr;                                    // Delete the array.
        }

    public:

        ArrayCopyClass(const int initialSize = 0)// Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up block size.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayCopyClass()                                            // Deconstructor.

        {
            freeArray();                                                // Free the Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;

            blockSize = 1;
            arraySize = 0;

            if(size <= count)
                count = size - 1;

            expandArray(size - 1);

            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)

        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;

                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }

            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)

        {
            this->blockSize = blockSize;
        }

        void empty(void)                                            // This function empties the array.

        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)
            {
                if(i >= arraySize)
                    expandArray(i);                                        // Expand the Array to size of i.

                count = i;
            }

            return arrayPtr[i];                                    // Return type at pos i.
        }
        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return arrayPtr[i];                                    // Return type at pos i.
        }
        inline TYPE* getArrayPtr(void)

        {
            return arrayPtr;
        }

        inline const TYPE* getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void)                                                // Return size of array.

        {
            return count + 1;
        }

        void replace_delete(const int pos)
        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                memcpy(arrayPtr[pos], arrayPtr[count], sizeof(TYPE));
            }
            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.

        {
            if(arrayPtr)
            {
                if(pos > count)
                    return;

                                                                                        // Copy all entry above deleted one down!
                memcpy(&arrayPtr[pos], &arrayPtr[pos + 1], sizeof(TYPE) * ((count - pos) - 1));
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)

        {
            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1 );                                // Expand the Array to size of pos.
            memmove(&arrayPtr[pos], &arrayPtr[pos + 1], sizeof(TYPE) * (count - pos));
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            memcpy(&arrayPtr[pos], &entry, sizeof(TYPE));
        }
        void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            TYPE& l = operator[](size());
            memcpy(&l, &entry, sizeof(TYPE));
        }
    ArrayCopyClass(const ArrayCopyClass& tocopy)

        {
            arrayPtr = 0;
            *this = tocopy;
        }

        ArrayCopyClass& operator=(const ArrayCopyClass& tocopy)

        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);

            if(tocopy.arraySize)
            {
                int i;

                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.

                for(i = 0; i <= tocopy.count; i++)            
                    memcpy(arrayPtr[i], tocopy.arrayPtr[i], sizeof(TYPE));
            }            
            count = tocopy.count;

            return *this;
        }
};

// ArrayPtrClass Template.
//
// This Template class will handle array allocation and resizing etc.
//
// The Difference between this class and ArrayClass is that this one allocates each entity as required
// and stores the pointers to them.
//
// It uses more memory but in Dynamic Allocation it is quicker.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayPtrClass
{

    protected:
        TYPE** arrayPtr;                        // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.

            if(newSize > arraySize)                            // If array is not already large enough.
            {

                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }

                if(arrayPtr)
                {
                    TYPE** oldArrayPtr = arrayPtr;        // Store Keep old pointer.

                    arrayPtr = new TYPE*[newSize];        // Allocate new array!
                    int i;                                                        // Loop Counter.

                    for(i = 0; i <= count; i++)                // Copy old array into new array. (Note a memcpy can't be used here because
                        arrayPtr[i] = oldArrayPtr[i];        // the type might be a class and will delete memory which it is using.
                    for(; i < newSize; i++)
                        arrayPtr[i] = new TYPE;
                    for(i = count + 1; i < arraySize; i++)
                        delete arrayPtr[i];
                    delete [] oldArrayPtr;                        // Delete the old array.
                }
                else
                {
                    arrayPtr = new TYPE*[newSize];
                    int i;
                    for(i = 0; i < newSize; i++)
                        arrayPtr[i] = new TYPE;
                }
                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)
        {
            arrayPtr = 0;                                        // Allocate block.
            count = -1;                                                    // Set count to -1 (eg. none allocated)
            arraySize = 0;                                            // Set array Size.
        }

        void freeArray(void)

        {
            int i;

            if(arrayPtr)
            {
                for(i = 0; i < arraySize; i++)            // Delete all entries.
                    delete arrayPtr[i];
                delete [] arrayPtr;                                    // Delete the array.
            }
            arrayPtr = 0;
        }

    public:
        ArrayPtrClass(int initialSize = 0)        // Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up blockSize.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayPtrClass()                                            // Deconstructor.
        {
            freeArray();                                                // Free Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;

            blockSize = 1;

            if(size <= count)
                count = size - 1;

            int i;

            for(i = count + 1; i < arraySize; i++)
                delete arrayPtr[i];
            arraySize = 0;

            expandArray(size - 1);

            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)

        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;

                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }

            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)

        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                // This function empties the array.

        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
            {

                if(i >= arraySize)                                // If tried to access pass end of alloacted array.
                    expandArray(i);                                    // Expand the Array to size of i.

                count = i;
            }

            return *arrayPtr[i];                                // Return type at pos i.
        }
        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return *arrayPtr[i];                                    // Return type at pos i.
        }
        inline TYPE** getArrayPtr(void)

        {
            return arrayPtr;
        }

        inline TYPE** const getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void) const                                    // Return size of array.

        {
            return count + 1;
        }

        void replace_delete(const int pos)

        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                if(arrayPtr[pos])
                    delete arrayPtr[pos];
                arrayPtr[pos] = arrayPtr[count];
                arrayPtr[count] = new TYPE;
            }

            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.

        {
            if(arrayPtr)
            {
                int i;

                if(pos > count)
                    return;

                if(arrayPtr[pos])
                    delete arrayPtr[pos];
                for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                    arrayPtr[i] = arrayPtr[i + 1];
                arrayPtr[i] = new TYPE;
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)

        {
            int i;

            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1);                                    // Expand the Array to size of i.

//            delete arrayPtr[count];
            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            arrayPtr[pos] = new TYPE;

            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            *arrayPtr[pos] = entry;
        }
        inline void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            operator[](size()) = entry;
        }
    ArrayPtrClass(const ArrayPtrClass& tocopy)

        {
            arrayPtr = 0;
            *this = tocopy;
        }

        ArrayPtrClass& operator=(const ArrayPtrClass& tocopy)

        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);

            if(tocopy.arraySize)
            {
                int i;

                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.

                for(i = 0; i <= tocopy.count; i++)
                {
                    *arrayPtr[i] = *tocopy.arrayPtr[i];
                }
            }
            count = tocopy.count;

            return *this;
        }

#ifdef notdef
        void setBlockSize(const int new_value)// Allows users to change the block size.
        {
            blockSize = new_value;                            // Set the new block size.
        }

        void empty(void)                                            // This function empties the array.

        {
            freeArray();                                                // Free Currently Allocated Memory.
            initArray(blockSize);                                // ReInitialize Array.
        }

        TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i >= arraySize)                                    // If tried to access pass end of alloacted array.
             expandArray(i);                                        // Expand the Array to size of i.

            if(i > count)                                                // If i > count then make count = i.
             count = i;

            if(!arrayPtr[i])                                        // If this entry hasn't been allocated
                arrayPtr[i] = new TYPE;                        // then Allocate it.
            return *arrayPtr[i];                                // Return type at pos i.
        }
        int size(void)                                                // Return size of array.
        {
            return count + 1;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.
        {
            int i;

            if(pos > count)
                return;

            delete arrayPtr[pos];
            for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                arrayPtr[i] = arrayPtr[i + 1];
            count--;                                                        // Correct number.
        }

        void insert_entry(const int pos)

        {
            int i;

            if((count + 1) >= arraySize)                // If adding access pass end of alloacted array.
              expandArray(count + 1);                        // Expand the Array to size of i.

            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            arrayPtr[pos] = entry;
        }
        void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            operator[]size()) = entry;
        }
#endif

    /*----------------------------------------------------------------------------------*//**
    * @bsiclass                                                     Piotr.Slowinski 06/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    class ConstIterator
    {

    friend class ArrayPtrClass<TYPE>;

    protected:

        TYPE**  m_p;

        ConstIterator (TYPE** p) : m_p (p)
            {
            }

    public:

        ConstIterator (void) : m_p (NULL)
            {
            }

        bool operator== (ConstIterator const& other) const
            {
            return m_p == other.m_p;
            }

        bool operator!= (ConstIterator const& other) const
            {
            return !(*this == other);
            }

        bool operator < (ConstIterator const& other) const
            {
            return m_p < other.m_p;
            }

        bool operator > (ConstIterator const& other) const
            {
            return m_p > other.m_p;
            }

	    ConstIterator& operator++ (void)
	    {
		    ++m_p;
		    return *this;
	    }

	    TYPE const& operator* (void) const
            {
            return *(*m_p);
            }

        TYPE const* operator-> (void) const
            {
            return *m_p;
            }

    }; // end ConstIterator class

    /*----------------------------------------------------------------------------------*//**
    * @bsiclass                                                     Piotr.Slowinski 06/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    class Iterator : public ConstIterator
    {

    friend class ArrayPtrClass<TYPE>;

    protected:

        Iterator (TYPE** p) : ConstIterator (p)
            {
            }

    public:

        Iterator ()
            {
            }

	    TYPE& operator* (void)
            {
            return *(*m_p);
            }

        TYPE* operator-> (void)
            {
            return *m_p;
            }

    }; // End Iterator class

    Iterator GetBegin (void)
        {
        return Iterator (arrayPtr);
        }

    Iterator GetEnd (void)
        {
        return Iterator (arrayPtr + size());
        }

    ConstIterator GetBegin (void) const
        {
        return ConstIterator (arrayPtr);
        }

    ConstIterator GetEnd (void) const
        {
        return ConstIterator (arrayPtr + size());
        }

}; // End ArrayPtrClass class

// ArrayNullPtrClass Template.
//
// This Template class will handle array allocation and resizing etc.
//
// The Difference between this class and ArrayClass is that this one allocates each entity as required
// and stores the pointers to them.
//
// It uses more memory but in Dynamic Allocation it is quicker.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayNullPtrClass
{

    protected:
        TYPE** arrayPtr;                        // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.

            if(newSize > arraySize)                            // If array is not already large enough.
            {

                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }

                if(arrayPtr)
                {
                    TYPE** oldArrayPtr = arrayPtr;        // Store Keep old pointer.

                    arrayPtr = new TYPE*[newSize];        // Allocate new array!
                    int i;                                                        // Loop Counter.

                    for(i = 0; i <= count; i++)                // Copy old array into new array. (Note a memcpy can't be used here because
                        arrayPtr[i] = oldArrayPtr[i];        // the type might be a class and will delete memory which it is using.
                    for(; i < newSize; i++)
                        arrayPtr[i] = 0;
                    for(i = newSize; i < arraySize; i++)
                        if(arrayPtr[i])
                            delete arrayPtr[i];
                    delete [] oldArrayPtr;                        // Delete the old array.
                }
                else
                {
                    arrayPtr = new TYPE*[newSize];
                    int i;
                    for(i = 0; i < newSize; i++)
                        arrayPtr[i] = 0;
                }
                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)
        {
            arrayPtr = 0;                                        // Allocate block.
            count = -1;                                                    // Set count to -1 (eg. none allocated)
            arraySize = 0;                                            // Set array Size.
        }

        void freeArray(void)

        {
            int i;

            if(arrayPtr)
            {
                for(i = 0; i < arraySize; i++)            // Delete all entries.
                    if(arrayPtr[i])
                        delete arrayPtr[i];
                delete [] arrayPtr;                                    // Delete the array.
            }
            arrayPtr = 0;
        }

    public:
        ArrayNullPtrClass(int initialSize = 0)        // Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up blockSize.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayNullPtrClass()                                            // Deconstructor.
        {
            freeArray();                                                // Free Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;

            blockSize = 1;
            arraySize = 0;

            if(size <= count)
                count = size - 1;

            expandArray(size - 1);

            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)

        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;

                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }

            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)

        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                // This function empties the array.

        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
            {

                if(i >= arraySize)                                // If tried to access pass end of alloacted array.
                    expandArray(i);                                    // Expand the Array to size of i.

                count = i;
            }

            if(!arrayPtr[i])
                arrayPtr[i] = new TYPE;
            return *arrayPtr[i];                                // Return type at pos i.
        }

        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return *arrayPtr[i];                                    // Return type at pos i.
        }
        inline TYPE** getArrayPtr(void)

        {
            return arrayPtr;
        }

        inline TYPE** const getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void) const                                    // Return size of array.

        {
            return count + 1;
        }

        void replace_delete(const int pos)

        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                if(arrayPtr[pos])
                    delete arrayPtr[pos];
                arrayPtr[pos] = arrayPtr[count];
                arrayPtr[count] = 0;
            }

            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.

        {
            if(arrayPtr)
            {
                int i;

                if(pos > count)
                    return;

                if(arrayPtr[pos])
                    if(arrayPtr[pos])
                        delete arrayPtr[pos];
                for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                    arrayPtr[i] = arrayPtr[i + 1];
                arrayPtr[i] = 0;
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)

        {
            int i;

            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1);                                    // Expand the Array to size of i.

//            delete arrayPtr[count];
            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            arrayPtr[pos] = 0;

            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            if(!arrayPtr[pos])
                arrayPtr[pos] = new TYPE;
            *arrayPtr[pos] = entry;
        }

        inline void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            operator[](size()) = entry;
        }
    ArrayNullPtrClass(const ArrayNullPtrClass& tocopy)

        {
            arrayPtr = 0;
            *this = tocopy;
        }

        ArrayNullPtrClass& operator=(const ArrayNullPtrClass& tocopy)

        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);

            if(tocopy.arraySize)
            {
                int i;

                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.

                for(i = 0; i <= tocopy.count; i++)
                {
                    *arrayPtr[i] = *tocopy.arrayPtr[i];
                }
            }
            count = tocopy.count;

            return *this;
        }
};

/*

Example Program.


struct name_age_struct                                            // Define our structure. (This can be a class or a single element)
{
    char name[40];
    int age;
};

ArrayClass <name_age_struct> name_age(20);  // Creates are array class with a chunk alocate size of 20.

void init(void)
{
    int size;

    name_age.empty();                                                // This is only needed if you have already filled in the array and wish to start
                                                                                    // again.
    
    strcpy(name_age[0].name, "First Name");    // Initalize first element.
    name_age[0].age = 31;

    // For speed if accessing a lot of each element field one at a time.

    //  Not this instance will only stay valid 
    // till the array is incresed.

    name_age_struct& name_age_single = name_age[1];

    strcpy(name_age_single.name, "Second Name");
    name_age_single.age = 35;

    size = name_age.size();                                // Get the number of elements used in the array (This will be 2)
}
*/

// ArrayClassWin Template.
//
// This Template class will handle array allocation and resizing etc.
//
// The difference between this class and ArrayClass is that this one will memcpy the
// array during expanding.  Note this might not be used with classes.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayClassWin
{

    protected:
        TYPE* arrayPtr;                            // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

    private:

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.
//            if(newSize > arraySize)                          // If array is not already large enough.
            {

                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }

                if (arrayPtr)
                    {
                    arrayPtr = (TYPE*)realloc (arrayPtr, sizeof(TYPE) *newSize);
                    }
                else
                    arrayPtr = (TYPE*)malloc (sizeof(TYPE)* newSize);

                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)                // Initalize the array.
        {
            arrayPtr = 0;                                        // Allocate first block.
            count = -1;                                                    // Set count to -1 (eg. none allocated)
            arraySize = 0;                                            // Set up initial values.
        }

        void freeArray (void)                                    // Frees the array.
            {

            if (arrayPtr)
                free (arrayPtr);
            }


    public:

        ArrayClassWin(const int initialSize = 0)// Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up block size.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayClassWin()                                            // Deconstructor.

        {
            freeArray();                                                // Free the Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;

            blockSize = 1;
            expandArray(size - 1);
            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)

        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;

                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }

            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)

        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                            // This function empties the array.

        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)
            {
                if(i >= arraySize)
                    expandArray(i);                                        // Expand the Array to size of i.

                count = i;
            }

            return arrayPtr[i];                                    // Return type at pos i.
        }
        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return arrayPtr[i];                                    // Return type at pos i.
        }
        inline TYPE* getArrayPtr(void)

        {
            return arrayPtr;
        }

        inline const TYPE* getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void)    const                                            // Return size of array.

        {
            return count + 1;
        }

        void replace_delete(const int pos)

        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                memcpy(&arrayPtr[pos], &arrayPtr[count], sizeof(TYPE));
            }
            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.

        {
            if(arrayPtr)
            {
                if(pos > count)
                    return;

                                                                                        // Copy all entry above deleted one down!
                memcpy(&arrayPtr[pos], &arrayPtr[pos + 1], sizeof(TYPE) * ((count - pos) - 1));
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)

        {
            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1 );                                // Expand the Array to size of pos.
            memmove(&arrayPtr[pos], &arrayPtr[pos + 1], sizeof(TYPE) * (count - pos));
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            memcpy(&arrayPtr[pos], &entry, sizeof(TYPE));
        }
        void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            TYPE& l = operator[](size());
            memcpy(&l, &entry, sizeof(TYPE));
        }
    ArrayClassWin(const ArrayClassWin& tocopy)

        {
            arrayPtr = 0;
            hMem = 0;
            *this = tocopy;
        }

        ArrayClassWin& operator=(const ArrayClassWin& tocopy)

        {
            freeArray();
            initArray(tocopy.arraySize);

            if(tocopy.arraySize)
            {
//                int i;

                blockSize = 1;

                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.

                blockSize = tocopy.blockSize;

//                for(i = 0; i <= tocopy.count; i++)
                memcpy((void*)arrayPtr, tocopy.arrayPtr, sizeof(TYPE) * (tocopy.count + 1));
            }            

            count = tocopy.count;

            return *this;
        }
};

// ArrayBulkPtrClass Template.
//
// This Template class will handle array allocation and resizing etc.
//
// The Difference between this class and ArrayClass is that this one allocates each entity as required
// and stores the pointers to them.
//
// It uses more memory but in Dynamic Allocation it is quicker.
//

//================================================================================
// @bsiclass                                        John.Doe 11/2000
//================================================================================
template <class TYPE> class ArrayBulkPtrClass
{

    protected:
        TYPE** arrayPtr;                        // Pointer to Array.
        int count;                                    // Number of Elements in 
        int arraySize;                            // Number of elements in array. (Note blocks are allocated in blocks)
        int blockSize;                            // Blocks are allocated in chunks.

        struct Allocation
        {
            int start;
            int end;
            TYPE* pointer;
        };

        ArrayClassWin<Allocation> m_allocation;

    private:
        inline void AllocateMem(int newSize, int start)
        {
            TYPE* data;
            int blockSize = 1000;

            while(start < newSize)
            {
                if(newSize - start < blockSize)
                {
                    blockSize = newSize - start;
                }

                data = new TYPE[blockSize];
                Allocation alloc;
                alloc.start = start;
                alloc.end = start + blockSize - 1;
                alloc.pointer = data;
                m_allocation.append_entry(alloc);

                for(int i = 0; i < blockSize; i++)
                    arrayPtr[i + start] = data++;
                start += blockSize;
            }

            int i;

            for(i = m_allocation.size() - 1; i >= 0; i--)
            {
                if(m_allocation[i].start < newSize)
                {
                    break;
                }
                delete [] m_allocation[i].pointer;
            }

            m_allocation.setLogicalLength(i + 1);
            }

        void expandArray(int newSize)                    // expandArray to newSize.
        {
            newSize++;                                                    // Accessing block n so as blocks start at 0 add one.
//            if(newSize > arraySize)                            // If array is not already large enough.
            {

                if(blockSize == 0)
                {
                    newSize = newSize + (40 + newSize) / 4;
                }
                else if(blockSize > 1)
                {
                    newSize /= blockSize;                            // Find out the number of chunks needed.
                    newSize = (newSize + 1) * blockSize;
                }

                if(arrayPtr)
                {
                arrayPtr = (TYPE**)realloc (arrayPtr, sizeof(TYPE*)* newSize);
                AllocateMem(newSize, arraySize);
                }
                else
                {
                arrayPtr = (TYPE**)malloc (sizeof(TYPE*)* newSize);

                    AllocateMem(newSize, 0);
                }

                arraySize = newSize;                            // Set new array size.
            }
        }

        void initArray(const int size)
        {
            arrayPtr = 0;                                        // Allocate block.
            count = -1;                                                    // Set count to -1 (eg. none allocated)
            arraySize = 0;                                            // Set up initial values.
        }

        void freeArray(void)

        {
            int i;

            if(arrayPtr)
            {
                for(i = 0; i < m_allocation.size(); i++)            // Delete all entries.
                {
                    TYPE* data = m_allocation[i].pointer;
                    delete [] data;
                }
                m_allocation.empty();
                free (arrayPtr);
            }

            arrayPtr = 0;
        }

    public:
        ArrayBulkPtrClass(int initialSize = 0)        // Constructor (initialSize depends on how big you are allocating!)
        {
            blockSize = initialSize;                        // Set up blockSize.
            initArray(blockSize);                                // Allocate first block.
        }

        ~ArrayBulkPtrClass()                                            // Deconstructor.
        {
            freeArray();                                                // Free Array.
        }

        inline void setPhysicalLength(const int size)
        {
            int tmpBlockSize = blockSize;

            blockSize = 1;
            expandArray(size - 1);
            blockSize = tmpBlockSize;
        }

        inline void setLogicalLength(const int size)

        {
            if(arraySize < size)
            {
                int tmpBlockSize = blockSize;

                blockSize = 1;
                expandArray(size - 1);
                blockSize = tmpBlockSize;
            }

            count = size - 1;
        }

        inline void setBlockSize(const int blockSize)

        {
            this->blockSize = blockSize;
        }

        inline void empty(void)                                // This function empties the array.

        {
            freeArray();                                                // Free the Array.
            initArray(blockSize);                                // Reallocate first block.
        }

        inline TYPE& operator[](int i)                // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
            {

                if(i >= arraySize)                                // If tried to access pass end of alloacted array.
                    expandArray(i);                                    // Expand the Array to size of i.

                count = i;
            }

            return *arrayPtr[i];                                // Return type at pos i.
        }
        inline const TYPE& operator[](int i) const        // The [] operator function. Returns the type at pos i.
        {
            if(i > count)                                                // If i > count then make count = i.
                i = count;
            return *arrayPtr[i];                                    // Return type at pos i.
        }
        inline TYPE** getArrayPtr(void)

        {
            return arrayPtr;
        }

        inline TYPE** const getArrayPtr(void) const
        {
            return arrayPtr;
        }

        inline int size(void) const                                    // Return size of array.

        {
            return count + 1;
        }

        void replace_delete(const int pos)

        {
            if(pos > count)
                return;
            else if(pos != count)
            {
                arrayPtr[pos] = arrayPtr[count];
            }
            count--;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.

        {
            if(arrayPtr)
            {
                int i;

                if(pos > count)
                    return;

                for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                    arrayPtr[i] = arrayPtr[i + 1];
                count--;                                                        // Correct number.
            }
        }

        void insert_entry(const int pos)

        {
            int i;

            if((count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
              expandArray(count + 1);                                    // Expand the Array to size of i.

            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            arrayPtr[pos] = entry;
        }
        inline void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            operator[](size()) = entry;
        }
    ArrayBulkPtrClass(const ArrayBulkPtrClass& tocopy)

        {
            arrayPtr = 0;
            hMem = 0;
            *this = tocopy;
        }

        ArrayBulkPtrClass& operator=(const ArrayBulkPtrClass& tocopy)

        {
            blockSize = tocopy.blockSize;
            freeArray();
            initArray(tocopy.arraySize);

            if(tocopy.arraySize)
            {
                int i;

                if((tocopy.count + 1) >= arraySize)                            // If adding access pass end of alloacted array.
                    expandArray(tocopy.count + 1);                                    // Expand the Array to size of i.

                for(i = 0; i <= tocopy.count; i++)
                {
                    *arrayPtr[i] = *tocopy.arrayPtr[i];
                }
            }
            count = tocopy.count;

            return *this;
        }

#ifdef notdef
        void setBlockSize(const int new_value)// Allows users to change the block size.
        {
            blockSize = new_value;                            // Set the new block size.
        }

        void empty(void)                                            // This function empties the array.

        {
            freeArray();                                                // Free Currently Allocated Memory.
            initArray(blockSize);                                // ReInitialize Array.
        }

        TYPE& operator[](int i)                                // The [] operator function. Returns the type at pos i.
        {
            if(i >= arraySize)                                    // If tried to access pass end of alloacted array.
             expandArray(i);                                        // Expand the Array to size of i.

            if(i > count)                                                // If i > count then make count = i.
             count = i;

            if(!arrayPtr[i])                                        // If this entry hasn't been allocated
                arrayPtr[i] = new TYPE;                        // then Allocate it.
            return *arrayPtr[i];                                // Return type at pos i.
        }
        int size(void)                                                // Return size of array.
        {
            return count + 1;
        }

        void delete_entry(const int pos)            // Removes an entry from the array.
        {
            int i;

            if(pos > count)
                return;

            delete arrayPtr[pos];
            for(i = pos; i < count; i++)                // Shifts all entrys above down one.
                arrayPtr[i] = arrayPtr[i + 1];
            count--;                                                        // Correct number.
        }

        void insert_entry(const int pos)

        {
            int i;

            if((count + 1) >= arraySize)                // If adding access pass end of alloacted array.
              expandArray(count + 1);                        // Expand the Array to size of i.

            for(i = count; i >= pos; i--)
                arrayPtr[i + 1] = arrayPtr[i];
            count++;
        }

        void insert_entry(const int pos, const TYPE& entry)

        {
            insert_entry(pos);
            arrayPtr[pos] = entry;
        }
        void append_entry(const TYPE& entry)                // Appends new element at end.

        {
            operator[]size()) = entry;
        }
#endif
};

#endif
