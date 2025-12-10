#!/usr/bin/env python3
"""
LoRa packet decoder for TTGO T-Beam sniffer output.

Usage:
  python decoder.py --hex "79 02 FF 01 01 FF FB 01 00 00 02 06 4E 6F 64 65 20 31"
  python decoder.py --json '{"channel": 868.1, "payload_hex": "79 02 FF ..."}'
"""

import argparse
import json
from typing import Dict, List, Tuple, Union

COMMON_HEADER_LEN = 11


def parse_hex_string(hex_str: str) -> List[int]:
    """Parse a space-separated hex string into a list of byte values."""
    hex_str = hex_str.replace("0x", " ")  # allow optional 0x prefixes
    parts = [p for p in hex_str.replace(",", " ").split() if p]
    return [int(p, 16) for p in parts]


def parse_header(data: List[int]) -> Dict[str, Union[int, str]]:
    if len(data) < COMMON_HEADER_LEN:
        raise ValueError(f"Packet too short for header: {len(data)} bytes")

    header = {
        "netid": data[0],
        "ttl": data[1],
        "receiver": data[2],
        "sender": data[3],
        "last_node": data[4],
        "next_node": data[5],
        "id": int.from_bytes(bytes(data[6:10]), byteorder="little"),
        "type": data[10],
    }

    if header["ttl"] <= 0:
        raise ValueError("Invalid TTL (must be > 0)")

    return header


def decode_payload(payload_type: int, data: List[int]) -> Tuple[str, Dict[str, Union[int, str, bytes]]]:
    if payload_type == 0:  # payload_msg
        if not data:
            raise ValueError("Missing message_size for type 0")
        msg_size = data[0]
        if len(data) < 1 + msg_size:
            raise ValueError(f"Payload shorter than message_size ({msg_size})")
        msg_bytes = bytes(data[1:1 + msg_size])
        return "payload_msg", {
            "message_size": msg_size,
            "message_raw": msg_bytes,
            "message_utf8": msg_bytes.decode("utf-8", errors="replace"),
        }

    if payload_type == 1:  # payload_ack
        if len(data) < 4:
            raise ValueError("Ack payload requires 4 bytes for packet_id")
        pkt_id = int.from_bytes(bytes(data[:4]), byteorder="little")
        return "payload_ack", {"packet_id": pkt_id}

    if payload_type == 2:  # payload_ann
        if not data:
            raise ValueError("Missing name_size for type 2")
        name_size = data[0]
        if len(data) < 1 + name_size:
            raise ValueError(f"Payload shorter than name_size ({name_size})")
        name_bytes = bytes(data[1:1 + name_size])
        return "payload_ann", {
            "name_size": name_size,
            "name_raw": name_bytes,
            "name_utf8": name_bytes.decode("utf-8", errors="replace"),
        }

    raise ValueError(f"Unknown payload type: {payload_type}")


def decode_packet_bytes(data: List[int]) -> Dict[str, Union[int, str, Dict[str, Union[int, str, bytes]]]]:
    header = parse_header(data)
    payload_type = header["type"]
    payload_bytes = data[COMMON_HEADER_LEN:]
    payload_name, payload = decode_payload(payload_type, payload_bytes)

    # Interpret special values
    def hop_label(value: int) -> str:
        return "broadcast" if value == 0xFF else str(value)

    return {
        "header": {
            **header,
            "receiver_label": hop_label(header["receiver"]),
            "next_node_label": hop_label(header["next_node"]),
        },
        "payload_type": payload_name,
        "payload": payload,
        "payload_len": len(payload_bytes),
    }


def decode_from_hex(hex_str: str) -> Dict[str, Union[int, str, Dict[str, Union[int, str, bytes]]]]:
    return decode_packet_bytes(parse_hex_string(hex_str))




def main():
    parser = argparse.ArgumentParser(description="Decode TTGO LoRa sniffer packets")
    parser.add_argument("--hex", help="Space-separated hex payload (payload_hex)")
    args = parser.parse_args()

    if not args.hex:
        parser.error("Provide --hex")

    try:
        decoded = decode_from_hex(args.hex)
    except Exception as exc: 
        print(f"Decode error: {exc}")
        return

    # Pretty-print
    print(json.dumps(decoded, indent=2, default=lambda b: b.hex()))


if __name__ == "__main__":
    main()
