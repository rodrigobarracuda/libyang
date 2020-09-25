/**
 * @file test_merge.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for complex data merges.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"


#define MODEL_CREATE(INPUT, MODEL) \
                MODEL_CREATE_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, MODEL)

#define CONTEXT_CREATE \
                CONTEXT_CREATE_PATH(NULL)

#define MODEL_CHECK_CHAR(MODEL, TEXT) \
                MODEL_CHECK_CHAR_PARAM(MODEL, TEXT, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK)

static void
test_batch(void **state)
{

    (void) state;
    const char *start =
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>";
    const char *data[] = {
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
            "<feature>writable-running</feature>"
            "<feature>candidate</feature>"
            "<feature>rollback-on-error</feature>"
            "<feature>validate</feature>"
            "<feature>startup</feature>"
            "<feature>xpath</feature>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    };
    const char *output_template =
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
            "<feature>writable-running</feature>"
            "<feature>candidate</feature>"
            "<feature>rollback-on-error</feature>"
            "<feature>validate</feature>"
            "<feature>startup</feature>"
            "<feature>xpath</feature>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>";

    CONTEXT_CREATE;
    struct lyd_node *target;
    MODEL_CREATE_PARAM(start, LYD_XML, LYD_PARSE_ONLY, 0, target);

    for (int32_t i = 0; i < 11; ++i) {
        struct lyd_node *source;
        MODEL_CREATE_PARAM(data[i], LYD_XML, LYD_PARSE_ONLY, 0, source);
        assert_int_equal(LY_SUCCESS, lyd_merge_siblings(&target, source, LYD_MERGE_DESTRUCT));
    }

    MODEL_CHECK_CHAR(target, output_template);

    MODEL_DESTROY(target);
    CONTEXT_DESTROY;
}

static void
test_leaf(void **state)
{
    (void) state;
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "    container A {"
                    "      leaf f1 {type string;}"
                    "      container B {"
                    "        leaf f2 {type string;}"
                    "      }"
                    "    }"
                    "  }";
    const char *trg = "<A xmlns=\"urn:x\"> <f1>block</f1> </A>";
    const char *src = "<A xmlns=\"urn:x\"> <f1>aa</f1> <B> <f2>bb</f2> </B> </A>";
    const char *result = "<A xmlns=\"urn:x\"><f1>aa</f1><B><f2>bb</f2></B></A>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source); //source
    MODEL_CREATE(trg, target); //target

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    MODEL_CHECK_CHAR(target, result);

    MODEL_DESTROY(target);
    MODEL_DESTROY(source);
    CONTEXT_DESTROY;
}

static void
test_container(void **state)
{
    (void) state;
    const char *sch =
        "module A {"
            "namespace \"aa:A\";"
            "prefix A;"
            "container A {"
                "leaf f1 {type string;}"
                "container B {"
                    "leaf f2 {type string;}"
                "}"
                "container C {"
                    "leaf f3 {type string;}"
                "}"
            "}"
        "}";

    const char *trg = "<A xmlns=\"aa:A\"> <B> <f2>aaa</f2> </B> </A>";
    const char *src = "<A xmlns=\"aa:A\"> <C> <f3>bbb</f3> </C> </A>";
    const char *result = "<A xmlns=\"aa:A\"><B><f2>aaa</f2></B><C><f3>bbb</f3></C></A>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source);
    MODEL_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    MODEL_CHECK_CHAR(target, result);

    /* destroy */
    MODEL_DESTROY(source);
    MODEL_DESTROY(target);
    CONTEXT_DESTROY;
}

static void
test_list(void **state)
{
    (void) state;
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"

        "container inner1 {"
            "list b-list1 {"
                "key p1;"
                "leaf p1 {"
                    "type uint8;"
                "}"
                "leaf p2 {"
                    "type string;"
                "}"
                "leaf p3 {"
                    "type boolean;"
                    "default false;"
                "}"
            "}"
        "}"
    "}";


    const char *trg =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>a</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";
    const char *src =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
        "</b-list1>"
    "</inner1>";
    const char *result =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source);
    MODEL_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    MODEL_CHECK_CHAR(target, result);

    MODEL_DESTROY(target);
    MODEL_DESTROY(source);
    CONTEXT_DESTROY;
}

static void
test_list2(void **state)
{
    (void) state;
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"

        "container inner1 {"
            "list b-list1 {"
                "key p1;"
                "leaf p1 {"
                    "type uint8;"
                "}"
                "leaf p2 {"
                    "type string;"
                "}"
                "container inner2 {"
                    "leaf p3 {"
                        "type boolean;"
                        "default false;"
                    "}"
                    "leaf p4 {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}"
    "}";


    const char *trg =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>a</p2>"
            "<inner2>"
                "<p4>val</p4>"
            "</inner2>"
        "</b-list1>"
    "</inner1>";
    const char *src =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
        "</b-list1>"
    "</inner1>";
    const char *result =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
            "<inner2>"
                "<p4>val</p4>"
            "</inner2>"
        "</b-list1>"
    "</inner1>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source);
    MODEL_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    MODEL_CHECK_CHAR(target, result);

    MODEL_DESTROY(source);
    MODEL_DESTROY(target);
    CONTEXT_DESTROY;
}

