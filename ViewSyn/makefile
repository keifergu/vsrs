# the SOURCE definiton lets you move your makefile to another position
CONFIG 				= CONSOLE

# set directories to your wanted values
SRC_DIR				= ./src
INC_DIR				= ./include
LIB_DIR				= ../lib
BIN_DIR				= ../bin


SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

#USER_INC_DIRS	= -I C:/Program\ Files/OpenCV/cv/include \
#				-I C:/Program\ Files/OpenCV/cxcore/include \
#				-I C:/Program\ Files/OpenCV/cvaux/include \
#				-I C:/Program\ Files/OpenCV/otherlibs/highgui \
#				-I ../CommonLibStatic/include \
#				-I ../ViewSynLibStatic/include \

USER_INC_DIRS	= `pkg-config opencv --cflags` \
				-I../CommonLibStatic/include \
				-I../ViewSynLibStatic/include \

#USR_LIB_DIRS		= -L C:/Program\ Files/OpenCV/lib \
USER_LIB_DIRS	= 


# intermediate directory for object files
OBJ_DIR				= ./objects

# set executable name
PRJ_NAME			= ViewSyn

# version information
MAJOR_VER		= 2
MINOR_VER		= 0
VER				= $(MAJOR_VER).$(MINOR_VER)

# defines to set
#DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE #DT
DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
					$(OBJ_DIR)/ViewSyn.o \

# set libs to link with
LIBS			= -ldl -lcv -lcxcore -lhighgui -lcvaux -lml

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			= 
DYN_LIBS			=

DYN_DEBUG_LIBS		= -lViewSynLibStaticd -lCommonLibStaticd
DYN_DEBUG_PREREQS		= $(LIB_DIR)/libViewSynLibStaticd.a $(LIB_DIR)/libCommonLibStaticd.a
STAT_DEBUG_LIBS		= -lViewSynLibStaticd -lCommonLibStaticd
STAT_DEBUG_PREREQS	=  $(LIB_DIR)/libViewSynLibStaticd.a $(LIB_DIR)/libCommonLibStaticd.a

DYN_RELEASE_LIBS	= -lViewSynLibStatic -lCommonLibStatic
DYN_RELEASE_PREREQS	= $(LIB_DIR)/libViewSynLibStatic.a $(LIB_DIR)/libCommonLibStatic.a
STAT_RELEASE_LIBS	= -lViewSynLibStatic -lCommonLibStatic
STAT_RELEASE_PREREQS	= $(LIB_DIR)/libViewSynLibStatic.a $(LIB_DIR)/libCommonLibStatic.a


# name of the base makefile
MAKE_FILE_NAME		= ../linux/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
