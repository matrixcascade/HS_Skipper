#include "PainterEngine_Application.h"
#define PSAPI_VERSION 1  

#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>   
#pragma comment(lib,"psapi.lib")

PX_Application App;

//netsh advfirewall firewall add rule name="nohhie" dir=out program="D:\Program Files (x86)\Hearthstone\Hearthstone.exe" action=block
//netsh advfirewall firewall show rule name="nohhie"
//netsh advfirewall firewall set rule name="nohhie" new enable=yes

px_bool PX_GetProcessPath(const char *processName, char* buffer)  
{   
	HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 procEntry = { 0 };
	BOOL bRet;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	bRet = Process32First(procSnap,&procEntry);
	if(procSnap == INVALID_HANDLE_VALUE)
	{
		return PX_FALSE;
	}

	while(bRet)
	{
		if (PX_strequ(procEntry.szExeFile,processName))
		{
			//
			FILE *pf=fopen("resource/hs_path.cfg","rb");
			if (!pf)
			{
				HANDLE h_Process=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,procEntry.th32ProcessID);
				if(!GetModuleFileNameEx(h_Process,NULL,buffer,MAX_PATH))
				{
					CloseHandle(h_Process);
					return PX_FALSE;
				}
				CloseHandle(h_Process);
				pf=fopen("resource/hs_path.cfg","wb");
				if (pf)
				{
					fwrite(buffer,1,PX_strlen(buffer)+1,pf);
				}
				fclose(pf);
			}
			else
			{
				px_int size;
				fseek(pf,0,SEEK_END);
				size=ftell(pf);
				fseek(pf,0,SEEK_SET);
				fread(buffer,1,size,pf);
				fclose(pf);
				if (GetFileAttributes(buffer)==INVALID_FILE_ATTRIBUTES)
				{
					HANDLE h_Process=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,procEntry.th32ProcessID);
					if(!GetModuleFileNameEx(h_Process,NULL,buffer,MAX_PATH))
					{
						CloseHandle(h_Process);
						return PX_FALSE;
					}
					CloseHandle(h_Process);
					pf=fopen("resource/hs_path.cfg","wb");
					if (pf)
					{
						fwrite(buffer,1,PX_strlen(buffer)+1,pf);
					}
					fclose(pf);
				}
			}
			return PX_TRUE;
		}
		bRet = Process32Next(procSnap,&procEntry);
	}
	return PX_FALSE;
}  


px_void PX_Object_OnOnekeyDown(PX_Object *pObject,PX_Object_Event e,px_void *ptr)
{
	px_char cmd[1024];
	PX_Application *App=(PX_Application *)ptr;
	px_char hsPath[MAX_PATH]={0};
	if (!PX_GetProcessPath("Hearthstone.exe",hsPath))
	{
		PX_MessageBoxAlertOk(&App->msgbox,(px_char *)L"炉石传说未运行");
		return;
	}
	if (App->bFirstInitialized)
	{
		WinExec("netsh advfirewall firewall delete rule name=\"hsconnect\"",SW_HIDE);
		Sleep(100);
		PX_sprintf1(cmd,sizeof(cmd),"netsh advfirewall firewall add rule name=\"hsconnect\" dir=out program=\"%1\" action=block",PX_STRINGFORMAT_STRING(hsPath));
		WinExec(cmd,SW_HIDE);
		App->bFirstInitialized=PX_FALSE;
		Sleep(100);
	}
	WinExec("netsh advfirewall firewall set rule name=\"hsconnect\" new enable=yes",SW_HIDE);

	App->status=PX_APPLICATION_STATUS_SKIPPING;
	App->SkipWaitTime=0;

}


