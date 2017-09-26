#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>


#define WRONG_NUMBER_PARAMETERS (-1)
#define CANT_OPEN_TEXTFILE (-2)
#define CANT_OPEN_SYNONYMSFILE (-3)
#define CANT_OPEN_NEWTEXTFILE (-4)
#define MAX_LINE_LEN (600)
#define MAX_WORD_LEN (30)
#define ERROR_IN_MALLOC (1)

typedef enum { false, true } bool;

//represent all the synonyms of one word
typedef struct  synonyms
{
    char** words; // array of the synonyms
    int len; // number of synonyms
}synonyms;

void deleteSynonyms(synonyms*);

//if the synonyms struct contain the given word return true else false
bool hasWord(synonyms*, char*);

//returns the synonyms struct that contain the given word, if none contain the given word returns NULL
synonyms* hasSynonyms(synonyms*, char*);

//returns random synonym of the given one
char* newWord(synonyms*);

//gets line from the synonyms file and create synonyms struct that contains all the synonyms in the line
int createSywords(synonyms* ,char*);

//make array of synonyms structs filled with the content of the synonyms file
synonyms* readcsv(char*);

//returns the number of lines in file
int countLines(FILE*);

//returns the number of words in line
int countWords(char*);

//create new file with the replaced synonyms
int replace(char*, char *);


int main(int argc, char ** argv){

    srand(time(NULL));
    //check if given the right number of arguments
    if(argc!=3)
    {
        printf("The program needs 2 files\n");
        return WRONG_NUMBER_PARAMETERS;
    }

    replace(argv[1],argv[2]);
    return 0;
}


bool hasWord(synonyms* ps, char* word)
{
    int i;
    for(i=0; i < (ps->len) ;i++)
    {
        if (strcmp(word,(ps->words)[i])==0)
            return true;
    }

    return false;
}

synonyms* hasSynonyms(synonyms* ps,char* word)
{
    if(ps->len == 0)
    {
        return NULL;
    }

    if(hasWord(ps,word)==true)
    {
        return ps;
    }

    else
        return hasSynonyms(++ps,word);

}


char* newWord(synonyms* ps)
{
    int r = rand();
    return (ps->words)[r%ps->len];
}

int createSywords(synonyms* ps, char* line)
{

    int numberOfWords = 0 , i;
    char* pch = NULL;


    if(strcmp(line,"")==0)
    {
        ps->len = 0;
        ps->words = NULL;
        return 0;
    }

    numberOfWords = countWords(line);
    if(NULL==(ps->words = (char**)malloc(sizeof(char*)*numberOfWords)))
    {
        printf("Error:: createSywords malloc\n");
        return ERROR_IN_MALLOC;
    }

    ps->len = numberOfWords;

    //go over all the words in the line and add them to the same synonyms struct
    pch = strtok (line,",");
    for (i=0;i<numberOfWords;i++)
    {
        (ps->words)[i] = (char*)malloc(sizeof(char)*(strlen(pch)+1)); // +1 for the null
        strcpy((ps->words)[i],pch);

        pch = strtok (NULL, ",");  // continue from the last point
    }

}

synonyms* readcsv(char* filename)
{
    FILE* fileSynonyms = NULL;
    synonyms* ps = NULL;
    int numberOfLines = 0, i=0;
    char line[MAX_LINE_LEN] = {0};

    fileSynonyms = fopen(filename,"r");

    if(!fileSynonyms)
    {
        printf("Can't open %s", filename);
        return CANT_OPEN_SYNONYMSFILE;
    }

    numberOfLines = countLines(fileSynonyms);

    if(NULL== (ps = (synonyms*) malloc(sizeof(synonyms)*(numberOfLines+1)))) // +1 for empty struct representing the end
    {
        printf("Error:: readcsv malloc\n");
        return ERROR_IN_MALLOC;
    }


    // for each line
    for(i =0;(i<numberOfLines) && fgets (line,MAX_LINE_LEN, fileSynonyms)!=NULL ;i++)
    {
        createSywords(ps+i,line);
    }

    createSywords(ps+numberOfLines,""); // the last struct

    fclose(fileSynonyms);
    return ps;
}


int countLines(FILE* fp)
{
    char ch;
    int lines = 0;

    while(!feof(fp))
    {
        ch = fgetc(fp);
        if(ch == '\n')
        {
            lines++;
        }
    }
    fseek(fp,0,SEEK_SET); //next time the reading/writing will be from the beginning
    return lines;
}

int countWords(char* line)
{
    char ch;
    int words = 0, i=0;

    while((ch=line[i++])!=NULL)
    {
        if(ch == ',') {
            words++;
        }
    }

    return words+1;
}

int replace(char* txtFileName,char* synonymsFileName)
{
    FILE* ftext = NULL;
    FILE* fnewText = NULL;
    synonyms* ps= NULL;
    synonyms* curr= NULL;
    char word[MAX_WORD_LEN] = {0};

    if(!(ftext = fopen(txtFileName,"r")))
    {
        printf("Can't open %s", txtFileName);
        return CANT_OPEN_TEXTFILE;
    }

    if(!( fnewText = fopen("newText.txt","w+")))
    {
        printf("Can't open %s", "newText.txt");
        return CANT_OPEN_NEWTEXTFILE;
    }

    if (CANT_OPEN_SYNONYMSFILE==(ps = readcsv(synonymsFileName)))
    {
        return CANT_OPEN_SYNONYMSFILE;
    }

    //go over the text file
    while ((word[0] = fgetc(ftext)) != EOF)
    {
        word[1] = '\0';
        // if the char is a letter and there are more letters after make a complete word
        if (isalpha(word[0])&&fscanf(ftext,"%[a-zA-Z]",word+1)!=EOF)
        {
            if(NULL!=(curr=hasSynonyms(ps,word))) // check if the word has synonyms
            {
                strcpy(word,newWord(curr));

            }
        }

        fwrite(word ,1,  strlen(word), fnewText );
    }

    free(ps);

    fclose(fnewText);
    fclose(ftext);
}

void deleteSynonyms(synonyms* ps)
{
    int i;
    if(ps!=NULL)
    {
        for(i = 0; i<ps->len; i++)
        {
            free(ps->words[i]);
        }

        deleteSynonyms(++ps);
    }
    free(ps);
}