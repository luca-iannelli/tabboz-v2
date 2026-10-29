#include "os.h"
#include "zarrosim.h"
#include "desktop.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <windowsx.h>

int TabbozTesting;
extern int TabbozDrawSplash(HINSTANCE,HDC,const RECT *);
extern void TabbozWindowState(HWND,int);
typedef struct { DLGPROC handler; int id; int scroll, extent; HFONT font, title; } DialogContext;
static HBRUSH background;
static HHOOK keyboard_hook;
static int close_busy;
#define WM_TABBOZ_FOCUS (WM_APP+42)
static void reveal_focus(HWND hwnd,DialogContext *context);
static UINT test_dpi;
static UINT dpi_for(HWND hwnd) { return test_dpi?test_dpi:GetDpiForWindow(hwnd); }
static int px(HWND hwnd,int value) { return MulDiv(value,dpi_for(hwnd),96); }
static void place(HWND hwnd,int id,int x,int y,int w,int h) {
    HWND child=GetDlgItem(hwnd,id);
    if(child) MoveWindow(child,px(hwnd,x),px(hwnd,y),px(hwnd,w),px(hwnd,h),TRUE);
}
static void main_fonts(HWND hwnd,DialogContext *context) {
    HFONT old_font=context->font,old_title=context->title;
        context->font=CreateFontW(-px(hwnd,14),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        context->title=CreateFontW(-px(hwnd,24),0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        for(HWND c=GetWindow(hwnd,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessage(c,WM_SETFONT,(WPARAM)context->font,TRUE);
        SendDlgItemMessage(hwnd,102,WM_SETFONT,(WPARAM)context->title,TRUE);SendDlgItemMessage(hwnd,105,WM_SETFONT,(WPARAM)context->title,TRUE);
    if(old_font)DeleteObject(old_font);if(old_title)DeleteObject(old_title);
}
static void layout(HWND hwnd) {
    RECT r; GetClientRect(hwnd,&r); int dpi=dpi_for(hwnd);
    int w=MulDiv(r.right,96,dpi), h=MulDiv(r.bottom,96,dpi); int left=280, right=w-30;
    int content=h<600?600:h;
    place(hwnd,102,28,22,w-300,36); place(hwnd,157,30,66,w-360,24); place(hwnd,105,w-270,26,240,34);
    int avatar_height=content-300; if(avatar_height>330)avatar_height=330;
    place(hwnd,103,45,112,200,avatar_height);
    int labels[]={301,302,303,170,304},values[]={152,151,153,154,156};
    for(int i=0;i<5;i++) {
        int y=112+i*59; place(hwnd,labels[i],left,y,right-left-110,21); place(hwnd,values[i],right-105,y,100,22);
        if(i<3)place(hwnd,310+i,left,y+29,right-left,8);
    }
    place(hwnd,155,left,310,right-left,22); place(hwnd,150,left,369,right-left,22);
    int actions[]={130,131,132,136,137,133,134,135,141,142,40155,140};
    int gap=10,bw=(w-60-3*gap)/4;
    for(int i=0;i<12;i++) place(hwnd,actions[i],30+(i%4)*(bw+gap),content-161+(i/4)*46,bw,36);
    DialogContext *context=(DialogContext*)GetWindowLongPtr(hwnd,DWLP_USER);
    if(context) {
        context->extent=px(hwnd,content);
        SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE|SIF_POS,0,context->extent-1,(UINT)r.bottom,context->scroll};
        SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
        ShowScrollBar(hwnd,SB_VERT,context->extent>r.bottom);
    }
    InvalidateRect(hwnd,NULL,TRUE);
}
static LRESULT CALLBACK Keyboard(int code,WPARAM wp,LPARAM lp) {
    MSG *msg=(MSG*)lp;
    if(code==MSGF_DIALOGBOX&&msg->message==WM_KEYDOWN&&(msg->wParam==VK_TAB||msg->wParam==VK_UP||msg->wParam==VK_DOWN||msg->wParam==VK_LEFT||msg->wParam==VK_RIGHT))
        PostMessage(GetAncestor(msg->hwnd,GA_ROOT),WM_TABBOZ_FOCUS,0,0);
    if(code==MSGF_DIALOGBOX && msg->message==WM_KEYDOWN && (GetKeyState(VK_CONTROL)&0x8000) && hWndMain && IsWindowEnabled(hWndMain)) {
        int command=msg->wParam=='S'?40002:msg->wParam=='O'?QX_LOAD:msg->wParam=='N'?40001:0;
        if(command) { PostMessage(hWndMain,WM_COMMAND,command,0); return 1; }
    }
    return CallNextHookEx(keyboard_hook,code,wp,lp);
}
static void scroll_dialog(HWND hwnd,DialogContext *context,int position) {
    SCROLLINFO si={sizeof(si),SIF_ALL};GetScrollInfo(hwnd,SB_VERT,&si);
    int max=si.nMax-(int)si.nPage+1;if(max<0)max=0;
    if(position<0)position=0;if(position>max)position=max;
    int delta=context->scroll-position; context->scroll=position;
    for(HWND child=GetWindow(hwnd,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) {
        RECT r;GetWindowRect(child,&r);MapWindowPoints(NULL,hwnd,(POINT*)&r,2);
        SetWindowPos(child,NULL,r.left,r.top+delta,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    }
    SetScrollPos(hwnd,SB_VERT,position,TRUE);InvalidateRect(hwnd,NULL,TRUE);
}
static void reveal_focus(HWND hwnd,DialogContext *context) {
    HWND focus=GetFocus();if(!focus||!IsChild(hwnd,focus))return;
    RECT r,client;GetWindowRect(focus,&r);MapWindowPoints(NULL,hwnd,(POINT*)&r,2);GetClientRect(hwnd,&client);
    int margin=px(hwnd,8);
    if(r.top<0)scroll_dialog(hwnd,context,context->scroll+r.top-margin);
    else if(r.bottom>client.bottom)scroll_dialog(hwnd,context,context->scroll+r.bottom-client.bottom+margin);
}
static INT_PTR CALLBACK DialogProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    DialogContext *context=(DialogContext *)GetWindowLongPtr(hwnd,DWLP_USER);
    if(msg==WM_INITDIALOG) { context=(DialogContext*)lp; SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)context); }
    if(!context) return FALSE;
    if(msg==WM_TABBOZ_FOCUS){reveal_focus(hwnd,context);return TRUE;}
    if(msg==WM_NEXTDLGCTL)PostMessage(hwnd,WM_TABBOZ_FOCUS,0,0);
    if(msg==WM_DRAWITEM) {
        DRAWITEMSTRUCT *draw=(DRAWITEMSTRUCT*)lp;
        if(draw->CtlType==ODT_STATIC) {
            if(context->id==12 && TabbozDrawSplash(hInst,draw->hDC,&draw->rcItem))return TRUE;
            HBITMAP bitmap=LoadBitmapA(hInst,MAKEINTRESOURCEA(draw->CtlID+1000));
            FillRect(draw->hDC,&draw->rcItem,(HBRUSH)(COLOR_WINDOW+1));
            if(bitmap) {
                BITMAP size;GetObject(bitmap,sizeof(size),&size);
                int cw=draw->rcItem.right-draw->rcItem.left,ch=draw->rcItem.bottom-draw->rcItem.top;
                int width=cw,height=MulDiv(width,size.bmHeight,size.bmWidth);
                if(height>ch){height=ch;width=MulDiv(height,size.bmWidth,size.bmHeight);}
                HDC memory=CreateCompatibleDC(draw->hDC);HGDIOBJ old=SelectObject(memory,bitmap);
                SetStretchBltMode(draw->hDC,HALFTONE);SetBrushOrgEx(draw->hDC,0,0,NULL);
                StretchBlt(draw->hDC,draw->rcItem.left+(cw-width)/2,draw->rcItem.top+(ch-height)/2,width,height,memory,0,0,size.bmWidth,size.bmHeight,SRCCOPY);
                SelectObject(memory,old);DeleteDC(memory);DeleteObject(bitmap);
            }
            return TRUE;
        }
    }
    if(msg==WM_VSCROLL || msg==WM_MOUSEWHEEL) {
        int move=0;
        if(msg==WM_MOUSEWHEEL)move=-GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA*px(hwnd,45);
        else switch(LOWORD(wp)){case SB_LINEUP:move=-px(hwnd,24);break;case SB_LINEDOWN:move=px(hwnd,24);break;case SB_PAGEUP:move=-px(hwnd,180);break;case SB_PAGEDOWN:move=px(hwnd,180);break;case SB_THUMBTRACK:{SCROLLINFO si={sizeof(si),SIF_TRACKPOS};GetScrollInfo(hwnd,SB_VERT,&si);move=si.nTrackPos-context->scroll;break;}}
        scroll_dialog(hwnd,context,context->scroll+move); return TRUE;
    }
    INT_PTR result=0;
    if(context->id==1 && TabbozDesktopMessage(hwnd,msg,wp,lp,&result)) return result;
    char name[30]="",surname[30]="";
    if(context->id==15 && msg==WM_COMMAND && LOWORD(wp)==IDOK) {
        GetDlgItemTextA(hwnd,701,name,sizeof(name));GetDlgItemTextA(hwnd,702,surname,sizeof(surname));
    }
    // The original handlers received a WORD command ID. Preserve that contract
    // while keeping HWND pointers and all other message parameters full width.
    result=context->handler(hwnd,msg,msg==WM_COMMAND?LOWORD(wp):wp,lp);
    if(name[0])strcpy_s(Nome,sizeof(Nome),name);
    if(surname[0])strcpy_s(Cognome,sizeof(Cognome),surname);
    if(msg==WM_INITDIALOG) {
        HICON small=(HICON)LoadImageA(hInst,MAKEINTRESOURCEA(1),IMAGE_ICON,GetSystemMetricsForDpi(SM_CXSMICON,GetDpiForWindow(hwnd)),GetSystemMetricsForDpi(SM_CYSMICON,GetDpiForWindow(hwnd)),LR_SHARED);
        HICON large=(HICON)LoadImageA(hInst,MAKEINTRESOURCEA(1),IMAGE_ICON,GetSystemMetricsForDpi(SM_CXICON,GetDpiForWindow(hwnd)),GetSystemMetricsForDpi(SM_CYICON,GetDpiForWindow(hwnd)),LR_SHARED);
        SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)small);SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)large);
        if(context->id==15){SetDlgItemTextA(hwnd,701,Nome);SetDlgItemTextA(hwnd,702,Cognome);SendDlgItemMessage(hwnd,701,EM_SETLIMITTEXT,29,0);SendDlgItemMessage(hwnd,702,EM_SETLIMITTEXT,29,0);}
        RECT r,work; GetWindowRect(hwnd,&r); SystemParametersInfo(SPI_GETWORKAREA,0,&work,0);
        int height=r.bottom-r.top,width=r.right-r.left;
        if(context->id!=1 && height>work.bottom-work.top-24) {
            RECT client;GetClientRect(hwnd,&client);context->extent=client.bottom;
            height=work.bottom-work.top-24;
            SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_VSCROLL);
            SetWindowPos(hwnd,NULL,0,0,width,height,SWP_NOMOVE|SWP_NOZORDER);
            GetClientRect(hwnd,&client);SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE,0,context->extent-1,(UINT)client.bottom};SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
        }
        if(width>work.right-work.left)width=work.right-work.left;
        SetWindowPos(hwnd,NULL,work.left+(work.right-work.left-width)/2,work.top+(work.bottom-work.top-height)/2,width,height,SWP_NOZORDER|SWP_NOACTIVATE);
        if(context->id==1)TabbozWindowState(hwnd,0);
        if(TabbozTesting) PostMessage(hwnd,WM_COMMAND,(context->id==15||context->id==12)?IDOK:IDCANCEL,0);
    }
    if(msg==WM_NCDESTROY) { if(context->font)DeleteObject(context->font);if(context->title)DeleteObject(context->title); }
    if(msg==WM_CLOSE) {
        context->handler(hwnd,WM_COMMAND,IDCANCEL,0);
        EndDialog(hwnd,IDCANCEL);return TRUE;
    }
    return result;
}
INT_PTR TabbozDialogBox(HINSTANCE instance,LPCSTR resource,HWND owner,DLGPROC handler) {
    DialogContext context={0};context.handler=handler;context.id=IS_INTRESOURCE(resource)?(int)(ULONG_PTR)resource:0;
    if(owner&&!IsWindow(owner))owner=NULL;
    return DialogBoxParamA(instance,resource,owner,DialogProc,(LPARAM)&context);
}
int TabbozMessageBox(HWND owner,LPCSTR text,LPCSTR title,UINT flags) {
    if(TabbozTesting) return (flags&MB_YESNO)?IDYES:IDOK;
    return MessageBoxA(owner,text,title,flags);
}
void TabbozDesktopInit(void) {
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_STANDARD_CLASSES|ICC_PROGRESS_CLASS}; InitCommonControlsEx(&init);
    if(!background)background=CreateSolidBrush(RGB(250,251,253));
    if(!keyboard_hook)keyboard_hook=SetWindowsHookEx(WH_MSGFILTER,Keyboard,NULL,GetCurrentThreadId());
}
BOOL TabbozDesktopMessage(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,INT_PTR *result) {
    DialogContext *context=(DialogContext*)GetWindowLongPtr(hwnd,DWLP_USER); *result=TRUE;
    switch(message) {
    case WM_INITDIALOG: {
        main_fonts(hwnd,context);
        for(int i=0;i<3;i++)CreateWindowExA(0,PROGRESS_CLASSA,"",WS_CHILD|WS_VISIBLE|PBS_SMOOTH,0,0,10,10,hwnd,(HMENU)(INT_PTR)(310+i),hInst,NULL);
        HMENU file=GetSubMenu(GetMenu(hwnd),0);InsertMenuA(file,0,MF_BYPOSITION|MF_STRING,40001,"&Nuova partita\tCtrl+N");InsertMenuA(file,2,MF_BYPOSITION|MF_STRING,40002,"&Salva partita\tCtrl+S");
        ModifyMenuA(file,QX_LOAD,MF_BYCOMMAND|MF_STRING,QX_LOAD,"&Apri partita...\tCtrl+O");
        RECT work;SystemParametersInfo(SPI_GETWORKAREA,0,&work,0);
        int w=px(hwnd,940),h=px(hwnd,710);if(w>work.right-work.left-20)w=work.right-work.left-20;if(h>work.bottom-work.top-20)h=work.bottom-work.top-20;
        SetWindowPos(hwnd,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);layout(hwnd);
        return FALSE;
    }
    case WM_SIZE:if(context->scroll)scroll_dialog(hwnd,context,0);layout(hwnd);return TRUE;
    case WM_GETMINMAXINFO:{MINMAXINFO *info=(MINMAXINFO*)lp;RECT work;SystemParametersInfo(SPI_GETWORKAREA,0,&work,0);info->ptMinTrackSize.x=min(px(hwnd,760),work.right-work.left);info->ptMinTrackSize.y=min(px(hwnd,440),work.bottom-work.top);return TRUE;}
    case WM_DPICHANGED:{main_fonts(hwnd,context);RECT *r=(RECT*)lp;SetWindowPos(hwnd,NULL,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER);layout(hwnd);return TRUE;}
    case WM_CTLCOLORDLG:*result=(INT_PTR)background;return TRUE;
    case WM_CTLCOLORSTATIC:{HDC dc=(HDC)wp;SetBkColor(dc,RGB(250,251,253));SetTextColor(dc,GetDlgCtrlID((HWND)lp)==105?RGB(23,95,191):RGB(35,45,60));*result=(INT_PTR)background;return TRUE;}
    case WM_ERASEBKGND:{RECT r;GetClientRect(hwnd,&r);FillRect((HDC)wp,&r,background);r.bottom=px(hwnd,4);HBRUSH b=CreateSolidBrush(RGB(32,105,201));FillRect((HDC)wp,&r,b);DeleteObject(b);return TRUE;}
    case WM_COMMAND:
        if(LOWORD(wp)==40155){SendMessage(hwnd,WM_COMMAND,QX_CELLULAR,0);return TRUE;}
        if(LOWORD(wp)==40002){SalvaTutto();if(!TabbozSaveSucceeded)TabbozStoreError(hwnd);return TRUE;}
        if(LOWORD(wp)==40001){if(MessageBoxW(hwnd,L"Vuoi iniziare una nuova partita? La partita corrente verra' sostituita. Puoi prima esportarla con Salva con nome.",L"Nuova partita",MB_YESNO|MB_ICONQUESTION)==IDYES){DialogBox(hInst,MAKEINTRESOURCE(15),hwnd,FormatTabboz);AggiornaPrincipale(hwnd);InvalidateRect(GetDlgItem(hwnd,103),NULL,TRUE);}return TRUE;}
        if(LOWORD(wp)==IDCANCEL)return TRUE;
        if(LOWORD(wp)!=QX_CLOSE)return FALSE;
        /* fall through to save-and-close */
    case WM_CLOSE:
        if(close_busy)return TRUE;close_busy=1;
        for(;;) {
            SalvaTutto();if(TabbozSaveSucceeded){TabbozWindowState(hwnd,1);EndDialog(hwnd,IDOK);break;}
            TASKDIALOG_BUTTON buttons[]={{1001,L"Riprova"},{1002,L"Salva con nome..."},{1003,L"Esci senza salvare"}};
            TASKDIALOGCONFIG dialog={0};int choice=IDCANCEL;
            dialog.cbSize=sizeof(dialog);dialog.hwndParent=hwnd;dialog.dwFlags=TDF_ALLOW_DIALOG_CANCELLATION;
            dialog.dwCommonButtons=TDCBF_CANCEL_BUTTON;dialog.pszWindowTitle=L"Salvataggio non riuscito";
            dialog.pszMainInstruction=L"Non e' stato possibile salvare la partita.";
            dialog.pszContent=L"Puoi riprovare, salvare in un'altra cartella o uscire senza salvare. I dati precedenti restano conservati.";
            dialog.cButtons=3;dialog.pButtons=buttons;dialog.nDefaultButton=1001;
            TaskDialogIndirect(&dialog,&choice,NULL,NULL);
            if(choice==1001)continue;
            if(choice==1002){TabbozChooseFile(hwnd,1);if(TabbozSaveSucceeded){TabbozWindowState(hwnd,1);EndDialog(hwnd,IDOK);}break;}
            if(choice==1003){TabbozWindowState(hwnd,1);EndDialog(hwnd,IDOK);}
            break;
        }
        close_busy=0;return TRUE;
    case WM_SYSCOMMAND:if((wp&0xfff0)==SC_CLOSE){SendMessage(hwnd,WM_CLOSE,0,0);return TRUE;}return FALSE;
    }
    return FALSE;
}
void TabbozDesktopRefresh(HWND hwnd) {
    SendDlgItemMessage(hwnd,310,PBM_SETPOS,Reputazione,0);SendDlgItemMessage(hwnd,311,PBM_SETPOS,Fama,0);SendDlgItemMessage(hwnd,312,PBM_SETPOS,Studio,0);
    InvalidateRect(GetDlgItem(hwnd,103),NULL,TRUE);
}
int TabbozProbeDialog(HINSTANCE instance,int id,DLGPROC handler,int command) {
    DialogContext context={0};context.handler=handler;context.id=id;
    HWND hwnd=CreateDialogParamA(instance,MAKEINTRESOURCEA(id),NULL,DialogProc,(LPARAM)&context);
    if(!hwnd) return 0;
    if(command) SendMessage(GetDlgItem(hwnd,command),BM_CLICK,0,0);
    int ok=!(id==14&&command==110)||((SendDlgItemMessage(hwnd,110,BM_GETCHECK,0,0)==BST_CHECKED)==(sound_active!=0));
    DestroyWindow(hwnd); return ok;
}
void TabbozChooseFile(HWND hwnd,int save) {
    wchar_t file[32768]=L""; OPENFILENAMEW ofn={0};
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwnd; ofn.lpstrFile=file; ofn.nMaxFile=32768;
    ofn.lpstrFilter=L"Partite Tabboz (*.tbz)\0*.tbz\0\0"; ofn.lpstrDefExt=L"tbz";
    ofn.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
    if(!(save?GetSaveFileNameW(&ofn):GetOpenFileNameW(&ofn))) return;
    if(!save && !TabbozStoreValidate(file)) {
        MessageBoxW(hwnd,L"Il file non contiene una partita Tabboz valida. La partita corrente e' rimasta invariata.",L"Caricamento non riuscito",MB_OK|MB_ICONERROR); return;
    }
    TabbozStoreSelect(file);
    if(save) SalvaTutto(); else TabbozLoadGame();
    if(save && !TabbozSaveSucceeded) TabbozStoreError(hwnd);
    TabbozStoreSelect(NULL);
    AggiornaPrincipale(hwnd); InvalidateRect(GetDlgItem(hwnd,103),NULL,TRUE);
}

