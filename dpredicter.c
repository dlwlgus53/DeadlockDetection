#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
  int start;
  int end;
} token_t;

/*parser structure*/
typedef struct{
   unsigned int toknext ;
   unsigned int pos ;
}parser;

/*Function Declaration*/
static token_t* alloc_token(parser *p,token_t *tokens,int num_tokens);

int main (int argc,char **argv){

   /*File Read & Copy to Buffer*/
    FILE *fp;
    const int maxBufLen = 1000;
    const int maxDataLen = 1000;
    char buffer[maxBufLen];
    char data[maxDataLen];
    fp=fopen(argv[1],"r");
    int length = 0;
    if (argc != 2){
        printf("interface error!\n");
    }
    while(fscanf(fp,"%s",buffer)>0)
    {
        if((length+strlen(buffer)+1)>=maxDataLen)break;
        strncpy(&data[length],buffer,strlen(buffer));
        length += strlen(buffer)+1;
        data[length-1]=' ';
    }
    data[length] = 0x00;
    //printf("The string is : \n%s\n",data);
    fclose(fp);

     /* Make Tokens */
    token_t tokens[100];
    /*Declaration Variables*/
    token_t* token;
    parser p;
    p.toknext=0;
    p.pos=0;
    int count = p.toknext;
    int i;
    
    /*Sweeping all Data*/
    for(;p.pos<strlen(data) && data[p.pos]!='\0';p.pos++){
        char c;
        c=data[p.pos];
        switch(c){
        case '[': 
            token = alloc_token(&p,tokens,100);
        
            if (token == NULL) {
                    return -1;
            }
            token->start = p.pos;
            break;

        case ']':
            for (i = p.toknext - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        token->end = p.pos + 1;
                        break;
                    }
            }
            break;
        default:
            break;
        }
    }

    /*confirm address*/
    //printf("%.*s\n",tokens[1].end-tokens[1].start-2,data+tokens[1].start+1);
    /*copy address*/
    char address[50];
    strncpy(address,data+tokens[1].start+1,tokens[1].end-tokens[1].start-2);
   // printf("%s\n",address);


    /*addr2line*/
    char command[50];
    strcpy(command,"addr2line -e dinning ");
    strcat(command,address);
    system(command);

    return(0);


}

static token_t* alloc_token(parser *p,token_t *tokens,int num_tokens)
{
    token_t* tok;
    if(p->toknext >= num_tokens ){
        return NULL;
    }
    tok = &tokens[p->toknext];
    p->toknext++;
    tok->start = tok->end = -1;
  
    return tok;
}