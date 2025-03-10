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

// Platform-specific input buffer handling
#ifdef _WIN32
void disable_input_buffering() {}
void restore_input_buffering() {}
#else
void disable_input_buffering(struct termios* old_tio);
void restore_input_buffering(struct termios* old_tio);
#endif


int menu();
int checkProjectList();
int getch();
void gotoxy(int x, int y);
char** readProjectList(int *projectCount);
int createProject(char name[]);
void openProject(char name[], int x, int y);
void deleteDirectoryContents(const char *dirPath);
void deleteProject(char name[]);
void readSceneStatus(char sceneName[], char projectName[], int sceneStatus[][11]);
void editScene(char *sceneName, char *projectName, int x, int y, int yOffset);

int main() {
    // DO NOT REMOVE OR MODIFY THE FOLLOWING IF STATEMENT
    if (checkProjectList() == 1) {
        printf("Project list created\n");
        printf("Start again to use the program\n");
        printf("Press any key to exit\n");
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
    int projectCount;
    FILE *projectListRead;
    projectListRead = fopen("./projects/projectList.hpo", "r");
    fscanf(projectListRead, "%d", &projectCount);
    char **projectNames = readProjectList(&projectCount);
    fclose(projectListRead);
    switch(choice) {
        case '1':
            system(CLEAR_SCREEN);
            printf("Enter the name of the project: ");
            char name[100];
            scanf("%s", name);
            createProject(name);
            menu();
            break;
        case '2':
            system(CLEAR_SCREEN);        
            for(int i=0; i<projectCount; i++) {
                printf("%d. %s\n", i+1, projectNames[i]);
            }
            int input = getch() - '0';
        
            if (input > projectCount) {
                return 1;
            }
            openProject(projectNames[input-1], 1 ,1);
            return 2;
        case '3':
            for(int i=0; i<projectCount; i++) {
                printf("%d. %s\n", i+1, projectNames[i]);
            }
            int Input = getch() - '0';

            if (input > projectCount) {
                return 1;
            }
            deleteProject(projectNames[Input-1]);
            return 1;
        case '4':
            return 2;
    }
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

// a function that moves the cursor to a specific position on the terminal
void gotoxy(int x, int y) {
    // ANSI escape code for moving the cursor
    printf("\033[%d;%dH", y, x);
}

// a function that checks if the projectList exists and creates it if it doesn't
int checkProjectList() {
    FILE *projectList;
    DIR *dir = opendir("./projects");
    if (dir) {
        closedir(dir);
    }
    else {
        MKDIR("./projects");
    }
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

// a function that creates a new project
int createProject(char name[]) {
    char path[110] = "./projects/";
    strcat(path, name);
    MKDIR(path);

    // Reads the project list
    FILE* projectListRead;
    projectListRead = fopen("./projects/projectList.hpo", "r");
    int projectCount = 0;
    fscanf(projectListRead, "%d", &projectCount);
    char projects[projectCount][200];
    for (int i = 0; i < projectCount; i++) {
        fscanf(projectListRead, "%s", projects[i]);
    }
    fclose(projectListRead);

    // Writes the old information with the new project into the project list
    FILE* projectListWrite;
    projectListWrite = fopen("./projects/projectList.hpo", "w");
    fprintf(projectListWrite, "%d\n", projectCount+1);
    for (int i = 0; i < projectCount; i++) {
        fprintf(projectListWrite, "%s\n", projects[i]);
    }
    fprintf(projectListWrite, "%s\n", name);
    fclose(projectListWrite);
    
    // Generates the scene list file
    FILE* sceneListWrite;
    char scenePath[210] = "./projects/";
    strcat(scenePath, name);
    strcat(scenePath, "/sceneList.hpo");
    sceneListWrite = fopen(scenePath, "w");
    fprintf(sceneListWrite, "0\n");

    return 0;

}

// a function that opens a project
void openProject(char name[], int x, int y) {
    int sceneCount;
    FILE *sceneListRead;

    char path[110] = "./projects/";
    strcat(path, name);

    sceneListRead = fopen(strcat(path, "/sceneList.hpo"), "r");
    fscanf(sceneListRead, "%d", &sceneCount);

    char **sceneNames;
    sceneNames = (char **)malloc(sceneCount * sizeof(char *));
    for (int i = 0; i < sceneCount; i++) {
        sceneNames[i] = (char *)malloc(100 * sizeof(char));
    }

    for (int i = 0; i < sceneCount; i++) {
        fscanf(sceneListRead, "%s", sceneNames[i]);
    }
    fclose(sceneListRead);

    int maxNameLength = 0;
    for (int i = 0; i < sceneCount; i++) {
        if (strlen(sceneNames[i]) > maxNameLength) {
            maxNameLength = strlen(sceneNames[i]);
        }
    }
    if (strlen(name) > maxNameLength) {
        maxNameLength = strlen(name);
    }

    int sceneCuts[sceneCount];
    for (int i = 0; i < sceneCount; i++) {
        FILE* sceneFile;
        char scenePath[310] = "./projects/";
        strcat(scenePath, name);
        strcat(scenePath, "/");
        strcat(scenePath, sceneNames[i]);
        strcat(scenePath, ".hpo");
        sceneFile = fopen(scenePath, "r");
        fscanf(sceneFile, "%d", &sceneCuts[i]);
        fclose(sceneFile);
    }

    int cutCount = 0;
    for(int i=0; i<sceneCount; i++) {
        cutCount += sceneCuts[i];
    }

    maxNameLength+=2;

    // Displayes everything on the screen
    gotoxy(1, 1);
    printf("%s", name);
    gotoxy(maxNameLength, 1);
    printf("| C || Sc | Sb | Vo | La | RA | TA | CA | Bg | Cp | Mu | Db ");
    gotoxy(1, 2);
    for (int i = 0; i < maxNameLength + 59; i++) {
        printf("-");
    }
    if (sceneCount == 0) {
        gotoxy(1, 3);
        printf("No scenes found press w to create a new scene or e to go back");
        int input = getch();
        // to be implemented
    }
    else {
        int line=3;
        for(int i=0; i<sceneCount; i++) {
            gotoxy(0, line-1);
            for(int j=0; j<maxNameLength; j++) {
                printf("-");
            }
            gotoxy(0, line);
            printf("%s", sceneNames[i]);
            for(int j=0; j<sceneCuts[i]; j++) {
                gotoxy(maxNameLength, line);
                printf("| %d ||\n", j+1);
                line++;
                gotoxy(maxNameLength, line);
                printf("|-----------------------------------------------------------");
                line++;
            }
        }
        gotoxy(0, line-1);
        for(int i=0; i<maxNameLength+59; i++) {
            printf(" ");
        }
        line = 3;

        
        for(int i=0; i<sceneCount; i++) {
            int sceneStatus[sceneCuts[i]][11];
            readSceneStatus(sceneNames[i], name, sceneStatus);
            for(int j=0; j<sceneCuts[i]; j++) {
                for(int k=0; k<11; k++) {
                    gotoxy(maxNameLength + k*5 + 5, line);
                    switch (sceneStatus[j][k]) {
#ifdef _WIN32
                        case 0:
                            printf("| \033[0;31m%c%c ", 219, 219);
                            break;
                        case 1:
                            printf("| \033[0;33m%c%c ", 219, 219);
                            break;
                        case 2:
                            printf("| \033[0;32m%c%c ", 219, 219);
                            break;
#else
                        case 0:
                            printf("| \033[0;31m██ ");
                            break;
                        case 1:
                            printf("| \033[0;33m██ ");
                            break;
                        case 2:
                            printf("| \033[0;32m██ ");
                            break;
#endif
                    }
                    printf("\033[0m");
                }
                line+=2;
            }
        }
        gotoxy(1, line-1);
        for(int i=0; i<maxNameLength+59; i++) {
            printf("-");
        }
        gotoxy(maxNameLength+x*5+1, 2+y*2-1);
        printf(":");
        gotoxy(maxNameLength+x*5+1+3, 2+y*2-1);
        printf(":");
        gotoxy(1, cutCount*2 + 3);
        printf("w: Create new scene | e: Exit | r: Delete scene | t: new cut\n");
        printf("Use the arrow keys to navigate and space to edit a scene\n");
        char input = getch();
    char newSceneName[100];
    char confirm[10];
    int sceneIndex = 0;
    int k = y;
    switch (input) {
        case 'w':
            break;
        case 'q':
        case 'e':
            menu();
            return;
            break;
        case 'd':
        case 'r':
            
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
            for(int i=0; i<sceneCount; i++) {
                if (sceneCuts[i] >= y) {
                    sceneIndex = i;
                    break;
                }
                else {
                    y -= sceneCuts[i];
                }
            }
            editScene(sceneNames[sceneIndex], name, x, y, k);
            return;
            break;
        }
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
    else if (y>cutCount) {
        y = cutCount;
    }
    openProject(name, x, y);
    return;
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

// a function that reads the scene status
void readSceneStatus(char sceneName[], char projectName[], int sceneStatus[][11]) {
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

    int cuts;
    fscanf(sceneFile, "%d", &cuts);
    
    // Read the scene status
    for(int i=0; i<cuts; i++) {
        for(int j=0; j<11; j++) {
            fscanf(sceneFile, "%d", &sceneStatus[i][j]);
        }
        fscanf(sceneFile, "\n");
    }

    fclose(sceneFile);
}

// a function that edits a scene
void editScene(char *sceneName, char *projectName, int x, int yScene, int y) {
    char path[310] = "./projects/";
    strcat(path, projectName);
    strcat(path, "/");
    strcat(path, sceneName);
    strcat(path, ".hpo");
    FILE *sceneFileRead;
    sceneFileRead = fopen(path, "r");
    int cuts;
    fscanf(sceneFileRead, "%d", &cuts);
    int sceneStatus[cuts][11];
    for (int i = 0; i < cuts; i++) {
        for (int j = 0; j < 11; j++) {
            fscanf(sceneFileRead, "%d ", &sceneStatus[i][j]);
        }
        fscanf(sceneFileRead, "\n");
    }
    fclose(sceneFileRead);

    FILE *sceneFileWrite;
    sceneFileWrite = fopen(path, "w");
    fprintf(sceneFileWrite, "%d\n", cuts);
    for (int i=0; i<cuts; i++) {
        for (int j=0; j<11; j++) {
            if (i == yScene-1 && j == x-1) {
                fprintf(sceneFileWrite, "%d ", (sceneStatus[i][j] + 1) % 3);
            }
            else {
                fprintf(sceneFileWrite, "%d ", sceneStatus[i][j]);
            }
        }
        fprintf(sceneFileWrite, "\n");
    }
    fclose(sceneFileWrite);
    openProject(projectName, x, y);
    return;
}
