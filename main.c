//Include library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>// This header file provides access to system call
#include <sys/wait.h> //This header file provides access to functions which are used for managing child processes and their exit status.
#include <signal.h>
#include <sys/fcntl.h>

//Define Constants
#define MAX_COMMAND_LENGTH 512
#define MAX_ARGS 11

int count_command = 0; //Counter the number of the command
int sum_arg = 0; //summarize the number of the arguments
int arg_count = 0;//Count the number of valid arguments
int flag_var = 0; // 0 - no var, 1-there is var
int stdout_copy;
int flag_dash_removed=0;


//Pre-Declaration Function

//General function
int find_char_index(char *str, char ch);//Function that search the index of a specific character
void execute_command(char *command, char *args[]);//Function to execute the command of system call
void save_variable(char var[], char result[]);//Function that save in the DS-linked list the variable
void promot();//A function that prints the promot line
void count_substrings(char *str, char **args);

//LinkedList function to process the variable and save them

// Define the node structure
struct Node {
    char *key;
    char *value;
    struct Node *next;
};

struct Node *head = NULL; //Create the head of the list as global variable

// Function to add a new node to the linked list
void addNode(struct Node **headRef, char *key, char *value) {
    // Allocate memory for the new node
    struct Node *newNode = (struct Node *) malloc(sizeof(struct Node));

    // Allocate memory for the key and value strings
    newNode->key = (char *) malloc(sizeof(char) * (strlen(key) + 1));
    newNode->value = (char *) malloc(sizeof(char) * (strlen(value) + 1));

    memset(newNode->key, 0, strlen(newNode->key)); //reset all the save array to null
    memset(newNode->value, 0, strlen(newNode->value)); //reset all the save array to null

    // Copy the key and value strings into the new node property
    strcpy(newNode->key, key);
    strcpy(newNode->value, value);

    // Set the new node's next pointer to the current head node
    newNode->next = *headRef;

    // Update the head pointer to the new node
    *headRef = newNode;
}

// Function to print the contents of the linked list
void printList(struct Node *head) {
    struct Node *current = head;

    while (current != NULL) {
        //printf("Key: %s, Value: %s\n", current->key, current->value);
        current = current->next;
    }
}

// Function to find a node in the linked list by key
struct Node *findNode(struct Node *head, char *key) {
    struct Node *current = head;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// Function to update the value of a node in the linked list
void updateNodeValue(struct Node *head, char *key, char *newValue) {
    struct Node *current = head;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            free(current->value);
            current->value = (char *) malloc(sizeof(char) * (strlen(newValue) + 1));
            strcpy(current->value, newValue);
            return;
        }
        current = current->next;
    }
}

// Function to free the memory allocated for the linked list
void freeList(struct Node *head) {
    struct Node *current = head;

    while (current != NULL) {
        struct Node *temp = current;
        current = current->next;
        free(temp->key);
        free(temp->value);
        free(temp);
    }
}

void sigtstp_handler(int sig) {
    printf("Stopping process...\n");
    raise(SIGSTOP);
}

/*
 * Function that search the index of a specific character
 * if the index is not exists return -1
 */
int find_char_index(char *str, char ch) {
    char *ptr = strchr(str, ch);   // Find first occurrence of ch in str
    if (ptr != NULL) {
        int index = ptr - str;     // Calculate index by subtracting pointer to start of str
        return index;
    } else {
        return -1;
    }
}

/*Function to execute the command of system call
 * that executes a given command and its arguments as a child process
 * using the fork() and execvp() system calls
 * The function takes two arguments: a char* pointing to the command to be
 * executed and an array of char*s representing its arguments.
 */
void execute_command(char *command, char *args[]) {
    pid_t pid = fork(); //creates a child process using and saves the resulting process ID in the pid variable.
    int status;
    if (pid == 0) {  //The current process is the child process
        // count_command++;
        //system call is used to replace the current process with the specified command and arguments
        execvp(command, args);
        // If execvp returns, there was an error
        perror("ERR\n");
        freeList(head);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        /*
         *  It waits for the child process to finish executing using the waitpid()
         *  system call, and saves the exit status of the child process in the status variable.
         */
        waitpid(pid, &status, 0);
        dup2(stdout_copy, STDOUT_FILENO);
        if (WIFEXITED(status) && WEXITSTATUS(status) ==
                                 0) { //checks whether a child process has terminated normally and has returned an exit status of 0.
            //printf("child process worked\n");
        } else {
            // printf("\before:  count command:%d,sum arg:%d,arg count:%d",count_command,sum_arg,arg_count);

            count_command--;//Don't count the command
            sum_arg -= arg_count;// don't count the arguments
            //printf("\n after: count command:%d,sum arg:%d,arg count:%d",count_command,sum_arg,arg_count);
        }
    } else {
        // Fork error
        perror("ERR\n");
        freeList(head);
        exit(1);
    }
}

