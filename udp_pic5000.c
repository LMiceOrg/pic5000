#include <pcap.h> /* libpcap */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>

#include <pthread.h>    /* libpthread */

#include "udp_pic5000.h"
#include "raw_pic5000.h"

struct timeval packageStartTime[NETCAP_CHANNEL_COUNT];
struct timeval packageEndTime[NETCAP_CHANNEL_COUNT];

int packageCounter[NETCAP_CHANNEL_COUNT];
char* pcapBuff[NETCAP_CHANNEL_COUNT];
volatile int pcapState[NETCAP_CHANNEL_COUNT];
pcap_t *pcapHandle[NETCAP_CHANNEL_COUNT] ={0,0,0,0};
unsigned char* pcapTag[NETCAP_CHANNEL_COUNT];

/** point to tiff Buffer */
char* tifBuff[NETCAP_CHANNEL_COUNT];

/** point to raw picture buffer */
char* rawBuff[NETCAP_CHANNEL_COUNT];

/** point to sensor buffer */
char* senBuff[NETCAP_CHANNEL_COUNT];

/** point to sensor data buffer */
char* senData[NETCAP_CHANNEL_COUNT];

pthread_t netcap_pthread_t[NETCAP_CHANNEL_COUNT];

struct thread_arg_s
{
    int channel;
    char devname[256];
};

struct thread_arg_s netcap_thread_arg[NETCAP_CHANNEL_COUNT];

int bdebugprint;

int get_netdev_list(char** devlist, int* size)
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i=0;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct netdev_i
    {
        char name[128];
        char desc[128];
    };

    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
        return 1;
    }
    fprintf(stderr, "fild pcap_devs \n");
    /* Print the list */

    for(d=alldevs,i=0; d; d=d->next,++i);

    *size = i;

    *devlist = malloc(i* sizeof(struct netdev_i) );
    memset(*devlist, 0, i* sizeof(struct netdev_i));


    for(d=alldevs,i=0; d; d=d->next,++i)
    {
        struct netdev_i* dev = (struct netdev_i*)(*devlist) + i;
        memcpy(dev->name, d->name, strlen(d->name)>127?127: strlen(d->name) );
        //        printf("%d. %s", i, d->name);
        if (d->description)
        {
            memcpy(dev->desc, d->description, strlen(d->description)>127?127: strlen(d->description) );
            //            printf(" (%s)\n", d->description);
        }
        // else
        //            printf(" (No description available)\n");

    }

    pcap_freealldevs(alldevs);
    return 0;
}

int free_netdev_list(char** devlist)
{
    free(*devlist);
    *devlist = NULL;
    return 0;
}


/**
 * @brief netcap_callback
 * 8304x6220x2 =103301760 14bit-color
 * header 42 1392+6 =1440
 * @param arg
 * @param pkthdr
 * @param packet
 */
void netcap_callback(u_char *arg, const struct pcap_pkthdr* pkthdr,const u_char* packet)
{

    const u_char* p = packet+42;
    int channel = *(int*)arg;
    unsigned int pcount = 0;

    (void)pkthdr;


    if(channel > NETCAP_CHANNEL_COUNT)
        return;

    packageCounter[channel] ++;

    /**packet count
       modified by lizhimin
       pcount = (*(p+2)<<16) +(*(p+5)<<8) +(*(p+4)) - 1;
    */
    pcount = (*(p+2)<<16) +(*(p+5)<<8) +(*(p+4));
    pcapTag[channel][pcount] = 1;

    if(bdebugprint)
    {
        printf("%d\n", pcount);
    }

    if(pcount >= NETCAP_PACKAGE_COUNT)
    {
        printf("pcount too big %d\n", pcount);
        return;
    }

    /* memcpy */
    memcpy(rawBuff[channel] + pcount*NETCAP_PACKAGE_DATA_LEN,
           packet+NETCAP_PACKAGE_HEAD_LEN,
           NETCAP_PACKAGE_DATA_LEN);

    /* cap status */
    if(pcount == NETCAP_PACKAGE_COUNT -1)
    {
        pcapState[channel] = 1;
    }

#if 0
    struct timezone tz;
    if(pcount == 0)
    {
        gettimeofday(& packageStartTime[channel], &tz);
    }
    else if(pcount == NETCAP_PACKAGE_COUNT -1)
    {
        gettimeofday(& packageEndTime[channel], &tz);
    }
#endif


}


