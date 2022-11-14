#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <omp.h>
#include <stack>
using namespace std;
const int n = 9;
stack<int **> grids;

void print_sudoku(int **sudoku)
{
  printf("The Sudoku contains:\n");
  for (int j = 0; j < 9; j++)
  {
    for (int i = 0; i < 9; i++)
    {
      printf("%d  ", sudoku[j][i]);
    }
    printf("\n");
  }
}

int check_square(int **sudoku, int num, int row, int col)
{
  int row_start = row - row % 3;
  int col_start = col - col % 3;
  int result = 1;
  for (int i = row_start; i < row_start + 3; i++)
  {
#pragma omp parallel for
    for (int j = col_start; j < col_start + 3; j++)
    {
      if (sudoku[i][j] == num)
      {
        result = 0;
      }
    }
  }
  return result;
}

int check_sudoku(int **sudoku, int num, int row, int col)
{
  int result = 1;
#pragma omp parallel for schedule(static)
  for (int i = 0; i < 9; i++)
  {
    if (num == sudoku[i][col])
    {
    }
  }
  if (!result)
  {
    return 0;
  }
#pragma omp parallel for schedule(static)
  for (int j = 0; j < 9; j++)
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

  if (check_square(sudoku, num, row, col) == 0)
  {
    return 0;
  }
  return result;
}

int solved(int **sudoku, int *rows, int *columns)
{
  for (int i = 0; i < 9; i++)
  {
    for (int j = 0; j < 9; j++)
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

int sudoku_solver_pro(int **sudoku, int depth)
{
  int rows;
  int columns;
  if (solved(sudoku, &rows, &columns) == 1)
  {
    return 1;
  }
  for (int i = 1; i <= 9; i++)
  {
    if (check_sudoku(sudoku, i, rows, columns) == 1)
    {
      sudoku[rows][columns] = i;
      if (sudoku_solver_pro(sudoku, depth))
      {
        return 1;
      }
      sudoku[rows][columns] = 0;
    }
  }
  return 0;
}

void solve_sudoku(int **sudoku, int depth)
{
  sudoku_solver_pro(sudoku, depth);
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

      for (int k = 1; k <= n; k++)
      {
        if (check_sudoku(sudoku, k, i, j))
        {
          possibility[i][j][indexes[i][j]] = k;
          indexes[i][j]++;
        }
      }
    }
  }
}
void printPossibilityMatrix(int ***possibilityMatrix, int **indexValue)
{
  for (int i = 0; i < n; i++)
  {
    printf("{");
    for (int j = 0; j < n; j++)
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
void generateSudokuPermutations(int **sudoku, int ***possibility, int **indexes, int n, int threadCount, int cellsToFill)
{
  for (int t = 0; t < threadCount; t++)
  {
    int **temp = sudoku;
    int currentFilled = 0;
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
      {
            }
    }
  }
}
#ifndef __testing
int main()
{
  int **sudoku = new int *[n];
  for (int i = 0; i < n; i++)
  {
    sudoku[i] = new int[n];
  }
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      cin >> sudoku[i][j];
    }
  }
  // int Sudoku[9][9] = {
  //     {1, 0, 6, 0, 0, 0, 0, 0, 0},
  //     {0, 5, 0, 0, 0, 0, 0, 0, 1},
  //     {0, 0, 0, 5, 0, 0, 0, 0, 0},
  //     {0, 0, 0, 9, 0, 0, 0, 0, 0},
  //     {5, 0, 1, 0, 0, 0, 9, 0, 0},
  //     {0, 0, 0, 0, 0, 0, 1, 0, 7},
  //     {0, 0, 0, 0, 0, 0, 0, 4, 0},
  //     {0, 0, 0, 0, 5, 0, 0, 0, 9},
  //     {9, 0, 0, 8, 0, 4, 0, 1, 0}};

  int ***possibilityMatrix;
  int **indexValue;
  possibilityMatrix = new int **[n];
  indexValue = new int *[n];
#pragma omp parallel for schedule(static)
  for (int i = 0; i < n; i++)
  {
    possibilityMatrix[i] = new int *[n];
    indexValue[i] = new int[n];
  }
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      possibilityMatrix[i][j] = new int[n];
      indexValue[i][j] = 0;
#pragma omp parallel for schedule(static)
      for (int k = 0; k < n; k++)
      {
        possibilityMatrix[i][j][k] = 0;
      }
    }
  }
  double init = omp_get_wtime();
  generatePossibilityMatrix(sudoku, possibilityMatrix, indexValue, n);
  double end = omp_get_wtime();
  printf("aaa%f", end - init);
  printPossibilityMatrix(possibilityMatrix, indexValue);
  //   printf("Input puzzle is:\n");
  //   print_sudoku(Sudoku);
  //   clock_t begin = clock();

  //   solve_sudoku(Sudoku,0);
  //   clock_t end = clock();
  // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  // printf("total time: %f\n",time_spent);
  //   printf("Solution is:\n");
  //   print_sudoku(Sudoku);
}

#endif