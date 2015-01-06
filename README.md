ImageMagick library
===================

ImageMagick Magick++ library bindings for versatile programatic image manipulation.

[![Build Status][BS img]][Build Status]

This is another [approach][node-imagemagick-native] to bring image manipulation capabilities to nodejs.
However conceptually it's more like [this][node-graphicsmagick]. You know, "same same, but different".

WARNING:

This is work in progress...


Magicklib provides an `Image` object which can be manipulated in both sync and async mode.
The async mode is triggered either by providing callback to utility method as the last argument or
by calling image.begin() to start batch mode.

##Example

###Synchronous example

```js
  var Image = require("magicklib-native").Image
```

```js
  var im = new Image("magick.png")
  fs.writeFileSync("magick.jpg",
    im.resize(100,100, "aspectfill greater").
      extent(100,100, "center").
      blur(0.3).
      format("JPEG").
      quality(80).
      write()
  );
  im.close() // free resources before gc does
```

###Asynchronous batch example

```js
  fs.readFile("magick.png", function(err, blob) {
    // bail on err
    new Image().
      begin().
      read(blob).
      resize(100,100, "aspectfill").
      extent(100,100, "center").
      blur(0.3).
      format("JPEG").
      quality(80).
      write(function(err, blob) {
        // bail on err
        fs.writeFile("magick.jpg", blob, function(err) {
          // done
        });
      });
      // in batch mode image is being closed by default
  });
```

To enable batch-only mode:

```js
  var im = new Image({batch: true}); // or
  // no im.begin() needed anymore
  // and no more synchronous calls
  im.batch = true;                   // or
  im.prop({batch: true});            // or
  im.begin(true);
```

To disable batch-only mode:

```js
  var im = new Image({batch: false}); // or
  im.batch = false;                   // or
  im.prop({batch: false});            // or
  im.end(false); // this also ends any pending commands
```

###Copy copy

Images may be copied to perform different transformations without reloading from file or blob.

```js
  var im = new Image(blob);

  var blobsmall = im.copy().
    sharpen(0.2).
    resize("100x100").
    write();

  var blobmedium = im.copy().
    blur(0.2).
    resize("300x300^").
    write();
```

###Streams


```js
  var im = new Image({batch: true}).
    format("JPEG").
    quality(80).
    resize(100, 100, "aspectfill").
    extent(100, 100, "center")

  // create write stream

  fs.createReadStream("image.jpg").
    pipe(im.createWriteStream());

  // create read stream

  im.blur(1).createReadStream().pipe(
    fs.createWriteStream("out100x100aspectfill_blurred.jpg")
  );

  im.end(function(err, image) {
    // all transformations done and sent down the stream
  });

```

Asynchronous copy + stream == read once, convert many

```js
  var im = new Image({batch: true, autoClose: false}).
    copy(true). // must be here otherwise restore() will restore empty image
    format("JPEG").
    quality(80).
    resize(100, 100, "aspectfill").
    extent(100, 100, "center")

  fs.createReadStream("image.jpg").
    pipe( im.createConvertStream() ).
    pipe( fs.createWriteStream("out100x100.jpg") );

  // create as many read streams as needed

  im.restore().  // restore copy
    resize(300, 500).
    createReadStream().pipe(
      fs.createWriteStream("out300x500.jpg")
    );

  im.restore().  // restore copy
    sharpen(2).
    // a sugar for createReadStream().pipe()
    pipe(
      fs.createWriteStream("sharpened.jpg")
    );

  im.close(true); // auto-close image after transformations
```

Both write and convert streams insert blob reader at the front of the batch queue.


To turn autoCopy mode on:

```js
  var im = new Image({src: buffer, autoCopy: true}); // or
  im.autoCopy = true;                                // or
  im.prop({autoCopy: true});                         // or
  im.copy(true);
```

###Save memory, auto-close

Image is being closed automatically after finishing batch by default,
so rss won't grow like crazy between gc sessions.

