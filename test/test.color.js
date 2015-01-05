var test = require("tap").test
  , magick = require("..")
  , async = require("async")
  , Image = magick.Image
  , Color = magick.Color

test("should paint some pixels synchronously", function(t) {
  var im = new Image(4, 4, "transparent")
    , red         = Color.RGB(1,0,0)
    , green       = Color.RGB(0,1,0)
    , magenta     = Color.RGB(1,0,1)
    , transparent = Color.RGB(0, 0, 0, 1)

  t.strictEqual(im,
    im.color(0, 0, red)
      .color(1, 1, green)
      .color(2, 2, magenta)
  )

  t.deepEqual(im.color(0, 0), red)
  t.deepEqual(im.color(1, 1), green)
  t.deepEqual(im.color(2, 2), magenta)
  t.deepEqual(im.color(3, 3), transparent)

  t.deepEqual(im.color([
    [0, 0], [1, 1], [2, 2], [3, 3]]),
    [red  , green , magenta, transparent])

  t.end()
})

test("should paint some pixels asynchronously", function(t) {
  var im = new Image({columns: 4, rows: 4, color: "white", autoClose: false, batch: true})
    , red     = Color.RGB(1,0,0)
    , green   = Color.RGB(0,1,0)
    , magenta = Color.RGB(1,0,1)
    , white   = Color.RGB(1,1,1)

  t.plan(1 + 2 + 2 + 2)

  t.strictEqual(im,
    im.color(0, 0, red)
      .color(1, 1, green)
      .color(2, 2, magenta, function(err, im2) {
      t.ok(err == null)
      t.strictEqual(im, im2)
      async.series([
        im.color.bind(im, 0, 0),
        im.color.bind(im, 1, 1),
        im.color.bind(im, 2, 2),
        im.color.bind(im, 3, 3)
      ], function(err, colors) {
        t.ok(err == null)
        t.deepEqual(colors, [red, green, magenta, white])
        im.color([[0, 0], [1, 1], [2, 2], [3, 3]], function(err, colors) {
          t.ok(err == null)
          t.deepEqual(colors, [red, green, magenta, white])
        })
      })
    })
  )

})
