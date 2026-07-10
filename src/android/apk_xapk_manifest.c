/*
 * apk_xapk_manifest.c — Minimal AndroidManifest.xml semantic extraction.
 */
#include "android/apk_xapk_manifest.h"

#include "android/apk_xapk_graph.h"
#include "foundation/constants.h"
#include "foundation/str_util.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *safe_s(const char *s) { return s ? s : ""; }

static bool tag_name_boundary(char c) {
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
        if (tag_name_boundary(p[SKIP_ONE + tag_len])) {
            return p;
        }
        p += SKIP_ONE;
    }
    return NULL;
}

static const char *tag_end(const char *start, const char *end) {
    const char *p = strchr(start, '>');
    return (p && p < end) ? p : NULL;
}

static bool is_self_closing_tag(const char *start, const char *open_end) {
    const char *p = open_end;
    while (p > start) {
        p--;
        if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            continue;
        }
        return *p == '/';
    }
    return false;
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

static bool android_name_attr(const char *start, const char *end, char *out, size_t out_sz) {
    return attr_copy(start, end, "android:name", out, out_sz) || attr_copy(start, end, "name", out, out_sz);
}

static int line_for_pos(const char *source, const char *pos) {
    int line = 1;
    for (const char *p = source; p && p < pos; p++) {
        if (*p == '\n') {
            line++;
        }
    }
    return line;
}

static void qualify_component(const char *package_name, const char *raw, char *out, size_t out_sz) {
    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!raw || raw[0] == '\0') {
        return;
    }
    if (raw[0] == '.') {
        snprintf(out, out_sz, "%s%s", safe_s(package_name), raw);
    } else if (strchr(raw, '.')) {
        snprintf(out, out_sz, "%s", raw);
    } else if (package_name && package_name[0]) {
        snprintf(out, out_sz, "%s.%s", package_name, raw);
    } else {
        snprintf(out, out_sz, "%s", raw);
    }
}

static const char *simple_name(const char *name) {
    const char *dot = name ? strrchr(name, '.') : NULL;
    return dot ? dot + SKIP_ONE : safe_s(name);
}

static void emit_action_or_category(cbm_gbuf_t *gbuf, const char *rel_path, int64_t filter_id,
                                    const char *tag_start, const char *tag_open_end,
                                    const char *label, const char *kind, const char *edge_type,
                                    int *count) {
    char name[CBM_SZ_512];
    if (!android_name_attr(tag_start, tag_open_end, name, sizeof(name)) || name[0] == '\0') {
        return;
    }
    char qn[CBM_SZ_1K];
    if (cbm_android_graph_qn(qn, sizeof(qn), kind, name, NULL, NULL) != 0) {
        return;
    }
    cbm_android_graph_prop_t extras[] = {{"name", name}};
    char props[CBM_SZ_2K];
    cbm_android_graph_props(props, sizeof(props), "manifest", "high", rel_path, kind, extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, label, simple_name(name), qn, rel_path, 0, 0, props);
    if (id > 0 && cbm_android_graph_insert_edge(gbuf, filter_id, id, edge_type, props) > 0 && count) {
        (*count)++;
    }
}

static void emit_deep_link(cbm_gbuf_t *gbuf, const char *rel_path, int64_t component_id,
                           int64_t filter_id, const char *tag_start, const char *tag_open_end,
                           cbm_android_manifest_stats_t *stats) {
    char scheme[CBM_SZ_128];
    char host[CBM_SZ_256];
    char path[CBM_SZ_512];
    char path_prefix[CBM_SZ_512];
    char path_pattern[CBM_SZ_512];
    scheme[0] = host[0] = path[0] = path_prefix[0] = path_pattern[0] = '\0';
    attr_copy(tag_start, tag_open_end, "android:scheme", scheme, sizeof(scheme));
    attr_copy(tag_start, tag_open_end, "android:host", host, sizeof(host));
    attr_copy(tag_start, tag_open_end, "android:path", path, sizeof(path));
    attr_copy(tag_start, tag_open_end, "android:pathPrefix", path_prefix, sizeof(path_prefix));
    attr_copy(tag_start, tag_open_end, "android:pathPattern", path_pattern, sizeof(path_pattern));
    if (scheme[0] == '\0' || host[0] == '\0') {
        return;
    }
    const char *path_part = path[0] ? path : (path_prefix[0] ? path_prefix : path_pattern);
    char uri[CBM_SZ_1K];
    snprintf(uri, sizeof(uri), "%s://%s%s", scheme, host, path_part ? path_part : "");
    char qn[CBM_SZ_2K];
    if (cbm_android_graph_qn(qn, sizeof(qn), "deeplink", uri, NULL, NULL) != 0) {
        return;
    }
    cbm_android_graph_prop_t extras[] = {{"scheme", scheme},
                                         {"host", host},
                                         {"path", path},
                                         {"path_prefix", path_prefix},
                                         {"path_pattern", path_pattern},
                                         {"uri", uri}};
    char props[CBM_SZ_4K];
    cbm_android_graph_props(props, sizeof(props), "manifest", "high", rel_path, "deeplink", extras,
                            (int)(sizeof(extras) / sizeof(extras[0])));
    int64_t id = cbm_android_graph_upsert_node(gbuf, "DeepLink", uri, qn, rel_path, 0, 0, props);
    if (id <= 0) {
        return;
    }
    cbm_android_graph_insert_edge(gbuf, component_id, id, "EXPOSES_DEEPLINK", props);
    cbm_android_graph_insert_edge(gbuf, filter_id, id, "HANDLES_DATA", props);
    if (stats) {
        stats->deep_links++;
    }
}

