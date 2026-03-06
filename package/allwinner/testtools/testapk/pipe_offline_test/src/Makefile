TARGET = pipe_offline_test

# Include paths for Allwinner platform headers
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include  -I$(STAGING_DIR)/usr/include/allwinner

# Include paths for MediaPipeX plugin architecture
INCLUDES += -I./plugins                                      # Plugin interface headers
INCLUDES += -I./plugins/v4l2src                             # V4L2 camera plugin
INCLUDES += -I./plugins/g2d                                 # G2D processing plugin
INCLUDES += -I./plugins/gdc                                 # GDC correction plugin
INCLUDES += -I./plugins/Encoder                             # H.264 encoder plugin
INCLUDES += -I./utils                                       # Utility functions

# Allwinner platform libraries
LIBS += -lpthread -ldl -lrt -lawion -lvencoder \
        -lVE -lvenc_base -lcdc_base -lMemAdapter -lvenc_codec -lpthread -lm

ifeq ($(USE_VIN_ISP),y)
    CompileFlags += -D__USE_VIN_ISP__
    LIBS += -lm -lisp -lisp_ini -lAWIspApi
endif

SRCS :=  $(shell find . -type f -name "*.c")

# 最终编译的源文件
SRCS += $(MAIN_SRC)

OBJS = $(SRCS:.c=.o)

LIBS_VIP += -lNBGlinker -lVIPhal
MYCFLAGS += -DNPU_SW_VERSION=2

%.o: %.c
	$(CC) $(MYCFLAGS) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LIBS_VIP) $(LDFLAGS)

all: $(TARGET)

clean:
	rm -rf $(TARGET) $(OBJS) *.o *~
