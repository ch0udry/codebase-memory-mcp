/*
 * test_android_pipeline.c — End-to-end Android indexing tests.
 */
#include "test_framework.h"
#include "test_helpers.h"

#include "cbm.h"
#include "foundation/compat_fs.h"
#include "mcp/mcp.h"
#include "pipeline/pipeline.h"
#include "store/store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char tmpdir[256];
    char dbpath[512];
    char *project;
    cbm_mcp_server_t *srv;
} AndroidE2EProj;

static void ae_to_fwd_slashes(char *p) {
    for (; p && *p; p++) {
        if (*p == '\\') {
            *p = '/';
        }
    }
}

static int ae_write_text(const char *root, const char *rel, const char *text) {
    char path[700];
    snprintf(path, sizeof(path), "%s/%s", root, rel);
    char *slash = strrchr(path, '/');
    if (slash && slash > path + strlen(root)) {
        *slash = '\0';
        cbm_mkdir_p(path, 0755);
        *slash = '/';
    }
    FILE *f = cbm_fopen(path, "wb");
    if (!f) {
        return -1;
    }
    fputs(text, f);
    fclose(f);
    return 0;
}

static int ae_count_label(cbm_store_t *store, const char *project, const char *label) {
    cbm_node_t *nodes = NULL;
    int count = 0;
    if (cbm_store_find_nodes_by_label(store, project, label, &nodes, &count) != CBM_STORE_OK) {
        return -1;
    }
    cbm_store_free_nodes(nodes, count);
    return count;
}

static cbm_store_t *ae_index(AndroidE2EProj *lp) {
    lp->project = cbm_project_name_from_path(lp->tmpdir);
    if (!lp->project) {
        return NULL;
    }
    const char *home = getenv("HOME");
    if (!home) {
        home = "/tmp";
    }
    char cache_dir[512];
    snprintf(cache_dir, sizeof(cache_dir), "%s/.cache/codebase-memory-mcp", home);
    cbm_mkdir(cache_dir);
    snprintf(lp->dbpath, sizeof(lp->dbpath), "%s/%s.db", cache_dir, lp->project);
    unlink(lp->dbpath);
    lp->srv = cbm_mcp_server_new(NULL);
    if (!lp->srv) {
        return NULL;
    }
    char args[700];
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\"}", lp->tmpdir);
    char *resp = cbm_mcp_handle_tool(lp->srv, "index_repository", args);
    if (!resp) {
        return NULL;
    }
    if (!strstr(resp, "indexed")) {
        free(resp);
        return NULL;
    }
    free(resp);
    return cbm_store_open_path(lp->dbpath);
}

static void ae_cleanup(AndroidE2EProj *lp, cbm_store_t *store) {
    if (store) {
        cbm_store_close(store);
    }
    if (lp->srv) {
        cbm_mcp_server_free(lp->srv);
        lp->srv = NULL;
    }
    free(lp->project);
    lp->project = NULL;
    th_rmtree(lp->tmpdir);
    unlink(lp->dbpath);
    char wal[600];
    char shm[600];
    snprintf(wal, sizeof(wal), "%s-wal", lp->dbpath);
    unlink(wal);
    snprintf(shm, sizeof(shm), "%s-shm", lp->dbpath);
    unlink(shm);
}

