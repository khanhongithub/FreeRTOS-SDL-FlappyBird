#include "multiplayer_config.h"
#include "multiplayer.h"
// determine whether in host or in client mode#

// obstacle field
// player position
// whether player is alive, can also be merged with the above one
// gap and collision counter

void HostSendData(host_package_t *data)
{
    /*if (aIOSocketPut(UDP, IPv4_addr, MOSI_PORT, (char *)data, sizeof(struct host_package))) {
            PRINT_ERROR("Failed to send struct data");
            return;
    }*/
}


void MultiplayerEnter(void) 
{

}

void MultiplayerRun(void)
{

}

void MultiplayerExit(void)
{

}

void vMultiplayerTask(void *pvArgs)
{
    // host or client <- retrive via struct

    // connect to client <- retrive via struct ip, port, etc

    // have both players started the game -> fix in main game loop

    // inject received data into local gameloop

    // grab relevant data to send and send it

    // check last time the data was received and establish wheter connection has been lost

    // loop and wait for reconnection / give player option to quit
        // if reconnect successfull continue playing
    
    // if both players are dead prompt gameover screen

    while (1)
    {
        /* code */
    }
    
}
