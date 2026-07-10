/*
 * apk_xapk_resources.c — Minimal Android resource/asset graph extraction.
 */
#include "android/apk_xapk_resources.h"

#include "android/apk_xapk_graph.h"
#include "foundation/constants.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *safe_s(const char *s) { return s ? s : ""; }

static const char *path_basename(const char *path) {
    const char *p = path ? strrchr(path, '/') : NULL;
    return p ? p + SKIP_ONE : path;
}

static bool has_suffix(const char *s, const char *suffix) {
    if (!s || !suffix) {
        return false;
    }
    size_t sl = strlen(s);
    size_t tl = strlen(suffix);
    return sl >= tl && strcmp(s + sl - tl, suffix) == 0;
}

static bool contains_path_part(const char *path, const char *part) {
    return path && part && strstr(path, part) != NULL;
}

static void strip_extension(const char *base, char *out, size_t out_sz) {
    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!base) {
        return;
    }
    size_t n = strlen(base);
    const char *dot = strrchr(base, '.');
    if (dot && dot > base) {
        n = (size_t)(dot - base);
    }
    if (n >= out_sz) {
        n = out_sz - SKIP_ONE;
    }
    memcpy(out, base, n);
    out[n] = '\0';
}

static bool tag_boundary(char c) {
    return c == '\0' || c == '>' || c == '/' || c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static const char *find_start_tag(const char *cursor, const char *end, const char *tag) {
    char needle[CBM_SZ_64];
    snprintf(needle, sizeof(needle), "<%s", tag);
    size_t tag_len = strlen(tag);
    const char *p = cursor;
    while (p && p < end) {
        p = strstr(p, needle);
        if (!p || p >= end) {
            return NULL;
        }
        if (tag_boundary(p[SKIP_ONE + tag_len])) {
            return p;
        }
        p += SKIP_ONE;
    }
    return NULL;
}

static const char *tag_end(const char *start, const char *end) {
    const char *p = start ? strchr(start, '>') : NULL;
    return (p && p < end) ? p : NULL;
}

static bool attr_copy(const char *start, const char *end, const char *attr, char *out, size_t out_sz) {
    if (!start || !end || !attr || !out || out_sz == 0) {
        return false;
    }
    out[0] = '\0';
    const char *p = start;
    size_t attr_len = strlen(attr);
    while (p && p < end) {
        p = strstr(p, attr);
        if (!p || p >= end) {
            return false;
        }
        if (p > start) {
            char prev = p[-1];
            if (prev != ' ' && prev != '\t' && prev != '\r' && prev != '\n' && prev != '<') {
                p += SKIP_ONE;
                continue;
            }
        }
        const char *q = p + attr_len;
        while (q < end && (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n')) {
            q++;
        }
        if (q >= end || *q != '=') {
            p += SKIP_ONE;
            continue;
        }
        q++;
        while (q < end && (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n')) {
            q++;
        }
        if (q >= end || (*q != '"' && *q != '\'')) {
            p += SKIP_ONE;
            continue;
        }
        char quote = *q++;
        const char *v = q;
        while (q < end && *q != quote) {
            q++;
        }
        if (q >= end) {
            return false;
        }
        size_t n = (size_t)(q - v);
        if (n >= out_sz) {
            n = out_sz - SKIP_ONE;
        }
        memcpy(out, v, n);
        out[n] = '\0';
        return true;
    }
    return false;
}

static void text_between(const char *start, const char *end, char *out, size_t out_sz) {
    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!start || !end || end <= start) {
        return;
    }
    while (start < end && (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')) {
        start++;
    }
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        end--;
    }
    size_t n = (size_t)(end - start);
    if (n >= out_sz) {
        n = out_sz - SKIP_ONE;
    }
    memcpy(out, start, n);
    out[n] = '\0';
}

static bool looks_like_url(const char *s) {
    return s && (strstr(s, "http://") || strstr(s, "https://") || strstr(s, "wss://"));
}

static void first_url(const char *s, char *out, size_t out_sz) {
    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!s) {
        return;
    }
    const char *p = strstr(s, "https://");
    if (!p) {
        p = strstr(s, "http://");
    }
    if (!p) {
        p = strstr(s, "wss://");
    }
    if (!p) {
        return;
    }
    const char *e = p;
    while (*e && *e != ' ' && *e != '\t' && *e != '\r' && *e != '\n' && *e != '"' && *e != '\'' &&
           *e != '<') {
        e++;
    }
    size_t n = (size_t)(e - p);
    if (n >= out_sz) {
        n = out_sz - SKIP_ONE;
    }
    memcpy(out, p, n);
    out[n] = '\0';
}

static int64_t ensure_app(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path) {
    char qn[CBM_SZ_1K];
    const char *app_name = (project_name && project_name[0]) ? project_name : "android";
    if (cbm_android_graph_qn(qn, sizeof(qn), "app", app_name, NULL, NULL) != 0) {
        return 0;
    }
    cbm_android_graph_prop_t extras[] = {{"project", app_name}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), "resource", "medium", rel_path, "app", extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    return cbm_android_graph_upsert_node(gbuf, "AndroidApp", app_name, qn, rel_path, 0, 0, props);
}

static int64_t emit_resource_file(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                                  int64_t app_id, cbm_android_resource_stats_t *stats) {
    char qn[CBM_SZ_2K];
    if (cbm_android_graph_qn(qn, sizeof(qn), "resourcefile", safe_s(project_name), rel_path, NULL) != 0) {
        return 0;
    }
    cbm_android_graph_prop_t extras[] = {{"path", rel_path}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), "resource", "high", rel_path, "resource_file", extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, "ResourceFile", path_basename(rel_path), qn, rel_path, 0,
                                               0, props);
    if (id > 0) {
        cbm_android_graph_insert_edge(gbuf, app_id, id, "HAS_RESOURCE", props);
        if (stats) {
            stats->resource_files++;
        }
    }
    return id;
}

