# Compilare e creare i pacchetti

## Requisiti

Windows 10 versione 1903 o successiva / Windows 11 x64; Visual Studio 2022 Build Tools con strumenti C++ x64, Windows SDK, CMake e Ninja. Python 3.8+ serve per rigenerare le risorse e i test relativi. La build usa soltanto librerie Windows di sistema e collega staticamente il runtime C.

Il minimo 1903 corrisponde anche al supporto del manifest UTF-8 usato dalle stringhe dell'applicazione. Il manifest abilita Common Controls v6 e DPI per monitor V2.

## Da Windows

Nella cartella del repository:

```powershell
python tools/generate_resources.py
python -m unittest discover -s tests -v
powershell -NoProfile -ExecutionPolicy Bypass -File tools/build-windows.ps1
```

Lo script trova Visual Studio tramite `vswhere`, carica l'ambiente x64, compila e avvia CTest. L'eseguibile e' copiato in `build/windows/Tabboz.exe`. L'albero CMake vive nella cartella temporanea Windows `Tabboz-v2-native-build`; questo evita il limite di CMD sulle directory di lavoro UNC quando i sorgenti sono su WSL.

## Da WSL

Rigenerare le risorse con `python3`, poi eseguire lo stesso script attraverso PowerShell Windows, usando il percorso restituito da:

```bash
wslpath -w "$PWD/tools/build-windows.ps1"
```

Il workspace deve essere raggiungibile dai programmi Windows. I test scrivono solo in cartelle di prova; `TABBOZ_DATA_DIR` consente di isolare i dati dell'applicazione. Non serve installare MinGW, Wine o un'altra toolchain Linux.

## Installer e ZIP

Serve Inno Setup 6.7.3 o successivo. Lo script usa un ISCC gia' installato oppure `.tools/inno/tools/ISCC.exe`; si puo' specificare `-InnoCompiler`.

Per usare il compilatore senza installazione globale, il workflow scarica il pacchetto NuGet `Tools.InnoSetup` 6.7.3 e ne verifica SHA-256:

`f780898e402ff80612cc8d9fcb8c6e02932bd1cb4c900ffdaa31f9341cfb49f4`

Il pacchetto e' un confezionamento di terze parti del compilatore Inno; la distribuzione ufficiale e' su [jrsoftware.org](https://jrsoftware.org/isdl.php).

```powershell
python tools/source_archive.py
powershell -NoProfile -ExecutionPolicy Bypass -File tools/package-windows.ps1
```

`dist/` contiene installer, ZIP portabile, archivio dei sorgenti corrispondenti e `SHA256SUMS.txt`. Il nuovo splash e l'icona sono risorse dell'eseguibile; anche i suoni sono incorporati. I pacchetti non includono BWCC.DLL e non richiedono un runtime esterno.

## Test GUI e installer

```powershell
./tools/smoke-windows.ps1 -Activities
./tools/smoke-windows.ps1 -ForceSaveFailure -OutputDirectory ./build/validation-error
./tools/test-installer.ps1
```

Questi test aprono finestre reali. Il primo acquisisce schermate e verifica avvio, aree di gioco, euro, salvataggio e chiusura. Il secondo provoca un errore di scrittura e sceglie l'uscita senza salvataggio. Il terzo installa in una directory temporanea, avvia il programma installato, ripete l'installazione come aggiornamento e disinstalla preservando i dati di prova.

I test del layout 100%, 125%, 150% e 200% simulano font e geometria della schermata principale. Non sostituiscono il controllo su monitor fisici con DPI differenti.

## Risorse e personalizzazione

Le finestre originali provengono da `resources/dialogs/dialogs.rc`; `tools/generate_resources.py` produce `native/tabboz.rc`, incluso nel repository. Lo splash e' `assets/splash.png`; l'icona multi-risoluzione e' `assets/app.ico`. Dopo una modifica a questi file rigenerare le risorse e ricompilare.

Il vecchio Makefile Emscripten e gli artefatti web restano materiale storico. La build verificata di questa edizione e' quella CMake/MSVC nativa.
