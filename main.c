//Include library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>// This header file provides access to system call
#include <sys/wait.h> //This header file provides access to functions which are used for managing child processes and their exit status.

//Define Constants
#define MAX_COMMAND_LENGTH 512
#define MAX_ARGS 11

int count_command = 0; //Counter the number of the command
int sum_arg = 0; //summarize the number of the arguments
int arg_count = 0;//Count the number of valid arguments

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
        printf("Key: %s, Value: %s\n", current->key, current->value);
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
        //system call is used to replace the current process with the specified command and arguments
        execvp(command, args);
        // If execvp returns, there was an error
        printf("Error: command not found\n");
        freeList(head);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        /*
         *  It waits for the child process to finish executing using the waitpid()
         *  system call, and saves the exit status of the child process in the status variable.
         */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { //checks whether a child process has terminated normally and has returned an exit status of 0.
            //printf("child process worked\n");
        } else {
            count_command--;//Don't count the command
            sum_arg -= arg_count;// don't count the arguments
        }
    } else {
        // Fork error
        printf("Error: fork failed\n");
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
        printf("Value of node with key %s is exists: %s\n", node->key, node->value);
    }
    //printList(head);
}

/*
 *This function split string by space
 * and every string slice checked if he contains $/cd/= signs
 */
void slice_by_space(char*str,char**args, int*count){
    char *token;
    token = strtok(str, " ");//split by space
    while (token != NULL){
        int index_equals_sign = find_char_index(token, '='); //get the index of the = in the string
        int index_dolar = find_char_index(token, '$'); //get the index of the $ in the string
        if( index_equals_sign != -1){
            if(index_equals_sign==0){
                return;
            }
            char var[510];//the key of the variable
            char result[510]; //the value of the variable
            memset(var, 0, strlen(var));//reset all the save array to null
            memset(result, 0, strlen(result));//reset all the save array to null
            // Copy the substring from src to dest
            strncpy(var, token , index_equals_sign);
            strncpy(result, token + index_equals_sign + 1, strlen(token) - 1);
            save_variable(var, result);
            printList(head);
            return; //return not to go the execute command
        }
        else if( index_dolar != -1){
            char key_value[510]; //the key of the variable
            memset(key_value, 0, strlen(key_value)); //reset all the save array to null
            strncat(key_value, token+index_dolar+1, strlen(token)-index_dolar);//copy to var value only the key dashes
            struct Node *node = findNode(head, key_value); //find if we have node with this key
            if (node != NULL) { // if we have node copy to subtoken the value
                strcpy(token+index_dolar,node->value);
            }
            else{
                strcpy(token+index_dolar,"");
            }
        }
        else if (strcmp(token, "cd") == 0) { //if we have cd print error not support
            printf("Error: cd not supported\n");
            arg_count = 0;
            return;
        }
        args[*count]=strdup(token);
        (*count)++;

        token= strtok(NULL, " ");
    }

}
/*
 * get the input from the user
 * and split the string and dividing by a semicolon will give the number of commands
Then each command is split by spaces and each word is sent to the additional function for testing
 */
void count_substrings(char *str, char **args) {
    int count = 0;//count the number of arguments
    /*
     * token - pointer that point to the beginning of the string until ;
     * saveptr1 - A pointer to the beginning of a string that was preceded by a semicolon
     * subtoken - A pointer to the start of a string split second this time by a space
     * saveptr2- Pointer to end of string by split second this time by space
     */
    char *token, *subtoken, *saveptr1, *saveptr2;

    token = strtok_r(str, ";", &saveptr1);//split by ;

    while (token != NULL) {
        count = 0;
        memset(args, 0, MAX_ARGS); //reset all the save array to null
        char *index = strchr(str, '"');
        if (index != NULL){
            int try = (int)(index -str);
            if(index-str != 0 && *(index-1) != ' '){ // here we check if there are '"' without space before them
                for (int i = try; i < strlen(str)-1; ++i) { // if that's the case we move all the chars from that index to index+1
                    str[i]=str[i+1];
                }
            }
        }
        subtoken = strtok_r(token, "\"", &saveptr2); //split by space
        int in_dash=0;
        if(subtoken[0]=='\"'){
            in_dash=1;
        }
        while (subtoken != NULL) {
            if(in_dash%2 ==0){
                char* dup = strdup(subtoken);
                slice_by_space(dup, args, &count);
                in_dash++;
            }
            else{
                args[count] =subtoken;
                (count)++;
                in_dash++;
            }
            subtoken = strtok_r(NULL, "\"", &saveptr2); //move to the next space
        }
        args[count] = NULL;
        if (count > MAX_ARGS-1) { //check if there is more than arguments
            fprintf(stderr, "Error: More than 10 arguments\n");
            return;
        }
        count_command++; // count command
        sum_arg+=count;
        execute_command(args[0], args); //execute the command
        token = strtok_r(NULL, ";", &saveptr1); //move to the next semicolon
    }
}

/*
 * A function that prints the promot line
 */
void promot() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) { // returns a non-null value. If getcwd() is successful,
        printf("#cmd:<%d>|#args:<%d>@ %s>", count_command, sum_arg, cwd);
        fflush(stdout);//forces the data in the output buffer to be written to the output device immediately, instead of waiting for the buffer to fill up or the program to end
    } else {
        perror("getcwd() error");
        exit(1);
    }

}

int main() {
    char command[MAX_COMMAND_LENGTH]; //The command array
    char *args[MAX_ARGS];//The argument array
    int count_out = 0;//The counter for count when to exit from the shell
    while (1) {

        fflush(stdin);

        // Print prompt
        promot();

        // Read command
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove trailing newline

        if (strlen(command) > 510) { //  if the arguments more than 510
            fprintf(stderr, "Error: More than 510 letters\n"); //print error
        }
        if (strcmp(command, "") == 0) { //count how much enter is press
            count_out++;
        } else {
            count_out = 0;
        }
        if (count_out == 3) { //if enter three press out
            freeList(head);
            exit(0);
        }
        count_substrings(command, args); //send to function ,that split the command to the args
    }
    return 0;
}
