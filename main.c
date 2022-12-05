#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix__
#include <stdlib.h>
#include <unistd.h>

#elif defined(_WIN32) || defined(WIN32)
#define OS_Windows
#include <windows.h>
#endif

#define MemoryCapacityN 100000

typedef struct {
  char cityName[80];
  int cases;
} RowData;

struct FileStruct {
  FILE *file;
  int position, maxSize;
  RowData *buffer;
};

int compareByCases(RowData *a, RowData *b) {
  if (a->cases == b->cases) {
    return 0;
  } else {
    if (a->cases < b->cases) {
      return -1;
    } else {
      return 1;
    }
  }
}

void saveFile(char *fileName, RowData *dataInMemory, int dataSize,
              int changeFinalLine) {
  int i;
  FILE *manipulatedFile = fopen(fileName, "a");

  for (i = 0; i < (dataSize - 1); i++) {
    fprintf(manipulatedFile, "%s;%d\n", dataInMemory[i].cityName,
            dataInMemory[i].cases);
  }

  if (changeFinalLine == 0) {
    fprintf(manipulatedFile, "%s;%d", dataInMemory[dataSize - 1].cityName,
            dataInMemory[dataSize - 1].cases);
  } else {
    fprintf(manipulatedFile, "%s;%d\n", dataInMemory[dataSize - 1].cityName,
            dataInMemory[dataSize - 1].cases);
  }

  fclose(manipulatedFile);
}

RowData getColumnContentsInRow(char *dataInMemory, int *columns,
                               RowData rowsData) {
  int i = 0, j = columns[0];
  char *columnContent = strtok(strdup(dataInMemory), ";");

  strcpy(rowsData.cityName, columnContent);
  columnContent = strtok(NULL, ";");

  rowsData.cases = atoi(columnContent);

  i++;
  j = columns[i];

  return rowsData;
}

int createSortedFiles() {
  static RowData dataInMemory[MemoryCapacityN];
  int numberOfFilesCount = 0, totalDataInMemory = 0;
  char newFileName[20];
  FILE *dataFile;

  if ((dataFile = fopen("municipios.csv", "r")) == NULL) {
    printf("Erro ao abrir o arquivo");
    return -1;
  }

  char rowContent[180];
  int columns[] = {1, 4}, i = 0;

  while (!feof(dataFile)) {
    fgets(rowContent, 180, dataFile);

    dataInMemory[totalDataInMemory] = getColumnContentsInRow(
        rowContent, columns, dataInMemory[totalDataInMemory]);
    totalDataInMemory++;

    if (totalDataInMemory == MemoryCapacityN) {
      numberOfFilesCount++;

      sprintf(newFileName, "temp%d.csv", numberOfFilesCount);
      qsort(dataInMemory, totalDataInMemory, sizeof(RowData), compareByCases);
      saveFile(newFileName, dataInMemory, totalDataInMemory, 0);

      totalDataInMemory = 0;
    }
  }

  if (totalDataInMemory > 0) {
    numberOfFilesCount++;
    sprintf(newFileName, "temp%d.csv", numberOfFilesCount);
    qsort(dataInMemory, totalDataInMemory, sizeof(RowData), compareByCases);
    saveFile(newFileName, dataInMemory, totalDataInMemory, 0);
  }

  fclose(dataFile);

  return numberOfFilesCount;
}

void insertDataInTheBuffer(struct FileStruct *file, int bufferSize) {
  int i;
  char rowContent[180];
  int columns[] = {1, 4};

  if (file->file == NULL) {
    return;
  }

  file->position = 0;
  file->maxSize = 0;

  for (i = 0; i < bufferSize; i++) {
    if (!feof(file->file)) {
      // fscanf(file->file, "%d", &file->buffer[file->maxSize].cases);
      file->maxSize++;
    } else {
      fclose(file->file);
      file->file = NULL;
      break;
    }
  }
}

