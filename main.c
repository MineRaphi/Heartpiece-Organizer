#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Include the appropriate header file for the operating system
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

int createProject(char name[]);
char** readProjectList(int *projectCount);

int main() {
    
    // Create a new project
    createProject("project1");
    
    return 0;
}



// Function defintions

// a function that creates a new project directory
int createProject(char name[]) {
    FILE *projectListWrite;
    int projectCount = 0;
    char projectName[110] = "projects/";
    // append the name of the project to the path
    strcat(projectName, name);

    const char *dirName = projectName;
    
    // Create a directory with read, write, and execute permissions for the owner
    if (MKDIR(dirName) == -1) {
        perror("Error creating directory");
        return 1;
    }

    // Use readProjectList to get the current list of projects
    char **projectNames = readProjectList(&projectCount);
    if (projectNames == NULL) {
        return 1;
    }

    // Open the projectList in write mode
    projectListWrite = fopen("projects/projectList.hpo", "w");
    if (projectListWrite == NULL) {
        perror("Error opening project list file for writing");
        return 1;
    }

    // Append the name of the new project to the projectList
    projectCount++;
    fprintf(projectListWrite, "%d\n", projectCount);
    for (int i = 0; i < projectCount - 1; i++) {
        fprintf(projectListWrite, "%s\n", projectNames[i]);
        free(projectNames[i]); // Free the allocated memory for each project name
    }
    fprintf(projectListWrite, "%s\n", name);
    free(projectNames); // Free the allocated memory for the project names array

    // Close the projectList file
    fclose(projectListWrite);
    printf("Directory created successfully\n");
    
    return 0;
}

char** readProjectList(int *projectCount) {
    FILE *projectListRead;
    char **projectNames;

    // Open the projectList in read mode
    projectListRead = fopen("projects/projectList.hpo", "r");
    if (projectListRead == NULL) {
        perror("Error opening project list file");
        return NULL;
    }

    // Read the number of projects in the projectList
    fscanf(projectListRead, "%d", projectCount);

    // Allocate memory for the array of project names
    projectNames = (char **)malloc(*projectCount * sizeof(char *));
    for (int i = 0; i < *projectCount; i++) {
        projectNames[i] = (char *)malloc(100 * sizeof(char));
    }

    // Read the names of the projects in the projectList
    for (int i = 0; i < *projectCount; i++) {
        fscanf(projectListRead, "%s", projectNames[i]);
    }

    // Close the projectList file
    fclose(projectListRead);

    return projectNames;
}
