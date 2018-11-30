
#ifdef PTENGINE_EXPORTS
#define PTENGINE_API EXPORT_ATTRIBUTE
#else
#ifdef POINTOOLS_API_INCLUDE
		#define PTENGINE_API 
	#else
		#define PTENGINE_API IMPORT_ATTRIBUTE
	#endif
#endif


