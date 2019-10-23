#ifndef COM_KMODULE_H
#define COM_KMODULE_H

#include <linux/module.h>
#include <linux/ctype.h>
typedef struct mailbox mailbox_ele, *mailbox_ptr;

#define MAX_LEN 1000
#define MAX_BUF_LEN 256

static mailbox_ptr id_table[MAX_LEN];
static bool recv_flag = false;
static char* glob_msg;
static struct sock *nlsk;

/* Each command defined in terms of a function */
typedef bool (*cmd_function)(int argc, char* argv[]);

/*Information about each command */
/* Organized as linked list in alphabetical order */
typedef struct CELE cmd_ele, *cmd_ptr;

struct CELE
{
    char* name;
    cmd_function operation;
    char *documentation;
    cmd_ptr next;
};


/* Add a new command */
void add_cmd(char* name, cmd_function operation, char *documentation);

/*Execute a command from a command line */
bool interpret_cmd(char *cmdline);

/* Extract interger from text and store at loc */
bool get_int(char *vname, int *loc);

/* Some global values */
static cmd_ptr cmd_list = NULL;

static bool quit_flag = false;

int fd_max = 0;

bool do_help_cmd(int argc, char *argv[]);
bool do_comment_cmd(int argc, char *argv[]);
bool do_quit_cmd(int argc, char* argv[]);


static bool interpret_cmda(int argc, char* argv[]);

/**Some support function */


char* strsave(char *s)
{
    size_t len;
    char *ss;
    if(!s)
        return NULL;
    len = strlen(s);
    ss = vmalloc(len+1);
    if(!ss)
        return NULL;
    return strcpy(ss, s);
}

void free_block(void *b, size_t bytes)
{
    if( b == NULL)
    {
        printk(KERN_INFO "Attempting to free null block");
        return;
    }
    vfree(b);
}

void free_string(char* s)
{
    if(s == NULL)
    {
        printk(KERN_INFO "Atempting to free null char* ");
        return;
    }
    free_block((void*) s, strlen(s)+1);
    return;
}

void free_array(void *b)
{
    if( b == NULL)
    {
        printk(KERN_INFO "Atempting to free null block");
        return;
    }
    vfree(b);
}


void init_cmd(void)
{
    cmd_list = NULL;
    quit_flag = false;
    /* add a some default cmd */
    add_cmd("help", do_help_cmd, "                   | Show documentation");
    add_cmd("quit", do_quit_cmd, "                   | Exit program");
    add_cmd("#", do_comment_cmd, " cmd arg ...       | Display comment");
}

void add_cmd(char *name, cmd_function operation, char* documentation)
{
    /*cmd_ptr which can point to a struct CELE which is a cmd struct type */
    cmd_ptr next_cmd = cmd_list;
    cmd_ptr ele;
    cmd_ptr *last_loc = &cmd_list;
    /**strcmp return value >0 if the first character that does not match has
     * a greater value in ptr1 than ptr2 */
    while( next_cmd && strcmp(name, next_cmd->name) > 0)
    {
        /* Change the cmd linked-ist in a alphabetical order */
        last_loc =
            &(next_cmd->next);
        next_cmd = next_cmd->next;
    }
    ele = (cmd_ele *) vmalloc(sizeof(cmd_ele));
    ele->name = name;
    ele->operation = operation;
    ele->documentation = documentation;
    ele->next = next_cmd;

    *last_loc = ele;
}

char **parse_args_m(char *line, int*argcp)
{
    /**
     * Must first deterine how many arguments are there.
     * Replace all white space with null characters
     */
    size_t len = strlen(line);
    size_t i;
    char **argv;
    /* First copy into buffer with each substring null-terminated */
    char *buf = vmalloc(len+1);
    char *src = line;
    char *dst = buf;
    bool skipping = true;
    int c;
    int argc = 0;
    while((c = *src++)!= '\0')
    {
        if(c == ' ')
        {
            if(!skipping)
            {
                /* Hit end of word */
                *dst++ = '\0';
                skipping = true;
            }
        }
        else
        {
            if(skipping)
            {
                /* Hit start of new world */
                argc++;
                skipping = false;
            }
            *dst++ = c;
        }
    }
    /*Now assemble into array of strings */
    argv = (char**) vmalloc(argc*sizeof(char*));
    src = buf;
    for(i=0; i<argc; i++)
    {
        argv[i] = strsave(src);
        src += strlen(argv[i])+1;
    }
    *argcp = argc;
    return argv;
}

