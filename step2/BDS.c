#include <sys/mman.h> //nei cun yin she
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define LOCK 0
#define BLOCKSIZE 256
#define SECTORS 100
#define CYLINDERS 100
#define BUFFER_SIZE 256
int cylinder_last =0;
int parseLine(char *line, char *command_array[])
{
    char *p;
    int count = 0;
    p = strtok(line, " ");

    // p = strtok(line, " ");
    while (p != NULL)
    {
        command_array[count] = p;
        count++;
        p = strtok(NULL, " ");
    }
    return count;
}

int generate_512bytes_data(int operations_types, int cylinder_num, int sector_num, char *data_256, char *data_512)
{
    memset(data_512, '\0', 512);
    int *data_in_int_512 = (int*) data_512;
    data_in_int_512[0] = operations_types;
    data_in_int_512[1] = cylinder_num;
    data_in_int_512[2] = sector_num;
    memcpy(data_512+256, data_256, 256);
}

void print_sector(char *block_data){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 64; j++){
            if(block_data[64*i+j] == '\0'){
                printf("y");
            }else{
                printf("%c",block_data[64*i+j]);
            }
            
        }
        printf("\n");
    }
}

int command_execute(int client_sockfd, char *buf, char *diskfile)
{
    // char *command_array[100];
    // int command_array_num = parseLine(buf, command_array);
    int *buf_int = (int *) buf;

    if (buf_int[0] == 0)
    {
        // write(client_sockfd, "Exit the file system\n", 21);
        return 0;
    }
    // Information
    // if (strcmp(command_array[0], "I") == 0)
    // {

    //     return 1;
    // }
    // Write
    if (buf_int[0] == 2)
    {
        
        int cylinder_num = buf_int[1];
        int sector_num = buf_int[2];
        char write_string[256];
        // int write_length = atoi(command_array[3]);
        // strcpy(write_string, command_array[4]);
        // size_t write_string_length = strlen(write_string);
        memcpy(write_string, buf+12, 256);
        // memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
        // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
        memcpy(&diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], write_string, 256);
        printf("write success");
        print_sector(write_string);
        // write(client_sockfd, diskfile, strlen(diskfile));
        return 1;
    }

    // Read
    if (buf_int[0] == 1)
    {
        int cylinder_num = buf_int[1];
        int sector_num = buf_int[2];
        // read_request(client_sockfd, cylinder_num, sector_num);
        char read_string[BLOCKSIZE];
        memcpy(read_string, &diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], BLOCKSIZE);
        // read_string[BLOCKSIZE - 1] = '\0';
        // write(client_sockfd, read_string, sizeof(read_string));
        print_sector(read_string);
        printf("%s\n",read_string);
        char read_string_to_system[512];
        memcpy(read_string_to_system,read_string, 256);
        read_string_to_system[256] = '\0';
        write(client_sockfd, read_string_to_system, sizeof(read_string_to_system));
        printf("read success");

        return 1;
    }







    // char *command_array[100];
    // int command_array_num = parseLine(buf, command_array);
    // if (strcmp(command_array[0], "E") == 0)
    // {
    //     write(client_sockfd, "Exit the file system", 21);
    //     return 0;
    // }
    
    
    // Information

    // if (strcmp(command_array[0], "I") == 0)
    // {
    //     int cylinders = 100;
    //     int sectors = 100;
    //     char buf[1024];
    //     sprintf(buf,"%d %d",cylinders, sectors);
    //     write(client_sockfd, buf, sizeof(buf)); 
    //     return 1;
    // }
    
    // Write

    // if (strcmp(command_array[0], "W") == 0)
    // {
    //     if(command_array_num < 5){
    //         write(client_sockfd, "FAIL", 5);
    //         return 1;
    //     }
    //     int cylinder_num = atoi(command_array[1]);
    //     int sector_num = atoi(command_array[2]);
    //     char write_string[256];
    //     int write_length = atoi(command_array[3]);
    //     strcpy(write_string, command_array[4]);
    //     size_t write_string_length = strlen(write_string);
    //     memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
    //     // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
    //     memcpy(&diskfile[BLOCKSIZE*(cylinder_num*SECTORS + sector_num)], write_string, strlen(write_string));
    //     // write(client_sockfd, diskfile, strlen(diskfile));
    //     write(client_sockfd, "YES", 4);
    //     return 1;
    // }

    // Read

    // if (strcmp(command_array[0], "R") == 0)
    // {
    //     if(command_array_num < 3){
    //         write(client_sockfd, "FAIL", 5);
    //         return 1;
    //     }
    //     int cylinder_num = atoi(command_array[1]);
    //     int sector_num = atoi(command_array[2]);
    //     // read_request(client_sockfd, cylinder_num, sector_num);
    //     char read_string[BLOCKSIZE+1];
    //     memcpy(read_string, &diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)],BLOCKSIZE);
    //     read_string[BLOCKSIZE] = '\0';
    //     char buf[1024];
    //     sprintf(buf,"YES\n%s",read_string);
    //     write(client_sockfd, buf, sizeof(buf));
    //     return 1;
    // }
}
int read_from_client(int client_sockfd, char *buf){
    int length = read(client_sockfd, buf, 1024);
}
void client_(int client_sockfd, char * diskfile)
{
    char buf[512];
    int request_counter = 0;
    printf("connect sucess");
    while (1)
    {
        char request_num[10];
        sprintf(request_num,"%d",request_counter);
        // write(client_sockfd, "Request ", 9);
        // write(client_sockfd, request_num, strlen(request_num));
        // write(client_sockfd, ": ", 3);
        memset(buf, 0, sizeof(buf));

        int read_state = (int)read(client_sockfd, buf, sizeof(buf));
        // write(client_sockfd, buf, strlen(buf));
        // printf("%s",buf);

        if (read_state < 0)
        {
            perror("ERROR: client read state");
            break;
        }
        // write(client_sockfd, "Respond ", 9);
        // write(client_sockfd, request_num, strlen(request_num));
        // write(client_sockfd, ": ", 3);
        int enter_pos = strlen(buf);
        buf[enter_pos - 1] = '\0';
        buf[enter_pos ] = '\0';
        char buf_to_command[512];
        memcpy(buf_to_command,buf,512);
        write(client_sockfd, buf, sizeof(buf));
        int exit_ = command_execute(client_sockfd, buf_to_command, diskfile);

        if (exit_ == 0)
        {
            // write(client_sockfd, "quit_success",13);
            // close(client_sockfd);
            // write(client_sockfd, "quit_success_fail",18);
            break;
        }
        request_counter ++;
    }
    return;
}

