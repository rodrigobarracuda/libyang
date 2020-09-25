/**
 * @file test_diff.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for lyd_diff()
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../macros.h"

#define MODEL_CREATE(INPUT, MODEL) \
                MODEL_CREATE_PARAM(INPUT, LYD_XML, LYD_PARSE_ONLY, 0, MODEL)

#define CONTEXT_CREATE \
                CONTEXT_CREATE_PATH(TESTS_DIR_MODULES_YANG); \
                assert_non_null(ly_ctx_load_module( CONTEXT_GET, "ietf-netconf-acm", "2018-02-14")); \
                assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, schema, LYS_IN_YANG, NULL))

#define MODEL_CHECK_CHAR(IN_MODEL, TEXT) \
                MODEL_CHECK_CHAR_PARAM(IN_MODEL, TEXT, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK)

#define MODEL_CREATE_DIFF(INPUT_1, INPUT_2, OUT_MODEL) \
                assert_int_equal(LY_SUCCESS, lyd_diff_siblings(INPUT_1, INPUT_2, 0, &OUT_MODEL));\
                assert_non_null(OUT_MODEL)

#define TEST_DIFF_3(XML_1, XML_2, XML_3, DIFF_1, DIFF_2, MERGE) \
                { \
                    CONTEXT_CREATE;\
                    /*decladation*/\
                    struct lyd_node *model_1;\
                    struct lyd_node *model_2;\
                    struct lyd_node *model_3;\
                    /*create*/\
                    MODEL_CREATE(XML_1, model_1);\
                    MODEL_CREATE(XML_2, model_2);\
                    MODEL_CREATE(XML_3, model_3);\
                    /* diff1 */ \
                    struct lyd_node * diff1;\
                    MODEL_CREATE_DIFF(model_1, model_2, diff1); \
                    MODEL_CHECK_CHAR(diff1, DIFF_1); \
                    assert_int_equal(lyd_diff_apply_all(&model_1, diff1), LY_SUCCESS); \
                    MODEL_CHECK(model_1, model_2); \
                    /* diff2 */ \
                    struct lyd_node * diff2;\
                    MODEL_CREATE_DIFF(model_2, model_3, diff2); \
                    MODEL_CHECK_CHAR(diff2, DIFF_2); \
                    assert_int_equal(lyd_diff_apply_all(&model_2, diff2), LY_SUCCESS);\
                    MODEL_CHECK(model_2, model_3);\
                    /* merge */ \
                    assert_int_equal(lyd_diff_merge_all(&diff1, diff2), LY_SUCCESS);\
                    MODEL_CHECK_CHAR(diff1, MERGE); \
                    /* CREAR ENV */ \
                    MODEL_DESTROY(model_1);\
                    MODEL_DESTROY(model_2);\
                    MODEL_DESTROY(model_3);\
                    MODEL_DESTROY(diff1);\
                    MODEL_DESTROY(diff2);\
                    CONTEXT_DESTROY;\
                }

