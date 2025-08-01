import os

# toolchains options
ARCH='arm'
CPU='cortex-m4'
CROSS_TOOL='gcc'

PROJECT_NAME = 'simplefoc-rtthread'

# bsp lib config
BSP_LIBRARY_TYPE = None

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
PLATFORM    = 'gcc'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'

PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
CXX = PREFIX + 'g++'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'elf'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'
DEVICE = ' -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections'
CFLAGS = DEVICE + ' -Dgcc'
AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
LFLAGS = DEVICE + f' -Wl,--gc-sections,-Map={PROJECT_NAME}.map,-cref,-u,Reset_Handler -T board/linker_scripts/link.lds'
CPATH = ''
LPATH = ''
if BUILD == 'debug':
    CFLAGS += ' -O0 -gdwarf-2 -g'
    AFLAGS += ' -gdwarf-2'
else:
    CFLAGS += ' -O2'
CXXFLAGS = CFLAGS
POST_ACTION = OBJCPY + f' -O binary $TARGET {PROJECT_NAME}.bin\n' + SIZE + ' $TARGET \n'


def dist_handle(BSP_ROOT, dist_dir):
    import sys
    cwd_path = os.getcwd()
    sys.path.append(os.path.join(os.path.dirname(BSP_ROOT), 'tools'))
    from sdk_dist import dist_do_building
    dist_do_building(BSP_ROOT, dist_dir)
