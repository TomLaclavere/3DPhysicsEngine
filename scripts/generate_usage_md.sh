#!/usr/bin/env bash
set -euo pipefail

OUTPUT="scripts/USAGE_HELP.md"
echo "# Scripts Help Summary" > "$OUTPUT"
echo "" >> "$OUTPUT"

for script in scripts/*.sh; do
    if grep -q 'usage()' "$script"; then
        echo "## $(basename "$script")" >> "$OUTPUT"
        echo "" >> "$OUTPUT"

        # Extract text between `cat <<EOF` or `cat <<'EOF'` and `EOF`
        awk "/cat <<'?EOF'?/{flag=1;next}/^EOF/{flag=0}flag" "$script" >> "$OUTPUT"

        echo "" >> "$OUTPUT"
        echo "---" >> "$OUTPUT"
        echo "" >> "$OUTPUT"
    fi
done

echo "Generated $OUTPUT"
