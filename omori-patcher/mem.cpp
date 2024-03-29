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
    Gp64* registers = new Gp64[] { rax, rbx, rcx, rdx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15, rsp };

    /**
     * Decodes given assembly instruction and returns its length
     * @param code Pointer to the instruction
     * @return instruction length
     */
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

    /**
     * Allocates space in the codecave, changes memory permissions to RWX
     * @param size Size for allocation
     * @return Pointer to codecave
     */
    void* codecaveAlloc(size_t size)
    {
        DWORD _;
        void* old = (void*) mallocI;
        VirtualProtect(old, size, PAGE_EXECUTE_READWRITE, &_);
        mallocI += size;
        if (mallocI > Consts::codecaveEnd) {
            Error("codecaveAlloc: No more free space in codecave");
            return nullptr;
        }
        return old;
    }

    /**
     * Calculates length of multiple instructions
     * @param insn Pointer to the first instruction
     * @param alignTo Minimum amount of bytes that need to be decoded
     * @return Padded length
     */
    size_t getPaddedLength(DWORD_PTR insn, size_t alignTo)
    {
        size_t i = 0;

        for (BYTE* reader; i < alignTo; i += codeLength(reader)) {
            reader = (BYTE*)(insn + i);
        }
        return i;
    }

    /**
     * Writes a call instruction at a specified address
     * @param targetInsn Place to write the call
     * @param offset Offset from targetInsn
     * @param targetFn Function to call
     * @param backupLen Recommended backup length
     * @return backup copy of instructions, length of call, padding
     */
    HookResult createCall(DWORD_PTR targetInsn, int offset, DWORD_PTR targetFn, size_t backupLen)
    {
        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        a.push(rax);
        a.call(zasm::Imm64(targetFn));

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, (int64_t) targetInsn+offset);
        if (res != zasm::Error::None) {
            Errorf("createCall: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        size_t size = serializer.getCodeSize();
        size_t padding = getPaddedLength(targetInsn+offset, size) - size;
        if (backupLen < size + padding) backupLen = size + padding;
        void* backup = malloc(backupLen);
        memcpy(backup, (void*) (targetInsn+offset), backupLen);
        DWORD _;
        VirtualProtect((LPVOID) (targetInsn+offset), backupLen, PAGE_EXECUTE_READWRITE, &_);
        memset((void*) (targetInsn+offset), 0xCC, size+padding); // add int3 padding

        memcpy((void*) (targetInsn+offset), serializer.getCode(), size);
        Infof("%p+%d (%d %d)", targetInsn, offset, size, padding);

        return HookResult {
            nullptr,
            backup,
            size,
            padding
        };
    }

    /**
     * Adds a hook to a function, removes it self after the first call to the function
     * @param targetInsn Target to hook
     * @param funcOffset Offset from targetInsn
     * @param hookFn Function to call
     * @param jmpToOffset Offset to jump back
     * @param backupLen Recommended backup length
     * @param asmCallback Callback asm modifier
     * @return
     */
    HookResult HookOnce(DWORD_PTR targetInsn, int funcOffset, DWORD_PTR hookFn, bool jmpToOffset, size_t backupLen,
                        void* cbAsmPtr)
    {
        auto hookRes = createCall(targetInsn, funcOffset, (DWORD_PTR) mallocI, backupLen);
        if (backupLen < hookRes.size + hookRes.padding) backupLen = hookRes.size + hookRes.padding;

        Infof("backup: %p", hookRes.backupPtr);

        zasm::Program program(zasm::MachineMode::AMD64);
        zasm::x86::Assembler a(program);

        for (int i = 0; i < 16; ++i) {
            a.sub(rsp, 16);
            a.movdqu(xmmword_ptr(rsp), xmms[i]);
        }

        // Backup cpu registers
        for (int i = 0; i < 16; ++i) {
            a.push(registers[i]);
        }

        a.sub(rsp, 32);

        if (cbAsmPtr != nullptr) {
            a.push(r15);
            a.mov(r15, zasm::Imm64((DWORD_PTR)cbAsmPtr));
            a.call(r15);
            a.pop(r15);
        }

        a.mov(r15, zasm::Imm64(hookFn));
        a.call(r15);

        a.mov(rcx, zasm::Imm64(targetInsn+funcOffset));
        a.mov(rdx, zasm::Imm64((DWORD_PTR) hookRes.backupPtr));
        a.mov(r8, zasm::Imm64(backupLen));
        a.mov(rax, zasm::Imm64((DWORD_PTR) memcpy));
        a.call(rax);

        a.add(rsp, 32);

        // Restore the original register values
        for (int i = 15; i >= 0; i--) {
            a.pop(registers[i]);
        }

        for (int i = 15; i >= 0; i--) {
            a.movdqu(xmms[i], xmmword_ptr(rsp));
            a.add(rsp, 16);
        }

        a.add(rsp, 16);
        a.jmp(jmpToOffset ? targetInsn + funcOffset : targetInsn);

        zasm::Serializer serializer{};
        auto res = serializer.serialize(program, (int64_t) mallocI);
        if (res != zasm::Error::None) {
            Errorf("HookOnce: Failed to serialize program %s", getErrorName(res));
            return {nullptr, nullptr, 0, 0};
        }

        void* codePtr = codecaveAlloc(serializer.getCodeSize());
        memcpy(codePtr, serializer.getCode(), serializer.getCodeSize());

        return hookRes;
    }

    /**
     * Creates a persistent hook for a function
     * @param targetInsn Instruction to hook
     * @param hookFn Function to call
     * @param useOffset Whether to use offsets or not
     * @param asmCallback Callback asm modifier
     */
    void HookAssembly(DWORD_PTR targetInsn, DWORD_PTR hookFn, bool useOffset, void(*asmCallback)(zasm::x86::Assembler a))
    {
        zasm::Program cbProgram(zasm::MachineMode::AMD64);
        zasm::x86::Assembler cbA(cbProgram);
        asmCallback(cbA);
        zasm::Serializer cbSerializer{};
        auto cbRes = cbSerializer.serialize(cbProgram, (int64_t) mallocI);
        if (cbRes != zasm::Error::None) {
            Errorf("HookAssembly: Failed to serialize program %s", getErrorName(cbRes));
            return;
        }

        void* cbAsmPtr = nullptr;
        if (cbSerializer.getCodeSize() > 0) {
            cbAsmPtr = codecaveAlloc(cbSerializer.getCodeSize());
            memcpy(cbAsmPtr, cbSerializer.getCode(), cbSerializer.getCodeSize());
        }

        // TODO: Add support for fastcall 5+ args
        int funcOffset = useOffset ? 1 : 0;
        auto hook1 = HookOnce(targetInsn, funcOffset, hookFn,  false, 50, cbAsmPtr);
        size_t hook1Len = hook1.size + hook1.padding;
        Infof("hook1 length: %d", hook1Len);

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
        if (res != zasm::Error::None) {
            Errorf("Hook: Failed to serialize program %s", getErrorName(res));
            return;
        }

        void* codePtr = codecaveAlloc(serializer.getCodeSize());
        memcpy(codePtr, serializer.getCode(), serializer.getCodeSize());

        Infof("hook2 codePtr: %p", codePtr);

        Infof("hook2 at %p", targetInsn+hook1Len);
        auto hook2 = HookOnce(targetInsn+hook1Len, 1, (DWORD_PTR)codePtr, true, 0, nullptr);
        Infof("hook2: %d %d", hook2.size, hook2.padding);

        DWORD _;
        VirtualProtect((LPVOID)targetInsn, hook1Len + hook2.size + hook2.padding, PAGE_EXECUTE_READWRITE, &_);

        memcpy((void*) ((DWORD_PTR)hook1.backupPtr+hook1Len), (void*) (targetInsn+funcOffset+hook1Len), 50 - hook1Len);
        Successf("Hooked into %p+%d", targetInsn, funcOffset);
    }

    /**
     * Creates a persistent hook for a function
     * @param targetInsn Instruction to hook
     * @param hookFn Function to call
     * @param useOffset Whether to use offsets or not
    */
    void Hook(DWORD_PTR targetInsn, DWORD_PTR hookFn, bool useOffset)
    {
        HookAssembly(targetInsn, hookFn, useOffset, [](zasm::x86::Assembler a){});
    }

}