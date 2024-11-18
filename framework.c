#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_NAME_SIZE 128
#define MAX_ROLES 1000

FILE *openFile(char *fileName, char *mode);

char *getDatasetName(char *fileName);

void writeMatrixToFile(int **matrix, int rows, int cols, char *fileName);

void writeMatrixTransposeToFile(int **matrix, int rows, int cols,
                                char *fileName);

void freeMatrix(int **upaMatrix, int userCount);

int **readUPAMatrix(FILE *f, int userCount, int permissionCount);

int **copyMatrix(int **matrix, int rows, int cols);

int isSubset(int *uc, int *p, int size);

int hasElement(int *uc, int *p, int size);

int selectVertexWithHeuristic(int **UC, int userCount, int permissionCount);

int selectVertexWithMaxUncoveredIncidentEdges(int **UC, int userCount,
                                              int permissionCount);

int hasUncoveredEdges(int **UC, int userCount, int permissionCount);

int concurrentProcessingFramework(int **upaMatrix, int userCount,
                                  int permissionCount, int mrcUser,
                                  int mrcPermission, char *dataset);

void modifyUC(int **UC, int *U, int *P, int userCount, int permissionCount);

int uniqueRole(int *U, int *P, int **uaMatrix, int **paMatrix, int userCount,
               int roleCount, int permissionCount);

int isSetEmpty(int *a, int size);

void addRoletoUA(int **uaMatrix, int *U, int userCount, int roleCount);

void addRoletoPA(int **paMatrix, int *P, int permissionCount, int roleCount);

void formRoleProcedure(int v, int userCount, int permissionCount,
                       int U[userCount], int P[permissionCount], int **UC,
                       int **V, int mrcUser, int mrcPerm, int *userRoleCount,
                       int *permRoleCount, int **uaMatrix, int **paMatrix,
                       int *roleCount);

void dualFormRoleProcedure(int v, int *U, int *P, int **UC, int **V,
                           int mrcUser, int mrcPerm, int *userRoleCount,
                           int *permRoleCount, int **uaMatrix, int **paMatrix,
                           int userCount, int permissionCount, int *roleCount);

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

  char *dataset = getDatasetName(upaFile);

  int mrcUser, mrcPermission;

  printf("Enter the value of the role-usage cardinality constraint: ");
  scanf("%d", &mrcUser);

  printf("Enter the value of the permission-distribution cardinality "
         "constraint: ");
  scanf("%d", &mrcPermission);

  int roleCount = concurrentProcessingFramework(
      upaMatrix, userCount, permissionCount, mrcUser, mrcPermission, dataset);

  freeMatrix(upaMatrix, userCount);
  free(dataset);

  if (roleCount != -1) {
    printf("Number of roles = %d\n", roleCount);
  }

  return 0;
}

FILE *openFile(char *fileName, char *mode) {
  FILE *f = fopen(fileName, mode);
  if (f == NULL) {
    perror("Unable to open file: ");
    exit(1);
  }
  return f;
}

char *getDatasetName(char *fileName) {
  const char *token = strrchr(fileName, '.');
  if (!token || token == fileName) {
    return strdup(fileName);
  }

  size_t length = token - fileName;

  char *datasetName = (char *)malloc(length + 1);
  if (datasetName) {
    strncpy(datasetName, fileName, length);
    datasetName[length] = '\0';
  }
  return datasetName;
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
    for (int j = 0; j < permissionCount; j++) {
      upaMatrix[i][j] = 0;
    }
  }

  int i, j;

  while (fscanf(f, " %d %d", &i, &j) != EOF) {
    upaMatrix[i - 1][j - 1] = 1;
  }

  return upaMatrix;
}

