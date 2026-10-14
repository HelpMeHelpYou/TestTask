#ifndef BASE_FILTER_H
#define BASE_FILTER_H

enum { MEDIAN_MAX_RADIUS = 8, MEDIAN_MAX_DIMENSION = 4096 };

typedef struct ImageParams {
    int width;
    int height;
    int input_stride;   /* bytes */
    int output_stride;  /* bytes */
} ImageParams;

#endif //BASE_FILTER_H