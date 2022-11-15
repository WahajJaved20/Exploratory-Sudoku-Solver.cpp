#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <omp.h>
#include <stack>
#include <unistd.h>
#include <math.h>
using namespace std;
int n;
int THREAD_COUNT = 1;
// stack to store the permutation grids
stack<int **> grids;
/*******************************************************************/
/********************* PRINTING MATRICES **************************/
/*****************************************************************/
void printSudoku(int **sudoku, int size)
{
  printf("The Sudoku contains:\n");
  for (int j = 0; j < size; j++)
  {
    for (int i = 0; i < size; i++)
    {
      printf("%d  ", sudoku[j][i]);
    }
    printf("\n");
  }
}

void printPossibilityMatrix(int ***possibilityMatrix, int **indexValue, int size)
{
  for (int i = 0; i < size; i++)
  {
    printf("{");
    for (int j = 0; j < size; j++)
    {
      printf("  {");
      for (int k = 0; k < indexValue[i][j]; k++)
      {
        printf("%d,", possibilityMatrix[i][j][k]);
      }
      printf("},\n");
    }
    printf("  }\n");
  }
}
/*******************************************************************/
/********************** HELPER FUNCTIONS **************************/
/*****************************************************************/
int checkSquare(int **sudoku, int num, int row, int col)
{
  int squareSize = sqrt(n);
  int row_start = row - row % squareSize;
  int col_start = col - col % squareSize;
  int result = 1;
  for (int i = row_start; i < row_start + squareSize; i++)
  {
    for (int j = col_start; j < col_start + squareSize; j++)
    {
      if (sudoku[i][j] == num)
      {
        result = 0;
      }
    }
  }
  return result;
}

int checkSudoku(int **sudoku, int num, int row, int col)
{
  int result = 1;
#pragma omp parallel num_threads(THREAD_COUNT)
  for (int i = 0; i < n; i++)
  {
    if (num == sudoku[i][col])
    {
      result = 0;
    }
  }
  if (!result)
  {
    return 0;
  }
#pragma omp parallel num_threads(THREAD_COUNT)
  for (int j = 0; j < n; j++)
  {
    if (num == sudoku[row][j])
    {
      result = 0;
    }
  }
  if (!result)
  {
    return 0;
  }

  if (checkSquare(sudoku, num, row, col) == 0)
  {
    return 0;
  }
  return result;
}

int solved(int **sudoku, int *rows, int *columns)
{
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      if (sudoku[i][j] == 0)
      {
        *(rows) = i;
        *(columns) = j;
        return 0;
      }
    }
  }
  return 1;
}
void generatePossibilityMatrix(int **sudoku, int ***possibility, int **indexes, int n)
{
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      if (sudoku[i][j] != 0)
      {
        continue;
      }
#pragma omp parallel for num_threads(THREAD_COUNT)
      for (int k = 1; k <= n; k++)
      {
        if (checkSudoku(sudoku, k, i, j))
        {

          possibility[i][j][indexes[i][j]] = k;
          indexes[i][j]++;
        }
      }
    }
  }
}

void generateSudokuPermutations(int **sudoku, int ***possibility, int **indexes, int n, int threadCount, int cellsToFill)
{
  if (threadCount > cellsToFill)
  {
    cellsToFill = threadCount;
  }
#pragma omp parallel for num_threads(THREAD_COUNT)
  for (int t = 0; t < threadCount; t++)
  {
    int **temp = new int *[n];
    for (int i = 0; i < n; i++)
    {
      temp[i] = new int[n];
      memcpy(temp[i], sudoku[i], n * sizeof(int));
    }
    int currentFilled = 0;
    for (int i = 0; i < n; i++)
    {
      if (currentFilled == cellsToFill)
      {
        break;
      }
      for (int j = 0; j < n; j++)
      {
        if (currentFilled == cellsToFill)
        {
          break;
        }
        if (temp[i][j] != 0)
        {
          continue;
        }
        else
        {
          bool valuePut = false;
          int passes = 0;
          while (!valuePut)
          {
            if (passes = 10)
            {
              break;
            }
            passes++;
            srand(time(NULL));
            int index = rand() % indexes[i][j];
            if (checkSudoku(temp, possibility[i][j][index], i, j))
            {
              temp[i][j] = possibility[i][j][index];
              currentFilled++;
              valuePut = true;
            }
          }
        }
      }
    }
    grids.push(temp);
  }
}
/*******************************************************************/
/********************* SOLUTION GENERATORS ************************/
/*****************************************************************/
int sudokuSolver(int **sudoku, int depth)
{
  int rows;
  int columns;
  if (solved(sudoku, &rows, &columns) == 1)
  {
    return 1;
  }
  for (int i = 1; i <= n; i++)
  {
    if (checkSudoku(sudoku, i, rows, columns) == 1)
    {
      sudoku[rows][columns] = i;
      if (sudokuSolver(sudoku, depth))
      {
        return 1;
      }
      sudoku[rows][columns] = 0;
    }
  }
  return 0;
}

