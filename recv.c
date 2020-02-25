#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "my_lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char **argv) {
    init(HOST, PORT);

    msg *recv_messages;
    msg name, size;
    msg ack;
    int i;

    msg nack;
    Packet aux;
    aux.information[0] = 'N';
    aux.length = 1;
    memcpy(nack.payload, &aux, MSGSIZE);
    nack.len = MSGSIZE;

    char payload[MSGSIZE];

    int ack_name = 0, ack_size = 0;
    char filename[50];
    int filesize;


    printf("[RECV]Prepared to receive information about file...\n");
    while (!ack_name) {
        recv_message(&name);
        Packet p = extract_packet(name);
        memcpy(payload, &p, MSGSIZE);
        char to_check[MSGSIZE];
        memcpy(to_check, p.information, p.length);
        memcpy(to_check + p.length, &p.sequence_number, 4);
        memcpy(to_check + p.length + 4, &p.length, 4);
        if (checksum(to_check, p.length + 8) == p.checksum) {
            ack_name = 1;
            ack.len = 1;
            strcpy(filename, p.information);
            ack.payload[0] = 'A';
            send_message(&ack);
            printf("[RECV]Name received. Sending ACK!\n");
        } else {
            ack_size = 1;
            ack.len = 1;
            ack.payload[0] = 'N';
            send_message(&ack);
            printf("[RECV]Name corrupted. Sending NACK!\n");
        }
    }

    while (!ack_size) {
        recv_message(&size);
        Packet p = extract_packet(size);
        char to_check[MSGSIZE];
        memcpy(to_check, p.information, p.length);
        memcpy(to_check + p.length, &p.sequence_number, 4);
        memcpy(to_check + p.length + 4, &p.length, 4);
        if (checksum(to_check, p.length + 8) == p.checksum) {
            ack_size = 1;
            ack.len = 1;
            filesize = atoi(p.information);
            ack.payload[0] = 'A';
            send_message(&ack);
            printf("[RECV]Size received. Sending ACK!\n");
        } else {
            ack_size = 1;
            ack.len = 1;
            ack.payload[0] = 'N';
            send_message(&ack);
            printf("[RECV]Size corrupted. Sending NACK!\n");
        }
    }

    Packet name_packet = extract_packet(name);
    strcpy(filename, name_packet.information);
    Packet size_packet = extract_packet(size);
    filesize = atoi(size_packet.information);
    int number_of_packets = (((filesize % (MSGSIZE - 12)) == 0) ? (filesize / (MSGSIZE - 12)) : (
            filesize / (MSGSIZE - 12) + 1));
    int acknowledged_messages[number_of_packets];
    for (i = 0; i < number_of_packets; i++) {
        acknowledged_messages[i] = 0;
    }
    printf("[RECV]File to be received: %s, %dB\n", filename, filesize);

    recv_messages = (msg *) malloc(number_of_packets * sizeof(msg));


    while (!are_all_acknowledged(acknowledged_messages, number_of_packets)) {

        msg rc;
        recv_message(&rc);

        Packet p = extract_packet(rc);
        printf("[RECV]Message received!\n");
        int aux_checksum = p.checksum;
        p.checksum = 0;


        char to_check[MSGSIZE];
        memcpy(to_check, &p, MSGSIZE);

        if (checksum(to_check, MSGSIZE) == aux_checksum) {
            ack = create_ack(p.sequence_number);
            send_message(&ack);
            printf("[RECV]Message with sequence number %d was received!\n", p.sequence_number);
            acknowledged_messages[p.sequence_number] = 1;
            recv_messages[p.sequence_number] = rc;
        } else {
            //send_message(&nack);
            printf("[RECV]Corrupt message received!\n");
        }

    }

    char new_filename[50];
    strcpy(new_filename, "recv_");
    strcat(new_filename, filename);
    new_filename[5 + strlen(filename)] = '\0';
    FILE * new = fopen(new_filename, "w");
    for (i = 0; i < number_of_packets; i++) {
        Packet aux_packet = extract_packet(recv_messages[i]);
        write(fileno(new), aux_packet.information, aux_packet.length);
    }
    fclose(new);
    return 0;
}
