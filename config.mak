 # Copyright (C) 2016  Nexell Co., Ltd.
 # Author: Sangjong, Han <hans@nexell.co.kr>
 #
 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public License
 #
 # as published by the Free Software Foundation; either version 2
 # of the License, or (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################
# Build Version info
###########################################################################
VERINFO			= V001

###########################################################################
# Build Environment
###########################################################################
#CHIPNAME		= NXP4330
CHIPNAME		= S5P4418

DEBUG			= n

MEMTYPE			= DDR3
#MEMTYPE		= LPDDR3
MEMTEST			= n

CRC_CHECK		= n

CFLAGS			:=

# System Log Message
SYSLOG			?= y

# Secure Boot
SECURE_ON		?= 0

ifeq ($(CHIPNAME), NXP4330)
BOARD			= LEPUS
#BOARD			= NAVI
else
#BOARD			= SVT
#BOARD			= ASB
#BOARD			= DRONE
#BOARD			= AVN
#BOARD			= LAVENDA
BOARD			?= RAPTOR
endif

SECURE			?= NO

# cross-tool pre-header
ifeq ($(OS),Windows_NT)
CROSS_TOOL_TOP		=
CROSS_TOOL		= $(CROSS_TOOL_TOP)arm-none-eabi-
else
CROSS_TOOL_TOP		=
CROSS_TOOL		= $(CROSS_TOOL_TOP)arm-eabi-
endif

###########################################################################
# Top Names
###########################################################################
PROJECT_NAME		= $(CHIPNAME)_dispatcher_$(MEMTYPE)_$(VERINFO)
TARGET_NAME		= armv7_dispatcher-$(shell echo $(BOARD) | tr A-Z a-z)
LDS_NAME		= armv7_dispatcher

###########################################################################
# Directories
###########################################################################
DIR_PROJECT_TOP		=

DIR_OBJOUTPUT		= obj
DIR_TARGETOUTPUT	= out

CODE_MAIN_INCLUDE	=

###########################################################################
# Build Environment
###########################################################################
CPU			= cortex-a9
CC			= $(CROSS_TOOL)gcc
LD 			= $(CROSS_TOOL)ld
AS 			= $(CROSS_TOOL)as
AR 			= $(CROSS_TOOL)ar
MAKEBIN			= $(CROSS_TOOL)objcopy
OBJCOPY			= $(CROSS_TOOL)objcopy
RANLIB 			= $(CROSS_TOOL)ranlib

GCC_LIB			= $(shell $(CC) -print-libgcc-file-name)

ifeq ($(SECURE_ON), 1)
CFLAGS			+=  -DSECURE_ON
endif

ifeq ($(DEBUG), y)
CFLAGS			= -DNX_DEBUG -O0
Q			=
else
CFLAGS			= -DNX_RELEASE -Os
Q			= @
endif

###########################################################################
# MISC tools for MS-DOS
###########################################################################
MKDIR			= mkdir
CD			= cd
ECHO			= echo
ifeq ($(OS),Windows_NT)
RM			= del /q /F
MV			= move
CP			= copy
RMDIR			= rmdir /S /Q
else
RM			= rm -f
MV			= mv
CP			= cp
RMDIR			= rm -rf
endif
###########################################################################
# FLAGS
###########################################################################
ARFLAGS			= rcs
ARFLAGS_REMOVE		= -d
ARLIBFLAGS		= -v -s

ASFLAG			= -D__ASSEMBLY__

CFLAGS			+=	-g -Wall				\
				-Wextra -ffreestanding -fno-builtin	\
				-msoft-float				\
				-mlittle-endian				\
				-mcpu=$(CPU)				\
				-mstructure-size-boundary=32		\
				$(CODE_MAIN_INCLUDE)			\
				-D__arm -DLOAD_FROM_$(BOOTFROM)		\
				-DMEMTYPE_$(MEMTYPE)			\
				-DCHIPID_$(CHIPNAME)			\
				-D$(BOARD)

ifeq ($(SYSLOG), y)
CFLAGS			+=	-DSYSLOG_ON
endif

ifeq ($(SECURE), YES)
CFLAGS			+=	-DSECURE_MODE
endif