void freeMatrix(int **matrix, int rows) {
  for (int i = 0; i < rows; i++) {
    free(matrix[i]);
  }
  free(matrix);
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

int isSubset(int *a, int *b, int size) {
  for (int i = 0; i < size; i++) {
    if (a[i] == 1 && b[i] != 1) {
      return 0;
    }
  }
  return 1;
}

int hasElement(int *a, int *b, int size) {
  for (int i = 0; i < size; i++) {
    if (a[i] == 1 && b[i] == 1) {
      return 1;
    }
  }
  return 0;
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
  int max = 0;

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

int hasUncoveredEdges(int **UC, int userCount, int permissionCount) {
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        return 1;
      }
    }
  }
  return 0;
}

// Alogrithm 4
int concurrentProcessingFramework(int **upaMatrix, int userCount,
                                  int permissionCount, int mrcUser, int mrcPerm,
                                  char *dataset) {
  int userRoleCount[userCount];
  for (int i = 0; i < userCount; i++) {
    userRoleCount[i] = 0;
  }
  int permRoleCount[permissionCount];
  for (int i = 0; i < permissionCount; i++) {
    permRoleCount[i] = 0;
  }
  int **uaMatrix = (int **)malloc(userCount * sizeof(int *));
  for (int i = 0; i < userCount; i++) {
    uaMatrix[i] = (int *)malloc(MAX_ROLES * sizeof(int));
    for (int j = 0; j < MAX_ROLES; j++) {
      uaMatrix[i][j] = 0;
    }
  }
  int **paMatrix = (int **)malloc(permissionCount * sizeof(int *));
  for (int i = 0; i < permissionCount; i++) {
    paMatrix[i] = (int *)malloc(MAX_ROLES * sizeof(int));
    for (int j = 0; j < MAX_ROLES; j++) {
      paMatrix[i][j] = 0;
    }
  }
  int **UC = copyMatrix(upaMatrix, userCount, permissionCount);

  int roleCount = 0;

  int i = 0, j = 0;

  // Phase 1
  while (hasUncoveredEdges(UC, userCount, permissionCount)) {
    if (UC[i][j] == 1 &&
        (userRoleCount[i] < mrcUser - 1 || permRoleCount[j] < mrcPerm - 1)) {
      int U[userCount], P[permissionCount];
      for (int k = 0; k < userCount; k++) {
        U[k] = 0;
      }
      for (int k = 0; k < permissionCount; k++) {
        P[k] = 0;
      }

      int vertex = selectVertexWithHeuristic(UC, userCount, permissionCount);

      if (vertex < userCount) {
        formRoleProcedure(vertex, userCount, permissionCount, U, P, UC,
                          upaMatrix, mrcUser, mrcPerm, userRoleCount,
                          permRoleCount, uaMatrix, paMatrix, &roleCount);
      } else if (vertex >= userCount && vertex < userCount + permissionCount) {
        dualFormRoleProcedure(vertex - userCount, U, P, UC, upaMatrix, mrcPerm,
                              mrcPerm, userRoleCount, permRoleCount, uaMatrix,
                              paMatrix, userCount, permissionCount, &roleCount);
      }
    }
    i = (i + 1) % userCount;
    j = (j + 1) % permissionCount;
  }

  i = 0;
  j = 0;

  // Phase 2
  while (hasUncoveredEdges(UC, userCount, permissionCount)) {
    if (UC[i][j] == 1 &&
        (userRoleCount[i] == mrcUser - 1 || permRoleCount[j] == mrcPerm - 1)) {
      int U[userCount], P[permissionCount];
      for (int k = 0; k < userCount; k++) {
        U[k] = 0;
      }
      for (int k = 0; k < permissionCount; k++) {
        P[k] = 0;
      }

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
          formRoleProcedure(vertex, userCount, permissionCount, U, P, UC,
                            upaMatrix, mrcUser, mrcPerm, userRoleCount,
                            permRoleCount, uaMatrix, paMatrix, &roleCount);
        }
      } else if (vertex >= userCount && vertex < userCount + permissionCount) {
        vertex -= userCount;
        int condition = 1;
        for (int k = 0; k < userCount; k++) {
          printf("%d\n", UC[k][vertex]);
          if (UC[k][vertex] == 1) {
            U[k] = 1;
            if (userRoleCount[k] > mrcUser - 1) {
              condition = 0;
            }
          }
        }
        if (condition) {
          dualFormRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                                userRoleCount, permRoleCount, uaMatrix,
                                paMatrix, userCount, permissionCount,
                                &roleCount);
        }
      }
    }
    i = (i + 1) % userCount;
    j = (j + 1) % permissionCount;
  }

  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        printf("The given set of constraints cannot be enforced\n");
        roleCount = -1;
        break;
      }
    }
    if (roleCount == -1) {
      break;
    }
  }

  if (roleCount != -1) {
    char uaFile[128], paFile[128];
    sprintf(uaFile, "%s_UA.txt", dataset);
    sprintf(paFile, "%s_PA.txt", dataset);

    writeMatrixToFile(uaMatrix, userCount, roleCount, uaFile);
    writeMatrixTransposeToFile(paMatrix, permissionCount, roleCount, paFile);
  }

  freeMatrix(uaMatrix, userCount);
  freeMatrix(paMatrix, permissionCount);
  freeMatrix(UC, userCount);

  return roleCount;
}