static INT_PTR CALLBACK DpiDialog(HWND h,UINT m,WPARAM w,LPARAM l) { return m==WM_INITDIALOG; }
int TabbozProbeDpi(HINSTANCE instance,UINT dpi) {
    DialogContext context={0};context.handler=DpiDialog;context.id=1;test_dpi=dpi;
    HWND hwnd=CreateDialogParamA(instance,MAKEINTRESOURCEA(1),NULL,DialogProc,(LPARAM)&context);
    int ok=hwnd!=NULL;
    if(hwnd) {
        RECT client;GetClientRect(hwnd,&client);
        for(HWND c=GetWindow(hwnd,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT)) {
            RECT r;GetWindowRect(c,&r);MapWindowPoints(NULL,hwnd,(POINT*)&r,2);
            if(r.left<0||r.top<0||r.right>client.right||r.bottom>context.extent)ok=0;
        }
        if(dpi==192) {
            SetWindowPos(hwnd,NULL,0,0,1300,700,SWP_NOMOVE|SWP_NOZORDER);
            if(!(GetWindowLongPtr(hwnd,GWL_STYLE)&WS_VSCROLL))ok=0;
            SetFocus(GetDlgItem(hwnd,140));SendMessage(hwnd,WM_APP+42,0,0);
            RECT focus;GetWindowRect(GetDlgItem(hwnd,140),&focus);MapWindowPoints(NULL,hwnd,(POINT*)&focus,2);GetClientRect(hwnd,&client);
            if(context.scroll<=0||focus.top<0||focus.bottom>client.bottom)ok=0;
        }
        DestroyWindow(hwnd);
    }
    test_dpi=0;return ok;
}

