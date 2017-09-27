
TOPDIR := ../..

TARGET_LIBS = libcommon
TARGET_SHLIBS = libcommon

INCDIRS += ../include

#Only if masterbuild is not specified
ifeq (,$(strip $(MASTERBUILD_BUILD_DIR)))

ifneq ($(JH_GCHEAP), no)
CFLAGS_PROG += -DGCHEAP_ENABLED
endif

endif

COMMON_SRCS = Thread.cpp Mutex.cpp

SRCS_libcommon = $(COMMON_SRCS)

include $(TOPDIR)/mkbuild/Make.Defaults
include $(TOPDIR)/mkbuild/Make.Rules


