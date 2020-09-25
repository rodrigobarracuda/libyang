/**
 * @file   macros.h
 * @author Radek Iša <isa@cesnet.cz>
 * @brief  this file contains macros for simplification test writing
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _LY_UTEST_MACROS_H_
#define _LY_UTEST_MACROS_H_

// cmocka header files
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

//local header files
#include "libyang.h"
#include "tests/config.h"

//macros
#define CONTEXT_GET \
                        ly_context

#define CONTEXT_CREATE_PATH(PATH) \
                        struct ly_ctx *ly_context;\
                        assert_int_equal(LY_SUCCESS, ly_ctx_new(PATH, 0, &ly_context))\

#define CONTEXT_DESTROY \
                        ly_ctx_destroy(ly_context, NULL)


#define MODEL_CREATE_PARAM(INPUT, INPUT_FORMAT, PARAM_PARSE, PARAM_VALID, OUT_MODEL) \
                        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ly_context, INPUT, INPUT_FORMAT, PARAM_PARSE, PARAM_VALID, & OUT_MODEL));\
                        assert_non_null(OUT_MODEL)

#define MODEL_DESTROY(MODEL) \
                        lyd_free_siblings(MODEL);

#define MODEL_CHECK_CHAR_PARAM(IN_MODEL, TEXT, FORMAT, PARAM) \
                         {\
                             char * test;\
                             lyd_print_mem(&test, IN_MODEL, FORMAT, PARAM);\
                             assert_string_equal(test, TEXT);\
                             free(test);\
                         }


#define MODEL_CHECK(IN_MODEL_1, IN_MODEL_2) \
                         {\
                             char * test_1;\
                             char * test_2;\
                             lyd_print_mem(&test_1, IN_MODEL_1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);\
                             lyd_print_mem(&test_2, IN_MODEL_2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);\
                             assert_string_equal(test_1, test_2);\
                             free(test_1);\
                             free(test_2);\
                         }


/* 
    LIBYANG NODE CHECKING   
*/
#define LYSC_NODE_CHECK(NODE, TYPE, NAME) \
                assert_non_null(NODE);\
                assert_string_equal(NAME, (NODE)->name);\
                assert_int_equal(TYPE,  (NODE)->nodetype);\
                assert_non_null((NODE)->prev);

#define LYD_VALUE_CHECK(NODE, TYPE_VAL, CANNONICAL_VAL, VALUE) \
                assert_non_null((NODE).canonical);\
                assert_string_equal(CANNONICAL_VAL, (NODE).canonical);\
                LYD_VALUE_CHECK_ ## TYPE_VAL (NODE, VALUE)



#define LYD_META_CHECK(NODE, PARENT, NAME, TYPE_VAL, CANNONICAL_VAL, VALUE)\
                assert_non_null(NODE);\
                assert_ptr_equal((NODE)->parent, PARENT);\
                assert_string_equal((NODE)->name, NAME);\
                LYD_VALUE_CHECK((NODE)->value, TYPE_VAL, CANNONICAL_VAL, VALUE)


#define LYD_NODE_OPAQ_CHECK(NODE, NAME, VALUE, FLAGS)\
                assert_non_null(NODE);\
                assert_true(((FLAGS & 0x1) == 0) ? ((NODE)->attr == NULL) : ((NODE)->attr != NULL));\
                assert_ptr_equal((NODE)->ctx, CONTEXT_GET);\
                assert_int_equal((NODE)->flags, 0);\
                assert_int_equal((NODE)->hash, 0);\
                assert_string_equal((NODE)->name, NAME);\
                assert_string_equal((NODE)->value, VALUE);\
                assert_non_null((NODE)->prev);\
                assert_null((NODE)->schema)

/* 
    LYD VALUES CHECKING SPECIALIZATION, DONT USE THESE MACROS
*/
#define LYD_VALUE_CHECK_EMPTY(NODE, VALUE) \
                assert_int_equal(LY_TYPE_EMPTY, (NODE).realtype->basetype); \
                assert_non_null((NODE).realtype)

#define LYD_VALUE_CHECK_INT8(NODE, VALUE) \
                assert_non_null((NODE).realtype);\
                assert_int_equal(LY_TYPE_INT8, (NODE).realtype->basetype); \
                assert_int_equal(VALUE, (NODE).int8)

#define LYD_VALUE_CHECK_UINT8(NODE, VALUE) \
                assert_non_null((NODE).realtype);\
                assert_int_equal(LY_TYPE_UINT8, (NODE).realtype->basetype); \
                assert_int_equal(VALUE, (NODE).uint8)


#define LYD_VALUE_CHECK_STRING(NODE, VALUE) \
                assert_non_null((NODE).realtype);\
                assert_int_equal(LY_TYPE_STRING, (NODE).realtype->basetype)
                /*assert_string_equal(VALUE, NODE.ptr)*/

#define LYD_VALUE_CHECK_LEAFREF(NODE, VALUE) \
                assert_non_null((NODE).realtype)\
                assert_int_equal(LY_TYPE_LEAFREF, (NODE).realtype->basetype);\
                assert_non_null((NODE).ptr)

#endif
