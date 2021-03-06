var test = require("tap").test
  , magick = require("..")
  , Image = magick.Image
  , Color = magick.Color
  , blobImage = require("fs").readFileSync("test.image.jpg")
  , blobTransparent = require("fs").readFileSync("test.transparent.png")
  , blobT160 = require("fs").readFileSync("test.transparent160.png")
  , blobTrim = require("fs").readFileSync("test.trim.png")

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

test("image.blur", function (t) {
  var im = new Image({src: blobT160, autoCopy: true, autoClose: false})
  t.plan(6 + 2*2*5 + 2*2*5 + 2*2*7 + 2*2*5)
  t.deepEqual(im.color(9,9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(11,11).rgba(), [ 0, 0, 0, 0 ])
  t.throws(function(){ im.blur({sigma:NaN}) }, new TypeError("blur() needs sigma number option"));
  t.throws(function(){ im.blur({}) }, new TypeError("blur() needs sigma number option"));
  t.throws(function(){ im.blur(void(0)) }, new TypeError("blur()'s sigma is not a number"));
  t.throws(function(){ im.blur() }, new TypeError("blur()'s arguments should be number(s)[, string][, boolean]"));

  [
    im.blur.bind(im, 1),
    im.blur.bind(im, {sigma: 1})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 90)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 12)
    })
  });
  [
    im.blur.bind(im, 1, 2),
    im.blur.bind(im, {sigma: 1, radius: 2})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 90)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 11)
    })
  });
  [
    im.blur.bind(im, 1, 2, 'yellow'),
    im.blur.bind(im, {sigma: 1, radius: 2, channel: 'yellow'})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha() , 1)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(), 0)
      var color = im.color(70,70)
      t.equal(im.color(70,70).r, im.color(70,70).g)
      t.ok(im.color(70,70).b < im.color(70,70).r)
    })
  });
  [
    im.blur.bind(im, 1, 2, true),
    im.blur.bind(im, {sigma: 1, radius: 2, gaussian: true})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 91)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 10)
    })
  });
})

test("image.crop", function (t) {
  var im = new Image({src: blobT160, autoCopy: true, autoClose: false})
  t.plan(10 + 6*2*9 + 6*2*9)
  t.deepEqual(im.size()               , [ 160, 160 ])
  t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(10,  10).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(60,  60).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(69,  69).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(80,  80).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(150,150).rgba(), [ 0, 0, 0, 1 ])

  t.throws(function(){ im.crop({}) }, new TypeError("crop() needs proper size or width and height options"));
  t.throws(function(){ im.crop() }, new TypeError("crop()'s arguments should be string or 4 numbers"));

  [
    im.crop.bind(im, 160, 160, 10, 10),
    im.crop.bind(im, [160, 160, 10, 10]),
    im.crop.bind(im, "160x160+10+10"),
    im.crop.bind(im, {width: 160, height: 160, x: 10, y: 10}),
    im.crop.bind(im, {size: [160, 160, 10, 10]}),
    im.crop.bind(im, {size: "160x160+10+10"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "160x160+10+10")
      t.deepEqual(im.size()               , [ 150, 150 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(10,  10).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(60,  60).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(69,  69).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(80,  80).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(150,150).rgba(), [ 0, 0, 0, 1 ])
    });
  });
  [
    im.crop.bind(im, 160, 160, -10, -10),
    im.crop.bind(im, [160, 160, -10, -10]),
    im.crop.bind(im, "160x160-10-10"),
    im.crop.bind(im, {width: 160, height: 160, x: -10, y: -10}),
    im.crop.bind(im, {size: [160, 160, -10, -10]}),
    im.crop.bind(im, {size: "160x160-10-10"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "160x160")
      t.deepEqual(im.size()               , [ 150, 150 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(10,  10).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(60,  60).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(69,  69).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(80,  80).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(150,150).rgba(), [ 0, 0, 0, 0 ])
    });
  });
})

