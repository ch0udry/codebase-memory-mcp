/*
 * test_android_graph.c — Android graph helper tests.
 */
#include "test_framework.h"

#include "android/apk_xapk_graph.h"
#include "graph_buffer/graph_buffer.h"

#include <string.h>

TEST(android_graph_builds_stable_qn_and_props) {
    char qn[256];
    ASSERT_EQ(cbm_android_graph_qn(qn, sizeof(qn), "component", "com.example", "activity",
                                   "com.example.MainActivity"),
              0);
    ASSERT_STR_EQ(qn, "android:component:com.example:activity:com.example.MainActivity");

    cbm_android_graph_prop_t extra[] = {{"package", "com.example"}, {"exported", "true"}};
    char props[1024];
    ASSERT_EQ(cbm_android_graph_props(props, sizeof(props), "manifest", "high", "AndroidManifest.xml",
                                      "activity", extra, 2),
              0);
    ASSERT_NOT_NULL(strstr(props, "\"source_format\":\"manifest\""));
    ASSERT_NOT_NULL(strstr(props, "\"confidence\":\"high\""));
    ASSERT_NOT_NULL(strstr(props, "\"evidence_path\":\"AndroidManifest.xml\""));
    ASSERT_NOT_NULL(strstr(props, "\"android_kind\":\"activity\""));
    ASSERT_NOT_NULL(strstr(props, "\"exported\":\"true\""));
    PASS();
}

TEST(android_graph_upsert_and_edge_validate_vocabulary) {
    cbm_gbuf_t *gb = cbm_gbuf_new("android-test", "/tmp/android-test");
    ASSERT_NOT_NULL(gb);

    int64_t app = cbm_android_graph_upsert_node(gb, "AndroidApp", "com.example",
                                                "android:app:com.example", "AndroidManifest.xml", 1,
                                                1, "{}");
    ASSERT_GT(app, 0);
    int64_t manifest = cbm_android_graph_upsert_node(gb, "Manifest", "AndroidManifest.xml",
                                                     "android:manifest:android-test:AndroidManifest.xml",
                                                     "AndroidManifest.xml", 1, 1, "{}");
    ASSERT_GT(manifest, 0);
    ASSERT_EQ(cbm_android_graph_insert_edge(gb, app, manifest, "HAS_MANIFEST", "{}") > 0, 1);
    ASSERT_EQ(cbm_gbuf_node_count(gb), 2);
    ASSERT_EQ(cbm_gbuf_edge_count(gb), 1);

    ASSERT_EQ(cbm_android_graph_upsert_node(gb, "NotAndroidNode", "bad", "android:bad", NULL, 0, 0,
                                            "{}"),
              0);
    ASSERT_EQ(cbm_android_graph_insert_edge(gb, app, manifest, "NOT_ANDROID_EDGE", "{}"), 0);

    cbm_gbuf_free(gb);
    PASS();
}

SUITE(android_graph) {
    RUN_TEST(android_graph_builds_stable_qn_and_props);
    RUN_TEST(android_graph_upsert_and_edge_validate_vocabulary);
}
