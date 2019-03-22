#ifndef POINTOOLS_WIN32_EXPORT
#define POINTOOLS_WIN32_EXPORT

#ifdef PTWIN32_EXPORTS
#define PTWIN32_API EXPORT_ATTRIBUTE
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define PTWIN32_API 
	#else
		#define PTWIN32_API IMPORT_ATTRIBUTE
	#endif
#endif

#endif

