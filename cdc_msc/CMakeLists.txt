# 在 CMakeLists.txt 顶部添加
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10.0)
set(project_name "stm32f4x")

set(CMAKE_SYSTEM_NAME Generic) # 'Generic' is used for embedded systems
set(CMAKE_SYSTEM_PROCESSOR arm)

#################################################
# 设置工具链
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# tells CMake not to try to link executables during its interal checks
# things are not going to link properly without a linker script
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_DEBUGGER arm-none-eabi-gdb)
set(CMAKE_DEBUGGER arm-none-eabi-gdb)
set(CMAKE_CPPFILT arm-none-eabi-c++filt)
#################################################

# 工程支持 C ASM
project(${project_name} VERSION 1.0.0 LANGUAGES C ASM)

###########################################################
# 设置 OpenOCD 路径变量
set(OPENOCD_DOWN_PATH "C:/openocd/openocd/scripts/interface/cmsis-dap.cfg")
set(OPENOCD_CHIP_PATH "C:/openocd/openocd/scripts/target/stm32f4x.cfg")

######################################
# 启动文件选择
set(STARTUP  startup_stm32f40_41xxx.s)

#############################################
# 用户添加的中间件库，例如：FATFS、CherryRB
#############################################
file(GLOB USER_MIDDLEWARES_SRC 
    "${PROJECT_SOURCE_DIR}/middlewares/FATFS/diskio.c"
	"${PROJECT_SOURCE_DIR}/middlewares/FATFS/ff.c"
	"${PROJECT_SOURCE_DIR}/middlewares/FATFS/ffsystem.c"
	"${PROJECT_SOURCE_DIR}/middlewares/FATFS/ffunicode.c"
	"${PROJECT_SOURCE_DIR}/middlewares/CherryRB/chry_ringbuffer.c"
)

set(USER_MIDDLEWARES_INC 
    "${PROJECT_SOURCE_DIR}/middlewares/FATFS"
	"${PROJECT_SOURCE_DIR}/middlewares/CherryRB"
) 

#############################################################
# 应用程序源文件
file(GLOB _APP_SRC 
    "${PROJECT_SOURCE_DIR}/applications/*.c"
    "${PROJECT_SOURCE_DIR}/hardware/src/*.c"
    ${USER_MIDDLEWARES_SRC}
)

set(APP_SOURCES 
    ${PROJECT_SOURCE_DIR}/applications/rtos_common/free_rtos.c
    ${_APP_SRC}
)

# 用户头文件
include_directories(
    "${PROJECT_SOURCE_DIR}/hardware/inc"
    "${PROJECT_SOURCE_DIR}/applications"
    "${PROJECT_SOURCE_DIR}/applications/rtos_common"
    ${USER_MIDDLEWARES_INC}
)

#############################################################
# 系统外设库
file(GLOB STD_PERIPH_LIB_SRC 
    "${PROJECT_SOURCE_DIR}/core/libraries/src/*.c"
    "${PROJECT_SOURCE_DIR}/core/cmsis/system_stm32f4xx.c"    
)

# 添加头文件路径
include_directories(
    "${PROJECT_SOURCE_DIR}/core/libraries/inc"
    "${PROJECT_SOURCE_DIR}/core/cmsis/device/include"
    "${PROJECT_SOURCE_DIR}/core/cmsis/include"
)

##############################################################
#                       Cherryusb
##############################################################
file(GLOB CHERRYUSB_SRC
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/core/usbd_core.c"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/port/dwc2/usb_dc_dwc2.c"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/class/cdc/usbd_cdc_acm.c"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/class/msc/usbd_msc.c"
)

include_directories(
    # CherryUSB
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/common"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/core"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/port/dwc2"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/config"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/class/cdc"
    "${PROJECT_SOURCE_DIR}/middlewares/CherryUSB-1.4.0/class/msc"
)

###############################################################
#                freertos source code and head file
###############################################################
file(GLOB FREERTOS_SOURCES
    "${PROJECT_SOURCE_DIR}/middlewares/FreeRTOS/src/*.c"
    "${PROJECT_SOURCE_DIR}/middlewares/FreeRTOS/portable/MemMang/heap_4.c"
    "${PROJECT_SOURCE_DIR}/middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c"
)

# FreeRTOS 头文件路径
include_directories(
    # FreeRTOS
    "${PROJECT_SOURCE_DIR}/middlewares/FreeRTOS/inc"
    "${PROJECT_SOURCE_DIR}/middlewares/FreeRTOS/portable/GCC/ARM_CM4F"
)

##########################################################################
# 启动文件（根据具体型号选择，例如startup_stm32f40xx.s）
set(STARTUP_ASM_FILE "${PROJECT_SOURCE_DIR}/core/cmsis/startup/${STARTUP}")

####################################################
# optimization
set(OPT -O1)

set(CPU_PARAMETERS 
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# 只给 C 编译器的选项
set(C_OPTIONS "-Wall -ffunction-sections -fdata-sections -O1 -g -gdwarf-2 -std=c99")

# 只给 ASM 编译器的选项
set(ASM_OPTIONS -O1)

# 追加设置
add_compile_options(
    ${CPU_PARAMETERS}
)

# 分别加到 C 和 ASM
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_OPTIONS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

###################################################################
# C defines
add_definitions(
    -DUSE_STDPERIPH_DRIVER
    -DSTM32F40_41xxx
)

###################################################################
# 创建可执行文件
add_executable(${PROJECT_NAME}.elf
    ${STD_PERIPH_LIB_SRC}
    ${APP_SOURCES}
    ${FREERTOS_SOURCES}
    ${CHERRYUSB_SRC}
    ${STARTUP_ASM_FILE}
)

####################################################################
# 可执行文件链接选项
target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_PARAMETERS}
    -specs=nosys.specs
    -specs=nano.specs 
    -T${PROJECT_SOURCE_DIR}/core/link/STM32F40_41xxx/STM32F417IGHx_FLASH.ld # 链接脚本路径 
    -Wl,-Map=${PROJECT_NAME}.map,--cref -Wl,--gc-sections 
    -lc -lrdimon -u _printf_float
)

########################################################
# 后构建步骤：生成HEX/BIN文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Generating HEX and BIN files"
)

#############################################
# 添加自定义下载目标
add_custom_target(download
    COMMAND openocd 
        -f ${OPENOCD_DOWN_PATH} 
        -f ${OPENOCD_CHIP_PATH}  
        -c "init" 
        -c "halt" 
        -c "flash write_image erase ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex" 
        -c "reset" 
        -c "halt" 
        -c "shutdown"
    DEPENDS ${TARGET}  # 确保目标文件已构建
    COMMENT "Flashing ${TARGET}.hex to device..."
)