#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"

// Comparator function for sorting
int compare(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    if (ia[0] != ib[0]) return ia[0] - ib[0]; // Sort by row first
    return ia[1] - ib[1]; // Then by column within the same row
}

void ReadMMtoCSR(const char *filename, CSRMatrix *matrix) {
    // Read file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read header
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '%') break;
    }
    sscanf(line, "%d %d %d", &matrix->num_rows, &matrix->num_cols, &matrix->num_non_zeros);

    // Dynamic memory allocation for the 3 CSR format arrays
    matrix->csr_data = (double *)malloc(matrix->num_non_zeros * sizeof(double));
    matrix->col_ind = (int *)malloc(matrix->num_non_zeros * sizeof(int));
    matrix->row_ptr = (int *)malloc((matrix->num_rows + 1) * sizeof(int));

    // Temporary storage for the matrix data
    int *temp_row = (int *)malloc(matrix->num_non_zeros * sizeof(int));
    int *temp_col = (int *)malloc(matrix->num_non_zeros * sizeof(int));
    double *temp_val = (double *)malloc(matrix->num_non_zeros * sizeof(double));

    // Read matrix data into temporary arrays
    int row, col;
    double value;
    int current_non_zero = 0;
    while (fscanf(file, "%d %d %lf", &row, &col, &value) == 3) {
        temp_row[current_non_zero] = row - 1; // Adjust for zero-based indexing
        temp_col[current_non_zero] = col - 1; // Adjust for zero-based indexing
        temp_val[current_non_zero] = value;
        current_non_zero++;
    }

    fclose(file);

    // Sort the temporary arrays
    int *temp_indices = (int *)malloc(2 * matrix->num_non_zeros * sizeof(int));
    for (int i = 0; i < matrix->num_non_zeros; i++) {
        temp_indices[2*i] = temp_row[i];
        temp_indices[2*i + 1] = temp_col[i];
    }
    qsort(temp_indices, matrix->num_non_zeros, 2 * sizeof(int), compare);

    // Fill CSR arrays
    memset(matrix->row_ptr, 0, (matrix->num_rows + 1) * sizeof(int));
    for (int i = 0; i < matrix->num_non_zeros; i++) {
        int r = temp_indices[2*i];
        matrix->row_ptr[r + 1]++;
        matrix->csr_data[i] = temp_val[i];
        matrix->col_ind[i] = temp_indices[2*i + 1];
    }

    // Compute the row pointers
    for (int i = 1; i <= matrix->num_rows; i++) {
        matrix->row_ptr[i] += matrix->row_ptr[i - 1];
    }

    free(temp_row);
    free(temp_col);
    free(temp_val);
    free(temp_indices);
}

CSRMatrix add_matrices(const CSRMatrix *A, const CSRMatrix *B) {
    CSRMatrix C;
    // Initialize C
    C.num_rows = A->num_rows;
    C.num_cols = A->num_cols;
    C.num_non_zeros = 0;
    C.csr_data = NULL;
    C.col_ind = NULL;
    C.row_ptr = (int *)malloc((C.num_rows + 1) * sizeof(int));
    memset(C.row_ptr, 0, (C.num_rows + 1) * sizeof(int));

    int *temp_col_ind = (int *)malloc((A->num_non_zeros + B->num_non_zeros) * sizeof(int));
    double *temp_csr_data = (double *)malloc((A->num_non_zeros + B->num_non_zeros) * sizeof(double));

    int k = 0;

    for (int i = 0; i < C.num_rows; ++i) {
        int startA = A->row_ptr[i];
        int endA = A->row_ptr[i + 1];
        int startB = B->row_ptr[i];
        int endB = B->row_ptr[i + 1];

        while (startA < endA && startB < endB) {
            if (A->col_ind[startA] < B->col_ind[startB]) {
                temp_col_ind[k] = A->col_ind[startA];
                temp_csr_data[k] = A->csr_data[startA];
                ++startA;
            } else if (A->col_ind[startA] > B->col_ind[startB]) {
                temp_col_ind[k] = B->col_ind[startB];
                temp_csr_data[k] = B->csr_data[startB];
                ++startB;
            } else {
                temp_col_ind[k] = A->col_ind[startA];
                temp_csr_data[k] = A->csr_data[startA] + B->csr_data[startB];
                ++startA;
                ++startB;
            }
            ++k;
        }

        while (startA < endA) {
            temp_col_ind[k] = A->col_ind[startA];
            temp_csr_data[k] = A->csr_data[startA];
            ++startA;
            ++k;
        }

        while (startB < endB) {
            temp_col_ind[k] = B->col_ind[startB];
            temp_csr_data[k] = B->csr_data[startB];
            ++startB;
            ++k;
        }

        C.row_ptr[i + 1] = k;
    }

    C.num_non_zeros = C.row_ptr[C.num_rows];
    C.csr_data = (double *)malloc(C.num_non_zeros * sizeof(double));
    C.col_ind = (int *)malloc(C.num_non_zeros * sizeof(int));

    memcpy(C.csr_data, temp_csr_data, C.num_non_zeros * sizeof(double));
    memcpy(C.col_ind, temp_col_ind, C.num_non_zeros * sizeof(int));

    free(temp_col_ind);
    free(temp_csr_data);

    return C;
}

