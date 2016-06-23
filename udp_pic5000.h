#ifndef UDP_PIC5000_H
#define UDP_PIC5000_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdint.h>
#define NETCAP_PACKAGE_COUNT    74211
#define NETCAP_PACKAGE_SIZE     1440
#define NETCAP_PACKAGE_DATA_LEN 1392
#define NETCAP_PACKAGE_HEAD_LEN 48
#define NETCAP_CHANNEL_COUNT    4



#if defined(__linux__)
extern struct timeval packageStartTime[NETCAP_CHANNEL_COUNT];
extern struct timeval packageEndTime[NETCAP_CHANNEL_COUNT];
#elif defined(_WIN32)
#include <Windows.h>
extern DWORD packageStartTime[NETCAP_CHANNEL_COUNT];
extern DWORD packageEndTime[NETCAP_CHANNEL_COUNT];

#else
#error("No implementation!")
#endif


extern int packageCounter[NETCAP_CHANNEL_COUNT];
extern char* pcapBuff[NETCAP_CHANNEL_COUNT];
extern volatile int pcapState[NETCAP_CHANNEL_COUNT];
extern unsigned char* pcapTag[NETCAP_CHANNEL_COUNT];

extern char* tifBuff[NETCAP_CHANNEL_COUNT];
extern char* rawBuff[NETCAP_CHANNEL_COUNT];
extern char* senBuff[NETCAP_CHANNEL_COUNT];
extern char* senData[NETCAP_CHANNEL_COUNT];

int netcap_init(int channel_count);
int netcap_finit(int channel_count);

int netcap_thread(int channel, const char* devname);
void start_netcap_thread(int channel);

int netcap_stop(int channel);
int get_netdev_list(char** devlist, int* size);
int free_netdev_list(char** devlist);

int netcap_snap(int channel, int time);
int netcap_retrans(int channel, int *pcnt);

int64_t difftimeval(const struct timeval tv1, const struct timeval tv2);


#ifdef __cplusplus
}
#endif

#endif // UDP_PIC5000_H
