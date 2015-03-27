var Parse = require('parse').Parse;
var parseArgs = require('minimist');

var args = parseArgs(process.argv.slice(2));

Parse.initialize(args.parseappid, args.parsejskey);

var RoomStatus = Parse.Object.extend('RoomStatus', {
  initialize: function(attrs, options) {
  },
}, {
  store: function(temperature, humidity) {
    var status = new RoomStatus();
    status.set('temperature', temperature);
    status.set('humidity', humidity);
    status.save(null, {
      success: function(status) {
      },
      error: function(status, error) {
        console.log('error: ' + error.message);
      }
    });
  }
});

module.exports.RoomStatus = RoomStatus;
