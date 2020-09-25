/*
 * @file test_parser_xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_xml.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../macros.h"

#include <stdio.h>
#include <string.h>

#include "context.h"
#include "parser.h"
#include "parser_data.h"
#include "printer.h"
#include "printer_data.h"
#include "tests/config.h"
#include "tree_data_internal.h"
#include "tree_schema.h"


const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;"
            "list l1 { key \"a b c\"; leaf a {type string;} leaf b {type string;} leaf c {type int16;} leaf d {type string;}}"
            "leaf foo { type string;}"
            "container c {"
                "leaf x {type string;}"
                "action act { input { leaf al {type string;} } output { leaf al {type uint8;} } }"
                "notification n1 { leaf nl {type string;} }"
            "}"
            "container cp {presence \"container switch\"; leaf y {type string;} leaf z {type int8;}}"
            "anydata any {config false;}"
            "leaf foo2 { type string; default \"default-val\"; }"
            "leaf foo3 { type uint32; }"
            "notification n2;}";



#define CONTEXT_CREATE \
                CONTEXT_CREATE_PATH(TESTS_DIR_MODULES_YANG);\
                assert_non_null(ly_ctx_load_module(CONTEXT_GET, "ietf-netconf-with-defaults", "2011-06-01"));\
                {\
                    const struct lys_module *mod;\
                    assert_non_null((mod = ly_ctx_load_module(CONTEXT_GET, "ietf-netconf", "2011-06-01")));\
                    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "writable-running"));\
                }\
                assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, schema_a, LYS_IN_YANG, NULL));\
                ly_set_log_clb(logger, 1)



#define MODEL_CREATE(INPUT, PARSE_OPTION, MODEL) \
                MODEL_CREATE_PARAM(INPUT, LYD_XML, PARSE_OPTION, LYD_VALIDATE_PRESENT, MODEL)

#define PARSER_CHECK_ERROR(INPUT, PARSE_OPTION, MODEL, RET_VAL, ERR_MESSAGE) \
                assert_int_equal(RET_VAL, lyd_parse_data_mem(CONTEXT_GET, data, LYD_XML, PARSE_OPTION, LYD_VALIDATE_PRESENT, &MODEL));\
                logbuf_assert(ERR_MESSAGE);\
                assert_null(MODEL)



#define MODEL_CHECK_CHAR(IN_MODEL, TEXT) \
                MODEL_CHECK_CHAR_PARAM(IN_MODEL, TEXT, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS)





#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

struct ly_ctx *ctx; /* context for tests */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}
#endif


void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_leaf(void **state)
{
    *state = test_leaf;

    const char *data = "<foo xmlns=\"urn:tests:a\">foo value</foo>";
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    CONTEXT_CREATE;

    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "foo value", "foo value");

    LYSC_NODE_CHECK(tree->next->next->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree->next->next;
    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);

    MODEL_DESTROY(tree);

    /* make foo2 explicit */
    data = "<foo2 xmlns=\"urn:tests:a\">default-val</foo2>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "default-val");
    assert_false(leaf->flags & LYD_DEFAULT);

    MODEL_DESTROY(tree);

    /* parse foo2 but make it implicit, skip metadata xxx from missing schema */
    data = "<foo2 xmlns=\"urn:tests:a\" xmlns:wd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" wd:default=\"true\" xmlns:x=\"urn:x\" x:xxx=\"false\">default-val</foo2>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);

    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
}

static void
test_anydata(void **state)
{
    (void) state;

    const char *data;
    struct lyd_node *tree;


    CONTEXT_CREATE;

    data =
    "<any xmlns=\"urn:tests:a\">"
        "<element1>"
            "<x:element2 x:attr2=\"test\" xmlns:a=\"urn:tests:a\" xmlns:x=\"urn:x\">a:data</x:element2>"
        "</element1>"
        "<element1a/>"
    "</any>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_ANYDATA, "any");
    const char *data_expected = "<any xmlns=\"urn:tests:a\">"
            "<element1>"
                "<element2 xmlns=\"urn:x\" xmlns:x=\"urn:x\" x:attr2=\"test\" xmlns:a=\"urn:tests:a\">a:data</element2>"
            "</element1>"
            "<element1a/>"
        "</any>";
    MODEL_CHECK_CHAR(tree, data_expected);

    MODEL_DESTROY(tree);
    CONTEXT_DESTROY;
}

