# Tabboz Simulator: porting nativo Windows e interfaccia aggiornata

Data: 29 ottobre 2026. Stato: approvato dall'utente. Integrazioni approvate: euro come valuta predefinita e salvataggio del codice/documentazione nel repository pubblico luca-iannelli/tabboz-v2.

## Obiettivo e vincoli

Consegnare Tabboz Simulator come applicazione desktop nativa per Windows 10 e 11, con eseguibile x64, installer e pacchetto portabile. Rendere il gioco più leggibile e comodo mantenendone identità, personaggi, linguaggio, attività e regole. Lo sviluppo avviene in WSL Ubuntu; compilazione e prove native sfruttano gli strumenti già presenti sulla macchina Windows 11.

L'utente ha scelto esplicitamente il motore C con interfaccia Windows nativa modernizzata. Si conserva il motore esistente: calcoli, prezzi, eventi casuali, difficoltà, avanzamento dei giorni, cataloghi e conseguenze delle azioni. Restano grafica del personaggio, suoni, nomi e testi originali. Le correzioni di portabilità o memoria vanno documentate separatamente da qualsiasi cambiamento di regole.

## Approcci considerati

1. **C e controlli Windows nativi, scelto dall'utente.** Recupera direttamente logica e risorse, elimina la dipendenza Borland, produce un programma leggero e limita i rischi di alterare il gioco. Richiede lavoro esplicito sui layout e sulle API a 64 bit.
2. **Motore C e interfaccia Qt.** Consente composizioni grafiche più libere, ma aumenta dipendenze, dimensioni e riscrittura del collegamento tra azioni e finestre. Non scelto.

La versione web esistente è una fonte aggiuntiva di confronto. L'eseguibile richiesto usa direttamente le API Windows.

## Esperienza proposta

La finestra principale parte con area utile indicativa di 920 × 640 pixel a 100%, è ridimensionabile e ricorda dimensioni/posizione valide. Deve restare utilizzabile su un display 1366 × 768 e adattarsi a DPI 125%, 150% e 200% senza tagli di testo o comandi irraggiungibili. Le misure finali e la griglia sono decisioni dell'implementatore, verificate visivamente.

- Intestazione: nome del personaggio, data del gioco e denaro sempre visibili.
- Personaggio originale ben visibile a sinistra, con proporzioni corrette e ingrandimento a pixel nitidi.
- Riepilogo a destra: reputazione, figosità, profitto scolastico, relazione, scooter e relativo stato; valori testuali accompagnati da indicatori dove opportuno. Stato vuoto esplicito quando manca uno scooter o una relazione.
- Accesso diretto ad attività e negozi: scooter, abbigliamento, disco, scuola, lavoro, relazione, compagnia, famiglia, tabacchi, palestra e telefono. Le azioni chiamano i gestori originali e non modificano costi o tempi.
- Menu e comandi coerenti per nuova partita, apertura, salvataggio, informazioni personali, impostazioni e crediti. Scorciatoie standard per apri/salva e navigazione con Tab, access key, Invio ed Esc.
- Impostazioni leggibili con etichette chiare; suoni e difficoltà restano disponibili. Nessun nuovo comando per accelerare il tempo o aggiungere vantaggi.

Le finestre secondarie mantengono contenuti e sequenze originali, usando font Segoe UI, controlli standard, spaziature migliori, pulsanti con etichette esplicite e dimensioni adeguate. Il porting copre tutte le finestre raggiungibili, comprese quelle meno comuni di eventi, annunci e acquisti. Nessun semplice rinvio alla vecchia BWCC.DLL.

Direzione visiva proposta dall'implementatore: fondo chiaro, pannelli sobri, un accento blu, gerarchia tipografica semplice, comandi Windows standard; icone e illustrazioni originali mantengono il carattere anni Novanta. Sono ammesse normali variazioni del tema Windows. La modalità scura personalizzata e nuove illustrazioni non fanno parte della prima consegna.

## Architettura e compatibilità

- Build riproducibile con CMake e MSVC 2022, usando librerie Windows di sistema. Runtime C collegato staticamente per evitare l'installazione separata del redistributable.
- Target Windows 10/11 x64. Manifest per visual styles Common Controls v6 e DPI per monitor V2, con comportamento di ripiego appropriato dove una API non è disponibile. L'esatto minimo di build dichiarato dall'installer deve coincidere con le API effettivamente usate.
- Sostituire i controlli Borland con BUTTON/STATIC/controlli comuni e template moderni. Preservare identificatori e instradamento delle azioni; aggiornare anche la semantica di radio button e checkbox, non soltanto i loro nomi.
- Correggere tutte le firme dei callback e gli scambi di puntatori: UINT, WPARAM, LPARAM, INT_PTR e LRESULT secondo l'API. Non mascherare troncamenti con cast.
- Separare in piccoli moduli Windows le funzioni di presentazione, persistenza, risoluzione dei percorsi, compatibilità delle API e avvio. Evitare una riscrittura completa del motore.
- Risorse native generate o assemblate da icone, bitmap, menu, stringhe e dialoghi presenti nei sorgenti. Nessuna dipendenza dal compilatore Borland o dal browser.
- Conservare il timer originale, inclusa la sua dipendenza dallo stato della finestra, finché un test non dimostra un comportamento equivalente. Nessuna accelerazione implicita dovuta al nuovo layout.

