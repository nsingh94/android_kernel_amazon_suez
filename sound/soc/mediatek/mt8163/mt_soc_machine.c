/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program
 * If not, see <http://www.gnu.org/licenses/>.
 */
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   mt_soc_machine.c
 *
 * Project:
 * --------
 *   Audio soc machine driver
 *
 * Description:
 * ------------
 *   Audio machine driver
 *
 * Author:
 * -------
 * Chipeng Chang
 *
 *------------------------------------------------------------------------------
 *
 *
 *******************************************************************************/


/*****************************************************************************
 *                     C O M P I L E R   F L A G S
 *****************************************************************************/


/*****************************************************************************
 *                E X T E R N A L   R E F E R E N C E S
 *****************************************************************************/

#include "AudDrv_Common.h"
#include "AudDrv_Def.h"
#include "AudDrv_Afe.h"
#include "AudDrv_Ana.h"
#include "AudDrv_Clk.h"
#include "AudDrv_Kernel.h"
#include "mt_soc_afe_control.h"

#include <sound/mt_soc_audio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <linux/io.h>
#include <asm/div64.h>
#include <mach/upmu_hw.h>
#include <mt-plat/mt_gpio.h>

#include <stdarg.h>
#include <linux/module.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/jack.h>
#include <linux/debugfs.h>
#include "mt_soc_codec_63xx.h"

static int mt_soc_lowjitter_control;
static struct dentry *mt_sco_audio_debugfs;
uint32_t AudDrv_Log_On;
#define DEBUG_FS_NAME "mtksocaudio"
#define DEBUG_ANA_FS_NAME "mtksocanaaudio"

static void Aud_Log_On(bool enable, uint32 value)
{
	pr_debug("Aud_Log_On enable = %d value = 0x%x\n", enable, value);

	if (enable)
		AudDrv_Log_On = value;
	else
		AudDrv_Log_On = 0;
}

static int mtmachine_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

static int mtmachine_prepare(struct snd_pcm_substream *substream)
{
	return 0;
}

static struct snd_soc_ops mt_machine_audio_ops = {

	.startup = mtmachine_startup,
	.prepare = mtmachine_prepare,
};

#if 0				/* not used */
static int mtmachine_compr_startup(struct snd_compr_stream *stream)
{
	return 0;
}

static struct snd_soc_compr_ops mt_machine_audio_compr_ops = {

	.startup = mtmachine_compr_startup,
};
#endif

static int mtmachine_startupmedia2(struct snd_pcm_substream *substream)
{
	return 0;
}

static int mtmachine_preparemedia2(struct snd_pcm_substream *substream)
{
	return 0;
}

static struct snd_soc_ops mtmachine_audio_ops2 = {

	.startup = mtmachine_startupmedia2,
	.prepare = mtmachine_preparemedia2,
};

static int mt_soc_audio_init(struct snd_soc_pcm_runtime *rtd)
{
	pr_debug("mt_soc_audio_init\n");
	return 0;
}

static int mt_soc_audio_init2(struct snd_soc_pcm_runtime *rtd)
{
	pr_debug("mt_soc_audio_init2\n");
	return 0;
}

static int mt_soc_ana_debug_open(struct inode *inode, struct file *file)
{
	pr_debug("mt_soc_ana_debug_open\n");
	return 0;
}

