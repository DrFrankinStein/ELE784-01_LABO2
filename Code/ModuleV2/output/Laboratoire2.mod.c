#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf4edea6c, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2deae9aa, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0x20a26e80, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x4307f807, __VMLINUX_SYMBOL_STR(usb_register_dev) },
	{ 0x701d4f6, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0xedc1a25a, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x10380b27, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xdf497eac, __VMLINUX_SYMBOL_STR(usb_find_interface) },
	{ 0x473fe3a3, __VMLINUX_SYMBOL_STR(usb_control_msg) },
	{ 0x4c4fef19, __VMLINUX_SYMBOL_STR(kernel_stack) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x7da51b40, __VMLINUX_SYMBOL_STR(usb_deregister_dev) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v046Dp08CCd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v046Dp0994d*dc*dsc*dp*ic*isc*ip*in*");