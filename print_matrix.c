#include <stdio.h>
/**
 * Find minimum between two numbers.
 */
int min(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}

int main() {
    int M = 11;
    int N = 13;
    int stride = 4;
    int A[M][M];
    int B[M][M];
    int diag[8];
    int mod = 0;
    
    int ii, jj, i, j, tmp; 
    
    for (i = 0; i < M; i++) {
        for (j = 0; j <M ; j++) {
            A[i][j] = i+j;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j <M ; j++) {
            B[i][j] = 0;
        }
    }

    for(ii = 0; ii < N; ii += stride) {                                              
         for(jj = 0; jj < M; jj += stride){
             for(i = ii; i < min(N, ii+stride); i++) {
               for(j = jj; j < min(N, jj+stride); j++) {
                    printf("%d, %d, %d, %d\n", ii, jj, i, j);
                     //B[j][i] = A[i][j];
                 }
             }
         }
    }
    /*
             // Diagonal tiles.
             if (ii == jj) {
                 for(i = ii; i < ii + 8; i++) {
                     for(j = jj; j <= jj + i%8; j++){
                         if (i == j) {
                             diag[i%8] = A[i][i];
                         }
                         else {
                             B[j][i] = A[i][j];
                             B[i][j] = A[j][i];
                         }
                     }
                 }
             }
             
             // Non-diagonal tiles.
             else { continue;
                 printf("non diag tile: (%d, %d)\n", ii, jj);
                 for (i = ii; i < ii + 8; i++){
                     for (j = jj; j < jj + 8; j++) {
                         B[j][i] = A[i][j];
                         printf("B[%d][%d] at: %d\t",j , i , B[j][i]);
                         
                         B[i][j] = A[j][i];
                         printf("B[%d][%d] at: %d\n",i , j , B[i][j]); 
                     }
                 }
             }
             // Tile finished. Do the diagonals now.
             if (ii == jj) {
                 for (i = ii; i < ii + 8; i++) {
                     B[i][i] = diag[i%8];
                     printf("B[%d][%d]: %d\n",i , i, B[i][i]); 
                 }
             }
        }
    }*/
    return 0;
}
    
    /* trans_01
    int diag[8];

    for (ii = 0; ii < 32; ii += 8){
        for (jj = 0; jj < 32; jj += 8){
            
            // For i != j, transfer all Aij to Bji.
            for (i = ii; i < ii+8; i++) {
                for (j = jj; j < jj + 8; j++) {
                    if (i == j) {
                  //      printf("A[%d][%d]: %d\n", i, i, A[i][i]);
                    }
                    else {
                        B[j][i] = A[i][j];
                    //    printf("B[%d][%d]: %d\n",j , i , B[j][i]);
                    }
                }
            }
            //printf("ii = %d, jj = %d\n", ii, jj); 
            // For i == j, transfer all Aii to Bii.
            if (ii == jj) {
                printf("ii = %d, jj = %d\n", ii, jj); 
                for (i = ii; i < ii + 8; i++) {
                    //printf("i: %d and imod7: %d\n", i, i%8);
                    printf("B[%d][%d] bf diag tr: %d\n", i, i, B[i][i]);
                    B[i][i] = A[i][i];
                    printf("B[%d][%d] af diag tr: %d\n", i, i, B[i][i]);
                }
            }
            // Move to next tile.
         }
    }
    */
/* Below counts the upper diagonal indices.
    for (ii=0;ii<M;ii+=8){
        for (jj=0;jj<M;jj+=8){
            for (i=ii;i<ii+8;i++){
                for (j=jj+i+1;j<jj+8;j++) {
                    printf("ii: %d, jj: %d, i: %d, j: %d\n", ii, jj, i, j);
                }
            }
        }
    }
*/
                    
                    //temp=A[i][j];
                    //A[i][j]=A[j][i];
                    //A[j][i]=temp;
    
    
