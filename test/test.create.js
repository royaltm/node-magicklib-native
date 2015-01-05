var test = require("tap").test
  , magick = require("..")
  , async = require("async")
  , Image = magick.Image
  , Color = magick.Color
  , blob = require('fs').readFileSync("test.transparent.png")
  , blob160 = require('fs').readFileSync("test.transparent160.png")
  , blobImage = require('fs').readFileSync("test.image.jpg");

test("Empty Image factory", function (t) {
  var im = new Image();
  t.type(im, Image, "is instance of Image")
  t.strictEqual(im.autoClose, true, "autoClose is true")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , true , "empty is true")
  t.strictEqual(im.batchSize, null,  "batchSize is null")
  t.strictEqual(im.isSync   , true,  "isSync is true")
  t.equal(im.rows, 0, "has 0 rows")
  t.equal(im.columns, 0, "has 0 columns")
  t.strictEqual(im.magick, "", "no magick")
  t.strictEqual(im.page, "", "no page")
  t.end()
})

test("Empty Image factory from properties", function (t) {
  var im = new Image({autoCopy: true, batch: true, autoClose: false})
  t.type(im, Image, "is instance of Image")
  t.strictEqual(im.autoClose, false, "autoClose is false")
  t.strictEqual(im.autoCopy , true , "autoCopy is true")
  t.strictEqual(im.batch    , true , "batch is true")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , true , "empty is true")
  t.strictEqual(im.batchSize, 0    , "batchSize is 0")
  t.strictEqual(im.isSync   , false, "isSync is false")
  t.equal(im.rows, 0, "has 0 rows")
  t.equal(im.columns, 0, "has 0 columns")
  t.strictEqual(im.magick, "", "no magick")
  t.strictEqual(im.page, "", "no page")
  t.end()
})

test("Not empty Image factory from properties", function (t) {
  var im = new Image({autoCopy: true, batch: true, autoClose: false, page: "A4", magick: "JPEG"})
  t.type(im, Image, "is instance of Image")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, 0    ,  "batchSize is 0")
  t.strictEqual(im.isSync   , false,  "isSync is false")
  t.equal(im.rows, 0, "has 0 rows")
  t.equal(im.columns, 0, "has 0 columns")
  t.strictEqual(im.magick, "JPEG", "magick JPEG")
  t.strictEqual(im.page, "595x842", "page A4")
  t.strictEqual(im.autoClose, false, "autoClose is false")
  t.strictEqual(im.autoCopy,  true,  "autoCopy is true")
  t.strictEqual(im.batch,     true,  "batch is true")
  t.end()
})

test("Image with size and color factory", function (t) {

  function test(t, im, color) {
    t.type(im, Image, "is instance of Image")
    t.strictEqual(im.busy     , false, "busy is false")
    t.strictEqual(im.empty    , false, "empty is false")
    t.strictEqual(im.batchSize, null,  "batchSize is null")
    t.strictEqual(im.isSync   , true,  "isSync is true")
    t.equal(im.columns, 640, "has 640 columns")
    t.equal(im.rows, 512, "has 512 rows")
    t.strictEqual(im.magick, "XC", "magick XC")
    t.strictEqual(im.page, "640x512", "page 640x512")
    t.strictEqual(im.autoClose, true, "autoClose is true")
    t.strictEqual(im.autoCopy, false, "autoCopy is false")
    t.strictEqual(im.batch,    false, "batch is false")
    t.deepEqual(im.color(1, 1).rgba(255), color , "has color")
    var colors = im.color([[0, 0], [539,511], [320, 256]])
    t.type(colors, Array, "colors is array")
    t.deepEqual(colors[0].rgba(255), color , "is color")
    t.deepEqual(colors[1].rgba(255), color , "is color")
    t.deepEqual(colors[2].rgba(255), color , "is color")
  }

  t.plan(9*17)
  test(t, new Image(640, 512)             , [0, 0, 0, 255])
  test(t, new Image(640, 512, "#FF880044"), [255, 136, 0, 187])
  test(t, new Image(640, 512, "black"), [0, 0, 0, 0])
  test(t, new Image(640, 512, "white"), [255, 255, 255, 0])
  test(t, new Image(640, 512, Color.RGB(1,128/255,16/255)), [255, 128, 16, 0])
  test(t, new Image("640x512", "#FF880044"), [255, 136, 0, 187])
  test(t, new Image("640x512", "black"), [0, 0, 0, 0])
  test(t, new Image("640x512", "white"), [255, 255, 255, 0])
  test(t, new Image("640x512", Color.RGB(1,128/255,16/255)), [255, 128, 16, 0])

})

