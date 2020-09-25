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

#include "../macros.h"

#define BUFSIZE 1024

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
                MODEL_CREATE_PARAM(INPUT, LYD_JSON, PARSE_OPTION, LYD_VALIDATE_PRESENT, MODEL)

#define PARSER_CHECK_ERROR(INPUT, PARSE_OPTION, MODEL, RET_VAL, ERR_MESSAGE) \
                assert_int_equal(RET_VAL, lyd_parse_data_mem(CONTEXT_GET, data, LYD_JSON, PARSE_OPTION, LYD_VALIDATE_PRESENT, &MODEL));\
                logbuf_assert(ERR_MESSAGE);\
                assert_null(MODEL)



#define MODEL_CHECK_CHAR(IN_MODEL, TEXT) \
                MODEL_CHECK_CHAR_PARAM(IN_MODEL, TEXT, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS)


const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1; import ietf-yang-metadata {prefix md;}"
            "md:annotation hint { type int8;}"
            "list l1 { key \"a b c\"; leaf a {type string;} leaf b {type string;} leaf c {type int16;} leaf d {type string;}}"
            "leaf foo { type string;}"
            "container c {"
                "leaf x {type string;}"
                "action act { input { leaf al {type string;} } output { leaf al {type uint8;} } }"
                "notification n1 { leaf nl {type string;} }"
            "}"
            "container cp {presence \"container switch\"; leaf y {type string;} leaf z {type int8;}}"
            "anydata any {config false;}"
            "leaf-list ll1 { type uint8; }"
            "leaf foo2 { type string; default \"default-val\"; }"
            "leaf foo3 { type uint32; }"
            "notification n2;}";



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
    (void) state;

    const char *data = "{\"a:foo\":\"foo value\"}";
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    CONTEXT_CREATE;

    MODEL_CREATE(data, 0,  tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "foo value", "");

    LYSC_NODE_CHECK(tree->next->next->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree->next->next;

    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "");
    assert_true(leaf->flags & LYD_DEFAULT);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* make foo2 explicit */
    data = "{\"a:foo2\":\"default-val\"}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "");
    assert_false(leaf->flags & LYD_DEFAULT);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* parse foo2 but make it implicit */
    data = "{\"a:foo2\":\"default-val\",\"@a:foo2\":{\"ietf-netconf-with-defaults:default\":true}}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo2");
    leaf = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(leaf->value, STRING, "default-val", "");
    assert_true(leaf->flags & LYD_DEFAULT);

    /* TODO default values
    lyd_print_tree(out, tree, LYD_JSON, LYD_PRINT_SHRINK);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    */
    MODEL_DESTROY(tree);

    /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:foo\":{\"a:hint\":1,\"a:hint\":2,\"x:xxx\":{\"value\":\"/x:no/x:yes\"}},\"a:foo\":\"xxx\"}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAF, "foo");
    LYD_META_CHECK(tree->meta, tree, "hint", INT8, "1", 1);

    assert_non_null(tree->meta->next);
    LYD_META_CHECK(tree->meta->next, tree, "hint", INT8, "2", 2);
    assert_null(tree->meta->next->next);

    MODEL_CHECK_CHAR(tree, "{\"a:foo\":\"xxx\",\"@a:foo\":{\"a:hint\":1,\"a:hint\":2}}");
    MODEL_DESTROY(tree);

    PARSER_CHECK_ERROR(data, LYD_PARSE_STRICT, tree, LY_EVALID, "Unknown (or not implemented) YANG module \"x\" for metadata \"x:xxx\". /a:foo");

    /* missing referenced metadata node */
    data = "{\"@a:foo\" : { \"a:hint\" : 1 }}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Missing JSON data instance to be coupled with @a:foo metadata. /");

    /* missing namespace for meatadata*/
    data = "{\"a:foo\" : \"value\", \"@a:foo\" : { \"hint\" : 1 }}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Metadata in JSON must be namespace-qualified, missing prefix for \"hint\". /a:foo");

    CONTEXT_DESTROY;
}