test("image.extent", function (t) {
  var im = new Image({src: blobTransparent, autoCopy: true, autoClose: false, background: "transparent"})
  t.plan(11 + 14*2*11 + 12*2*10 + 12*2*11 + 28*2*12);
  t.deepEqual(im.size()               , [ 16, 16 ])
  t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(1,    1).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(6,    6).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(7,    7).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(8,    8).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(14,  14).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 1 ])

  t.throws(function(){ im.extent() }, new TypeError("extent()'s arguments should be string or number(s)"));
  t.throws(function(){ im.extent({}) }, new TypeError("extent() needs proper size or width and height options"));

  [
    im.extent.bind(im, 14, 14),
    im.extent.bind(im, 14, 14, "centeR"),
    im.extent.bind(im, 14, 14, 1, 1),
    im.extent.bind(im, [14, 14, 1, 1]),
    im.extent.bind(im, [14, 14], "Center"),
    im.extent.bind(im, "14x14+1+1"),
    im.extent.bind(im, "14x14", "center"),
    im.extent.bind(im, {width: 14, height: 14}),
    im.extent.bind(im, {width: 14, height: 14, gravity: "centeR"}),
    im.extent.bind(im, {width: 14, height: 14, x: 1, y: 1}),
    im.extent.bind(im, {size: [14, 14, 1, 1]}),
    im.extent.bind(im, {size: [14, 14], gravity: "Center"}),
    im.extent.bind(im, {size: "14x14+1+1"}),
    im.extent.bind(im, {size: "14x14", gravity: "center"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "14x14")
      t.deepEqual(im.size()               , [ 14, 14 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(1,    1).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(6,    6).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(7,    7).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(8,    8).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(13,  13).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(14,  14).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 0 ])
    });
  });
  [
    im.extent.bind(im, 14, 14, "nOrThWeST"),
    im.extent.bind(im, 14, 14, 0, 0),
    im.extent.bind(im, [14, 14, 0, 0]),
    im.extent.bind(im, [14, 14], "NORTHwest"),
    im.extent.bind(im, "14x14+0+0"),
    im.extent.bind(im, "14x14", "nortHWesT"),
    im.extent.bind(im, {width: 14, height: 14, gravity: "nOrThWeST"}),
    im.extent.bind(im, {width: 14, height: 14, x: 0, y: 0}),
    im.extent.bind(im, {size: [14, 14, 0, 0]}),
    im.extent.bind(im, {size: [14, 14], gravity: "NORTHwest"}),
    im.extent.bind(im, {size: "14x14+0+0"}),
    im.extent.bind(im, {size: "14x14", gravity: "nortHWesT"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "14x14")
      t.deepEqual(im.size()               , [ 14, 14 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(1,    1).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(7,    7).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(8,    8).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(13,  13).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(14,  14).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 1 ])
    });
  });
  [
    im.extent.bind(im, 14, 14, "EAST"),
    im.extent.bind(im, 14, 14, 2, 1),
    im.extent.bind(im, [14, 14, 2, 1]),
    im.extent.bind(im, [14, 14], "east"),
    im.extent.bind(im, "14x14+2+1"),
    im.extent.bind(im, "14x14", "eASt"),
    im.extent.bind(im, {width: 14, height: 14, gravity: "EAST"}),
    im.extent.bind(im, {width: 14, height: 14, x: 2, y: 1}),
    im.extent.bind(im, {size: [14, 14, 2, 1]}),
    im.extent.bind(im, {size: [14, 14], gravity: "east"}),
    im.extent.bind(im, {size: "14x14+2+1"}),
    im.extent.bind(im, {size: "14x14", gravity: "eASt"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "14x14")
      t.deepEqual(im.size()               , [ 14, 14 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(1,    1).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(5,    6).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(6,    7).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(7,    8).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(12,  13).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(13,  13).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(14,  14).rgba(), [ 0, 0, 0, 1 ])
      t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 1 ])
    });
  });
  [
    im.extent.bind(im, 18, 18, "#00FF00"),
    im.extent.bind(im, 18, 18, Color.RGB(0,1,0)),
    im.extent.bind(im, 18, 18, "center", "#00FF00"),
    im.extent.bind(im, 18, 18, "center", Color.RGB(0,1,0)),
    im.extent.bind(im, 18, 18, -1, -1, "#00FF00"),
    im.extent.bind(im, 18, 18, -1, -1, Color.RGB(0,1,0)),
    im.extent.bind(im, [18, 18, -1, -1], "#00FF00"),
    im.extent.bind(im, [18, 18, -1, -1], Color.RGB(0,1,0)),
    im.extent.bind(im, [18, 18], "center", "#00FF00"),
    im.extent.bind(im, [18, 18], "center", Color.RGB(0,1,0)),
    im.extent.bind(im, "18x18-1-1", "#00FF00"),
    im.extent.bind(im, "18x18-1-1", Color.RGB(0,1,0)),
    im.extent.bind(im, "18x18", "center", "#00FF00"),
    im.extent.bind(im, "18x18", "center", Color.RGB(0,1,0)),
    im.extent.bind(im, {width: 18, height: 18, color: "#00FF00"}),
    im.extent.bind(im, {width: 18, height: 18, color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {width: 18, height: 18, gravity: "center", color: "#00FF00"}),
    im.extent.bind(im, {width: 18, height: 18, gravity: "center", color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {width: 18, height: 18, x: -1, y: -1, color: "#00FF00"}),
    im.extent.bind(im, {width: 18, height: 18, x: -1, y: -1, color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {size: [18, 18, -1, -1], color: "#00FF00"}),
    im.extent.bind(im, {size: [18, 18, -1, -1], color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {size: [18, 18], gravity: "center", color: "#00FF00"}),
    im.extent.bind(im, {size: [18, 18], gravity: "center", color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {size: "18x18-1-1", color: "#00FF00"}),
    im.extent.bind(im, {size: "18x18-1-1", color: Color.RGB(0,1,0)}),
    im.extent.bind(im, {size: "18x18", gravity: "center", color: "#00FF00"}),
    im.extent.bind(im, {size: "18x18", gravity: "center", color: Color.RGB(0,1,0)})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "18x18")
      t.deepEqual(im.size()               , [ 18, 18 ])
      t.deepEqual(im.color(0,    0).rgba(), [ 0, 1, 0, 0 ])
      t.deepEqual(im.color(1,    1).rgba(), [ 0, 1, 0, 0 ])
      t.deepEqual(im.color(2,    2).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(7,    7).rgba(), [ 0, 1, 0, 0 ])
      t.deepEqual(im.color(8,    8).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(9,    9).rgba(), [ 1, 1, 1, 0 ])
      t.deepEqual(im.color(10,  10).rgba(), [ 0, 1, 0, 0 ])
      t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 0 ])
      t.deepEqual(im.color(16,  16).rgba(), [ 0, 1, 0, 0 ])
      t.deepEqual(im.color(17,  17).rgba(), [ 0, 1, 0, 0 ])
    });
  });
});