const char *schema =
    "module defaults {"
        "yang-version 1.1;"
        "namespace \"urn:libyang:tests:defaults\";"
        "prefix df;"

        "feature unhide;"

        "typedef defint32 {"
            "type int32;"
            "default \"42\";"
        "}"

        "leaf hiddenleaf {"
            "if-feature \"unhide\";"
            "type int32;"
            "default \"42\";"
        "}"

        "container df {"
            "leaf foo {"
                "type defint32;"
            "}"

            "leaf hiddenleaf {"
                "if-feature \"unhide\";"
                "type int32;"
                "default \"42\";"
            "}"

            "container bar {"
                "presence \"\";"
                "leaf hi {"
                    "type int32;"
                    "default \"42\";"
                "}"

                "leaf ho {"
                    "type int32;"
                    "mandatory true;"
                "}"
            "}"

            "leaf-list llist {"
                "type defint32;"
                "ordered-by user;"
            "}"

            "leaf-list dllist {"
                "type uint8;"
                "default \"1\";"
                "default \"2\";"
                "default \"3\";"
            "}"

            "list list {"
                "key \"name\";"
                "leaf name {"
                    "type string;"
                "}"

                "leaf value {"
                    "type int32;"
                    "default \"42\";"
                "}"
            "}"

            "choice select {"
                "default \"a\";"
                "case a {"
                    "choice a {"
                        "leaf a1 {"
                            "type int32;"
                            "default \"42\";"
                        "}"

                        "leaf a2 {"
                            "type int32;"
                            "default \"24\";"
                        "}"
                    "}"
                "}"

                "leaf b {"
                    "type string;"
                "}"

                "container c {"
                    "presence \"\";"
                    "leaf x {"
                        "type int32;"
                        "default \"42\";"
                    "}"
                "}"
            "}"

            "choice select2 {"
                "default \"s2b\";"
                "leaf s2a {"
                    "type int32;"
                    "default \"42\";"
                "}"

                "case s2b {"
                    "choice s2b {"
                        "default \"b1\";"
                        "case b1 {"
                            "leaf b1_1 {"
                                "type int32;"
                                "default \"42\";"
                            "}"

                            "leaf b1_2 {"
                                "type string;"
                            "}"

                            "leaf b1_status {"
                                "type int32;"
                                "default \"42\";"
                                "config false;"
                            "}"
                        "}"

                        "leaf b2 {"
                            "type int32;"
                            "default \"42\";"
                        "}"
                    "}"
                "}"
            "}"
        "}"

        "container hidden {"
            "leaf foo {"
                "type int32;"
                "default \"42\";"
            "}"

            "leaf baz {"
                "type int32;"
                "default \"42\";"
            "}"

            "leaf papa {"
                "type int32;"
                "default \"42\";"
                "config false;"
            "}"
        "}"

        "rpc rpc1 {"
            "input {"
                "leaf inleaf1 {"
                    "type string;"
                "}"

                "leaf inleaf2 {"
                    "type string;"
                    "default \"def1\";"
                "}"
            "}"

            "output {"
                "leaf outleaf1 {"
                    "type string;"
                    "default \"def2\";"
                "}"

                "leaf outleaf2 {"
                    "type string;"
                "}"
            "}"
        "}"

        "notification notif {"
            "leaf ntfleaf1 {"
                "type string;"
                "default \"def3\";"
            "}"

            "leaf ntfleaf2 {"
                "type string;"
            "}"
        "}"
    "}";

static void
test_invalid(void **state)
{
    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    CONTEXT_CREATE;

    struct lyd_node *model_1;
    MODEL_CREATE(xml, model_1);

    struct lyd_node * diff = NULL;
    assert_int_equal(lyd_diff_siblings(model_1, lyd_node_children(model_1, 0), 0, &diff), LY_EINVAL);
    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, NULL), LY_EINVAL);

    MODEL_DESTROY(model_1);
    MODEL_DESTROY(diff);
    CONTEXT_DESTROY;
}

static void
test_same(void **state)
{
    (void) state;
    const char *xml = "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                        "<enable-nacm>true</enable-nacm>"
                        "<read-default>permit</read-default>"
                        "<write-default>deny</write-default>"
                        "<exec-default>permit</exec-default>"
                        "<enable-external-groups>true</enable-external-groups>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    CONTEXT_CREATE;

    struct lyd_node *model_1;
    struct lyd_node *model_2;

    MODEL_CREATE(xml, model_1);
    MODEL_CREATE(xml, model_2);

    struct lyd_node * diff = NULL;
    assert_int_equal(lyd_diff_siblings(model_1, model_2, 0, &diff), LY_SUCCESS);
    assert_null(diff);
    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    MODEL_CHECK(model_1, model_2);

    MODEL_DESTROY(model_1);
    MODEL_DESTROY(model_2);
    MODEL_DESTROY(diff);
    CONTEXT_DESTROY;
}

static void
test_empty1(void **state)
{
    (void) state;
    const char *xml_in =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>";

    CONTEXT_CREATE;

    struct lyd_node *model_1 = NULL;
    struct lyd_node *model_2;
    MODEL_CREATE(xml_in, model_2);

    struct lyd_node * diff;
    MODEL_CREATE_DIFF(model_1, model_2, diff);
    MODEL_CHECK_CHAR(diff, "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>");
    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    MODEL_CHECK(model_1, model_2);

    MODEL_DESTROY(model_1);
    MODEL_DESTROY(model_2);
    MODEL_DESTROY(diff);
    CONTEXT_DESTROY;
}

