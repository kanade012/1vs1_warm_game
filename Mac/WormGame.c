#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define FIELD_WIDTH 50
#define FIELD_HEIGHT 25
#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80
#define ITEM_MAX 2
#define LEFT_MARGIN 30
#define TOP_MARGIN 3
#define DELAYTIME 200000

typedef struct _WORM {
    int x;
    int y;
    char direction;
    struct _WORM* next;
    struct _WORM* before;
} WORM, *pWORM;

typedef struct _ITEM {
    int x;
    int y;
    struct _ITEM* next;
} ITEM, *pITEM;

// 커서를 특정 위치로 이동
void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + TOP_MARGIN, x + LEFT_MARGIN * 2);
}

// 키보드 입력 확인
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// 방향키 입력 처리
char get_arrow_key() {
    char c;
    if ((c = getchar()) == '\033') { // ESC
        if ((c = getchar()) == '[') { // '['
            c = getchar(); // 방향키 값 읽기
            switch (c) {
                case 'A': return UP;    // ↑
                case 'B': return DOWN;  // ↓
                case 'C': return RIGHT; // →
                case 'D': return LEFT;  // ←
            }
        }
    }
    return 0; // 방향키가 아님
}

// 화면 지우기
void clear_screen() {
    printf("\033[2J\033[1;1H");
}

// 게임 필드 출력
void PrintField() {
    for (int i = 0; i <= FIELD_WIDTH; i++) {
        gotoxy(i, 0);
        printf("-");
        gotoxy(i, FIELD_HEIGHT);
        printf("-");
    }
    for (int i = 0; i <= FIELD_HEIGHT; i++) {
        gotoxy(0, i);
        printf("|");
        gotoxy(FIELD_WIDTH, i);
        printf("|");
    }
}

// 웜 추가
void AddWorm(pWORM wormTailNode) {
    pWORM newNode = (pWORM)malloc(sizeof(WORM));
    pWORM temp = wormTailNode->next;

    newNode->before = wormTailNode;
    newNode->next = temp;
    wormTailNode->next = newNode;
    if (temp) temp->before = newNode;

    newNode->x = wormTailNode->x;
    newNode->y = wormTailNode->y;
    newNode->direction = wormTailNode->direction;
}

// 웜 이동
void MoveWorm(pWORM wormTailNode, pWORM wormHeadNode) {
    pWORM curr = wormHeadNode;
    int prevX = curr->x;
    int prevY = curr->y;

    if (curr->direction == LEFT) curr->x--;
    if (curr->direction == RIGHT) curr->x++;
    if (curr->direction == UP) curr->y--;
    if (curr->direction == DOWN) curr->y++;

    curr = curr->before;
    while (curr != NULL) {
        int tempX = curr->x;
        int tempY = curr->y;

        curr->x = prevX;
        curr->y = prevY;

        prevX = tempX;
        prevY = tempY;

        curr = curr->before;
    }
}

// 웜 출력
void PrintWorm(pWORM wormTailNode) {
    pWORM curr = wormTailNode;
    while (curr != NULL) {
        gotoxy(curr->x, curr->y);
        printf("O");
        curr = curr->next;
    }
}

// 웜 지우기
void ClearWorm(int x, int y) {
    gotoxy(x, y);
    printf(" ");
}

// 웜 메모리 해제
void FreeWormList(pWORM wormTailNode) {
    pWORM curr = wormTailNode;
    while (curr != NULL) {
        pWORM temp = curr;
        curr = curr->next;
        free(temp);
    }
}

// 점수 출력
void PrintScore(int score) {
    gotoxy(FIELD_WIDTH + 5, 2);
    printf("Score: %d", score);
}

// 아이템 생성
void CreateItem(pITEM itemNode) {
    pITEM newItem = (pITEM)malloc(sizeof(ITEM));
    newItem->next = itemNode->next;
    itemNode->next = newItem;

    newItem->x = 1 + rand() % (FIELD_WIDTH - 2);
    newItem->y = 1 + rand() % (FIELD_HEIGHT - 2);
}

// 아이템 출력
void PrintItem(pITEM itemNode) {
    pITEM curr = itemNode->next;
    while (curr != NULL) {
        gotoxy(curr->x, curr->y);
        printf("@");
        curr = curr->next;
    }
}

// 아이템 충돌 검사
int CheckItemHit(pWORM wormHeadPointer, pITEM itemNode) {
    pITEM curr = itemNode->next;
    while (curr != NULL) {
        if (wormHeadPointer->x == curr->x && wormHeadPointer->y == curr->y) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

// 메인 함수
int main() {
    srand((unsigned int)time(NULL));

    pWORM wormHeadNode = (pWORM)malloc(sizeof(WORM));
    pWORM wormTailNode = (pWORM)malloc(sizeof(WORM));
    wormHeadNode->next = NULL;
    wormTailNode->before = NULL;

    wormHeadNode->x = FIELD_WIDTH / 2;
    wormHeadNode->y = FIELD_HEIGHT / 2;
    wormHeadNode->direction = RIGHT;

    wormTailNode->next = wormHeadNode;
    wormHeadNode->before = wormTailNode;

    pITEM itemNode = (pITEM)malloc(sizeof(ITEM));
    itemNode->next = NULL;

    int score = 0;
    int itemCounter = 0;

    clear_screen();
    PrintField();

    while (1) {
        if (kbhit()) {
            char key = get_arrow_key();
            if (key == 'q') break;

            if (key == LEFT && wormHeadNode->direction != RIGHT)
                wormHeadNode->direction = LEFT;
            else if (key == RIGHT && wormHeadNode->direction != LEFT)
                wormHeadNode->direction = RIGHT;
            else if (key == UP && wormHeadNode->direction != DOWN)
                wormHeadNode->direction = UP;
            else if (key == DOWN && wormHeadNode->direction != UP)
                wormHeadNode->direction = DOWN;
        }

        ClearWorm(wormTailNode->x, wormTailNode->y);
        MoveWorm(wormTailNode, wormHeadNode);

        if (wormHeadNode->x <= 0 || wormHeadNode->x >= FIELD_WIDTH ||
            wormHeadNode->y <= 0 || wormHeadNode->y >= FIELD_HEIGHT) {
            clear_screen();
            gotoxy(FIELD_WIDTH / 2 - 5, FIELD_HEIGHT / 2);
            printf("Game Over! Score: %d", score);
            break;
        }

        if (itemCounter < ITEM_MAX) {
            CreateItem(itemNode);
            itemCounter++;
        }

        if (CheckItemHit(wormHeadNode, itemNode)) {
            AddWorm(wormTailNode);
            score += 100;
            itemCounter--;
        }

        PrintItem(itemNode);
        PrintWorm(wormTailNode);
        PrintScore(score);

        usleep(DELAYTIME);
    }

    FreeWormList(wormTailNode);
    free(itemNode);
    return 0;
}
//
// Created by 장영하 on 2024. 11. 28..
//
