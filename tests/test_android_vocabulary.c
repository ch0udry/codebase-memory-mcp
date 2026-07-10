/*
 * test_android_vocabulary.c — Android APK/XAPK vocabulary registry tests.
 */
#include "test_framework.h"
#include "android/apk_xapk_vocabulary.h"

#include <stddef.h>

TEST(android_vocab_lists_are_nonempty) {
    size_t count = 0;

    ASSERT_NOT_NULL(cbm_android_vocab_node_labels(&count));
    ASSERT_GT(count, 0);

    ASSERT_NOT_NULL(cbm_android_vocab_edge_types(&count));
    ASSERT_GT(count, 0);

    ASSERT_NOT_NULL(cbm_android_vocab_properties(&count));
    ASSERT_GT(count, 0);

    ASSERT_NOT_NULL(cbm_android_vocab_categories(&count));
    ASSERT_GT(count, 0);

    ASSERT_NOT_NULL(cbm_android_vocab_source_formats(&count));
    ASSERT_GT(count, 0);

    ASSERT_NOT_NULL(cbm_android_vocab_confidence_values(&count));
    ASSERT_GT(count, 0);

    PASS();
}

TEST(android_vocab_recognizes_core_node_labels) {
    ASSERT_TRUE(cbm_android_vocab_is_node_label("AndroidApp"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("Manifest"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("Activity"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("Service"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("BroadcastReceiver"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("ContentProvider"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("DeepLink"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("ApiEndpoint"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("StorageKey"));
    ASSERT_TRUE(cbm_android_vocab_is_node_label("BusinessFlow"));
    ASSERT_FALSE(cbm_android_vocab_is_node_label("NotAndroidNode"));

    PASS();
}

TEST(android_vocab_recognizes_core_edge_types) {
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("DECLARES_COMPONENT"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("REQUESTS_PERMISSION"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("EXPOSES_DEEPLINK"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("CALLS_ENDPOINT"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("READS_PREFERENCE"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("WRITES_PREFERENCE"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("MAPS_TO_SMALI"));
    ASSERT_TRUE(cbm_android_vocab_is_edge_type("INDICATES_BUSINESS_FLOW"));
    ASSERT_FALSE(cbm_android_vocab_is_edge_type("NOT_ANDROID_EDGE"));

    PASS();
}

TEST(android_vocab_recognizes_properties_categories_sources_and_confidence) {
    ASSERT_TRUE(cbm_android_vocab_is_property("android_kind"));
    ASSERT_TRUE(cbm_android_vocab_is_property("source_file"));
    ASSERT_TRUE(cbm_android_vocab_is_property("source_format"));
    ASSERT_TRUE(cbm_android_vocab_is_property("confidence"));
    ASSERT_TRUE(cbm_android_vocab_is_property("value_hash"));
    ASSERT_FALSE(cbm_android_vocab_is_property("not_android_property"));

    ASSERT_TRUE(cbm_android_vocab_is_category("order_tracking"));
    ASSERT_TRUE(cbm_android_vocab_is_category("rider_assignment"));
    ASSERT_TRUE(cbm_android_vocab_is_category("device_integrity"));
    ASSERT_FALSE(cbm_android_vocab_is_category("not_android_category"));

    ASSERT_TRUE(cbm_android_vocab_is_source_format("manifest"));
    ASSERT_TRUE(cbm_android_vocab_is_source_format("xml_resource"));
    ASSERT_TRUE(cbm_android_vocab_is_source_format("jadx_java"));
    ASSERT_TRUE(cbm_android_vocab_is_source_format("smali"));
    ASSERT_FALSE(cbm_android_vocab_is_source_format("not_android_source"));

    ASSERT_TRUE(cbm_android_vocab_is_confidence("high"));
    ASSERT_TRUE(cbm_android_vocab_is_confidence("medium"));
    ASSERT_TRUE(cbm_android_vocab_is_confidence("low"));
    ASSERT_FALSE(cbm_android_vocab_is_confidence("certain"));

    PASS();
}

SUITE(android_vocabulary) {
    RUN_TEST(android_vocab_lists_are_nonempty);
    RUN_TEST(android_vocab_recognizes_core_node_labels);
    RUN_TEST(android_vocab_recognizes_core_edge_types);
    RUN_TEST(android_vocab_recognizes_properties_categories_sources_and_confidence);
}
