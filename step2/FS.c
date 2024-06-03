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
#include <semaphore.h>

#define CLEAR 0

#define LOCK 0
#define BLOCKSIZE 256
#define SECTORS 100
#define CYLINDERS 100
#define BUFFER_SIZE 256
int DISK_sockfd;


void read_from_disk(int sockfd, char *buffer){
    int flag;
    flag = read(sockfd, buffer, 256);
    if(flag < 0){
        perror("read disk error");
        exit(1);
    }
}
void write_from_disk(int sockfd, char *buffer){
    int flag;
    flag = write(sockfd, buffer, 512);
    if(flag < 0){
        perror("write disk error");
        exit(1);
    }
}
void close_disk(int sockfd){
    close(sockfd);
}

int generate_512bytes_data(int operations_types, int cylinder_num, int sector_num, char *data_256, char *data_512)
{
    memset(data_512, '\0', 512);
    int *data_in_int_512 = (int *)data_512;
    data_in_int_512[0] = operations_types;
    data_in_int_512[1] = cylinder_num;
    data_in_int_512[2] = sector_num;
    memcpy(data_512 + 256, data_256, 256);
}

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
void print_sector(char *block_data)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            if (block_data[64 * i + j] == '\0')
            {
                printf("y");
            }
            else
            {
                printf("%c", block_data[64 * i + j]);
            }
        }
        printf("\n");
    }
    int *index = (int *) block_data;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j <16;j ++){
            printf("%d ", index[16*i + j]);
        }
        printf("\n");
    }
}
int command_execute(int client_sockfd, char *buf, char *diskfile, char *return_data)
{
    // char *command_array[100];
    // int command_array_num = parseLine(buf, command_array);
    int *buf_int = (int *)buf;

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
        memcpy(write_string, buf + 256, 256);
        // memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
        // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
        memcpy(&diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], write_string, 256);
        // print_sector(write_string);
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
        memcpy(return_data, read_string, 256);
        // read_string[BLOCKSIZE - 1] = '\0';
        // write(client_sockfd, read_string, sizeof(read_string));
        // print_sector(read_string);
        // printf("%s\n",read_string);

        return 1;
    }
}
// char *filename = "test.bin";
// int fd = open(filename, O_RDWR | O_CREAT, 0);
// long FILESIZE = BLOCKSIZE * SECTORS * CYLINDERS;
// ftruncate(fd, FILESIZE);
// char *diskfile;
//     diskfile = (char *)mmap(NULL, FILESIZE,
//                             PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
char *diskfile;

#define BLOCK_NUM_INTOTAL 1024
#define TEST_LOCK 1

// #define BLOCKSIZE 256
// #define SECTORS 100

sem_t BLOCK_MUTEX[BLOCK_NUM_INTOTAL];
char BITMAP[BLOCK_NUM_INTOTAL];

typedef struct Inode
{
    // int user_id; // user owner id
    // int group_id; // group owner id
    // int access_time; // last access time
    // int merge_time; // last change time
    int file_type; // 1 : directory 2: file 3: root 4:user 5:share directory
    int file_size;
    int sector_id;           // inode sector id
    int pre_inode_sector_id; // pre inode sector id, father sector id
    int block_num;           // the num of block that allocate to this inode
    int name_inode;          // the name block

    int direct_block[38]; // record the block num, the same as below
    int indirect_block[12];
    int double_indirect_block[8];

} Inode;
int inode_init();
int create_inode(Inode *inode, int *sector_id, int pre_inode_sector_id, int file_type);
int store_bitmap();
void initial_bitmap()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i++)
    {
        BITMAP[i] = '0';
    }
    for (int i = 0; i < BLOCK_NUM_INTOTAL / 256; i++)
    {
        BITMAP[i] = '1';
    }
    store_bitmap();
}

#define TEST_FLAG 0

#if TEST_FLAG
int command_execute(int client_sockfd, char *buf, char *diskfile)
{
    // char *command_array[100];
    // int command_array_num = parseLine(buf, command_array);
    int *buf_int = (int *)buf;

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
        memcpy(write_string, buf + 256, 256);
        // memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
        // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
        memcpy(&diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], write_string, 256);
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
        printf("%s\n", read_string);

        return 1;
    }
}

#endif

// init semaphore
int init_semaphore()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i++)
    {
        sem_init(&BLOCK_MUTEX[i], 0, 1);
    }
}
int destroy_semaphore()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i++)
    {
        // sem_init(&BLOCK_MUTEX[i], 0, 1);
        sem_destroy(&BLOCK_MUTEX[i]);
    }
}
// block data part
int write_block(int sector_id, char *data_write)
{
    // wait
    sem_wait(&BLOCK_MUTEX[sector_id]);
#if TEST_LOCK
    char buffer[512];
    generate_512bytes_data(2, sector_id / 100, sector_id % 100, data_write, buffer);
    char data_read[256];
    command_execute(0, buffer, diskfile, NULL);

#endif

    char data[256];
    // transfer to diskserver

    sem_post(&BLOCK_MUTEX[sector_id]);
}
int read_block(int sector_id, char *data_read)
{
    sem_wait(&BLOCK_MUTEX[sector_id]);

#if TEST_LOCK
    char buffer[512];
    char tem[256];
    generate_512bytes_data(1, sector_id / 100, sector_id % 100, tem, buffer);
    // char data_read[256];
    command_execute(0, buffer, diskfile, data_read);

#endif

    // read from BDS

    sem_post(&BLOCK_MUTEX[sector_id]);
}
// renew bitmap with the system and storage
int store_bitmap()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i += 256)
    {
        char data[256];
        memcpy(data, BITMAP + i, 256);
        write_block(i / 256, data);
    }
    return 0;
}
int load_bitmap()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i += 256)
    {
        char data[256];
        read_block(i / 256, data);
        memcpy(BITMAP + i, data, 256);
    }
    return 0;
}

// get free block

int get_free_block()
{
    load_bitmap();
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i++)
    {
        if (BITMAP[i] == '0')
        {
            return i;
        }
    }
    return -1;
}

// get file/directory inode
int get_inode(int sector_id, Inode *inode)
{
    load_bitmap();
    if (sector_id < 0 || sector_id >= BLOCK_NUM_INTOTAL || BITMAP[sector_id] == '0')
    {
        return -1;
    }
    // load the inode from disk
    char inode_data[256];
    read_block(sector_id, inode_data);
    memcpy(inode, inode_data, 256);
    return 0;
}

