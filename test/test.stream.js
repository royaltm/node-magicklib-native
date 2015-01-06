var test = require("tap").test
  , magick = require("..")
  , fs = require("fs")
  , stream = require("stream")
  , Image = magick.Image
  , blobImage = fs.readFileSync("test.image.jpg");

test("should read from stream", function(t) {
  var im = new Image()
    , readstream = fs.createReadStream("test.image.jpg")
    , writestream = im.createWriteStream()

  t.type(writestream, stream.Writable, "is writable")

  t.strictEqual(readstream.pipe(writestream), writestream)

  im.end(function(err, im2) {
    t.ok(err == null)
    t.type(im2, Image)
    t.strictEqual(im, im2)
    t.equal(im.columns, 113)
    t.equal(im.rows, 150)
    t.end()
  })

})

test("should error reading from stream", function(t) {
  var im = new Image()
    , readstream = fs.createReadStream("test.stream.js")
    , writestream = im.createWriteStream()

  t.plan(7 + 6 + 1)
  t.type(writestream, stream.Writable, "is writable")

  t.strictEqual(im.busy, true, "is busy")
  t.equal(im.batchSize, 0)

  writestream.on('error', function(err) {
    throw new Error("this should never happen")
  });

  t.strictEqual(readstream.pipe(writestream), writestream)

  t.equal(im.batchSize, 0)
  t.strictEqual(im.busy, true, "is busy")
  im.resize(100, 100)
  t.equal(im.batchSize, 1)
  im.end(function(err, im2) {
    t.equal(im.batchSize, 0)
    t.type(err, Error, "should be error")
    t.like(err.toString(), (/Error: .*no decode delegate/))
    t.type(im2, 'undefined')
    t.equal(im.columns, 0)
    t.equal(im.rows, 0)
  })
  t.equal(im.batchSize, 1)
})


var util = require('util');

util.inherits(MockWritable, stream.Writable);

function MockWritable(test, buffer) {
  this.test = test;
  this.buffer = buffer;
  stream.Writable.call(this, {});
}

MockWritable.prototype._write = function(chunk, _, callback) {
  var t = this.test;
  t.deepEqual(chunk, this.buffer, "buffers equal")
  callback()
};

test("should write to stream", function(t) {
  var im = new Image(blobImage)
    , blob = im.write()

  t.plan(2 + 5 + 1)
  t.type(im.pipe(new MockWritable(t, blob)), stream.Writable)
  t.strictEqual(im.busy, true, "is busy")
  im.end(function(err, im2) {
    t.ok(err == null)
    t.type(im2, Image)
    t.strictEqual(im, im2)
    t.equal(im.columns, 113)
    t.equal(im.rows, 150)
  })

})

test("should error writing to stream", function(t) {
  var im = new Image()

  t.plan(2 + 3 + 5)
  var reader = im.createReadStream().on('error', function(err) {
    t.equal(im.batchSize, 0)
    t.type(err, Error, "should be error")
  });
  t.type(reader, stream.Readable)
  t.type(reader.pipe(new MockWritable(t)), stream.Writable)
  t.strictEqual(im.busy, true, "is busy")
  im.end(function(err, im2) {
    t.ok(err == null)
    t.type(im2, Image)
    t.strictEqual(im, im2)
    t.equal(im.columns, 0)
    t.equal(im.rows, 0)
  })

})

test("should stream convert", function(t) {
  var im1 = new Image().begin(true).format("PNG").blur(1)
    , im2 = new Image().begin(true).format("JPEG").quality(80).sharpen(2)
    , writestream = new MockWritable(t, new Image(
                          new Image(blobImage).format("PNG").blur(1).write()
                      ).
                      format("JPEG").quality(80).sharpen(2).write())
    , readstream = fs.createReadStream("test.image.jpg")
    , convertstream1 = im1.createConvertStream()
    , convertstream2 = im2.createConvertStream()

  t.plan(3 + 5 + 1)

  t.type(convertstream1, stream.Transform, "is transform")
  t.type(convertstream2, stream.Transform, "is transform")

  t.strictEqual(readstream.pipe(convertstream1).pipe(convertstream2).pipe(writestream),
      writestream)

  im2.end(function(err, im) {
    t.ok(err == null)
    t.type(im, Image)
    t.strictEqual(im2, im)
    t.equal(im.columns, 113)
    t.equal(im.rows, 150)
  })
})

test("should converter error reading stream", function(t) {
  var im = new Image().begin(true).format("PNG").blur(1).quantize(16)
    , writestream = new MockWritable(t)
    , readstream = fs.createReadStream("test.stream.js")
    , convertstream = im.createConvertStream()

  t.plan(6 + 3 + 5 + 1)

  t.type(convertstream, stream.Transform, "is transform")

  t.strictEqual(im.busy, true, "is busy")
  t.equal(im.batchSize, 4)

  convertstream.on('error', function(err) {
    t.equal(im.batchSize, 0)
    t.type(err, Error, "should be error")
    t.like(err.toString(), (/Error: .*no decode delegate for this image format/))
  });

  t.strictEqual(readstream.pipe(convertstream).pipe(writestream),
      writestream)

  t.equal(im.batchSize, 4)
  t.strictEqual(im.busy, true, "is busy")

  im.end(function(err, im2) {
    t.equal(im.batchSize, 0)
    t.ok(err == null)
    t.strictEqual(im2, im)
    t.equal(im.columns, 0)
    t.equal(im.rows, 0)
  })
  t.equal(im.batchSize, 4)
})

test("should converter error writing stream", function(t) {
  var im = new Image().begin(true).copy(true).size(0,0)
    , writestream = new MockWritable(t)
    , readstream = fs.createReadStream("test.image.jpg")
    , convertstream = im.createConvertStream()

  t.plan(6 + 3 + 7 + 1)

  t.type(convertstream, stream.Transform, "is transform")

  t.strictEqual(im.busy, true, "is busy")
  t.equal(im.batchSize, 3)

  convertstream.on('error', function(err) {
    t.equal(im.batchSize, 0)
    t.type(err, Error, "should be error")
    t.like(err.toString(), (/Error: .*no pixels defined in cache/))
  });

  t.strictEqual(readstream.pipe(convertstream).pipe(writestream),
      writestream)

  t.equal(im.batchSize, 3)
  t.strictEqual(im.busy, true, "is busy")

  im.end(function(err, im2) {
    t.equal(im.batchSize, 0)
    t.ok(err == null)
    t.notStrictEqual(im2, im)
    t.equal(im.columns, 113)
    t.equal(im.rows, 150)
    t.equal(im2.columns, 0)
    t.equal(im2.rows, 0)
  })
  t.equal(im.batchSize, 3)
})
