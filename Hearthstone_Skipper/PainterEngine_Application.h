#ifndef PAINTERENGINE_APPLICATION_H
#define PAINTERENGINE_APPLICATION_H
#include "PainterEngine_Startup.h"

//////////////////////////////////////////////////////////////////////////
//Configures
#define  PX_WINDOW_NAME     "炉石传说 酒馆战棋"

#define  PX_WINDOW_WIDTH    240
#define  PX_WINDOW_HEIGHT   180

//memorypool for runtime(bytes)
#define PX_MEMORY_UI_SIZE (1024*1024)
#define PX_MEMORY_RESOURCES_SIZE (1024*1024)
#define PX_MEMORY_GAME_SIZE (64*1024)
//////////////////////////////////////////////////////////////////////////

typedef enum
{
	PX_APPLICATION_STATUS_STANBY,
	PX_APPLICATION_STATUS_SKIPPING,
}PX_APPLICATION_STATUS;

typedef struct
{
	PX_Instance Instance;
	PX_Object *root;
	PX_Object *OneKeySkip;
	px_texture Logo;
	PX_FontModule fm;
	PX_APPLICATION_STATUS status;
	px_dword SkipWaitTime;
	PX_MessageBox msgbox;
	px_bool bFirstInitialized;
}PX_Application;

extern PX_Application App;

px_bool PX_ApplicationInitialize(PX_Application *App);
px_void PX_ApplicationUpdate(PX_Application *App,px_dword elpased);
px_void PX_ApplicationRender(PX_Application *App,px_dword elpased);
px_void PX_ApplicationPostEvent(PX_Application *App,PX_Object_Event e);

#endif
