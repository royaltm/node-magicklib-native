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


##magick


###Magick resource control

```js
 
  limit(type[, limit]) -> current limit
 
  type: "memory", "disk" or "thread"
 
  limit: for "memory" and "disk" it is number of bytes,
         for "threads" no. threads
```

##magick.Image


###Image object factory

```js
 
  new Image([options])
  new Image(buffer)
  new Image(geometry, color)
  new Image(width, height[, color="transparent"])
  new Image(file)
  new Image(image)
 
  buffer:      data Buffer to synchronously load from
  geometry:    geometry string
  color:       color string or Color object instance
  image:       Image object to clone
  file:        file name to synchronously load from
               (not recommended, uses Magick++ blocking I/O)
 
  options:
 
    - src:       image data Buffer
    - columns:   number of columns (width)
    - rows:      number of rows (height)
    - color:     new image color string or Color object, default: transparent
    - magick:    set format, default: Magick++ default
    - page:      page description string
    - batch:     set persistent batch mode, default: false
    - autoClose: set auto close mode, default: true
    - autoCopy:  set auto copy mode, default: false
    - background: set background color, default: unknown
    - fuzz:       set color fuzz, default: 0
```

###Begin batch mode

```js
 
  image.begin(batch)
  image.begin(options)
 
  options:
 
    - batch: set persistent batch mode, default: false
 
  if batch or options argument is provided this function
  immediately changes batch image property.
```

###Blur image

```js
 
  image.blur(sigma[, radius][, channel][, gaussian][, callback(err, image)])
  image.blur(options[, callback(err, image)])
 
  options:
 
    - sigma: standard deviation of the Laplacian or the Gaussian bell curve
    - radius: the radius of the Gaussian, in pixels or number of neighbor
              pixels to be included in the convolution mask
    - channel: channel type string, e.g.: "yellow"
    - gaussian: true for gaussian blur
 
```

###Copy image

```js
 
  image.copy([callback(err, newimage)])
  image.copy(autoCopy[, callback(err, newimage)]) -> image
  image.copy(options[, callback(err, newimage)]) -> image
 
  options:
 
    - autoCopy: set auto copy mode, default: false
```

###Close image

```js
 
  image.close([callback(err, image)])
  image.close(autoClose) -> image
  image.close(options) -> image
  
  options:
 
    - autoClose: set auto close mode, default: true
 
  free magick image resource (even if image.autoCopy==true)
 
  if autoClose or options argument is provided this function
  immediately changes autoClose image property.
```

###Comment image

```js
 
  image.comment(comment[,callback(err, image)])
  image.comment([callback(err, comment)])
```

###Color pixel plot or peek

```js
 
  in synchronous context:
  image.color(x, y) -> color
  image.color(points) -> colors
  image.color(x, y, newcolor) -> image
 
  in asynchronous context:
  image.color(x, y, callback(err, color))
  image.color(points, callback(err, colors))
  image.color(x, y, newcolor, callback(err, image))
 
  points: an Array of [x, y] tuples
  newcolor: string or Color object
  color: Color object
  colors: an Array of Color objects
```

###Crop image

```js
 
  image.crop(width, height, x, y[, callback(err, image)])
  image.crop(size[, callback(err, image)])
  image.crop(geometry[, callback(err, image)])
 
  size: an Array of [width, height, x, y]
  geometry: geometry string
```

###End batch

```js
  
  image.end([callback(err, image)])
  image.end(batch[, callback(err, image)])
  image.end(options[, callback(err, image)])
 
  options:
 
    - batch: set persistent batch mode, default: false
 
  if callback is provided executes batch otherwise clears batch tail from jobs
 
  if batch or options argument is provided this function
  immediately changes batch image property.
```

###Extent image

