var test = require("tap").test
  , magick = require("..")
  , Image = magick.Image
  , Color = magick.Color
  , blobImage = require("fs").readFileSync("test.image.jpg")

function testCopy(t, im, im2, width, height) {
  t.type(im , Image)
  t.type(im2, Image)
  t.notStrictEqual(im        , im2             , "not same")
  t.equal(im.rows            , im2.rows        , "same dimenstions")
  t.equal(im.columns         , im2.columns     , "same dimenstions")
  t.equal(im.rows            , height          , height + " rows")
  t.equal(im.columns         , width           , width + " columns")
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
}

test("should copy Image synchronously", function(t) {
  var im = new Image(blobImage)

  t.plan(1 + 2*(9 + 100 + 1 + 100))

  t.strictEqual(im, im.copy(false))

  var im2 = im.copy()

  testCopy(t, im , im2       , 113, 150)
  testCopy(t, im2, im2.copy(), 113, 150)
})

test("should copy Image asynchronously", function(t) {
  var im = new Image({src: blobImage, autoClose: false})
    , clones = []

  t.plan(1 + 1 + 2 + 3 + 2*(9 + 100 + 1 + 100))

  im.copy(function(err, im2) {
    t.ok(err == null)
    clones.push(im2)
  })
  .copy()
  .end(function(err, im2) {
    t.ok(err == null)
    clones.push(im2)
  })
  .end(function(err, im2) {
    t.ok(err == null)
    t.strictEqual(im, im2)
    setImmediate(function() {
      t.notStrictEqual(im, clones[0])
      t.notStrictEqual(im, clones[1])
      t.notStrictEqual(clones[0], clones[1])
      testCopy(t, im       , clones[0], 113, 150)
      testCopy(t, clones[0], clones[1], 113, 150)
    })
  })
})

test("should auto copy Image synchronously", function(t) {
  var im = new Image({src: blobImage, autoCopy: true})
  var im2 = im.properties({autoCopy: false})

  t.plan(1 + 3*(9 + 100 + 1 + 100))

  testCopy(t, im , im2 , 113, 150)

  im2.autoCopy = true;
  var im3 = im2.properties({autoCopy: false})
  testCopy(t, im2, im3, 113, 150)

  t.notStrictEqual(im3, im3.copy(true))

  var im4 = im3.properties({autoCopy: false})
  testCopy(t, im3, im4, 113, 150)
})

test("should auto copy Image asynchronously", function(t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false})
    , clones = []

  t.plan(1 + 1 + 2 + 3 + 2*(9 + 100 + 1 + 100))

  im.properties({autoCopy: false}, function(err, im2) {
    t.ok(err == null)
    clones.push(im2)
  })
  .copy(true)
  .copy(false)
  .end(function(err, im2) {
    t.ok(err == null)
    clones.push(im2)
  })
  .end(function(err, im2) {
    t.ok(err == null)
    t.strictEqual(im, im2)
    setImmediate(function() {
      t.notStrictEqual(im, clones[0])
      t.notStrictEqual(im, clones[1])
      t.notStrictEqual(clones[0], clones[1])
      testCopy(t, im       , clones[0], 113, 150)
      testCopy(t, clones[0], clones[1], 113, 150)
    })
  })

})
