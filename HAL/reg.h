#ifndef __REG_H__
#define __REG_H__

/** \brief  Instruction Synchronization Barrier

    Instruction Synchronization Barrier flushes the pipeline in the processor,
    so that all instructions following the ISB are fetched from cache or
    memory, after the instruction has been completed.
 */
#define __ISB()                           __isb(0xF)


/** \brief  Data Synchronization Barrier

    This function acts as a special kind of Data Memory Barrier.
    It completes when all explicit memory accesses before this instruction complete.
 */
#define __DSB()                           __dsb(0xF)

/** \brief  Wait For Interrupt

    Wait For Interrupt is a hint instruction that suspends execution
    until one of a number of events occurs.
 */
#define __WFI                             __wfi


/** \brief  Wait For Event

    Wait For Event is a hint instruction that permits the processor to enter
    a low-power state until one of a number of events occurs.
 */
#define __WFE                             __wfe

#define CMSDK_APB_BASE          (0x40000000UL)
#define CMSDK_AHB_BASE          (0x40010000UL)

/* APB peripherals*/                                                          
#define CMSDK_TIMER0_BASE       (CMSDK_APB_BASE + 0x0000UL)
#define CMSDK_TIMER1_BASE       (CMSDK_APB_BASE + 0x1000UL)
#define CMSDK_DUALTIMER_BASE    (CMSDK_APB_BASE + 0x2000UL)
#define CMSDK_DUALTIMER_1_BASE  (CMSDK_DUALTIMER_BASE)
#define CMSDK_DUALTIMER_2_BASE  (CMSDK_DUALTIMER_BASE + 0x20UL)
#define CMSDK_UART0_BASE        (CMSDK_APB_BASE + 0x4000UL)
#define CMSDK_UART1_BASE        (CMSDK_APB_BASE + 0x5000UL)
#define CMSDK_UART2_BASE        (CMSDK_APB_BASE + 0x6000UL)
#define CMSDK_WATCHDOG_BASE     (CMSDK_APB_BASE + 0x8000UL)
#define CMSDK_PL230_BASE        (CMSDK_APB_BASE + 0xF000UL)

/* AHB peripherals*/
#define CMSDK_SYSCTRL_BASE      (CMSDK_AHB_BASE + 0xF000UL)

#define ISOCON     (*((volatile unsigned int*)0x4000C000U))
#define ISO_DATA   (*((volatile unsigned int*)0x4000C004U))
#define ISO_DONE   (*((volatile unsigned int*)0x4000C008U))	
#define ISO_TMR    (*((volatile unsigned int*)0x4000C00CU))	

#define ETU_COUNT  (*((volatile unsigned int*)0x4000C010U))
#define ETU_CTRL   (*((volatile unsigned int*)0x4000C014U))
#define ETU_START  (*((volatile unsigned int*)0x4000C018U))
#define ETU_CLEAR  (*((volatile unsigned int*)0x4000C01CU))

// Reg Control Signal
#define NVM_CON    (*((volatile unsigned int*)0x4000D000U))
#define NVM_INT    (*((volatile unsigned int*)0x4000D004U))
#define CLKCON     (*((volatile unsigned int*)0x4000D008U))
#define CLKCFG     (*((volatile unsigned int*)0x4000D00CU))
#define VDET       (*((volatile unsigned int*)0x4000D010U))
#define IFC        (*((volatile unsigned int*)0x4000D014U))
#define PWR        (*((volatile unsigned int*)0x4000D018U))
#define TRIM       (*((volatile unsigned int*)0x4000D01CU))
#define NVM_SIZE   (*((volatile unsigned int*)0x4000D020U))
#define SRC_ADDR   (*((volatile unsigned int*)0x4000D024U))
#define DST_ADDR   (*((volatile unsigned int*)0x4000D028U))
#define DEBUG      (*((volatile unsigned int*)0x4000D02CU))
	
#define NFC_BUFFER ( ( unsigned char*)0x20002700U )
#define CL_CON     (*((volatile unsigned int*)0x40003004U))
#define CL_CFG     (*((volatile unsigned int*)0x40003008U))
#define CL_SIZE    (*((volatile unsigned int*)0x4000300CU))	

#define CRC_INIT   (*((volatile unsigned int*)0x40007000U))
#define CRC_MODE   (*((volatile unsigned int*)0x40007004U))
#define CRC_DATA   (*((volatile unsigned int*)0x40007008U))

#define RAND_CTRL  (*((volatile unsigned char*)0x40009000U))
#define RAND_DATA  (*((volatile unsigned int*)0x40009004U))

#define DES_CON   (*((volatile unsigned int*) 0x4000A000U))
#define DES_IN    (*((volatile unsigned int*) 0x4000A004U))
#define DES_OUT   (*((volatile unsigned int*) 0x4000A008U))

// AES
#define AES_CON   (*((volatile unsigned char*) 0x4000B000U))
	
#define AES_DATA_0  (*((volatile unsigned int*)  0x4000B004U))
#define AES_DATA_1  (*((volatile unsigned int*)  0x4000B008U))
#define AES_DATA_2  (*((volatile unsigned int*)  0x4000B00CU))
#define AES_DATA_3  (*((volatile unsigned int*)  0x4000B010U))

#define AES_KEY_0 (*((volatile unsigned int*)  0x4000B014U))
#define AES_KEY_1 (*((volatile unsigned int*)  0x4000B018U))
#define AES_KEY_2 (*((volatile unsigned int*)  0x4000B01CU))
#define AES_KEY_3 (*((volatile unsigned int*)  0x4000B020U))
#define AES_KEY_4 (*((volatile unsigned int*)  0x4000B024U))
#define AES_KEY_5 (*((volatile unsigned int*)  0x4000B028U))
#define AES_KEY_6 (*((volatile unsigned int*)  0x4000B02CU))
#define AES_KEY_7 (*((volatile unsigned int*)  0x4000B030U))
	
#endif /*__REG_H__*/
