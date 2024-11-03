#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Include the appropriate header file for the operating system
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <conio.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define CLEAR_SCREEN "cls"
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#define CLEAR_SCREEN "clear"
#endif

int menu();
int checkProjectList();
char** readProjectList(int *projectCount);
int createProject(char name[]);

// Platform-specific input buffer handling
#ifdef _WIN32
void disable_input_buffering() {}
void restore_input_buffering() {}
#else
void disable_input_buffering(struct termios* old_tio);
void restore_input_buffering(struct termios* old_tio);
#endif

int getch();
void selectProject();
void openProject(char name[], int x, int y);
char** readSceneList(int *sceneCount, char projectName[]);
void remove_prefix(char *original_string, const char *prefix);
void gotoxy(int x, int y);
void readSceneStatus(char sceneName[], char projectName[], int sceneStatus[]);
void createNewScene(char sceneName[], char projectName[]);
void deleteDirectoryContents(const char *dirPath);
void deleteProject(char name[]);
void deleteScene(char sceneName[], char projectName[]);
void editScene(char sceneName[], char projectName[], int x, int y);

int main() {
    // DO NOT REMOVE OR MODIFY THE FOLLOWING IF STATEMENT
    if (checkProjectList() == 1) {
        printf("Project list created\n");
        printf("Start again to use the program\n");
        getch();
        return 1;
    }

    while (1) {
        if (menu() == 2) {
            return 0;
        }
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
            menu();
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
            menu();
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

#ifdef _WIN32
#else
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
#endif

// Function to detect key input, including arrow keys
int getch() {
    #ifdef _WIN32
    int ch = _getch();

    if (ch == 0 || ch == 224) {
        ch = _getch();
        switch(ch) {
            case 72: return 1;  // Up arrow
            case 80: return 2;  // Down arrow
            case 77: return 3;  // Right arrow
            case 75: return 4;  // Left arrow
        }
    }

    #else
    struct termios old_tio;
    disable_input_buffering(&old_tio);  // Disable canonical mode and echo

    int ch = getchar();  // Read one character

    if (ch == '\033') {  // Escape character for arrow keys
        if (getchar() == '[') {  // Skip the '[' character
            switch (getchar()) {
                case 'A': ch = 1; break;  // Up arrow
                case 'B': ch = 2; break;  // Down arrow
                case 'C': ch = 3; break;  // Right arrow
                case 'D': ch = 4; break;  // Left arrow
            }
        }
    }


    restore_input_buffering(&old_tio);  // Restore original settings
    return ch;
    #endif
}

// a function that allows the user to select a project which they want to open
void selectProject() {
    system(CLEAR_SCREEN);
    int length = 0;
    char** projectNames = readProjectList(&length);
    if (length == 0) {
        printf("No projects found\n");
        getch();
        menu();
        return;
    }
    for (int i=0; i<length; i++) {
        printf("%d. %s\n", i+1, projectNames[i]);
    }
    char input = getch();
    printf("%s", projectNames[input - '0' -1]);
    openProject(projectNames[input - '0' -1], 1, 1);
}

// a function that prints the project details and allows the user to edit the project
void openProject(char name[], int x, int y) {
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
#ifdef _WIN32
                case 0:
                    printf("\033[0;31m%c%c", 219, 219);
                    break;
                case 1:
                    printf("\033[0;33m%c%c", 219, 219);
                    break;
                case 2:
                    printf("\033[0;32m%c%c", 219, 219);
                    break;
#else
                case 0:
                    printf("\033[0;31m██");
                    break;
                case 1:
                    printf("\033[0;33m██");
                    break;
                case 2:
                    printf("\033[0;32m██");
                    break;
#endif
            }
            printf("\033[0m");
        }

        gotoxy(1, 4 + i);
        for (int j = 0; j < sideBarWidth + 54; j++) {
            printf("-");
        }
    }
    if (sceneCount == 0) {
        gotoxy(1, 3);
        printf("No scenes found, press w to create one\n");
    }
    else {
        gotoxy(sideBarWidth+x*5-4, 2+y*2-1);
        printf(":");
        gotoxy(sideBarWidth+x*5-4+3, 2+y*2-1);
        printf(":");
        gotoxy(1, sceneCount*2 + 3);
        printf("w: Create new scene | e: Exit | r: Delete scene\n");
        printf("Use the arrow keys to navigate and space to change the scene status\n\n");
    }

    // input for what to do next
    char input = getch();
    char newSceneName[100];
    char confirm[10];
    switch (input) {
        case 'w':
            printf("Name for new scene: ");
            scanf("%s", newSceneName);
            createNewScene(newSceneName, name);
            return;
        case 'q':
        case 'e':
            menu();
            return;
            break;
        case 'd':
        case 'r':
            printf("Enter the name of the scene to delete: ");
            scanf("%s", newSceneName);
            printf("Are you sure you want to delete the scene %s? (y/n): ", newSceneName);
            scanf("%s", confirm);
            if (confirm[0] == 'y' || confirm[0] == 'Y') {
                deleteScene(newSceneName, name);
                getch();
            }
            else {
                printf("Cancelling...\n");
            }
            return;
            break;

        case 1:
            y--;
            break;
        case 2:
            y++;
            break;
        case 3:
            x++;
            break;
        case 4:
            x--;
            break;
        case ' ':
            editScene(sceneNames[y-1], name, x-1, y-1);
            return;
            break;
    }
    if (x<1) {
        x = 1;
    }
    else if (x>11) {
        x = 11;
    }
    if (y<1) {
        y = 1;
    }
    else if (y>sceneCount) {
        y = sceneCount;
    }
    openProject(name, x, y);
    return;
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
    openProject(projectName, 1, 1);
}

