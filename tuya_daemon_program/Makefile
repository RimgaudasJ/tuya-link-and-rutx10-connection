include $(TOPDIR)/rules.mk

PKG_NAME:=tuya_daemon
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/tuya_daemon
	CATEGORY:=Base system
	TITLE:=tuya_daemon
	DEPENDS:= +libtuya_link +libubus +libblobmsg-json +libubox +libjson-c +libuci
endef

define Package/tuya_daemon/description
	Tuya Daemon for rutOS.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
endef

define Package/tuya_daemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuya_daemon $(1)/usr/bin
	$(INSTALL_BIN) ./files/tuya_daemon.init $(1)/etc/init.d/tuya_daemon
	$(INSTALL_CONF) ./files/tuya_daemon.config $(1)/etc/config/tuya_daemon
endef

$(eval $(call BuildPackage,tuya_daemon))