static void
test_empty2(void **state)
{


    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    CONTEXT_CREATE;

    struct lyd_node *model_1;
    MODEL_CREATE(xml, model_1);

    struct lyd_node * diff;
    MODEL_CREATE_DIFF(model_1, NULL, diff);
    MODEL_CHECK_CHAR(diff, "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>" );

    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    assert_ptr_equal(model_1, NULL);

    MODEL_DESTROY(diff);
    MODEL_DESTROY(model_1);
    CONTEXT_DESTROY;
}

static void
test_empty_nested(void **state)
{
    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    CONTEXT_CREATE;

    struct lyd_node *model_1;
    MODEL_CREATE(xml, model_1);

    struct lyd_node * diff = NULL;
    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, &diff), LY_SUCCESS);
    assert_null(diff);

    struct lyd_node * diff1;
    MODEL_CREATE_DIFF(NULL, lyd_node_children(model_1, 0), diff1);
    MODEL_CHECK_CHAR(diff1, "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"create\">42</foo>"
        "</df>");

    struct lyd_node * diff2;
    MODEL_CREATE_DIFF(lyd_node_children(model_1, 0), NULL, diff2);
    MODEL_CHECK_CHAR(diff2, "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"delete\">42</foo>"
        "</df>");

    MODEL_DESTROY(model_1);
    MODEL_DESTROY(diff1);
    MODEL_DESTROY(diff2);
    CONTEXT_DESTROY;
}

