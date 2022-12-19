#include <iostream>
#include <list>
#include <algorithm>
#include <ncurses.h>
#include <signal.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
using namespace std;

/************* 定義方向 ****************/
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define W 119
#define S 115
#define A 97
#define D 100
#define R 114
#define Q 113
#define fruit_pair 5
#define bomb_pair 6
#define player1 7
#define player2 8
#define winner_pair 9
// Define Node which contains x and y coordinates.
struct SNode // 結點
{
    int x;
    int y;
    // SNode *next;
} fruit, bomb; // 順便宣告 fruit <- 蛇蛇要吃的。

// Define Snake
struct Snake
{
    SNode front;              // Head
    SNode back;               // Tail
    list<SNode> turn;         // 用list仿造一個隊列
    list<int> turn_direction; // 該list主要用來存轉彎信息，方便推斷。
    int front_direction;      // 頭的前進方向
    int back_direction;       // 尾的消失方向
    int len;                  // Length of Snake
} snake0, snake1;             // 順便宣告 -> 我的蛇蛇

// 預先宣告一些會使用的參數以及函示
int ch, eat, i, timer = 0;
int randx, randy;

bool gameStatus = true;
list<SNode>::iterator turn_iter;

void randomXY();              // 隨機產生XY座標在(row, col)裡頭
void initSnake(Snake &snake); // 初始化蛇蛇
void drawBorder();            // draw the game border

// 把蛇、水果畫上Terminal
void draw_node(Snake snake, char paint);
void draw_node(SNode node, char paint);
void draw_node(SNode node, char paint, int number);

void show(int signumber);

// 蛇蛇控制器
void controllerP0(Snake &snake);
void controllerP1(Snake &snake);

// end game logistics
void end_game(char *winner);
void restartGame();
bool crash(Snake &snake);

// 初始頁面、最終頁面
void welcomeMessage();
void gameoverMessage();

// Main Function
int main()
{
    curs_set(0);
    welcomeMessage(); // 顯示遊戲首頁
    clear();

    // Time Settings
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 200000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 200000;

    // Terminal Settings
    initscr();            // 初始化虛擬屏幕
    raw();                // 禁用行緩沖
    noecho();             // 關閉鍵盤回顯
    keypad(stdscr, TRUE); // 開啟功能鍵盤

    // 蛇蛇初始資料
    initSnake(snake0);
    initSnake(snake1);

    // 隨機播種
    randomXY();
    fruit.x = randx; // col / 2 - 41 + 10;
    fruit.y = randy; // row / 2
    randomXY();
    bomb.x = randx;
    bomb.y = randy;

    // Draw init location.
    start_color();
    init_pair(fruit_pair, COLOR_BLACK, COLOR_GREEN);
    init_pair(bomb_pair, COLOR_BLACK, COLOR_RED);
    init_pair(player1, COLOR_BLACK, COLOR_BLUE);
    init_pair(player2, COLOR_BLACK, COLOR_RED);
    init_pair(winner_pair, COLOR_BLACK, COLOR_YELLOW);
    clear();
    draw_node(snake0, 'O');
    draw_node(snake1, 'Q');
    draw_node(fruit, '*', 0);
    draw_node(bomb, 'x', 1);
    drawBorder(); // draw the game border

    int row, col;               // to store the number of rows and the number of colums of the screen
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    attron(COLOR_PAIR(player1));
    mvprintw(row / 2 - 11, col / 2 - 40, "******  Player 1  Len:%d  ******", snake0.len);
    attroff(COLOR_PAIR(player1));
    attron(COLOR_PAIR(player2));
    mvprintw(row / 2 - 11, col / 2 + 9, "******  Player 2  Len:%d  ******", snake1.len);
    attroff(COLOR_PAIR(player2));
    refresh();

    // Ready to start
    getch();                              // 等待接收一個空字符，開始遊戲
    signal(SIGALRM, &show);               // Start Alarm
    setitimer(ITIMER_REAL, &value, NULL); // 開啟定時器

    // Continuing
    while (gameStatus)
    {
        while (ch != KEY_F(2) && gameStatus) // Press F2 to exit the game.
        {
            ch = getch();
            controllerP0(snake0);
            controllerP1(snake1); // 控制：上下左右
            if (crash(snake0) || (snake0.front.x == bomb.x && snake0.front.y == bomb.y))
            {
                gameStatus = false;
                end_game("player 2");
            }
            if (crash(snake1) || (snake1.front.x == bomb.x && snake1.front.y == bomb.y))
            {
                gameStatus = false;
                end_game("player 1");
            }
        }
        end_game("Tie");
        // End Page
        clear();
        gameoverMessage();

        do
        {
            ch = getch();
        } while (ch != R && ch != Q);

        switch (ch)
        {
        case R:
            restartGame();
            break;

        default:
            break;
        }
    }
    // 遊戲結束
    endwin();

    return 0;
}

