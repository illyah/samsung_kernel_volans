/*
#ifndef _ASM_ARM_ARCH_RESERVED_MEM_H
#define _ASM_ARM_ARCH_RESERVED_MEM_H

#define CONFIG_RESERVED_MEM_CMM_JPEG_MFC_POST_CAMERA

#define DRAM_END_ADDR (PHYS_OFFSET+PHYS_SIZE)


 * Defualt reserved memory size
 * MFC		: 6 MB
 * Post		: 8 MB
 * JPEG		: 8 MB
 * CMM		: 8 MB
 * TV		: 8 MB
 * Camera	: 15 MB
 * These sizes can be modified


#define RESERVED_MEM_MFC	(6 * 1024 * 1024)
#define RESERVED_MEM_POST	(8 * 1024 * 1024)
#define RESERVED_MEM_JPEG	(8 * 1024 * 1024)
#define RESERVED_MEM_CMM	(8 * 1024 * 1024)
#define RESERVED_MEM_TV 	(8 * 1024 * 1024)
#define RESERVED_MEM_CAMERA	(15 * 1024 * 1024)

#if defined(CONFIG_RESERVED_MEM_JPEG)
#define JPEG_RESERVED_MEM_START 	(DRAM_END_ADDR - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_JPEG_POST)	
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST)
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_MFC)			
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_MFC)
#define PHYS_UNRESERVED_SIZE		(MFC_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_MFC_POST)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define PHYS_UNRESERVED_SIZE		(MFC_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_JPEG_MFC_POST)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_MFC) 
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_JPEG_CAMERA)
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_JPEG_POST_CAMERA)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST)
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_MFC_CAMERA)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_MFC)
#define PHYS_UNRESERVED_SIZE		(MFC_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_MFC_POST_CAMERA)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define PHYS_UNRESERVED_SIZE		(MFC_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_JPEG_MFC_POST_CAMERA)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_JPEG)
#define PHYS_UNRESERVED_SIZE		(JPEG_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_CMM_MFC_POST)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define CMM_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_CMM)
#define PHYS_UNRESERVED_SIZE		(CMM_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_CMM_JPEG_MFC_POST_CAMERA)
#if 0
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define JPEG_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_JPEG)
#define CMM_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_JPEG - RESERVED_MEM_CMM)
#define PHYS_UNRESERVED_SIZE		(CMM_RESERVED_MEM_START - PHYS_OFFSET)

#elif defined(CONFIG_RESERVED_MEM_TV_MFC_POST_CAMERA)
#define POST_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST)
#define MFC_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC)
#define TV_RESERVED_MEM_START		(DRAM_END_ADDR - RESERVED_MEM_CAMERA - RESERVED_MEM_POST - RESERVED_MEM_MFC - RESERVED_MEM_TV)
#define PHYS_UNRESERVED_SIZE		(TV_RESERVED_MEM_START - PHYS_OFFSET)

#else
#define PHYS_UNRESERVED_SIZE		(DRAM_END_ADDR - PHYS_OFFSET)
#endif
#define CMM_RESERVED_MEM_START      (0x55300000 + 0x05000000)
#define JPEG_RESERVED_MEM_START     (0x55B00000 + 0x05000000)
#define MFC_RESERVED_MEM_START      (0x56300000 + 0x05000000)
#define POST_RESERVED_MEM_START     (0x56900000 + 0x05000000)
#endif 

#endif  _ASM_ARM_ARCH_RESERVED_MEM_H

*/
#ifndef _ASM_ARM_ARCH_RESERVED_MEM_H
#define _ASM_ARM_ARCH_RESERVED_MEM_H

#include <linux/types.h>
#include <linux/list.h>
#include <asm/setup.h>

#define DRAM_END_ADDR                           (PHYS_OFFSET + PHYS_SIZE)

#ifdef CONFIG_SEC_LOG_BUF

#define SEC_LOG_BUF_DATA_SIZE           (1 << CONFIG_LOG_BUF_SHIFT)

#define SEC_LOG_BUF_FLAG_SIZE           (4 * 1024)

#define SEC_LOG_BUF_SIZE                        (SEC_LOG_BUF_FLAG_SIZE + SEC_LOG_BUF_DATA_SIZE)

#define SEC_LOG_BUF_START                       (DRAM_END_ADDR - SEC_LOG_BUF_SIZE)

#define SEC_LOG_BUF_MAGIC                       0x404C4F47      /* @LOG */

struct sec_log_buf {
        unsigned int *flag;
        unsigned int *count;
        char *data;
};

