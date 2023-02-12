#include <cstdlib>
#include "mem.h"
#include "utils.h"
#include <Zydis/Zydis.h>

namespace Mem
{

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

    size_t CalculatePaddedLength(DWORD_PTR insn)
    {
        BYTE* reader;
        size_t i = 0;

        for (; i < 13; i += codeLength(reader))
        {
            reader = (BYTE*)(insn + i);
            Utils::Infof("At: 0x%p insn: %X i=%d i += %d", reader, (*reader) & 0xFF, i, codeLength(reader));
        }
        return i;
    }

    HookResult _hook(DWORD_PTR insn, DWORD_PTR targetFn, size_t freeSpace = 100)
    {
        DWORD _;
        void* targetCall = Mem::CreateCall(targetFn);
        size_t i = CalculatePaddedLength(insn);

        BYTE* reader = (BYTE*)insn;
        void* trampoline = codecaveAlloc(i + 13 + freeSpace);
        void* backup = malloc(i + 2);

        Utils::Infof("Address of trampoline: %p", trampoline);
        Utils::Infof("Backing up: %d bytes", i);
        memcpy(backup, (void*)reader, i + 2);

        Utils::Infof("Replacing instructions");
        memset(trampoline, 0x90, i + 13 + freeSpace);
        memcpy(trampoline, targetCall, 13);
        ((BYTE*)trampoline)[13 + freeSpace + i] = 0xC3;
        VirtualProtect(trampoline, i + 13 + 1, PAGE_EXECUTE_READ, &_);

        Utils::Infof("trampoline: %p", trampoline);
        void* trampolineCall = Mem::CreateCall((DWORD_PTR)trampoline);

        Mem::Write(insn, trampolineCall, 13);
        Utils::Infof("Adding: %d bytes of padding", i - 13);


        for (size_t j = 0; j < (i-13); j++)
        {
            Mem::Write(insn+13+j, new BYTE[1]{ 0xCC }, 1);
        }

        return {
                trampoline,
                backup,
                i,
        };
    }

