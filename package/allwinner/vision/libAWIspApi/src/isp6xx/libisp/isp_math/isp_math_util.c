
#include "isp_math_util.h"

#define NUM 8

void HorizontalMirror(int arr[64])
{
	int i, j, temp;

	for (i = 0; i < NUM; i++) {
		for (j = 0; j < NUM / 2; j++) {
			temp = arr[i*NUM+j];
			arr[i*NUM+j] = arr[i*NUM+NUM -j-1];
			arr[i*NUM+NUM -j-1] = temp;
		}
	}
}

void VerticalMirror(int arr[64])
{
	int i, j, temp;

	for (i = 0; i < NUM; i++) {
		for (j = 0; j < NUM / 2; j++) {
			temp = arr[j*NUM+i];
			arr[j*NUM+i]= arr[(NUM -j-1)*NUM+i];
			arr[(NUM -j-1)*NUM+i] = temp;
		}
	}
}

int ValueInterp(int curr, int x_low, int x_high, int y_low, int y_high)
{
	if ((x_high - x_low) == 0)
		return y_low;

	if (curr <= x_low)
		return y_low;
	else if (curr >= x_high)
		return y_high;

	return (y_low + (y_high - y_low)*(curr - x_low)/(x_high - x_low));
}

void Conv(int u[],int v[],int w[], int m, int n)
{
	// w = u*v; length(u) = m; length(v) = n.
	int i, j;
	int k = m+n-1;

	for (i = 0; i < k; i++) {
		for (j = max(0, i+1-n); j <= min(i, m-1); j++) {
			w[i] += ((((HW_S64)u[j])*v[i-j])>>10);
		}
	}
}

int ArrayFindMinIndex(int array[], int num)
{
	int i, ind = 0;
	int tmp = array[0];

	for (i = 0; i < num; i++) {
		if (array[i] < tmp) {
			tmp = array[i];
			ind = i;
		}
	}
	return ind;
}

int ArrayFindSecondMinIndex(int array[], int num, int exclude)
{
	int i, ind = 0;
	int tmp = array[0];

	if (exclude == 0) {
		tmp = array[num - 1];
		ind = num - 1;
	}

	for (i = 0; i < num; i++) {
		if (array[i] < tmp && i != exclude) {
			tmp = array[i];
			ind = i;
		}
	}
	return ind;
}

int ArrayFindMaxIndex(int array[],int num)
{
	int i, ind = 0;
	int tmp = array[0];

	for (i = 0; i < num; i++) {
		if (array[i] > tmp) {
			tmp = array[i];
			ind = i;
		}
	}
	return ind;
}

int ArrayFindSecondMaxIndex(int array[], int num, int exclude)
{
	int i, ind = 0;
	int tmp = array[0];

	if (exclude == 0) {
		tmp = array[num - 1];
		ind = num - 1;
	}

	for (i = 0; i < num; i++) {
		if (array[i] > tmp && i != exclude) {
			tmp = array[i];
			ind = i;
		}
	}
	return ind;
}

void ShellSort(int *a,int *sub, int n)
{
	int i, j, k, x, y;
	k = n/2;

	while (k >= 1) {
		for (i = k; i < n; i++) {
			x = a[i];
			y = sub[i];
			j = i-k;
			while(j>=0 && x>a[j]) {
				a[j+k] = a[j];
				sub[j+k] = sub[j];
				j -= k;
			}
			a[j+k] = x;
			sub[j+k] = y;
		}
		k /= 2;
	}
}

int ArraySum(int *array, int len)
{
	int i, sum = 0;

	for (i = 0;i<len;i++) {
		sum += array[i];
	}
	return sum;
}

int SqrtM(unsigned int a)
{
	int x, i = 100, tmp = 0;

	if (a == 0)
		return 0;
	x = (a+1)/2;
	while ((i--) && (abs(tmp-x) > 1)) {
		tmp = x;
		x = (x+a/x+1)/2;
	}
	return x;
}

