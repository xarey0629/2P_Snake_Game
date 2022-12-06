#include <iostream>
#include <list>
#include <algorithm>
#include <ncurses.h>
#include <signal.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
using namespace std;

/************* 定義方向 ****************/
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define random(x) (rand() % x + 1) //用來產生隨機數

// Define Node which contains x and y coordinates.
struct SNode // 結點
{
    int x;
    int y;
    // SNode *next;
} fruit; // 順便宣告 fruit <- 蛇蛇要吃的。

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
} mysnake;                    // 順便宣告 -> 我的蛇蛇

// 預先宣告一些會使用的參數以及函示
int ch, eat, i;
list<SNode>::iterator turn_iter;
void show(int signumber);
void controller(void);
void draw_node(SNode node, char paint);
void end_game();
bool crash();

// Main Function
int main()
{
    // Time Settings
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 200000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 200000;
    signal(SIGALRM, &show);
    //	setitimer(ITIMER_REAL,&value,NULL);

    // Terminal Settings
    initscr();            //初始化虛擬屏幕
    raw();                //禁用行緩沖
    noecho();             //關閉鍵盤回顯
    keypad(stdscr, TRUE); //開啟功能鍵盤

    // Draw Border
    for (int i = 0; i < 40; i++)
    {
        mvaddch(0, i, '-');
        mvaddch(21, i, '-');
    }
    for (int i = 0; i < 21; i++)
    {
        mvaddch(i, 0, '|');
        mvaddch(i, 41, '|');
    }

    // 蛇蛇初始資料
    mysnake.front.x = 2;
    mysnake.front.y = 1;
    mysnake.back.x = 1;
    mysnake.back.y = 1;
    mysnake.len = 2;
    // 隨機播種
    fruit.x = random(20);
    fruit.y = random(20);
    // Draw init location.
    draw_node(mysnake.front, '*');
    draw_node(mysnake.back, '*');
    draw_node(fruit, '#');
    mysnake.front_direction = RIGHT;
    mysnake.back_direction = RIGHT;

    mvprintw(22, 0, "******  Game: FoodySnake  Len:%d  ******", mysnake.len);
    refresh();

    // Ready to start
    getch();                              //等待接收一個空字符，開始遊戲
    setitimer(ITIMER_REAL, &value, NULL); //開啟定時器

    // Continuing
    while (ch != KEY_F(2))
    {
        controller(); // 控制：上下左右
    }

    // GaveOver
    endwin(); // 結束遊戲

    return 0;
}

void controller(void)
{
    ch = getch();
    switch (ch)
    {
    case KEY_UP:
        if (mysnake.front_direction != DOWN && mysnake.front_direction != UP)
        {
            mysnake.front_direction = UP;
            mysnake.turn_direction.push_back(mysnake.front_direction);
            mysnake.turn.push_back(mysnake.front);
            usleep(100000);
        }
        break;
    case KEY_DOWN:
        if (mysnake.front_direction != UP && mysnake.front_direction != DOWN)
        {
            mysnake.front_direction = DOWN;
            mysnake.turn_direction.push_back(mysnake.front_direction);
            mysnake.turn.push_back(mysnake.front);
            usleep(100000);
        }
        break;
    case KEY_LEFT:
        if (mysnake.front_direction != RIGHT && mysnake.front_direction != LEFT)
        {
            mysnake.front_direction = LEFT;
            mysnake.turn_direction.push_back(mysnake.front_direction);
            mysnake.turn.push_back(mysnake.front);
            usleep(100000);
        }
        break;
    case KEY_RIGHT:
        if (mysnake.front_direction != LEFT && mysnake.front_direction != RIGHT)
        {
            mysnake.front_direction = RIGHT;
            mysnake.turn_direction.push_back(mysnake.front_direction);
            mysnake.turn.push_back(mysnake.front);
            usleep(100000);
        }
        break;
    }
}
void show(int signumber)
{
    if (signumber == SIGALRM)
    {
        eat = 0;
        draw_node(mysnake.back, ' ');
        // 吃到果實
        if (mysnake.front.x == fruit.x && mysnake.front.y == fruit.y)
            eat = 1;
        for (int i = 0; i <= eat; i++)
        {
            switch (mysnake.front_direction)
            {
            case UP:
                mysnake.front.y--;
                break;
            case DOWN:
                mysnake.front.y++;
                break;
            case LEFT:
                mysnake.front.x--;
                break;
            case RIGHT:
                mysnake.front.x++;
                break;
            }
            draw_node(mysnake.front, '*');
        }
        switch (mysnake.back_direction)
        {
        case UP:
            mysnake.back.y--;
            break;
        case DOWN:
            mysnake.back.y++;
            break;
        case LEFT:
            mysnake.back.x--;
            break;
        case RIGHT:
            mysnake.back.x++;
            break;
        }
        if (mysnake.turn_direction.size() && (mysnake.back.x == mysnake.turn.front().x && mysnake.back.y == mysnake.turn.front().y))
        {
            mysnake.back_direction = mysnake.turn_direction.front();
            mysnake.turn_direction.pop_front();
            mysnake.turn.pop_front();
        }

        if (crash())
            end_game();
        if (eat)
        {
            mysnake.len++;
            mvprintw(22, 0, "******  Game: FoodySnake  Len:%d  ******", mysnake.len);
            fruit.x = random(20);
            fruit.y = random(20);
            draw_node(fruit, '#');
        }
        refresh();
    }
}

void draw_node(SNode node, char paint)
{
    mvaddch(node.y, node.x, paint);
}

void end_game()
{
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, NULL);
    mvprintw(10, 18, "Game_over");
}
bool crash()
{
    int Max, Min;
    SNode tmp;
    if (mysnake.front.x > 40 || mysnake.front.x <= 0 || mysnake.front.y <= 0 || mysnake.front.y > 20)
        return true; //撞墻
    /*推斷是否撞到自己*/
    if (!mysnake.turn.empty())
    {
        i = mysnake.turn.size() - 1;
        turn_iter = mysnake.turn.end();

        while (i--)
        {
            tmp = *turn_iter;
            turn_iter--;
            if ((*turn_iter).x == tmp.x && (*turn_iter).x == mysnake.front.x)
            {
                Max = max((*turn_iter).y, tmp.y);
                Min = min((*turn_iter).y, tmp.y);
                if (mysnake.front.y >= Min && mysnake.front.y <= Max)
                    return true;
            }
            else if ((*turn_iter).y == tmp.y && (*turn_iter).y == mysnake.front.y)
            {
                Max = max((*turn_iter).x, tmp.x);
                Min = min((*turn_iter).x, tmp.x);
                if (mysnake.front.x >= Min && mysnake.front.x <= Max)
                    return true;
            }
        }
        turn_iter = mysnake.turn.begin();
        if ((*turn_iter).x == mysnake.back.x && (*turn_iter).x == mysnake.front.x)
        {
            Max = max((*turn_iter).y, mysnake.back.y);
            Min = min((*turn_iter).y, mysnake.back.y);
            if (mysnake.front.y >= Min && mysnake.front.y <= Max)
                return true;
        }
        else if ((*turn_iter).y == mysnake.back.y && (*turn_iter).y == mysnake.front.y)
        {
            Max = max((*turn_iter).x, mysnake.back.x);
            Min = min((*turn_iter).x, mysnake.back.x);
            if (mysnake.front.x >= Min && mysnake.front.x <= Max)
                return true;
        }
    }

    return false;
}