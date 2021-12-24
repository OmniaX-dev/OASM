
clona il repository https://github.com/Sylar39/OASM.git

per compilare il progetto su Linux, devi avere 'cmake', 'g++' e 'make' installati, e nella variabile <PATH> di sistema
poi semplicemente, in un terminale nella cartella OASM esegui:
	./run-cmake
E il progetto dovrebbe essere compilato senza problemi

in un terminale, entra nella cartella OASM/bin

Esegui questo comando per compilare lo script fibonacci.oasm
	./oasm-as -D -i Esempi/fibonacci.oasm -I ostd -lsd ostd/ostd.oslib

Esegui questo comando per compilare lo script random.oasm
	./oasm-as -D -i Esempi/random.oasm -I ostd -lsd ostd/ostd.oslib

poi sempre nella cartella OASM/bin puoi usare:
	./oasm-vm -i Esempi/fibonacci.oex
oppure
	./oasm-vm -i Esempi/random.oex
per eseguire lo script che vuoi


'oasm-as' è l'eseguibile del compilatore
'oasm-vm' è l'eseguibile della VirtualMachine
'oasm-dbg' è l'eseguibile del debugger
Questi sono solo frontend, il codice per tutti e tre si trova nella libreria condivisa liboasm-lib.so


Spiegazione dei parametri di 'oasm-as':
	-D									Usato per compilare lo script generando anche tutte le informazioni per il debug
	-i <percorso/del/file> 				Usato per specificare il percorso del file sorgente da compilare
	-I <percorso/della/directory>		Usato per specificare un percorso dove cercare i file quando viene usata la direttiva .include [...] nel codice
	-lsd <percorso/della/libreria>		Usato per linkare lo script contro una specifica libreria statica
	
Le estensioni dei file:
	*.oasm, *.oh		Sono per file che contengono codice sorgente
	*.odb				Usata per i file contenenti le informazioni di debug
	*.oex				Usata per i file contenenti il codice compilato (si può considerare come un file eseguibile per la VirtualMachine)
I file *.oex sono gli unici ad essere file binari, gli altri sono tutti file di testo

Dopo aver compilato gli script, toverai nella cartella Esempi vari file generati dal compilatore

Il progetto è scritto usando VisualStudio-Code come editor
Per vedere i file *.oasm con un po' di syntax-highlight, io specifico il linguaggio come "C++",
e così almeno mi evidenzia i numeri, le strnghe, i commenti e cose così




