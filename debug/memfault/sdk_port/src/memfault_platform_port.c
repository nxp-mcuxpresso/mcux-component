//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See LICENSE for details
//!
//! Copyright 2025 Memfault, Inc
//! Copyright 2025 NXP
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!
//! Glue layer between the Memfault SDK and the underlying platform
//!

#include <stdbool.h>
#include <stdio.h>
#include "memfault/panics/arch/arm/cortex_m.h"
#include "memfault/components.h"
#include "mflash_drv.h"
#include "fsl_debug_console.h"
#include "memfault/ports/reboot_reason.h"

#ifdef SDK_OS_FREE_RTOS
#include "memfault/ports/freertos.h"
#include "memfault/ports/freertos_coredump.h"
#endif

#ifndef MEMFAULT_DEBUG_LOG_BUFFER_SIZE_BYTES
  #define MEMFAULT_DEBUG_LOG_BUFFER_SIZE_BYTES (128)
#endif

#define MEMFAULT_COREDUMP_MAX_TASK_REGIONS ((MEMFAULT_PLATFORM_MAX_TRACKED_TASKS)*2)

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  // !FIXME: Populate with platform device information

  // IMPORTANT: All strings returned in info must be constant
  // or static as they will be used _after_ the function returns

  // See https://mflt.io/version-nomenclature for more context
  *info = (sMemfaultDeviceInfo){
    // An ID that uniquely identifies the device in your fleet
    // (i.e serial number, mac addr, chip id, etc)
    // Regular expression defining valid device serials: ^[-a-zA-Z0-9_]+$
    .device_serial = CONFIG_MEMFAULT_DEVICE_SERIAL,
    // A name to represent the firmware running on the MCU.
    // (i.e "ble-fw", "main-fw", or a codename for your project)
    .software_type = CONFIG_MEMFAULT_SW_TYPE,
    // The version of the "software_type" currently running.
    // "software_type" + "software_version" must uniquely represent
    // a single binary
    .software_version = CONFIG_MEMFAULT_SW_VERSION,
    // The revision of hardware for the device. This value must remain
    // the same for a unique device.
    // (i.e evt, dvt, pvt, or rev1, rev2, etc)
    // Regular expression defining valid hardware versions: ^[-a-zA-Z0-9_\.\+]+$
    .hardware_version = CONFIG_MEMFAULT_HW_VERSION,
  };
}

//! Last function called after a coredump is saved. Should perform
//! any final cleanup and then reset the device
void memfault_platform_reboot(void) {
  NVIC_SystemReset();
  while (true) { }  // unreachable
}

bool memfault_platform_time_get_current(sMemfaultCurrentTime *time) {
  // !FIXME: If the device tracks real time, update 'unix_timestamp_secs' with seconds since epoch
  // This will cause events logged by the SDK to be timestamped on the device rather than when they
  // arrive on the server
  *time = (sMemfaultCurrentTime){
    .type = kMemfaultCurrentTimeType_UnixEpochTimeSec,
    .info = { .unix_timestamp_secs = 0 },
  };

  // !FIXME: If device does not track time, return false, else return true if time is valid
  return false;
}

size_t memfault_platform_sanitize_address_range(void *start_addr, size_t desired_size) {
  static const struct {
    uint32_t start_addr;
    size_t length;
  } s_mcu_mem_regions[] = {
    { .start_addr = 0x00000000u, .length = 0xFFFFFFFFu },
  };

  for (size_t i = 0; i < MEMFAULT_ARRAY_SIZE(s_mcu_mem_regions); i++) {
    const uint32_t lower_addr = s_mcu_mem_regions[i].start_addr;
    const uint32_t upper_addr = lower_addr + s_mcu_mem_regions[i].length;
    if ((uint32_t)(uint8_t *)start_addr >= lower_addr && ((uint32_t)(uint8_t *)start_addr < upper_addr)) {
      return MEMFAULT_MIN(desired_size, upper_addr - (uint32_t)(uint8_t *)start_addr);
    }
  }

  return 0;
}

#ifdef SDK_OS_FREE_RTOS

static uint32_t prv_read_psp_reg(void) {
  uint32_t reg_val;
  __asm volatile("mrs %0, psp" : "=r"(reg_val));
  return reg_val;
}

