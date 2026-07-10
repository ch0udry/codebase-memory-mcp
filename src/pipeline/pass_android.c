/*
 * pass_android.c — Android APK/XAPK semantic graph pass.
 *
 * This pass scans the normal discovered file list for already-extracted Android
 * artifacts and emits Android graph facts through the existing graph buffer.
 */
#include "pipeline/pass_android.h"

#include "android/apk_xapk_manifest.h"
#include "android/apk_xapk_resources.h"
#include "foundation/compat.h"
#include "foundation/compat_fs.h"
#include "foundation/constants.h"
#include "foundation/limits.h"
#include "foundation/log.h"
#include "foundation/str_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *android_itoa(int val) {
    enum { RING_BUF_COUNT = 16, RING_BUF_MASK = 15 };
    static CBM_TLS char bufs[RING_BUF_COUNT][CBM_SZ_32];
    static CBM_TLS int idx = 0;
    int i = idx;
    idx = (idx + SKIP_ONE) & RING_BUF_MASK;
    snprintf(bufs[i], sizeof(bufs[i]), "%d", val);
    return bufs[i];
}

static const char *android_basename(const char *path) {
    const char *p = path ? strrchr(path, '/') : NULL;
    return p ? p + SKIP_ONE : path;
}

static int android_has_suffix(const char *s, const char *suffix) {
    if (!s || !suffix) {
        return 0;
    }
    size_t sl = strlen(s);
    size_t tl = strlen(suffix);
    return sl >= tl && strcmp(s + sl - tl, suffix) == 0;
}

static int android_path_has_part(const char *path, const char *part) {
    return path && part && strstr(path, part) != NULL;
}

static int android_starts_with(const char *s, const char *prefix) {
    return s && prefix && strncmp(s, prefix, strlen(prefix)) == 0;
}

static int android_is_manifest_path(const char *rel_path) {
    const char *base = android_basename(rel_path);
    return base && strcmp(base, "AndroidManifest.xml") == 0;
}

static int android_is_resource_or_asset_path(const char *rel_path) {
    return android_starts_with(rel_path, "res/") || android_path_has_part(rel_path, "/res/") ||
           android_starts_with(rel_path, "assets/") || android_path_has_part(rel_path, "/assets/") ||
           android_has_suffix(rel_path, ".so");
}

static int android_resource_needs_source(const char *rel_path) {
    const char *base = android_basename(rel_path);
    return base && strcmp(base, "strings.xml") == 0 &&
           (android_starts_with(rel_path, "res/values/") || android_path_has_part(rel_path, "/res/values/"));
}

static char *android_read_file(const char *path, int *out_len) {
    if (out_len) {
        *out_len = 0;
    }
    FILE *f = cbm_fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    (void)fseek(f, 0, SEEK_END);
    long size = ftell(f);
    (void)fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > cbm_max_file_bytes()) {
        (void)fclose(f);
        return NULL;
    }
    char *buf = malloc((size_t)size + SKIP_ONE);
    if (!buf) {
        (void)fclose(f);
        return NULL;
    }
    size_t nread = fread(buf, SKIP_ONE, (size_t)size, f);
    (void)fclose(f);
    if (nread > (size_t)size) {
        nread = (size_t)size;
    }
    buf[nread] = '\0';
    if (out_len) {
        *out_len = (int)nread;
    }
    return buf;
}

int cbm_pipeline_pass_android(cbm_pipeline_ctx_t *ctx, const cbm_file_info_t *files, int file_count) {
    if (!ctx || !ctx->gbuf || !files || file_count <= 0) {
        return 0;
    }

    int manifests = 0;
    int apps = 0;
    int components = 0;
    int permissions = 0;
    int intent_filters = 0;
    int deep_links = 0;
    int resource_files = 0;
    int string_resources = 0;
    int layout_resources = 0;
    int assets = 0;
    int native_libraries = 0;
    int api_endpoints = 0;

    for (int i = 0; i < file_count; i++) {
        if (cbm_pipeline_check_cancel(ctx) != 0) {
            return CBM_NOT_FOUND;
        }
        const char *rel_path = files[i].rel_path;
        if (!android_is_manifest_path(rel_path) && !android_is_resource_or_asset_path(rel_path)) {
            continue;
        }
        int source_len = 0;
        char *source = NULL;
        if (android_is_manifest_path(rel_path) || android_resource_needs_source(rel_path)) {
            source = android_read_file(files[i].path, &source_len);
            if (!source) {
                continue;
            }
        }

        if (android_is_manifest_path(rel_path)) {
            cbm_android_manifest_stats_t stats;
            int rc = cbm_android_manifest_emit(ctx->gbuf, ctx->project_name, rel_path, source, source_len, &stats);
            if (rc != 0) {
                cbm_log_error("pass.android.err", "file", rel_path, "phase", "manifest");
                free(source);
                continue;
            }
            manifests += stats.manifests;
            apps += stats.apps;
            components += stats.components;
            permissions += stats.permissions;
            intent_filters += stats.intent_filters;
            deep_links += stats.deep_links;
        }

        if (android_is_resource_or_asset_path(rel_path)) {
            cbm_android_resource_stats_t rstats;
            int rc = cbm_android_resources_emit_file(ctx->gbuf, ctx->project_name, rel_path, source, source_len,
                                                     &rstats);
            if (rc != 0) {
                cbm_log_error("pass.android.err", "file", rel_path, "phase", "resources");
                free(source);
                continue;
            }
            resource_files += rstats.resource_files;
            string_resources += rstats.string_resources;
            layout_resources += rstats.layout_resources + rstats.menu_resources + rstats.navigation_resources +
                                rstats.drawable_resources + rstats.xml_configs + rstats.raw_resources;
            assets += rstats.assets;
            native_libraries += rstats.native_libraries;
            api_endpoints += rstats.api_endpoints;
        }
        free(source);
    }

    cbm_log_info("pass.android.done", "manifests", android_itoa(manifests), "apps", android_itoa(apps),
                 "components", android_itoa(components), "permissions", android_itoa(permissions),
                 "intent_filters", android_itoa(intent_filters), "deep_links", android_itoa(deep_links),
                 "resource_files", android_itoa(resource_files), "string_resources", android_itoa(string_resources),
                 "layout_resources", android_itoa(layout_resources), "assets", android_itoa(assets),
                 "native_libraries", android_itoa(native_libraries), "api_endpoints", android_itoa(api_endpoints));
    return 0;
}
