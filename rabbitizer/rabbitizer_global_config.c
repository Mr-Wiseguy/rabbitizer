/* SPDX-FileCopyrightText: © 2022 Decompollaborate */
/* SPDX-License-Identifier: MIT */

/**
 * Wrapper to expose rabbitizer's global configuration
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "common/RabbitizerConfig.h"


#define DEF_MEMBER_GET_BOOL(category, name) \
    static PyObject *rabbitizer_global_config_get_##category##_##name(PyObject *self, PyObject *Py_UNUSED(ignored)) { \
        (void)self; \
        if (RabbitizerConfig_Cfg.category.name) { \
            Py_RETURN_TRUE; \
        } \
        Py_RETURN_FALSE; \
    }
#define DEF_MEMBER_SET_BOOL(category, name) \
    static int rabbitizer_global_config_set_##category##_##name(PyObject *self, PyObject *value, void *Py_UNUSED(closure)) { \
        (void)self; \
        if (value == NULL) { \
            PyErr_SetString(PyExc_TypeError, "Cannot delete '" #category "." #name "' attribute"); \
            return -1; \
        } \
        RabbitizerConfig_Cfg.category.name = PyObject_IsTrue(value); \
        return 0; \
    }

#define DEF_MEMBER_GET_SET_BOOL(category, name) \
    DEF_MEMBER_GET_BOOL(category, name) \
    DEF_MEMBER_SET_BOOL(category, name)


#define DEF_MEMBER_GET_INT(category, name) \
    static PyObject *rabbitizer_global_config_get_##category##_##name(PyObject *self, PyObject *Py_UNUSED(ignored)) { \
        (void)self; \
        return PyLong_FromLong(RabbitizerConfig_Cfg.category.name); \
    }
#define DEF_MEMBER_SET_INT(category, name, rangeCheck, minVal, maxVal) \
    static int rabbitizer_global_config_set_##category##_##name(PyObject *self, PyObject *value, void *Py_UNUSED(closure)) { \
        long val; \
        (void)self; \
        if (value == NULL) { \
            PyErr_SetString(PyExc_TypeError, "Cannot delete '" #category "." #name "' attribute"); \
            return -1; \
        } \
        val = PyLong_AsLong(value);\
        if (val == -1) { \
            PyObject *err = PyErr_Occurred(); \
            if (err != NULL) { \
                return -1; \
            } \
        } \
        if (rangeCheck && (val < minVal || val > maxVal)) { \
            PyErr_SetString(PyExc_TypeError, "Invalid value for '" #category "." #name "' attribute"); \
            return -1; \
        } \
        RabbitizerConfig_Cfg.category.name = val;\
        return 0; \
    }

#define DEF_MEMBER_GET_SET_INT(category, name, rangeCheck, minVal, maxVal) \
    DEF_MEMBER_GET_INT(category, name) \
    DEF_MEMBER_SET_INT(category, name, rangeCheck, minVal, maxVal)


#define MEMBER_GET(category, name, docs, closure)      { #category "_" #name, (getter) rabbitizer_global_config_get_##category##_##name, (setter) NULL,                                             PyDoc_STR(docs), closure }
#define MEMBER_SET(category, name, docs, closure)      { #category "_" #name, (getter) NULL,                                             (setter) rabbitizer_global_config_set_##category##_##name, PyDoc_STR(docs), closure }
#define MEMBER_GET_SET(category, name, docs, closure)  { #category "_" #name, (getter) rabbitizer_global_config_get_##category##_##name, (setter) rabbitizer_global_config_set_##category##_##name, PyDoc_STR(docs), closure }


DEF_MEMBER_GET_SET_BOOL(regNames, namedRegisters)
DEF_MEMBER_GET_SET_INT(regNames, gprAbiNames, true, 0, RABBITIZER_ABI_MAX-1)
DEF_MEMBER_GET_SET_INT(regNames, fprAbiNames, true, 0, RABBITIZER_ABI_MAX-1)
DEF_MEMBER_GET_SET_BOOL(regNames, userFpcCsr)
DEF_MEMBER_GET_SET_BOOL(regNames, vr4300Cop0NamedRegisters)
DEF_MEMBER_GET_SET_BOOL(regNames, vr4300RspCop0NamedRegisters)

DEF_MEMBER_GET_SET_BOOL(pseudos, enablePseudos)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoBeqz)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoBnez)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoB)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoMove)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoNot)
DEF_MEMBER_GET_SET_BOOL(pseudos, pseudoNegu)

DEF_MEMBER_GET_SET_BOOL(toolchainTweaks, treatJAsUnconditionalBranch)
DEF_MEMBER_GET_SET_BOOL(toolchainTweaks, sn64DivFix)

DEF_MEMBER_GET_SET_INT(misc, opcodeLJust, false, 0, 0)
DEF_MEMBER_GET_SET_BOOL(misc, unknownInstrComment)


static PyGetSetDef rabbitizer_global_config_GetSets[] = {
    MEMBER_GET_SET(regNames, namedRegisters, "", NULL),
    MEMBER_GET_SET(regNames, gprAbiNames, "", NULL),
    MEMBER_GET_SET(regNames, fprAbiNames, "", NULL),
    MEMBER_GET_SET(regNames, userFpcCsr, "", NULL),
    MEMBER_GET_SET(regNames, vr4300Cop0NamedRegisters, "", NULL),
    MEMBER_GET_SET(regNames, vr4300RspCop0NamedRegisters, "", NULL),

    MEMBER_GET_SET(pseudos, enablePseudos, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoBeqz, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoBnez, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoB, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoMove, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoNot, "", NULL),
    MEMBER_GET_SET(pseudos, pseudoNegu, "", NULL),

    MEMBER_GET_SET(toolchainTweaks, treatJAsUnconditionalBranch, "", NULL),
    MEMBER_GET_SET(toolchainTweaks, sn64DivFix, "", NULL),

    MEMBER_GET_SET(misc, opcodeLJust, "", NULL),
    MEMBER_GET_SET(misc, unknownInstrComment, "", NULL),

    { 0 },
};

PyTypeObject rabbitizer_global_config_TypeObject = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rabbitizer.config",
    .tp_doc = PyDoc_STR(""),
    .tp_basicsize = sizeof(PyObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_getset = rabbitizer_global_config_GetSets,
};