static void
test_leaflist(void **state)
{
    (void) state;

    const char *data = "{\"a:ll1\":[10,11]}";
    struct lyd_node *tree;
    struct lyd_node_term *ll;

    CONTEXT_CREATE;

    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(ll->value, UINT8, "10", 10);

    assert_non_null(tree->next);
    LYSC_NODE_CHECK(tree->next->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree->next;
    LYD_VALUE_CHECK(ll->value, UINT8, "11", 11);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* simple metadata */
    data = "{\"a:ll1\":[10,11],\"@a:ll1\":[null,{\"a:hint\":2}]}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(ll->value, UINT8, "10", 10);
    assert_null(ll->meta);

    assert_non_null(tree->next);
    LYSC_NODE_CHECK(tree->next->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree->next;
    LYD_VALUE_CHECK(ll->value, UINT8, "11", 11);
    LYD_META_CHECK(ll->meta, ll, "hint", INT8, "2", 2);
    assert_null(ll->meta->next);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

        /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:ll1\" : [{\"a:hint\" : 1, \"x:xxx\" :  { \"value\" : \"/x:no/x:yes\" }, \"a:hint\" : 10},null,{\"a:hint\" : 3}], \"a:ll1\" : [1,2,3]}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree;
    LYD_VALUE_CHECK(ll->value, UINT8, "1", 1);
    assert_non_null(ll->meta);
    LYD_META_CHECK(ll->meta, ll, "hint", INT8, "1", 1);

    assert_non_null(ll->meta->next);
    LYD_META_CHECK(ll->meta->next, ll->meta->next->parent, "hint", INT8, "10", 10);

    assert_non_null(tree->next);
    LYSC_NODE_CHECK(tree->next->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree->next;
    LYD_VALUE_CHECK(ll->value, UINT8, "2", 2);
    assert_null(ll->meta);

    assert_non_null(tree->next->next);
    LYSC_NODE_CHECK(tree->next->next->schema, LYS_LEAFLIST, "ll1");
    ll = (struct lyd_node_term*)tree->next->next;
    LYD_VALUE_CHECK(ll->value, UINT8, "3", 3);
    assert_non_null(ll->meta);
    LYD_META_CHECK(ll->meta, ll, "hint", INT8, "3", 3);
    assert_null(ll->meta->next);

    MODEL_CHECK_CHAR(tree, "{\"a:ll1\":[1,2,3],\"@a:ll1\":[{\"a:hint\":1,\"a:hint\":10},null,{\"a:hint\":3}]}");
    MODEL_DESTROY(tree);

    /* missing referenced metadata node */
    data = "{\"@a:ll1\":[{\"a:hint\":1}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Missing JSON data instance to be coupled with @a:ll1 metadata. /");

    data = "{\"a:ll1\":[1],\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Missing JSON data instance no. 2 of a:ll1 to be coupled with metadata. /");

    data = "{\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2},{\"a:hint\":3}],\"a:ll1\" : [1, 2]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Missing 3rd JSON data instance to be coupled with @a:ll1 metadata. /");

    CONTEXT_DESTROY;
}

static void
test_anydata(void **state)
{
    (void) state;

    const char *data;
    struct lyd_node *tree;

    CONTEXT_CREATE;

    data = "{\"a:any\":{\"x:element1\":{\"element2\":\"/a:some/a:path\",\"list\":[{},{\"key\":\"a\"}]}}}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_ANYDATA, "any");
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
}

