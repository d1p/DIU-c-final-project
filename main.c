#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#undef calculate
#undef getrusage

#define MIN_BOARD_SIZE 3
#define MAX_BOARD_SIZE 9
#define SERVER_ADDRESS "www.example.com"

char id[11];

int board[MIN_BOARD_SIZE][MIN_BOARD_SIZE];
int dimension;

// Function prototypes
void greet(void);
void init(void);
void clear(void);
void draw(void);
int move(int tile);
int won(void);
int getInt(void);
int getLvl(void);
double calculate(const struct rusage* b, const struct rusage* a);
void error(char a[100]);

int main(void)
{
    FILE *file = fopen("log.txt", "w");
    if (file == NULL)
    {
        printf("Unable to open log file.\n");
    }

    greet();

    // structs for timing data
    struct rusage before, after;
    double time_taken = 0.0;
    getrusage(RUSAGE_SELF, &before);

    init();

    while (1 == 1)
    {
        break;
        clear();
        draw();
        for (int i = 0; i < dimension; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                fprintf(file, "%i", board[i][j]);
                if (j < dimension - 1)
                {
                    fprintf(file, "|");
                }
            }
            fprintf(file, "\n");
        }
        fflush(file);

        if (won() == 1)
        {
            printf("Done! You win ^_~ hi5\n");
            break;
        }
        printf("Tile to move: ");
        int tile = getInt();

        // Quit the game if the user enters 0 (TEST)
        if (move(tile) == 0)
        {
            printf("Illegal move.\n");
            usleep(500000);
        }

        // Logging for testing purpose.
        fprintf(file, "%i\n", tile);
        fflush(file);

        if (move == 0)
        {
            printf("\nIllegal move.\n");
            usleep(400000);
        }
    }
    usleep(400000);
    fclose(file);
    getrusage(RUSAGE_SELF, &after);
    time_taken = calculate(&before, &after);

    int port_number = 443;
    char *host = SERVER_ADDRESS;
    char *message_fmt = "POST /id=%s&time=%lf HTTP/1.0\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024], response[4096];
    sprintf(message, message_fmt, id, time_taken);
    printf("Request:\n%s\n",message);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error openning socket.");
    }
    server = gethostbyname(host);
    if (server == NULL)
    {
        error("Error, Invalid host.");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response testing purpose*/
    printf("Response:\n%s\n",response);

    return 0;
}

int getInt(void)
{
    int i;
    scanf("%i", &i);
    return i;
}

// Clears the screen using ANSI escape sequences.
void clear(void)
{
    printf("\033[2J");
    printf("\033[%d;%dH", 0, 0);
}

void greet(void)
{
    clear();
    printf("Welcome to Puzzle. Please enter your student ID: ");
    scanf("%[^\n]%*c", id);
    dimension = getLvl();
    usleep(1000000);
}
int getLvl(void)
{
    printf("Enter the diffecalaty level (3-9): ");
    int i = getInt();
    if (i < 3 || i > 9)
    {
        printf("Error!, Enter the diffecality between 3 - 9\n");
        getLvl();
    }
    return i;
}
void init(void)
{
    // Calculate number of slots needed.
    int tiles_number = (dimension * dimension) - 1;

    // This checks if the board is even or odd. Using shorthand if else.
    int is_even = dimension * dimension % 2 ? 0 : 1;

    // Loop through columns and rows.
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            // If even then flip.
            if (is_even == 1 && tiles_number == 2)
            {
                board[i][j] = 1;
            }
            else if (is_even == 1 && tiles_number == 1)
            {
                board[i][j] = 2;
            }
            else
            {
                board[i][j] = tiles_number;
            }
            tiles_number--;
        }
    }
}

void draw(void)
{
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            if (board[i][j] == 0)
            {
                // This is the tile that user can move.
                printf("%2c", '_');
            }
            else
            {
                printf("%2d", board[i][j]);
            }
            printf(" ");
        }
        printf("\n");
    }
}

int won(void)
{
    int win_tiles = 1;
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            // check if tiles number is equal to the winning tiles number
            if (board[i][j] == win_tiles)
            {
                win_tiles++;
            }
            // check if last tile is equal to blank tile
            else if (win_tiles == dimension * dimension && board[i][j] == 0)
            {
                return 1;
            }
            // return false if any tile number not equal to winning tile order
            else
            {
                return 0;
            }
        }
    }
    return 1;
}

int move(int tile)
{
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            if (board[i][j] == tile) // Checking is this is a legal move.
            {
                // moving up (tile is within board, blank tile is above)
                if (i - 1 >= 0 && board[i - 1][j] == 0)
                {
                    board[i - 1][j] = tile;
                    board[i][j] = 0;
                }
                // moving down (tile is within board, blank tile is below)
                else if (i + 1 < dimension && board[i + 1][j] == 0)
                {
                    board[i + 1][j] = tile;
                    board[i][j] = 0;
                }
                // moving left (tile is within board, blank tile on left)
                else if (j - 1 >= 0 && board[i][j - 1] == 0)
                {
                    board[i][j - 1] = tile;
                    board[i][j] = 0;
                }
                // move right (tile is within board, blank tile on right)
                else if (j + 1 < dimension && board[i][j + 1] == 0)
                {
                    board[i][j + 1] = tile;
                    board[i][j] = 0;
                }
                // move is illegal
                else
                {
                    return 0;
                }
                // return true after a valid/legal move
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Returns number of seconds between b and a.
 */
double calculate(const struct rusage* b, const struct rusage* a)
{
    if (b == NULL || a == NULL)
    {
        return 0.0;
    }
    else
    {
        return ((((a->ru_utime.tv_sec * 1000000 + a->ru_utime.tv_usec) -
                 (b->ru_utime.tv_sec * 1000000 + b->ru_utime.tv_usec)) +
                ((a->ru_stime.tv_sec * 1000000 + a->ru_stime.tv_usec) -
                 (b->ru_stime.tv_sec * 1000000 + b->ru_stime.tv_usec)))
                / 1000000.0);
    }
}

void error(char a[100])
{
    printf("%s\n", a);
    exit(1);
}