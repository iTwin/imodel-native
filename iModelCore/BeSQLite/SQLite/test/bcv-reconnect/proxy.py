#!/usr/bin/env python3
"""TCP proxy between bcv clients and the daemon (stands in for ztunnel).
SIGUSR1: cut every connection (both sides) at once, like a ztunnel restart.
SIGUSR2: cut only the daemon side now; the client side stays open (and is ignored)
         for HALF_OPEN_SECS seconds, to widen the window in which the client
         has not noticed the drop yet.
SIGHUP:  cut only the client side (RST); the daemon side stays open for
         HALF_OPEN_SECS seconds, so the client reconnects before the daemon
         has seen its old connection hang up.
While the file BLACKHOLE exists, new connections are accepted and then never served."""
import asyncio, os, signal, sys
listen_port, target_port = int(sys.argv[1]), int(sys.argv[2])
HALF_OPEN_SECS = float(os.environ.get("HALF_OPEN_SECS", "3"))
BLACKHOLE = os.environ["BLACKHOLE"]
conns = set()
half = set()

async def pipe(r, w):
    try:
        while True:
            d = await r.read(65536)
            if not d: break
            w.write(d); await w.drain()
    except Exception: pass
    finally:
        if w in half: return
        try: w.close()
        except Exception: pass

async def handle(cr, cw):
    if os.path.exists(BLACKHOLE):
        await asyncio.sleep(3600); return
    try:
        dr, dw = await asyncio.open_connection("127.0.0.1", target_port)
    except Exception:
        cw.close(); return
    c = (cw, dw); conns.add(c)
    await asyncio.gather(pipe(cr, dw), pipe(dr, cw))
    conns.discard(c)

def cut_all():
    print(f"proxy: cutting {len(conns)} connections", flush=True)
    for cw, dw in list(conns):
        for w in (cw, dw):
            s = w.get_extra_info("socket")
            try: s.setsockopt(__import__("socket").SOL_SOCKET, __import__("socket").SO_LINGER, b"\1\0\0\0\0\0\0\0")
            except Exception: pass
            w.close()

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
        s = cw.get_extra_info("socket")
        try: s.setsockopt(__import__("socket").SOL_SOCKET, __import__("socket").SO_LINGER, b"\1\0\0\0\0\0\0\0")
        except Exception: pass
        cw.close()
        asyncio.get_running_loop().call_later(HALF_OPEN_SECS, dw.close)

async def main():
    loop = asyncio.get_running_loop()
    loop.add_signal_handler(signal.SIGUSR1, cut_all)
    loop.add_signal_handler(signal.SIGUSR2, cut_daemon_side)
    loop.add_signal_handler(signal.SIGHUP, cut_client_side)
    srv = await asyncio.start_server(handle, "127.0.0.1", listen_port)
    print(f"proxy {listen_port}->{target_port} pid {os.getpid()}", flush=True)
    async with srv: await srv.serve_forever()
asyncio.run(main())
