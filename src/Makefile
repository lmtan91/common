
TOPDIR := ../..

TARGET_LIBS = libcommon
TARGET_SHLIBS = libcommon

INCDIRS += ../include

EXTRA_CCFLAGS += -DVERBOSE_LOGGING

ifeq ($(NO_LINT), yes)
NO_LINT=1
endif

COMMON_SRCS = Thread.cpp \
Mutex.cpp \
Condition.cpp \
EventQueue.cpp \
Timer.cpp \
EventDispatcher.cpp \
EventThread.cpp \
Selector.cpp \
logging.cpp \
logging_syslog.cpp \
Socket.cpp \
ErrCode.cpp \
FdReaderWriter.cpp \
CircularBuffer.cpp \
HttpHeaderBase.cpp \
File.cpp \
URI.cpp \
HttpHeader.cpp \
HttpRequest.cpp \

SRCS_libcommon = $(COMMON_SRCS)

include $(TOPDIR)/mkbuild/Make.Defaults
include $(TOPDIR)/mkbuild/Make.Rules