static void scan_intent_filters(cbm_gbuf_t *gbuf, const char *source, const char *block_start,
                                const char *block_end, const char *rel_path, const char *component_qn,
                                int64_t component_id, cbm_android_manifest_stats_t *stats) {
    int filter_index = 0;
    const char *cursor = block_start;
    while (cursor && cursor < block_end) {
        const char *start = find_start_tag(cursor, block_end, "intent-filter");
        if (!start) {
            break;
        }
        const char *open_end = tag_end(start, block_end);
        if (!open_end) {
            break;
        }
        const char *content_start = open_end + SKIP_ONE;
        const char *content_end = block_end;
        const char *close = strstr(content_start, "</intent-filter>");
        if (close && close < block_end) {
            content_end = close;
        }

        char idx[CBM_SZ_32];
        snprintf(idx, sizeof(idx), "%d", filter_index++);
        char qn[CBM_SZ_2K];
        if (cbm_android_graph_qn(qn, sizeof(qn), "intentfilter", component_qn, idx, NULL) != 0) {
            cursor = content_end + SKIP_ONE;
            continue;
        }
        cbm_android_graph_prop_t extras[] = {{"component_qn", component_qn}, {"index", idx}};
        char props[CBM_SZ_2K];
        cbm_android_graph_props(props, sizeof(props), "manifest", "high", rel_path, "intent_filter",
                                extras, (int)(sizeof(extras) / sizeof(extras[0])));
        int line = line_for_pos(source, start);
        int64_t filter_id = cbm_android_graph_upsert_node(gbuf, "IntentFilter", "intent-filter", qn,
                                                          rel_path, line, line, props);
        if (filter_id <= 0) {
            cursor = content_end + SKIP_ONE;
            continue;
        }
        cbm_android_graph_insert_edge(gbuf, component_id, filter_id, "HAS_INTENT_FILTER", props);
        if (stats) {
            stats->intent_filters++;
        }

        int ignored_count = 0;
        const char *child = content_start;
        while (child && child < content_end) {
            const char *a = find_start_tag(child, content_end, "action");
            if (!a) {
                break;
            }
            const char *ae = tag_end(a, content_end);
            if (!ae) {
                break;
            }
            emit_action_or_category(gbuf, rel_path, filter_id, a, ae, "IntentAction", "intentaction",
                                    "HANDLES_ACTION", &ignored_count);
            child = ae + SKIP_ONE;
        }
        child = content_start;
        while (child && child < content_end) {
            const char *c = find_start_tag(child, content_end, "category");
            if (!c) {
                break;
            }
            const char *ce = tag_end(c, content_end);
            if (!ce) {
                break;
            }
            emit_action_or_category(gbuf, rel_path, filter_id, c, ce, "IntentCategory", "intentcategory",
                                    "HANDLES_CATEGORY", &ignored_count);
            child = ce + SKIP_ONE;
        }
        child = content_start;
        while (child && child < content_end) {
            const char *d = find_start_tag(child, content_end, "data");
            if (!d) {
                break;
            }
            const char *de = tag_end(d, content_end);
            if (!de) {
                break;
            }
            emit_deep_link(gbuf, rel_path, component_id, filter_id, d, de, stats);
            child = de + SKIP_ONE;
        }
        cursor = close ? close + strlen("</intent-filter>") : open_end + SKIP_ONE;
    }
}

