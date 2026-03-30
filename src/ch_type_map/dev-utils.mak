APPS=
APPS+=ch_type_map

APP_OBJS_ch_type_map=ch_type_map.o
DEP_FILES_ch_type_map=$(addsuffix .d,$(basename $(APP_OBJS_ch_type_map)))


#
# Tools
#
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
PKG_CONFIG=$(CROSS_COMPILE)pkg-config


#
# Flags
#
DEFINES=-D_GNU_SOURCE
CPPFLAGS=-MMD $(DEFINES) -I.
CFLAGS=-pipe -Wall
#
# By default, use debug flags.
# For release flags, run 'make RELEASE=1'
#
ifneq ($(RELEASE),1)
CFLAGS+=-Og -g
else
CPPFLAGS+=-DNDEBUG
CFLAGS+=-O3
endif
LDFLAGS=

#
# C++ flags are by default the same as C flags
#
CXXFLAGS=$(CFLAGS)

#
# Check for specific sysroot
#
ifneq ($(SYSROOT_DIR),)
CFLAGS+=--sysroot=$(SYSROOT_DIR)
LDFLAGS+=--sysroot=$(SYSROOT_DIR)
endif


#
# Rules
#
all:	$(APPS)


ch_type_map:	$(APP_OBJS_ch_type_map)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


clean:
	rm -f $(APPS) *.o *.d

#
# Dependencies
#
-include $(DEP_FILES_ch_type_map)