static void
test_list(void **state)
{
    (void) state;

    const char *data = "<l1 xmlns=\"urn:tests:a\"><a>one</a><b>one</b><c>1</c></l1>";
    struct lyd_node *tree, *iter;
    struct lyd_node_inner *list;
    struct lyd_node_term *leaf;

    /* check hashes */
    CONTEXT_CREATE;

    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LIST, "l1");
    list = (struct lyd_node_inner*)tree;
    LY_LIST_FOR(list->child, iter) {
        assert_int_not_equal(0, iter->hash);
    }
    MODEL_DESTROY(tree); 

    /* missing keys */
    data = "<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"a\". /a:l1[b='b'][c='1']");

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"b\". /a:l1[a='a']");

    data = "<l1 xmlns=\"urn:tests:a\"><b>b</b><a>a</a></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"c\". /a:l1[a='a'][b='b']");

    /* key duplicate */
    data = "<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b><a>a</a><c>1</c></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Duplicate instance of \"c\". /a:l1[a='a'][b='b'][c='1'][c='1']/c");

    /* keys order */
    data = "<l1 xmlns=\"urn:tests:a\"><d>d</d><a>a</a><c>1</c><b>b</b></l1>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LIST, "l1");
    list = (struct lyd_node_inner*)tree;
    assert_non_null(leaf = (struct lyd_node_term*)list->child);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "a");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "b");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "c");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "d");
    logbuf_assert("Invalid position of the key \"b\" in a list.");
    logbuf_clean();
    MODEL_DESTROY(tree); 

    data = "<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b><a>a</a></l1>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LIST, "l1");
    list = (struct lyd_node_inner*)tree;
    assert_non_null(leaf = (struct lyd_node_term*)list->child);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "a");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "b");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "c");
    logbuf_assert("Invalid position of the key \"a\" in a list.");
    logbuf_clean();
    MODEL_DESTROY(tree);


    PARSER_CHECK_ERROR(data, LYD_PARSE_STRICT, tree, LY_EVALID, "Invalid position of the key \"b\" in a list. Line number 1.");

    CONTEXT_DESTROY;
}

static void
test_container(void **state)
{
    (void) state;

    const char *data = "<c xmlns=\"urn:tests:a\"/>";
    struct lyd_node *tree;
    struct lyd_node_inner *cont;

    CONTEXT_CREATE;
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_CONTAINER, "c");
    cont = (struct lyd_node_inner*)tree;
    assert_true(cont->flags & LYD_DEFAULT);
    MODEL_DESTROY(tree);

    data = "<cp xmlns=\"urn:tests:a\"/>";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_CONTAINER, "cp");
    cont = (struct lyd_node_inner*)tree;
    assert_false(cont->flags & LYD_DEFAULT);
    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
}

static void
test_opaq(void **state)
{
    (void) state;
    const char *data;
    struct lyd_node *tree;

    CONTEXT_CREATE;

    /* invalid value, no flags */
    data = "<foo3 xmlns=\"urn:tests:a\"/>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Invalid empty uint32 value. /a:foo3");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    assert_null(tree->schema);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "foo3", "", 0); 
    MODEL_CHECK_CHAR(tree, "<foo3 xmlns=\"urn:tests:a\"/>");
    MODEL_DESTROY(tree);

    /* missing key, no flags */
    data = "<l1 xmlns=\"urn:tests:a\"><a>val_a</a><b>val_b</b><d>val_d</d></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"c\". /a:l1[a='val_a'][b='val_b']");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    assert_null(tree->schema);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0); 
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* invalid key, no flags */
    data = "<l1 xmlns=\"urn:tests:a\"><a>val_a</a><b>val_b</b><c>val_c</c></l1>";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Invalid int16 value \"val_c\". /a:l1/c");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    assert_null(tree->schema);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0); 
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* opaq flag and fail */
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(CONTEXT_GET, "<a xmlns=\"ns\"><b>x</b><c xml:id=\"D\">1</c></a>", LYD_XML,
                                                   LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Unknown XML prefix \"xml\". Line number 1.");

    CONTEXT_DESTROY;
}

static void
test_rpc(void **state)
{
    (void ) state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    data =
        "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" msgid=\"25\" custom-attr=\"val\">"
            "<edit-config>"
                "<target>"
                    "<running/>"
                "</target>"
                "<config xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
                    "<l1 xmlns=\"urn:tests:a\" nc:operation=\"replace\">"
                        "<a>val_a</a>"
                        "<b>val_b</b>"
                        "<c>val_c</c>"
                    "</l1>"
                    "<cp xmlns=\"urn:tests:a\">"
                        "<z nc:operation=\"delete\"/>"
                    "</cp>"
                "</config>"
            "</edit-config>"
        "</rpc>";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_XML, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_RPC , "edit-config");

    assert_non_null(tree);
    assert_null(tree->schema);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc", "",0x1);
    node = lyd_node_children(tree, 0);
    LYSC_NODE_CHECK(node->schema, LYS_RPC , "edit-config");
    node = lyd_node_children(node, 0)->next;
    LYSC_NODE_CHECK(node->schema, LYS_ANYXML, "config");

    node = ((struct lyd_node_any *)node)->value.tree;
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "cp");

    node = lyd_node_children(node, 0);
    /* z has no value */
    assert_null(node->schema);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "z", "",0x1);
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "l1", "",0x1);

    const char *str_expected = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" msgid=\"25\" custom-attr=\"val\">"
            "<edit-config>"
                "<target>"
                    "<running/>"
                "</target>"
                "<config>"
                    "<cp xmlns=\"urn:tests:a\">"
                        "<z xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"delete\"/>"
                    "</cp>"
                    "<l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\">"
                        "<a>val_a</a>"
                        "<b>val_b</b>"
                        "<c>val_c</c>"
                    "</l1>"
                "</config>"
            "</edit-config>"
        "</rpc>";
    MODEL_CHECK_CHAR(tree, str_expected);

    MODEL_DESTROY(tree);
    CONTEXT_DESTROY;
    /* wrong namespace, element name, whatever... */
    /* TODO */

}

