# Power Manager Enablement for RT1170

The power manager for RT1170 is an essential component that integrates many features of RT1170's power architecture. It manages the power states of a system and provides a wide range of options to optimize power consumption. According to NXP's official datasheet, the power manager provides up to 17 typical system power states, each with its own unique set of features and capabilities.

The RT1170 PM covers the following features:

1. On/Off clock sources, including 29 oscillators or PLLs.
2. Power on/off power domains, including CM7 domain, CM4 domain, MEGA domain, DISPLAY domain, WAKEUP domain, and LPSR domain.
3. Enable/disable power supplies, including DCDC and LDOs.
4. Enable/disable system STANDBY request.
5. Enable/disable bandgap and body biasing.
6. Control wakeup generator (optional).

Based on the features listed above, it is clear that the power manager controls the ON/OFF status of the system's basic resources (clock source, power domain, power supply). Please note that clock roots, clock gates and clock groups are not controlled by the power manager; it is the application's responsibility to manage them.

## Resource Constraints

It is important to note that the operation modes of basic resources can vary depending on the power state. For example, in low power states, certain resources may be disabled or put into sleep mode to conserve energy. The table below outlines the differences in operation modes between different power states. By carefully selecting the appropriate operation mode for basic resources, you can ensure optimal energy efficiency.

| Resource | Setpoint0 | SetPoint1 | Setpoint2 | Setpoint3 | Setpoint4 | Setpoint5 | Setpoint6 | Setpoint7 | Setpoint8 | Setpoint9 | Setpoint10 | Setpoint11 | Setpoint12 | Setpoint13 | Setpoint14 | Setpoint15 | SNVS |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| CM7 Platform | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | OFF |
| CM4 Platform | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | RUN/STOP/WAIT/SUSPEND | OFF |
| MEGA Domain | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| WAKUP Domain | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| DISPLAY Domain | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| LPSR Domain | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF |
| OSC_RC_16M | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON |
| OSC_RC_48M  | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| OSC_RC_48M_DIV2 | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| OSC_RC_400M | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF | OFF |
| OSC_24M | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| OSC_24M_CLK | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| ARM_PLL | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| ARM_PLL_CLK | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2_CLK | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2_PFD0 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2_PFD1 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2_PFD2 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL2_PFD3 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_CLK | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_DIV2 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_PFD0 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_PFD1 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_PFD2 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL3_PFD3 | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL1 | OFF | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL1_DIV2 | OFF | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SYS_PLL1_DIV5 | OFF | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| AUDIO PLL | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| AUDIO_PLL_CLK | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| VIDEO_PLL | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| VIDEO_PLL_CLK | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| DCDC | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| LPSR_ANA_LDO | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | ON | ON | ON | ON | ON | OFF |
| LPSR_DIG_LDO | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | ON | ON | ON | ON | ON | OFF |
| PLL_LDO | ON | ON | ON | ON | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| M7_FBB | OFF | ON | ON | ON | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
| SOC_RBB | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | ON | OFF | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| LPSR_RBB | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF | ON | OFF | ON | OFF | OFF | OFF | OFF | OFF | OFF |
| STANDBY_REQ | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | ON/OFF | OFF |

Three of the resources in question are optional in different power states: the CM7 domain's operation mode, the CM4 domain's operation mode, and standby request.

The CPU domain can operate in different modes based on application settings:

- RUN: The CPU core is active and running under normal operation. All blocks inside the CPU platform can be accessed when needed.
- WAIT: The CPU is in the WFI/WFE state, but can return to RUN mode with very short latency. In typical applications, the CPU enters WAIT mode whenever there is no active thread running. In WAIT mode, the clock to the CPU core is gated off, the cache is clock gated, and the TCM is still active since there are other modules, such as DMA, that still need access to it.
- STOP: The CPU is in the WFI/WFE state, but can return to RUN mode with very short latency. In typical applications, the CPU enters STOP mode whenever there is no active thread running. In STOP mode, the clock to the CPU core is gated off, the cache is clock gated, and the TCM is still active since there are other modules, such as DMA, that still need access to it.
- SUSPEND: Entering SUSPEND mode results in the lowest power consumption, and exit time is not critical. In SUSPEND mode, the CPU, cache, and peripherals are all power gated. The biggest difference between SUSPEND and STOP mode is that the peripherals are power gated. Because the peripherals are power gated, entering SUSPEND mode requires the CPU to save the state of the peripherals. When exiting from SUSPEND mode, the CPU needs to restore these states.

