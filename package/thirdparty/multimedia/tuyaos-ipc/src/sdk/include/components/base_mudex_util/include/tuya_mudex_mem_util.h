

#ifndef __TUYA_MUDEX_MEM_UTIL_H__
#define __TUYA_MUDEX_MEM_UTIL_H__

#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>


#define USE_TUYA_MEM_TOOL


#ifdef USE_TUYA_MEM_TOOL
#define MUDEX_MALLOC(size)     tym_malloc(size)
#define MUDEX_CALLOC(n,size)   tym_calloc(n,size)
#define MUDEX_FREE(ptr)   av_free(ptr)
#else
#define MUDEX_MALLOC(size)     malloc(size)
#define MUDEX_CALLOC(n,size)   calloc(n,size)
#endif

void *tym_malloc(size_t size) ;


void *av_malloc_array(size_t nmemb, size_t size);

void *av_mallocz_array(size_t nmemb, size_t size);


void *tym_calloc(size_t nmemb, size_t size) ;


void *av_realloc(void *ptr, size_t size) ;


int av_reallocp(void *ptr, size_t size);

void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);
void *av_realloc_array(void *ptr, size_t nmemb, size_t size);

int av_reallocp_array(void *ptr, size_t nmemb, size_t size);

void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size);

void av_free(void *ptr);

void av_freep(void *ptr);

char *av_strdup(const char *s);






int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem);



static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    /* Hack inspired from glibc: don't try the division if nelem and elsize
     * are both less than sqrt(SIZE_MAX). */
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return -1;
    *r = t;
    return 0;
}

#endif /* AVUTIL_MEM_H */
