# Verifica della Windows Edition

Le osservazioni di questo documento riguardano la build nativa 2.0.0 sul Windows 11 che ospita WSL Ubuntu. La build utilizza MSVC 19.42 / toolset 14.42.34433 e SDK Windows 10.0.22621.0.

## Risultati gia' osservati

- Sei test Python: copertura dei dialoghi originali, rimozione dei controlli Borland, semantica dei controlli standard e presenza degli asset.
- Test C sugli importi: zero, mezzi euro, euro interi e importi maggiori.
- Suite nativa di integrazione: 143 controlli senza fallimenti, comprese tutte le 81 finestre, inizializzazione delle attivita', salvataggio/caricamento, integrita', errori di scrittura e layout simulati 96/120/144/192 DPI.
- Salvataggio su percorso UNC WSL con caratteri Unicode: riprodotto l'errore Windows 1 della vecchia API INI; il nuovo percorso con file I/O diretto passa il test.
- Dodici aree del gioco aperte, catturate e chiuse nella prova GUI finale. Impostazioni e Scuola controllate visivamente dopo le correzioni di spaziatura.
- Prova GUI normale: avvio con splash, nuova partita, importo `5,00 EUR`, salvataggio e chiusura con exit code 0.
- Prova GUI con errore forzato: compare una scelta di recupero; `Esci senza salvare` termina con exit code 0.
- Splash e nuova icona sono forniti dall'utente; il PNG e' visualizzato senza tagli. L'ICO contiene otto risoluzioni da 16 a 256 pixel.

I report dettagliati e le schermate locali sono in `build/validation*/`; i pacchetti e i loro hash sono in `dist/`. Installazione silenziosa per utente in directory temporanea, avvio del programma installato, aggiornamento e disinstallazione: verificati su Windows 11 con dati di prova conservati. La verifica statica finale degli artefatti viene registrata prima del checkpoint di pubblicazione.

## Correzioni distinte dalle regole di gioco

Il vecchio caricamento rientrava in `Giorno` e poteva ripetere effetti quotidiani. Inoltre il flag di primo avvio poteva reimpostare un successivo caricamento. Due regressioni del test hanno dimostrato il problema; il porting ricostruisce soltanto lo stato del calendario e azzera correttamente quel flag.

Il formato monetario conserva il rapporto gia' previsto dalla modalita' euro originale: due unita' interne corrispondono a un euro. I centesimi evitano il precedente troncamento; prezzi, soglie e formule del motore restano nelle unita' originali.

## Limiti espliciti

- Il programma non e' stato eseguito su una macchina Windows 10. Compatibilita' dichiarata da manifest/API/minimo installer: Windows 10 1903+ x64. Il collaudo osservato e' Windows 11.
- I DPI multipli sono simulati per font/geometria; il trascinamento tra monitor fisici con scale diverse non e' stato verificato.
- Le prove coprono creazione di tutti i dialoghi e azioni rappresentative; non costituiscono una partita completa o un confronto runtime esaustivo con l'eseguibile del 2001.
- Il formato/checksum `.tbz` e l'importazione del registro originale sono implementati; non era disponibile un profilo storico reale dell'utente da migrare. I round trip usano fixture prodotte dal serializer del gioco, inclusa una gerarchia di registro isolata creata e rimossa dal test.
- I filesystem di rete/WSL possono non implementare la richiesta di flush durevole di Windows. La scrittura usa un file temporaneo e una sostituzione del nome; su filesystem locali si richiede anche il flush dei dati.
- Eseguibile e installer non sono firmati digitalmente.

## Artefatti e revisione

La [revisione indipendente e le regressioni corrette](CODE_REVIEW.md) e la [verifica finale con REA](research/windows-native-artifact-verification.md) riportano i dettagli. Il binario verificato e quello nello ZIP sono identici. Installer e ZIP finali sono stati ricreati dopo le correzioni; installazione, avvio, aggiornamento e disinstallazione sono passati nuovamente.