// write inode back to disk
int write_inode_to_disk(Inode *inode)
{
    char inode_data[256];
    memcpy(inode_data, inode, 256);
    write_block(inode->sector_id, inode_data);
    return 0;
}

// get the num i sector of a file
int get_sector_id_by_index(Inode *inode, int index, int *sector_id)
{
    if (index >= inode->block_num)
    {
        return -1;
    }
    if (index < 38)
    {
        *sector_id = inode->direct_block[index];
        // if the return is -1, it means that the block does not exist.
        return 0;
    }
    if (index < 38 + 12 * 64)
    {
        int indirect_index = index - 38;
        int first = indirect_index / 64;
        int second = indirect_index % 64;
        char indirect_block_data[256];
        read_block(inode->indirect_block[first], indirect_block_data);
        int indirect_block[64];
        memcpy(indirect_block, indirect_block_data, 256);
        *sector_id = indirect_block[second];
        return 0;
    }
    if (index < 38 + 12 * 64 + 8 * 64 * 64)
    {
        int double_indirect_index = index - 38 - 12 * 64;
        int first = (double_indirect_index / 64) / 64;
        int second = (double_indirect_index / 64) % 64;
        int third = double_indirect_index % 64;
        char indirect_block_data[256];
        int indirect_block[64];
        read_block(inode->double_indirect_block[first], indirect_block_data);
        memcpy(indirect_block, indirect_block_data, 256);
        read_block(indirect_block[second], indirect_block_data);
        memcpy(indirect_block, indirect_block_data, 256);
        *sector_id = indirect_block[third];
        return 0;
    }
    return -1;
}
// create new block in inode
int allocate_new_block_to_inode(Inode *inode, int *sector_id)
{
    // file is full
    if (inode->block_num >= 38 + 12 * 64 + 8 * 64 * 64)
    {
        return -1;
    }
    *sector_id = get_free_block();
    if (*sector_id == -1)
    {
        return -1;
    }
    inode->block_num++;
    // bitmap update
    BITMAP[*sector_id] = '1';
    store_bitmap();
    int index = inode->block_num - 1;
    // assert the information of the block id
    if (index < 38)
    {
        inode->direct_block[index] = *sector_id;
        write_inode_to_disk(inode);
        return 0;
    }
    if (index < 38 + 64 * 12)
    {
        int indirect_index = index - 38;
        int first = indirect_index / 64;
        int second = indirect_index % 64;
        // second = 0, need a new indirect block
        if (second == 0)
        {
            inode->indirect_block[first] = get_free_block();
            if (inode->indirect_block[first] == -1)
            {
                // collect the block allocated
                inode->block_num--;
                BITMAP[*sector_id] = '0';
                store_bitmap();
                return -1;
            }
            BITMAP[inode->indirect_block[first]] = '1';
            store_bitmap();
        }
        char indirect_block_data[256];
        read_block(inode->indirect_block[first], indirect_block_data);
        int indirect_block[64];
        memcpy(indirect_block, indirect_block_data, 256);
        indirect_block[second] = *sector_id;
        memcpy(indirect_block_data, indirect_block, 256);
        write_block(inode->indirect_block[first], indirect_block_data);
        write_inode_to_disk(inode);
        return 0;
    }

    if (index < 38 + 12 * 64 + 8 * 64 * 64)
    {
        int double_indirect_index = index - 38 - 12 * 64;
        int first = (double_indirect_index / 64) / 64;
        int second = (double_indirect_index / 64) % 64;
        int third = double_indirect_index % 64;
        if (second == 0 && third == 0 && inode->double_indirect_block[first] == -1)
        {
            inode->double_indirect_block[first] = get_free_block();
            if (inode->double_indirect_block[first] == -1)
            {
                // collect allocated block
                inode->block_num--;
                BITMAP[*sector_id] = '0';
                store_bitmap();
                return -1;
            }
            // if fail in the  following step, the block allocated for this inode will not be collected
            BITMAP[inode->double_indirect_block[first]] = '1';
            store_bitmap();
        }

        if (third == 0)
        {
            char indirect_block_data[256];
            read_block(inode->double_indirect_block[first], indirect_block_data);
            int indirect_block[64];
            memcpy(indirect_block, indirect_block_data, 256);
            indirect_block[second] = get_free_block();
            if (indirect_block[second] == -1)
            {
                // collect allocated block
                inode->block_num--;
                BITMAP[*sector_id] = '0';
                store_bitmap();
                return -1;
            }
            BITMAP[indirect_block[second]] = '1';
            store_bitmap();
            write_block(inode->double_indirect_block[first], indirect_block_data);
        }

        //
        char indirect_block_data[256];        // the inner
        char double_indirect_block_data[256]; // the outer indirect block
        read_block(inode->double_indirect_block[first], indirect_block_data);
        int indirect_block[64];
        memcpy(indirect_block, indirect_block_data, 256);
        read_block(indirect_block[second], double_indirect_block_data);
        int double_indirect_block[64];
        memcpy(double_indirect_block, double_indirect_block_data, 256);
        double_indirect_block[third] = *sector_id;
        memcpy(double_indirect_block_data, double_indirect_block, 256);
        write_block(indirect_block[second], double_indirect_block_data);

        write_inode_to_disk(inode);
        return 0;
    }
    return -1;
}

int inode_write_data_to_disk(Inode *inode, int index, char *data)
{
    int sector_id;
    int flag = get_sector_id_by_index(inode, index, &sector_id);
    // load_bitmap();
    if (flag == -1 || sector_id < 0 || BITMAP[sector_id] == '0')
    {
        return -1;
    }
    write_block(sector_id, data);
    return 0;
}

int inode_read_data_to_disk(Inode *inode, int index, char *data)
{
    int sector_id;
    int flag = get_sector_id_by_index(inode, index, &sector_id);
    // load_bitmap();
    if (flag == -1 || sector_id < 0 || BITMAP[sector_id] == '0')
    {
        return -1;
    }
    read_block(sector_id, data);
    return 0;
}

