#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <asm-generic/gpio.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#endif
#endif
#include "lcm_drv.h"
#include <mt-plat/mt_gpio.h>

extern unsigned int GPIO_IT6151_RST; //EDP_RESET 
extern unsigned int GPIO_LCM_PWR;    //LCM_POWER_EN 3.3V
extern unsigned int GPIO_IT6151_1V8; //EDP_POWER_EN 1.8/1.2V
extern unsigned int GPIO_IT6151_BL;  //EDP_ENPSR
extern unsigned int GPIO_IT6151_STB; //EDP_STANDBY
extern unsigned int GPIO_IT6151_INT; //EDP_INT
extern unsigned int GPIO_VLED_EN;	//VLED_EN
//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------
//#define IT6151_DEBUG
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FHD_1920_1080
//#define FHD_1366_768

#ifdef FHD_1920_1080
#define FRAME_WIDTH  (1920)
#define FRAME_HEIGHT (1080)
#define PANEL_RESOLUTION_1920x1080p60
#endif

#ifdef FHD_1366_768
#define FRAME_WIDTH  (1368)
#define FRAME_HEIGHT (768)
#define PANEL_RESOLUTION_1366x768
#endif
//---------------------------------------------------------------------------
//LocalVariables
//---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = 
{
    .set_reset_pin = NULL,
    .udelay = NULL,
    .mdelay = NULL,
};

typedef struct
{
  u8 dev_addr;   
  u8 addr;
  u8 data;
}it6151_setting_table;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define TRUE 1
#define FALSE 0
//---------------------------------------------------------------------------
//LocalFunctions
//---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                                                      lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                           lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#define LCM_DSI_CMD_MODE    0

#define DP_I2C_ADDR     (0x5C << 0)
#define MIPI_I2C_ADDR   (0x6C << 0)
#define REGFLAG_DELAY   (0xAB)

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
        gpio_direction_output(GPIO, output);
        gpio_set_value(GPIO, output);
}

static void lcd_reset(unsigned char enabled)
{
    if(enabled)
    {
        lcm_set_gpio_output(GPIO_IT6151_RST,GPIO_OUT_ONE);//reset
    }
    else
    {
        lcm_set_gpio_output(GPIO_IT6151_RST,0);//reset
	}
}

static void lcd_3v3(unsigned char enabled)
{
    if(enabled)
    {
        lcm_set_gpio_output(GPIO_LCM_PWR,GPIO_OUT_ONE);//reset
    }
    else
    {
        lcm_set_gpio_output(GPIO_LCM_PWR,0);//reset
    }
}
static void lcd_1v8(unsigned char enabled)
{
    if(enabled)
    {
        lcm_set_gpio_output(GPIO_IT6151_1V8,GPIO_OUT_ONE);//reset
    }
    else
    {
        lcm_set_gpio_output(GPIO_IT6151_1V8,0);//reset
    }
}

static void lcd_bl(unsigned char enabled)
{
    if (enabled)
    {
        lcm_set_gpio_output(GPIO_IT6151_BL, GPIO_OUT_ONE);
        MDELAY(20);
    }
    else
    {   
        lcm_set_gpio_output(GPIO_IT6151_BL, 0);
        MDELAY(20);       
    }  
}

static void lcd_stb(unsigned char enabled)
{
    if (enabled)
    {
        lcm_set_gpio_output(GPIO_IT6151_STB, GPIO_OUT_ONE);
        MDELAY(20);
    }
    else
    {   
        lcm_set_gpio_output(GPIO_IT6151_STB, 0);
        MDELAY(20);       
    }  
}

#ifdef BUILD_LK
#define IT6151_BUSNUM   I2C0

static u32 it6151_i2c_write_byte(u8 dev_addr,u8 addr, u8 data)
{
    u32 ret_code = I2C_OK;
    u8 write_data[I2C_FIFO_SIZE], len;
    struct mt_i2c_t i2c;
    
    i2c.id = IT6151_BUSNUM;
    i2c.addr = dev_addr;
    i2c.mode = ST_MODE;
    i2c.speed = 100;

    write_data[0]= addr;
    write_data[1] = data;
    len = 2;

    #ifdef IT6151_DEBUG

    printf("[it6151_i2c_write] dev_addr = 0x%x, write_data[0x%x] = 0x%x \n", dev_addr, write_data[0], write_data[1]);
    #endif
    
    ret_code = i2c_write(&i2c, write_data, len);

    return ret_code;
}

