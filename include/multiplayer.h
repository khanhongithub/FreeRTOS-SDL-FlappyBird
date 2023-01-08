typedef struct host_package {
    char type_of_msg;
    short int obstacle_field;
    short int gap_counter;
    short int collision_counter;

    short int player_position;

    short int player_texture_id;
    bool player_alive;    
}host_package_t;

typedef struct client_package {
    char type_of_msg;
    short int player_position;
    short int player_texture_id;
}client_package_t;

typedef struct mulitiplayer_data {
    short int obstacle_field;
    short int gap_counter;
    short int collsion_counter;

    short int player_position;
}mulitiplayer_data_t;

void HostSendData(host_package_t *data);
void ClientSendData(client_package_t *data);

void MultiplayerEnter(void);
void MultiplayerRun(void);
void MultiplayerExit(void);

extern 