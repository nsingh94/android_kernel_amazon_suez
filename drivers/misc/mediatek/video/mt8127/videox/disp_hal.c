/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "disp_drv_platform.h"
#include "lcm_drv.h"
#include "disp_hal.h"
#include "lcd_drv.h"
#include "lcd_reg.h"
#include "dpi_drv.h"
#include "dpi_reg.h"
#include "dsi_drv.h"
#include "dsi_reg.h"

static LCD_IF_ID ctrl_if = LCD_IF_PARALLEL_0;
static LCM_PARAMS s_lcm_params = { 0 };

static bool disp_use_mmu;
LCM_PARAMS *lcm_params = &s_lcm_params;

int disphal_process_dbg_opt(const char *opt)
{
	if (0 == strncmp(opt, "dbi:", 4)) {
		if (0 == strncmp(opt + 4, "on", 2))
			LCD_PowerOn();
		else if (0 == strncmp(opt + 4, "off", 3))
			LCD_PowerOff();
		else
			goto Error;
	} else if (0 == strncmp(opt, "dpi:", 4)) {
		if (0 == strncmp(opt + 4, "on", 2))
			DPI_PowerOn();
		else if (0 == strncmp(opt + 4, "off", 3))
			DPI_PowerOff();
		else
			goto Error;
	} else if (0 == strncmp(opt, "dsi:", 4)) {
		if (0 == strncmp(opt + 4, "on", 2))
			DSI_PowerOn();
		else if (0 == strncmp(opt + 4, "off", 3))
			DSI_PowerOff();
		else
			goto Error;

	} else if (0 == strncmp(opt, "te:", 3)) {
		if (0 == strncmp(opt + 3, "on", 2)) {
			if (DSI_Get_EXT_TE()) {
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "EXT TE is enabled, can not enable BTA TE now\n"); */
			} else {
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "Before: BTA_TE = %d, EXT_TE = %d\n",
				   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */
				LCD_TE_Enable(true);
				DSI_TE_Enable(true);
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "After : BTA_TE = %d, EXT_TE = %d\n",
				   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */

			}
		} else if (0 == strncmp(opt + 3, "off", 3)) {

			/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
			   "Before: BTA_TE = %d, EXT_TE = %d\n",
			   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */
			LCD_TE_Enable(false);
			DSI_TE_Enable(false);
			/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
			   "After : BTA_TE = %d, EXT_TE = %d\n",
			   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */

		} else {
			goto Error;
		}
	} else if (0 == strncmp(opt, "te_ext:", 7)) {
		if (0 == strncmp(opt + 7, "on", 2)) {
			if (DSI_Get_BTA_TE()) {
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "BTA TE is enabled, can not enable EXT TE now\n"); */
			} else {
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "Before: BTA_TE = %d, EXT_TE = %d\n",
				   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */
				LCD_TE_Enable(true);
				DSI_TE_EXT_Enable(true);
				/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
				   "After : BTA_TE = %d, EXT_TE = %d\n",
				   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */
			}
		} else if (0 == strncmp(opt + 7, "off", 3)) {

			/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
			   "Before: BTA_TE = %d, EXT_TE = %d\n",
			   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */
			LCD_TE_Enable(false);
			DSI_TE_EXT_Enable(false);
			/* DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI",
			   "After : BTA_TE = %d, EXT_TE = %d\n",
			   DSI_Get_BTA_TE(),DSI_Get_EXT_TE()); */

		} else {
			goto Error;
		}
	} else if (0 == strncmp(opt, "reg:", 4)) {
		if (0 == strncmp(opt + 4, "dpi", 3)) {
			DPI_DumpRegisters();
			/*DPI1_DumpRegisters(); */
		} else if (0 == strncmp(opt + 4, "dsi", 3)) {
			DSI_DumpRegisters();
		} else {
			goto Error;
		}
	} else if (0 == strncmp(opt, "lcdlog:", 7)) {
		if (0 == strncmp(opt + 7, "on", 2))
			dbi_log_enable(true);
		else if (0 == strncmp(opt + 7, "off", 3))
			dbi_log_enable(false);
		else
			goto Error;

	} else if (0 == strncmp(opt, "dsilog:", 7)) {
		if (0 == strncmp(opt + 7, "on", 2))
			DSI_Enable_Log(true);
		else if (0 == strncmp(opt + 7, "off", 3))
			DSI_Enable_Log(false);
		else
			goto Error;

	} else if (0 == strncmp(opt, "dpilog:", 7)) {
		if (0 == strncmp(opt + 7, "on", 2))
			;
		else
			goto Error;

	} else
		goto Error;
	return 0;