int ArrayStdVar(int x[], int n)
{
	int i, xaver = 0, x2aver = 0;

	for (i = 0; i < n;++i) {
		xaver+=x[i];
		x2aver+=x[i]*x[i];
	}
	xaver/=n;
	x2aver/=n;
	return (int)sqrtf((float)(x2aver-xaver*xaver));
}

int RoundQ4(int x)
{
	int ret;
	//printf("x&0xf = %d\n",x&0xf);
	if (0x8 > (x&0xf) ) {
		ret = x>>4;
	} else {
		ret = (x>>4)+1;
	}
	return ret;
}

void matrix_zero(struct matrix *m)
{
	unsigned int i, j;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			m->value[i][j] = 0.0;
	}
}

void matrix_invert(struct matrix *m)
{
	/* Invert the matrix using the transpose of the matrix of cofactors. The
	 * Gauss-Jordan elimination would be faster in the general case, but we
	 * know that the matrix is 3x3.
	 */
	const float eps = 1e-6;
	struct matrix out;
	unsigned int i, j;
	float det;

	out.value[0][0] = m->value[1][1] * m->value[2][2]
			- m->value[1][2] * m->value[2][1];
	out.value[0][1] = m->value[0][2] * m->value[2][1]
			- m->value[0][1] * m->value[2][2];
	out.value[0][2] = m->value[0][1] * m->value[1][2]
			- m->value[0][2] * m->value[1][1];
	out.value[1][0] = m->value[1][2] * m->value[2][0]
			- m->value[1][0] * m->value[2][2];
	out.value[1][1] = m->value[0][0] * m->value[2][2]
			- m->value[0][2] * m->value[2][0];
	out.value[1][2] = m->value[0][2] * m->value[1][0]
			- m->value[0][0] * m->value[1][2];
	out.value[2][0] = m->value[1][0] * m->value[2][1]
			- m->value[1][1] * m->value[2][0];
	out.value[2][1] = m->value[0][1] * m->value[2][0]
			- m->value[0][0] * m->value[2][1];
	out.value[2][2] = m->value[0][0] * m->value[1][1]
			- m->value[1][0] * m->value[0][1];

	det = m->value[0][0] * out.value[0][0] +
			m->value[0][1] * out.value[1][0] +
			m->value[0][2] * out.value[2][0];

	if (det < eps)
		return;

	det = 1/det;

	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
			m->value[i][j] = out.value[i][j] * det;
}

void matrix_multiply(struct matrix *a, const struct matrix *b)
{
	struct matrix out;

	/* Compute a * b and return the result in a. */
	out.value[0][0] = a->value[0][0] * b->value[0][0]
			+ a->value[0][1] * b->value[1][0]
			+ a->value[0][2] * b->value[2][0];
	out.value[0][1] = a->value[0][0] * b->value[0][1]
			+ a->value[0][1] * b->value[1][1]
			+ a->value[0][2] * b->value[2][1];
	out.value[0][2] = a->value[0][0] * b->value[0][2]
			+ a->value[0][1] * b->value[1][2]
			+ a->value[0][2] * b->value[2][2];
	out.value[1][0] = a->value[1][0] * b->value[0][0]
			+ a->value[1][1] * b->value[1][0]
			+ a->value[1][2] * b->value[2][0];
	out.value[1][1] = a->value[1][0] * b->value[0][1]
			+ a->value[1][1] * b->value[1][1]
			+ a->value[1][2] * b->value[2][1];
	out.value[1][2] = a->value[1][0] * b->value[0][2]
			+ a->value[1][1] * b->value[1][2]
			+ a->value[1][2] * b->value[2][2];
	out.value[2][0] = a->value[2][0] * b->value[0][0]
			+ a->value[2][1] * b->value[1][0]
			+ a->value[2][2] * b->value[2][0];
	out.value[2][1] = a->value[2][0] * b->value[0][1]
			+ a->value[2][1] * b->value[1][1]
			+ a->value[2][2] * b->value[2][1];
	out.value[2][2] = a->value[2][0] * b->value[0][2]
			+ a->value[2][1] * b->value[1][2]
			+ a->value[2][2] * b->value[2][2];

	*a = out;
}

