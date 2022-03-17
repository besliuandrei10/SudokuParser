#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_header.h"
#include "cJSON.h"

#define MASK0 "0000000000000000000000000000000000000000000000000"
#define MASK1 "0000000000001000000100000010000001000000100000000"
#define MASK2 "0000000011111000000100111110010000001111100000000"
#define MASK3 "0000000011111000000100111110000001001111100000000"
#define MASK4 "0000000010001001000100111110000001000000100000000"
#define MASK5 "0000000011111001000000111110000001001111100000000"
#define MASK6 "0000000011111001000000111110010001001111100000000"
#define MASK7 "0000000011111000000100000010000001000000100000000"
#define MASK8 "0000000011111001000100111110010001001111100000000"
#define MASK9 "0000000011111001000100111110000001001111100000000"

#define FILE_LEN 50
#define SIZE 73
#define BOARDSIZE 9
#define CELL_SIZE 55
#define NUMBER_OF_COLORS 3

char* CreateFileName(char* path, int task, int json)
{

  char *output = calloc(FILE_LEN, sizeof(char));

  char number[3] = "\0";
  char *p = strstr(path, "board");

  p = p + 5;
  strncpy(number, p, 2);

  if(json == 1) {

    sprintf(output, "output_task%d_board%s.bmp", task, number);

  } else {

    sprintf(output, "output_task%d_board%s.json", task, number);

  }

  return output;
}

void FreePixelMatrix(int*** mat)
{

  for(int i = 0; i < NUMBER_OF_COLORS; i++) {

    for(int j = 0; j < SIZE; j++) {

      free(mat[i][j]);

    }

    free(mat[i]);
  }

  free(mat);
}

int*** AllocPixelMatrix()
{

  int*** mat = calloc(NUMBER_OF_COLORS, sizeof(int**));
  if(mat == NULL) {

    printf("Allocation Failure! Stopping.");
    return NULL;

  }

  for(int i = 0; i < 3; i++) {

    mat[i] = calloc(SIZE, sizeof(int *));
    if(mat[i] == NULL) {

      printf("Allocation Failure! Stopping.");
      return NULL;

    }

    for(int j = 0; j < SIZE; j++) {

      mat[i][j] = calloc(SIZE, sizeof(int));
      if(mat[i][j] == NULL) {

        printf("Allocation Failure! Stopping.");
        return NULL;

      }

    }

  }

  return mat;

}

int** AllocSimpleMatrix()
{

  int **output = calloc(BOARDSIZE, sizeof(int *));

  for(int i = 0 ; i < BOARDSIZE; i++) {

      output[i] = calloc(BOARDSIZE, sizeof(int));
  }

  return output;
}

bmp_fileheader* AllocBmpFile()
{

  bmp_fileheader *imgHeader = calloc(1, sizeof(bmp_fileheader));

  return imgHeader;
}

bmp_infoheader* AllocBmpInfo()
{

  bmp_infoheader *imgInfo = calloc(1, sizeof(bmp_infoheader));

  return imgInfo;
}

char* readInputToString(FILE * fInput)
{

  char* input;
  u_int64_t length;

  fseek(fInput, 0, SEEK_END);
  length = ftell(fInput);
  fseek(fInput, 0, SEEK_SET);

  input = calloc(1 + length, sizeof(char));
  if(input == NULL) {

    printf("Allocation failure!\n");
    return NULL;

  }

  fgets(input, 1 + length, fInput);

  return input;
}

void ReadJSONPixels(cJSON* json, int*** mat)
{

  cJSON *bitmap = cJSON_GetObjectItem(json, "bitmap");
  int jsonIndex = 0;

  for(int i = 0; i < SIZE; i++) {

    for(int j = 0; j < SIZE; j++) {

      // 0 = Blue
      mat[0][i][j] = ( cJSON_GetArrayItem(bitmap, jsonIndex)->valueint );
      ++jsonIndex;

      // 1 = Green
      mat[1][i][j] = ( cJSON_GetArrayItem(bitmap, jsonIndex)->valueint );
      ++jsonIndex;

      // 2 = Red
      mat[2][i][j] = ( cJSON_GetArrayItem(bitmap, jsonIndex)->valueint );
      ++jsonIndex;

    }

  }

}

