#ifndef TRANFERTORGB_H
#define TRANFERTORGB_H

#define CAMERA_PICTURE_WIDTH    8304L
#define CAMERA_PICTURE_HEIGHT   6220L
#define CAMERA_GRAY_BITS        14L
#define CAMERA_PIXEL_BYTES      2L
#define CAMERA_PICTURE_BYTES    ( CAMERA_PICTURE_WIDTH * CAMERA_PICTURE_HEIGHT*CAMERA_PIXEL_BYTES )

#define CONV_PICTURE_WIDTH      8176
#define CONV_PICTURE_HEIGHT     6132
#define CONV_PICTURE_BITS       8   /*8 RGB888 */
#define CONV_PIXEL_BYTES        3   /*3 RGB888    */
#define CONV_PICTURE_BYTES  (CONV_PICTURE_WIDTH*CONV_PICTURE_HEIGHT*CONV_PIXEL_BYTES)
#define CONV_PICTURE_LEFT_POS   (1+10+4+1+4+28+16)
#define CONV_PICTURE_TOP_POS    (29+16)
#if CONV_PICTURE_BITS == 5
#define BMPTYPE struct bmp555_t
#define QIMG_FORMAT QImage::Format_RGB555
#else
#define BMPTYPE struct bmp888_t
#define QIMG_FORMAT QImage::Format_RGB888
#endif

#define TIFF_BYTES_COUNT    (8+138)

#define CONV_PIXEL_TYPE unsigned short

#include <stdint.h>

#pragma pack(1)
struct bmp888_t
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
struct bmp555_t
{

    unsigned blue:5;
    unsigned green:5;
    unsigned red:5;
    unsigned reserved:1;
};

struct bmpany_t
{
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};

#pragma pack()

void pc_main(int index);

void tiff_to_bmp(const char* filename);
void raw_to_bmp(const char* filename);

#endif // TRANFERTORGB_H
