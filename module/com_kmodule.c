#include "com_kmodule.h"

static int __init com_kmodule_init(void)
{
    printk(KERN_INFO "Enter module. Hello world!\n");
    return 0;
}

static void __exit com_kmodule_exit(void)
{
    printk(KERN_INFO "Exit module. Bye~\n");
}

module_init(com_kmodule_init);
module_exit(com_kmodule_exit);