static ssize_t mt_soc_ana_debug_read(struct file *file, char __user *buf,
				     size_t count, loff_t *pos)
{
	const int size = 4096;
	char buffer[size];
	int n = 0;

	pr_debug("mt_soc_ana_debug_read count = %zu\n", count);
	AudDrv_ANA_Clk_On();
	AudDrv_Clk_On();
	audckbufEnable(true);

	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON0  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON0));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON1  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON1));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON2  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON2));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON3  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON3));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON4  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON4));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON5  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON5));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON6  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON6));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON7  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON7));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON8  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON8));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON9  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON9));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON10  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON10));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_CON11  = 0x%x\n", Ana_Get_Reg(ABB_AFE_CON11));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_STA0  = 0x%x\n", Ana_Get_Reg(ABB_AFE_STA0));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_STA1  = 0x%x\n", Ana_Get_Reg(ABB_AFE_STA1));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_STA2  = 0x%x\n", Ana_Get_Reg(ABB_AFE_STA2));
	n += scnprintf(buffer + n, size - n, "AFE_UP8X_FIFO_CFG0  = 0x%x\n",
		       Ana_Get_Reg(AFE_UP8X_FIFO_CFG0));
	n += scnprintf(buffer + n, size - n, "AFE_UP8X_FIFO_LOG_MON0  = 0x%x\n",
		       Ana_Get_Reg(AFE_UP8X_FIFO_LOG_MON0));
	n += scnprintf(buffer + n, size - n, "AFE_UP8X_FIFO_LOG_MON1  = 0x%x\n",
		       Ana_Get_Reg(AFE_UP8X_FIFO_LOG_MON1));
	n += scnprintf(buffer + n, size - n, "AFE_PMIC_NEWIF_CFG0  = 0x%x\n",
		       Ana_Get_Reg(AFE_PMIC_NEWIF_CFG0));
	n += scnprintf(buffer + n, size - n, "AFE_PMIC_NEWIF_CFG1  = 0x%x\n",
		       Ana_Get_Reg(AFE_PMIC_NEWIF_CFG1));
	n += scnprintf(buffer + n, size - n, "AFE_PMIC_NEWIF_CFG2  = 0x%x\n",
		       Ana_Get_Reg(AFE_PMIC_NEWIF_CFG2));
	n += scnprintf(buffer + n, size - n, "AFE_PMIC_NEWIF_CFG3  = 0x%x\n",
		       Ana_Get_Reg(AFE_PMIC_NEWIF_CFG3));
	n += scnprintf(buffer + n, size - n, "ABB_AFE_TOP_CON0  = 0x%x\n",
		       Ana_Get_Reg(ABB_AFE_TOP_CON0));
	n += scnprintf(buffer + n, size - n, "ABB_MON_DEBUG0  = 0x%x\n",
		       Ana_Get_Reg(ABB_MON_DEBUG0));

	n += scnprintf(buffer + n, size - n, "TOP_CKPDN0  = 0x%x\n", Ana_Get_Reg(TOP_CKPDN0));
	n += scnprintf(buffer + n, size - n, "TOP_CKPDN1  = 0x%x\n", Ana_Get_Reg(TOP_CKPDN1));
	n += scnprintf(buffer + n, size - n, "TOP_CKPDN2  = 0x%x\n", Ana_Get_Reg(TOP_CKPDN2));
	n += scnprintf(buffer + n, size - n, "TOP_CKCON1  = 0x%x\n", Ana_Get_Reg(TOP_CKCON1));
	n += scnprintf(buffer + n, size - n, "SPK_CON0  = 0x%x\n", Ana_Get_Reg(SPK_CON0));
	n += scnprintf(buffer + n, size - n, "SPK_CON1  = 0x%x\n", Ana_Get_Reg(SPK_CON1));
	n += scnprintf(buffer + n, size - n, "SPK_CON2  = 0x%x\n", Ana_Get_Reg(SPK_CON2));
	n += scnprintf(buffer + n, size - n, "SPK_CON6  = 0x%x\n", Ana_Get_Reg(SPK_CON6));
	n += scnprintf(buffer + n, size - n, "SPK_CON7  = 0x%x\n", Ana_Get_Reg(SPK_CON7));
	n += scnprintf(buffer + n, size - n, "SPK_CON8  = 0x%x\n", Ana_Get_Reg(SPK_CON8));
	n += scnprintf(buffer + n, size - n, "SPK_CON9  = 0x%x\n", Ana_Get_Reg(SPK_CON9));
	n += scnprintf(buffer + n, size - n, "SPK_CON10  = 0x%x\n", Ana_Get_Reg(SPK_CON10));
	n += scnprintf(buffer + n, size - n, "SPK_CON11  = 0x%x\n", Ana_Get_Reg(SPK_CON11));
	n += scnprintf(buffer + n, size - n, "SPK_CON12  = 0x%x\n", Ana_Get_Reg(SPK_CON12));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON0  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON0));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON1  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON1));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON2  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON2));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON3  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON3));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON4  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON4));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON5  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON5));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON6  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON6));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON7  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON7));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON8  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON8));
	n += scnprintf(buffer + n, size - n, "AUDTOP_CON9  = 0x%x\n", Ana_Get_Reg(AUDTOP_CON9));
	pr_debug("mt_soc_ana_debug_read len = %d\n", n);

	audckbufEnable(false);
	AudDrv_Clk_Off();
	AudDrv_ANA_Clk_Off();

	return simple_read_from_buffer(buf, count, pos, buffer, n);
}


static int mt_soc_debug_open(struct inode *inode, struct file *file)
{
	pr_debug("mt_soc_debug_open\n");
	return 0;
}

