/*
 * test_android_manifest.c — AndroidManifest.xml semantic extraction tests.
 */
#include "test_framework.h"

#include "android/apk_xapk_manifest.h"
#include "graph_buffer/graph_buffer.h"

#include <string.h>

static const char *ANDROID_MANIFEST_FIXTURE =
    "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\" package=\"com.example.app\">\n"
    "  <uses-permission android:name=\"android.permission.INTERNET\" />\n"
    "  <application android:debuggable=\"false\">\n"
    "    <activity android:name=\".MainActivity\" android:exported=\"true\">\n"
    "      <intent-filter>\n"
    "        <action android:name=\"android.intent.action.VIEW\" />\n"
    "        <category android:name=\"android.intent.category.BROWSABLE\" />\n"
    "        <data android:scheme=\"https\" android:host=\"example.com\" android:pathPrefix=\"/open\" />\n"
    "      </intent-filter>\n"
    "    </activity>\n"
    "    <service android:name=\"com.example.app.SyncService\" android:enabled=\"true\" />\n"
    "    <receiver android:name=\".BootReceiver\" android:exported=\"false\" />\n"
    "    <provider android:name=\".DataProvider\" android:authorities=\"com.example.app.provider\" />\n"
    "  </application>\n"
    "</manifest>\n";

TEST(android_manifest_emits_core_nodes_and_edges) {
    cbm_gbuf_t *gb = cbm_gbuf_new("android-manifest", "/tmp/android-manifest");
    ASSERT_NOT_NULL(gb);

    cbm_android_manifest_stats_t stats;
    ASSERT_EQ(cbm_android_manifest_emit(gb, "android-manifest", "AndroidManifest.xml",
                                         ANDROID_MANIFEST_FIXTURE,
                                         (int)strlen(ANDROID_MANIFEST_FIXTURE), &stats),
              0);
    ASSERT_EQ(stats.apps, 1);
    ASSERT_EQ(stats.manifests, 1);
    ASSERT_EQ(stats.components, 4);
    ASSERT_EQ(stats.permissions, 1);
    ASSERT_EQ(stats.intent_filters, 1);
    ASSERT_EQ(stats.deep_links, 1);

    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:app:com.example.app"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:manifest:android-manifest:AndroidManifest.xml"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb,
                                        "android:component:com.example.app:activity:com.example.app.MainActivity"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:permission:android.permission.INTERNET"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:deeplink:https://example.com/open"));
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "HAS_MANIFEST"), 1);
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "DECLARES_COMPONENT"), 4);
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "REQUESTS_PERMISSION"), 1);
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "HAS_INTENT_FILTER"), 1);
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "EXPOSES_DEEPLINK"), 1);

    const cbm_gbuf_node_t *activity =
        cbm_gbuf_find_by_qn(gb, "android:component:com.example.app:activity:com.example.app.MainActivity");
    ASSERT_NOT_NULL(activity);
    ASSERT_NOT_NULL(strstr(activity->properties_json, "\"exported\":\"true\""));
    ASSERT_NOT_NULL(strstr(activity->properties_json, "\"evidence_path\":\"AndroidManifest.xml\""));

    cbm_gbuf_free(gb);
    PASS();
}

TEST(android_manifest_missing_optional_attrs_is_ok) {
    const char *src = "<manifest package=\"com.example.min\"><application><activity android:name=\"Main\" /></application></manifest>";
    cbm_gbuf_t *gb = cbm_gbuf_new("android-min", "/tmp/android-min");
    ASSERT_NOT_NULL(gb);
    cbm_android_manifest_stats_t stats;
    ASSERT_EQ(cbm_android_manifest_emit(gb, "android-min", "nested/AndroidManifest.xml", src,
                                         (int)strlen(src), &stats),
              0);
    ASSERT_EQ(stats.apps, 1);
    ASSERT_EQ(stats.components, 1);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:component:com.example.min:activity:com.example.min.Main"));
    cbm_gbuf_free(gb);
    PASS();
}

SUITE(android_manifest) {
    RUN_TEST(android_manifest_emits_core_nodes_and_edges);
    RUN_TEST(android_manifest_missing_optional_attrs_is_ok);
}
