#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NetLayer.h"

int main(char ** argv, int argc) {
	NetLayer net_layer = NetLayer(false, 7777, 40);//20);
	char * buffer = "oi tudo bem, essa mensagem eh um teste!!!";
	net_layer.SendDataTo(buffer, strlen(buffer), "1.2.3.4", 9999);
	return 0;
}