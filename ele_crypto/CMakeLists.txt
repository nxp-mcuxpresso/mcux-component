# Copyright 2024-2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.ele_crypto)

    # Common headers always included regardless of variant
    mcux_add_source(
        SOURCES ele_crypto.h
                ele_crypto_internal.h
    )
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_macro(
        "-DCACHE_MODE_WRITE_THROUGH=1"
    )

    # Common functions: ele_crypto.c compiled for both S400 and S100.
    # Contains: session/key management, Hash, RNG, Open/Close services,
    # ELE_Mac, ELE_Cipher, ELE_Sign, ELE_Verify (S400 versions guarded
    # with #if !defined(FSL_FEATURE_ELE_IS_S100)).
    mcux_add_source(
        SOURCES ele_crypto.c
    )

    # S400 backend (RT1180) - S400-specific crypto functions
    if (CONFIG_MCUX_COMPONENT_component.ele_crypto_s400)
        mcux_add_source(
            SOURCES ele_crypto_s400.c
        )
    endif()

    # S100 backend (RT2660) - S100-specific crypto functions
    if (CONFIG_MCUX_COMPONENT_component.ele_crypto_s100)
        mcux_add_source(
            SOURCES ele_crypto_s100.c
                    ele_crypto_s100_internal.h
        )
        mcux_add_macro(
            "-DFSL_FEATURE_ELE_IS_S100=1"
        )
    endif()

endif()

if (CONFIG_MCUX_COMPONENT_component.ele_nvm_manager)
    mcux_add_source(
        SOURCES ele_nvm_manager.c
                ele_nvm_manager.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
