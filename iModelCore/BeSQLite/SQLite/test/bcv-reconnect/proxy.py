#!/usr/bin/env python3
"""TCP proxy between bcv clients and the daemon (stands in for ztunnel).
SIGUSR1: cut every connection (both sides) at once, like a ztunnel restart.
SIGUSR2: cut only the daemon side now; the client side stays open (and is ignored)
         for HALF_OPEN_SECS seconds, to widen the window in which the client
         has not noticed the drop yet.
SIGHUP:  cut only the client side (RST); the daemon side stays open for
         HALF_OPEN_SECS seconds, so the client reconnects before the daemon
         has seen its old connection hang up.
SIGQUIT: cut every connection now AND drop the first reply on the next
         reconnect (after forwarding HELLO to the daemon), to test retry
         idempotence when HELLO_REPLY is lost.
While the file BLACKHOLE exists, new connections are accepted and then never served.
When SYN_BLACKHOLE exists (with SYN_BLACKHOLE_MODE=1), the listen backlog is
saturated so new client connect() calls stay in SYN_SENT and hit poll() timeout."""
import asyncio, os, signal, socket, sys
listen_port, target_port = int(sys.argv[1]), int(sys.argv[2])
HALF_OPEN_SECS = float(os.environ.get("HALF_OPEN_SECS", "3"))
BLACKHOLE = os.environ["BLACKHOLE"]
SYN_BLACKHOLE = os.environ.get("SYN_BLACKHOLE", "")
SYN_BLACKHOLE_MODE = os.environ.get("SYN_BLACKHOLE_MODE") == "1"
conns = set()
half = set()
drop_next_reply = 0
syn_fillers = []

def rst_close(w):
    s = w.get_extra_info("socket")
    try: s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, b"\1\0\0\0\0\0\0\0")
    except Exception: pass
    try: w.close()
    except Exception: pass

async def pipe(r, w, drop_first=False):
    try:
        while True:
            d = await r.read(65536)
            if not d: break
            if drop_first:
                print("proxy: dropping first reply and resetting", flush=True)
                rst_close(w)
                return
            w.write(d); await w.drain()
    except Exception: pass
    finally:
        if w in half: return
        try: w.close()
        except Exception: pass

async def handle(cr, cw):
    global drop_next_reply
    if os.path.exists(BLACKHOLE):
        await asyncio.sleep(3600); return
    try:
        dr, dw = await asyncio.open_connection("127.0.0.1", target_port)
    except Exception:
        cw.close(); return
    drop_reply = False
    if drop_next_reply > 0:
        drop_next_reply -= 1
        drop_reply = True
    c = (cw, dw); conns.add(c)
    await asyncio.gather(pipe(cr, dw), pipe(dr, cw, drop_first=drop_reply))
    if drop_reply:
        rst_close(dw)
    conns.discard(c)

def cut_all():
    print(f"proxy: cutting {len(conns)} connections", flush=True)
    for cw, dw in list(conns):
        for w in (cw, dw):
            rst_close(w)

def cut_and_drop_hello_reply():
    global drop_next_reply
    drop_next_reply = 1
    cut_all()

def cut_daemon_side():
    print(f"proxy: half-cutting {len(conns)} connections", flush=True)
    for cw, dw in list(conns):
        half.add(cw)
        dw.close()
        asyncio.get_running_loop().call_later(HALF_OPEN_SECS, cw.close)

def cut_client_side():
    print(f"proxy: client-cutting {len(conns)} connections", flush=True)
    for cw, dw in list(conns):
        half.add(dw)
        rst_close(cw)
        asyncio.get_running_loop().call_later(HALF_OPEN_SECS, dw.close)

async def watch_syn_blackhole(srv_ref):
    blk_sock = None
    while True:
        want = bool(SYN_BLACKHOLE and os.path.exists(SYN_BLACKHOLE))
        if want and blk_sock is None:
            srv_ref[0].close()
            await srv_ref[0].wait_closed()
            blk_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            blk_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            blk_sock.bind(("127.0.0.1", listen_port))
            blk_sock.listen(0)
            for _ in range(4):
                c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                c.setblocking(False)
                c.connect_ex(("127.0.0.1", listen_port))
                syn_fillers.append(c)
            print("proxy: syn-blackhole active", flush=True)
        elif not want and blk_sock is not None:
            for c in syn_fillers:
                try: c.close()
                except Exception: pass
            syn_fillers.clear()
            blk_sock.close()
            blk_sock = None
            srv_ref[0] = await asyncio.start_server(
                handle, "127.0.0.1", listen_port, reuse_address=True
            )
            print("proxy: syn-blackhole cleared", flush=True)
        await asyncio.sleep(0.05)

async def main():
    loop = asyncio.get_running_loop()
    loop.add_signal_handler(signal.SIGUSR1, cut_all)
    loop.add_signal_handler(signal.SIGUSR2, cut_daemon_side)
    loop.add_signal_handler(signal.SIGHUP, cut_client_side)
    loop.add_signal_handler(signal.SIGQUIT, cut_and_drop_hello_reply)
    srv_ref = [await asyncio.start_server(
        handle, "127.0.0.1", listen_port, reuse_address=True
    )]
    if SYN_BLACKHOLE_MODE:
        asyncio.create_task(watch_syn_blackhole(srv_ref))
    print(f"proxy {listen_port}->{target_port} pid {os.getpid()}", flush=True)
    await asyncio.Future()
asyncio.run(main())
