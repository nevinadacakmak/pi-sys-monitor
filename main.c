#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "metrics.h"
#include "socket_client.h"

int main(){

    /* steps:
    run socket_server.c outside
    call metrics
    send with socket_client
    repeat
    */

    char metrics[4096];

    //calculate_metrics(samp,delay);
    calculate_metrics(10,100000, metrics, sizeof(metrics));

    //socket_client();
    socket_client(metrics);

    return 0;
}