void ReadJSONData(cJSON* json, bmp_fileheader* imgHeader)
{
  cJSON *fileHeader = cJSON_GetObjectItem(json, "file_Header");


  cJSON *offset = cJSON_GetObjectItem(fileHeader, "offset");
  imgHeader->imageDataOffset = offset->valueint;

  cJSON *signature = cJSON_GetObjectItemCaseSensitive(fileHeader, "signature");
  char *temp = signature->valuestring;
  imgHeader->fileMarker1 = temp[0];
  imgHeader->fileMarker2 = temp[1];

  cJSON *reserved = cJSON_GetObjectItemCaseSensitive(fileHeader, "reserved");
  imgHeader->unused1 = reserved->valueint;
  imgHeader->unused2 = reserved->valueint;

  cJSON *file_size = cJSON_GetObjectItemCaseSensitive(fileHeader, "file_size");
  imgHeader->bfSize = file_size->valueint;

}

void ReadJSONInfo(cJSON* json, bmp_infoheader* imgInfo)
{

  cJSON *info = cJSON_GetObjectItem(json, "info_header");


  cJSON *colors_used = cJSON_GetObjectItem(info, "colors_used");
  imgInfo->biClrUsed = colors_used->valueint;

  cJSON *size = cJSON_GetObjectItem(info, "size");
  imgInfo->biSize = size->valueint;

  cJSON *colors_imp = cJSON_GetObjectItem(info, "colors_important");
  imgInfo->biClrImportant = colors_imp->valueint;

  cJSON *img_size = cJSON_GetObjectItem(info, "image_size");
  imgInfo->biSizeImage = img_size->valueint;

  cJSON *ypm = cJSON_GetObjectItem(info, "y_pixels_per_meter");
  imgInfo->biYPelsPerMeter = ypm->valueint;

  cJSON *xpm = cJSON_GetObjectItem(info, "x_pixels_per_meter");
  imgInfo->biXPelsPerMeter = xpm->valueint;

  cJSON *width = cJSON_GetObjectItem(info, "width");
  imgInfo->width = width->valueint;

  cJSON *height = cJSON_GetObjectItem(info, "height");
  imgInfo->height = height->valueint;

  cJSON *planes = cJSON_GetObjectItem(info, "planes");
  imgInfo->planes = planes->valueint;

  cJSON *bit_count = cJSON_GetObjectItem(info, "bit_count");
  imgInfo->bitPix = bit_count->valueint;

  cJSON *compression = cJSON_GetObjectItem(info, "compression");
  imgInfo->biCompression = compression->valueint;


}

void ReadBMPixels(FILE* fInput, int*** mat)
{

  for(int i = SIZE - 1; i >= 0; i--) {

    for(int j = 0; j < SIZE; j++) {

      fread(&(mat[0][i][j]), 1, 1, fInput);
      fread(&(mat[1][i][j]), 1, 1, fInput);
      fread(&(mat[2][i][j]), 1, 1, fInput);

    }

    fseek(fInput, 1, SEEK_CUR);
  }

}

void ReadBMPData(FILE* fInput, bmp_fileheader* imgHeader)
{

  fread(&(imgHeader->fileMarker1), 1, 1, fInput);
  fread(&(imgHeader->fileMarker2), 1, 1, fInput);

  fread(&(imgHeader->bfSize), 4, 1, fInput);
  fread(&(imgHeader->unused1), 2, 1, fInput);
  fread(&(imgHeader->unused2), 2, 1, fInput);
  fread(&(imgHeader->imageDataOffset), 4, 1, fInput);

}

void ReadBMPInfo(FILE* fInput, bmp_infoheader* imgInfo)
{

  fread(&(imgInfo->biSize), 4, 1, fInput);

  fread(&(imgInfo->width), 4, 1, fInput);
  fread(&(imgInfo->height), 4, 1, fInput);
  fread(&(imgInfo->planes), 2, 1, fInput);
  fread(&(imgInfo->bitPix), 2, 1, fInput);

  fread(&(imgInfo->biCompression), 4, 1, fInput);
  fread(&(imgInfo->biSizeImage), 4, 1, fInput);
  fread(&(imgInfo->biXPelsPerMeter), 4, 1, fInput);
  fread(&(imgInfo->biYPelsPerMeter), 4, 1, fInput);
  fread(&(imgInfo->biClrUsed), 4, 1, fInput);
  fread(&(imgInfo->biClrImportant), 4, 1, fInput);

}

