#include <iostream>
#include <stack>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
using namespace std;
int SIZE;
int THREAD_COUNT;
int** solutionGrid;
void printSudoku(int **sudoku){
    printf("The Sudoku contains:\n");
    for (int i = 0; i < SIZE; i++){
        for (int j = 0; j < SIZE; j++){
            printf("%d  ", sudoku[i][j]);
        }
        printf("\n");
    }
}

void printPossibilityMatrix(int ***possibilityMatrix, int **indexes){
    for (int i = 0; i < SIZE; i++){
        printf("{");
        for (int j = 0; j < SIZE; j++){
            printf("  {");
            for (int k = 0; k < indexes[i][j]; k++){
                printf("%d,", possibilityMatrix[i][j][k]);
            }
            printf("},\n");
        }
        printf("  }\n");
    }
}

int checkSquare(int **sudoku, int num, int row, int col){
    int squareSize = sqrt(SIZE);
    int rowStart = row - row % squareSize;
    int colStart = col - col % squareSize;
    int result = 1;
    for (int i = rowStart; i < rowStart + squareSize; i++){
        for (int j = colStart; j < colStart + squareSize; j++){
            if (sudoku[i][j] == num){
                result = 0;
            }
        }
    }
    return result;
}

int solved(int **sudoku, int *rows, int *columns){
    for (int i = 0; i < SIZE; i++){
        for (int j = 0; j < SIZE; j++){
            if (sudoku[i][j] == 0){
                *(rows) = i;
                *(columns) = j;
                return 0;
            }
        }
    }
    return 1;
}
int checkSudoku(int **sudoku, int num, int row, int col){
    int result = 1;
    #pragma omp parallel num_threads(THREAD_COUNT)
    for (int i = 0; i < SIZE; i++){
        if (num == sudoku[i][col]){
            result = 0;
        }
    }
    if (!result){
        return 0;
    }
    #pragma omp parallel num_threads(THREAD_COUNT)
    for (int j = 0; j < SIZE; j++){
        if (num == sudoku[row][j]){
            result = 0;
        }
    }
    if (!result){
        return 0;
    }
    if (checkSquare(sudoku, num, row, col) == 0){
        return 0;
    }
    return result;
}
int verifySudoku(int** sudoku){
    int result = 1;
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(sudoku[i][j] == 0){
                continue;
            }
            int num = sudoku[i][j];
            sudoku[i][j]=0;
            if(!checkSudoku(sudoku,num,i,j)){
                result = 0;
                return result;
            }
            sudoku[i][j] = num;
        }
    }
    return result;
}

void resetIndexArray(int** indexes){
    for(int i=0;i<SIZE;i++){
        #pragma omp parallel for num_threads(THREAD_COUNT)
        for(int j=0;j<SIZE;j++){
            indexes[i][j]=0;
        }
    }
}

void performElimination(int** sudoku,int***possibility,int**indexes){
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(sudoku[i][j] != 0){
                continue;
            }
            #pragma omp parallel for num_threads(THREAD_COUNT)
            for (int k = 1; k <= SIZE; k++){
                if (checkSudoku(sudoku, k, i, j)){
                    possibility[i][j][indexes[i][j]] = k;
                    indexes[i][j]++;
                }
            }
            if(indexes[i][j] == 1){
                sudoku[i][j] = possibility[i][j][indexes[i][j]-1];
                resetIndexArray(indexes);
                i=-1;
                break;
            }
        }
    }
}

void findMinimumSizePossibility(int** indexes,int *row,int* col){
    int minimum=SIZE+1;
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(indexes[i][j] == 0){
                continue;
            }
            if(indexes[i][j]<minimum){
                minimum = indexes[i][j];
                *(row) = i;
                *(col) = j;
            }
        }
    }
}