static ssize_t mt_soc_debug_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	const int size = 4096;
	char buffer[size];
	int n = 0;

	AudDrv_ANA_Clk_On();
	AudDrv_Clk_On();

	pr_debug("mt_soc_debug_read\n");
	n = scnprintf(buffer + n, size - n, "AUDIO_TOP_CON0  = 0x%x\n",
		      Afe_Get_Reg(AUDIO_TOP_CON0));
	n += scnprintf(buffer + n, size - n, "AUDIO_TOP_CON1  = 0x%x\n",
		       Afe_Get_Reg(AUDIO_TOP_CON1));
	n += scnprintf(buffer + n, size - n, "AUDIO_TOP_CON2  = 0x%x\n",
		       Afe_Get_Reg(AUDIO_TOP_CON2));
	n += scnprintf(buffer + n, size - n, "AUDIO_TOP_CON3  = 0x%x\n",
		       Afe_Get_Reg(AUDIO_TOP_CON3));
	n += scnprintf(buffer + n, size - n, "AFE_DAC_CON0  = 0x%x\n", Afe_Get_Reg(AFE_DAC_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_DAC_CON1  = 0x%x\n", Afe_Get_Reg(AFE_DAC_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_I2S_CON  = 0x%x\n", Afe_Get_Reg(AFE_I2S_CON));
	n += scnprintf(buffer + n, size - n, "AFE_DAIBT_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_DAIBT_CON0));

	n += scnprintf(buffer + n, size - n, "AFE_CONN0  = 0x%x\n", Afe_Get_Reg(AFE_CONN0));
	n += scnprintf(buffer + n, size - n, "AFE_CONN1  = 0x%x\n", Afe_Get_Reg(AFE_CONN1));
	n += scnprintf(buffer + n, size - n, "AFE_CONN2  = 0x%x\n", Afe_Get_Reg(AFE_CONN2));
	n += scnprintf(buffer + n, size - n, "AFE_CONN3  = 0x%x\n", Afe_Get_Reg(AFE_CONN3));
	n += scnprintf(buffer + n, size - n, "AFE_CONN4  = 0x%x\n", Afe_Get_Reg(AFE_CONN4));
	n += scnprintf(buffer + n, size - n, "AFE_I2S_CON1  = 0x%x\n", Afe_Get_Reg(AFE_I2S_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_I2S_CON2  = 0x%x\n", Afe_Get_Reg(AFE_I2S_CON2));
	n += scnprintf(buffer + n, size - n, "AFE_MRGIF_CON  = 0x%x\n", Afe_Get_Reg(AFE_MRGIF_CON));

	n += scnprintf(buffer + n, size - n, "AFE_DL1_BASE  = 0x%x\n", Afe_Get_Reg(AFE_DL1_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_DL1_CUR  = 0x%x\n", Afe_Get_Reg(AFE_DL1_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_DL1_END  = 0x%x\n", Afe_Get_Reg(AFE_DL1_END));
	n += scnprintf(buffer + n, size - n, "AFE_I2S_CON3  = 0x%x\n", Afe_Get_Reg(AFE_I2S_CON3));

	n += scnprintf(buffer + n, size - n, "AFE_DL2_BASE  = 0x%x\n", Afe_Get_Reg(AFE_DL2_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_DL2_CUR  = 0x%x\n", Afe_Get_Reg(AFE_DL2_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_DL2_END  = 0x%x\n", Afe_Get_Reg(AFE_DL2_END));
	n += scnprintf(buffer + n, size - n, "AFE_CONN5  = 0x%x\n", Afe_Get_Reg(AFE_CONN5));
	n += scnprintf(buffer + n, size - n, "AFE_CONN_24BIT  = 0x%x\n",
		       Afe_Get_Reg(AFE_CONN_24BIT));
	n += scnprintf(buffer + n, size - n, "AFE_AWB_BASE  = 0x%x\n", Afe_Get_Reg(AFE_AWB_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_AWB_END  = 0x%x\n", Afe_Get_Reg(AFE_AWB_END));
	n += scnprintf(buffer + n, size - n, "AFE_AWB_CUR  = 0x%x\n", Afe_Get_Reg(AFE_AWB_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_VUL_BASE  = 0x%x\n", Afe_Get_Reg(AFE_VUL_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_VUL_END  = 0x%x\n", Afe_Get_Reg(AFE_VUL_END));
	n += scnprintf(buffer + n, size - n, "AFE_VUL_CUR  = 0x%x\n", Afe_Get_Reg(AFE_VUL_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_CONN6  = 0x%x\n", Afe_Get_Reg(AFE_CONN6));
	n += scnprintf(buffer + n, size - n, "AFE_DAI_BASE  = 0x%x\n", Afe_Get_Reg(AFE_DAI_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_DAI_END  = 0x%x\n", Afe_Get_Reg(AFE_DAI_END));
	n += scnprintf(buffer + n, size - n, "AFE_DAI_CUR  = 0x%x\n", Afe_Get_Reg(AFE_DAI_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MSB  = 0x%x\n", Afe_Get_Reg(AFE_MEMIF_MSB));

	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_MON0));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_MON1));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MON2  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_MON2));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MON4  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_MON4));

	n += scnprintf(buffer + n, size - n, "AFE_ADDA_DL_SRC2_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_DL_SRC2_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_DL_SRC2_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_DL_SRC2_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_UL_SRC_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_UL_SRC_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_UL_SRC_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_UL_SRC_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_TOP_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_TOP_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_UL_DL_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_UL_DL_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_SRC_DEBUG  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_SRC_DEBUG));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_SRC_DEBUG_MON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_SRC_DEBUG_MON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_SRC_DEBUG_MON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_SRC_DEBUG_MON1));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_NEWIF_CFG0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_NEWIF_CFG0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_NEWIF_CFG1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_NEWIF_CFG1));

	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_DEBUG  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_DEBUG));
	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_MON  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_MON));
	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_COEFF  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_COEFF));
	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_SIDETONE_GAIN  = 0x%x\n",
		       Afe_Get_Reg(AFE_SIDETONE_GAIN));
	n += scnprintf(buffer + n, size - n, "AFE_SGEN_CON0  = 0x%x\n", Afe_Get_Reg(AFE_SGEN_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_TOP_CON0  = 0x%x\n", Afe_Get_Reg(AFE_TOP_CON0));

	n += scnprintf(buffer + n, size - n, "AFE_ADDA_PREDIS_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_PREDIS_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA_PREDIS_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA_PREDIS_CON1));

	n += scnprintf(buffer + n, size - n, "AFE_MRG_MON0  = 0x%x\n", Afe_Get_Reg(AFE_MRGIF_MON0));
	n += scnprintf(buffer + n, size - n, "AFE_MRG_MON1  = 0x%x\n", Afe_Get_Reg(AFE_MRGIF_MON1));
	n += scnprintf(buffer + n, size - n, "AFE_MRG_MON2  = 0x%x\n", Afe_Get_Reg(AFE_MRGIF_MON2));

	n += scnprintf(buffer + n, size - n, "AFE_MOD_DAI_BASE  = 0x%x\n",
		       Afe_Get_Reg(AFE_MOD_DAI_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_MOD_DAI_END  = 0x%x\n",
		       Afe_Get_Reg(AFE_MOD_DAI_END));
	n += scnprintf(buffer + n, size - n, "AFE_MOD_DAI_CUR  = 0x%x\n",
		       Afe_Get_Reg(AFE_MOD_DAI_CUR));

	n += scnprintf(buffer + n, size - n, "AFE_DL1_D2_BASE  = 0x%x\n",
		       Afe_Get_Reg(AFE_DL1_D2_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_DL1_D2_END  = 0x%x\n",
		       Afe_Get_Reg(AFE_DL1_D2_END));
	n += scnprintf(buffer + n, size - n, "AFE_DL1_D2_CUR  = 0x%x\n",
		       Afe_Get_Reg(AFE_DL1_D2_CUR));

	n += scnprintf(buffer + n, size - n, "AFE_VUL_D2_BASE  = 0x%x\n",
		       Afe_Get_Reg(AFE_VUL_D2_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_VUL_D2_END  = 0x%x\n",
		       Afe_Get_Reg(AFE_VUL_D2_END));
	n += scnprintf(buffer + n, size - n, "AFE_VUL_D2_CUR  = 0x%x\n",
		       Afe_Get_Reg(AFE_VUL_D2_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_CON  = 0x%x\n", Afe_Get_Reg(AFE_IRQ_MCU_CON));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MCU_CON  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_CON));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_STATUS  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_STATUS));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_CLR  = 0x%x\n", Afe_Get_Reg(AFE_IRQ_MCU_CLR));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MCU_CNT1  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_CNT1));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MCU_CNT2  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_CNT2));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MCU_EN  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_EN));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MON2  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_MON2));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ1_CNT_MON  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ1_MCU_CNT_MON));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ2_CNT_MON  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ2_MCU_CNT_MON));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ1_EN_CNT_MON  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ1_MCU_EN_CNT_MON));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_MAXLEN  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_MAXLEN));
	n += scnprintf(buffer + n, size - n, "AFE_MEMIF_PBUF_SIZE  = 0x%x\n",
		       Afe_Get_Reg(AFE_MEMIF_PBUF_SIZE));
	n += scnprintf(buffer + n, size - n, "AFE_IRQ_MCU_CNT7  = 0x%x\n",
		       Afe_Get_Reg(AFE_IRQ_MCU_CNT7));

	n += scnprintf(buffer + n, size - n, "AFE_APLL1_TUNER_CFG  = 0x%x\n",
		       Afe_Get_Reg(AFE_APLL1_TUNER_CFG));
	n += scnprintf(buffer + n, size - n, "AFE_APLL2_TUNER_CFG  = 0x%x\n",
		       Afe_Get_Reg(AFE_APLL2_TUNER_CFG));

	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN1_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN1_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CON2  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN1_CON2));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CON3  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN1_CON3));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CONN  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN1_CONN));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN1_CUR  = 0x%x\n", Afe_Get_Reg(AFE_GAIN1_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CON2  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CON2));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CON3  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CON3));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CONN  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CONN));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CUR  = 0x%x\n", Afe_Get_Reg(AFE_GAIN2_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_GAIN2_CONN2  = 0x%x\n",
		       Afe_Get_Reg(AFE_GAIN2_CONN2));

	n += scnprintf(buffer + n, size - n, "FPGA_CFG2  = 0x%x\n", Afe_Get_Reg(FPGA_CFG2));
	n += scnprintf(buffer + n, size - n, "FPGA_CFG3  = 0x%x\n", Afe_Get_Reg(FPGA_CFG3));
	n += scnprintf(buffer + n, size - n, "FPGA_CFG0  = 0x%x\n", Afe_Get_Reg(FPGA_CFG0));
	n += scnprintf(buffer + n, size - n, "FPGA_CFG1  = 0x%x\n", Afe_Get_Reg(FPGA_CFG1));
	n += scnprintf(buffer + n, size - n, "FPGA_STC  = 0x%x\n", Afe_Get_Reg(FPGA_STC));

	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON0  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON1  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON2  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON2));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON3  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON3));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON4  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON4));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON5  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON5));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON6  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON6));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON7  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON7));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON8  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON8));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON9  = 0x%x\n", Afe_Get_Reg(AFE_ASRC_CON9));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON10  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON10));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON11  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON11));
	n += scnprintf(buffer + n, size - n, "PCM_INTF_CON1  = 0x%x\n", Afe_Get_Reg(PCM_INTF_CON));
	n += scnprintf(buffer + n, size - n, "PCM_INTF_CON2  = 0x%x\n", Afe_Get_Reg(PCM_INTF_CON2));
	n += scnprintf(buffer + n, size - n, "PCM2_INTF_CON  = 0x%x\n", Afe_Get_Reg(PCM2_INTF_CON));

	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON13  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON13));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON14  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON14));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON15  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON15));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON16  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON16));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON17  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON17));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON18  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON18));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON19  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON19));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON20  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON20));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC_CON21  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC_CON21));

	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON2  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON2));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON3  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON3));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON4  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON4));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON5  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON5));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON6  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON6));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC4_CON7  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC4_CON7));

	n += scnprintf(buffer + n, size - n, "AFE_ASRC2_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC2_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC2_CON5  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC2_CON5));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC2_CON6  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC2_CON6));

	n += scnprintf(buffer + n, size - n, "AFE_ASRC3_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC3_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC3_CON5  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC3_CON5));
	n += scnprintf(buffer + n, size - n, "AFE_ASRC3_CON6  = 0x%x\n",
		       Afe_Get_Reg(AFE_ASRC3_CON6));

	/* add */
	n += scnprintf(buffer + n, size - n, "AFE_ADDA4_TOP_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA4_TOP_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA4_UL_SRC_CON0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA4_UL_SRC_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA4_UL_SRC_CON1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA4_UL_SRC_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA4_NEWIF_CFG0  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA4_NEWIF_CFG0));
	n += scnprintf(buffer + n, size - n, "AFE_ADDA4_NEWIF_CFG1  = 0x%x\n",
		       Afe_Get_Reg(AFE_ADDA4_NEWIF_CFG1));

	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_AUDDIV_0  = 0x%x\n",
		       Afe_Get_Reg(AUDIO_CLK_AUDDIV_0));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_AUDDIV_1  = 0x%x\n",
		       Afe_Get_Reg(AUDIO_CLK_AUDDIV_1));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_CFG_6      = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_CFG_6));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_CFG_6_SET  = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_CFG_6_SET));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_CFG_6_CLR  = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_CFG_6_CLR));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_AUD_DIV0   = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_AUD_DIV0));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_AUD_DIV1   = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_AUD_DIV1));
	n += scnprintf(buffer + n, size - n, "AUDIO_CLK_AUD_DIV2   = 0x%x\n",
		       GetClkCfg(AUDIO_CLK_AUD_DIV2));
	n += scnprintf(buffer + n, size - n, "AP_PLL_CON5   = 0x%x\n", GetpllCfg(AP_PLL_CON5));

	n += scnprintf(buffer + n, size - n, "AFE_HDMI_BASE  = 0x%x\n", Afe_Get_Reg(AFE_HDMI_BASE));
	n += scnprintf(buffer + n, size - n, "AFE_HDMI_CUR  = 0x%x\n", Afe_Get_Reg(AFE_HDMI_CUR));
	n += scnprintf(buffer + n, size - n, "AFE_HDMI_END  = 0x%x\n", Afe_Get_Reg(AFE_HDMI_END));

	n += scnprintf(buffer + n, size - n, "AFE_HDMI_CONN0  = 0x%x\n",
		       Afe_Get_Reg(AFE_HDMI_CONN0));
	n += scnprintf(buffer + n, size - n, "AFE_HDMI_CONN1  = 0x%x\n",
		       Afe_Get_Reg(AFE_HDMI_CONN1));
	n += scnprintf(buffer + n, size - n, "AFE_HDMI_OUT_CON0	= 0x%x\n",
		       Afe_Get_Reg(AFE_HDMI_OUT_CON0));
	n += scnprintf(buffer + n, size - n, "AFE_TDM_CON1	= 0x%x\n",
		       Afe_Get_Reg(AFE_TDM_CON1));
	n += scnprintf(buffer + n, size - n, "AFE_TDM_CON2	= 0x%x\n",
		       Afe_Get_Reg(AFE_TDM_CON2));

	pr_debug("mt_soc_debug_read len = %d\n", n);
	AudDrv_Clk_Off();
	AudDrv_ANA_Clk_Off();

	return simple_read_from_buffer(buf, count, pos, buffer, n);
}