static u32 it6151_i2c_read_byte(u8 dev_addr,u8 addr, u8 *dataBuffer)
{
    u32 ret_code = I2C_OK;
    u8 len;
    struct mt_i2c_t i2c;
    
    *dataBuffer = addr;

    i2c.id = IT6151_BUSNUM;
    i2c.addr = dev_addr;
    i2c.mode = ST_MODE;
    i2c.speed = 100;
    len = 1;

    ret_code = i2c_write_read(&i2c, dataBuffer, len, len);

    #ifdef IT6151_DEBUG
    /* dump write_data for check */
    printf("[it6151_read_byte] dev_addr = 0x%x, read_data[0x%x] = 0x%x \n", dev_addr, addr, *dataBuffer);
    #endif
    
    return ret_code;
}
 
 /******************************************************************************
 *IIC drvier,:protocol type 2 add by chenguangjian end
 ******************************************************************************/
#else
extern int it6151_i2c_read_byte(u8 dev_addr, u8 addr, u8 *returnData);
extern int it6151_i2c_write_byte(u8 dev_addr, u8 addr, u8 writeData);
#endif
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
///       for it6151 defines start                   ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////


//#define PANEL_RESOLUTION_1280x800_NOUFO
//#define PANEL_RESOLUTION_2048x1536_NOUFO_18B
//#define PANEL_RESOLUTION_2048x1536
// #define PANEL_RESOLUTION_2048x1536_NOUFO // FOR INTEL Platform
// #define PANEL_RESOLUTION_1920x1200p60RB
//#define PANEL_RESOLUTION_1920x1080p60
//#define PANEL_RESULUTION_1536x2048

#define MIPI_4_LANE     (3)
#define MIPI_3_LANE     (2)
#define MIPI_2_LANE     (1)
#define MIPI_1_LANE     (0)

// MIPI Packed Pixel Stream
#define RGB_24b         (0x3E)
#define RGB_30b         (0x0D)
#define RGB_36b         (0x1D)
#define RGB_18b_P       (0x1E)
#define RGB_18b_L       (0x2E)
#define YCbCr_16b       (0x2C)
#define YCbCr_20b       (0x0C)
#define YCbCr_24b       (0x1C)

// DPTX reg62[3:0]
#define B_DPTXIN_6Bpp   (0)
#define B_DPTXIN_8Bpp   (1)
#define B_DPTXIN_10Bpp  (2)
#define B_DPTXIN_12Bpp  (3)

#define B_LBR           (1)
#define B_HBR           (0)

#define B_4_LANE        (3)
#define B_2_LANE        (1)
#define B_1_LANE        (0)

#define B_SSC_ENABLE    (1)
#define B_SSC_DISABLE   (0)

///////////////////////////////////////////////////////////////////////////
//CONFIGURE
///////////////////////////////////////////////////////////////////////////
#define TRAINING_BITRATE    (B_HBR)//(B_LBR)
#define DPTX_SSC_SETTING    (B_SSC_ENABLE)//(B_SSC_DISABLE)
#define HIGH_PCLK           (1)
#define MP_MCLK_INV         (1)
#define MP_CONTINUOUS_CLK   (1)
#define MP_LANE_DESKEW      (1)
#define MP_PCLK_DIV         (2)
#define MP_LANE_SWAP        (0)
#define MP_PN_SWAP          (0)

#define DP_PN_SWAP          (0)
#define DP_AUX_PN_SWAP      (0)
#define DP_LANE_SWAP        (0) //(0) our convert board need to LANE SWAP for data lane
#define FRAME_RESYNC        (0)
#define LVDS_LANE_SWAP      (0)
#define LVDS_PN_SWAP        (0)
#define LVDS_DC_BALANCE     (0)

#define LVDS_6BIT           (0) // '0' for 8 bit, '1' for 6 bit
#define VESA_MAP            (1) // '0' for JEIDA , '1' for VESA MAP

#define INT_MASK            (3)
#define MIPI_INT_MASK       (0)
#define TIMER_CNT           (0x0A)
///////////////////////////////////////////////////////////////////////
// Global Setting
///////////////////////////////////////////////////////////////////////
#ifdef PANEL_RESOLUTION_1280x800_NOUFO
#define PANEL_WIDTH 1280
#define VIC 0
#define MP_HPOL 0
#define MP_VPOL 1
#define DPTX_LANE_COUNT  B_2_LANE
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif

