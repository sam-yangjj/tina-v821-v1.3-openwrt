#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "aw_person_detection_v821.h"

#define  DBL_MAX   100000000
#define EPSILON 1e-9


double _min(double a, double b) {
	return (fabs(a - b) < EPSILON) ? a : b;
}

double _max(double a, double b) {
	return (fabs(a - b) < EPSILON) ? b : a;
}

double getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}


int main(int argc, char **argv)
{
	int i = 0;
	char image_path[256] = "0000004.png";
	int target_w = 320;
	int target_h = 192;
	int target_c = 3;
	float conf_thres = 0.3;
	int loop = 10;

	double timeMin = DBL_MAX;
	double timeMax = -DBL_MAX;
	double timeAvg = 0;

	AW_Det_Container *container = (AW_Det_Container *)calloc(1, sizeof(AW_Det_Container));
	if (!container)
	{
		fprintf(stderr, "fail to allocate memory for [container]\n");
	}

	AW_Det_Outputs *outputs = (AW_Det_Outputs *)calloc(1, sizeof(AW_Det_Outputs));
	if (!outputs)
	{
		fprintf(stderr, "fail to allocate memory for [outputs]\n");
	}

	double start1 = getCurrentTime();
	int flag = aw_init_person_det(container, outputs, target_w, target_h, target_c, conf_thres);
	if (flag == -1)
	{
		printf("fail to make smile detction container\n");
	}
	double end1 = getCurrentTime();
	double time1 = end1 - start1;
	printf("aw_init_person_det cost time:%8.2f ms\n", time1);

	AW_Img img = aw_load_image_stb(image_path, 3);
	AW_Img rsz_img = aw_resize_image(&img, target_w, target_h);

	float ratio_w = img.w * 1.0 / target_w;
	float ratio_h = img.h * 1.0 / target_h;

	double start2 = getCurrentTime();

	for (int i = 0; i < loop; i++) {
		aw_run_person_det(container, &rsz_img, outputs, conf_thres);
	}
	double end2 = getCurrentTime();
	double time2 = end2 - start2;
	timeAvg = time2 / loop;
	fprintf(stderr, "The function aw_run_person_det costs time: avg = %8.2f ms\n", timeAvg);

	for (int i = 0; i < outputs->num; i++)
	{
		float prob = outputs->persons[i].prob;
		int x1 = (int)(round(outputs->persons[i].bbox.tl_x * ratio_w));
		int y1 = (int)(round(outputs->persons[i].bbox.tl_y * ratio_h));
		int x2 = (int)(round(outputs->persons[i].bbox.br_x * ratio_w));
		int y2 = (int)(round(outputs->persons[i].bbox.br_y * ratio_h));

		fprintf(stderr, "%d, prob %f, x1 %d, y1 %d, x2 %d, y2 %d\n", i, prob, x1, y1, x2, y2);
	}

	aw_free_image(&img);
	aw_free_image(&rsz_img);
	aw_free_person_det(container, outputs);
	free(container);
	container = NULL;
	free(outputs);
	outputs = NULL;

	return 0;
}