int netcap_thread(int channel, const char *devname)
{
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    u_int netmask =0xffffff;
    //"ip and udp";//setup the package type
    char packet_filter[] = "udp and port 6000";
    struct bpf_program fcode;

    /* check handle running */
    if(pcapHandle[channel] != NULL)
    {
        return -1;
    }

    /* Open the adapter */
    if ( (adhandle= pcap_open_live(devname, // name of the device
                                   65536, // portion of the packet to capture.
                                   // 65536 grants that the whole packet will be captured on all the MACs.
                                   1, // promiscuous mode
                                   1000, // read timeout
                                   errbuf // error buffer
                                   ) ) == NULL)
    {
        printf("\nUnable to open the adapter. %s is not supported by WinPcap\n", devname);
        return -1;
    } else {

        printf("pcap_open_live done\n");
    }
    //set buffer size
    pcap_set_buffer_size(adhandle, 1024*1024*128);

    //    printf("do compile\n");
    //compile the filter
    if(pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) <0 ){
        printf("\nUnable to compile the packet filter. Check the syntax.\n");
        return -1;
    }
    //set the filter
    if(pcap_setfilter(adhandle, &fcode)<0){
        printf("\nError setting the filter.\n");
        return -1;
    }

    pcapHandle[channel] = adhandle;

    /* start the capture */
    printf("pcap %d startting...\n", channel);
    pcap_loop(adhandle, -1, netcap_callback, (u_char*)&channel);
    printf("pcap %d stopped.\n", channel);
    return 0;
}

int netcap_stop(int channel)
{
    if(pcapHandle[channel] != NULL)
    {
        pcap_breakloop(pcapHandle[channel]);
        pcapHandle[channel] = NULL;
    }
    return 0;
}

/*
Header
*/
#define ETHER_ADDR_LEN 6
//from linux's ethernet.h
#define ETHERTYPE_PUP           0x0200
#define ETHERTYPE_SPRITE        0x0500
#define ETHERTYPE_IP            0x0800
#define ETHERTYPE_ARP           0x0806
#define ETHERTYPE_REVARP        0x8035
#define ETHERTYPE_AT            0x809B
#define ETHERTYPE_AARP          0x80F3
#define ETHERTYPE_VLAN          0x8100
#define ETHERTYPE_IPX           0x8137
#define ETHERTYPE_IPV6          0x86dd
#define ETHERTYPE_LOOPBACK      0x9000

/*14 bytes ehter header*/
struct   ether_header{
u_char   ether_dhost[ETHER_ADDR_LEN];
u_char   ether_shost[ETHER_ADDR_LEN];
u_short   ether_type;  //如果上一层为IP协议。则ether_type的值就是0x0800
};
typedef struct ether_header ether_header;

/* 4 bytes IP address */
typedef struct ip_address{
u_char byte1;
u_char byte2;
u_char byte3;
u_char byte4;
}ip_address;
/* IPv4 header */
typedef struct ip_header{
u_char         ihl_ver;  //ip   header   length and version
u_char tos; // Type of service
u_short tot_len; // Total length
u_short id; // Identification
u_short frag_off; // Flags (3 bits) + Fragment offset (13 bits)
u_char ttl; // Time to live
u_char protocol; // Protocol
u_short check; // Header checksum
/*ip_address*/int saddr; // Source address
/*ip_address*/int daddr; // Destination address
u_int op_pad; // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header{
u_short sport; // Source port                    2 bytes
u_short dport; // Destination port
u_short len; // Datagram length
u_short checkl; // Checksum
}udp_header;

/*UDP data*/
typedef struct udp_data{
u_short data[696];
}udp_data;

/*UDP data*/
typedef struct udp_id{
    u_char id2;
    u_char id3;
    u_char id0;
    u_char id1;
}udp_id;
/*伪UDP首部*/
struct Psd_Header {
        uint32_t sourceip; //源IP地址
        uint32_t destip; //目的IP地址
        uint8_t mbz; //置空(0)
        uint8_t ptcl; //协议类型
        uint16_t plen; //TCP/UDP数据包的长度(即从TCP/UDP报头算起到数据包结束的长度 单位:字节)
};

