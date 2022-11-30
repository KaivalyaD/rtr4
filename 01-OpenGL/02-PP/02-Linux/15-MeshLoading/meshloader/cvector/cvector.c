/*
 * cvector Implementation
 * Date: 21 November, 2022
 * Author: Kaivalya Deshpande
 */

#include"cvector.h"

struct fvec *fvec_create(void) {
    struct fvec *vec = (struct fvec *)malloc(sizeof(struct fvec));
    if(!vec)
        return NULL;
    
    vec->pf = (float *)malloc(sizeof(float));  // initialize with a capacity of 1 float
    vec->occupied_size = 0;
    vec->actual_size = 1;

    return vec;
}

struct ivec *ivec_create(void) {
    struct ivec *vec = (struct ivec *)malloc(sizeof(struct ivec));
    if(!vec)
        return NULL;
    
    vec->pi = (int *)malloc(sizeof(int));  // initialize with a capacity of 1 int
    vec->occupied_size = 0;
    vec->actual_size = 1;

    return vec;
}

void fvec_destroy(struct fvec *vec) {
    free(vec->pf);
    vec->pf = NULL;

    vec->actual_size = 0;
    vec->occupied_size = 0;

    free(vec);
    vec = NULL;
}

void ivec_destroy(struct ivec *vec) {
    free(vec->pi);
    vec->pi = NULL;
    
    vec->actual_size = 0;
    vec->occupied_size = 0;

    free(vec);
    vec = NULL;
}

int fvec_push_back(struct fvec *vec, float value) {
    int power = 0;

    while(vec->occupied_size >= (1 << power))
        ++power;

    if((1 << power) > vec->actual_size) {
        vec->pf = (float *)realloc(vec->pf, sizeof(float) * (1 << power));
        if(!vec->pf)
            return ML_ERR_MEMNOTENOUGH;

        vec->actual_size = 1 << power;
    }

    vec->pf[vec->occupied_size] = value;
    vec->occupied_size += 1;

    return ML_OK;
}

int ivec_push_back(struct ivec *vec, int value) {
    int power = 0;

    while(vec->occupied_size >= (1 << power))
        ++power;

    if((1 << power) > vec->actual_size) {
        vec->pi = (int *)realloc(vec->pi, sizeof(int) * (1 << power));
        if(!vec->pi)
            return ML_ERR_MEMNOTENOUGH;

        vec->actual_size = 1 << power;
    }

    vec->pi[vec->occupied_size] = value;
    vec->occupied_size += 1;

    return ML_OK;
}

int fvec_pop_back(struct fvec *vec) {
    int power = 0;
    
    if(vec->occupied_size <= 0)
        return ML_UNDERFLOW;

    vec->occupied_size -= 1;
    while(vec->occupied_size >= (1 << power))
        ++power;

    if(vec->actual_size > (1 << power)) {
        vec->pf = (float *)realloc(vec->pf, sizeof(float) * (1 << power));
        if(!vec->pf)
            return ML_ERR_MEMNOTENOUGH;

        vec->actual_size = 1 << power;
    }

    return ML_OK;
}

int ivec_pop_back(struct ivec *vec) {
    int power = 0;
    
    if(vec->occupied_size <= 0)
        return ML_UNDERFLOW;

    vec->occupied_size -= 1;
    while(vec->occupied_size >= (1 << power))
        ++power;

    if(vec->actual_size > (1 << power)) {
        vec->pi = (int *)realloc(vec->pi, sizeof(int) * (1 << power));
        if(!vec->pi)
            return ML_ERR_MEMNOTENOUGH;

        vec->actual_size = 1 << power;
    }

    return ML_OK;
}

int fvec_show(struct fvec *vec, char *separator) {
    int i;
    for(i = 0; i < vec->occupied_size; i++)
        printf("%.2f%s", vec->pf[i], separator);

    return i;
}

int ivec_show(struct ivec *vec, char *separator) {
    int i;
    for(i = 0; i < vec->occupied_size; i++)
        printf("%.2f%s", vec->pi[i], separator);

    return i;
}