// a function that deletes the contents of a directory
void deleteDirectoryContents(const char *dirPath) {
#ifdef _WIN32
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
    char path[MAX_PATH];

    // Prepare the path pattern
    snprintf(path, MAX_PATH, "%s\\*", dirPath);

    hFind = FindFirstFile(path, &fileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        // Skip the "." and ".." directories
        if (strcmp(fileData.cFileName, ".") == 0 || strcmp(fileData.cFileName, "..") == 0) {
            continue;
        }

        // Construct the full path
        snprintf(path, MAX_PATH, "%s\\%s", dirPath, fileData.cFileName);

        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // If it's a directory, call recursively
            deleteDirectoryContents(path);
            RemoveDirectory(path);
        } else {
            // If it's a file, delete it
            DeleteFile(path);
        }

    } while (FindNextFile(hFind, &fileData) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(dirPath);
    struct dirent *entry;
    struct stat statbuf;
    char filePath[512];

    if (!dir) return;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        if (stat(filePath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            deleteDirectoryContents(filePath);  // Recursive call
            rmdir(filePath);
        } else {
            unlink(filePath);  // Remove file
        }
    }

    closedir(dir);
#endif
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

// a function that deletes a scene
void deleteScene(char sceneName[], char projectName[]) {
    char path[310] = "./projects/";
    strcat(path, projectName);
    strcat(path, "/");
    strcat(path, sceneName);
    strcat(path, ".hpo");
    remove(path);
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
    fprintf(sceneListWrite, "%d\n", sceneCount-1);
    for (int i = 0; i < sceneCount; i++) {
        if (strcmp(scenes[i], sceneName) != 0) {
            fprintf(sceneListWrite, "%s\n", scenes[i]);
        }
    }
    fclose(sceneListWrite);
    printf("Scene successfully deleted\n");
    getch();
    openProject(projectName, 1, 1);
}

// a function that edits a scene
void editScene(char sceneName[], char projectName[], int x, int y) {
    char path[310] = "./projects/";
    strcat(path, projectName);
    strcat(path, "/");
    strcat(path, sceneName);
    strcat(path, ".hpo");
    FILE *sceneFileRead;
    sceneFileRead = fopen(path, "r");
    int sceneStatus[11];
    for (int i = 0; i < 11; i++) {
        fscanf(sceneFileRead, "%d", &sceneStatus[i]);
    }
    fclose(sceneFileRead);

    FILE *sceneFileWrite;
    sceneFileWrite = fopen(path, "w");
    for (int i = 0; i < 11; i++) {
        if (i == x) {
            fprintf(sceneFileWrite, "%d\n", (sceneStatus[i] + 1) % 3);
        }
        else {
            fprintf(sceneFileWrite, "%d\n", sceneStatus[i]);
        }
    }
    fclose(sceneFileWrite);
    openProject(projectName, x+1, y+1);
    return;
}