Error:
	return -1;
}

const DISP_IF_DRIVER *disphal_get_if_driver(void)
{
	switch (lcm_params->type) {
	case LCM_TYPE_DBI:
		return DISP_GetDriverDBI();
	case LCM_TYPE_DPI:
		return DISP_GetDriverDPI();
	case LCM_TYPE_DSI:
		return DISP_GetDriverDSI();
	default:
		ASSERT(0);
	}
	return NULL;
}

static void lcm_set_reset_pin(uint32_t value)
{
	LCD_SetResetSignal(value);
}

static void lcm_udelay(uint32_t us)
{
	udelay(us);
}

static void lcm_mdelay(uint32_t ms)
{
	msleep(ms);
}

static void lcm_send_cmd(uint32_t cmd)
{
	if (lcm_params == NULL)
		return;

	ASSERT(LCM_CTRL_SERIAL_DBI == lcm_params->ctrl ||
	       LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

	LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_LOW, cmd, lcm_params->dbi.cpu_write_bits));
}

static void lcm_send_data(uint32_t data)
{
	if (lcm_params == NULL)
		return;

	ASSERT(LCM_CTRL_SERIAL_DBI == lcm_params->ctrl ||
	       LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

	LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_HIGH, data, lcm_params->dbi.cpu_write_bits));
}