    HookResult HookOnce(DWORD_PTR insn, DWORD_PTR targetFn, BYTE restoreLen)
    {
        // TODO: Make sure HookOnce works with non 13 aligned instructions
        DWORD _;
        void* hookBackup = malloc(restoreLen);
        HookResult result = _hook(insn, targetFn, 157);
        memcpy((void*)hookBackup, result.trampolinePtr, restoreLen);

        // Make insn to insn+50 rwx
        VirtualProtect((LPVOID)insn, 50, PAGE_EXECUTE_READWRITE, &_);

        BYTE* pre = new BYTE[62]
                {
                        // Make a copy of the return address to r12
                        0x41, 0x5C,     // pop r12

                        // backup used rN registers
                        0x41, 0x50, // push r8
                        0x41, 0x51, // push r9
                        0x41, 0x52, // push r10
                        0x41, 0x53, // push r11

                        // Backup registers
                        0x50,       // push rax
                        0x51,       // push rcx
                        0x52,       // push rdx
                        0x53,       // push rbx
                        0x55,       // push rbp
                        0x56,       // push rsi
                        0x57,       // push rdi

                        // Backup xmm0 to xmm4
                        0x83, 0xEC, 0x10, // sub esp, 0x10
                        0x67, 0xF3, 0x0F, 0x7F, 0x04, 0x24, // movdqu xmmword ptr [esp], xmm0

                        0x83, 0xEC, 0x10, // sub esp, 0x10
                        0x67, 0xF3, 0x0F, 0x7F, 0x0C, 0x24, // movdqu xmmword ptr [esp], xmm1

                        0x83, 0xEC, 0x10, // sub esp, 0x10
                        0x67, 0xF3, 0x0F, 0x7F, 0x14, 0x24, // movdqu xmmword ptr [esp], xmm2

                        0x83, 0xEC, 0x10, // sub esp, 0x10
                        0x67, 0xF3, 0x0F, 0x7F, 0x1C, 0x24, // movdqu xmmword ptr [esp], xmm3

                        0x83, 0xEC, 0x10, // sub esp, 0x10
                        0x67, 0xF3, 0x0F, 0x7F, 0x24, 0x24, // movdqu xmmword ptr [esp], xmm4
                };

        BYTE* post = new BYTE[106]
                {
                        // subtract 0x0D (13) from the ret address
                        0x49, 0x83, 0xEC, restoreLen, // sub r12, subOffset

                        // memcpy(r12, backup, 13);
                        // RCX: _Dst
                        // RDX: _Src
                        // R8 : _Size

                        0x49, 0x83, 0xC4, static_cast<BYTE>(restoreLen - 0x0D), // add r12, subOffset - 0x0D
                        0x49, 0x8B, 0xCC, // mov rcx, r12
                        0xBA, 0x00, 0x00, 0x00, 0x00, // mov edx, backup
                        0x41, 0xB8, restoreLen, 0x00, 0x00, 0x00, // mov r8d, 0x0D
                        0x48, 0xB8, 0x50, 0x34, 0x2C, 0x43, 0x01, 0x00, 0x00, 0x00, // mov rax, 0x00000001432C3450 <omori.&memcpy>
                        0xFF, 0x10, // call qword ptr ds:[rax]

                        0x49, 0x83, 0xEC, static_cast<BYTE>(restoreLen - 0x0D), // sub r12, subOffset - 0x0D

                        // restore xmm registers
                        0x67, 0xF3, 0x0F, 0x6F, 0x24, 0x24, // movdqu xmm4, xmmword ptr [esp]
                        0x83, 0xC4, 0x10, // add esp, 0x10

                        0x67, 0xF3, 0x0F, 0x6F, 0x1C, 0x24, // movdqu xmm3, xmmword ptr [esp]
                        0x83, 0xC4, 0x10, // add esp, 0x10

                        0x67, 0xF3, 0x0F, 0x6F, 0x14, 0x24, // movdqu xmm2, xmmword ptr [esp]
                        0x83, 0xC4, 0x10, // add esp, 0x10

                        0x67, 0xF3, 0x0F, 0x6F, 0x0C, 0x24, // movdqu xmm1, xmmword ptr [esp]
                        0x83, 0xC4, 0x10, // add esp, 0x10

                        0x67, 0xF3, 0x0F, 0x6F, 0x04, 0x24, // movdqu xmm0, xmmword ptr [esp]
                        0x83, 0xC4, 0x10, // add esp, 0x10

                        // restore registers
                        0x5F,       // pop rdi
                        0x5E,       // pop rsi
                        0x5D,       // pop rbp
                        0x5B,       // pop rbx
                        0x5A,       // pop rdx
                        0x59,       // pop rcx
                        0x58,       // pop rax

                        // restore used rN registers
                        0x41, 0x5B, // push r11
                        0x41, 0x5A, // push r10
                        0x41, 0x59, // push r9
                        0x41, 0x58, // push r8

                        // Push the return address back on the stack
                        0x41, 0x54, // push r12
                        0x41, 0xBC, 0x00, 0x00, 0x00, 0x00 // mov r12d, 0
                };

        memcpy((void*)((DWORD_PTR)post + 12), &result.backupPtr, 4);

        Mem::Write((DWORD_PTR)result.trampolinePtr, pre, 62);
        Mem::Write((DWORD_PTR)result.trampolinePtr + 62, hookBackup, 13);
        Mem::Write((DWORD_PTR)result.trampolinePtr + 75, post, 106);
        Utils::Infof("backupPtr: 0x%p trampolinePtr: 0x%p", result.backupPtr, result.trampolinePtr);
        return result;
    }

    void Hook(DWORD_PTR insn, DWORD_PTR targetFn)
    {
        void* backup = malloc(13);
        void* hookRestore = codecaveAlloc(36);

        Utils::Infof("void* backup = %p", backup);

        memcpy(hookRestore, (void*) new BYTE[36]
                {
                        // memcpy(r12 - 0x0F, backup, 12);
                        // RCX: _Dst
                        // RDX: _Src
                        // R8 : _Size

                        0x49, 0x8B, 0xCC, // mov, rcx, r12
                        0x48, 0x83, 0xE9, 0x18, // sub rcx, 0x18
                        0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov, rdx, backup
                        0x41, 0xB8, 0x0C, 0x00, 0x00, 0x00, // mov r8d, 0x0C
                        0x48, 0xB8, 0x50, 0x34, 0x2C, 0x43, 0x01, 0x00, 0x00, 0x00, // mov rax, 0x00000001432C3450 <omori.&memcpy>
                        0xFF, 0x10, // call qword ptr ds:[rax]
                        0xC3, // ret
        }, 36);

        memcpy((void*)((DWORD_PTR)hookRestore + 9), &backup, 8);

        DWORD_PTR nextInsn = insn + CalculatePaddedLength(insn);
        HookOnce(nextInsn, (DWORD_PTR) hookRestore, 0x0D);
        Utils::Infof("hookRestore: 0x%p", hookRestore);

        HookOnce(insn, targetFn, 0x0D);
        memcpy(backup, (void*) insn, 13);
    }

}