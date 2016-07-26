#include <stdio.h>
#include <math.h>

/* Doesn't work when a and b are the same object
 * - assigns zero (0) to the object in that case */
#define XORSWAP_UNSAFE(a, b)    ((a)^=(b),(b)^=(a),(a)^=(b))

struct elements {
    int v1;
    int v2;
};

void segmentedBitonicSort(float *data, int *seg_id, int *seg_start, int ele_num, int seg_num);

void bitonicSort(bool ascending, float *data, int lower_bound, int upper_bound);

void bitonicMerge(bool ascending, float *data, int lower_bound, int upper_bound);

int main() {
    float data[10] = {0.8, -INFINITY, 0.2, 0.3, NAN, NAN, 12, 1, 0.1, 0.5};

    int seg_id[10] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1};

    int seg_start[3] = {0, 7, 10};

    int n = 10;

    int m = 2;

    segmentedBitonicSort(data, seg_id, seg_start, n, m);

    for (int i = 0; i < n; i++)
        printf("%f\n", data[i]);

    return 0;
}

inline void segmentedBitonicSort(float *data, int *seg_id, int *seg_start, int ele_num, int seg_num) {
    if (ele_num < 1 || seg_num < 1)
        fprintf(stderr, "ERROR: the ele_num or seg_num must more than 0");

    /* assume there is no inf in data */
    for (int i = 0; i < ele_num; i++)
        if (data[i] != data[i]) data[i] = INFINITY;

    for (int i = 0; i < seg_num; i++)//可并行
        bitonicSort(true, data, seg_start[i], seg_start[i + 1] - 1);

    for (int i = 0; i < ele_num; i++)
        if (data[i] == INFINITY) data[i] = NAN;
}

inline void bitonicSort(bool ascending, float *data, int lower_bound, int upper_bound) {
    int n = upper_bound - lower_bound + 1;
    int margin;

    for (margin = 2; margin <= n; margin <<= 1) {//自底而上
        for (int i = upper_bound; i - margin + 1 >= lower_bound; i -= margin)//可并行
            bitonicMerge(ascending, data, i - margin + 1, i);

        if (n % margin > 1)
            bitonicMerge(ascending, data, lower_bound, n % margin - 1 + lower_bound);
    }

    if (margin >> 1 < n)
        bitonicMerge(ascending, data, lower_bound, upper_bound);
}

inline void bitonicMerge(bool ascending, float *data, int lower_bound, int upper_bound) {
    int m, n=upper_bound - lower_bound + 1;

    /* Variable-length automatic arrays are allowed in ISO C99,
     * and as an extension GCC accepts them in C90 mode and in C++. */
    elements stack[(int)log2(n)];
    int index = 0;

    stack[index].v1 = lower_bound;
    stack[index++].v2 = upper_bound;

    while (index > 0) {
        lower_bound = stack[--index].v1;
        upper_bound = stack[index].v2;

        while (upper_bound > lower_bound) {
            n = upper_bound - lower_bound + 1;

            m = 1;
            while (m < n) m <<= 1;
            m >>= 1;

            for (int i = lower_bound; i < (n - m) + lower_bound; i++)//可并行
                if ((data[i] > data[i + m]) == ascending) {
                    /* Runtime must ensure sizeof(int) equal to sizeof(float) */
                    XORSWAP_UNSAFE(*(int *) (data + i),
                                   *(int *) (data + i + m));
                }


            if (ascending) {
                stack[index].v1 = lower_bound;
                stack[index++].v2 = lower_bound + m - 1;

                lower_bound = lower_bound + m;
            } else {
                stack[index].v1 = lower_bound;
                stack[index++].v2 = lower_bound + (n - m) - 1;

                lower_bound = lower_bound + (n - m);
            }
        }
    }
}