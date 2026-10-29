# ADR-0001 — Motore C, Win32 moderno e valuta euro

Stato: accettato. Data: 2026-10-29.

L'utente ha scelto di conservare il motore C e modernizzare l'interfaccia con controlli Windows nativi, mantenendo il gioco riconoscibile. Ha approvato il progetto e richiesto euro e pubblicazione su `luca-iannelli/tabboz-v2`.

La build usa MSVC/CMake x64, risorse rigenerate e API Windows di sistema. Il rapporto euro originale e' conservato, aggiungendo i centesimi; non vengono riconvertite le formule del motore. I callback sono corretti per la larghezza dei puntatori. I nuovi asset forniti dall'utente sono incorporati.

I salvataggi usano file separati dal programma, checksum originale, scrittura temporanea e sostituzione atomica. Il file I/O diretto sostituisce le API di scrittura INI, che il test ha dimostrato non funzionanti sul percorso UNC WSL. Il caricamento non rigioca una giornata. La chiusura offre uscita senza salvataggio quando la scrittura fallisce.

Conseguenze: applicazione leggera e senza browser/BWCC; la presentazione richiede gestione esplicita di font, layout e DPI. Il minimo supportato e' Windows 10 1903 per il manifest UTF-8. Windows 10 e multi-monitor fisico richiedono ancora collaudo su macchine appropriate.
