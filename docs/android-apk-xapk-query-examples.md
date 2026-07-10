# Android APK/XAPK query examples

These examples assume you already have an extracted Android APK/XAPK reverse-engineering folder on disk and have indexed it with the normal project flow:

```bash
codebase-memory-mcp cli index_repository '{"repo_path":"/path/to/extracted-android-folder"}'
```

This project does **not** unpack APK/XAPK files, run jadx/apktool, perform dynamic analysis, bypass protections, or produce malware verdicts. It only indexes files that already exist in the folder: `AndroidManifest.xml`, Java/Kotlin/smali, resources, assets, native-library paths, and related extracted artifacts.

Use your indexed project name in the examples below. If unsure, run `get_graph_schema` first.

## Inspect available Android labels

```bash
codebase-memory-mcp cli get_graph_schema '{"project":"my-extracted-app"}'
```

Look for labels such as:

- `AndroidApp`
- `Manifest`
- `Activity`
- `Service`
- `BroadcastReceiver`
- `ContentProvider`
- `Permission`
- `IntentFilter`
- `DeepLink`
- `ResourceString`
- `LayoutResource`
- `ApiEndpoint`
- `Asset`

## Search by label

List manifest-declared activities:

```bash
codebase-memory-mcp cli search_graph '{"project":"my-extracted-app","label":"Activity","limit":20}'
```

Find a specific component by name pattern:

```bash
codebase-memory-mcp cli search_graph '{"project":"my-extracted-app","label":"Activity","name_pattern":".*Main.*","limit":20}'
```

List string resources:

```bash
codebase-memory-mcp cli search_graph '{"project":"my-extracted-app","label":"ResourceString","limit":50}'
```

Find API endpoint strings discovered from resources:

```bash
codebase-memory-mcp cli search_graph '{"project":"my-extracted-app","label":"ApiEndpoint","limit":50}'
```

## Query manifest structure

List activities:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (a:Activity) RETURN a.name, a.file LIMIT 50"}'
```

List services:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (s:Service) RETURN s.name, s.file LIMIT 50"}'
```

List broadcast receivers:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (r:BroadcastReceiver) RETURN r.name, r.file LIMIT 50"}'
```

List content providers:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (p:ContentProvider) RETURN p.name, p.file LIMIT 50"}'
```

## Query permissions and deep links

Requested permissions:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (app:AndroidApp)-[:REQUESTS_PERMISSION]->(p:Permission) RETURN app.name, p.name LIMIT 100"}'
```

Components that expose deep links:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (c)-[:EXPOSES_DEEPLINK]->(d:DeepLink) RETURN c.name, d.name LIMIT 100"}'
```

Intent filters and actions:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (f:IntentFilter)-[:HANDLES_ACTION]->(a:IntentAction) RETURN f.name, a.name LIMIT 100"}'
```

## Query resources and assets

String resources:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (s:ResourceString) RETURN s.name, s.file LIMIT 100"}'
```

Layout resources:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (l:LayoutResource) RETURN l.name, l.file LIMIT 100"}'
```

Assets and native-library evidence:

```bash
codebase-memory-mcp cli query_graph '{"project":"my-extracted-app","query":"MATCH (a:Asset) RETURN a.name, a.file LIMIT 100"}'
```

## Inspect evidence properties

Android-specific nodes include JSON properties such as:

- `source_format`
- `confidence`
- `evidence_path`
- `source_file`
- `android_kind`
- manifest/resource-specific fields such as `exported`, `scheme`, `host`, `path_prefix`, `resource_name`, or `value`

Use `search_graph` when you want properties inline:

```bash
codebase-memory-mcp cli search_graph '{"project":"my-extracted-app","label":"DeepLink","limit":20}'
```

Then inspect each result's `properties_json` to see why the node exists and where the evidence came from.

## Current extraction scope

The Android pass currently emits facts from already-extracted text/path artifacts:

- `AndroidManifest.xml`: app, manifest, components, permissions, intent filters, deep links.
- `res/values/strings.xml`: string resources and obvious URL endpoints.
- `res/layout`, `res/menu`, `res/navigation`, `res/drawable*`, `res/xml`, `res/raw`: resource nodes by path/name.
- `assets/**`: asset nodes.
- `*.so`: native-library path evidence as `Asset` nodes with native-library properties.

Java/Kotlin/smali files continue through the normal language indexing pipeline alongside these Android semantic nodes.