void matrix_float_to_s2q8(HW_S8Q8 out[3][3], const struct matrix *in)
{
	unsigned int i, j;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			out[i][j] = ((HW_S8Q8)(in->value[i][j] * 256) & 0x3ff);
	}
}

void matrix_float_to_s4q8(HW_S8Q8 out[3][3], const struct matrix *in)
{
	unsigned int i, j;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			out[i][j] = ((HW_S8Q8)(in->value[i][j] * 256) & 0xfff);
	}
}

void matrix_float_to_s8q8(HW_S8Q8 out[3][3], const struct matrix *in)
{
	unsigned int i, j;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			out[i][j] = ((HW_S8Q8)(in->value[i][j] * 256));
	}
}

void spline(float x[], float y[], int n, float yp1, float ypn, float y2[])
{
	float u[100],aaa,sig,p,bbb,ccc,qn,un;
	int i, k;

	if (yp1 > 9.9e+29) {
		y2[1] = 0;
		u[1] = 0;
	} else {
		y2[1] = -0.5;
		aaa = (y[2] - y[1]) / (x[2] - x[1]);
		u[1] = (3.0 / (x[2] - x[1])) * (aaa - yp1);
	}

	for (i = 2; i <= n-1; i++) {
		sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
		p = sig * y2[i - 1] + 2.0;
		y2[i] = (sig - 1.0) / p;
		aaa = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
		bbb = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
		ccc = x[i + 1] - x[i - 1];
		u[i] = (6.0 * (aaa - bbb) / ccc - sig * u[i - 1]) / p;
	}

	if (ypn > 9.9e+29) {
		qn = 0.0;
		un = 0.0;
	} else {
		qn = 0.5;
		aaa = ypn - (y[n] - y[n - 1]) / (x[n] - x[n - 1]);
		un = (3.0/ (x[n] - x[n - 1])) * aaa;
	}
	y2[n] = (un - qn * u[n - 1]) / (qn * y2[n - 1] + 1.0);
	for (k = n - 1;k>=1;k--)
		y2[k] = y2[k] * y2[k + 1] + u[k];
}

void splint(float xa[], float ya[], float y2a[], int n, float x, float *y)
{
	int klo,khi,k;
	float h,a,b,aaa,bbb;
	klo = 1;
	khi = n;
loop:
	if (khi - klo > 1 ) {
		k = (khi + klo) / 2;
		if (xa[k] > x)
			khi = k;
		else
			klo = k;
		goto loop;
	}
	h = xa[khi] - xa[klo];
	if (h == 0 ) {
		printf("pause 'bad xa input'");
		return;
	}
	a = (xa[khi] - x) / h;
	b = (x - xa[klo]) / h;
	aaa = a * ya[klo] + b * ya[khi];
	bbb = (a*a*a - a) * y2a[klo] + (b*b*b - b) * y2a[khi];
	*y = aaa + bbb * h*h /6.0;
}

void spline_interp_u16(unsigned short x[], unsigned short y[], int n,
				unsigned short x2[], unsigned short y2[], int n2)
{
	int i;
	float *xa, *ya, *y_sec, y_temp;
	xa = malloc((n+1) * sizeof(float));
	ya = malloc((n+1) * sizeof(float));
	y_sec = malloc((n+1) * sizeof(float));

	xa[0] = ya[0] = y_sec[0] = 0;
	for (i = 1; i <= n; i++) {
		xa[i] = (float)x[i-1];
		ya[i] = (float)y[i-1];
		y_sec[i] = 0;
	}
	spline(xa, ya, n, 1e+31, 1e+31, y_sec);
	for (i = 0; i < n2; i++) {
		splint(xa, ya, y_sec, n, (float)(x2[i]), &y_temp);
		y2[i] = (unsigned short)(y_temp+0.5);
	}
#if 0
	printf("\n");

	for (i = 0; i < n2; i++) {
		printf("%d,", y2[i]);
	}

	printf("\n");
#endif
	free(xa);
	free(ya);
	free(y_sec);
}

