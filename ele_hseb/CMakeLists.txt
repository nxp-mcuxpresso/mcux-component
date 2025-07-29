# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.ele_hseb_mu_driver)
    mcux_add_source(
        SOURCES drivers/mu/hse_mu.c
                drivers/mu/hse_mu.h
    )
    mcux_add_include(
        INCLUDES ./
                 drivers/mu
                 BASE_PATH ${SdkRootDirPath}/components/ele_hseb/
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ele_hseb_framework)
    mcux_add_source(
        SOURCES framework/host_crypto_helper/hse_common_test.h
                framework/host_crypto_helper/hse_host_aead.c
                framework/host_crypto_helper/hse_host_aead.h
                framework/host_crypto_helper/hse_host_attrs.c
                framework/host_crypto_helper/hse_host_attrs.h
                framework/host_crypto_helper/hse_host_cipher.c
                framework/host_crypto_helper/hse_host_cipher.h
                framework/host_crypto_helper/hse_host_cmac_with_counter.c
                framework/host_crypto_helper/hse_host_cmac_with_counter.h
                framework/host_crypto_helper/hse_host_ecc.c
                framework/host_crypto_helper/hse_host_ecc.h
                framework/host_crypto_helper/hse_host_format_key_catalogs.c
                framework/host_crypto_helper/hse_host_format_key_catalogs.h
                framework/host_crypto_helper/hse_host_hash.c
                framework/host_crypto_helper/hse_host_hash.h
                framework/host_crypto_helper/hse_host_impex_stream.c
                framework/host_crypto_helper/hse_host_impex_stream.h
                framework/host_crypto_helper/hse_host_import_key.c
                framework/host_crypto_helper/hse_host_import_key.h
                framework/host_crypto_helper/hse_host_kdf.c
                framework/host_crypto_helper/hse_host_kdf.h
                framework/host_crypto_helper/hse_host_mac.c
                framework/host_crypto_helper/hse_host_mac.h
                framework/host_crypto_helper/hse_host_monotonic_counters.c
                framework/host_crypto_helper/hse_host_monotonic_counters.h
                framework/host_crypto_helper/hse_host_rng.c
                framework/host_crypto_helper/hse_host_rng.h
                framework/host_crypto_helper/hse_host_rsa_cipher.c
                framework/host_crypto_helper/hse_host_rsa_cipher.h
                framework/host_crypto_helper/hse_host_sign.c
                framework/host_crypto_helper/hse_host_sign.h
                framework/host_crypto_helper/hse_host_utils.c
                framework/host_crypto_helper/hse_host_utils.h
                framework/host_hse/host_compiler_api.h
                framework/host_hse/hse_host.c
                framework/host_hse/hse_host.h
                framework/host_hse/hse_tracing.h
                framework/host_hse/hse_b/hse_default_config.h
                framework/host_keymgmt/hse_keys_allocator.c
                framework/host_keymgmt/hse_keys_allocator.h
    )
    mcux_add_include(
        INCLUDES ./
                 framework/host_crypto_helper
                 framework/host_hse
                 framework/host_hse/hse_b
                 framework/host_keymgmt
                 BASE_PATH ${SdkRootDirPath}/components/ele_hseb/
    )
endif()