Users can also choose to request standby or not in different low power states. Standby mode is a low-power mode that has distinct settings outside of CPU mode and setpoint mode. After asserting standby request, **BANDGAP**, **M7_FBB**, **SOC_RBB**, **LPSR_RBB**, **PLL_LDO**, **LPSR_ANA_LDO**, and **LPSR_DIG_LDO** will enter standby mode. By default, the STANDBY request is asserted. Users can de-assert standby request by setting corresponding resource constraints.

Based on the table above, NXP provides macros that allow users to transfer the basic resources' operating mode to the power manager.

| Marco of clock sources’ constraints |
| --- |
| PM_RESC_OSC_RC_16M_ON                    PM_RESC_OSC_RC_16M_OFF |
| PM_RESC_OSC_RC_48M_ON                     PM_RESC_OSC_RC_48M_OFF |
| PM_RESC_OSC_RC_48M_DIV2_ON                PM_RESC_OSC_RC_48M_DIV2_OFF |
| PM_RESC_OSC_RC_400M_ON                    PM_RESC_OSC_RC_400M_OFF |
| PM_RESC_OSC_RC_24M_ON                     PM_RESC_OSC_RC_24M_OFF |
| PM_RESC_OSC_RC_24M_CLK_ON                 PM_RESC_OSC_RC_24M_CLK_OFF |
| PM_RESC_ARM_PLL_ON                        PM_RESC_ARM_PLL_OFF |
| PM_RESC_ARM_PLL_CLK_ON                    PM_RESC_ARM_PLL_CLK_OFF |
| PM_RESC_SYS_PLL2_ON                       PM_RESC_SYS_PLL2_OFF |
| PM_RESC_SYS_PLL2_CLK_ON                   PM_RESC_SYS_PLL2_CLK_OFF |
| PM_RESC_SYS_PLL2_PFD0_ON                  PM_RESC_SYS_PLL2_PFD0_OFF |
| PM_RESC_SYS_PLL2_PFD1_ON                  PM_RESC_SYS_PLL2_PFD1_OFF |
| PM_RESC_SYS_PLL2_PFD2_ON                  PM_RESC_SYS_PLL2_PFD2_OFF |
| PM_RESC_SYS_PLL2_PFD3_ON                  PM_RESC_SYS_PLL2_PFD3_OFF |
| PM_RESC_SYS_PLL3_ON                       PM_RESC_SYS_PLL3_OFF |
| PM_RESC_SYS_PLL3_CLK_ON                   PM_RESC_SYS_PLL3_CLK_OFF |
| PM_RESC_SYS_PLL3_DVI2_ON                  PM_RESC_SYS_PLL3_DVI2_OFF |
| PM_RESC_SYS_PLL3_PFD0_ON                  PM_RESC_SYS_PLL3_PFD0_OFF |
| PM_RESC_SYS_PLL3_PFD1_ON                  PM_RESC_SYS_PLL3_PFD1_OFF |
| PM_RESC_SYS_PLL3_PFD2_ON                  PM_RESC_SYS_PLL3_PFD2_OFF |
| PM_RESC_SYS_PLL3_PFD3_ON                  PM_RESC_SYS_PLL3_PFD3_OFF |
| PM_RESC_SYS_PLL1_ON                       PM_RESC_SYS_PLL1_OFF |
| PM_RESC_SYS_PLL1_CLK_ON                   PM_RESC_SYS_PLL1_CLK_OFF |
| PM_RESC_SYS_PLL1_DIV2_ON                  PM_RESC_SYS_PLL1_DIV2_OFF |
| PM_RESC_SYS_PLL1_DIV5_ON                  PM_RESC_SYS_PLL1_DIV5_OFF |
| PM_RESC_AUDIO_PLL_ON                      PM_RESC_AUDIO_PLL_OFF |
| PM_RESC_AUDIO_PLL_CLK_ON                  PM_RESC_AUDIO_PLL_CLK_OFF |
| PM_RESC_VIDEO_PLL_ON                      PM_RESC_VIDEO_PLL_OFF |
| PM_RESC_VIDEO_PLL_CLK_ON                  PM_RESC_VIDEO_PLL_CLK_OFF |

