/*--------------------------------------------------------------------------*/ 
/*	Pointools tool class definition											*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 26 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLS_TOOL_BASECLASS_HEADER
#define POINTOOLS_TOOL_BASECLASS_HEADER


#include <pt/geomtypes.h>
#include <pttool/pttool.h>
#include <ptcmdppe/eventdefs.h>
#include <ptmodm/Module.h>
#ifdef HAVE_OPENGL
#include <ptgl/glCamera.h>
#endif
#include <pt/parametermap.h>

namespace ptapp
{
class Tools;
}
namespace pt
{

class PTTOOL_API Tool
{
	public:
		virtual bool initialize();

		enum LicenseState
		{
			LicEvaluation		= 1,
			LicInvalid			= 2,
			LicLicensed			= 4,
			LicCorrupt			= 8,
			LicExtended			= 16,
			LicFileMissing		= 32,
			LicInternalError	= 64,
			LicNotRequired		= 128,
			LicUndetermined		= 256,
			LicExpired			= 512,
			LicInvalidFile		= 1024
		};
		enum CheckFlags
		{
			CheckUnchecked		= 32
		};
		friend class ptapp::Tools;

		/*red herrings */ 
		virtual void *licenseInfo() const;
		virtual bool isLicenseValid() const;

		/*command stack control*/ 
		static void drop();
		static void callCmd(const char*cmd, pt::ParameterList *args=0);
		static void pushCmd(const char *cmd);

		/*results*/ 
		static bool peepResult(int &i) ;
		static void pushResult(const int &i);
		static bool popResult(int &i);
		
 		static bool peepResult(bool &b); 
		static void pushResult(const bool &b);
		static bool popResult(bool &b);
		
		static bool peepResult(float &i);		
		static void pushResult(const float &f);
		static bool popResult(float &f);
		
		static bool peepResult(pt::vector3 &v);		
		static void pushResult(const pt::vector3 &v);
		static bool popResult(pt::vector3 &v);

		static bool peepResult(void *&ptr);
		static void pushResult(void *ptr);
		static bool popResult(void *&ptr);	

		/*args*/ 
		static pt::ParameterList *getCallParameters();
		static void requestModule(pt::RequestObj *r);

	protected:

		/*register commands*/ 
		static bool registerCmd(const std::string &cmd, ptapp::functionCB cb, unsigned int flags = 0);
		static bool registerEventCmd(const std::string &cmd, ptapp::eventCB cb, 
									unsigned int eventmask, const char* prompt, 
									unsigned int cursor = 0, unsigned int flags = 0);
		static bool registerScript(const std::string &name, const std::string &script, 
									const std::string &prompts, ptapp::eventCB cb = 0, 
									unsigned int flags = 0);
		
		static bool registerEventCallback(ptapp::eventCB cb);

		/*ui*/ 
		static void buildWindow(const char *script, pt::ParameterMap *t);
		static void showWindow(const char*win);
		static void hideWindow(const char*win);
		static void updateUI(const char *spec);
		static void showTabbedPanel(const char *panel);
		static void* panelPointer(const char *panel);

		static void appendText(const char*wid, const char *text);
		static void saveText(const char*wid);
		static void loadText(const char*wid, const char *filename);

		static void clearText(const char*wid);
		static void getText(const char*wid, char *buffer);

		/*front buffer draw*/ 
		static void begin3dDraw();
		static void beginPixelDraw();
		static void beginXORDraw();
		static void endXORDraw();
		static void endPixelDraw();
		static void endDraw();
#ifdef HAVE_OPENGL
		static ptgl::Camera *camera();
#endif
		static void redrawViewport();
		static void refreshViewport();
		static void dynamicRedrawViewport();

		static void pushViewLockFull();
		static void pushViewLockHalf();
		static void popViewLock();

		static void setCursor(unsigned int cursor);

		static bool okcancelMessage(const char *title, const char *message);
		static void okMessage(const char *title, const char *message);
		static void errorMessage(const char *title, const char *message);
		static bool yesnoMessage(const char*title, const char *message);

		static const char* getOpenFilepath(const char *extension, const char *filetype, const char *initialpath=0);
		static const char* getSaveFilepath(const char *extension, const char *filetype, const char *initialpath=0);

		/* licensing */ 
		struct VersionInfo
		{
			unsigned char	version[4];
			unsigned int	*check;
			void*			license_status;
			const char		*localpwd;
			const char		*globalpwd;
			const char		*plugin;
			int				execs_left;
			int				days_left;
		};

		virtual void *versionInfo() const;
};
}
#endif