# Build & Test in a Linux Container on Windows

## Bootstrap

Ensure both Containers and Hyper-V are enabled in Windows.

- Open the Start Menu and starting typing "Turn Windows features on or off" to select it
- Ensure both 'Containers' and all of 'Hyper-V' are enabled.
    - Hyper-V requires that virtualization is enabled in your BIOS. Every BIOS is different, so use Google or ask around if you encounter related errors.
        - HP Z4 Desktop
            - Reboot and hold ESC to show startup menu
            - Select 'BIOS Setup (F10)'
            - Use the Right Arrow to select the 'Security' tab
            - Select 'System Security'
            - Enable both options ('Virtualization Technology (VTx)' and 'Virtualization Technology for Directed I/O (VTd)')
            - Use the Left Arrow to go back to the 'Main' tab
            - Select 'Save Changes and Exit' and confirm

Install [Docker CE](https://hub.docker.com/editions/community/docker-ce-desktop-windows) ([direct](https://download.docker.com/win/stable/Docker%20for%20Windows%20Installer.exe)). Keep all defaults.

Customize Docker settings (in taskbar notification area, right-click the Docker icon and select 'Settings').
- Shared Drives: You must share the drive(s) that contain your source code **and** your home drive (typically C).
- Advanced: Building is resource-intesive; we recommend at least 4 vCPUs and 4 GB of RAM.

Update VS Code to at least 1.35.0 (May 2019).

Install/update VS Code extension "Remote - Containers".

## How to use the container

You must first bootstrap and pull an imodel02 tree on your host computer. See [Get and Build Native imodel02 Code](/iModel-Technology/iModel02/Get-and-Build-Native-imodel02-Code).

Close all current instances of VS Code, and launch it from an imodel02 shell (e.g. so that %SrcRoot% is defined in the environment), e.g. `code %SrcRoot%imodel02`. Then, click the green area in the status bar, and select VS Code command 'Remote-Containers: Reopen Folder in Container'. Once connected, the green section of the status bar should say something like "Dev Container: imodel02-linux".

Use VS Code's Terminal to perform normal build commands such as `bb build` and `bb rebuild ...`. The container builds strategy 'iModelJsNodeAddon.Dev' by default; you can change the default by running `export BuildStrategy=...`.

When you are done, click the green area in the status bar, and select 'Remote-Containers: Reopen Folder Locally' to switch back to a local view on your host.

### Debug native tests

It is trivial to launch and debug the native unit tests. Launch profiles have been added for all of them, with the prefix 'LNX Native ...' (vs. 'LNX Native JS-...'). Just set breakpoints in your CPP files as you normally would.

### Debug native code in JS tests

This requires more setup and has some caveats. There are some launch profiles to get you started, but they are not exhaustive; they start with the prefix 'LNX Native JS-...'.

- Start by building the native code: `bb build`
- Clone/pull imodeljs into /workspace/imodeljs: `git clone https://bentleycs@dev.azure.com/bentleycs/iModelTechnologies/_git/imodeljs /workspace/imodeljs`
    - You can optionally add the '/workspace/imodeljs' directory to the VS Code workspace GUI to browse its files.
- Configure npm (only needed once): `npm config set @bentley:registry https://npm.bentley.com/npm/npm/`
- Follow the normal workflow to configure your imodeljs tree:
    - `cd /workspace/imodeljs`
    - Switch to the 'imodel02' branch: `git checkout imodel02`
    - `rush install`
    - `../src/imodel02/iModelJsNodeAddon/installnativeplatformLinux.sh .`
    - `rush rebuild`

Debugging the addon while running JS tests has a hurdle: our test framework runs a wrapper script in node, which then spawns a second node process to actually run the tests. While it is trivial to attach and debug this "parent" node process, VS Code seems unable to follow the fork to the child. As such, you must run the tests directly, not through the wrapper. This can be done with the following workflow.

*The JS Backend test suite has already been configured in launch.json for default arguments. You can use this workflow if you want to customize the arguments, or add another JS test suite.*

- Patch 'tools/build/scripts/test.js' to add `console.log(args);` just before the `return spawn(...` call towards the bottom
- Edit 'launch.json':
    - Find the JS test suite you want to run (or copy an existing)
    - Uncomment the top set of "args" and adjust as needed; these can be copied from imodeljs's launch.json file
    - Comment the bottom set of "args"
- Launch the test suite (this time through the wrapper, printing the child process arguments); you can terminate it once it's printed the arguments
- Copy the printed arguments
- Edit 'launch.json':
    - Comment the top set of "args"
    - Uncomment the bottom set of "args"
    - Replace the bottom args with the printed arguments (and find/replace ' with ")
- Launch again

Set breakpoints in your CPP files as you normally would.

### Debug JS *and* native code at the same time in JS tests

I have tested the following workflow, where the JS debugger is the primary, and the native debugger is attached later. This requires that you can set a JS breakpoint of interest first (e.g. at the top of your JS test), and then enable native debugging.

- Set a JS breakpoint
- Use a "LNX Script..." launch profile to start a JS test suite in script debugging mode
- When the breakpoint is hit, select the "LNX Native Node Attach" launch profile, and **click the green arrow** to start (do **not** press F5)
    - You'll be given a list of all active 'node' processes to choose from
    - This is a process of elimination. You do **not** want:
        - Anything with args in "/root/.vscode-server/...""
        - The node that's running ".../test.js"
    - This should leave a single node process, typically starting with args like "node --inspect=9229..."
- Set a native breakpoint
- Resume debugging

### Notes

The container is configured to mount (i.e. share) %SrcRoot% into /workspace/src, but output will be generated inside the container at /workspace/out.

VS Code injects Git authorization tokens into the container, so you can fetch/pull/push as you would otherwise do on the host. VS Code also mounts your .gitconfig file from the host in the container, so your configured name/email/aliases etc. are available in the container. You cannot perform any remote operations with Mercurial repositories inside the container.

The first time you "open" a container for a project, it will be built and persisted on your host computer. After you've built the first time, the container should retain the source and output until you either tell VS Code to rebuild the container, or you tell docker to delete it.

### Terminal

- Use VS Code's Terminal window to interact directly with the container.
- The file system is **case-sensitive** (tab completion is configured to be *not* case-sensitive)
- The container is configured with zsh + oh-my-zsh + fzf for some additional niceties (e.g. case-insensitivity, and fuzzy history search).

### Common Issues

- `bb build` fails with "error: redefinition of ..." and the diagnostic indicates the same header with different relative paths
    - Cause: Unknown
    - Workaround: Run `bb build` again; it seems random and goes away when retried
- `bb build` fails with "clang++: error: unable to execute command: Killed"
    - Cause: Not enough memory
    - Solution: Allocate more memory to the Docker VM.
    - Workaround: You can sometimes just `bb build` again to get past this, depending exactly what is building in parallel in the container
