#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

struct point3_s {
    /** 经纬高 */
    double x;
    double y;
    double z;
};

struct area_s {
    struct point3_s lt;
    struct point3_s rb;
};

struct net_device_s {
    char name[256];
};

struct control_config_s {
    /* net camera channel size*/
    int channel_count;
    struct net_device_s net_device[12];
    /*
     * 0: as soon as possible
     * n: wait n mil-second for next
     */
    int time_period;
    int area_count;
    struct area_s area[12];
    char pic_path[256];
};
typedef struct control_config_s control_config_t;

void control_thread_start();
void control_thread_stop();

#ifdef __cplusplus
}
#endif

#endif /** CONTROL_CONFIG_H */