| Marco of Power Domains’ constraints |
| --- |
| PM_RESC_MEGA_MIX_ON        PM_RESC_MEGA_MIX_OFF |
| PM_RESC_DISPLAY_MIX_ON           PM_RESC_DISPLAY_MIX_OFF |
| PM_RESC_WAKEUP_MIX_ON            PM_RESC_WAKEUP_MIX_OFF |
| PM_RESC_LPSR_MIX_ON              PM_RESC_LPSR_MIX_OFF  |

| Marco of core domain constraints |
| --- |
| PM_RESC_CORE_DOMAIN_RUN  |
| PM_RESC_CORE_DOMAIN_WAIT |
| PM_RESC_CORE_DOMAIN_STOP |
| PM_RESC_CORE_DOMAIN_SUSPEND |

| Marco of power supplies’ constraints |
| --- |
| PM_RESC_DCDC_ON                PM_RESC_DCDC_OFF |
| PM_RESC_LPSR_ANA_LDO_ON         PM_RESC_LPSR_ANA_LDO_OFF |
| PM_RESC_LPSR_DIG_LDO_ON         PM_RESC_LPSR_DIG_LDO_OFF |
| PM_RESC_BANDGAP_ON              PM_RESC_BANDGAP_OFF |
| PM_RESC_FBB_M7_ON               PM_RESC_FBB_M7_OFF |
| PM_RESC_RBB_SOC_ON              PM_RESC_RBB_SOC_OFF |
| PM_RESC_RBB_LPSR_ON             PM_RESC_RBB_LPSR_OFF |
| PM_RESC_PLL_LDO_ON              PM_RESC_PLL_LDO_OFF |
| PM_RESC_STANDBY_REQ_ON         PM_RESC_STANDBY_REQ_OFF |

## Power State Constraints

The RT1170 provides 17 different power states, each allowing for varying levels of power consumption. Users can set the lowest power state that the application is allowed to use by creating a power state constraint. This constraint ensures that the application is operating at the most energy-efficient level possible, ultimately helping to extend the device's battery life.

```c
PM_LP_STATE_SP0
PM_LP_STATE_SP1
PM_LP_STATE_SP2
PM_LP_STATE_SP3
PM_LP_STATE_SP4
PM_LP_STATE_SP5
PM_LP_STATE_SP6
PM_LP_STATE_SP7
PM_LP_STATE_SP8
PM_LP_STATE_SP9
PM_LP_STATE_SP10
PM_LP_STATE_SP11
PM_LP_STATE_SP12
PM_LP_STATE_SP13
PM_LP_STATE_SP14
PM_LP_STATE_SP15
PM_LP_STATE_SNVS
```

## Wakeup Source ID

The power manager framework also allows you to configure the RT1170's wakeup generator (in the GPC module). This covers all wakeup sources except for **debug_wakeup** and the **wakeup event**.

It is important to note that the power manager framework only enables interrupts in the NVIC and unmasks related interrupts in the GPC. It is the responsibility of the application to set and enable peripheral interrupts.