#define RESERVED_PMEM_END_ADDR          (DRAM_END_ADDR - (1 * 1024 * 1024))             /* Reserved 1MB for Frame & Log Buffer */

extern void sec_log_buf_init(void);

#else

#define RESERVED_PMEM_END_ADDR          (DRAM_END_ADDR)

#endif

#define RESERVED_MEM_CMM                (3 * 1024 * 1024)
#define RESERVED_MEM_MFC                (6 * 1024 * 1024)
#define RESERVED_PMEM_PICTURE           (6 * 1024 * 1024)       /* PMEM_PIC and MFC use share area */
#define RESERVED_PMEM_JPEG              (3 * 1024 * 1024)
#define RESERVED_PMEM_PREVIEW           (2 * 1024 * 1024)
#define RESERVED_PMEM_RENDER            (2 * 1024 * 1024)
#define RESERVED_PMEM_STREAM            (2 * 1024 * 1024)
#define RESERVED_G3D                    (32 * 1024 * 1024)      /* G3D is shared with uppper memory areas */
#define RESERVED_PMEM_GPU1              (0)
#define RESERVED_PMEM                   (8 * 1024 * 1024)
#define RESERVED_PMEM_SKIA              (0)

#define RESERVED_G3D_UI                 (6 * 1024 * 1024)
#define RESERVED_G3D_SHARED             (RESERVED_MEM_CMM + RESERVED_MEM_MFC + RESERVED_PMEM_STREAM + RESERVED_PMEM_JPEG + RESERVED_PMEM_PREVIEW + RESERVED_PMEM_RENDER)
#define RESERVED_G3D_APP                (RESERVED_G3D - RESERVED_G3D_UI - RESERVED_G3D_SHARED)

//#if defined(CONFIG_RESERVED_MEM_CMM_JPEG_MFC_POST_CAMERA)
#define CMM_RESERVED_MEM_START          (RESERVED_PMEM_END_ADDR - RESERVED_MEM_CMM)
#define MFC_RESERVED_MEM_START          (CMM_RESERVED_MEM_START - RESERVED_MEM_MFC)
#define PICTURE_RESERVED_PMEM_START     (MFC_RESERVED_MEM_START)
#define JPEG_RESERVED_PMEM_START        (MFC_RESERVED_MEM_START - RESERVED_PMEM_JPEG)
#define PREVIEW_RESERVED_PMEM_START     (JPEG_RESERVED_PMEM_START - RESERVED_PMEM_PREVIEW)
#define RENDER_RESERVED_PMEM_START      (PREVIEW_RESERVED_PMEM_START - RESERVED_PMEM_RENDER)
#define STREAM_RESERVED_PMEM_START      (RENDER_RESERVED_PMEM_START - RESERVED_PMEM_STREAM)
#define G3D_RESERVED_START              (RESERVED_PMEM_END_ADDR - RESERVED_G3D)          /* G3D is shared */
#define GPU1_RESERVED_PMEM_START        (G3D_RESERVED_START - RESERVED_PMEM_GPU1)
#define RESERVED_PMEM_START             (GPU1_RESERVED_PMEM_START - RESERVED_PMEM)
#define PHYS_UNRESERVED_SIZE            (RESERVED_PMEM_START - PHYS_OFFSET)

#define SKIA_RESERVED_PMEM_START        (0)
//#else
//#define PHYS_UNRESERVED_SIZE            (RESERVED_PMEM_END_ADDR - PHYS_OFFSET)

//#endif

struct s3c6410_pmem_setting{
        resource_size_t pmem_start;
        resource_size_t pmem_size;
        resource_size_t pmem_gpu1_start;
        resource_size_t pmem_gpu1_size;
        resource_size_t pmem_render_start;
        resource_size_t pmem_render_size;
        resource_size_t pmem_render_pic_start;
        resource_size_t pmem_render_pic_size;
        resource_size_t pmem_stream_start;
        resource_size_t pmem_stream_size;
        resource_size_t pmem_preview_start;
        resource_size_t pmem_preview_size;
        resource_size_t pmem_picture_start;
        resource_size_t pmem_picture_size;
        resource_size_t pmem_jpeg_start;
        resource_size_t pmem_jpeg_size;
        resource_size_t pmem_skia_start;
        resource_size_t pmem_skia_size;
};

void s3c6410_add_mem_devices (struct s3c6410_pmem_setting *setting);

#endif /* _ASM_ARM_ARCH_RESERVED_MEM_H */