static void
test_case(void **state)
{
    (void) state;
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"
        "container cont {"
            "choice ch {"
                "container inner {"
                    "leaf p1 {"
                        "type string;"
                    "}"
                "}"
                "case c2 {"
                    "leaf p1 {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}"
    "}";

    const char *trg =
    "<cont xmlns=\"http://test/merge\">"
        "<inner>"
            "<p1>1</p1>"
        "</inner>"
    "</cont>";
    const char *src =
    "<cont xmlns=\"http://test/merge\">"
        "<p1>1</p1>"
    "</cont>";
    const char *result =
    "<cont xmlns=\"http://test/merge\">"
        "<p1>1</p1>"
    "</cont>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source);
    MODEL_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    MODEL_CHECK_CHAR(target, result);

    MODEL_DESTROY(source);
    MODEL_DESTROY(target);
    CONTEXT_DESTROY;
}

static void
test_dflt(void **state)
{
    (void) state;
    const char *sch =
    "module merge-dflt {"
        "namespace \"urn:merge-dflt\";"
        "prefix md;"
        "container top {"
            "leaf a {"
                "type string;"
            "}"
            "leaf b {"
                "type string;"
            "}"
            "leaf c {"
                "type string;"
                "default \"c_dflt\";"
            "}"
        "}"
    "}";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *target = NULL;
    struct lyd_node *source = NULL;

    assert_int_equal(lyd_new_path(NULL, CONTEXT_GET, "/merge-dflt:top/c", "c_dflt", 0, &target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, CONTEXT_GET, "/merge-dflt:top/a", "a_val", 0, &source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(source, CONTEXT_GET, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&source, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&target, source, LYD_MERGE_DESTRUCT | LYD_MERGE_DEFAULTS), LY_SUCCESS);
    source = NULL;

    /* c should be replaced and now be default */
    assert_string_equal(lyd_node_children(target, 0)->prev->schema->name, "c");
    assert_true(lyd_node_children(target, 0)->prev->flags & LYD_DEFAULT);

    MODEL_DESTROY(target);
    MODEL_DESTROY(source);
    CONTEXT_DESTROY;
}

static void
test_dflt2(void **state)
{
    (void) state;
    const char *sch =
    "module merge-dflt {"
        "namespace \"urn:merge-dflt\";"
        "prefix md;"
        "container top {"
            "leaf a {"
                "type string;"
            "}"
            "leaf b {"
                "type string;"
            "}"
            "leaf c {"
                "type string;"
                "default \"c_dflt\";"
            "}"
        "}"
    "}";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node * target;
    struct lyd_node * source;
    assert_int_equal(lyd_new_path(NULL, CONTEXT_GET, "/merge-dflt:top/c", "c_dflt", 0, &target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, CONTEXT_GET, "/merge-dflt:top/a", "a_val", 0, &source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(source, CONTEXT_GET, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&source, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);

    /* c should not be replaced, so c remains not default */
    assert_false(lyd_node_children(target, 0)->flags & LYD_DEFAULT);

    MODEL_DESTROY(target);
    MODEL_DESTROY(source);
    CONTEXT_DESTROY;
}

static void
test_leafrefs(void **state)
{
    (void) state;
    const char *sch = "module x {"
                      "  namespace urn:x;"
                      "  prefix x;"
                      "  list l {"
                      "    key n;"
                      "    leaf n { type string; }"
                      "    leaf t { type string; }"
                      "    leaf r { type leafref { path '/l/n'; } }}}";
    const char *trg = "<l xmlns=\"urn:x\"><n>a</n></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>";
    const char *src = "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>";
    const char *res = "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>";

    CONTEXT_CREATE;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, sch, LYS_IN_YANG, NULL));

    struct lyd_node *source, *target;
    MODEL_CREATE(src, source);
    MODEL_CREATE(trg, target);

    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);

    MODEL_CHECK_CHAR(target, res);

    MODEL_DESTROY(source);
    MODEL_DESTROY(target);
    CONTEXT_DESTROY;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_batch),
        cmocka_unit_test(test_leaf),
        cmocka_unit_test(test_container),
        cmocka_unit_test(test_list),
        cmocka_unit_test(test_list2),
        cmocka_unit_test(test_case),
        cmocka_unit_test(test_dflt),
        cmocka_unit_test(test_dflt2),
        cmocka_unit_test(test_leafrefs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