int collect_the_block_from_end(Inode *inode)
{
    if (inode->block_num == 0)
    {
        return -1;
    }
    int index = inode->block_num - 1;
    if (index < 38)
    {
        BITMAP[inode->direct_block[index]] = '0';
        store_bitmap();
        inode->direct_block[index] = -1;
        inode->block_num--;
        write_inode_to_disk(inode);
        return 0;
    }
    if (index < 38 + 12 * 64)
    {
        int indirect_index = index - 38;
        int first = indirect_index / 64;
        int second = indirect_index % 64;
        char indirect_block_data[256];
        read_block(inode->indirect_block[first], indirect_block_data);
        int indirect_data[64];
        memcpy(indirect_data, indirect_block_data, 256);
        // collect the block
        BITMAP[indirect_data[second]] = '0';
        store_bitmap();
        indirect_data[second] = -1;
        memcpy(indirect_block_data, indirect_data, 256);
        write_block(inode->indirect_block[first], indirect_block_data);
        inode->block_num--;
        if (second == 0)
        {
            BITMAP[inode->indirect_block[first]] = '0';
            store_bitmap();
            inode->indirect_block[first] = -1;
        }
        write_inode_to_disk(inode);
        return 0;
    }
    if (index < 38 + 12 * 64 + 8 * 64 * 64)
    {
        int double_indirect_index = index - 38 - 12 * 64;
        int first = (double_indirect_index / 64) / 64;
        int second = (double_indirect_index / 64) % 64;
        int third = double_indirect_index % 64;

        char inner_indirect_block_data[256];
        char outer_indirect_block_data[256];
        read_block(inode->double_indirect_block[first], inner_indirect_block_data);
        int *inner_indirect_block = (int *)inner_indirect_block_data;
        read_block(inner_indirect_block[second], outer_indirect_block_data);
        int *outer_indirect_block = (int *)outer_indirect_block_data;
        BITMAP[outer_indirect_block[third]] = '0';
        store_bitmap();
        outer_indirect_block[third] = -1;
        write_block(inner_indirect_block[second], outer_indirect_block_data);
        inode->block_num--;
        if (third == 0)
        {
            BITMAP[inner_indirect_block[second]] = '0';
            store_bitmap();
            inner_indirect_block[second] = -1;
            write_block(inode->double_indirect_block[first], inner_indirect_block_data);
        }
        if (second == 0 && third == 0)
        {
            BITMAP[inode->double_indirect_block[first]] = '0';
            store_bitmap();
            inode->double_indirect_block[first] = -1;
        }
        write_inode_to_disk(inode);
        return 0;
    }
    return -1;
}

// inode dealing
// tested
int inode_init(Inode *inode, int sector_id, int pre_inode_sector_id, int file_type)
{
    inode->file_type = file_type;
    inode->sector_id = sector_id;
    inode->block_num = 0;
    inode->pre_inode_sector_id = pre_inode_sector_id;
    inode->name_inode = -1;
    inode->file_size = 0;
    for (int i = 0; i < 38; i++)
    {
        inode->direct_block[i] = -1;
    }
    for (int i = 0; i < 12; i++)
    {
        inode->indirect_block[i] = -1;
    }
    for (int i = 0; i < 8; i++)
    {
        inode->double_indirect_block[i] = -1;
    }
    write_inode_to_disk(inode);
    return 0;
}
// tested
int create_inode(Inode *inode, int *sector_id, int pre_inode_sector_id, int file_type)
{
    // 1 : directory 2: file 3: root
    *sector_id = get_free_block();
    // *sector_id = 4;
    if (*sector_id == -1)
    {
        return -1;
    }
    // load_bitmap();
    BITMAP[*sector_id] = '1';
    store_bitmap();
    inode_init(inode, *sector_id, pre_inode_sector_id, file_type);
    write_inode_to_disk(inode);
    return 0;
}

// 1 directory
typedef struct File_Name
{
    char name[252];
    int inode_id;
} File_Name;
int search_file_by_name(Inode *d_inode, char *name, int *id_return);
// root inode is behind the bitmap, in this system, the root inode is 4
// finished
int initial_the_file_system()
{
    initial_bitmap();
    int sector_id;
    Inode root_inode;
    create_inode(&root_inode, &sector_id, -1, 3);
}
// finished
int judge_bitmap_qualified()
{
    for (int i = 0; i < BLOCK_NUM_INTOTAL; i++)
    {
        if (BITMAP[i] != '0' && BITMAP[i] != '1')
        {
            return 0;
        }
    }
    return 1;
}
// finished
int open_file_system()
{
    // initial the semaphore
    init_semaphore();
    char root_inode_data[256];
    load_bitmap();
    if (!judge_bitmap_qualified())
    {
        initial_the_file_system();
    }
    int bitmap_block_num = BLOCK_NUM_INTOTAL / BLOCKSIZE;
    read_block(4, root_inode_data);
    Inode tem;
    memcpy(&tem, root_inode_data, 256);
    if (tem.file_type != 3)
    {
        initial_the_file_system();
        return 0;
    }
}
// finished
int fill_space_to_name(char *src, char *name_after)
{
    int src_size = strlen(src);
    strcpy(name_after, src);
    memset(name_after + src_size, '\0', 252 - src_size);
}
// finished
int create_normalized_name(char *src, File_Name *file_name, int file_sector_id)
{
    char name_after[252];
    fill_space_to_name(src, name_after);
    char tem[256];
    memcpy(tem, name_after, 252);
    int *tem_int = (int *)tem;
    tem_int[63] = file_sector_id;
    memcpy(file_name, tem, 256);
}
// add name to the child file or child directory name to
// finished
int add_name(Inode *father_inode, Inode *child_inode, char *name)
{
    if (father_inode->file_type == 2)
    {
        return -1;
    }
    int file_inode_id;
    // the name has existeds
    if (search_file_by_name(father_inode, name, &file_inode_id) == 0)
    {
        return -1;
    }
    int new_block_id;
    allocate_new_block_to_inode(father_inode, &new_block_id);

    File_Name child_inode_filename;
    create_normalized_name(name, &child_inode_filename, child_inode->sector_id);

    write_block(new_block_id, (char *)&child_inode_filename);
    child_inode->name_inode = new_block_id;
    write_inode_to_disk(father_inode);
    write_inode_to_disk(child_inode);
    return 0;
}
// finished
int remove_name(Inode *inode, char *name)
{
    // the inode is file
    if (inode->file_type == 2)
    {
        return -1;
    }
    int directory_num = inode->block_num;
    int end_index = directory_num - 1;
    File_Name end_file_name;
    int end_block_id;
    get_sector_id_by_index(inode, end_index, &end_block_id);
    read_block(end_block_id, (char *)&end_file_name);
    for (int i = 0; i < directory_num; i++)
    {
        File_Name tem_name;
        int id;
        get_sector_id_by_index(inode, i, &id);
        read_block(id, (char *)&tem_name);
        if (strcmp(tem_name.name, name) == 0 && i != end_index)
        {
            write_block(id, (char *)&end_file_name);
            collect_the_block_from_end(inode);
            return 0;
        }
        else if (strcmp(tem_name.name, name) == 0 && i == end_index)
        {
            collect_the_block_from_end(inode);
            return 0;
        }
    }
    return -1;
}
// should be changed
// finished
int create_directory_in_current_dir(Inode *inode, Inode *father_inode, char *name)
{
    if (father_inode->file_type == 2)
    {
        return -1;
    }
    // inode->file_type = 1;
    int sector_id_for_creation;
    int flag = create_inode(inode, &sector_id_for_creation, father_inode->sector_id, 1);
    if (flag == -1)
    {
        return -1;
    }
    // inode->pre_inode_sector_id = father_inode -> sector_id;

    // Add name to root inode and add name to new inode
    File_Name directory_name;
    create_normalized_name(name, &directory_name, inode->sector_id);
    int name_flag = add_name(father_inode, inode, name);
    if (name_flag == -1)
    {
        return -1;
    }
    return 0;
}

