
#include <ptengine/RangeSet.h>
#include <vector>
#include <PTRMI/DataBuffer.h>

void testRange();

void testRangeSet();

void testDataBuffer();


void f(void)
{
	testDataBuffer();

	testRange();

	testRangeSet();
}


void testDataBuffer(void)
{
	PTRMI::DataBuffer	dataBuffer;

	dataBuffer.createInternalBuffer(4);

	unsigned int t;
	unsigned int n = 10;

	typedef float Value;

	Value v;

	PTRMI::GUID guid, guidIn;
	PTRMI::URL url, urlIn;

	for(t = 1; t <= n; t++)
	{
		guid.generate();

		wchar_t b[256];
		wsprintf(b, L"This is %d", t);

		url = &(b[0]);

		dataBuffer << guid;
		dataBuffer << url;
	}

	for(t = 1; t <= n / 2; t++)
	{
		dataBuffer >> guidIn;
		dataBuffer >> urlIn;
	}


	for(t = 1; t <= n * 2; t++)
	{
		guid.generate();

		wchar_t b[256];
		wsprintf(b, L"This is %d", t);

		url = &(b[0]);

		dataBuffer << guid;
		dataBuffer << url;
	}


	for(t = 1; t <= n * 2 + n/2; t++)
	{
		dataBuffer >> guidIn;
		dataBuffer >> urlIn;
	}


}


void testRange(void)
{
	typedef pointsengine::Range<unsigned int, std::vector<void *>> TestRange;
	TestRange	r1, r2;

	r1.set(7, 8);
	r2.set(4, 5);

	r1.setSearchMode(TestRange::SearchModeAdjacentMax);
	r2.setSearchMode(TestRange::SearchModeNULL);

	bool a = r1 < r2;
	bool b = r2 < r1;

	r1.setSearchMode(TestRange::SearchModeEquals);
	r2.setSearchMode(TestRange::SearchModeNULL);
	
	r1.set(1, 3);
	r2.set(2, 4);
															// Test equality
	bool c = r1 < r2 || r2 < r1;


	int x = 1;
}


void testRangeSet(void)
{
	typedef pointsengine::Range<unsigned int, std::vector<void *>> TestRange;
	typedef pointsengine::RangeSet<unsigned int, std::vector<void *>> TestRangeSet;

	TestRangeSet	set;

	TestRange r[4];

	r[0].set(1, 3);
	r[1].set(4, 6);
	r[2].set(7, 9);
	r[3].set(10, 12);

	set.insert(r[0]);
	set.insert(r[2]);
	set.insert(r[1]);
	set.insert(r[3]);
}
