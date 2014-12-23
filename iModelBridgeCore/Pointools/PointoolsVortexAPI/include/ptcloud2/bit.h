#ifndef _BIT_MANIPULATION_HEADER_290403
#define _BIT_MANIPULATION_HEADER_290403

#define bitsize(a) sizeof(a) * 8

namespace _bit
{
	struct _count
	{
		static int count(unsigned long a)
		{
			return _count2(a);
		}
		static int count(unsigned int a)
		{
			unsigned int bit = 1;
			int c = 0;
			for (int i=0; i<32; i++)
			{
				if (a & bit) c++;
				bit <<= 1;
			}
			return c;
		}
		static int count(unsigned short a)
		{
			if (!a) return 0;
			if (a == 0xffff) return 16;

			int c = 0;

			if (a & 0x0001) c++;
			if (a & 0x0002) c++;
			if (a & 0x0004) c++;
			if (a & 0x0008) c++;
			if (a & 0x0010) c++;
			if (a & 0x0020) c++;
			if (a & 0x0040) c++;
			if (a & 0x0080) c++;
			if (a & 0x0100) c++;
			if (a & 0x0200) c++;
			if (a & 0x0400) c++;
			if (a & 0x0800) c++;
			if (a & 0x1000) c++;
			if (a & 0x2000) c++;
			if (a & 0x4000) c++;
			if (a & 0x8000) c++;
		}
		static int count(unsigned char a)
		{
			if (!a) return 0;
			if (a == 0xff) return 8;
			int c = 0;

			if (a & 0x01) c++;
			if (a & 0x02) c++;
			if (a & 0x04) c++;
			if (a & 0x08) c++;
			if (a & 0x10) c++;
			if (a & 0x20) c++;
			if (a & 0x40) c++;
			if (a & 0x80) c++;
		}		
		
		//int 32 optimized low-level
		static int _count1(unsigned int a)
		{
			int c;
			c = 0;
			while( a != 0 )       /* until no bits remain, */
			{
				c++;                /* "tally" ho, then */
				a = a &~ -a;        /* clear lowest bit */
			}
			return(c);
		}

		static int _count2(unsigned long a)              /* a: 32  1-bit tallies */
		{
			a = (a&0x55555555) + ((a>>1) &(0x55555555));  /* a: 16  2-bit tallies */
			a = (a&0x33333333) + ((a>>2) &(0x33333333));  /* a:  8  4-bit tallies */
			a = (a&0x07070707) + ((a>>4) &(0x07070707));  /* a:  4  8-bit tallies */
			/* a %= 255; return(a); may replace what follows */
			a = (a&0x000F000F) + ((a>>8) &(0x000F000F));  /* a:  2 16-bit tallies */
			a = (a&0x0000001F) + ((a>>16)&(0x0000001F));  /* a:  1 32-bit tally */
			return(a);
		}

		static int _count3(unsigned long a)
		{
			unsigned long mask, sum;
			if (a == 0)                 /* a common case */
				return(0);
			else if (a == 0xffffffff)   /* ditto, but the early return is essential: */
				return(32);             /* it leaves mod 31 (not 33) final states */
			mask = 0x42108421L;
			sum = a & mask;             /* 5x: accumulate through a 1-in-5 sieve */
			sum += (a>>=1) & mask;
			sum += (a>>=1) & mask;
			sum += (a>>=1) & mask;
			sum += (a>>=1) & mask;
			sum %= (mask = 31);         /* casting out mod 31 (save that constant) */
			return(sum ? sum : mask);   /* return bits (zero indicated 31 bits on) */
		}
	};
};
#endif