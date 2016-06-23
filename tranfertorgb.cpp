#include "tranfertorgb.h"

#include "udp_pic5000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <QImage>

int g_index = 0;


unsigned short rgb_24_2_565(int r, int g, int b)
{
    return (unsigned short)(((unsigned(r) << 8) & 0xF800) |
            ((unsigned(g) << 3) & 0x7E0)  |
            ((unsigned(b) >> 3)));
}

unsigned short rgb_24_2_555(int r, int g, int b)
{
    return (unsigned short)(((unsigned(r) << 7) & 0x7C00) |
            ((unsigned(g) << 2) & 0x3E0)  |
            ((unsigned(b) >> 3)));
}

inline unsigned int camera_to_conv_color(unsigned short s)
{
    //return pow(2, log2(s)*CONV_PICTURE_BITS/CAMERA_GRAY_BITS);
    return s *(1<<CONV_PICTURE_BITS)/(1<<CAMERA_GRAY_BITS);
}

inline CONV_PIXEL_TYPE getp(int j, int i)
{
    //    return camera_to_conv_color( *(unsigned short*)(rawBuff[0] + j*CAMERA_PICTURE_WIDTH*CAMERA_PIXEL_BYTES + i*CAMERA_PIXEL_BYTES) );
//    return (*(bit8buffer+j*CAMERA_PICTURE_WIDTH+i) ) & ((1<<CONV_PICTURE_BITS)-1);
    return *(CONV_PIXEL_TYPE *)(rawBuff[g_index] +(j*CAMERA_PICTURE_WIDTH+i)*CAMERA_PIXEL_BYTES);
}

/*      -   *   -
 *      -   0   -
 *      -   *   -
 */
inline CONV_PIXEL_TYPE getud(int j, int i)
{
    return (getp(j-1, i) +
            getp(j+1, i))/2 ;//& ((1<<CONV_PICTURE_BITS)-1);
}

/*      -   -   -
 *      *   0   *
 *      -   -   -
 */
inline CONV_PIXEL_TYPE getlr(int j, int i)
{
    return (getp(j,i-1) +
            getp(j, i+1))/2;// & ((1<<CONV_PICTURE_BITS)-1);
}

/*          *   -   *
 *          -   0   -
 *          *   -   *
 */
inline CONV_PIXEL_TYPE getqa(int j, int i)
{
    return (getp(j-1, i-1) +
            getp(j-1, i+1) +
            getp(j+1, i-1) +
            getp(j+1, i+1))/4;// & ((1<<CONV_PICTURE_BITS)-1) ;
}

/*          -   *   -
 *          *   0   *
 *          -   *   -
 */
inline CONV_PIXEL_TYPE getqd(int j, int i)
{
    return (getp(j-1, i) +
            getp(j+1, i) +
            getp(j, i-1) +
            getp(j, i+1))/4 ;//& ((1<<CONV_PICTURE_BITS)-1);
}


