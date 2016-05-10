/*--------------------------------------------------------------------------*/ 
/*	BitVector																*/ 
/*  (C) 2003 Copyright Igloosoft Ltd										*/ 
/*																			*/ 
/*  Last Updated 29 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef BITSVECTOR_INTERFACE_29_11_03
#define BITSVECTOR_INTERFACE_29_11_03

#include <assert.h>
#include <memory.h>

/* assumes typedef unsigned char bits */ 
#define BITS_SZ 8

/*macros*/ 
#define NELEM(N,ELEMPER) ((N + (ELEMPER) - 1) / (ELEMPER))

#define CANONIZE(Array,NumInts,NumElem)                                 \
   (Array)[NumInts - 1] &= (bit)~0 >> (BITS_SZ - ((NumElem % BITS_SZ)   \
                                                  ? (NumElem % BITS_SZ) \
		                                  : BITS_SZ));
namespace pt
{
class bitvector
{
public:
	typedef unsigned char           bit;
	typedef unsigned int			elem_t;

	size_t	size;
	size_t	bitsize;
	bit *	bits;

	bitvector() : bits(0), bitsize(0), size(0) {};
	bitvector(elem_t sz) : bits(0), bitsize(0), size(0) { allocate(sz); };
	~bitvector() {	release(); }