#ifdef PANEL_RESOLUTION_1920x1080p60
#define PANEL_WIDTH 1920
#define VIC 0x10
#define MP_HPOL 1
#define MP_VPOL 1
#define DPTX_LANE_COUNT  B_2_LANE
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp // B_DPTXIN_8Bpp // B_DPTXIN_6Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_24b // RGB_24b // RGB_18b_L
#define MP_H_RESYNC     1 //    1
#define MP_V_RESYNC     0 //0
#endif

#ifdef PANEL_RESOLUTION_1920x1200p60RB
#define PANEL_WIDTH 1920
#define VIC 0 // non-Zero value for CEA setting, check the given input format.
#define MP_HPOL 1
#define MP_VPOL 0
#define DPTX_LANE_COUNT  B_2_LANE
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif

#ifdef PANEL_RESOLUTION_2048x1536
#define PANEL_WIDTH 2048
#define VIC 0 // non-Zero value for CEA setting, check the given input format.
#define MP_HPOL 0
#define MP_VPOL 1
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_LANE_COUNT  B_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 1
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         0
#define MP_V_RESYNC         0
#endif

#ifdef PANEL_RESOLUTION_2048x1536_NOUFO
#define PANEL_WIDTH 2048
#define VIC 0 // non-Zero value for CEA setting, check the given input format.
#define MP_HPOL 0
#define MP_VPOL 1
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_LANE_COUNT  B_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif

#ifdef PANEL_RESOLUTION_2048x1536_NOUFO_18B
#define PANEL_WIDTH 2048
#define VIC 0 // non-Zero value for CEA setting, check the given input format.
#define MP_HPOL 0
#define MP_VPOL 1
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_LANE_COUNT  B_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_18b_P
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif

#ifdef PANEL_RESULUTION_1536x2048
#define PANEL_WIDTH 1536
#define VIC 0 // non-Zero value for CEA setting, check the given input format.
#define MP_HPOL 0
#define MP_VPOL 1
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_LANE_COUNT  B_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 1
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif
#ifdef PANEL_RESOLUTION_1366x768
#define PANEL_WIDTH 1368
#define VIC 0x10
#define MP_HPOL 1
#define MP_VPOL 1
#define DPTX_LANE_COUNT  B_1_LANE
#define MIPI_LANE_COUNT  MIPI_4_LANE
#define DPTX_BPP         B_DPTXIN_8Bpp
#define EN_UFO 0
#define MIPI_PACKED_FMT     RGB_24b
#define MP_H_RESYNC         1
#define MP_V_RESYNC         0
#endif

///////////////////////////////////////////////////////////////////////////

//#define DP_I2C_ADDR 0x5C
//#define MIPI_I2C_ADDR 0x6C

/////////////////////////////////////////////////////////////////////
///       for it6151 defines end                   /////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Function
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void IT6151_DPTX_init(void)
    {   
#ifndef BUILD_LK
    printk("[KERNEL/LCM] IT6151_DPTX_init !!!\n");
#else
    printf("[LK/LCM] IT6151_DPTX_init\n");
#endif  
    it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x29);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x00);
    
    it6151_i2c_write_byte(DP_I2C_ADDR,0x09,INT_MASK);// Enable HPD_IRQ,HPD_CHG,VIDSTABLE
    it6151_i2c_write_byte(DP_I2C_ADDR,0x0A,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x0B,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xC5,0xC1);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xB5,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xB7,0x80);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xC4,0xF0);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x06,0xFF);// Clear all interrupt
    it6151_i2c_write_byte(DP_I2C_ADDR,0x07,0xFF);// Clear all interrupt
    it6151_i2c_write_byte(DP_I2C_ADDR,0x08,0xFF);// Clear all interrupt
    
    it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x0c,0x08);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x21,0x05);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x3a,0x04);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x5f,0x06);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xc9,0xf5);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xca,0x4c);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xcb,0x37);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xce,0x80);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xd3,0x03);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xd4,0x60);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xe8,0x11);
    it6151_i2c_write_byte(DP_I2C_ADDR,0xec,VIC);
    MDELAY(5);          

    it6151_i2c_write_byte(DP_I2C_ADDR,0x62,DPTX_BPP);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x23,0x42);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x24,0x07);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x25,0x01);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x26,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x27,0x10);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x2B,0x05);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x23,0x40);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x22,(DP_AUX_PN_SWAP<<3)|(DP_PN_SWAP<<2)|0x03);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x16,(DPTX_SSC_SETTING<<4)|(DP_LANE_SWAP<<3)|(DPTX_LANE_COUNT<<1)|TRAINING_BITRATE);
    //it6151_i2c_write_byte(DP_I2C_ADDR,0x62,0x00);  //rgb 6bit 
    it6151_i2c_write_byte(DP_I2C_ADDR,0x0f,0x01);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x76,0xa7);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x77,0xaf);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x7e,0x8f);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x7f,0x07);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x80,0xef);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x81,0x5f);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x82,0xef);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x83,0x07);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x88,0x38);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x89,0x1f);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x8a,0x48);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x0f,0x00);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x5c,0xf3);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x17,0x04);
    it6151_i2c_write_byte(DP_I2C_ADDR,0x17,0x01);
    MDELAY(5);  
}

