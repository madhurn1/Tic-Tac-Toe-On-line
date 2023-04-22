#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFLEN 1024

typedef struct handle
{
    int socket;
} handle_t;

typedef struct msg
{
    char code[5];
    char *fields[256];
    int num_fields;
} msg_t;

int field_count(char *type);
int read_field(char *buf, int start, int end, char **field);
int p_recv(int, msg_t *msg);

int main()
{

    char name[BUFLEN];
    char buf[BUFLEN];
    int bytes;
    printf("First, what is your name?\n");
    bytes = read(STDIN_FILENO, name, BUFLEN);
    name[bytes - 1] = '\0';
    bytes = snprintf(buf, BUFLEN, "PLAY|%d|%s|\n", bytes, name);
    write(STDOUT_FILENO, buf, bytes);

    struct msg pass;
    pass.num_fields = 0;
    return p_recv(STDIN_FILENO, &pass);
}
int field_count(char *type)
{
    if (strcmp(type, "WAIT") == 0)
        return 2;
    if (strcmp(type, "BEGN") == 0 || strcmp(type, "INVL") == 0 || strcmp(type, "DRAW") == 0 || strcmp(type, "OVER") == 0)
        return 3;
    if (strcmp(type, "MOVD"))
        return 4;

    return 0;
}

int read_field(char *buf, int start, int end, char **field)
{
    int len = end - start;
    *field = malloc(len + 1);
    if (*field == NULL)
    {
        perror("malloc");
        return -1;
    }
    strncpy(*field, buf + start, len);
    (*field)[len] = '\0';
    return 0;
}

int p_recv(int sock, msg_t *msg)
{
    char buf[BUFLEN];
    int len = 0;
    int msgend = 0;
    int leftover_length = 0;
    int field_num = 0;

    while (1)
    {
        int bytes_read = read(sock, buf + len, BUFLEN - len);
        buf[bytes_read - 1] = '\0';

        if (bytes_read == -1)
        {
            perror("read");
            return -1;
        }
        else if (bytes_read == 0)
        {
            printf("closed\n");
            // connection closed
            return 0;
        }

        len += bytes_read;

        // check for complete message
        for (int i = msgend; i < len; i++)
        {
            if (buf[i] == '|')
            {
                if (msg->num_fields == 0)
                {
                    // first field is the code
                    if (i - msgend != 4)
                    {
                        fprintf(stderr, "error: invalid code length\n");
                        return -1;
                    }
                    strncpy(msg->code, buf + msgend, 4);
                    msg->code[4] = '\0';
                    field_num = field_count(msg->code);
                    printf("%s %d\n", msg->code, field_num);
                }
                else if (msg->num_fields == 1)
                {
                    // second field is the message length
                    char *size_str;
                    if (read_field(buf, msgend, i, &size_str) == -1)
                    {
                        return -1;
                    }
                    int size = atoi(size_str);
                    if (size < 0 || size > BUFLEN)
                    {
                        fprintf(stderr, "error: invalid message size\n");
                        free(size_str);
                        return -1;
                    }
                    free(size_str);
                    printf("i: %d, msgend: %d, len: %d, bytes: %d\n", i, msgend, len, bytes_read);
                    if (size > bytes_read - i - 1)
                        // message is not complete
                        return 0;

                    printf("%d\n", size);
                }
                else
                {
                    // subsequent fields are variable-length strings
                    char *field;
                    if (read_field(buf, msgend, i, &field) == -1)
                    {
                        return -1;
                    }
                    msg->fields[msg->num_fields - 2] = field;
                    printf("%s\n", field);
                }
                msgend = i + 1;
                msg->num_fields++;
            }
        }

        printf("%d\n", len);
        // check for improperly formatted message
        if (buf[len - 2] != '|')
        {
            fprintf(stderr, "error: message not terminated with '|'\n");
            return -1;
        }

        // there's more data to come, so move leftover data to the front of the buffer
        if (msgend != 0)
        {
            leftover_length = len - msgend;
            memmove(buf, buf + msgend, leftover_length);
            len = leftover_length;
            msgend = 0;
        }
    }
}
