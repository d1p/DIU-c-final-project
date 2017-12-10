#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MIN_BOARD_SIZE 3
#define MAX_BOARD_SIZE 9
#define SERVER_ADDRESS "lit-harbor-28534.herokuapp.com"

char id[30];

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
void error(char a[100]);

int main(void)
{
    // First remove the log file if that exists.
    remove("log.txt");

    // Now open the log file to record all the moves.
    FILE *file = fopen("log.txt", "w");
    if (file == NULL)
    {
        printf("Unable to open log file.\n");
    }

    greet();

    // structs for timing data
    time_t start_t, end_t;
    double time_taken = 0.0;
    time(&start_t);

    init();

    while (1 == 1)
    {
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
    time(&end_t);
    time_taken = difftime(end_t, start_t);

    int port_number = 80;
    char *host = SERVER_ADDRESS;
    char *message_fmt = "GET /submit/?id=%s&time=%lf&level=%i HTTP/1.0\r\nHost: lit-harbor-28534.herokuapp.com\r\n\r\n";
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024], response[4096];
    sprintf(message, message_fmt, id, time_taken, dimension);
    //printf("Request:\n%s\n",message);
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
        error("ERROR Connecting to the internet");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR Connecting to the internet");
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
            error("ERROR! Score saving failed.");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        error("ERROR! Score saving failed.");

    /* close the socket */
    close(sockfd);

    /* process response testing purpose*/
    //printf("Response:\n%s\n",response);
    printf("Score saved. Check your score at https://lit-harbor-28534.herokuapp.com/\n");
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
    printf("Welcome to Puzzle. Please enter your name: ");
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
    // We need to remove 1 from the board because 
    //we need the place where user can move the tile.
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
    // Basically we have to check if board[0][0] == 1, board[0][1] == 2 and on and on.
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
                return 1;
            }
        }
    }
    return 0;
}

void error(char a[100])
{
    printf("%s\n", a);
    exit(1);
}