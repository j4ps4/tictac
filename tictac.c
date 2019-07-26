#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <glib.h>
#include <stdlib.h>
#include <ctype.h>

#define max(a,b) ((a.value)>(b.value)) ? (a) : (b)
#define min(a,b) ((a.value)<(b.value)) ? (a) : (b)
#define maxP(a,b) (a)->value > (b)->value ? (a) : (b)
#define minP(a,b) (a)->value < (b)->value ? (a) : (b)
#define maxI(a,b) a > b ? a : b
#define GOOD INT_MAX - 1
#define BAD INT_MIN + 1
#define TIME_LIMIT 10.0

clock_t begin;
int cutoff;
int top_level;
int max_threads;

typedef enum
{EMPTY, CROSS, CIRCLE} square;

typedef struct
{
    square* squares;
    int size;
} board;

inline square board_get(const board *node, int x, int y)
{
    const int S = node->size;
    return node->squares[y*S + x];
}

inline void board_set(board *node, int x, int y, square marker)
{
    const int S = node->size;
    node->squares[y*S + x] = marker;
}

board *board_new(int size)
{
    board *new = (board*)g_slice_alloc(sizeof(board));
    square *data = (square*)g_slice_alloc(sizeof(square)*size*size);
    new->squares = data;
    new->size = size;
    return new;
}

void board_init(board *b)
{
    const int S = b->size;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
        {
            board_set(b, j, i, EMPTY);
        }
    }
}

void board_free(board *b)
{
    const int S = b->size;
    g_slice_free1(sizeof(square)*S*S, b->squares);
    g_slice_free1(sizeof(board), b);
}

void board_print(const board *node)
{
    const int S = node->size;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
        {
            square marker = board_get(node, j, i);
            switch(marker)
            {
                case EMPTY:
                    printf(". ");
                    break;
                case CROSS:
                    printf("x ");
                    break;
                case CIRCLE:
                    printf("o ");
                    break;
            }
        }
        printf("\n");
    }
}

void children(const board *node, GPtrArray **childs, bool maxPlayer)
{
    const int S = node->size;
    *childs = g_ptr_array_sized_new(S*(S-1));
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
        {
            square marker = board_get(node, j, i);
            if (marker == EMPTY)
            {
                board *cpy = board_new(S);
                memcpy(cpy->squares, node->squares, sizeof(square)*S*S);
                if (maxPlayer)
                {
                    board_set(cpy, j, i, CROSS);
                }
                else
                {
                    board_set(cpy, j, i, CIRCLE);
                }
                g_ptr_array_add(*childs, (gpointer)cpy);
            }
        }
    }
}

