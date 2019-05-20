#include <pt/parametermap.h>

#ifndef POINTOOLS_SERIALIZE_PARAMETER_MAP
#define POINTOOLS_SERIALIZE_PARAMETER_MAP

namespace pt
{

struct SerializeWriteVisitor
{
	SerializeWriteVisitor(void *buffer, int numparams) : pos(0), ptr((unsigned char*)buffer)
	{
		/*version */ 
		unsigned char s = 2;
		ptr[pos++] = s;	

		/* overall size */ 
		writeParam(numparams);
	}

	template <class T>
	void operator() (const T &v)
	{
		unsigned char type_index = Variant(v).which();
		writeIdentifier();
		writeParam(type_index);
		writeParam(v);
	}

	void operator() (const pt::String &v)
	{
		unsigned char type_index = Variant(v).which();
		writeIdentifier();
		writeParam(type_index);
		writeString(v);
	}
	std::string identifier;

private:
	template <class T>
	void writeParam(const T &v)
	{
		memcpy(&ptr[pos], &v, sizeof(T));
		pos += sizeof(T);
	}
	void writeString(const pt::String &s)
	{
		unsigned char len = s.length();
		writeParam(len);
		memcpy(&ptr[pos], s.c_str(), len);
		pos += len;
	}
	void writeIdentifier()
	{
		writeString(pt::String(identifier.c_str()));
	}
	unsigned char *ptr;
	unsigned int pos;
};
struct SerializeSizeVisitor
{
	SerializeSizeVisitor() : size(5) {};
	template <class T>
		void operator() (const T &v)
	{ 
		size += 2; /*type_index + length of identifier */ 
		size += identifier.length();
		size += sizeof(T); 
	}
	int size;
	std::string identifier;
};
struct SerializeRead
{
	SerializeRead(void *buffer, ParameterMap *pm) : pos(0), ptr((unsigned char*)buffer)
	{
		/* create a new parameter map and copy values into pm */ 
		ParameterMap readmap;
		unsigned char version = ptr[0];
		ptr++;
		if (version == 2)
		{
			/* overall size */ 
			int numparams = 0;
			readParam(numparams);
			
			int i=0;
			for (;i<numparams;i++)
			{
				std::string id = readIdentifier();
				unsigned char type_index;
				readParam(type_index);

				/* check type */ 
				insertValue(&readmap, id, type_index);
			}
			pm->copyValues(readmap);
			success = true;
		}
		success = false;
	}
	void insertValue(ParameterMap *pm, const std::string &id, unsigned char typeindex)
	{
		switch (typeindex)
		{
		case 0: { ttl::meta::get<Variant::list, 0>::type v; readParam(v); pm->insert(id, v); } break;
		case 1: { ttl::meta::get<Variant::list, 1>::type v; readParam(v); pm->insert(id, v); } break;
		case 2: { ttl::meta::get<Variant::list, 2>::type v; readParam(v); pm->insert(id, v); } break;
		case 3: { ttl::meta::get<Variant::list, 3>::type v; readParam(v); pm->insert(id, v); } break;
		case 4: { ttl::meta::get<Variant::list, 4>::type v; readParam(v); pm->insert(id, v); } break;
		case 5: { ttl::meta::get<Variant::list, 5>::type v; readParam(v); pm->insert(id, v); } break;
		case 6: { ttl::meta::get<Variant::list, 6>::type v; readParam(v); pm->insert(id, v); } break;
		case 7: { ttl::meta::get<Variant::list, 7>::type v; readParam(v); pm->insert(id, v); } break;
		case 8: { ttl::meta::get<Variant::list, 8>::type v; readParam(v); pm->insert(id, v); } break;
		case 9: { ttl::meta::get<Variant::list, 9>::type v; readParam(v); pm->insert(id, v); } break;
		}
	}

private:
	template <class T>
	void readParam(T &v)
	{
		memcpy(&v, &ptr[pos], sizeof(T));
		pos += sizeof(T);
	}
	std::string readIdentifier()
	{
		return std::string(readString().c_str());
	}
	pt::String readString()
	{
		unsigned char len;
		readParam(len);
		char buffer[256];
		memcpy(buffer, &ptr[pos], len);
		pos += len;
		buffer[len] = '\0';
		return pt::String(buffer);
	}

	int pos;
	bool valid;
	bool success;
	unsigned char *ptr;
	bool res;
};
}
#endif