static const char ParSetkeyAfe[] = "Setafereg";
static const char ParSetkeyAna[] = "Setanareg";
static const char ParSetkeyCfg[] = "Setcfgreg";
static const char PareGetkeyAfe[] = "Getafereg";
static const char PareGetkeyAna[] = "Getanareg";
static const char ParEnableLog[] = "Enablelog";
/* static const char ParGetkeyCfg[] = "Getcfgreg"; */
/* static const char ParSetAddr[] = "regaddr"; */
/* static const char ParSetValue[] = "regvalue"; */

static ssize_t mt_soc_debug_write(struct file *f, const char __user *buf,
				  size_t count, loff_t *offset)
{
	int ret = 0;
	char InputString[256];
	char *token1 = NULL;
	char *token2 = NULL;
	char *token3 = NULL;
	char *token4 = NULL;
	char *token5 = NULL;
	char *temp = NULL;

	unsigned long int regaddr = 0;
	unsigned long int regvalue = 0;
	char delim[] = " ,";

	memset((void *)InputString, 0, 256);
	if (count > 256)
		count = 256;
	if (copy_from_user((InputString), buf, count)) {
		pr_debug("copy_from_user mt_soc_debug_write count = %zu temp = %s\n",
			count, InputString);
	}
	temp = kstrdup(InputString, GFP_KERNEL);
	pr_debug("copy_from_user mt_soc_debug_write count = %zu temp = %s pointer = %p\n",
		count, InputString, InputString);
	token1 = strsep(&temp, delim);
	pr_debug("token1 = %s\n", token1);
	token2 = strsep(&temp, delim);
	pr_debug("token2 = %s\n", token2);
	token3 = strsep(&temp, delim);
	pr_debug("token3 = %s\n", token3);
	token4 = strsep(&temp, delim);
	pr_debug("token4 = %s\n", token4);
	token5 = strsep(&temp, delim);
	pr_debug("token5 = %s\n", token5);

	AudDrv_ANA_Clk_On();
	AudDrv_Clk_On();
	audckbufEnable(true);

	if (strcmp(token1, ParSetkeyAfe) == 0) {
		pr_debug("strcmp (token1,ParSetkeyAfe)\n");
		ret = kstrtol(token3, 16, &regaddr);
		ret = kstrtol(token5, 16, &regvalue);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyAfe, regaddr, regvalue);
		Afe_Set_Reg(regaddr, regvalue, 0xffffffff);
		regvalue = Afe_Get_Reg(regaddr);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyAfe, regaddr, regvalue);
	}
	if (strcmp(token1, ParSetkeyAna) == 0) {
		pr_debug("strcmp (token1,ParSetkeyAna)\n");
		ret = kstrtol(token3, 16, &regaddr);
		ret = kstrtol(token5, 16, &regvalue);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyAna, regaddr, regvalue);
		Ana_Set_Reg(regaddr, regvalue, 0xffffffff);
		regvalue = Ana_Get_Reg(regaddr);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyAna, regaddr, regvalue);
	}
	if (strcmp(token1, ParSetkeyCfg) == 0) {
		pr_debug("strcmp (token1,ParSetkeyCfg)\n");
		ret = kstrtol(token3, 16, &regaddr);
		ret = kstrtol(token5, 16, &regvalue);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyCfg, regaddr, regvalue);
		SetClkCfg(regaddr, regvalue, 0xffffffff);
		regvalue = GetClkCfg(regaddr);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", ParSetkeyCfg, regaddr, regvalue);
	}
	if (strcmp(token1, PareGetkeyAfe) == 0) {
		pr_debug("strcmp (token1,PareGetkeyAfe)\n");
		ret = kstrtol(token3, 16, &regaddr);
		regvalue = Afe_Get_Reg(regaddr);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", PareGetkeyAfe, regaddr, regvalue);
	}
	if (strcmp(token1, PareGetkeyAna) == 0) {
		pr_debug("strcmp (token1,PareGetkeyAna)\n");
		ret = kstrtol(token3, 16, &regaddr);
		regvalue = Ana_Get_Reg(regaddr);
		pr_debug("%s regaddr = 0x%lu regvalue = 0x%lu\n", PareGetkeyAna, regaddr, regvalue);
	}
	if (strcmp(token1, ParEnableLog) == 0) {
		ret = kstrtol(token3, 16, &regvalue);
		if (strncmp(token2, "On", 2) == 0) {
			pr_debug("Enable aud_drv debug log (0x%lu)\n", regvalue);
			Aud_Log_On(true, regvalue);
		} else {
			pr_debug("Disable aud_drv debug log!\n");
			Aud_Log_On(false, regvalue);
		}
	}

	audckbufEnable(false);
	AudDrv_Clk_Off();
	AudDrv_ANA_Clk_Off();

	return count;
}