```js
 
  image.extent(width, height[, gravity="center"][, color="transparent"][, callback(err, image)])
  image.extent(width, height, x, y[, color="transparent"][, callback(err, image)])
  image.extent(size[, gravity="ignore"][, color="transparent"][, callback(err, image)])
  image.extent(geometry[, gravity="ignore"][, color="transparent"][, callback(err, image)])
 
  size: an Array of [width, height] or [width, height, x, y]
  geometry: geometry string
  color: string or Color object
```

###Filter for resize

```js
  
  image.filter(filter[, callback(err, image)])
 
  filter: filter string, default: "Lanczos"
```

###Flip image

```js
  
  image.flip([callback(err, image)])
```

###Flop image

```js
  
  image.flop([callback(err, image)])
```

###Format change

```js
 
  image.format(magick[, callback(err, image)])
 
  magick: format string, e.g. "JPEG"
```

###Histogram

```js
 
  image.histogram([callback(err, histogram)])
```

###Negate image

```js
 
  image.negate([grayscale=false][, callback(err, image)])
```

###Noise image

```js
 
  image.noise(noise[, channel][, callback(err, image)])
  image.noise(options[, callback(err, image)])
 
  options:
 
    - noise: noise type string, e.g.: "multiplicative"
    - channel: channel type string, e.g.: "matte"
 
  see: http://www.imagemagick.org/Magick++/Enumerations.html#NoiseType
       http://www.imagemagick.org/Magick++/Enumerations.html#ChannelType
```

###Normalize image

```js
 
  image.normalize([callback(err, image)])
```

###Oilpaint image

```js
 
  image.oil([radius=3][, callback(err, image)])
```

###Ping image

```js
 
  image.ping(file|buffer[, callback(err, size)])
 
  buffer: a Buffer object to read image from (recommended)
  file: a file to read from (not recommended, uses Magick++ blocking I/O)
  size: an Array of [width, height]
 
  todo: implement async read for file
```

###Properties set or get

```js
 
  in synchronous context:
  image.properties() -> properties
  image.properties(options)
 
  in asynchronous context:
  image.properties(callback(err, properties))
  image.properties(options, callback(err, image))
 
  properties: an Object with image properties
 
  options:
 
    - columns:    number of columns (width)
    - rows:       number of rows (height)
    - magick:     set format
    - page:       page description string
    - batch:      set persistent batch mode
    - autoClose:  set auto close mode
    - autoCopy:   set auto copy mode
    - background: set background color
    - fuzz:       set color fuzz
```

###Quality change

```js
 
  image.quality(quality[, callback(err, image)])
 
  quality: a number 0 - 100
```

###Quantize image

```js
 
  image.quantize([colors][, colorspace][, dither][, callback(err, image)])
  image.quantize(options, callback(err, image)])
 
  options:
 
    - colors: a number of quantized colors
    - colorspace: a colorspace string
    - dither: true or false
```

###Restore image

```js
 
  image.restore([callback(err, image)])
```

###Rotate image

```js
 
  image.rotate(degrees[, callback(err, image)])
 
  degrees: a number 0 - 360
```

###Sharpen image

```js
 
  image.sharpen(sigma[, radius][, channel][, callback(err, image)])
  image.sharpen(options[, callback(err, image)])
 
  options:
 
    - sigma: standard deviation of the Laplacian
    - radius: the radius of the Gaussian, in pixels
    - channel: channel type string, e.g.: "blue"
 
```

###Size get or set

```js
 
  in synchronous context:
  image.size() -> size
  image.size(size)
  image.size(width, height)
 
  in asynchronous context:
  image.size(callback(err, size))
  image.size(size, callback(err, image))
  image.size(width, height, callback(err, image))
 
  size: an Array of [width, height]
```

###Read image

```js
 
  image.read(file|buffer[, callback(err, image)])
 
  buffer: a Buffer object to read image from (recommended)
  file: a file to read from (not recommended, uses Magick++ blocking I/O)
 
  todo: implement async read for file
```

###Reset image page

```js
 
  image.reset([callback])
  image.reset(page[, callback])
  image.reset(width, height[, x, y][, callback(err, image)])
 
  page: geometry string or page description, e.g. "A4"
```

