/**
 @file g3derror.cpp
 
 @author Morgan McGuire, graphics3d.com
  
 @created 2002-02-01
 @edited  2003-02-15
 */

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include "G3D/g3derror.h"

namespace G3D {

/**
 @internal
 */
namespace _internal {

/**
 @internal
 */
ErrorConstant _utility_error(
    const char*			level,
    const char*			message,
    bool				showPrompt,
    const char*			filename,
    int					line) {

    time_t t;
    time(&t);

    //fprintf(stderr, "__________________________________________________________________\n");
    //fprintf(stderr, "%s  %s at %s:%d\n", ctime(&t), level, filename, line);
    //fprintf(stderr, "\"%s\"\n\n", message);

      if (showPrompt) {
        if (strcmp(level, "Critical Error") == 0) {
            char* choice[] = {"Quit"};
            prompt(level, message, (const char **)choice, 1);
            return QUIT_ERROR;
        } else {
            char* choice[] = {"Ignore", "Quit"};
            if (prompt(level, message, (const char **)choice, 2) == 1) {
                return QUIT_ERROR;
            }
        }
    }

    return IGNORE_ERROR;
}


/**
 @internal
**/
ErrorConstant _utility_error(const char *level, const std::string &message, bool showPrompt, const char *filename, int line) {
	return _utility_error(level, message.c_str(), showPrompt, filename, line);
}

/**
 @internal
 */
ErrorConstant _utility_error(const std::string &level, const std::string &message, bool showPrompt,const char *filename, int line){
	return _utility_error(level.c_str(), message.c_str(), showPrompt, filename, line);
}

} // namespace _internal

} // namespace G3D

#endif