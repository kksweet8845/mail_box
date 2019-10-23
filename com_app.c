#include <stdio.h>
#include <malloc.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "com_app.h"

void init_app()
{
    init_cmd();
    add_cmd("Send", do_send_cmd, "Send [id] [message]| Send to kernel with [id] and [message]");
    add_cmd("Recv", do_recv_cmd, "Recv               | Receive message from kernel ");
}
extern int errno;

bool registration_call()
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;
    gen_hdr(&dest_addr, &nlh, &iov, &msg);
    int argc = 3;
    char** argv = calloc(argc,sizeof(char*));
    argv[0] = "Registration.";
    argv[1] = malloc(strlen(id_str)+4);
    memset(argv[1], '\0', strlen(id_str)+4);
    strcat(argv[1], "id=");
    strcat(argv[1], id_str);
    argv[2] = malloc(strlen(type) + 6);
    memset(argv[2], '\0', sizeof(strlen(type)+6));
    strcat(argv[2], "type=");
    strcat(argv[2], type);

    strcpy(NLMSG_DATA(nlh), gen_str_src(argc, argv));
    int err = sendmsg(sock_fd, &msg, 0);
    if(err == -1)
    {
        printf("Fail to register to kernel module\n");
        printf("%d\n", errno);
        return false;
    }
    recvmsg(sock_fd, &msg, 0);
    recvecho((char*)NLMSG_DATA(msg.msg_iov->iov_base));
    if(strcmp((char*)NLMSG_DATA(msg.msg_iov->iov_base), "Fail") == 0)
        return false;
    return true;
}

void handle_sigint(int sig)
{
    if( do_unmount_cmd())
    {
        printf("Application %d closed\n", id);
        exit(0);
    }
}


int main(int argc, char *argv[])
{

    signal(SIGINT, handle_sigint);
    if(argc != 3)
    {
        printf("No specified id and type\n");
        return 1;
    }
    id = atoi(argv[1]);
    id_str = argv[1];
    type = argv[2];
    printf("Id: %d\n", id);
    printf("Type: %s\n", type);
    bool ok = true;
    if(ini_sock() == -1)
    {
        printf("Socket build failed\n");
        return -1;
    }
    ok = ok && registration_call();
    init_app();
    ok = ok && run_console();
    ok = ok && finish_cmd();
    close(sock_fd);
    return ok ? 0 : 1;
}
