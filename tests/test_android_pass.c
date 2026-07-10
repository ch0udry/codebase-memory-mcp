/*
 * test_android_pass.c — Android semantic pipeline pass tests.
 */
#include "test_framework.h"

#include "foundation/compat.h"
#include "foundation/compat_fs.h"
#include "graph_buffer/graph_buffer.h"
#include "pipeline/pass_android.h"

#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

static int write_text_file(const char *path, const char *text) {
    FILE *f = cbm_fopen(path, "wb");
    if (!f) {
        return -1;
    }
    fputs(text, f);
    fclose(f);
    return 0;
}

TEST(android_pass_noops_for_non_android_repo) {
    atomic_int cancelled;
    atomic_init(&cancelled, 0);
    cbm_gbuf_t *gb = cbm_gbuf_new("plain", "/tmp/plain");
    ASSERT_NOT_NULL(gb);
    cbm_pipeline_ctx_t ctx = {0};
    ctx.project_name = "plain";
    ctx.repo_path = "/tmp/plain";
    ctx.gbuf = gb;
    ctx.cancelled = &cancelled;

    cbm_file_info_t files[1] = {{0}};
    files[0].path = "/tmp/plain/main.go";
    files[0].rel_path = "main.go";
    ASSERT_EQ(cbm_pipeline_pass_android(&ctx, files, 1), 0);
    ASSERT_EQ(cbm_gbuf_node_count(gb), 0);
    ASSERT_EQ(cbm_gbuf_edge_count(gb), 0);
    cbm_gbuf_free(gb);
    PASS();
}

TEST(android_pass_processes_discovered_manifest) {
    char dir[256];
    snprintf(dir, sizeof(dir), "%s/cbm_android_pass_XXXXXX", cbm_tmpdir());
    ASSERT_NOT_NULL(cbm_mkdtemp(dir));

    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/AndroidManifest.xml", dir);
    const char *manifest =
        "<manifest package=\"com.example.pass\">\n"
        "  <uses-permission android:name=\"android.permission.INTERNET\" />\n"
        "  <application><activity android:name=\".MainActivity\" android:exported=\"true\" /></application>\n"
        "</manifest>\n";
    ASSERT_EQ(write_text_file(manifest_path, manifest), 0);

    atomic_int cancelled;
    atomic_init(&cancelled, 0);
    cbm_gbuf_t *gb = cbm_gbuf_new("android-pass", dir);
    ASSERT_NOT_NULL(gb);
    cbm_pipeline_ctx_t ctx = {0};
    ctx.project_name = "android-pass";
    ctx.repo_path = dir;
    ctx.gbuf = gb;
    ctx.cancelled = &cancelled;

    cbm_file_info_t files[1] = {{0}};
    files[0].path = manifest_path;
    files[0].rel_path = "AndroidManifest.xml";
    ASSERT_EQ(cbm_pipeline_pass_android(&ctx, files, 1), 0);
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb, "android:app:com.example.pass"));
    ASSERT_NOT_NULL(cbm_gbuf_find_by_qn(gb,
                                        "android:component:com.example.pass:activity:com.example.pass.MainActivity"));
    ASSERT_EQ(cbm_gbuf_edge_count_by_type(gb, "REQUESTS_PERMISSION"), 1);

    cbm_gbuf_free(gb);
    cbm_unlink(manifest_path);
    cbm_rmdir(dir);
    PASS();
}

SUITE(android_pass) {
    RUN_TEST(android_pass_noops_for_non_android_repo);
    RUN_TEST(android_pass_processes_discovered_manifest);
}
