include $(TOPDIR)/rules.mk

PKG_NAME:=esp_module
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/esp_module
	CATEGORY:=Base system
	TITLE:=esp_module
	DEPENDS:= +libserialport +libubus +libblobmsg-json +libubox +libjson-c 
endef

define Package/esp_module/description
	ubus ESP module for rutOS.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
endef

define Package/esp_module/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/esp_module $(1)/usr/bin
	$(INSTALL_BIN) ./files/esp_module.init $(1)/etc/init.d/esp_module
endef

$(eval $(call BuildPackage,esp_module))