int IT6151_init(void)
{
    unsigned char VenID[2], DevID[2], RevID;
	//u8 temp,i;

    printk("[KERNEL/LCM] IT6151_init\n");
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x00, &VenID[0]); 
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x01, &VenID[1]);
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x02, &DevID[0]);
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x03, &DevID[1]);
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x04, &RevID);
                
#ifndef BUILD_LK    
    printk("Current DPDevID=%02X%02X\n", DevID[1], DevID[0]);
    printk("Current DPVenID=%02X%02X\n", VenID[1], VenID[0]);
    printk("Current DPRevID=%02X\n\n", RevID);  
#endif
                
    if( VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x51 && DevID[1]==0x61 ){

#ifndef BUILD_LK    
        printk("[KERNEL/LCM] ===== qinrq ===== Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);
#else
        printf("[LK/LCM] ===== qinrq ===== Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);
#endif
        it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x04);// DP SW Reset
        it6151_i2c_write_byte(DP_I2C_ADDR,0xfd,(MIPI_I2C_ADDR<<1)|1);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x00);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x0c,(MP_LANE_SWAP<<7)|(MP_PN_SWAP<<6)|(MIPI_LANE_COUNT<<4)|EN_UFO);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x11,MP_MCLK_INV);

        if(RevID == 0xA1){          
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19, MP_LANE_DESKEW); 
        }else{
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19,(MP_CONTINUOUS_CLK<<1) | MP_LANE_DESKEW); 
        }
                
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x27, MIPI_PACKED_FMT);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x28,((PANEL_WIDTH/4-1)>>2)&0xC0);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x29,(PANEL_WIDTH/4-1)&0xFF);
        
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2e,0x34);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2f,0x01);
        
        
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4e,(MP_V_RESYNC<<3)|(MP_H_RESYNC<<2)|(MP_VPOL<<1)|(MP_HPOL));
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x80,(EN_UFO<<5)|MP_PCLK_DIV);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x84,0x8f);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x09,MIPI_INT_MASK);
        it6151_i2c_write_byte(MIPI_I2C_ADDR,0x92,TIMER_CNT);        
        IT6151_DPTX_init();

//test
/*
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x0d, &temp);
	printk(" ======= Reg0x0D = 0x%x \r\n",temp);
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x0e, &temp);
	printk(" ======= Reg0x0E = 0x%x \r\n",temp);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x0d, &temp);
	printk(" ======= Reg0x0D = 0x%x \r\n",temp);
	for(i=0x30;i<=0x43;i++){
		it6151_i2c_read_byte(MIPI_I2C_ADDR, i, &temp);
		printk(" ======= Reg0x%x = 0x%x \r\n",i,temp);
	}
	for(i=0x50;i<=0x57;i++){
		it6151_i2c_read_byte(MIPI_I2C_ADDR, i, &temp);
		printk(" ======= Reg0x%x = 0x%x \r\n",i,temp);
	}*/
//test end
        return 0;
    }

#ifndef BUILD_LK    
    printk(" Test 2 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);
#endif

    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x00, &VenID[0]);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x01, &VenID[1]);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x02, &DevID[0]);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x03, &DevID[1]);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x04, &RevID);

#ifndef BUILD_LK
    printk("Current MPDevID=%02X%02X\n", DevID[1], DevID[0]);
    printk("Current MPVenID=%02X%02X\n", VenID[1], VenID[0]);
    printk("Current MPRevID=%02X\n\n", RevID);
