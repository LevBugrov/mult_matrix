#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <fstream>

int** randomMatrix(int n, int m);
void res_zeros();
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
    int n = 256;
    row1 = n;
    col1 = n;
    col2 = n;

    matrix1 = randomMatrix(row1, col1);
    matrix2 = randomMatrix(col1, col2);
    result = randomMatrix(row1, col2);
    res_zeros();

    thread_count = 128;
    clock_t time;
    std::ofstream fout;
    fout.open("results.txt");
    // print_matrix(matrix1, row1, col1);
    // print_matrix(matrix2, col1, col2);
    
    // линейное умножение
    time = clock();
    multLin();
    //print_matrix(result, row1, col2);
    fout << clock()-time<<"\n";
    res_zeros();

    pthread_t *thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    

    for(thread_count=1; thread_count<=9; thread_count*=2){
        // умножение по строкам
        time = clock();
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_create(&thread_handles[thread], NULL, multRow, (void*) thread);
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_join(thread_handles[thread], NULL);
        
        print_matrix(result, row1, col2);
        fout << clock()-time<<" ";
        res_zeros();
        

        // умножение по столбцам
        time = clock();
        pthread_mutex_init(&mutex, NULL);
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_create(&thread_handles[thread], NULL, multColumn, (void*) thread);
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_join(thread_handles[thread], NULL);
        
        pthread_mutex_destroy(&mutex);
        print_matrix(result, row1, col2);
        fout << clock()-time<<" ";
        res_zeros();

        // умножение по блокам
        time = clock();
        pthread_mutex_init(&mutex, NULL);
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_create(&thread_handles[thread], NULL, multBlock, (void*) thread);
        for(long thread = 0; thread < thread_count; ++thread)
            pthread_join(thread_handles[thread], NULL);
        
        pthread_mutex_destroy(&mutex);
        print_matrix(result, row1, col2);
        fout << clock()-time<<" ";
        res_zeros();
        fout <<"\n";
    }
    

    fout.close();
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

void res_zeros(){
    for (size_t i = 0; i < row1; i++)
        for (size_t j = 0; j < col2; j++)
            result[i][j] = 0;    
}

void print_matrix(int** matrix, int n, int m) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j){
            std::cout<<matrix[i][j]<<"\t";
            }
        std::cout<<"\n";
    }
    std::cout<<"\n";
}

void multLin() {
    for (int i = 0; i < row1; i++)
        for (int j = 0; j < col2; j++){
            result[i][j] = 0;
            for (int k = 0; k < col1; k++)
                result[i][j] += matrix1[i][k] * matrix2[k][j];
        }
}


void* multRow(void* arg){
    long th_num = (long)(arg);    
    long block_size = row1 / thread_count;
    long row_first = th_num * block_size;
    long row_last = (th_num+1) * block_size;
        
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

void* multColumn(void* arg){
    long th_num = (long)(arg);
    long block_size = col1/ thread_count;
    long col_first = th_num * block_size;
    long col_last = (th_num+1) * block_size;
    
    for (int i = col_first; i < col_last; ++i)
        for (int j = 0; j < col2; ++j) 
            for (int k = 0; k < row1; ++k){
                long buff = result[k][j] + matrix1[k][i] * matrix2[i][j];
                pthread_mutex_lock(&mutex);
                result[k][j] = buff;
                pthread_mutex_unlock(&mutex);
            }
    return NULL;
}

void* multBlock(void* arg){
    long i, j, k, th_num, portion_size_columns, portion_size_rows, column_start, col_last, num_of_blocks, row_start, row_end, num_of_column_blocks;
    th_num = (long)(arg);
    num_of_blocks = (long)pow(2, thread_count);
    num_of_column_blocks = (long)num_of_blocks/2;
    if(num_of_blocks == 1) {num_of_column_blocks = 1;}
    portion_size_columns = (long)(col2/num_of_column_blocks);
    portion_size_rows = (long)(row1/2);
    if(num_of_blocks == 1) {portion_size_rows = row1;}
    column_start = (th_num%num_of_column_blocks) * portion_size_columns;
    col_last = ((th_num)%num_of_column_blocks + 1) * portion_size_columns;
    row_start = (int)(th_num/num_of_column_blocks) * portion_size_rows;
    row_end = (int)(((th_num)/num_of_column_blocks)+1) * portion_size_rows;
    pthread_mutex_lock(&mutex);
    for(int k=row_start; k < row_end; k++) {
            for(int h=0; h < col2; h++) {
                for(int l=column_start; l<col_last; l++) {
                    result[k][h] += matrix1[k][l]*matrix2[l][h];
                }
            }
        }
    pthread_mutex_unlock(&mutex);
    return NULL;
}