test("image.flip", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false})
  t.plan(2*im.rows);
  syncAsync(im.flip.bind(im), function(err, im2) {
    for(var y = 0, cols = im2.columns, rows = im2.rows; y < rows; ++y) {
      var x = (Math.random() * cols)|0;
      t.deepEqual(im.color(x,y), im2.color(x,-y-1));
    }
  });
});

test("image.flop", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false})
  t.plan(2*im.columns);
  syncAsync(im.flop.bind(im), function(err, im2) {
    for(var x = 0, cols = im2.columns, rows = im2.rows; x < cols; ++x) {
      var y = (Math.random() * rows)|0;
      t.deepEqual(im.color(x,y), im2.color(-x-1,y))
    }
  });
});

test("image.negate", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(2*(2 + 10*3))
  var histogram0 = im.histogram()
  syncAsync(im.negate.bind(im), function(err, im2) {
    t.ok(err == null)
    var histogram = im.histogram()
    t.ok(histogram.length == histogram0.length)
    for(var i = 0; i < 10; ++i) {
      var x = (Math.random() * im2.columns)|0
        , y = (Math.random() * im2.rows)|0
        , c1 = im.color(x, y)
        , c2 = im2.color(x, y)
      t.equal(c1.r, magick.QUANTUM_RANGE - c2.r)
      t.equal(c1.g, magick.QUANTUM_RANGE - c2.g)
      t.equal(c1.b, magick.QUANTUM_RANGE - c2.b)
    }
  })
})