int findLowerValue(struct FileStruct *file, int numberOfFiles, int bufferSize,
                   RowData *lowerValue) {
  int i, foundAValue = -1;

  for (i = 0; i < numberOfFiles; i++) {
    if (file[i].position < file[i].maxSize) {
      if (foundAValue == -1) {
        foundAValue = i;
      } else {
        if (file[i].buffer[file[i].position].cases <
            file[foundAValue].buffer[file[foundAValue].position].cases) {
          foundAValue = i;
        }
      }
    }
  }

  if (foundAValue != -1) {
    *lowerValue = file[foundAValue].buffer[file[foundAValue].position];

    file[foundAValue].position++;

    if (file[foundAValue].position == file[foundAValue].maxSize) {
      insertDataInTheBuffer(&file[foundAValue], bufferSize);
    }

    return 1;
  } else {
    return 0;
  }
}

void mergeSortedFiles(int numberOfFiles, int bufferSize) {
  char newFileName[20];
  int i;
  RowData *buffer = (RowData *)malloc(bufferSize * sizeof(RowData));

  struct FileStruct *file;

  file = (struct FileStruct *)malloc(numberOfFiles * sizeof(struct FileStruct));

  for (i = 0; i < numberOfFiles; i++) {
    sprintf(newFileName, "temp%d.csv", i + 1);
    file[i].file = fopen(newFileName, "r");
    file[i].position = 0;
    file[i].maxSize = 0;
    file[i].buffer = (RowData *)malloc(bufferSize * sizeof(RowData));
    insertDataInTheBuffer(&file[i], bufferSize);
  }

  int numberOfBuffers = 0;
  RowData lowerValue;

  while (findLowerValue(file, numberOfFiles, bufferSize, &lowerValue) == 1) {
    buffer[numberOfBuffers] = lowerValue;
    numberOfBuffers++;

    if (numberOfBuffers == bufferSize) {
      saveFile(newFileName, buffer, numberOfBuffers, 1);
    }
  }

  if (numberOfBuffers != 0) {
    saveFile(newFileName, buffer, numberOfBuffers, 1);
  }

  // for (i = 0; i < numberOfFiles; i++) {
  //   free(file[i].buffer);
  // }

  // free(file);
  // free(buffer);
}

void externalQuicksort() {
  char newFileName[20];
  int numberOfFiles = createSortedFiles();
  int i, bufferSize = MemoryCapacityN / (numberOfFiles + 1);

  mergeSortedFiles(numberOfFiles, bufferSize);

  // for (i = 0; i < numberOfFiles; i++) {
  //   sprintf(newFileName, "temp%d.csv", i + 1);
  //   remove(newFileName);
  // }
}

void handleSelectedMenuOption(int menuOptionNumber) {
#ifdef OS_Windows
  system("cls");
#else
  system("clear");
#endif

  switch (menuOptionNumber) {
  case 1:
    externalQuicksort();
    printf("\nfunção de ordenar por óbitos\n");
    break;

  case 2:
    printf("Função para mostrar o município com mais casos\n");
    break;

  case 0:
    printf("Até a próxima! Bye bye!!!\n");
    break;

  default:
    printf("Digite uma opção válida!\n");
  }

#ifdef OS_Windows
  system("pause");
  system("cls");
#else
  system("echo 'Pressione enter para continuar...'; read dummy;");
  system("clear");
#endif
}

void menu() {
  int menuOptionNumber = 0;

  do {
    printf("|-----------------------------------------------------|\n");
    printf("|                         Menu                        |\n");
    printf("|-----------------------------------------------------|\n");
    printf("| 1. Ordenar por óbitos                               |\n");
    printf("|-----------------------------------------------------|\n");
    printf("| 2. Município com mais casos                         |\n");
    printf("|-----------------------------------------------------|\n");
    printf("| 0. Sair                                             |\n");
    printf("|-----------------------------------------------------|\n");

    scanf("%d", &menuOptionNumber);

    handleSelectedMenuOption(menuOptionNumber);
  } while (menuOptionNumber);
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "Portuguese");
  menu();

  return 0;
}
