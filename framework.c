#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_NAME_SIZE 128

FILE *openFile(char *fileName, char *mode);

void writeMatrixToFile(int **matrix, int rows, int cols, char *fileName);

void freeUPAMatrix(int **upaMatrix, int userCount);

int **readUPAMatrix(FILE *f, int userCount, int permissionCount);

void concurrentProcessingFramework(int **upaMatrix, int userCount,
                                   int permissionCount);

// TODO: Modify the function signature
void formRoleProcedure(int **uaMatrix, int **paMatrix);

int main() {
  char upaFile[MAX_FILE_NAME_SIZE];
  printf("Enter the name of the UPA matrix file: ");
  scanf("%s", upaFile);

  FILE *f = openFile(upaFile, "r");

  int userCount, permissionCount;
  fscanf(f, "%d", &userCount);
  fscanf(f, "%d", &permissionCount);

  int **upaMatrix = readUPAMatrix(f, userCount, permissionCount);

  fclose(f);

  concurrentProcessingFramework(upaMatrix, userCount, permissionCount);

  freeUPAMatrix(upaMatrix, userCount);
}

FILE *openFile(char *fileName, char *mode) {
  FILE *f = fopen(fileName, mode);
  if (f == NULL) {
    perror("Unable to open file: ");
    exit(1);
  }
  return f;
}

void writeMatrixToFile(int **matrix, int rows, int cols, char *fileName) {
  FILE *f = openFile(fileName, "w");

  fprintf(f, "%d\n%d\n", rows, cols);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fprintf(f, "%d ", matrix[i][j]);
    }
    fprintf(f, "\n");
  }

  fclose(f);
}

int **readUPAMatrix(FILE *f, int userCount, int permissionCount) {
  int **upaMatrix = (int **)malloc(userCount * sizeof(int *));
  for (int i = 0; i < userCount; i++) {
    upaMatrix[i] = (int *)malloc(permissionCount * sizeof(int));
  }

  int i, j;

  while (fscanf(f, "%d %d", &i, &j) != EOF) {
    upaMatrix[i - 1][j - 1] = 1;
  }

  return upaMatrix;
}

void freeUPAMatrix(int **upaMatrix, int userCount) {
  for (int i = 0; i < userCount; i++) {
    free(upaMatrix[i]);
  }
  free(upaMatrix);
}

// TODO: Implement the function
void concurrentProcessingFramework(int **upaMatrix, int userCount,
                                   int permissionCount) {}

// TODO: Implement the function
void formRoleProcedure(int **uaMatrix, int **paMatrix) {}