test("image.noise", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(2 + 2*2*2 + 2*2*2)

  t.throws(function(){ im.noise({}) }, new TypeError("noise() needs noise string option"));
  t.throws(function(){ im.noise() }, new TypeError("noise()'s arguments should be strings"));

  var histogram0 = im.histogram();
  [
    im.noise.bind(im, "gaussian"),
    im.noise.bind(im, {noise: "gaussian"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      var histogram = im.histogram()
      t.ok(histogram.length > histogram0.length * 2)
    })
  });
  [
    im.noise.bind(im, "poisson","yellow"),
    im.noise.bind(im, {noise: "poisson", channel: "yellow"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      var histogram = im.histogram()
      t.ok(histogram.length < histogram0.length)
    })
  });
})

test("image.normalize", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(2*2)
  var histogram0 = im.histogram()
  syncAsync(im.normalize.bind(im), function(err, im) {
    t.ok(err == null)
    var histogram = im.histogram()
    t.ok(histogram.length < histogram0.length)
  })
})

test("image.oil", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(2*2*2 + 2*2);
  [
    im.oil.bind(im),
    im.oil.bind(im, 3)
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      var histogram = im.histogram()
      t.ok(histogram.length < 2000)
    })
  });
  syncAsync(im.oil.bind(im, 10), function(err, im) {
    t.ok(err == null)
    var histogram = im.histogram()
    t.ok(histogram.length < 1000)
  });
})

test("image.quantize/histogram", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(3 + 4 + 3 + 2*2*4 + 2*2*(5+16*2) + 3*2*5)
  var histogram = im.histogram();
  t.equal(im.type(), "TrueColor")
  t.type(histogram, Array, "histogram is array");
  t.equal(histogram.length, 5353)
  var pair = histogram[(Math.random()*histogram.length)|0];
  t.type(pair, Array, "histogram item is array");
  t.equal(pair.length, 2);
  t.type(pair[0], Color)
  t.type(pair[1], 'number')
  t.throws(function() { im.quantize(-1) }, new TypeError("number of colors should be > 0"))
  t.throws(function() { im.quantize({colors: -1}) }, new TypeError("number of colors should be > 0"));
  t.equal(im.quantize().type(), "Palette");
  [
    im.quantize.bind(im, 1),
    im.quantize.bind(im, {colors: 1})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.equal(im.type(), "Palette")
      var histogram = im.histogram()
      t.equal(histogram.length, 1)
      t.equal(histogram[0][1], 113*150)
    })
  });
  [
    im.quantize.bind(im, 16, "gray", false),
    im.quantize.bind(im, {colors: 16, colorspace: "gray", dither: false})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.equal(im.type(), "Grayscale")
      im.histogram(function(err, histogram) {
        t.ok(err == null)
        t.equal(histogram.length, 16)
        var total = histogram.reduce(function(a, pair) {
          t.ok(pair[0].r == pair[0].g, "red != green")
          t.ok(pair[0].g == pair[0].b, "green != blue")
          return a + pair[1];
        }, 0)
        t.equal(total, 113*150)
      })
    })
  });
  [
    im.quantize.bind(im, 0),
    im.quantize.bind(im, {}),
    im.quantize.bind(im, {colors: 0})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      im.type(function(err, type) {
        t.ok(err == null)
        t.equal(type, "Palette")
        im.histogram(function(err, histogram) {
          t.ok(err == null)
          t.equal(histogram.length, 256)
          var total = histogram.reduce(function(a, pair) {
            return a + pair[1];
          }, 0)
          t.equal(total, 113*150)
        })
      })
    })
  })
})

