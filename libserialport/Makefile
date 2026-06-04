include $(TOPDIR)/rules.mk

PKG_NAME:=libserialport
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://sigrok.org/libserialport
PKG_SOURCE_VERSION:=HEAD
PKG_MIRROR_HASH:=skip

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=LGPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/autotools.mk

define Package/libserialport
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Cross-platform serial port library
	URL:=https://sigrok.org/wiki/Libserialport
endef

define Package/libserialport/description
 A lightweight, cross-platform serial port access library.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libserialport.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libserialport.{a,la,so*} $(1)/usr/lib/
endef

define Package/libserialport/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libserialport.so.* $(1)/usr/lib/
endef


$(eval $(call BuildPackage,libserialport))