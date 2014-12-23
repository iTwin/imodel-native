#pragma warning (disable : 4251 )
#pragma warning ( disable : 4786 )

#ifdef PTENGINE_EXPORTS
#define PTENGINE_API __declspec(dllexport)
#else
#ifdef POINTOOLS_API_INCLUDE
		#define PTENGINE_API 
	#else
		#define PTENGINE_API __declspec(dllimport)
	#endif
#endif


