#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include <immintrin.h>
// #include <stddef.h>

// Globals
int n,m,k;

// Data types
struct input_arrays {
    float *x;  // nxn (single block allocation)
    float *b;   // m
}; typedef struct input_arrays input_arrays_t;

struct output_array {
    float *y; // nxn (single block allocation)
}; typedef struct output_array output_array_t;

// Declarations
input_arrays_t* init_input_arrays(void);
output_array_t* init_output_array(void);
void print_float_array(float *array, int size);
void cleanup(input_arrays_t *input, output_array_t *output);
void output_array_manipulation(input_arrays_t *input, output_array_t *output);

// Function implementations
input_arrays_t* init_input_arrays(void) {
    
    input_arrays_t *obj = malloc(sizeof(input_arrays_t));

    float *x = malloc(n * n * sizeof(float));
    float *b = malloc(m * sizeof(float));
    // assume malloc does not fail

    // Initialize all elements to 2
    // single block allocation is mroe cache performant,
    // since it is a continuous block of memory.
    for (int i = 0; i < n * n; i++) {
        x[i] = 2.0f;
    }

    for (int i = 0; i < m; i++) {
        b[i] = 1.0f;
    }

    (*obj).x = x;
    (*obj).b = b;

    return obj;
}

output_array_t* init_output_array(void) {
    
    output_array_t *obj = malloc(sizeof(output_array_t));

    float *y = malloc(n * n * sizeof(float));
    // assume malloc does not fail

    // Initialize all elements to 2
    // single block allocation is mroe cache performant,
    // since it is a continuous block of memory.
    for (int i = 0; i < n * n; i++) {
        y[i] = 1.0f;
    }

    (*obj).y = y;
    return obj;
}

void print_float_array(float *array, int size) {
    for(int i = 0; i < (size-1); i++){
        printf("%.1f, ", array[i]);
    }
    printf("%.1f\n", array[size-1]);
}

void cleanup(input_arrays_t *input, output_array_t *output) {
    free(input->x);
    free(input->b);
    free(output->y);
    free(input);
    free(output);
}

void output_array_manipulation(input_arrays_t *input, output_array_t *output) {
    float *y = output->y;
    float *x = input->x;
    float *b = input->b;
    for(int i = 0; i < k*n; i++){
        register float cur_y = y[i];
        register float cur_x = x[i];

        // y[i] = (y[i] + 2*x[i]) / 5
        y[i] = fmaf(0.2f, cur_y, 0.4f * cur_x); // 2 FLOPS
    }
    for(int i = (k+1)*n; i < n*n; i++){
        register float cur_y = y[i];
        register float cur_x = x[i];

        // y[i] = (y[i] + 2*x[i]) / 5
        y[i] = fmaf(0.2f, cur_y, 0.4f * cur_x); // 2 FLOPS
    }

    int i_zero = 0;
    int ceil_n = ((n-m) >> 31) ? (n) : (m);
    int bound = fma(n,k,ceil_n);
    for(int i = k*n; i < bound; ++i){ // k=1 --> i from 2 to 4
        register float cur_y = y[i];
        register float cur_x = x[i];
        register float cur_b = b[i_zero];

        register float z = fmaf(2.0, cur_x, cur_y);
        // y[i] = ((y[i] + 2*x[i]) / 5) + b
        y[i] = fmaf(0.2, z, cur_b); // 2 FLOPS
        i_zero++;
    }
}

int main(int argc, char *argv[]) {
    n = atoi(argv[1]);;
    m = atoi(argv[2]);;
    k = atoi(argv[3]);;

    if(k >= n) {
        printf("Invaild args. k must be less than n.\n");
        return 1;
    }

    printf("n= %d, m= %d, k= %d\n", n,m,k);

    input_arrays_t *input = init_input_arrays();
    output_array_t *output = init_output_array();

    output_array_manipulation(input, output);

    printf("new y array = ");
    print_float_array(output->y, (n*n));

    printf("done");
    cleanup(input, output);
    return 0;  // exit status, 0 means success
}