static bool interpret_cmda(int argc, char *argv[])
{
    cmd_ptr next_cmd;
    if(argc == 0)
        return true;
    next_cmd = cmd_list;
    bool ok = true;
    while(next_cmd && strcmp(argv[0], next_cmd->name) != 0)
        next_cmd = next_cmd->next;
    if(next_cmd)
    {
        ok = next_cmd->operation(argc, argv);
        if(!ok)
            printk(KERN_INFO "There are error when pushing cmd\n");
    }
    else
    {
        printk(KERN_INFO "Unkown command '%s'\n", argv[0]);
        ok = false;
    }
    return ok;
}


bool interpret_cmd(char *cmdline)
{
    int argc;
    char **argv;
    if(quit_flag)
        return false;

    argv = parse_args_m(cmdline, &argc);
    bool ok = interpret_cmda(argc, argv);
    int i;
    for(i = 0; i<argc; i++)
        free_string(argv[i]);
    free_array(argv);
    return ok;
}

bool do_quit_cmd(int argc, char*argv[])
{
    cmd_ptr c = cmd_list;
    bool ok = true;
    while(c)
    {
        cmd_ptr ele = c;
        c = c->next;
        free_block(ele, sizeof(ele));
    }
    quit_flag = true;
    return ok;
}

bool do_help_cmd(int argc, char* argv[])
{
    cmd_ptr clist = cmd_list;
    printk(KERN_INFO "Commands: %s\n", argv[0]);
    while(clist)
    {
        printk(KERN_INFO "\t%s\t%s\n", clist->name, clist->documentation);
        clist=clist->next;
    }
    return true;
}

bool do_comment_cmd(int argc, char* argv[])
{
    int i;
    for(i =0; i <argc; i++)
    {
        printk(KERN_INFO "%s ", argv[i]);
    }
    printk(KERN_INFO "\n");
    return true;
}

struct mailbox
{
    //0: unqueued
    //1: queued
    unsigned char type;
    unsigned char msg_data_count;
    struct msg_data *msg_data_head;
    struct msg_data *msg_data_tail;
};

typedef struct msg_data msg_data_ele, *msg_data_ptr;

struct msg_data
{
    char buf[256];
    struct msg_data *next;
};

/** macro of mailbox and msg */
#define for_each_msg(item, head)                                     \
    for(item=head->msg_data_head;item != NULL, item=item->next)

#define for_each_msg_safe(item, safe, head)                          \
    for(item=head->msg_data_head, safe=item->next;                   \
    item != NULL; \
    item=safe, safe=safe->next)

#define INIT_MAILBOX(ptr) (ptr->msg_data_head = ptr->msg_data_tail = NULL)
/**
 * Mailbox implementation
 */

bool create_mailbox(mailbox_ptr *mptr)
{
    bool ok = true;
    *mptr = vmalloc(sizeof(mailbox_ele));
    if(*mptr == NULL)
    {
        printk(KERN_INFO"Mailbox registration is failed\n");
        ok = false;
    }
    return ok;
}
bool regist_mailbox(mailbox_ptr *mptr, int id, unsigned char type)
{

    bool ok = true;
    if(id_table[id] != NULL)
    {
        printk(KERN_INFO"The %d mailbox has already existed.\n", id);
        ok = false;
    }
    ok = ok && create_mailbox(mptr);
    if(ok)
    {
        (*mptr)->type = (int) type;
        (*mptr)->msg_data_count = 0;
        (*mptr)->msg_data_head = NULL;
        (*mptr)->msg_data_tail = NULL;
        id_table[id] = *mptr;
        printk(KERN_INFO"Regist mailbox id: %d, type: %d\n", id, (*mptr)->type);
    }
    return ok;
}

/* Remove the mailbox when the user press ctrl C or quit command */
bool remove_mailbox(int id)
{
    bool ok = true;
    mailbox_ptr *mptr;
    if(id_table[id] == NULL)
    {
        printk(KERN_INFO"The %d mailbox is not exist when removing mailbox operation\n", id);
        ok = false;
    }
    mptr = &id_table[id];
    msg_data_ptr item, safe;
    for_each_msg_safe(item, safe, (*mptr))
    {
        printk(KERN_INFO"Remove %s data of %d mailbox\n", item->buf, id);
        vfree(item);
    }
    printk(KERN_INFO"Remove %d mailbox\n", id);
    vfree(*mptr);
    *mptr = NULL;
    return ok;
}

/** msg section */

bool create_msg(msg_data_ptr *msg_ptr, char* msg_str)
{
    bool ok = true;
    *msg_ptr = vmalloc(sizeof(msg_data_ele));
    if(*msg_ptr == NULL)
    {
        printk(KERN_INFO"Unable to vmalloc msg_data\n");
        ok = false;
        return ok;
    }
    memset((*msg_ptr)->buf, '\0', MAX_BUF_LEN);
    memcpy((*msg_ptr)->buf, msg_str, (strlen(msg_str) > MAX_BUF_LEN) ? MAX_BUF_LEN : strlen(msg_str));
    (*msg_ptr)->next = NULL;
    return ok;
}

