/* ======================================================================
 * YOU ARE EXPECTED TO MODIFY THIS FILE.
 * ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "d1_udp.h" 



#define IP "127.0.0.1"
#define PORT 3678
#define PACKET_MAX 1024
static int isConnected = 0;

/* 
    Self defined function to check for errors
    Based on system calls returning -1 on errors
 */
void check_error(int res, char *msg) {
    if(res == -1) {
        perror(msg);
    }
};


void print_error_line(int line, int file) {
    printf("\033[0;31m✘: on line: %d file: %s\033[0m\n", line, file);
}

void print_line(int line, int file) {
    printf("\033[0;32m--> Going through line: %d file: %s\033[0m\n", line, file);
}


/**
 * Calculates the checksum for the given buffer.
 *
 * @param newBuffer The buffer containing the data to calculate the checksum for.
 * @param size The size of the buffer.
 * @return The calculated checksum as a 16-bit unsigned integer.
 */
uint16_t calculate_checksum(char* newBuffer, int size) {
    // Calculate the checksum
    uint8_t checksum_odd = 0;
    uint8_t checksum_even = 0;

    for (int i = 0; i < size; i++) {
        if (i % 2 == 0) {
            checksum_odd ^= newBuffer[i];
        } else {
            checksum_even ^= newBuffer[i];
        }
    }
    if(size % 2 == 1) {
        checksum_odd ^= 0;
    };

    // This combines the two checksums, which are one byte each, into a two byte full one
    /** no googling involved at all no no
     * Combines the odd and even checksum values into a single 16-bit checksum.
     *
     * @param checksum_odd The checksum value from X-ORing the odd bytes, this one also Xors with a 0 if the size is odd
     * @param checksum_even The  checksum value from X-ORing the even bytes.
     * @return The combined 16-bit checksum.
     */
    uint16_t checksum = (checksum_odd << 8) | checksum_even;

    return checksum;
}


/** 
 * 
 *  Create a UDP socket that is not bound to any port yet. This is used as a
 *  client port.
 *  @return
 *  Returns the pointer to a structure on the heap in case of success or NULL
 *  in case of failure.
 */
D1Peer* d1_create_client() {
    // Function implementation goes here
    int wc;

    // malloc memory for the struct, and check if it actually allocated memory
    D1Peer* peer = (D1Peer*)malloc(sizeof(D1Peer));
    if (peer == NULL) {
        return NULL;
    }

    // Create the file descriptor for the socket (sock-fd), that holds a socket
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(sockfd, "socket creation");
    if (sockfd == -1) {
        return NULL;
    }
    peer->socket = sockfd;

    return peer;
}

D1Peer* d1_delete( D1Peer* peer ) {

    if (peer != NULL) {
        // Close the socket
        close(peer->socket);
        
        // Free the memory of the server struct
        free(peer);
    }
    return NULL;
}

/** Determine the socket address structure that belongs to the server's name
 *  and port. The server's name may be given as a hostname, or it may be given
 *  in dotted decimal format.
 *  In this excersize, you do not support IPv6. There are no extra points for
 *  supporting IPv6.
 *  If the discovery succeeds, store the information in the D1Peer structure
 *  client.
 *  Otherwise, terminate the client and quit.
 * @return Returns 0 in case of error and 1 in case of success.
 */
int d1_get_peer_info( struct D1Peer* peer, const char* peername, uint16_t server_port )
{
    // Set the destination address to the socket. Set IPv4
    struct sockaddr_in dest_addr;
    int wc = inet_pton(AF_INET, IP, &dest_addr.sin_addr);
    
    // Otherwise, terminate the client and quit.
    if (wc < 1) {
        fprintf(stderr, "Invalid IP address\n");
        d1_delete(peer);
        return 0;
    }
    dest_addr.sin_family = AF_INET;
    //Check out
    dest_addr.sin_port = htons(PORT);
    peer->addr = dest_addr;

    return 1;
}
int d1_recv_data(struct D1Peer* peer, char* buffer, size_t sz) {
    
    char* packet = (char*)malloc(sz);
    
    D1Header* header = (D1Header*)malloc(sizeof(D1Header));

    ssize_t bytes_received = recvfrom(peer->socket, packet, sz, 0, NULL, NULL );

    memcpy(&header->flags, packet, sizeof(header->flags));
    memcpy(&header->checksum, packet + sizeof(header->flags), sizeof(header->checksum));
    memcpy(&header->size, packet + sizeof(header->flags) + sizeof(header->checksum), sizeof(header->size));
    
    header->flags = ntohs(header->flags);
    header->checksum = ntohs(header->checksum);
    header->size = ntohl(header->size);

    // Create a copy of the packet with byte 3 and 4 set to null
    char* packet_copy = (char*)malloc(bytes_received);
    memcpy(packet_copy, packet, bytes_received);
    packet_copy[2] = '\0';
    packet_copy[3] = '\0';

    uint16_t checksum = calculate_checksum(packet_copy, bytes_received);
    uint32_t size = bytes_received;
    
    // check if checksum and size is correct with actual values.
    if (checksum == header->checksum && size == header->size) {
        d1_send_ack(peer, peer->next_seqno);
    }
    
    check_error(bytes_received, "recvfrom");
    memcpy(buffer, packet + sizeof(D1Header), bytes_received - sizeof(D1Header));
    return bytes_received - sizeof(D1Header);
}


/**
 * @brief Waits for an acknowledgment pack from a D1Peer.
 * 
 * Function must always block after sending a data packet or connect packet until it has received the
 * correct ACK. If it receives the wrong ACK or does not receive an ACK within 1 second, it
 * must return -1 to resend its parents data packet or connect packet
 *
 * @param peer The D1Peer to wait for acknowledgment from.
 * @param buffer The buffer to store the received acknowledgment.
 * @param sz The size of the buffer.
 * @return Returns 1 if the ack was received and successful, -1 if there is an error, 0 on timeout
 */
