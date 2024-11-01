#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
// Include the appropriate header file for the operating system
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) RemoveDirectory(path)
#define CLEAR_SCREEN "cls"
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#define CLEAR_SCREEN "clear"
#endif

int menu();
int checkProjectList();
char** readProjectList(int *projectCount);
int createProject(char name[]);
void disable_input_buffering(struct termios* old_tio);
void restore_input_buffering(struct termios* old_tio);
int getch();
void selectProject();
void openProject(char name[]);
char** readSceneList(int *sceneCount, char projectName[]);
void remove_prefix(char *original_string, const char *prefix);
void gotoxy(int x, int y);
void readSceneStatus(char sceneName[], char projectName[], int sceneStatus[]);
void createNewScene(char sceneName[], char projectName[]);
void deleteProject(char name[]);

int main() {
    // DO NOT REMOVE OR MODIFY THE FOLLOWING IF STATEMENT
    if (checkProjectList() == 1) {
        printf("Project list created\n");
        printf("Start again to use the program\n");
        getch();
        return 1;
    }

    if (menu() == 2) {
        return 0;
    }

    return 0;
}

/*
     ______                _   _                    _       __ _       _   _                  
    |  ____|              | | (_)                  | |     / _(_)     | | (_)                 
    | |__ _   _ _ __   ___| |_ _  ___  _ __      __| | ___| |_ _ _ __ | |_ _  ___  _ __  ___  
    |  __| | | | '_ \ / __| __| |/ _ \| '_ \    / _` |/ _ \  _| | '_ \| __| |/ _ \| '_ \/ __| 
    | |  | |_| | | | | (__| |_| | (_) | | | |  | (_| |  __/ | | | | | | |_| | (_) | | | \__ \ 
    |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|   \__,_|\___|_| |_|_| |_|\__|_|\___/|_| |_|___/ 
*/

// a function that displays the menu
int menu() {
    system(CLEAR_SCREEN);
    char choice;
    printf("1. Create a new project\n");
    printf("2. Open an existing project\n");
    printf("3. Delete a project\n");
    printf("4. Exit\n");
    choice = getch();
    char name[500];
    switch (choice) {
        case '1':
            system(CLEAR_SCREEN);
            printf("Enter the name of the project: ");
            scanf("%s", name);
            createProject(name);
            break;
        case '2':
            selectProject();
            break;
        case '3':
            printf("Enter the name of the project to delete: ");
            scanf("%s", name);
            printf("Are you sure you want to delete the project %s? (y/n): ", name);
            char confirm[10]; 
            scanf("%s", confirm);
            if (confirm[0] == 'y' || confirm[0] == 'Y') {
                deleteProject(name);
            }
            else {
                printf("Cancelling...\n");
            }
            break;
        case '4':
            printf("Exiting...\n");
            return 2;
            break;
    }
    return 0;
}

// a function that checks if the projectList exists and creates it if it doesn't
int checkProjectList() {
    FILE *projectList;

    // Open the projectList in read mode
    projectList = fopen("./projects/projectList.hpo", "r");

    if (projectList == NULL) {
        fclose(projectList);
        projectList = fopen("./projects/projectList.hpo", "w");
        fprintf(projectList, "0\n");
        return 1;
    }
    else {
        fclose(projectList);
        return 0;
    }
}