CSRMatrix subtract_matrices(const CSRMatrix *A, const CSRMatrix *B) {
    CSRMatrix C;
    // Initialize C
    C.num_rows = A->num_rows;
    C.num_cols = A->num_cols;
    C.num_non_zeros = 0;
    C.csr_data = NULL;
    C.col_ind = NULL;
    C.row_ptr = (int *)malloc((C.num_rows + 1) * sizeof(int));
    memset(C.row_ptr, 0, (C.num_rows + 1) * sizeof(int));
    
    int *temp_col_ind = (int *)malloc((A->num_non_zeros + B->num_non_zeros) * sizeof(int));
    double *temp_csr_data = (double *)malloc((A->num_non_zeros + B->num_non_zeros) * sizeof(double));

    int k = 0;

    for (int i = 0; i < C.num_rows; ++i) {
        int startA = A->row_ptr[i];
        int endA = A->row_ptr[i + 1];
        int startB = B->row_ptr[i];
        int endB = B->row_ptr[i + 1];

        while (startA < endA && startB < endB) {
            if (A->col_ind[startA] < B->col_ind[startB]) {
                temp_col_ind[k] = A->col_ind[startA];
                temp_csr_data[k] = A->csr_data[startA];
                ++startA;
            } else if (A->col_ind[startA] > B->col_ind[startB]) {
                temp_col_ind[k] = B->col_ind[startB];
                temp_csr_data[k] = -B->csr_data[startB];
                ++startB;
            } else {
                temp_col_ind[k] = A->col_ind[startA];
                temp_csr_data[k] = A->csr_data[startA] - B->csr_data[startB];
                ++startA;
                ++startB;
            }
            ++k;
        }

        while (startA < endA) {
            temp_col_ind[k] = A->col_ind[startA];
            temp_csr_data[k] = A->csr_data[startA];
            ++startA;
            ++k;
        }

        while (startB < endB) {
            temp_col_ind[k] = B->col_ind[startB];
            temp_csr_data[k] = -B->csr_data[startB];
            ++startB;
            ++k;
        }

        C.row_ptr[i + 1] = k;
    }

    C.num_non_zeros = C.row_ptr[C.num_rows];
    C.csr_data = (double *)malloc(C.num_non_zeros * sizeof(double));
    C.col_ind = (int *)malloc(C.num_non_zeros * sizeof(int));

    memcpy(C.csr_data, temp_csr_data, C.num_non_zeros * sizeof(double));
    memcpy(C.col_ind, temp_col_ind, C.num_non_zeros * sizeof(int));

    free(temp_col_ind);
    free(temp_csr_data);

    return C;
}