test("image.reset", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false});
  t.plan(3 + 2*2 + 2*2 + 2*2 + 2*2);
  t.deepEqual(im.size(), [ 113, 150 ])
  t.deepEqual(im.page,   "113x150");
  im.page = "A4";
  t.deepEqual(im.page,   "595x842");
  syncAsync(im.reset.bind(im), function(err, im) {
    t.strictEqual(im.page               , "113x150")
    t.deepEqual(im.size()               , [113,150])
  });
  syncAsync(im.reset.bind(im, "200x200+10+10"), function(err, im) {
    t.strictEqual(im.page               , "200x200+10+10")
    t.deepEqual(im.size()               , [113,150])
  });
  syncAsync(im.reset.bind(im, 400, 300), function(err, im) {
    t.strictEqual(im.page               , "400x300")
    t.deepEqual(im.size()               , [113,150])
  });
  syncAsync(im.reset.bind(im, 300, 400, 10, -10), function(err, im) {
    t.strictEqual(im.page               , "300x400+10-10")
    t.deepEqual(im.size()               , [113,150])
  });
});

test("image.resize", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false});
  t.plan(6 + 12*2*3 + 20*2*2 + 14*2*2 + 14*2*2 + 6*2*4 + 6*2*4);
  t.deepEqual(im.size(), [ 113, 150 ])
  t.deepEqual(im.page,   "113x150")

  t.throws(function(){ im.resize({size: "foo"}) }, new TypeError("resize() needs proper size or width and height options"));
  t.throws(function(){ im.resize({}) }, new TypeError("resize() needs proper size or width and height options"));
  t.throws(function(){ im.resize("foo") }, new TypeError("resize()'s arguments should be string or numbers"));
  t.throws(function(){ im.resize() }, new TypeError("resize()'s arguments should be string or numbers"));

  var histogram0 = im.histogram()
    , histogramFilter
    , histogramScale
    , histogramSample;
  [
    im.resize.bind(im, 200, 200),
    im.resize.bind(im, 200, 200, "aspectfit"),
    im.resize.bind(im, [200, 200]),
    im.resize.bind(im, [200, 200], "aspectfit"),
    im.resize.bind(im, "200x200"),
    im.resize.bind(im, "200x200", "aspectfit"),
    im.resize.bind(im, {width: 200, height: 200}),
    im.resize.bind(im, {width: 200, height: 200, mode: "aspectfit"}),
    im.resize.bind(im, {size: [200, 200]}),
    im.resize.bind(im, {size: [200, 200], mode: "aspectfit"}),
    im.resize.bind(im, {size: "200x200"}),
    im.resize.bind(im, {size: "200x200", mode: "aspectfit"}),
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "151x200")
      t.deepEqual(im.size()               , [151,200])
      histogramFilter = im.histogram()
      t.ok(histogramFilter.length > histogram0.length)
    });
  });
  [
    im.resize.bind(im, 200, 200, "fill"),
    im.resize.bind(im, 200, 200, "noaspect"),
    im.resize.bind(im, 200, 200, "!"),
    im.resize.bind(im, [200, 200], "fill"),
    im.resize.bind(im, [200, 200], "noaspect"),
    im.resize.bind(im, [200, 200], "!"),
    im.resize.bind(im, "200x200!"),
    im.resize.bind(im, "200x200", "fill"),
    im.resize.bind(im, "200x200", "noaspect"),
    im.resize.bind(im, "200x200", "!"),
    im.resize.bind(im, {width: 200, height: 200, mode: "fill"}),
    im.resize.bind(im, {width: 200, height: 200, mode: "noaspect"}),
    im.resize.bind(im, {width: 200, height: 200, mode: "!"}),
    im.resize.bind(im, {size: [200, 200], mode: "fill"}),
    im.resize.bind(im, {size: [200, 200], mode: "noaspect"}),
    im.resize.bind(im, {size: [200, 200], mode: "!"}),
    im.resize.bind(im, {size: "200x200!"}),
    im.resize.bind(im, {size: "200x200", mode: "fill"}),
    im.resize.bind(im, {size: "200x200", mode: "noaspect"}),
    im.resize.bind(im, {size: "200x200", mode: "!"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "200x200")
      t.deepEqual(im.size()               , [200,200])
    });
  });
  [
    im.resize.bind(im, 200, 200, "larger"),
    im.resize.bind(im, 200, 200, ">"),
    im.resize.bind(im, [200, 200], "larger"),
    im.resize.bind(im, [200, 200], ">"),
    im.resize.bind(im, "200x200>"),
    im.resize.bind(im, "200x200", "larger"),
    im.resize.bind(im, "200x200", ">"),
    im.resize.bind(im, {width: 200, height: 200, mode: "larger"}),
    im.resize.bind(im, {width: 200, height: 200, mode: ">"}),
    im.resize.bind(im, {size: [200, 200], mode: "larger"}),
    im.resize.bind(im, {size: [200, 200], mode: ">"}),
    im.resize.bind(im, {size: "200x200>"}),
    im.resize.bind(im, {size: "200x200", mode: "larger"}),
    im.resize.bind(im, {size: "200x200", mode: ">"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "113x150")
      t.deepEqual(im.size()               , [113,150])
    });
  });
  [
    im.resize.bind(im, 100, 100, "smaller"),
    im.resize.bind(im, 100, 100, "<"),
    im.resize.bind(im, [100, 100], "smaller"),
    im.resize.bind(im, [100, 100], "<"),
    im.resize.bind(im, "100x100<"),
    im.resize.bind(im, "100x100", "smaller"),
    im.resize.bind(im, "100x100", "<"),
    im.resize.bind(im, {width: 100, height: 100, mode: "smaller"}),
    im.resize.bind(im, {width: 100, height: 100, mode: "<"}),
    im.resize.bind(im, {size: [100, 100], mode: "smaller"}),
    im.resize.bind(im, {size: [100, 100], mode: "<"}),
    im.resize.bind(im, {size: "100x100<"}),
    im.resize.bind(im, {size: "100x100", mode: "smaller"}),
    im.resize.bind(im, {size: "100x100", mode: "<"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "113x150")
      t.deepEqual(im.size()               , [113,150])
    });
  });
  [
    im.resize.bind(im, 200, 200, "scale"),
    im.resize.bind(im, [200, 200], "scale"),
    im.resize.bind(im, "200x200", "scale"),
    im.resize.bind(im, {width: 200, height: 200, mode: "scale"}),
    im.resize.bind(im, {size: [200, 200], mode: "scale"}),
    im.resize.bind(im, {size: "200x200", mode: "scale"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "151x200")
      t.deepEqual(im.size()               , [151,200])
      histogramScale = im.histogram()
      t.ok(histogramScale.length < histogramFilter.length)
      t.ok(histogramScale.length > histogram0.length)
    });
  });
  [
    im.resize.bind(im, 200, 200, "sample"),
    im.resize.bind(im, [200, 200], "sample"),
    im.resize.bind(im, "200x200", "sample"),
    im.resize.bind(im, {width: 200, height: 200, mode: "sample"}),
    im.resize.bind(im, {size: [200, 200], mode: "sample"}),
    im.resize.bind(im, {size: "200x200", mode: "sample"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "151x200")
      t.deepEqual(im.size()               , [151,200])
      histogramSample = im.histogram()
      t.ok(histogramSample.length < histogramScale.length)
      t.ok(histogramSample.length == histogram0.length)
    });
  });
});

