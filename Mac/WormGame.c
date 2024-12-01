#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define MAP_WIDTH 30
#define MAP_HEIGHT 20
#define ITEM_COUNT 2

typedef struct Node {
    int x, y;
    struct Node *prev, *next;
} Node;

typedef struct Worm {
    Node *head, *tail;
    int length;
    char headChar;
    char bodyChar;
} Worm;

typedef struct Item {
    int x, y;
} Item;

// Function declarations
void drawMenu(int currentSelection);
int selectMenu();
void showInstructions();
void drawModeMenu(int currentSelection);
int selectMode();
void showResult(int winner);
void initializeWorm(Worm *worm, int startX, int startY, char headChar, char bodyChar);
void moveWorm(Worm *worm, char input);
int checkCollision(Worm *worm, Worm *opponent, Item items[], int itemCount);
void addSegment(Worm *worm);
void spawnItems(Item items[], int itemCount, Worm *worm1, Worm *worm2);
void drawMap(Worm *worm1, Worm *worm2, Item items[], int itemCount);
void clearScreen();
char getInput();

int main() {
    srand(time(NULL));

    Worm worm1, worm2;
    Item items[ITEM_COUNT];
    char input1 = 'd', input2 = 'l';
    int gameMode = 0;

    while (1) {
        int menuSelection = selectMenu();
        if (menuSelection == 2) {
            showInstructions();
            continue; // 메뉴로 돌아가기
        }
        if (menuSelection == 3) return 0; // 게임 종료

        // 모드 선택
        gameMode = selectMode();

        // 초기화
        initializeWorm(&worm1, 5, 5, '&', 'o');
        initializeWorm(&worm2, MAP_WIDTH - 6, MAP_HEIGHT - 6, '@', 'o');
        spawnItems(items, ITEM_COUNT, &worm1, &worm2);

        int winner = 0;

        // 게임 루프
        while (1) {
            clearScreen();
            drawMap(&worm1, &worm2, items, ITEM_COUNT);

            char input = getInput();
            if (input == 'q') break;

            if (input != 0) {
                if (input == 'w' || input == 'a' || input == 's' || input == 'd') input1 = input;
                if (input == 'i' || input == 'j' || input == 'k' || input == 'l') input2 = input;
            }

            moveWorm(&worm1, input1);
            moveWorm(&worm2, input2);

            int collision1 = checkCollision(&worm1, &worm2, items, ITEM_COUNT);
            int collision2 = checkCollision(&worm2, &worm1, items, ITEM_COUNT);

            if (collision1 == -1 || collision2 == -1) {
                winner = (collision1 == -1) ? 2 : 1;
                break;
            }

            if (gameMode == 1 && (worm1.length >= 15 || worm2.length >= 15)) {
                winner = (worm1.length >= 15) ? 1 : 2;
                break;
            }

            usleep(200000);
        }

        // 결과 화면 표시
        if (winner > 0) {
            showResult(winner);
        }
    }

    return 0;
}

// === 메뉴 관련 함수 ===
void drawMenu(int currentSelection) {
    clearScreen();
    printf("=== 1vs1 Worm Game ===\n");
    printf("%s Start Game\n", currentSelection == 1 ? ">>" : "  ");
    printf("%s Game Instructions\n", currentSelection == 2 ? ">>" : "  ");
    printf("%s Exit Game\n", currentSelection == 3 ? ">>" : "  ");
}

int selectMenu() {
    int selection = 1;
    char input;

    while (1) {
        drawMenu(selection);
        input = getInput();
        if (input == 'w' && selection > 1) selection--;
        if (input == 's' && selection < 3) selection++;
        if (input == '\n') break;
        usleep(100000);
    }

    return selection;
}

void showInstructions() {
    clearScreen();
    printf("=== Game Instructions ===\n");
    printf("1. Player 1 uses WASD keys. Head: &, Body: o.\n");
    printf("2. Player 2 uses Arrow keys. Head: @, Body: o.\n");
    printf("3. Collect items (*) to grow your worm.\n");
    printf("4. Avoid colliding with the map boundaries or your opponent.\n");
    printf("\nPress ENTER to return to the main menu...");
    while (getchar() != '\n'); // ENTER 키 대기
}

void drawModeMenu(int currentSelection) {
    clearScreen();
    printf("Select Game Mode:\n");
    printf("%s Speedrun (First to 15 points)\n", currentSelection == 1 ? ">>" : "  ");
    printf("%s Annihilation (Survive to win)\n", currentSelection == 2 ? ">>" : "  ");
}

int selectMode() {
    int selection = 1;
    char input;

    while (1) {
        drawModeMenu(selection);
        input = getInput();
        if (input == 'w' && selection > 1) selection--;
        if (input == 's' && selection < 2) selection++;
        if (input == '\n') break;
        usleep(100000);
    }

    return selection;
}

void showResult(int winner) {
    clearScreen();
    printf("=== Game Over ===\n");
    printf("Player %d wins!\n", winner);
    printf("\nPress ENTER to return to the main menu...");
    while (getchar() != '\n'); // ENTER 키 대기
}

