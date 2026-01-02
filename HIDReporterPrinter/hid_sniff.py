#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import sys
import time
from typing import Optional, Tuple

import hid


def hexdump(b: bytes) -> str:
    return " ".join(f"{x:02X}" for x in b)


def parse_report_id3(packet: bytes, expect_len: int = 12) -> Optional[str]:
    """
    Parse your descriptor's Report ID=3 layout:
      Byte0: ReportID = 0x03
      Byte1..6: X,Y,Z,Rz,Rx,Ry (int8 each, -127..127)
      Byte7: Hat (uint8, logical 1..8 per your descriptor)
      Byte8..11: Buttons bitfield (uint32 little-endian), Button1 is bit0
    """
    if len(packet) < 1:
        return None
    rid = packet[0]
    if rid != 0x03:
        return None
    if len(packet) != expect_len:
        # still try parse if long enough
        if len(packet) < 12:
            return f"[RID=3] length={len(packet)} < 12 (insufficient) raw={hexdump(packet)}"

    # Ensure at least 12 bytes for parsing
    p = packet[:12]

    # int8 conversion
    def to_i8(x: int) -> int:
        return x - 256 if x >= 128 else x

    axes = [to_i8(x) for x in p[1:7]]
    hat = p[7]

    buttons_u32 = int.from_bytes(p[8:12], byteorder="little", signed=False)

    # list pressed buttons (1..32)
    pressed = [i + 1 for i in range(32) if (buttons_u32 >> i) & 1]

    return (
        f"[RID=3] axes(int8) X={axes[0]} Y={axes[1]} Z={axes[2]} "
        f"Rz={axes[3]} Rx={axes[4]} Ry={axes[5]} | "
        f"hat={hat} | buttons_u32=0x{buttons_u32:08X} pressed={pressed}"
    )


def list_devices(filter_vid: Optional[int], filter_pid: Optional[int], filter_usage_page: Optional[int]) -> None:
    devs = hid.enumerate()
    rows = []
    for d in devs:
        vid = d.get("vendor_id", 0)
        pid = d.get("product_id", 0)
        up = d.get("usage_page", None)
        if filter_vid is not None and vid != filter_vid:
            continue
        if filter_pid is not None and pid != filter_pid:
            continue
        if filter_usage_page is not None and up != filter_usage_page:
            continue
        rows.append(d)

    if not rows:
        print("No matching HID devices found.")
        return

    for i, d in enumerate(rows):
        print(f"--- [{i}] ---")
        print(f"  path        : {d.get('path')!r}")
        print(f"  vid:pid     : {d.get('vendor_id'):04X}:{d.get('product_id'):04X}")
        print(f"  manufacturer: {d.get('manufacturer_string')}")
        print(f"  product     : {d.get('product_string')}")
        print(f"  serial      : {d.get('serial_number')}")
        print(f"  interface   : {d.get('interface_number')}")
        print(f"  usage_page  : {d.get('usage_page')}  usage: {d.get('usage')}")
        print("")


def open_device(
    path: Optional[str],
    vid: Optional[int],
    pid: Optional[int],
    serial: Optional[str],
) -> hid.device:
    h = hid.device()
    if path is not None:
        h.open_path(path.encode() if isinstance(path, str) else path)
        return h

    if vid is None or pid is None:
        raise ValueError("Need --path OR both --vid and --pid")

    # If serial specified, open(vid,pid,serial) works on many platforms.
    # Otherwise open the first match.
    if serial:
        h.open(vid, pid, serial)
        return h

    # deterministic: enumerate and open first matching path
    matches = [d for d in hid.enumerate(vid, pid)]
    if not matches:
        raise RuntimeError(f"No device found for VID:PID {vid:04X}:{pid:04X}")

    h.open_path(matches[0]["path"])
    return h


def sniff(
    h: hid.device,
    read_size: int,
    timeout_ms: int,
    parse_id3: bool,
    expect_len: int,
    max_packets: Optional[int],
) -> None:
    h.set_nonblocking(False)

    count = 0
    t0 = time.time()
    try:
        while True:
            data = h.read(read_size, timeout_ms)
            if not data:
                continue
            packet = bytes(data)

            ts = time.time() - t0
            print(f"{ts:9.3f}s  len={len(packet):2d}  hex={hexdump(packet)}  dec={[b for b in packet]}")

            if parse_id3:
                parsed = parse_report_id3(packet, expect_len=expect_len)
                if parsed:
                    print("          " + parsed)

            count += 1
            if max_packets is not None and count >= max_packets:
                break
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        try:
            h.close()
        except Exception:
            pass


def main():
    ap = argparse.ArgumentParser(
        description="HID raw report sniffer (prints hex + optional parser for ReportID=3 gamepad layout)."
    )
    ap.add_argument("--list", action="store_true", help="List HID devices and exit.")
    ap.add_argument("--vid", type=lambda x: int(x, 16), help="Vendor ID in hex, e.g. 303A")
    ap.add_argument("--pid", type=lambda x: int(x, 16), help="Product ID in hex, e.g. 1001")
    ap.add_argument("--serial", type=str, default=None, help="Serial number filter (optional).")
    ap.add_argument("--path", type=str, default=None, help="Device path from --list output (exact string).")

    ap.add_argument("--usage-page", type=lambda x: int(x, 16), default=None, help="Filter usage_page when listing (hex).")

    ap.add_argument("--read-size", type=int, default=64, help="Max bytes per read().")
    ap.add_argument("--timeout", type=int, default=5000, help="Read timeout in ms.")
    ap.add_argument("--max", type=int, default=None, help="Stop after N packets.")
    ap.add_argument("--parse-id3", action="store_true", help="Parse ReportID=3 as: 6x int8 axes + hat + 32 buttons.")
    ap.add_argument("--expect-len", type=int, default=12, help="Expected length for ReportID=3 packet (default 12).")

    args = ap.parse_args()

    if args.list:
        list_devices(args.vid, args.pid, args.usage_page)
        return

    try:
        h = open_device(args.path, args.vid, args.pid, args.serial)
    except Exception as e:
        print(f"Failed to open device: {e}", file=sys.stderr)
        print("Tip: run with --list first, then use --path or --vid/--pid.", file=sys.stderr)
        sys.exit(1)

    # Print some info if available
    try:
        print("Opened device:")
        print("  manufacturer:", h.get_manufacturer_string())
        print("  product     :", h.get_product_string())
        print("  serial      :", h.get_serial_number_string())
    except Exception:
        pass

    sniff(
        h=h,
        read_size=args.read_size,
        timeout_ms=args.timeout,
        parse_id3=args.parse_id3,
        expect_len=args.expect_len,
        max_packets=args.max,
    )


if __name__ == "__main__":
    main()