static int netcap_send_command(int channel, const char* command, int cmdsize)
{
    //200x4+4+4+48
    char buffer[856]={0};
    ether_header* pether_header =   (ether_header*)buffer;
    ip_header* pip_herder            =       (ip_header*)(buffer + sizeof(ether_header));
    udp_header* pudp_herder            =       (udp_header*)(buffer + sizeof(ether_header) + sizeof(ip_header));
    /*Des MAC addr e8-4e-06-04-04-0a*/
    pether_header->ether_dhost[1] = (char)0x0a;               //0x0a
    pether_header->ether_dhost[0] = (char)0x04;               //0x04
    pether_header->ether_dhost[3] = (char)0x04;               //0x04
    pether_header->ether_dhost[2] = (char)0x06;               //0x06
    pether_header->ether_dhost[5] = (char)0x4e;               //0x4e
    pether_header->ether_dhost[4] = (char)0xe8;               //0xe8
    /*Soc MAC addr e8-4e-06-04-04-0a*/
    pether_header->ether_shost[0] = (char)0x0b;               //0x0b
    pether_header->ether_shost[1] = (char)0x04;               //0x04
    pether_header->ether_shost[2] = (char)0x04;               //0x04
    pether_header->ether_shost[3] = (char)0x06;               //0x06
    pether_header->ether_shost[4] = (char)0x4e;               //0x4e
    pether_header->ether_shost[5] = (char)0xe8;               //0xe8


    pether_header->ether_type = htons(ETHERTYPE_IP);//or set 0x0800

    //构建IP数据头
    if((sizeof(ip_header) % 4) != 0)
    {
            printf("[IP Header error]/n");
            return -1;
    }

    pip_herder->ihl_ver = sizeof(ip_header) / 4;
    pip_herder->ihl_ver += 4<<4;
    pip_herder->tos = 0;
    pip_herder->tot_len = htons(sizeof(buffer) - sizeof(ether_header));
    pip_herder->id = htons(0x1000);
    pip_herder->frag_off = htons(0);
    pip_herder->ttl = 0x80;
    //#define IPPROTO_UDP 17 /* user datagram protocol */
    pip_herder->protocol = 17;
    pip_herder->check = 0;
    pip_herder->saddr = inet_addr("192.168.18.*");
    pip_herder->daddr = inet_addr("122.*.*.*");
    pip_herder->check  = 0;//in_cksum((u_int16_t*)pip_herder, sizeof(ip_header));

    //构建UDP数据头;
    pudp_herder->dport = htons(7865);
    pudp_herder->sport = htons(2834);
    pudp_herder->len = htons(sizeof(buffer) - sizeof(ether_header) - sizeof(ip_header));
    pudp_herder->checkl = 0;

    //add udp data
    char * cmd=buffer+sizeof(ether_header)+sizeof(ip_header)+sizeof(udp_header)+2;
    memcpy(cmd, command, cmdsize);



    //构造伪UDP首部

    //pudp_herder->checkl  = in_cksum((u_int16_t*)pudp_herder, 24);

    //char buffer2[64] = { 0 };
    //Psd_Header* psd = (Psd_Header*)buffer2;
    //psd->sourceip = inet_addr("192.168.18.*");
   // psd->destip = inet_addr("122.*.*.*");
    //psd->ptcl = IPPROTO_UDP;
   // psd->plen =  htons(sizeof(buffer) - sizeof(ether_header) - sizeof(ip_header));
   // psd->mbz = 0;

    //memcpy(buffer2 + sizeof(Psd_Header), (void*)pudp_herder, sizeof(buffer) - sizeof(ether_header) - sizeof(ip_header));
    //pudp_herder->checkl  = in_cksum((u_int16_t *)buffer2, sizeof(buffer) - sizeof(ether_header) - sizeof(ip_header) + sizeof(Psd_Header));
    if(pcap_sendpacket(pcapHandle[channel], (const u_char*)buffer, 856) == -1)
    {
        printf("[pcap_sendpacket error]/n");
        return -1;
    }

    return 0;
}

int netcap_snap(int channel, int time)
{

    int c=0;
    char cmd[8]={0};

    bdebugprint = 0;

    //reset tag
    memset(pcapTag[channel],0, 74211);

    cmd[3] = (char)c;//0x0=Snap, 0x1=reqPkt
    *((int*)cmd +1) = time;

    if(pcapHandle[channel] == NULL)
        return -1;

    return netcap_send_command(channel, cmd, 8);
}