// finished
int create_file_in_current_dir(Inode *inode, Inode *father_inode, char *name)
{
    if (father_inode->file_type == 2)
    {
        return -1;
    }
    // inode->file_type = 1;
    int sector_id_for_creation;
    int flag = create_inode(inode, &sector_id_for_creation, father_inode->sector_id, 2);
    if (flag == -1)
    {
        return -1;
    }
    // inode->pre_inode_sector_id = father_inode -> sector_id;

    // Add name to root inode and add name to new inode
    File_Name directory_name;
    create_normalized_name(name, &directory_name, inode->sector_id);
    int name_flag = add_name(father_inode, inode, name);
    if (name_flag == -1)
    {
        return -1;
    }
    return 0;
}

// finished

int search_file_by_name(Inode *d_inode, char *name, int *id_return)
{

    // the rest characters are replaced by '\0'
    char name_for_search[252];
    memcpy(name_for_search, name, 252);
    // if the directory inode is not a directory return -1
    if (d_inode->file_type == 2)
    {
        return -1;
    }
    int files_num = d_inode->block_num;
    for (int i = 0; i < files_num; i++)
    {
        File_Name name_block_for_search;
        int file_id;
        get_sector_id_by_index(d_inode, i, &file_id);
        read_block(file_id, (char *)&name_block_for_search);
        if (strcmp(name_block_for_search.name, name_for_search) == 0)
        {
            *id_return = name_block_for_search.inode_id;
            return 0;
        }
    }
    return -1;
}

// // add file to directory
// int add_file_to_directory(Inode *d_inode, Inode*file_inode)
// {
//     if (d_inode->file_type == 0)
//     {
//         return -1;
//     }
// }

// finished
// get the file name in the directory and the num of files in total
int list_file_in_current_directory(Inode *inode, char name[256][256], int *name_num)
{
    // if inode is file
    if (inode->file_type == 2)
    {
        return -1;
    }
    int directory_num = inode->block_num;
    for (int i = 0; i < directory_num; i++)
    {
        File_Name tem_name;
        int id;
        get_sector_id_by_index(inode, i, &id);
        read_block(id, (char *)&tem_name);
        // get the file type of the name
        // first get the inode
        Inode tem_inode;
        int tem_filetype;
        get_inode(tem_name.inode_id, &tem_inode);
        tem_filetype = tem_inode.file_type;
        int *f_type_of_name = (int *)name[i];
        memcpy(name[i], &tem_name, 256);
        f_type_of_name[63] = tem_filetype;
    }
    *name_num = directory_num;
    return 0;
}
int get_current_route(Inode *inode, char *name_new, char *user_name, char *write_to_client)
{
    Inode tem_inode = *inode;
    if (inode->file_type == 2)
    {
        return -1;
    }
    char name[4096 + 256];
    memset(name, '\0', 4096 + 256);
    // strcpy(name, name_new);
    while (tem_inode.pre_inode_sector_id != -1)
    {
        File_Name tem_name;
        read_block(tem_inode.name_inode, (char *)&tem_name);
        char buffer[4096];
        char name_3840[4096 - 256];
        strcpy(name_3840, name);
        sprintf(buffer, "/%s%s", tem_name.name, name_3840);
        strcpy(name, buffer);
        get_inode(tem_inode.pre_inode_sector_id, &tem_inode);
    }
    printf("\033[0;32m%s\033[0;37m:\033[0;34m~%s\033[0;37m$ ", user_name, name);
    sprintf(write_to_client, "\033[0;32m%s\033[0;37m:\033[0;34m~%s\033[0;37m$ ", user_name, name);
    return 0;
}
int create_directory_for_client(Inode *current_directory_inode, Inode *directory_inode, char *name)
{
    int sector_id;
    // if(create_inode(directory_inode, &sector_id, current_directory_inode ->sector_id, 1) == -1){
    //     return -1;
    // }
    return 0;
}
//
int clear_file_content(Inode *inode)
{
    // directory return
    if (inode->file_type != 2)
    {
        return -1;
    }
    int block_num_delete = inode->block_num;
    for (int i = 0; i < block_num_delete; i++)
    {
        collect_the_block_from_end(inode);
    }
    inode->block_num = 0;
    inode->file_size = 0;
    // update inode to disk
    write_inode_to_disk(inode);
    return 0;
}

// delete a file with "name"
// finished
int delete_file_from_directory(Inode *directory_inode, char *name)
{
    Inode file_inode;

    int file_inode_id;
    if (search_file_by_name(directory_inode, name, &file_inode_id) == -1)
    {
        return -1;
    }
    get_inode(file_inode_id, &file_inode);
    // fail to clear the content of file
    if (clear_file_content(&file_inode) == -1)
    {
        return -1;
    }
    if (remove_name(directory_inode, name) == -1)
    {
        return -1;
    }
    BITMAP[file_inode.sector_id] = '0';
    store_bitmap();
    return 0;
}
int delete_directory_from_directory(Inode *father_inode, char *name);
// int clear_directory(Inode *inode)
// {
//     // the inode is file
//     if(inode->file_type == 2){
//         return -1;
//     }
//     int files_num = inode -> block_num;
//     for(int i = 0; i < files_num; i++){
//         Inode tem_file;

