TAGET = pdet_demo
OBJ = $(SRC:.c=.o)

CUR_INSTALLDEV_DIR := InstallDev
CUR_INSTALL_DIR := install

ifneq ($V,1)
Q ?= @
endif

LINK_TYPE ?= static

PROJECT_ROOT_PATH = $(shell pwd)
VERSION = 1.0.0

$(info "CXX**********: $(CXX)")

# compiler option
CFLAGS   = -O3 -fvisibility=hidden -s -Wall -fPIC -c -ffunction-sections -fdata-sections -march=rv32imafdc_zicsr_zifencei -mabi=ilp32d
CPPFLAGS = -O3 -fvisibility=hidden -s -Wall -fPIC -fexceptions -c -ffunction-sections -fdata-sections -march=rv32imafdc_zicsr_zifencei -mabi=ilp32d

DEFS += -D__RISCV__
DEFS += -DTARGET_RISCV

CFLAGS   += $(DEFS)
CPPFLAGS += $(DEFS)

INCLUDE += -I$(PROJECT_ROOT_PATH) -I$(PROJECT_ROOT_PATH)/include -I$(PROJECT_ROOT_PATH)/3rdparty/include
SRC = pdet_demo.c

LDFLAGS = -lpthread

# $(info "lib**********: $(PERSON_DETECTION_LIBC)")

ifeq ($(LINK_TYPE), dynamic)
	LDLIBS += -L$(PROJECT_ROOT_PATH)/3rdparty/lib/$(PERSON_DETECTION_LIBC) -law_utilities_standalone_v821
	LDLIBS += -L$(PROJECT_ROOT_PATH)/lib/$(PERSON_DETECTION_LIBC) -law_person_det_v821 -lnn_p -lnn_qt_dq_p -law_person_det_nn_v821
endif

ifeq ($(LINK_TYPE), static)
	LDLIBS  += $(PROJECT_ROOT_PATH)/lib/$(PERSON_DETECTION_LIBC)/libnn_p.a
	LDLIBS  += $(PROJECT_ROOT_PATH)/lib/$(PERSON_DETECTION_LIBC)/libnn_qt_dq_p.a
	LDLIBS  += $(PROJECT_ROOT_PATH)/lib/$(PERSON_DETECTION_LIBC)/libaw_person_det_nn_v821.a
	LDLIBS  += $(PROJECT_ROOT_PATH)/lib/$(PERSON_DETECTION_LIBC)/libaw_person_det_v821.a
	LDLIBS  += $(PROJECT_ROOT_PATH)/3rdparty/lib/$(PERSON_DETECTION_LIBC)/libaw_utilities_standalone_v821.a
endif

.PHONY: all
all: $(TAGET) InstallDev install
ifeq ($(LINK_TYPE), static)
$(TAGET):$(OBJ) $(C_OBJ)
	$Q echo [$(TAGET) LINK_TYPE=$(LINK_TYPE)]
	$Q $(CXX) -o $(TAGET) $(OBJ) $(INCLUDE) -Wl,--start-group $(LDLIBS) -Wl,--end-group $(LDFLAGS)
	$Q rm -f $(OBJ)
endif

ifeq ($(LINK_TYPE), dynamic)
$(TAGET):$(OBJ) $(C_OBJ)
	$Q echo [$(TAGET) LINK_TYPE=$(LINK_TYPE)]
	$Q $(CXX) -o $@ $< -Wl,--start-group $(LDLIBS) -Wl,--end-group $(LDFLAGS)
	$Q rm -f $(OBJ)
endif

InstallDev:
	@echo "install dev of person_detection"
	mkdir -p $(CUR_INSTALLDEV_DIR)/usr/lib/pdet
	mkdir -p $(CUR_INSTALLDEV_DIR)/usr/include/pdet
	cp lib/$(PERSON_DETECTION_LIBC)/* $(CUR_INSTALLDEV_DIR)/usr/lib/pdet
	cp 3rdparty/lib/$(PERSON_DETECTION_LIBC)/* $(CUR_INSTALLDEV_DIR)/usr/lib/pdet
	cp include/*.h $(CUR_INSTALLDEV_DIR)/usr/include/pdet
	cp 3rdparty/include/*.h $(CUR_INSTALLDEV_DIR)/usr/include/pdet

install:
	@echo "install of person_detection, TOOLCHAIN_LIBC=${PERSON_DETECTION_LIBC}"
	@echo "static_lib=${MPPCFG_COMPILE_STATIC_LIB}, dynamic_lib=${MPPCFG_COMPILE_DYNAMIC_LIB}"
ifeq (${MPPCFG_COMPILE_DYNAMIC_LIB}, Y)
	mkdir -p $(CUR_INSTALL_DIR)/usr/lib/pdet
	cp lib/$(PERSON_DETECTION_LIBC)/*.so $(CUR_INSTALL_DIR)/usr/lib/pdet
	cp 3rdparty/lib/$(PERSON_DETECTION_LIBC)/*.so $(CUR_INSTALL_DIR)/usr/lib/pdet
endif

.c.o:
	$Q echo $(CC) $<
	$Q $(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE) -Wno-strict-aliasing

.cpp.o:
	$Q echo $(CPP) $<
	$Q $(CXX) -c $(CPPFLAGS) $< -o $@ $(INCLUDE) -Wno-strict-aliasing

clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) *~ $(TAGET)
	$Q rm -rf $(CUR_INSTALLDEV_DIR) $(CUR_INSTALL_DIR)
