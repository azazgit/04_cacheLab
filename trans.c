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
#define tileLength 8 // Used to find the best miss rate. Not used in submit func.

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
int min(int n1, int n2){return (n1 > n2) ? n2 : n1;}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {

/* Assignment says can't use arrays. So can swap for 8 local vars if necessary.
 * 
 * Code be tidied up for M = 32 and 64. There is alot of duplicate code, getting 
 * rid of which will require the use of a local var to hold tileLength and
 * assign to it 8 for 32x32 and 4 for 64x64 tiles.
 * 
 * The code for 61x67 does not delay transposing the diagonals. This is because
 * the tileLength of 17 gives the best miss rate. Saving the 17 diag elems of
 * the tile in local vars[or array] will exceed the max of 12 allowed local vars. 
 */ 
    int ii, jj, i, j;
    
    // 32 x 32 matrix. Blocksize 8 gives the best miss rate.
    if (M == 32) {
        int diag[8]; // For diag vals of tile.
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

    // 64 x 64 matrix. Blocksize 4 gives the best miss rate.
    else if (M == 64) {
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
    
    else { // 61 x 67 matrix. Blocksize 17 gives the best miss rate.
        
        // Traverse 4x4 tiles horizontally.
        for(ii = 0; ii < N; ii += 17) {
            for(jj = 0; jj < M; jj += 17){
                
                // For i != j, transfer all Aij to Bji.
                for(i = ii; i < min(N, ii + 17); i++) {
                    for(j = jj; j < min(M, jj + 17); j++){
                        B[j][i] = A[i][j];
                    }
                }
                // Move onto next tile. 
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 * 


char trans_00_desc[] = "Transpose 00";
void trans_00(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            for(i = ii; i < ii + tileLength; i++) {
                for(j = jj; j < jj + tileLength; j++){
                    B[j][i] = A[i][j];
                }
            }
        }
    }
}*/


char trans_3232_id_desc[] = "3232. Inline diag. 343 misses with tilelength 8.";
void trans_3232_id(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    // Traverse 8x8 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    B[j][i] = A[i][j];
                }
            }
          // Move onto next tile. 
        }
    }
}


char trans_3232_dd_desc[] = "3232. Delay diag. 289 misses with tilelength 8.";
void trans_3232_dd(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    int diag[tileLength];

    // Traverse 8x8 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    if (i == j) {
                        diag[i%tileLength] = A[i][i];
                    }
                    else {B[j][i] = A[i][j];}
                }
            }
           // For i == j, transfer all Aii to Bii.
           if (ii == jj) {
              for (i = ii; i < ii + tileLength; i++) {
                 B[i][i] = diag[i%tileLength];
              }
           }
          // Move onto next tile. 
        }
    }
}


char trans_6464_id_desc[] = "6464. Inline diag. Misses 1891.";
void trans_6464_id(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    // Traverse 4x4 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    B[j][i] = A[i][j];
                }
           }
          // Move onto next tile. 
        }
    }
}


char trans_6464_dd_desc[] = "6464. Delay diag. Misses 1797.";
void trans_6464_dd(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    int diag[tileLength];

    // Traverse 4x4 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    if (i == j) {
                        diag[i%tileLength] = A[i][i];
                    }
                    else {B[j][i] = A[i][j];}
                }
            }
           // For i == j, transfer all Aii to Bii.
           if (ii == jj) {
              for (i = ii; i < ii + tileLength; i++) {
                 B[i][i] = diag[i%tileLength];
              }
           }
          // Move onto next tile. 
        }
    }
}


char trans_6167_id_desc[] = "6167. Inline diag. 1950 misses with tilelength 17.";
void trans_6167_id(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;

    // Traverse 4x4 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    B[j][i] = A[i][j];
                }
           }
          // Move onto next tile. 
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
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans_3232_dd, trans_3232_dd_desc);
    //registerTransFunction(trans_6464_dd, trans_6464_dd_desc);
    //registerTransFunction(trans_6167_id, trans_6167_id_desc);
    
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


