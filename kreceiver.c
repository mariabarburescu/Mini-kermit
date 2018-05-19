#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

msg *try_to_receive(int *seq, int *receive_status){
	int count_receive = 3;
	msg t;
	msg *y;

	while (count_receive > 0){
		y = receive_message_timeout(5000);
		if (y == NULL){
			count_receive--;
		}
		else {
			//Verific crc-ul payload-ului cu crc-ul din payload
			unsigned short crc_sender = ((*y).payload[(*y).len - 2] << 8) | (*y).payload[(*y).len - 3];
			unsigned short crc = crc16_ccitt((*y).payload, (*y).len - 3);
			if (crc_sender == crc){
				*receive_status = 1; //Mesajul este bun
				//Variabila receive_status imi va spune cand sa ies din bucla repetitiva din main+
				if (y->payload[3] == 'Z'){
					*receive_status = 2; //EOF
				}
				if (y->payload[3] == 'B'){
					*receive_status = 3; //EOT
				}
				//Trimit mesaj doar cu secventa si ACK
				*seq = ((unsigned char)y->payload[2] + 1) % 64;
				t.payload[2] = *seq;
				t.payload[3] = 'Y';
				printf("[%d] ACK pentru [%d]\n", *seq, *seq - 1);
				send_message(&t);
				*seq = (*seq + 2) % 64;
				break;
			}
			else{			
				//Trimit mesaj doar cu secventa si NAK
				t.payload[2] = *seq;
				t.payload[3] = 'N';
				printf("[%d] NAK pentru [%d]\n", *seq, *seq - 1);
				send_message(&t);
				*seq = (*seq + 2) % 64;	
				//Resetez contorul de timeout
				count_receive = 3;
			}
		}
	}
	return y;
}

int main(int argc, char** argv) {
    msg *y;
    FILE *f;
    int seq = 1;
    int length;
    int receive_status;
    char *old_file;
    char *new_file;

    init(HOST, PORT);

    //Primesc pachetul Send-Init
    y = try_to_receive(&seq, &receive_status);

    if (y == NULL){
    	return 0;
    }

    while (1){
    	//Primesc pachetul File-Header
    	y = try_to_receive(&seq, &receive_status);
    	if (y == NULL){
    		return 0;
    	}
    	//In cazul in care pachetul este de tip EOT
    	if (receive_status == 3){
    		break;
    	}
    	//Creez fisierul in care o sa pun datele din pachet
   		length = (*y).payload[1] - 4;
  		old_file = (char *)calloc(length, sizeof(char));
   		memcpy(old_file, y->payload + 4, length);
   		new_file = (char *)calloc(length + 5, sizeof(char));
   		strcpy(new_file, "recv_");
   		strcat(new_file, old_file);
   		new_file[length+4] = '\0';
   		//Deschid fisierul creat
   		f = fopen(new_file, "wb");
   		//Primesc pachete de tip Date
   		while (1){
   			y = try_to_receive(&seq, &receive_status);
   			if (y == NULL){
                return 0;
   			}
   			//In cazul in care pachetul este de tip EOF
   			if (receive_status == 2){
   				break;
   			}
   			length = (*y).payload[1] - 4;
		   	fwrite(&((*y).payload[4]), 1, length - 1, f);
   		}
   		//Inchid fisierul creat
   		fclose(f);
	}
	//S-a terminat transmisia
	printf("[%d] EOT\n", seq);
	return 0;
}