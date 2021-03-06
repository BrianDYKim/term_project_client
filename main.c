#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char client_name_info[NAME_SIZE] = "DEFAULT";
char msg[BUF_SIZE];

// TODO send_msg, recv_msg를 수정해서 원하는대로 client의 동작을 조절한다
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "[%s]", argv[3]); // 메세지에 붙어서 나갈 이름 포맷을 결정하기
    sprintf(client_name_info, "%s", argv[3]); // 해당 client가 사용할 대화명을 결정하기
    sock = socket(PF_INET, SOCK_STREAM, 0); // IPv4를 사용하는 TCP 소켓을 하나 할당

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    // connect가 이뤄지자마자 server측에 자신의 이름 정보를 넘겨준다
    write(sock, client_name_info, strlen(client_name_info));

    // send, receive에 대한 thread를 각각 만든다
    pthread_create(&snd_thread, NULL, send_msg, (void *) &sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *) &sock);
    // blocking 함수를 이용해서 쓰레드가 종료될 때까지 main process를 잡아둔다
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void *send_msg(void *arg)   // send thread main
{
    int sock = *((int *) arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void *recv_msg(void *arg)   // read thread main
{
    int sock = *((int *) arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    while (1) {
        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        if (str_len == -1)
            return (void *) -1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}