/** for unqueued */
bool change_msg(mailbox_ptr mptr, char* msg_str)
{
    bool ok = true;
    msg_data_ptr msg_ptr = NULL;
    if(mptr->msg_data_head != mptr->msg_data_tail)
    {
        printk(KERN_INFO"The unqueued is not correct\n");
        ok = false;
        return ok;
    }
    if(mptr->msg_data_head == NULL)
    {
        ok = ok && create_msg(&msg_ptr, msg_str);
        mptr->msg_data_head = mptr->msg_data_tail = msg_ptr;
        return ok;
    }
    msg_ptr = mptr->msg_data_head;
    memset(msg_ptr->buf, '\0', MAX_BUF_LEN);
    memcpy(msg_ptr->buf, msg_str,((strlen(msg_str) > MAX_BUF_LEN) ? MAX_BUF_LEN : strlen(msg_str)));
    return ok;
}

/** queued */
bool push_msg(mailbox_ptr mptr, char* msg_str)
{
    bool ok = true;
    msg_data_ptr msg_ptr;
    ok = ok && create_msg(&msg_ptr, msg_str);
    if(ok)
    {
        msg_data_ptr cur;
        if(mptr->msg_data_head == NULL && mptr->msg_data_tail == NULL)
        {
            cur = mptr->msg_data_tail = mptr->msg_data_head = msg_ptr;
        }
        else
        {
            cur = mptr->msg_data_tail;
            cur->next = msg_ptr;
            mptr->msg_data_tail = msg_ptr;
        }
    }
    return ok;
}

bool pop_msg(mailbox_ptr mptr, char** msg)
{
    bool ok = true;
    msg_data_ptr cur;
    cur = mptr->msg_data_head;
    if(cur == NULL)
    {
        ok = false;
        recv_flag = false;
        printk(KERN_INFO"No data to read\n");
        return ok;
    }
    mptr->msg_data_head = cur->next;
    if(mptr->msg_data_head == NULL)
        INIT_MAILBOX(mptr);
    cur->next = NULL;
    if(*msg != NULL)
        vfree(*msg);
    *msg = vmalloc(strlen((cur->buf)));
    strcpy(*msg, cur->buf);
    vfree(cur);
    return ok;
}

/* message comming  */
bool write_msg(int id, char* msg_str)
{
    bool ok = true;
    if(id_table[id] == NULL)
    {
        printk(KERN_INFO"Try to write a not registered %d mailbox\n", id);
        ok = false;
        return ok;
    }
    unsigned char type = id_table[id]->type;
    if((int) type == 0)   /* unqueued */
    {
        ok = ok && change_msg(id_table[id], msg_str);
    }
    else if((int) type == 1)    /* queued */
    {
        ok = ok && push_msg(id_table[id], msg_str);
    }
    return ok;
}

/** Reading message */
bool read_msg(int id, char** msg)
{
    bool ok = true;
    if(id_table[id] == NULL)
    {
        printk(KERN_INFO"Try to read a not registered %d mailbox\n", id);
        ok = false;
        return ok;
    }
    unsigned type = id_table[id]->type;
    mailbox_ptr mptr = id_table[id];
    if( type == 0)
    {
        *msg = mptr->msg_data_head->buf;
        if(*msg == NULL)
            return false;
    }
    else if ( type == 1)
    {
        ok = ok && pop_msg(mptr, msg);
    }
    return ok;
}



bool do_send_cmd(int argc, char *argv[]);
bool do_recv_cmd(int argc, char *argv[]);
bool do_regist_cmd(int argc, char* argv[]);
bool do_unmount_cmd(int argc, char* argv[]);
bool send_message(int pid, int seq, void* payload);
void init_mailbox(void);


/* Extract integer from text and store at loc */
bool get_int(char *vname, int *loc)
{
    if (!kstrtoint(vname, 10, loc))
        return true;
    return false;
}

void init_module_c(void)
{
    init_cmd();
    add_cmd("Send", do_send_cmd, " ");
    add_cmd("Recv", do_recv_cmd, " ");
    add_cmd("Registration.", do_regist_cmd, " ");
    add_cmd("Unmount", do_unmount_cmd, " ");
    init_mailbox();
}

char* gen_str(int argc, char* argv[])
{
    int i=0, len=0;
    for(i=2; i<argc; i++)
    {
        len += strlen(argv[i])+1;
    }
    char* rst = vmalloc(len);
    memset(rst, '\0', len);
    for(i=2; i<argc; i++)
    {
        strcat(rst, argv[i]);
        strcat(rst, " ");
    }
    return rst;
}


