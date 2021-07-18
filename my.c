#include<stdlib.h >
#include<malloc.h>
#include<math.h>
#include"my.h"
#include<omp.h>
//#include"my.h"
_Bool check_next_layer(network * net, int i, int nth_layer)
{/*
 �������ܣ���鵱ǰ��i�Ƿ�����Ҫ�ü������һ��
 ����˵���� i	ĳ�������
			nth_layer	���ü�������
����ֵ	 ��0 ���ǣ� 1 �ǡ�
 */
	int flag = 0;
	if (i <= nth_layer)
		return 0;
	else
	{
		for (int m = nth_layer+1; m <= i;m++)
		{
			if (net->layers[m].type == CONVOLUTIONAL)
				flag++;
		}
	}
	if (flag == 1)
		return 1;
	else
		return 0;

}
void * safe_malloc( int size)
{
	void* filters_sum = malloc(size);
	if (filters_sum == NULL)
		perror("error, malloc failed...");
	memset(filters_sum, 0, size);
	return filters_sum;
}

void quick_sort(struct stu* s, int l, int r)
{
	if (l < r)
	{
		int i = l, j = r;

		struct stu  x = s[l];
		while (i<j)
		{
			while (i<j && s[j].value >= x.value)//���ҵ����ҵ���һ��С��x����  
				j--;
			if (i<j)
				s[i++] = s[j];

			while (i<j && s[i].value <= x.value)//���������ҵ���һ������x����  
				i++;
			if (i<j)
				s[j--] = s[i];
		}
		s[i] = x;//i = j��ʱ�򣬽�x�����м�λ��  
		quick_sort(s, l, i - 1);//�ݹ���� 
		quick_sort(s, i + 1, r);
	}
}

/*
 ���ܣ���ÿ������˺�ƫ��ȡ����ֵ��ͣ��õ���ÿ������Ϊָ�����۾���˵���Ҫ�ԡ�
 ����˵����weights Ȩ������ bias���� filter_number�������Ŀ prune_filtersҪ�����ľ������Ŀ
 */
int* sort_filters(float * weights, float *bias, int filter_number, int prune_filters, int size_per_filter)
{
	//filters_sum�ﱣ����ÿ������˵�Ȩֵ��ƫ�еľ���ֵ�ĺ͡�
	float* filters_sum = (float*)safe_malloc(sizeof(float)* filter_number);

	int* result = (int*)safe_malloc(sizeof(int)*prune_filters);

//��������
	for (int i = 0; i < _msize(weights) / sizeof(float); i++)
	{
        //printf("weights[%d]:%f\t, after abs wei:%f\t\n",i,weights[i], fabsf(weights[i]));
		filters_sum[i/size_per_filter] += fabsf(weights[i]);//���� �� /
	}
	printf("before sort filters_sum are:\n");
	for (int i = 0; i < filter_number; i++)
	{
        //printf("bias[%d]:%f\t, after abs:%f\n ", i, bias[i], fabsf(bias[i]));
		filters_sum[i] += fabsf(bias[i]);
		//printf("%f\n",filters_sum[i]);
	}
	//��filters_sum�������򣬵õ�prune_filters����С�ľ���˵ı�š�
	struct stu* filters_sum_index = (struct stu*)safe_malloc(sizeof(struct stu)*filter_number);

	for (int i = 0; i < filter_number; i++)
	{
		filters_sum_index[i].index = i;
		filters_sum_index[i].value = filters_sum[i];
	}
	quick_sort(filters_sum_index, 0, filter_number-1);
//	printf("after sort  filtersum 's order are:\n");
//	for (int i = 0; i < filter_number; i++)
//	{
//	printf("%f\n", filters_sum_index[i].value);
//	}
//	printf("after sort the least %d filter's indexs are:\n", prune_filters);
	for (int i = 0; i < prune_filters; i++)
	{
			 result[i] = filters_sum_index[i].index;
//			 printf(" index: %d\t,fitersum %f\n", result[i], filters_sum[result[i]]);
	}
	free(filters_sum_index);
	free(filters_sum);
	return result;
}
int get_channel_index(int weight_index,int kernel_size,int channel_size,int n)
{
	int result = weight_index % (kernel_size*kernel_size*channel_size) / (kernel_size*kernel_size);
	return result;
}
_Bool check_index(int i, int size_per_filter, int* prune_filters_index)
{/*
 ���ܣ����Ȩֵ�������Ƿ����ڱ������ľ����
 */
	int flag = 0;
	int index_num = _msize(prune_filters_index) / sizeof(int);
	for (int j = 0; j < index_num; j++)
	{
		if ((int)(i / size_per_filter) != prune_filters_index[j])
			continue;
        else {
            flag++;
            break;
        }
			
	}
	if (flag)
		return 0;
	else
		return 1;
}
_Bool check_index_channel(int channel, int* prune_filters_index)
{/*
 ���ܣ����Ȩֵ�������Ƿ����ھ���˱�������ͨ��
 */
	int flag = 0;
	int index_num = _msize(prune_filters_index) / sizeof(int);
	for (int j = 0; j < index_num; j++)
	{
		if (channel != prune_filters_index[j])
			continue;
        else {
            flag++;
            break;
        }
			
	}
	if (flag)
		return 0;
	else
		return 1;
}

