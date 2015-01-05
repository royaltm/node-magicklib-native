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

var util = require('util');

util.inherits(MockWritable, stream.Writable);

function MockWritable(test, buffer) {
  this.test = test;
  this.buffer = buffer;
  stream.Writable.call(this, {});
}

MockWritable.prototype._write = function(chunk, _, callback) {
  var t = this.test;
  t.strictEqual(chunk.toString('base64'), this.buffer.toString('base64'), "buffers equal")
  callback()
};

test("should write to stream", function(t) {
  var im = new Image(blobImage)
    , blob = im.write()

  t.plan(1 + 5 + 1)

  t.type(im.pipe(new MockWritable(t, blob)), stream.Writable)

  im.end(function(err, im2) {
    t.ok(err == null)
    t.type(im2, Image)
    t.strictEqual(im, im2)
    t.equal(im.columns, 113)
    t.equal(im.rows, 150)
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
