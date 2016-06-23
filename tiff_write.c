#include <stdio.h>
#include <tiffio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
  // Define an image
    
    unsigned short *buffer = (unsigned short*)malloc(8304 * 6220*2);
  //memset(buffer, 0xcc, 8304*6220*2);
    FILE* fp = fopen("a.bin", "rb");

    size_t sz = fread(buffer, 1, 8304*6220*2, fp);
    fclose(fp);
        printf("fp is %x, read %ld bytes\n", fp,  sz);
    
    unsigned short line[8304];
    for(int j=0; j<6220; ++j)
    {
        unsigned short *p = buffer + 8304*j;
        for(int i=0; i<8304/2; ++i)
        {
            line[i] = (*(p+i*2+1) & 16383);
            line[i] *=4;
            line[8303-i] = (*(p+i*2) & 16383);
            line[8303-i] *=4;
        }
        memcpy((char*)p, (const char*)line, 8304*2);
    }
  TIFF *image;

  // Open the TIFF file
  if((image = TIFFOpen("output.tif", "w")) == NULL){
    printf("Could not open output.tif for writing\n");
    exit(42);
  }
  // We need to set some values for basic tags before we can add any data
  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, 8304 );
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, 6220);
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 16);
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, 6220);
  TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  //TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
  //TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
//  TIFFSetField(image, TIFFTAG_XRESOLUTION, 30.0);
//  TIFFSetField(image, TIFFTAG_YRESOLUTION, 30.0);
//  TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
  
  // Write the information to the file
  TIFFWriteEncodedStrip(image, 0, (char*)buffer, 8304 * 6220 *2);
//    for (j = 0; j < 144; j++)
//        TIFFWriteScanline(image, &buffer[j * 25], j, 0);
  // Close the file
  TIFFClose(image);
}