// a function that reads the project list and returns an array of project names
char** readProjectList(int *projectCount) {
    FILE *projectListRead;
    char **projectNames;

    // Open the projectList in read mode
    projectListRead = fopen("./projects/projectList.hpo", "r");
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

// a function that creates a new project directory
int createProject(char name[]) {
    FILE *projectListWrite;
    int projectCount = 0;
    char projectName[110] = "./projects/";
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
    projectListWrite = fopen("./projects/projectList.hpo", "w");
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

    FILE *sceneListWrite;
    char scenePath[210] = "./projects/";
    strcat(scenePath, name);
    strcat(scenePath, "/sceneList.hpo");

    // Open the sceneList in write mode
    sceneListWrite = fopen(scenePath, "w");
    fprintf(sceneListWrite, "0\n");

    printf("Project successfully created\n");
    
    return 0;
}

// Function to disable canonical mode and echo for stdin
void disable_input_buffering(struct termios* old_tio) {
    struct termios new_tio;

    // Get current terminal attributes and save them in old_tio
    tcgetattr(STDIN_FILENO, old_tio);
    
    // Copy the current attributes to modify them
    new_tio = *old_tio;
    
    // Disable canonical mode (ICANON) and echoing (ECHO)
    new_tio.c_lflag &= ~(ICANON | ECHO);
    
    // Apply the modified attributes immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

// Function to restore the original terminal attributes
void restore_input_buffering(struct termios* old_tio) {
    // Restore the original terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, old_tio);
}

// Function to detect key input, including arrow keys
int getch() {
    struct termios old_tio;
    disable_input_buffering(&old_tio);  // Disable canonical mode and echo

    int ch = getchar();  // Read one character

    if (ch == '\033') {  // Escape character for arrow keys
        getchar();       // Skip the '[' character
        switch (getchar()) {
            case 'A': ch = 1; break;  // Up arrow
            case 'B': ch = 2; break;  // Down arrow
            case 'C': ch = 3; break;  // Right arrow
            case 'D': ch = 4; break;  // Left arrow
        }
    }

    restore_input_buffering(&old_tio);  // Restore original settings
    return ch;
}

// a function that allows the user to select a project which they want to open
void selectProject() {
    system(CLEAR_SCREEN);
    int length = 0;
    char** projectNames = readProjectList(&length);
    for (int i=0; i<length; i++) {
        printf("%d. %s\n", i+1, projectNames[i]);
    }
    char input = getch();
    printf("%s", projectNames[input - '0' -1]);
    openProject(projectNames[input - '0' -1]);
}

// a function that prints the project details and allows the user to edit the project
void openProject(char name[]) {
    char path[110] = "./projects/";
    strcat(path, name);

    int sceneCount = 0;
    
    char** sceneNames = readSceneList(&sceneCount, name);

    int sideBarWidth = 0;
    for (int i = 0; i < sceneCount; i++) {
        if (strlen(sceneNames[i]) > sideBarWidth) {
            sideBarWidth = strlen(sceneNames[i]);
        }
    }
    if (strlen(name) > sideBarWidth) {
        sideBarWidth = strlen(name);
    }
    sideBarWidth += 2;
    
    system(CLEAR_SCREEN);

    gotoxy(1, 1);
    printf("%s", name);
    gotoxy(sideBarWidth, 1);
    printf("| Sc | Sb | Vo | La | RA | TA | CA | Bg | Cp | Mu | Db ");
    gotoxy(1, 2);
    for (int i = 0; i < sideBarWidth + 54; i++) {
        printf("-");
    }
    for (int i = 0; i < sceneCount*2; i+=2) {
        gotoxy(1, 3 + i);
        printf("%s", sceneNames[i/2]);

        int sceneStatus[11];
        for (int j = 0; j<11; j++) {
            sceneStatus[j] = 2;
        }
        
        readSceneStatus(sceneNames[i/2], name, sceneStatus);

        for (int j = 0; j<11; j++) {
            gotoxy(sideBarWidth + j*5, 3 + i);
            printf("| ");
            switch (sceneStatus[j]) {
                case 0:
                    printf("\033[0;31m██");
                    break;
                case 1:
                    printf("\033[0;33m██");
                    break;
                case 2:
                    printf("\033[0;32m██");
                    break;
            }
            //printf("██");
            printf("\033[0m");
        }

        gotoxy(1, 4 + i);
        for (int j = 0; j < sideBarWidth + 54; j++) {
            printf("-");
        }
    }
    printf("\n");
    char input = getch();
    char newSceneName[100];
    switch (input) {
        case 'w':
            printf("Name for new scene: ");
            scanf("%s", newSceneName);
            createNewScene(newSceneName, name);
        case 'q':
        case 'e':
            menu();
            break;
        case 'r':
            // remove project
            // to be implemented
            break;
    }
}

// a function that removes a prefix from a string
void remove_prefix(char *original_string, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    if (strncmp(original_string, prefix, prefix_len) == 0) {
        // Move the pointer to skip the prefix
        original_string += prefix_len;
    }
}

// a function that reads the scene list and returns an array of scene names
char** readSceneList(int *sceneCount, char sceneName[]) {
    FILE *projectListRead;
    char **projectNames;

    char scenePath[210] = "./projects/";

    strcat(scenePath, sceneName);
    strcat(scenePath, "/sceneList.hpo");

    // Open the projectList in read mode
    projectListRead = fopen(scenePath, "r");
    if (projectListRead == NULL) {
        perror("Error opening project list file");
        return NULL;
    }

    // Read the number of scenes in the sceneList
    fscanf(projectListRead, "%d", sceneCount);

    // Allocate memory for the array of scene names
    projectNames = (char **)malloc(*sceneCount * sizeof(char *)* *sceneCount);
    for (int i = 0; i < *sceneCount; i++) {
        projectNames[i] = (char *)malloc(100 * sizeof(char));
    }

    // Read the names of the projects in the projectList
    for (int i = 0; i < *sceneCount; i++) {
        fscanf(projectListRead, "%s", projectNames[i]);
    }

    // Close the projectList file
    fclose(projectListRead);

    return projectNames;
}

// a function that moves the cursor to a specific position on the terminal
void gotoxy(int x, int y) {
    // ANSI escape code for moving the cursor
    printf("\033[%d;%dH", y, x);
}

// a function that reads the scene status
void readSceneStatus(char sceneName[], char projectName[], int sceneStatus[]) {
    FILE *sceneFile;
    char scenePath[310] = "./projects/";

    strcat(scenePath, projectName);
    strcat(scenePath, "/");
    strcat(scenePath, sceneName);
    strcat(scenePath, ".hpo");

    // Open the scene file in read mode
    sceneFile = fopen(scenePath, "r");
    if (sceneFile == NULL) {
        perror("Error opening scene file");
        return;
    }

    // Read the scene status
    for (int i = 0; i < 11; i++) {
        fscanf(sceneFile, "%d\n", &sceneStatus[i]);
    }

    // Close the scene file
    fclose(sceneFile);
}

// a function that creates a new scene
void createNewScene(char sceneName[], char projectName[]) {
    char path[310] = "./projects/";
    strcat(path, projectName);
    strcat(path, "/");
    strcat(path, sceneName);
    strcat(path, ".hpo");
    FILE *sceneFile;
    sceneFile = fopen(path, "w");
    for (int i = 0; i < 11; i++) {
        fprintf(sceneFile, "0\n");
    }
    fclose(sceneFile);
    FILE *sceneListRead;
    char sceneListPath[210] = "./projects/";
    strcat(sceneListPath, projectName);
    strcat(sceneListPath, "/sceneList.hpo");
    sceneListRead = fopen(sceneListPath, "r");
    int sceneCount = 0;
    fscanf(sceneListRead, "%d", &sceneCount);
    char scenes[sceneCount][200];
    for (int i = 0; i < sceneCount; i++) {
        fscanf(sceneListRead, "%s", scenes[i]);
    }
    fclose(sceneListRead);
    FILE *sceneListWrite;
    sceneListWrite = fopen(sceneListPath, "w");
    fprintf(sceneListWrite, "%d\n", sceneCount+1);
    for (int i = 0; i < sceneCount; i++) {
        fprintf(sceneListWrite, "%s\n", scenes[i]);
    }
    fprintf(sceneListWrite, "%s\n", sceneName);
    fclose(sceneListWrite);
    openProject(projectName);
}

// a function that deletes the contents of a directory
void deleteDirectoryContents(const char *dirPath) {
    struct dirent *entry;
    DIR *dir = opendir(dirPath);

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    char filePath[512];
    struct stat statbuf;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        if (stat(filePath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            deleteDirectoryContents(filePath);
        }

        if (remove(filePath) != 0) {
            perror("Error deleting file or directory");
        }
    }

    closedir(dir);
}

// a function that deletes a project
void deleteProject(char name[]) {
    char path[110] = "./projects/";
    strcat(path, name);
    deleteDirectoryContents(path);
    RMDIR(path);

    FILE *projectListRead;
    projectListRead = fopen("./projects/projectList.hpo", "r");
    int projectCount = 0;
    fscanf(projectListRead, "%d", &projectCount);
    char projects[projectCount][200];
    for (int i = 0; i < projectCount; i++) {
        fscanf(projectListRead, "%s", projects[i]);
    }
    fclose(projectListRead);

    FILE *projectListWrite;
    projectListWrite = fopen("./projects/projectList.hpo", "w");
    fprintf(projectListWrite, "%d\n", projectCount-1);
    for (int i = 0; i < projectCount; i++) {
        if (strcmp(projects[i], name) != 0) {
            fprintf(projectListWrite, "%s\n", projects[i]);
        }
    }
    fclose(projectListWrite);
    printf("Project successfully deleted\n");
    getch();
    getch();
    menu();
}