test("image.rotate", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false, background: "#FF00FF"});
  t.plan(2 + 4*2*2 + 2*4)
  t.deepEqual(im.size(), [ 113, 150 ]);
  t.strictEqual(im.page,   "113x150");
  [
    im.rotate.bind(im, 90),
    im.rotate.bind(im, -90),
    im.rotate.bind(im, 270),
    im.rotate.bind(im, -270),
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.deepEqual(im.size(), [ 150, 113 ]);
      t.strictEqual(im.page,   "150x113");
    });
  });
  syncAsync(im.rotate.bind(im, 45), function(err, im) {
    t.deepEqual(im.size(), [ 187, 188 ]);
    t.strictEqual(im.page,   "187x188-37-19");
    t.deepEqual(im.color(0,0).rgba(), [ 1, 0, 1, 0 ]);
    t.deepEqual(im.color(187,188).rgba(), [ 1, 0, 1, 0 ]);
  });
});

test("image.sharpen", function (t) {
  var im = new Image({src: blobT160, autoClose: false}).blur(1).copy(true)
  t.plan(8 + 2*2*5 + 4*2*5 + 2*2*5)
  t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
  t.equal(im.color(9,9).alpha(100)|0  , 90)
  t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
  t.equal(im.color(11,11).alpha(100)|0, 12)

  t.throws(function(){ im.sharpen({sigma:NaN}) }, new TypeError("sharpen() needs sigma number option"));
  t.throws(function(){ im.sharpen({}) }, new TypeError("sharpen() needs sigma number option"));
  t.throws(function(){ im.sharpen(void(0)) }, new TypeError("sharpen()'s sigma is not a number"));
  t.throws(function(){ im.sharpen() }, new TypeError("sharpen()'s arguments should be number(s)[, string]"));

  [
    im.sharpen.bind(im, 1),
    im.sharpen.bind(im, {sigma: 1})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 93)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.ok(im.color(11,11).alpha(100) < 2)
    })
  });
  [
    im.sharpen.bind(im, 1, 0.5),
    im.sharpen.bind(im, {sigma: 1, radius: 0.5}),
    im.sharpen.bind(im, 1, 0.5, "matte"),
    im.sharpen.bind(im, {sigma: 1, radius: 0.5, channel: "matte"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 92)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 5)
    })
  });
  [
    im.sharpen.bind(im, 1, 0.5, "yellow"),
    im.sharpen.bind(im, {sigma: 1, radius: 0.5, channel: "yellow"})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.ok(err == null)
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 90)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 12)
    })
  });
})

