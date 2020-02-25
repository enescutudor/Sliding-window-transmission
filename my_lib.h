//
// Created by tudor on 3/19/19.
//

#ifndef MY_LIB_H
#define MY_LIB_H
#define MSGSIZE 1400

typedef struct packet {
    char information[MSGSIZE - 12];
    int sequence_number;
    int length;
    int checksum;
}Packet;

int checksum(char const *data, int len) {
    int i;
    int sum = 0;

    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum % 2;
}

int file_length(char const *file_name) {
    FILE *f;
    f = fopen(file_name, "r");
    fseek(f, 0, SEEK_END);
    int size = (int) ftell(f);
    fclose(f);
    return size;
}

void zero(char *c, int length) {
    int i;
    for(i = 0; i < length; i++) {
        c[i] = '\0';
    }
}

int how_many_packets_left(int *ack, int number_of_packets) {
    int i;
    int l = 0;
    for(i = 0; i < number_of_packets; i++) {
        if(ack[i] == 0)
            ++l;
    }
    return l;
}

msg create_message(char const *payload, int sequence_number, int length) {
    msg m;
    Packet p;

    memset(&p, '\0', MSGSIZE);

    memcpy(p.information, payload, length);
    p.sequence_number = sequence_number;
    p.length = length;
    p.checksum = 0;

    char to_check[MSGSIZE];
    memcpy(to_check, &p, MSGSIZE);

    int aux_checksum = checksum(to_check, MSGSIZE);
    p.checksum = aux_checksum;


    m.len = MSGSIZE;

    memcpy(m.payload, &p, MSGSIZE);

    return m;
}

int extract_checksum(msg m) {
    Packet p;
    memcpy(&p, m.payload, MSGSIZE);
    return p.checksum;
}

int extract_seq_number(msg m) {
    Packet p;
    memcpy(&p, m.payload, MSGSIZE);
    return p.sequence_number;
}

int extract_length(msg m) {
    Packet p;
    memcpy(&p, m.payload, MSGSIZE);
    return p.length;
}

Packet extract_packet(msg m) {
    Packet p;
    memcpy(&p, m.payload, MSGSIZE);
    return p;
}

int are_all_acknowledged(int *v, int number_of_packets) {
    int i;
    for(i = 0; i < number_of_packets; i++) {
        if(v[i] == 0) {
            return 0;
        }
    }
    return 1;
}

int next_to_be_sent(int *v, int number_of_packets, int last_sent) {
    int i;
    for(i = last_sent + 1; i < number_of_packets; i++) {
        if(v[i] == 0) {
            return i;
        }
    }

    for(i = 0; i <= last_sent; i++) {
        if(v[i] == 0) {
            return i;
        }
    }
    return -1;
}

msg create_ack(int seq_number) {
    msg ack;
    Packet p;
    p.sequence_number = seq_number;
    p.information[0] = 'A';
    p.length = 1;
    memcpy(ack.payload, &p, MSGSIZE);
    ack.len = MSGSIZE;
    return ack;
}


#endif //MY_LIB_H
