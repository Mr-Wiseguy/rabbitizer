/* SPDX-FileCopyrightText: © 2022 Decompollaborate */
/* SPDX-License-Identifier: MIT */

#include "instructions/RabbitizerInstruction.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "common/Utils.h"
#include "common/RabbitizerConfig.h"
#include "instructions/RabbitizerRegister.h"
#include "instructions/RabbitizerInstrSuffix.h"


#define RAB_DEF_OPERAND(prefix, operand) [RAB_OPERAND_##prefix##_##operand] = RabbitizerOperandType_process_##prefix##_##operand,

const OperandCallback instrOpercandCallbacks[] = {
#include "instructions/operands/RabbitizerOperandType_cpu.inc"
#include "instructions/operands/RabbitizerOperandType_rsp.inc"
#include "instructions/operands/RabbitizerOperandType_r5900.inc"
};

#undef RAB_DEF_OPERAND

size_t RabbitizerInstruction_getSizeForBufferOperandsDisasm(const RabbitizerInstruction *self, size_t immOverrideLength) {
    size_t totalSize = 0;
    char auxBuffer[0x100] = { 0 };
    char immOverride[0x100] = { 0 };

    for (size_t i = 0; i < ARRAY_COUNT(self->descriptor->operands) && self->descriptor->operands[i] != RAB_OPERAND_ALL_INVALID; i++) {
        RabbitizerOperandType operand;
        OperandCallback callback;

        if (i != 0) {
            totalSize += 2;
        }

        operand = self->descriptor->operands[i];
        assert(operand > RAB_OPERAND_ALL_INVALID);
        assert(operand < RAB_OPERAND_ALL_MAX);

        callback = instrOpercandCallbacks[operand];
        assert(callback != NULL);
        totalSize += callback(self, auxBuffer, immOverride, immOverrideLength);
    }

    return totalSize;
}

size_t RabbitizerInstruction_disassembleOperands(const RabbitizerInstruction *self, char *dst, const char *immOverride, size_t immOverrideLength) {
    size_t totalSize = 0;

    for (size_t i = 0; i < ARRAY_COUNT(self->descriptor->operands) && self->descriptor->operands[i] != RAB_OPERAND_ALL_INVALID; i++) {
        RabbitizerOperandType operand;
        OperandCallback callback;

        if (i != 0) {
            RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, ',');
            RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, ' ');
        }

        operand = self->descriptor->operands[i];
        assert(operand > RAB_OPERAND_ALL_INVALID);
        assert(operand < RAB_OPERAND_ALL_MAX);

        callback = instrOpercandCallbacks[operand];
        assert(callback != NULL);

        RABUTILS_BUFFER_ADVANCE(dst, totalSize, callback(self, dst, immOverride, immOverrideLength));
    }

    *dst = '\0';
    return totalSize;
}

size_t RabbitizerInstruction_getSizeForBufferInstrDisasm(const RabbitizerInstruction *self, size_t immOverrideLength, int extraLJust) {
    size_t totalSize = 0;
    size_t opcodeNameLength;

    opcodeNameLength = strlen(RabbitizerInstrId_getOpcodeName(self->uniqueId));

    totalSize += opcodeNameLength;

    totalSize += RabbitizerInstrSuffix_getSizeForBuffer(self, self->descriptor->instrSuffix);

    if (self->descriptor->operands[0] == RAB_OPERAND_ALL_INVALID) {
        // There are no operands
        return totalSize;
    }

    if (RabbitizerConfig_Cfg.misc.opcodeLJust > 0) {
        totalSize += RabbitizerConfig_Cfg.misc.opcodeLJust;
    }
    if (extraLJust > 0) {
        totalSize += extraLJust;
    }
    totalSize++;

    totalSize += RabbitizerInstruction_getSizeForBufferOperandsDisasm(self, immOverrideLength);

    return totalSize;
}