static uint32_t lcm_read_data(void)
{
	uint32_t data = 0;

	if (lcm_params == NULL)
		return 0;

	ASSERT(LCM_CTRL_SERIAL_DBI == lcm_params->ctrl ||
	       LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

	LCD_CHECK_RET(LCD_ReadIF(ctrl_if, LCD_IF_A0_HIGH, &data, lcm_params->dbi.cpu_write_bits));

	return data;
}

static const LCM_UTIL_FUNCS lcm_utils = {
	.set_reset_pin = lcm_set_reset_pin,
	.udelay = lcm_udelay,
	.mdelay = lcm_mdelay,
	.send_cmd = lcm_send_cmd,
	.send_data = lcm_send_data,
	.read_data = lcm_read_data,
	.dsi_set_cmdq = (void (*)(unsigned int *, unsigned int, unsigned char))DSI_set_cmdq,
	.dsi_set_cmdq_V2 = DSI_set_cmdq_V2,
	.dsi_set_cmdq_V3 =
	    (void (*)(LCM_setting_table_V3 *, unsigned int, unsigned char))DSI_set_cmdq_V3,
	.dsi_write_cmd = DSI_write_lcm_cmd,
	.dsi_write_regs = DSI_write_lcm_regs,
	.dsi_read_reg = DSI_read_lcm_reg,
	.dsi_dcs_read_lcm_reg = DSI_dcs_read_lcm_reg,
	.dsi_dcs_read_lcm_reg_v2 = DSI_dcs_read_lcm_reg_v2,
};

LCM_UTIL_FUNCS fbconfig_lcm_utils = {
	.set_reset_pin = lcm_set_reset_pin,
	.udelay = lcm_udelay,
	.mdelay = lcm_mdelay,
	.send_cmd = lcm_send_cmd,
	.send_data = lcm_send_data,
	.read_data = lcm_read_data,
	.dsi_set_cmdq = (void (*)(unsigned int *, unsigned int, unsigned char))DSI_set_cmdq,
	.dsi_set_cmdq_V2 = DSI_set_cmdq_V2,
	.dsi_set_cmdq_V3 =
	    (void (*)(LCM_setting_table_V3 *, unsigned int, unsigned char))DSI_set_cmdq_V3,
	.dsi_write_cmd = DSI_write_lcm_cmd,
	.dsi_write_regs = DSI_write_lcm_regs,
	.dsi_read_reg = DSI_read_lcm_reg,
	.dsi_dcs_read_lcm_reg = DSI_dcs_read_lcm_reg,
	.dsi_dcs_read_lcm_reg_v2 = DSI_dcs_read_lcm_reg_v2,
};



static inline LCD_IF_WIDTH to_lcd_if_width(LCM_DBI_DATA_WIDTH data_width)
{
	switch (data_width) {
	case LCM_DBI_DATA_WIDTH_8BITS:
		return LCD_IF_WIDTH_8_BITS;
	case LCM_DBI_DATA_WIDTH_9BITS:
		return LCD_IF_WIDTH_9_BITS;
	case LCM_DBI_DATA_WIDTH_16BITS:
		return LCD_IF_WIDTH_16_BITS;
	case LCM_DBI_DATA_WIDTH_18BITS:
		return LCD_IF_WIDTH_18_BITS;
	case LCM_DBI_DATA_WIDTH_24BITS:
		return LCD_IF_WIDTH_24_BITS;
	default:
		ASSERT(0);
	}
	return LCD_IF_WIDTH_18_BITS;
}

int disphal_get_fb_alignment(void)
{
	return MTK_FB_ALIGNMENT;
}

int disphal_init_ctrl_if(void)
{
	const LCM_DBI_PARAMS *dbi = NULL;

	if (lcm_params == NULL)
		return -1;

	dbi = &(lcm_params->dbi);
	switch (lcm_params->ctrl) {
	case LCM_CTRL_NONE:
	case LCM_CTRL_GPIO:
		return 0;

	case LCM_CTRL_SERIAL_DBI:
		ASSERT(dbi->port <= 1);
		LCD_CHECK_RET(LCD_Init());
		ctrl_if = LCD_IF_SERIAL_0 + dbi->port;
#if (MTK_LCD_HW_SIF_VERSION == 1)
		LCD_ConfigSerialIF(ctrl_if,
				   (LCD_IF_SERIAL_BITS) dbi->data_width,
				   dbi->serial.clk_polarity,
				   dbi->serial.clk_phase,
				   dbi->serial.cs_polarity,
				   dbi->serial.clock_base,
				   dbi->serial.clock_div, dbi->serial.is_non_dbi_mode);
#else				/* /(MTK_LCD_HW_SIF_VERSION == 2) */
		LCD_ConfigSerialIF(ctrl_if,
				   (LCD_IF_SERIAL_BITS) dbi->data_width,
				   dbi->serial.sif_3wire,
				   dbi->serial.sif_sdi,
				   dbi->serial.sif_1st_pol,
				   dbi->serial.sif_sck_def,
				   dbi->serial.sif_div2,
				   dbi->serial.sif_hw_cs,
				   dbi->serial.css,
				   dbi->serial.csh,
				   dbi->serial.rd_1st,
				   dbi->serial.rd_2nd, dbi->serial.wr_1st, dbi->serial.wr_2nd);
#endif
		break;

	case LCM_CTRL_PARALLEL_DBI:
		ASSERT(dbi->port <= 2);
		LCD_CHECK_RET(LCD_Init());
		ctrl_if = LCD_IF_PARALLEL_0 + dbi->port;
		LCD_ConfigParallelIF(ctrl_if,
				     (LCD_IF_PARALLEL_BITS) dbi->data_width,
				     (LCD_IF_PARALLEL_CLK_DIV) dbi->clock_freq,
				     dbi->parallel.write_setup,
				     dbi->parallel.write_hold,
				     dbi->parallel.write_wait,
				     dbi->parallel.read_setup,
				     dbi->parallel.read_hold,
				     dbi->parallel.read_latency,
				     dbi->parallel.wait_period, dbi->parallel.cs_high_width);
		break;

	default:
		ASSERT(0);
	}

	LCD_CHECK_RET(LCD_SelectWriteIF(ctrl_if));

	LCD_CHECK_RET(LCD_ConfigIfFormat(dbi->data_format.color_order,
					 dbi->data_format.trans_seq,
					 dbi->data_format.padding,
					 dbi->data_format.format,
					 to_lcd_if_width(dbi->data_format.width)));
	return 0;
}

int disphal_wait_not_busy(void)
{
	if (lcm_params->type == LCM_TYPE_DBI)
		LCD_WaitForNotBusy();
	else if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
		DSI_WaitForNotBusy();
	return 0;
}

int disphal_panel_enable(const LCM_DRIVER *lcm_drv, struct mutex *pLcmCmdMutex, bool enable)
{
	if (enable) {
		if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
			DSI_SetMode(CMD_MODE);

		mutex_lock(pLcmCmdMutex);
		lcm_drv->resume();

		if (lcm_drv->check_status)
			lcm_drv->check_status();

		DSI_LP_Reset();
		mutex_unlock(pLcmCmdMutex);

		if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) {
			DSI_WaitForNotBusy();
			DSI_SetMode(lcm_params->dsi.mode);
		}
	} else {
		LCD_CHECK_RET(LCD_WaitForNotBusy());
		if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
			DSI_CHECK_RET(DSI_WaitForNotBusy());

		if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) {
			DPI_CHECK_RET(DPI_DisableClk());
			/* msleep(200); */
			DSI_Reset();
			DSI_clk_HS_mode(0);
			DSI_SetMode(CMD_MODE);
		}

		mutex_lock(pLcmCmdMutex);
		lcm_drv->suspend();
		mutex_unlock(pLcmCmdMutex);
	}
	return 0;
}

