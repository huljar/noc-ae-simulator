#
# OMNeT++/OMNEST Makefile for HaecComm
#
# This file was generated with the command:
#  opp_makemake -f --deep
#

# Name of target to be created (-o option)
TARGET = HaecComm$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I. -ICore -IMW -IMessages -IUtil

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/Core/Clock.o \
    $O/Core/HaecModule.o \
    $O/MW/RouterXY.o \
    $O/MW/NetworkCoding.o \
    $O/MW/ReportArrival.o \
    $O/MW/cMiddlewareBase.o \
    $O/MW/DummyMW.o \
    $O/MW/GenTraffic.o \
    $O/Util/CryptoManager.o \
    $O/Util/NetworkCodingManager.o \
    $O/Util/NcGen.o \
    $O/Messages/NcCombination_m.o

# Message files
MSGFILES = \
    Messages/NcCombination.msg

# SM files
SMFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f HaecComm HaecComm.exe libHaecComm.so libHaecComm.a libHaecComm.dll libHaecComm.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h
	$(Q)-rm -f Core/*_m.cc Core/*_m.h Core/*_sm.cc Core/*_sm.h
	$(Q)-rm -f MW/*_m.cc MW/*_m.h MW/*_sm.cc MW/*_sm.h
	$(Q)-rm -f Messages/*_m.cc Messages/*_m.h Messages/*_sm.cc Messages/*_sm.h
	$(Q)-rm -f Util/*_m.cc Util/*_m.h Util/*_sm.cc Util/*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc Core/*.cc MW/*.cc Messages/*.cc Util/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/Core/Clock.o: Core/Clock.cc \
	Core/Clock.h \
	Core/Signals.h
$O/Core/HaecModule.o: Core/HaecModule.cc \
	Core/HaecModule.h
$O/MW/DummyMW.o: MW/DummyMW.cc \
	Core/HaecModule.h \
	MW/DummyMW.h \
	MW/cMiddlewareBase.h
$O/MW/GenTraffic.o: MW/GenTraffic.cc \
	Core/HaecModule.h \
	MW/GenTraffic.h \
	MW/cMiddlewareBase.h
$O/MW/NetworkCoding.o: MW/NetworkCoding.cc \
	Core/HaecModule.h \
	MW/NetworkCoding.h \
	MW/cMiddlewareBase.h \
	Messages/NcCombination_m.h \
	Util/NcGen.h \
	Util/NetworkCodingManager.h
$O/MW/ReportArrival.o: MW/ReportArrival.cc \
	Core/HaecModule.h \
	MW/ReportArrival.h \
	MW/cMiddlewareBase.h
$O/MW/RouterXY.o: MW/RouterXY.cc \
	Core/HaecModule.h \
	MW/RouterXY.h \
	MW/cMiddlewareBase.h
$O/MW/cMiddlewareBase.o: MW/cMiddlewareBase.cc \
	Core/HaecModule.h \
	MW/cMiddlewareBase.h
$O/Messages/NcCombination_m.o: Messages/NcCombination_m.cc \
	Messages/NcCombination_m.h
$O/Util/CryptoManager.o: Util/CryptoManager.cc \
	Messages/NcCombination_m.h \
	Util/CryptoManager.h
$O/Util/NcGen.o: Util/NcGen.cc \
	Messages/NcCombination_m.h \
	Util/NcGen.h
$O/Util/NetworkCodingManager.o: Util/NetworkCodingManager.cc \
	Messages/NcCombination_m.h \
	Util/NcGen.h \
	Util/NetworkCodingManager.h