static void
test_action(void **state)
{
    (void) state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    CONTEXT_CREATE;    

    data =
        "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" msgid=\"25\" custom-attr=\"val\">"
            "<action xmlns=\"urn:ietf:params:xml:ns:yang:1\">"
                "<c xmlns=\"urn:tests:a\">"
                    "<act>"
                        "<al>value</al>"
                    "</act>"
                "</c>"
            "</action>"
        "</rpc>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_XML, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_ACTION,"act");

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc", "",0x1);
    node = lyd_node_children(tree, 0);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "action", "",0x0);
    
    const char *str_exp = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" msgid=\"25\" custom-attr=\"val\">"
            "<action xmlns=\"urn:ietf:params:xml:ns:yang:1\">"
                "<c xmlns=\"urn:tests:a\">"
                    "<act>"
                        "<al>value</al>"
                    "</act>"
                "</c>"
            "</action>"
        "</rpc>";

    MODEL_CHECK_CHAR(tree, str_exp);

    MODEL_DESTROY(tree);
    CONTEXT_DESTROY;
    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_notification(void **state)
{
    (void) state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *ntf;
    const struct lyd_node *node;

    CONTEXT_CREATE;

    data =
        "<notification xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\">"
            "<eventTime>2037-07-08T00:01:00Z</eventTime>"
            "<c xmlns=\"urn:tests:a\">"
                "<n1>"
                    "<nl>value</nl>"
                "</n1>"
            "</c>"
        "</notification>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(CONTEXT_GET, in, LYD_XML, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    LYSC_NODE_CHECK(ntf->schema, LYS_NOTIF,"n1");

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "notification", "", 0x0);
    node = lyd_node_children(tree, 0);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "eventTime", "2037-07-08T00:01:00Z", 0x0);
    node = node->next;
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "c");

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* top-level notif without envelope */
    data = "<n2 xmlns=\"urn:tests:a\"/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(CONTEXT_GET, in, LYD_XML, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    LYSC_NODE_CHECK(ntf->schema, LYS_NOTIF,"n2");

    assert_non_null(tree);
    assert_ptr_equal(ntf, tree);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_reply(void **state)
{
    (void )state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *request, *tree, *op;
    const struct lyd_node *node;

    CONTEXT_CREATE;

    data =
        "<c xmlns=\"urn:tests:a\">"
            "<act>"
                "<al>value</al>"
            "</act>"
        "</c>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_XML, &request, NULL));
    ly_in_free(in, 0);

    data =
        "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" msgid=\"25\">"
            "<al xmlns=\"urn:tests:a\">25</al>"
        "</rpc-reply>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_reply(request, in, LYD_XML, &tree, &op));
    ly_in_free(in, 0);
    lyd_free_all(request);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_ACTION, "act");
    node = lyd_node_children(op, 0);
    LYSC_NODE_CHECK(node->schema, LYS_LEAF, "al");
    assert_true(node->schema->flags & LYS_CONFIG_R);


    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc-reply", "", 0x1);
    node = lyd_node_children(tree, 0);
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "c");
    node = lyd_node_children(tree, 0);
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "c");

    /* TODO print only rpc-reply node and then output subtree */
    MODEL_CHECK_CHAR(lyd_node_children(op, 0), "<al xmlns=\"urn:tests:a\">25</al>");
    MODEL_DESTROY(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
    CONTEXT_DESTROY;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_leaf),
        cmocka_unit_test(test_anydata),
        cmocka_unit_test(test_list),
        cmocka_unit_test(test_container),
        cmocka_unit_test(test_opaq),
        cmocka_unit_test(test_rpc),
        cmocka_unit_test(test_action),
        cmocka_unit_test(test_notification),
        cmocka_unit_test(test_reply),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
