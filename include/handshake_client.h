/* 
conduct the handshake between host and client
*/

typedef struct handshake {
    int msg;
} handshake_t;


void vHandshakeTaskClient(void *pvParameter);
void HandshakeTaskEnter(void);
void HandshakeTaskStop(void);