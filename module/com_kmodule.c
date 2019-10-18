#include "com_kmodule.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/init.h>
#include <linux/skbuff.h>
/**
 * In net/sock
 */
#include <net/sock.h>
#include <linux/vmalloc.h> /* In kernel module, it is needed to use kmalloc or vmalloc*/
static struct sock *nlsk;

/**
 * struct netlink_kernel_cfg {
 *	unsigned int groups;
 *	unsigned int flags;
 *	void (*input)(struct skbuff *skb);
 *	struct mutext *cb_mutex;
 *	int (*bind)(struct net *net, int group)
 *	void (*unbind)(struct net *net, int group)
 *	bool (*compare)(struct net *ntet, struct sock *sk);
 * }
 * */
static struct netlink_kernel_cfg *cfg;


static void udp_reply(int pid, int seq, void *payload)
{

    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int size = strlen(payload)+1;
    int len = NLMSG_SPACE(size);
    void *data;
    int ret;

    skb = alloc_skb(len, GFP_ATOMIC);
    if(!skb)
        return;
    /* skb. portid, seq, type, payload, flags*/
    nlh = __nlmsg_put(skb, pid, seq, 0, size, NLM_F_ACK);
    nlh->nlmsg_flags=0;
    char r[14] = "Fuck you too.\0";
    data = NLMSG_DATA(nlh);
    memcpy(data, (void*)r, sizeof(r));
    NETLINK_CB(skb).portid = 0; /* From kernel */
    NETLINK_CB(skb).dst_group = 0; /* unicast */
    ret = netlink_unicast(nlsk, skb, pid, MSG_DONTWAIT);
    if(ret < 0)
    {
        printk("send failed\n");
        return;
    }
    return;

nlmsg_failure:
    if(skb)
        kfree_skb(skb);
}

void udp_receive(struct sk_buff *skb)
{
    printk(KERN_INFO "Callbak occrring\n");
    u_int uid, pid, seq, sid;
    void *data = NULL;
    struct nlmsghdr *nlh;

    nlh = (struct nlmsghdr*) skb->data;
    /**
     * NETLINK_CREDS which will parse out the private variables of msg;
     */
    pid = NETLINK_CREDS(skb)->pid;
    uid = NETLINK_CREDS(skb)->uid.val;
    sid = NETLINK_CB(skb).nsid;
    seq = nlh->nlmsg_seq;
    data = NLMSG_DATA(nlh);
    printk("Recv skb from user space uid:%d pid:%d seq:%d, sid:%d\n", uid, pid, seq, sid);
    printk("data is :%s\n", (char*) data);
    udp_reply(pid, seq, data);
    return;
}

int netlink_bind(struct net *net, int group)
{
    return 0;
}


static int __init com_kmodule_init(void)
{
    /* Unsigned long size*/
    cfg = vmalloc(sizeof(struct netlink_kernel_cfg));
    cfg->groups = 0; /* Unicast */
    cfg->flags = NL_CFG_F_NONROOT_SEND | NL_CFG_F_NONROOT_RECV; /* Reserved for user mode socket protocols*/
    cfg->input = &udp_receive;
    cfg->cb_mutex = NULL;
    cfg->bind = NULL;
    cfg->unbind = NULL;
    cfg->compare = NULL;

    nlsk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, cfg);
    if(!nlsk)
    {
        printk(KERN_INFO "The netlink kernel create return NULL\n");
        return -1;
    }
    printk(KERN_INFO "Enter module. Hello world!\n");
    return 0;
}

static void __exit com_kmodule_exit(void)
{
    netlink_kernel_release(nlsk);
    printk(KERN_INFO "Exit module. Bye~\n");
}

module_init(com_kmodule_init);
module_exit(com_kmodule_exit);
