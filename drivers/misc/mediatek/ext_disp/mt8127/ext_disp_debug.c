/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See http://www.gnu.org/licenses/gpl-2.0.html for more details.
*/

#if defined(CONFIG_MTK_HDMI_SUPPORT)
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

/* #include <mach/mt_typedefs.h> */


#if defined(CONFIG_MTK_INTERNAL_HDMI_SUPPORT)
#include "internal_hdmi_drv.h"
#elif defined(MTK_INTERNAL_MHL_SUPPORT)
#include "inter_mhl_drv.h"
#else
#include "hdmi_drv.h"
#endif
/* #include "hdmitx.h" */
/* #include "hdmitx_drv.h" */


#if defined(MTK_INTERNAL_MHL_SUPPORT)
#include "mhl_dbg.h"
#endif
#if defined(MTK_INTERNAL_HDMI_SUPPORT)
#include "hdmi_debug.h"
#endif

#include "ext_disp_debug.h"
#include "hdmitx_drv.h"
/* --------------------------------------------------------------------------- */
/* External variable declarations */
/* --------------------------------------------------------------------------- */

/* extern LCM_DRIVER *lcm_drv; */
/* --------------------------------------------------------------------------- */
/* Debug Options */
/* --------------------------------------------------------------------------- */

static char STR_HELP[] =
	"\n"
	"USAGE\n"
	"        echo [ACTION]... > hdmi\n"
	"\n" "ACTION\n"
	"        hdmitx:[on|off]\n"
	"             enable hdmi video output\n"
	"\n";

/* TODO: this is a temp debug solution */
/* extern void hdmi_cable_fake_plug_in(void); */
/* extern int hdmi_drv_init(void); */
static void process_dbg_opt(const char *opt)
{
	if (0)
		pr_err("Empty opt\n");
#if defined(CONFIG_MTK_HDMI_SUPPORT)
	else if (0 == strncmp(opt, "on", 2))
		hdmi_power_on();
	else if (0 == strncmp(opt, "off", 3))
		hdmi_power_off();
	else if (0 == strncmp(opt, "suspend", 7))
		hdmi_suspend();
	else if (0 == strncmp(opt, "resume", 6))
		hdmi_resume();
	/*else if (0 == strncmp(opt, "colorbar", 8)) {}
	else if (0 == strncmp(opt, "ldooff", 6)) {	} */
	else if (0 == strncmp(opt, "log:", 4)) {
		if (0 == strncmp(opt + 4, "on", 2))
			hdmi_log_enable(true);
		else if (0 == strncmp(opt + 4, "off", 3))
			hdmi_log_enable(false);
		else
			goto Error;
	} else if (0 == strncmp(opt, "fakecablein:", 12)) {
		if (0 == strncmp(opt + 12, "enable", 6))
			hdmi_cable_fake_plug_in();
		else if (0 == strncmp(opt + 12, "disable", 7))
			hdmi_cable_fake_plug_out();
		else
			goto Error;
	}
#if defined(MTK_INTERNAL_HDMI_SUPPORT)
	else if ((0 == strncmp(opt, "dbgtype:", 8)) ||
		 (0 == strncmp(opt, "w:", 2)) ||
		 (0 == strncmp(opt, "r:", 2)) ||
		 (0 == strncmp(opt, "hdcp:", 5)) ||
		 (0 == strncmp(opt, "status", 6)) ||
		 (0 == strncmp(opt, "help", 4)) ||
		 (0 == strncmp(opt, "res:", 4)) || (0 == strncmp(opt, "edid", 4)))
		mt_hdmi_debug_write(opt);
#endif
#endif
	else if (0 == strncmp(opt, "hdmimmp:", 8)) {
		if (0 == strncmp(opt + 8, "on", 2))
			hdmi_mmp_enable(1);
		else if (0 == strncmp(opt + 8, "init", 4))
			init_hdmi_mmp_events();
		else if (0 == strncmp(opt + 8, "off", 3))
			hdmi_mmp_enable(0);
		else if (0 == strncmp(opt + 8, "img", 3))
			hdmi_mmp_enable(7);
		else
			goto Error;
	} else if (0 == strncmp(opt, "hdmi_pattern:", 13)) {
		if (0 == strncmp(opt + 13, "on", 2))
			hdmi_pattern(1);
		else if (0 == strncmp(opt + 13, "off", 3))
			hdmi_pattern(0);
		else if (0 == strncmp(opt + 13, "svp", 3))
			hdmi_pattern(2);
		else
			goto Error;
	} else
		goto Error;

	return;

Error:
	pr_err("[hdmitx] parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
	char *tok;

	pr_err("[hdmitx] %s\n", cmd);

	while ((tok = strsep(&cmd, " ")) != NULL)
		process_dbg_opt(tok);
}

/* --------------------------------------------------------------------------- */
/* Debug FileSystem Routines */
/* --------------------------------------------------------------------------- */

struct dentry *hdmitx_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}


static char debug_buffer[4096];

static ssize_t debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	int n = 0;

	n = strlen(debug_buffer);
	debug_buffer[n++] = 0;
	return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(debug_buffer) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&debug_buffer, ubuf, count))
		return -EFAULT;

	debug_buffer[count] = 0;

	process_dbg_cmd(debug_buffer);

	return ret;
}


static const struct file_operations debug_fops = {
	.read = debug_read,
	.write = debug_write,
	.open = debug_open,
};


void HDMI_DBG_Init(void)
{
	hdmitx_dbgfs = debugfs_create_file("hdmi", S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);
}


void HDMI_DBG_Deinit(void)
{
	debugfs_remove(hdmitx_dbgfs);
}

#endif