```c
// Following wakeup source avaiable for dual cores.
PM_WSID_DMA0_DMA16_IRQ
PM_WSID_DMA1_DMA16_IRQ
PM_WSID_DMA2_DMA18_IRQ
PM_WSID_DMA3_DMA19_IRQ
PM_WSID_DMA4_DMA20_IRQ
PM_WSID_DMA5_DMA21_IRQ
PM_WSID_DMA6_DMA22_IRQ
PM_WSID_DMA7_DMA23_IRQ
PM_WSID_DMA8_DMA24_IRQ
PM_WSID_DMA9_DMA25_IRQ
PM_WSID_DMA10_DMA26_IRQ
PM_WSID_DMA11_DMA27_IRQ
PM_WSID_DMA12_DMA28_IRQ
PM_WSID_DMA13_DMA29_IRQ
PM_WSID_DMA14_DMA30_IRQ
PM_WSID_DMA15_DMA31_IRQ
PM_WSID_DMA_ERROR_IRQ
PM_WSID_CORE_IRQ
PM_WSID_LPUART1_IRQ
PM_WSID_LPUART2_IRQ
PM_WSID_LPUART3_IRQ
PM_WSID_LPUART4_IRQ
PM_WSID_LPUART5_IRQ
PM_WSID_LPUART6_IRQ
PM_WSID_LPUART7_IRQ
PM_WSID_LPUART8_IRQ
PM_WSID_LPUART9_IRQ
PM_WSID_LPUART10_IRQ
PM_WSID_LPUART11_IRQ
PM_WSID_LPUART12_IRQ
PM_WSID_LPI2C1_IRQ
PM_WSID_LPI2C2_IRQ
PM_WSID_LPI2C3_IRQ
PM_WSID_LPI2C4_IRQ
PM_WSID_LPI2C5_IRQ
PM_WSID_LPI2C6_IRQ
PM_WSID_LPSPI1_IRQ
PM_WSID_LPSPI2_IRQ
PM_WSID_LPSPI3_IRQ
PM_WSID_LPSPI4_IRQ
PM_WSID_LPSPI5_IRQ
PM_WSID_LPSPI6_IRQ
PM_WSID_CAN1_IRQ
PM_WSID_CAN1_ERROR_IRQ
PM_WSID_CAN2_IRQ
PM_WSID_CAN2_ERROR_IRQ
PM_WSID_CAN3_IRQ
PM_WSID_CAN3_ERROR_IRQ
PM_WSID_KPP_IRQ
PM_WSID_GPR_IRQ
PM_WSID_ELCDIF_IRQ
PM_WSID_LCDIFV2_IRQ
PM_WSID_CSI_IRQ
PM_WSID_PXP_IRQ
PM_WSID_MIPI_CSI_IRQ
PM_WSID_MIPI_DSI_IRQ
PM_WSID_GPU2D_IRQ
PM_WSID_DAC_IRQ
PM_WSID_KEY_MANAGER_IRQ
PM_WSID_WDOG2_IRQ
PM_WSID_SNVS_HP_NON_TZ_IRQ
PM_WSID_SNVS_HP_TZ_IRQ
PM_WSID_SNVS_PLUSE_EVENT_IRQ
PM_WSID_CAAM_JQ0_IRQ
PM_WSID_CAAM_JQ1_IRQ
PM_WSID_CAAM_JQ2_IRQ
PM_WSID_CAAM_JQ3_IRQ
PM_WSID_CAAM_RECORVE_ERROR_IRQ
PM_WSID_CAAM_RTIC_IRQ
PM_WSID_CDOG_IRQ
PM_WSID_SAI1_IRQ
PM_WSID_SAI2_IRQ
PM_WSID_SAI3_RX_IRQ
PM_WSID_SAI3_TX_IRQ
PM_WSID_SAI4_RX_IRQ
PM_WSID_SAI4_TX_IRQ
PM_WSID_SPDIF_IRQ
PM_WSID_TMPSNS_IRQ
PM_WSID_TMPSNS_LOW_HIGH_IRQ
PM_WSID_TMPSNS_PANIC_IRQ
PM_WSID_LPSR_LP8_BROWNOUT_IRQ
PM_WSID_LPSR_LP0_BROWNOUT_IRQ
PM_WSID_ADC1_IRQ
PM_WSID_ADC2_IRQ
PM_WSID_USBPHY1_IRQ
PM_WSID_USBPHY2_IRQ
PM_WSID_RDC_IRQ
PM_WSID_GPIO13_COMBINED_0_31_IRQ
PM_WSID_DCIC1_IRQ
PM_WSID_DCIC2_IRQ
PM_WSID_ASRC_IRQ
PM_WSID_FLEXRAM_ECC_IRQ
PM_WSID_GPIO1_COMBINED_0_15_IRQ
PM_WSID_GPIO1_COMBINED_16_31_IRQ
PM_WSID_GPIO2_COMBINED_0_15_IRQ
PM_WSID_GPIO2_COMBINED_16_31_IRQ
PM_WSID_GPIO3_COMBINED_0_15_IRQ
PM_WSID_GPIO3_COMBINED_16_31_IRQ
PM_WSID_GPIO4_COMBINED_0_15_IRQ
PM_WSID_GPIO4_COMBINED_16_31_IRQ
PM_WSID_GPIO5_COMBINED_0_15_IRQ
PM_WSID_GPIO5_COMBINED_16_31_IRQ
PM_WSID_FLEXIO1_IRQ
PM_WSID_FLEXIO2_IRQ
PM_WSID_WDOG1_IRQ
PM_WSID_EWM_IRQ
PM_WSID_OCOTP_READ_FUSE_ERROR_IRQ
PM_WSID_OCOTP_READ_DONE_ERROR_IRQ
PM_WSID_GPC_IRQ
PM_WSID_GPT1_IRQ
PM_WSID_GPT2_IRQ
PM_WSID_GPT3_IRQ
PM_WSID_GPT4_IRQ
PM_WSID_GPT5_IRQ
PM_WSID_GPT6_IRQ
PM_WSID_PWM1_0_IRQ
PM_WSID_PWM1_1_IRQ
PM_WSID_PWM1_2_IRQ
PM_WSID_PWM1_3_IRQ
PM_WSID_PWM1_FAULT_IRQ
PM_WSID_FLEXSPI1_IRQ
PM_WSID_FLEXSPI2_IRQ
PM_WSID_SEMC_IRQ
PM_WSID_USDHC1_IRQ
PM_WSID_USDHC2_IRQ
PM_WSID_USB_OTG2_IRQ
PM_WSID_USB_OTG1_IRQ
PM_WSID_ENET_IRQ
PM_WSID_ENET_1588_TIMER_IRQ
PM_WSID_ENET_1G_MAC0_TX_RX_1_IRQ
PM_WSID_ENET_1G_MAC0_TX_RX_2_IRQ
PM_WSID_ENET_1G_IRQ
PM_WSID_ENET_1G_1588_TIMER_IRQ
PM_WSID_XBAR1_0_1_IRQ
PM_WSID_XBAR1_2_3_IRQ
PM_WSID_ADC_ETC_0_IRQ
PM_WSID_ADC_ETC_1_IRQ
PM_WSID_ADC_ETC_2_IRQ
PM_WSID_ADC_ETC_3_IRQ
PM_WSID_ADC_ETC_ERROR_IRQ
PM_WSID_PIT1_IRQ
PM_WSID_PIT2_IRQ
PM_WSID_ACMP1_IRQ
PM_WSID_ACMP2_IRQ
PM_WSID_ACMP3_IRQ
PM_WSID_ACMP4_IRQ
PM_WSID_ENC1_IRQ
PM_WSID_ENC2_IRQ
PM_WSID_ENC3_IRQ
PM_WSID_ENC4_IRQ
PM_WSID_TMR1_IRQ
PM_WSID_TMR2_IRQ
PM_WSID_TMR3_IRQ
PM_WSID_TMR4_IRQ
PM_WSID_SEMA4_CP0_IRQ
PM_WSID_SEMA4_CP1_IRQ
PM_WSID_PWM2_0_IRQ
PM_WSID_PWM2_1_IRQ
PM_WSID_PWM2_2_IRQ
PM_WSID_PWM2_3_IRQ
PM_WSID_PWM2_FAULT_IRQ
PM_WSID_PWM3_0_IRQ
PM_WSID_PWM3_1_IRQ
PM_WSID_PWM3_2_IRQ
PM_WSID_PWM3_3_IRQ
PM_WSID_PWM3_FAULT_IRQ
PM_WSID_PWM4_0_IRQ
PM_WSID_PWM4_1_IRQ
PM_WSID_PWM4_2_IRQ
PM_WSID_PWM4_3_IRQ
PM_WSID_PWM4_FAULT_IRQ
PM_WSID_PDM_HWVAD_EVENT_IRQ
PM_WSID_PDM_HWVAD_ERROR_IRQ
PM_WSID_PDM_EVENT_IRQ
PM_WSID_PDM_ERROR_IRQ
PM_WSID_EMVSIM1_IRQ
PM_WSID_EMVSIM2_IRQ
PM_WSID_MECC1_IRQ
PM_WSID_MECC1_FATAL_IRQ
PM_WSID_MECC2_IRQ
PM_WSID_MECC2_FATAL_IRQ
PM_WSID_XECC_FLEXSPI1_IRQ
PM_WSID_XECC_FLEXSPI1_FATAL_IRQ
PM_WSID_XECC_FLEXSPI2_IRQ
PM_WSID_XECC_FLEXSPI2_FATAL_IRQ
PM_WSID_XECC_SEMC_IRQ
PM_WSID_XECC_SEMC_FATAL_IRQ
PM_WSID_ENET_QOS_IRQ
PM_WSID_ENET_QOS_PMT_IRQ
// Following wakeup source only availaboe for CM7.
#if __CORTEX_M == 7
PM_WSID_GPIO6_COMBINED_0_15_IRQ
PM_WSID_GPIO6_COMBINED_16_31_IRQ
PM_WSID_CM7_GPIO2_3_IRQ
PM_WSID_RTWDOG3_IRQ
PM_WSID_MUA_IRQ
PM_WSID_CTI_TRIGGER_OUT0_IRQ
PM_WSID_CTI_TRIGGER_OUT1_IRQ
PM_WSID_FLEXRAM_IRQ
#endif
// Following wakeup source only availaboe for CM4.
#if __CORTEX_M == 4
PM_WSID_GPIO12_COMBINED_0_15_IRQ
PM_WSID_GPIO12_COMBINED_16_31_IRQ
PM_WSID_GPIO7_8_9_10_11_IRQ
PM_WSID_RTWDOG4_IRQ
PM_WSID_MUB_IRQ
#endif
```

