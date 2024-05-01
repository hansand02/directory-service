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



#define PACKET_MAX 1024
#define PRINT_DEBUG_INFO 0


void check_error(int res, char *msg, int line, char* file) {
    if(res == -1) {
        print_error_line(line, file, msg);
    }
};

void print_error_line(int line, char* file, char* message) {
    printf("\033[0;31m✘: D1 Error: %s\033[0m\n", message );
    printf("\033[0;31m✘: on line: %d file: %s\033[0m\n", line, file);
}

void print_line(int line, const char *file, const char *message) {
    if(PRINT_DEBUG_INFO) {
        printf("\033[0;36m \t--> Going through line: %d file: %s\033[0m\n", line, file);
        printf("\033[0;36m \t--> Done D1 task { \033[1;39m%s\033[1;39m  } \033[0m\n\n", message);
    }
}

/**
 *This function calculates the checsum as described in the handout/ChecksumExplanation.md
 *
 * @param newBuffer The buffer containing the data to calculate the checksum for.
 * @param size the size of the buffer.
 * @return The calculated checksum as a 16-bit unsigned integer.
 */
uint16_t calculate_checksum(char* newBuffer, int size) {
    // Calculate the checksum

    uint8_t checksum_odd = 0;
    uint8_t checksum_even = 0;

    for (int i = 0; i < size; i++) {
        if (i  == 2 || i == 3) {
            // We are in the checksum field, so we skip it
            continue;
        }
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
    uint16_t checksum = (checksum_odd << 8) | checksum_even;

    return checksum;
}

void reset_socket_options(D1Peer* peer) {
    // Reset the socket options to standard values
    setsockopt(peer->socket, SOL_SOCKET, SO_RCVTIMEO, NULL, 0);
}

/** 
 * 
 *  Create a UDP socket that is not bound to any port yet. This is used as a
 *  client port.
 *  @return Returns the pointer to a structure on the heap in case of success or NULL
 *  in case of failure.
 */
D1Peer* d1_create_client() {
    int wc = 0;

    //Calloc memory for the struct, using this to avoid "== Uninitialised value was created by a heap allocation"
    D1Peer* peer = (D1Peer*)calloc(1, sizeof(D1Peer));
    if (peer == NULL) {
        return NULL;
    }

    //Create the file descriptor for the socket (sock-fd), that holds a socket
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(sockfd, "socket creation", __LINE__, __FILE__);
    if (sockfd == -1) {
        d1_delete(peer);
        return NULL;
    }
    peer->socket = sockfd;

    print_line(__LINE__, __FILE__, "Created client (d1_create_client)");
    return peer;
}

D1Peer* d1_delete( D1Peer* peer ) {
    // delete the peer and close the socketfd
    if (peer != NULL) {
        close(peer->socket);
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
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(server_port);

    //changed from earlier, to cover "erver's name may be given as a hostname, or it may be givenin dotted decimal format."
    // inet_pton is only for dotted decimal, and if hosname is given, we need to use gethostbyname
    if (inet_pton(AF_INET, peername, &dest_addr.sin_addr) <= 0) {
        struct hostent* host = gethostbyname(peername);
        if (host == NULL || host->h_addrtype != AF_INET) {
            check_error(-1, "gethostbyname", __LINE__, __FILE__);
            d1_delete(peer); // Im told to terminate the client, but the test file also does this on return=0; but im doing it:)
            return 0;
        }
        memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);
    }
    peer->addr = dest_addr;
    print_line(__LINE__, __FILE__, "Got peer info (d1_get_peer_info)");
    return 1;
}

/**
 * @brief Call this to wait for a single packet from the peer. The function checks if the
 *  size indicated in the header is correct and if the checksum is correct.
 * 
 * @param peer The D1Peer structure representing the peer connection.
 * @param buffer The buffer to store the received data.
 * @param sz The size of the buffer.
 * @return The number of bytes received on success (can be 0), or -1 on failure.
 */
int d1_recv_data(struct D1Peer* peer, char* buffer, size_t sz) {

    char packet[sizeof(D1Header) + sz];
    int notCompleted = 1;
    ssize_t bytes_received = 0;
    D1Header* header;
    
    // Using recvfrom, since we are using Udp, so source adress is more critical.
    socklen_t fromlen = sizeof(peer->addr);
    bytes_received = recvfrom(peer->socket, packet, sz + sizeof(D1Header), 0, (struct sockaddr*)&(peer->addr), &fromlen); 
    /* bytes_received = recv(peer->socket, packet, sz + sizeof(D1Header), 0); */
    if (bytes_received < 0 || bytes_received > sz + sizeof(D1Header)) {
        check_error(bytes_received, "error with bytes received(d1_recv_data)", __LINE__, __FILE__);
        return -1;
    } 

    header = (D1Header*)packet;
    header->flags = ntohs(header->flags);
    header->checksum = ntohs(header->checksum);
    header->size = ntohl(header->size);
    
    

    // Calculate the checksum, does not calculate over the checksum field
    uint16_t checksum = calculate_checksum(header, bytes_received);
    
    // check if checksum and size is correct with actual values.
    // send ack with correct seqno if correct, else send ack with wrong seqno, this should trigger server to retransmit
    if ((checksum != header->checksum) | (bytes_received != header->size)) {
        d1_send_ack(peer, !(header->flags & SEQNO));
    } else if (header->flags & FLAG_DATA) {
        d1_send_ack(peer, header->flags & SEQNO);
    }
  
    memcpy(buffer, packet + sizeof(D1Header), sz);
   
    print_line(__LINE__, __FILE__, "Received data (d1_recv_data)");
    
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
 * @return Returns 1 if the ack was received and successful, -1 if there is an error or timeout
 */
int d1_wait_ack(D1Peer* peer, char* buffer, size_t sz) {

    int real_seqno = peer->next_seqno;
    int pack_received = -1; // -1 means not received
    //Looks quirky, but it just sets the timeout to 1 second. 
    setsockopt(peer->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&(struct timeval){1, 0}, sizeof(struct timeval));
    // Helper variable for recvfrom
    socklen_t fromlen = sizeof(peer->addr);


    while (pack_received == -1) {
        char received_packet[sz];
        ssize_t bytes_received = recvfrom(peer->socket, received_packet, sz + sizeof(D1Header), 0, (struct sockaddr*)&(peer->addr), &fromlen);
        if(bytes_received == -1) {
            reset_socket_options(peer);
            check_error(bytes_received, "timeout, ack not received", __LINE__, __FILE__);
            return -1;
        }

        if (bytes_received > 0 ) {
            pack_received = 1;
            // ACK received if 8th bit equal to 1 
            int received_ackno = ((ntohs(received_packet[0]) & (1<<8)) >> 8); 
            int received_seqno = ((ntohs(received_packet[1]) & (1<<8)) >> 8); 

            if (received_seqno != real_seqno ) {
                // If the received ack is not the same as the one we sent, resend the packet, done at a higher level
                check_error(-1, "Received ack is not the same as the one we sent", __LINE__, __FILE__);
                reset_socket_options(peer);
                return -1;
            } else {
                peer->next_seqno = !peer->next_seqno;
                reset_socket_options(peer);
                print_line(__LINE__, __FILE__, "Received ack (d1_wait_ack)");
                return 1; // Return a positive value in case of success
            }
        }
    }
}

/**
 * @brief If the buffer does not exceed the packet size, the function adds the D1 header and sends
 *  it to the peer.
 *
 * @param peer    The D1Peer to send the data to.
 * @param buffer  The buffer containing the data to send.
 * @param sz      The size of the data to send.
 * @return        Returns bytes sent on success, or a negative value on failure.
 */

int d1_send_data(D1Peer* peer, char* buffer, size_t sz) {
    int wc = 0;
    // TODO: Check if the buffer size exceeds the packet size
    int size = sz + sizeof(D1Header);
    if (size > PACKET_MAX) {
        // data and header size exceeds 1024 bytes
        check_error(-1, "Data and header size exceeds 1024 bytes", __LINE__, __FILE__);
        return -1;
    }

    // Create the D1 header
    D1Header* header = (D1Header*)malloc(sizeof(D1Header));
    if(header == NULL) {
        check_error(-1, "Malloc header d1_send_data", __LINE__, __FILE__);
        return -1;
    }
    
    // Keep it simple, could use bitwise and with peer->next_seqno, but this is way readable. 
    if(peer->next_seqno) {
        header->flags = htons(FLAG_DATA |  SEQNO); // Set the data packet flag and seqno if it is one
    } else {
        header->flags = htons(FLAG_DATA); // Set the data packet flag
    }
    
    header->size = htonl(size);       // Set the length of the data in network long byte order
    char newBuffer[size];

    // Place the header in the "packet", step by step
    // Else, only the flags will be put in memory, as header is a pointer and only will add flags
    memcpy(newBuffer, &(header->flags), 2);
    memcpy(newBuffer + 2, &(header->checksum), 2);
    memcpy(newBuffer + 4, &(header->size), 4);

    // Place the data in the packet, at place sizeof(header), which means right after the header
    memcpy(newBuffer + sizeof(header), buffer, sz);
    
    // Set the checksum
    header->checksum= htons(calculate_checksum(newBuffer, size));
    memcpy(newBuffer + 2, &(header->checksum), 2);

    // SEND THE PACKET
    int bytes_sent = sendto(peer->socket, newBuffer, size, 0, (struct sockaddr*)&(peer->addr), sizeof(peer->addr));
    check_error(wc, "sendto", __LINE__, __FILE__);    

    // Wait for the ack, this goes on forever if there is no ack arriving, but this is what the handout says. 
    wc = d1_wait_ack(peer, buffer, sz);
    if(wc == -1) {
        check_error(wc, "d1_wait_ack", __LINE__, __FILE__);
        free(header);
        return d1_send_data(peer, buffer, sz);
    }
    
    print_line(__LINE__, __FILE__, "Sent data (d1_send_data)");
    free(header);
    return bytes_sent;
}

/**
 * Sends an acknowledgment to a peer with the specified sequence number.
 *
 * @param peer The pointer to the D1Peer structure representing the peer.
 * @param seqno The sequence number to be included in the acknowledgment (0 or 1).
 */
void d1_send_ack( struct D1Peer* peer, int seqno )
{
    
    int wc = 0;
    int size = 8;

    // Create the D1 header
    D1Header* header = (D1Header*)malloc(sizeof(D1Header));
    if(header == NULL) {
        check_error(-1, "Malloc header d1_send_ack", __LINE__, __FILE__);
        return -1;
    }

    // Keep it simple, could use bitwise and with peer->next_seqno, but this is way readable. 
    // Since the only value seqno can have is 0 or 1, this works:). Dont know why i had to reverse seqno, but it works. 
    if(!seqno) {
        header->flags = htons(FLAG_ACK |  ACKNO); // Set the data packet flag and seqno
    } else {
        header->flags = htons(FLAG_ACK); // Set the data packet flag
    }

    header->size = htonl(size);       // Set the length of the data
    char newBuffer[size];

    // Place the header in the packet, step by step
    // Else, only the flags will be put in memory, as header is a pointer and only will add flags
    memcpy(newBuffer, &(header->flags), 2);
    memcpy(newBuffer + 2, &(header->checksum), 2);
    memcpy(newBuffer + 4, &(header->size), 4);
    // Place the data in the packet, at place sizeof(header), which means right after the header
    header->checksum= htons(calculate_checksum(newBuffer, size));
    memcpy(newBuffer + 2, &(header->checksum), 2);

    wc = sendto(peer->socket, newBuffer, size, 0, (struct sockaddr*)&(peer->addr), sizeof(peer->addr));
    if(wc == -1) {
        check_error(wc, "sending ack d1_send_ack", __LINE__, __FILE__);
        free(header); // No stated reason to recursively call send_ack. 
    }
    free(header);
    print_line(__LINE__, __FILE__, "Sent ack (d1_send_ack)");
}

