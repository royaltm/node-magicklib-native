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

Image methods return image (this) object unless they read something from image in synchronous mode.

Invoking method without callback either executes synchronously or adds command to asynchronous batch.
Image method will perform synchronously if: `image.isSync == true`

The synchronous mode is mainly for performing image operation while loading modules or to use in repl.
The preferred way of dealing with images in nodejs should always be asynchronous.

The direct file operations, such as in read(file) or new Image(file) should be avoided until they are
implemented in node-friendly way. For now they use blocking I/O from Magick++ calls.
It's better to use streams, or blobs.

EOF

render () { 
  echo -e "\n##${1}\n" >> "$apifile"
  grep -E -e "^   \*|^  \/\*\*" "$2" | \
    sed 's#^  /\*\*##' | \
    sed 's#^   \*\*/#```#' | \
    sed 's#^   \*/##' | \
    sed 's/^   \* \([A-Z].*\)/###\1\n\n```js/' | \
    sed 's#^   \*# #' \
    >> "$apifile"
}

render magick src/nodemagick.cc
render magick.Image src/image.cc
render "magick.Image (stream)" magick.js
render magick.Color src/color.cc
