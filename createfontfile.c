/**
 *  createfontfile.c
 *  -------------
 *
 *  Create font file for a 12x12 font (12 dots horizontally and 12 dots
 *  vertically) from a text file describing the font.
 *  Each line in the text file describes the leftmost 10 columns of a
 *  row of the font. Each character has 9 lines describing the top
 *  9 rows of the font. 
 *  The rightmost 2 columns and the bottom 3 rows are always blank.
 *  A dash (-) represents a dot that is not printed.
 *  A hash (#) represents a dot that is printed.
 *  Blank lines are ignored. They are usually placed between each character.
 *  
 *  In the font file two bytes represent the 12 columnns of the font.
 *  The first byte has the leftmost 8 columns, with the most significant
 *  bit representing the leftmost column. The second byte represents the
 *  remaining four columns, with the most significant bit representing
 *  the leftmost column. The least significant four bits in the second byte
 *  are always zero.
 *  The rows of a character are in sequence, with 12 rows or 24 bytes
 *  for each character.
 */

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FONT_EXT  ".D12"


static void convert(FILE* fi, FILE* fo);
static void usage(const char* progname);


int main(int argc, char* argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        char* infile  = argv[i];
        char* outfile = malloc(strlen(infile) + 4);
        char* ext     = strrchr(basename(infile), '.');
        FILE* fi;

        if (ext != NULL) {
            strcpy(outfile, infile);
            strcpy(strrchr(outfile, '.'), FONT_EXT);
        } else {
            strcat(strcpy(outfile, infile), FONT_EXT);
        }

        printf("%s", infile);

        fi = fopen(infile, "rb");

        if (fi == NULL) {
            puts(" cannot be opened.");
        } else {
            FILE* fo = fopen(outfile, "wb");

            if (fo == NULL) {
                puts(": Cannot open output file.");
            } else {
                convert(fi, fo);
                fclose(fo);
            }
            fclose(fi);
        }
    }
    return 0;
}


void
convert(FILE* fi, FILE* fo)
{
  int character;
  int row;
  int column;
  int lineno = 0;
  
  for (character = 0; character < 256; character++) {
    for (int row = 0; row < 9; row++) {
      char line[80];
      int column;
      int nextdot;
      int dots = 0;
      char* result;
      while ((result = fgets(line, 80, fi)) != NULL && line[0] == '\n')
        lineno++;
      lineno++;
      if (result == NULL) {
        fprintf(stderr, "Unexpected end of file (character %d, row %d, lineno %d)\n", character, row, lineno);
        exit(1);
      }
      if (strlen(line) != 10) {
        fprintf(stderr, "Line was %ld characters, should be 9 characters (character %d, row %d), lineno %d\n",
         strlen(line), character, row, lineno);
      }
      //fprintf(stderr, "Character %d inputrow %d lineno %4d %s", character, row, lineno, line);
      for (column = 0, nextdot = 0; column < 9; column++) {
        dots <<= 1;  
        if (line[column] == '#') {
          if (nextdot) {
            fprintf(stderr, "# followed # (character %d, row %d, column %d, lineno %d)\n",
              character, row, column, lineno);
          }
          dots ^= 1;
          nextdot = 1;
        } else if (line[column] == '-') {
          if (nextdot) {
            dots ^= 1;
            nextdot = 0;
          }
        } else {
          fprintf(stderr, "Character was %c, should be # or - (character %d, row %d, lineno %d)\n",
            line[column], character, row, lineno);
        }        
      }
      /* columns 9 to 11 are not coded for */
      for (column = 9; column < 12; column++) {
        dots <<= 1;
        if (nextdot) {
          dots ^= 1;
          nextdot = 0;
        }
      }
      dots <<= 4; // pad 4 bits to take to 16 bits;
        
      fputc(dots >> 8 & 0xff, fo); // Left most dots go first
      fputc(dots & 0xff, fo);
    }
    for (row = 9; row < 12; row++) { // Last 6 rows are always empty
      fputc(0, fo);
      fputc(0, fo);
    }
  }
}


void
usage(const char* progname)
{
    printf("Usage: %s fontfile [fontfile ...]\n", progname);
}

