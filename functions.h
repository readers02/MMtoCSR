#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// ###########################################################
// Do not change this part
typedef struct {
    double *csr_data;   // Array of non-zero values
    int *col_ind;       // Array of column indices
    int *row_ptr;       // Array of row pointers
    int num_non_zeros;  // Number of non-zero elements
    int num_rows;       // Number of rows in matrix
    int num_cols;       // Number of columns in matrix
} CSRMatrix;


void ReadMMtoCSR(const char *filename, CSRMatrix *matrix);
/* <Here you can add the declaration of functions you need.>
<The actual implementation must be in functions.c>
Here what "potentially" you need:
1. "addition" function receiving const CSRMatrix A, const CSRMatrix B, and computing C=A+B
2. "subtraction" function receiving const CSRMatrix A, const CSRMatrix B, and computing C=A-B
3. "multiplication" function receiving const CSRMatrix A, const CSRMatrix B, and computing C=A*B
4. "transpose" function receiving const CSRMatrix A, computing the transpose of A (C=A^T)
		
It is up to you how to save and return the product of each function, matrix C
*/

CSRMatrix add_matrices(const CSRMatrix *A, const CSRMatrix *B);

CSRMatrix subtract_matrices(const CSRMatrix *A, const CSRMatrix *B);

CSRMatrix multiply_matrices(const CSRMatrix *A, const CSRMatrix *B);

CSRMatrix transpose_matrix(const CSRMatrix *A);

void print_matrix(const CSRMatrix *matrix);

int compare(const void *a, const void *b);

#endif
