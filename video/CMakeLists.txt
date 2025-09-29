# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.video-common)

    mcux_add_source(
        SOURCES fsl_video_common.h
                fsl_video_common.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()


if (CONFIG_MCUX_COMPONENT_driver.dc-fb-common)

    mcux_add_source(
        SOURCES display/dc/fsl_dc_fb.h
    )

    mcux_add_include(
        INCLUDES ./display/dc
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-common)

    mcux_add_source(
        SOURCES display/fsl_display.h
    )

    mcux_add_include(
        INCLUDES ./display
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.video-i2c)

    mcux_add_source(
        SOURCES i2c/fsl_video_i2c.h
                i2c/fsl_video_i2c.c
    )

    mcux_add_include(
        INCLUDES ./i2c
    )

    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.fbdev)

    mcux_add_source(
        SOURCES display/fbdev/fsl_fbdev.h
                display/fbdev/fsl_fbdev.c
    )

    mcux_add_include(
        INCLUDES ./display/fbdev/
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi)

    mcux_add_source(
        SOURCES display/dbi/fsl_dbi.h
                display/dbi/fsl_dbi.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dcif)

    mcux_add_source(
        SOURCES display/dbi/dcif/fsl_dbi_dcif.h
                display/dbi/dcif/fsl_dbi_dcif.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/dcif
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi_flexio_edma)

    mcux_add_source(
        SOURCES display/dbi/flexio/fsl_dbi_flexio_edma.h
                display/dbi/flexio/fsl_dbi_flexio_edma.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/flexio
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi_flexio_smartdma)

    mcux_add_source(
        SOURCES display/dbi/flexio/fsl_dbi_flexio_smartdma.h
                display/dbi/flexio/fsl_dbi_flexio_smartdma.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/flexio
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi_lcdic_dma)

    mcux_add_source(
        SOURCES display/dbi/lcdic/fsl_dbi_lcdic_dma.h
                display/dbi/lcdic/fsl_dbi_lcdic_dma.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/lcdic
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi_lcdif)

    mcux_add_source(
        SOURCES display/dbi/lcdif/fsl_dbi_lcdif.h
                display/dbi/lcdif/fsl_dbi_lcdif.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/lcdif
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dbi_lpc_spi_dma)

    mcux_add_source(
        SOURCES display/dbi/lpc_spi/fsl_dbi_spi_dma.h
                display/dbi/lpc_spi/fsl_dbi_spi_dma.c
    )

    mcux_add_include(
        INCLUDES ./display/dbi/lpc_spi
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-mipi-dsi-cmd)

    mcux_add_source(
        SOURCES display/mipi_dsi_cmd/fsl_mipi_dsi_cmd.h
                display/mipi_dsi_cmd/fsl_mipi_dsi_cmd.c
    )

    mcux_add_include(
        INCLUDES ./display/mipi_dsi_cmd
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-elcdif)

    mcux_add_source(
        SOURCES display/dc/elcdif/fsl_dc_fb_elcdif.h
                display/dc/elcdif/fsl_dc_fb_elcdif.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/elcdif
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-lcdif)

    mcux_add_source(
        SOURCES display/dc/lcdif/fsl_dc_fb_lcdif.h
                display/dc/lcdif/fsl_dc_fb_lcdif.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/lcdif
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-lcdifv2)

    mcux_add_source(
        SOURCES display/dc/lcdifv2/fsl_dc_fb_lcdifv2.h
                display/dc/lcdifv2/fsl_dc_fb_lcdifv2.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/lcdifv2
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-lcdifv3)

    mcux_add_source(
        SOURCES display/dc/lcdifv3/fsl_dc_fb_lcdifv3.h
                display/dc/lcdifv3/fsl_dc_fb_lcdifv3.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/lcdifv3
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-dcif)

    mcux_add_source(
        SOURCES display/dc/dcif/fsl_dc_fb_dcif.h
                display/dc/dcif/fsl_dc_fb_dcif.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/dcif
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-dcif_1)

    mcux_add_source(
        SOURCES display/dc/dcif_1/fsl_dc_fb_dcif.h
                display/dc/dcif_1/fsl_dc_fb_dcif.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/dcif_1
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-dbi)

    mcux_add_source(
        SOURCES display/dc/dbi/fsl_dc_fb_dbi.h
                display/dc/dbi/fsl_dc_fb_dbi.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/dbi
    )

    mcux_add_configuration(
        CC "-DMCUX_DBI_LEGACY=0"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-ssd1963)

    mcux_add_source(
        SOURCES display/dc/ssd1963/fsl_dc_fb_ssd1963.h
                display/dc/ssd1963/fsl_dc_fb_ssd1963.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/ssd1963
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.dc-fb-dsi-cmd)

    mcux_add_source(
        SOURCES display/dc/dsi_cmd/fsl_dc_fb_dsi_cmd.h
                display/dc/dsi_cmd/fsl_dc_fb_dsi_cmd.c
    )

    mcux_add_include(
        INCLUDES ./display/dc/dsi_cmd
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-hx8394)

    mcux_add_source(
        SOURCES display/hx8394/fsl_hx8394.h
                display/hx8394/fsl_hx8394.c
    )

    mcux_add_include(
        INCLUDES ./display/hx8394
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rm68191)

    mcux_add_source(
        SOURCES display/rm68191/fsl_rm68191.h
                display/rm68191/fsl_rm68191.c
    )

    mcux_add_include(
        INCLUDES ./display/rm68191
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rm68200)

    mcux_add_source(
        SOURCES display/rm68200/fsl_rm68200.h
                display/rm68200/fsl_rm68200.c
    )

    mcux_add_include(
        INCLUDES ./display/rm68200
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rm67191)

    mcux_add_source(
        SOURCES display/rm67191/fsl_rm67191.h
                display/rm67191/fsl_rm67191.c
    )

    mcux_add_include(
        INCLUDES ./display/rm67191
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rm67162)

    mcux_add_source(
        SOURCES display/rm67162/fsl_rm67162.h
                display/rm67162/fsl_rm67162.c
    )

    mcux_add_include(
        INCLUDES ./display/rm67162
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-co5300)

    mcux_add_source(
        SOURCES display/co5300/fsl_co5300.h
                display/co5300/fsl_co5300.c
    )

    mcux_add_include(
        INCLUDES ./display/co5300
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rm692c9)

    mcux_add_source(
        SOURCES display/rm692c9/fsl_rm692c9.h
                display/rm692c9/fsl_rm692c9.c
    )

    mcux_add_include(
        INCLUDES ./display/rm692c9
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-rpi)

    mcux_add_source(
        SOURCES display/rpi/fsl_rpi.h
                display/rpi/fsl_rpi.c
    )

    mcux_add_include(
        INCLUDES ./display/rpi
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-adv7535)

    mcux_add_source(
        SOURCES display/adv7535/fsl_adv7535.h
                display/adv7535/fsl_adv7535.c
    )

    mcux_add_include(
        INCLUDES ./display/adv7535
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-it6161)

    mcux_add_source(
        SOURCES display/it6161/fsl_it6161.h
                display/it6161/fsl_it6161.c
                display/it6161/hdmi_tx.h
                display/it6161/hdmi_tx.c
                display/it6161/mipi_rx.h
                display/it6161/mipi_rx.c
    )

    mcux_add_include(
        INCLUDES ./display/it6161
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-it6263)

    mcux_add_source(
        SOURCES display/it6263/fsl_it6263.h
                display/it6263/fsl_it6263.c
    )

    mcux_add_include(
        INCLUDES ./display/it6263
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.display-sn65dsi83)

    mcux_add_source(
        SOURCES display/sn65dsi83/fsl_sn65dsi83.h
                display/sn65dsi83/fsl_sn65dsi83.c
    )

    mcux_add_include(
        INCLUDES ./display/sn65dsi83
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-common)

    mcux_add_source(
        SOURCES camera/fsl_camera.h
    )

    mcux_add_include(
        INCLUDES ./camera
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-common)

    mcux_add_source(
        SOURCES camera/device/fsl_camera_device.h
    )

    mcux_add_include(
        INCLUDES ./camera/device
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-sccb)

    mcux_add_source(
        SOURCES camera/device/sccb/fsl_sccb.h
                camera/device/sccb/fsl_sccb.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/sccb
    )

    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-ov5640)

    mcux_add_source(
        SOURCES camera/device/ov5640/fsl_ov5640.h
                camera/device/ov5640/fsl_ov5640.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/ov5640
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-ov7670)

    mcux_add_source(
        SOURCES camera/device/ov7670/fsl_ov7670.h
                camera/device/ov7670/fsl_ov7670.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/ov7670
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-ov7725)

    mcux_add_source(
        SOURCES camera/device/ov7725/fsl_ov7725.h
                camera/device/ov7725/fsl_ov7725.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/ov7725
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-mt9m114)

    mcux_add_source(
        SOURCES camera/device/mt9m114/fsl_mt9m114.h
                camera/device/mt9m114/fsl_mt9m114.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/mt9m114
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-max9286)

    mcux_add_source(
        SOURCES camera/device/max9286/fsl_max9286.h
                camera/device/max9286/fsl_max9286.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/max9286
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-device-ap1302)

    mcux_add_source(
        SOURCES camera/device/ap1302/fsl_ap1302.h
                camera/device/ap1302/fsl_ap1302.c
    )

    mcux_add_include(
        INCLUDES ./camera/device/ap1302
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-receiver-common)

    mcux_add_source(
        SOURCES camera/receiver/fsl_camera_receiver.h
    )

    mcux_add_include(
        INCLUDES ./camera/receiver
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-receiver-csi)

    mcux_add_source(
        SOURCES camera/receiver/csi/fsl_csi_camera_adapter.h
                camera/receiver/csi/fsl_csi_camera_adapter.c
    )

    mcux_add_include(
        INCLUDES ./camera/receiver/csi
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-receiver-isi)

    mcux_add_source(
        SOURCES camera/receiver/isi/fsl_isi_camera_adapter.h
                camera/receiver/isi/fsl_isi_camera_adapter.c
    )

    mcux_add_include(
        INCLUDES ./camera/receiver/isi
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.camera-receiver-flexio)

    mcux_add_source(
        SOURCES camera/receiver/flexio/fsl_flexio_camera_adapter.h
                camera/receiver/flexio/fsl_flexio_camera_adapter.c
    )

    mcux_add_include(
        INCLUDES ./camera/receiver/flexio
    )

endif()
