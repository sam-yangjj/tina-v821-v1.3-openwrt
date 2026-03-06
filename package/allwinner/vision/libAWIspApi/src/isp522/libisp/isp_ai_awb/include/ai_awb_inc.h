#ifndef __AI_AWB_INC__H
#define __AI_AWB_INC__H

#ifdef __cplusplus
extern "C" {
#endif
typedef void* AI_AWB_HANDLE;

typedef struct {
    float r_gain;
    float g_gain;
    float b_gain;
} RGB_GAIN_T;

AI_AWB_HANDLE aw_ai_white_balance_init(const char *weight_path);
int aw_ai_white_balance_perf_run(AI_AWB_HANDLE handle, unsigned int *r_avgs, unsigned int *g_avgs, unsigned int *b_avgs, RGB_GAIN_T *result_gain);
void aw_ai_white_balance_destroy(AI_AWB_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif