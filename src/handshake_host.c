
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Font.h"
#include "TUM_Utils.h"
#include "TUM_Print.h"
#include "TUM_Font.h"

#include "AsyncIO.h"

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "multiplayer_config.h"
#include "gui.h"
#include "resources.h"
#include "handshake_host.h"


#define MOSI_PORT 1234
#define MISO_PORT 4321

#define IPv4_addr "127.0.0.1"

TaskHandle_t HandshakeTask = NULL;

aIO_handle_t handshakehost_handle;



void HandshakeTaskEnter(void)
{  

    if(xTaskCreate(vHandshakeTaskHost, "HandshakeTask", 
                   mainGENERIC_STACK_SIZE / 2, 
                   NULL, mainGENERIC_PRIORITY + 2, 
                   &HandshakeTask) != pdPASS) {
        DEBUG_PRINT("failed to send msg\n");
    }
}

void HandshakecallbackHost(size_t recv_size, char *buffer, void *args)
{

   handshake_t my_handshake_host = {.msg = 1};
   int client_msg = *((int *) buffer);
   printf("This is host , receive msg %d from client\n",client_msg);
   if(aIOSocketPut(UDP, IPv4_addr, MOSI_PORT, (char *)&(my_handshake_host.msg), sizeof(int)))
   PRINT_ERROR("Failed to send msg to host");

}
void HandshakeTaskInit(void)
{       
    printf("This is host\n");
    handshakehost_handle = aIOOpenUDPSocket(IPv4_addr, MOSI_PORT, sizeof(int), 
    HandshakecallbackHost, NULL);
}
void vHandshakeTaskHost(void *pvParameters)
{
    tumDrawBindThread();
    HandshakeTaskInit();
     
       if(handshakehost_handle == NULL){
        PRINT_ERROR(" UDP socket failed to open");
        exit(EXIT_FAILURE);
        }

    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);


        // Basic sleep of 1000 milliseconds
        vTaskDelay((TickType_t)1000);
        }
}

void HandshakeTaskStop(void)
{
    vTaskDelete(HandshakeTask);
}