stack<int**> generateNewBoards(int **sudoku,int ***possibility,int **indexes,int row,int col){
    stack<int**> grids;
    
    for(int i=0;i<indexes[row][col];i++){
        int** temp = new int*[SIZE];
        for(int j=0;j<SIZE;j++){
            temp[j] = new int[SIZE];
            memcpy(temp[j], sudoku[j], SIZE * sizeof(int));
        }
        temp[row][col] = possibility[row][col][i];
        grids.push(temp);
    }
    return grids;
}

int** generateIndexInit(int ** indexes){
    int ** myIndex = new int*[SIZE];
    for(int i=0;i<SIZE;i++){
        myIndex[i] = new int[SIZE];
        for(int j=0;j<SIZE;j++){
            myIndex[i][j] = 0;
        }
    }
    return myIndex;
}

int*** generatePossibilityInit(int*** possibility){
    int ***myPossibility = new int**[SIZE];
    for(int i=0;i<SIZE;i++){
        myPossibility[i] = new int*[SIZE];
    }
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            myPossibility[i][j] = new int[SIZE];
        }
    }
    return myPossibility;
}
int solveSudoku(int*** possibility,int** sudoku,int** indexes){
    int rows,columns;
    if(solved(sudoku,&rows,&columns)){
        solutionGrid = sudoku;
        return 1;
    }else{
        performElimination(sudoku,possibility,indexes);
        if(solved(sudoku,&rows,&columns)){
            solutionGrid = sudoku;
            return 1;
        }
        int r,c;
        findMinimumSizePossibility(indexes,&r,&c);
        stack<int**> grids = generateNewBoards(sudoku,possibility,indexes,r,c);
        int stackSize = grids.size();
        #pragma omp parallel for num_threads(indexes[r][c])
        for(int i=0;i<stackSize;i++){
            cout<<"Creating Board["<<r<<"]["<<c<<"]"<<endl;
            int **mySudoku;
            #pragma omp critical
            {
                mySudoku = grids.top();
                grids.pop();
            }
            int** myIndex = generateIndexInit(indexes);
            int*** myPossibility = generatePossibilityInit(possibility);

            if(verifySudoku(mySudoku)){
                solveSudoku(myPossibility,mySudoku,myIndex);
            }else{
                for (int i = 0; i < SIZE; i++){
                    delete[] myIndex[i];
                    delete[] mySudoku[i];
                    for (int j = 0; j < SIZE; j++){
                        delete[] myPossibility[i][j];
                    }
                    delete[] myPossibility[i];
                }
                delete myPossibility;
                delete myIndex;
                delete mySudoku;
                myPossibility = NULL;
                myIndex = NULL;
                mySudoku = NULL;
            }
            
        }
        cout<<"Eliminating Board["<<r<<"]["<<c<<"]"<<endl;
    }
    return 0;

}


int main(int argc, char *argv[]) {
    if (!argv[1] || !argv[2]) {
        cout << "Please run the code as ./main <board_size> <thread_count>" << endl;
        return 0;
    }

    SIZE = stoi(argv[1]);
    THREAD_COUNT = stoi(argv[2]);
    int ***possibilityMatrix = new int **[SIZE];
    int **indexValue = new int *[SIZE];
    int **sudoku = new int *[SIZE];

    #pragma omp parallel for num_threads(THREAD_COUNT)
    for (int i = 0; i < SIZE; i++) {
        sudoku[i] = new int[SIZE];
        possibilityMatrix[i] = new int *[SIZE];
        indexValue[i] = new int[SIZE];
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            possibilityMatrix[i][j] = new int[SIZE];
            indexValue[i][j] = 0;
            #pragma omp parallel for num_threads(THREAD_COUNT)
            for (int k = 0; k < SIZE; k++) {
                possibilityMatrix[i][j][k] = 0;
            }
        }
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cin >> sudoku[i][j];
        }
    }
    double start = omp_get_wtime();
    solveSudoku(possibilityMatrix,sudoku,indexValue);
    double end = omp_get_wtime();
    printSudoku(solutionGrid);

    cout<<"Total parallel time taken to solve a "<<SIZE<<"x"<<SIZE<<" Sudoku Board : "<<end-start<<endl;
}