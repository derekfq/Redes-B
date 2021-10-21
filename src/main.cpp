#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NetLayer.h"

int main(char ** argv, int argc) {
	NetLayer net_layer = NetLayer(false, 7777, 2);//20);
	char * buffer = "abcde";//"AbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMnAbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMn";
	net_layer.SendDataTo(buffer, strlen(buffer), "192.168.15.11", 9999);

	NetLayer net_layer2 = NetLayer(false, 9999, 64);//20);
	char * buffer2 = "AbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMnOpQrStUvXyZAbCdEfGhIjKlMn";
	net_layer2.SendDataTo(buffer2, strlen(buffer2), "192.168.15.11", 7777);

	return 0;
}