/*
 * Function that save in the DS-linked list the variable
 * first he check if the value is exists if it is his he updates his value
 * else add the new variable
 */
void save_variable(char var[], char result[]) {
    // Find a node by key and print its value
    struct Node *node = findNode(head, var);
    if (node == NULL) { //if the node is not exists
        addNode(&head, var, result); // add the node
    } else {
        updateNodeValue(head, var, result); // update the node value
    }
    //printList(head);
}

/*
 * This is a C function called dolar_in_dash that takes a string as input and
 * replaces all instances of $<variable_name>
 * with the corresponding value of the variable stored in a linked list.
 */
void dolar_in_dash(char *str) {
    char *take_str;
    char var_i[510]; //the key of the variable
    memset(var_i, 0, strlen(var_i)); //reset all the save array to null

    char str_variable[510]; //the key of the variable
    memset(str_variable, 0, strlen(str_variable)); //reset all the save array to null
    int first_char_dollar =0;
    if (str[0] != '$') {
        int index = find_char_index(str, '$');
        strncat(str_variable, str, index);
        first_char_dollar=1;
    }
    take_str = strtok(str, "$");
    int min_char = 0;
    while (take_str != NULL) {
        if(first_char_dollar%2 !=0){
            first_char_dollar++;
            take_str = strtok(NULL, "$");
            continue;
        }
        int space = find_char_index(take_str, ' ');
        int find_quotes = find_char_index(take_str, '"');
        if ((space < find_quotes && space != -1)||find_quotes==-1 && space > -1) {
            min_char = space;
        } else {
            min_char = find_quotes;
        }
        if (space == -1 && find_quotes == -1) {
            min_char = strlen(take_str);
        }
        strncpy(var_i, take_str, min_char);
        struct Node *node = findNode(head, var_i); //find if we have node with this key
        if (node != NULL) { // if we have node copy to subtoken the value
            strcat(str_variable, node->value);
           if(min_char==space){
               strcat(str_variable, " ");
               if (min_char != strlen(take_str) - 1) {
                   strcat(str_variable, take_str + space + 1);
               }
           }
           else if(min_char==find_quotes){
               flag_dash_removed=1;
               strcat(str_variable, take_str + min_char + 1);

           }
        } else {
            strcat(str_variable, "");
        }
        take_str = strtok(NULL, "$");
    }
    strcpy(str, str_variable);
}

/*
 * method to remove all the dashes that are inside the word and not on the side
 */
void remove_dashes(char *str) {
    int i, j;
    int len = strlen(str);

    for (i = 0; i < len; i++) {
        if (str[i] == '"') {
            if(i==len-1){
                continue;
            }
            // Check if the dash is surrounded by non-space characters
            if ((i == 0 || str[i - 1] != ' ') && (i == len - 1 || str[i + 1] != ' ')) {
                // Shift the remaining characters in the string left by one
                flag_dash_removed=1;
                for (j = i; j < len - 1; j++) {
                    str[j] = str[j + 1];
                }
                str[len - 1] = '\0'; // Null-terminate the string
                len--; // Decrease the length of the string by 1
                i--; // Decrement i to account for the removed dash
            }
        }
    }
}

void remove_spaces(char *str) {
    int i, j;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ') {
            for (j = i; str[j] != '\0'; j++) {
                str[j] = str[j + 1];
            }
            i--; // To check the current position again in the next iteration
        }
    }
}

void redirect_to_file(char *command, char *filename) {
    // open the file for writing
    int file = open(filename, O_WRONLY | O_APPEND|O_CREAT | O_TRUNC, 0666);
    if (file < 0) {
        printf("Error opening file for writing.\n");
        return;
    }

    // redirect stdout to the file
    stdout_copy = dup(STDOUT_FILENO);
    if (dup2(file, STDOUT_FILENO) == -1) {
        printf("Error redirecting stdout to file.\n");
        close(file);
        return;
    }

//    // execute the command in the shell
//    system(command);

//    // restore stdout to the console
//    fflush(stdout);
//    dup2(stdout_copy, STDOUT_FILENO);

    // close the file
    close(file);
}

