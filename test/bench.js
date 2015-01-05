require('colors')
var ben       = require('ben')
,   im        = require('imagemagick')
,   im_native = require('imagemagick-native')
,   magick    = require('..')
,   Image     = magick.Image
,   async     = require('async')
,   assert    = require('assert')
,   debug     = 0
,   fs        = require('fs')
,   ntimes    = 20
,   ptimes    = 10
;
// console.log("memory", mb(magick.limit("memory", 10000000)))
console.log("thread: ", magick.limit("thread"))
console.log("memory: ", mb(magick.limit("memory")))
console.log("disk:   ", mb(magick.limit("disk")))

var files  = process.argv.slice(2)
  , bodies = [];
for(var i=0; i < ptimes; ++i)
  bodies = bodies.concat(files.map(function(name) { return fs.readFileSync(name) }));

function saveFile(name, next, err, blob) {
  assert( blob.length > 0 );
  fs.writeFile( "./out." + name + ".jpg", blob, 'binary', next );
}

function resize_im(callback) {
  async.map(bodies, function(body, next) {
    async.parallel([
      function(next) {
        im.resize({
          width: 100,
          height: 100,
          format: 'jpg',
          filter: 'Lagrange',
          quality: 0.8,
          sharpening: 0,
          strip: true,
          srcData: body
        }, async.apply(saveFile, "im-fork-1", next));
      },
      function(next) {
        im.resize({
          width: 200,
          height: 200,
          format: 'jpg',
          filter: 'Lagrange',
          quality: 0.8,
          sharpening: 0,
          strip: true,
          srcData: body
        }, async.apply(saveFile, "im-fork-2", next));
      },
      function(next) {
        im.resize({
          width: 300,
          height: 300,
          format: 'jpg',
          filter: 'Lagrange',
          quality: 0.8,
          sharpening: 0,
          strip: true,
          srcData: body
        }, async.apply(saveFile, "im-fork-3", next));
      },
    ], next);
  }, callback);
}

function resize_im_native(callback) {
  async.map(bodies, function(body, next) {
    async.parallel([
      function(next) {
        im_native.convert({
          srcData: body,
          width: 100,
          height: 100,
          resizeStyle: 'aspectfit',
          quality: 80,
          format: 'JPEG',
          filter: 'Lagrange',
          strip: true,
          debug: debug
        }, async.apply(saveFile, "im-native-1", next));
      },
      function(next) {
        im_native.convert({
          srcData: body,
          width: 200,
          height: 200,
          resizeStyle: 'aspectfit',
          quality: 80,
          format: 'JPEG',
          filter: 'Lagrange',
          strip: true,
          debug: debug
        }, async.apply(saveFile, "im-native-2", next));
      },
      function(next) {
        im_native.convert({
          srcData: body,
          width: 300,
          height: 300,
          resizeStyle: 'aspectfit',
          quality: 80,
          format: 'JPEG',
          filter: 'Lagrange',
          strip: true,
          debug: debug
        }, async.apply(saveFile, "im-native-3", next));
      }
    ], next);
  }, callback);
}
/* api-optimized */
function resize_mi_lib_opt(callback) {
  async.map(bodies, function(body, next) {
    var togo = 3;
    function written() { if (!--togo) next(); }
    var im = new Image({batch: true})
      .read(body)
      .format('JPEG')
      .filter('Lagrange')
      .strip()
      .quality(80)
      .properties({autoCopy: true})
      .resize(100,100);
      im.pipe(fs.createWriteStream("./out.mi-lib-opt-1.jpg")).on('finish', written);
      // .write(written)
      im.restore()
      .resize(200,200)
      .pipe(fs.createWriteStream("./out.mi-lib-opt-2.jpg")).on('finish', written);
      // .write(written)
      im.restore()
      .resize(300,300)
      .pipe(fs.createWriteStream("./out.mi-lib-opt-3.jpg")).on('finish', written);
      // .write(written)
  }, callback);
}
/* naive test */
function resize_mi_lib_naiv(callback) {
  async.map(bodies, function(body, next) {
    async.parallel([
      function(next) {
        new Image({batch: true})
        .read(body)
        .format('JPEG')
        .filter('Lagrange')
        .resize(100,100)
        .strip()
        .quality(80)
        .write(async.apply(saveFile, "mi-lib-naiv-1", next))
        //.write(next)
      },
      function(next) {
        new Image({batch: true})
        .read(body)
        .format('JPEG')
        .filter('Lagrange')
        .resize(200,200)
        .strip()
        .quality(80)
        .write(async.apply(saveFile, "mi-lib-naiv-2", next))
        //.write(next)
      },
      function(next) {
        new Image({batch: true})
        .read(body)
        .format('JPEG')
        .filter('Lagrange')
        .resize(300,300)
        .strip()
        .quality(80)
        .write(async.apply(saveFile, "mi-lib-naiv-3", next))
        //.write(next)
      }
    ], next);
  }, callback);
}

async.waterfall([
  function (callback) {
    console.log( "before: ", mem(process.memoryUsage()).cyan );
    callback();
  },
  function (callback) {
    // callback();
    ben.async( ntimes, resize_im, function (ms) {
      console.log( "imagemagick: " + (ms).toFixed(2).yellow + " ms per iteration" );
      callback();
    });
  },
  function (callback) {
    // callback();
    console.log( "after:  ", mem(process.memoryUsage()).cyan );
    'gc' in global && gc();
    console.log( "before: ", mem(process.memoryUsage()).cyan );
    ben.async( ntimes, resize_im_native, function (ms) {
      console.log( "imagemagick-native: " + (ms).toFixed(2).yellow + " ms per iteration" );
      callback();
    });
  },
  function (callback) {
    // callback();
    console.log( "after:  ", mem(process.memoryUsage()).cyan );
    'gc' in global && gc();
    console.log( "before: ", mem(process.memoryUsage()).cyan );
    ben.async( ntimes, resize_mi_lib_naiv, function (ms) {
      console.log( "magicklib-naive: " + (ms).toFixed(2).yellow + " ms per iteration" );
      callback();
    });
  },
  function (callback) {
    // callback();
    console.log( "after:  ", mem(process.memoryUsage()).cyan );
    'gc' in global && gc();
    console.log( "before: ", mem(process.memoryUsage()).cyan );
    ben.async( ntimes, resize_mi_lib_opt, function (ms) {
      console.log( "magicklib-opt: " + (ms).toFixed(2).yellow + " ms per iteration" );
      callback();
    });
  },
], function(err) {
  console.log( "after:  ", mem(process.memoryUsage()).cyan );
  'gc' in global && gc();
  if ( err ) {
    console.log("err: ", err);
  } else {
    setImmediate( function() {
      'gc' in global && gc();
      setTimeout(function() {
        'gc' in global && gc();
        console.log( "done:   ", mem(process.memoryUsage()).cyan );
      }, 2000);
    });
  }
});

function mem(info) {
  return "rss: " + mb(info.rss) + " used: " + mb(info.heapUsed) + " total: " + mb(info.heapTotal);
}

function mb(value) {
  return (value / 1000000).toFixed(1) + 'MB';
}

/*
resize:     16.09ms per iteration
resize_native: 0.89ms per iteration
*/
