/* SPDX-FileCopyrightText: © 2022 Decompollaborate */
/* SPDX-License-Identifier: MIT */

#ifndef RABBITIZER_INSTRCATEGORY_H
#define RABBITIZER_INSTRCATEGORY_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define RABBITIZER_DEF_INSTR_CATEGORY(name) RABBITIZER_INSTRCAT_##name

typedef enum RabbitizerInstrCategory {
    #include "instructions/InstrCategory.inc"

    RABBITIZER_DEF_INSTR_CATEGORY(MAX),
} RabbitizerInstrCategory;

#undef RABBITIZER_DEF_INSTR_CATEGORY

extern const char *const RabbitizerInstrCategory_Names[];

RabbitizerInstrCategory RabbitizerInstrCategory_fromStr(const char *name);


#ifdef __cplusplus
}
#endif

#endif
