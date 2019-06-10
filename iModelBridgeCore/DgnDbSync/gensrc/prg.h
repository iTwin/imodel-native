/*__BENTLEY_INTERNAL_ONLY__*/
/* String representations of version information. */
#define RELEASE_DATE __DATE__
#define V_CERTIFIED 1
#define REL_V "02"
#define MAJ_V "01"
#define MIN_V "00"
#define SUBMIN_V "999"
#define VERSION_STRING ""
#define V_COMMENT ""
#define REL_VW L"02"
#define MAJ_VW L"01"
#define MIN_VW L"00"
#define SUBMIN_VW L"999"
#define VERSION_STRING_WIDE L""
#define V_COMMENTW L""
#if !defined (VERSION_COMPTOOLS)
# define VERSION_COMPTOOLS REL_V "." MAJ_V "." MIN_V "." SUBMIN_V " " V_COMMENT
#endif
#define DEV_BUILD 1
#define DEVELOPMENT_NUMBER 0