static sMfltCoredumpRegion s_coredump_regions[MEMFAULT_COREDUMP_MAX_TASK_REGIONS +
                                              2   /* active stack(s) */
                                              + 1 /* _kernel variable */
                                              + 1 /* __memfault_capture_start */
                                              + 2 /* s_task_tcbs + s_task_watermarks */
];

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)

extern uint32_t Image$$RW_m_memfault_bss$$Base[];
extern uint32_t Image$$RW_m_memfault_bss$$ZI$$Limit[];
static const uint32_t memfault_capture_bss_start = (uintptr_t)Image$$RW_m_memfault_bss$$Base;
static const uint32_t memfault_capture_bss_end   = (uintptr_t)Image$$RW_m_memfault_bss$$ZI$$Limit;

#elif defined(__ICCARM__)

#pragma section="memfault_zi"
static const uint32_t memfault_capture_bss_start = (uintptr_t)__section_begin("memfault_zi");
static const uint32_t memfault_capture_bss_end   = (uintptr_t)__section_end("memfault_zi");

#elif defined(__GNUC__)
extern uint32_t __memfault_capture_bss_start;
extern uint32_t __memfault_capture_bss_end;
static const uint32_t memfault_capture_bss_start = (uintptr_t)&__memfault_capture_bss_start;
static const uint32_t memfault_capture_bss_end   = (uintptr_t)&__memfault_capture_bss_end;

#endif

MEMFAULT_WEAK const sMfltCoredumpRegion *memfault_platform_coredump_get_regions(
  const sCoredumpCrashInfo *crash_info, size_t *num_regions) {
  size_t region_idx = 0;
  const size_t active_stack_size_to_collect = 512;

  // first, capture the active stack (and ISR if applicable)
  const bool msp_was_active = (crash_info->exception_reg_state->exc_return & (1UL << 2)) == 0UL;

  size_t stack_size_to_collect = memfault_platform_sanitize_address_range(
    crash_info->stack_address, MEMFAULT_PLATFORM_ACTIVE_STACK_SIZE_TO_COLLECT);

  s_coredump_regions[region_idx] =
    MEMFAULT_COREDUMP_MEMORY_REGION_INIT(crash_info->stack_address, stack_size_to_collect);
  region_idx++;

  if (msp_was_active) {
    // System crashed in an ISR but the running task state is on PSP so grab that too
    void *psp = (uint8_t*)prv_read_psp_reg();

    // Collect a little bit more stack for the PSP since there is an
    // exception frame that will have been stacked on it as well
    const uint32_t extra_stack_bytes = 128;
    stack_size_to_collect = memfault_platform_sanitize_address_range(
      psp, active_stack_size_to_collect + extra_stack_bytes);
    s_coredump_regions[region_idx] =
      MEMFAULT_COREDUMP_MEMORY_REGION_INIT(psp, stack_size_to_collect);
    region_idx++;
  }

  // Scoop up memory regions necessary to perform unwinds of the FreeRTOS tasks
  const size_t memfault_region_size =
    memfault_capture_bss_end - memfault_capture_bss_start;

  s_coredump_regions[region_idx] =
    MEMFAULT_COREDUMP_MEMORY_REGION_INIT((const uint8_t*)memfault_capture_bss_start, memfault_region_size);
  region_idx++;

#if !(defined(MEMFAULT_TEST_USE_PORT_TEMPLATE) && (MEMFAULT_TEST_USE_PORT_TEMPLATE == 1))
  region_idx += memfault_freertos_get_task_regions(
    &s_coredump_regions[region_idx], MEMFAULT_ARRAY_SIZE(s_coredump_regions) - region_idx);
#endif

  *num_regions = region_idx;
  return &s_coredump_regions[0];
}

#else /* !SDK_OS_FREE_RTOS */

MEMFAULT_WEAK const sMfltCoredumpRegion *memfault_platform_coredump_get_regions(
    const sCoredumpCrashInfo *crash_info, size_t *num_regions) {

    static sMfltCoredumpRegion s_coredump_regions[1];

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    extern uint32_t Image$$ARM_LIB_STACK$$ZI$$Limit[];
    const uint32_t stack_top_addr = (uint32_t)Image$$ARM_LIB_STACK$$ZI$$Limit;
#elif defined(__GNUC__)
#if defined(__MCUXPRESSO)
    extern uint32_t _vStackTop;
    const uint32_t stack_top_addr = (uint32_t)&_vStackTop;
#else
    extern uint32_t __StackTop;
    const uint32_t stack_top_addr = (uint32_t)&__StackTop;
#endif
#elif defined(__ICCARM__)
#pragma section="CSTACK"
    const uint32_t stack_top_addr = (uintptr_t)__section_end("CSTACK");
#else
#error "Unsupported Compiler."
#endif

    s_coredump_regions[0] = MEMFAULT_COREDUMP_MEMORY_REGION_INIT(
        crash_info->stack_address, stack_top_addr - (uintptr_t)crash_info->stack_address);

    *num_regions = 1;

    return s_coredump_regions;
}
#endif /* SDK_OS_FREE_RTOS */

