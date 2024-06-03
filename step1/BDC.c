#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char * message);

int main(int argc, char *argv[])
{
    int sock;
    char *server_addr;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len;

    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[2]);
    server_addr = argv[1];

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling((char*)"socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, server_addr, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling((char*)"connect() error!");
    
    // str_len = read(sock, message, sizeof(message)-1);
    // if (str_len == -1)
    //     error_handling((char*)"read() error");
    
    // printf("Message from server: %s\n", message);
    printf("This is a command-line driven client.\nExcept for the commands in Protocol, you can\ninput 'E' whenever you wanna exit.\n");
    int command_num = 0;
    while(1){
        // str_len = read(sock, message, sizeof(message)-1);
        // if(str_len != -1){
        //     printf("%s\n",message);
        // }
        // * Read the command from the user
        char buffer[1024];
        memset(buffer,'\0', 1024);
        printf("Request %d: ",command_num);
        bzero(buffer, 1024);
        fgets(buffer, 1024, stdin);

        // * Send the command to the server
        int n = write(sock, buffer, strlen(buffer));
        if (n < 0) {
            perror("write");
            exit(1);
        }

        // * Read the response from the server
        bzero(buffer, 1024);
        n = read(sock, buffer, 1024);
        if (n < 0) {
            perror("read");
            exit(1);
        }

        // * Check if the server closed the connection
        if (n == 0) {
            printf("Server closed the connection\n");
            break;
        }

        // * Print the response
        printf("Response %d: %s\n",command_num, buffer);
        command_num++;
        if(strcmp(buffer, "Exit the file system") == 0){
            break;
        }
    }
    


    close(sock);
    return 0;
}

void error_handling(char * message)
{
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}