void WriteFileHeader(bmp_fileheader* imgHeader, FILE* output)
{
  fputc(imgHeader->fileMarker1, output);
  fputc(imgHeader->fileMarker2, output);

  fwrite(&(imgHeader->bfSize), 4, 1, output);
  fwrite(&(imgHeader->unused1), 2, 1, output);
  fwrite(&(imgHeader->unused2), 2, 1, output);

  fwrite(&(imgHeader->imageDataOffset), 4, 1, output);

}

void WriteInfoHeader(bmp_infoheader* imgInfo, FILE* output)
{

  fwrite(&(imgInfo->biSize), 4, 1, output);
  fwrite(&(imgInfo->width), 4, 1, output);
  fwrite(&(imgInfo->height), 4, 1, output);

  fwrite(&(imgInfo->planes), 2, 1, output);
  fwrite(&(imgInfo->bitPix), 2, 1, output);

  fwrite(&(imgInfo->biCompression), 4, 1, output);
  fwrite(&(imgInfo->biSizeImage), 4, 1, output);
  fwrite(&(imgInfo->biXPelsPerMeter), 4, 1, output);
  fwrite(&(imgInfo->biYPelsPerMeter), 4, 1, output);
  fwrite(&(imgInfo->biClrUsed), 4, 1, output);
  fwrite(&(imgInfo->biClrImportant), 4, 1, output);

}

void WritePixels(int*** mat, FILE* output)
{

  for(int i = SIZE - 1; i >= 0; i--) {

    for(int j = 0; j < SIZE; j++) {

      fwrite(&(mat[0][i][j]), 1, 1, output);
      fwrite(&(mat[1][i][j]), 1, 1, output);
      fwrite(&(mat[2][i][j]), 1, 1, output);

    }

    fputc(0, output);
  }

}

void FlipCell(int startline, int startcol, int*** mat)
{

  for(int i = startline; i < startline + 7; i++) {

    for(int j = startcol; j < startcol + 3; j++) {

      int aux = 0;
      // 0 = Blue
      aux = mat[0][i][j];
      mat[0][i][j] = mat[0][i][startcol + 6 - (j - startcol)];
      mat[0][i][startcol + 6 - (j - startcol)] = aux;

      // 1 = Green
      aux = mat[1][i][j];
      mat[1][i][j] = mat[1][i][startcol + 6 - (j - startcol)];
      mat[1][i][startcol + 6 - (j - startcol)] = aux;

      // 2 = Red
      aux = mat[2][i][j];
      mat[2][i][j] = mat[2][i][startcol + 6 - (j - startcol)];
      mat[2][i][startcol + 6 - (j - startcol)] = aux;

    }

  }

}

void ReadCell(int*** mat, int startline, int startcol, int** board)
{

  int b, g, r;
  char* number = calloc(CELL_SIZE, sizeof(char));

  for(int i = startline; i < startline + 7; i++) {

    for(int j = startcol; j < startcol + 7; j++) {

      // 0 = Blue
      b = mat[0][i][j];

      // 1 = Green
      g = mat[1][i][j];

      // 2 = Red
      r = mat[2][i][j];

      if(b == 255 && g == 255 && r == 255) {

        char tmp[2] = "0";
        strcat(number, tmp);

      } else {

        char tmp[2] = "1";
        strcat(number, tmp);

      }


    }

    if(strcmp(number, MASK0) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 0;

    } else if(strcmp(number, MASK1) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 1;

    } else if(strcmp(number, MASK2) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 2;

    } else if(strcmp(number, MASK3) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 3;

    } else if(strcmp(number, MASK4) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 4;

    } else if(strcmp(number, MASK5) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 5;

    } else if(strcmp(number, MASK6) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 6;

    } else if(strcmp(number, MASK7) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 7;

    } else if(strcmp(number, MASK8) == 0) {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 8;

    } else {

      board[(startline - 1) / 8][(startcol - 1) / 8] = 9;

    }

  }

  free(number);

}

void FreeSimpleMatrix(int ** mat)
{

  for(int i = 0; i < BOARDSIZE; i++) {

    free(mat[i]);

  }

  free(mat);
}

void CreateSimpleMatrix(int *** mat, int ** output)
{

  for(int i = 1; i < SIZE; i += 8) {

    for(int j = 1; j < SIZE; j += 8) {

      ReadCell(mat, i, j, output);

    }
  }

}

int CheckVertical(int col, int ** board)
{
  int freq[10] = {0};

  for(int i = 0; i < BOARDSIZE; i++) {

    freq[board[i][col]]++;

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] != 1) {

      return 1;

    }

  }

  return 0;
}