#endif
    if( VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x51 && DevID[1]==0x61 ){
    
#ifndef BUILD_LK    
            printk(" Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);
#else
            printf("[LK/LCM] Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x\n", DP_I2C_ADDR, MIPI_I2C_ADDR);
#endif
            it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x04);// DP SW Reset
            it6151_i2c_write_byte(DP_I2C_ADDR,0xfd,(MIPI_I2C_ADDR<<1)|1);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x00);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x0c,(MP_LANE_SWAP<<7)|(MP_PN_SWAP<<6)|(MIPI_LANE_COUNT<<4)|EN_UFO);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x11,MP_MCLK_INV);
    
            if(RevID == 0xA1){          
                it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19, MP_LANE_DESKEW); 
            }else{
                it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19,(MP_CONTINUOUS_CLK<<1) | MP_LANE_DESKEW); 
            }
                    
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x27, MIPI_PACKED_FMT);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x28,((PANEL_WIDTH/4-1)>>2)&0xC0);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x29,(PANEL_WIDTH/4-1)&0xFF);
            
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2e,0x34);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2f,0x01);
            
            
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4e,(MP_V_RESYNC<<3)|(MP_H_RESYNC<<2)|(MP_VPOL<<1)|(MP_HPOL));
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x80,(EN_UFO<<5)|MP_PCLK_DIV);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x84,0x8f);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x09,MIPI_INT_MASK);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x92,TIMER_CNT);        
            IT6151_DPTX_init();
    
            return 0;
        }

    if(VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x21 && DevID[1]==0x61 ){
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x33);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x40);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x00);
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x0c,(MP_LANE_SWAP<<7)|(MP_PN_SWAP<<6)|(MIPI_LANE_COUNT<<4));
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x11, MP_MCLK_INV); 
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19,(MP_CONTINUOUS_CLK<<1) | MP_LANE_DESKEW);  
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4B,(FRAME_RESYNC<<4));
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4E,(MP_V_RESYNC<<3)|(MP_H_RESYNC<<2)|(MP_VPOL<<1)|(MP_HPOL));      
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x72,0x01); 
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x73,0x03); 
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0x80,MP_PCLK_DIV); 
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC0,(HIGH_PCLK<< 4) | 0x0F);   
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC1,0x01);  
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC2,0x47);  
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC3,0x67);  
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC4,0x04);  
            it6151_i2c_write_byte(MIPI_I2C_ADDR,0xCB,(LVDS_PN_SWAP<<5)|(LVDS_LANE_SWAP<<4)|(LVDS_6BIT<<2)|(LVDS_DC_BALANCE<<1)| VESA_MAP);  
           return 1;
  } 
    return -1;
}

static unsigned int IT6151_ESD_Check(void)
{
#ifndef BUILD_LK
    static  unsigned char ucIsIT6151=0xFF;
    unsigned char ucReg, ucStat;
 // unsigned char cmdBuffer;
    
  //return FALSE;

    if(ucIsIT6151==0xFF){
        unsigned char VenID[2], DevID[2];
                
#ifndef BUILD_LK
        printk("\nIT6151 1st IRQ !!!\n");
#endif
                
        it6151_i2c_read_byte(DP_I2C_ADDR, 0x00, &VenID[0]);
        it6151_i2c_read_byte(DP_I2C_ADDR, 0x01, &VenID[1]);
        it6151_i2c_read_byte(DP_I2C_ADDR, 0x02, &DevID[0]);
        it6151_i2c_read_byte(DP_I2C_ADDR, 0x03, &DevID[1]);
    
#ifndef BUILD_LK
        printk("Current DevID=%02X%02X\n", DevID[1], DevID[0]);
        printk("Current VenID=%02X%02X\n", VenID[1], VenID[0]);
            #endif
                    
        if( VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x51 && DevID[1]==0x61){
                ucIsIT6151 = 1;
        }else{
                ucIsIT6151 = 0;
    }
  }
    if(ucIsIT6151==1){
        it6151_i2c_read_byte(DP_I2C_ADDR, 0x0D, &ucReg);
#ifndef BUILD_LK            
        printk("\nIT6151 Reg0x0D=0x%x !!!\n", ucReg);
#endif
        if(ucReg & 0x80){
            it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x06, &ucStat);
            if(ucStat & 0x01){
                it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x06, ucStat); 

                it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x0D, &ucStat);
                if(ucStat & 0x10){
                    //disable timer
                    it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
                    it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
                }else{
                    //enable timer
                    it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x40);               
                }                               
            }
            it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x08, &ucStat);
            if(ucStat & 0x40){
                if(ucStat & 0x20){
                    //disable timer
                    it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
                    it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
                }else{
                    return TRUE;
                }
            }       
        }
        if(ucReg & 0x01){   //DP_IRQ
            it6151_i2c_read_byte(DP_I2C_ADDR, 0x21, &ucStat);
            if(ucStat & 0x02){
                it6151_i2c_write_byte(DP_I2C_ADDR, 0x21, ucStat);
            }   
            it6151_i2c_read_byte(DP_I2C_ADDR, 0x06, &ucReg);
            it6151_i2c_read_byte(DP_I2C_ADDR, 0x0D, &ucStat);
            if(ucReg & 0x03){
                if(ucStat & 0x02){
                    return TRUE;
                }
            }                                       
        }           
    }
    return FALSE;
    #endif
}

