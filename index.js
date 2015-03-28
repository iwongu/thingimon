var SerialPort = require("serialport").SerialPort;
var RoomStatus = require('./store').RoomStatus;

var dataRequest = [0xDE];
var body = [0xDE, 0xAD, 0xBE, 0xAF, 0x00, 0x00, 0x00, 0x00];
var n = 0;
var sp = new SerialPort('/dev/ttyUSB0');
sp.on('data', function(data) {
  for (var i = 0; i < data.length; i++) {
    if (n < 4) {
      if (data[i] == body[n]) {
	n++;
      } else {
	n = 0;
      }
    } else {
      body[n] = data[i];
      n++;
    }
    if (n == 8) {
      var temp = (body[4] * 0xFF + body[5]) / 100;
      var humi = (body[6] * 0xFF + body[7]) / 100;
      console.log(new Date().toISOString());
      console.log('temp: ' + temp);
      console.log('humi: ' + humi);
      RoomStatus.store(temp, humi);
      n = 0;
    }
  }
});

setInterval(function() {
  sp.write(dataRequest);
}, 10000);