static int64_t emit_path_resource(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                                  const char *type, const char *label, const char *kind, int64_t app_id,
                                  cbm_android_resource_stats_t *stats) {
    char name[CBM_SZ_512];
    strip_extension(path_basename(rel_path), name, sizeof(name));
    if (name[0] == '\0') {
        return 0;
    }
    char qn[CBM_SZ_2K];
    if (cbm_android_graph_qn(qn, sizeof(qn), "resource", type, name, NULL) != 0) {
        return 0;
    }
    cbm_android_graph_prop_t extras[] = {{"resource_type", type}, {"resource_name", name}, {"path", rel_path}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), "resource", "high", rel_path, kind, extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, label, name, qn, rel_path, 0, 0, props);
    if (id <= 0) {
        return 0;
    }
    cbm_android_graph_insert_edge(gbuf, app_id, id, "HAS_RESOURCE", props);
    int64_t file_id = emit_resource_file(gbuf, project_name, rel_path, app_id, stats);
    if (file_id > 0) {
        cbm_android_graph_insert_edge(gbuf, file_id, id, "CONTAINS_RESOURCE", props);
    }
    if (stats) {
        if (strcmp(label, "LayoutResource") == 0) {
            stats->layout_resources++;
        } else if (strcmp(label, "MenuResource") == 0) {
            stats->menu_resources++;
        } else if (strcmp(label, "NavigationResource") == 0) {
            stats->navigation_resources++;
        } else if (strcmp(label, "DrawableResource") == 0) {
            stats->drawable_resources++;
        } else if (strcmp(label, "XmlConfig") == 0) {
            stats->xml_configs++;
        } else if (strcmp(label, "RawResource") == 0) {
            stats->raw_resources++;
        }
    }
    return id;
}

static void emit_api_endpoint(cbm_gbuf_t *gbuf, const char *rel_path, const char *url, int64_t owner_id,
                              cbm_android_resource_stats_t *stats) {
    if (!url || url[0] == '\0') {
        return;
    }
    char qn[CBM_SZ_2K];
    if (cbm_android_graph_qn(qn, sizeof(qn), "endpoint", url, NULL, NULL) != 0) {
        return;
    }
    cbm_android_graph_prop_t extras[] = {{"url", url}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), "resource", "medium", rel_path, "api_endpoint", extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, "ApiEndpoint", url, qn, rel_path, 0, 0, props);
    if (id > 0) {
        cbm_android_graph_insert_edge(gbuf, owner_id, id, "CONTAINS", props);
        if (stats) {
            stats->api_endpoints++;
        }
    }
}