TEST(android_pipeline_indexes_extracted_folder_and_queries_android_facts) {
    AndroidE2EProj lp;
    memset(&lp, 0, sizeof(lp));
    snprintf(lp.tmpdir, sizeof(lp.tmpdir), "/tmp/cbm_android_e2e_XXXXXX");
    ASSERT_NOT_NULL(cbm_mkdtemp(lp.tmpdir));
    ae_to_fwd_slashes(lp.tmpdir);

    ASSERT_EQ(ae_write_text(lp.tmpdir, "AndroidManifest.xml",
                            "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\" "
                            "package=\"com.example.e2e\">\n"
                            "  <uses-permission android:name=\"android.permission.INTERNET\" />\n"
                            "  <application>\n"
                            "    <activity android:name=\".MainActivity\" android:exported=\"true\">\n"
                            "      <intent-filter>\n"
                            "        <action android:name=\"android.intent.action.VIEW\" />\n"
                            "        <category android:name=\"android.intent.category.BROWSABLE\" />\n"
                            "        <data android:scheme=\"https\" android:host=\"example.com\" "
                            "android:pathPrefix=\"/open\" />\n"
                            "      </intent-filter>\n"
                            "    </activity>\n"
                            "  </application>\n"
                            "</manifest>\n"),
              0);
    ASSERT_EQ(ae_write_text(lp.tmpdir, "res/values/strings.xml",
                            "<resources>\n"
                            "  <string name=\"app_name\">E2E</string>\n"
                            "  <string name=\"api_base\">https://api.example.com/v1</string>\n"
                            "</resources>\n"),
              0);
    ASSERT_EQ(ae_write_text(lp.tmpdir, "res/layout/activity_main.xml", "<LinearLayout />\n"), 0);
    ASSERT_EQ(ae_write_text(lp.tmpdir, "smali/com/example/e2e/MainActivity.smali",
                            ".class public Lcom/example/e2e/MainActivity;\n"
                            ".super Landroid/app/Activity;\n"
                            ".method public onCreate()V\n  return-void\n.end method\n"),
              0);

    cbm_store_t *store = ae_index(&lp);
    ASSERT_NOT_NULL(store);
    ASSERT_GTE(ae_count_label(store, lp.project, "AndroidApp"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "Manifest"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "Activity"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "Permission"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "DeepLink"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "ResourceString"), 2);
    ASSERT_GTE(ae_count_label(store, lp.project, "LayoutResource"), 1);
    ASSERT_GTE(ae_count_label(store, lp.project, "ApiEndpoint"), 1);
    ASSERT_GTE(cbm_store_count_edges_by_type(store, lp.project, "DECLARES_COMPONENT"), 1);
    ASSERT_GTE(cbm_store_count_edges_by_type(store, lp.project, "REQUESTS_PERMISSION"), 1);
    ASSERT_GTE(cbm_store_count_edges_by_type(store, lp.project, "EXPOSES_DEEPLINK"), 1);
    ASSERT_GTE(cbm_store_count_edges_by_type(store, lp.project, "HAS_RESOURCE"), 2);

    char args[1024];
    snprintf(args, sizeof(args),
             "{\"project\":\"%s\",\"query\":\"MATCH (a:Activity) RETURN a LIMIT 5\"}",
             lp.project);
    char *qresp = cbm_mcp_handle_tool(lp.srv, "query_graph", args);
    ASSERT_NOT_NULL(qresp);
    ASSERT_NOT_NULL(strstr(qresp, "MainActivity"));
    free(qresp);

    snprintf(args, sizeof(args),
             "{\"project\":\"%s\",\"label\":\"ResourceString\",\"name_pattern\":\"api_base\","
             "\"limit\":5}",
             lp.project);
    char *sresp = cbm_mcp_handle_tool(lp.srv, "search_graph", args);
    ASSERT_NOT_NULL(sresp);
    ASSERT_NOT_NULL(strstr(sresp, "api_base"));
    ASSERT_NOT_NULL(strstr(sresp, "https://api.example.com/v1"));
    free(sresp);

    snprintf(args, sizeof(args), "{\"project\":\"%s\"}", lp.project);
    char *schema = cbm_mcp_handle_tool(lp.srv, "get_graph_schema", args);
    ASSERT_NOT_NULL(schema);
    ASSERT_NOT_NULL(strstr(schema, "Activity"));
    ASSERT_NOT_NULL(strstr(schema, "ResourceString"));
    free(schema);

    ae_cleanup(&lp, store);
    PASS();
}

SUITE(android_pipeline) {
    RUN_TEST(android_pipeline_indexes_extracted_folder_and_queries_android_facts);
}