static void
test_list(void **state)
{
    *state = test_list;

    const char *data = "{\"a:l1\":[{\"a\":\"one\",\"b\":\"one\",\"c\":1}]}";
    struct lyd_node *tree, *iter;
    struct lyd_node_inner *list;
    struct lyd_node_term *leaf;

    CONTEXT_CREATE;

    /* check hashes */
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema,LYS_LIST, "l1");
    list = (struct lyd_node_inner*)tree;
    LY_LIST_FOR(list->child, iter) {
        assert_int_not_equal(0, iter->hash);
    }

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* missing keys */
    data = "{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\"}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"a\". /a:l1[b='b'][c='1']");

    data = "{ \"a:l1\": [ {\"a\" : \"a\"}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"b\". /a:l1[a='a']");

    data = "{ \"a:l1\": [ {\"b\" : \"b\", \"a\" : \"a\"}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"c\". /a:l1[a='a'][b='b']");

    /* key duplicate */
    data = "{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\", \"a\" : \"a\", \"c\" : 1}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Duplicate instance of \"c\". /a:l1[a='a'][b='b'][c='1'][c='1']/c");

    /* keys order, in contrast to XML, JSON accepts keys in any order even in strict mode */
    logbuf_clean();
    data = "{ \"a:l1\": [ {\"d\" : \"d\", \"a\" : \"a\", \"c\" : 1, \"b\" : \"b\"}]}";
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
    logbuf_assert("");

    MODEL_CHECK_CHAR(tree, "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1,\"d\":\"d\"}]}");
    MODEL_DESTROY(tree);

    /*  */
    data = "{\"a:l1\":[{\"c\":1,\"b\":\"b\",\"a\":\"a\"}]}";
    MODEL_CREATE(data, LYD_PARSE_STRICT, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_LIST, "l1");
    list = (struct lyd_node_inner*)tree;
    assert_non_null(leaf = (struct lyd_node_term*)list->child);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "a");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "b");
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    LYSC_NODE_CHECK(leaf->schema, LYS_LEAF, "c");
    logbuf_assert("");


    MODEL_CHECK_CHAR(tree, "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1}]}");
    MODEL_DESTROY(tree);

    data = "{\"a:cp\":{\"@\":{\"a:hint\":1}}}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_CONTAINER, "cp");
    assert_non_null(tree->meta);
    LYD_META_CHECK(tree->meta, tree, "hint", INT8, "1", 1);
    assert_null(tree->meta->next);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
}

static void
test_container(void **state)
{
   (void) state;

    const char *data = "{\"a:c\":{}}";
    struct lyd_node *tree;
    struct lyd_node_inner *cont;

    CONTEXT_CREATE;

    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_CONTAINER, "c");
    cont = (struct lyd_node_inner*)tree;
    assert_true(cont->flags & LYD_DEFAULT);

    MODEL_CHECK_CHAR(tree, "{}");
    MODEL_DESTROY(tree);

    data = "{\"a:cp\":{}}";
    MODEL_CREATE(data, 0, tree);
    LYSC_NODE_CHECK(tree->schema, LYS_CONTAINER, "cp");
    cont = (struct lyd_node_inner*)tree;
    assert_false(cont->flags & LYD_DEFAULT);

    MODEL_CHECK_CHAR(tree, data);
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
    data = "{\"a:foo3\":[null]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Invalid empty uint32 value. /a:foo3");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "foo3", "", 0);
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* missing key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"d\":\"val_d\"}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "List instance is missing its key \"c\". /a:l1[a='val_a'][b='val_b']");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0);
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* invalid key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}";
    PARSER_CHECK_ERROR(data, 0, tree, LY_EVALID, "Invalid int16 value \"val_c\". /a:l1/c");

    /* opaq flag */
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0);
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":{\"val\":\"val_c\"}}]}";
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0);
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\"}]}";
    MODEL_CREATE(data, LYD_PARSE_OPAQ, tree);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "l1", "", 0);
    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    CONTEXT_DESTROY;
}

