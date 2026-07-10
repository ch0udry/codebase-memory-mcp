/*
 * apk_xapk_graph.c — Android graph insertion helpers.
 */
#include "android/apk_xapk_graph.h"

#include "android/apk_xapk_vocabulary.h"
#include "foundation/str_util.h"

#include <stdio.h>
#include <string.h>

static const char *android_safe(const char *s) { return s ? s : ""; }

int cbm_android_graph_qn(char *buf, size_t buf_size, const char *kind, const char *a,
                         const char *b, const char *c) {
    if (!buf || buf_size == 0 || !kind || kind[0] == '\0') {
        return -1;
    }
    int n = 0;
    if (c && c[0]) {
        n = snprintf(buf, buf_size, "android:%s:%s:%s:%s", kind, android_safe(a), android_safe(b), c);
    } else if (b && b[0]) {
        n = snprintf(buf, buf_size, "android:%s:%s:%s", kind, android_safe(a), b);
    } else if (a && a[0]) {
        n = snprintf(buf, buf_size, "android:%s:%s", kind, a);
    } else {
        n = snprintf(buf, buf_size, "android:%s", kind);
    }
    return (n > 0 && (size_t)n < buf_size) ? 0 : -1;
}

int cbm_android_graph_props(char *buf, int buf_size, const char *source_format,
                            const char *confidence, const char *evidence_path,
                            const char *android_kind, const cbm_android_graph_prop_t *extra,
                            int extra_count) {
    if (!buf || buf_size <= 0) {
        return -1;
    }

    char source_esc[512];
    char conf_esc[128];
    char evidence_esc[1024];
    char kind_esc[256];
    cbm_json_escape(source_esc, (int)sizeof(source_esc), android_safe(source_format));
    cbm_json_escape(conf_esc, (int)sizeof(conf_esc), android_safe(confidence));
    cbm_json_escape(evidence_esc, (int)sizeof(evidence_esc), android_safe(evidence_path));
    cbm_json_escape(kind_esc, (int)sizeof(kind_esc), android_safe(android_kind));

    int off = snprintf(buf, (size_t)buf_size,
                       "{\"source_format\":\"%s\",\"confidence\":\"%s\","
                       "\"evidence_path\":\"%s\",\"source_file\":\"%s\","
                       "\"android_kind\":\"%s\"",
                       source_esc, conf_esc, evidence_esc, evidence_esc, kind_esc);
    if (off < 0 || off >= buf_size) {
        buf[buf_size - 1] = '\0';
        return -1;
    }

    for (int i = 0; extra && i < extra_count; i++) {
        if (!extra[i].key || !extra[i].value) {
            continue;
        }
        char key_esc[256];
        char val_esc[1024];
        cbm_json_escape(key_esc, (int)sizeof(key_esc), extra[i].key);
        cbm_json_escape(val_esc, (int)sizeof(val_esc), extra[i].value);
        CBM_SNPRINTF_APPEND(buf, buf_size, off, ",\"%s\":\"%s\"", key_esc, val_esc);
    }
    CBM_SNPRINTF_APPEND(buf, buf_size, off, "}");
    return (off < buf_size - 1) ? 0 : -1;
}

int64_t cbm_android_graph_upsert_node(cbm_gbuf_t *gbuf, const char *label, const char *name,
                                      const char *qualified_name, const char *file_path,
                                      int start_line, int end_line, const char *properties_json) {
    if (!gbuf || !label || !name || !qualified_name || !cbm_android_vocab_is_node_label(label)) {
        return 0;
    }
    return cbm_gbuf_upsert_node(gbuf, label, name, qualified_name, file_path, start_line, end_line,
                                properties_json ? properties_json : "{}");
}

int64_t cbm_android_graph_insert_edge(cbm_gbuf_t *gbuf, int64_t source_id, int64_t target_id,
                                      const char *type, const char *properties_json) {
    if (!gbuf || source_id <= 0 || target_id <= 0 || !type || !cbm_android_vocab_is_edge_type(type)) {
        return 0;
    }
    return cbm_gbuf_insert_edge(gbuf, source_id, target_id, type,
                                properties_json ? properties_json : "{}");
}
