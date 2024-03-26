#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GODOT=${GODOT:-godot}
PROJECT=${PROJECT:-$SCRIPT_DIR/../demo}

die() {
    echo "$@" >/dev/stderr
    exit 1
}

# Regenerate the XML data from the API.
$GODOT --doctool "$SCRIPT_DIR/.." --path "$PROJECT" --gdextension-docs --xr-mode off \
    || die "Failed to regenerate XML using Godot's --doctool"

openxr_vendors_classes=$(cd "$SCRIPT_DIR/../thirdparty/godot_openxr_vendors/doc_classes" && ls *.xml)
for x in $openxr_vendors_classes; do
    rm -f "$SCRIPT_DIR/../doc_classes/$x"
done
