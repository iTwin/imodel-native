#pragma warning (disable : 4251 )
#pragma warning ( disable : 4786 )

#ifdef PCLOUD_EXPORTS
#define PCLOUD_API __declspec(dllexport)
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define PCLOUD_API 
	#else
		#define PCLOUD_API __declspec(dllimport)
	#endif
#endif



