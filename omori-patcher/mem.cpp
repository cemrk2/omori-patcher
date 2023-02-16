#include <cstdlib>
#include "mem.h"
#include "utils.h"
#include "zasm/program/program.hpp"
#include "zasm/x86/assembler.hpp"
#include "zasm/serialization/serializer.hpp"
#include "zasm/x86/x86.hpp"
#include <Zydis/Zydis.h>

namespace Mem
{
    using namespace zasm::x86;

    Xmm* xmms = new Xmm[] { xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15 };

    size_t codeLength(BYTE* code)
    {
        ZyanU8* data = code;
        size_t codeLength = 0;

        auto runtime_address = reinterpret_cast<ZyanU64>(code);
        ZyanUSize offset = 0;
        ZydisDisassembledInstruction instruction;

        while (ZYAN_SUCCESS(ZydisDisassembleIntel(
                ZYDIS_MACHINE_MODE_LONG_64,
                runtime_address,
                data + offset,
                sizeof(data) - offset,
                &instruction
        ))) {
            codeLength += instruction.info.length;
            offset += instruction.info.length;
        }

        return codeLength;
    }

    void* codecaveAlloc(size_t size)
    {
        DWORD _;
        void* old = (void*) mallocI;
        VirtualProtect(old, size, PAGE_EXECUTE_READWRITE, &_);
        mallocI += size;
        if (mallocI > Consts::codecaveEnd)
        {
            Utils::Error("codecaveAlloc: No more free space in codecave");
            return nullptr;
        }
        return old;
    }

    size_t getPaddedLength(DWORD_PTR insn, size_t alignTo)
    {
        size_t i = 0;

        for (BYTE* reader; i < alignTo; i += codeLength(reader))
        {
            reader = (BYTE*)(insn + i);
        }
        return i;
    }

    HookResult createCall(DWORD_PTR targetInsn, int offset, DWORD_PTR targetFn, bool restorePerms, size_t backupLen)
    {
        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        a.push(rax);
        a.call(zasm::Imm64(targetFn));

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, (int64_t) targetInsn+offset);
        if (res != zasm::Error::None)
        {
            Utils::Errorf("createCall: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        size_t size = serializer.getCodeSize();
        size_t padding = getPaddedLength(targetInsn+offset, size) - size;
        if (backupLen < size + padding) backupLen = size + padding;
        void* backup = malloc(backupLen);
        memcpy(backup, (void*) (targetInsn+offset), backupLen);
        DWORD permBackup;
        VirtualProtect((LPVOID) (targetInsn+offset), backupLen, PAGE_EXECUTE_READWRITE, &permBackup);
        memset((void*) (targetInsn+offset), 0xCC, size+padding); // add int3 padding

        memcpy((void*) (targetInsn+offset), serializer.getCode(), size);
        Utils::Infof("%p+%d (%d %d)", targetInsn, offset, size, padding);

        if (restorePerms) VirtualProtect((LPVOID) (targetInsn+offset), backupLen, permBackup, &permBackup);

        return HookResult
        {
                nullptr,
                backup,
                size,
                padding
        };
    }

    HookResult HookOnce(DWORD_PTR targetInsn, int funcOffset, DWORD_PTR hookFn, bool jmpToOffset, size_t backupLen)
    {
        auto hookRes = createCall(targetInsn, funcOffset, (DWORD_PTR) mallocI, false, backupLen);
        if (backupLen < hookRes.size + hookRes.padding) backupLen = hookRes.size + hookRes.padding;

        Utils::Infof("backup: %p", hookRes.backupPtr);

        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        for (int i = 0; i < 16; ++i) {
            a.sub(rsp, 16);
            a.movdqu(xmmword_ptr(rsp), xmms[i]);
        }

        // Backup rax, rbx, rcx, rdx, rbp, rsi, rdi and r8 through r15
        a.push(rax);
        a.push(rbx);
        a.push(rcx);
        a.push(rdx);
        a.push(rbp);
        a.push(rsi);
        a.push(rdi);
        a.push(r8);
        a.push(r9);
        a.push(r10);
        a.push(r11);
        a.push(r12);
        a.push(r13);
        a.push(r14);
        a.push(r15);
        a.push(rsp);

        a.sub(rsp, 32);

        a.mov(r15, zasm::Imm64(hookFn));
        a.call(r15);

        a.mov(rcx, zasm::Imm64(targetInsn+funcOffset));
        a.mov(rdx, zasm::Imm64((DWORD_PTR) hookRes.backupPtr));
        a.mov(r8, zasm::Imm64(backupLen));
        a.mov(rax, zasm::Imm64((DWORD_PTR) memcpy));
        a.call(rax);

        a.add(rsp, 32);

        // Restore the original register values
        a.pop(rsp);
        a.pop(r15);
        a.pop(r14);
        a.pop(r13);
        a.pop(r12);
        a.pop(r11);
        a.pop(r10);
        a.pop(r9);
        a.pop(r8);
        a.pop(rdi);
        a.pop(rsi);
        a.pop(rbp);
        a.pop(rdx);
        a.pop(rcx);
        a.pop(rbx);
        a.pop(rax);

        for (int i = 15; i >= 0; i--) {
            a.movdqu(xmms[i], xmmword_ptr(rsp));
            a.add(rsp, 16);
        }

        a.add(rsp, 16);
        a.jmp(jmpToOffset ? targetInsn + funcOffset : targetInsn);

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, (int64_t) mallocI);
        if (res != zasm::Error::None)
        {
            Utils::Errorf("HookOnce: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        void* codePtr = codecaveAlloc(serializer.getCodeSize());
        memcpy(codePtr, serializer.getCode(), serializer.getCodeSize());

        return hookRes;
    }

    void Hook(DWORD_PTR targetInsn, int funcOffset, DWORD_PTR hookFn)
    {
        auto hook1 = HookOnce(targetInsn, funcOffset, hookFn,  false, 50);
        size_t hook1Len = hook1.size + hook1.padding;
        Utils::Infof("hook1 length: %d", hook1Len);

        void* hookBackup = malloc(hook1Len);
        memcpy(hookBackup, (void*) (targetInsn + funcOffset), hook1Len);

        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        a.mov(rcx, zasm::Imm64(targetInsn + funcOffset));
        a.mov(rdx, zasm::Imm64((DWORD_PTR)hookBackup));
        a.mov(r8, zasm::Imm64(hook1Len));
        a.mov(r15, zasm::Imm64((DWORD_PTR) memcpy));
        a.call(r15);
        a.ret();

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, (int64_t) mallocI);
        if (res != zasm::Error::None)
        {
            Utils::Errorf("Hook: Failed to serialize program %s", getErrorName(res));
            return;
        }

        void* codePtr = codecaveAlloc(serializer.getCodeSize());
        memcpy(codePtr, serializer.getCode(), serializer.getCodeSize());

        Utils::Infof("hook2 codePtr: %p", codePtr);

        Utils::Infof("hook2 at %p", targetInsn+hook1Len);
        auto hook2 = HookOnce(targetInsn+hook1Len, 1, (DWORD_PTR)codePtr, true, 0);
        Utils::Infof("hook2: %d %d", hook2.size, hook2.padding);

        DWORD _;
        VirtualProtect((LPVOID)targetInsn, hook1Len + hook2.size + hook2.padding, PAGE_EXECUTE_READWRITE, &_);

        memcpy((void*) ((DWORD_PTR)hook1.backupPtr+hook1Len), (void*) (targetInsn+funcOffset+hook1Len), 50 - hook1Len);
        Utils::Successf("Hooked into %p+%d", targetInsn, funcOffset);
    }

}