/*
 *This function split string by space
 * and every string slice checked if he contains $/cd/= signs
 */
void slice_by_space(char *str, char **args, int *count) {
    int flag_arrow = 0;//check if the arrow have space before and after him
    char *token;
    char str_copy[510];
    strcpy(str_copy, str);
    char file_name_space[510];
    int index_arrow = find_char_index(str, '>'); //get the index of the arrow '>' in the string
    if (str[index_arrow + 1] == ' ' || str[index_arrow - 1] == ' ') {
        flag_arrow = 1;
        strncpy(file_name_space, index_arrow + str + 1, strlen(str) - 1);
        remove_spaces(file_name_space);
    }

    token = strtok(str, " ");//split by space
    while (token != NULL) {
        flag_var = 0;
        int index_equals_sign = find_char_index(token, '='); //get the index of the = in the string
        int index_dolar = find_char_index(token, '$'); //get the index of the $ in the string
        int index_arrow = find_char_index(token, '>'); //get the index of the arrow '>' in the string
        if (index_equals_sign != -1) {
            if (index_equals_sign == 0) {
                return;
            }
            flag_var = 1;
            char var[510];//the key of the variable
            char result[510]; //the value of the variable
            memset(var, NULL, 510);//reset all the save array to null
            memset(result, NULL, 510);//reset all the save array to null
            // Copy the substring from src to dest
            strncpy(var, token, index_equals_sign);
            strncpy(result, token + index_equals_sign + 1, strlen(token) - 1);
            save_variable(var, result);
            printList(head);
            return; //return not to go the execute command
        } else if (index_dolar != -1) {
            char key_value[510]; //the key of the variable
            memset(key_value, 0, strlen(key_value)); //reset all the save array to null
            strncat(key_value, token + index_dolar + 1,
                    strlen(token) - index_dolar);//copy to var value only the key dashes
            struct Node *node = findNode(head, key_value); //find if we have node with this key
            if (node != NULL) { // if we have node copy to subtoken the value
                strcpy(token + index_dolar, node->value);
            } else {
                strcpy(token + index_dolar, "");
            }
        } else if (strcmp(token, "cd") == 0) { //if we have cd print error not support
            printf("cd not supported\n");
            arg_count = 0;
            return;
        } else if (strchr(token, '>') != NULL) {
            char var_command[510];//the key of the variable
            char file_name[510]; //the value of the variable
            memset(var_command, 0, strlen(var_command));//reset all the save array to null
            memset(file_name, 0, strlen(file_name));//reset all the save array to null
            // Copy the substring from src to dest
            if (flag_arrow == 0) {
                strncpy(var_command, token, index_arrow);
                strncpy(file_name, token + index_arrow + 1, strlen(token) - 1);
                remove_spaces(file_name);
                if (token[0] == '>') {
                    strcpy(var_command, args[0]);
                } else if (token[strlen(token) - 1] == '>') {
                    var_command[strlen(var_command) - 1] = '\0';
                }
                strcpy(token, var_command);
                redirect_to_file(var_command, file_name);
            } else {
                if (token[strlen(token) - 1] == '>' && token[0] != '>') {
                    token[strlen(token) - 1] = '\0';
                    args[*count] = token;
                    (*count)++;
                    strcpy(file_name, file_name_space);
                    redirect_to_file(token, file_name);
                    return;
                }
                strncpy(var_command, token, index_arrow);
                strcpy(file_name, file_name_space);
                strcpy(token, var_command);
                redirect_to_file(var_command, file_name);
                return;
            }
        }
        args[*count] = token;
        (*count)++;
        arg_count++;
        token = strtok(NULL, " ");
    }
}

void fix_dash_dollar(char* str){
    int dash_index=0; int dollar_index=0;int space_index=0;int flag_count_value=0;int count_dash =0;
    for (int i = 0; i < strlen(str); ++i) {
        if(str[i]==' '){
            space_index=i;
        }
        else if(str[i]=='$'){
            dollar_index=i;
            flag_count_value=1;
        }
        else if(str[i]=='"'){
            dash_index=i;
            count_dash++;
        }
        if(space_index< dollar_index &&space_index<dash_index){
            if(flag_count_value==1){
                flag_count_value=0;
                if(dollar_index>dash_index && count_dash%2==0){
                    remove_dashes(str);
                }
                else{
                    dolar_in_dash(str);
                }
            }

        }
    }
}