px_bool PX_ApplicationInitialize(PX_Application *App)
{

	App->status=PX_APPLICATION_STATUS_STANBY;
	if(!PX_FontModuleInitialize(&App->Instance.runtime.mp_resources,&App->fm))
	{
		return PX_FALSE;
	}
	
	if (!PX_LoadFontModuleFromFile(&App->fm,"resource/chs.pxf"))
	{
		return PX_FALSE;
	}
	if (!PX_LoadTextureFromFile(&App->Instance.runtime.mp_resources,&App->Logo,"resource/hs.traw"))
	{
		return PX_FALSE;
	}

	if(!PX_MessageBoxInitialize(&App->Instance.runtime,&App->msgbox,&App->fm,PX_WINDOW_WIDTH,PX_WINDOW_HEIGHT))
	{
		return PX_FALSE;
	}

	App->msgbox.PX_MESSAGEBOX_STAGE_1_HEIGHT=10;
	App->msgbox.PX_MESSAGEBOX_STAGE_2_HEIGHT=80;
	App->msgbox.btn_Ok->Visible=PX_FALSE;
	App->bFirstInitialized=PX_TRUE;
	App->root=PX_ObjectCreate(&App->Instance.runtime.mp_ui,PX_NULL,0,0,0,0,0,0);
	App->OneKeySkip=PX_Object_PushButtonCreate(&App->Instance.runtime.mp_ui,App->root,20,135,200,40,"",PX_COLOR(255,0,255,0));
	PX_Object_PushButtonSetBackgroundColor(App->OneKeySkip,PX_COLOR(0,0,0,0));
	PX_Object_PushButtonSetBorderColor(App->OneKeySkip,PX_COLOR(255,0,255,0));
	PX_Object_PushButtonSetCursorColor(App->OneKeySkip,PX_COLOR(64,0,255,0));
	PX_Object_PushButtonSetPushColor(App->OneKeySkip,PX_COLOR(128,0,255,0));

	PX_ObjectRegisterEvent(App->OneKeySkip,PX_OBJECT_EVENT_EXECUTE,PX_Object_OnOnekeyDown,App);

	return PX_TRUE;
}

px_void PX_ApplicationUpdate(PX_Application *App,px_dword elpased)
{
	if (App->msgbox.show)
	{
		PX_MessageBoxUpdate(&App->msgbox,elpased);
		return;
	}

	if (App->status==PX_APPLICATION_STATUS_SKIPPING)
	{
		if (App->SkipWaitTime<2100)
		{
			App->SkipWaitTime+=elpased;
		}
		else
		{
			WinExec("netsh advfirewall firewall set rule name=\"hsconnect\" new enable=no",SW_HIDE);
			App->status=PX_APPLICATION_STATUS_STANBY;
		}
	}

}

px_void PX_ApplicationRender(PX_Application *App,px_dword elpased)
{
	px_surface *renderSurface=&App->Instance.runtime.RenderSurface;

	if (App->msgbox.show)
	{
		PX_SurfaceClear(renderSurface,0,0,App->Instance.runtime.RenderSurface.width-1,App->Instance.runtime.RenderSurface.height-1,PX_COLOR(255,255,255,255));
		PX_MessageBoxRender(renderSurface,&App->msgbox,elpased);
		return;
	}


	switch(App->status)
	{
	case PX_APPLICATION_STATUS_STANBY:
		{
			PX_ObjectSetVisible(App->OneKeySkip,PX_TRUE);
		}
		break;
	case PX_APPLICATION_STATUS_SKIPPING:
		{
			PX_ObjectSetVisible(App->OneKeySkip,PX_FALSE);
		}
		break;
	}

	PX_SurfaceClear(renderSurface,0,0,App->Instance.runtime.RenderSurface.width-1,App->Instance.runtime.RenderSurface.height-1,PX_COLOR(255,0,0,0));
	PX_TextureRender(renderSurface,&App->Logo,PX_WINDOW_WIDTH/2,PX_WINDOW_HEIGHT/2-24,PX_TEXTURERENDER_REFPOINT_CENTER,PX_NULL);

	PX_ObjectRender(&App->Instance.runtime.RenderSurface,App->root,elpased);

	switch(App->status)
	{
	case PX_APPLICATION_STATUS_STANBY:
		{
			PX_FontModuleDrawText(renderSurface,\
				(px_int)(App->OneKeySkip->x+App->OneKeySkip->Width/2),\
				(px_int)(App->OneKeySkip->y+App->OneKeySkip->Height/2+6),\
				(px_word *)L"一键跳过战斗动画",\
				PX_COLOR(255,0,255,0),\
				&App->fm,\
				PX_FONT_ALIGN_XCENTER\
				);
		}
		break;
	case PX_APPLICATION_STATUS_SKIPPING:
		{
			PX_FontModuleDrawText(renderSurface,\
				(px_int)(App->OneKeySkip->x+App->OneKeySkip->Width/2),\
				(px_int)(App->OneKeySkip->y+App->OneKeySkip->Height/2+6),\
				(px_word *)L"整活中",\
				PX_COLOR(255,0,255,0),\
				&App->fm,\
				PX_FONT_ALIGN_XCENTER\
				);
		}
		break;
	}

	
}

px_void PX_ApplicationPostEvent(PX_Application *App,PX_Object_Event e)
{
	if (App->msgbox.show)
	{
		if (e.Event==PX_OBJECT_EVENT_CURSORDOWN)
		{
			PX_MessageBoxClose(&App->msgbox);
		}
		return;
	}

	PX_ObjectPostEvent(App->root,e);
}

