
TOPDIR := ../..

TARGET_LIBS = libcommon
TARGET_SHLIBS = libcommon

INCDIRS += ../include

ifeq ($(NO_LINT), yes)
NO_LINT=1
endif

COMMON_SRCS = Thread.cpp Mutex.cpp Condition.cpp EventQueue.cpp

SRCS_libcommon = $(COMMON_SRCS)

include $(TOPDIR)/mkbuild/Make.Defaults
include $(TOPDIR)/mkbuild/Make.Rules

