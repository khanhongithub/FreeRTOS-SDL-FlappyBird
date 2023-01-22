/* 
conduct the handshake between host and client
*/

typedef struct handshake {
    int msg;
} handshake_t;


void vHandshakeTaskHost(void *pvParameter);
void HandshakeTaskEnter(void);
void HandshakeTaskStop(void);