static void emit_permissions(cbm_gbuf_t *gbuf, const char *source, const char *end, const char *rel_path,
                             int64_t app_id, cbm_android_manifest_stats_t *stats) {
    const char *cursor = source;
    while (cursor && cursor < end) {
        const char *start = find_start_tag(cursor, end, "uses-permission");
        if (!start) {
            break;
        }
        const char *open_end = tag_end(start, end);
        if (!open_end) {
            break;
        }
        char perm[CBM_SZ_512];
        if (android_name_attr(start, open_end, perm, sizeof(perm)) && perm[0]) {
            char qn[CBM_SZ_1K];
            if (cbm_android_graph_qn(qn, sizeof(qn), "permission", perm, NULL, NULL) == 0) {
                cbm_android_graph_prop_t extras[] = {{"permission", perm}};
                char props[CBM_SZ_2K];
                cbm_android_graph_props(props, sizeof(props), "manifest", "high", rel_path, "permission",
                                        extras, (int)(sizeof(extras) / sizeof(extras[0])));
                int line = line_for_pos(source, start);
                int64_t pid = cbm_android_graph_upsert_node(gbuf, "Permission", perm, qn, rel_path, line,
                                                            line, props);
                if (pid > 0 && cbm_android_graph_insert_edge(gbuf, app_id, pid, "REQUESTS_PERMISSION", props) >
                                  0 &&
                    stats) {
                    stats->permissions++;
                }
            }
        }
        cursor = open_end + SKIP_ONE;
    }
}

static void emit_components(cbm_gbuf_t *gbuf, const char *source, const char *end, const char *rel_path,
                            const char *package_name, int64_t manifest_id,
                            cbm_android_manifest_stats_t *stats) {
    static const struct {
        const char *tag;
        const char *label;
        const char *kind;
    } components[] = {{"activity-alias", "Activity", "activity_alias"},
                      {"activity", "Activity", "activity"},
                      {"service", "Service", "service"},
                      {"receiver", "BroadcastReceiver", "receiver"},
                      {"provider", "ContentProvider", "provider"}};

    for (size_t ci = 0; ci < sizeof(components) / sizeof(components[0]); ci++) {
        const char *cursor = source;
        while (cursor && cursor < end) {
            const char *start = find_start_tag(cursor, end, components[ci].tag);
            if (!start) {
                break;
            }
            const char *open_end = tag_end(start, end);
            if (!open_end) {
                break;
            }
            char raw_name[CBM_SZ_512];
            if (!android_name_attr(start, open_end, raw_name, sizeof(raw_name)) || raw_name[0] == '\0') {
                cursor = open_end + SKIP_ONE;
                continue;
            }
            char fqcn[CBM_SZ_1K];
            qualify_component(package_name, raw_name, fqcn, sizeof(fqcn));
            char qn[CBM_SZ_2K];
            if (cbm_android_graph_qn(qn, sizeof(qn), "component", package_name, components[ci].kind, fqcn) !=
                0) {
                cursor = open_end + SKIP_ONE;
                continue;
            }
            char exported[CBM_SZ_64];
            char enabled[CBM_SZ_64];
            char permission[CBM_SZ_512];
            char authorities[CBM_SZ_512];
            exported[0] = enabled[0] = permission[0] = authorities[0] = '\0';
            attr_copy(start, open_end, "android:exported", exported, sizeof(exported));
            attr_copy(start, open_end, "android:enabled", enabled, sizeof(enabled));
            attr_copy(start, open_end, "android:permission", permission, sizeof(permission));
            attr_copy(start, open_end, "android:authorities", authorities, sizeof(authorities));
            cbm_android_graph_prop_t extras[] = {{"tag", components[ci].tag},
                                                 {"class_name", fqcn},
                                                 {"raw_name", raw_name},
                                                 {"exported", exported},
                                                 {"enabled", enabled},
                                                 {"permission", permission},
                                                 {"authorities", authorities}};
            char props[CBM_SZ_4K];
            cbm_android_graph_props(props, sizeof(props), "manifest", "high", rel_path, components[ci].kind,
                                    extras, (int)(sizeof(extras) / sizeof(extras[0])));
            int line = line_for_pos(source, start);
            int64_t component_id = cbm_android_graph_upsert_node(gbuf, components[ci].label, simple_name(fqcn),
                                                                 qn, rel_path, line, line, props);
            if (component_id > 0) {
                cbm_android_graph_insert_edge(gbuf, manifest_id, component_id, "DECLARES_COMPONENT", props);
                if (permission[0]) {
                    char pqn[CBM_SZ_1K];
                    if (cbm_android_graph_qn(pqn, sizeof(pqn), "permission", permission, NULL, NULL) == 0) {
                        int64_t pid = cbm_android_graph_upsert_node(gbuf, "Permission", permission, pqn,
                                                                    rel_path, line, line, props);
                        cbm_android_graph_insert_edge(gbuf, component_id, pid, "PROTECTED_BY_PERMISSION",
                                                      props);
                    }
                }
                if (stats) {
                    stats->components++;
                }
                const char *block_start = open_end + SKIP_ONE;
                const char *block_end = block_start;
                if (!is_self_closing_tag(start, open_end)) {
                    char close_tag[CBM_SZ_64];
                    snprintf(close_tag, sizeof(close_tag), "</%s>", components[ci].tag);
                    const char *close = strstr(block_start, close_tag);
                    block_end = (close && close < end) ? close : open_end;
                }
                scan_intent_filters(gbuf, source, block_start, block_end, rel_path, qn, component_id, stats);
            }
            cursor = open_end + SKIP_ONE;
        }
    }
}