```js
  var im = new Image({batch: true});
  im.autoClose == true;
  im.read(blob).
    blur(0.5).
    write(function(err, blob) {
      // play with blob
    }).
    crop(200, 200, 50, 50).
    end(function(err, im) {
      setImmediate(function() {
        // im is closed by now to free Magick memory as soon as possible
        im.empty == true
        im.size() // => [0, 0]
      });
      // im is not yet closed, you can play more with it
      im.empty == false
    });
```

To turn off auto close:

```js
  new Image({autoClose: false}); // or
  im.autoClose = false;          // or
  im.prop({autoClose: false});   // or
  im.close(false);

  // to close manually
  // batch or sync
  im.close();
  // async
  im.close(function(err, im) { /*...*/ });
```

Closing image doesn't mean one can't use instantiated JS object anymore.
On the contrary - close() simply destroys internal Magick memory associated with Image,
bringing back JS object to its pristine state as if new magick.Image() was called.

So yes, one can re-use closed images.

## Installation

### Unix

Requires [ImageMagick][imagemagick-install-source] at least v6.8.7 C++ library and headers.

* [Linux][imagemagick-download-linux]

```
  $ sudo yum install ImageMagick-c++-devel
```

or

```
  $ sudo apt-get install libmagick++-dev
```

* [OS X][imagemagick-download-maxosx]

```
  brew install imagemagick
```

Magick++-config should be in PATH.

```
  $ npm install magicklib-native
```

According to [imagemagick-native][node-imagemagick-native-install]

* RHEL/CentOS: If the version of ImageMagick required is not available in RPM repository, please try the `-last` version offered by Les RPM de Remi, for example:

```
  $ sudo yum remove -y ImageMagick
  $ sudo yum install -y http://rpms.famillecollet.com/enterprise/remi-release-6.rpm
  $ sudo yum install -y --enablerepo=remi ImageMagick-last-c++-devel
```

* Mac OS X: You might need to install `pkgconfig` first:

```
  $ brew install pkgconfig
```

### Windows

Tested on Windows 7 x64, Vista x64 and XP x86.

1. Install Python >= 2.7.3

2. Install [Visual Studio C++ 2010 Express][vs-2010-express-download] or (Windows 7/8 only) [Microsoft Visual Studio C++ 2012/13][vs-2012-express-download]

3. (VS 2010 and 64-bit only) [Install Windows 7 64-bit SDK][win-7-64bit-sdk-download] and [compiler update for the Windows SDK 7.1][cusp-win-sdk-71-download]

4. Install [ImageMagick Q16/Q8 x64/x86 dll][imagemagick-download-windows] and please remember to check "Install development headers and libraries for C and C++" during install. "-static" library versions won't work.

See [node-gyp installation][node-gyp-troubleshooting] for general troubleshooting.

```
  $ npm install magicklib-native
```

##API

See [API.md](API.md) for more.

##Performance

```
  node --expose-gc test/bench.js test/test.image.jpg
```

##TODO

- (much) more Magick++ methods

[node-graphicsmagick]: https://github.com/networkimprov/node-graphicsmagick
[node-imagemagick-native]: https://github.com/mash/node-imagemagick-native
[imagemagick-install-source]: http://www.imagemagick.org/script/install-source.php#unix
[imagemagick-download-linux]: http://www.imagemagick.org/script/binary-releases.php#unix
[imagemagick-download-maxosx]: http://www.imagemagick.org/script/binary-releases.php#macosx
[imagemagick-download-windows]: http://www.imagemagick.org/script/binary-releases.php#windows
[node-imagemagick-native-install]: https://github.com/mash/node-imagemagick-native/#user-content-installation
[vs-2010-express-download]: http://go.microsoft.com/?linkid=9709949
[vs-2012-express-download]: http://go.microsoft.com/?linkid=9816758
[win-7-64bit-sdk-download]: http://www.microsoft.com/en-us/download/details.aspx?id=8279
[cusp-win-sdk-71-download]: http://www.microsoft.com/en-us/download/details.aspx?id=4422
[node-gyp-troubleshooting]: https://github.com/TooTallNate/node-gyp#installation
[Build Status]: https://travis-ci.org/royaltm/node-magicklib-native
[BS img]: https://travis-ci.org/royaltm/node-magicklib-native.svg