size_t RabbitizerInstruction_disassembleInstruction(const RabbitizerInstruction *self, char *dst, const char *immOverride, size_t immOverrideLength,
                                                    int extraLJust) {
    size_t totalSize = 0;
    const char *opcodeName = RabbitizerInstrId_getOpcodeName(self->uniqueId);

    RABUTILS_BUFFER_CPY(dst, totalSize, opcodeName);

    RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerInstrSuffix_processSuffix(self, dst, self->descriptor->instrSuffix));

    if (self->descriptor->operands[0] == RAB_OPERAND_ALL_INVALID) {
        // There are no operands
        *dst = '\0';
        return totalSize;
    }

    RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerUtils_CharFill(dst, RabbitizerConfig_Cfg.misc.opcodeLJust + extraLJust - totalSize, ' '));
    RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, ' ');

    RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerInstruction_disassembleOperands(self, dst, immOverride, immOverrideLength));

    *dst = '\0';
    return totalSize;
}

size_t RabbitizerInstruction_getSizeForBufferDataDisasm(UNUSED const RabbitizerInstruction *self, int extraLJust) {
    size_t totalSize = 0;

    totalSize += strlen(".word");
    totalSize += RabbitizerConfig_Cfg.misc.opcodeLJust + extraLJust;
    totalSize += 11;
    return totalSize;
}

size_t RabbitizerInstruction_disassembleAsData(const RabbitizerInstruction *self, char *dst, int extraLJust) {
    size_t totalSize = 0;

    RABUTILS_BUFFER_CPY(dst, totalSize, ".word");

    RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerUtils_CharFill(dst, RabbitizerConfig_Cfg.misc.opcodeLJust + extraLJust - totalSize, ' '));

    RABUTILS_BUFFER_SPRINTF(dst, totalSize, " 0x%08X", RabbitizerInstruction_getRaw(self));
    return totalSize;
}

bool RabbitizerInstruction_mustDisasmAsData(const RabbitizerInstruction *self) {
    if (RabbitizerConfig_Cfg.toolchainTweaks.sn64DivFix) {
        if (self->uniqueId == RABBITIZER_INSTR_ID_cpu_break) {
            return true;
        }
    }

    if (!RabbitizerInstruction_isValid(self)) {
        return true;
    }
    return false;
}

size_t RabbitizerInstruction_getSizeForBuffer(const RabbitizerInstruction *self, size_t immOverrideLength, int extraLJust) {
    if (!RabbitizerInstruction_isImplemented(self) || RabbitizerInstruction_mustDisasmAsData(self)) {
        size_t totalSize = RabbitizerInstruction_getSizeForBufferDataDisasm(self, extraLJust);

        if (RabbitizerConfig_Cfg.misc.unknownInstrComment) {
            totalSize += 40;
            totalSize += 3;
            totalSize += RabbitizerInstruction_getSizeForBufferInstrDisasm(self, immOverrideLength, extraLJust);
            totalSize += 11;
        }
        return totalSize;
    }

    return RabbitizerInstruction_getSizeForBufferInstrDisasm(self, immOverrideLength, extraLJust);
}

size_t RabbitizerInstruction_disassemble(const RabbitizerInstruction *self, char *dst, const char *immOverride, size_t immOverrideLength, int extraLJust) {
    assert(dst != NULL);

    if (!RabbitizerInstruction_isImplemented(self) || RabbitizerInstruction_mustDisasmAsData(self)) {
        size_t totalSize = 0;

        RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerInstruction_disassembleAsData(self, dst, extraLJust));

        if (RabbitizerConfig_Cfg.misc.unknownInstrComment) {
            uint32_t validBits;

            RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerUtils_CharFill(dst, 40 - totalSize, ' '));

            RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, ' ');
            RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, '#');
            RABUTILS_BUFFER_WRITE_CHAR(dst, totalSize, ' ');

            RABUTILS_BUFFER_ADVANCE(dst, totalSize, RabbitizerInstruction_disassembleInstruction(self, dst, immOverride, immOverrideLength, extraLJust));

            validBits = RabbitizerInstruction_getValidBits(self);

            RABUTILS_BUFFER_SPRINTF(dst, totalSize, " # %08X", ((~validBits) & self->word));
        }

        return totalSize;
    }

    return RabbitizerInstruction_disassembleInstruction(self, dst, immOverride, immOverrideLength, extraLJust);
}
