#ifndef COMMONCLASSES_INTERFACE_111103
#define COMMONCLASSES_INTERFACE_111103

#ifdef COMMONCLASSES_EXPORTS
#define CCLASSES_API __declspec(dllexport)
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define CCLASSES_API 
	#else
		#define CCLASSES_API __declspec(dllimport)
	#endif
#endif
#pragma warning ( disable : 4251 ) /*need to have dll-interface to be used by clients*/ 

#endif