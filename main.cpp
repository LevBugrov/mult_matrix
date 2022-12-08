#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int** randomMatrix(int n, int m);
void print_matrix(int** matrix, int n, int m);
void multLin();

void* multRow(void *arg);
void* multColumn(void *arg);
void* multBlock(void *arg);

int row1;
int col1;
int col2;
int** matrix1;
int** matrix2;
int** result;

int thread_count;
pthread_mutex_t mutex;

int main(){
    row1 = 3;
    col1 = 4;
    col2 = 4;

    matrix1 = randomMatrix(row1, col1);
    matrix2 = randomMatrix(col1, col2);
    result = randomMatrix(row1, col2);

    thread_count = 2;

    pthread_t *thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);

    // умножение по строкам
    for(long thread = 0; thread < thread_count; ++thread)
        pthread_create(&thread_handles[thread], NULL, multRow, (void*) thread);
    for(long thread = 0; thread < thread_count; ++thread)
        pthread_join(thread_handles[thread], NULL);
    
    pthread_mutex_destroy(&mutex);

    print_matrix(matrix1, row1, col1);
    print_matrix(matrix2, col1, col2);
    print_matrix(result, row1, col2);


    result = randomMatrix(row1, col2);
    multLin();
    print_matrix(result, row1, col2);
    
    return 0;
}


int** randomMatrix(int n, int m){
    srand(time(NULL));
    int** A = new int*[n];
    for (size_t i = 0; i < n; i++){
        A[i] = new int[m];
        for (size_t j = 0; j < m; j++)
            A[i][j] = rand()%100;    
        }
    return A;
}

void print_matrix(int** matrix, int n, int m) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j){
            std::cout<<matrix[i][j]<<"\t";
            }
        printf("\n");
    }
    printf("\n");
}

void multLin() {
    for (int i = 0; i < row1; i++){

        for (int j = 0; j < col2; j++){
            result[i][j] = 0;
            for (int k = 0; k < col1; k++)
                result[i][j] += matrix1[i][k] * matrix2[k][j];
        }
  }
}

void *multRow(void* arg){
    long th_num = (long)(arg);    
    long block_size = row1 / thread_count;
    long row_first = th_num * block_size;
    long row_last = (th_num+1) * block_size;
    if ((row1-1 == row_last) && (th_num == thread_count-1))
        row_last = row1;
        
    int sum;
    for (int i = row_first; i < row_last; i++) 
        for (int j = 0; j < col1; j++){
            sum = 0;
            for (int k = 0; k < col2; k++)
                sum += matrix1[i][k] * matrix2[k][j];
            result[i][j] = sum;
        }

    return NULL;
}

void *multColumn( void *arg ){
    long i, j, k, th_num, portion_size, column_start, column_end;
    int * sum = (int *) malloc( row1 * sizeof(double) );
    th_num = *(long *)(arg);
    portion_size = col1/ thread_count;
    column_start = th_num * portion_size;
    column_end = (th_num+1) * portion_size;

    if (th_num == thread_count-1 && column_end != col1) {column_end = col1;}
    pthread_mutex_lock(&mutex);
        for (i = column_start; i < column_end; ++i) {
            for (j = 0; j < col2; ++j) {
                for (k = 0; k < row1; ++k) {
                    result[k][j] += matrix1[k][i] * matrix2[ i ][ j ];
                }
            }
    }
    pthread_mutex_unlock(&mutex);
}

void *multBlock( void *arg )
{
    long i, j, k, th_num, portion_size_columns, portion_size_rows, column_start, column_end, num_of_blocks, row_start, row_end, num_of_column_blocks;
    th_num = *(long *)(arg);
    num_of_blocks = (long)pow(2, thread_count);
    num_of_column_blocks = (long)num_of_blocks/2;
    if(num_of_blocks == 1) {num_of_column_blocks = 1;}
    portion_size_columns = (long)(col2/num_of_column_blocks);
    portion_size_rows = (long)(row1/2);
    if(num_of_blocks == 1) {portion_size_rows = row1;}
    column_start = (th_num%num_of_column_blocks) * portion_size_columns;
    column_end = ((th_num)%num_of_column_blocks + 1) * portion_size_columns;
    row_start = (int)(th_num/num_of_column_blocks) * portion_size_rows;
    row_end = (int)(((th_num)/num_of_column_blocks)+1) * portion_size_rows;
    pthread_mutex_lock(&mutex);
    for(int k=row_start; k < row_end; k++) {
            for(int h=0; h < col2; h++) {
                for(int l=column_start; l<column_end; l++) {
                    result[k][h] += matrix1[k][l]*matrix2[l][h];
                }
            }
        }
    pthread_mutex_unlock(&mutex);
}