int CheckHorizontal(int line, int ** board)
{
  int freq[10] = {0};

  for(int i = 0; i < BOARDSIZE; i++) {

    freq[board[line][i]]++;

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] != 1) {

      return 1;

    }

  }

  return 0;
}

int CheckSquare(int line, int col, int ** board)
{
  int freq[10] = {0};

  for(int i = line; i < line + 3; i++) {

    for(int j = col; j < col + 3; j++) {

      freq[board[i][j]]++;

    }

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] != 1) {

      return 1;

    }

  }

  return 0;
}

int CheckGame(int** board)
{

  for(int i = 1; i < BOARDSIZE; i++) {

    if(CheckHorizontal(i, board) == 1) {

      return 1;

    }

    if(CheckVertical(i, board) == 1) {

      return 1;

    }


  }

  for(int i = 0; i < BOARDSIZE; i += 3) {

    for(int j = 0; j < BOARDSIZE; j += 3) {

      if(CheckSquare(i, j, board) == 1) {

        return 1;

      }


    }

  }

  return 0;
}

int SearchVertical(int col, int** board)
{
  int freq[10] = {0}, value;

  for(int i = 0; i < BOARDSIZE; i++) {

    freq[board[i][col]]++;

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] == 0) {

      value = i;

    }

  }

  if(freq[0] != 1) {

    return -1;

  } else {

    return value;

  }

}

int SearchHorizontal(int line, int** board)
{

  int freq[10] = {0}, value;

  for(int i = 0; i < BOARDSIZE; i++) {

    freq[board[line][i]]++;

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] == 0) {

      value = i;

    }

  }

  if(freq[0] != 1) {

    return -1;

  } else {

    return value;

  }

}

int SearchCell(int line, int col, int** board)
{

  int freq[10] = {0}, value;
  int startLine = line;
  int startCol = col;

  if(line % 3 != 0)
  {

    startLine /= 3;

  }

  if(startCol % 3 != 0) {

    startCol /= 3;

  }

  for(int i = startLine; i < startLine + 3; i++) {

    for(int j = startCol; j < startCol + 3; j++) {

      freq[board[i][j]]++;

    }

  }

  for(int i = 1; i < 10; i++) {

    if(freq[i] == 0) {

      value = i;

    }

  }

  if(freq[0] != 1) {

    return -1;

  } else {

    return value;
  }

}

void FillCell(int line, int col, int value, int*** mat)
{
  int startLine = (line * 8) + 1;
  int startCol = (col * 8) + 1;
  char *output = calloc(CELL_SIZE, sizeof(char));

  if(value == 1) {

    strcpy(output, MASK1);

  } else if(value == 2) {

    strcpy(output, MASK2);

  } else if(value == 3) {

    strcpy(output, MASK3);

  } else if(value == 4) {

    strcpy(output, MASK4);

  } else if(value == 5) {

    strcpy(output, MASK5);

  } else if(value == 6) {

    strcpy(output, MASK6);

  } else if(value == 7) {

    strcpy(output, MASK7);

  } else if(value == 8) {

    strcpy(output, MASK8);

  } else if(value == 9) {

    strcpy(output, MASK9);

  }

  int k = 0;

  for(int i = startLine; i < startLine + 7; i++) {

    for(int j = startCol; j < startCol + 7; j++) {

      if(output[k] == '1') {

        mat[0][i][j] = 255;
        mat[1][i][j] = 0;
        mat[2][i][j] = 255;

      }

      ++k;

    }

  }

  free(output);
}

void SolveSudoku(int*** mat, int** board)
{

  int done = 1;

  do
  {
    done = 0;

    for(int i = 0; i < BOARDSIZE; i++) {

      for(int j = 0; j < BOARDSIZE; j++) {

        if(board[i][j] == 0) {

          int outputVert = SearchVertical(j, board);
          int outputHorz = SearchHorizontal(i, board);
          int outputCell = SearchCell(i, j, board);

          if(i == 7 && j == 3) {

            printf("hoz:%d vert:%d cell:%d\n", outputHorz, outputVert, outputCell);

          }

          if(outputVert != -1) {

            board[i][j] = outputVert;
            FillCell(i, j, outputVert, mat);

          } else if(outputHorz != -1) {

            board[i][j] = outputHorz;
            FillCell(i, j, outputHorz, mat);

          } else if(outputCell != -1) {

            board[i][j] = outputCell;
            FillCell(i, j, outputCell, mat);

          } else {

            done = 1;

          }

        }
      }
    }

  }
  while(done == 1);

}

