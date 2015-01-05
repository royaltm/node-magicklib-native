var magick = module.exports = require(__dirname + '/build/Release/magicklib.node')
  , QUANTUM_RANGE = magick.QUANTUM_RANGE
  , Color = magick.Color
  , Image = magick.Image;

  Image.prototype.prop = Image.prototype.properties;

/* Color utils  */

  Color.toQuantum = function(value) {
    return (value * QUANTUM_RANGE)|0;
  };

  Color.fromQuantum = function(quantum) {
    return (quantum / QUANTUM_RANGE);
  };

  Color.prototype.red = function(scale) {
    return scale==null ? this.r/QUANTUM_RANGE : this.r/QUANTUM_RANGE * scale;
  };

  Color.prototype.green = function(scale) {
    return scale==null ? this.g/QUANTUM_RANGE : this.g/QUANTUM_RANGE * scale;
  };

  Color.prototype.blue = function(scale) {
    return scale==null ? this.b/QUANTUM_RANGE : this.b/QUANTUM_RANGE * scale;
  };

  Color.prototype.alpha = function(scale) {
    return scale==null ? this.a/QUANTUM_RANGE : this.a/QUANTUM_RANGE * scale;
  };

  Color.RGB = function(r, g, b, a) {
    return a == null ?
      new Color(r*QUANTUM_RANGE, g*QUANTUM_RANGE, b*QUANTUM_RANGE) :
      new Color(r*QUANTUM_RANGE, g*QUANTUM_RANGE, b*QUANTUM_RANGE, a*QUANTUM_RANGE);
  };


  /* STREAMS */

  var stream = require('stream');
  var util = require('util');

  var Writable = stream.Writable;

  function Writer(image, options) {
    this._image = image._hold();
    this._bufs = [];

    Writable.call(this, options);

    this.on('finish', function() {
      this._image._wrap(Buffer.concat(this._bufs));
      this._bufs = this._image = null;
    });
  }

  util.inherits(Writer, Writable);

  Writer.prototype._write = function(chunk, enc, done) {
    this._bufs.push(chunk);
    done();
  };

  /**
   * Writeable image stream
   *
   * image.createWriteStream() -> stream
   *
   * stream: Writeable stream instance
   *
   * note: can't create many simultaneous writable streams from one image instance
   **/
  Image.prototype.createWriteStream = function(_options) {
    return new Writer(this, _options);
  }

  // http://codewinds.com/blog/2013-08-20-nodejs-transform-streams.html
  var Transform = stream.Transform;

  function Convert(image, options) {

    var self = this;

    this._image = image._hold().write(function(err, data) {
      var done = self._done;
      self._image = null;
      if (err) return done(err);
      self.push(data);
      done();
    });
    this._bufs = [];

    Transform.call(this, options);
  }

  util.inherits(Convert, Transform);

  Convert.prototype._transform = function (chunk, enc, done) {
    this._bufs.push(chunk);
    done();
  };

  Convert.prototype._flush = function(done) {
    this._done = done;
    this._image._wrap(Buffer.concat(this._bufs))
  };

  /**
   * Convert image stream
   *
   * image.createConvertStream() -> convert
   *
   * convert: Transform stream instance
   *
   * note: can't create many simultaneous convert streams from one image instance
   **/
  Image.prototype.createConvertStream = function(_options) {
    return new Convert(this, _options);
  };


  var Readable = stream.Readable;

  function Reader(image, options) {
    this._image = image;

    Readable.call(this, options);
  }

  util.inherits(Reader, Readable);

  Reader.prototype._read = function() {
    var image = this._image
      , self = this;
    if (image) {
      image.write(function(err, data) {
        if (err) return self.emit('error', err);
        self.push(data);
      });
      this._image = image = null;
    } else {
      self.push(null);
    }
  };

  /**
   * Read stream from image
   *
   * image.createReadStream() -> stream
   *
   * stream: Readable stream instance
   **/
  Image.prototype.createReadStream = function(_options) {
    return new Reader(this, _options);
  };

  Image.prototype.pipe = function(writer) {
    return new Reader(this).pipe(writer);
  };