#if USE_ARM_NEON
static void gaussianSmooth_Neon(float *srcTbl, int RowNum, int ColNum)
{
	int i = 1, j = 1;
	float *tmpSrcTbl = NULL;
	int x0, x1, x2;
	int y0, y1, y2;
	float gKernel[3 * 3] = {
		0.010340243, 0.081131135, 0.010340243, //sigma=0.05
		0.081131135, 0.634114491, 0.081131135,
		0.010340243, 0.081131135, 0.010340243
	};

	tmpSrcTbl = (float *)malloc(sizeof(float) * RowNum * ColNum);
	if (!tmpSrcTbl) {
		ISP_ERR("malloc tmpSrcTbl failed!\n");
		return;
	}
	memcpy(tmpSrcTbl, srcTbl, sizeof(float) * RowNum * ColNum);

	for (i = 1; i < RowNum-1; i++) {
		for (j = 1; j < ColNum-4; j += 4) { // Process 4 elements at a time using Neon
			float32x4_t sum = vdupq_n_f32(0.0f);

			// Calculate indices for accessing kernel elements
			x0 = j - 1;
			x1 = j;
			x2 = j + 1;
			y0 = i - 1;
			y1 = i;
			y2 = i + 1;

			// Load input data into Neon registers
			float32x4_t input0 = vld1q_f32(&tmpSrcTbl[y0 * ColNum + x0]);
			float32x4_t input1 = vld1q_f32(&tmpSrcTbl[y0 * ColNum + x1]);
			float32x4_t input2 = vld1q_f32(&tmpSrcTbl[y0 * ColNum + x2]);
			float32x4_t input3 = vld1q_f32(&tmpSrcTbl[y1 * ColNum + x0]);
			float32x4_t input4 = vld1q_f32(&tmpSrcTbl[y1 * ColNum + x1]);
			float32x4_t input5 = vld1q_f32(&tmpSrcTbl[y1 * ColNum + x2]);
			float32x4_t input6 = vld1q_f32(&tmpSrcTbl[y2 * ColNum + x0]);
			float32x4_t input7 = vld1q_f32(&tmpSrcTbl[y2 * ColNum + x1]);
			float32x4_t input8 = vld1q_f32(&tmpSrcTbl[y2 * ColNum + x2]);

			// Multiply input with kernel elements and accumulate the sum
			sum = vmlaq_n_f32(sum, input0, gKernel[0 * 3 + 0]);
			sum = vmlaq_n_f32(sum, input1, gKernel[0 * 3 + 1]);
			sum = vmlaq_n_f32(sum, input2, gKernel[0 * 3 + 2]);
			sum = vmlaq_n_f32(sum, input3, gKernel[1 * 3 + 0]);
			sum = vmlaq_n_f32(sum, input4, gKernel[1 * 3 + 1]);
			sum = vmlaq_n_f32(sum, input5, gKernel[1 * 3 + 2]);
			sum = vmlaq_n_f32(sum, input6, gKernel[2 * 3 + 0]);
			sum = vmlaq_n_f32(sum, input7, gKernel[2 * 3 + 1]);
			sum = vmlaq_n_f32(sum, input8, gKernel[2 * 3 + 2]);

			// Store the result back to the output array
			vst1q_f32(&srcTbl[i * ColNum + j], sum);
		}
	}

	for (i = 0; i < RowNum; i++) {
		for (; j < ColNum; j++) {
			y0 = i - 1;
			if (y0 < 0)
				y0 = 1;
			y1 = i;
			y2 = i + 1;
			if (y2 > RowNum - 1)
				y2 = RowNum - 2;
			x0 = j - 1;
			if (x0 < 0)
				x0 = 1;
			x1 = j;
			x2 = j + 1;
			if (x2 > ColNum - 1)
				x2 = ColNum - 2;
			srcTbl[i * ColNum + j]
			    = tmpSrcTbl[y0 * ColNum + x0] * gKernel[0 * 3 + 0]
				+ tmpSrcTbl[y0 * ColNum + x1] * gKernel[0 * 3 + 1]
			    + tmpSrcTbl[y0 * ColNum + x2] * gKernel[0 * 3 + 2]
				+ tmpSrcTbl[y1 * ColNum + x0] * gKernel[1 * 3 + 0]
				+ tmpSrcTbl[y1 * ColNum + x1] * gKernel[1 * 3 + 1]
				+ tmpSrcTbl[y1 * ColNum + x2] * gKernel[1 * 3 + 2]
				+ tmpSrcTbl[y2 * ColNum + x0] * gKernel[2 * 3 + 0]
				+ tmpSrcTbl[y2 * ColNum + x1] * gKernel[2 * 3 + 1]
				+ tmpSrcTbl[y2 * ColNum + x2] * gKernel[2 * 3 + 2];
		}
	}
	for (i = 0; i < RowNum; i+=(RowNum-1)) {
		for (j = 0; j < ColNum; j++) {
			y0 = i - 1;
			if (y0 < 0)
				y0 = 1;
			y1 = i;
			y2 = i + 1;
			if (y2 > RowNum - 1)
				y2 = RowNum - 2;
			x0 = j - 1;
			if (x0 < 0)
				x0 = 1;
			x1 = j;
			x2 = j + 1;
			if (x2 > ColNum - 1)
				x2 = ColNum - 2;
			srcTbl[i * ColNum + j]
				= tmpSrcTbl[y0 * ColNum + x0] * gKernel[0 * 3 + 0]
				+ tmpSrcTbl[y0 * ColNum + x1] * gKernel[0 * 3 + 1]
				+ tmpSrcTbl[y0 * ColNum + x2] * gKernel[0 * 3 + 2]
				+ tmpSrcTbl[y1 * ColNum + x0] * gKernel[1 * 3 + 0]
				+ tmpSrcTbl[y1 * ColNum + x1] * gKernel[1 * 3 + 1]
				+ tmpSrcTbl[y1 * ColNum + x2] * gKernel[1 * 3 + 2]
				+ tmpSrcTbl[y2 * ColNum + x0] * gKernel[2 * 3 + 0]
				+ tmpSrcTbl[y2 * ColNum + x1] * gKernel[2 * 3 + 1]
				+ tmpSrcTbl[y2 * ColNum + x2] * gKernel[2 * 3 + 2];
		}
	}
	for (i = 1; i < RowNum-1; i++) {
		y0 = i - 1;
		if (y0 < 0)
			y0 = 1;
		y1 = i;
		y2 = i + 1;
		if (y2 > RowNum - 1)
			y2 = RowNum - 2;
		x0 = 1;
		x1 = 0;
		x2 = 1;
		if (x2 > ColNum - 1)
			x2 = ColNum - 2;
		srcTbl[i * ColNum]
			= tmpSrcTbl[y0 * ColNum + x0] * gKernel[0 * 3 + 0]
			+ tmpSrcTbl[y0 * ColNum + x1] * gKernel[0 * 3 + 1]
			+ tmpSrcTbl[y0 * ColNum + x2] * gKernel[0 * 3 + 2]
			+ tmpSrcTbl[y1 * ColNum + x0] * gKernel[1 * 3 + 0]
			+ tmpSrcTbl[y1 * ColNum + x1] * gKernel[1 * 3 + 1]
			+ tmpSrcTbl[y1 * ColNum + x2] * gKernel[1 * 3 + 2]
			+ tmpSrcTbl[y2 * ColNum + x0] * gKernel[2 * 3 + 0]
			+ tmpSrcTbl[y2 * ColNum + x1] * gKernel[2 * 3 + 1]
			+ tmpSrcTbl[y2 * ColNum + x2] * gKernel[2 * 3 + 2];
	}

	free(tmpSrcTbl);
	tmpSrcTbl = NULL;
}
#endif