/* Function for 64x64 4x8 and 4x4 i think.    
    
    for(ii = 0; ii < M; ii += 4) {
        for(jj = 0; jj < M; jj += 8) {
            for(i = ii; i < ii + 4; i++){
                for(j = jj; j < jj + 8; j++){
                    if (i == j) {
                        //printf("**(%d, %d, %d, %d)\t", ii, jj, i, j);
                        mod = i%4;
                        diag[mod] = A[i][i];
                        //printf("diag[%d]: %d\n", mod, diag[i%8]);
                    }
                    else {
                        //printf("ii: %d, jj: %d, i: %d, j: %d\n", ii, jj, i, j);
                    }
                }
            }
            if ((ii == jj) || (ii - 4 == jj)) {
                //printf("ii: %d, jj: %d\n\n", ii, jj);
                for (i = ii; i < ii + 4; i++) {
                    //printf("i: %d and imod7: %d\n", i, i%8);
                    //printf("B[%d][%d] bf diag tr: %d\n", i, i, B[i][i]);
                    B[i][i] = diag[i%4];
                    printf("B[%d][%d] af diag tr: %d\n", i, i, B[i][i]);
                }        
            }
        }
    }
    for(ii = 0; ii < M; ii += 4) {
        for(jj = 0; jj < M; jj += 8) {
            for(i = ii; i < ii + 4; i++){
                for(j = jj; j < jj + 8; j++){
                    if (i == j) {
                        //printf("**(%d, %d, %d, %d)\t", ii, jj, i, j);
                        mod = i%4;
                        diag[mod] = A[i][i];
                        //printf("diag[%d]: %d\n", mod, diag[i%8]);
                    }
                    else {
                        //printf("ii: %d, jj: %d, i: %d, j: %d\n", ii, jj, i, j);
                    }
                }
            }
            if ((ii == jj) || (ii - 4 == jj)) {
                //printf("ii: %d, jj: %d\n\n", ii, jj);
                for (i = ii; i < ii + 4; i++) {
                    //printf("i: %d and imod7: %d\n", i, i%8);
                    //printf("B[%d][%d] bf diag tr: %d\n", i, i, B[i][i]);
                    B[i][i] = diag[i%4];
                    printf("B[%d][%d] af diag tr: %d\n", i, i, B[i][i]);
                }        
            }
        }
    }
    for(ii = 0; ii < M; ii += 4) {
        for(jj = 0; jj < M; jj += 8) {
            for(i = ii; i < ii + 4; i++){
                for(j = jj; j < jj + 8; j++){
                    if (i == j) {
                        //printf("**(%d, %d, %d, %d)\t", ii, jj, i, j);
                        mod = i%4;
                        diag[mod] = A[i][i];
                        //printf("diag[%d]: %d\n", mod, diag[i%8]);
                    }
                    else {
                        //printf("ii: %d, jj: %d, i: %d, j: %d\n", ii, jj, i, j);
                    }
                }
            }
            if ((ii == jj) || (ii - 4 == jj)) {
                //printf("ii: %d, jj: %d\n\n", ii, jj);
                for (i = ii; i < ii + 4; i++) {
                    //printf("i: %d and imod7: %d\n", i, i%8);
                    //printf("B[%d][%d] bf diag tr: %d\n", i, i, B[i][i]);
                    B[i][i] = diag[i%4];
                    printf("B[%d][%d] af diag tr: %d\n", i, i, B[i][i]);
                }        
            }
        }
    }
    for(ii = 0; ii < M; ii += 4) {
        for(jj = 0; jj < M; jj += 8) {
            for(i = ii; i < ii + 4; i++){
                for(j = jj; j < jj + 8; j++){
                    if (i == j) {
                        //printf("**(%d, %d, %d, %d)\t", ii, jj, i, j);
                        mod = i%4;
                        diag[mod] = A[i][i];
                        //printf("diag[%d]: %d\n", mod, diag[i%8]);
                    }
                    else {
                        //printf("ii: %d, jj: %d, i: %d, j: %d\n", ii, jj, i, j);
                    }
                }
            }
            if ((ii == jj) || (ii - 4 == jj)) {
                //printf("ii: %d, jj: %d\n\n", ii, jj);
                for (i = ii; i < ii + 4; i++) {
                    //printf("i: %d and imod7: %d\n", i, i%8);
                    //printf("B[%d][%d] bf diag tr: %d\n", i, i, B[i][i]);
                    B[i][i] = diag[i%4];
                    printf("B[%d][%d] af diag tr: %d\n", i, i, B[i][i]);
                }        
            }
        }
    }

    Youtube algo 
    for(ii = 0; ii < 32; ii += 8) {                                              
         for(jj = 0; jj <= ii; jj += 8){
             
             // Diagonal tiles.
             if (ii == jj) {
                 for(i = ii; i < ii + 8; i++) {
                     for(j = jj; j <= jj + i%8; j++){
                         if (i == j) {
                             diag[i%8] = A[i][i];
                         }
                         else {
                             B[j][i] = A[i][j];
                             B[i][j] = A[j][i];
                         }
                     }
                 }
             }
             
             // Non-diagonal tiles.
             else { continue;
                 printf("non diag tile: (%d, %d)\n", ii, jj);
                 for (i = ii; i < ii + 8; i++){
                     for (j = jj; j < jj + 8; j++) {
                         B[j][i] = A[i][j];
                         printf("B[%d][%d] at: %d\t",j , i , B[j][i]);
                         
                         B[i][j] = A[j][i];
                         printf("B[%d][%d] at: %d\n",i , j , B[i][j]); 
                     }
                 }
             }
             // Tile finished. Do the diagonals now.
             if (ii == jj) {
                 for (i = ii; i < ii + 8; i++) {
                     B[i][i] = diag[i%8];
                     printf("B[%d][%d]: %d\n",i , i, B[i][i]); 
                 }
             }
        }
    }
*/
