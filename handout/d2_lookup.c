/* ======================================================================
 * YOU ARE EXPECTED TO MODIFY THIS FILE.
 * ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "d2_lookup.h"


#define PRINT_DEBUG_INFO 1

void check_error_d2(int res, char *msg, int line, char* file) {
    if(res == -1) {
        print_error_line_d2(line, file, msg);
    }
};

void print_error_line_d2(int line, char* file, char* message) {
    printf("\033[0;31m✘: D2 Error: %s\033[0m\n", message );
    printf("\033[0;31m✘: on line: %d file: %s\033[0m\n", line, file);
}

void print_line_d2(int line, const char *file, const char *message) {
    if(PRINT_DEBUG_INFO) {
        printf("\033[1;36m --> Going through line: %d file: %s\033[0m\n", line, file);
        printf("\033[1;36m --> Done D2 task { %s } \033[0m\n\n", message);
    }
}


D2Client* d2_client_create( const char* server_name, uint16_t server_port )
{
    D2Client* client = (D2Client*)malloc(sizeof(D2Client));
    if( !client ) {
        check_error_d2(-1, "Failed to allocate memory for D2Client", __LINE__, __FILE__);
        return NULL;
    }

    // Split up the code for better readability and error handling
    D1Peer* peer = d1_create_client(server_name, server_port);
    if (peer == NULL) {
        free(client);
        check_error_d2(-1, "Failed to create D1Peer", __LINE__, __FILE__);
        return NULL;
    }

    int wc = d1_get_peer_info(peer, server_name, server_port);
    if (wc == 0) {
        d1_delete(peer);
        free(client);
        check_error_d2(-1, "Failed to get peer info", __LINE__, __FILE__);
        return NULL;
    }
    client->peer=peer;
    print_line_d2(__LINE__, __FILE__, "Created D2 client");
    return client;
}

D2Client* d2_client_delete( D2Client* client )
{
    /* implement this */
    return NULL;
}

int d2_send_request( D2Client* client, uint32_t id ) {
    
    if( id <= 1000) {
        check_error_d2(-1, "D2 student ID is too low and not accepted", __LINE__, __FILE__);
        return -1;
    }

    PacketRequest* pack = (PacketRequest*)malloc(sizeof(PacketRequest));
    if( !pack ) {
        check_error_d2(-1, "Failed to allocate memory for PacketRequest", __LINE__, __FILE__);
        return -1;
    }

    pack->id = htonl(id);
    pack->type = htons(TYPE_REQUEST);

    int wc = d1_send_data(client->peer, pack, sizeof(PacketRequest));
    if( wc <= 0 ) {
        check_error_d2(-1, "Failed to send data", __LINE__, __FILE__);
        return -1;
    }

    print_line_d2(__LINE__, __FILE__, "Sent request to server");
    return wc;
}

int d2_recv_response_size( D2Client* client )
{
    char buffer[sizeof(PacketResponseSize)];
    int wc = d1_recv_data(client->peer, buffer, sizeof(PacketResponseSize));
    
    if( wc <= 0 ) {
        check_error_d2(-1, "Failed to receive data", __LINE__, __FILE__);
        return -1;
    }

    // This is just to check that it has the correct type, would not cause any issues without it, but clean
    PacketHeader* packCheck = (PacketHeader*)buffer;
    if( ntohs(packCheck->type) != TYPE_RESPONSE_SIZE ) {
        check_error_d2(-1, "Received wrong packet type", __LINE__, __FILE__);
        return -1;
    }

    PacketResponseSize* pack = (PacketResponseSize*)buffer;
    int num_netNodes = ntohs(pack->size);

    print_line_d2(__LINE__, __FILE__, "Received response size from server (d2_recv_response_size)");
    return num_netNodes;
}

int d2_recv_response( D2Client* client, char* buffer, size_t sz )
{
    /* implement this */
    return 0;
}

LocalTreeStore* d2_alloc_local_tree( int num_nodes )
{
    /* implement this */
    return NULL;
}

void  d2_free_local_tree( LocalTreeStore* nodes )
{
    /* implement this */
}

int d2_add_to_local_tree( LocalTreeStore* nodes_out, int node_idx, char* buffer, int buflen )
{
    /* implement this */
    return 0;
}

void d2_print_tree( LocalTreeStore* nodes_out )
{
    /* implement this */
}