bool do_send_cmd(int argc, char* argv[])
{
    bool ok = true;
    int id;
    /*Valid the format */
    if(argc < 3)
        return false;
    char *t = vmalloc(strlen(argv[1]) + 1);
    memset(t, '\0', strlen(argv[1]) + 1);
    strcpy(t, argv[1]);
    ok = ok && get_int(t, &id); /* The kstrtol must be null-terminated */
    char* msg = gen_str(argc, argv);
    ok = ok && write_msg(id, msg);
    return ok;
}

bool do_recv_cmd(int argc, char* argv[])
{
    bool ok = true;
    int id;
    ok = ok && get_int(argv[1], &id);
    recv_flag = true;
    ok = ok && read_msg(id, &glob_msg);
    return ok;
}


bool do_regist_cmd(int argc, char* argv[])
{

    mailbox_ptr mptr;
    bool ok = true;
    char *id_str = vmalloc(strlen(argv[1]));
    memset(id_str, '\0', strlen(id_str));
    strcpy(id_str, argv[1]+3);
    char *type_str = vmalloc(strlen(argv[2]));
    memset(type_str, '\0', strlen(type_str));
    strcpy(type_str, argv[2]+5);
    int id, type;
    get_int(id_str, &id);
    if(strcmp(type_str, "unqueued") == 0)
        type = 0;
    else if(strcmp(type_str, "queued") == 0)
        type = 1;
    ok = ok && regist_mailbox(&mptr, id, type);
    return ok;
}

bool do_unmount_cmd(int argc, char* argv[])
{
    mailbox_ptr mptr;
    bool ok = true;
    int id, type;
    get_int(argv[1], &id);
    ok = ok && remove_mailbox(id);
    return ok;
}

bool send_message(int pid, int seq, void* payload)
{
    bool ok = true;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int size = strlen(payload)+1;
    int len = NLMSG_SPACE(size);
    void *data = NULL;
    int ret;
    skb = alloc_skb(len, GFP_ATOMIC);
    if(!skb)
    {
        printk(KERN_INFO"Can not alloc skb\n");
        ok = false;
        return ok;
    }
    nlh = __nlmsg_put(skb, pid, seq, 0, size, NLM_F_ACK);
    nlh->nlmsg_flags=0;
    data = NLMSG_DATA(nlh);
    memcpy(data, (void*) payload, size);
    NETLINK_CB(skb).portid = 0;
    NETLINK_CB(skb).dst_group = 0;
    ret = netlink_unicast(nlsk, skb, pid, MSG_DONTWAIT);
    if(ret < 0)
    {
        printk(KERN_INFO"Send failed\n");
        ok = false;
        return ok;
    }
    printk(KERN_INFO"Finished from ack\n");
    return ok;

    if(skb)
        kfree_skb(skb);
}



void init_mailbox(void)
{
    int i=0;
    for(i=0; i<MAX_LEN; i++)
    {
        id_table[i] = (mailbox_ptr) NULL;
    }
    return;
}

bool module_done(void)
{
    return quit_flag;
}


bool module_ack(bool ok, int pid, int seq)
{
    char* ans = NULL;
    if(!ok)   /* fail ack */
    {
        ans = "Fail\0";
    }
    else if(ok && !recv_flag)
    {
        ans = "Successful\0";
    }
    else if(ok && recv_flag)
    {
        ans = glob_msg;
    }
    ok = send_message(pid, seq, (void*) ans) && ok;
    recv_flag = false;
    return ok;
}

void run_module(struct sk_buff *skb)
{
    printk(KERN_INFO "Callback occurring\n");
    u_int uid, pid, seq, sid;
    void *data = NULL;
    struct nlmsghdr *nlh;
    bool ok = true;
    nlh = (struct nlmsghdr*) skb->data;

    pid = NETLINK_CREDS(skb)->pid;
    uid = NETLINK_CREDS(skb)->uid.val;
    sid = NETLINK_CB(skb).nsid;
    seq = nlh->nlmsg_seq;
    data = NLMSG_DATA(nlh);
    ok = ok && interpret_cmd((char*) data);
    if(!ok)
    {
        printk(KERN_INFO"Recv skb from user space uid:%d pid:%d seq:%d, sid:%d\n", uid, pid, seq, sid);
        printk(KERN_INFO"data is :%s\n", (char*) data);
    }
    ok = module_ack(ok, pid, seq) && ok;
    if(!ok)
    {
        printk(KERN_INFO"=================================\n");
        printk(KERN_INFO"There are some error occurred.\n");
    }
    return;
}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Apple pie");
MODULE_DESCRIPTION("A Simple Hello World module");

#endif  //ifndef COM_KMODULE_H