## Optional Settings

Users can disable certain features of the power manager by changing the macros in fsl_pm_device_config.h.

```c
#define FSL_PM_SUPPORT_NOTIFICATION          (true)
#define FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER (true)
#define FSL_PM_SUPPORT_LP_TIMER_CONTROLLER   (true)
```

## Suggested Uses

As previously mentioned, the power manager framework is responsible only for turning system basic resources ON/OFF. If an application requires the core domain to remain active at a different setpoint, it is the responsibility of the application to set the clock root's mux and clock frequency at that setpoint. Note that some clock sources are disabled in specific setpoints.

For different setpoints, NXP has provided recommended settings:

| Resource settings | Setpoint0 | Setpoint1 | Setpoint2 | Setpoint3 | Setpoint4 | Setpoint5 | Setpoint6 | Setpoint7 | Setpoint8 | Setpoint9 | Setpoint10 | Setpoint11 | Setpoint12 | Setpoint13 | Setpoint14 | Setpoint15 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| CM7 clock root mux/div | ARM_PLL | SYS_PLL1 | SYS_PLL1 | SYS_PLL1 | ARM_PLL | SYS_PLL3/2 | SYS_PLL3/2 | RC400/2 | RC_400M/2 | RC_16M | RC_16M | RC_16M | RC_16M | RC_16M | RC_16M | RC_16M |
| CM7 maximum clock root frequency | 700MHz | 1GHz | 1GHz | 1GHz | 700MHz | 240MHz | 240MHz | 200MHz | 200MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz |
| CM4 clock root mux/div | PLL3/2 | PLL3PFD3  | PLL3/2 | PLL3/4 | PLL3/4 | PLL3/4 | PLL3/4 | RC400/4 | RC400/4 | RC400/4 | RC16 | RC400/2 | RC400/4 | RC16 | RC16 | RC16 |
| CM4 maximum clock root frequency | 240MHz | 400MHz | 240MHz | 120MHz | 120MHz | 120MHz | 120MHz | 100MHz | 100MHz | 100MHz | 16MHz | 200MHz | 100MHz | 16MHz | 16MHz | 16MHz |
| BUS clock root mux/div | PLL2PFD3/2 | PLL3/2 | PLL3/2 | PLL3/2 | PLL2PFD3/2 | PLL2PFD3/4 | PLL2PFD3/4 | RC400/4 | RC400/4 | RC16 | RC16 | RC16 | RC16 | RC16 | RC16 | RC16 |
| BUS clock root  frequency | 200MHz | 240MHz | 240MHz | 240MHz | 200MHz | 100MHz | 100MHz | 100MHz | 100MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz | 16MHz |
| BUS_LPSR clock root mux/div | PLL3/4 | PLL3/3 | PLL3/4 | PLL3/8 | PLL3/8 | PLL3/8 | PLL3/8 | RC400/8 | RC400/8 | RC400/8 | RC16 | RC400/4 | RC400/8 | RC16 | RC16 | RC16 |
| BUS_LPSR clock frequency | 120MHz | 160MHz | 120MHz | 60MHz | 60MHz | 60MHz | 60MHz | 50MHz | 50MHz | 50MHz | 16MHz | 100MHz | 50MHz | 16MHz | 16MHz | 16MHz |

## Action Items

- [ ]  Warm-boot is not enabled.
