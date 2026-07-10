/*
 * test_android_resources.c — Android resource/asset extraction tests.
 */
#include "test_framework.h"

#include "android/apk_xapk_resources.h"
#include "graph_buffer/graph_buffer.h"

#include <string.h>

TEST(android_resources_emit_strings_layout_assets_and_urls) {
    cbm_gbuf_t *gb = cbm_gbuf_new("android-res", "/tmp/android-res");
    ASSERT_NOT_NULL(gb);

    const char *strings =
        "<resources>\n"
        "  <string name=\"app_name\">Example</string>\n"
        "  <string name=\"api_base\">https://api.example.com/v1</string>\n"
        "</resources>\n";
    cbm_android_resource_stats_t stats;
    ASSERT_EQ(cbm_android_resources_emit_file(gb, "android-res", "res/values/strings.xml", strings,
                                               (int)strlen(strings), &stats),
              0);
    ASSERT_EQ(stats.resource_files, 1);
    ASSERT_EQ(stats.string_resources, 2);
    ASSERT_EQ(stats.api_endpoints, 1);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:app:android-res"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:resource:string:app_name"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:resource:string:api_base"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:endpoint:https://api.example.com/v1"));

    ASSERT_EQ(cbm_android_resources_emit_file(gb, "android-res", "res/layout/activity_main.xml", NULL, 0,
                                               &stats),
              0);
    ASSERT_EQ(stats.layout_resources, 1);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:resource:layout:activity_main"));

    ASSERT_EQ(cbm_android_resources_emit_file(gb, "android-res", "assets/config.json", NULL, 0, &stats), 0);
    ASSERT_EQ(stats.assets, 1);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:asset:android-res:assets/config.json"));

    ASSERT_EQ(cbm_android_resources_emit_file(gb, "android-res", "lib/arm64-v8a/libnative.so", NULL, 0,
                                               &stats),
              0);
    ASSERT_EQ(stats.native_libraries, 1);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:native:android-res:lib/arm64-v8a/libnative.so"));

    ASSERT_GTE(cbm_gbuf_edge_count_by_type(gb, "HAS_RESOURCE"), 3);
    ASSERT_GTE(cbm_gbuf_edge_count_by_type(gb, "CONTAINS_RESOURCE"), 3);
    ASSERT_GTE(cbm_gbuf_edge_count_by_type(gb, "CONTAINS"), 3);

    cbm_gbuf_free(gb);
    PASS();
}

TEST(android_resources_ignore_unrelated_files) {
    cbm_gbuf_t *gb = cbm_gbuf_new("android-res-empty", "/tmp/android-res-empty");
    ASSERT_NOT_NULL(gb);
    cbm_android_resource_stats_t stats;
    ASSERT_EQ(cbm_android_resources_emit_file(gb, "android-res-empty", "src/Main.java", NULL, 0, &stats), 0);
    ASSERT_EQ(stats.resource_files, 0);
    ASSERT_EQ(stats.string_resources, 0);
    ASSERT_EQ(stats.assets, 0);
    ASSERT_EQ(stats.native_libraries, 0);
    /* The extractor only emits app/resource facts for Android resource-like paths. */
    ASSERT_EQ(cbm_gbuf_node_count(gb), 1);
    cbm_gbuf_free(gb);
    PASS();
}

SUITE(android_resources) {
    RUN_TEST(android_resources_emit_strings_layout_assets_and_urls);
    RUN_TEST(android_resources_ignore_unrelated_files);
}