void modifyUC(int **UC, int *U, int *P, int userCount, int permissionCount) {
  for (int i = 0; i < userCount; i++) {
    if (U[i] == 1) {
      for (int j = 0; j < permissionCount; j++) {
        if (P[j] == 1) {
          UC[i][j] = 0;
        }
      }
    }
  }
}

int uniqueRole(int *U, int *P, int **uaMatrix, int **paMatrix, int userCount,
               int roleCount, int permissionCount) {
  for (int i = 0; i < roleCount; i++) {
    int flag = 1;
    for (int j = 0; j < userCount; j++) {
      if (uaMatrix[j][i] != U[j]) {
        flag = 0;
        break;
      }
    }
    for (int j = 0; j < permissionCount; j++) {
      if (paMatrix[j][i] != P[j]) {
        flag = 0;
        break;
      }
    }
    if (flag) {
      return 0;
    }
  }
  return 1;
}

int isSetEmpty(int *a, int size) {
  for (int i = 0; i < size; i++) {
    if (a[i] == 1) {
      return 0;
    }
  }
  return 1;
}

void addRoletoUA(int **uaMatrix, int *U, int userCount, int roleCount) {
  for (int i = 0; i < userCount; i++) {
    uaMatrix[i][roleCount - 1] = U[i];
  }
}

void addRoletoPA(int **paMatrix, int *P, int permissionCount, int roleCount) {
  for (int j = 0; j < permissionCount; j++) {
    paMatrix[j][roleCount - 1] = P[j];
  }
}

void formRoleProcedure(int v, int userCount, int permissionCount,
                       int U[userCount], int P[permissionCount], int **UC,
                       int **V, int mrcUser, int mrcPerm, int *userRoleCount,
                       int *permRoleCount, int **uaMatrix, int **paMatrix,
                       int *roleCount) {

  int tempUserRoleCount[userCount];
  for (int i = 0; i < userCount; i++) {
    tempUserRoleCount[i] = userRoleCount[i];
  }
  int tempPermRoleCount[permissionCount];
  for (int i = 0; i < permissionCount; i++) {
    tempPermRoleCount[i] = permRoleCount[i];
  }
  U[v] = 1;
  tempUserRoleCount[v] += 1;

  for (int i = 0; i < permissionCount; i++) {
    int p = UC[v][i];
    if (p == 1 && tempPermRoleCount[i] < mrcPerm - 1) {
      P[i] = 1;
      tempPermRoleCount[i] += 1;
    }
  }

  for (int i = 0; i < userCount; i++) {
    if (i != v && tempUserRoleCount[i] < mrcUser - 1 &&
        isSubset(P, V[i], permissionCount) &&
        hasElement(UC[i], P, permissionCount)) {
      U[i] = 1;
      tempUserRoleCount[i] += 1;
    } else if (tempUserRoleCount[i] == mrcUser - 1 &&
               isSubset(P, V[i], permissionCount) &&
               isSubset(UC[i], P, permissionCount)) {
      U[i] = 1;
      tempUserRoleCount[i] += 1;
    }
  }

  if (isSetEmpty(P, permissionCount) ||
      !uniqueRole(U, P, uaMatrix, paMatrix, userCount, *roleCount,
                  permissionCount)) {
    return;
  }

  for (int i = 0; i < userCount; i++) {
    userRoleCount[i] = tempUserRoleCount[i];
  }
  for (int i = 0; i < permissionCount; i++) {
    permRoleCount[i] = tempPermRoleCount[i];
  }

  *roleCount += 1;

  modifyUC(UC, U, P, userCount, permissionCount);
  addRoletoUA(uaMatrix, U, userCount, *roleCount);
  addRoletoPA(paMatrix, P, permissionCount, *roleCount);
}

