######################################
# target
######################################
TARGET = Hao

######################################
SYS_DIR    = -I./hardware/inc
SYS_SOURCE = ./hardware/src
######################################

# user_Source 添加.c文件
USER_SOURCE =                     \
	$(wildcard $(SYS_SOURCE)/*.c) \
	./core/cmsis/system_stm32f4xx.c    \
	./applications/rtos_common/free_rtos.c


######################################
# 用户添加的中间件库，例如：FATFS、CherryRB  
######################################
USER_MIDDLEWARES_SRC = \
	./middlewares/FATFS/diskio.c   \
	./middlewares/FATFS/ff.c       \
	./middlewares/FATFS/ffsystem.c \
	./middlewares/FATFS/ffunicode.c \
	./middlewares/CherryRB/chry_ringbuffer.c

USER_MIDDLEWARES_INC = \
	-I ./middlewares/FATFS  \
	-I ./middlewares/CherryRB

######################################
#openocd路径配置 
OPENOCD_DOWN_PATH = C:/openocd/openocd/scripts/interface/cmsis-dap.cfg
OPENOCD_CHIP_PATH = C:/openocd/openocd/scripts/target/stm32f4x.cfg


######################################
# 启动文件选择
STARTUP = startup_stm32f40_41xxx.s

# 系统外设库
STD_PERIPH_LIB_SRC = \
	$(wildcard ./core/libraries/src/*.c)

STD_PERIPH_LIB_INC =             \
	-I ./core/libraries/inc           \
	-I ./core/cmsis/device/include    \
	-I ./core/cmsis/include


##############################################################
# Cherryusb
CHERRYUSB_SRC =                        \
	./middlewares/CherryUSB-1.4.0/core/usbd_core.c \
	./middlewares/CherryUSB-1.4.0/port/dwc2/usb_dc_dwc2.c \
	./middlewares/CherryUSB-1.4.0/class/cdc/usbd_cdc_acm.c \
	./middlewares/CherryUSB-1.4.0/class/msc/usbd_msc.c

CHERRYUSB_INC =                      \
	-I ./middlewares/CherryUSB-1.4.0/common      \
	-I ./middlewares/CherryUSB-1.4.0/core        \
	-I ./middlewares/CherryUSB-1.4.0/port/dwc2   \
	-I ./middlewares/CherryUSB-1.4.0/config      \
	-I ./middlewares/CherryUSB-1.4.0/class/cdc   \
	-I ./middlewares/CherryUSB-1.4.0/class/msc

# freertos source code and head file
FREERTOS_SRC = 	\
	$(wildcard ./middlewares/FreeRTOS/src/*.c)	       \
	./middlewares/FreeRTOS/portable/MemMang/heap_4.c   \
	./middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c

FREERTOS_INC =	        \
	-I ./middlewares/FreeRTOS/inc	\
	-I ./middlewares/FreeRTOS/portable/GCC/ARM_CM4F

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O0

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =                   \
	$(wildcard ./applications/*.c)   \
	$(USER_SOURCE)            \
	$(STD_PERIPH_LIB_SRC)     \
    $(FREERTOS_SRC)           \
	$(CHERRYUSB_SRC)          \
	$(USER_MIDDLEWARES_SRC)


# ASM sources
ASM_SOURCES =  \
	./core/cmsis/startup/$(STARTUP)


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =                   \
-DUSE_STDPERIPH_DRIVER     \
-DSTM32F40_41xxx

# AS includes
AS_INCLUDES =  

# C includes
C_INCLUDES =                 \
	-I./applications                \
	-I./applications/rtos_common    \
	$(SYS_DIR)				 \
	$(STD_PERIPH_LIB_INC)    \
    $(FREERTOS_INC)          \
	$(CHERRYUSB_INC)         \
	$(USER_MIDDLEWARES_INC)

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=c99

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = ./core/link/STM32F40_41xxx/STM32F417IGHx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -lc -lrdimon -u _printf_float


# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
#windows
.PHONY : clean
clean:
	del /s/q $(BUILD_DIR)
#linux
#clean:
#	rm -rf $(BUILD_DIR)

download:
	openocd -f $(OPENOCD_DOWN_PATH) -f $(OPENOCD_CHIP_PATH)  -c init -c halt -c "flash write_image erase ./build/$(TARGET).hex" -c reset -c halt -c shutdown

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***