const { spawn } = require("child_process");
const os = require("os");

spawn("yarn", ["run", "http_server"], {
  stdio: "inherit",
  shell: os.platform() === "win32"
});