MEMFAULT_PUT_IN_SECTION(".noinit.mflt_reboot_info")
static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

void memfault_reboot_reason_get(sResetBootupInfo *info) {
  //! !FIXME: Read reboot reason register
  //! Fill in sResetBootupInfo with reboot reason and reboot register value
  *info = (sResetBootupInfo){
    .reset_reason_reg = 0x0,
    .reset_reason = kMfltRebootReason_Unknown,
  };
}

void memfault_platform_reboot_tracking_boot(void) {
  sResetBootupInfo reset_info = { 0 };
  memfault_reboot_reason_get(&reset_info);
  memfault_reboot_tracking_boot(s_reboot_tracking, &reset_info);
}

#ifndef SDK_OS_FREE_RTOS
//! !FIXME: Remove if using FreeRTOS port. The FreeRTOS port will provide this definition
bool memfault_platform_metrics_timer_boot(uint32_t period_sec,
                                          MemfaultPlatformTimerCallback *callback) {
  //! !FIXME: Initiate a periodic timer/task/thread to call callback every period_sec
  (void)period_sec;
  (void)callback;
  return false;
}

//! !FIXME: Remove if using FreeRTOS port. The FreeRTOS port will provide this definition
MEMFAULT_WEAK uint64_t memfault_platform_get_time_since_boot_ms(void) {
  //! !FIXME: Return the time since the device booted in milliseconds
  return 0;
}
#endif

void memfault_platform_log(eMemfaultPlatformLogLevel level, const char *fmt,
                           ...) {
  const char *lvl_str;
  switch (level) {
    case kMemfaultPlatformLogLevel_Debug:
      lvl_str = "D";
      break;

    case kMemfaultPlatformLogLevel_Info:
      lvl_str = "I";
      break;

    case kMemfaultPlatformLogLevel_Warning:
      lvl_str = "W";
      break;

    case kMemfaultPlatformLogLevel_Error:
      lvl_str = "E";
      break;

    default:
      return;
      break;
  }

  va_list args;
  va_start(args, fmt);

  char log_buf[MEMFAULT_DEBUG_LOG_BUFFER_SIZE_BYTES];
  (void)vsnprintf(log_buf, sizeof(log_buf), fmt, args);

  PRINTF("[%s] MFLT: %s\r\n", lvl_str, log_buf);

  va_end(args);
}

//! to starting an RTOS or baremetal loop.
int memfault_platform_boot(void) {

  (void)mflash_drv_init();

  memfault_build_info_dump();
  memfault_device_info_dump();

#ifdef SDK_OS_FREE_RTOS
  memfault_freertos_port_boot();
#endif

  memfault_platform_reboot_tracking_boot();

  // initialize the event storage buffer
  static uint8_t s_event_storage[1024];
  const sMemfaultEventStorageImpl *evt_storage =
    memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));

  // configure trace events to store into the buffer
  (void)memfault_trace_event_boot(evt_storage);

  // record the current reboot reason
  (void)memfault_reboot_tracking_collect_reset_info(evt_storage);

  // configure the metrics component to store into the buffer
  size_t crash_count = memfault_reboot_tracking_get_crash_count();
  sMemfaultMetricBootInfo boot_info = {
    .unexpected_reboot_count = crash_count,
  };
  (void)memfault_metrics_boot(evt_storage, &boot_info);

  MEMFAULT_LOG_INFO("Memfault Initialized!");

  return 0;
}

#ifndef SDK_OS_FREE_RTOS
static uint32_t s_globalIntEnableStat;

bool memfault_self_test_platform_disable_irqs(void) {
    s_globalIntEnableStat = DisableGlobalIRQ();
    return true;
}

bool memfault_self_test_platform_enable_irqs(void) {
    EnableGlobalIRQ(s_globalIntEnableStat);
    return true;
}
#endif

void memfault_platform_log_raw(const char *fmt, ...)
{
}
