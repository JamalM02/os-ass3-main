/*
  Operating Systems
  Semester 1 5784
  Kinneret College on the Sea of Galilee
  Michael J. May
  
  Assignment 3:
  Threads and Mutual Exclusion: Long Words

  Students:
  Jamal Majadle TZ-207513722
  Nagham Mousa TZ-208260018
  
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include "monitor.h"
#include "checkin.h"


/* The maximum length of each word in a file */
#define MAX_LINE_LEN 2000


/* word_length
* Calculates the number of alphabetical characters in a given word.
* Input: word - a pointer to the character array representing the word.
* Output: The number of alphabetical characters in the word.
*/
int word_length(char* word) {
    int word_length = 0;

    for (int i = 0; word[i] != '\0'; i++) {
        if (isalpha(word[i])) {
            word_length++;
        }
    }
    return word_length;
}



/* strcasecmp_alpha
* Performs a case-insensitive comparison of two strings in order finally to sort the given array in printLongestWords alphabiticlly.
* Input: a, b - pointers to the strings to compare, casted to void pointers to match qsort's requirements.
* Output: An integer less than, equal to, or greater than zero if the first string is found,
* respectively, to be less than, to match, or be greater than the second string.
*/
int strcasecmp_alpha(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcasecmp(str1, str2); 
}


/* printLongestWords
* Prints the longest words tracked by the monitor, sorted case-insensitively.
* Input: None.
* Output: Prints to stdout the longest words up to theMonitor->topCount.
*/
void printLongestWords() {
    int totalPrinted = 0;
    for (int i = MAX_LINE_LEN; i > 0 && totalPrinted < theMonitor->topCount; i--) {
        if (theMonitor->wordCount[i] > 0) {
            char *wordsCopy = strdup(theMonitor->words[i]);
            if (!wordsCopy) {
                fprintf(stderr, "Failed to allocate memory for words copy.\n");
                continue;
            }

            char **wordsArray = malloc(theMonitor->wordCount[i] * sizeof(char *));
            int wordsIndex = 0;
            for (char *token = strtok(wordsCopy, " "); token && wordsIndex < theMonitor->wordCount[i]; token = strtok(NULL, " ")) {
                wordsArray[wordsIndex++] = token;
            }

            qsort(wordsArray, wordsIndex, sizeof(char *), strcasecmp_alpha);
            int printed = 0;
            for (int j = 0; j < wordsIndex && totalPrinted < theMonitor->topCount; j++) {
                if (j == 0 || strcmp(wordsArray[j], wordsArray[j - 1]) != 0) {
                    printf("%d %s\n",totalPrinted+1, wordsArray[j]);
                    printed++;
                    totalPrinted++;
                }
            }
            printf("    %d words length %d\n", printed, i);
            free(wordsArray);
            free(wordsCopy);
        }
    }
}


/* processFile
* Processes the thread's files to count and categorize words by their alphabetical length in theMonitor->words.
* Input: param - a pointer to a character array representing the file's name.
* Output: NULL, indicating the function has completed.
*/
void* processFile(void *param) {
    time_t current_time;
    struct tm *local_time;
    time(&current_time);
    local_time = localtime(&current_time);
    char time_str[100];

    FILE *inFile = fopen((char*)param, "r");
    if (!inFile) {
        perror("Error opening file");
        return (void*)-1; 
    }

    char line[MAX_LINE_LEN];
    int chapter = 1;
    while (fgets(line, sizeof(line), inFile) != NULL) {
        if (strstr(line, "Chapter") == line) {
            strftime(time_str, sizeof(time_str), "%c", local_time);
            printf("%s %s started chapter %d.\n", time_str, (char*)param, chapter);
            checkin(theMonitor, (char*)param, chapter);
            chapter++;
        }

        char *saveptr;
        char* token = strtok_r(line, " \t\n", &saveptr);
        while (token != NULL) {
            int alphaLength = word_length(token); 
            if (alphaLength > 0) {
                pthread_mutex_lock(&theMonitor->lock);
                if (!theMonitor->words[alphaLength]) {
                    theMonitor->words[alphaLength] = strdup(token);
                    theMonitor->wordCount[alphaLength] = 1; 
                } else {
                    size_t newSize = strlen(theMonitor->words[alphaLength]) + strlen(token) + 2;
                    char* newStr = (char*)realloc(theMonitor->words[alphaLength], newSize);
                    if (newStr) {
                        theMonitor->words[alphaLength] = newStr;
                        strcat(theMonitor->words[alphaLength], " "); 
                        strcat(theMonitor->words[alphaLength], token);
                        theMonitor->wordCount[alphaLength]++;
                    } else {
                        fprintf(stderr, "Memory allocation failed for word concatenation.\n");
                    }
                }
                pthread_mutex_unlock(&theMonitor->lock);
            }
            token = strtok_r(NULL, " \t\n", &saveptr);
        }
    }
    fclose(inFile);
    printf("%s %s completed.\n", time_str, (char*)param);
    return NULL;
}
