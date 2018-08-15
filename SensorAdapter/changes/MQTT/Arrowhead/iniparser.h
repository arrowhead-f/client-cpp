
#ifndef INIPARSER_H
#define INIPARSER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void fillParameters(char *line, char **servDef,char **servUnit, char **systemName);

void readIni(char *filePath, char *_id,  char **servDef, char **servUnit, char **systemName);

#endif
