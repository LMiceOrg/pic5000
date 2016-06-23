#include "control_config.h"
#include "mtig_host.h"

#include <jansson.h>
#include <string.h>

#include <pthread.h>

pthread_t capcontrol_thread_t;
control_config_t cap_config;
volatile int thread_state;

#define json_object_get_number(root, obj, name, value) do {   \
    obj = json_object_get(root, name);  \
    value=0;   \
    if( json_is_number(obj) ) {   \
    value = json_number_value(obj); \
    }   \
    }while(0)

#define json_object_get_integer(root, obj, name, value) do {   \
    obj = json_object_get(root, name);  \
    value=0;   \
    if( json_is_integer(obj) ) {   \
    value = json_integer_value(obj); \
    }   \
    }while(0)

#define json_object_get_string(root, obj, name, value, len) do {   \
    obj = json_object_get(root, name);  \
    memset(value, 0, len); \
    if( json_is_string(obj) ) {   \
    strncpy(value,json_string_value(obj), len -1);\
    }   \
    }while(0)

/*
{
    "time_period": 6000,
    "pic_path:  "/media/data",
    "areas":[
        {
            "lt":{"x":40,"y":12,"z":0},
            "rb":{"x":41,"y":13,"z":0}
        },
        {
            "lt":{"x":38,"y":12,"z":0},
            "rb":{"x":39,"y":12.5,"z":0}
        }
    ]
}

*/
int load_config(const char* file, control_config_t *cfg)
{
    json_error_t err;
    json_t *root;
    json_t *obj;
    json_t *area;
    json_t *jarray;
    json_t *point3;
    int i;


    root = json_load_file(file, 0, &err);
    if(!root)
    {
        return -1;
    }

    memset(cfg, 0, sizeof(control_config_t));

    json_object_get_integer(root, obj, "channel_count", cfg->channel_count);
    jarray = json_object_get(root, "net_device");
    if(json_is_array(jarray))
    {
        json_array_foreach(jarray, i, obj)
        {
            if(i >= sizeof(cfg->net_device)/sizeof(struct net_device_s) )
                break;
            if(json_is_string(obj))
            {
                strncpy(cfg->net_device[i].name, json_string_value(obj), sizeof(cfg->net_device[i].name) -1 );
            }
        }
    }

    json_object_get_integer(root, obj, "time_period", cfg->time_period);

    json_object_get_string(root, obj, "pic_path", cfg->pic_path, sizeof(cfg->pic_path));

    jarray = json_object_get(root, "areas");
    if(json_is_array(jarray))
    {
        cfg->area_count = json_array_size(jarray);
        json_array_foreach(jarray, i, area)
        {
            point3 = json_object_get(area, "lt");
            if(point3)
            {
                json_object_get_number(point3, obj, "x", cfg->area[i].lt.x);
                json_object_get_number(point3, obj, "y", cfg->area[i].lt.y);
                json_object_get_number(point3, obj, "z", cfg->area[i].lt.z);
            }

            point3 = json_object_get(area, "rb");
            if(point3)
            {
                json_object_get_number(point3, obj, "x", cfg->area[i].rb.x);
                json_object_get_number(point3, obj, "y", cfg->area[i].rb.y);
                json_object_get_number(point3, obj, "z", cfg->area[i].rb.z);
            }
        }
    }

    json_decref(root);

    return 0;
}

static double adj_latitude(double* x)
{
    if(*x<0)
        *x = 360 + *x;
    return *x;
}
static double adj_longitude(double* y)
{
//    if(y<0)
//        y=180+y;
    return *y;
}

void* capcontrol_pthread(void* arg)
{
    control_config_t* cfg = (control_config_t*)arg;
    int i;
    int pos_status = 0;
    mtig_data_t mt_data;

    if(cfg->area_count <= 0)
        return NULL;

    /** normalize config position */
    for(i=0; i<cfg->area_count; ++i)
    {
            adj_latitude(&cfg->area[i].lt.x);
            adj_longitude(&cfg->area[i].lt.y);
            adj_latitude(&cfg->area[i].rb.x);
            adj_longitude(&cfg->area[i].rb.y);
    }

    /** fetch mtig data and normalize */
//    mtig_read_data(&mt_data);
//    adj_latitude(&mt_data.Lat);
//    adj_longitude(&mt_data.Lon);


    /** check position status */
    for(i=0; i<cfg->area_count; ++i)
    {
        if( mt_data.Lat > cfg->area[i].lt.x && mt_data.Lon < cfg->area[i].rb.x
                && mt_data.Lon < cfg->area[i].lt.y && mt_data.Lon > cfg->area[i].rb.y )
        {
            pos_status = 1;
            break;
        }
    }
    return NULL;
}

void control_thread_start()
{
    if(thread_state == 0)
    {
        load_config("cfg.json", &cap_config);
        thread_state = 1;
        pthread_create(&capcontrol_thread_t, NULL, capcontrol_pthread, &cap_config);
    }
}

void control_thread_stop()
{
    if(thread_state == 1)
    {
        pthread_join(capcontrol_thread_t, NULL);
    }
    thread_state = 0;
}