static void
test_rpc(void **state)
{
    (void) state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    data = "{\"ietf-netconf:rpc\":{\"edit-config\":{"
              "\"target\":{\"running\":[null]},"
              "\"config\":{\"a:cp\":{\"z\":[null],\"@z\":{\"ietf-netconf:operation\":\"replace\"}},"
                          "\"a:l1\":[{\"@\":{\"ietf-netconf:operation\":\"replace\"},\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}"
            "}}}";

    CONTEXT_CREATE;
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_RPC, "edit-config");

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc", "", 0);
    /* TODO support generic attributes in JSON ?
    assert_non_null(((struct lyd_node_opaq *)tree)->attr);
    */
    node = lyd_node_children(tree, 0);
    LYSC_NODE_CHECK(node->schema, LYS_RPC, "edit-config");
    node = lyd_node_children(node, 0)->next;
    LYSC_NODE_CHECK(node->schema, LYS_ANYXML, "config");

    node = ((struct lyd_node_any *)node)->value.tree;
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "cp");
    node = lyd_node_children(node, 0);
    /* z has no value */
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "z", "", 0x1);
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "l1", "", 0x1);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
    CONTEXT_DESTROY;
}

static void
test_action(void **state)
{
    *state = test_action;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    CONTEXT_CREATE;

    data = "{\"ietf-netconf:rpc\":{\"yang:action\":{\"a:c\":{\"act\":{\"al\":\"value\"}}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_ACTION, "act");

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc", "", 0);
    node = lyd_node_children(tree, 0);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "action", "", 0);

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
    CONTEXT_DESTROY;
}

static void
test_notification(void **state)
{
    *state = test_notification;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *ntf;
    const struct lyd_node *node;

    CONTEXT_CREATE;

    data = "{\"ietf-restconf:notification\":{\"eventTime\":\"2037-07-08T00:01:00Z\",\"a:c\":{\"n1\":{\"nl\":\"value\"}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(CONTEXT_GET, in, LYD_JSON, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    LYSC_NODE_CHECK(ntf->schema, LYS_NOTIF, "n1");

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "notification", "", 0);
    node = lyd_node_children(tree, 0);
    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)node, "eventTime", "2037-07-08T00:01:00Z", 0);
    node = node->next;
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "c");

    MODEL_CHECK_CHAR(tree, data);
    MODEL_DESTROY(tree);

    /* top-level notif without envelope */
    data = "{\"a:n2\":{}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(CONTEXT_GET, in, LYD_JSON, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    LYSC_NODE_CHECK(ntf->schema, LYS_NOTIF, "n2");

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
    (void) state;

    const char *data;
    struct ly_in *in;
    struct lyd_node *request, *tree, *op;
    const struct lyd_node *node;

    CONTEXT_CREATE;

    data = "{\"a:c\":{\"act\":{\"al\":\"value\"}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(CONTEXT_GET, in, LYD_JSON, &request, NULL));
    ly_in_free(in, 0);

    data = "{\"ietf-netconf:rpc-reply\":{\"a:al\":25}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_reply(request, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);
    MODEL_DESTROY(request);

    assert_non_null(op);
    LYSC_NODE_CHECK(op->schema, LYS_ACTION, "act");
    node = lyd_node_children(op, 0);
    LYSC_NODE_CHECK(node->schema, LYS_LEAF, "al");
    assert_true(node->schema->flags & LYS_CONFIG_R);

    LYD_NODE_OPAQ_CHECK((struct lyd_node_opaq *)tree, "rpc-reply", "", 0);
    node = lyd_node_children(tree, 0);
    LYSC_NODE_CHECK(node->schema, LYS_CONTAINER, "c");

    /* TODO print only rpc-reply node and then output subtree */
    MODEL_CHECK_CHAR(lyd_node_children(op, 0), "{\"a:al\":25}");
    MODEL_CHECK_CHAR(lyd_node_children(tree, 0), "{\"a:c\":{\"act\":{\"al\":25}}}");
    MODEL_DESTROY(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
    CONTEXT_DESTROY;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_leaf),
        cmocka_unit_test(test_leaflist),
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
