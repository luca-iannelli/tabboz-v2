#ifndef TABBOZ_DESKTOP_H
#define TABBOZ_DESKTOP_H
#include <windows.h>
#include <stddef.h>
extern int TabbozTesting;
extern int TabbozSaveSucceeded;
INT_PTR TabbozDialogBox(HINSTANCE instance, LPCSTR resource, HWND owner, DLGPROC handler);
int TabbozMessageBox(HWND owner, LPCSTR text, LPCSTR title, UINT flags);
BOOL TabbozDesktopMessage(HWND hwnd, UINT message, WPARAM wp, LPARAM lp, INT_PTR *result);
void TabbozDesktopRefresh(HWND hwnd);
void TabbozDesktopInit(void);
int TabbozSelfTest(HINSTANCE instance);
int TabbozStoreInit(void);
void TabbozStoreInvalidate(void);
int TabbozStoreBegin(void);
int TabbozStoreCommit(void);
void TabbozStoreWrite(const char *key, const char *value);
char *TabbozStoreRead(const char *key, char *value, size_t capacity);
int TabbozStoreValidate(const wchar_t *path);
int TabbozStoreImportRegistry(HKEY source,const wchar_t *path);
void TabbozStoreSelect(const wchar_t *path);
const wchar_t *TabbozStorePath(void);
void TabbozStoreError(HWND owner);
DWORD TabbozStoreLastError(void);
void TabbozChooseFile(HWND owner, int save);
void TabbozLoadGame(void);
#endif