void randomXY()
{
    randx = 0;
    randy = 0;
    int row, col;
    getmaxyx(stdscr, row, col);
    randx = (col / 2 - 41) + rand() % 80 + 1;
    randy = (row / 2 - 10) + rand() % 20 + 1;
    return;
}

void initSnake(Snake &snake)
{
    int row, col;
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    randomXY();
    int positionX = randx;
    int positionY = randy;

    snake.len = 2;
    snake.front.x = randx + 1, snake.front.y = randy;
    snake.back.x = randx, snake.back.y = randy;
    snake0.front_direction = RIGHT, snake0.back_direction = RIGHT;
    snake1.front_direction = D, snake1.back_direction = D;
}
void controllerP0(Snake &snake)
{
    switch (ch)
    {
    case KEY_UP:
        if (snake.front_direction != DOWN && snake.front_direction != UP)
        {
            snake.front_direction = UP;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case KEY_DOWN:
        if (snake.front_direction != UP && snake.front_direction != DOWN)
        {
            snake.front_direction = DOWN;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case KEY_LEFT:
        if (snake.front_direction != RIGHT && snake.front_direction != LEFT)
        {
            snake.front_direction = LEFT;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case KEY_RIGHT:
        if (snake.front_direction != LEFT && snake.front_direction != RIGHT)
        {
            snake.front_direction = RIGHT;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    }
}
void controllerP1(Snake &snake)
{
    switch (ch)
    {
    case W:
        if (snake.front_direction != W && snake.front_direction != S)
        {
            snake.front_direction = W;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case S:
        if (snake.front_direction != W && snake.front_direction != S)
        {
            snake.front_direction = S;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case A:
        if (snake.front_direction != D && snake.front_direction != A)
        {
            snake.front_direction = A;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    case D:
        if (snake.front_direction != A && snake.front_direction != D)
        {
            snake.front_direction = D;
            snake.turn_direction.push_back(snake.front_direction);
            snake.turn.push_back(snake.front);
            usleep(500000);
        }
        break;
    }
}
void draw_node(Snake snake, char paint)
{
    mvaddch(snake.front.y, snake.front.x, paint);
    mvaddch(snake.back.y, snake.back.x, paint);
}
void draw_node(SNode node, char paint)
{
    mvaddch(node.y, node.x, paint);
}
void draw_node(SNode node, char paint, int number)
{
    // fruit
    if (number == 0)
    {
        attron(COLOR_PAIR(fruit_pair));
        mvaddch(node.y, node.x, paint);
        attroff(COLOR_PAIR(fruit_pair));
        refresh();
    }

    // bomb
    if (number == 1)
    {
        attron(COLOR_PAIR(bomb_pair));
        mvaddch(node.y, node.x, paint);
        attroff(COLOR_PAIR(bomb_pair));
        refresh();
    }
}

void show(int signumber)
{
    timer++;
    if (timer % 10 == 0) 
    {
        draw_node(bomb, ' ');
        randomXY();
        bomb.x = randx;
        bomb.y = randy;
        draw_node(bomb, 'x', 1);
    }
    curs_set(0);
    int row, col;
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    if (signumber == SIGALRM)
    {
        // snake0
        eat = 0;
        draw_node(snake0.back, ' '); // 先把尾巴的點刪掉
        // 吃到水果
        if (snake0.front.x == fruit.x && snake0.front.y == fruit.y)
        {
            eat = 1;
            snake0.len++;
            attron(COLOR_PAIR(player1));
            mvprintw(row / 2 - 11, col / 2 - 40, "******  Player 1  Len:%d  ******", snake0.len);
            attroff(COLOR_PAIR(player1));
            randomXY();
            fruit.x = randx;
            fruit.y = randy;

            draw_node(fruit, '*', 0);
            
        }
        // 在頭的行進方向畫上一個新的點（代表蛇向前進），若吃到水果 -> eat = 1，迴圈跑兩次（共畫上兩個點）
        for (int i = 0; i <= eat; i++)
        {
            switch (snake0.front_direction)
            {
            case UP:
                snake0.front.y--;
                break;
            case DOWN:
                snake0.front.y++;
                break;
            case LEFT:
                snake0.front.x--;
                break;
            case RIGHT:
                snake0.front.x++;
                break;
            }
            draw_node(snake0.front, 'O');
        }
        // 紀錄尾巴方向
        switch (snake0.back_direction)
        {
        case UP:
            snake0.back.y--;
            break;
        case DOWN:
            snake0.back.y++;
            break;
        case LEFT:
            snake0.back.x--;
            break;
        case RIGHT:
            snake0.back.x++;
            break;
        }
        if (snake0.turn_direction.size() && (snake0.back.x == snake0.turn.front().x && snake0.back.y == snake0.turn.front().y))
        {
            snake0.back_direction = snake0.turn_direction.front();
            snake0.turn_direction.pop_front();
            snake0.turn.pop_front();
        }
        if (crash(snake0) || (snake0.front.x == bomb.x && snake0.front.y == bomb.y))
        {
            gameStatus = false;
            end_game("player 2");
        }

        // snake1
        eat = 0;
        draw_node(snake1.back, ' ');
        if (snake1.front.x == fruit.x && snake1.front.y == fruit.y)
        {
            eat = 1;
            snake1.len++;
            attron(COLOR_PAIR(player2));
            mvprintw(row / 2 - 11, col / 2 + 9, "******  Player 2  Len:%d  ******", snake1.len);
            attroff(COLOR_PAIR(player2));
            randomXY();
            fruit.x = randx;
            fruit.y = randy;

            draw_node(fruit, '*', 0);
            
        }
        for (int i = 0; i <= eat; i++)
        {
            switch (snake1.front_direction)
            {
            case W:
                snake1.front.y--;
                break;
            case S:
                snake1.front.y++;
                break;
            case A:
                snake1.front.x--;
                break;
            case D:
                snake1.front.x++;
                break;
            }
            draw_node(snake1.front, 'Q');
        }
        switch (snake1.back_direction)
        {
        case W:
            snake1.back.y--;
            break;
        case S:
            snake1.back.y++;
            break;
        case A:
            snake1.back.x--;
            break;
        case D:
            snake1.back.x++;
            break;
        }
        if (snake1.turn_direction.size() && (snake1.back.x == snake1.turn.front().x && snake1.back.y == snake1.turn.front().y))
        {
            snake1.back_direction = snake1.turn_direction.front();
            snake1.turn_direction.pop_front();
            snake1.turn.pop_front();
        }
        if (crash(snake1) || (snake1.front.x == bomb.x && snake1.front.y == bomb.y))
        {
            gameStatus = false;
            end_game("player 1");
        }

        // refresh board
        refresh();
    }
}
void end_game(char *winner)
{
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, NULL);
    int row, col;               // to store the number of rows and the number of colums of the screen
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    attron(COLOR_PAIR(winner_pair));
    mvprintw(row / 2 - 12, col / 2 - 21, "*****  Game_over Winner is %s  *****", winner);
    attroff(COLOR_PAIR(winner_pair));
    refresh();
}

void restartGame()
{
    clear();
    // Time Settings
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 200000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 200000;
    // Terminal Settings
    initscr();            // 初始化虛擬屏幕
    raw();                // 禁用行緩沖
    noecho();             // 關閉鍵盤回顯
    keypad(stdscr, TRUE); // 開啟功能鍵盤
    drawBorder();         // draw the game border

    // 蛇蛇初始資料
    initSnake(snake0);
    initSnake(snake1);

    // 隨機播種
    randomXY();
    fruit.x = randx;
    fruit.y = randy;
    randomXY();
    bomb.x = randx;
    bomb.y = randy;

    // Draw init location.
    draw_node(snake0, 'O');
    draw_node(snake1, 'Q');
    draw_node(fruit, '*', 0);
    draw_node(bomb, 'x', 1);

    int row, col;               // to store the number of rows and the number of colums of the screen
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    attron(COLOR_PAIR(player1));
    mvprintw(row / 2 - 11, col / 2 - 40, "******  Player 1  Len:%d  ******", snake0.len);
    attroff(COLOR_PAIR(player1));
    attron(COLOR_PAIR(player2));
    mvprintw(row / 2 - 11, col / 2 + 9, "******  Player 2  Len:%d  ******", snake1.len);
    attroff(COLOR_PAIR(player2));
    refresh();

    // Ready to start
    getch();                              // 等待接收一個空字符，開始遊戲
    signal(SIGALRM, &show);               // Start Alarm
    setitimer(ITIMER_REAL, &value, NULL); // 開啟定時器
    gameStatus = true;
}

// 判定遊戲結束與否
bool crash(Snake &snake)
{
    int row, col;
    getmaxyx(stdscr, row, col); // get the number of rows and columns
    int Max, Min;
    SNode tmp;

    // 是否撞牆
    if (snake.front.x >= (col / 2 + 40) || snake.front.x <= (col / 2 - 41) || snake.front.y <= (row / 2 - 10) || snake.front.y >= (row / 2 + 10))
        return true;

    // 是否撞到自己
    if (!snake.turn.empty())
    {
        i = snake.turn.size() - 1;    // last index
        turn_iter = snake.turn.end(); //

        while (i--)
        {
            tmp = *turn_iter; // 尾巴前一個node
            turn_iter--;      // 尾巴前前一個node
            if ((*turn_iter).x == tmp.x && (*turn_iter).x == snake.front.x)
            {
                Max = max((*turn_iter).y, tmp.y);
                Min = min((*turn_iter).y, tmp.y);
                if (snake.front.y >= Min && snake.front.y <= Max)
                    return true;
            }
            else if ((*turn_iter).y == tmp.y && (*turn_iter).y == snake.front.y)
            {
                Max = max((*turn_iter).x, tmp.x);
                Min = min((*turn_iter).x, tmp.x);
                if (snake.front.x >= Min && snake.front.x <= Max)
                    return true;
            }
        }

        turn_iter = snake.turn.begin();
        if ((*turn_iter).x == snake.back.x && (*turn_iter).x == snake.front.x)
        {
            Max = max((*turn_iter).y, snake.back.y);
            Min = min((*turn_iter).y, snake.back.y);
            if (snake.front.y >= Min && snake.front.y <= Max)
                return true;
        }
        else if ((*turn_iter).y == snake.back.y && (*turn_iter).y == snake.front.y)
        {
            Max = max((*turn_iter).x, snake.back.x);
            Min = min((*turn_iter).x, snake.back.x);
            if (snake.front.x >= Min && snake.front.x <= Max)
                return true;
        }
    }

    return false;
}

void welcomeMessage() // foodySnake
{
    int row, col;
    WINDOW *win;
    initscr();
    raw(); // 禁用行緩沖
    noecho();
    char mesg[7][150] = {"  ______    ______    ______    _____    __    __    ____     _     _      ___      _   _     ______  ",
                         " |  ____|  | ____ |  | ____ |  |  __ \\   \\ \\  / /   / __ \\   |  \\  | |    / _ \\    | | / /   |  ____| ",
                         " | |____   | |  | |  | |  | |  | |  \\ \\   \\ \\/ /   / /  \\_\\  | \\ \\ | |   / / \\ \\   | |/ /    | |____  ",
                         " |  ____|  | |  | |  | |  | |  | |  | |    \\  /    \\ \\____   | |\\ \\| |  | |___| |  |   \\     |  ____| ",
                         " | |       | |  | |  | |  | |  | |  | |     | |     \\____ \\  | | \\ \\ |  | _____ |  | |\\ \\    | |      ",
                         " | |       | |__| |  | |__| |  | |__/ /     | |    _____| |  | |  \\  |  | |   | |  | | \\ \\   | |____  ",
                         " |_|       |______|  |______|  |_____/      |_|    \\______/  |_|   \\_|  |_|   |_|  |_|  \\_\\  |______| "};

    // 以下註解為方便更改字型設計所用
    //  "  ______    ______    ______    _____    __    __    ____     _     _      ___      _   _     ______  ",
    //  " |  ____|  | ____ |  | ____ |  |  __ \   \ \  / /   / __ \   |  \  | |    / _ \    | | / /   |  ____| ",
    //  " | |____   | |  | |  | |  | |  | |  \ \   \ \/ /   / /  \_\  | \ \ | |   / / \ \   | |/ /    | |____  ",
    //  " |  ____|  | |  | |  | |  | |  | |  | |    \  /    \ \____   | |\ \| |  | |___| |  |   \     |  ____| ",
    //  " | |       | |  | |  | |  | |  | |  | |     | |     \____ \  | | \ \ |  | _____ |  | |\ \    | |      ",
    //  " | |       | |__| |  | |__| |  | |__/ /     | |    _____| |  | |  \  |  | |   | |  | | \ \   | |____  ",
    //  " |_|       |______|  |______|  |_____/      |_|    \______/  |_|   \_|  |_|   |_|  |_|  \_\  |______| "};

    getmaxyx(stdscr, row, col);                                                    // get the number of rows and columns
    for (int i = -6; i < 1; i++)                                                   // 標題置於中央往上移三行
        mvprintw(row / 2 + i, (col - strlen(mesg[i + 6])) / 2, "%s", mesg[i + 6]); // print the message at the center of the screen
    char pressMesg[] = "Press R/r to see the game rule.              Press any other key to start.";
    mvprintw(row / 2 + 4, (col - strlen(pressMesg)) / 2, "%s", pressMesg); // pressMesg置於中央往下移4行

    refresh();
    ch = getch();
    switch (ch)
    {
    case R: /* 按 'r' 顯示規則頁 */
        win = newwin(10, 90, 0, col / 2 - 45);
        box(win, '|', '-');
        mvwaddstr(win, 1, 2, "This is the rule for the game:");
        mvwaddstr(win, 2, 2, "1. In order to win, you must not touch the edge or your opponent's tail.");
        mvwaddstr(win, 3, 2, "2. To make your snake stronger, you can eat the fruit. It looks like this->*");
        mvwaddstr(win, 4, 2, "3. However, if you touch the bomb, Boom! Game over!! The bomb looks like this->X");
        mvwaddstr(win, 5, 2, "4. You can play again or quit by following the instructions shown on the screen.");
        mvwaddstr(win, 6, 2, "5. Player 1 can use UP/DOWN/LEFT/RIGHT button to move");
        mvwaddstr(win, 7, 2, "6. Player 2 can use W/S/A/D button to move.");
        mvwaddstr(win, 8, 2, "7. Let's have fun!");

    case '\t':         /* 按 [TAB] 鍵 呼叫另一視窗   */
        touchwin(win); /* wrefresh() 前需 touchwin() */
        wrefresh(win);
        getch(); /* 按任意鍵關閉視窗 */
        touchwin(stdscr);
        break;

    default:
        break;
    }
}

void gameoverMessage()
{
    char mesg[7][150] = {"   _____       ___      _     _    ______    ______    _      _   ______    ______   ",
                         "  / ___ \\     / _ \\    | \\   / |  |  ____|  | ____ |  | |   | |  |  ____|  |  __  \\  ",
                         " / |   |_|   / / \\ \\   |  \\_/  |  | |____   | |  | |  | |   | |  | |____   | |__/ /  ",
                         " | |   __   | |___| |  | \\   / |  |  ____|  | |  | |  | |   | |  |  ____|  |  _  /   ",
                         " | |  |_ |  | _____ |  | |\\_/| |  | |       | |  | |  \\  \\_/  /  | |       | | \\ \\   ",
                         " \\ \\___/ /  | |   | |  | |   | |  | |____   | |__| |   \\     /   | |____   | |  \\ \\  ",
                         "  \\_____/   |_|   |_|  |_|   |_|  |______|  |______|    \\___/    |______|  |_|   \\_\\ "};

    // 以下註解為方便更改字型設計所用
    //  "   _____       ___      _     _    ______    ______    _      _   ______    ______   ",
    //  "  / ___ \     / _ \    | \   / |  |  ____|  | ____ |  | |   | |  |  ____|  |  __  \  ",
    //  " / |   |_|   / / \ \   |  \_/  |  | |____   | |  | |  | |   | |  | |____   | |__/ /  ",
    //  " | |   __   | |___| |  | \   / |  |  ____|  | |  | |  | |   | |  |  ____|  |  _  /   ",
    //  " | |  |_ |  | _____ |  | |\_/| |  | |       | |  | |  \  \_/  /  | |       | | \ \   ",
    //  " \ \___/ /  | |   | |  | |   | |  | |____   | |__| |   \     /   | |____   | |  \ \  ",
    //  "  \_____/   |_|   |_|  |_|   |_|  |______|  |______|    \___/    |______|  |_|   \_\ "};

    int row, col;                                                                  // to store the number of rows and the number of colums of the screen
    initscr();                                                                     // start the curses mode(initialize the ncurses data structures)
    getmaxyx(stdscr, row, col);                                                    // get the number of rows and columns
    for (int i = -6; i < 1; i++)                                                   // 標題置於中央往上移三行
        mvprintw(row / 2 + i, (col - strlen(mesg[i + 6])) / 2, "%s", mesg[i + 6]); // print the message at the center of the screen
    char pressMesg[] = "Press R/r to play again.               Press Q/q to quit.";
    mvprintw(row / 2 + 4, (col - strlen(pressMesg)) / 2, "%s", pressMesg); // pressMesg置於中央往下移4行
    refresh();                                                             // to update the physical terminal optimally
}

void drawBorder()
{
    int row, col;                  // to store the number of rows and the number of colums of the screen
    initscr();                     // start the curses mode(initialize the ncurses data structures)
    getmaxyx(stdscr, row, col);    // get the number of rows and columns
    for (int i = -40; i < 40; i++) // x長80 y寬21
    {
        mvaddch(row / 2 - 13, col / 2 + i, '-');
        mvaddch(row / 2 - 10, col / 2 + i, '-');
        mvaddch(row / 2 + 10, col / 2 + i, '-');
    }
    for (int i = -13; i < 11; i++)
    {
        mvaddch(row / 2 + i, col / 2 - 41, '|');
        mvaddch(row / 2 + i, col / 2 + 40, '|');
        mvaddch(row / 2 + i, col / 2 - 42, '|');
        mvaddch(row / 2 + i, col / 2 + 41, '|');
    }
    refresh(); // to update the physical terminal optimally
    // getch();                                                               // wait for user input a character
    // endwin();                                                              // clean up all allocated resources from ncurses
}
