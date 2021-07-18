#ifndef MY_H
#define MY_H
#include "network.h"
//����ڴ�й©����

struct stu {
	float value;
	int index;
};
void * safe_malloc(int size);
_Bool check_next_layer(network * net, int i, int nth_layer);
int* sort_filters(float *, float *, int filter_number, int prune_filters, int size_per_filter);
inline int cmp2digits(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;//���Ǵ�С�����������ǴӴ�С�ĳɣ� return *(int *)b-*(int *)a;
}
float* prune_weights(float *,  int* prune_filters, int size_per_filter, int filters_left);
float* prune_bias(float *, int* prune_filters, int filters_left);
float* prune_weights_nextlayer(float * weights, int kernel_size, int channel_size, int n, int* channel_index);

#endif