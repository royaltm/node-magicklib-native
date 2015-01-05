var test = require("tap").test
  , magick = require("..")
  , async = require("async")
  , Image = magick.Image
  , Color = magick.Color

test("Image should have readable properties", function(t) {
  var options = {columns: 200, rows: 100, color: "white", background: "#FF0000", magick: "PNG", page: "A4"}
    , im = new Image(options)
    , ip = im.properties()

  t.equal(im.columns, 200)
  t.equal(ip.columns, 200)
  t.equal(im.rows, 100)
  t.equal(ip.rows, 100)
  t.deepEqual(im.background, Color.RGB(1,0,0))
  t.deepEqual(ip.background, Color.RGB(1,0,0))
  t.equal(im.magick, "PNG")
  t.equal(ip.magick, "PNG")
  t.equal(im.page, "595x842")
  t.equal(ip.page, "595x842")
  im.properties(function(err, p2) {
    t.ok(err == null)
    t.deepEqual(ip, p2);
    t.end()
  })
})

test("Image should have writable properties", function(t) {
  var im = new Image()
    , ip = im.properties()

  t.equal(im.columns, 0)
  t.equal(ip.columns, 0)
  im.columns = 200
  t.equal(im.columns, 200)
  t.equal(im.properties().columns, 200)

  t.equal(im.rows, 0)
  t.equal(ip.rows, 0)
  im.rows = 100
  t.equal(im.rows, 100)
  t.equal(im.properties().rows, 100)

  t.deepEqual(im.size(), [200, 100])

  im.background = "white"
  t.deepEqual(im.background, Color.RGB(1,1,1))
  t.deepEqual(im.properties().background, Color.RGB(1,1,1))

  t.equal(im.magick, "")
  t.equal(ip.magick, "")
  im.magick = "PNG"
  t.equal(im.magick, "PNG")
  t.equal(im.properties().magick, "PNG")

  t.equal(im.page, "")
  t.equal(ip.page, "")
  im.page = "A4+10+10"
  t.equal(im.page, "595x842+10+10")
  t.equal(im.properties().page, "595x842+10+10")

  t.strictEqual(im.properties({
    columns: 400,
    rows: 300,
    magick: "JPEG",
    page: "300x300",
    background: "transparent"
  }), im)
  ip = im.properties()
  t.equal(im.columns, 400)
  t.equal(ip.columns, 400)
  t.equal(im.rows, 300)
  t.equal(ip.rows, 300)
  t.deepEqual(im.size(), [400, 300])
  t.deepEqual(im.background, Color.RGB(0,0,0,1))
  t.deepEqual(ip.background, Color.RGB(0,0,0,1))
  t.equal(im.magick, "JPEG")
  t.equal(ip.magick, "JPEG")
  t.equal(im.page, "300x300")
  t.equal(ip.page, "300x300")

  t.strictEqual(im.properties({
    columns: 250,
    rows: 150,
    magick: "XC",
    page: "",
    background: "black"
  }, function(err, im2) {
    t.ok(err == null)
    t.equal(im.columns, 250)
    t.equal(im.rows, 150)
    t.deepEqual(im.background, Color.RGB(0,0,0))
    t.equal(im.magick, "XC")
    t.equal(im.page, "")
    im.properties(function(err, p2) {
      t.ok(err == null)
      t.equal(p2.columns, 250)
      t.equal(p2.rows, 150)
      t.deepEqual(p2.background, Color.RGB(0,0,0))
      t.equal(p2.magick, "XC")
      t.equal(p2.page, "")
      im.size(function(err, size) {
        t.ok(err == null)
        t.deepEqual(size, [250, 150])
        t.end()
      })
    })
  }), im)
})