if (CONFIG_MCUX_COMPONENT_component.ele_hseb_interface)
    mcux_add_source(
        SOURCES interface/hse_gpr_status.h
                interface/hse_interface.h
                interface/hse_srv_responses.h
                interface/hse_status_and_errors.h
                interface/std_typedefs.h
                interface/config/hse_b_config.h
                interface/config/hse_compile_defs.h
                interface/config/hse_compiler_abs.h
                interface/config/hse_platform.h
                interface/config/hse_target.h
                interface/inc_common/hse_common_types.h
                interface/inc_common/hse_defs.h
                interface/inc_common/hse_keymgmt_common_types.h
                interface/inc_services/hse_srv_aead.h
                interface/inc_services/hse_srv_attr.h
                interface/inc_services/hse_srv_bootdatasig.h
                interface/inc_services/hse_srv_cmac_with_counter.h
                interface/inc_services/hse_srv_combined_auth_enc.h
                interface/inc_services/hse_srv_crc32.h
                interface/inc_services/hse_srv_firmware_update.h
                interface/inc_services/hse_srv_hash.h
                interface/inc_services/hse_srv_ipsec.h
                interface/inc_services/hse_srv_key_derive.h
                interface/inc_services/hse_srv_key_generate.h
                interface/inc_services/hse_srv_key_import_export.h
                interface/inc_services/hse_srv_key_mgmt_utils.h
                interface/inc_services/hse_srv_mac.h
                interface/inc_services/hse_srv_monotonic_cnt.h
                interface/inc_services/hse_srv_msc_key_mgmt.h
                interface/inc_services/hse_srv_otfad_install.h
                interface/inc_services/hse_srv_publish_sys_img.h
                interface/inc_services/hse_srv_random.h
                interface/inc_services/hse_srv_rsa_cipher.h
                interface/inc_services/hse_srv_sbaf_update.h
                interface/inc_services/hse_srv_self_test.h
                interface/inc_services/hse_srv_she_cmds.h
                interface/inc_services/hse_srv_sign.h
                interface/inc_services/hse_srv_siphash.h
                interface/inc_services/hse_srv_smr_install.h
                interface/inc_services/hse_srv_sym_cipher.h
                interface/inc_services/hse_srv_sys_authorization.h
                interface/inc_services/hse_srv_tmu_reg_config.h
                interface/inc_services/hse_srv_utils.h
    )
    mcux_add_include(
        INCLUDES ./
                 interface
                 interface/config
                 interface/inc_common
                 interface/inc_services
                 BASE_PATH ${SdkRootDirPath}/components/ele_hseb/
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ele_hseb_services)
    mcux_add_source(
        SOURCES services/inc/global_defs.h
                services/inc/hse_config.h
                services/inc/hse_demo_app_services.h
                services/inc/hse_global_types.h
                services/inc/hse_global_variables.h
                services/inc/hse_host_boot.h
                services/inc/hse_host_common.h
                services/inc/hse_host_flashSrv.h
                services/inc/hse_host_sys_authorization.h
                services/inc/hse_host_test.h
                services/inc/hse_host_wrappers.h
                services/inc/hse_memory_update_protocol.h
                services/inc/hse_monotonic_cnt.h
                services/inc/hse_sbaf_update.h
                services/inc/hse_she_api.h
                services/inc/hse_she_commands.h
                services/inc/standard/hse_b_catalog_formatting.h
                services/src/common/hse_config.c
                services/src/common/hse_global_variables.c
                services/src/fw_crypto/hse_crypto.c
                services/src/fw_crypto/inc/hse_aead_example.h
                services/src/fw_crypto/inc/hse_aes_example.h
                services/src/fw_crypto/inc/hse_BD_example.h
                services/src/fw_crypto/inc/hse_ecc_example.h
                services/src/fw_crypto/inc/hse_fastcmac_example.h
                services/src/fw_crypto/inc/hse_get_key_info.h
                services/src/fw_crypto/inc/hse_hash_example.h
                services/src/fw_crypto/inc/hse_host_Kdf_examples.h
                services/src/fw_crypto/inc/hse_host_km_import_key.h
                services/src/fw_crypto/inc/hse_host_km_utils.h
                services/src/fw_crypto/inc/hse_host_msc_km.h
                services/src/fw_crypto/inc/hse_publish_keystore_RamToFlash_example.h
                services/src/fw_crypto/inc/hse_rsa_example.h
                services/src/fw_crypto/inc/hse_sessionkeys_example.h
                services/src/fw_crypto/inc/hse_sys_auth_example.h
                services/src/fw_crypto/inc/hse_UpdateNvmKeys_example.h
                services/src/fw_crypto/src/hse_aead_example.c
                services/src/fw_crypto/src/hse_aes_example.c
                services/src/fw_crypto/src/hse_BurchesmesterDesmt_example.c
                services/src/fw_crypto/src/hse_ecc_example.c
                services/src/fw_crypto/src/hse_fastcmac_example.c
                services/src/fw_crypto/src/hse_get_key_info.c
                services/src/fw_crypto/src/hse_hash_example.c
                services/src/fw_crypto/src/hse_host_Kdf_examples.c
                services/src/fw_crypto/src/hse_host_km_import_key.c
                services/src/fw_crypto/src/hse_host_km_utils.c
                services/src/fw_crypto/src/hse_host_sys_authorization.c
                services/src/fw_crypto/src/hse_publish_keystore_RamToFlash_example.c
                services/src/fw_crypto/src/hse_rsa_example.c
                services/src/fw_crypto/src/hse_sessionkeys_example.c
                services/src/fw_crypto/src/hse_sys_auth_example.c
                services/src/fw_crypto/src/hse_UpdateNvmKeys_example.c
                services/src/hse_b_catalog_formatting.c
                services/src/hse_demo_app_config.c
                services/src/monotonic_cnt/hse_monotonic_cnt.c
                services/src/sbaf_update/sbaf_update.c
                services/src/shecommandapp/hse_host_common.c
                services/src/shecommandapp/hse_host_test.c
                services/src/shecommandapp/hse_host_wrappers.c
                services/src/shecommandapp/hse_memory_update_protocol.c
                services/src/shecommandapp/hse_she_command_main.c
                services/src/shecommandapp/hse_she_commands.c
                services/src/standard/hse_fwType_config.c
                services/src/utest/hse_utest.c
    )
    mcux_add_include(
        INCLUDES ./
                 services/inc
                 services/inc/standard
                 services/src/fw_crypto/inc/
                 BASE_PATH ${SdkRootDirPath}/components/ele_hseb/
    )
endif()