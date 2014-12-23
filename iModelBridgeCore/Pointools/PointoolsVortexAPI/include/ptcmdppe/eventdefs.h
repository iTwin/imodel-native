#ifndef PT_EVENT_DEFINITIONS_HEADER
#define PT_EVENT_DEFINITIONS_HEADER

/*events*/
#define CP_MOUSE_L_DBL			0x01
#define CP_MOUSE_L_UP			0x02
#define CP_MOUSE_L_DOWN			0x04
#define CP_MOUSE_R_UP			0x08
#define CP_MOUSE_R_DOWN			0x10
#define CP_MOUSE_M_UP			0x20
#define CP_MOUSE_M_DOWN			0x40
#define CP_MOUSE_MOVE			0x80
#define CP_MOUSE_WHEEL_UP		0x100
#define CP_MOUSE_WHEEL_DOWN		0x200
#define CP_MOUSE_MASK			0x3ff

#define CP_KEY_SHIFT			0x400
#define KEY_ALT					0x800
#define KEY_CTRL				0x1000
#define KEY_MASK				0x1c00

#define CP_SCRIPT_ADVANCE		0x8000
#define CP_TERMINATE			0x10000
#define CP_INITIALIZE			0x20000
#define CP_BUFFER_SWAP			0x40000
#define CP_PREPARE_GL			0x80000
#define CP_DRAW_GL				0x100000
#define CP_DRAW_GL_OVERLAY		0x200000
#define CP_DRAW_GL_FRONT		0x400000

#define CP_STARTUP_EVENT		0x800000
#define CP_LOG_EVENT			0x1000000

/*flags*/
#define CPF_CALL_ONLY				0x01
#define CPF_NO_REPEAT				0x02
#define CPF_RECIEVE_CHILD_EVENTS	0x04
#define CPF_POP_AFTER_CALL			0x08
#define CPF_CALL_FAST				0x10
#define CPF_LOOP_CMD				0x20
#define CPF_EXPECT_USER_DROP		0x40
#define CPF_FULL_VIEW_LOCK			0x80
#define CPF_HALF_VIEW_LOCK			0x100

/*flags 0 - 0xff reserved for command pipe*/
#define CPF_USER_FLAGS				0xffffff00

//cursors
#define CPCUR_DEFAULT	0
#define CPCUR_ARROW		1
#define CPCUR_CROSS		2
#define CPCUR_WAIT		3
#define CPCUR_INSERT	4
#define CPCUR_HAND		5
#define CPCUR_HELP		6	
#define CPCUR_MOVE		7
#define CPCUR_NS		8	
#define CPCUR_WE		9
#define CPCUR_NWSE		10
#define CPCUR_NESW		11
#define CPCUR_NONE		12

#include <boost\function\function0.hpp>
#include <boost\function\function1.hpp>
#include <boost\function\function2.hpp>
#include <boost\function\function3.hpp>

#ifdef PT_ORIGINAL_EVENTINFO
struct EventInfo
{
	unsigned int	event;
	unsigned int	mbutton;
	float  			mouse[3];
	float 			actual[3];
	float			raydir[3];
	bool			actual_valid;
	unsigned int	key;
	int				script_pos;
};
#else
struct EventInfo
{
	unsigned int	event;
	unsigned int	mbutton;
	float  			mouse[3];
	float 			actual[3];
	float			raydir[3];
	unsigned int	key;
	int				script_pos;
	const char		*event_string;
	bool			actual_valid;
	char			res[3];
};
#endif

namespace ptapp
{
typedef boost::function0<std::string>						queryCB;
typedef boost::function2<bool, const EventInfo &, void*>	eventCB;
typedef boost::function0<void>								resetCB;
typedef boost::function0<void>								functionCB;
typedef boost::function2<bool, unsigned int, unsigned int>	envtestCB;
}

#endif