int netcap_retrans(int channel, int *pcnt)
{
    int c=1;
    int count;
    char cmd[200*4+4+4];
    int *pint;
    count = 0;
    int i;
    bdebugprint = 1;

    cmd[3] = (char)c;//0x0=Snap, 0x1=reqPkt
    pint = (int*)cmd + 1;
    for(i=0; i<74211;++i)
    {
        if(pcapTag[channel][i] == 0)
        {

            *((unsigned char*)pint+1) = (unsigned char)(i*174);
            *((unsigned char*)pint+2) = (unsigned char)(i*174)>>8;
            *((unsigned char*)pint+3) = (unsigned char)(i*174)>>16;

            if(count == 199)
                break;

            pint ++;
            count ++;
        }
    }
    *(cmd+6) = (unsigned char)(count>>8) ;
    *(cmd+7) = (unsigned char)count;

    *pcnt = count;

    if(count == 0)
        return 0;

    if(pcapHandle[channel] == NULL)
        return -1;
    return netcap_send_command(channel, cmd, 808);
}

int64_t difftimeval(const struct timeval tv1, const struct timeval tv2)
{
    int64_t difft = tv2.tv_sec*1000000.0 + tv2.tv_usec*1.0 - tv1.tv_sec*1000000.0 - tv1.tv_usec*1.0;
    return difft;
}


int netcap_init(int channel_count)
{
    const unsigned char tiff_head[] = {
        0x49, 0x49, 0x2a, 0x00,
        0x88, 0x42, 0x28, 0x06
    };
    const unsigned char tiff_tail[] = {
        0x0b, 0x00,
        0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x70, 0x20, 0x00, 0x00,
        0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4c, 0x18, 0x00, 0x00,
        0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x03, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x0a, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x11, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x16, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4c, 0x18, 0x00, 0x00,
        0x17, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x80, 0x42, 0x28, 0x06,
        0x1c, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    for(int i=0; i<channel_count; ++i)
    {
        packageCounter[i] = 0;
        pcapState[i] = 0;
        pcapTag[i] = malloc(NETCAP_PACKAGE_COUNT);
        memset(pcapTag[i], 0, NETCAP_PACKAGE_COUNT);

        // pcap memory buffer
        pcapBuff[i] = malloc(TIFF_FILE_SIZE);
        memset(pcapBuff[i], 0, TIFF_FILE_SIZE);
        tifBuff[i] = pcapBuff[i];
        rawBuff[i] = pcapBuff[i]+ TIFF_HEAD_LEN;
        senBuff[i] = pcapBuff[i] + RAW_PIC_SIZE + TIFF_SIZE;
        senData[i] = senBuff[i]+7;

        // 8 tiff-header 138 tiff-tail
        memcpy(pcapBuff[i], tiff_head, TIFF_HEAD_LEN);
        memcpy(pcapBuff[i]+TIFF_HEAD_LEN + RAW_PIC_SIZE, tiff_tail, TIFF_TAIL_LEN);

        // 3 sensor header, sensor data len
        memcpy(senBuff[i], SENSOR_TAG, 3);
        *(int*)(senBuff[i]+3) = SENSOR_LEN;


        start_netcap_thread(i);
    }

    return 0;
}



static void* netcap_pthread(void* data)
{
    struct thread_arg_s arg;
    memcpy(&arg, data, sizeof(struct thread_arg_s));
    printf("a.channel = %d \t a.devname = %s\n", arg.channel, arg.devname);

    netcap_thread(arg.channel, arg.devname);

    return 0;
}

void start_netcap_thread(int channel)
{


    struct thread_arg_s* arg = & netcap_thread_arg[channel];
    memset(arg, 0, sizeof(struct thread_arg_s));
    arg->channel = channel;
    sprintf(arg->devname, "en5");
    int ret;
    ret = pthread_create(&netcap_pthread_t[channel], NULL, netcap_pthread, arg);

}


int netcap_finit(int channel_count)
{
    int i;

    //stop thread
    for(i=0; i<channel_count; ++i)
    {
        netcap_stop(i);
        pthread_join(netcap_pthread_t[i], NULL);
    }

    //clear memory buff
    for(i=0; i< channel_count; ++i)
    {
        free(pcapBuff[i]);
    }

    return 0;
}