static const struct file_operations mtaudio_debug_ops = {

	.open = mt_soc_debug_open,
	.read = mt_soc_debug_read,
	.write = mt_soc_debug_write,
};


static const struct file_operations mtaudio_ana_debug_ops = {

	.open = mt_soc_ana_debug_open,
	.read = mt_soc_ana_debug_read,
};


/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link mt_soc_dai_common[] = {

	/* FrontEnd DAI Links */
	{
	 .name = "MultiMedia1",
	 .stream_name = MT_SOC_DL1_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_DL1DAI_NAME,
	 .platform_name = MT_SOC_DL1_PCM,
	 .codec_dai_name = MT_SOC_CODEC_TXDAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "MultiMedia2",
	 .stream_name = MT_SOC_UL1_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_UL1DAI_NAME,
	 .platform_name = MT_SOC_UL1_PCM,
	 .codec_dai_name = MT_SOC_CODEC_RXDAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "Voice_MD1",
	 .stream_name = MT_SOC_VOICE_MD1_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOICE_MD1_NAME,
	 .platform_name = MT_SOC_VOICE_MD1,
	 .codec_dai_name = MT_SOC_CODEC_VOICE_MD1DAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "HDMI_OUT",
	 .stream_name = MT_SOC_HDMI_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_HDMI_NAME,
	 .platform_name = MT_SOC_HDMI_PCM,
	 .codec_dai_name = MT_SOC_CODEC_HDMI_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "ULDLOOPBACK",
	 .stream_name = MT_SOC_ULDLLOOPBACK_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_ULDLLOOPBACK_NAME,
	 .platform_name = MT_SOC_ULDLLOOPBACK_PCM,
	 .codec_dai_name = MT_SOC_CODEC_ULDLLOOPBACK_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "I2S0OUTPUT",
	 .stream_name = MT_SOC_I2S0_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_I2S0_NAME,
	 .platform_name = MT_SOC_I2S0_PCM,
	 .codec_dai_name = MT_SOC_CODEC_I2S0_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "MRGRX",
	 .stream_name = MT_SOC_MRGRX_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_MRGRX_NAME,
	 .platform_name = MT_SOC_MRGRX_PCM,
	 .codec_dai_name = MT_SOC_CODEC_MRGRX_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "MRGRXCAPTURE",
	 .stream_name = MT_SOC_MRGRX_CAPTURE_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_MRGRX_NAME,
	 .platform_name = MT_SOC_MRGRX_AWB_PCM,
	 .codec_dai_name = MT_SOC_CODEC_MRGRX_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "I2S0DL1OUTPUT",
	 .stream_name = MT_SOC_I2SDL1_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_I2S0DL1_NAME,
	 .platform_name = MT_SOC_I2S0DL1_PCM,
	 .codec_dai_name = MT_SOC_CODEC_I2S0TXDAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "DL1AWBCAPTURE",
	 .stream_name = MT_SOC_DL1_AWB_RECORD_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_DL1AWB_NAME,
	 .platform_name = MT_SOC_DL1_AWB_PCM,
	 .codec_dai_name = MT_SOC_CODEC_DL1AWBDAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "Voice_MD1_BT",
	 .stream_name = MT_SOC_VOICE_MD1_BT_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOICE_MD1_BT_NAME,
	 .platform_name = MT_SOC_VOICE_MD1_BT,
	 .codec_dai_name = MT_SOC_CODEC_VOICE_MD1_BTDAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "VOIP_CALL_BT_PLAYBACK",
	 .stream_name = MT_SOC_VOIP_BT_OUT_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOIP_CALL_BT_OUT_NAME,
	 .platform_name = MT_SOC_VOIP_BT_OUT,
	 .codec_dai_name = MT_SOC_CODEC_VOIPCALLBTOUTDAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "VOIP_CALL_BT_CAPTURE",
	 .stream_name = MT_SOC_VOIP_BT_IN_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOIP_CALL_BT_IN_NAME,
	 .platform_name = MT_SOC_VOIP_BT_IN,
	 .codec_dai_name = MT_SOC_CODEC_VOIPCALLBTINDAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "TDM_Debug_CAPTURE",
	 .stream_name = MT_SOC_TDM_CAPTURE_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_TDMRX_NAME,
	 .platform_name = MT_SOC_TDMRX_PCM,
	 .codec_dai_name = MT_SOC_CODEC_TDMRX_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "FM_MRG_TX",
	 .stream_name = MT_SOC_FM_MRGTX_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_FM_MRGTX_NAME,
	 .platform_name = MT_SOC_FM_MRGTX_PCM,
	 .codec_dai_name = MT_SOC_CODEC_FMMRGTXDAI_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "MultiMedia3",
	 .stream_name = MT_SOC_UL1DATA2_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_UL2DAI_NAME,
	 .platform_name = MT_SOC_UL2_PCM,
	 .codec_dai_name = MT_SOC_CODEC_RXDAI2_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "I2S0_AWB_CAPTURE",
	 .stream_name = MT_SOC_I2S0AWB_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_I2S0AWBDAI_NAME,
	 .platform_name = MT_SOC_I2S0_AWB_PCM,
	 .codec_dai_name = MT_SOC_CODEC_I2S0AWB_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },

	{
	 .name = "Voice_MD2",
	 .stream_name = MT_SOC_VOICE_MD2_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOICE_MD2_NAME,
	 .platform_name = MT_SOC_VOICE_MD2,
	 .codec_dai_name = MT_SOC_CODEC_VOICE_MD2DAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "PLATOFRM_CONTROL",
	 .stream_name = MT_SOC_ROUTING_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_ROUTING_DAI_NAME,
	 .platform_name = MT_SOC_ROUTING_PCM,
	 .codec_dai_name = MT_SOC_CODEC_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init2,
	 .ops = &mtmachine_audio_ops2,
	 },
	{
	 .name = "Voice_MD2_BT",
	 .stream_name = MT_SOC_VOICE_MD2_BT_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_VOICE_MD2_BT_NAME,
	 .platform_name = MT_SOC_VOICE_MD2_BT,
	 .codec_dai_name = MT_SOC_CODEC_VOICE_MD2_BTDAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "HP_IMPEDANCE",
	 .stream_name = MT_SOC_HP_IMPEDANCE_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_HP_IMPEDANCE_NAME,
	 .platform_name = MT_SOC_HP_IMPEDANCE_PCM,
	 .codec_dai_name = MT_SOC_CODEC_HP_IMPEDANCE_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "FM_I2S_RX_Playback",
	 .stream_name = MT_SOC_FM_I2S_PLAYBACK_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_FM_I2S_NAME,
	 .platform_name = MT_SOC_FM_I2S_PCM,
	 .codec_dai_name = MT_SOC_CODEC_FM_I2S_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
	{
	 .name = "FM_I2S_RX_Capture",
	 .stream_name = MT_SOC_FM_I2S_CAPTURE_STREAM_NAME,
	 .cpu_dai_name = MT_SOC_FM_I2S_NAME,
	 .platform_name = MT_SOC_FM_I2S_AWB_PCM,
	 .codec_dai_name = MT_SOC_CODEC_FM_I2S_DUMMY_DAI_NAME,
	 .codec_name = MT_SOC_CODEC_DUMMY_NAME,
	 .init = mt_soc_audio_init,
	 .ops = &mt_machine_audio_ops,
	 },
};

static const char * const I2S_low_jittermode[] = { "Off", "On" };

static const struct soc_enum mt_soc_machine_enum[] = {

	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(I2S_low_jittermode), I2S_low_jittermode),
};