###Resize image

```js
 
  image.resize(width, height[, mode="aspectfit"][, callback(err, image)])
  image.resize(size[, mode="aspectfit"][, callback(err, image)])
  image.resize(options[, callback(err, image)])
 
  size: an Array of [width, height] or geometry string, e.g.: "100x100^"
 
  mode:
    - "#" or "aspectfit"          Resize the image based on the largest fitting dimenstion (default)
    - "^" or "aspectfill"         Resize the image based on the smallest fitting dimension
    - "!" or "noaspect" or "fill" Force exact size
    - ">" or "larger"             Resize only if larger than geometry
    - "!>" or "nolarger"          Don't resize only if smaller than geometry
    - "<" or "smaller"            Resize only if smaller than geometry
    - "!<" or "nosmaller"         Don't resize only if larger than geometry
    - "%" or "percent"            Interpret width & height as percentages
    - "!%" or "nopercent"         Don't interpret width & height as percentages
    - "@" or "limit"              Resize using a pixel area count limit
    - "!@" or "nolimit"           Don't resize using a pixel area count limit
    - "filter"                    Use resize filter (default)
    - "sample"                    Use pixel sampling algorithm (no filters)
    - "scale"                     Use simple ratio algorithm (no filters)
 
  mode tags can be combined (separated by space, comma or (semi)colon),
 
  options:
 
    - size: target size [witdth, height] or geometry string
    - width: target width
    - height: target height
    - mode: resize mode
```

###Strip image of profiles and comments

```js
 
  image.strip([callback(err, image)])
```

###Trim image

```js
 
  image.trim([fuzz][, callback(err, image)])
 
  fuzz: a percent string "50%" or a number 0.0 - 1.0
        colors within fuzz distance are considered equal
```

###Type image

```js
 
  image.type([callback(err, imageType)])
  image.type(imageType[, callback(err, image)])
 
  imageType: an image type string, e.g. "greyscale", "palette", "truecolor"
  see: http://www.imagemagick.org/Magick++/Enumerations.html#ImageType
```

###Write image

```js
 
  image.write(file[, callback(err, image)])
  image.write([callback(err, buffer)])
 
  file: a file to write to (not recommended, uses Magick++ blocking I/O)
 
  without file or if file == null the result is buffer
 
  todo: implement async write to file
```

###Image properties

```js
 
  image.busy -> true if asynchronous operation in progress


  image.empty -> true if image is initialized as empty or closed


  image.batchSize -> number of jobs pushed to batch


  image.isSync -> true if image is ready for synchronous operation
                  (not busy and not in batch mode)


  image.batch -> true if image in batch-only (persistent) mode
  image.batch = boolean changes batch-only (persistent) mode state


  image.autoClose -> true if image in close mode
  image.autoClose = boolean changes close mode state


  image.autoCopy -> true if image in copy mode
  image.autoCopy = boolean changes copy mode state


  image.columns -> number of columns in image
  image.columns = number changes number of columns


  image.rows -> number of rows in image
  image.rows = number changes number of rows


  image.magick -> image format string
  image.magick = string changes format


  image.page -> image page string
  image.page = string changes page


  image.background -> image background color
  image.background = changes background color


  image.fuzz -> image color fuzz number
  image.fuzz = changes fuzz
```

##magick.Image (stream)


###Writeable image stream

```js
 
  image.createWriteStream() -> stream
 
  stream: Writeable stream instance
 
  note: can't create many simultaneous writable streams from one image instance
```

###Convert image stream

```js
 
  image.createConvertStream() -> convert
 
  convert: Transform stream instance
 
  note: can't create many simultaneous convert streams from one image instance
```

###Read stream from image

```js
 
  image.createReadStream() -> stream
 
  stream: Readable stream instance
```

##magick.Color


###Color object factory