bool board_terminal(const board *node, int *value)
{
    const int S = node->size;
    int circles;
    int crosses;
    int crossesH = 0;
    int circlesH = 0;
    int crossesV = 0;
    int circlesV = 0;
    int crossesD = 0;
    int circlesD = 0;
    int oldCrossesH;
    int oldCirclesH;
    int oldCrossesV;
    int oldCirclesV;
    int empties;
    // Horizontal
    for (int i = 0; i < S; i++)
    {
        circles = crosses = 0;
        oldCrossesH = 0;
        oldCirclesH = 0;
        empties = 0;
        for (int j = 0; j < S; j++)
        {
            square this = board_get(node, j, i);
            if (this == CROSS)
            {
                crosses++;
                oldCrossesH++;
            }
            else if (this == CIRCLE)
            {
                circles++;
                oldCirclesH++;
            }
            else if (this == EMPTY)
                empties++;
        }
        if (crosses == S)
        {
            *value = GOOD;
            return true;
        }
        else if (circles == S)
        {
            *value = BAD;
            return true;
        }
        int tmp = oldCrossesH;
        oldCrossesH -= oldCirclesH;
        oldCirclesH -= tmp;
        if (oldCrossesH < 0)
            oldCrossesH = 0;
        if (oldCirclesH < 0)
            oldCirclesH = 0;
        /*if (oldCrossesH > 0 && oldCirclesH > 0)*/
        /*{*/
            /*oldCrossesH = oldCirclesH = 0;*/
        /*}*/
        /*else if (oldCrossesH > 0 && oldCrossesH + empties == S)*/
        /*{*/
            /*oldCrossesH = S;*/
        /*}*/
        /*else if (oldCirclesH > 0 && oldCirclesH + empties == S)*/
        /*{*/
            /*oldCirclesH = S;*/
        /*}*/
        crossesH = maxI(crossesH, oldCrossesH);
        circlesH = maxI(circlesH, oldCirclesH);
    }
    // Vertical
    for (int j = 0; j < S; j++)
    {
        circles = crosses = 0;
        oldCrossesV = 0;
        oldCirclesV = 0;
        empties = 0;
        for (int i = 0; i < S; i++)
        {
            square this = board_get(node, j, i);
            if (this == CROSS)
            {
                oldCrossesV++;
                crosses++;
            }
            else if (this == CIRCLE)
            {
                circles++;
                oldCirclesV++;
            }
            else if (this == EMPTY)
                empties++;
        }
        if (crosses == S)
        {
            *value = GOOD;
            return true;
        }
        else if (circles == S)
        {
            *value = BAD;
            return true;
        }
        int tmp = oldCrossesV;
        oldCrossesV -= oldCirclesV;
        oldCirclesV -= tmp;
        if (oldCrossesV < 0)
            oldCrossesV = 0;
        if (oldCirclesV < 0)
            oldCirclesV = 0;
        /*if (oldCrossesV > 0 && oldCirclesV > 0)*/
        /*{*/
            /*oldCrossesV = oldCirclesV = 0;*/
        /*}*/
        /*else if (oldCrossesV > 0 && oldCrossesV + empties == S)*/
        /*{*/
            /*oldCrossesV = S;*/
        /*}*/
        /*else if (oldCirclesV > 0 && oldCirclesV + empties == S)*/
        /*{*/
            /*oldCirclesV = S;*/
        /*}*/
        crossesV = maxI(crossesV, oldCrossesV);
        circlesV = maxI(circlesV, oldCirclesV);
    }
    // Diagonal
    circles = crosses = 0;
    for (int j = 0; j < S; j++)
    {
        if (board_get(node, j, j) == CROSS)
        {
            crosses++;
            crossesD++;
        }
        else if (board_get(node, j, j) == CIRCLE)
        {
            circles++;
            circlesD++;
        }
    }
    /*if (crossesD > 0 && circlesD > 0)*/
    /*{*/
        /*crossesD = circlesD = 0;*/
    /*}*/
    if (crosses == S)
    {
        *value = GOOD;
        return true;
    }
    else if (circles == S)
    {
        *value = BAD;
        return true;
    }
    int tmp = crossesD;
    crossesD -= circlesD;
    circlesD -= tmp;
    if (crossesD < 0)
        crossesD = 0;
    if (circlesD < 0)
        circlesD = 0;
    int oldCrossesD = crossesD;
    int oldCirclesD = circlesD;
    crossesD = circlesD = 0;
    circles = crosses = 0;
    for (int j = S-1; j >= 0; j--)
    {
        if (board_get(node, j, (S-1)-j) == CROSS)
        {
            crosses++;
            crossesD++;
        }
        else if (board_get(node, j, (S-1)-j) == CIRCLE)
        {
            circles++;
            circlesD++;
        }
    }
    tmp = crossesD;
    crossesD -= circlesD;
    circlesD -= tmp;
    if (crossesD < 0)
        crossesD = 0;
    if (circlesD < 0)
        circlesD = 0;
    crossesD = maxI(crossesD, oldCrossesD);
    circlesD = maxI(circlesD, oldCirclesD);
    /*if (crossesD > 0 && circlesD > 0)*/
    /*{*/
        /*crossesD = circlesD = 0;*/
    /*}*/
    if (crosses == S)
    {
        *value = GOOD;
        return true;
    }
    else if (circles == S)
    {
        *value = BAD;
        return true;
    }
    int filled = 0;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
        {
            if (board_get(node, j, i) != EMPTY)
                filled++;
        }
    }
    if (filled == S*S)
    {
        *value = 0;
        return true;
    }
    int maxRet = maxI(crossesH, crossesV);
    maxRet = maxI(maxRet, crossesD);
    int minRet = maxI(circlesH, circlesV);
    minRet = maxI(minRet, circlesD);
    /*if (maxRet > minRet)*/
        /**value = maxRet;*/
    /*else*/
        /**value = -minRet;*/
    *value = maxRet - minRet;
    return false;
}

/*typedef struct _intlist*/
/*{*/
    /*int val;*/
    /*struct _intlist *next;*/
    /*struct _intlist *prev;*/
/*} intlist;*/

typedef struct
{
    int value;
    bool end;
    int move_x;
    int move_y;
} payload;

typedef struct
{
    int x;
    int y;
} coord;

coord board_cmp(const board *board1, const board *board2)
{
    const int S = board1->size;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
        {
            if (board_get(board1, j, i) != board_get(board2, j, i))
            {
                coord retval = {j, i};
                return retval;
            }
        }
    }
    coord retval = {INT_MAX, INT_MAX};
    return retval;
}

typedef struct {
    const board *node;
    int depth;
    int alpha;
    int beta;
    bool maxPlayer;
} ab_args;

payload alphabeta(const board *node, int depth, int alpha, int beta, bool maxPlayer);