static void emit_string_resources(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                                  const char *source, int source_len, int64_t app_id,
                                  cbm_android_resource_stats_t *stats) {
    if (!source || source_len <= 0) {
        return;
    }
    const char *end = source + source_len;
    int64_t file_id = emit_resource_file(gbuf, project_name, rel_path, app_id, stats);
    const char *cursor = source;
    while (cursor && cursor < end) {
        const char *start = find_start_tag(cursor, end, "string");
        if (!start) {
            break;
        }
        const char *open_end = tag_end(start, end);
        if (!open_end) {
            break;
        }
        char name[CBM_SZ_512];
        if (!attr_copy(start, open_end, "name", name, sizeof(name)) || name[0] == '\0') {
            cursor = open_end + SKIP_ONE;
            continue;
        }
        const char *close = strstr(open_end + SKIP_ONE, "</string>");
        if (!close || close > end) {
            cursor = open_end + SKIP_ONE;
            continue;
        }
        char value[CBM_SZ_1K];
        text_between(open_end + SKIP_ONE, close, value, sizeof(value));
        char qn[CBM_SZ_2K];
        if (cbm_android_graph_qn(qn, sizeof(qn), "resource", "string", name, NULL) == 0) {
            cbm_android_graph_prop_t extras[] = {{"resource_type", "string"},
                                                 {"resource_name", name},
                                                 {"value", value}};
            char props[CBM_SZ_4K];
            cbm_android_graph_props(props, sizeof(props), "resource", "high", rel_path, "string", extras,
                                    (int)(sizeof(extras) / sizeof(extras[0])));
            int64_t id = cbm_android_graph_upsert_node(gbuf, "ResourceString", name, qn, rel_path, 0, 0,
                                                       props);
            if (id > 0) {
                cbm_android_graph_insert_edge(gbuf, app_id, id, "HAS_RESOURCE", props);
                if (file_id > 0) {
                    cbm_android_graph_insert_edge(gbuf, file_id, id, "CONTAINS_RESOURCE", props);
                }
                if (stats) {
                    stats->string_resources++;
                }
                if (looks_like_url(value)) {
                    char url[CBM_SZ_1K];
                    first_url(value, url, sizeof(url));
                    emit_api_endpoint(gbuf, rel_path, url, id, stats);
                }
            }
        }
        cursor = close + strlen("</string>");
    }
}

static void emit_asset(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path, int64_t app_id,
                       bool native_library, cbm_android_resource_stats_t *stats) {
    char qn[CBM_SZ_2K];
    const char *kind = native_library ? "native" : "asset";
    if (cbm_android_graph_qn(qn, sizeof(qn), kind, safe_s(project_name), rel_path, NULL) != 0) {
        return;
    }
    cbm_android_graph_prop_t extras[] = {{"path", rel_path}, {"native_library", native_library ? "true" : "false"}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), native_library ? "native_library" : "asset", "high", rel_path,
                            kind, extras, (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, "Asset", path_basename(rel_path), qn, rel_path, 0, 0,
                                               props);
    if (id > 0) {
        cbm_android_graph_insert_edge(gbuf, app_id, id, "CONTAINS", props);
        if (stats) {
            if (native_library) {
                stats->native_libraries++;
            } else {
                stats->assets++;
            }
        }
    }
}

int cbm_android_resources_emit_file(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                                    const char *source, int source_len,
                                    cbm_android_resource_stats_t *stats) {
    if (stats) {
        memset(stats, 0, sizeof(*stats));
    }
    if (!gbuf || !rel_path) {
        return 0;
    }
    int64_t app_id = ensure_app(gbuf, project_name, rel_path);
    if (app_id <= 0) {
        return -1;
    }

    if (has_suffix(rel_path, ".so")) {
        emit_asset(gbuf, project_name, rel_path, app_id, true, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/assets/") || strncmp(rel_path, "assets/", strlen("assets/")) == 0) {
        emit_asset(gbuf, project_name, rel_path, app_id, false, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/values/") || strncmp(rel_path, "res/values/", strlen("res/values/")) == 0) {
        if (strcmp(path_basename(rel_path), "strings.xml") == 0) {
            emit_string_resources(gbuf, project_name, rel_path, source, source_len, app_id, stats);
            return 0;
        }
        emit_resource_file(gbuf, project_name, rel_path, app_id, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/layout/") || strncmp(rel_path, "res/layout/", strlen("res/layout/")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "layout", "LayoutResource", "layout", app_id, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/menu/") || strncmp(rel_path, "res/menu/", strlen("res/menu/")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "menu", "MenuResource", "menu", app_id, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/navigation/") ||
        strncmp(rel_path, "res/navigation/", strlen("res/navigation/")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "navigation", "NavigationResource", "navigation", app_id,
                           stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/drawable") || strncmp(rel_path, "res/drawable", strlen("res/drawable")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "drawable", "DrawableResource", "drawable", app_id,
                           stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/xml/") || strncmp(rel_path, "res/xml/", strlen("res/xml/")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "xml", "XmlConfig", "xml_config", app_id, stats);
        return 0;
    }
    if (contains_path_part(rel_path, "/res/raw/") || strncmp(rel_path, "res/raw/", strlen("res/raw/")) == 0) {
        emit_path_resource(gbuf, project_name, rel_path, "raw", "RawResource", "raw", app_id, stats);
        return 0;
    }
    return 0;
}
