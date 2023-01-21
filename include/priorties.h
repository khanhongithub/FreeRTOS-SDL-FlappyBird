/*
    defines a bunch priorities standart priorites
*/
#define STATEMACHINE_PRIORTY 8
#define STATEMACHINE_STACK_SIZE ((unsigned short) 2560)
 
#define highGENERIC_PRIORITY 7
#define highGENERIC_STACK_SIZE ((unsigned short) 2 * 5120)

#define mainGENERIC_PRIORITY 5
#define mainGENERIC_STACK_SIZE ((unsigned short) 2560)

#define lowGENERIC_PRIORITY 3
#define lowGENERIC_STACK_SIZE ((unsigned short) 1000)