void client_fork_new(int client_sockfd, char *diskfile)
{
    
    client_(client_sockfd, diskfile);
    printf("close success");
    close(client_sockfd);
    printf("close client success");
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("Usage: %s <disk name> <cylinder num> <sector num> <track-to-track delay> <port>\n", argv[0]);
        exit(1);
    }
    char *filename = "disk.bin";
    int fd = open(filename, O_RDWR | O_CREAT, 0);
    if (fd < 0)
    {
        printf("Error:Could not open file '%s'.\n", filename);
    }
    long FILESIZE = BLOCKSIZE * SECTORS * CYLINDERS;

    

    if (ftruncate(fd, FILESIZE) == -1){
        perror("ftruncate fail\n");
        exit(EXIT_FAILURE);
    }
    char *diskfile;
    diskfile = (char *)mmap(NULL, FILESIZE,
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (diskfile == MAP_FAILED)
    {
        close(fd);
        printf("Error:Could not map file.\n");
    }
    int port = atoi(argv[5]);
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("ERROR bind error");
        // exit(-3);
    }
    // listen(sockfd, 5);
    if (listen(sockfd, 5) == -1)
    {
        perror("ERROR listen error");
        // exit(-4);
    }
    while (1)
    {
        int client_sockfd;
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&len);

        if (client_sockfd == -1)
        {
            continue;
        }


        // write(client_sockfd, "Hello! This is BDS\n", 20);
        client_fork_new(client_sockfd, diskfile);
        // close(client_sockfd);
    }
    if (munmap(diskfile, FILESIZE) == -1)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return 0;
}