//     }
// }
// finished
int delete_directory_from_directory(Inode *father_inode, char *name)
{
    Inode child_directory_inode;
    int child_directory_inode_id;
    if (search_file_by_name(father_inode, name, &child_directory_inode_id) == -1)
    {
        return -1;
    }
    get_inode(child_directory_inode_id, &child_directory_inode);
    if (child_directory_inode.file_type == 2)
    {
        return -1;
    }
    int child_directory_sub_filenum = child_directory_inode.block_num;
    // if the directory is not empty ,then clear
    // clear_directory(&child_directory_inode);
    for (int i = 0; i < child_directory_sub_filenum; i++)
    {
        File_Name tem_name;
        Inode tem_inode;
        int id;
        get_sector_id_by_index(&child_directory_inode, i, &id);
        read_block(id, (char *)&tem_name);
        int tem_inode_id = tem_name.inode_id;
        get_inode(tem_inode_id, &tem_inode);
        if (tem_inode.file_type == 2)
        {
            delete_file_from_directory(&child_directory_inode, tem_name.name);
        }
        else
        {
            delete_directory_from_directory(&child_directory_inode, tem_name.name);
        }
    }
    remove_name(father_inode, name);
    BITMAP[child_directory_inode.sector_id] = '0';
    store_bitmap();
    return 0;
}
// finished
int list_directory(Inode *inode, char *data_return)
{
    if (inode->file_type == 2)
    {
        return -1;
    }
    char name[256][256];
    int file_num;
    list_file_in_current_directory(inode, name, &file_num);
    for (int i = 0; i < file_num; i++)
    {
        char file_name[252];
        memcpy(file_name, name[i], 252);
        int file_type;
        int *file_type_inname = (int *)name[i];
        file_type = file_type_inname[63];
        if (file_type == 2)
        {
            printf("\033[0;37m%s\n", file_name);
            sprintf(data_return, "%s\033[0;37m%s\n", data_return, file_name);
        }
        else
        {
            printf("\033[0;34m%s\n\033[0;37m", file_name);
            sprintf(data_return, "%s\033[0;34m%s\n\033[0;37m", data_return, file_name);
        }
    }
    return 0;
}

int write_file(Inode *inode, int length, char *data)
{
    if (inode->file_type != 2)
    {
        return -1;
    }
    clear_file_content(inode);
    int blocknum_total = (length + 255) / 256;
    inode->file_size = length;
    // remember to write inode to disk
    for (int i = 0; i < blocknum_total; i++)
    {
        int id;
        allocate_new_block_to_inode(inode, &id);
        char data_write_to_block[256];
        memcpy(data_write_to_block, data + i * 256, 256);
        write_block(id, data_write_to_block);
    }
    write_inode_to_disk(inode);
    return 0;
}

int read_file(Inode *inode, int length, char *data)
{
    if (inode->file_type != 2)
    {
        return -1;
    }
    int file_size = inode->file_size;
    if (length > file_size)
    {
        return -1;
    }

    int blocknum_total = (length + 256) / 256;
    for (int i = 0; i < blocknum_total; i++)
    {
        char data_read_from_block[256];
        inode_read_data_to_disk(inode, i, data_read_from_block);
        memcpy(data + i * 256, data_read_from_block, 256);
    }
    return file_size;
}

int rewrite_file()
{
    // delete the file and make new file
}

// int insert_data_in_file()
// {
// }

// int delete_data_in_file()
// {
// }
// finished, the directory inode will be switched to upper directory
int change_to_father_directory(Inode *inode)
{
    // the inode is file
    if (inode->file_type == 2)
    {
        return -1;
    }
    // roots
    if (inode->pre_inode_sector_id == -1)
    {
        return -1;
    }
    write_inode_to_disk(inode);
    get_inode(inode->pre_inode_sector_id, inode);
}
int change_to_child_directory(Inode *inode, char *name)
{
    if (inode->file_type == 2)
    {
        return -1;
    }
    int child_inode_id;
    if (search_file_by_name(inode, name, &child_inode_id) == -1)
    {
        return -1;
    }
    Inode child_inode;
    get_inode(child_inode_id, &child_inode);
    if (child_inode.file_type == 2)
    {
        return -1;
    }
    write_inode_to_disk(inode);
    memcpy((char *)inode, (char *)&child_inode, 256);
    return 0;
}

// deal_with_operations

int make_file(Inode *directory_inode, char *file_name)
{
    Inode file_inode;

    return create_file_in_current_dir(&file_inode, directory_inode, file_name);
}
int make_directory(Inode *directory_inode, char *directory_name)
{
    Inode child_directory_inode;
    return create_directory_in_current_dir(&child_directory_inode, directory_inode, directory_name);
}
int remove_file(Inode *directory_inode, char *file_name)
{
    return delete_file_from_directory(directory_inode, file_name);
}
int remove_directory(Inode *directory_inode, char *directory_name)
{
    return delete_directory_from_directory(directory_inode, directory_name);
}
// unfinished
int parse_path(char *path, char path_after[256][252], int *num)
{
    *num = 0;
    char *p;
    char tem_path[256];
    strcpy(tem_path, path);
    p = strtok(path, "/");
    while (p != NULL)
    {
        // command_array[count] = p;
        strcpy(path_after[*num], p);
        (*num)++;
        p = strtok(NULL, "/");
    }
    return 0;
}

int change_directory(Inode *directory, char *path)
{
    char path_parsed[256][252];
    int path_num = 0;
    parse_path(path, path_parsed, &path_num);
    // the function must be changed in step 3
    // store the inode
    write_inode_to_disk(directory);
    Inode tem_inode = *directory;
    for (int i = 0; i < path_num; i++)
    {
        if (strcmp(path_parsed[i], "..") == 0)
        {
            if (change_to_father_directory(&tem_inode) == -1)
            {
                return -1;
            }
        }
        else if (strcmp(path_parsed[i], ".") == 0)
        {
            continue;
        }
        else
        {
            if (change_to_child_directory(&tem_inode, path_parsed[i]) == -1)
            {
                return -1;
            }
        }
    }
    // this function must be changed in step 3
    *directory = tem_inode;
    return 0;
}

