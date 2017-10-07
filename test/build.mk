ifndef LOADED_uconnect/src/build.mk
LOADED_uconnect/src/build.mk := 1

DIR := common/include

TARGET_PROGS = conditionTest threadTest eventAgentTest eventThreadTest listTest

INCDIRS_$(DIR) := $(TOPSRCDIR)/$(DIR) $(TOPSRCDIR)/$(DIR)/pubinc


$(DIR)_THREAD_TEST_SRCS = threadTest.cpp
$(DIR)_APPSTARTER = appstarter.cpp
$(DIR)_SERVERTEST = servertest.cpp

SRCS_listTest := listTest.cpp

SRCS_conditionTest := conditionTest.cpp
LDFLAGS_conditionTest = -lcommon

SRCS_threadTest := $($(DIR)_THREAD_TEST_SRCS)
LDFLAGS_threadTest = -lcommon

SRCS_eventAgentTest := eventAgentTest.cpp
LDFLAGS_eventAgentTest = -lcommon

SRCS_eventThreadTest := eventThreadTest.cpp
LDFLAGS_eventThreadTest = -lcommon

SRCS_appstarter := $($(DIR)_APPSTARTER)
LDFLAGS_appstarter = -luconnect

SRCS_servertest := $($(DIR)_SERVERTEST)
LDFLAGS_servertest = -luconnect

BIN = test

include $(TOPSRCDIR)/mkbuild/rules.mk

endif