```js
 
  new Color(redQuantum, greenQuantum, blueQuantum[, alphaQuantum=opaque])
  new Color(color)
  new Color(value)
 
  colorQuantum: an integer from 0 - magick.QUANTUM_RANGE
  color:        color string or Color object instance
  value:        a 32-bit integer color value
```

###Red color

```js
 
  color.red([scale=1]) -> number
```

###Green color

```js
 
  color.red([scale=1]) -> number
```

###Blue color

```js
 
  color.red([scale=1]) -> number
```

###Alpha color

```js
 
  color.alpha([scale=1]) -> number
```

###Y color (from YUV model)

```js
 
  color.y([scale=1]) -> number
```

###U color (from YUV model)

```js
 
  color.u([scale=1]) -> number
```

###V color (from YUV model)

```js
 
  color.v([scale=1]) -> number
```

###Hue color (from HSB model)

```js
 
  color.hue([scale=1]) -> number
```

###Lightness color (from HSL model)

```js
 
  color.lightness([scale=1]) -> number
```

###Value color (from HSV model)

```js
 
  color.value([scale=1]) -> number
```

###Brightness color (from HSB model)

```js
 
  color.brightness([scale=1]) -> number
```

###Intensity color (from HSI model)

```js
 
  color.intensity([scale=1]) -> number
```

###RGB scaled color (from RGB model)

```js
 
  color.rgb([scale=1]) -> Array(3)
```

###RGBA scaled color (from RGB model)

```js
 
  color.rgba([scale=1]) -> Array(4)
```

###YUV scaled color (from YUV model)

```js
 
  color.yuv([scale=1]) -> Array(3)
```

###YUVA scaled color (from YUV model)

```js
 
  color.yuva([scale=1]) -> Array(4)
```

###HSL scaled color (from HSL model)

```js
 
  color.hsl([scale=1]) -> Array(3)
```

###HSLA scaled color (from HSL model)

```js
 
  color.hsla([scale=1]) -> Array(4)
```

###HSV scaled color (from HSV model)

```js
 
  color.hsv([scale=1]) -> Array(3)
```

###HSVA scaled color (from HSV model)

```js
 
  color.hsva([scale=1]) -> Array(4)
```

###HSB scaled color (from HSB model)

```js
 
  color.hsb([scale=1]) -> Array(3)
```

###HSBA scaled color (from HSB model)

```js
 
  color.hsba([scale=1]) -> Array(4)
```

###HSI scaled color (from HSI model)

```js
 
  color.hsi([scale=1]) -> Array(3)
```

###HSIA scaled color (from HSI model)

```js
 
  color.hsia([scale=1]) -> Array(4)
```

###HWB scaled color (from HWB model)

```js
 
  color.hwb([scale=1]) -> Array(3)
```

###HWBA scaled color (from HWB model)

```js
 
  color.hwba([scale=1]) -> Array(4)
```

###RGB Color object factory

```js
 
    Color.RGB(red, green, blue[, alpha=opaque]) -> color
```

###YUV Color object factory

```js
 
    Color.YUV(y, u, v[, alpha=opaque]) -> color
```

###HSL Color object factory

```js
 
    Color.HSL(hue, saturation, lightness[, alpha=opaque]) -> color
```

###HSV Color object factory

```js
 
    Color.HSV(hue, saturation, value[, alpha=opaque]) -> color
```

###HSB Color object factory

```js
 
    Color.HSB(hue, saturation, brightness[, alpha=opaque]) -> color
```

###HSI Color object factory

```js
 
    Color.HSI(hue, saturation, intensity[, alpha=opaque]) -> color
```

###HWB Color object factory

```js
 
    Color.HWB(hue, whiteness, blackness[, alpha=opaque]) -> color
```

###Value of color

```js
 
  color.valueOf() -> a 32-bit integer color value
```

###String name of color

```js
 
  color.toString() -> an x11/html color value
```

###Quantum from normalized value

```js
 
    Color.toQuantum(value) -> integer
```

###Normalized value form quantum

```js
 
    Color.fromQuantum(quantum) -> number
```