int disphal_update_screen(const LCM_DRIVER *lcm_drv,
			  struct mutex *pLcmCmdMutex, uint32_t x, uint32_t y,
			  uint32_t width, uint32_t height)
{
	if (lcm_drv->update) {
		mutex_lock(pLcmCmdMutex);
		lcm_drv->update(x, y, width, height);
		DSI_LP_Reset();
		mutex_unlock(pLcmCmdMutex);
	}
	return 0;
}

int disphal_set_backlight(const LCM_DRIVER *lcm_drv, struct mutex *pLcmCmdMutex, uint32_t level)
{
/* if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) */
/* DSI_SetMode(CMD_MODE); */

	mutex_lock(pLcmCmdMutex);
	lcm_drv->set_backlight(level);
/* DSI_LP_Reset(); */
	mutex_unlock(pLcmCmdMutex);

/* if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) */
/* DSI_SetMode(lcm_params->dsi.mode); */

	return 0;
}

int disphal_set_backlight_mode(const LCM_DRIVER *lcm_drv, struct mutex *pLcmCmdMutex,
			       uint32_t mode)
{
/* if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) */
/* DSI_SetMode(CMD_MODE); */

	mutex_lock(pLcmCmdMutex);
	lcm_drv->set_backlight_mode(mode);
/* DSI_LP_Reset(); */
	mutex_unlock(pLcmCmdMutex);

/* if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE) */
/* DSI_SetMode(lcm_params->dsi.mode); */

	return 0;
}

int disphal_set_pwm(const LCM_DRIVER *lcm_drv, struct mutex *pLcmCmdMutex, uint32_t divider)
{
	mutex_lock(pLcmCmdMutex);
	lcm_drv->set_pwm(divider);
/* DSI_LP_Reset(); */
	mutex_unlock(pLcmCmdMutex);
	return 0;
}

int disphal_get_pwm(const LCM_DRIVER *lcm_drv, struct mutex *pLcmCmdMutex,
		    uint32_t divider, unsigned int *freq)
{
	mutex_lock(pLcmCmdMutex);
	*freq = lcm_drv->get_pwm(divider);
	mutex_unlock(pLcmCmdMutex);
	return 0;
}

DISP_STATUS disphal_fm_desense_query(void)
{
	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		return (DISP_STATUS) LCD_FMDesense_Query();
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		return (DISP_STATUS) DPI_FMDesense_Query();
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		return (DISP_STATUS) DSI_FMDesense_Query();
	}
	return DISP_STATUS_ERROR;
}

DISP_STATUS disphal_fm_desense(unsigned long freq)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		LCD_CHECK_RET(LCD_FM_Desense(ctrl_if, freq));
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		DPI_CHECK_RET(DPI_FM_Desense(freq));
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		DSI_CHECK_RET(DSI_FM_Desense(freq));
	} else {
		ret = DISP_STATUS_ERROR;
	}
	return ret;
}

DISP_STATUS disphal_reset_update(void)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		LCD_CHECK_RET(LCD_Reset_WriteCycle(ctrl_if));
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		DPI_CHECK_RET(DPI_Reset_CLK());
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		DSI_CHECK_RET(DSI_Reset_CLK());
	} else {
		ret = DISP_STATUS_ERROR;
	}
	return ret;
}

DISP_STATUS disphal_get_default_updatespeed(unsigned int *speed)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		LCD_CHECK_RET(LCD_Get_Default_WriteCycle(ctrl_if, speed));
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		DPI_CHECK_RET(DPI_Get_Default_CLK(speed));
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		DSI_CHECK_RET(DSI_Get_Default_CLK(speed));
	} else {
		ret = DISP_STATUS_ERROR;
	}
	return ret;
}