static void IT6151_ESD_Recover(void)
{
    unsigned char ucStat;

  #ifndef BUILD_LK
    printk("\nIT6151_ESD_Recover\n");
    #endif
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x08, &ucStat);
    if(ucStat & 0x40){
        it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x0B, 0x00);
        it6151_i2c_write_byte(MIPI_I2C_ADDR, 0x08, 0x40);
        #ifndef BUILD_LK
        IT6151_init();
    #endif
    }else{
        IT6151_DPTX_init();
    }
}

static void lcm_get_params(LCM_PARAMS *params)
{   
//test
/*
	u8 temp,i;
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x0d, &temp);
	printk(" ======= DP_I2C_ADDR Reg0x0D = 0x%x \r\n",temp);
    it6151_i2c_read_byte(DP_I2C_ADDR, 0x0e, &temp);
	printk(" ======= DP_I2C_ADDR Reg0x0E = 0x%x \r\n",temp);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x0d, &temp);
	printk(" ======= MIPI_I2C_ADDR Reg0x0D = 0x%x \r\n",temp);
    it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x0e, &temp);
	printk(" ======= MIPI_I2C_ADDR  Reg0x0E = 0x%x \r\n",temp);	
	for(i=0x0;i<0xff;i++){
		it6151_i2c_read_byte(MIPI_I2C_ADDR, i, &temp);
		printk(" ======= MIPI_I2C_ADDR Reg0x%x = 0x%x \r\n",i,temp);
	}
	for(i=0x0;i<0xff;i++){
		it6151_i2c_read_byte(DP_I2C_ADDR, i, &temp);
		printk(" ======= DP_I2C_ADDR  Reg0x%x = 0x%x \r\n",i,temp);
	}
*/
//test end

	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dsi.mode    = SYNC_PULSE_VDO_MODE; // BURST_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;


	params->dsi.vertical_sync_active = 10;
	params->dsi.vertical_backporch =  10;
	params->dsi.vertical_frontporch = 10;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 50;
	params->dsi.horizontal_backporch =	50;
	params->dsi.horizontal_frontporch = 50;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=360; 
	
    params->dsi.cont_clock = 1;
	//params->dsi.ssc_disable = 1;
	//params->dsi.edp_panel = 1;
}



static void lcm_init(void)
{
    printk("lcm_init\n");
	lcm_set_gpio_output(GPIO_VLED_EN,0);
	MDELAY(50);
	lcm_set_gpio_output(GPIO_IT6151_INT,0);
    MDELAY(10);	
    lcd_3v3(1);    
    MDELAY(20);  
	lcd_bl(0);
	lcd_stb(0);
    lcd_1v8(1);
    lcd_reset(0);
    MDELAY(20);
    lcd_reset(1);
    MDELAY(100);

	IT6151_init();

    MDELAY(200);
	lcm_set_gpio_output(GPIO_VLED_EN,1);
	//MDELAY(20);
}



static void lcm_suspend_power(void)
{
    printk("lcm_suspend_power\n");
	lcm_set_gpio_output(GPIO_VLED_EN,0);
	MDELAY(10);
    lcd_3v3(0);
    lcd_bl(1);
	lcd_1v8(0);
    lcd_stb(1);
    lcd_reset(0);
}

static void lcm_resume_power(void)
{
    printk("lcm_resume_power\n");
    lcm_init();
}

static void lcm_init_power(void){

}
  
static void lcm_suspend_jd9367(void)
{

}

static void lcm_resume_jd9367(void)
{

}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    printk("lcm_esd_check\n\n");
#endif
    return IT6151_ESD_Check();
}

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
    printk("lcm_esd_recover\n\n");
#endif
    IT6151_ESD_Recover();
    return 0;
}

LCM_DRIVER c118_it6151_edp_dsi_lcm_drv= 
{
	.name			= "c118_it6151_edp_dsi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9367,
	.resume				=lcm_resume_jd9367,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	.esd_check      = lcm_esd_check,
    .esd_recover    = lcm_esd_recover,
	
};