void Task123(int*** mat, char* filename, bmp_fileheader* imgHeader,
             bmp_infoheader* imgInfo, int** board)
{

  // Read
  FILE * fInput = fopen(filename, "r");
  char* input;
  input = readInputToString(fInput);
  cJSON *json = cJSON_Parse(input);

  ReadJSONPixels(json, mat);
  ReadJSONData(json, imgHeader);
  ReadJSONInfo(json, imgInfo);

  // Task 1
  char *fOutName = CreateFileName(filename, 1, 1);
  FILE * fout = fopen(fOutName, "wb");

  WriteFileHeader(imgHeader, fout);
  WriteInfoHeader(imgInfo, fout);
  WritePixels(mat, fout);
  fclose(fout);
  free(fOutName);

  // Task 2
  fOutName = CreateFileName(filename, 2, 1);
  fout = fopen(fOutName, "wb");

  for(int i = 1; i < SIZE; i += 8) {

    for(int j = 1; j < SIZE; j += 8) {

      FlipCell(i, j, mat);

    }

  }

  WriteFileHeader(imgHeader, fout);
  WriteInfoHeader(imgInfo, fout);
  WritePixels(mat, fout);
  fclose(fout);
  free(fOutName);

  // Task 3
  fOutName = CreateFileName(filename, 3, 0);
  fout = fopen(fOutName, "w");

  CreateSimpleMatrix(mat, board);
  cJSON *jsonOut = cJSON_CreateObject();

  int condition = CheckGame(board); // 0 = won, 1 = loss

  if(condition == 0) {

    cJSON *input_file = cJSON_CreateString(strstr(filename, "board"));
    cJSON *game_state = cJSON_CreateString("Win!");

    cJSON_AddItemToObject(jsonOut, "input_file", input_file);
    cJSON_AddItemToObject(jsonOut, "game_state", game_state);

  } else {

    cJSON *input_file = cJSON_CreateString(strstr(filename, "board"));
    cJSON *game_state = cJSON_CreateString("Loss :(");

    cJSON_AddItemToObject(jsonOut, "input_file", input_file);
    cJSON_AddItemToObject(jsonOut, "game_state", game_state);

  }

  char *output = cJSON_Print(jsonOut);
  fprintf(fout, "%s\n", output);

  fclose(fInput);
  fclose(fout);
  cJSON_Delete(jsonOut);
  cJSON_Delete(json);
  free(input);
  free(output);
  free(fOutName);
}

void Task4(int*** pixelMatrix, char* filename, bmp_fileheader* imgHeader,
             bmp_infoheader* imgInfo, int** board)
{

  char *fOutName = CreateFileName(filename, 4, 1);
  FILE * fout = fopen(fOutName, "wb");
  FILE * fInput = fopen(filename, "rb");

  ReadBMPData(fInput, imgHeader);
  ReadBMPInfo(fInput, imgInfo);
  ReadBMPixels(fInput, pixelMatrix);

  CreateSimpleMatrix(pixelMatrix, board);
  SolveSudoku(pixelMatrix, board);

  WriteFileHeader(imgHeader, fout);
  WriteInfoHeader(imgInfo, fout);
  WritePixels(pixelMatrix, fout);

  fclose(fout);
  fclose(fInput);
  free(fOutName);

}

int main(int argc, char* argv[])
{

  // preluare argumente program pentru a afla taskul
  if( argc > 3 ) {
    printf("Too many arguments supplied! Stopping.");
    return 1;
  }
  int tasks = atoi(argv[2]);

  // alocare structs pentru datele imaginii
  int*** pixelMatrix = AllocPixelMatrix();
  bmp_fileheader* imgHeader = AllocBmpFile();
  bmp_infoheader* imgInfo = AllocBmpInfo();

  if( imgInfo == NULL || imgHeader == NULL || pixelMatrix == NULL) {

    return 1;

  }

  // actually solving the problem
  int** board = AllocSimpleMatrix();
  switch (tasks) {

    case 123: Task123(pixelMatrix, argv[1], imgHeader, imgInfo, board); break;
    case 4: Task4(pixelMatrix, argv[1], imgHeader, imgInfo, board); break;

  }

  // free-uri
  FreeSimpleMatrix(board);
  FreePixelMatrix(pixelMatrix);
  free(imgHeader);
  free(imgInfo);

  return 0;
}
