#include <sys/ioctl.h>
#include <net/if.h>

int get_mtu(int fd, const char *device) {
struct ifreq ifr;

if (!device)
    return 65535;

memset(&ifr, 0, sizeof(ifr));
strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
    perror("Error opening:");
    exit(-1);
}

return ifr.ifr_mtu;
}

int main(char ** argv, int argc) {
    printf("%d\n", get_mtu(0,"/sys/class/net/enp3s0/mtu"));
    return 0;
}