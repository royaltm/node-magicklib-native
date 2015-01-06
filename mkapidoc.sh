#!/bin/sh
apifile=API.md
cat > "$apifile" <<'EOF'
Magicklib Api
=============

```
  npm install magicklib-native
```

```js
  var magick = require('magicklib-native');
  var Image = magick.Image;
```

Most Image methods return image (this) object.

EOF

render () { 
  echo -e "\n##${1}\n" >> "$apifile"
  grep -E -e "^   \*|^  \/\*\*" "$2" | \
    sed 's#^  /\*\*##' | \
    sed 's#^   \*\*/#```#' | \
    sed 's#^   \*/##' | \
    sed 's/^   \* \([A-Z].*\)/\1\n\n```js/' | \
    sed 's#^   \*# #' \
    >> "$apifile"
}

render magick src/nodemagick.cc
render magick.Image src/image.cc
render "magick.Image (stream)" magick.js
render magick.Color src/color.cc