test("image.strip", function (t) {
  var im = new Image({src: blobImage, autoCopy: true, autoClose: false});
  t.plan(4 + 2*5);
  t.deepEqual(im.size(), [ 113, 150 ]);
  t.strictEqual(im.page,   "113x150");
  t.type(im.comment(), 'string');
  t.ok(im.comment().length > 0, "comment is set");
  syncAsync(im.strip.bind(im), function(err, im) {
    t.deepEqual(im.size(), [ 113, 150 ]);
    t.strictEqual(im.page,   "113x150");
    im.comment(function(err, comment){
      t.ok(err == null);
      t.type(comment, 'string');
      t.equal(comment.length, 0, "comment is empty");
    });
  });
});

test("image.trim", function (t) {
  var im = new Image({src: blobTrim, autoCopy: true, autoClose: false});
  t.plan(2 + 3*2*2 + 2*2*2 + 2*2*2 + 4*2*2);
  t.deepEqual(im.size(), [ 16, 16 ])
  t.equal(im.fuzz      ,   0);
  [
    im.trim.bind(im),
    im.trim.bind(im, 0.05),
    im.trim.bind(im, '5%')
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "16x16+1+1")
      t.deepEqual(im.size()               , [ 14, 14 ])
    });
  });
  [
    im.trim.bind(im, 0.1),
    im.trim.bind(im, '10%'),
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "16x16+2+2")
      t.deepEqual(im.size()               , [ 12, 12 ])
    });
  });
  [
    im.trim.bind(im, 0.5),
    im.trim.bind(im, '50%')
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "16x16+3+3")
      t.deepEqual(im.size()               , [ 10, 10 ])
    });
  });
  [
    im.trim.bind(im, 0.75),
    im.trim.bind(im, '75%'),
    im.trim.bind(im, 0.99),
    im.trim.bind(im, '99%')
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "16x16+4+4")
      t.deepEqual(im.size()               , [  8,  8 ])
    });
  });
});
