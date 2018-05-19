#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define CHUNK_TYPE 0
#define END_FILE 1

#define HOST "127.0.0.1"
#define PORT 10000

int try_to_send(msg *t, int *seq){
	int count_send = 3;
	int send_status = 0;
	int data_len = (*t).len - 7;
	int crc_index;

	while (1){
		msg *y = receive_message_timeout(5000);
		//In caz de timeout
		if (y == NULL){
			count_send--;
			//Daca s-a ajuns la al trilei timeout
			if (count_send == 0){
				break;
			}
			send_message(t);
 		}
 		//Daca am primit mesajul
		else {
			//In caz de ACK
			if ((*y).payload[3] == 'Y'){
				send_status = 1;
				//Actualizam secventa si iesim din bucla
				*seq = ((unsigned char)y->payload[2] + 1) % 64;
				break;
			}
			//In caz de NAK
			else{
				//Formam payload-ul cu secventa ultimului mesaj trimis si calculam crc-ul
				t->payload[2] = *seq;
				unsigned short crc = crc16_ccitt((*t).payload, (*t).len - 3);
				crc_index = data_len + 4;
				memcpy(t->payload + crc_index, &crc, 2);
				send_message(t);
				//Actualizam secventa si timeout-ul
				*seq = (*seq + 2) % 64;
				count_send = 3;
			}
		}
	}
	return send_status;
}

int main(int argc, char** argv) {
    msg t;

	init(HOST, PORT);

	int i;
	int seq = 0;
	int nr;
	int send_status = 0;
    packet p;
    sendinit si;

    //Trimitem pachetul Send-Init
    init_sendinit(&si);
    make_payload(&si, &p, &t, seq, 'S', SEND_INIT_SIZE);
    send_message(&t);
    printf("[%d] Send-init\n", seq);
    seq = (seq + 2) % 64;
    send_status = try_to_send(&t, &seq);

    if (send_status == 0){
        return 0;
    }

   	for (i = 1; i < argc; i++){
   		printf("=====%s=====\n", argv[i]);
   		//Trimitem pachetul de tip File Header
   		char *nume_fisier = argv[i];
   		make_payload(nume_fisier, &p, &t, seq, 'F', strlen(nume_fisier));	 
  		send_message(&t);
   		printf("[%d] File Header %d\n", seq, i);
   		seq = (seq + 2) % 64;
		send_status = try_to_send(&t, &seq);
   		if (send_status == 0){
            return 0;
    	}

    	FILE *f = fopen(argv[i], "rb");
    	nr = 1;
    	//Trimitem pachetul de tip Date
    	while(1) {
    		char date_fisier[250];
			int chunk_size = fread(date_fisier, 1, sizeof(date_fisier) - 1, f);
			if (chunk_size == 0){
				//Trimitem pachetul de tip EOF
				make_payload(NULL, &p, &t, seq, 'Z', chunk_size);
   				send_message(&t);
   				printf("[%d] EOF\n", seq);
   				seq = (seq + 2) % 64;
   				send_status = try_to_send(&t, &seq);
   				if (send_status == 0){
           			return 0;
   				}
				break;
			}

			t.len = chunk_size ;
			make_payload(&date_fisier, &p, &t, seq, 'D', t.len);
   			send_message(&t);
   			printf("[%d] Data %d\n", seq, nr);
   			seq = (seq + 2) % 64;
   			send_status = try_to_send(&t, &seq);
	    	if (send_status == 0){
                return 0;
	    	}
	    	nr++;
		}
		memset(t.payload, 0, t.len);
		fclose(f);
   	}
   	//Trimitem pachetul de tip EOT
   	make_payload(NULL, &p, &t, seq, 'B', 0);
   	send_message(&t);
   	send_status = try_to_send(&t, &seq);
 	if (send_status == 0){
        return 0;
   	}
   	printf("[%d] EOT\n", seq);
    return 0;
}