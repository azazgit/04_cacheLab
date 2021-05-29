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

    /* Note: Using tilelength local var makes the code neater. Without this var
     * there will a lot of duplicate code written in different conditionals for
     * 32x32 and 61x67 matrices.
     * With this var the no of local vars is 13, 1 over the max allowed.
     */ 
    int tileLength; // Divide matrix into square tiles with length tileLength.
    int ii;         // Traverse matrices' A & B's tiles row-wise.
    int jj;         // Traverse matrices' A & B's tiles column-wise.
    int i;          // Traverse each tile's rows.
    int j;          // Traverse each tile's columns.
    int t0, t1, t2, t3, t4, t5, t6, t7; // For local storage.
    
    // The same blocking strategy works for both 32x32 and 61x67 matrices.
    if (M == 32 || M == 61) {
        
        // For 32x32 tile length of 8 gives the best miss rate.
        if (M == 32) {tileLength = 8;}
        else {tileLength = 17;} // For 61x67 17 gives the best miss rate.

        // Traverse matrices by tiles row-wise.
        for(ii = 0; ii < N; ii += tileLength) {
            for(jj = 0; jj < M; jj += tileLength){
            
                // Traverse each tile row-wise.
                for(i = ii; i < min(N, ii + tileLength); i++) {
                    for(j = jj; j < min(M, jj + tileLength); j++){
                    
                        // Save diagonals.
                        if (i == j) {
                            t0 = A[i][i];
                            t1 = i;
                        }
                    
                        // Copy all non-diagonal Aij to Bji.
                        else {B[j][i] = A[i][j];}
                    }

                    // Copy diag only after the whole row is copied.
                    if (ii == jj){B[t1][t1] = t0;}
                }
                // Move onto next tile.
            }
        }
    }
    
    // 64x64. Use 4x4 tiles inside of 8x8 tiles. 
    else {

        tileLength = 8;
        // Traverse outer tiles horizontally.
        for(ii = 0; ii < N; ii += tileLength) {
            for(jj = 0; jj < M; jj += tileLength){
                
                // Traverse each 8x8 tile in 4 tiles of 4x4.
                for(i = ii; i < ii + 4; i++) {
                    
                    // Store row of tile in local vars.
                    t0 = A[i][jj];
                    t1 = A[i][jj+1];
                    t2 = A[i][jj+2];
                    t3 = A[i][jj+3];
                    t4 = A[i][jj+4];
                    t5 = A[i][jj+5];
                    t6 = A[i][jj+6];
                    t7 = A[i][jj+7];

                    // Top left mini tile of B gets its values.
                    B[jj][i] = t0;
                    B[jj+1][i] = t1;
                    B[jj+2][i] = t2;
                    B[jj+3][i] = t3;
                    
                    /* Top right mini tile of B is used as temp storage.
                     * At the end of the for loop, this tile will hold the 
                     * transpose of top left mini tile of A.This tile's 
                     * contents identically match what needs to 
                     * go in the bottom left mini tile of B.
                     */  
                    B[jj][i+4] = t4;
                    B[jj+1][i+4] = t5;
                    B[jj+2][i+4] = t6;
                    B[jj+3][i+4] = t7;
                }
                
                // Update bottom left and bottom right mini tiles.
                for (i = jj; i < jj + 4; i++) {

                    /* Prepare columns of bottom left tile of A for transfer to
                     * top right tile of B.
                     */ 
                    t4 = A[ii+4][i];
                    t5 = A[ii+5][i];
                    t6 = A[ii+6][i];
                    t7 = A[ii+7][i];

                    /* Prepare the transfer from B's top right to its bottom
                     * left tile.
                     */
                    t0 = B[i][ii+4];
                    t1 = B[i][ii+5];
                    t2 = B[i][ii+6];
                    t3 = B[i][ii+7];

                    // B's top right tile's row i is free to take its values.
                    B[i][ii+4] = t4;
                    B[i][ii+5] = t5;
                    B[i][ii+6] = t6;
                    B[i][ii+7] = t7;

                    // Transfer into B's bottom left tile.
                    B[i+4][ii] = t0;
                    B[i+4][ii+1] = t1;
                    B[i+4][ii+2] = t2;
                    B[i+4][ii+3] = t3;

                    // Bottom right tiles of A and B.
                    B[i+4][ii+4] = A[ii+4][i+4];
                    B[i+4][ii+5] = A[ii+5][i+4];
                    B[i+4][ii+6] = A[ii+6][i+4];
                    B[i+4][ii+7] = A[ii+7][i+4];

                }
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
    int tileLength = 8;
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
    
    int tileLength = 8;
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

char trans_32_diag_desc[] = "32. Copy diag after each most inner iteration. 287 misses.";
void trans_32_diag(int M, int N, int A[N][M], int B[M][N]) {

    int tileLength = 8;
    int ii, jj, i, j, temp, d;

    // Traverse 8x8 tiles horizontally.
    for(ii = 0; ii < N; ii += tileLength) {
        for(jj = 0; jj < M; jj += tileLength){
            
            // For i != j, transfer all Aij to Bji.
            for(i = ii; i < min(N, ii + tileLength); i++) {
                for(j = jj; j < min(M, jj + tileLength); j++){
                    if (i == j) {
                        temp = A[i][i];
                        d = i;
                    }
                    else {B[j][i] = A[i][j];}
                }
                if (ii == jj){B[d][d] = temp;}
            }
          // Move onto next tile. 
        }
    }
}

char trans_6464_id_desc[] = "6464. Inline diag. Misses 1891.";
void trans_6464_id(int M, int N, int A[N][M], int B[M][N]) {
    
    int tileLength = 4;
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
    
    int tileLength = 4;
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
    
    int tileLength = 17;
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
    //registerTransFunction(trans_32_diag, trans_32_diag_desc);
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
*/
