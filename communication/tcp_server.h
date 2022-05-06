#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "decodageTC.h"
#include "encodageTM.h"
#include "struct_allocation.h"

/**
 * @brief Create a tcp socket 
 * 
 * @return short 
 */
short SocketCreate(void);

/**
 * @brief Bind an existing socket to a port and accepting any incoming addr
 * 
 * @param hSocket ref to the socket
 * @return int 
 */
int BindCreatedSocket(int hSocket);