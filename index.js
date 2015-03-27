var serialport = require("serialport");
var SerialPort = serialport.SerialPort;

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
	    console.log('temp: ' + (body[4] * 0xFF + body[5]) / 100);
	    console.log('humi: ' + (body[6] * 0xFF + body[7]) / 100);
	    n = 0;
	}
    }
});