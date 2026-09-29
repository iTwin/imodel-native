const { spawn } = require("child_process");
const os = require("os");

if (os.platform() === "win32") {
  spawn("yarn", ["run", "dhcp_server"], { stdio: "inherit", shell: true });
} else {
  spawn("sudo", ["yarn", "run", "dhcp_server"], { stdio: "inherit" });
}
