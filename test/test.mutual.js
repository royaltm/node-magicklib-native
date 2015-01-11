var test = require("tap").test
  , magick = require("..")
  , Image = magick.Image
  , blobImage = require("fs").readFileSync("test.image.jpg")

var q = require("async").queue(function(task, next) {
  task.method(function(err, arg) {
    task.callback(err, arg, next);
  });
}, 1);

function syncAsync(method, callback) {
  var im2 = method();
  callback(null, im2);
  q.push({method: method, callback: function(err, arg, next) {
    setImmediate(function() {
      callback(err, arg);
      next();
    });
  }}, function(err) {
    if (err) throw err;
  });
}

function testComposition(t, im, src, x, y) {
  t.deepEqual(im.color(x - 1, y - 1).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(x + src.columns, y + src.rows).rgba(), [ 0, 0, 0, 1 ])
  for(var i = 0; i < 8; ++i) {
    var srcx = (Math.random()*src.columns)|0
     ,  srcy = (Math.random()*src.rows)|0
    t.notDeepEqual(im.color(x + srcx, y + srcy).rgba(), [ 0, 0, 0, 1 ], "at: " + srcx + "x" + srcy)
  }
}

test("image.composite", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})

  t.plan(6 + 20*2*(3 + 10) + 8*2*(3 + 10))

  t.throws(function() { im.composite(new Image().begin()) },
      {name: "Error", message: "Could not synchronize images"})

  t.throws(function() { im.composite(new Image(), "xxxx") },
      {name: "Error", message: "Unrecognized composite operator"})

  t.throws(function() { im.composite(new Image(), "center", "xxxx") },
      {name: "Error", message: "Unrecognized composite operator"})

  t.throws(function() { im.composite(new Image(), 0, 0, "xxxx") },
      {name: "Error", message: "Unrecognized composite operator"})

  t.throws(function() { im.composite({image: new Image(), compose: "xxxx"}) },
      {name: "Error", message: "Unrecognized composite operator"})

  var src = new Image(blobImage).resize(120,160,"fill").close(false)

  t.throws(function() { im.composite({}) },
      {name: "TypeError", message: "missing Image"});

  [
    im.composite.bind(im, src),
    im.composite.bind(im, src, "over"),
    im.composite.bind(im, src, "center"),
    im.composite.bind(im, src, "center", "over"),
    im.composite.bind(im, src, 90, 70),
    im.composite.bind(im, src, 90, 70, "over"),
    im.composite.bind(im, src, [0,0,90,70]),
    im.composite.bind(im, src, [0,0,90,70], "over"),
    im.composite.bind(im, src, "+90+70"),
    im.composite.bind(im, src, "+90+70", "over"),
    im.composite.bind(im, {image: src}),
    im.composite.bind(im, {image: src, compose: "over"}),
    im.composite.bind(im, {image: src, gravity: "center"}),
    im.composite.bind(im, {image: src, gravity: "center", compose: "over"}),
    im.composite.bind(im, {image: src, geometry: "+90+70"}),
    im.composite.bind(im, {image: src, geometry: "+90+70", compose: "over"}),
    im.composite.bind(im, {image: src, geometry: [0,0,90,70]}),
    im.composite.bind(im, {image: src, geometry: [0,0,90,70], compose: "over"}),
    im.composite.bind(im, {image: src, x: 90, y: 70}),
    im.composite.bind(im, {image: src, x: 90, y: 70, compose: "over"}),
  ].forEach(function(method) {
    syncAsync(method, function(err, im2) {
      t.ok(err == null, "no error")
      t.notStrictEqual(im2, im)
      t.deepEqual(im2, im)
      testComposition(t, im2, src, im2.columns/2 - src.columns/2, im2.rows/2 - src.rows/2)
    })
  });

  [
    im.composite.bind(im, src, "southwest", "copy"),
    im.composite.bind(im, src, 0, 140, "copy"),
    im.composite.bind(im, src, [0,0,0,140], "copy"),
    im.composite.bind(im, src, "+0+140", "copy"),
    im.composite.bind(im, {image: src, gravity: "southwest", compose: "copy"}),
    im.composite.bind(im, {image: src, geometry: "+0+140", compose: "copy"}),
    im.composite.bind(im, {image: src, geometry: [0,0,0,140], compose: "copy"}),
    im.composite.bind(im, {image: src, x: 0, y: 140, compose: "copy"}),
  ].forEach(function(method) {
    syncAsync(method, function(err, im2) {
      t.ok(err == null, "no error")
      t.notStrictEqual(im2, im)
      t.deepEqual(im2, im)
      testComposition(t, im2, src, 0, 140)
    })
  })

})

