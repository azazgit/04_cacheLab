/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;
    for(ii = 0; ii < N; ii += 8) {
        for(jj = 0; jj < M; jj += 8){
            for(i = ii; i < ii + 8; i++) {
                for(j = jj; j < jj + 8; j++){
                    if(ii + i == jj + j) {continue;} // ignore diagonals.
                    B[j][i] = A[i][j];
                }
            }
        }
    }

    // Add diagonals.
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (i == j) {
                B[j][i] = A[i][j];
            }
        }
    }
    if (is_transpose(M, N, A, B)) {
        printf("transposed correctly.");
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

char trans_00_desc[] = "Transpose 00";
void trans_00(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;
    for(ii = 0; ii < N; ii += 8) {
        for(jj = 0; jj < M; jj += 8){
            for(i = ii; i < ii + 8; i++) {
                for(j = jj; j < jj + 8; j++){
                    B[j][i] = A[i][j];
                }
            }
        }
    }
}

char trans_01_desc[] = "Transpose 01";
void trans_01(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    int diag[8];

    // Traverse 8x8 tiles horizontally.
    for(ii = 0; ii < N; ii += 8) {
        for(jj = 0; jj < M; jj += 8){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < ii + 8; i++) {
                for(j = jj; j < jj + 8; j++){
                    if (i == j) {
                        diag[i%8] = A[i][i];
                    }
                    else {B[j][i] = A[i][j];}
                }
            }
           // For i == j, transfer all Aii to Bii.
           if (ii == jj) {
              for (i = ii; i < ii + 8; i++) {
                 B[i][i] = diag[i%8];
              }
           }
          // Move onto next tile. 
        }
    }
}

char trans_64x64_4x4_desc[] = "64x64. 4x4. Delay diag. ";
void trans_64x64_4x4(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    int diag[4];

    // Traverse 4x4 tiles horizontally.
    for(ii = 0; ii < N; ii += 4) {
        for(jj = 0; jj < M; jj += 4){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < ii + 4; i++) {
                for(j = jj; j < jj + 4; j++){
                    if (i == j) {
                        diag[i%4] = A[i][i];
                    }
                    else {B[j][i] = A[i][j];}
                }
            }
           // For i == j, transfer all Aii to Bii.
           if (ii == jj) {
              for (i = ii; i < ii + 4; i++) {
                 B[i][i] = diag[i%4];
              }
           }
          // Move onto next tile. 
        }
    }
}
char trans_algo_desc[] = "Youtube algo";
void trans_algo(int M, int N, int A[N][M], int B[M][N])
{
    int ii, jj, i, j;

    // Read Tiles into cache.
    for (ii = 0; ii < 4; ii++) {
        for (jj = 0; jj <= ii; jj++) {
            
            // Traverse tile row wise.        
            for (i = ii*8; i < ii*8 + 8; i++) {
                for (j = jj*8; j < jj*8 + 8; j++) {
                    
                    // Transfer Aij to Bji.
                    B[j][i] = A[i][j];
                    
                    // Transfer Aji to Bij.
                    B[i][j] = A[j][i];
                }
            }
        }
    }    
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}



/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    //registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 
    
    //registerTransFunction(trans_00, trans_00_desc);
    //registerTransFunction(trans_01, trans_01_desc);
    registerTransFunction(trans_02, trans_02_desc);
    registerTransFunction(trans_03, trans_03_desc);
    registerTransFunction(trans_04, trans_04_desc);
    //registerTransFunction(trans_algo, trans_algo_desc);
    
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

