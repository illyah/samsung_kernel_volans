
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "KXSD9_dev.h"
#include "common.h"
#include "KXSD9_main.h"

int KXSD9_i2c_drv_init(void);
void KXSD9_i2c_drv_exit(void);

static int KXSD9_probe (struct i2c_client *, const struct i2c_device_id *id);
static int KXSD9_remove(struct i2c_client *);
static void KXSD9_shutdown(struct i2c_client *);
static int KXSD9_suspend(struct i2c_client *, pm_message_t mesg);
static int KXSD9_resume(struct i2c_client *);

static const struct i2c_device_id KXSD9_drv_id_table[] = 
{
    {"KXSD9", 0},
    { },
};

MODULE_DEVICE_TABLE(i2c,KXSD9_drv_id_table);

static struct i2c_driver KXSD9_i2c_driver =
{
    .driver = {
        .name = "KXSD9_driver",
    },
	
    .probe = KXSD9_probe,
    .remove = KXSD9_remove,

    .shutdown = KXSD9_shutdown,
    .suspend = KXSD9_suspend,
    .resume = KXSD9_resume,
    .id_table = KXSD9_drv_id_table,
};

static int KXSD9_probe (struct i2c_client *client, 
	                           const struct i2c_device_id *id)
{
    int ret = 0;

    debug("KXSD9 i2c driver KXSD9_probe called"); 

    if( strcmp(client->name, "KXSD9") != 0 )
    {
        ret = -1;
        debug("KXSD9_probe: device not supported");
    }
    else if( (ret = KXSD9_dev_init(client, KXSD9_main_getparam())) < 0 )
    {
        debug("KXSD9_dev_init failed");
    }

    return ret;
}

static int KXSD9_remove(struct i2c_client *client)
{
    int ret = 0;

    debug("KXSD9 i2c driver KXSD9_remove called"); 

    if( strcmp(client->name, "KXSD9") != 0 )
    {
        ret = -1;
        debug("KXSD9_remove: device not supported");
    }
    else if( (ret = KXSD9_dev_exit()) < 0 )
    {
        debug("KXSD9_dev_exit failed");
    }

    return ret;
}

static void KXSD9_shutdown(struct i2c_client *client)
{
    debug("KXSD9 i2c driver KXSD9_shutdown called"); 

    if( strcmp(client->name, "KXSD9") != 0 )
    {
        debug("KXSD9_shutdown: device not supported");
    }
    else if( KXSD9_dev_disable() < 0 )
    {
        debug("KXSD9_dev_disable failed");
    }
}

static int KXSD9_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret = 0;
	   
    debug("KXSD9 i2c driver KXSD9_suspend called"); 

    if( strcmp(client->name, "KXSD9") != 0 )
    {
        ret = -1;
        debug("KXSD9_suspend: device not supported");
    }
    else if( (ret = KXSD9_dev_disable()) < 0 )
    {
        debug("KXSD9_dev_disable failed");
    }

    return ret;
}

static int KXSD9_resume(struct i2c_client *client)
{
    int ret = 0;
	   
    debug("KXSD9 i2c driver KXSD9_resume called"); 

    if( strcmp(client->name, "KXSD9") != 0 )
    {
        ret = -1;
        debug("KXSD9_resume: device not supported");
    }
    else if( (ret = KXSD9_dev_enable()) < 0 )
    {
        debug("KXSD9_dev_enable failed");
    }
 
    return ret;
}

int KXSD9_i2c_drv_init(void)
{	
    int ret = 0;

    debug("KXSD9 i2c driver KXSD9_i2c_driver_init called"); 

    if ( (ret = i2c_add_driver(&KXSD9_i2c_driver) < 0) ) 
    {
        error("KXSD9 i2c_add_driver failed");
    }

    return ret;
}

void KXSD9_i2c_drv_exit(void)
{
    debug("KXSD9 i2c driver KXSD9_i2c_driver_exit called"); 

    i2c_del_driver(&KXSD9_i2c_driver);
}