static int mt8163_get_lowjitter(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: mt_soc_lowjitter_control = %d\n", __func__, mt_soc_lowjitter_control);
	ucontrol->value.integer.value[0] = mt_soc_lowjitter_control;
	return 0;
}

static int mt8163_set_lowjitter(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s()\n", __func__);
	mt_soc_lowjitter_control = ucontrol->value.integer.value[0];
	return 0;
}


static const struct snd_kcontrol_new mt_soc_controls[] = {

	SOC_ENUM_EXT("I2S low Jitter function", mt_soc_machine_enum[0], mt8163_get_lowjitter,
		     mt8163_set_lowjitter),
};

static struct snd_soc_card snd_soc_card_mt = {

	.name = "mt-snd-card",
	.dai_link = mt_soc_dai_common,
	.num_links = ARRAY_SIZE(mt_soc_dai_common),
	.controls = mt_soc_controls,
	.num_controls = ARRAY_SIZE(mt_soc_controls),
};

static struct platform_device *mt_snd_device;

static int __init mt_soc_snd_init(void)
{
	int ret;
	struct snd_soc_card *card = &snd_soc_card_mt;

	pr_debug("mt_soc_snd_init card addr = %p\n", card);

	mt_snd_device = platform_device_alloc("soc-audio", -1);
	if (!mt_snd_device) {
		pr_err("mt6589_probe  platform_device_alloc fail\n");
		return -ENOMEM;
	}
	platform_set_drvdata(mt_snd_device, &snd_soc_card_mt);
	ret = platform_device_add(mt_snd_device);

	if (ret != 0) {
		pr_err("mt_soc_snd_init goto put_device fail\n");
		goto put_device;
	}

	pr_debug("mt_soc_snd_init dai_link = %p\n", snd_soc_card_mt.dai_link);

	pr_debug("mt_soc_snd_init dai_link -----\n");
	/* create debug file */
	mt_sco_audio_debugfs = debugfs_create_file(DEBUG_FS_NAME,
						   S_IFREG | S_IRUGO, NULL, (void *)DEBUG_FS_NAME,
						   &mtaudio_debug_ops);


	/* create analog debug file */
	mt_sco_audio_debugfs = debugfs_create_file(DEBUG_ANA_FS_NAME,
						   S_IFREG | S_IRUGO, NULL,
						   (void *)DEBUG_ANA_FS_NAME,
						   &mtaudio_ana_debug_ops);

	return 0;
put_device:
	platform_device_put(mt_snd_device);
	return ret;

}

static void __exit mt_soc_snd_exit(void)
{
	platform_device_unregister(mt_snd_device);
}
module_init(mt_soc_snd_init);
module_exit(mt_soc_snd_exit);

/* Module information */
MODULE_AUTHOR("ChiPeng <chipeng.chang@mediatek.com>");
MODULE_DESCRIPTION("ALSA SoC driver ");

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mt-snd-card");