	void release() { if (bits) delete [] bits; size =0; bitsize = 0; bits=0; }
	/*
	   allocate()
   
	   PURPOSE: dynamically allocate space for an array of `nelems' bits
			 and initalize the bits to all be zero.
	   PRE:  nelems is the number of Boolean values required in an array
	   POST: either a pointer to an initialized (all zero) array of bit 
		  OR
			 space was not available and 0 was returned
	   NOTE: calloc() guarantees that the space has been initialized to 0.

	   Used by: ba_ul2b(), ba_intersection() and ba_union().
	*/
	bool allocate(const elem_t nelems) 
	{
		if (bitsize != nelems)
		{
			release();

			bitsize = nelems;
			size = NELEM(nelems,(BITS_SZ));
			bits = new bit[size];
			memset(bits, 0, sizeof(bit)*size);
			/*((bit *)calloc(size, sizeof(bit)));*/ 
		}
		return !(0 == bits);
	}
	/*
	   ba_copy()

	   PRE:  `dst' has been initialized to hold `size' elements.  `src' 
			 is the array of bit to be copied to `dst'.
	   POST: `dst' is identical to the first `size' bits of `src'.  
			 `src' is unchanged.
	   Used by: ba_union()
	*/
	void copy(const bitvector &src) 
	{
		register elem_t i;

		for (i=0; i < src.size; i++) bits[i] = src.bits[i];
	} 
	/*
	   ba_assign()

	   PURPOSE: set or clear the bit in position `elem' of the array
			 `arr'
	   PRE:     arr[elem] is to be set (assigned to 1) if value is TRUE, 
				otherwise it is to be cleared (assigned to 0).  
	   POST:    PRE fulfilled.  All other bits unchanged.
	   SEE ALSO: ba_all_assign()
	   Used by:  ba_ul2b()
	*/
	inline void assign(elem_t elem, const bool &value) 
	{
	   if (value) 
		  bits[elem / BITS_SZ] |= (1 << (elem % BITS_SZ));
	   else 
		  bits[elem / BITS_SZ] &= ~(1 << (elem % BITS_SZ));
	} 
	inline void assign_true(elem_t elem)
	{
		bits[elem / BITS_SZ] |= (1 << (elem % BITS_SZ));
	}
	inline void assign_false(elem_t elem)
	{
		bits[elem / BITS_SZ] &= ~(1 << (elem % BITS_SZ));
	}
	inline bit bitmask_before(elem_t elem) const
	{
		assert(BITS_SZ == 8);
		static bit m [] = { 1, 3, 7, 15, 31, 63, 127, 255 };
		return m[elem % BITS_SZ];
	}
	inline bit bitmask_after(elem_t elem) const
	{
		assert(BITS_SZ == 8);
		static bit m [] = { 255, 127, 63, 31, 15, 7, 3 };
		return m[elem % BITS_SZ];
	}
	inline void assign(elem_t start, elem_t end, const bool &value) 
	{
		bit    setval = (value) ?~0 :0;

		elem_t startblock = start / BITS_SZ;
		elem_t endblock = end / BITS_SZ;

		if (value)
		{
			bits[startblock] |= bitmask_after(start);
			bits[endblock] |= bitmask_before(end);
		}
		else
		{
			bits[startblock] &= ~bitmask_after(start);
			bits[endblock] &= ~bitmask_before(end);
		}
		elem_t i;
		for (i=startblock + 1; i<endblock -1; i++) bits[i] = setval; 
	} 
/*
   ba_all_assign()

   PRE:  arr has been initialized to have *exactly* size elements.
   POST: All `size' elements of arr have been set to `value'.  
         The array is in canonical form, i.e. trailing elements are 
         all 0.
   NOTE: The array allocated by ba_new() has all elements 0 and is 
         therefore in canonical form.
   SEE ALSO: ba_assign()
   Used by: ba_ul2b()
*/
	void assign_all(bool value) 
	{
		bit    setval = (value) ?~0 :0;
		register elem_t i;

		for (i=0; i < size; i++) bits[i] = setval;

		/* force canonical form */ 
		CANONIZE(bits,size,bitsize);
	} 
/*
   ba_ul2b()

   PRE:  Either 
           `arr' points to space allocated to hold enough `bit's to
         represent `num' (namely the ceiling of the base 2 logarithm
         of `num').  `size' points to the number of bit to use.
       OR
           `arr' is 0 and the caller is requesting that enough
         space be allocated to hold the representation before the
         translation is made. `size' points to space allocated to
         hold the count of the number of bit needed for the
         conversion (enough for MAXLONG).
   POST: A pointer to a right-aligned array of bits representing the
         unsigned value num has been returned and `size' points to
         the number of `bit's needed to hold the value.
       OR
         the request to allocate space for such an array could not
         be granted

   NOTES: - The first argument is unsigned.  
          - It is bad to pass a `size' that is too small to hold the
            bit array representation of `num' [K&R II, p.100].
          - Should the `size' be the maximum size (if size > 0) even
            if more bits are needed?  The user can always use a filter
            composed of all 1s (see ba_all_assign()) intersected with
            result (see ba_intersection()).
*/
	bool from_long(unsigned long num) 
	{
	   register elem_t i;
		elem_t numbits = sizeof(num)*8;
	   if (!allocate(numbits)) return false;

	   /* usual base conversion algorithm */ 
	   for (i=0; num; num >>= 1, i++) 
	   {
		   /*this has not been validated!!!!!!!!!!!!!!!!!*/ 
		  assign( (numbits - i - 1), (1 == (num & 01)) );
	   }
	   return true;
	}
	/*
	   ba_value()

	   PRE:  arr must have at least elem elements
	   POST: The value of the `elem'th element of arr has been returned
			 (as though `arr' was just a 1-dimensional array of bit)
	   Used by: ba_b2str() and ba_count()
	*/
	inline int valuei(const elem_t &elem) const
	{
		return (bits[elem / BITS_SZ] & (1 << (elem % BITS_SZ))) ? 1 : 0;
	}
	inline bool value(const elem_t &elem) const
	{
		return (bits[elem / BITS_SZ] & (1 << (elem % BITS_SZ))) ? true : false;
	}
/*
   ba_count()

   PRE:  `arr' is an allocated bit array with at least `size'
         elements
   POST: The number of 1 bits in the first `size' elements of `arr'
         have been returned.
   NOTE: if arr is not in canonical form, i.e. if some unused bits
         are 1, then an unexpected value may be returned.
*/
	unsigned long count() const
	{

	  register     unsigned long count; 
	  register     elem_t        i;     

	  static const unsigned bitcount[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 
			2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 
			4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 
			3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 
			3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 
			4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 
			5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 
			2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 
			4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 
			4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 
			6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 
			4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 
			5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 
			6, 6, 7, 6, 7, 7, 8};

	   if (bitcount[(sizeof bitcount / sizeof bitcount[0]) - 1] == BITS_SZ)
	   {
		 /* lookup table will speed this up a lot */ 
		 for (count = 0L, i = 0; i < size; i++)
			count += bitcount[bits[i]];
	   } 
	   else 
	   {
		 for (count = 0L, i = 0; i < bitsize; i++) 
		 {
			if (value(i)) 
			   count++;
		 }
	   }
	   return (count);
	}
	unsigned long count(elem_t start, elem_t end) const
	{
		assert(BITS_SZ == 8);

		static const unsigned bitcount[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 
			2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 
			4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 
			3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 
			3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 
			4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 
			5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 
			2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 
			4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 
			4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 
			6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 
			4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 
			5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 
			6, 6, 7, 6, 7, 7, 8};


		register     unsigned long count; 
		register     elem_t        i;     

		elem_t startblock = start / BITS_SZ;
		elem_t endblock = 1 + (end / BITS_SZ);

		count = bitcount[bitmask_after(start)];
		count += bitcount[bitmask_before(end)];

		for (i = startblock + 1; i < endblock - 1; i++)
			count += bitcount[bits[i]]; 
		return (count);
	}
	/*
	ba_intersection()

	PRE:  `first'  is a bit array of at least `size_first'  elements.
		 `second' is a bit array of at least `size_second' elements.
		 `result' points to enough space to hold the as many elements
		 as the smallest of `size_first' and `size_second';
	   OR
		 `result' points to 0 and such space is to be dynamically 
		 allocated.
	POST: TRUE has been returned and 
		 `result' points to a bit array containing the intersection
		 of the two arrays up to the smallest of the two sizes;
	   OR
		 FALSE has been returned and
		 `result' pointed to 0 (a request was made to allocate 
		 enough memory to store the intersection) but the required 
		 memory could not be obtained.
	NOTE: This runs faster if the `first' array is not smaller than
	   `second'.
	*/
	bool intersection(const bitvector &B, bitvector &result)
	{
	   register elem_t i;
				elem_t numints;   
				unsigned char largest=0, smallest=1;
				const bitvector *bv[2];

		bv[largest] = this;
		bv[smallest] = &B;

		first_is_biggest(bv, &largest, &smallest);

		/* allocate space if required*/ 
		if (!result.allocate(static_cast<int>(bv[largest]->size))) 
		{
		  return false; /* can't get memory, so can't continue */ 
		} 
		else 
		{
            numints = static_cast<elem_t>(B.size);

			for (i=0; i < numints; i++) 
			{
				result.bits[i] = (bv[smallest]->bits[i] & bv[largest]->bits[i]);
			}
			/* bits beyond size_second should be zero -- canonical form */ 
			CANONIZE(result.bits, numints, B.bitsize);
			return true;
	   }
	} 
	void intersection(const bitvector &B)
	{
	   register elem_t i;
				elem_t numblocks;   

		numblocks = B.size > size ? static_cast<elem_t>(size) : static_cast<elem_t>(B.size);
		for (i=0; i < numblocks; i++) bits[i] &= B.bits[i];
	} 
	static void first_is_biggest(const bitvector *bv[2], unsigned char* _big, unsigned char* _small)
	{
	   if (bv[*_big]->size < bv[*_small]->size) 
	   {
		  unsigned char temp;

		  temp = *_big;
		  *_big = *_small;
		  *_small = temp;
	   }
	}
	 /*
	   ba_union()

	   PRE:  `first'  is a bit array of at least `size_first'  elements.
			 `second' is a bit array of at least `size_second' elements.
			 `result' points to enough space to hold the as many elements
			 as the largest of `size_first' and `size_second';
		   OR
			 `result' points to 0 and such space is to be dynamically 
			 allocated. 
	   POST: TRUE has been returned and 
			 `result' points to a bit array containing the union of the
			 two arrays (up to the size of the largest of the two sizes);
		   OR
			 FALSE has been returned and 
			 `result' pointed to 0 (a request was made to allocate 
			 enough memory to store the union) but the required memory 
			 could not be obtained.
	   NOTE: This runs faster if the `first' array is not smaller than
			 `second'.
	 */
	bool union_(const bitvector &B, bitvector &result) 
	{
		register elem_t    i;
				 elem_t    numints;   
				 unsigned char largest=0, smallest=1;
				 const		bitvector *bv[2];
		
	   bv[largest] = this;
	   bv[smallest] = &B;
	   first_is_biggest(bv, &largest, &smallest);

	   if (!result.allocate(static_cast<elem_t>(bv[largest]->bitsize)))
	   {
		  return false; 
	   } 
	   else 
	   {
		  result.copy(*bv[largest]);

		  numints = static_cast<elem_t>(bv[smallest]->size);

		  for (i=0; i < numints; i++) {
			 result.bits[i] |= bv[smallest]->bits[i];
		  }
		  CANONIZE(result.bits, numints, bv[largest]->bitsize);
		  return true;
	   }
	} 
	void union_(const bitvector &B) 
	{
	   register elem_t i;
				elem_t numblocks;   

		numblocks = B.size > size ? static_cast<elem_t>(size) : static_cast<elem_t>(B.size);
		for (i=0; i < numblocks; i++) bits[i] |= B.bits[i];
	}
 /*
   ba_diff()

   PRE:  `first'  is a bit array of at least `size_first'  elements.
         `second' is a bit array of at least `size_second' elements.
         `diff' points to enough space to hold the as many elements
         as the largest of `size_first' and `size_second';
       OR
         `diff' points to 0 and such space is to be dynamically 
         allocated. 
   POST: TRUE has been returned and 
         `diff' points to a bit array containing the union of the
         two arrays (up to the size of the largest of the two sizes);
       OR
         FALSE has been returned and 
         `result' pointed to 0 (a request was made to allocate 
         enough memory to store the result) but the required memory 
         could not be obtained.
   NOTE: This runs faster if the `first' array is not smaller than
         `second'.
 */
	bool diff( const bitvector &B, bitvector &diff) 
	{
		register elem_t    i;
				 elem_t    numints;   
				 unsigned char largest=0, smallest=1;
				 const bitvector *bv[2];
		
	   bv[largest] = this;
	   bv[smallest] = &B;
	   first_is_biggest(bv, &largest, &smallest);

	   if (!diff.allocate(static_cast<elem_t>(bv[largest]->bitsize)))
	   {
		  return false; 
	   } 
	   else 
	   {
		  diff.copy(*bv[largest]);

		  numints = static_cast<elem_t>(bv[smallest]->size);

		  for (i=0; i < numints; i++) 
		  {
			 diff.bits[i] ^= bv[smallest]->bits[i];
		  }
		  CANONIZE(diff.bits, numints, bv[largest]->bitsize);
		  return true;
	   }
	} 
	void diff(const bitvector &B)
	{
	   register elem_t i;
				elem_t numblocks;   

		numblocks = B.size > size ? static_cast<elem_t>(size) : static_cast<elem_t>(B.size);
		for (i=0; i < numblocks; i++) bits[i] ^= B.bits[i];

	}
	/*
	   ba_complement()

	   PRE:  `arr' is a bit array composed of *exactly* `size' elements.
	   POST: All the bits in `arr' have been flipped and `arr' is in 
			canonical form.
	   SEE ALSO: ba_toggle()
	*/
	void complement()
	{
		register elem_t i;

		for (i=0; i < size; i++) bits[i] = ~bits[i];

		/* force canonical form */ 
		CANONIZE(bits, size, bitsize); 
	} 

	/*
	   ba_dotprod()

	   PRE: `first' is an array of at least `size_first' bits.  `second'
			 is an array of at least `size_second' bits.
	   POST: The scalar product of the two vectors represented by the
			 first `size_first' elements of `first' and the first
			 `size_second' elements of `second' have been returned.
	*/
	unsigned long dotprod(const bitvector &B) 
	{
	   register elem_t        i, j;
	   register unsigned long sum = 0L;

	   for (i=0; i < bitsize; i++) {
		  for (j=0; j < B.bitsize; j++) 
		  {
			 sum += (bits[i/BITS_SZ]  & (1<<(i % BITS_SZ))) && (B.bits[j/BITS_SZ] & (1<<(j % BITS_SZ)));
		  }
	   }
	   return (sum);
	}
};
}
#endif