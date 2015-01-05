var test = require("tap").test
  , magick = require("..")
  , async = require("async")
  , Image = magick.Image
  , blob = require('fs').readFileSync("test.transparent160.png")
  , blobImage = require("fs").readFileSync("test.image.jpg");

test("Image batch sanity", function (t) {
  var im = new Image({src: blob, batch: true, autoClose: false})
  t.type(im, Image, "is instance of Image")
  t.strictEqual(im.autoClose, false, "autoClose is false")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , true,  "batch is true")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, 0,     "batchSize is 0")
  t.strictEqual(im.isSync   , false, "isSync is false")
  t.equal(im.rows, 160, "has 160 rows")
  t.equal(im.columns, 160, "has 160 columns")
  t.strictEqual(im.magick, "PNG", "PNG magick")
  t.strictEqual(im.page, "160x160", "160x160 page")

  t.strictEqual(im.end(false), im, "im.end() -> this")
  t.strictEqual(im.autoClose, false, "autoClose is false")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, null,  "batchSize is null")
  t.strictEqual(im.isSync   , true,  "isSync is true")
  t.deepEqual(im.size(), [160, 160], "has 160x160")

  t.strictEqual(im.begin(), im, "im.begin() -> this")
  t.strictEqual(im.autoClose, false, "autoClose is false")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, 0,     "batchSize is 0")
  t.strictEqual(im.isSync   , false, "isSync is false")

  t.strictEqual(im.size(), im, "im.size() -> this")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, 1,     "batchSize is 1")
  t.strictEqual(im.isSync   , false, "isSync is false")

  t.strictEqual(im.end(function(err, size) {
    t.strictEqual(err         , null,  "err is null")
    t.deepEqual(size          , [160, 160], "has 160x160")
    t.strictEqual(im.batch    , false, "batch is false")
    t.strictEqual(im.busy     , true, "busy is true")
    t.strictEqual(im.empty    , false, "empty is false")
    t.strictEqual(im.batchSize, 0,     "batchSize is 0")
    t.strictEqual(im.isSync   , false, "isSync is false")

    setImmediate(function() {
      t.strictEqual(im.batch    , false, "batch is false")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , false, "empty is false")
      t.strictEqual(im.batchSize, null,  "batchSize is null")
      t.strictEqual(im.isSync   , true,  "isSync is true")
      t.deepEqual(im.size(), [160, 160], "has 160x160")

      t.strictEqual(im.begin(true), im, "im.begin(true) -> this")
      t.strictEqual(im.batch    , true,  "batch is true")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , false, "empty is false")
      t.strictEqual(im.batchSize, 0,     "batchSize is 0")
      t.strictEqual(im.isSync   , false, "isSync is false")

      t.strictEqual(im.size(), im, "im.size() -> this")
      t.strictEqual(im.batch    , true,  "batch is true")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , false, "empty is false")
      t.strictEqual(im.batchSize, 1,     "batchSize is 1")
      t.strictEqual(im.isSync   , false, "isSync is false")

      t.strictEqual(im.end()    , im,    "im.end() -> this")
      t.strictEqual(im.batch    , true,  "batch is true")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , false, "empty is false")
      t.strictEqual(im.batchSize, 0,     "batchSize is 0")
      t.strictEqual(im.isSync   , false, "isSync is false")

      t.strictEqual(im.end(false)    , im,    "im.end(false) -> this")
      t.strictEqual(im.batch    , false, "batch is false")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , false, "empty is false")
      t.strictEqual(im.batchSize, null,  "batchSize is null")
      t.strictEqual(im.isSync   , true,  "isSync is true")

      t.end()
    })

  }), im, "im.end() -> this")

  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , true,  "busy is true")
  t.strictEqual(im.empty    , false, "empty is false")
  t.ok(im.batchSize == 0 || im.batchSize == 1, "batchSize is 0 or 1")
  t.strictEqual(im.isSync   , false, "isSync is false")
  
})

test("Image batch should auto close", function (t) {
  var im = new Image({src: blob})
  t.type(im, Image, "is instance of Image")
  t.strictEqual(im.autoClose, true,  "autoClose is true")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, null,  "batchSize is null")
  t.strictEqual(im.isSync   , true,  "isSync is true")
  t.equal(im.rows, 160, "has 160 rows")
  t.equal(im.columns, 160, "has 160 columns")
  t.strictEqual(im.magick, "PNG", "PNG magick")
  t.strictEqual(im.page, "160x160", "160x160 page")

  t.strictEqual(im.end(), im, "im.end() -> this")
  t.deepEqual(im.size(), [160, 160], "has 160x160")
  t.strictEqual(im.autoClose, true,  "autoClose is true")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, null,  "batchSize is null")
  t.strictEqual(im.isSync   , true,  "isSync is true")

  t.strictEqual(im.begin(), im, "im.begin() -> this")
  t.strictEqual(im.autoClose, true,  "autoClose is true")
  t.strictEqual(im.autoCopy , false, "autoCopy is false")
  t.strictEqual(im.batch    , false, "batch is false")
  t.strictEqual(im.busy     , false, "busy is false")
  t.strictEqual(im.empty    , false, "empty is false")
  t.strictEqual(im.batchSize, 0,     "batchSize is 0")
  t.strictEqual(im.isSync   , false, "isSync is false")

  t.strictEqual(im.end(function(err, image) {
    t.strictEqual(err         , null,  "err is null")
    t.strictEqual(image       , im,   "image === im")
    t.strictEqual(im.autoClose, true,  "autoClose is true")
    t.strictEqual(im.autoCopy , false, "autoCopy is false")
    t.strictEqual(im.batch    , false, "batch is false")
    t.strictEqual(im.busy     , true, "busy is true")
    t.strictEqual(im.empty    , false, "empty is false")
    t.strictEqual(im.batchSize, 0,     "batchSize is 0")
    t.strictEqual(im.isSync   , false, "isSync is false")
    t.equal(im.rows, 160, "has 160 rows")
    t.equal(im.columns, 160, "has 160 columns")
    t.strictEqual(im.magick, "PNG", "PNG magick")
    t.strictEqual(im.page, "160x160", "160x160 page")

    setImmediate(function() {
      t.strictEqual(im.batch    , false, "batch is false")
      t.strictEqual(im.busy     , false, "busy is false")
      t.strictEqual(im.empty    , true,  "empty is true")
      t.strictEqual(im.batchSize, null,  "batchSize is null")
      t.strictEqual(im.isSync   , true,  "isSync is true")
      t.deepEqual(im.size(), [0, 0], "has 0x0")

      t.end()
    })
  }), im, "im.end() -> this")
})

test("Properties should be accessible during async process", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false});

  t.plan(10*9 + 1)

  async.times(10, function(i, next) {
    im.blur(10, function(err, im2) {
      t.ok(err == null                , "no error")
      t.type(im2, Image               , "have Image")
      t.notStrictEqual(im2, im        , "have copy")
      t.strictEqual(im2.autoCopy, true, "copy has autoCopy")
      im2.autoCopy = false
      im2.blur(1, function(err, im3) {
        t.strictEqual(im3, im2)
        next()
      })
      t.strictEqual(im2.page, "113x150")
      im2.page = "A0"
      t.strictEqual(im2.page, "2384x3370")
    })

    t.notEqual(im.page, "595x842+10+10")
    im.page = "A4+10+10"
    t.strictEqual(im.page, "595x842+10+10")
    im.page = "113x150"
  }, function(err, images) {

    t.ok(err == null, "no error")
  })
});