int d1_wait_ack(D1Peer* peer, char* buffer, size_t sz) {

    int real_seqno = peer->next_seqno;
    int pack_received = -1; // -1 means not received
    int counter = 0;

    while (pack_received == -1 && counter < 100) {
        char received_packet[sz];
        ssize_t bytes_received = recvfrom(peer->socket, received_packet, sz, 0, NULL, NULL);
        sleep( 0.01 ); // 10ms sleep, to not bloat CPU from more advanced blocking, ( may be more than 1 second total tho, but i would argue that this is better than blocking the cpu for exactly one second)
        counter++;

        if (bytes_received > 0 ) {
            pack_received = 1;
            // ACK received if 8th bit equal to 1 
            int received_ackno = ((ntohs(received_packet[0]) & (1<<8)) >> 8); 
            int received_seqno = ((ntohs(received_packet[1]) & (1<<8)) >> 8); 
            
            printf("Received ackno: 0x%02X\n", received_ackno);
            printf("Received seqkno: 0x%02X\n", received_seqno);
            
            printf("Real seqno: %d\n", received_seqno);


            if (received_seqno != real_seqno ) {
                // If the received ack is not the same as the one we sent, resend the packet, done at a higher level
                print_error_line(__LINE__,__FILE__);
                return -1;
            } else {
                peer->next_seqno = !peer->next_seqno;
                return 1; // Return a positive value in case of success
            }
        }
    }
    return 0;
}

int d1_send_data(D1Peer* peer, char* buffer, size_t sz) {
  
    // TODO: Check if the buffer size exceeds the packet size
    int size = sz + sizeof(D1Header);
    if (size > PACKET_MAX) {
        // data and header size exceeds 1024 bytes
        perror("exceeding 1024 bytes pack limit\n");
        return -1;
    }
    
    int wc;

    // Create the D1 header
    D1Header* header = (D1Header*)malloc(sizeof(D1Header));


    print_line(__LINE__, __FILE__);
    
    // Keep it simple, could use bitwise and with peer->next_seqno, but this is way readable. 
    if(peer->next_seqno) {
        header->flags = htons(FLAG_DATA |  SEQNO); // Set the data packet flag and seqno
    } else {
        header->flags = htons(FLAG_DATA); // Set the data packet flag
    }
    
    printf("Flags (hex): 0x%04X\n", ntohs(header->flags));
    printf("This is the next seqno: %d\n", peer->next_seqno);
    header->size = htonl(size);       // Set the length of the data
    header->checksum = 0 ;    // Set the checksum to 0, since we are not implementing it
    char newBuffer[size];

    // Place the header in the "packet", step by step
    // Else, only the flags will be put in memory, as header is a pointer and only will add flags
    memcpy(newBuffer, &(header->flags), 2);
    memcpy(newBuffer + 2, &(header->checksum), 2);
    memcpy(newBuffer + 4, &(header->size), 4);
    // Place the data in the packet, at place sizeof(header), which means right after the header
    memcpy(newBuffer + sizeof(header), buffer, sz);
    check_error(wc, "memcpy udp header :in: d1_send_data");
    
    header->checksum= htons(calculate_checksum(newBuffer, size));
    memcpy(newBuffer + 2, &(header->checksum), 2);
    check_error(wc, "memcpy udp checksum :in: d1_send_data");

    wc = sendto(peer->socket, newBuffer, size, 0, (struct sockaddr*)&(peer->addr), sizeof(peer->addr));
    check_error(wc, "sendto");    

    wc = d1_wait_ack(peer, buffer, sz);

    check_error(wc, "d1_wait_ack");

    if(wc == -1) {
        d1_send_data(peer, buffer, sz);
    } else if (wc == 0)
    {
        printf("Timeout, server disconnected\n");
    }

    return wc;
}

void d1_send_ack( struct D1Peer* peer, int seqno )
{
    
    int wc;
    int size = 8;

    // Create the D1 header
    D1Header* header = (D1Header*)malloc(sizeof(D1Header));

        // Keep it simple, could use bitwise and with peer->next_seqno, but this is way readable. 
    if(peer->next_seqno) {
        header->flags = htons(FLAG_ACK |  ACKNO); // Set the data packet flag and seqno
    } else {
        header->flags = htons(FLAG_ACK); // Set the data packet flag
    }

    header->size = htonl(size);       // Set the length of the data
    header->checksum = 0 ;    // Set the checksum to 0, since we are not implementing it
    char newBuffer[size];

    printf("Flags (binary): ");
    for (int i = 0; i <= 15; i++) {
        printf("%d", (ntohs(header->flags) >> i) & 1);
    }
    printf("\n");

    // Place the header in the "packet", step by step
    // Else, only the flags will be put in memory, as header is a pointer and only will add flags
    wc = memcpy(newBuffer, &(header->flags), 2);
    wc = memcpy(newBuffer + 2, &(header->checksum), 2);
    wc = memcpy(newBuffer + 4, &(header->size), 4);
    // Place the data in the packet, at place sizeof(header), which means right after the header
    check_error(wc, "memcpy udp header :in: d1_send_data");
    
    header->checksum= htons(calculate_checksum(newBuffer, size));
    wc = memcpy(newBuffer + 2, &(header->checksum), 2);
    check_error(wc, "memcpy udp checksum :in: d1_send_data");

    wc = sendto(peer->socket, newBuffer, size, 0, (struct sockaddr*)&(peer->addr), sizeof(peer->addr));
    check_error(wc, "sendto");    
    return wc;

}