test("mutual synchronization async read", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (4 + 4) + (3 + 2 + 10))
  src.begin().read(blobImage).resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.type(src.batchSize, "number")
  src.end(function(err, src2) {
    t.ok(err == null, "no error")
    t.strictEqual(src2, src)
    t.equal(src2.columns, 120)
    t.equal(src2.rows, 160)
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
    })
  })
  im.end(function(err, im2) {
    t.ok(err == null, "no error")
    setImmediate(function() {
      t.strictEqual(im.busy, false)
      t.strictEqual(im.batchSize, null)
    })
    t.notStrictEqual(im2, im)
    t.deepEqual(im2, im)
    testComposition(t, im2, new Image(blobImage).resize(120,160,"fill"), 90, 70)
  })
})

test("mutual synchronization hold and finish", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (4 + 4) + (3 + 2 + 10))
  src._hold()
  src.resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.equal(src.batchSize, 2)
  src.end(function(err, src2) {
    t.ok(err == null, "no error")
    t.strictEqual(src2, src)
    t.equal(src2.columns, 120)
    t.equal(src2.rows, 160)
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
    })
  })
  im.end(function(err, im2) {
    t.ok(err == null, "no error")
    setImmediate(function() {
      t.strictEqual(im.busy, false)
      t.strictEqual(im.batchSize, null)
    })
    t.notStrictEqual(im2, im)
    t.deepEqual(im2, im)
    testComposition(t, im2, new Image(blobImage).resize(120,160,"fill"), 90, 70)
  })
  src._wrap(blobImage)
})

test("mutual synchronization hold and error", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (3 + 4) + (3 + 2))
  src._hold()
  src.resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.equal(src.batchSize, 2)
  src.end(function(err, src2) {
    t.type(err, Error)
    t.like(err.toString(), (/zero-length blob not permitted/))
    t.strictEqual(src2, void(0))
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
    })
  })
  im.end(function(err, im2) {
    t.type(err, Error)
    t.like(err.toString(), (/Could not synchronize images/))
    t.strictEqual(im2, void(0))
    setImmediate(function() {
      t.strictEqual(im.busy, false)
      t.strictEqual(im.batchSize, null)
    })
  })
  src._wrap(new Buffer(0))
})

test("mutual synchronization hold and error 2", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (3 + 6))
  src._hold()
  src.resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.equal(src.batchSize, 2)
  im.end(function(err, im2) {
    t.type(err, Error)
    t.like(err.toString(), (/Could not synchronize images/))
    t.strictEqual(im2, void(0))
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
      t.strictEqual(im.busy, false)
      t.strictEqual(im.batchSize, null)
    })
  })
  src._wrap(new Buffer(0))
})

test("mutual synchronization error", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(5 + (3 + 6))
  src.begin().read(new Buffer(0)).resize(120,160,"fill")
  t.equal(src.batchSize, 2)
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.type(src.batchSize, "number")
  im.end(function(err, im2) {
    t.type(err, Error)
    t.like(err.toString(), (/Could not synchronize images/))
    t.strictEqual(im2, void(0))
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
      t.strictEqual(im.busy, false)
      t.strictEqual(im.batchSize, null)
    })
  })
})

test("mutual synchronization premature end", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (4 + 4) + 2)
  src.begin().read(blobImage).resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.type(src.batchSize, "number")
  src.end(function(err, src2) {
    t.ok(err == null, "no error")
    t.strictEqual(src2, src)
    t.equal(src2.columns, 120)
    t.equal(src2.rows, 160)
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
      t.end()
    })
  })
  im.end()
  t.strictEqual(im.busy, false)
  t.strictEqual(im.batchSize, null)
})

test("mutual synchronization hold premature end", function (t) {
  var im = new Image({autoClose: false, autoCopy: true, columns: 300, rows: 300, color: "transparent", type: "truecolor", magick: "PNG"})
  var src = new Image()
  t.plan(4 + (4 + 4) + 2)
  src._hold()
  src.resize(120,160,"fill")
  im.begin().composite(src)
  t.strictEqual(im.busy, false)
  t.equal(im.batchSize, 1)
  t.strictEqual(src.busy, true)
  t.type(src.batchSize, "number")
  src.end(function(err, src2) {
    t.ok(err == null, "no error")
    t.strictEqual(src2, src)
    t.equal(src2.columns, 120)
    t.equal(src2.rows, 160)
    setImmediate(function() {
      t.strictEqual(src.busy, false)
      t.strictEqual(src.batchSize, null)
      t.equal(src.columns, 0)
      t.equal(src.rows, 0)
      t.end()
    })
  })
  im.end()
  t.strictEqual(im.busy, false)
  t.strictEqual(im.batchSize, null)
  src._wrap(blobImage)
})
