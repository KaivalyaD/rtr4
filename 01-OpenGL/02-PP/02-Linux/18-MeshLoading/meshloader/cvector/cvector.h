/*
 * cvector Implementation
 * Date: 21 November, 2022
 * Author: Kaivalya Deshpande
 */

#ifndef CVECTOR_H
#define CVECTOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/* headers */
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<math.h>

/* macro definitions */
#define ML_OK 0
#define ML_ERR_MEMNOTENOUGH -1
#define ML_OVERFLOW -2
#define ML_UNDERFLOW -3

/* struct definitions */
struct ivec {
    int *pi;
    int occupied_size;
    int actual_size;
};

struct fvec {
    float *pf;
    int occupied_size;
    int actual_size;
};

/* API */
struct fvec *fvec_create(void);
struct ivec *ivec_create(void);

void fvec_destroy(struct fvec *vec);
void ivec_destroy(struct ivec *vec);

int fvec_push_back(struct fvec *vec, float value);
int ivec_push_back(struct ivec *vec, int value);

int fvec_pop_back(struct fvec *vec);
int ivec_pop_back(struct ivec *vec);

int fvec_show(struct fvec *vec, char *separator);
int ivec_show(struct ivec *vec, char *separator);

#ifdef __cplusplus
}
#endif

#endif  // CVECTOR_H
