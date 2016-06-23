#ifndef RAW_PIC5000_H
#define RAW_PIC5000_H

#define RAW_PIC_WIDTH           8304
#define RAW_PIC_HEIGHT          6220
#define RAW_PIC_BYTES_PER_PIXEL 2
#define RAW_PIC_SIZE ( (RAW_PIC_BYTES_PER_PIXEL)*(RAW_PIC_WIDTH)*(RAW_PIC_HEIGHT) )

#define SENSOR_TAG          "MTI"
#define SENSOR_LEN          75
#define SENSOR_SIZE         (3 + 4 + SENSOR_LEN)

#define TIFF_HEAD_LEN           8
#define TIFF_TAIL_LEN           138
#define TIFF_SIZE               (TIFF_HEAD_LEN + TIFF_TAIL_LEN)

#define TIFF_FILE_SIZE   (RAW_PIC_SIZE + SENSOR_SIZE + TIFF_SIZE)

/**
  Tiff file format
  TIFF_HEAD
  RAW_PICTURE
  TIFF_TAIL
  SENSOR_DATA
**/

#endif /** RAW_PIC5000_H */