int solveSudoku(int **sudoku, int depth)
{
  return sudokuSolver(sudoku, depth);
}

/*******************************************************************/
/**************************** MAIN ********************************/
/*****************************************************************/
int main(int argc, char *argv[])
{
  if (!argv[1] || !argv[2])
  {
    cout << "Please run the code as ./main <board_size> <thread_count>" << endl;
    return 0;
  }
  n = stoi(argv[1]);
  THREAD_COUNT = stoi(argv[2]);
  // dynamically allocate the required variables
  int ***possibilityMatrix = new int **[n];
  int **indexValue = new int *[n];
  int **sudoku = new int *[n];
#pragma omp parallel for num_threads(THREAD_COUNT)
  for (int i = 0; i < n; i++)
  {
    sudoku[i] = new int[n];
    possibilityMatrix[i] = new int *[n];
    indexValue[i] = new int[n];
  }
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      possibilityMatrix[i][j] = new int[n];
      indexValue[i][j] = 0;
#pragma omp parallel for num_threads(THREAD_COUNT)
      for (int k = 0; k < n; k++)
      {
        possibilityMatrix[i][j][k] = 0;
      }
    }
  }
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      cin >> sudoku[i][j];
    }
  }
  int **temp = new int *[n];
  for (int i = 0; i < n; i++)
  {
    temp[i] = new int[n];
    memcpy(temp[i], sudoku[i], n * sizeof(int));
  }
  // push initial board as well in the case where the permutations are not valid
  grids.push(temp);
  cout << "Grid pushed" << endl;
  // generate the list of all possible numbers availaible to each cell
  generatePossibilityMatrix(sudoku, possibilityMatrix, indexValue, n);
  cout << "Possibility Matrix Generated" << endl;
  // generate some count of sudoku permutations
  generateSudokuPermutations(sudoku, possibilityMatrix, indexValue, n, THREAD_COUNT, n);
  cout << "Permutations Generated" << endl;
  // test the code Serially
  // clock_t begin = clock();
  // solveSudoku(sudoku, 0);
  // clock_t end = clock();
  // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  // printf("total serial time: %f\n", time_spent);
  // printSudoku(sudoku, n);
  // test the code parallely
  bool solved = false;
  int stackSize = grids.size();
  cout << "Stack Size: " << stackSize << endl;
  double init = omp_get_wtime();

#pragma omp parallel shared(solved) num_threads(THREAD_COUNT)
  {
    int **grid;
#pragma omp master
    {
      cout << "Total Threads Running: " << omp_get_num_threads() << endl;
    }
#pragma omp for
    for (int i = 0; i < stackSize; i++)
    {

      if (solved)
      {
        continue;
      }
      else
      {
#pragma omp critical
        {
          if (!grids.empty())
          {
            grid = grids.top();
            grids.pop();
          }
        }
        cout << "Solving " << i << endl;
        if (solveSudoku(grid, 0) == 1)
        {
          solved = true;
          printf("Solved %d\n", i);
          printSudoku(grid, n);
        }
        else
        {
          printf("Couldn't solve %d\n", i);
        }
      }
    }
  }

  double endit = omp_get_wtime();
  printf("Total Parallel Time:%f \n", endit - init);
  // delete DMA memory
  printSudoku(sudoku, n);
  for (int i = 0; i < n; i++)
  {
    delete[] indexValue[i];
    delete[] sudoku[i];
    delete[] temp[i];
    for (int j = 0; j < n; j++)
    {
      delete[] possibilityMatrix[i][j];
    }
    delete[] possibilityMatrix[i];
  }
  delete possibilityMatrix;
  delete indexValue;
  delete sudoku;
  delete temp;
  possibilityMatrix = NULL;
  indexValue = NULL;
  sudoku = NULL;
  temp = NULL;
}