#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int check_isalphanum(char *s)
{
  int i;
  
  for(i = 0; i < strlen(s); i++)
    if(!isalnum(s[i]) && s[i] != '_')
    {
      return 0;
    }

  return 1;
}

int check_isalpha(char *s)
{
  int i;
  
  for(i = 0; i < strlen(s); i++)
    if(!isalpha(s[i]) && s[i] != '_')
      return 0;

  return 1;
}

int main(int argc, char *argv[])
{
    int i;
    char *readline = NULL;
    char *line;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&readline, &len, stdin)) != -1) 
    {
        line = readline;

        // remove comments
        {
          char *ptr = index(line, '#');
          if(ptr != NULL)
            *ptr = '\0';
        }

        // remove whitespace
        {
          // at begin
          for(i = 0; i < strlen(line); i++)
            if(!isspace(line[i]))
              break;
          line += i;

          // trailing whitespace
          for(i = strlen(line)-1; i > 0; i--)
          {
            if(isspace(line[i]))
              line[i] = '\0';
            else
              break;
          }
        }

        char *key,*value,*sep;
        sep = index(line, '=');

        if(sep == NULL)
          continue;

        *sep = '\0';
        key = line;
        value = sep+1;

        if(!check_isalpha(key))
          continue;

        if(value[0] == '"' && value[strlen(value)-1] == '"')
        {
          value[strlen(value)-1] = '\0';
          value++;
        }

        if(!strlen(key) || !strlen(value))
          continue;
        if(!check_isalphanum(value))
          continue;

        printf("%s=\"%s\"\n", key,value);
    }

    if (readline)
        free(readline);

    exit(EXIT_SUCCESS);
}

