#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1.mtx> <file2.mtx> <operation> [print]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file1 = argv[1];

    CSRMatrix A, B, C;

	if (argc == 2) {
        // Input: ./main <file.mtx>
        ReadMMtoCSR(file1, &A);
        print_matrix(&A);
        // Free the allocated memory for matrix A
        free(A.csr_data);
        free(A.col_ind);
        free(A.row_ptr);
	}else if (argc == 5){
		// Input: ./main <file1.mtx> <file2.mtx> <operation> <print>
		const char *file2 = argv[2];
		const char *operation = argv[3];
		int print = (argc > 4) ? atoi(argv[4]) : 0;

		ReadMMtoCSR(file1, &A);
		ReadMMtoCSR(file2, &B);

		if (strcmp(operation, "addition") == 0) {
			C = add_matrices(&A, &B);
		} else if (strcmp(operation, "subtraction") == 0) {
			C = subtract_matrices(&A, &B);
		} else if (strcmp(operation, "multiplication") == 0) {
			C = multiply_matrices(&A, &B);
		} else {
			fprintf(stderr, "Unknown operation: %s\n", operation);
			exit(EXIT_FAILURE);
		}

		if (print) {
			print_matrix(&A);
			print_matrix(&B);
			print_matrix(&C);
		}

	    free(A.csr_data);
        free(A.col_ind);
        free(A.row_ptr);
        free(B.csr_data);
        free(B.col_ind);
        free(B.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(C.row_ptr);

    }else if(argc == 4){
		// Input: ./main <file1.mtx> <transpose> <print>
		const char *operation = argv[2];
		int print = (argc > 3) ? atoi(argv[3]) : 0;

		ReadMMtoCSR(file1, &A);

		if (strcmp(operation, "transpose") == 0) {
			C = transpose_matrix(&A);
		} else {
			fprintf(stderr, "Unknown operation: %s\n", operation);
			exit(EXIT_FAILURE);
		}

		if (print) {
			print_matrix(&A);
			print_matrix(&C);
		}

	    free(A.csr_data);
        free(A.col_ind);
        free(A.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(C.row_ptr);
	}else{
        fprintf(stderr, "Invalid arguments.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}