void *alphabeta_wrapper(gpointer local_args)
{
    ab_args *args = (ab_args *) local_args;
    payload retval = alphabeta(args->node, args->depth, args->alpha, args->beta, args->maxPlayer);
    payload *cpy = malloc(sizeof(payload));
    memcpy(cpy, &retval, sizeof(payload));
    return cpy;
}

payload alphabeta(const board *node, int depth, int alpha, int beta, bool maxPlayer)
{
    /*static int evaluated = 0;*/
    /*evaluated++;*/
    int value;
    int chosen;
    int a = alpha;
    int b = beta;
    if (board_terminal(node, &value) || depth == 0)
    {
        /*board_print(node);*/
        payload retval = {.value = value, .end = true};
        /*printf("evaluated: %d\n", evaluated);*/
        return retval;
    }
    if (maxPlayer)
    {
        payload bestValue = {.value = INT_MIN};
        GPtrArray *childs;
        children(node, &childs, true);
        size_t size = childs->len;
        if (depth != top_level || size < max_threads)
        {
            for (int i = 0; i < size; i++)
            {
                board *child = g_ptr_array_index(childs, i);
                payload retval = alphabeta(child, depth-1, a, b, false);
                int oldBest = bestValue.value;
                bestValue = max(bestValue, retval);
                a = a > bestValue.value ? a : bestValue.value;
                if (oldBest != bestValue.value)
                {
                    chosen = i;
                }
                if (b <= a)
                    break;
                clock_t now = clock();
                if ((now - begin)/CLOCKS_PER_SEC > TIME_LIMIT && depth <= cutoff)
                {
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < size; i += max_threads)
            {
                board **subchilds = malloc(sizeof(board*)*max_threads);
                for (int j = 0; j < max_threads; j++)
                {
                    subchilds[j] = i+j < size ? g_ptr_array_index(childs, i+j) : NULL;
                }
                GThread **threads = calloc(max_threads, sizeof(GThread*)); // initializes to NULL
                int oldBest;
                ab_args *args = malloc(sizeof(ab_args)*max_threads);
                for (int j = 0; j < max_threads; j++)
                {
                    if (subchilds[j] != NULL)
                    {
                        args[j].node = subchilds[j], args[j].depth = depth-1, args[j].alpha = a, args[j].beta = b, args[j].maxPlayer = false;
                        threads[j] = g_thread_new(NULL, (GThreadFunc)alphabeta_wrapper, (gpointer)&args[j]);
                    }
                }
                for (int j = 0; j < max_threads; j++)
                {
                    if (threads[j] != NULL)
                    {
                        payload *ret = (payload*)g_thread_join(threads[j]);
                        oldBest = bestValue.value;
                        bestValue = *(maxP(&bestValue, ret));
                        if (oldBest != bestValue.value)
                        {
                            chosen = i+j;
                        }
                        free(ret);
                    }
                }
                free(subchilds);
                free(threads);
                free(args);
                a = a > bestValue.value ? a : bestValue.value;
                if (b <= a)
                    break;
            }
        }
        board *elem = g_ptr_array_index(childs, chosen);
        coord cmp = board_cmp(node, elem);
        bestValue.move_x = cmp.x;
        bestValue.move_y = cmp.y;
        /*board_print(elem);*/
        for (int i = 0; i < size; i++)
        {
            board *elem = g_ptr_array_index(childs, i);
            board_free(elem);
        }
        g_ptr_array_free(childs, TRUE);
        /*printf("evaluated: %d\n", evaluated);*/
        return bestValue;
    }
    else
    {
        payload bestValue = {.value = INT_MAX};
        GPtrArray *childs;
        children(node, &childs, false);
        size_t size = childs->len;
        if (depth != top_level || size < max_threads)
        {
            for (int i = 0; i < size; i++)
            {
                board *child = g_ptr_array_index(childs, i);
                payload retval = alphabeta(child, depth-1, a, b, true);
                int oldBest = bestValue.value;
                bestValue = min(bestValue, retval);
                b = b < bestValue.value ? b : bestValue.value;
                if (oldBest != bestValue.value)
                {
                    chosen = i;
                }
                if (b <= a)
                    break;
                clock_t now = clock();
                if ((now - begin)/CLOCKS_PER_SEC > TIME_LIMIT && depth <= cutoff)
                {
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < size; i += max_threads)
            {
                board **subchilds = malloc(sizeof(board*)*max_threads);
                for (int j = 0; j < max_threads; j++)
                {
                    subchilds[j] = i+j < size ? g_ptr_array_index(childs, i+j) : NULL;
                }
                GThread **threads = calloc(max_threads, sizeof(GThread*)); // initializes to NULL
                int oldBest;
                ab_args *args = malloc(sizeof(ab_args)*max_threads);
                for (int j = 0; j < max_threads; j++)
                {
                    if (subchilds[j] != NULL)
                    {
                        args[j].node = subchilds[j], args[j].depth = depth-1, args[j].alpha = a, args[j].beta = b, args[j].maxPlayer = true;
                        threads[j] = g_thread_new(NULL, (GThreadFunc)alphabeta_wrapper, (gpointer)&args[j]);
                    }
                }
                for (int j = 0; j < max_threads; j++)
                {
                    if (threads[j] != NULL)
                    {
                        payload *ret = (payload*)g_thread_join(threads[j]);
                        oldBest = bestValue.value;
                        bestValue = *(minP(&bestValue, ret));
                        if (oldBest != bestValue.value)
                        {
                            chosen = i+j;
                        }
                        free(ret);
                    }
                }
                free(subchilds);
                free(threads);
                free(args);
                b = b < bestValue.value ? b : bestValue.value;
                if (b <= a)
                    break;
            }
        }
        board *elem = g_ptr_array_index(childs, chosen);
        coord cmp = board_cmp(node, elem);
        bestValue.move_x = cmp.x;
        bestValue.move_y = cmp.y;
        /*board_print(elem);*/
        for (int i = 0; i < size; i++)
        {
            board *elem = g_ptr_array_index(childs, i);
            board_free(elem);
        }
        g_ptr_array_free(childs, TRUE);
        /*printf("evaluated: %d\n", evaluated);*/
        return bestValue;
    }
}

bool s_isdigit(const char* s)
{
    for (size_t i = 0; i < strlen(s); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

bool is_help(const char* s)
{
    return strcmp(s, "--help") == 0 || 
        strcmp(s, "-h") == 0;
}

void usage(int code)
{
    printf("usage: ./tictactoe [BOARD_SIZE] [MAX_SEARCH_DEPTH]\n");
    exit(code);
}

int convert_or_exit(const char* s)
{
    if (s_isdigit(s))
        return atoi(s);
    else
    {
        printf("error: non-integer argument\n");
        usage(1);
    }
}

int main(int argc, char **argv)
{
    int size;
    int max_depth = 10;
    if (argc == 1)
        size = 3;
    if (argc >= 2)
    {
        if (is_help(argv[1]))
            usage(0);
        else
            size = convert_or_exit(argv[1]);
    }
    if (argc == 3)
        max_depth = convert_or_exit(argv[2]);
    if (argc > 3)
    {
        usage(1);
    }
    cutoff = max_depth - 3;
    top_level = max_depth;
    max_threads = g_get_num_processors();
    printf("using %d threads.\n", max_threads);
    board *start = board_new(size);
    board_init(start);
    /*board_print(start);*/
    int value;
    bool maxPlayer = true;
    printf("Choose your side: (0) neither, (1) crosses, (2) noughts: ");
    int selection;
    scanf("%d", &selection);
    bool playerCrosses;
    bool neither = false;
    if (selection == 0)
        neither = true;
    else if (selection == 1)
        playerCrosses = true;
    else if (selection == 2)
        playerCrosses = false;
    else
    {
        printf("wrong side.");
        abort();
    }
    while (!board_terminal(start, &value))
    {

        if (!neither && maxPlayer && playerCrosses)
        {
            board_print(start);
            int x, y;
            bool accepted = false;
            while (!accepted)
            {
                printf(">> ");
                scanf("%d %d", &x, &y);
                if (x >= 0 && x <= size-1 && y >= 0 && y <= size-1)
                    accepted = true;
                else
                {
                    printf("error: coordinates outside the board\n");
                    char c;
                    scanf("%c", &c);
                }
            }
            board_set(start, x, y, CROSS);
        }
        else if (!neither && !maxPlayer && !playerCrosses)
        {
            board_print(start);
            int x, y;
            bool accepted = false;
            while (!accepted)
            {
                printf(">> ");
                scanf("%d %d", &x, &y);
                if (x >= 0 && x <= size-1 && y >= 0 && y <= size-1)
                    accepted = true;
                else
                {
                    printf("error: coordinates outside the board\n");
                    char c;
                    scanf("%c", &c);
                }
            }
            board_set(start, x, y, CIRCLE);
        }
        else
        {
            if (neither)
                board_print(start);
            begin = clock();
            payload retval = alphabeta(start, max_depth, INT_MIN, INT_MAX, maxPlayer);
            if (maxPlayer)
            {
                board_set(start, retval.move_x, retval.move_y, CROSS);
            }
            else
            {
                board_set(start, retval.move_x, retval.move_y, CIRCLE);
            }
        }
        printf("\n");
        maxPlayer = !maxPlayer;
    }
    board_print(start);
    switch (value)
    {
        case GOOD:
            printf("Crosses win!\n");
            break;
        case 0:
            printf("Draw.\n");
            break;
        case BAD:
            printf("Noughts win!\n");
            break;
    }
    board_free(start);
    return 0;
}