void gaussianSmooth(float *srcTbl, int RowNum, int ColNum)
{
#if USE_ARM_NEON
	gaussianSmooth_Neon(srcTbl, RowNum, ColNum);
#else
	int i, j;
	float *tmpSrcTbl = NULL;
	int x0, x1, x2;
	int y0, y1, y2;
	float gKernel[3 * 3] = {
		0.010340243, 0.081131135, 0.010340243, //sigma=0.05
		0.081131135, 0.634114491, 0.081131135,
		0.010340243, 0.081131135, 0.010340243
	};

	tmpSrcTbl = (float *)malloc(sizeof(float) * RowNum * ColNum);
	if (!tmpSrcTbl) {
		ISP_ERR("malloc tmpSrcTbl failed!\n");
		return;
	}
	memcpy(tmpSrcTbl, srcTbl, sizeof(float) * RowNum * ColNum);
	for (i = 0; i < RowNum; i++) {
		for (j = 0; j < ColNum; j++) {
			y0 = i - 1;
			if (y0 < 0)
				y0 = 1;
			y1 = i;
			y2 = i + 1;
			if (y2 > RowNum - 1)
				y2 = RowNum - 2;
			x0 = j - 1;
			if (x0 < 0)
				x0 = 1;
			x1 = j;
			x2 = j + 1;
			if (x2 > ColNum - 1)
				x2 = ColNum - 2;
			srcTbl[i * ColNum + j]
			    = tmpSrcTbl[y0 * ColNum + x0] * gKernel[0 * 3 + 0]
				+ tmpSrcTbl[y0 * ColNum + x1] * gKernel[0 * 3 + 1]
			    + tmpSrcTbl[y0 * ColNum + x2] * gKernel[0 * 3 + 2]
				+ tmpSrcTbl[y1 * ColNum + x0] * gKernel[1 * 3 + 0]
				+ tmpSrcTbl[y1 * ColNum + x1] * gKernel[1 * 3 + 1]
				+ tmpSrcTbl[y1 * ColNum + x2] * gKernel[1 * 3 + 2]
				+ tmpSrcTbl[y2 * ColNum + x0] * gKernel[2 * 3 + 0]
				+ tmpSrcTbl[y2 * ColNum + x1] * gKernel[2 * 3 + 1]
				+ tmpSrcTbl[y2 * ColNum + x2] * gKernel[2 * 3 + 2];
		}
	}
	free(tmpSrcTbl);
	tmpSrcTbl = NULL;
#endif
}

void cat32X24TableTo16X16Table(float *srcTbl, float *dstTbl)
{
	int i = 0, j = 0, p1 = 0, p2 = 0, p3 = 0, p4 = 0;
	for (i = 0; i < 16; i+=2) {
		for (j = 0; j < 16; j++) {
			p1 = (i / 2 + i) * 32 + 2 * j;
			p2 = p1 + 1;
			p3 = p1 + 32;
			p4 = p1 + 33;
			dstTbl[i*16+j] = (0.66666666667f * (srcTbl[p1] + srcTbl[p2]) + 0.33333333333f * (srcTbl[p3] + srcTbl[p4])) / 2;
		}
	}
	for (i = 1; i < 16; i+=2) {
		for (j = 0; j < 16; j++) {
			p1 = (i / 2 + i) * 32 + 2 * j;
			p2 = p1 + 1;
			p3 = p1 + 32;
			p4 = p1 + 33;
			dstTbl[i*16+j] = (0.33333333333f * (srcTbl[p1] + srcTbl[p2]) + 0.66666666667f * (srcTbl[p3] + srcTbl[p4])) / 2;
		}
	}
}