int list_directory_(Inode *directory, char name[256][256])
{
}
int catch_file(Inode *directory, char *file_name, char *data)
{
    int file_inode_id;
    if (search_file_by_name(directory, file_name, &file_inode_id) == -1)
    {
        return -1;
    }
    Inode file_inode;
    get_inode(file_inode_id, &file_inode);
    if (file_inode.file_type != 2)
    {
        return -1;
    }
    int file_length = read_file(&file_inode, file_inode.file_size, data);
    data[file_length] = '\0';
    return file_length;
}
// read_file and write_file function may appear the access overline
int write_file_for_operations(Inode *directory, char *file_name, int length, char *data)
{
    // the input is invalid
    // if(length > sizeof(data)){
    //     return -1;
    // }
    int file_inode_id;
    if (search_file_by_name(directory, file_name, &file_inode_id) == -1)
    {
        return -1;
    }
    Inode file_inode;
    get_inode(file_inode_id, &file_inode);
    return write_file(&file_inode, length, data);
}

int insert_file_data(Inode *directory, char *file_name, int pos, int length, char *data)
{
    Inode file_inode;
    int file_inode_id;
    if (search_file_by_name(directory, file_name, &file_inode_id) == -1)
    {
        return -1;
    }
    get_inode(file_inode_id, &file_inode);
    if (pos > file_inode.file_size)
    {
        pos = file_inode.file_size;
    }
    int new_length = file_inode.file_size + length;
    char buffer[new_length + 256];
    read_file(&file_inode, file_inode.file_size, buffer);
    for (int i = new_length - 1; i >= pos + length; i--)
    {
        buffer[i] = buffer[i - length];
    }
    for (int i = 0; i < length; i++)
    {
        buffer[pos + i] = data[i];
    }
    return write_file(&file_inode, new_length, buffer);
}
int delete_data_in_file(Inode *directory, char *file_name, int pos, int length)
{
    Inode file_inode;
    int file_inode_id;
    if (search_file_by_name(directory, file_name, &file_inode_id) == -1)
    {
        return -1;
    }
    get_inode(file_inode_id, &file_inode);
    if (pos + length > file_inode.file_size)
    {
        length = file_inode.file_size - pos;
    }
    int pre_length = file_inode.file_size;
    int new_length = file_inode.file_size - length;
    char buffer[pre_length + 256];
    read_file(&file_inode, pre_length, buffer);
    memcpy(buffer + pos, buffer + pos + length, pre_length - length - pos + 1);
    memset(buffer + pre_length - length, '\0', sizeof(buffer) - new_length);
    write_file(&file_inode, new_length, buffer);
    return 0;
}

// #define LOCK 0
// #define BLOCKSIZE 256
// #define SECTORS 100
// #define CYLINDERS 100
// #define BUFFER_SIZE 256
// the instruction is changed to
// read: R <cylinder-num> <sector-num> data
// write: W <cylinder-num> <sector-num> data
// R -> 1, W -> 2

// int generate_512bytes_data(int operations_types, int cylinder_num, int sector_num, char *data_256, char *data_512)
// {
//     memset(data_512, '\0', 512);
//     int *data_in_int_512 = (int*) data_512;
//     data_in_int_512[0] = operations_types;
//     data_in_int_512[1] = cylinder_num;
//     data_in_int_512[2] = sector_num;
//     memcpy(data_512+256, data_256, 256);
// }