void dualFormRoleProcedure(int v, int *U, int *P, int **UC, int **V,
                           int mrcUser, int mrcPerm, int *userRoleCount,
                           int *permRoleCount, int **uaMatrix, int **paMatrix,
                           int userCount, int permissionCount, int *roleCount) {
  int tempPermRoleCount[permissionCount];
  for (int i = 0; i < permissionCount; i++) {
    tempPermRoleCount[i] = permRoleCount[i];
  }
  int tempUserRoleCount[userCount];
  for (int i = 0; i < userCount; i++) {
    tempUserRoleCount[i] = userRoleCount[i];
  }
  P[v] = 1;
  tempPermRoleCount[v] += 1;

  for (int i = 0; i < userCount; i++) {
    int u = UC[i][v];
    if (u == 1 && tempUserRoleCount[i] < mrcUser - 1) {
      U[i] = 1;
      tempUserRoleCount[i] += 1;
    }
  }

  int **transposeV = (int **)malloc(permissionCount * sizeof(int *));
  int **transposeUC = (int **)malloc(permissionCount * sizeof(int *));
  for (int i = 0; i < permissionCount; i++) {
    transposeV[i] = (int *)malloc(userCount * sizeof(int));
    transposeUC[i] = (int *)malloc(userCount * sizeof(int));
    for (int j = 0; j < userCount; j++) {
      transposeV[i][j] = V[j][i];
      transposeUC[i][j] = UC[j][i];
    }
  }

  for (int i = 0; i < permissionCount; i++) {
    if (i != v && tempPermRoleCount[i] < mrcPerm - 1 &&
        isSubset(U, transposeV[i], userCount) &&
        hasElement(transposeUC[i], U, userCount)) {
      P[i] = 1;
      tempPermRoleCount[i] += 1;
    } else if (tempPermRoleCount[i] == mrcPerm - 1 &&
               isSubset(U, transposeV[i], userCount) &&
               isSubset(transposeUC[i], U, userCount)) {
      P[i] = 1;
      tempPermRoleCount[i] += 1;
    }
  }

  freeMatrix(transposeV, permissionCount);
  freeMatrix(transposeUC, permissionCount);

  if (isSetEmpty(P, permissionCount) ||
      !uniqueRole(U, P, uaMatrix, paMatrix, userCount, permissionCount,
                  *roleCount)) {
    printf("dual form role procedure\n");
    int numberOfUncoveredEdges = 0;
    for (int i = 0; i < userCount; i++) {
      for (int j = 0; j < permissionCount; j++) {
        numberOfUncoveredEdges += UC[i][j];
      }
    }
    printf("%d\n", numberOfUncoveredEdges);
    return;
  }
  for (int i = 0; i < userCount; i++) {
    userRoleCount[i] = tempUserRoleCount[i];
  }
  for (int i = 0; i < permissionCount; i++) {
    permRoleCount[i] = tempPermRoleCount[i];
  }

  *roleCount += 1;

  modifyUC(UC, U, P, userCount, permissionCount);
  addRoletoUA(uaMatrix, U, userCount, *roleCount);
  addRoletoPA(paMatrix, P, permissionCount, *roleCount);
}
