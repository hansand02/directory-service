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

D1Peer* d1_create_client( )
{
    /* implement this */
    return NULL;
}

D1Peer* d1_delete( D1Peer* peer )
{
    /* implement this */
    return NULL;
}

int d1_get_peer_info( struct D1Peer* peer, const char* peername, uint16_t server_port )
{
    /* implement this */
    return 0;
}

int d1_recv_data( struct D1Peer* peer, char* buffer, size_t sz )
{
    /* implement this */
    return 0;
}

int d1_wait_ack( D1Peer* peer, char* buffer, size_t sz )
{
    /* This is meant as a helper function for d1_send_data.
     * When D1 data has send a packet, this one should wait for the suitable ACK.
     * If the arriving ACK is wrong, it resends the packet and waits again.
     *
     * Implementation is optional.
     */
    return 0;
}

int d1_send_data( D1Peer* peer, char* buffer, size_t sz )
{
    /* implement this */
    return 0;
}

void d1_send_ack( struct D1Peer* peer, int seqno )
{
    /* implement this */
}

