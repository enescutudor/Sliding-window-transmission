#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "my_lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

/*all functions wa included in my_lib.h*/



int main(int argc, char **argv) {
    init(HOST, PORT);

    int i;

    int file_size = file_length(argv[1]);
    int window_size = (atoi(argv[2]) * atoi(argv[3]) * 1000) / (sizeof(msg) * 8);
    int number_of_packets = (((file_size % (MSGSIZE - 12)) == 0) ? (file_size / (MSGSIZE - 12)) : (
            (file_size / (MSGSIZE - 12)) + 1));

    int delay = atoi(argv[3]);

    printf("[SEND]File to be sent: %s!\n", argv[1]);
    msg *cached_messages, name, size;


    cached_messages = (msg *) malloc(sizeof(msg) * number_of_packets);

    name = create_message(argv[1], 0, strlen(argv[1]) + 1);
    char str_file_size[10];
    sprintf(str_file_size, "%d", file_size);
    printf("[SEND]File size:%sB\n", str_file_size);
    printf("[SEND]Window size:%d\n", window_size);
    printf("[SEND]Number of packets:%d\n", number_of_packets);

    printf("[SEND]Number of packets:%dms\n", delay);
    size = create_message(str_file_size, 0, strlen(str_file_size) + 1);

    FILE *f = fopen(argv[1], "r");

    printf("[SEND]Caching file...\n");
    for (i = 0; i < number_of_packets; i++) {
        char buff[MSGSIZE - 12];
        if (file_size >= MSGSIZE - 12) {
            read(fileno(f), buff, MSGSIZE - 12);
            cached_messages[i] = create_message(buff, i, MSGSIZE - 12);
            file_size = file_size - (MSGSIZE - 12);
        } else {
            read(fileno(f), buff, file_size);
            cached_messages[i] = create_message(buff, i, file_size);
            file_size = 0;
        }
    }
    printf("[SEND]Caching finished!\n");

    int acknowledged_messages[number_of_packets];
    for (i = 0; i < number_of_packets; i++) {
        acknowledged_messages[i] = 0;
    }

    msg ack;
    int ack_name = 0, ack_size = 0;

    printf("[SEND]Sending file information...\n");
    printf("[SEND]Sending file name...\n");
    while (!ack_name) {
        send_message(&name);
        if (recv_message_timeout(&ack, 2 * delay) != -1) {
            if (ack.payload[0] == 'A') {
                ack_name = 1;
            }
        }
    }
    printf("[SEND]File name sent!\n");

    printf("[SEND]Sending file size...\n");
    while (!ack_size) {
        send_message(&size);
        if (recv_message_timeout(&ack, 2 * delay) != -1) {
            if (ack.payload[0] == 'A') {
                ack_size = 1;
            }
        }
    }
    printf("[SEND]File size sent!\n");
    printf("[SEND]File information sent!\n");


    int to_be_sent = ((number_of_packets <= window_size) ? number_of_packets : window_size) - 1;


    for (i = 0; i <= to_be_sent; i++) {
        send_message(&cached_messages[i]);
        printf("[SEND]Message with sequence number %d sent!\n", i);
    }


    do {

        to_be_sent = next_to_be_sent(acknowledged_messages, number_of_packets, to_be_sent);
        if (recv_message_timeout(&ack, 2 * delay) == -1) {
            printf("[SEND]Failed to receive ACK!\n");
        } else {
            Packet p = extract_packet(ack);
            printf("[SEND]Received ACK for message with sequence number %d received!\n", p.sequence_number);
            acknowledged_messages[p.sequence_number] = 1;
        }

        if (to_be_sent != -1) {
            send_message(&cached_messages[to_be_sent]);
            printf("[SEND]Message with sequence number %d sent!\n", to_be_sent);
        }

    } while (to_be_sent != -1);
    return 0;
}