test("Image with size and color factory from properties", function (t) {

  function test(t, im, color) {
    t.type(im, Image, "is instance of Image")
    t.strictEqual(im.busy     , false, "busy is false")
    t.strictEqual(im.empty    , false, "empty is false")
    t.strictEqual(im.batchSize, 0,     "batchSize is 0")
    t.strictEqual(im.isSync   , false, "isSync is false")
    t.equal(im.columns, 640, "has 640 columns")
    t.equal(im.rows, 512, "has 512 rows")
    t.strictEqual(im.magick, "JPEG", "magick JPEG")
    t.strictEqual(im.page, "595x842", "page A4")
    t.strictEqual(im.autoClose, false, "autoClose is false")
    t.strictEqual(im.autoCopy, true, "autoCopy is true")
    t.strictEqual(im.batch,    true, "batch is true")
    im.end(false);
    t.strictEqual(im.batchSize, null,  "batchSize is null")
    t.strictEqual(im.isSync   , true,  "isSync is true")
    var pixel = im.color(1, 1)
    t.type(pixel, Color , "is instance of Color")
    t.deepEqual(pixel.rgba(255), color , "has color")
    var colors = im.color([[0, 0], [539,511], [320, 256]])
    t.type(colors, Array, "colors is array")
    t.deepEqual(colors[0].rgba(255), color , "is color")
    t.deepEqual(colors[1].rgba(255), color , "is color")
    t.deepEqual(colors[2].rgba(255), color , "is color")
  }

  t.plan(5*20)
  test(t, new Image({columns: 640, rows: 512, autoCopy: true, batch: true, autoClose: false,
    page: "A4", magick: "JPEG"}),
                                       [0, 0, 0, 255])
  test(t, new Image({columns: 640, rows: 512, autoCopy: true, batch: true, autoClose: false,
    page: "A4", magick: "JPEG", color: "#FF880044"}),
                                       [255, 136, 0, 187])
  test(t, new Image({columns: 640, rows: 512, autoCopy: true, batch: true, autoClose: false,
    page: "A4", magick: "JPEG", color: "black"}),
                                       [0, 0, 0, 0])
  test(t, new Image({columns: 640, rows: 512, autoCopy: true, batch: true, autoClose: false,
    page: "A4", magick: "JPEG", color: "white"}),
                                       [255, 255, 255, 0])
  test(t, new Image({columns: 640, rows: 512, autoCopy: true, batch: true, autoClose: false,
    page: "A4", magick: "JPEG", color: Color.RGB(1,128/255,16/255)}),
                                       [255, 128, 16, 0])
})

test("Image from file factory", function (t) {

  function test(t, im) {
    t.type(im, Image, "is instance of Image")
    t.equal(im.rows, 16, "has 16 rows")
    t.equal(im.columns, 16, "has 16 columns")
    t.strictEqual(im.magick, "PNG", "PNG magick")
    t.strictEqual(im.page, "16x16", "16x16 page")
    t.strictEqual(im.autoClose, true, "autoClose is true")
    t.strictEqual(im.autoCopy, false, "autoCopy is false")
    t.strictEqual(im.batch,    false, "batch is false")
    t.deepEqual(im.color(8,8).rgba(255), [255,255,255,   0] , "has color")
    t.deepEqual(im.color(1,1).rgba(255), [  0,  0,  0,   0] , "has color")
    t.deepEqual(im.color(2,2).rgba(255), [  0,  0,  0, 255] , "has color")
  }

  t.plan(5 * 11 + 2)
  test(t, new Image(blob))
  test(t, new Image({src: blob}))
  test(t, new Image("test.transparent.png"))
  test(t, new Image({src: "test.transparent.png"}))
  var image = new Image(blob), image2 = new Image(image)
  t.deepEqual(image, image2)
  t.notStrictEqual(image, image2)
  image.close()
  test(t, image2)

})

test("Image from image", function (t) {
  var im = new Image({src: blobImage, autoClose: false})
  var im2 = new Image(im)
  t.plan(9 + 100 + 1 + 100)
  t.type(im , Image)
  t.type(im2, Image)
  t.notStrictEqual(im        , im2             , "not same")
  t.equal(im.rows            , im2.rows        , "same dimenstions")
  t.equal(im.columns         , im2.columns     , "same dimenstions")
  t.equal(im.rows            , 150             , "150 rows")
  t.equal(im.columns         , 113             , "113 columns")
  t.deepEqual(im.properties(), im2.properties(), "alike")
  t.deepEqual(im             , im2             , "alike")
  for(var i = 0; i < 100; ++i) {
    var x = (Math.random()*im.columns)|0
      , y = (Math.random()*im.rows)|0
    t.deepEqual(im.color(x, y), im2.color(x, y), "very alike")
  }
  t.strictEqual(im.type('grayscale'), im)
  for(var i = 0; i < 100; ++i) {
    var x = (Math.random()*im.columns)|0
      , y = (Math.random()*im.rows)|0
    t.notDeepEqual(im.color(x, y), im2.color(x, y), "not shared image")
  }
})

test("Image must not be synchronously cloned during async process", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false})

  t.plan(10 + 1)

  async.times(10, function(i,next) {
    im.blur(10, next)
    t.throws(function() { new Image(im) }, "image is busy and could not be cloned synchronously")
  }, function(err) {
    t.equal(err, void(0), "no error")
  })
})

test("Ping image", function(t) {
  var im = new Image()
  t.ok(im.empty, "is empty")
  t.deepEqual(im.ping(blob160), [160, 160])
  t.ok(im.empty, "is empty")
  t.strictEqual(im.read(blob), im)
  t.ok(!im.empty, "not empty")
  t.equal(im.columns, 16)
  t.equal(im.rows, 16)
  t.strictEqual(im.ping(blobImage, function(err, size) {
    t.ok(err == null)
    t.deepEqual(size, [113, 150]);
    t.ok(!im.empty, "not empty")
    t.equal(im.columns, 16)
    t.equal(im.rows, 16)
    t.end()
  }), im)
})
