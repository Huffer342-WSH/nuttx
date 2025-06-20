/****************************************************************************
 * arch/avr/include/avr/irq.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/* This file should never be included directly but, rather, only indirectly
 * through nuttx/irq.h.
 */

#ifndef __ARCH_AVR_INCLUDE_AVR_IRQ_H
#define __ARCH_AVR_INCLUDE_AVR_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/irq.h>
#include <arch/avr/avr.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register state save array indices */

#define REG_SPH           0 /* Stack pointer on exception entry */
#define REG_SPL           1
#define REG_R27           2 /* r26-r27 */
#define REG_R26           3
#define REG_R31           4 /* r18-r31 */
#define REG_R30           5
#define REG_R29           6
#define REG_R28           7
#define REG_R23           8 /* r2-r23 */
#define REG_R22           9
#define REG_R21          10
#define REG_R20          11
#define REG_R19          12
#define REG_R18          13
#define REG_R17          14
#define REG_R16          15
#define REG_R15          16
#define REG_R14          17
#define REG_R13          18
#define REG_R12          19
#define REG_R11          20
#define REG_R10          21
#define REG_R9           22
#define REG_R8           23
#define REG_R7           24
#define REG_R6           25
#define REG_R5           26
#define REG_R4           27
#define REG_R3           28
#define REG_R2           29
#define REG_R1           30 /* r1 - the "zero" register */
#define REG_R0           31 /* r0 */

#if defined(AVR_HAS_RAMPZ)
#  define REG_RAMPZ      32 /* RAMPZ register for ELPM instruction */
#  define REG_OFFSET_RAMPZ  1
#else
#  define REG_OFFSET_RAMPZ  0 /* MCU does not have RAMPZ */
#endif

#define REG_SREG         (32 + REG_OFFSET_RAMPZ) /* Status register */
#define REG_R25          (33 + REG_OFFSET_RAMPZ) /* r24-r25 */
#define REG_R24          (34 + REG_OFFSET_RAMPZ)

/* The program counter is automatically pushed when the interrupt occurs */

#define REG_PC0          (35 + REG_OFFSET_RAMPZ) /* PC */
#define REG_PC1          (36 + REG_OFFSET_RAMPZ)
#if AVR_PC_SIZE > 16
#  define REG_PC2        (37 + REG_OFFSET_RAMPZ)
#endif

#define XCPTCONTEXT_SIZE XCPTCONTEXT_REGS

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This struct defines the way the registers are stored. */

#ifndef __ASSEMBLY__
struct xcptcontext
{
  /* These are saved copies of PC and SR used during signal processing.
   *
   * REVISIT:  Because there is only one copy of these save areas,
   * only a single signal handler can be active.  This precludes
   * queuing of signal actions.  As a result, signals received while
   * another signal handler is executing will be ignored!
   */

  uint8_t saved_pc1;
  uint8_t saved_pc0;
#if defined(REG_PC2)
  uint8_t saved_pc2;
#endif
#if defined(REG_RAMPZ)
  uint8_t saved_rampz;
#endif
  uint8_t saved_sreg;

  /* Register save area */

  uint8_t regs[XCPTCONTEXT_REGS];
};
#endif

/****************************************************************************
 * Inline functions
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* Name: up_irq_save, up_irq_restore, and friends.
 *
 * NOTE: This function should never be called from application code and,
 * as a general rule unless you really know what you are doing, this
 * function should not be called directly from operation system code either:
 * Typically, the wrapper functions, enter_critical_section() and
 * leave_critical section(), are probably what you really want.
 */

/* Read/write the SREG */

static inline_function irqstate_t getsreg(void)
{
  irqstate_t sreg;
  asm volatile ("in %0, __SREG__" : "=r" (sreg) ::);
  return sreg;
}

/* Return the current value of the stack pointer */

static inline_function uint16_t up_getsp(void)
{
  uint8_t spl;
  uint8_t sph;

  __asm__ __volatile__
  (
    "in %0, __SP_L__\n\t"
    "in %1, __SP_H__\n"
    : "=r" (spl), "=r" (sph)
    :
  );

  return (uint16_t)sph << 8 | spl;
}

/* Interrupt enable/disable */

static inline_function void up_irq_enable()
{
  asm volatile ("sei" ::: "memory");
}

/* Save the current interrupt enable state & disable all interrupts */

static inline_function irqstate_t up_irq_save(void)
{
  irqstate_t sreg;
  asm volatile
    (
      "\tin %0, __SREG__\n"
      "\tcli\n"
      : "=&r" (sreg) :: "memory"
    );
  return sreg;
}

/* Restore saved interrupt state */

static inline_function void up_irq_restore(irqstate_t flags)
{
  asm volatile ("out __SREG__, %0" : : "r" (flags) : "memory");
}
#endif /* __ASSEMBLY__ */

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: up_getusrpc
 ****************************************************************************/

#if defined(REG_PC2)
#  define up_getusrpc(regs) \
    ((regs) ? \
     ((((uint8_t *)(regs))[REG_PC0] << 16) | \
      (((uint8_t *)(regs))[REG_PC1] <<  8) | \
      (((uint8_t *)(regs))[REG_PC2] <<  0)) : \
     (((uint8_t *)up_current_regs())[REG_PC0] << 16) | \
     (((uint8_t *)up_current_regs())[REG_PC1] <<  8) | \
     (((uint8_t *)up_current_regs())[REG_PC2] <<  0))
#else
#  define up_getusrpc(regs) \
    ((regs) ? \
     ((((uint8_t *)(regs))[REG_PC0] << 8) | \
      (((uint8_t *)(regs))[REG_PC1] << 0)) : \
     (((uint8_t *)up_current_regs())[REG_PC0] << 8) | \
     (((uint8_t *)up_current_regs())[REG_PC1] << 0))
#endif

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif

#endif /* __ARCH_AVR_INCLUDE_AVR_IRQ_H */
