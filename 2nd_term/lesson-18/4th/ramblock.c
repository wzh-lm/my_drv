#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#define RAMBLOCK_SIZE 1024*1024   

static DEFINE_SPINLOCK(ramblock_lock);
struct gendisk 		 *ramblock_disk;
request_queue_t		 *ramblock_queue;
static unsigned char *ramblock_buffer;
int major;

static int ramblock_getgeo(struct block_device *blkdev, struct hd_geometry *geo)
{
	geo->heads     = 2;
	geo->cylinders = 24;
	geo->sectors   = RAMBLOCK_SIZE/2/24/512;
	return 0;
}

static struct block_device_operations ramblock_ops = {
	.owner  = THIS_MODULE,
	.getgeo = ramblock_getgeo
};

static void do_ramblock_request(request_queue_t * q)
{
	struct request *req;
	static int r_cnt = 0;
	static int w_cnt = 0;
	
	//printk("[%d]Do_Ramblock_Request!\n", cnt++);
	while ((req = elv_next_request(q)) != NULL) {
		unsigned long offset = req->sector * 512;
		unsigned long len    = req->current_nr_sectors * 512;
		
		if(rq_data_dir(req) == READ){
			printk("[%d]request read. \n", ++r_cnt);
			memcpy(req->buffer, ramblock_buffer+offset, len);
		}else{
			printk("[%d]request write.\n", ++w_cnt);
			memcpy(ramblock_buffer+offset, req->buffer, len);
		}
		end_request(req, 1);
	}
}

static int ramblock_init(void)
{
	ramblock_disk = alloc_disk(16);
	if(!ramblock_disk){
		printk("alloc disk ERROR!");
		return 1;
	}
	major = register_blkdev(0, "ramblock");
	if(!major){
		printk("Auto register blkdev ERROR!");
		return 1;
	}
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
	if(!ramblock_queue){
		printk("ramblock init queue ERROR!");
		return 1;
	}
	ramblock_buffer = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);
	if(!ramblock_buffer){
		printk("ramblock buffer alloc ERROR!");
		return 1;
	}
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->major 	   = major;
	ramblock_disk->queue       = ramblock_queue;
	ramblock_disk->capacity    = RAMBLOCK_SIZE / 512;
	ramblock_disk->first_minor = 0;
	ramblock_disk->fops        = &ramblock_ops;
	add_disk(ramblock_disk);
	return 0;
}

static void ramblock_exit(void)
{
    if (unregister_blkdev( major, "ramblock" ))
		printk( "unregister blkdev ERROR!");
    del_gendisk(ramblock_disk);
    put_disk(ramblock_disk);
    blk_cleanup_queue(ramblock_queue);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");