int parse_command(char *line, char *command_array[])
{
    char *p;
    int count = 0;
    p = strtok(line, " ");

    // p = strtok(line, " ");
    while (p != NULL && count <= 5)
    {
        command_array[count] = p;
        count++;
        p = strtok(NULL, " ");
    }
    // return count;
    // f
    if (strcmp(command_array[0], "f") == 0)
    {
        return 1;
    }
    // e
    if (strcmp(command_array[0], "e") == 0)
    {
        return 1;
    }
    // mk f
    if (strcmp(command_array[0], "mk") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // mkdir d
    if (strcmp(command_array[0], "mkdir") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // rm f
    if (strcmp(command_array[0], "rm") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // cd path
    if (strcmp(command_array[0], "cd") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // rmdir d
    if (strcmp(command_array[0], "rmdir") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // ls
    if (strcmp(command_array[0], "ls") == 0)
    {
        return 1;
    }
    // cat f
    if (strcmp(command_array[0], "cat") == 0)
    {
        if (count < 2)
        {
            return -1;
        }
        return 2;
    }
    // w f l data
    if (strcmp(command_array[0], "w") == 0)
    {
        if (count < 4)
        {
            return -1;
        }
        return 4;
    }
    // i f pos l data
    if (strcmp(command_array[0], "i") == 0)
    {
        if (count < 5)
        {
            return -1;
        }
        return 5;
    }
    // d f pos l
    if (strcmp(command_array[0], "d") == 0)
    {
        if (count < 4)
        {
            return -1;
        }
        return 4;
    }
}
int file_system_command_execute(int client_sockfd, char *commandline, Inode *current_directory, Inode *root, int length, char *data_return)
{
    // the return 0 means the exit of the file system
    // the return -1 menas that cannot commit the command
    // char command_array[5][1024];
    char *command_array[10];
    // char commandline[1024];
    // printf("\n%s\n",commandline);
    // memcpy(commandline, commandline1, length);
    // commandline[length-2] = '\0';
    // commandline[length-1] = '\0';
    int num_of_command = parse_command(commandline, command_array);
    if (num_of_command == -1)
    {
        return -1;
    }
    if (strcmp(command_array[0], "f") == 0)
    {
        initial_the_file_system();
        get_inode(4, current_directory);
        get_inode(4, root);
        return 1;
    }
    // e
    if (strcmp(command_array[0], "e") == 0)
    {
        return 0;
    }
    // mk f
    if (strcmp(command_array[0], "mk") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        if (make_file(current_directory, name) == -1)
        {
            return -1;
        }
        return 2;
    }
    // mkdir d
    if (strcmp(command_array[0], "mkdir") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        if (make_directory(current_directory, name) == -1)
        {
            return -1;
        }
        return 2;
    }
    // rm f
    if (strcmp(command_array[0], "rm") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        if (remove_file(current_directory, name) == -1)
        {
            return -1;
        }
        return 2;
    }
    // cd path
    if (strcmp(command_array[0], "cd") == 0)
    {
        char path[256];
        strcpy(path, command_array[1]);
        if (change_directory(current_directory, path) == -1)
        {
            return -1;
        }
        return 2;
    }
    // rmdir d
    if (strcmp(command_array[0], "rmdir") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        if (remove_directory(current_directory, name) == -1)
        {
            return -1;
        }
        return 2;
    }
    // ls
    if (strcmp(command_array[0], "ls") == 0)
    {
        // char route[1024];
        char file_data[1024];
        if (list_directory(current_directory, file_data) == -1)
        {
            return -1;
        }
        // sprintf(data_return,"%s",output_data);
        strcpy(data_return, file_data);
        return 1;
    }
    // cat f
    if (strcmp(command_array[0], "cat") == 0)
    {
        char file_data[1024];
        char name[252];
        strcpy(name, command_array[1]);
        if (catch_file(current_directory, name, file_data) == -1)
        {
            return -1;
        }
        printf("%s\n", file_data);
        // strcat(data_return, file_data);
        sprintf(data_return, "%s\n", file_data);
        return 2;
    }
    // w f l data
    if (strcmp(command_array[0], "w") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        int length = atoi(command_array[2]);
        char data[1024];
        if (length > sizeof(commandline) - (int)(command_array[3] - commandline))
        {
            length = sizeof(commandline) - (int)(command_array[3] - commandline);
        }
        if (length <= 1024)
        {
            memcpy(data, command_array[3], length);
        }
        else
        {
            memcpy(data, command_array[3], 1024);
        }
        if (write_file_for_operations(current_directory, name, length, data) == -1)
        {
            return -1;
        }
        return 4;
    }
    // i f pos l data
    if (strcmp(command_array[0], "i") == 0)
    {
        char name[252];
        int length = atoi(command_array[3]);
        int pos = atoi(command_array[2]);
        strcpy(name, command_array[1]);
        char data[1024];
        if (length > sizeof(commandline) - (int)(command_array[4] - commandline))
        {
            length = sizeof(commandline) - (int)(command_array[4] - commandline);
        }
        if (length <= 1024)
        {
            memcpy(data, command_array[4], length);
        }
        else
        {
            memcpy(data, command_array[4], 1024);
        }
        if (insert_file_data(current_directory, name, pos, length, data) == -1)
        {
            return -1;
        }
        return 5;
    }
    // d f pos l
    if (strcmp(command_array[0], "d") == 0)
    {
        char name[252];
        strcpy(name, command_array[1]);
        int length = atoi(command_array[3]);
        int pos = atoi(command_array[2]);
        if (delete_data_in_file(current_directory, name, pos, length) == -1)
        {
            return -1;
        }
        return 4;
    }
}
// void print_sector(char *block_data){
//     for(int i = 0; i < 4; i++){
//         for(int j = 0; j < 64; j++){
//             if(block_data[64*i+j] == '\0'){
//                 printf("y");
//             }else{
//                 printf("%c",block_data[64*i+j]);
//             }

//         }
//         printf("\n");
//     }
// }
#if LOCK
int command_execute(int client_sockfd, char *buf, char *diskfile)
{
    // char *command_array[100];
    // int command_array_num = parseLine(buf, command_array);
    int *buf_int = (int *)buf;

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
        memcpy(write_string, buf + 256, 256);
        // memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
        // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
        memcpy(&diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], write_string, 256);
        // print_sector(write_string);
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
        // print_sector(read_string);
        // printf("%s\n",read_string);

        return 1;
    }
}
#endif

// int command_execute(int client_sockfd, char *buf, char *diskfile, char *return_data)
// {
//     // char *command_array[100];
//     // int command_array_num = parseLine(buf, command_array);
//     int *buf_int = (int *) buf;

//     if (buf_int[0] == 0)
//     {
//         // write(client_sockfd, "Exit the file system\n", 21);
//         return 0;
//     }
//     // Information
//     // if (strcmp(command_array[0], "I") == 0)
//     // {

//     //     return 1;
//     // }
//     // Write
//     if (buf_int[0] == 2)
//     {
//         int cylinder_num = buf_int[1];
//         int sector_num = buf_int[2];
//         char write_string[256];
//         // int write_length = atoi(command_array[3]);
//         // strcpy(write_string, command_array[4]);
//         // size_t write_string_length = strlen(write_string);
//         memcpy(write_string, buf+256, 256);
//         // memset(write_string + write_string_length, '\0', sizeof(write_string) - write_string_length);
//         // write_request(client_sockfd, cylinder_num, sector_num, write_length, write_string);
//         memcpy(&diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], write_string, 256);
//         // print_sector(write_string);
//         // write(client_sockfd, diskfile, strlen(diskfile));
//         return 1;
//     }

//     // Read
//     if (buf_int[0] == 1)
//     {
//         int cylinder_num = buf_int[1];
//         int sector_num = buf_int[2];
//         // read_request(client_sockfd, cylinder_num, sector_num);
//         char read_string[BLOCKSIZE];
//         memcpy(read_string, &diskfile[BLOCKSIZE * (cylinder_num * SECTORS + sector_num)], BLOCKSIZE);
//         // read_string[BLOCKSIZE - 1] = '\0';
//         // write(client_sockfd, read_string, sizeof(read_string));
//         // print_sector(read_string);
//         // printf("%s\n",read_string);

//         return 1;
//     }
// }

// void client_(int client_sockfd, char *diskfile)
// {
//     char buf[1024];
//     int request_counter = 0;
//     while (1)
//     {
//         char request_num[10];
//         sprintf(request_num, "%d", request_counter);
//         write(client_sockfd, "Request ", 9);
//         write(client_sockfd, request_num, strlen(request_num));
//         write(client_sockfd, ": ", 3);
//         memset(buf, 0, sizeof(buf));

//         int read_state = (int)read(client_sockfd, buf, sizeof(buf));
//         // write(client_sockfd, buf, strlen(buf));
//         // printf("%s",buf);

//         if (read_state < 0)
//         {
//             perror("ERROR: client read state");
//             break;
//         }
//         write(client_sockfd, "Respond ", 9);
//         write(client_sockfd, request_num, strlen(request_num));
//         write(client_sockfd, ": ", 3);
//         int enter_pos = strlen(buf);
//         buf[enter_pos - 1] = '\0';
//         buf[enter_pos - 2] = '\0';
//         char data_read[256];
//         int exit_ = command_execute(client_sockfd, buf, diskfile, data_read);

//         if (exit_ == 0)
//         {
//             write(client_sockfd, "quit_success", 13);
//             // close(client_sockfd);
//             write(client_sockfd, "quit_success_fail", 18);
//             break;
//         }
//         request_counter++;
//     }
//     return;
// }

// void client_fork_new(int client_sockfd, char *diskfile)
// {
//     client_(client_sockfd, diskfile);
//     close(client_sockfd);
//     printf("close client success");
//     return;
// }

// Inode root_of_system;
// Inode current_directory;

void client_(int client_sockfd, char *diskfile)
{
    char buf[1024];
    int request_counter = 0;
    open_file_system();
    Inode root_of_system;
    get_inode(4, &root_of_system);
    Inode current_directory = root_of_system;
    char begin_output[1024];
    get_current_route(&current_directory, "", "mysystem", begin_output);
    write(client_sockfd, begin_output, sizeof(begin_output));
    int current_directory_inode_id;
    while (1)
    {
        char request_num[10];
        sprintf(request_num, "%d", request_counter);
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
        buf[enter_pos] = '\0';
        // int exit_ = command_execute(client_sockfd, buf, diskfile);
        {
            char route[2048];
            char return_data[2048];
            memset(return_data, '\0', 2048);
            // get_current_route(&current_directory, "", "mysystem", route);
            char command_line[1024];
            // fgets(command_line, sizeof(command_line), stdin);
            strcpy(command_line, buf);
            // printf("command:%s\n",command_line);
            // command_line[strlen(command_line) - 1] = '\0';
            int flag = file_system_command_execute(0, command_line, &current_directory, &root_of_system, 0, return_data);
            get_current_route(&current_directory, "", "mysystem", route);
            // printf("\nthe route is:\n%s\nend of the route\n", route);
            // printf("the returndata is:\n%s\nend of the returndata\n", return_data);
            char write_to_client_data[1024];
            memset(write_to_client_data,'\0',1024);
            if (flag == -1)
            {
                printf("error to execute the command\n");
                strcat(write_to_client_data,"error to execute the command\n");
            }
            strcat(write_to_client_data, return_data);
            // strcat(write_to_client_data,"\n");
            strcat(write_to_client_data,route);
            // sprintf(write_to_client_data, "%s\n%s",return_data, route);
            
            
            if (flag == 0)
            {
                break;
            }
            write(client_sockfd, write_to_client_data, sizeof(write_to_client_data));
            write_inode_to_disk(&current_directory);
            get_inode(4, &root_of_system);
        }
        int exit_ = 1;
        // write(client_sockfd, buf, sizeof(buf));

        if (exit_ == 0)
        {
            // write(client_sockfd, "quit_success",13);
            // close(client_sockfd);
            // write(client_sockfd, "quit_success_fail",18);
            break;
        }
        request_counter++;
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

void create_server(int *sockfd, int port)
{
    struct sockaddr_in server_addr;
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1)
    {
        fprintf(stderr, "Error: cannot create the socket\n");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (bind(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "Error: cannot bind the server to the port\n");
        close(*sockfd);
        exit(1);
    }
    listen(*sockfd, 5);
    printf("Success: create the server\n");
    printf("Server is listening on port %d\n", port);
}
void create_disk_server(int port)
{
    // *create FS server
    int sockfd;
    create_server(&sockfd, port);

    // *client execution
    while (1)
    {

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_sockfd == -1)
        {
            fprintf(stderr, "Error: cannot accept the client\n");
            continue;
        }

        printf("Client %s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        client_fork_new(client_sockfd, diskfile);
    }
}

int main(int argc, char *argv[])
{
    char *Disk_Server_Address;
    int BDS_port;
    int FS_port;
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
    // char *diskfile;
    diskfile = (char *)mmap(NULL, FILESIZE,
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (diskfile == MAP_FAILED)
    {
        close(fd);
        printf("Error:Could not map file.\n");
    }
    // print_sector(diskfile+4*256);
    // print_sector(diskfile+6*256);
    
    if (argc < 4)
    {
        fprintf(stderr,
                "Usage: %s <DiskServerAddress> <BDSPort> "
                "<FSPort>\n",
                argv[0]);
                exit(-1);
    }
    Disk_Server_Address = argv[1];
    BDS_port = atoi(argv[2]);
    FS_port = atoi(argv[3]);
    // create diskserver address
    // using BDS_port Disk_Server_Address socket_fd
    struct sockaddr_in bds_serv_addr;
    DISK_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(DISK_sockfd == -1){
        perror("socket()error");
        exit(1);
    }
    memset(&bds_serv_addr, 0, sizeof(bds_serv_addr));
    bds_serv_addr.sin_family = AF_INET;
    bds_serv_addr.sin_addr.s_addr = inet_addr(Disk_Server_Address);
    bds_serv_addr.sin_port = htons(BDS_port);
    if(inet_pton(AF_INET, Disk_Server_Address, &bds_serv_addr.sin_addr) == -1){
        perror("inet_pton() error");
        exit(1);
    }
    if(connect(DISK_sockfd, (struct sockaddr *)&bds_serv_addr, sizeof(bds_serv_addr)) == -1){
        perror("connect() error");
        exit(1);
    }


    // initial the filesystem

    // open_file_system();
    // Inode root_of_system;
    // get_inode(4, &root_of_system);
    // Inode current_directory = root_of_system;
    // current_directory = root_of_system;
    // print_sector(diskfile+5*256);

    // while (1)
    // {
    //     char route[2048];
    //     char return_data[2048];
    //     memset(return_data,'\0',2048);
    //     get_current_route(&current_directory, "", "mysystem", route);
    //     char command_line[1024];
    //     fgets(command_line, sizeof(command_line), stdin);
    //     // printf("command:%s\n",command_line);
    //     command_line[strlen(command_line) - 1] = '\0';
    //     int flag = file_system_command_execute(0, command_line, &current_directory, &root_of_system, 0, return_data);
    //     // print_sector(diskfile+4*256);
    //     printf("\nthe route is:\n%s\nend of the route\n",route);
    //     printf("the returndata is:\n%s\nend of the returndata\n",return_data);
    //     if (flag == -1)
    //     {
    //         printf("error to execute the command\n");
    //     }
    //     if (flag == 0)
    //     {
    //         break;
    //     }
    //     write_inode_to_disk(&current_directory);
    //     get_inode(4, &root_of_system);
    // }
    // print_sector(diskfile+4*256);

    // create_disk_server(FS_port);

    int port = atoi(argv[3]);
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