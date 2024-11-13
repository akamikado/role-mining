#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_NAME_SIZE 128
#define MAX_ROLES 1000

FILE *openFile(char *fileName, char *mode);

void writeMatrixToFile(int **matrix, int rows, int cols, char *fileName);

void writeMatrixTransposeToFile(int **matrix, int rows, int cols,
                                char *fileName);

void freeMatrix(int **upaMatrix, int userCount);

int **readUPAMatrix(FILE *f, int userCount, int permissionCount);

int **copyMatrix(int **matrix, int rows, int cols);

int **transposeMatrix(int **matrix, int rows, int cols);

int selectVertexWithHeuristic(int **UC, int userCount, int permissionCount);

int selectVertexWithMaxUncoveredIncidentEdges(int **UC, int userCount,
                                              int permissionCount);

void concurrentProcessingFramework(int **upaMatrix, int userCount,
                                   int permissionCount, int mrcUser,
                                   int mrcPermission);

// TODO: Modify the function signature
void formRoleProcedure(int v, int *U, int *P, int **uaMatrix, int **paMatrix);

void dualFormRoleProcedure(int v, int *U, int *P, int **uaMatrix,
                           int **paMatrix);

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

  int mrcUser, mrcPermission;
  scanf("%d %d", &mrcUser, &mrcPermission);

  concurrentProcessingFramework(upaMatrix, userCount, permissionCount, mrcUser,
                                mrcPermission);

  freeMatrix(upaMatrix, userCount);
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

void writeMatrixTransposeToFile(int **matrix, int rows, int cols,
                                char *fileName) {
  FILE *f = openFile(fileName, "w");

  fprintf(f, "%d\n%d\n", cols, rows);

  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
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

void freeMatrix(int **upaMatrix, int userCount) {
  for (int i = 0; i < userCount; i++) {
    free(upaMatrix[i]);
  }
  free(upaMatrix);
}

int **copyMatrix(int **matrix, int rows, int cols) {
  int **copy = (int **)malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    copy[i] = (int *)malloc(cols * sizeof(int));
    for (int j = 0; j < cols; j++) {
      copy[i][j] = matrix[i][j];
    }
  }
  return copy;
}

int selectVertexWithHeuristic(int **UC, int userCount, int permissionCount) {
  int vertex = -1;
  int min = userCount + permissionCount;

  for (int i = 0; i < userCount; i++) {
    int count = 0;
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    if (count > 0 && count < min) {
      min = count;
      vertex = i;
    }
  }

  for (int j = 0; j < permissionCount; j++) {
    int count = 0;
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    if (count > 0 && (count < min || (count == min && vertex >= userCount))) {
      min = count;
      vertex = j + userCount;
    }
  }

  return vertex;
}

int selectVertexWithMaxUncoveredIncidentEdges(int **UC, int userCount,
                                              int permissionCount) {
  int vertex = -1;
  int max = -1;

  for (int i = 0; i < userCount; i++) {
    int count = 0;
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    if (count > max) {
      max = count;
      vertex = i;
    }
  }

  for (int j = 0; j < permissionCount; j++) {
    int count = 0;
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    if (count > max) {
      max = count;
      vertex = j + userCount;
    }
  }

  return vertex;
}

// Alogrithm 4
void concurrentProcessingFramework(int **upaMatrix, int userCount,
                                   int permissionCount, int mrcUser,
                                   int mrcPerm) {
  int userRoleCount[userCount];
  for (int i = 0; i < userCount; i++) {
    userRoleCount[i] = 0;
  }
  int permRoleCount[permissionCount];
  for (int i = 0; i < permissionCount; i++) {
    permRoleCount[i] = 0;
  }
  int **uaMatrix, **paMatrix;
  uaMatrix = (int **)malloc(userCount * sizeof(int *));
  for (int i = 0; i < userCount; i++) {
    uaMatrix[i] = (int *)malloc(MAX_ROLES * sizeof(int));
  }
  paMatrix = (int **)malloc(permissionCount * sizeof(int *));
  for (int i = 0; i < permissionCount; i++) {
    paMatrix[i] = (int *)malloc(MAX_ROLES * sizeof(int));
  }
  int **UC = copyMatrix(upaMatrix, userCount, permissionCount);

  // Phase 1
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1 && userRoleCount[i] < mrcUser - 1) {
        int *U = (int *)malloc(userCount * sizeof(int));
        int *P = (int *)malloc(permissionCount * sizeof(int));

        int vertex = selectVertexWithHeuristic(UC, userCount, permissionCount);

        if (vertex < userCount) {
          formRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
        } else if (vertex >= userCount &&
                   vertex < userCount + permissionCount) {
          dualFormRoleProcedure(vertex - userCount, U, P, uaMatrix, paMatrix);
        }

        free(U);
        free(P);
      }
    }
  }
  for (int j = 0; j < permissionCount; j++) {
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1 && permRoleCount[j] < mrcPerm - 1) {
        int *U = (int *)malloc(userCount * sizeof(int));
        int *P = (int *)malloc(permissionCount * sizeof(int));

        int vertex = selectVertexWithHeuristic(UC, userCount, permissionCount);

        if (vertex < userCount) {
          formRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
        } else if (vertex >= userCount &&
                   vertex < userCount + permissionCount) {
          dualFormRoleProcedure(vertex - userCount, U, P, uaMatrix, paMatrix);
        }

        free(U);
        free(P);
      }
    }
  }

  // Phase 2
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1 && userRoleCount[i] < mrcUser - 1) {
        int *U = (int *)malloc(userCount * sizeof(int));
        int *P = (int *)malloc(permissionCount * sizeof(int));

        int vertex = selectVertexWithMaxUncoveredIncidentEdges(UC, userCount,
                                                               permissionCount);

        if (vertex < userCount) {
          int condition = 1;
          for (int k = 0; k < permissionCount; k++) {
            if (UC[vertex][k] == 1) {
              P[k] = 1;
              if (permRoleCount[k] > mrcPerm - 1) {
                condition = 0;
              }
            }
          }
          if (condition) {
            formRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
          } else {
            dualFormRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
          }
        }

        free(U);
        free(P);
      }
    }
  }
  for (int j = 0; j < permissionCount; j++) {
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1 && permRoleCount[j] < mrcPerm - 1) {
        int *U = (int *)malloc(userCount * sizeof(int));
        int *P = (int *)malloc(permissionCount * sizeof(int));

        int vertex = selectVertexWithMaxUncoveredIncidentEdges(UC, userCount,
                                                               permissionCount);

        if (vertex < userCount) {
          int condition = 1;
          for (int k = 0; k < permissionCount; k++) {
            if (UC[vertex][k] == 1) {
              P[k] = 1;
              if (permRoleCount[k] > mrcPerm - 1) {
                condition = 0;
              }
            }
          }
          if (condition) {
            formRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
          } else {
            dualFormRoleProcedure(vertex, U, P, uaMatrix, paMatrix);
          }
        }

        free(U);
        free(P);
      }
    }
  }

  freeMatrix(uaMatrix, userCount);
  freeMatrix(paMatrix, permissionCount);
  freeMatrix(UC, userCount);
}

// TODO: Implement the function
void formRoleProcedure(int v, int *U, int *P, int **uaMatrix, int **paMatrix) {}

// TODO: Implement the function
void dualFormRoleProcedure(int v, int *U, int *P, int **uaMatrix,
                           int **paMatrix) {}
