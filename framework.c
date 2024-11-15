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

int concurrentProcessingFramework(int **upaMatrix, int userCount,
                                  int permissionCount, int mrcUser,
                                  int mrcPermission);

void modifyUC(int **UC, int *U, int *P, int userCount, int permissionCount);

void modifyUA(int **uaMatrix, int *U, int userCount, int roleCount);

void modifyPA(int **paMatrix, int *P, int permissionCount, int roleCount);

void formRoleProcedure(int v, int *U, int *P, int **UC, int **V, int mrcUser,
                       int mrcPerm, int *userRoleCount, int *permRoleCount,
                       int **uaMatrix, int **paMatrix, int userCount,
                       int permissionCount, int *roleCount);

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

  int mrcUser, mrcPermission;

  printf("Enter the value of the role-usage cardinality constraint: ");
  scanf("%d", &mrcUser);

  printf("Enter the value of the permission-distribution cardinality "
         "constraint: ");
  scanf("%d", &mrcPermission);

  int roleCount = concurrentProcessingFramework(
      upaMatrix, userCount, permissionCount, mrcUser, mrcPermission);

  printf("Number of roles = %d\n", roleCount);

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
int concurrentProcessingFramework(int **upaMatrix, int userCount,
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

  int roleCount = 0;

  // Phase 1
  for (int i = 0; i < userCount; i++) {
    for (int j = 0; j < permissionCount; j++) {
      if (UC[i][j] == 1 && userRoleCount[i] < mrcUser - 1) {
        int *U = (int *)malloc(userCount * sizeof(int));
        int *P = (int *)malloc(permissionCount * sizeof(int));

        int vertex = selectVertexWithHeuristic(UC, userCount, permissionCount);

        if (vertex < userCount) {
          formRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                            userRoleCount, permRoleCount, uaMatrix, paMatrix,
                            userCount, permissionCount, &roleCount);
        } else if (vertex >= userCount &&
                   vertex < userCount + permissionCount) {
          dualFormRoleProcedure(vertex - userCount, U, P, UC, upaMatrix,
                                mrcPerm, mrcPerm, userRoleCount, permRoleCount,
                                uaMatrix, paMatrix, userCount, permissionCount,
                                &roleCount);
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
          formRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                            userRoleCount, permRoleCount, uaMatrix, paMatrix,
                            userCount, permissionCount, &roleCount);
        } else if (vertex >= userCount &&
                   vertex < userCount + permissionCount) {
          dualFormRoleProcedure(vertex - userCount, U, P, UC, upaMatrix,
                                mrcUser, mrcUser, userRoleCount, permRoleCount,
                                uaMatrix, paMatrix, userCount, permissionCount,
                                &roleCount);
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
            formRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                              userRoleCount, permRoleCount, uaMatrix, paMatrix,
                              userCount, permissionCount, &roleCount);
          } else {
            dualFormRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                                  userRoleCount, permRoleCount, uaMatrix,
                                  paMatrix, userCount, permissionCount,
                                  &roleCount);
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
            formRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                              userRoleCount, permRoleCount, uaMatrix, paMatrix,
                              userCount, permissionCount, &roleCount);
          } else {
            dualFormRoleProcedure(vertex, U, P, UC, upaMatrix, mrcUser, mrcPerm,
                                  userRoleCount, permRoleCount, uaMatrix,
                                  paMatrix, userCount, permissionCount,
                                  &roleCount);
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

void modifyUA(int **uaMatrix, int *U, int userCount, int roleCount) {
  for (int i = 0; i < userCount; i++) {
    uaMatrix[i][roleCount - 1] = U[i];
  }
}

void modifyPA(int **paMatrix, int *P, int permissionCount, int roleCount) {
  for (int j = 0; j < permissionCount; j++) {
    paMatrix[j][roleCount - 1] = P[j];
  }
}

// TODO: Implement the function
void formRoleProcedure(int v, int *U, int *P, int **UC, int **V, int mrcUser,
                       int mrcPerm, int *userRoleCount, int *permRoleCount,
                       int **uaMatrix, int **paMatrix, int userCount,
                       int permissionCount, int *roleCount) {
                        
  U[v]=1;
  UserRoleCount[v] += 1;
  for(int i=0 ;i < permissionCount;i++)
  {
    int p = UC[v][i];
    if (p == 1 && permRoleCount[i] < mrcPerm - 1)
    {
      P[i]=1;
      PermRoleCount[i]+=1;
    }
    
  }

  for(int i=0;i < userRoleCount[i];i++)
  {
      if(i!=v && UserRoleCount[i]< mrcUser-1 && isSubset(P,V[i],permCount) && hasElement(UC[i],P,permCount))
      {
        U[i]=1;
        UserRoleCount[i] += 1;
      }

      else 
      {
        if(UserRoleCount[i]<mrcUser-1 && isSubset(P,V[i],permCount) && isSubset(UC[i],P,permCount))
        {
          U[i]=1;
          UserRoleCount[i] += 1;
        }
      }
  }
  int isSubset(int *uc, int *p, int size)
  {
    for(int i=0;i<size;i++)
    {
      if(uc[i]!=1 && p[i]==1)
      {
        return 0;
      }
    }
     return 1;
  }

  int hasElement(int *uc, int *p, int size) {
    for (int i = 0; i < size; i++) {
        if (uc[i] == 1 && p[i] == 1) {
            return 1;
        }
    }
    return 0;
}



                       }

// TODO: Implement the function
void dualFormRoleProcedure(int v, int *U, int *P, int **UC, int **V,
                           int mrcUser, int mrcPerm, int *userRoleCount,
                           int *permRoleCount, int **uaMatrix, int **paMatrix,
                           int userCount, int permissionCount, int *roleCount) {
}
