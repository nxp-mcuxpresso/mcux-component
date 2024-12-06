#include <errno.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>
#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int memc_init(const struct device *dev)
{
    int8_t rc = 0;
      
    if (dev != NULL)
    {
        rc = dev->init(dev);
        if (rc != 0)
        {
            rc = -rc;
        }

        dev->state->init_res = rc;
    }
    dev->state->initialized = true;
    
    return rc;
}


#ifdef __cplusplus
}
#endif
