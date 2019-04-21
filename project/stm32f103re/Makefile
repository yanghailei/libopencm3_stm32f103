##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY = mytest

OUT_DIR = out

SRC_DIR = \
mypro \
driver \
driver/common \
driver/printf \
driver/led \
driver/sht20 \
driver/baiduiot\


# define head file dir
CFLAGS = \
-Imypro \
-Idriver \
-Idriver/common \
-Idriver/printf \
-Idriver/led \
-Idriver/sht20 \
-Idriver/baiduiot\


SRCS = \
mypro/main.c  \
driver/driver_face.c \
driver/common/common.c  \
driver/printf/printf.c \
driver/led/led_breath.c \
driver/led/breath_table.c \
driver/led/iic.c \
driver/led/sht20.c \
driver/baiduiot/baiduiot.c \
driver/baiduiot/esp8266.c \
driver/baiduiot/mqtt.c \



OBJS += $(patsubst %.c, ${OUT_DIR}/%.o , $(notdir ${SRCS})) 

vpath %.c $(sort $(SRC_DIR))



LDSCRIPT = stm32f103xe.ld









LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

################################################################################
# OpenOCD specific variables

OOCD		?= openocd
OOCD_INTERFACE	?= flossjtag
OOCD_TARGET	?= stm32f1x

################################################################################
# Black Magic Probe specific variables
# Set the BMP_PORT to a serial port and then BMP is used for flashing
BMP_PORT	?=

################################################################################
# texane/stlink specific variables
#STLINK_PORT	?= :4242


include ../rules.mk
