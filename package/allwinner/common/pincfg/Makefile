include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=pincfg
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=System
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=pincfg for Allwinner
  DEPENDS:=+libncurses
endef

define Package/$(PKG_NAME)/description
The tools to get or set pin config for Allwinner
endef

define Build/Prepare
	@$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	@$(CP) ./src $(PKG_BUILD_DIR)
endef

#MR153
ifeq ($(LICHEE_CHIP), sun8iw22p1)
    X_GPIO_BASE_ADDR:=0x03604080
    X_GPIO_PORT_GROUP_SPACING:=0x80
    S_GPIO_BASE_ADDR:=0x07096000
    S_GPIO_PORT_GROUP_SPACING:=0x30
    GPIO_PORT_DATA_OFFSET:=0x10
    X_GPIO_OS_SEL_OFFSET:=0x780
    S_GPIO_OS_SEL_OFFSET:=0x00
#MR813
else ifeq ($(LICHEE_CHIP), sun50iw10p1)
    X_GPIO_BASE_ADDR:=0x0300b000
    X_GPIO_PORT_GROUP_SPACING:=0x24
    S_GPIO_BASE_ADDR:=0x07022000
    S_GPIO_PORT_GROUP_SPACING:=0x00
    GPIO_PORT_DATA_OFFSET:=0x10
    X_GPIO_OS_SEL_OFFSET:=0x0
    S_GPIO_OS_SEL_OFFSET:=0x0
#MR527
else ifeq ($(LICHEE_CHIP), sun55iw3p1)
    X_GPIO_BASE_ADDR:=0x02000000
    X_GPIO_PORT_GROUP_SPACING:=0x30
    S_GPIO_BASE_ADDR:=0x07022000
    S_GPIO_PORT_GROUP_SPACING:=0x30
    GPIO_PORT_DATA_OFFSET:=0x10
    X_GPIO_OS_SEL_OFFSET:=0x0
    S_GPIO_OS_SEL_OFFSET:=0x0
#MR536
else ifeq ($(LICHEE_CHIP), sun55iw6p1)
    X_GPIO_BASE_ADDR:=0x03604080
    X_GPIO_PORT_GROUP_SPACING:=0x80
    S_GPIO_BASE_ADDR:=0x07096000
    S_GPIO_PORT_GROUP_SPACING:=0x30
    GPIO_PORT_DATA_OFFSET:=0x10
    X_GPIO_OS_SEL_OFFSET:=0x0
    S_GPIO_OS_SEL_OFFSET:=0x0
endif

TARGET_CFLAGS += -D_GNU_SOURCE \
                 -DX_GPIO_BASE_ADDR=$(X_GPIO_BASE_ADDR) \
                 -DX_GPIO_PORT_GROUP_SPACING=$(X_GPIO_PORT_GROUP_SPACING) \
                 -DS_GPIO_BASE_ADDR=$(S_GPIO_BASE_ADDR) \
                 -DS_GPIO_PORT_GROUP_SPACING=$(S_GPIO_PORT_GROUP_SPACING) \
                 -DGPIO_PORT_DATA_OFFSET=$(GPIO_PORT_DATA_OFFSET) \
                 -DX_GPIO_OS_SEL_OFFSET=$(X_GPIO_OS_SEL_OFFSET) \
                 -DS_GPIO_OS_SEL_OFFSET=$(S_GPIO_OS_SEL_OFFSET)

define Build/Compile
	@$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/pincfg $(1)/sbin/
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mr536))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mr536_pinconfig $(1)/etc/pinconfig
else ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mr527))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mr527_pinconfig $(1)/etc/pinconfig
else ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mr153))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mr153_pinconfig $(1)/etc/pinconfig
else ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mr813))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mr813_pinconfig $(1)/etc/pinconfig
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
