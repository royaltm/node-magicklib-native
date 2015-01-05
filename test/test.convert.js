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
  t.plan(2 + 2*2*4 + 2*2*4 + 2*2*6 + 2*2*4)
  t.deepEqual(im.color(9,9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(11,11).rgba(), [ 0, 0, 0, 0 ]);
  [
    im.blur.bind(im, 1),
    im.blur.bind(im, {sigma: 1})
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
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
      t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
      t.equal(im.color(9,9).alpha(100)|0  , 91)
      t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
      t.equal(im.color(11,11).alpha(100)|0, 10)
    })
  });
})

test("image.crop", function (t) {
  var im = new Image({src: blobT160, autoCopy: true, autoClose: false})
  t.plan(8 + 3*2*9 + 3*2*9)
  t.deepEqual(im.size()               , [ 160, 160 ])
  t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(10,  10).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(60,  60).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(69,  69).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(80,  80).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(150,150).rgba(), [ 0, 0, 0, 1 ]);
  [
    im.crop.bind(im, 160, 160, 10, 10),
    im.crop.bind(im, [160, 160, 10, 10]),
    im.crop.bind(im, "160x160+10+10")
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
    im.crop.bind(im, "160x160-10-10")
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
  t.plan(9 + 7*2*11 + 6*2*10 + 6*2*11);
  t.deepEqual(im.size()               , [ 16, 16 ])
  t.deepEqual(im.color(0,    0).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(1,    1).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(6,    6).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(7,    7).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(8,    8).rgba(), [ 1, 1, 1, 0 ])
  t.deepEqual(im.color(9,    9).rgba(), [ 0, 0, 0, 1 ])
  t.deepEqual(im.color(14,  14).rgba(), [ 0, 0, 0, 0 ])
  t.deepEqual(im.color(15,  15).rgba(), [ 0, 0, 0, 1 ]);
  [
    im.extent.bind(im, 14, 14),
    im.extent.bind(im, 14, 14, "centeR"),
    im.extent.bind(im, 14, 14, 1, 1),
    im.extent.bind(im, [14, 14, 1, 1]),
    im.extent.bind(im, [14, 14], "Center"),
    im.extent.bind(im, "14x14+1+1"),
    im.extent.bind(im, "14x14", "center")
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
    im.extent.bind(im, "14x14", "nortHWesT")
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
    im.extent.bind(im, "14x14", "eASt")
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

test("image.noise", function (t) {
  var im = new Image({src: blobImage, magick: "PNG", autoCopy: true, autoClose: false})
  t.plan(2*2*2 + 2*2*2)
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
  t.throws(function() { im.quantize(-1) }, "number of colors should be > 0")
  t.throws(function() { im.quantize({colors: -1}) }, "number of colors should be > 0");
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
  t.plan(2 + 6*2*2 + 10*2*2 + 7*2*2 + 7*2*2);
  t.deepEqual(im.size(), [ 113, 150 ])
  t.deepEqual(im.page,   "113x150");
  [
    im.resize.bind(im, 200, 200),
    im.resize.bind(im, 200, 200, "aspectfit"),
    im.resize.bind(im, [200, 200]),
    im.resize.bind(im, [200, 200], "aspectfit"),
    im.resize.bind(im, "200x200"),
    im.resize.bind(im, "200x200", "aspectfit")
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "151x200")
      t.deepEqual(im.size()               , [151,200])
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
    im.resize.bind(im, "200x200", "!")
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
    im.resize.bind(im, "200x200", ">")
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
    im.resize.bind(im, "100x100", "<")
  ].forEach(function(method) {
    syncAsync(method, function(err, im) {
      t.strictEqual(im.page               , "113x150")
      t.deepEqual(im.size()               , [113,150])
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
    syncAsync(im.rotate.bind(im, 90), function(err, im) {
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
  t.plan(4 + 2*5)
  t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
  t.equal(im.color(9,9).alpha(100)|0  , 90)
  t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
  t.equal(im.color(11,11).alpha(100)|0, 12)
  syncAsync(im.sharpen.bind(im, 1), function(err, im) {
    t.ok(err == null)
    t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
    t.equal(im.color(9,9).alpha(100)|0  , 93)
    t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
    t.ok(im.color(11,11).alpha(100) < 2)
  });
  syncAsync(im.sharpen.bind(im, 1, 0.5), function(err, im) {
    t.ok(err == null)
    t.deepEqual(im.color(9,9).rgb()         , [ 0, 0, 0 ])
    t.equal(im.color(9,9).alpha(100)|0  , 92)
    t.deepEqual(im.color(11,11).rgb()       , [ 0, 0, 0 ])
    t.equal(im.color(11,11).alpha(100)|0, 5)
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
