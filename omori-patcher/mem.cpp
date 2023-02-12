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

    void Write(DWORD_PTR dst, void* src, size_t len)
    {
        DWORD old;
        VirtualProtect((LPVOID)dst, len, PAGE_EXECUTE_READWRITE, &old);
        memcpy((void*)dst, src, len);
        VirtualProtect((LPVOID)dst, len, old, &old);
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

    void* CreateCall(DWORD_PTR addr)
    {
        void* code = new BYTE[13]
                {
                        0x50, // push rax
                        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, ...
                        0xFF, 0xD0  // call rax
                };

        memcpy((void*)((DWORD_PTR)code + 3), &addr, 8);

        return code;
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

    HookResult createCall(DWORD_PTR targetInsn, DWORD_PTR targetFn, bool restorePerms)
    {
        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        a.push(rax);
        a.call(zasm::Imm64(targetFn));

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, targetInsn);
        if (res != zasm::Error::None)
        {
            Utils::Errorf("createCall: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        size_t size = serializer.getCodeSize();
        size_t padding = getPaddedLength(targetInsn, size) - size;
        void* backup = malloc(size + padding);
        memcpy(backup, (void*) targetInsn, size + padding);
        DWORD permBackup;
        VirtualProtect((LPVOID) targetInsn, size + padding, PAGE_EXECUTE_READWRITE, &permBackup);
        memset((void*) targetInsn, 0xCC, size + padding); // add int3 padding

        memcpy((void*) targetInsn, serializer.getCode(), size);
        Utils::Infof("%p (%d %d)", targetInsn, size, padding);

        if (restorePerms) VirtualProtect((LPVOID) targetInsn, size + padding, permBackup, &permBackup);

        return HookResult
        {
                nullptr,
                backup,
                size,
                padding
        };
    }

    HookResult HookOnce(DWORD_PTR targetInsn, DWORD_PTR hookFn)
    {
        auto hookRes = createCall(targetInsn, (DWORD_PTR) mallocI, false);

        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

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

        a.sub(rsp, 8);
        a.mov(r9, zasm::Imm64(targetInsn));
        a.mov(rcx, r9);
        a.mov(rdx, zasm::Imm64((DWORD_PTR) hookRes.backupPtr));
        a.mov(r8, zasm::Imm64(hookRes.size + hookRes.padding));
        a.mov(rax, zasm::Imm64((DWORD_PTR) Write));
        a.call(rax);
        a.add(rsp, 24);

        // Restore the original register values
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

        a.push(rax);
        a.mov(rax, targetInsn);
        a.add(rsp, 8); // This is really hacky, but it *works*
        a.mov(qword_ptr(rsp), rax);
        a.sub(rsp, 8);
        a.pop(rax);
        a.ret();

        Utils::Infof("%p", targetInsn);

        // a.mov(qword_ptr(rsp), targetInsn);

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, mallocI);
        if (res != zasm::Error::None)
        {
            Utils::Errorf("HookOnce: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        void* codePtr = codecaveAlloc(serializer.getCodeSize());
        memcpy(codePtr, serializer.getCode(), serializer.getCodeSize());

        return hookRes;
    }

    void Hook(DWORD_PTR targetInsn, DWORD_PTR hookFn)
    {

    }

}