/*
 * get the input from the user
 * and split the string and dividing by a semicolon will give the number of commands
Then each command is split by spaces and each word is sent to the additional function for testing
 */
void count_substrings(char *str, char **args) {
    int count = 0;//count the number of arguments
    flag_dash_removed=0;
    /*
     * token - pointer that point to the beginning of the string until ;
     * saveptr1 - A pointer to the beginning of a string that was preceded by a semicolon
     * subtoken - A pointer to the start of a string split second this time by a space
     * saveptr2- Pointer to end of string by split second this time by space
     */
    char *token, *subtoken, *saveptr1, *saveptr2;
    token = strtok_r(str, ";", &saveptr1);//split by ;
    int dollar = find_char_index(token, '$');
    int find_quotes = find_char_index(token, '"');
    //remove_dashes(token);
    if (dollar == -1) {
        remove_dashes(token);
    }
//    else if(dollar<find_quotes){
//        dolar_in_dash(token);
//    }
    else fix_dash_dollar(token);
    while (token != NULL) {
        count = 0;
        arg_count = 0;
        memset(args, 0, MAX_ARGS); //reset all the save array to null
        int count_dash=0;
        if(flag_dash_removed==0){
            for(int i = 0; i < strlen(token); ++i) {
                if(token[i]=='"') count_dash++;
            }
        }

        if(count_dash%2!=0){
            char temp_copy_token[510];
            strcpy(temp_copy_token,token);
            strcat(temp_copy_token,";");
            token = strtok_r(NULL, ";", &saveptr1); //move to the next semicolon
            strcat(temp_copy_token,token);
            subtoken = strtok_r(temp_copy_token, "\"", &saveptr2);
        }
        else subtoken = strtok_r(token, "\"", &saveptr2); //split by space
        int in_dash = 0;
        if (subtoken[0] == '\"') {
            in_dash = 1;
        }
        while (subtoken != NULL) {
            if (in_dash % 2 == 0) {
                char *dup = strdup(subtoken);
                slice_by_space(dup, args, &count);
                in_dash++;
            } else {
                int index_dolar = find_char_index(subtoken, '$'); //get the index of the $ in the string
                if (index_dolar != -1) {
                    dolar_in_dash(subtoken);
                }
                args[count] = subtoken;
                (count)++;
                arg_count++;
                in_dash++;
            }
            subtoken = strtok_r(NULL, "\"", &saveptr2); //move to the next space
        }
        args[count] = NULL;
        if (count > MAX_ARGS - 1) { //check if there is more than arguments
            fprintf(stderr, "ERR\n");
            return;
        }
        count_command++; // count command
        sum_arg += count;
        if (flag_var != 1) {
            execute_command(args[0], args); //execute the command
        } else count_command--;
        token = strtok_r(NULL, ";", &saveptr1); //move to the next semicolon
    }
}

/*
 * A function that prints the promot line
 */
void promot() {
    char cwd[510];
    if (getcwd(cwd, sizeof(cwd)) != NULL) { // returns a non-null value. If getcwd() is successful,
        printf("#cmd:<%d>|#args:<%d>@ %s>", count_command, sum_arg, cwd);
        fflush(stdout);//forces the data in the output buffer to be written to the output device immediately, instead of waiting for the buffer to fill up or the program to end
    } else {
        perror("ERR\n");
        exit(1);
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH]; //The command array
    char *args[MAX_ARGS];//The argument array
    int count_out = 0;//The counter for count when to exit from the shell
    // Installation signal handler for SIGTSTP
    signal(SIGTSTP, sigtstp_handler);
    while (1) {

        fflush(stdin);

        // Print prompt
        promot();

        // Read command
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove trailing newline

        if (strlen(command) > 510) { //  if the arguments more than 510
            fprintf(stderr, "ERR\n"); //print error
        }
        if (strcmp(command, "") == 0) { //count how much enter is press
            count_out++;
            if (count_out == 3) { //if enter three press out
                freeList(head);
                exit(0);
            }
        } else {
            count_out = 0;
            count_substrings(command, args); //send to function ,that split the command to the args
        }
    }
    return 0;
}


// finish code Sof 1.1