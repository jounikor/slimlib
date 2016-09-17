.SUFFIXES:
#.SILENT:

SOURCES		:=	source
INCLUDES	:=	include
BUILD		:=	build
TARGET		:=	sl4lib
ARCH		:=	i386
LIBUSB		:=	/usr/local/include/libusb-1.0


ASFLAGS		:=
CXXFLAGS	:=	-O2 -Wall  -Wno-multichar -Wunused -fno-rtti
CFLAGS		:=	-O2 -Wall

UNAME := $(shell uname -s)

ifneq (,$(findstring MINGW,$(UNAME)))
	PLATFORM	:= win32
	EXE		:= .exe
	SYS_LIBS	:= -mwindows -lole32 -luuid -lcomctl32 -lwsock32 -lsupc++
	LDFLAGS		:= -s
endif

BINARIES	:=	$(TARGET)$(EXE)

ifneq (,$(findstring Linux,$(UNAME)))
	PLATFORM	:= linux
	EXE		:=
	SYS_LIB_PATH	:= -L/usr/X11R6/lib
	SYS_LIBS	:= -lm -lXext -lX11
	BINARIES	+= $(TARGET)_static
	LDFLAGS		:= -s
endif


ifneq (,$(findstring Darwin,$(UNAME)))
	PLATFORM	:= OSX
	EXE		:=
	SYS_LIB_PATH	:=
	SYS_LIBS	:= -lc -lstdc++
	BINARIES	+=
	LDFLAGS		:= -arch $(ARCH) -framework CoreFoundation -framework IOKit
	CXXFLAGS	+= -arch $(ARCH) -I$(LIBUSB)
endif

LDLIBS := /usr/local/lib/libusb-1.0.a $(SYS_LIB_PATH) $(SYS_LIBS)

LINKFLTK	:=


CXX			:=	g++
CC			:=	gcc

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))


export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
ASMFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifneq (,$(findstring MINGW,$(UNAME)))
	RESOURCEFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.rc)))
endif

ifneq (,$(findstring Linux,$(UNAME)))
export	STATIC		:= $(OUTPUT)_static
endif


export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export DEPENDS	:=	$(OFILES:.o=.d)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo cleaning ...
	@rm -fr $(BUILD) $(OUTPUT)$(EXE)

run:
	$(OUTPUT)$(EXE)

#---------------------------------------------------------------------------------
dist:
#---------------------------------------------------------------------------------
	@tar --exclude=*svn* -cvjf $(OUTPUT)_$(PLATFORM).tar.bz2 include source \
		Makefile docs README $(BINARIES) $(EXTRAS)

#---------------------------------------------------------------------------------
else

$(OUTPUT)$(EXE):	$(STATIC) $(OFILES)
	@echo Linking $(notdir $@)...
	@$(CXX) $(LDFLAGS)  $(OFILES) $(LDLIBS) -o $@
	@strip $(OUTPUT)$(EXE)

$(STATIC):	$(OFILES)
	@echo Linking static $(notdir $@)...
	@$(CXX) -static $(LDFLAGS)  $(OFILES) $(LDLIBS) -o $@


%.o:	%.cpp
	@echo Compiling $(notdir $<)...
	@$(CXX)  -MMD $(CXXFLAGS) $(INCLUDE) -c $<

%.o:	%.c
	@echo Compiling $(notdir $<)...
	@$(CC)  -MMD $(CFLAGS) $(INCLUDE) -c $<

%.o:	%.s
	@echo Compiling $(notdir $<)...
	@$(CXX) -MMD $(ASFLAGS) -c $< -o $@

%.o:	%.rc
	@echo Compiling resource $(notdir $<)...
	@windres -i $< -o $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
