TAGET = motion_detection
OBJ = $(SRC:.c=.o)

CUR_INSTALLDEV_DIR := InstallDev
CUR_INSTALL_DIR := install

ifneq ($V,1)
Q ?= @
endif

LINK_TYPE ?= static

PROJECT_ROOT_PATH = $(shell pwd)
VERSION = 1.0.0

$(info "CC**********: $(CC)")

LOCAL_TARGET_BIN := motion_detect_demo

SRCCS := motion_detect_demo.c

INCLUDE_DIRS := \
	. \
	./include \
	$(STAGING_DIR)/usr/include/

$(info "library**********: $(MOTION_DETECTION_LIBC)")

LIB_SEARCH_PATHS := \
	. \
	./library/$(MOTION_DETECTION_LIBC)/ \
	$(STAGING_DIR)/usr/lib/

LOCAL_STATIC_LIBS :=

LOCAL_SHARED_LIBS := \
	libpthread \
	liblog

ifeq ($(MPPCFG_COMPILE_DYNAMIC_LIB),Y)
LOCAL_SHARED_LIBS += \
	libMotionDetect
else
LOCAL_STATIC_LIBS += \
	libMotionDetect
endif

#generate include directory flags for gcc.
inc_paths := $(foreach inc,$(filter-out -I%,$(INCLUDE_DIRS)),$(addprefix -I, $(inc))) $(filter -I%, $(INCLUDE_DIRS))
#Extra flags to give to the C compiler
LOCAL_CFLAGS := $(CFLAGS) $(CEDARX_EXT_CFLAGS) $(inc_paths) \
	-fPIC -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-label \
	-Wno-unused-function
#Extra flags to give to the C++ compiler
LOCAL_CXXFLAGS := $(CXXFLAGS) $(CEDARX_EXT_CFLAGS) $(inc_paths) -fPIC -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-label -Wno-unused-function
#Extra flags to give to the C preprocessor and programs that use it (the C and Fortran compilers).
LOCAL_CPPFLAGS := $(CPPFLAGS)
#target device arch: x86, arm
LOCAL_TARGET_ARCH := $(ARCH)
#Extra flags to give to compilers when they are supposed to invoke the linker,‘ld’.
LOCAL_LDFLAGS := $(LDFLAGS)

empty:=
space:= $(empty) $(empty)

LOCAL_BIN_LDFLAGS := $(LOCAL_LDFLAGS) \
    $(patsubst %,-L%,$(LIB_SEARCH_PATHS)) \
    -Wl,-rpath-link=$(subst $(space),:,$(strip $(LIB_SEARCH_PATHS))) \
    -Wl,-Bstatic \
    -Wl,--start-group $(foreach n, $(LOCAL_STATIC_LIBS), -l$(patsubst lib%,%,$(patsubst %.a,%,$(notdir $(n))))) -Wl,--end-group \
    -Wl,-Bdynamic \
    $(foreach y, $(LOCAL_SHARED_LIBS), -l$(patsubst lib%,%,$(patsubst %.so,%,$(notdir $(y)))))

#generate object files
OBJS := $(SRCCS:%=%.o) #OBJS=$(patsubst %,%.o,$(SRCCS))
DEPEND_LIBS := $(wildcard $(foreach p, $(patsubst %/,%,$(LIB_SEARCH_PATHS)), \
                            $(addprefix $(p)/, \
                              $(foreach y,$(LOCAL_SHARED_LIBS),$(patsubst %,%.so,$(patsubst %.so,%,$(notdir $(y))))) \
                              $(foreach n,$(LOCAL_STATIC_LIBS),$(patsubst %,%.a,$(patsubst %.a,%,$(notdir $(n))))) \
                            ) \
                          ) \
               )

.PHONY: all
all:$(LOCAL_TARGET_BIN) InstallDev install
	@echo "build $(LOCAL_TARGET_BIN) done"

$(LOCAL_TARGET_BIN):$(OBJS) $(DEPEND_LIBS)
	$(CXX) $(OBJS) $(LOCAL_BIN_LDFLAGS) -o $@

$(filter %.c.o, $(OBJS)): %.o: %
	$(CC) $(LOCAL_CFLAGS) $(LOCAL_CPPFLAGS) -MD -MP -MF $(@:%=%.d) -c -o $@ $<

InstallDev:
	@echo "install dev of motion_detection"
	mkdir -p $(CUR_INSTALLDEV_DIR)/usr/lib/
	mkdir -p $(CUR_INSTALLDEV_DIR)/usr/include/
	cp library/$(MOTION_DETECTION_LIBC)/* $(CUR_INSTALLDEV_DIR)/usr/lib/
	cp include/*.h $(CUR_INSTALLDEV_DIR)/usr/include/

install:
	@echo "install of motion_detection, TOOLCHAIN_LIBC=${MOTION_DETECTION_LIBC}"
	@echo "static_lib=${MPPCFG_COMPILE_STATIC_LIB}, dynamic_lib=${MPPCFG_COMPILE_DYNAMIC_LIB}"
ifeq (${MPPCFG_COMPILE_DYNAMIC_LIB}, Y)
	mkdir -p $(CUR_INSTALL_DIR)/usr/lib/
	cp library/$(MOTION_DETECTION_LIBC)/*.so $(CUR_INSTALL_DIR)/usr/lib/
endif

clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) *~ $(TAGET)
	$Q rm -rf $(CUR_INSTALLDEV_DIR) $(CUR_INSTALL_DIR)
