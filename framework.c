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

enum VertexType { USER, PERMISSION };

typedef struct Vertex {
  int index;
  enum VertexType type;
} Vertex;

Vertex selectVertexWithHeuristic(int **UC, int mrcUser, int mrcPerm,
                                 int *userRoleCount, int *permRoleCount,
                                 int userCount, int permissionCount);

Vertex selectVertexWithMaxUncoveredIncidentEdges(int **UC, int userCount,
                                                 int permissionCount,
                                                 int *userRoleCount,
                                                 int *permRoleCount,
                                                 int mrcUser, int mrcPerm);

int hasUncoveredEdges(int **UC, int userCount, int permissionCount);

int concurrentProcessingFramework(int **upaMatrix, int userCount,
                                  int permissionCount, int mrcUser,
                                  int mrcPermission, char *dataset);

int modifyUC(int **UC, int *U, int *P, int userCount, int permissionCount);

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

Vertex selectVertexWithHeuristic(int **UC, int userCount, int permissionCount,
                                 int *userRoleCount, int *permRoleCount,
                                 int mrcUser, int mrcPerm) {
  int min = userCount + permissionCount;

  /* printf("user count: %d\n", userCount); */
  /* printf("permission count: %d\n", permissionCount); */

  Vertex v = {-1, PERMISSION};

  for (int j = 0; j < permissionCount; j++) {
    int uncoveredEdges = 0;
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1 && userRoleCount[i] < mrcUser - 1) {
        uncoveredEdges++;
      }
    }

    if (uncoveredEdges > 0 && uncoveredEdges < min) {
      min = uncoveredEdges;
      v.index = j;
      v.type = PERMISSION;
    }
  }

  for (int i = 0; i < userCount; i++) {
    int uncoveredEdges = 0;
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1 && permRoleCount[j] < mrcPerm - 1) {
        uncoveredEdges++;
      }
    }

    if (uncoveredEdges > 0 &&
        (uncoveredEdges < min ||
         (uncoveredEdges == min && v.type == PERMISSION))) {
      min = uncoveredEdges;
      v.index = i;
      v.type = USER;
    }
  }

  printf("Count: %d\n", min);

  return v;
}

