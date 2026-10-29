#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "desktop.h"

static wchar_t default_path[32768], selected_path[32768], staging[32768];
static int writing, failed;
static DWORD save_error;
static void try_legacy_import(void);
DWORD TabbozStoreLastError(void) { return save_error; }
int TabbozSaveSucceeded = 1;
static void wide(const char *s, wchar_t *out, int length) {
    if (!MultiByteToWideChar(CP_ACP,0,s,-1,out,length)) out[0]=0;
}
const wchar_t *TabbozStorePath(void) { return selected_path[0] ? selected_path : default_path; }
void TabbozStoreSelect(const wchar_t *path) {
    if (path) wcsncpy_s(selected_path,32768,path,_TRUNCATE); else selected_path[0]=0;
}
int TabbozStoreInit(void) {
    wchar_t folder[32768]; PWSTR appdata=NULL;
    DWORD n=GetEnvironmentVariableW(L"TABBOZ_DATA_DIR",folder,32768);
    if (!n) {
        if (FAILED(SHGetKnownFolderPath(&FOLDERID_LocalAppData,0,NULL,&appdata))) return 0;
        swprintf_s(folder,32768,L"%s\\Tabboz Simulator",appdata); CoTaskMemFree(appdata);
    } else if (n>=32768) return 0;
    int status=SHCreateDirectoryExW(NULL,folder,NULL);
    if (status!=ERROR_SUCCESS && status!=ERROR_ALREADY_EXISTS && status!=ERROR_FILE_EXISTS) return 0;
    if (wcslen(folder)+64>=32768) return 0;
    swprintf_s(default_path,32768,L"%s\\current.tbz",folder);
    TabbozStoreInvalidate();
    if(GetFileAttributesW(default_path)!=INVALID_FILE_ATTRIBUTES && !TabbozStoreValidate(default_path)) {
        wchar_t backup[32768];swprintf_s(backup,32768,L"%s.invalid-%lu.tbz",default_path,GetCurrentProcessId());
        if(!MoveFileExW(default_path,backup,0))return 0;
        if(!TabbozTesting)MessageBoxW(NULL,L"Il salvataggio non e' valido ed e' stato conservato con il suffisso .invalid. Puoi iniziare una nuova partita senza sovrascrivere il file da recuperare.",L"Recupero salvataggio",MB_OK|MB_ICONWARNING);
    }
    if(!n && !TabbozTesting && GetFileAttributesW(default_path)==INVALID_FILE_ATTRIBUTES)try_legacy_import();
    return 1;
}
static HANDLE output_file=INVALID_HANDLE_VALUE;
static wchar_t cached_path[32768],cached_text[32768];
static int cache_valid;
void TabbozStoreInvalidate(void) { cache_valid=0; }
static int load_ini(const wchar_t *path) {
    if(cache_valid && !_wcsicmp(cached_path,path))return 1;
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return 0;
    LARGE_INTEGER size;if(!GetFileSizeEx(file,&size)||size.QuadPart>65534||size.QuadPart<2){CloseHandle(file);return 0;}
    BYTE bytes[65536];DWORD read=0;
    int ok=ReadFile(file,bytes,(DWORD)size.QuadPart,&read,NULL)&&read==size.QuadPart;CloseHandle(file);if(!ok)return 0;
    int length;
    if(bytes[0]==0xff&&bytes[1]==0xfe) {
        if(read%2)return 0;length=(read-2)/2;memcpy(cached_text,bytes+2,length*sizeof(wchar_t));
    } else {
        int skip=read>=3&&bytes[0]==0xef&&bytes[1]==0xbb&&bytes[2]==0xbf?3:0;
        length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char*)bytes+skip,read-skip,cached_text,32767);
        if(!length)length=MultiByteToWideChar(1252,0,(const char*)bytes,read,cached_text,32767);
        if(!length)return 0;
    }
    cached_text[length]=0;wcsncpy_s(cached_path,32768,path,_TRUNCATE);cache_valid=1;return 1;
}
static DWORD read_ini(const wchar_t *path,const wchar_t *key,wchar_t *out,DWORD capacity) {
    if(!capacity)return 0;out[0]=0;if(!load_ini(path))return 0;
    const wchar_t *line=cached_text;int section=0;
    while(*line) {
        size_t n=wcscspn(line,L"\r\n");const wchar_t *end=line+n,*start=line;
        while(start<end&&(*start==L' '||*start==L'\t'))start++;
        if(start<end&&*start==L'[')section=(end-start==8&&!_wcsnicmp(start,L"[Tabboz]",8));
        else if(section&&start<end&&*start!=L';') {
            const wchar_t *equal=wmemchr(start,L'=',end-start);
            if(equal) {
                const wchar_t *key_end=equal;while(key_end>start&&key_end[-1]==L' ')key_end--;
                if((size_t)(key_end-start)==wcslen(key)&&!_wcsnicmp(start,key,key_end-start)) {
                    const wchar_t *value=equal+1;
                    if(end-value>=2&&*value==L'"'&&end[-1]==L'"'){value++;end--;}
                    size_t count=end-value;if(count>=capacity)count=capacity-1;
                    wmemcpy(out,value,count);out[count]=0;return (DWORD)count;
                }
            }
        }
        line+=n;while(*line==L'\r'||*line==L'\n')line++;
    }
    return 0;
}
int TabbozStoreBegin(void) {
    DWORD done;static const wchar_t header[]=L"\xfeff[Tabboz]\r\n";
    writing=0;failed=0;save_error=0;TabbozSaveSucceeded=0;
    if(wcslen(TabbozStorePath())+5>=32768)return 0;
    swprintf_s(staging,32768,L"%s.tmp",TabbozStorePath());
    output_file=CreateFileW(staging,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(output_file==INVALID_HANDLE_VALUE){save_error=GetLastError();return 0;}
    writing=1;
    if(!WriteFile(output_file,header,sizeof(header)-sizeof(wchar_t),&done,NULL)||done!=sizeof(header)-sizeof(wchar_t)){save_error=GetLastError();failed=1;}
    return 1;
}
void TabbozStoreWrite(const char *key,const char *value) {
    wchar_t k[128],v[1024],line[1200];DWORD done;
    if(!writing)return;wide(key,k,128);wide(value,v,1024);
    for(wchar_t *p=v;*p;p++)if(*p==L'\r'||*p==L'\n')*p=L' ';
    int length=swprintf_s(line,1200,L"%s=%s\r\n",k,v);
    if(length<0||!WriteFile(output_file,line,length*sizeof(wchar_t),&done,NULL)||done!=length*sizeof(wchar_t)){failed=1;save_error=GetLastError();}
}
int TabbozStoreCommit(void) {
    if(!writing)return 0;
    // Some WSL/network filesystems do not implement FlushFileBuffers. Local NTFS does.
    if(!FlushFileBuffers(output_file)) {
        DWORD error=GetLastError();if(error!=ERROR_INVALID_FUNCTION&&error!=ERROR_NOT_SUPPORTED){failed=1;save_error=error;}
    }
    CloseHandle(output_file);output_file=INVALID_HANDLE_VALUE;writing=0;
    if(failed||!MoveFileExW(staging,TabbozStorePath(),MOVEFILE_REPLACE_EXISTING)) {
        if(!save_error)save_error=GetLastError();DeleteFileW(staging);return 0;
    }
    cache_valid=0;TabbozSaveSucceeded=1;return 1;
}
char *TabbozStoreRead(const char *key,char *value,size_t capacity) {
    wchar_t k[128],v[512];wide(key,k,128);if(!capacity)return NULL;value[0]=0;
    if(!read_ini(TabbozStorePath(),k,v,512))return NULL;
    char converted[2048];if(!WideCharToMultiByte(CP_ACP,0,v,-1,converted,sizeof(converted),NULL,NULL))return NULL;
    strncpy_s(value,capacity,converted,_TRUNCATE);return value;
}
static int integer_value(const wchar_t *path,const wchar_t *key,long long min,long long max) {
    wchar_t text[64],*end; long long n;
    if (!read_ini(path,key,text,64)) return 0;
    n=_wcstoi64(text,&end,10);
    return end!=text && !*end && n>=min && n<=max;
}
int TabbozStoreValidate(const wchar_t *path) {
    TabbozStoreInvalidate();
    wchar_t value[128];
    if (!integer_value(path,L"Mese",1,12) || !integer_value(path,L"Giorno",1,31) || !integer_value(path,L"GiornoSet",1,7) ||
        !integer_value(path,L"CompMese",1,12) || !integer_value(path,L"CompGiorno",1,31) ||
        !integer_value(path,L"NumeroDitta",0,8) || !integer_value(path,L"Testa",0,4) ||
        !integer_value(path,L"Giubbotto",0,19) || !integer_value(path,L"Pantaloni",0,19) ||
        !integer_value(path,L"Scarpe",0,7) || !integer_value(path,L"Soldi",0,0xffffffffULL) ||
        !integer_value(path,L"Scooter\\Carburatore",0,5) || !integer_value(path,L"Scooter\\CC",0,5) ||
        !integer_value(path,L"Scooter\\Marmitta",0,3) || !integer_value(path,L"Scooter\\Filtro",0,4)) return 0;
    if (!integer_value(path,L"Scooter\\Attivita",0,6)) return 0;
    DWORD sex_length=read_ini(path,L"Sesso",value,128);
    if(sex_length && (sex_length!=1 || (value[0]!=L'M' && value[0]!=L'F'))) return 0;
    if (!read_ini(path,L"City",value,128)) return 0;
    if (read_ini(path,L"Materie",value,128)!=9) return 0;
    for(int i=0;i<9;i++) if(value[i]<L'A'||value[i]>L'K') return 0;
    // The order and 32-bit arithmetic match proteggi.c and SalvaTutto exactly.
    static const wchar_t *keys[]={L"Soldi",L"Paghetta",L"Reputazione",L"Studio",L"Fama",L"Rapporti",L"Stato",L"DdP",L"FigTipa",L"Fortuna",L"Impegno",L"GiorniDiLavoro",L"Stipendio",L"Sigarette",L"Testa",L"Giubbotto",L"Pantaloni",L"Scarpe",L"Scooter\\Speed",L"Scooter\\Marmitta",L"Scooter\\Carburatore",L"Scooter\\CC",L"Scooter\\Filtro",L"Scooter\\Prezzo",L"Scooter\\Attivita",L"Scooter\\Stato",L"Scooter\\Benzina",L"Cellular\\DualOnly",L"Cellular\\CreditoRest",L"Cellular\\DualBand",L"Cellular\\Stato",L"Cellular\\Prezzo"};
    static const DWORD r1[]={0xed0f,0xff0d,0x3392,0xabcd,0xc79c,0x23df,0x0706,0xc39c},r2[]={0xb47f,0xc37b,0x1070,0x1999,0xfb1e};
    DWORD checksum=1979; int p1=0,p2=2;
    for(int i=0;i<sizeof(keys)/sizeof(keys[0]);i++) {
        wchar_t *end;long long number;
        if(!read_ini(path,keys[i],value,128))return 0;
        number=_wcstoi64(value,&end,10);int is_unsigned=(i==0||i==1||i==7);
        if(end==value||*end||number < (is_unsigned?0:-1) || number>(is_unsigned?0xffffffffLL:0x7fffffffLL))return 0;
        DWORD a1=((DWORD)number|r1[p1])*(p2+1);p1=(p1+1)%8;
        DWORD a2=(a1&r2[p2])*(p1+1);p2=(p2+1)%5;
        a2=a2>=a1?a2-a1:a2+a1;checksum+=a2;
    }
    read_ini(path,L"SoftCheck",value,128);
    wchar_t *end; unsigned long long saved=_wcstoui64(value,&end,10);
    if(end==value||*end||saved!=checksum)return 0;
    return 1;
}
void TabbozStoreError(HWND owner) {
    if (!TabbozTesting) { wchar_t text[1024];swprintf_s(text,1024,L"Impossibile completare il salvataggio (errore Windows %lu).\nI dati precedenti non sono stati sovrascritti.\n\nPercorso: %.700s",save_error,TabbozStorePath());MessageBoxW(owner,text,L"Salvataggio non riuscito",MB_OK|MB_ICONERROR); }
}
static void import_registry_tree(HKEY source,const wchar_t *prefix,int depth) {
    if(depth>4)return;
    for(DWORD index=0;;index++) {
        wchar_t key[128],value[1024],joined[512];DWORD key_size=128,value_size=sizeof(value),type;HKEY child;
        LONG status=RegEnumKeyExW(source,index,key,&key_size,NULL,NULL,NULL,NULL);
        if(status==ERROR_NO_MORE_ITEMS)break;if(status!=ERROR_SUCCESS)continue;
        if(RegOpenKeyExW(source,key,0,KEY_READ,&child)!=ERROR_SUCCESS)continue;
        swprintf_s(joined,512,prefix[0]?L"%s\\%s":L"%s%s",prefix,key);
        status=RegQueryValueExW(child,NULL,NULL,&type,(BYTE*)value,&value_size);
        if(status==ERROR_SUCCESS&&type==REG_SZ) {
            value[1023]=0;char k[2048],v[4096];
            if(WideCharToMultiByte(CP_UTF8,0,joined,-1,k,sizeof(k),NULL,NULL)&&WideCharToMultiByte(CP_UTF8,0,value,-1,v,sizeof(v),NULL,NULL))TabbozStoreWrite(k,v);
        }
        import_registry_tree(child,joined,depth+1);RegCloseKey(child);
    }
}
int TabbozStoreImportRegistry(HKEY source,const wchar_t *path) {
    TabbozStoreSelect(path);int ok=0;
    if(TabbozStoreBegin()){import_registry_tree(source,L"",0);ok=TabbozStoreCommit();}
    TabbozStoreSelect(NULL);return ok&&TabbozStoreValidate(path);
}
static void try_legacy_import(void) {
    HKEY legacy;
    if(RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Obscured Truckware\\Tabboz Simulator 32",0,KEY_READ|KEY_WOW64_32KEY,&legacy)!=ERROR_SUCCESS)return;
    if(MessageBoxW(NULL,L"E' presente un profilo della versione originale. Vuoi importarlo? Il registro originale rimarra' invariato.",L"Importa partita originale",MB_YESNO|MB_ICONQUESTION)!=IDYES){RegCloseKey(legacy);return;}
    wchar_t candidate[32768];swprintf_s(candidate,32768,L"%s.legacy-%lu.tbz",default_path,GetCurrentProcessId());
    int ok=TabbozStoreImportRegistry(legacy,candidate);RegCloseKey(legacy);
    if(ok&&MoveFileExW(candidate,default_path,0))TabbozStoreInvalidate();
    else MessageBoxW(NULL,L"Il profilo originale non ha superato il controllo di integrita'. Il registro non e' stato modificato; puoi iniziare una nuova partita o importare un file .tbz valido.",L"Importazione non riuscita",MB_OK|MB_ICONWARNING);
}
