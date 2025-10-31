var dhcp = require("dhcp");

var s = dhcp.createServer({
  range: [
    "192.168.3.1",
    "192.168.3.99"
  ],
  forceOptions: ["hostname", "wpad"],
  hostname: "kacknup",
  wpad: () => "http://wpad.com/wpad.dat",
  server: "192.168.3.1",
});

s.on("message", (data) => {
  console.log(data);
});

s.on("error", (err, data) => {
  console.log(err, data);
});

s.on("listening", (sock) => {
  var address = sock.address();
  console.info(`Server Listening: ${address.address}:${address.port}`);
});

s.on("close", () => {
  console.log("close");
});

s.listen();