int TabbozProbeClose(HINSTANCE instance,int id,DLGPROC handler,int command) {
    DialogContext context={0};context.handler=handler;context.id=id;
    HWND hwnd=CreateDialogParamA(instance,MAKEINTRESOURCEA(id),NULL,DialogProc,(LPARAM)&context);
    if(!hwnd)return 0;
    SendMessage(GetDlgItem(hwnd,command),BM_CLICK,0,0);
    SendMessage(hwnd,WM_CLOSE,0,0);if(IsWindow(hwnd))DestroyWindow(hwnd);return 1;
}
int TabbozProbeRadio(HINSTANCE instance,int id,DLGPROC handler,int first,int second) {
    DialogContext context={0};context.handler=handler;context.id=id;
    HWND hwnd=CreateDialogParamA(instance,MAKEINTRESOURCEA(id),NULL,DialogProc,(LPARAM)&context);
    if(!hwnd)return 0;
    SendMessage(GetDlgItem(hwnd,first),BM_CLICK,0,0);SendMessage(GetDlgItem(hwnd,second),BM_CLICK,0,0);
    int ok=SendDlgItemMessage(hwnd,first,BM_GETCHECK,0,0)==BST_UNCHECKED&&SendDlgItemMessage(hwnd,second,BM_GETCHECK,0,0)==BST_CHECKED;
    DestroyWindow(hwnd);return ok;
}
