/* $Id: //depot/blt/kernel/cpuid.S#1 $
**
** Copyright 1998 Sidney Cammeresi
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define __ASM__
#include "smp.h"

.section ".text.init"

.globl smp_check_cpu

smp_check_cpu:
		#.byte 0x66
		pushfl
		#.byte 0x66
		popl %eax
		movl %eax, %ebx
		andl $0x00200000, %eax
		jz not_pentium_or_newer

		xor %eax, %eax
		cpuid
		mov %eax, cpuid_max_level

		cmp %ebx, CPUID_GEN_EBX
		jne not_genuine_intel
		cmp %ecx, CPUID_GEN_ECX
		jne not_genuine_intel
		cmp %edx, CPUID_GEN_EDX
		jne not_genuine_intel

		mov %eax, cpuid_eax
		mov %edx, cpuid_edx
		mov $1, %eax
		ret

not_pentium_or_newer:
not_genuine_intel:
		xor %eax, %eax
		mov %ebx, %eax
		ret