DISP_STATUS disphal_get_current_updatespeed(unsigned int *speed)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		LCD_CHECK_RET(LCD_Get_Current_WriteCycle(ctrl_if, speed));
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		DPI_CHECK_RET(DPI_Get_Current_CLK(speed));
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		DSI_CHECK_RET(DSI_Get_Current_CLK(speed));
	} else {
		ret = DISP_STATUS_ERROR;
	}
	return ret;
}

DISP_STATUS disphal_change_updatespeed(unsigned int speed)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
		LCD_CHECK_RET(LCD_Change_WriteCycle(ctrl_if, speed));
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
		DPI_CHECK_RET(DPI_Change_CLK(speed));
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* DSI */
		DSI_CHECK_RET(DSI_Change_CLK(speed));
	} else {
		ret = DISP_STATUS_ERROR;
	}
	return ret;
}

int disphal_prepare_suspend(void)
{
	if (lcm_params->type == LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		DSI_SetMode(CMD_MODE);

	return 0;
}

LCM_DRIVER *disphal_get_lcm_driver(const char *lcm_name, unsigned int *lcm_index)
{
	LCM_DRIVER *lcm = NULL;
	bool isLCMFound = false;

	DISP_MSG("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);
	DISP_MSG("[LCM Auto Detect], try to find driver for [%s]\n",
		 (lcm_name == NULL) ? "unknown" : lcm_name);

	if (lcm_count == 1) {
		/* we need to verify whether the lcm is connected */
		/* even there is only one lcm type defined */
		lcm = lcm_driver_list[0];
		lcm->set_util_funcs(&lcm_utils);
		*lcm_index = 0;
		pr_notice("[DISP/DDP]LCM: %s\n", (lcm->name == NULL) ? "unknown" : lcm->name);
		isLCMFound = true;
		goto done;
	} else {
		int i;

		for (i = 0; i < lcm_count; i++) {
			lcm = lcm_driver_list[i];
			DISP_MSG("[LCM Auto Detect] [%d] - [%s]\t", i,
				 (lcm->name == NULL) ? "unknown" : lcm->name);
			lcm->set_util_funcs(&lcm_utils);
			memset((void *)&s_lcm_params, 0, sizeof(LCM_PARAMS));
			lcm->get_params(&s_lcm_params);

			disphal_init_ctrl_if();
			LCD_Set_DrivingCurrent(&s_lcm_params);
			LCD_Init_IO_pad(&s_lcm_params);

			if (lcm_name != NULL) {
				if (!strcmp(lcm_name, lcm->name)) {
					pr_info("\t\t[success]\n");
					*lcm_index = i;
					isLCMFound = true;
					goto done;
				} else {
					pr_info("\t\t[fail]\n");
				}
			} else {
				if (LCM_TYPE_DSI == lcm_params->type)
					init_dsi(false);


				if (lcm->compare_id != NULL && lcm->compare_id()) {
					pr_info("\t\t[success]\n");
					isLCMFound = true;
					*lcm_index = i;
					goto done;
				} else {
					if (LCM_TYPE_DSI == lcm_params->type)
						DSI_Deinit();
					pr_info("\t\t[fail]\n");
				}
			}
		}
	}
done:
	if (isLCMFound) {
		memset((void *)&s_lcm_params, 0, sizeof(LCM_PARAMS));
		lcm->get_params(&s_lcm_params);
		return lcm;
	} else
		return NULL;
}

int disphal_register_event(char *event_name, DISPHAL_EVENT_HANDLER event_handler)
{
	DISP_INTERRUPT_CALLBACK_STRUCT cb;

	cb.pFunc = event_handler;
	if (strcmp(event_name, "DISP_CmdDone") == 0) {
		if (LCM_TYPE_DBI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_LCD_TRANSFER_COMPLETE_INT, &cb);
		else if ((LCM_TYPE_DSI == lcm_params->type) && (CMD_MODE == lcm_params->dsi.mode))
			DISP_SetInterruptCallback(DISP_DSI_CMD_DONE_INT, &cb);
	} else if (strcmp(event_name, "DISP_RegUpdate") == 0) {
		if (LCM_TYPE_DBI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_LCD_REG_COMPLETE_INT, &cb);
		else if (LCM_TYPE_DPI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DPI_REG_UPDATE_INT, &cb);
		else if (LCM_TYPE_DSI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DSI_REG_UPDATE_INT, &cb);
	} else if (strcmp(event_name, "DISP_VSync") == 0) {
		if (LCM_TYPE_DPI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DPI_VSYNC_INT, &cb);
		else if ((LCM_TYPE_DSI == lcm_params->type) && (CMD_MODE != lcm_params->dsi.mode))
			DISP_SetInterruptCallback(DISP_DSI_VSYNC_INT, &cb);
	} else if (strcmp(event_name, "DISP_TargetLine") == 0) {
		if (LCM_TYPE_DPI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DPI_TARGET_LINE_INT, &cb);
		else if ((LCM_TYPE_DSI == lcm_params->type) && (CMD_MODE != lcm_params->dsi.mode))
			DISP_SetInterruptCallback(DISP_DSI_TARGET_LINE_INT, &cb);
	} else if (strcmp(event_name, "DISP_HWDone") == 0) {
		if ((LCM_TYPE_DSI == lcm_params->type) && (CMD_MODE != lcm_params->dsi.mode))
			DISP_SetInterruptCallback(DISP_DSI_VMDONE_INT, &cb);
	} else if (strcmp(event_name, "DISP_ScrUpdStart") == 0) {
		if (LCM_TYPE_DSI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DSI_SCREEN_UPDATE_START_INT, &cb);
		else if (LCM_TYPE_DPI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DPI_SCREEN_UPDATE_START_INT, &cb);

	} else if (strcmp(event_name, "DISP_ScrUpdEnd") == 0) {
		if (LCM_TYPE_DSI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DSI_SCREEN_UPDATE_END_INT, &cb);
		else if (LCM_TYPE_DPI == lcm_params->type)
			DISP_SetInterruptCallback(DISP_DPI_SCREEN_UPDATE_END_INT, &cb);

	}

	return 0;
}

int disphal_enable_te(bool enable)
{
	LCD_TE_Enable(enable);
	return 0;
}

int disphal_pm_restore_noirq(struct device *device)
{
#ifndef CONFIG_OF
	/* DPI0 */
	mt_irq_set_sens(disp_dev.irq[DISP_REG_DPI0], MT_LEVEL_SENSITIVE);
	mt_irq_set_polarity(disp_dev.irq[DISP_REG_DPI0], MT_POLARITY_LOW);

	/* DSI */
	mt_irq_set_sens(disp_dev.irq[DISP_REG_DSI], MT_LEVEL_SENSITIVE);
	mt_irq_set_polarity(disp_dev.irq[DISP_REG_DSI], MT_POLARITY_LOW);
#endif
	return 0;
}

int disphal_enable_mmu(bool enable)
{
	disp_use_mmu = enable;
	return 0;
}

int disphal_allocate_fb(struct resource *res, unsigned int *pa,
			unsigned int *va, unsigned int *dma_pa)
{
	unsigned int size = res->end - res->start + 1;

#ifdef CONFIG_MTK_M4U

	int ret = 0;
	/* m4u_alloc_mva(M4U_CLNTMOD_LCDC_UI, res->start, size, 0, 0, dma_pa); */
	ret = m4u_fill_linear_pagetable(res->start, size);
	if (ret)
		pr_info("fill_linear_pagetable error, %d\n", ret);
	*pa = res->start;
	*va = (unsigned int)ioremap_nocache(res->start, size);
	*dma_pa = *pa;
	DISP_MSG("display use M4U\n");

#elif defined(CONFIG_MTK_IOMMU)

	dma_addr_t handle_pa;
	void *va_addr;
	struct dma_iommu_mapping *dma_mapping;

	va_addr = dma_alloc_coherent(disp_dev.dev_use_imu, size, &handle_pa,
				     GFP_KERNEL /* | DMA_ATTR_FORCE_CONTIGUOUS */);

	DISP_MSG("[DISPHAL] dma_alloc_coherent:0x%x va:0x%p size:0x%x\n",
		 (unsigned int)handle_pa, va_addr, size);

	*va = (unsigned int)va_addr;
	/* can not use __pa, why? */
	/**pa = (unsigned int)__pa(va_addr);*/
	dma_mapping = disp_dev.pimudev->dev.archdata.iommu;
	*pa = (unsigned int)iommu_iova_to_phys(dma_mapping->domain, handle_pa);
	DISP_MSG("[DISPHAL] iommu_iova_to_phys pa:0x%x __pa(va):0x%x\n", *pa, __pa(va_addr));
	*dma_pa = (unsigned int)handle_pa;
	DISP_MSG("display use IOMMU\n");

#else

	unsigned int addr;
	unsigned int page_count;
	unsigned int i = 0;
	struct page **pages;

	page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	pages = kmalloc((sizeof(struct page *) * page_count), GFP_KERNEL);
	DISP_MSG("page count: %d\n", page_count);
	for (i = 0; i < page_count; i++) {
		addr = res->start + i * PAGE_SIZE;
		pages[i] = pfn_to_page(addr >> PAGE_SHIFT);
	}

	*va = (unsigned int)vmap(pages, page_count, VM_MAP, PAGE_KERNEL);
	/*kfree(pages); */
	*pa = res->start;
	*dma_pa = *pa;

	/* disp_path_stop_access_memory(); */
	DISP_MSG("display not use MMU va is 0x%08X; size is 0x%08X\n", *va, size);
#endif

	ASSERT(dma_pa);
	pr_err("[DISPHAL] FB MVA is 0x%08X PA is 0x%08X\n", *dma_pa, *pa);

	return 0;
}

int disphal_map_overlay_out_buffer(unsigned int va, unsigned int size, unsigned int *dma_pa)
{
#ifdef CONFIG_MTK_M4U
	int ret;

	ret = m4u_alloc_mva(DISP_WDMA, va, size, 0, 0, dma_pa);
	if (ret != 0)
		return ret;

	m4u_dma_cache_maint(DISP_WDMA, (const void *)va, size, DMA_BIDIRECTIONAL);
#endif
	return 0;
}

int disphal_unmap_overlay_out_buffer(unsigned int va, unsigned int size, unsigned int dma_pa)
{
#ifdef CONFIG_MTK_M4U
	m4u_dealloc_mva(DISP_WDMA, va, size, dma_pa);
#endif
	return 0;
}

int disphal_sync_overlay_out_buffer(unsigned int va, unsigned int size)
{
#ifdef CONFIG_MTK_M4U
	m4u_dma_cache_maint(DISP_WDMA, (const void *)va, size, DMA_BIDIRECTIONAL);
#endif
	return 0;
}

int disphal_dma_map_kernel(unsigned int dma_pa, unsigned int size, unsigned int *kva,
			   unsigned int *mapsize)
{
#ifdef CONFIG_MTK_M4U
	m4u_mva_map_kernel(dma_pa, size, 0, kva, mapsize);
#else
	kva = ioremap_nocache(dma_pa, size);
	*mapsize = size;
#endif
	return 0;
}

int disphal_dma_unmap_kernel(unsigned int dma_pa, unsigned int size, unsigned int kva)
{
#ifdef CONFIG_MTK_M4U
	m4u_mva_unmap_kernel(dma_pa, size, kva);
#else
	iounmap((void *)dma_pa);
#endif
	return 0;
}

int disphal_init_overlay_to_memory(void)
{
#ifdef CONFIG_MTK_M4U
	M4U_PORT_STRUCT portStruct;

	portStruct.ePortID = DISP_WDMA;	/* hardware port ID, defined in M4U_PORT_ID_ENUM */
	portStruct.Virtuality = 1;
	portStruct.Security = 0;
	portStruct.domain = 0;	/* domain : 0 1 2 3 */
	portStruct.Distance = 1;
	portStruct.Direction = 0;
	m4u_config_port(&portStruct);
#endif
	disp_module_clock_on(DISP_MODULE_WDMA, "disphal overlay2mem");
	return 0;
}

int disphal_deinit_overlay_to_memory(void)
{
#ifdef CONFIG_MTK_M4U
	M4U_PORT_STRUCT portStruct;

	portStruct.ePortID = DISP_WDMA;	/* hardware port ID, defined in M4U_PORT_ID_ENUM */
	portStruct.Virtuality = 1;
	portStruct.Security = 0;
	portStruct.domain = 0;	/* domain : 0 1 2 3 */
	portStruct.Distance = 1;
	portStruct.Direction = 0;
	m4u_config_port(&portStruct);
#endif
	disp_module_clock_off(DISP_MODULE_WDMA, "disphal overlay2mem");
	return 0;
}

unsigned int disphal_bls_query(void)
{
	return DSI_BLS_Query();
}

void disphal_bls_enable(bool enable)
{
	DSI_BLS_Enable(enable);
}

unsigned int disphal_check_lcm(uint32_t color)
{
	unsigned int ret = 0;

	if (LCM_TYPE_DBI == lcm_params->type) {	/* DBI */
	} else if (LCM_TYPE_DPI == lcm_params->type) {	/* DPI */
	} else if (LCM_TYPE_DSI == lcm_params->type) {	/* dsi */
		ret = DSI_Check_LCM(color);
		if (lcm_params->dsi.mode != CMD_MODE) {
			DSI_SetMode(lcm_params->dsi.mode);
			DSI_clk_HS_mode(1);
			DSI_CHECK_RET(DSI_StartTransfer(false));
		}
	} else {
		pr_info("DISP_AutoTest():unknown interface\n");
		ret = 0;
	}
	return ret;
}


void fbconfig_disp_set_te_enable(char enable)
{
	bool en = (bool) enable;

	DSI_TE_Enable(en);
}

void fbconfig_disp_set_continuous_clock(int enable)
{
	fbconfig_DSI_Continuous_HS(enable);
}

/* This part is for Display Customization Tool Implementation**************** */

static void disphal_fbconfig_dsi_set_cmd_mode(void)
{
	fbconfig_set_cmd_mode();
	DSI_clk_HS_mode(0);

}

static int disphal_fbconfig_mipi_clk_set(unsigned int clk)
{
	int ret = DSI_STATUS_ERROR;

	ret = fbconfig_DSI_set_CLK(clk);
	return ret;
}

static void disphal_fbconfig_dsi_post(void)
{

	fbconfig_set_vdo_mode();
/* needStartDSI =  true;  //need ??xk.shi */
	DSI_StartTransfer(true);
}

static void disphal_fbconfig_mipi_lane_set(unsigned int lane_num)
{
	fbconfig_DSI_set_lane_num(lane_num);
}

static void disphal_fbconfig_mipi_timing_set(MIPI_TIMING timing)
{
	fbconfig_DSI_set_timing(timing);
}

static void disphal_fbconfig_disp_set_te_enable(char enable)
{
	bool en = (bool) enable;

	DSI_TE_Enable(en);
}

static void disphal_fbconfig_disp_set_continuous_clock(int enable)
{
	fbconfig_DSI_Continuous_HS(enable);
}

static int disphal_fbconfig_DSI_set_get_misc(const char *name, void *parameter)
{
	LCM_REG_READ *lcm_read = (LCM_REG_READ *) parameter;

	if (!strcmp(name, "reg_read")) {
		fbconfig_dsi_dcs_read_lcm_reg_v2(lcm_read->check_addr, lcm_read->check_type,
						 lcm_read->check_buffer, lcm_read->check_para_num);
		return 0;	/* return 0 ?? is ok ? */
	} else
		return -1;
}

static int disphal_fbconfig_DSI_set_Fre_Spread(unsigned int fre)
{
	int ret = 0;

	if ((fre > 0) && (fre < 9)) {
		int ret = DSI_STATUS_ERROR;

		ret = fbconfig_DSI_set_SSC(fre);
		return ret;
	}

	switch (fre) {
	case 9:
		ret = fbconfig_get_dsi_ssc();
		break;
	case 10:
		ret = fbconfig_get_Continuous_status();
		break;
	case 11:
		ret = fbconfig_get_dsi_CLK();
		break;
	case 12:
		ret = fbconfig_get_dsi_lane_num();
		break;
	case 13:
		ret = fbconfig_get_TE_enable();
		break;
	default:
		break;
	}
	if ((fre >= 100) && (fre < 355))
		ret = fbconfig_get_dsi_timing(fre - 100);
	return ret;

}

const FBCONFIG_DISP_IF *disphal_fbconfig_get_if(void)
{
	static const FBCONFIG_DISP_IF FBCONFIG_DISP_DRV = {
		.set_cmd_mode = disphal_fbconfig_dsi_set_cmd_mode,
		.set_mipi_clk = disphal_fbconfig_mipi_clk_set,
		.set_dsi_post = disphal_fbconfig_dsi_post,
		.set_lane_num = disphal_fbconfig_mipi_lane_set,
		.set_mipi_timing = disphal_fbconfig_mipi_timing_set,
		.set_te_enable = disphal_fbconfig_disp_set_te_enable,
		.set_continuous_clock = disphal_fbconfig_disp_set_continuous_clock,
		.set_spread_frequency = disphal_fbconfig_DSI_set_Fre_Spread,
		.set_get_misc = disphal_fbconfig_DSI_set_get_misc,
	};

	return &FBCONFIG_DISP_DRV;
}
