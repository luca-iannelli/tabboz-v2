/* Native integration tests: invoked only by --self-test, never exposed as gameplay. */
#include "os.h"
#include "zarrosim.h"
#include "desktop.h"
#include <stdio.h>
#include <shlobj.h>
#include <string.h>
#include <wchar.h>
extern void InitTabboz(void);
extern void ResetMe(int first);
extern int firsttime;
extern int euro;
extern int vvc(int value);
extern int TabbozProbeDialog(HINSTANCE instance,int id,DLGPROC handler,int command);
extern int TabbozProbeDpi(HINSTANCE instance,UINT dpi);
extern int TabbozProbeClose(HINSTANCE instance,int id,DLGPROC handler,int command);
extern int TabbozProbeRadio(HINSTANCE instance,int id,DLGPROC handler,int first,int second);
extern INT_PTR CALLBACK AcquistaScooter(HWND,UINT,WPARAM,LPARAM);
extern INT_PTR CALLBACK CompraQualcosa(HWND,UINT,WPARAM,LPARAM);
extern INT_PTR CALLBACK TabbozWndProc(HWND,UINT,WPARAM,LPARAM);
static INT_PTR CALLBACK EmptyDialog(HWND h,UINT m,WPARAM w,LPARAM l) { return m==WM_INITDIALOG; }
#define CHECK(c) do { if(!(c)){fprintf(report,"FAIL line %d: %s\n",__LINE__,#c); failures++;} else passed++; } while(0)
int TabbozSelfTest(HINSTANCE instance) {
    wchar_t temp[32768],folder[32768],report_path[32768];
    GetTempPathW(32768,temp); swprintf_s(folder,32768,L"%stabboz-tests-%lu",temp,GetCurrentProcessId());
    SetEnvironmentVariableW(L"TABBOZ_DATA_DIR",folder);
    GetEnvironmentVariableW(L"TABBOZ_TEST_REPORT",report_path,32768);
    if(!report_path[0]) swprintf_s(report_path,32768,L"%stabboz-test-report.txt",temp);
    FILE *report=_wfopen(report_path,L"w"); if(!report) return 2;
    int failures=0,passed=0; TabbozTesting=1; hInst=instance;
    CHECK(TabbozStoreInit()); InitTabboz(); sound_active=0;
    CHECK(euro==1); CHECK(!strcmp(MostraSoldi(1),"0,50 EUR"));
    CHECK(vvc(-1)==0 && vvc(101)==100 && vvc(45)==45);
    // Reading a save must not replay daily wages or mutate work days.
    ResetMe(1); sound_active=0; Soldi=123; impegno=5; giorni_di_lavoro=7; x_giorno=26;
    strcpy(Nome,"Test"); strcpy(Cognome,"Euro"); SalvaTutto();
    CHECK(TabbozSaveSucceeded); CHECK(TabbozStoreValidate(TabbozStorePath()));
    Soldi=0; TabbozLoadGame();
    CHECK(Soldi==123); CHECK(giorni_di_lavoro==7); CHECK(x_giorno==26); CHECK(!strcmp(Nome,"Test"));
    Soldi=0xffffffffUL;Paghetta=0xfffffffeUL;DDP=0xffffffffUL;SalvaTutto();
    CHECK(TabbozStoreValidate(TabbozStorePath()));
    Soldi=Paghetta=DDP=0;TabbozLoadGame();
    CHECK(Soldi==0xffffffffUL);CHECK(Paghetta==0xfffffffeUL);CHECK(DDP==0xffffffffUL);
    Soldi=123;Paghetta=30;DDP=0;SalvaTutto();
    // Extreme is the fifth legitimate air filter in the original catalogue.
    ScooterData.filtro=4;SalvaTutto();CHECK(TabbozStoreValidate(TabbozStorePath()));
    TabbozLoadGame();CHECK(ScooterData.filtro==4);
    ScooterData.filtro=0;ScooterData.attivita=7;SalvaTutto();
    CHECK(!TabbozStoreValidate(TabbozStorePath()));
    ScooterData.attivita=0;SalvaTutto();
    // This text field is outside the legacy checksum; validate its size before load.
    wchar_t oversized_sex[240];for(int i=0;i<239;i++)oversized_sex[i]=L'M';oversized_sex[239]=0;
    WritePrivateProfileStringW(L"Tabboz",L"Sesso",oversized_sex,TabbozStorePath());
    int rejected_sex=!TabbozStoreValidate(TabbozStorePath());CHECK(rejected_sex);
    if(rejected_sex){TabbozLoadGame();CHECK(sesso=='M');}
    SalvaTutto();
    // An incomplete/corrupted snapshot must be rejected before touching game state.
    wchar_t corrupt[32768]; swprintf_s(corrupt,32768,L"%s\\invalid.tbz",folder);
    WritePrivateProfileStringW(L"Tabboz",L"Mese",L"99",corrupt);
    CHECK(!TabbozStoreValidate(corrupt)); CHECK(Soldi==123);
    // Failed destination replacement leaves the previous autosave readable.
    TabbozStoreSelect(folder); Soldi=444; SalvaTutto(); CHECK(!TabbozSaveSucceeded);
    TabbozStoreSelect(NULL); TabbozLoadGame(); CHECK(Soldi==123);
    // A modified numerical value must fail the original checksum.
    WritePrivateProfileStringW(L"Tabboz",L"Soldi",L"999",TabbozStorePath());
    CHECK(!TabbozStoreValidate(TabbozStorePath()));
    SalvaTutto();
    HRSRC r; int ids[]={1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,88,89,91,92,93,95,100,101,102,103,104,105,106,110,120,121,122,123,190,191,192,200,201,202,203,204,205,206,207,208,210,290,291,292,293,294,295,296,297,390,391,392,393,394,395,396,397};
    int dialog_count=0;
    // Enumerate actual resources, not a guessed list of reachable IDs.
    for(int id=1;id<500;id++) if((r=FindResource(instance,MAKEINTRESOURCE(id),RT_DIALOG))!=NULL) {
        int ok=TabbozProbeDialog(instance,id,EmptyDialog,0); if(!ok)fprintf(report,"Dialog %d failed, Win32 error %lu\n",id,GetLastError());CHECK(ok); dialog_count++;
    }
    CHECK(dialog_count==81);
    struct {int id; DLGPROC handler;} activities[]={{4,Disco},{5,Famiglia},{6,Compagnia},{7,Scooter},{8,Vestiti},{9,Tipa},{10,Scuola},{11,PersonalInfo},{13,Lavoro},{14,Configuration},{88,Tabaccaio},{89,Palestra},{120,Cellular}};
    for(int i=0;i<sizeof(activities)/sizeof(activities[0]);i++) CHECK(TabbozProbeDialog(instance,activities[i].id,activities[i].handler,0));
    ResetMe(1); sound_active=0; Studio=0; unsigned long before=Soldi;
    CHECK(TabbozProbeDialog(instance,5,Famiglia,101)); CHECK(Soldi==before); CHECK(Paghetta==30);
    // Configuration changes model and visible check state exactly once.
    sound_active=0; CHECK(TabbozProbeDialog(instance,14,Configuration,110)); CHECK(sound_active==1);
    sound_active=0;
    ScooterData=ScooterMem[1];unsigned long purchase_money=Soldi;
    CHECK(TabbozProbeClose(instance,78,AcquistaScooter,121));
    CHECK(scelta==-1);CHECK(ScooterData.prezzo==ScooterMem[1].prezzo);CHECK(ScooterData.stato==100);CHECK(Soldi==purchase_money);
    CHECK(TabbozProbeRadio(instance,81,CompraQualcosa,107,110));
    CHECK(TabbozProbeRadio(instance,83,CompraQualcosa,114,117));
    UINT scales[]={96,120,144,192};
    for(int i=0;i<4;i++)CHECK(TabbozProbeDpi(instance,scales[i]));
    wchar_t unc[32768];
    if(GetEnvironmentVariableW(L"TABBOZ_UNC_TEST_DIR",unc,32768)) {
        wchar_t save[32768];swprintf_s(save,32768,L"%s\\unicode-prova-\x00e8.tbz",unc);
        SHCreateDirectoryExW(NULL,unc,NULL);TabbozStoreSelect(save);SalvaTutto();
        fprintf(report,"UNC save error: %lu\n",TabbozStoreLastError());
        CHECK(TabbozSaveSucceeded);CHECK(TabbozStoreValidate(save));TabbozStoreSelect(NULL);
    }
    // A corrupt autosave must be preserved for recovery, not passed to game tables.
    WritePrivateProfileStringW(L"Tabboz",L"Mese",L"99",TabbozStorePath());
    CHECK(TabbozStoreInit());CHECK(GetFileAttributesW(TabbozStorePath())==INVALID_FILE_ATTRIBUTES);
    // A genuine nested legacy registry fixture must import without changing its source.
    SalvaTutto();
    wchar_t registry_name[160];swprintf_s(registry_name,160,L"Software\\TabbozSimulator-Test-%lu-%llu",GetCurrentProcessId(),GetTickCount64());
    HKEY legacy;DWORD disposition;
    LONG reg_status=RegCreateKeyExW(HKEY_CURRENT_USER,registry_name,0,NULL,0,KEY_ALL_ACCESS,NULL,&legacy,&disposition);
    CHECK(reg_status==ERROR_SUCCESS && disposition==REG_CREATED_NEW_KEY);
    if(reg_status==ERROR_SUCCESS && disposition==REG_CREATED_NEW_KEY) {
        const char *fixture_keys[]={"Soldi","Paghetta","Reputazione","Studio","Fama","Rapporti","Stato","DdP","FigTipa","Nome","Cognome","Nometipa","City","Residenza","Street","Materie","Fortuna","Mese","Giorno","GiornoSet","AnnoBisestile","CompMese","CompGiorno","NumeroDitta","ScadPalGiorno","ScadPalMese","Impegno","GiorniDiLavoro","Stipendio","Sigarette","Testa","Giubbotto","Pantaloni","Scarpe","Euro","STARTcmdShow","TimerActive","SoundActive","Sesso","NetEnable","NetPort","Debug","Scooter\\Speed","Scooter\\Marmitta","Scooter\\Carburatore","Scooter\\CC","Scooter\\Filtro","Scooter\\Prezzo","Scooter\\Attivita","Scooter\\Stato","Scooter\\Benzina","Scooter\\Antifurto","Scooter\\Nome","Cellular\\DualOnly","Cellular\\CreditoRest","Cellular\\NomeAbb","Cellular\\DualBand","Cellular\\Stato","Cellular\\Prezzo","Cellular\\Nome","SoftCheck","Version"};
        for(int i=0;i<sizeof(fixture_keys)/sizeof(fixture_keys[0]);i++) {
            char value[255];wchar_t key[128],text[512];HKEY child;TabbozStoreRead(fixture_keys[i],value,sizeof(value));
            MultiByteToWideChar(CP_UTF8,0,fixture_keys[i],-1,key,128);MultiByteToWideChar(CP_UTF8,0,value,-1,text,512);
            if(RegCreateKeyExW(legacy,key,0,NULL,0,KEY_ALL_ACCESS,NULL,&child,NULL)==ERROR_SUCCESS) {
                RegSetValueExW(child,NULL,0,REG_SZ,(BYTE*)text,(DWORD)((wcslen(text)+1)*sizeof(wchar_t)));RegCloseKey(child);
            }
        }
        wchar_t imported[32768];swprintf_s(imported,32768,L"%s\\legacy-roundtrip.tbz",folder);
        CHECK(TabbozStoreImportRegistry(legacy,imported));CHECK(TabbozStoreValidate(imported));
        HKEY preserved;CHECK(RegOpenKeyExW(legacy,L"Scooter\\Speed",0,KEY_READ,&preserved)==ERROR_SUCCESS);RegCloseKey(preserved);
        RegCloseKey(legacy);RegDeleteTreeW(HKEY_CURRENT_USER,registry_name);
    } else if(reg_status==ERROR_SUCCESS)RegCloseKey(legacy);
    fprintf(report,"Passed: %d\nFailed: %d\nDialogs constructed: %d\n",passed,failures,dialog_count);
    fclose(report); return failures?1:0;
}
