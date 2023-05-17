#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>

char s[100];
char expr[100] = {};
char *command[100] = {};
int ex = 0;
int count = 0;
int i, status;
pid_t childID, endID;
time_t when;
int forground = 0;



void write_to_log_file(){
   FILE *fptr;
   fptr = fopen("/home/eman/Desktop/lab1/track.text","a");
   fprintf(fptr,"Child terminated\n");
   fclose(fptr);
}
void proc_exit()
{
      int wstat;
      pid_t pid;
      while (1)
      {
         pid = wait3(&wstat, WNOHANG, NULL);
         if (pid == 0 || pid == -1)
         {
            forground = 1;
            return;
         }
         write_to_log_file();
      }
}
void setup_environment()
{
   chdir("/home/eman");
}

void execute_shell_bultin()
{
   if ((strcmp(command[0], "cd") == 0))
   {
      int err = 0;
      if ((command[1] == NULL) || (strcmp(command[1], "~") == 0))
      {
         err = chdir("/home/eman");
      }
      else
      {
         err = chdir(command[1]);
      }
      if (err != 0)
      {
         printf("error in command\n");
      }
   }
   else if ((strcmp(command[0], "echo") == 0))
   {
      command[1] = strtok(command[1], "\"");
      command[count - 1] = strtok(command[count - 1], "\"");
      for (int j = 1; j < count; j++)
      {
         printf("%s ", command[j]);
      }
      printf("\n");
   }else if ((strcmp(command[0], "export") == 0)){
      char *token =strtok(command[1],"=");
      char *varName=token;
      char *varVal;
      token=strtok(NULL,"\0");
      if(strchr(token,'\"')==NULL){
         varVal=token;
      }else{
         token++;
         token=strtok(token,"\"");
         varVal=token;
      }
      
      setenv(varName,varVal,1);
   }
}

void execute_command()
{
   int err=0;
   childID = fork();

   if (childID == -1)
   { /* Start a child process.      */
      perror("fork error");
      exit(EXIT_FAILURE);
   }
   else if (childID == 0)
   { /* This is the child.          */
      err=execvp(command[0], command);
      if(err==-1) printf("error in command\n");
      exit(EXIT_SUCCESS);
   }
   else if ((childID > 0) && (forground == 1))
   {
         endID = waitpid(childID, &status,0);
         if(endID == childID){
            write_to_log_file();
         }
   }
}

void shell()
{
   do
   {
      printf("%s ", getcwd(s, 100));
      forground = 0;
      for (int j = 0; j < count; j++)
      {
         command[j] = NULL;
      }
      count = 0;
      int c=0;
      fgets(expr, 100, stdin);
      expr[strcspn(expr, "\n")] = '\0';      
      char *token = strtok(expr, " ");
      while (token != NULL)
      {

         command[count] = token;
         if(strcmp(command[0], "export") == 0){
            count++;
            token = strtok(NULL, "\0");
            if(token != NULL) command[count] = token;
            count++;
            break;//
         }
         count++;
         token = strtok(NULL, " ");
      }

      for(int z=0;z<count;z++){
         
         if(strchr(command[z],'$')!=NULL){
            char *tok=strtok(command[z],"$");
            char *varN=tok;
            tok=strtok(NULL," ");
            if(tok!=NULL)varN=tok;
            if(strchr(varN,'\"')!=NULL){
            varN=strtok(varN,"\"");
            }
            char *varV=getenv(varN);
            command[z]=varV;
         }
         if(strchr(command[z], ' ') != NULL && (strcmp(command[0], "export") != 0)){
            char* temp=malloc(sizeof(char) * 254);
            strcpy(temp,command[z]);
            char* temp1=strtok(temp, " ");
            while (temp1 != NULL)
            {
               command[z+c]=temp1;
               temp1 = strtok(NULL, " ");
               c++;
            }
         }
         
      }
      if(c>0)count=count+c-1;
      if (strchr(command[count - 1], '&') != NULL)
      {
         command[count - 1] = strtok(command[count - 1], "&");
         forground = 0;
      }
      else
      {
         forground = 1;
      }

      if ((strcmp(command[0], "cd") == 0) || (strcmp(command[0], "echo") == 0) || (strcmp(command[0], "export") == 0))
      {
         execute_shell_bultin();
      }
      else if ((strcmp(command[0], "exit") == 0))
      {
         ex = 1;
      }
      else
      {
         execute_command();
      }

   } while (ex != 1);
}

int main()
{
   signal(SIGCHLD, proc_exit);
   setup_environment();
   shell();
   return 0;
}