int cbm_android_manifest_emit(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                              const char *source, int source_len,
                              cbm_android_manifest_stats_t *stats) {
    if (stats) {
        memset(stats, 0, sizeof(*stats));
    }
    if (!gbuf || !source || source_len <= 0 || !rel_path) {
        return 0;
    }
    const char *end = source + source_len;
    const char *manifest = find_start_tag(source, end, "manifest");
    if (!manifest) {
        return 0;
    }
    const char *manifest_open_end = tag_end(manifest, end);
    if (!manifest_open_end) {
        return 0;
    }

    char package_name[CBM_SZ_512];
    if (!attr_copy(manifest, manifest_open_end, "package", package_name, sizeof(package_name)) ||
        package_name[0] == '\0') {
        snprintf(package_name, sizeof(package_name), "%s", safe_s(project_name));
    }

    cbm_android_graph_prop_t app_extra[] = {{"package", package_name}, {"project", safe_s(project_name)}};
    char app_props[CBM_SZ_2K];
    cbm_android_graph_props(app_props, sizeof(app_props), "manifest", "high", rel_path, "app", app_extra,
                            (int)(sizeof(app_extra) / sizeof(app_extra[0])));
    char app_qn[CBM_SZ_1K];
    if (cbm_android_graph_qn(app_qn, sizeof(app_qn), "app", package_name, NULL, NULL) != 0) {
        return 0;
    }
    int64_t app_id = cbm_android_graph_upsert_node(gbuf, "AndroidApp", package_name, app_qn, rel_path,
                                                   line_for_pos(source, manifest), line_for_pos(source, manifest),
                                                   app_props);
    if (app_id <= 0) {
        return -1;
    }
    if (stats) {
        stats->apps++;
    }

    cbm_android_graph_prop_t manifest_extra[] = {{"package", package_name}};
    char manifest_props[CBM_SZ_2K];
    cbm_android_graph_props(manifest_props, sizeof(manifest_props), "manifest", "high", rel_path, "manifest",
                            manifest_extra, (int)(sizeof(manifest_extra) / sizeof(manifest_extra[0])));
    char manifest_qn[CBM_SZ_1K];
    if (cbm_android_graph_qn(manifest_qn, sizeof(manifest_qn), "manifest", safe_s(project_name), rel_path,
                             NULL) != 0) {
        return -1;
    }
    int64_t manifest_id = cbm_android_graph_upsert_node(gbuf, "Manifest", "AndroidManifest.xml", manifest_qn,
                                                        rel_path, line_for_pos(source, manifest),
                                                        line_for_pos(source, manifest), manifest_props);
    if (manifest_id <= 0) {
        return -1;
    }
    cbm_android_graph_insert_edge(gbuf, app_id, manifest_id, "HAS_MANIFEST", manifest_props);
    if (stats) {
        stats->manifests++;
    }

    emit_permissions(gbuf, source, end, rel_path, app_id, stats);
    emit_components(gbuf, source, end, rel_path, package_name, manifest_id, stats);
    return 0;
}
