/* ======================================================================
 * YOU ARE EXPECTED TO MODIFY THIS FILE.
 * ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "d2_lookup.h"


#define PRINT_DEBUG_INFO 0


void print_error_line_d2(int line, char* file, char* message) {
    printf("\033[0;31m✘: D2 Error: %s\033[0m\n", message );
    printf("\033[0;31m✘: on line: %d file: %s\033[0m\n", line, file);
}

void print_line_d2(int line, const char *file, const char *message) {
    if(PRINT_DEBUG_INFO) {
        printf("\n\033[1;36m --> Going through line: %d file: %s\033[0m\n", line, file);
        printf("\033[1;36m --> Done D2 task { \033[1;39m%s\033[1;39m } \033[0m\n\n", message);
    }
}

void check_error_d2(int res, char *msg, int line, char* file) {
    if(res == -1) {
        print_error_line_d2(line, file, msg);
    }
};



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
    if( client ) {
        d1_delete(client->peer);
        free(client);
        print_line_d2(__LINE__, __FILE__, "Deleted D2 client");
    }
    return NULL;
}

int d2_send_request( D2Client* client, uint32_t id ) {
    
    if( id <= 1000) {
        check_error_d2(-1, "D2 student ID is too low and not accepted", __LINE__, __FILE__);
        return -1;
    }

    // Use calloc to initialise value.
    PacketRequest* pack = (PacketRequest*)calloc(1, sizeof(PacketRequest));
    if( !pack ) {
        check_error_d2(-1, "Failed to allocate memory for PacketRequest", __LINE__, __FILE__);
        return -1;
    }


    pack->id = htonl(id);
    pack->type = htons(TYPE_REQUEST);

    int wc = d1_send_data(client->peer, pack, sizeof(PacketRequest));
    if( wc <= 0 ) {
        free(pack);
        check_error_d2(-1, "Failed to send data", __LINE__, __FILE__);
        return -1;
    }

    free(pack);
    print_line_d2(__LINE__, __FILE__, "Sent request to server");
    return wc;
}

int d2_recv_response_size( D2Client* client )
{
    char buffer[sizeof(PacketResponseSize)];
    int wc = d1_recv_data(client->peer, buffer, sizeof(PacketResponseSize));

    if( wc <= 0 ) {
        check_error_d2(-1, "Failed to receive size data", __LINE__, __FILE__);
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
    // Subtract the size of D1Header from the total size to account for the packet header.
    int wc = d1_recv_data(client->peer, buffer, sz - sizeof(D1Header));
    if( wc <= 0 ) {
        check_error_d2(-1, "Failed to receive data", __LINE__, __FILE__);
        return -1;
    }

    // Check the packet type to ensure it is correct.
    PacketResponse* packCheck = (PacketResponse*)buffer;

    if( ntohs(packCheck->type) != TYPE_RESPONSE && ntohs(packCheck->type) != TYPE_LAST_RESPONSE ){
        check_error_d2(-1, "Received wrong packet type", __LINE__, __FILE__);
        return -1;
    }

    print_line_d2(__LINE__, __FILE__, "Received response from server (d2_recv_response)");
    return wc;
}

LocalTreeStore* d2_alloc_local_tree( int num_nodes ) {

    // Again using calloc to initialize value at adress -> avoid warnings. 
    LocalTreeStore* nodes = (LocalTreeStore*)calloc(1, sizeof(LocalTreeStore));
    if( !nodes ) {
        check_error_d2(-1, "Failed to allocate memory for LocalTreeStore", __LINE__, __FILE__);
        return NULL;
    }

    nodes->number_of_nodes = num_nodes;
    nodes->root = calloc(num_nodes, sizeof(NetNode));

    if( !nodes->root ) {
        free(nodes);
        check_error_d2(-1, "Failed to allocate memory for NetNode", __LINE__, __FILE__);
        return NULL;
    }

    return nodes;
}

void  d2_free_local_tree( LocalTreeStore* nodes )
{
    if( nodes ) {
        if(nodes->root) {
            free(nodes->root);
        }
        free(nodes);
        print_line_d2(__LINE__, __FILE__, "Freed local tree");
    }
}

int d2_add_to_local_tree(LocalTreeStore* nodes_out, int node_idx, char* buffer, int buflen) {

    // Well well, this is a bit tricky, but eveything is checked i guess..
    if ( !nodes_out || !buffer || node_idx < 0 || node_idx >= nodes_out->number_of_nodes || buflen < 0) {
        fprintf(stderr, "Invalid input parameters.\n");
        return -1;
    }

    // Removing exactly 5 times 4 bytes, because "unused children fields are not sent over thenetwork"
    size_t net_node_base_size = sizeof(NetNode) - sizeof(uint32_t) * 5;

    
    while (buflen >= net_node_base_size) {

        NetNode node;
        memcpy(&node, buffer, net_node_base_size); 

        node.id = ntohl(node.id);
        node.value = ntohl(node.value);
        node.num_children = ntohl(node.num_children);

        buffer += net_node_base_size;
        buflen -= net_node_base_size;

        if (buflen < sizeof(uint32_t) * node.num_children) {
            fprintf(stderr, "Not enough data for children IDs.\n");
            return -1;
        }

        //this handles the children, after the "header" of NetNode is set above. 
        for (uint32_t i = 0; i < node.num_children; ++i) {
            memcpy(&node.child_id[i], buffer, sizeof(uint32_t));
            node.child_id[i] = ntohl(node.child_id[i]);

            // unit32_t is the size of children id given in slides, so for each slize ad 8 bytes to buffer adress.
            buffer += sizeof(uint32_t);
            buflen -= sizeof(uint32_t);
        }
        nodes_out->root[node_idx++] = node; // Store the node
    }

    return node_idx; 
}


void display_node(LocalTreeStore *store, int index, int level) {
    for (int j = 0; j < level; j++) {

        printf("--"); // Indent based on the level of depth
    }

    //Print the Node inside the tree
    NetNode *current = &store->root[index];
    printf("id: %d, value: %d, children: %d\n", current->id, current->value, current->num_children);

    // Gotta love recursion, do the same for children
    for (int i = 0; i < current->num_children; i++) {
        display_node(store, current->child_id[i], level + 1);
    }
}

void d2_print_tree(LocalTreeStore *store) {
    if (store != NULL && store->root != NULL) {
        display_node(store, 0, 0); // Start from the root with depth 0
    } else {
        printf("Empty or uninitialized tree.\n");
    }
}
