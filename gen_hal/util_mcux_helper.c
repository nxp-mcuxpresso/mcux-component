#include "util_mcux_helper.h"
#include <zephyr/kernel.h>


bool k_is_in_isr(void)
{
  return (__get_IPSR()) ? (true) : (false);
}