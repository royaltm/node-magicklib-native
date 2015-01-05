var test = require("tap").test
var magick = require("..")

test("magick sanity", function (t) {
  t.type(magick.Image, "function", "Image is function")
  t.type(magick.Color, "function", "Color is function")
  t.type(magick.limit, "function", "limit is function")
  t.type(magick.QUANTUM_DEPTH, "number", "QUANTUM_DEPTH is number")
  t.type(magick.QUANTUM_RANGE, "number", "QUANTUM_RANGE is number")
  t.equal(magick.QUANTUM_RANGE, (1 << magick.QUANTUM_DEPTH) - 1, "QUANTUM_RANGE is 2^QUANTUM_DEPTH - 1")
  t.end()
})

test("limit", function(t) {
  var thread, disk, memory
  t.similar(String(thread = magick.limit("thread")), (/^\d+$/))
  t.similar(String(disk   = magick.limit("disk")),   (/^\d+$/))
  t.similar(String(memory = magick.limit("memory")), (/^\d+$/))

  t.equal(magick.limit("thread", 1)           , thread)
  t.equal(magick.limit("disk", 100*1000*1000) , disk)
  t.equal(magick.limit("memory", 10*1000*1000), memory)

  t.equal(magick.limit("thread"), 1)
  t.equal(magick.limit("disk")  , 100*1000*1000)
  t.equal(magick.limit("memory"), 10*1000*1000)

  t.equal(magick.limit("thread", thread), 1)
  t.equal(magick.limit("disk",   disk)  , 100*1000*1000)
  t.equal(magick.limit("memory", memory), 10*1000*1000)

  t.equal(magick.limit("thread"),   thread)
  t.equal(magick.limit("disk")  ,   disk)
  t.equal(magick.limit("memory"),   memory)
  t.end()
})
