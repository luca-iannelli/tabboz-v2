#include <windows.h>
#include <wchar.h>
#include "desktop.h"
typedef struct { DWORD magic,dpi; WINDOWPLACEMENT placement; } WindowState;
static void state_path(wchar_t *path) {
    wcscpy_s(path,32768,TabbozStorePath());wchar_t *slash=wcsrchr(path,L'\\');
    if(slash)wcscpy_s(slash+1,32768-(slash+1-path),L"window-state.bin");
}
void TabbozWindowState(HWND hwnd,int save) {
    if(TabbozTesting)return;
    wchar_t path[32768];state_path(path);WindowState state={0};DWORD count;
    if(save) {
        state.magic=0x325a4254;state.dpi=GetDpiForWindow(hwnd);state.placement.length=sizeof(WINDOWPLACEMENT);
        if(!GetWindowPlacement(hwnd,&state.placement))return;
        HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(file!=INVALID_HANDLE_VALUE){WriteFile(file,&state,sizeof(state),&count,NULL);CloseHandle(file);}
    } else {
        HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if(file==INVALID_HANDLE_VALUE)return;
        BOOL ok=ReadFile(file,&state,sizeof(state),&count,NULL);CloseHandle(file);
        if(!ok||count!=sizeof(state)||state.magic!=0x325a4254||state.dpi<72||state.dpi>768)return;
        RECT work;SystemParametersInfo(SPI_GETWORKAREA,0,&work,0);
        RECT *r=&state.placement.rcNormalPosition;int dpi=GetDpiForWindow(hwnd);
        int width=MulDiv(r->right-r->left,dpi,state.dpi),height=MulDiv(r->bottom-r->top,dpi,state.dpi);
        if(width<640||height<320)return;
        if(width>work.right-work.left)width=work.right-work.left;if(height>work.bottom-work.top)height=work.bottom-work.top;
        if(r->left<work.left||r->left+width>work.right)r->left=work.left+(work.right-work.left-width)/2;
        if(r->top<work.top||r->top+height>work.bottom)r->top=work.top+(work.bottom-work.top-height)/2;
        r->right=r->left+width;r->bottom=r->top+height;
        state.placement.length=sizeof(WINDOWPLACEMENT);state.placement.showCmd=state.placement.showCmd==SW_SHOWMAXIMIZED?SW_SHOWMAXIMIZED:SW_SHOWNORMAL;
        SetWindowPlacement(hwnd,&state.placement);
    }
}