/* The below functions tried to implement with success the following algo:
 * https://www.youtube.com/watch?v=v9Y2uiThKBY&t=563s
 */
char trans_algo_desc[] = "Youtube algo";
void trans_algo(int M, int N, int A[N][M], int B[M][N])
{
    int ii, jj, i, j;

    // Read Tiles into cache.
    for (ii = 0; ii < 32; ii += tileLength) {
        for (jj = 0; jj <= ii; jj += tileLength) {
            
            // Traverse tile row wise.        
            for (i = ii; i < ii + tileLength; i++) {
                for (j = jj; j <= jj + i; j++) {
                    
                    // Transfer Aij to Bji.
                    B[j][i] = A[i][j];
                    
                    // Transfer Aji to Bij.
                    B[i][j] = A[j][i];
                }
            }
        }
    }    
}

char trans_algo2_desc[] = "Youtube algo 2";
void trans_algo2(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;
    int diag[tileLength];
    
    for (ii = 0; ii < 32; ii += tileLength) {
       for (jj = 0; jj <= ii; jj += tileLength) {

          // Diagonal tiles.
          if (ii == jj) {
             for (i = ii; i < ii + tileLength; i++) {
                for (j = jj; j <= jj + i%tileLength; j++) {
                   if (i == j) {
                      diag[i%tileLength] = A[i][i];
                   }
                   else {
                       B[j][i] = A[i][j];
                   }
                }
             }

             // Aji -> Bij.
             for (i = ii; i < ii + tileLength; i++) {
                 for (j = jj; j < jj + i%tileLength; j++) {
                     B[i][j] = A[j][i];
                 }
             }
          }

          // Non-diagonal tiles.
          else {
              // Aij -> Bji.
              for (i = ii; i < ii + tileLength; i++) {
                  for (j = jj; j < jj + tileLength; j++) {
                      B[j][i] = A[i][j];
                  }
              }
              // Aji -> Bij.
              for (i = ii; i < ii + tileLength; i++) {
                  for (j = jj; j < jj + tileLength; j++) {
                      B[i][j] = A[j][i];
                  }
              }
          }
          // Tile finished. Copy diag elems.
          if (ii == jj) {
              for (i = ii;i < ii + tileLength; i++) {
                  B[i][i] = diag[i%8];
              }
          }
       }
    }
}


char trans_algo64_desc[] = "Youtube algo 64";
void trans_algo64(int M, int N, int A[N][M], int B[M][N]) {
    
    int ii, jj, i, j;
    int diag[tileLength];
    
    for (ii = 0; ii < 64; ii += tileLength) {
       for (jj = 0; jj <= ii; jj += tileLength) {

          // Diagonal tiles.
          if (ii == jj) {
             for (i = ii; i < ii + tileLength; i++) {
                for (j = jj; j <= jj + i%tileLength; j++) {
                   if (i == j) {
                      diag[i%tileLength] = A[i][i];
                   }
                   else {
                       B[j][i] = A[i][j];
                   }
                }
             }

             // Aji -> Bij.
             for (i = ii; i < ii + tileLength; i++) {
                 for (j = jj; j < jj + i%tileLength; j++) {
                     B[i][j] = A[j][i];
                 }
             }
          }

          // Non-diagonal tiles.
          else {
              // Aij -> Bji.
              for (i = ii; i < ii + tileLength; i++) {
                  for (j = jj; j < jj + tileLength; j++) {
                      B[j][i] = A[i][j];
                  }
              }
              // Aji -> Bij.
              for (i = ii; i < ii + tileLength; i++) {
                  for (j = jj; j < jj + tileLength; j++) {
                      B[i][j] = A[j][i];
                  }
              }
          }
          // Tile finished. Copy diag elems.
          if (ii == jj) {
              for (i = ii;i < ii + tileLength; i++) {
                  B[i][i] = diag[i%tileLength];
              }
          }
       }
    }
}