Vertex selectVertexWithMaxUncoveredIncidentEdges(int **UC, int userCount,
                                                 int permissionCount,
                                                 int *userRoleCount,
                                                 int *permRoleCount, int mrUser,
                                                 int mrcPerm) {
  int max = 0;

  Vertex v = {-1, USER};

  for (int i = 0; i < userCount; i++) {
    if (userRoleCount[i] >= mrUser - 1) {
      continue;
    }

    int count = 0;
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    /* printf("User: %d count: %d\n", i, count); */
    if (count > max) {
      max = count;
      v.index = i;
      printf("User: %d chosen for count %d\n", i, count);
    }
  }

  for (int j = 0; j < permissionCount; j++) {
    if (permRoleCount[j] >= mrcPerm - 1) {
      continue;
    }

    int count = 0;
    for (int i = 0; i < userCount; i++) {
      if (UC[i][j] == 1) {
        count++;
      }
    }
    /* printf("Permission: %d count: %d\n", j, count); */
    if (count > max) {
      max = count;
      v.index = j;
      v.type = PERMISSION;
      printf("Permission: %d chosen for count %d\n", j, count);
    }
  }
  return v;
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

  int loopCount = 0;

  int remainingUncoveredEdges = 0;

  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      remainingUncoveredEdges += UC[i][j];
    }
  }

  // Phase 1
  printf("Phase 1\n");
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      loopCount++;
      if (loopCount % 1000 == 0) {
        printf("Phase 1 Loop %d: Remaining uncovered edges: %d\n", loopCount,
               remainingUncoveredEdges);
      }
      if (remainingUncoveredEdges == 0) {
        break;
      }
      if (UC[i][j] == 1 &&
          (userRoleCount[i] < mrcUser - 1 || permRoleCount[j] < mrcPerm - 1)) {
        int U[userCount], P[permissionCount];
        for (int k = 0; k < userCount; k++) {
          U[k] = 0;
        }
        for (int k = 0; k < permissionCount; k++) {
          P[k] = 0;
        }

        Vertex vertex = selectVertexWithHeuristic(
            UC, userCount, permissionCount, userRoleCount, permRoleCount,
            mrcUser, mrcPerm);

        if (vertex.index == -1) {
          printf("No vertex selected\n");
          continue;
        }

        if (vertex.type == USER) {
          formRoleProcedure(vertex.index, userCount, permissionCount, U, P, UC,
                            upaMatrix, mrcUser, mrcPerm, userRoleCount,
                            permRoleCount, uaMatrix, paMatrix, &roleCount);
        } else if (vertex.type == PERMISSION) {
          dualFormRoleProcedure(vertex.index, U, P, UC, upaMatrix, mrcPerm,
                                mrcPerm, userRoleCount, permRoleCount, uaMatrix,
                                paMatrix, userCount, permissionCount,
                                &roleCount);
        }
        remainingUncoveredEdges =
            remainingUncoveredEdges -
            modifyUC(UC, U, P, userCount, permissionCount);
      }
    }
    if (remainingUncoveredEdges == 0) {
      break;
    }
  }

  i = 0;
  j = 0;
  loopCount = 0;

  // Phase 2
  printf("Phase 2\n");
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      loopCount++;
      if (loopCount % 1000 == 0) {
        printf("Phase 2 Loop %d: Remaining uncovered edges: %d\n", loopCount,
               remainingUncoveredEdges);
      }
      if (remainingUncoveredEdges == 0) {
        break;
      }

      if (UC[i][j] == 1 && (userRoleCount[i] == mrcUser - 1 ||
                            permRoleCount[j] == mrcPerm - 1)) {
        int U[userCount], P[permissionCount];
        for (int k = 0; k < userCount; k++) {
          U[k] = 0;
        }
        for (int k = 0; k < permissionCount; k++) {
          P[k] = 0;
        }

        Vertex vertex = selectVertexWithMaxUncoveredIncidentEdges(
            UC, userCount, permissionCount, userRoleCount, permRoleCount,
            mrcUser, mrcPerm);
        printf("Vertex: %d type %d\n", vertex.index, vertex.type);

        if (vertex.index == -1) {
          printf("No vertex selected\n");
          break;
        }

        if (vertex.type == USER) {
          int condition = 1;
          for (int k = 0; k < permissionCount; k++) {
            if (UC[vertex.index][k] == 1) {
              P[k] = 1;
              if (permRoleCount[k] > mrcPerm - 1) {
                condition = 0;
              }
            }
          }
          if (condition) {
            formRoleProcedure(vertex.index, userCount, permissionCount, U, P,
                              UC, upaMatrix, mrcUser, mrcPerm, userRoleCount,
                              permRoleCount, uaMatrix, paMatrix, &roleCount);
          }
        } else if (vertex.type == PERMISSION) {
          int condition = 1;
          for (int k = 0; k < userCount; k++) {
            printf("%d\n", UC[k][vertex.index]);
            if (UC[k][vertex.index] == 1) {
              U[k] = 1;
              if (userRoleCount[k] > mrcUser - 1) {
                condition = 0;
              }
            }
          }
          if (condition) {
            dualFormRoleProcedure(vertex.index, U, P, UC, upaMatrix, mrcUser,
                                  mrcPerm, userRoleCount, permRoleCount,
                                  uaMatrix, paMatrix, userCount,
                                  permissionCount, &roleCount);
          }
        }

        remainingUncoveredEdges =
            remainingUncoveredEdges -
            modifyUC(UC, U, P, userCount, permissionCount);
      }
    }
    if (remainingUncoveredEdges == 0) {
      break;
    }
  }

  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1) {
        printf("The given set of constraints cannot be enforced\n");
        roleCount = -1;
        for (int k = 0; k < userCount; k++) {
          for (int l = 0; l < permissionCount; l++) {
            if (UC[k][l] == 1) {
              if (userRoleCount[k] < mrcUser - 1) {
                printf("User %d\n", k);
              }
              if (permRoleCount[l] < mrcPerm - 1) {
                printf("Permission %d\n", l);
              }
            }
          }
        }
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

int modifyUC(int **UC, int *U, int *P, int userCount, int permissionCount) {
  int modifications = 0;

  for (int i = 0; i < userCount; i++) {
    if (U[i] == 1) {
      for (int j = 0; j < permissionCount; j++) {
        if (P[j] == 1 && UC[i][j] == 1) {
          UC[i][j] = 0;
          modifications++;
        }
      }
    }
  }
  return modifications;
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

  int *tempPermRoleCount = (int *)malloc(permissionCount * sizeof(int));
  int *tempUserRoleCount = (int *)malloc(userCount * sizeof(int));
  int *tempU = (int *)malloc(userCount * sizeof(int));
  int *tempP = (int *)malloc(permissionCount * sizeof(int));
  memcpy(tempPermRoleCount, permRoleCount, permissionCount * sizeof(int));
  memcpy(tempUserRoleCount, userRoleCount, userCount * sizeof(int));
  memcpy(tempU, U, userCount * sizeof(int));
  memcpy(tempP, P, permissionCount * sizeof(int));

  tempU[v] = 1;
  tempUserRoleCount[v] += 1;

  for (int i = 0; i < permissionCount; i++) {
    int p = UC[v][i];
    printf("%d ", p);
    if (p == 1 && tempPermRoleCount[i] < mrcPerm - 1) {
      tempP[i] = 1;
      tempPermRoleCount[i] += 1;
    }
  }
  printf("\n");

  for (int i = 0; i < userCount; i++) {
    if (i != v && tempUserRoleCount[i] < mrcUser - 1 &&
        isSubset(tempP, V[i], permissionCount) &&
        hasElement(UC[i], tempP, permissionCount)) {
      tempU[i] = 1;
      tempUserRoleCount[i] += 1;
    } else if (tempUserRoleCount[i] == mrcUser - 1 &&
               isSubset(tempP, V[i], permissionCount) &&
               isSubset(UC[i], tempP, permissionCount)) {
      tempU[i] = 1;
      tempUserRoleCount[i] += 1;
    }
  }

  if (isSetEmpty(tempP, permissionCount)) {
    printf("U: \n");
    for (int i = 0; i < userCount; i++) {
      printf("%d ", tempU[i]);
    }
    printf("\nP: \n");
    for (int i = 0; i < permissionCount; i++) {
      printf("%d ", tempP[i]);
    }
    printf("\nUser role count: \n");
    for (int i = 0; i < userCount; i++) {
      printf("%d ", tempUserRoleCount[i]);
    }
    printf("\nPermission role count: \n");
    for (int i = 0; i < permissionCount; i++) {
      printf("%d ", tempPermRoleCount[i]);
    }
    printf("\n");
    free(tempPermRoleCount);
    free(tempUserRoleCount);
    free(tempU);
    free(tempP);
    perror("Empty P set in formRoleProcedure");
    return;
  }

  if (!uniqueRole(tempU, tempP, uaMatrix, paMatrix, userCount, *roleCount,
                  permissionCount)) {
    /* for (int i = 0; i < userCount; i++) { */
    /*   printf("%d ", U[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < permissionCount; i++) { */
    /*   printf("%d ", tempP[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < userCount; i++) { */
    /*   printf("%d ", tempUserRoleCount[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < permissionCount; i++) { */
    /*   printf("%d ", tempPermRoleCount[i]); */
    /* } */
    /* printf("\n"); */
    free(tempPermRoleCount);
    free(tempUserRoleCount);
    free(tempU);
    free(tempP);
    return;
  }

  memcpy(userRoleCount, tempUserRoleCount, userCount * sizeof(int));
  memcpy(permRoleCount, tempPermRoleCount, permissionCount * sizeof(int));
  memcpy(U, tempU, userCount * sizeof(int));
  memcpy(P, tempP, permissionCount * sizeof(int));

  *roleCount += 1;

  addRoletoUA(uaMatrix, U, userCount, *roleCount);
  addRoletoPA(paMatrix, P, permissionCount, *roleCount);

  free(tempPermRoleCount);
  free(tempUserRoleCount);
  free(tempU);
  free(tempP);
}

void dualFormRoleProcedure(int v, int *U, int *P, int **UC, int **V,
                           int mrcUser, int mrcPerm, int *userRoleCount,
                           int *permRoleCount, int **uaMatrix, int **paMatrix,
                           int userCount, int permissionCount, int *roleCount) {
  int *tempPermRoleCount = (int *)malloc(permissionCount * sizeof(int));
  int *tempUserRoleCount = (int *)malloc(userCount * sizeof(int));
  int *tempU = (int *)malloc(userCount * sizeof(int));
  int *tempP = (int *)malloc(permissionCount * sizeof(int));
  memcpy(tempPermRoleCount, permRoleCount, permissionCount * sizeof(int));
  memcpy(tempUserRoleCount, userRoleCount, userCount * sizeof(int));
  memcpy(tempU, U, userCount * sizeof(int));
  memcpy(tempP, P, permissionCount * sizeof(int));

  tempP[v] = 1;
  tempPermRoleCount[v] += 1;

  for (int i = 0; i < userCount; i++) {
    int u = UC[i][v];
    if (u == 1 && tempUserRoleCount[i] < mrcUser - 1) {
      tempU[i] = 1;
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
        isSubset(tempU, transposeV[i], userCount) &&
        hasElement(transposeUC[i], tempU, userCount)) {
      tempP[i] = 1;
      tempPermRoleCount[i] += 1;
    } else if (tempPermRoleCount[i] == mrcPerm - 1 &&
               isSubset(tempU, transposeV[i], userCount) &&
               isSubset(transposeUC[i], tempU, userCount)) {
      tempP[i] = 1;
      tempPermRoleCount[i] += 1;
    }
  }

  freeMatrix(transposeV, permissionCount);
  freeMatrix(transposeUC, permissionCount);

  if (isSetEmpty(tempU, userCount)) {
    printf("U: \n");
    for (int i = 0; i < userCount; i++) {
      printf("%d ", tempU[i]);
    }
    printf("\nP: \n");
    for (int i = 0; i < permissionCount; i++) {
      printf("%d ", tempP[i]);
    }
    printf("\nUser role count: \n");
    for (int i = 0; i < userCount; i++) {
      printf("%d ", tempUserRoleCount[i]);
    }
    printf("\nPermission role count: \n");
    for (int i = 0; i < permissionCount; i++) {
      printf("%d ", tempPermRoleCount[i]);
    }
    printf("\n");
    free(tempPermRoleCount);
    free(tempUserRoleCount);
    free(tempU);
    free(tempP);
    perror("Empty U set in dualFormRoleProcedure");
    return;
  }

  if (!uniqueRole(tempU, tempP, uaMatrix, paMatrix, userCount, *roleCount,
                  permissionCount)) {
    /* for (int i = 0; i < userCount; i++) { */
    /*   printf("%d ", tempU[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < permissionCount; i++) { */
    /*   printf("%d ", tempP[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < userCount; i++) { */
    /*   printf("%d ", tempUserRoleCount[i]); */
    /* } */
    /* printf("\n"); */
    /* for (int i = 0; i < permissionCount; i++) { */
    /*   printf("%d ", tempPermRoleCount[i]); */
    /* } */
    /* printf("\n"); */
    free(tempPermRoleCount);
    free(tempUserRoleCount);
    free(tempU);
    free(tempP);
    return;
  }

  memcpy(userRoleCount, tempUserRoleCount, userCount * sizeof(int));
  memcpy(permRoleCount, tempPermRoleCount, permissionCount * sizeof(int));
  memcpy(U, tempU, userCount * sizeof(int));
  memcpy(P, tempP, permissionCount * sizeof(int));

  *roleCount += 1;

  addRoletoUA(uaMatrix, tempU, userCount, *roleCount);
  addRoletoPA(paMatrix, tempP, permissionCount, *roleCount);

  free(tempPermRoleCount);
  free(tempUserRoleCount);
  free(tempU);
  free(tempP);
}