float* prune_weights(float * weights, int* prune_filters_index, int size_per_filter, int filters_left)
{/*
�β�˵����weights��ʼȨֵ  prune_filters_index ��Ҫ�޸ĵľ�������� 
			size_per_filter ÿ�������Ԫ�ظ���   filters_left �ü��������ٸ������
����˵�����޼�֮���Ȩֵ��ַ	
 */			
	int weights_elements_num = _msize(weights) / sizeof(float);
	int index_num = _msize(prune_filters_index) / sizeof(int);
	float *new_weights = (float*)safe_malloc(sizeof(float)*size_per_filter* filters_left);
	int m = 0;
    int i;
    #pragma omp parallel for
	for (i = 0; i < weights_elements_num; i++)
	{
		if (check_index(i, size_per_filter, prune_filters_index))
			new_weights[m++] = weights[i];
		else
			continue;
	}
	free(weights);//���������ģ�ʵ��һ�¡�
	return new_weights;
}
float* prune_weights_nextlayer(float * weights, int kernel_size, int channel_size,int n, int* channel_index)
{/*
 ���ܣ��Ա����ü�������ȥ��Ӧ��ͨ��
 �β�˵����weights��ʼȨֵ  kernel_size ����˳ߴ�  channel_size�����ͨ����
		   n �ò����˸���	channel_index ���޼���ͨ����
 ����˵�����޼�֮���Ȩֵ��ַ
 */
	int weights_elements_num = _msize(weights) / sizeof(float);
	int index_num = _msize(channel_index) / sizeof(int);
	int size_per_filter = kernel_size*kernel_size*channel_size;
	int size_per_filter_after_prune = kernel_size*kernel_size*(channel_size - index_num);
	float *new_weights = (float*)safe_malloc(sizeof(float)*size_per_filter_after_prune* n);
	int m = 0,i =0,j=0,k=0;
//    printf("weights_elements_num:%d,\t index_num:%d,\t size_per_filter:%d,\t n:%d,\tC:%d,\t", weights_elements_num, index_num,
//        size_per_filter, n, channel_size);
    #pragma omp parallel for
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < channel_size; j++)
		{
			for (k = 0; k < kernel_size*kernel_size;k++) {//��ÿ��ԭ����Ȩֵ�����������ж�
				int index = (i*size_per_filter)+j*kernel_size*kernel_size + k;
				int channel = get_channel_index(index, kernel_size, channel_size, n);
				if (check_index_channel(channel, channel_index))
					new_weights[m++] = weights[index];
				else
					continue;
			}
		}
	}
	free(weights);
	return new_weights;
}
_Bool check_bias_index(int i, int* prune_filters_index)
{
	int flag = 0;
	int index_num = _msize(prune_filters_index) / sizeof(int);
	for (int j = 0; j < index_num; j++)
	{
		if (i != prune_filters_index[j])
			continue;
        else
        {
            flag++;
            break;
        }	
	}
	if (flag)
		return 0;
	else
		return 1;
}
float* prune_bias(float * bias, int* prune_filters_index,int filters_left)
{//����˵���� biasԭʼƫ������ prune_filters_index ��Ҫ������ƫ������ filters_leftʣ��ƫ�еĸ�����
//				���������µ�ƫ�������ڴ�
//���ز������ü�֮���ƫ�������ַ
	int bias_elements_num = _msize(bias) / sizeof(float);
	int index_num = _msize(prune_filters_index) / sizeof(int);
	float *new_bias = (float*)safe_malloc(sizeof(float)* filters_left);
	int m = 0, i = 0;
    #pragma omp parallel for
	for (i = 0; i < bias_elements_num; i++)
	{
		if (check_bias_index(i,  prune_filters_index))
			new_bias[m++] = bias[i];
		else
			continue;
	}
	free(bias);
	return new_bias;
}