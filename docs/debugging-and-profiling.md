# Debugging and profiling

Most commands here are for Linux. On Windows, Visual Studio and the `cppvsdbg` entries in `.vscode/launch.json` cover the debugging parts.

## Debug builds

Set `DEBUG=1` and `LLVM_DEBUG=1` instead of `NDEBUG` and `PRG` (see [README](../README.md#environment)). Without `LLVM_DEBUG` a debug build has no symbols.

## Native tests

`.vscode/launch.json` has an entry per test executable and platform, grouped as `0_Windows`, `1_Linux` and `2_MacOS`. The Linux entries start the executable from `$OutRoot/LinuxX64/static/ProductCopy/`, which only a full build fills. After a focused product build (`bb -p ECDb-Gtest ...`), point `program` at `$OutRoot/LinuxX64/Product/<Product>/` instead.

Useful arguments:

- `--gtest_filter=Fixture.Test` runs a single test. It replaces the ignore list (see [README](../README.md#ignore-lists)).
- `--gtest_break_on_failure` stops in the debugger at the first failed assertion.

## The Node addon

`launch.json` has entries for the addon's own tests (`LNX Native iModelJsNodeAddon Tests`, `Native debugger for index.js tests`) and for attaching a native debugger to a running Node process (`LNX Native Node Attach`, `Native attach`).

To debug native code called from TypeScript, start the TypeScript side under its debugger, stop at a breakpoint after `IModelHost.startup()` (which loads the addon), then attach the native debugger to that Node process.

### Inside another application

To debug a crash in an application that consumes the published addon:

1. Find the `@bentley/imodeljs-native` version the application uses, in `package-lock.json` or `pnpm-lock.yaml`.
2. Find where that package is installed. With pnpm or rush, follow the symlinks (`realpath`).
3. Check out the tag `v<version>` in both `imodel-native` and `imodel-native-internal`, and build the addon with a debug build.
4. Copy the build output from `$OutRoot/<arch>/imodeljsnodeaddon_pkgs/` over the installed package. `iModelJsNodeAddon/installnativeplatform*.sh` and `.bat` show which folders to copy; they target an itwinjs-core checkout.
5. Debug the application as described above.

## gdb scripting

Continue automatically when the stack contains a function. Prefer a conditional breakpoint; this helps when you cannot filter the stops yourself, for example under Valgrind:

```text
(gdb) python
def handle_stop(e):
  stack = []
  f = gdb.newest_frame()
  while f:
    stack.append(f.name())
    f = f.older()
  if 'MY_FUNCTION_NAME' in stack:
    gdb.execute("continue")

gdb.events.stop.connect(handle_stop)
end
```

Run a script repeatedly until it fails, and stay in gdb on failure:

```sh
for i in $(seq 1 20); do
  echo "Run $i"
  gdb node -ex 'python gdb.events.exited.connect(lambda t: t.exit_code == 0 and gdb.execute("q"))' \
           -ex "run script.js > run_$i.log 2>&1" || echo "failed with exit code: $?"
done
```

## CPU profiling

`perf` profiles native and JavaScript frames of a Node process together. It needs symbols, which a debug build has, but timings of a debug build differ from optimized code.

```sh
# -D 1000 skips the first second, which is mostly module loading.
perf record -F 999 -g -D 1000 node --perf-basic-prof --interpreted-frames-native-stack script.js
perf script > script.perf
rm perf.data
npx speedscope script.perf
```

For JavaScript only, `node --cpu-prof --cpu-prof-name script.cpuprofile script.js` writes a profile that speedscope and Chrome DevTools open.

## Memory

In rising order of effort:

1. **JavaScript heap.** Write a snapshot with `require('v8').writeHeapSnapshot()` (or another method from the [Node guide](https://nodejs.org/en/learn/diagnostics/memory/using-heap-snapshot)) and load it in the Memory tab of Chrome or Edge DevTools. If the total heap size is small compared to the process, the memory is native.
2. **[heaptrack](https://github.com/KDE/heaptrack).** Records every allocation with its call stack. Run Node directly, not through npm, so heaptrack follows the right process: `heaptrack node script.js`, then open the result with `heaptrack_gui heaptrack.node.<pid>.zst`.
3. **strace on mmap.** Catches direct `mmap` calls that heaptrack misses, at a large slowdown:

   ```sh
   strace -k -e trace=mmap,munmap node script.js > out.strace 2>&1
   ```

   This script groups identical stacks and counts them:

   ```python
   import sys

   stacks = {}
   stack = []
   curr_call = ""
   for line in sys.stdin:
       if line.startswith(" "):
           stack.append(line)
           continue
       full_stack = "".join(stack)
       item = stacks.setdefault(full_stack, {"count": 0, "mmap_size": []})
       item["count"] += 1
       if curr_call.startswith("mmap("):
           item["mmap_size"].append(curr_call.split(",")[1].strip())
       stack.clear()
       curr_call = line

   for s, item in sorted(stacks.items(), key=lambda t: t[1]["count"]):
       print(f"found {item['count']}")
       print(f"mmap_sizes: {','.join(item['mmap_size'])}")
       print(s)
   ```

   `python dedup_stacks.py < out.strace > counted_stacks.txt`

4. **Diffing `/proc/<pid>/smaps`.** Shows new or grown memory mappings between two points in time:

   ```sh
   grep -P 'Rss|^[^A-Z]' /proc/$PID/smaps > event1.txt
   # ... let the process run ...
   grep -P 'Rss|^[^A-Z]' /proc/$PID/smaps > event2.txt
   # only mappings of at least 1000 kB
   diff -u --color <(grep -P -B1 '\d{4,} kB' event1.txt) <(grep -P -B1 '\d{4,} kB' event2.txt)
   ```

## Binary size

The addon binary is large. To see what takes the space:

```sh
ADDON=node_modules/@bentley/imodeljs-native/imodeljs-linux-x64/imodeljs.node

# symbols sorted by size, sizes in decimal
readelf -sWC --sym-base=10 $ADDON | sort -n -k 3,3 | less

# total size of all symbols
nm -C --size-sort -S $ADDON | awk '{s+=strtonum("0x"$2)} END {print s}'

# which object files or libraries define symbols matching a pattern
nm -AC $(find $OutRoot -name '*.o' -o -name '*.a') | grep '\<BLE_'
```

## Editor support

The repository has `.clangd` and `compile_flags.txt`, so clangd works without a build. `compile_flags.txt` holds one set of include paths for all files. For exact per-file flags, record a compilation database with [Bear](https://github.com/rizsotto/Bear) while building, and point clangd at the resulting `compile_commands.json`:

```sh
bear --append --output ${SrcRoot}imodel-native/compile_commands.json -- <command behind your bb function> build
```