// === Worm Initialization ===
void initializeWorm(Worm *worm, int startX, int startY, char headChar, char bodyChar) {
    worm->head = (Node *)malloc(sizeof(Node));
    Node *body = (Node *)malloc(sizeof(Node));
    if (!worm->head || !body) {
        printf("Memory allocation failed for worm\n");
        exit(1);
    }

    worm->head->x = startX;
    worm->head->y = startY;
    worm->head->prev = NULL;
    worm->head->next = body;

    body->x = startX - 1;
    body->y = startY;
    body->prev = worm->head;
    body->next = NULL;

    worm->tail = body;
    worm->length = 2;
    worm->headChar = headChar;
    worm->bodyChar = bodyChar;
}

// === Item Initialization ===
void spawnItems(Item items[], int itemCount, Worm *worm1, Worm *worm2) {
    for (int i = 0; i < itemCount; i++) {
        int validPosition = 0;
        while (!validPosition) {
            items[i].x = rand() % (MAP_WIDTH - 2) + 1;
            items[i].y = rand() % (MAP_HEIGHT - 2) + 1;

            validPosition = 1;
            Node *current = worm1->head;
            while (current) {
                if (current->x == items[i].x && current->y == items[i].y) {
                    validPosition = 0;
                    break;
                }
                current = current->next;
            }
            current = worm2->head;
            while (current) {
                if (current->x == items[i].x && current->y == items[i].y) {
                    validPosition = 0;
                    break;
                }
                current = current->next;
            }
        }
    }
}

// === Collision Check ===
int checkCollision(Worm *worm, Worm *opponent, Item items[], int itemCount) {
    if (!worm->head) return -1;

    // 벽 충돌
    if (worm->head->x <= 0 || worm->head->x >= MAP_WIDTH - 1 ||
        worm->head->y <= 0 || worm->head->y >= MAP_HEIGHT - 1) {
        return -1;
    }

    // 상대 몸체 충돌
    Node *current = opponent->head;
    while (current) {
        if (worm->head->x == current->x && worm->head->y == current->y) {
            return -1;
        }
        current = current->next;
    }

    // 아이템 충돌
    for (int i = 0; i < itemCount; i++) {
        if (worm->head->x == items[i].x && worm->head->y == items[i].y) {
            addSegment(worm);
            items[i].x = rand() % (MAP_WIDTH - 2) + 1;
            items[i].y = rand() % (MAP_HEIGHT - 2) + 1;
            return 0;
        }
    }

    return 0;
}

// === Worm Movement ===
void moveWorm(Worm *worm, char input) {
    int dx = 0, dy = 0;
    switch (input) {
        case 'w': dy = -1; break;
        case 'a': dx = -1; break;
        case 's': dy = 1; break;
        case 'd': dx = 1; break;
        case 'i': dy = -1; break;
        case 'j': dx = -1; break;
        case 'k': dy = 1; break;
        case 'l': dx = 1; break;
    }

    Node *newHead = (Node *)malloc(sizeof(Node));
    newHead->x = worm->head->x + dx;
    newHead->y = worm->head->y + dy;
    newHead->prev = NULL;
    newHead->next = worm->head;
    worm->head->prev = newHead;
    worm->head = newHead;

    if (worm->length > 1) {
        Node *temp = worm->tail;
        worm->tail = worm->tail->prev;
        worm->tail->next = NULL;
        free(temp);
    }
}

void addSegment(Worm *worm) {
    Node *newTail = (Node *)malloc(sizeof(Node));
    newTail->x = worm->tail->x;
    newTail->y = worm->tail->y;
    newTail->next = NULL;
    newTail->prev = worm->tail;
    worm->tail->next = newTail;
    worm->tail = newTail;
    worm->length++;
}

// === Map Drawing ===
void drawMap(Worm *worm1, Worm *worm2, Item items[], int itemCount) {
    char map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = ' ';
        }
    }

    for (int i = 0; i < MAP_WIDTH; i++) {
        map[0][i] = map[MAP_HEIGHT - 1][i] = '#';
    }
    for (int i = 0; i < MAP_HEIGHT; i++) {
        map[i][0] = map[i][MAP_WIDTH - 1] = '#';
    }

    Node *current = worm1->head;
    map[current->y][current->x] = worm1->headChar;
    current = current->next;
    while (current) {
        map[current->y][current->x] = worm1->bodyChar;
        current = current->next;
    }

    current = worm2->head;
    map[current->y][current->x] = worm2->headChar;
    current = current->next;
    while (current) {
        map[current->y][current->x] = worm2->bodyChar;
        current = current->next;
    }

    for (int i = 0; i < itemCount; i++) {
        map[items[i].y][items[i].x] = '*';
    }

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}

void clearScreen() {
    printf("\033[H\033[J");
}

char getInput() {
    struct termios oldt, newt;
    char ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    if (ch == EOF) {
        ch = '\0';
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return ch;
}