static void
test_leaf(void **state)
{
    (void) state;
    const char *xml1 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>";
    const char *xml2 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>41</foo><b1_1>42</b1_1>"
        "</df>";
    const char *xml3 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>40</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>40</foo>"
        "</hidden>";

    const char *out_diff_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">41</foo>"
            "<b1_1 yang:operation=\"create\">42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo>"
            "<baz>42</baz>"
        "</hidden>";

    const char *out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"41\">40</foo>"
            "<b1_1 yang:operation=\"delete\">42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>40</foo>"
        "</hidden>";

    const char *out_merge = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">40</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">40</foo>"
            "<baz yang:operation=\"delete\">42</baz>"
        "</hidden>";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_list(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>a</name><value>1</value></list>"
                         "<list><name>b</name><value>2</value></list>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>b</name><value>-2</value></list>"
                         "<list><name>c</name><value>3</value></list>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>b</name><value>-2</value></list>"
                         "<list><name>a</name><value>2</value></list>"
                       "</df>";

    const char * out_diff_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"delete\">"
                "<name>a</name>"
                "<value>1</value>"
            "</list>"
            "<list yang:operation=\"none\">"
                "<name>b</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"2\">-2</value>"
            "</list>"
            "<list yang:operation=\"create\">"
                "<name>c</name>"
                "<value>3</value>"
            "</list>"
        "</df>";
    const char * out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"delete\">"
                "<name>c</name>"
                "<value>3</value>"
            "</list>"
            "<list yang:operation=\"create\">"
                "<name>a</name>"
                "<value>2</value>"
            "</list>"
        "</df>";
    const char * out_merge  = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"none\">"
                "<name>a</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"1\">2</value>"
            "</list>"
            "<list yang:operation=\"none\">"
                "<name>b</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"2\">-2</value>"
            "</list>"
        "</df>";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_llist(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                         "<llist>5</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>2</llist>"
                         "<llist>5</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>5</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>2</llist>"
                       "</df>";


    const char *out_diff_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>"
        "</df>";
    const char *out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"\">1</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"\">5</llist>"
        "</df>";
    const char *out_merge  = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>"
            "<llist yang:orig-value=\"\" yang:operation=\"delete\">1</llist>"
            "<llist yang:orig-value=\"2\" yang:value=\"\" yang:operation=\"replace\">5</llist>"
        "</df>";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_llist2(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>4</llist>"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>3</llist>"
                       "</df>";

    const char *out_diff_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"2\">4</llist>"
        "</df>";
    const char *out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">4</llist>"
        "</df>";
    const char *out_merge  = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"\">4</llist>"
            "<llist yang:orig-value=\"1\" yang:operation=\"delete\">2</llist>"
        "</df>";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_mix(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>3</llist>"
                         "<llist>1</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                       "</df>";

    const char * out_diff_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">3</llist>"
        "</df>";
    const char * out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"\">1</llist>"
            "<llist yang:operation=\"create\" yang:value=\"1\">4</llist>"
        "</df>";
    const char * out_merge  = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">3</llist>"
            "<llist yang:orig-value=\"3\" yang:value=\"\" yang:operation=\"replace\">1</llist>"
            "<llist yang:value=\"1\" yang:operation=\"create\">4</llist>"
        "</df>";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_wd(void **state)
{
    (void) state;
    const struct lys_module *mod;
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<foo>41</foo><dllist>4</dllist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<foo>42</foo><dllist>4</dllist><dllist>1</dllist>"
                       "</df>";

    CONTEXT_CREATE;
    mod = ly_ctx_get_module_implemented(CONTEXT_GET, "defaults");
    assert_non_null(mod);

    struct lyd_node *model_1 = NULL;
    assert_int_equal(lyd_validate_module(&model_1, mod, 0, NULL), LY_SUCCESS);
    assert_ptr_not_equal(model_1, NULL);

    struct lyd_node *model_2;
    struct lyd_node *model_3;
    MODEL_CREATE_PARAM(xml2, LYD_XML, 0, LYD_VALIDATE_PRESENT, model_2);
    MODEL_CREATE_PARAM(xml3, LYD_XML, 0, LYD_VALIDATE_PRESENT, model_3);

    /* diff1 */
    struct lyd_node *diff1 = NULL;
    assert_int_equal(lyd_diff_siblings(model_1, model_2, LYD_DIFF_DEFAULTS, &diff1), LY_SUCCESS);
    assert_non_null(diff1);

    const char *diff1_out_1 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-default=\"true\" yang:orig-value=\"42\">41</foo>"
            "<dllist yang:operation=\"delete\">1</dllist>"
            "<dllist yang:operation=\"delete\">2</dllist>"
            "<dllist yang:operation=\"delete\">3</dllist>"
            "<dllist yang:operation=\"create\">4</dllist>"
        "</df>";

    MODEL_CHECK_CHAR_PARAM(diff1, diff1_out_1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK);
    assert_int_equal(lyd_diff_apply_all(&model_1, diff1), LY_SUCCESS);
    MODEL_CHECK(model_1, model_2);

    /* diff2 */
    struct lyd_node *diff2;
    assert_int_equal(lyd_diff_siblings(model_2, model_3, LYD_DIFF_DEFAULTS, &diff2), LY_SUCCESS);
    assert_non_null(diff2);
    MODEL_CHECK_CHAR(diff2, "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"41\">42</foo>"
            "<dllist yang:operation=\"create\">1</dllist>"
        "</df>");

    assert_int_equal(lyd_diff_apply_all(&model_2, diff2), LY_SUCCESS);
    MODEL_CHECK(model_2, model_3);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&diff1, diff2), LY_SUCCESS);

    const char *diff1_out_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:orig-default=\"true\" yang:operation=\"none\">42</foo>"
            "<dllist yang:operation=\"none\" yang:orig-default=\"true\">1</dllist>"
            "<dllist yang:operation=\"delete\">2</dllist>"
            "<dllist yang:operation=\"delete\">3</dllist>"
            "<dllist yang:operation=\"create\">4</dllist>"
        "</df>";
    MODEL_CHECK_CHAR_PARAM(diff1, diff1_out_2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK);

    MODEL_DESTROY(model_1);
    MODEL_DESTROY(model_2);
    MODEL_DESTROY(model_3);
    MODEL_DESTROY(diff1);
    MODEL_DESTROY(diff2);
    CONTEXT_DESTROY;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_invalid),
        cmocka_unit_test(test_same),
        cmocka_unit_test(test_empty1),
        cmocka_unit_test(test_empty2),
        cmocka_unit_test(test_empty_nested),
        cmocka_unit_test(test_leaf),
        cmocka_unit_test(test_list),
        cmocka_unit_test(test_userord_llist),
        cmocka_unit_test(test_userord_llist2),
        cmocka_unit_test(test_userord_mix),
        cmocka_unit_test(test_wd),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
