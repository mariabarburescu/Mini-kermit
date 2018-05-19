# Mini-kermit

#########
# lib.h #
#########
	- Am definit o structura packet care contine 7 campuri asemenea celor
descrise in cerinta temei si o structura sendinit pentru campul DATA in cazul
pachetului "S" (Send-Init).
	- Functia init-sendinit primeste ca parametru o structura de tip sendinit si
o initializeaza.
	- Functia make_payload seteaza un pachet si formeaza payload-ul care urmeaza
a fi trimis.

###########
# ksender #
###########
	- Functia try_to_send primeste un mesaj si secventa curenta si trimite mesaje
catre receiver. In cazul in care primeste de la receiver de 3 ori la rand
timeout, transmisia se va opri. In cazul in care mesajul a fost trimis cu
succes, se va afisa ACK, se va actualiza secventa si se va continua programul, 
altfel se actualizeaza datele din payload, se recalculeaza crc-ul si se trimite
din nou mesajul si cu secventa actualizata.
Functia returneaza 0 daca nu s-a trimis cu succes pachetul sau 1 daca s-a
trimis cu succes.
	- Prima oara initializez structura sendinit si formez payload-ul. Trimit
pachetul de tip Send-Init si actualizez secventa. Daca nu s-a putut trimite, se
incheie transmisia. In continuare iau primul fisier din linia de comanda si
formez pachetul de tip File-Header cu numele acestuia. Daca nu s-a reusit
trimiterea, se incheie transmisia, altfel deschid fisierul si citesc data din
el, pe care le pun in cate un pachet si trimit pachete de tip Date. In cazul 
in care fisierul s-a terminat, trimit un pachet de tipul End of File, dupa care
continui acelasi algoritm si pe urmatoarele fisiere. Cand se termina fisierele
trimit un pachet de tip End of Transaction.

#############
# kreceiver #
#############
	- Functia try_to_receive are 3 parametrii: secventa curenta si o variabila
care imi spune daca mesajul primit este bun si daca pachetul este de tip EOF
sau EOT. In caz de timeout consecutiv de 3 ori se incheie transmisia, altfel
verific crc-ul si pun intr-un mesaj secventa actualizata si tipul pachetului pe
care il trimit, ACK sau NAK.
	- Prima oara primesc pachetul de tip Send-Init si verific daca este valid.
In continuare primesc un pachet de tip File-Header sau EOT. Variabila
receive_status imi va spune tipul pachetului. Daca este de tip EOT inchei
transmisia, daca este de tip File-Header creez un fisier cu prefixul "recv_" si
concatenez la nume campul data din payload. Deschid fisierul creat si astept
sa primesc pachete de tip Date sau EOF. Din nou variabila receive_status imi va
spune daca pachetul este de tip EOF, caz in care ies din bucla in care primeam
aceste pachete si inchid fisierul, dupa care o iau de la capat cu primirea
pachetelor de tip File-Header sau EOT, altfel, daca pachetul este de tip Date,
iau datele care ma intereseaza din payload si le scriu in fisierul creat si 
deschis.
