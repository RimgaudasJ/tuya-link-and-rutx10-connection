include $(TOPDIR)/rules.mk

PKG_NAME:=libtuya_link
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tuya/tuya-iot-core-sdk.git
PKG_SOURCE_VERSION:=HEAD
PKG_MIRROR_HASH:=skip

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

CMAKE_BINARY_SUBDIR:=build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON

define Build/Install
	# Upstream project has no install target.
endef

define Package/libtuya_link
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Tuya Link IoT Core SDK
  URL:=https://github.com/tuya/tuya-iot-core-sdk
  DEPENDS:=+libstdcpp +libatomic
endef

define Package/libtuya_link/description
 Tuya IoT Core SDK for OpenWrt
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/tuya_link
	$(INSTALL_DIR) $(1)/usr/lib

	
	cd $(PKG_BUILD_DIR) && \
    find . -type f -name '*.h' -exec $(CP) -r {} $(1)/usr/include/tuya_link/ \;


	$(CP) $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/lib/lib*.so* $(1)/usr/lib/
endef

define Package/libtuya_link/install
	$(INSTALL_DIR) $(1)/usr/lib

	# Install libraries
	$(CP) $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR)/lib/lib*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtuya_link))