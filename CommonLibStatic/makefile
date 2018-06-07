# the SOURCE definiton lets you move your makefile to another position
CONFIG 			= LIBRARY

# set directories to your wanted values
SRC_DIR			= ./src
INC_DIR			= ./include
LIB_DIR			= ../lib
BIN_DIR			= ../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= `pkg-config opencv --cflags` \
				-I../CommonLibStatic/include \

#USR_LIB_DIRS	= -L C:/Program\ Files/OpenCV/lib \
USER_LIB_DIRS	=

# intermediate directory for object files
OBJ_DIR			= ./objects

# the library name
PRJ_NAME	 	= CommonLib

# version information
MAJOR_VER		= 2
MINOR_VER		= 0
VER				= $(MAJOR_VER).$(MINOR_VER)

# defines to set
DEFS			= -DMSYS_LINUX -DMSYS_UNIX_LARGEFILE -D_LARGEFILE64_SOURCE

# set objects
OBJS          	= \
				$(OBJ_DIR)/ParameterBase.o \
				$(OBJ_DIR)/upsample.o \
				$(OBJ_DIR)/yuv.o \

# set libs to link with
LIBS			= 

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			=
DYN_LIBS			= 

# the libraries to link with
DYN_DEBUG_LIBS		= 
DYN_DEBUG_PREREQS	= 
STAT_DEBUG_LIBS		=
STAT_DEBUG_PREREQS		=
DYN_RELEASE_LIBS	= 
DYN_RELEASE_PREREQS	= 
STAT_RELEASE_LIBS	=
STAT_RELEASE_PREREQS	=

# name of the base makefile
MAKE_FILE_NAME		= ../linux/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