## Salvataggi e dati dell'utente

I nuovi dati persistenti risiedono sotto `%LOCALAPPDATA%\Tabboz Simulator\`, separati dai file del programma. Salvataggio automatico alla chiusura, gestione esplicita degli errori e scrittura temporanea con sostituzione sicura per evitare file parziali. Le operazioni apri/salva devono funzionare con spazi e caratteri Unicode nei percorsi Windows.

Mantenere l'importazione/esportazione `.tbz` e gli algoritmi di codifica dei valori originali; introdurre una fixture di round trip prima di dichiarare la compatibilità. Al primo avvio, proporre il recupero di un eventuale salvataggio storico dal registro senza sovrascriverlo o modificarlo automaticamente. Se non è possibile provarlo, documentare precisamente il limite invece di dichiarare una migrazione riuscita.

Una nuova partita richiede conferma quando sostituisce la partita corrente. La disinstallazione conserva i salvataggi. La chiusura salva e termina il gioco; la vecchia opzione per spegnere l'intero PC viene eliminata dal flusso moderno.

## Installer e consegne

Produrre in `dist/` un installer `.exe` tramite Inno Setup e uno ZIP portabile, con versione esplicita e SHA-256. L'installer usa per impostazione predefinita `%LOCALAPPDATA%\Programs\Tabboz Simulator`, consente installazione senza amministratore, crea una voce nel menu Start e offre un collegamento desktop facoltativo. Gestisce aggiornamento e disinstallazione senza cancellare dati di gioco.

Il pacchetto include eseguibile, suoni necessari, licenza e attribuzioni. La consegna comprende sorgenti modificati e istruzioni di build per rendere disponibile il codice corrispondente. Nessuna pubblicazione remota o firma digitale è inclusa nella richiesta corrente; eventuali limiti dell'installer non firmato saranno descritti fattualmente nel risultato.

Prima si cerca un compilatore Inno Setup già presente. Se manca, si prepara il componente necessario alla build nel workspace, evitando l'installazione di toolchain duplicate sul sistema.

## Verifiche e criteri di accettazione

1. Build Release x64 reale con MSVC e compilazione delle risorse; nessun collegamento a BWCC o runtime web. Verificare architettura, manifest e dipendenze dell'eseguibile anche con REA.
2. Verifica significativa delle regole conservate: confrontare formule e tabelle con il commit originale; testare serializzazione e un insieme rappresentativo di azioni con stato e casualità controllati. Non promettere identità bit per bit del generatore casuale Borland.
3. Verificare che ogni dialogo sia costruibile e che tutte le attività raggiungibili aprano la finestra corretta; prove dedicate per radio button, checkbox, annullamento e comandi predefiniti. Eventi meno comuni possono essere attivati dal solo harness di test, senza introdurre cheat nell'interfaccia di gioco.
4. Su Windows 11: avvio, nuova partita, nome/personaggio, azioni rappresentative di ogni area, denaro/statistiche aggiornati, grafica e audio, salvataggio/caricamento, riavvio e percorsi Unicode. Eseguire prove di errori di persistenza e salvataggi non validi.
5. Layout: prove a 100%, 125%, 150% e 200% con nessuna sovrapposizione bloccante; controllo del focus, tastiera e finestre secondarie. Una simulazione della scala senza cambio reale di monitor va dichiarata come tale.
6. Installer: installazione in directory di prova isolata, avvio dall'installazione, aggiornamento e disinstallazione, preservazione dei salvataggi, verifica anche del pacchetto portabile. Eventuali modifiche a percorsi Windows esterni al workspace passano per le autorizzazioni effettive del sandbox.
7. Windows 10: controllare manifest/API/minimo dichiarato e build compatibile. Se manca una macchina Windows 10 su cui eseguire il programma, riportare quel collaudo come non eseguito. Non presentare un test su Windows 11 come prova su Windows 10.

Il lavoro è concluso quando esistono gli artefatti compilati e le prove descritte, oppure quando un impedimento documentato rende impossibile una specifica verifica e il risultato è consegnato con quel limite esplicito. Un progetto CMake o uno script installer da soli non soddisfano la richiesta.

## Stato e decisioni lasciate all'implementatore

Sorgenti acquisiti, analisi REA e toolchain individuata sono registrati in `docs/research/2026-10-29-original-windows-assessment.md`. L'analisi statica non costituisce una prova runtime dell'originale. Non è ancora stato costruito il nuovo programma.

L'implementatore decide dettagli di organizzazione dei moduli, generazione risorse, etichette coerenti, dimensioni finali e struttura dei test entro questi vincoli. Le attività di progetto restano sul branch locale `native/windows-modernization`; nessun commit, push o release pubblica viene eseguito senza successiva autorizzazione.