CSRMatrix multiply_matrices(const CSRMatrix *A, const CSRMatrix *B) {
    __clock_t start, end;
    double cpu_time;
    start=clock();

    CSRMatrix C;
    // Initialize matrix C
    C.num_rows = A->num_rows;
    C.num_cols = B->num_cols;
    C.num_non_zeros = 0;
    C.csr_data = NULL;
    C.row_ptr = (int *) malloc((C.num_rows + 1) * sizeof(int));
    C.col_ind = NULL;

    // Initialize row pointers for matrix C
    C.row_ptr[0] = 0;
    for (int i = 0; i < A->num_rows; ++i) {
        int row_start = A->row_ptr[i];
        int row_end = A->row_ptr[i + 1];

        // Initialize a temp array to store the result row
        double *temp_row = (double *) calloc(B->num_cols, sizeof(double));

        // Iterate through each non-zero element in the current matrix A row
        for (int j = row_start; j < row_end; ++j) {
            int colA = A->col_ind[j];
            double valA = A->csr_data[j];

            // Iterate through corresponding row in B
            int rowB_start = B->row_ptr[colA];
            int rowB_end = B->row_ptr[colA + 1];
            for (int k = rowB_start; k < rowB_end; ++k) {
                int colB = B->col_ind[k];
                double valB = B->csr_data[k];
                temp_row[colB] += valA * valB;
            }
        }

        // Copy temp_row into matrix C
        for (int col = 0; col < B->num_cols; ++col) {
            if (temp_row[col] != 0.0) {
                // Append non-zero value to values array of C
                C.csr_data = (double *) realloc(C.csr_data, (C.num_non_zeros + 1) * sizeof(double));
                C.col_ind = (int *) realloc(C.col_ind, (C.num_non_zeros + 1) * sizeof(int));
                C.csr_data[C.num_non_zeros] = temp_row[col];
                C.col_ind[C.num_non_zeros] = col;
                C.num_non_zeros++;
            }
        }

        // Update row pointer for matrix C
        C.row_ptr[i + 1] = C.num_non_zeros;

        // Free temporary row array
        free(temp_row);
    }

    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken by C function: %f seconds\n", cpu_time);

    return C;
}

CSRMatrix transpose_matrix(const CSRMatrix *A) {
    CSRMatrix C;
    C.num_rows = A->num_cols;
    C.num_cols = A->num_rows;
    C.num_non_zeros = A->num_non_zeros;
    C.csr_data = (double *)malloc(C.num_non_zeros * sizeof(double));
    C.col_ind = (int *)malloc(C.num_non_zeros * sizeof(int));
    C.row_ptr = (int *)malloc((C.num_rows + 1) * sizeof(int));

    int *temp_counts = (int *)calloc(C.num_rows, sizeof(int));
    int *temp_index = (int *)malloc(C.num_non_zeros * sizeof(int));

    // Initialize row_ptr for matrix C
    memset(C.row_ptr, 0, (C.num_rows + 1) * sizeof(int));

    // Count non-zero elements in each column of matrix A
    for (int i = 0; i < A->num_rows; ++i) {
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; ++j) {
            int col = A->col_ind[j];
            ++temp_counts[col];
        }
    }

    // Set up row pointers for matrix C
    C.row_ptr[0] = 0;
    for (int i = 1; i <= C.num_rows; ++i) {
        C.row_ptr[i] = C.row_ptr[i - 1] + temp_counts[i - 1];
    }

    // Place elements into the transposed matrix
    memset(temp_index, -1, C.num_non_zeros * sizeof(int));

    for (int i = 0; i < A->num_rows; ++i) {
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; ++j) {
            int row = i;
            int col = A->col_ind[j];
            int dest = C.row_ptr[col]++;
            temp_index[dest] = row;
            C.csr_data[dest] = A->csr_data[j];
        }
    }

    // Fix row_ptr for C to correct positions
    for (int i = C.num_rows; i > 0; --i) {
        C.row_ptr[i] = C.row_ptr[i - 1];
    }
    C.row_ptr[0] = 0;

    // Set column indices
    for (int i = 0; i < C.num_non_zeros; ++i) {
        C.col_ind[i] = temp_index[i];
    }

    free(temp_index);
    free(temp_counts);

    return C;
}

void print_matrix(const CSRMatrix *matrix) {
    printf("Number of non-zeros: %d\n", matrix->num_non_zeros);
    printf("Row Pointer: ");
    for (int i = 0; i <= matrix->num_rows; i++) {
        printf("%d ", matrix->row_ptr[i]);
    }
    printf("\nColumn Index: ");
    for (int i = 0; i < matrix->num_non_zeros; i++) {
        printf("%d ", matrix->col_ind[i]);
    }
    printf("\nValues: ");
    for (int i = 0; i < matrix->num_non_zeros; i++) {
        printf("%.4f ", matrix->csr_data[i]);
    }
    printf("\n");
}