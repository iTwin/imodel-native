
#include <PTRMI/VariantGraph.h>

#include <PTRMI/ManagerInfo.h>

namespace PTRMI
{

VariantGraphStatics(StringVariantGraph)


void testVariants(void)
{

	ManagerInfoVariantGraph	vm;

	ManagerInfoVariantGraph::KeyPath	p0, p1, p2, p3;

	p0.push_back(std::wstring(L"MyBool"));
	vm.add(p0, StringVariantGraph::VGVariantBool(true));
	p0.pop_back();

	PTRMI::GUID guid;
	guid.generate();

	ManagerInfoVariantGraph::VGVariantGUID vg(guid);
	p0.push_back(std::wstring(L"MyGUID"));
	vm.add(p0, vg);

	p1.push_back(std::wstring(L"G1"));

	p2 = p1;
	p2.push_back(std::wstring(L"String"));
	vm.add(p2, StringVariantGraph::VGVariantWString(std::wstring(L"Hello")));

	p1.push_back(std::wstring(L"G2"));

	p2 = p1;
	p2.push_back(std::wstring(L"Int1"));
	vm.add(p2, VariantInt<unsigned char>(5));

	p2 = p1;
	p2.push_back(std::wstring(L"Int2"));
	vm.add(p2, VariantInt<unsigned char>(6));

	p2 = p1;
	p2.push_back(std::wstring(L"UInt1"));
	vm.add(p2, VariantUnsignedInt<unsigned char>(7));

	p2 = p1;
	p2.push_back(std::wstring(L"Float1"));
	vm.add(p2, VariantFloat<unsigned char>(1.23));

	vm.writeFile(L"C:\\Variants1.txt");

	DataBuffer	buffer;

	buffer.createInternalBuffer(1024*1024);

	vm.write(buffer);

	StringVariantGraph	vm2;

	vm2.read(buffer);

	vm2.writeFile(L"C:\\Variants2.txt");


	StringVariantGraph	vm3;

	vm3.readFile(L"C:\\Variants2.txt");

	vm3.writeFile(L"C:\\Variants3.txt");

	StringVariantGraph	vm4, vm5;

	vm4 = vm3;

	vm5 = vm4;

	vm5.writeFile(L"C:\\VariantsCopy.txt");
}


}
