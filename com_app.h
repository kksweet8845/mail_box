#ifndef COM_APP_H
#define COM_APP_H

#include <stdbool.h>
#include <string.h>
#include <error.h>
#include "console.h"

#define QUEUE_l "queued"
#define UNQUEUE_l "unqueued"

/* Define the operation 1. Send 2.Recv */
#define SEND 0
#define RECV 1
#define MAX_PAYLOAD 2048

static int sock_fd;
static int id;
static char* id_str;
static char* type;
static struct sockaddr_nl src_addr;


bool do_send_cmd(int argc, char* argv[]);
bool do_recv_cmd(int argc, char* argv[]);
bool do_unmount_cmd(void);

void recvecho(char* s)
{
    printf("%s\n", s);
    return;
}

int ini_sock()
{
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;
    if( (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) == -1))
    {
        return -1;
    }
    return 0;
}

char* gen_str_src(int argc, char* argv[])
{
    int i=0, len=0;
    for(i=0; i<argc; i++)
    {
        len += strlen(argv[i])+1;
    }
    char* rst = malloc(len);
    memset(rst, '\0', len);
    for(i=0; i<argc; i++)
    {
        strcat(rst, argv[i]);
        strcat(rst, " ");
    }
    return rst;
}

char* gen_str(int argc, char* argv[])
{
    int i=0, len=0;
    for(i=2; i<argc; i++)
    {
        len += strlen(argv[i])+1;
    }
    char* rst = malloc(len);
    memset(rst, '\0', len);
    for(i=2; i<argc; i++)
    {
        strcat(rst, argv[i]);
        strcat(rst, " ");
    }
    return rst;
}

void gen_hdr(struct sockaddr_nl* dest_addr_ptr,
             struct nlmsghdr **nlh_ptr,
             struct iovec* iov_ptr,
             struct msghdr* msg_ptr)
{
    memset(dest_addr_ptr, 0, sizeof(*dest_addr_ptr));
    dest_addr_ptr->nl_family = AF_NETLINK;
    dest_addr_ptr->nl_pid = 0;
    dest_addr_ptr->nl_groups = 0;

    *nlh_ptr = (struct nlmsghdr*) malloc(NLMSG_SPACE(MAX_PAYLOAD));
    (*nlh_ptr)->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    (*nlh_ptr)->nlmsg_pid = getpid();
    (*nlh_ptr)->nlmsg_flags = 0;

    iov_ptr->iov_base = (void*) *nlh_ptr;
    iov_ptr->iov_len = (*nlh_ptr)->nlmsg_len;
    memset(msg_ptr, 0, sizeof(*msg_ptr));
    msg_ptr->msg_name = (void*) dest_addr_ptr;
    msg_ptr->msg_namelen = sizeof(*dest_addr_ptr);
    msg_ptr->msg_iov = iov_ptr;
    msg_ptr->msg_iovlen = 1;
}



bool do_send_cmd(int argc, char* argv[])
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;

    gen_hdr(&dest_addr, &nlh, &iov, &msg);
    strcpy(NLMSG_DATA(nlh), gen_str_src(argc, argv));
    if( sendmsg(sock_fd, &msg, 0) == -1)
    {
        printf("Send failed\n");
        return false;
    }
    recvmsg(sock_fd, &msg, 0);
    recvecho((char*)NLMSG_DATA(msg.msg_iov->iov_base));
    return true;
}

bool do_recv_cmd(int argc, char* argv[])
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;

    gen_hdr(&dest_addr, &nlh, &iov, &msg);
    argc = 2;
    argv = (char**) malloc(2*sizeof(char*));
    argv[0] = "Recv";
    argv[1] = id_str;
    strcpy(NLMSG_DATA(nlh), gen_str_src(argc, argv));
    if( sendmsg(sock_fd, &msg, 0) == -1)
    {
        printf("Receiving from kernel failed\n");
        return false;
    }
    recvmsg(sock_fd, &msg, 0);
    recvecho((char*)NLMSG_DATA(msg.msg_iov->iov_base));
    return true;
}

bool do_unmount_cmd(void)
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int argc = 0;
    char **argv;
    gen_hdr(&dest_addr, &nlh, &iov, &msg);
    argc = 2;
    argv = (char**) malloc(sizeof(char*));
    argv[0] = "Unmount";
    argv[1] = id_str;
    strcpy(NLMSG_DATA(nlh), gen_str_src(argc, argv));
    if( sendmsg(sock_fd, &msg, 0) == -1)
    {
        printf("Receiving from kernel failed\n");
        return false;
    }
    recvmsg(sock_fd, &msg, 0);
    recvecho((char*)NLMSG_DATA(msg.msg_iov->iov_base));
    return true;
}


#endif  //ifndef COM_APP_H
