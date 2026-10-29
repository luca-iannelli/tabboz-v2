/* Decode the user-supplied PNG through the Windows GDI+ flat API. */
#define COBJMACROS
#include <windows.h>
#include <objidl.h>
#include <shlwapi.h>
typedef void GpImage;
typedef void GpGraphics;
typedef struct { UINT32 version; void *debug_callback; BOOL no_background_thread; BOOL no_external_codecs; } StartupInput;
__declspec(dllimport) int WINAPI GdiplusStartup(ULONG_PTR *,const StartupInput *,void *);
__declspec(dllimport) void WINAPI GdiplusShutdown(ULONG_PTR);
__declspec(dllimport) int WINAPI GdipLoadImageFromStream(IStream *,GpImage **);
__declspec(dllimport) int WINAPI GdipGetImageWidth(GpImage *,UINT *);
__declspec(dllimport) int WINAPI GdipGetImageHeight(GpImage *,UINT *);
__declspec(dllimport) int WINAPI GdipCreateFromHDC(HDC,GpGraphics **);
__declspec(dllimport) int WINAPI GdipSetInterpolationMode(GpGraphics *,int);
__declspec(dllimport) int WINAPI GdipDrawImageRectI(GpGraphics *,GpImage *,int,int,int,int);
__declspec(dllimport) int WINAPI GdipDeleteGraphics(GpGraphics *);
__declspec(dllimport) int WINAPI GdipDisposeImage(GpImage *);
static ULONG_PTR token;
static IStream *stream;
static GpImage *image;
int TabbozDrawSplash(HINSTANCE instance,HDC dc,const RECT *rect) {
    if(!image) {
        StartupInput input={1,NULL,FALSE,FALSE};
        if(!token && GdiplusStartup(&token,&input,NULL))return 0;
        HRSRC resource=FindResourceW(instance,MAKEINTRESOURCEW(5001),MAKEINTRESOURCEW(10));
        if(!resource)return 0;
        HGLOBAL loaded=LoadResource(instance,resource);if(!loaded)return 0;
        stream=SHCreateMemStream(LockResource(loaded),SizeofResource(instance,resource));
        if(!stream||GdipLoadImageFromStream(stream,&image))return 0;
    }
    UINT iw,ih;GdipGetImageWidth(image,&iw);GdipGetImageHeight(image,&ih);
    int cw=rect->right-rect->left,ch=rect->bottom-rect->top,w=cw,h=MulDiv(w,ih,iw);
    if(h>ch){h=ch;w=MulDiv(h,iw,ih);}
    GpGraphics *graphics=NULL;if(GdipCreateFromHDC(dc,&graphics))return 0;
    GdipSetInterpolationMode(graphics,7);
    int status=GdipDrawImageRectI(graphics,image,rect->left+(cw-w)/2,rect->top+(ch-h)/2,w,h);
    GdipDeleteGraphics(graphics);return status==0;
}
void TabbozSplashShutdown(void) {
    if(image){GdipDisposeImage(image);image=NULL;}
    if(stream){IStream_Release(stream);stream=NULL;}
    if(token){GdiplusShutdown(token);token=0;}
}
