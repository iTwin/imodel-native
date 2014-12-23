
#pragma once

#include <PTRMI/VariantGraph.h>

#define VARIANT_TYPE_NAME_GUID							L"GUID"

#define ManagerVariantGraphStatics(VG)	VG::VariantType			VG::VGVariantGUID::type;			\
										VG::VariantTypeName		VG::VGVariantGUID::typeName;		\
										bool					VG::managerInfoVariantGraphInitialized = false;


namespace PTRMI
{


template<typename VT> class VariantGUID : public VariantT<PTRMI::GUID, VT>
{
public:

	typedef VariantGUID		thisType;

public:

	VariantGUID(void) : VariantT<PTRMI::GUID, unsigned char>()
	{

	}

	VariantGUID(const PTRMI::GUID &guid)
	{
		set(guid);
	}

	bool copy(Variant<VT> *variant)
	{
		return copyTyped<thisType>(variant);
	}

	void readString(std::wifstream &in)
	{
		std::wstring	str;

		in >> str;

		if(in.good())
		{
			get().setHexString(str);
		}
	}

	void getString(std::wofstream &in, std::wstring &string)
	{
															// Return Hex string of GUID
		get().getHexString(string);
	}

};


inline std::wostringstream& operator << (std::wostringstream &out, PTRMI::GUID &guid)
{
	std::wstring string;

	guid.getHexString(string);

	out << string;

	return out;
}



class ManagerInfoVariantGraph : public StringVariantGraph
{

public:

	typedef VariantGUID<VariantType>	VGVariantGUID;

public:

	const static VariantType			variantTypeGUID			= 20;

	typedef ManagerInfoVariantGraph::VGVariantUnsignedShort	ManagerInfoVariantMessageVersion;

protected:

	static bool managerInfoVariantGraphInitialized;

public:

	ManagerInfoVariantGraph(void)
	{
		if(getInitialized() == false)
		{
			addType<VGVariantGUID>(variantTypeGUID, std::wstring(VARIANT_TYPE_NAME_GUID));

			setInitialized(true);
		}
	}


	void setInitialized(bool init)
	{
		managerInfoVariantGraphInitialized = init;
	}

	bool getInitialized(void)
	{
		return managerInfoVariantGraphInitialized;
	}

};


} // End PTRMI namespace