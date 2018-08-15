
#include "iniparser.h"

void fillParameters(char *line, char **servDef, char **servUnit, char **systemName){
    int i=0;
    int j=0;
    int first=1;
    char key[100];
    char value[100];
    value[0]='\0';
    key[0]='\0';

    for(;i<256;++i){
	if(line[i]=='\n'){
	    value[j]='\0';
	    break;
	}

	if(line[i]=='='){
	    key[i]='\0';
	    first=0;
	    continue;
	}

	if(first){
	    key[i]=line[i];
	}
	else{
	    value[j]=line[i];
	    ++j;
	}
    }

    if( strcmp(key, "serviceDefinition") == 0 ){
	*servDef = malloc(100*sizeof(char));
	strcpy(*servDef, value);
    }
    else if( strcmp(key, "serviceUnit") == 0 ){
	*servUnit = malloc(100*sizeof(char));
	strcpy(*servUnit, value);
    }
    else if( strcmp(key, "systemName") == 0 ){
	*systemName = malloc(100*sizeof(char));
	strcpy(*systemName, value);
    }

}

void readIni(char *filePath, char *_id, char **servDef, char **servUnit, char **systemName){
    FILE *pFile = fopen(filePath, "r");
    char line[256];
    char tmp[256] = "[";
    int match = 0;

    strcat(tmp,_id);
    strcat(tmp,"]\n");

    while(fgets(line, sizeof(line), pFile) != NULL){
	if(line[0]==';') continue;
	if(line[0]=='[') match=0;
	if(strcmp(line, tmp) == 0) {match=1; continue;}

	if(match){
	    fillParameters(line, servDef, servUnit, systemName);
	}
    }

    fclose(pFile);
}
