/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_SMP_H
#define _ASM_RISCV_SMP_H

#include <linux/cpumask.h>
#include <linux/irqreturn.h>
#include <linux/thread_info.h>

#define INVALID_HARTID ULONG_MAX

struct seq_file;
extern unsigned long boot_cpu_hartid;

struct riscv_ipi_ops {
	void (*ipi_inject)(const struct cpumask *target);
	void (*ipi_clear)(void);
};

#ifdef CONFIG_SMP
/*
 * Mapping between linux logical cpu index and hartid.
 */
extern unsigned long __cpuid_to_hartid_map[NR_CPUS];
#define cpuid_to_hartid_map(cpu)    __cpuid_to_hartid_map[cpu]

/* print IPI stats */
void show_ipi_stats(struct seq_file *p, int prec);

/* SMP initialization hook for setup_arch */
void __init setup_smp(void);

/* Called from C code, this handles an IPI. */
void handle_IPI(struct pt_regs *regs);

/* Hook for the generic smp_call_function_many() routine. */
void arch_send_call_function_ipi_mask(struct cpumask *mask);

/* Hook for the generic smp_call_function_single() routine. */
void arch_send_call_function_single_ipi(int cpu);

int riscv_hartid_to_cpuid(int hartid);
void riscv_cpuid_to_hartid_mask(const struct cpumask *in, struct cpumask *out);

/* Set custom IPI operations */
void riscv_set_ipi_ops(struct riscv_ipi_ops *ops);

/* Clear IPI for current CPU */
void riscv_clear_ipi(void);

/* Secondary hart entry */
asmlinkage void smp_callin(void);

/*
 * Obtains the hart ID of the currently executing task.  This relies on
 * THREAD_INFO_IN_TASK, but we define that unconditionally.
 */
#define raw_smp_processor_id() (current_thread_info()->cpu)

#if defined CONFIG_HOTPLUG_CPU
int __cpu_disable(void);
void __cpu_die(unsigned int cpu);
void cpu_stop(void);
#else
#endif /* CONFIG_HOTPLUG_CPU */

#else

static inline void show_ipi_stats(struct seq_file *p, int prec)
{
}

static inline int riscv_hartid_to_cpuid(int hartid)
{
	if (hartid == boot_cpu_hartid)
		return 0;

	return -1;
}
static inline unsigned long cpuid_to_hartid_map(int cpu)
{
	return boot_cpu_hartid;
}

static inline void riscv_cpuid_to_hartid_mask(const struct cpumask *in,
					      struct cpumask *out)
{
	cpumask_clear(out);
	cpumask_set_cpu(boot_cpu_hartid, out);
}

static inline void riscv_set_ipi_ops(struct riscv_ipi_ops *ops)
{
}

static inline void riscv_clear_ipi(void)
{
}

#endif /* CONFIG_SMP */

#if defined(CONFIG_HOTPLUG_CPU) && (CONFIG_SMP)
bool cpu_has_hotplug(unsigned int cpu);
#else
static inline bool cpu_has_hotplug(unsigned int cpu)
{
	return false;
}
#endif

#ifdef CONFIG_VIPI
#include <asm/csr.h>

static inline
unsigned long rdvcpuid(void) {
#ifdef CONFIG_FIRESIM
	register long vcpu_id asm("a0");

	asm volatile ("\n"
			".option push\n"
			".option norvc\n"

			/* rdvcpuid */
			".word 0xf8102577\n"

			".option pop"
			: "=r"(vcpu_id)
			:
			: "memory");

    return vcpu_id;
#else
    return csr_read(CSR_VCPUID);
#endif
}

static inline
void wrvcpuid(unsigned long val) {
#ifdef CONFIG_FIRESIM
	register long vcpu_id asm("a0") = val;

	asm volatile ("\n"
			".option push\n"
			".option norvc\n"

			/* wrvcpuid */
			".word 0xf8a01077\n"

			".option pop"
			:
			: "r"(vcpu_id)
			: "memory");
#else
    csr_write(CSR_VCPUID, val);
#endif
}

static inline 
void setvipi0(unsigned long val) {
#ifdef CONFIG_FIRESIM
	register long vipi_id asm("a0") = val;

	asm volatile ("\n"
			".option push\n"
			".option norvc\n"

			/* set_vipi0 */
			".word 0xc8a03077\n"

			".option pop"
			:
			: "r"(vipi_id)
			: "memory");
#else
    csr_set(CSR_VIPI0, val);
#endif
}

static inline 
void clrvipi0(unsigned long val) {
#ifdef CONFIG_FIRESIM
    /* FIXME: why ~val? */
	register long vipi_id asm("a0") = ~val;

	asm volatile ("\n"
			".option push\n"
			".option norvc\n"

			/* clr_vipi0 */
			".word 0xc8a02077\n"

			".option pop"
			:
			: "r"(vipi_id)
			: "memory");
#else
    csr_clear(CSR_VIPI0, val);
#endif
}

static inline
unsigned long rdvipi0(void) {
#ifdef CONFIG_FIRESIM
	register long vipi_id asm("a0");

	asm volatile ("\n"
			".option push\n"
			".option norvc\n"

			/* rdvipi0 */
			".word 0xc8101577\n"

			".option pop"
			: "=r"(vipi_id)
			:
			: "memory");

    return vipi_id;
#else
    return csr_read(CSR_VIPI0);
#endif
}
#endif

#endif /* _ASM_RISCV_SMP_H */