void pc_main(int index)
{
    g_index = index;
    // Define an image

//    FILE* fp = fopen("a.bin", "rb");
//    size_t sz = fread(rawBuff[0], 1, CAMERA_PICTURE_BYTES, fp);
//    if(sz != CAMERA_PICTURE_BYTES)
//    {
//        printf("sz is %ld (%ld) \n%ld\t%ld\t%ld\n", sz, CAMERA_PICTURE_BYTES,
//               CAMERA_PICTURE_WIDTH, CAMERA_PICTURE_HEIGHT, CAMERA_PIXEL_BYTES);
//        return;
//    }
//    fclose(fp);

//    //    printf("fp is %x, read %ld bytes\n", fp,  sz);

//   // bit8buffer = (unsigned char*)malloc(CAMERA_PICTURE_BYTES/CAMERA_PIXEL_BYTES);

//    unsigned short line[CAMERA_PICTURE_WIDTH];
//    for(int j=0; j<CAMERA_PICTURE_HEIGHT; ++j)
//    {
//        unsigned short *p = (unsigned short*)(rawBuff[0] + CAMERA_PICTURE_WIDTH*CAMERA_PIXEL_BYTES*j);
//        for(int i=0; i<CAMERA_PICTURE_WIDTH/2; ++i)
//        {
//            line[i] = (*(p+i*2+1) & ((1<<CAMERA_GRAY_BITS)-1) );
//            //line[i] *=4;
//            line[CAMERA_PICTURE_WIDTH-1-i] = (*(p+i*2) & ((1<<CAMERA_GRAY_BITS)-1) );
//            //line[8303-i] *=4;
//        }
//        memcpy((char*)p, (const char*)line, CAMERA_PICTURE_WIDTH*2);
//        // fill 8bit buffer
//        for(int i=0; i<CAMERA_PICTURE_WIDTH; ++i)
//        {
//            *(bit8buffer+j*CAMERA_PICTURE_WIDTH+i) = camera_to_conv_color( line[i] );
//        }
//    }
    for(int j=0; j<CAMERA_PICTURE_HEIGHT; ++j)
    {
        for(int i=0; i<CAMERA_PICTURE_WIDTH; ++i)
        {
            *(unsigned short*)(rawBuff[0] + (CAMERA_PICTURE_WIDTH*j+i)*CAMERA_PIXEL_BYTES )
                    /= 4;
        }
    }
    //Bayer pattern (R, GR, GB, B)
    /*
     * first line is GRG
     * first color is red
     */
    enum line_type {GRG_LINE = 0,
                    BGB_LINE};
    line_type ltype = GRG_LINE;
    bool GRG_start_with_green = true;


    printf("sizeof bmp type= %ld\n", sizeof(BMPTYPE));

    int posj =CONV_PICTURE_TOP_POS ;
    int posi = CONV_PICTURE_LEFT_POS ;
    int pos13, pos24;
    bmpany_t bmptemp;
    bmpany_t *p = &bmptemp;
    float green_factor =1;

    unsigned char *bmp565 = (unsigned char*)malloc(CONV_PICTURE_BYTES);

//    unsigned char* bmp555 = (unsigned char*)malloc(CONV_PICTURE_BYTES);
//    memset(bmp555, 0, CONV_PICTURE_BYTES);
    for(int j=posj; j<CONV_PICTURE_HEIGHT+posj; ++j)
    {
        for(int i=posi; i<CONV_PICTURE_WIDTH+posi; ++i)
        {
            int pos = ((j-posj)*CONV_PICTURE_WIDTH+(i-posi))*CONV_PIXEL_BYTES;
            //bmp888_t * p = (bmp888_t*)(bmp555+pos);
            //BMPTYPE* p = (BMPTYPE*)(bmp565+pos);
            switch(ltype)
            {
            case GRG_LINE:
                if(GRG_start_with_green)
                {
                    p->green = getp(j,i) *green_factor;
                    p->red = getlr(j, i);
                    p->blue = getud(j ,i);
                }
                else
                {
                    //  B       G       B   G
                    //  G       R(j,i)  G   R
                    //  B       G       B   G
                    p->red = getp(j,i);
                    p->blue = getqa(j, i);
                    pos13 = fabs( (int)getp(j-2, i) - (int)getp(j+2, i) );
                    pos24 = fabs((int)getp(j, i-2) - (int)getp(j, i+2));
                    if(pos13<pos24)
                        p->green = getud(j, i);
                    else if(pos13 > pos24)
                        p->green = getlr(j, i);
                    else
                        p->green = getqd(j, i) *green_factor;
                }
                break;
            case BGB_LINE:
                if(GRG_start_with_green)
                {
                    //  G   R   G   R
                    //  B   G   B   G
                    //  G   R   G   R
                    p->blue = getp(j, i);
                    p->red =  getqa(j, i);

                    pos13 = fabs( (int)getp(j-2, i) - (int)getp(j+2, i) );
                    pos24 = fabs((int)getp(j, i-2) - (int)getp(j, i+2));
                    if(pos13<pos24)
                        p->green = getud(j, i);
                    else if(pos13 > pos24)
                        p->green = getlr(j, i);
                    else
                        p->green = getqd(j, i) *green_factor;

                }
                else
                {
                    p->green = getp(j, i) *green_factor;
                    p->red = getud(j, i);
                    p->blue = getlr(j ,i);

                }
                break;
            } //end switch
            GRG_start_with_green = !GRG_start_with_green;

            pos = ((j-posj)*CONV_PICTURE_WIDTH+(i-posi))*CONV_PIXEL_BYTES;
//           *(unsigned short*)(bmp565+pos) = rgb_24_2_555(p->red>>6, p->green>>6, p->blue>>6);
//            *(unsigned short*)(bmp565+pos) = (p->red<<10) |
//                    (p->green<<5) |
//                     (p->blue);

//                        *(unsigned short*)(bmp565+pos) = (( (p->red>>9)<<10) ) |
//                    (((p->green>>9)<<5) ) |
//                     (( p->blue>>9) );
            BMPTYPE* tp = (BMPTYPE*)(bmp565+pos);
            tp->red = p->red >> (CAMERA_GRAY_BITS - CONV_PICTURE_BITS);
            tp->green = p->green >> (CAMERA_GRAY_BITS - CONV_PICTURE_BITS);
            tp->blue = p->blue >> (CAMERA_GRAY_BITS - CONV_PICTURE_BITS);
        }// end for i

        if(ltype == GRG_LINE)
            ltype = BGB_LINE;
        else
            ltype = GRG_LINE;
    }
    QImage img(bmp565, CONV_PICTURE_WIDTH, CONV_PICTURE_HEIGHT, QIMG_FORMAT);
    img.save(QString("a%1%2%3.bmp").arg(CONV_PICTURE_BITS)
             .arg(CONV_PICTURE_BITS)
             .arg(CONV_PICTURE_BITS) );
    free(bmp565);
}

void pc_to_bmp(int index, const char* name)
{
    pc_main(index);
    char buff[512];
    char name2[512];
    memset(buff, 0, 512);

    memset(name2, 0, 512);
    memcpy(name2, name, strlen(name));
#ifdef _WIN32
    for(size_t i=0; i< strlen(name); ++i)
    {
        if(*(name2+i) == '/')
            *(name2+i) = '\\';
    }

    sprintf(buff, "copy /Y a888.bmp %s.bmp", name2);
#else
    sprintf(buff, "cp a888.bmp %s.bmp", name2);
#endif
    printf("buff = %s\n", buff);
    system(buff);
}

void tiff_to_bmp(const char* filename)
{
        g_index = 0;
        //memset(buffer, 0xcc, CAMERA_PICTURE_BYTES);
        FILE* fp = fopen(filename, "rb");
        size_t sz = fread(tifBuff[g_index], 1, CAMERA_PICTURE_BYTES +TIFF_BYTES_COUNT, fp);
        if(sz != CAMERA_PICTURE_BYTES+TIFF_BYTES_COUNT)
        {
            printf("sz is %ld (%ld) \n%ld\t%ld\t%ld\n", sz, CAMERA_PICTURE_BYTES+TIFF_BYTES_COUNT,
                   CAMERA_PICTURE_WIDTH, CAMERA_PICTURE_HEIGHT, CAMERA_PIXEL_BYTES);
            return;
        }
        fclose(fp);

        //pc_main(g_index);
        pc_to_bmp(g_index, filename);
}
